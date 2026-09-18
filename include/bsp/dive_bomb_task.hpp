#ifndef BSP_DIVE_BOMB_TASK_HPP
#define BSP_DIVE_BOMB_TASK_HPP

// The dive-bomb bot task (kind 8), from its per-tick arm 009C8790 to the two
// ordnance release sites 009C60F1 (aimdive) and 009C5777 (aimglide).
// docs/DIVE_BOMB_TASK.md carries the evidence; docs/BOT_TASKS.md owns the task
// object and the thirteen classes, docs/BOT_TASK_STATES.md owns the state
// family, and docs/TORPEDO_TASK_ARM.md is the sibling this file follows.
//
// Every name here is a hypothesis, not a recovered symbol. Nothing is a
// binary-compatible layout: the offset constants are the native ones, the
// structs are not.
//
// Contracts named but not reconstructed: the release request 007BBBA0 and the
// spawn behind it (docs/TORPEDO_RELEASE_SPAWN.md); the approach update's tail
// 009C7C5B-009C7E9E; the pitch/roll/yaw producers inside the aimdive tick
// 009C58D0; the flyabove tick 009C62B0; the turndown tick 009C44F0; the goaway
// tick 009C4A40; the attackrun tick 009C4220; the follow base tick 009C1FD0.

#include "bsp/bot_task_states.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The ten dive-bomb states, as whole-object offsets from the task. 009C83E0,
// 009C8310 and 009C7910 compare exactly these pointers; the registrar 009C73A0
// names all ten through BSP_BotStateRegistry_Add at 009C7680-009C76C6.
// ---------------------------------------------------------------------------
enum class DiveBombState : int {
    kNone = 0,
    kMoveTo = 0x4F0,     // "moveto (DiveBomb)"
    kFollow = 0x52C,     // "follow (DiveBomb)"
    kPrepare = 0x5C4,    // "DiveBomb/prepare"
    kDone = 0x664,       // "DiveBomb/done"
    kGoAway = 0x704,     // "DiveBomb/goaway"
    kAimDive = 0x734,    // "DiveBomb/aimdive"
    kAimGlide = 0x754,   // "DiveBomb/aimglide"
    kFlyAbove = 0x778,   // "DiveBomb/flyabove"
    kTurnDown = 0x79C,   // "DiveBomb/turndown"
    kAttackRun = 0x7BC,  // "DiveBomb/attackrun"
};

// ---------------------------------------------------------------------------
// Task fields the dive-bomb rules read. Whole-object offsets; the task is 7E0h.
// ---------------------------------------------------------------------------
namespace dive_bomb_task_off {
inline constexpr int kCurrentState = 0x310;      // 009C8794
inline constexpr int kApproach = 0x3F8;          // the ECX of 009C7A80 at 009C87E1
inline constexpr int kUnit = 0x3FC;              // the ECX of every 007BBBA0
inline constexpr int kPilotControlBlock = 0x404;  // approach->+0Ch, unit+9D4h
inline constexpr int kRoundsPending = 0x424;     // the manual-release budget
inline constexpr int kLatchedTarget = 0x440;     // the break-off's latched target
inline constexpr int kPlanStepResult = 0x2E4;    // copied back at 009C885B
inline constexpr int kPlanStepScratch = 0x4C4;   // the arm sets 0FFh at 009C87A3
inline constexpr int kMoveToRangeNear = 0x4A4;   // approach+ACh, 009C87EF
inline constexpr int kMoveToSpeed = 0x4AC;       // approach+B4h, 009C87FC
// The two approach-relative flags the whole machine keys on. 0x4C8-0x3F8 = 0xD0
// and 0x4C9-0x3F8 = 0xD1, and the aimdive branch of 009C83E0 reads the same
// 0xD1 through the state's owner pointer at 009C8664.
inline constexpr int kInRangeLatch = 0x4C8;      // approach+D0h
inline constexpr int kHasBombOrdnance = 0x4C9;   // approach+D1h
// prepare (+5C4h) + the follow drop countdown (+98h). 009C8248 writes it as
// [ESI+65Ch]; the registrar seeds it with -1.0f at 009C7452.
inline constexpr int kPrepareDropTimer = 0x5C4 + 0x98;
}  // namespace dive_bomb_task_off

// ---------------------------------------------------------------------------
// Approach fields, approach-relative (task+3F8h). 009C7A80 writes all of these.
// ---------------------------------------------------------------------------
namespace dive_bomb_approach_off {
inline constexpr int kUnitPose = 0x04;        // the entity the world position comes off
inline constexpr int kPilotControl = 0x0C;    // ctl, unit+9D4h
inline constexpr int kCommandBlock = 0x18;    // cmd, task+4h
inline constexpr int kRoundsRemaining = 0x2C;  // every release does -= 1
inline constexpr int kTarget = 0x48;          // 009C7B0A: no target -> clear the latch
inline constexpr int kExtraRange = 0x50;      // added to +D4h and +ACh in three tests
inline constexpr int kDriftRate = 0x64;       // 009C7A8C, per-tick timer feed
inline constexpr int kDiveAltitude = 0xA8;    // the aimdive release floor
inline constexpr int kBeginAltitude = 0xAC;   // = ctl->+398h every tick
inline constexpr int kAttackDistance = 0xB4;  // the moveto speed argument
inline constexpr int kInRangeDistance = 0xB8;  // the latch threshold
inline constexpr int kPlanarDistance = 0xBC;  // sqrt(dx^2 + dz^2) to the target
inline constexpr int kTargetBearing = 0xC0;   // pi/2 - atan2(...), wrapped to [0,2pi)
inline constexpr int kElapsed = 0xC4;         // += dt at 009C7A8C
inline constexpr int kWeaponSelect = 0xCC;    // aimdive writes 0, flyabove writes 3
inline constexpr int kInRangeLatch = 0xD0;    // task+4C8h
inline constexpr int kHasBombOrdnance = 0xD1;  // task+4C9h
inline constexpr int kReleaseRange = 0xD4;    // the aimdive abort test's range
inline constexpr int kAimPointX = 0xD8;       // the computed lead point
inline constexpr int kAimPointY = 0xDC;
inline constexpr int kAimPointZ = 0xE0;
}  // namespace dive_bomb_approach_off

