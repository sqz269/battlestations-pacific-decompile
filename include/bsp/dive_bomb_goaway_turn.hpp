#pragma once

namespace bsp {
// Semantic interfaces, not native object layouts or binary replacements.
// Names are hypotheses. Native addresses, ABI, widths and limits:
// docs/DIVE_BOMB_GOAWAY_TURN.md.
//
// The dive-bomb goaway state's evasive turn: its enter 009C4950 (no Ghidra
// function; body 009C4950-009C4A3C, `void __thiscall(state)`, plain RET), the
// tick's timer block 009C4A6D-009C4ACD, the re-roll 009C4CF1-009C4D9A, the
// split 009C4DAD and its two arms 009C4DAF-009C4DF6 (bank) and 009C4E05-009C4E1D
// (heading). The heading arm's heading is 009C47D0, which is 009D0C10 (the
// torpedo break-off geometry, bsp/torpedo_goaway_tick.hpp) with different
// fields and literals; its arithmetic after the 009FD570 call is the same
// instruction for instruction, so it is not reconstructed twice.

namespace dive_bomb_goaway_turn {
// --- 009C4950, the enter.
inline constexpr float kSideEven = 1.0f;          // 00D7A24C, MOVSS at 009C4973
inline constexpr float kSideOdd = -1.0f;          // 00D7A260, MOVSS at 009C4969
inline constexpr float kEnterCountdown = 15.0f;   // 00CE5380, MOVSS at 009C4983
inline constexpr float kStandoffJitterLo = 1.0f;  // FLD1 at 009C49F7
inline constexpr float kStandoffJitterHi = 1.25f; // 00CF29A8, FLD dword at 009C49E5
// 00CE3D78, FMUL qword at 009C4A28, only when ctl+369h and [00E17BF2] are set.
inline constexpr double kOrderedStandoffScale = 1.5;

// --- 009C4A6D-009C4ACD, the timers, every tick.
inline constexpr double kRingSlack = 100.0;       // 00D7A220, FSUB qword at 009C4A76
inline constexpr float kForceClockGate = 1.0f;    // 00D7A24C, COMISS dword at 009C4AA4
inline constexpr double kForceWindowPad = 6.0;    // 00CE6628, FADD qword at 009C4AB0
inline constexpr float kForcedCountdown = -1.0f;  // 00D7A260, MOVSS at 009C4ABE

// --- 009C4CF1-009C4D9A, the re-roll, flag-0 side only.
inline constexpr float kBankWindowLo = 3.0f;      // 00CE3854, FLD dword at 009C4D0A
inline constexpr float kBankWindowHi = 6.0f;      // 00CE6630, FLD dword at 009C4CFB
inline constexpr float kCountdownLo = 12.0f;      // 00CEB4B8, FLD dword at 009C4D2A
inline constexpr float kCountdownHi = 15.0f;      // 00CE5380, FLD dword at 009C4D1E
// 00419010(x0, y0, x1, y1, altitude) at 009C4D84: 1.0 at 100 m, 1.5 at 400 m.
inline constexpr float kScaleLowAltitude = 100.0f;   // 00CE3D08, FLD dword 009C4D7B
inline constexpr float kScaleLow = 1.0f;             // FLD1 at 009C4D75
inline constexpr float kScaleHighAltitude = 400.0f;  // 00CFD710, FLD dword 009C4D6B
inline constexpr float kScaleHigh = 1.5f;            // 00CE380C, FLD dword 009C4D61
inline constexpr double kClockLeadIn = 0.5;          // 00D7A280, FMUL qword 009C4D92

// --- 009C4DAF-009C4DF6, the bank arm.
inline constexpr float kBankClampLo = -1.2f;      // 00D05EA4, FLD dword at 009C4DBC
inline constexpr float kBankClampHi = 1.2f;       // 00CE3814, MOVSS at 009C4DD8

// --- 009C47D0's own literals (the rest are 009D0C10's, same addresses).
inline constexpr float kFlyToOffsetScale = 1.0f;  // FLD1 at 009C47E9, 009FD570 arg6
// 007F0280's half-extents, stored by MOVSS at 009C4819/009C482C/009C483A, and
// its mode byte PUSH 1 at 009C4824.
inline constexpr float kProbeExtentX = 80.0f;     // 00CE5444
inline constexpr float kProbeExtentY = 50.0f;     // 00CEB4D4
inline constexpr float kProbeExtentZ = 100.0f;    // 00CE3D08
}  // namespace dive_bomb_goaway_turn

// The goaway state object, task+704h (= approach+30Ch; the next state's vtable
// is at +33Ch, task+734h). Only the fields the turn reads or writes; +0h is the
// vtable 00D20CA0 and +4h the approach.
struct DiveBombGoAwayTurnState {
    // +18h, the turn side: +-1.0 from the enter, and 009FD570's arg4 IN AND OUT
    // (its 009FDC48 write-back). The bank arm multiplies by it.
    float side_18 = dive_bomb_goaway_turn::kSideEven;
    // +1Ch, the heading 009C47D0 publishes. The constructor zeroes it at
    // 009C751B; only 009C493C writes it after that.
    float heading_1c = 0.0f;
    // +20h, the standoff: 009FD570's arg3 and 009C7F00's completion distance.
    // The constructor writes 2 * approach+B4h at 009C7523; the enter rewrites it.
    float standoff_20 = 0.0f;
    // +24h, the countdown to the next re-roll. Constructor 100.0 (009C750A),
    // enter 15.0 (009C498B).
    float countdown_24 = 100.0f;
    // +28h and +2Ch: the bank clock and the bank window. NEITHER the constructor
    // (009C7506-009C7523 writes +1Ch, +20h, +24h only) nor the enter writes
    // them, so their value before the first re-roll is whatever the task
    // allocation held. Zero here is a stated stand-in, not a recovered value.
    float clock_28 = 0.0f;
    float window_2c = 0.0f;
};

struct DiveBombGoAwayEnterInputs {
    float attack_distance_b4 = 0.0f;  // approach+B4h, 009C4990
    // 007B5BE0(target) at 009C49B4 when approach+48h is non-null and its
    // vtable[5Ch](5) answers true (009C49A5-009C49B0).
    bool has_extent_target = false;
    float target_extent = 0.0f;
    // 00BD2F10 UniformFloatRange(1.0, 1.25) at 009C49FC.
    float standoff_jitter = dive_bomb_goaway_turn::kStandoffJitterLo;
    // [00F876B0] & 1 (signed remainder, 009C4958-009C4967): the step counter's
    // parity. Odd takes -1.0.
    bool step_odd = false;
    bool control_flag_369 = false;  // (approach+0Ch)+369h, 009C4A16
    bool global_e17bf2 = false;     // [00E17BF2], 009C4A1F
};

// 009C4950-009C4A3C. Writes +18h, +24h and +20h; nothing else.
void dive_bomb_goaway_enter_009c4950(const DiveBombGoAwayEnterInputs& in,
                                     DiveBombGoAwayTurnState& state) noexcept;

struct DiveBombGoAwayTimerInputs {
    float dt = 0.0f;                  // [ESP+18h], the tick's only argument
    float planar_distance_bc = 0.0f;  // approach+BCh, 009C4A6D
    // approach+C4h, 009C4AA4. Seeded 3600.0 (00CFDEB0) at 009C3FD2 and
    // 009C8A74 and advanced `+= dt` at 009C7A8C; no other writer was found, so
    // on a dive bomber the forced re-roll below cannot fire.
    float approach_clock_c4 = 3600.0f;
};

struct DiveBombGoAwayTimerReport {
    bool counted_down = false;  // 009C4A84 JBE not taken
    bool forced = false;        // 009C4AC6 ran
};

// 009C4A6D-009C4ACD, run on every goaway tick before the climb and the split.
DiveBombGoAwayTimerReport dive_bomb_goaway_timers_009c4a6d(
    const DiveBombGoAwayTimerInputs& in, DiveBombGoAwayTurnState& state) noexcept;

struct DiveBombGoAwayRerollInputs {
    float altitude = 0.0f;  // unit+100h, 009C4D54
    // The two 00BD2F10 draws, 009C4D13 then 009C4D33.
    float window_draw = dive_bomb_goaway_turn::kBankWindowLo;
    float countdown_draw = dive_bomb_goaway_turn::kCountdownLo;
};

// 009C4CF1-009C4D9A. Runs only when 0 > +24h (009C4CF1 COMISS, `76` JBE out).
// Returns true when it ran.
bool dive_bomb_goaway_reroll_009c4cf1(const DiveBombGoAwayRerollInputs& in,
                                      DiveBombGoAwayTurnState& state) noexcept;

struct DiveBombGoAwayTurnCommand {
    // 009C4DAD: +2Ch > +28h takes the bank arm, else the heading arm.
    bool bank_arm = false;
    float bank_target_2c4 = 0.0f;  // 009C4DF6, bank arm only
    float heading_2c0 = 0.0f;      // 009C4E17, heading arm only
    int heading_mode_2cc = 1;      // 009C4DF0 EBX = 1, or 009C4E1D literal 2
};

// 009C4D9D-009C4E1D. `heading_1c` is 009C47D0's result for this tick, used on
// the heading arm only (009C47D0 is called only there, at 009C4E09).
DiveBombGoAwayTurnCommand dive_bomb_goaway_turn_split_009c4dad(
    const DiveBombGoAwayTurnState& state, float heading_1c) noexcept;

}  // namespace bsp
