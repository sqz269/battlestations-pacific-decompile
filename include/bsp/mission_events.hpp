#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/world_entities.hpp"

// Packet mission_events_update. The object the in-mission world tick runs at
// game+21E0h through 00987590 is the WarningManager, the warning and radio
// chatter director, not an objective or win/lose director. The class name comes
// from three literals on the same object: "WarningManager Report Error" and
// "WarningManager unknown message id: %s" in 009763e0 and
// "---WarningManager invalid Message--- %d" in 009781e0. See
// docs/MISSION_EVENTS_UPDATE.md for the evidence behind every offset below.
//
// The per-frame queue walk itself is NOT redeclared here:
// bsp::update_mission_events_00987590 in bsp/world_entities.hpp already owns it,
// and kMissionEventTickPeriod is reused from that header.
namespace bsp {

// ---------------------------------------------------------------------------
// Constants read from the listing
// ---------------------------------------------------------------------------

// 00ce3d34, 40800000h. Also the mission-start grace the report helpers apply to
// the clock 00f876a4 before they will raise anything (00977050).
inline constexpr float kWarningReportGraceSeconds = kMissionEventTickPeriod;

// 00d7a308, the double 2.0. Held by 00977990 as the proximity latch and by
// 00976f10 as the cancel cooldown.
inline constexpr float kWarningProximityHoldSeconds = 2.0f;

// 00d09fe8, the double 4.0e6: a squared distance, so a 2000 m radius (00977990).
inline constexpr float kWarningProximityRadiusSquared = 4.0e6f;

// 00cf0dd8, the double 2000.0, and 00ce3868, 0.25f. 005bbdc0 forms
// (kVoiceAudibleRange - distance) / kVoiceAudibleRange. For finite inputs the
// admission gate includes 1500 m. The exact branch ordering, including masked
// NaN admission, lives in voice_playback.cpp; this value is not playback volume.
inline constexpr float kVoiceAudibleRange = 2000.0f;
inline constexpr float kVoiceMinimumAttenuation = 0.25f;

// Pushed to 004c43c0 at 0096d540. The action table entry is not identified, so
// this is a raw index rather than a named control.
inline constexpr int kWarningPromptInputAction = 3;

// [game+1ED4h]->vtable[5Ch] takes this before [game+1ED4h]+390h is set (0096d540).
inline constexpr int kWarningPromptHudSlot = 0x54;

// The Warning record is operator_new(0x84) in 00977050 and its siblings, and the
// WarningManager is operator_new(0x1B0) at 004dc8b3.
inline constexpr std::size_t kWarningRecordSize = 0x84;
inline constexpr std::size_t kWarningManagerSize = 0x1B0;

// Virtual slot offsets on a Warning, established by use. Slots 1, 2 and 9 upward
// were not observed.
inline constexpr int kWarningVtableDestructor = 0x00; // void(int), always 1
inline constexpr int kWarningVtableOnAccepted = 0x0C; // void()
inline constexpr int kWarningVtableKind = 0x10; // int()
inline constexpr int kWarningVtableMatches = 0x14; // bool(Warning*)
inline constexpr int kWarningVtablePriority = 0x18; // float()
inline constexpr int kWarningVtablePendingLifetime = 0x1C; // float()
inline constexpr int kWarningVtableAppliedLifetime = 0x20; // float()

// 00976f10 cancels only warnings whose vtable[4]() reports this kind.
inline constexpr int kWarningKindTargeted = 4;

// ---------------------------------------------------------------------------
// The named event channels
// ---------------------------------------------------------------------------

// The sixteen kinds 0097e360 parses out of a Lua event block. These are the
// literals in its body, not a recovered enumeration.
inline constexpr const char* kWarningEventKinds[] = {
    "ammoType", "command", "entityKilled", "exitzone", "failure", "generate",
    "hpEvent", "input", "musicOver", "player", "recon", "repair", "shipLanded",
    "stock", "surrender", "target",
};
inline constexpr std::size_t kWarningEventKindCount = 16;

// One row per channel-dispatch routine in segment 65, keyed by the literal that
// routine looks up in the map at manager+F8h.
struct WarningChannelDispatch {
    const char* name;
    std::uint32_t dispatcher; // the segment-65 routine that looks the name up
};
const WarningChannelDispatch* warning_channel_dispatches() noexcept;
std::size_t warning_channel_dispatch_count() noexcept;

// Case-insensitive, matching BSP_NativeString_LessCaseInsensitive (00443d00),
// which is the comparator the map at manager+F8h is built with (00980150).
const WarningChannelDispatch* find_warning_channel(const char* name) noexcept;

// ---------------------------------------------------------------------------
// Records
// ---------------------------------------------------------------------------

// Projection of the 84h-byte Warning. Only the fields the director and the
// report path touch are recovered; the rest of the record is not modelled.
struct WarningRecord {
    std::string message_id{}; // +4h size, +8h char*
    int state{0}; // +Ch, 1 queued and 2 applied
    float applied_at{0.0f}; // +10h, the clock 00f876a4 written by 00974070
    std::size_t clip_count{0}; // (+1Ch - +18h) / 0Ch, the sound clip vector at +14h
    std::uint32_t flat_speaker{0}; // +68h, 005bbc10's fourth argument
    std::uint32_t positional_speaker{0}; // +50h, non-zero selects 005bbdc0
    std::uint32_t target_id{0}; // +80h, matched by 00976f10
    int kind{0}; // vtable[4]()
    float priority{0.0f}; // vtable[6]()
    float pending_lifetime{0.0f}; // vtable[7]()
    float applied_lifetime{0.0f}; // vtable[8]()
};

// A Lua handler registered on one channel. The subscription object's own class is
// not recovered: +4h as the callback name comes from 0097b8c0 and 00982540, and
// the nested per-action list at +10h from 00982540 alone.
struct WarningSubscription {
    std::string callback{}; // subscription+4h
    bool condition_holds{false}; // subscription->vtable[3](params)
    std::vector<int> input_actions{}; // the nested list at +10h, id at entry+0Ch
};

// Projection of the WarningManager fields this packet establishes.
struct WarningManagerState {
    float periodic_accumulator{0.0f}; // +198h
    bool suppressed{false}; // +D0h
    float air_raid_deadline{0.0f}; // +174h
    bool air_raid_active{false}; // +178h
    float collision_deadline{0.0f}; // +17Ch
    bool collision_active{false}; // +180h
    std::string prompt_callback{}; // +19Ch
    std::vector<WarningRecord> pending{}; // the +E0h list
    std::vector<WarningRecord> applied{}; // the +ECh list
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 00987677 on the +E0h queue and the matching test in 009763e0: FCOMIP of
// applied_at against now - lifetime, then JBE into the retire path, so the
// comparison is not-greater-than. 009763e0 uses the applied lifetime, vtable[8],
// when it walks the +ECh queue instead.
bool warning_is_expired(float now, float applied_at, float lifetime) noexcept;

// 009876c9: FCOMIP of the candidate's priority against the held one's, then JBE
// back to the loop head. Strictly greater wins, so ties keep the earlier entry.
bool warning_outranks(float candidate_priority, float held_priority) noexcept;

// 009875a7..009875d9. The accumulator takes the scaled delta and the periodic
// pass fires only on strictly greater than 4.0f; 009875d1 then stores zero
// rather than subtracting, so the overshoot is discarded and the period is a
// floor. Returns whether 00977990 runs and writes the new accumulator back.
bool advance_warning_periodic(float& accumulator, float scaled_delta) noexcept;

// 00968550. Both tests are a strict less-than against the clock, so a flag
// clears on the first frame past its deadline. The collision flag additionally
// drives a virtual call, reported through the return value.
struct WarningDeadlineResult {
    bool air_raid_expired{false};
    bool collision_expired{false}; // true means [[[00e198c4]+C4h]+60h]->vtable[34h](0) ran
};
WarningDeadlineResult update_warning_deadlines(WarningManagerState& state, float now) noexcept;

// 005b71d0. ECX is the voice manager at [[00e198c4]+A4h] and the stack argument
// is warning+14h; the decompiler prints the call with one argument and loses the
// ECX, so the order here follows the listing. Every clause is a gate: a false
// from any of them is the return value.
// The stateful readiness and positional playback sequences are declared in
// voice_playback.hpp. 005B7251/54 checks ONE slot, and 007027B0 can mutate it;
// a precomputed busy-slot count cannot represent the native per-clip calls.

// 00977990's distance test, which compares the squared separation against the
// squared radius without a square root.
bool warning_within_proximity(float dx, float dy, float dz) noexcept;

// 00977050's clock guard. The same 4.0f the accumulator uses doubles as a
// mission-start grace, so nothing is reported in the first four seconds.
bool warning_report_clock_allows(float now) noexcept;

// ---------------------------------------------------------------------------
// Host boundary
// ---------------------------------------------------------------------------

// One method per native call site, in the style of bsp::ApplicationFrameHost.
// There are no default implementations: nothing here stands in for unrecovered
// game behaviour.
struct WarningManagerHost {
    virtual ~WarningManagerHost() = default;

