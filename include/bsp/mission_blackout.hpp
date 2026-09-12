#pragma once
#include <cstdint>
#include <string>

#include "bsp/interface_runtime_tail.hpp"

// The `Blackout` mission Lua binding, the fade it arms, the fade's per-frame
// update, and the completion callback that re-enters the mission script by name
// (packet cc_mission_blackout). Evidence, ABI and uncertainty:
// docs/MISSION_BLACKOUT.md.
//
// The fade lives in the HUD narrative screen, registry slot 33h, reached as
// `*(00E198C4 + A4h)`. `bsp/interface_runtime_tail.hpp` already established that
// screen (constructor 005BA7B0, vtable 00CF0ED8, E8h bytes), its `Blackout_Icon`
// widget at `+BCh` and the widget's set-visible `+34h` and set-colour `+50h`
// virtuals; none of that is repeated here. What this header adds is the five
// fields above the widget pointer and the three routines that drive them:
//
//   008D1340  the `Blackout` binding: decodes four Lua arguments, calls 005B9BA0
//   005B9BA0  arm: writes target, remaining and the callback name, then steps once
//   005B9800  update: blends the level, fires the callback, paints the widget
//
// The per-frame caller is `BSP_Game_UpdateInterfaceOnly` 004C40F0 at 004C429A,
// which calls `BSP_FrontEndScreen_Update` 004F71F0 on `*(00E198C4 + A4h)` with
// the scaled frame delta `game+21F0h`; that dispatches the screen's vtable +20h,
// which for vtable 00CF0ED8 is 005BC920, and 005BC920 calls 005B9800 at 005BC9FC.
//
// Names are hypotheses, not recovered symbols. Nothing here is binary compatible
// with the original.
namespace bsp {

// ---------------------------------------------------------------------------
// The five fields, offsets from the slot 33h screen
// ---------------------------------------------------------------------------

// +C0h current level, +C4h target level, +C8h seconds remaining, +CCh/+D0h the
// callback name as a NativeString {size, data}. 005BA7B0 zeroes +CCh and +D0h at
// 005BA839/005BA83F and writes none of +C0h, +C4h or +C8h; see the uncertainty
// note in docs/MISSION_BLACKOUT.md.
inline constexpr std::uint16_t kHudNarrativeFadeLevelOffset = 0x00C0;
inline constexpr std::uint16_t kHudNarrativeFadeTargetOffset = 0x00C4;
inline constexpr std::uint16_t kHudNarrativeFadeRemainingOffset = 0x00C8;
inline constexpr std::uint16_t kHudNarrativeFadeCallbackOffset = 0x00CC;

// The widget virtual 005B9800 uses that 005B6960 does not: +54h fills a colour
// from the widget at 005B99C9, the level is written into its alpha, and the
// +50h setter named in bsp/interface_runtime_tail.hpp writes it back at 005B9A3B.
inline constexpr std::uint16_t kGuiWidgetGetColourVirtual = 0x0054;

// ---------------------------------------------------------------------------
// The constants the three routines read
// ---------------------------------------------------------------------------

// 00D7A268, a double, `FADD double ptr` at 005B9BAA onto the float duration. The
// eight bytes are 00 00 00 E0 E2 36 1A 3F, which is 1.0e-4. A zero-duration arm
// therefore still leaves a positive remaining, and the single arm-time step below
// consumes exactly that much, so `Blackout(x, cb, 0)` completes on the arm.
inline constexpr double kMissionBlackoutArmEpsilon = 1.0e-4;
// 00CE3C68, a float (17 B7 D1 38 = 1.0e-4), the step 005B9BF5 passes to 005B9800
// immediately after arming.
inline constexpr float kMissionBlackoutArmStep = 1.0e-4f;
// 00D7A24C, 1.0f: the level when the Lua call omits argument 4.
inline constexpr float kMissionBlackoutDefaultLevel = 1.0f;
// 00D7A218, eight zero bytes: the `level <= 0` and `remaining > 0` comparands.
inline constexpr float kMissionBlackoutZero = 0.0f;
// 00D7A280, a double (0.5), and 00CEE07C, a float (0.75). Both are read only on
// the `game+1FE4h != 0` arm of the paint tail at 005B99ED..005B9A20.
inline constexpr double kMissionBlackoutSessionAlphaScale = 0.5;
inline constexpr float kMissionBlackoutSessionForceShowLevel = 0.75f;
// 005B989F pushes 20h as the interface id when a fade to a positive level ends.
inline constexpr int kMissionBlackoutCompletionInterfaceRequest = 0x20;
// 0076D310 builds a session message of this kind from the level and the duration.
inline constexpr int kMissionBlackoutSessionMessageKind = 0x2B;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

// The fade, as the five fields hold it. `callback` is the NativeString at +CCh;
// empty means "no callback armed", which is how 005B98E7 tests it (it compares
// against the empty literal at 00CE3A0C, case-insensitively, through 00449AF0).
struct MissionBlackoutFade {
    float level{0.0f};      // +C0h, what the widget's alpha is painted from
    float target{0.0f};     // +C4h
    float remaining{0.0f};  // +C8h, seconds
    std::string callback;   // +CCh size, +D0h data
};

// The `Blackout` call frame as 008D1340 reads it. The native reads argument 1 as
// a boolean unconditionally and the rest only when the result count 00B663F0
// reports is large enough, so the count is part of the input.
struct MissionBlackoutLuaCall {
    int argument_count{0};
    bool enable{false};             // argument 1, 00B66250 at 008D143E, always read
    std::string callback;           // argument 2, 00B662B0 at 008D1490, read when count > 1
    bool duration_is_boolean{false};  // argument 3 kind, 00B66000 at 008D1521
    bool duration_boolean{false};     // argument 3 when it is a boolean, 00B66250 at 008D1557
    float duration_number{0.0f};      // argument 3 when it is a number, 00B66270 at 008D1598
    float level_number{0.0f};         // argument 4, 00B66270 at 008D15E8, read when count > 3
};

// What 008D1340 hands 005B9BA0 at 008D1635: the stack order is (level, duration,
// &name) with the screen in ECX.
struct MissionBlackoutArgs {
    float level{0.0f};
    float duration{0.0f};
    std::string callback;
};

// What one 005B9800 step did, for the caller's log. The native returns only
// `remaining > 0` in EAX (005B9A50 / 005B9A6A); the rest is this process's record.
struct MissionBlackoutStep {
    bool callback_fired{false};   // the 005B9933 arm ran
    std::string callback;         // the name it called, before +CCh was cleared
    bool reached_target{false};   // the 005B9842 arm ran this step
    bool still_fading{false};     // EAX, `remaining > 0` at 005B9A45
};

// ---------------------------------------------------------------------------
// Host: one pure-virtual per native call site the three routines make
// ---------------------------------------------------------------------------

// The widget half is `bsp::BlackoutOverlayHost` from bsp/interface_runtime_tail.hpp;
// this extends it with the getter 005B9800 also uses and with the six non-widget
// call sites. Every method names its call site, not its argument list.
struct MissionBlackoutHost : BlackoutOverlayHost {
    // Widget vtable +54h at 005B99C9, __thiscall(widget, float(&)[4]). Fills the
    // colour whose alpha the level replaces.
    virtual void blackout_icon_get_colour(BlackoutFillColour& colour) = 0;

