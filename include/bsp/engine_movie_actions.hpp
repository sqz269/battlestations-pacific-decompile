#pragma once

// Engine-movie (.ema) actions: the action type table built by 00460B80, the
// `Action { ... }` block reader 0045E5E0 and the parameter decisions of the run
// bodies that reduce to data. Evidence: docs/CC13_SCENE_BROWSER.md ("Engine-movie
// actions"). Every name is a hypothesis; nothing here is wired into a game host.

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace bsp {

// One 58h-byte record of the vector at 00E18550, built by 0045D480 from eight
// strings and six flags. A field is read by 0045E5E0 when its flag is set; in
// every record a flag is set exactly when its label is non-empty, which is how
// this table stores it.
struct MovieActionTypeInfo {
    const char* name;            // record +0
    const char* refer_label;     // +8, flag +10h: ReferPath -> action +18h
    const char* num_labels[3];   // +14h/+20h/+2Ch, flags +1Ch/+28h/+34h: Num1..3 -> +20h..+28h
    // Member order is not native order. 0045D480 gets the label as its sixth
    // string and stores it at +38h (0045D5E9/0045D5ED), then gets the '|'
    // options as its seventh and stores them at +40h (0045D620/0045D624).
    const char* enum_options;    // +40h, the '|' separated choices
    const char* enum_label;      // +38h, e.g. "Type:"; flag +48h: Enum -> action +2Ch
    const char* offset_label;    // +4Ch, flag +54h: OffsetX/Y/Z -> +34h/+3Ch/+38h
    bool constructible;          // 0045DE50 has a case for the index
};

// 00460B80, in push order (00460AD0 calls 00460E37..00464285). Index 4 is built
// from empty strings only and has no factory case.
extern const std::array<MovieActionTypeInfo, 32> kMovieActionTypes;

// 0045E7A0 loop: the first record whose name equals `type` (_stricmp; an empty
// record name matches only a null token). Native has no miss path: after 32
// records it reads the type from a null action. The host returns nullopt.
std::optional<std::int32_t> find_movie_action_type_0045e5e0(std::string_view type);

// The token reader calls of 0045E5E0, in order. 008D9930 expect, 008D9980
// read, 008D9B40 read float and the quoted-string reader used for ReferPath and
// Enum.
struct MovieActionTokens {
    virtual ~MovieActionTokens() = default;
    virtual void expect(const char* token) = 0;
    virtual std::string read_token() = 0;
    virtual std::string read_nonempty_string() = 0;
    virtual float read_float() = 0;
};

// Host view of the 44h-byte action (48h for aParticleEffect).
struct MovieAction {
    std::int32_t type{};             // +10h, written by each constructor
    float activate_time{};           // +04h
    std::string comment;             // +08h, the Comment token
    std::string refer_path;          // +18h
    std::array<float, 3> num{};      // +20h, +24h, +28h
    std::string enum_text;           // +2Ch
    float offset_x{};                // +34h
    float offset_z{};                // +38h (third float read)
    float offset_y{};                // +3Ch (second float read)
};

// Address: 0045E5E0 (body 0045E5E0..0045EC96). Original ABI: __thiscall,
// ECX = owner (kept at [ESP+18h], the target of the 0045A210 append),
// [ESP+4] = tokenizer (EBX, ECX of every tokenizer call), [ESP+8] not traced;
// RET 8. This host form takes the tokenizer only. Reads
//   Action { Type : <name> ActivateTime : <f> Comment : <tok>
//            [ReferPath : "s"] [Enum : "s"] [Num1 : f] [Num2 : f] [Num3 : f]
//            [OffsetX : f OffsetY : f OffsetZ : f] }
// where each bracketed group is read only when the type record's flag is set
// (each keyword is followed by one read_token for the ':'). Unknown types are
// nullopt (native dereferences a null action). The factory 0045DE50 and the
// list append 0045A210 are not modelled.
std::optional<MovieAction> parse_movie_action_0045e5e0(MovieActionTokens& tokens);

// _stricmp / BSP_NativeString_EqualsCStringInsensitive against the Enum text.
bool movie_action_enum_is(const MovieAction& action, const char* value);

// Address: 00465290 (aSetGunFireEnabled run) and 00465350 (aSetGunFireDisabled).
// Both act only on an entity answering IsKindOf(5) and update the dword at
// unit+634h, a mask of disabled weapon categories: AA_Flak 1, Artillery 2,
// Torpedo 4, DC 8. Enabled with All stores 0 to the low byte; Disabled with All
// ORs FFh. Other Enum values leave the mask unchanged.
std::uint32_t movie_gun_fire_enabled_mask_00465290(std::uint32_t disabled_mask,
                                                   const MovieAction& action);
std::uint32_t movie_gun_fire_disabled_mask_00465350(std::uint32_t disabled_mask,
                                                    const MovieAction& action);

// Address: 00464A30 (pSetGear run). Auto -> 007B8CE0(0, 0); Down -> (1, 1);
// Up -> (2, 1); anything else makes no call. 007B8CE0 stores the mode at
// plane+C08h and, for a non-zero mode with the flag, plane+C1Ch = (mode == 1).
std::optional<std::pair<std::int32_t, bool>> movie_set_gear_request_00464a30(const MovieAction& action);

// Address: 004660B0 (cBlackOut run): BSP_HudNarrativeScreen_ArmBlackoutFade
// (005B9BA0) with (level = Num1, duration = Num2, empty callback); see
// include/bsp/mission_blackout.hpp for the arm.
std::pair<float, float> movie_blackout_level_duration_004660b0(const MovieAction& action);

// The movie object's clock (74h-byte object from 00460B80, kept at game+38h):
// length +0 (Movielen), time +4, a 20-sample history of frame deltas +8..+54h,
// state +60h (1 started by 0045DB40, 2 half speed, 0 and 3 idle, 5 failed load).
struct EngineMovieClock {
    float length{};
    float time{};
    std::array<float, 20> history{};
    std::int32_t state{};
};

// Address: 0045C5B0 (the clock part). __fastcall, ECX = movie, RET. Calls every
// track's vtable+8 (not modelled), then zeroes +4..+54h: time and history.
void rewind_engine_movie_clock_0045c5b0(EngineMovieClock& clock) noexcept;

enum class EngineMovieStep { Idle, Running, Finished };

// Address: 0045F970. __thiscall, ECX = movie, [ESP+4] = float frame delta,
// RET 4; caller BSP_Game_UpdateCutscenePlayback (004C6B20). A delta above 1.0
// becomes 0.01f (00D7A238). States 0 and 3 return. State 2 halves the delta
// (FMUL double 0.5). The step is (history sum + delta) / 21.0 (00CE4D78): the
// sum is accumulated left to right through a float temporary, the add and
// divide run on the x87 stack and are stored as float. The history shifts left
// and takes the (scaled) delta; time = float(time + step). time >= length ->
// Finished (0045F600, the stop/teardown); otherwise Running, and the caller
// runs 0045C560: every track's vtable+0Ch(time).
EngineMovieStep advance_engine_movie_clock_0045f970(EngineMovieClock& clock, float delta) noexcept;

// Address: 004560E0. __thiscall, ECX = track, [ESP+4] = float time, RET 4;
// tail of the entity track's update 00459020 (vtable+0Ch of 00CE4DB8). The
// track keeps its actions in a std::list (+108h) with a cursor (+144h). From
// the cursor, while an action's ActivateTime (+4) is below `time`: run it
// (action vtable slot 0 with the track's entity +11Ch) when the entity exists
// and its bytes +5Eh and +5Dh are clear, then advance the cursor whether or
// not it ran. Returns the new cursor.
std::size_t run_due_movie_actions_004560e0(const std::vector<MovieAction>& actions,
                                           std::size_t cursor, float time, bool entity_ready,
                                           const std::function<void(const MovieAction&)>& run);

} // namespace bsp