// ---------------------------------------------------------------------------
// The five plane control slots of the command block cmd = approach->+18h =
// task+4h, from docs/PILOT_CONTROLS.md rows 0..4 and docs/PILOT_COMMAND_PATH.md
// for the axis names. Each slot is {live, desired, active}; the mode word says
// which producer owns the axis this frame.
// ---------------------------------------------------------------------------
namespace dive_bomb_cmd_off {
inline constexpr int kThrottleDesired = 0x278;   // unit+9F0h
inline constexpr int kThrottleActive = 0x27C;
inline constexpr int kYawDesired = 0x284;        // unit+9E4h
inline constexpr int kYawActive = 0x288;
inline constexpr int kRollDesired = 0x290;       // unit+9ECh
inline constexpr int kRollActive = 0x294;
inline constexpr int kPitchDesired = 0x29C;      // unit+9E8h
inline constexpr int kPitchActive = 0x2A0;
inline constexpr int kAirBrakeDesired = 0x2A8;   // unit+9F4h
inline constexpr int kAirBrakeActive = 0x2AC;
inline constexpr int kSpeedFlag = 0x2B0;
inline constexpr int kAltitudeDesired = 0x2BC;
inline constexpr int kHeadingDesired = 0x2C0;
inline constexpr int kAutoAltitude = 0x2C4;
inline constexpr int kModeRoll = 0x2CC;          // slot 2
inline constexpr int kModePitch = 0x2D0;         // slot 3
inline constexpr int kModeYaw = 0x2D4;           // slot 1
inline constexpr int kModeThrottle = 0x2D8;      // slot 0
}  // namespace dive_bomb_cmd_off

// ---------------------------------------------------------------------------
// The constants, by address, with the width the instruction actually reads.
// Every double below is loaded with an `fld/fsub/fadd/fmul qword`, so writing
// it as a float would be wrong; every float below is a `movss`/`fld dword`.
// ---------------------------------------------------------------------------
namespace dive_bomb_constant {
inline constexpr double kHalfPi = 1.5707963705062866;   // 00CE3830, qword
inline constexpr double kTwoPi = 6.2831854820251465;    // 00CE3828, qword
inline constexpr double kDistanceEpsilonSq = 1e-10;     // 00CE3820, qword
inline constexpr double kMoveToRangeBias = 100.0;       // 00D7A220, qword
inline constexpr float kNegativeZero = -0.0f;           // 00D7A208, movss
inline constexpr float kZero = 0.0f;                    // 00D7A218, movss
inline constexpr float kMinusOne = -1.0f;               // 00D7A260, movss
inline constexpr float kPlusOne = 1.0f;                 // 00D7A24C, movss
inline constexpr double kDiveAltitudeDecay = 0.05;      // 00D7A270, qword
inline constexpr float kCruisingAltitudeThird = 9999.0f;  // 00CE4C04, the +39Ch write

// The aimdive release, 009C60A9-009C60EC.
inline constexpr double kAimDiveReleaseErrorLimit = 25.0;  // 00CE3880, qword
inline constexpr double kPullOutAltitudeFraction = 0.5;    // 00D7A280, qword
inline constexpr float kAimDiveRearmLow = 0.5f;            // 00CE3800
inline constexpr float kAimDiveRearmHigh = 1.0f;           // the FLD1 at 009C60F6

// The aimdive dive-abort, 009C5B01-009C5B3E.
inline constexpr float kAbortRollFloor = -1.0471975803375244f;  // 00D20338, -60 deg
inline constexpr double kAbortRangeSlope = 0.30000001192092896;  // 00CE3DC8, qword
inline constexpr double kAbortRangeBias = 150.0;                 // 00CE3DD8, qword

// The aimglide release, 009C5693-009C5755.
inline constexpr float kGlideDiveAngleLimit = 0.5235987901687622f;  // 00CEC724, 30 deg
inline constexpr double kGlideHeightMargin = 50.0;        // 00CE3938, qword
inline constexpr double kGlideLateralLimit = 120.0;       // 00D1F3F8, qword
inline constexpr double kGlideLeadMargin = 5.0;           // 00D7A370, qword
inline constexpr double kGlideLeadScale = 3.0;            // 00D7A2B0, qword
inline constexpr float kAimGlideRearmLow = 0.2f;          // 00CE54A0
inline constexpr float kAimGlideRearmHigh = 0.5f;         // 00CE3800
inline constexpr double kGlideTravelScale = 0.3499999940395355;  // 00D04690, qword

// The turndown, 009C7800 and 009C8587-009C85B6.
inline constexpr float kTurnMagnitudeLow = 0.800000011920929f;  // 00CE74F8
inline constexpr float kTurnMagnitudeHigh = 1.0f;               // the 3F800000 immediate
inline constexpr float kTurnCoinRange = 1024.0f;                // 00D05B54
inline constexpr int kTurnCoinThreshold = 0x200;                // 009C85AA

// The turndown-complete test, 009C7EA0.
inline constexpr float kTurnDownRollGate = -1.2999999523162842f;  // 00D1F98C
inline constexpr float kTurnDownPitchGate = 2.356194496154785f;   // 00D20E80, 135 deg

// The arming slot 009C8200 and the goaway-complete test 009C7F00.
inline constexpr float kArmCountdown = 5.0f;              // 00CE3850, written to prepare+98h
inline constexpr double kGoAwayDistanceScale = 0.8999999761581421;  // 00D7A390, qword

// docs/BOT_TASKS.md's tuning rows for this class, kept here for the profile.
inline constexpr float kCruisingAltitude = 1300.0f;  // Pilot/DiveBomb/CruisingAlt
inline constexpr float kBeginAltRange = 1000.0f;     // Pilot/DiveBomb/BeginAltRange/1
inline constexpr float kAttackDistance = 1100.0f;    // Pilot/DiveBomb/AttackDist
inline constexpr float kSafeDistance = 100.0f;       // Pilot/DiveBomb/SafeDist
inline constexpr float kReferenceSpeed = 280.0f;     // Pilot/DiveBomb, KMH(280)
}  // namespace dive_bomb_constant