    // The two per-frame calls 00987590 makes whose bodies are not modelled here,
    // both with ECX = the manager. 0096d540 and 00968550 are modelled instead.
    virtual void pump_input_channel_00982540() = 0; // 009875a2
    virtual void scan_proximity_00977990() = 0; // 009875d9

    // 0096d540's body, split at its native call sites.
    virtual bool input_action_pressed(int action) = 0; // 004c43c0
    virtual void raise_prompt_hud(int slot) = 0; // [game+1ED4h] vtable +5Ch then +390h = 1
    virtual void call_mission_lua(const std::string& name) = 0; // 00887e50

    // 00968550's collision branch.
    virtual void stop_collision_sound() = 0; // [[[00e198c4]+C4h]+60h] vtable +34h with 0

    // 009763e0's decision points.
    virtual bool message_id_known(const std::string& id) = 0; // the +108h table, 0096dbb0
    virtual void report_error(const char* text) = 0; // 004254b0
    virtual void destroy_warning(std::size_t index, bool pending) = 0; // vtable[0](1)
    virtual void on_warning_accepted(std::size_t index) = 0; // vtable[3]()
    virtual bool voice_ready(std::size_t index) = 0; // 005b71d0 on the +E0h entry
    virtual void apply_warning_00974070(std::size_t index) = 0; // 0098773d and 009763e0
};

// 009763e0's error literals, as they appear in the image.
inline constexpr const char* kWarningReportErrorText = "WarningManager Report Error";
inline constexpr const char* kWarningUnknownIdFormat = "WarningManager unknown message id: %s";

// What one 009763e0 call did. Exactly one of these is true on any path that
// reaches the queue; an empty message id stops before the walk.
enum class WarningReportOutcome {
    RejectedEmptyId, // warning+4h == 0
    RetiredExpired, // one expired entry was destroyed and the call returned
    SuppressedDuplicate, // an entry's vtable[5] matched; the incoming warning died
    RejectedUnknownId, // the id missed the +108h table
    Queued, // accepted, inserted, left for the next frame
    QueuedAndApplied, // accepted and applied at once, warning+50h set and voice ready
};

struct WarningReportResult {
    WarningReportOutcome outcome{WarningReportOutcome::RejectedEmptyId};
    // Which queue entry the call touched: the one it retired, or the one whose
    // timestamp it refreshed. kNoMissionEvent when neither happened.
    std::size_t touched{kNoMissionEvent};
    bool touched_applied_queue{false}; // the +ECh walk rather than the +E0h walk
};

// 009763e0, __thiscall(this, Warning*). Walks the pending queue then the applied
// queue; the first expired entry is retired and the call returns, and the first
// entry whose duplicate test matches refreshes its own timestamp and destroys the
// incoming warning. Only when both walks fall through is the message id looked up
// and the warning queued.
WarningReportResult report_warning_009763e0(
    WarningManagerState& state, WarningManagerHost& host, const WarningRecord& incoming,
    float now);

// 0096d540, __fastcall(this). Returns whether the callback was dispatched; on a
// dispatch the pending name is cleared with the empty literal at 00ce3a0c.
bool poll_warning_prompt_0096d540(WarningManagerState& state, WarningManagerHost& host);

// 0097b8c0, __cdecl(channel, params). Collects the callback name of every
// subscription whose condition holds, in list order.
std::vector<std::string> evaluate_warning_channel_0097b8c0(
    const std::vector<WarningSubscription>& subscriptions);

// 00982540's inner test, applied to one subscription: an action fires when any of
// its nested entries names an action the frame reports pressed.
bool subscription_input_fires(
    const WarningSubscription& subscription, const std::vector<int>& pressed_actions) noexcept;

// The three calls 00987590 makes before the queue walk, in listing order:
// 00982540, then 00977990 when the accumulator crossed, then 0096d540 and
// 00968550. The queue walk that follows is bsp::update_mission_events_00987590.
struct WarningManagerTickResult {
    bool periodic_fired{false};
    bool prompt_dispatched{false};
    WarningDeadlineResult deadlines{};
};
WarningManagerTickResult run_warning_manager_update(
    WarningManagerState& state, WarningManagerHost& host, float scaled_delta, float now);

} // namespace bsp