    // 00887E50 at 005B9969, __thiscall(*(00E188A8)+1A08h, 0, &name, 0, 0, -1).
    // Self key 0, no arguments and no stack range, so this is `_G[name]()`.
    virtual void mission_lua_call_named_00887e50(const std::string& name) = 0;

    // 00E188D8 read at 005B9867 and 005B9893: the local player unit, or null.
    virtual void* local_player_unit_00e188d8() = 0;

    // 005B66D0 at 005B987C, __fastcall(00E198C4). Its body reads the manager's
    // pending interface id at +20h and answers true for 29h, 2Bh, 2Ch and 2Dh.
    virtual bool interface_request_pending_005b66d0() = 0;

    // 00644220 at 005B988E, __thiscall(*(00E198C4 + 40h), int). Its whole body is
    // `[this+1Ch] = argument`; the call site passes 0.
    virtual void ingame_interface_store_1c_00644220(int value) = 0;

    // 004CC460 at 005B98A1, __thiscall(00E198C4, int id, void* payload). The id is
    // 20h and the payload is the local player unit.
    virtual void push_interface_request_004cc460(int request_id, void* payload) = 0;

    // `*(00E188A8) + 1FE4h`, read at 005B99DE in the paint tail and at 005B9C0C in
    // the arm. Zero in an offline mission.
    virtual int game_session_kind_1fe4h() = 0;

    // 0076D310 at 005B9C36, __thiscall(*(00E188A8) + 1EF0h, float, float). Builds
    // session message 2Bh from (level, duration) and broadcasts it through
    // 00784790. Reached only when `game+1FE4h` is exactly 1.
    virtual void session_broadcast_blackout_0076d310(float level, float duration) = 0;

    // 004F83B0 then vtable +18h on `*(00E19698)` at 005B9A14/005B9A20, after both
    // of that screen's visibility bytes are set. Only on the session arm of the
    // paint tail, and only while the level is above 0.75f.
    virtual void force_show_please_wait_screen_00e19698() = 0;
};

// ---------------------------------------------------------------------------
// Rules
// ---------------------------------------------------------------------------

// 005B98AB..005B98D5, read from the x87 sequence rather than the pseudocode:
// FLD +C0h, FLD +C4h, FSUBP, FMULP by the step, FDIVP by remaining, FADDP.
// `level + step * (target - level) / remaining`, evaluated in x87 and stored back
// as a float32.
float mission_blackout_blend_005b98ab(float level, float target, float remaining,
    float step) noexcept;

// 008D1340's argument decode, 008D142F..008D1612. `configured_duration` is
// `*(float*)(00432650() + E0h)`, the global-config default the native reads twice
// (008D14E2 and the boolean-false arm at 008D156B).
MissionBlackoutArgs mission_blackout_decode_008d1340(const MissionBlackoutLuaCall& call,
    float configured_duration);

// ---------------------------------------------------------------------------
// Sequence routines
// ---------------------------------------------------------------------------

// 005B9800, __thiscall(screen, float step), RET 4. Three arms, then the paint
// tail. Returns the step record; `still_fading` is the native's EAX.
MissionBlackoutStep mission_blackout_update_005b9800(MissionBlackoutFade& fade,
    float step, MissionBlackoutHost& host);

// 005B9BA0, __thiscall(screen, float level, float duration, const NativeString*),
// RET 0Ch. Writes the three fields and the name, steps the update once with
// kMissionBlackoutArmStep, then broadcasts when `game+1FE4h == 1`.
MissionBlackoutStep mission_blackout_arm_005b9ba0(MissionBlackoutFade& fade,
    const MissionBlackoutArgs& args, MissionBlackoutHost& host);

// 008D1340, __fastcall(lua_State*), the binding body without its Lua marshalling.
// Pushes no result (008D163E takes the count from 00B66400 with nothing pushed).
MissionBlackoutStep mission_blackout_binding_008d1340(const MissionBlackoutLuaCall& call,
    float configured_duration, MissionBlackoutFade& fade, MissionBlackoutHost& host);

}  // namespace bsp