// ---------------------------------------------------------------------------
// 009C7910, __thiscall(task, state) -> bool, RET 4. Eight pointer compares:
// every state but moveto and follow counts as attacking.
// ---------------------------------------------------------------------------
bool dive_bomb_is_attacking_009c7910(DiveBombState state) noexcept;

// ---------------------------------------------------------------------------
// The engagement test 009C83F8-009C8417, inlined at the head of 009C83E0 and
// not a function of its own. The dive bomber is engaged while it is inside its
// attack range, or while the pilot control block reports mode 2 with a target
// still latched at task+440h.
// ---------------------------------------------------------------------------
struct DiveBombEngagedInputs {
    bool in_range_latch_4c8 = false;  // task+4C8h, 009C83F8
    int control_mode_370 = 0;         // ctl->+370h, 009C8407
    bool has_latched_target_440 = false;  // task+440h, 009C8410
};
bool dive_bomb_engaged_009c83f8(const DiveBombEngagedInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009C8310, the entry chooser, __fastcall(task), body 009C8310-009C83D2.
// The analogue of the torpedo's 009D3F60.
// ---------------------------------------------------------------------------
struct DiveBombEntryInputs {
    int control_mode_370 = 0;         // 0 -> prepare
    bool has_bomb_ordnance_4c9 = false;
    bool control_flag_369 = false;    // ctl->+369h, 009C8358
    bool global_e17bf2 = false;       // [00E17BF2], 009C8361
    bool in_range_latch_4c8 = false;  // 009C8379
};
DiveBombState dive_bomb_entry_state_009c8310(const DiveBombEntryInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009C83E0, the transition rule, __fastcall(task), RET 4 (the dt the arm pushes
// at 009C8831 is never read). Body 009C83E0-009C8785.
//
// The rule runs BEFORE the state's own tick, so every flag it reads was written
// by the previous frame's tick or by 009C7A80 earlier in the same arm.
// ---------------------------------------------------------------------------
struct DiveBombTransitionInputs {
    DiveBombState current = DiveBombState::kNone;
    DiveBombEngagedInputs engaged;
    DiveBombEntryInputs entry;
    bool unit_lacks_follow_target = true;  // 007B8AD0 at 009C841F
    bool should_break_off = false;         // task->vtable[1Ch] = 009C8A90

    // flyabove's own flags, whole-object +790h/+791h/+792h.
    bool flyabove_ready_791 = false;   // 009C8630: 0 -> fall through to +792h
    bool flyabove_can_dive_790 = false;  // 0 -> aimglide
    bool flyabove_leave_792 = false;   // -> goaway
    int flyabove_turn_side_798 = 0;    // 0 -> pick a side from the attitude

    // aimdive's own flags, +74Ch/+74Dh, and the approach flag it reads through
    // its owner pointer at 009C8664.
    bool aimdive_alive_74d = false;    // 0 -> aimglide
    bool aimdive_pull_out_74c = false;  // -> goaway

    bool aimglide_pull_out_76c = false;  // +76Ch -> goaway
    bool aimglide_out_of_bombs = false;  // 009C7850, !HasGeneralBombOrdnance
    bool turndown_complete = false;      // 009C7EA0 -> aimdive
    bool goaway_complete = false;        // 009C7F00

    // The attitude the flyabove branch uses to pick a turn side when +798h is 0.
    float unit_bank_c68 = 0.0f;
    float bank_high_00ce398c = 0.0f;  // > this -> side +1
    float bank_low_00d1fbc0 = 0.0f;   // < this -> side -1
    int random_turn_side = 1;         // the coin at 009C85A3, +1 or -1
};
struct DiveBombTransitionResult {
    DiveBombState next = DiveBombState::kNone;
    bool changed = false;
    bool wrote_turn_direction = false;  // 009C7800 ran
    int turn_side = 0;                  // the sign it was given
};
DiveBombTransitionResult dive_bomb_next_state_009c83e0(
    const DiveBombTransitionInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009C7A80's in-range latch, 009C7BFB-009C7C31. A hysteresis: it arms inside
// approach+B8h and only disarms past approach+B8h + 100.0, and never disarms
// while the control flag and the global agree.
// ---------------------------------------------------------------------------
struct DiveBombRangeLatchInputs {
    bool latched = false;         // the previous approach+D0h
    float planar_distance = 0.0f;  // approach+BCh
    float in_range_distance = 0.0f;  // approach+B8h
    bool control_flag_369 = false;
    bool global_e17bf2 = false;
};
bool dive_bomb_in_range_latch_009c7c31(const DiveBombRangeLatchInputs& in) noexcept;

// 009C7A94-009C7AB4: the dive altitude decays toward the third cruise altitude.
float dive_bomb_decay_dive_altitude_009c7a94(float dive_altitude_a8,
                                             float control_alt_39c) noexcept;

// ---------------------------------------------------------------------------
// 009C58D0, the aimdive tick's release, 009C608C-009C6119, and the pull-out
// latch at 009C611C-009C6154. RET 4. One bomb per gate, unlike the glide.
// ---------------------------------------------------------------------------
struct DiveBombAimDiveReleaseInputs {
    float altitude = 0.0f;         // pose+100h, the world Y
    float dive_altitude_a8 = 0.0f;  // approach+A8h
    float rearm_timer_1c = 0.0f;   // state+1Ch, counted down by dt at 009C58E9
    // The signed ALONG-TRACK MISS DISTANCE in metres written at 009C5C9B, from
    // dive_bomb_aim_error_009c5c9b below. Not an angle: the 25.0 the gate
    // compares it against is a 25-metre window.
    float aim_error = 0.0f;
    float rearm_draw = 0.0f;       // BSP_Random_UniformFloatRange(0.5, 1.0)
};

// ---------------------------------------------------------------------------
// 009C59BA-009C5C9B, the quantity the aimdive tick both steers on and releases
// on. Two BSP_Math_InterpolateClamped calls (009C5C49, 009C5C92) keyed on the
// aircraft's HEIGHT ABOVE THE TARGET, over the same x window:
//
//   x0 = approach->+A8h + 100.0 (the double at 00D7A220)
//   x1 = approach->+ACh + approach->+50h
//
// The first runs y from 0 to (approach->+14h)->+5Ch and is a lead distance; the
// second runs y from 1.0 to (approach->+14h)->+60h and is a dimensionless gain.
// Everything here is metres, which is what settles the units of the release
// gate: 009C59D6 writes the height as aircraft.y - targetPoint.y, and 009C5C97
// multiplies the gain into a distance.
// ---------------------------------------------------------------------------
struct DiveBombAimErrorInputs {
    float height_above_target = 0.0f;  // 009C59D6, aircraft.y - target.y
    float dive_altitude_a8 = 0.0f;     // approach+A8h
    float begin_altitude_ac = 0.0f;    // approach+ACh
    float extra_range_50 = 0.0f;       // approach+50h
    float lead_at_high_5c = 0.0f;      // (approach->+14h)->+5Ch, 009C5C20
    float gain_at_high_60 = 0.0f;      // (approach->+14h)->+60h, 009C5C69
    float bearing_error = 0.0f;        // BSP_Math_SubtractWrappedAngle, 009C5AF1
    float planar_distance = 0.0f;      // the sqrt at 009C5A40
};
struct DiveBombAimError {
    float lead = 0.0f;        // the first interpolation
    float gain = 1.0f;        // the second
    float along_track = 0.0f;  // cos(bearing) * distance - lead
    float error = 0.0f;       // gain * along_track, metres
};
DiveBombAimError dive_bomb_aim_error_009c5c9b(const DiveBombAimErrorInputs& in) noexcept;

// 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x), RET 14h. Kept here
// because both calls above are inside a body with no Ghidra function coverage.
float dive_bomb_interpolate_clamped_00419010(float x0, float y0, float x1, float y1,
                                             float x) noexcept;
struct DiveBombAimDiveReleaseResult {
    bool released = false;
    bool consumed_round = false;   // approach+2Ch -= 1
    float rearm_timer_1c = 0.0f;
    bool pull_out = false;         // state+18h = 1, the transition's goaway edge
};
DiveBombAimDiveReleaseResult dive_bomb_aimdive_release_009c60f1(
    const DiveBombAimDiveReleaseInputs& in) noexcept;

// 009C5B01-009C5B48: the dive-abort that clears +18h and +19h and drops the
// state to aimglide on the next transition.
struct DiveBombDiveAbortInputs {
    float release_range_d4 = 0.0f;   // approach+D4h
    float extra_range_50 = 0.0f;     // approach+50h
    float slant_range = 0.0f;        // [ESP+14h], the range the test compares
    float aim_point_distance = 0.0f;  // [ESP+1Ch], the planar distance to +D8h/+E0h
    float unit_attitude_c64 = 0.0f;  // pose+C64h, 009C5B1D
};
bool dive_bomb_dive_abort_009c5b43(const DiveBombDiveAbortInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009C5180, the aimglide tick's release, 009C5693-009C57A6. A salvo: it asks
// 007C1DB0 how many rounds the unit still has and calls 007BBBA0 that many
// times in the loop at 009C5771-009C5784.
//
// coverage: partial. The three range terms are frame slots whose producers this
// packet did not trace; the gates, the constants and the loop are transcribed.
// ---------------------------------------------------------------------------
struct DiveBombAimGlideReleaseInputs {
    float dive_angle = 0.0f;        // [ESP+18h], compared against 30 deg
    float height_above = 0.0f;      // [ESP+1Ch]
    float height_limit = 0.0f;      // [ESP+20h]
    float lateral_a = 0.0f;         // [ESP+14h]
    float lateral_b = 0.0f;         // [ESP+10h]
    float travel_accumulator_20 = 0.0f;  // state+20h
    int rounds_available = 0;       // 007C1DB0(unit)
    int rounds_cap = 0;             // the EBP cap the loop clamps against
    float rearm_draw = 0.0f;        // BSP_Random_UniformFloatRange(0.2, 0.5)
    float drift_rate_a4 = 0.0f;     // approach+A4h, the +20h feed
};
struct DiveBombAimGlideReleaseResult {
    bool released = false;
    int rounds_released = 0;        // one 007BBBA0 each, one approach+2Ch each
    float rearm_timer_1c = 0.0f;
    float travel_accumulator_20 = 0.0f;
};
DiveBombAimGlideReleaseResult dive_bomb_aimglide_release_009c5777(
    const DiveBombAimGlideReleaseInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009C8200, task vtable slot +24h: the arming entry BSP_PilotBot_Tick walks the
// task vector with. Unlike the torpedo's 009D49A0 this one returns 1 on every
// path; the arming itself is the +424h increment and the prepare countdown.
// ---------------------------------------------------------------------------
struct DiveBombArmInputs {
    bool has_bomb_ordnance_4c9 = false;
    bool has_unit = false;
    bool device_requests_release = false;  // (unit+72Ch)->vtable[38h]
    DiveBombState current = DiveBombState::kNone;
};
struct DiveBombArmResult {
    bool consumed = true;     // the return value; always 1 for this class
    bool queued_round = false;  // task+424h += 1
    bool armed = false;       // prepare+98h was written
    float prepare_timer = 0.0f;
};
DiveBombArmResult dive_bomb_arm_drop_009c8200(const DiveBombArmInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009C7800, __thiscall(turndown_state, int sign). The roll the turndown rolls
// with: a random magnitude in [0.8, 1.0] carrying the caller's sign.
// ---------------------------------------------------------------------------
float dive_bomb_turn_direction_009c7800(int sign, float magnitude_draw) noexcept;

// ---------------------------------------------------------------------------
// 009C4220, the attackrun tick (vtable 00D20C68 slot +Ch), Ghidra body
// 009C4220-009C447D, __thiscall(state, float dt), RET 4.
//
// This is the run-in, and it is the `009D07B0` / `009A3770` shape
// docs/BOT_TASK_STATES.md tabulates, with dive-bomb constants. It does not
// close the range itself: it commands a heading at the target with mode 2, and
// the plane flies it.
//
// Jump senses from the branch bytes: 009C424F `0F 82` JC, 009C4329 `76` JBE,
// 009C4379 `76` JBE.
// ---------------------------------------------------------------------------
namespace dive_bomb_attackrun_constant {
inline constexpr double kDistanceClamp = 2000.0;      // 00CF0DD8 qword / 00CFFD60 float
inline constexpr double kMarginCeiling = 1400.0;      // 00D1F8D0 qword
inline constexpr double kMarginFloor = 50.0;          // 00CE3938 qword / 00CEB4D4 float
inline constexpr float kThrottleRatioLow = 0.10000000149011612f;   // 00D7A2F0
inline constexpr float kThrottleAtLow = 0.4000000059604645f;       // 00CE7804
inline constexpr float kThrottleRatioHigh = 0.3499999940395355f;   // 00CF6560
inline constexpr float kThrottleAtHigh = 1.0f;                     // the FLD1
inline constexpr double kLateralOffsetScale = 0.5235987901687622;  // 00CEC730 qword
inline constexpr float kSamplerA = 80.0f;             // 00CE5444
inline constexpr float kSamplerB = 60.0f;             // 00CEB4B0
inline constexpr float kSamplerC = 120.0f;            // 00D05804
}  // namespace dive_bomb_attackrun_constant

struct DiveBombAttackRunInputs {
    float dt = 0.0f;
    float reroll_timer_1c = 0.0f;   // state+1Ch, 009C4227
    float reroll_period_18 = 0.0f;  // state+18h, 009C4255
    float lateral_offset_20 = 0.0f;  // state+20h, kept across the countdown
    float target_bearing_c0 = 0.0f;  // approach+C0h, 009C422D
    float planar_distance_bc = 0.0f;  // approach+BCh, 009C4311
    float altitude = 0.0f;           // pose+100h, 009C435B
    float begin_altitude_ac = 0.0f;  // approach+ACh, 009C43ED
    float extra_range_50 = 0.0f;     // approach+50h
    float attack_distance_b4 = 0.0f;  // approach+B4h, 009C43E3
    // 007F0280(ctl, pose, ...) at 009C42B8, the lateral-offset sampler, with the
    // last argument 1 (the torpedo passes 0). Its three float arguments are the
    // 80, 60 and 120 above. The body is a contract; the host supplies its result.
    float sampler_result = 0.0f;
    bool sampler_ran = false;
};
struct DiveBombAttackRunResult {
    float reroll_timer_1c = 0.0f;
    float lateral_offset_20 = 0.0f;
    bool rerolled = false;
    float commanded_heading_2c0 = 0.0f;  // cmd+2C0h, with cmd+2CCh = 2
    float clamped_distance = 0.0f;       // min(approach+BCh, 2000)
    float height_margin = 0.0f;          // max(1400 - altitude, 50)
    float throttle_ratio = 0.0f;         // margin / clamped distance
    float commanded_throttle = 0.0f;     // the fourth argument to 009FBA50
    float commanded_altitude_base = 0.0f;  // its first
    bool wrote_full_throttle = true;     // cmd+278h = 1.0f, +27Ch = 1
    bool wrote_zero_air_brake = true;    // cmd+2A8h = 0.0f, +2ACh = 1
};
DiveBombAttackRunResult dive_bomb_attackrun_tick_009c4220(
    const DiveBombAttackRunInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009C44F0, the turndown tick (vtable 00D20C84 slot +Ch), body
// 009C44F0-009C4736, __thiscall(state, float dt), RET 4. The dt is never read.
//
// The roll-in: it banks the aircraft toward inverted at the rate 009C7800 drew,
// latches once past 150 degrees, then pulls the nose down. Its one exit to the
// transition rule is through 009C7EA0, which reads the same two pose angles.
//
// Every jump sense below was read from the branch byte, not the mnemonic:
// 009C45BB `76` JBE, 009C465B `76` JBE, 009C4687 `76` JBE.
// ---------------------------------------------------------------------------
namespace dive_bomb_turndown_constant {
inline constexpr double kWrapLow = -3.1415927410125732;   // 00CE3D18, qword
inline constexpr double kWrapHigh = 3.1415927410125732;   // 00CE3D28, qword
inline constexpr double kRollHandOver = 0.800000011920929;  // 00CE3D40, 45.8 deg
inline constexpr float kPi = 3.1415927410125732f;         // 00D7A264, movss
inline constexpr float kLatchBank = 2.6179940700531006f;   // 00D1FED0, 150 deg
inline constexpr float kPitchHoldBand = 0.3490658700466156f;  // 00CE398C, 20 deg
inline constexpr float kEaseOffAngle = 0.5235987901687622f;   // 00CEC724, 30 deg
inline constexpr float kFullPitchAngle = 0.05235987901687622f;  // 00D0CBA0, 3 deg
// 007C47F0 = tuning+24Ch * classDesc+184h. Both halves are named elsewhere:
// docs/GAME_TUNING_SINGLETON.md row +24Ch is Dynamics/SpdMultipliers/LevelFlight,
// default 1.8, and docs/PLANE_FLIGHT.md row +184h is the authored StallSpd,
// default 17.5. docs/PLANE_GROUND_OPS.md step 6 already forms the same product.
// So the turndown's desired speed is the level-flight speed, about 31.5 m/s on
// a default class. The record at approach+8h is the plane class descriptor:
// 009C7A94 reads its +188h MaxSpd through the same pointer.
inline constexpr float kLevelFlightMultiplier = 1.8f;   // tuning+24Ch
inline constexpr float kStallSpeedDefault = 17.5f;      // classDesc+184h
}  // namespace dive_bomb_turndown_constant

struct DiveBombTurnDownInputs {
    float bank_c68 = 0.0f;       // pose+C68h, 009C4530
    float pitch_c64 = 0.0f;      // pose+C64h, 009C4666
    bool rolled_latch_1c = false;  // state+1Ch, 009C45A9
    float roll_command_18 = 0.0f;  // state+18h, what 009C7800 wrote
    // 007C47F0(approach+8h) at 009C450D: LevelFlight * StallSpd, the
    // level-flight speed. approach+8h is the plane class descriptor.
    float desired_speed = 0.0f;
};
struct DiveBombTurnDownResult {
    // The speed pair every path writes first, 009C4512-009C4524.
    float speed_2b4 = 0.0f;
    bool speed_flag_2b0_cleared = true;
    bool throttle_one_shot_2d8 = true;  // docs/PILOT_THROTTLE_CUT_RAISER.md
    // The bank, wrapped into (-pi, pi] and folded to its absolute value.
    float folded_bank = 0.0f;
    float angle_to_inverted = 0.0f;  // max(pi - |bank|, 0)
    // The roll axis, slot 2: +290h desired, +294h active, mode +2CCh.
    bool wrote_roll = false;
    float roll_290 = 0.0f;
    // The hand-over arm: +2C4h = pi with mode +2CCh = 1, no roll.
    bool released_roll = false;
    // The pitch axis, slot 3: +29Ch desired, +2A0h active, mode +2D0h.
    bool wrote_pitch = false;
    float pitch_29c = 0.0f;
    // The altitude arm: +2BCh = 0 with mode +2D0h = 2.
    bool wrote_altitude_hold = false;
    bool latch_1c_set = false;  // 009C465D
};
DiveBombTurnDownResult dive_bomb_turndown_tick_009c44f0(
    const DiveBombTurnDownInputs& in) noexcept;

// 009C4530-009C4575: fmod by 2pi through 00BF857A, then the (-pi, pi] wrap.
float dive_bomb_wrap_signed_pi_009c4551(float angle) noexcept;

// 009C7EA0, __fastcall(state) -> bool. True ends the turndown for aimdive.
// The two pose angles in the order the body reads them: +C64h is the one the
// -1.3 and -1.0 gates compare, +C68h the one folded to its absolute value
// against 135 degrees. docs/PILOT_CONTROLS.md calls +C64h pitch and +C68h
// bank, so the parameter names here are the offsets, not a claim about which
// axis each carries.
bool dive_bomb_turndown_complete_009c7ea0(float attitude_c64, float attitude_c68) noexcept;

// ---------------------------------------------------------------------------
// 009C8790's own two argument computations, 009C87EF-009C8822. The moveto range
// pair is the near value biased by the 100.0 double and the raw value.
// ---------------------------------------------------------------------------
float dive_bomb_arm_move_to_near_009c8814(float range_near_4a4) noexcept;

// 009C8790's diving predicate, 009C8794-009C87D7: four pointer compares that
// decide the third argument 009C7A80 receives.
bool dive_bomb_arm_is_diving_009c87d2(DiveBombState state) noexcept;

// 009C88BE-009C88C9, the manual-release passthrough. Six states refuse it.
bool dive_bomb_manual_passthrough_009c88c4(int rounds_pending_424, bool has_unit,
                                           bool device_requests_release,
                                           DiveBombState current) noexcept;

// ---------------------------------------------------------------------------
// 009C8920, the per-tick cruise profile (task vtable slot +54h), in the shape
// docs/BOT_TASKS.md tabulates for all ten overriding classes. The dive bomb is
// one of the two that write the third altitude.
// ---------------------------------------------------------------------------
struct DiveBombCruiseProfileInputs {
    bool unit_lacks_follow_target = true;  // 007B8AD0; false skips everything
    bool overridden_38d = false;
    float control_alt_380 = 0.0f;
    bool one_shot_3a9 = false;
    bool overridden_38c = false;
    float control_alt_37c = 0.0f;
    bool one_shot_3aa = false;
    bool overridden_38e = false;
    float control_alt_384 = 0.0f;
    bool one_shot_3ab = false;
    float cruising_altitude = dive_bomb_constant::kCruisingAltitude;
    float begin_altitude = dive_bomb_constant::kBeginAltRange;
    float attack_distance = dive_bomb_constant::kAttackDistance;
    float speed_ratio_41c = 1.0f;
    float current_attack_distance_4b0 = 0.0f;  // task+4B0h, 009C8A30
};
struct DiveBombCruiseProfileResult {
    bool wrote_394 = false;
    float altitude_394 = 0.0f;
    bool wrote_398 = false;
    float altitude_398 = 0.0f;
    bool wrote_39c = false;
    float altitude_39c = 0.0f;
    bool dirty_3ad = false;
    float attack_distance_4b0 = 0.0f;
    // 009C8A74: task+4BCh = [00CFDEB0], and 009C8A7C re-runs the approach update
    // with dt = 0.0f and diving = false before the tail jump to 0099B740.
    bool reran_approach_update = false;
};
DiveBombCruiseProfileResult dive_bomb_cruise_profile_009c8920(
    const DiveBombCruiseProfileInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009C8A90, the break-off test (task vtable slot +1Ch), the shape
// docs/BOT_TASKS.md gives for the four overriding classes.
// ---------------------------------------------------------------------------
struct DiveBombBreakOffInputs {
    bool base_0099c230 = false;       // false -> 0 outright
    bool has_latched_target = false;  // task+440h
    bool target_dead_5d = false;      // target+5Dh
    bool control_flag_369 = false;
    bool global_e17bf2 = false;
    bool has_bomb_ordnance_4c9 = false;  // the class extra
    float distance_to_target = 0.0f;
    float safe_distance = dive_bomb_constant::kSafeDistance;
    float speed_ratio_41c = 1.0f;
};
bool dive_bomb_should_break_off_009c8a90(const DiveBombBreakOffInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Which command installs kind 8. docs/ATTACK_COMMANDS.md: 007EEC50 picks the
// class and 0099A170 turns it into a task, so a task exists only for a unit
// whose chosen class is the divebomb one. Nothing else installs it, and in
// particular an `artillery` order, which 0046AAB0 resolves to `attackmove`
// 00E08F78 outright, never reaches the chooser at all.
// ---------------------------------------------------------------------------
inline constexpr unsigned int kDiveBombCommandClass = 0x00E08F20u;

// The gate the host arm needs: the task runs for this unit only when 007EEC50
// actually chose the divebomb class for it. Carrying bomb ordnance and holding
// some commanded target is not the same test, and the difference is 27 aircraft
// in IJN01 that the image never gives a kind 8 task.
inline bool dive_bomb_task_installed_for_class(unsigned int chosen_class) noexcept {
    return chosen_class == kDiveBombCommandClass;
}

// ---------------------------------------------------------------------------
// The host. One virtual per native call site the arm sequence reaches, on top
// of the shared BotTaskStateHost of include/bsp/bot_task_states.hpp.
// ---------------------------------------------------------------------------
struct DiveBombTaskHost : BotTaskStateHost {
    // 009C7A80 at 009C87EA, ECX = task+3F8h, RET 8. The dive-bomb approach
    // update, the analogue of the torpedo's 009D3420. Body 009C7A80-009C7E9E.
    // The third argument is the diving predicate of 009C87D2.
    virtual void update_dive_bomb_approach_009c7a80(void* approach, float dt, bool diving) = 0;

    // 009C83E0 at 009C8834, ECX = task. Reconstructed above; the host supplies
    // the predicate values the rule cannot compute.
    virtual DiveBombTransitionInputs read_transition_inputs(void* task) = 0;
    virtual void set_dive_bomb_state(void* task, DiveBombState next) = 0;

    // 009C7800 at 009C8574, 009C85C1 and 009C85E9, ECX = the turndown state.
    virtual void write_turn_direction(void* task, float roll) = 0;

    // The aimdive tick 009C58D0 (vtable 00D20CE8 slot +Ch) and the aimglide
    // tick 009C5180 (vtable 00D20CC8 slot +Ch). Both bodies are analyzed for
    // their release gates and their command-block writes only; the host runs
    // the native tick and answers the gate inputs.
    virtual DiveBombAimDiveReleaseInputs read_aimdive_inputs(void* state, float dt) = 0;
    virtual void apply_aimdive_result(void* state, const DiveBombAimDiveReleaseResult& r) = 0;
    virtual DiveBombAimGlideReleaseInputs read_aimglide_inputs(void* state, float dt) = 0;
    virtual void apply_aimglide_result(void* state, const DiveBombAimGlideReleaseResult& r) = 0;

    // 007C1DB0 at 009C575D and in the aimglide enter 009C4F00, ECX = unit.
    // Walks the device list at unit+48h and sums 006E3500 over every device
    // answering IsKindOf(25h): the rounds the aircraft still carries.
    virtual int rounds_remaining_007c1db0(void* unit) = 0;

    // BSP_Random_UniformFloatRange 00BD2F10 at 009C6114, 009C579E, 009C859E
    // and inside 009C7800, always with ECX = 1.
    virtual float uniform_between(float low, float high) = 0;

    // The four flags 009C7A80 leaves on the approach that the transition rule
    // and the two arms read back.
    virtual bool approach_has_bomb_ordnance(void* approach) = 0;
    virtual bool approach_in_range_latch(void* approach) = 0;
    virtual void spend_round(void* approach) = 0;  // approach+2Ch -= 1

    // task->+424h, the manual-release budget, and the two plan-step fields.
    virtual int rounds_pending(void* task) = 0;
    virtual void spend_pending_round(void* task) = 0;
    virtual void set_plan_step_scratch(void* task, int value) = 0;
    virtual void commit_plan_step_result(void* task) = 0;

    // prepare+98h, task+65Ch. 009C8200 raises it; nothing in this class spends
    // it, because the dive bomb's done/prepare tick 009C7270 is a bare tail
    // call to the follow base 009C1FD0.
    virtual void write_drop_timer(void* task, float value) = 0;

    // The moveto state task+4F0h that 009BDE80 is called on at 009C8825, and
    // the two floats the arm reads off the task for it.
    virtual void* move_to_state(void* task) = 0;
    virtual float arm_move_to_range(void* task) = 0;   // task+4A4h
    virtual float arm_move_to_speed(void* task) = 0;   // task+4ACh
};

// ---------------------------------------------------------------------------
// The context the arm sequence works over.
// ---------------------------------------------------------------------------
struct DiveBombTaskContext {
    void* task = nullptr;
    void* approach = nullptr;
    void* unit = nullptr;
    void* command_block = nullptr;        // approach->+18h == task+4h
    void* pilot_control_block = nullptr;  // approach->+0Ch == unit+9D4h
    void* state = nullptr;                // task->+310h, the object
    DiveBombState current = DiveBombState::kNone;
};

// ---------------------------------------------------------------------------
// 009C8790, task vtable 00D20E18 slot +64h, the per-tick arm. No Ghidra
// function; raw listing 009C8790-009C88D5 inclusive, INT3 padding from
// 009C88D6, RET 4, `void __thiscall(task, float dt)`.
// ---------------------------------------------------------------------------
struct DiveBombArmTickResult {
    DiveBombState state_before = DiveBombState::kNone;
    DiveBombState state_after = DiveBombState::kNone;
    bool transitioned = false;
    bool was_diving = false;            // the 009C87D2 predicate
    float move_to_near = 0.0f;
    float move_to_far = 0.0f;
    float move_to_speed = 0.0f;
    bool ran_aimdive = false;
    DiveBombAimDiveReleaseResult aimdive;
    bool ran_aimglide = false;
    DiveBombAimGlideReleaseResult aimglide;
    bool manual_release = false;        // the 009C88C4 passthrough fired
    int releases = 0;                   // 007BBBA0 requests this tick
};
DiveBombArmTickResult dive_bomb_task_arm_009c8790(DiveBombTaskHost& host,
                                                  DiveBombTaskContext& ctx,
                                                  float dt);

}  // namespace bsp

#endif  // BSP_DIVE_BOMB_TASK_HPP
