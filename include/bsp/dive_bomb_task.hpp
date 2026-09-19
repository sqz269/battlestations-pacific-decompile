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
// CORRECTION: this was `kExtraRange`, "added to +D4h and +ACh in three tests".
// It is component 1 of the aim point approach->vtable[0] (009C40A0) hands out
// from +4Ch/+50h/+54h, and the aimdive tick subtracts it from the aircraft's Y
// to get the height above that point, so it is a HEIGHT. Adding it to +ACh is
// altitude plus altitude. docs/DIVE_BOMB_TASK.md.
inline constexpr int kAimPointHeight = 0x50;
inline constexpr int kAimPointEast = 0x4C;    // out[0] of 009C40A0
inline constexpr int kAimPointNorth = 0x54;   // out[2] of 009C40A0
inline constexpr int kDriftRate = 0x64;       // 009C7A8C, per-tick timer feed
inline constexpr int kDiveAltitude = 0xA8;    // the aimdive release floor,
// Uniform(dive_bomb_release_alt_1_044, dive_bomb_release_alt_2_048) at 009C3F29
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
// CORRECTED TWICE. These were first misnamed kAimPointX/Y/Z (the aim point
// 009C40A0 hands out is +4Ch/+50h/+54h), then renamed kRunInOrigin* on the
// reading below - "the aircraft's own world position latched once at task
// construction". That second reading is ALSO withdrawn: 009C4065 is one of
// three writers and the other two are in 009C7A80, which runs every arm tick,
// so the constructor's value survives no ticks at all. During a dive these
// three are the PREDICTED BOMB IMPACT POINT; the note above
// dive_bomb_impact_point_009c7d71 below carries the listing and the census.
// The names are kept because nothing else references them and a third rename
// would only cost a diff.
//
// What the constructor's own store is, unchanged, is: the
// aimdive tick subtracts +D8h and +E0h from the aim point to take a bearing
// along the attack run as it was set up.
//
// The chain that names EDI: 009C73C5 and 009C73C8 push EBP then EAX, so EAX is
// the constructor's FIRST argument, and 009C3ECA `MOV EDI,[ESP+20h]` reads it
// past the seven prologue pushes. 009C3ED2 then pushes that same EDI as
// 009F9CE0's first argument, and 009F9CE0 stores its `[ESP+4]` into `[ECX+4]`
// at 009F9CEA - which is approach+4h, the unit. So EDI is the aircraft, and
// 009C405D-009C407D copies its +FCh/+100h/+104h here.
// docs/DIVE_BOMB_TASK.md.
inline constexpr int kRunInOriginX = 0xD8;
inline constexpr int kRunInOriginY = 0xDC;
inline constexpr int kRunInOriginZ = 0xE0;
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
// 007BCC80's two, and the bias 009C7D99 adds to its result.
inline constexpr double kGravity = 9.8100004196167;     // 00CF9058, qword
inline constexpr float kFallTimeVerticalBias = 3.0f;    // 00E08E54, fld dword
inline constexpr double kImpactPointFallTimeBias = 0.1;  // 00D7A3A0, qword

// approach+D4h, set once by the constructor 009C3EA0 at 009C3FFB-009C4045.
// The same 00D7A280 half that kPullOutAltitudeFraction names, in its other role:
// the mean of the drawn release altitude and the begin altitude.
inline constexpr double kDiveEntryHeightMean = 0.5;      // 00D7A280, qword
inline constexpr double kDiveEntryHeightMargin = 250.0;  // 00CF8850, qword

// The aimdive release, 009C60A9-009C60EC.
inline constexpr double kAimDiveReleaseErrorLimit = 25.0;  // 00CE3880, qword
inline constexpr double kPullOutAltitudeFraction = 0.5;    // 00D7A280, qword
inline constexpr float kAimDiveRearmLow = 0.5f;            // 00CE3800
inline constexpr float kAimDiveRearmHigh = 1.0f;           // the FLD1 at 009C60F6

// The aimdive dive-abort, 009C5B01-009C5B3E.
inline constexpr float kAbortRollFloor = -1.0471975803375244f;  // 00D20338, -60 deg
inline constexpr double kAbortRangeSlope = 0.30000001192092896;  // 00CE3DC8, qword
inline constexpr double kAbortRangeBias = 150.0;                 // 00CE3DD8, qword

// The aimglide release, 009C5689-009C5755.
// CORRECTION, packet cc8_dive_glide. `kGlideDiveAngleLimit` is not a limit on a
// dive angle. 009C569B compares it against [ESP+18h], and [ESP+18h]'s producer
// is the chain 009C5357-009C53CA: FLD [ESP+40h] / FLD [ESP+38h] (the planar
// aimPoint - unit vector) -> 00BF701A atan2 -> `pi/2 - atan2`, +2pi when
// negative (00CE3830, 00CE3828 - the same wrap 009C7B8A-009C7BB0 builds for
// approach+C0h) -> 00438B10 against 009C4F80's aim heading -> abs at
// 009C53A5-009C53CA. It is a HORIZONTAL BEARING ERROR, so the gate is a 30 deg
// bearing tolerance and the 009C5715 cosine is the projection of the throw onto
// the line of sight. The name is kept because the ledger and the doc carry it.
inline constexpr float kGlideDiveAngleLimit = 0.5235987901687622f;  // 00CEC724, 30 deg
// 009C53DD `MOV EBP,2` / 009C53E2 `LEA EBX,[EBP-1]`, both on the straight-line
// path into 009C53E5's branch, so both arms carry EBP=2 and EBX=1 to the salvo:
// 009C5762 caps the loop at two rounds and 009C5782 steps it by one. This is a
// literal immediate, not a substitution.
inline constexpr int kGlideSalvoCap = 2;         // 009C53DD, the EBP cap
inline constexpr double kGlideHeightMargin = 50.0;        // 00CE3938, qword
inline constexpr double kGlideLateralLimit = 120.0;       // 00D1F3F8, qword
inline constexpr double kGlideLeadMargin = 5.0;           // 00D7A370, qword
// 009C4F50: the floor the aimglide enter 009C4F00 puts under state+20h when its
// argument is under the 5.0 at 00D7A370 (009C4F4E, byte 76 JBE). The accumulator
// therefore starts at max(arg, 5.0) and is never zero, which is what makes the
// lead gates at 009C5743/009C5751 satisfiable at all.
inline constexpr float kGlideTravelSeed = 5.0f;           // 00CE3850
// 009C4F32's scale on `(rounds - 1)` in that same seed. RECOVERED, packet
// cc8_dive_glide: 009C4F00 takes no stack argument, so the seed is
// `max((007C1DB0(unit) - 1) * 0.07 * approach+A4h, 5.0)` entirely locally.
inline constexpr double kGlideSeedScale = 0.07000000029802322;  // 00CED0D8, qword
// 00CEFFB0, the qword 009C3F00 scales the plane class descriptor's +188h
// MaxSpd by into approach+A4h, in the constructor FUN_009C3EA0 - the one and
// only writer of that field in 009C3E00-009CA000.
inline constexpr double kApproachDriftScaleA4 = 0.949999988079071;  // 00CEFFB0, qword
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
// Named for the axis each one tests, which is the opposite of what the old
// names said: 009C7ED8 compares 00D1F98C against pose+C64h (the PITCH) and
// 009C7EEA compares the folded pose+C68h (the BANK) against 00D20E80.
inline constexpr float kTurnDownPitchComplete = -1.2999999523162842f;  // 00D1F98C
inline constexpr float kTurnDownInvertedBank = 2.356194496154785f;     // 00D20E80, 135 deg

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
    // 009C7C31-009C7CFE, the spent-member arm, added by packet
    // cc8_dive_approach. `009C7C31 CMP byte [ESI+D1h],0` sets the flags that
    // `009C7C3E JNZ` consumes - the intervening store does not write flags - so
    // a bomber with no bombs left falls THROUGH the store into a second,
    // independent clear. `009C7CFE AND byte [ESI+D0h],AL` can only clear.
    bool has_bomb_ordnance_d1 = true;   // approach+D1h; true skips the whole arm
    // 009C7C5D `MOV EDI,[EAX+3D0h]` with EAX = approach+0Ch, then 009C7C63
    // `CMP EDI,[ESI+4h] / JZ`: the flight leader itself is exempt.
    bool is_flight_leader = false;
    bool leader_known = false;          // no squadron resolves, so no arm runs
    // 009C7C7C-009C7CC6. NOTE what the two endpoints are: `009C7C9B CALL [[ESI]]`
    // is the approach's vtable slot 0, 009C40A0, which copies the AIM POINT
    // approach+4Ch/+50h/+54h, and 009C7C82's EDI is the LEADER's pose. The
    // subtractions at 009C7CAE and 009C7CBA are `aimPoint - leader`, components
    // 0 and 2, and 00414C60 takes the 2-D length. The unit's own position is
    // not in this expression at all.
    float leader_to_aim_point = 0.0f;
};
bool dive_bomb_in_range_latch_009c7c31(const DiveBombRangeLatchInputs& in) noexcept;

// 009C7A94-009C7AB4: the dive altitude decays toward the third cruise altitude.
float dive_bomb_decay_dive_altitude_009c7a94(float dive_altitude_a8,
                                             float control_alt_39c) noexcept;

// ---------------------------------------------------------------------------
// 007BCC80, RET 4, __thiscall(unit, float height): how long a body released now
// takes to fall `height` metres. Read whole from the listing 007BCC80-007BCCEB:
//
//   007BCC86  height <= 0            -> 0.0
//   007BCC95  vy = unit->vtable[+34h]().y - [00E08E54]   ([00E08E54] = 3.0f)
//   007BCCB0  d  = vy*vy + 2*height*[00CF9058]           ([00CF9058] = 9.81)
//   007BCCCA  t  = (sqrt(d) + vy) / [00CF9058]
//
// The + sign on vy is what fixes the sign convention: with vy the world Y
// velocity, upward positive, vy = 0 gives sqrt(2h/g) and a descending aircraft
// gets a shorter fall, which is the free-fall solution of h = -vy*t + g*t^2/2.
// The 3.0f is a constant bias on the vertical velocity, in .data, not traced to
// its writer here.
//
// This is a SHARED helper, not a dive-bomb one: 009D139D in the torpedo
// approach update calls it for the drop lead, where docs/TORPEDO_APPROACH_UPDATE
// .md carries it as the unimplemented contract `fall_time`. It is declared here
// because this packet is the first to read its body; nothing on the torpedo side
// is changed by this commit.
// ---------------------------------------------------------------------------
float weapon_fall_time_007bcc80(float height_above_aim_point,
                                float unit_velocity_y) noexcept;

// ---------------------------------------------------------------------------
// 009C7A80's second arm, 009C7D71-009C7E33, and the CORRECTION it forces on
// `approach+D8h`/`+DCh`/`+E0h`.
//
// Those three are NOT "the aircraft's own position latched once", either at task
// construction (the note above `kRunInOriginX` said so) or at dive entry. The
// approach update rewrites all three EVERY tick, through one of two arms chosen
// at 009C7D04-009C7D12 by the `diving` argument (the third argument 009C87EA
// pushes, read back at `[ESP+38h]`) and by approach+D0h:
//
//   diving == 0 && approach+D0h == 0   ->  009C7D27: the raw unit position
//   otherwise                          ->  009C7D71: the arm below
//
// The four states the arm calls `diving` include aimdive and aimglide, so during
// a dive it is always the second arm:
//
//   h    = unit.y - aimPoint.y                       ; 009C7B49, held negated
//   tf   = weapon_fall_time_007BCC80(h, v.y) + 0.1   ; 009C7D94, 00D7A3A0
//   v    = unit->vtable[+34h]()                      ; 009C7DB0, the velocity
//   +D8h = unit.x + tf * v.x                         ; 009C7DDC-009C7E13
//   +DCh = aimPoint.y                                ; 009C7E33 overwrites +0.0
//   +E0h = unit.z + tf * v.z                         ; 009C7E01-009C7E27
//
// So the triple is the **predicted impact point** of a bomb released this tick:
// the aircraft advanced by its own velocity over the bomb's time of flight. The
// aimdive tick differences the aim point against it (009C5950 `FSUB [EDI+0D8h]`,
// 009C5960 `FSUB [EDI+0E0h]`) and that difference feeds the second sqrt at
// 009C5A40, `[ESP+5Ch]`, and the bearing at 009C5AF1, `[ESP+18h]` - the two
// inputs of the aim error. The release gate at 00CE3880 is therefore a 25-metre
// CCIP window: release when the predicted impact point is within 25 m, along
// track, of the aim point.
//
// `unit->vtable[+34h]` is the velocity getter on the unit's own vtable, not the
// approach's: 009C7D9F loads it through `[ESI+4]`. It is identified from
// 007BCC80 itself, which reads `.y` of the same slot's result as a vertical
// velocity in a free-fall solution; the slot has no recovered name.
// ---------------------------------------------------------------------------
struct DiveBombImpactPointInputs {
    float unit_position[3]{};   // unit+FCh, +100h, +104h
    float unit_velocity[3]{};   // unit->vtable[+34h]()
    float aim_point_y = 0.0f;   // approach->vtable[0]().y
};
struct DiveBombImpactPoint {
    float point[3]{};        // approach+D8h, +DCh, +E0h
    float fall_time = 0.0f;  // the tf above, seconds
};
DiveBombImpactPoint dive_bomb_impact_point_009c7d71(
    const DiveBombImpactPointInputs& in) noexcept;

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
    float aim_point_height_50 = 0.0f;       // approach+50h
    // (approach->+14h) is &PilotBotConfig.levels[difficultyIndex], a
    // PilotBotParameters row; include/bsp/robot_config.hpp names its members and
    // a row offset N is the member whose suffix is N + 0Ch. So these two are the
    // authored, difficulty-scaled aiming imprecision, which is why the release
    // gate is a 25-metre window rather than an angle.
    float lead_at_high_5c = 0.0f;   // dive_bomb_aim_prec_dist_068, 009C5C20
    float gain_at_high_60 = 0.0f;   // dive_bomb_aim_prec_mul_06c, 009C5C69
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
    float aim_point_height_50 = 0.0f;     // approach+50h
    // CORRECTION (packet cc8_dive_aim_error). This was `slant_range`, "the
    // range the test compares", and the host fed it the planar range. [ESP+14h]
    // is not a range: 009C59BA-009C59D6 writes it as `unit+100h - target.y`,
    // and it is the SAME slot the aim error reads as its interpolation x
    // (DiveBombAimErrorInputs::height_above_target, 009C59D6). Both places the
    // abort uses it - 009C5B12 against approach+D4h + approach+50h, and
    // 009C5B30's `* 0.3` - are therefore in metres of ALTITUDE above the aim
    // point. Feeding a range made 009C5B3E read `0.3*range + 150 > range`,
    // i.e. an abort at any range under 214 m whatever the altitude.
    float height_above_target_14 = 0.0f;   // [ESP+14h]
    // [ESP+1Ch] at 009C5B14, which is the FIRST sqrt (009C5A0B, over the
    // aircraft-relative dx/dz at [ESP+34h]/[ESP+3Ch]), not the latched one -
    // that is [ESP+5Ch], written by the second sqrt at 009C5A4D.
    float aim_point_distance = 0.0f;
    float unit_attitude_c64 = 0.0f;  // pose+C64h, 009C5B1D
};
bool dive_bomb_dive_abort_009c5b43(const DiveBombDiveAbortInputs& in) noexcept;

// ---------------------------------------------------------------------------
// The aimglide tick is 009C5180-009C580B, and its release chain begins at
// 009C5689, not 009C5693: this comment read "009C5693-009C57A6", the narrow
// range this repository cited everywhere until packet cc8_dive_glide walked the
// body. Correct it in place wherever it survives. A salvo: it asks
// 007C1DB0 how many rounds the unit still has and calls 007BBBA0 that many
// times in the loop at 009C5771-009C5784.
//
// coverage: complete for 009C5689-009C57BB. Packet cc8_dive_glide walked
// 009C5180-009C580B with frame bases (SUB ESP,58h + PUSH EBX/EBP/ESI/EDI, so
// the canonical frame is 0x68 below entry and [ESP+6Ch] is the `dt` argument,
// which the body then reuses as scratch) and traced every one of the five gate
// slots to its producer. The `three range terms ... not traced` note is
// withdrawn.
// ---------------------------------------------------------------------------
struct DiveBombAimGlideReleaseInputs {
    // 009C5689 `COMISS XMM2,[ESI+1Ch]` with XMM2 = 0.0 from 009C562D, which
    // dominates it (every jump into 009C562D-009C5689 starts inside that
    // block). JBE bails, so the release needs `state+1Ch < 0`: the same rearm
    // countdown the aimdive gates on, drawn as uniform(0.2, 0.5) at 009C579E
    // and counted down by dt at 009C519D-009C51A5. The host was skipping this
    // gate entirely, which would let a satisfied geometry release every tick.
    float rearm_timer_1c = 0.0f;    // state+1Ch, [ESI+1Ch]
    // RENAMED from `dive_angle`, packet cc8_dive_glide: it is the abs of the
    // horizontal bearing error, not a dive angle. See kGlideDiveAngleLimit.
    float bearing_error_18 = 0.0f;  // [ESP+18h], 009C53CA, already abs
    // CORRECTION. These two were `height_above` at [ESP+1Ch] and `height_limit`
    // at [ESP+20h], and the gate was written `height_above + 50 > height_limit`
    // - the operands the wrong way round, which made it demand ALTITUDE rather
    // than a ceiling. 009C56A6 loads [ESP+20h] FIRST and 009C56AA loads
    // [ESP+1Ch] second, so 009C56B4's FCOMIP compares `[ESP+1Ch] + 50` against
    // `[ESP+20h]`: the ceiling is the +1Ch slot.
    //
    // Both producers are now read.
    //  [ESP+20h], written at 009C5281: the same construction as the flyabove's
    //  B - 009C5278 calls approach->vtable[0], 009C527A takes its out[1] and
    //  009C527D FSUBR subtracts it from the aircraft's Y. The height above the
    //  aim point.
    //  [ESP+1Ch], written at 009C5493: `(approach+14h)->+40h * approach+A8h`
    //  (009C548A/009C548D). approach+14h is the 0x248-stride robots row viewed
    //  0xCh in, so ->+40h is row+4Ch, dive_bomb_new_release_mul_04c. This
    //  installation's SPNormal row authors DiveBombNewReleaseMul = 0.6, and its
    //  comment says it outright: "ha nem leboritott manoverrel bombaz, csak
    //  siman rarepulve, akkor a fenti ReleaseAlt erteket ennyivel megszorozva
    //  hasznalja" - bombing without the wingover, it uses ReleaseAlt times this.
    // So the glide release ceiling is 0.6 * 350.0 + 50.0 = 260.0 m.
    float height_above_aim_point = 0.0f;   // [ESP+20h]
    float glide_release_ceiling = 0.0f;    // [ESP+1Ch]
    // BOTH PRODUCERS RECOVERED, packet cc8_dive_glide. The hypothesis this
    // stream carried - `lateral_a` the throw and `lateral_b` the MISS - is half
    // right and half withdrawn. 009C5204-009C5256 builds two planar vectors
    // from approach+D8h/+E0h, and an earlier block 009C51CC-009C51F7 builds a
    // third, so there are THREE, not two:
    //   [ESP+38h]/[ESP+40h] = aimPoint - unit    (009C51D5-009C51F7)
    //   [ESP+44h]/[ESP+4Ch] = impactPoint - unit (009C5207-009C522E, the throw;
    //       the 009C521B PUSH EAX is why 009C521C's literal [ESP+48h] and
    //       009C522E's literal [ESP+50h] are frame slots 44h and 4Ch)
    //   [ESP+50h]/[ESP+58h] = aimPoint - impactPoint (009C5234-009C5256)
    // Their magnitudes go to [ESP+10h] (009C52B6/009C52C1), [ESP+14h]
    // (009C52F8/009C5303) and [ESP+1Ch] (009C533A/009C5345) respectively.
    // 009C56BE/009C56C2 then read [ESP+14h] and [ESP+10h] - so the 120 m gate
    // and the lead are the THROW against the RANGE TO THE AIM POINT. The miss
    // is not a release input at all: its only consumer is 009C53D8's 140 m
    // command-arm split, and 009C5493 overwrites [ESP+1Ch] with the release
    // ceiling before the gates run.
    float lateral_a = 0.0f;         // [ESP+14h], |impactPoint - unit| planar
    float lateral_b = 0.0f;         // [ESP+10h], |aimPoint - unit| planar
    float travel_accumulator_20 = 0.0f;  // state+20h
    int rounds_available = 0;       // 007C1DB0(unit)
    int rounds_cap = 0;             // EBP, the literal 2 at 009C53DD
    float rearm_draw = 0.0f;        // BSP_Random_UniformFloatRange(0.2, 0.5)
    float drift_rate_a4 = 0.0f;     // approach+A4h, the +20h feed
};
struct DiveBombAimGlideReleaseResult {
    bool released = false;
    int rounds_released = 0;        // one 007BBBA0 each, one approach+2Ch each
    float rearm_timer_1c = 0.0f;
    float travel_accumulator_20 = 0.0f;
    // INSTRUMENTATION, not an image output: how far down the gate chain the
    // call got, so a run can name the binding gate instead of only counting
    // zeroes. 0 rearm 009C5689, 1 bearing 009C569B, 2 ceiling 009C56B8,
    // 3 lateral 009C56FE, 4 lead-upper 009C5745, 5 lead-lower 009C5755,
    // 6 past every gate. The image returns nothing at all.
    int gate_reached = 0;
    float lead = 0.0f;              // 009C5725, the value the window tests
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
// 009C62B0, the flyabove tick. Ghidra had no function; this packet defined one
// over 009C62B0-009C7085, 3542 bytes. Its can-dive flag is recovered; the
// roll-in flag is not. docs/DIVE_BOMB_TASK.md has the flag census.
// ---------------------------------------------------------------------------

// 009C67C7-009C680E: flyabove+18h, the can-dive decision the transition rule
// reads at 009C8563. The height above the target comes from 009C647D-009C6493,
// the same quantity the aimdive tick forms at 009C59D6: the double at [ESP+10h]
// minus the y of the point approach->vtable[0] returns. `JBE` at 009C67F6 takes
// the zero arm, so the flag is set only when the height strictly exceeds the
// release range.
bool dive_bomb_flyabove_can_dive_009c680e(float height_above_target,
                                          float release_range_d4) noexcept;

// 009C673F-009C67B0: flyabove+19h, the roll-in permission the transition rule
// reads at 009C854D. The bearing error comes from the wrap into [0, 2pi) at
// 009C6749 and BSP_Math_SubtractWrappedAngle at 009C6765, folded to its
// absolute value at 009C6796. `JA` at 009C67A7 sets the flag when that error
// exceeds the double 1.6 at 00CE3D48, 91.7 degrees: the target is behind the
// wing line, which is when a dive bomber rolls in.
//
// The second arm, `COMISS`/`JC` at 009C67A9, sets it when the clamped quantity
// at frame slot K=104 is not positive. That slot is max(x, 0) from
// 009C65E3-009C65FD, and x's own producer is one level further back and NOT
// established, so the caller passes it and the host still substitutes.
inline constexpr double kFlyAboveRollInBearing = 1.600000023841858;  // 00CE3D48, qword; 009C6790 FLD double ptr. The FLOAT at those bytes is -1.084202e-19, so the width is load-bearing.
bool dive_bomb_flyabove_roll_in_009c67b0(float bearing_error,
                                         float clamped_slot) noexcept;

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
inline constexpr float kDescentScaleRatioLow = 0.10000000149011612f;   // 00D7A2F0
inline constexpr float kDescentScaleAtLow = 0.4000000059604645f;       // 00CE7804
inline constexpr float kDescentScaleRatioHigh = 0.3499999940395355f;   // 00CF6560
inline constexpr float kDescentScaleAtHigh = 1.0f;                     // the FLD1
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
    float aim_point_height_50 = 0.0f;     // approach+50h
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
    float descent_scale = 0.0f;     // 009FBA50 arg3, then 009FB800 arg2
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
inline constexpr double kRollHandOver = 0.800000011920929;  // 00CE3D40, qword; 009C45B3 FLD double ptr, 45.8 deg
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

// ---------------------------------------------------------------------------
// 009C6493-009C66F2, the one height the flyabove tick's two decisive flags
// share. B is the aircraft's height above the aim point: 009C647B calls the
// approach's vtable[0] (009C40A0), 009C647D takes its out[1] - approach+50h -
// and 009C6482/009C6493 store `aircraftY - out[1]`. The same frame slot feeds
// 009C67C7, the first argument of the can-dive test, so all three read one
// value. docs/DIVE_BOMB_TASK.md, "T closed".
//
// Jump senses from the bytes: 009C65A9 `76` JBE, 009C67AE `72` JC,
// 009C66E1 `76` JBE.
// ---------------------------------------------------------------------------
namespace dive_bomb_flyabove_constant {
// 009C65AB / 009C658D: the floor under B, as a float and as the double the
// compare loads. dive_bomb_constant::kMoveToRangeBias is the same 00D7A220.
inline constexpr float kHeightFloor = 100.0f;            // 00CE3D08
// 009C65C5 and 009C65CB.
inline constexpr double kHeightScale = 0.7000000029802322;  // 00CEFFA0, qword
inline constexpr double kHeightBias = 200.0;                // 00CE4D70, qword
// 009C662F and 009C660B: the leave tolerance runs from 20 degrees to pi.
inline constexpr float kLeaveToleranceLow = 0.3490658700466156f;  // 00CE398C
inline constexpr float kLeaveTolerancePi = 3.1415927410125732f;   // 00D7A264
// 009C661B: the share of approach+B4h the tolerance's far endpoint uses.
inline constexpr double kLeaveSpanScale = 0.800000011920929;      // 00CE3D40, qword
// The altitude arm's constants, packet cc8_dive_entry. Each is at the width of
// the instruction that loads it: FMUL/FLD `double ptr` for the three doubles,
// MOVSS for the two floats, and the 1.0 at 009C6EBD/009C6F1B is an FLD1.
inline constexpr double kNewReleaseMargin = 1.100000023841858;  // 00CE3DF0, qword; 009C6570 FMUL double ptr
inline constexpr double kDeadBandScale = 0.15000000596046448;   // 00CE6618, qword; 009C6E6B FMUL double ptr
inline constexpr double kClimbGain = 3.0;                       // 00D7A2B0, qword; 009C6EDF FMUL double ptr
inline constexpr float kClimbReferenceCap = 1.0f;               // 00D7A24C, MOVSS at 009C6EB5/009C6F23
inline constexpr float kDiveReferenceCap = 0.800000011920929f;  // 00CE74F8, MOVSS at 009C6F53
// The bank arm's constants, packet cc8_dive_flyover. Each at the width of the
// instruction that loads it: FLD/FSUB `double ptr` for the two doubles, FLD
// `dword ptr` for the two dead-band endpoints, MOVSS for the slew limit.
// 009C6662 / 009C6658: the span dead band of 009C6674, 30 degrees at span 0
// falling to 0 at span 200 m. Both FLD `dword ptr`.
inline constexpr float kSpanDeadBandLow = 0.5235987901687622f;  // 00CEC724
inline constexpr float kSpanDeadBandSpan = 200.0f;              // 00CE386C
inline constexpr double kRollInSinHalf = 0.5;      // 00D7A280, qword; 009C686F FLD double ptr
inline constexpr double kRollInAlongTrack = 120.0; // 00D1F3F8, qword; 009C68C2 FSUB double ptr
inline constexpr double kTurnCircleMul = 1.399999976158142;      // 00D045F0, qword; 009C684D
inline constexpr float kBankDeadBandNear = 1.7453292608261108f;  // 00CEDD00, FLD at 009C68FC
inline constexpr float kBankDeadBandFar = 0.1745329350233078f;   // 00CE3990, FLD at 009C68E5
// 009C65A3/009C682C MOVSS. L, the slew limit on the commanded heading. The
// 10-degree L that 009C6497 loads survives only on the `+1Bh != 0 && BL == 0`
// path, and kOldStyleBombing1b below closes that path in this installation.
inline constexpr float kHeadingSlewLimit = 1.5707963705062866f;  // 00CE3C64
// flyabove+1Bh, packet cc8_dive_flyover. 009C6813 `MOV [ESI+1Bh],DL` is the
// ONLY non-constant writer of that byte anywhere in the image, and on every
// path into it DL is base[ESP+27h], loaded at 009C654A or 009C67CB from
// squadron+3A8h - the SAME old-style-bombing flag 009C6554's skip reads. The
// other three writers are constant zero: the state constructor 009C61A6, the
// task constructor's inlined copy 009C75D1 (EBX = 0), and the fly-over's own
// enter 009C628F. A whole-image store census over offset 0x1B at both disp8
// and disp32 (tools/store_census.py 0x1b, 19 hits) finds no other. No script
// in this installation calls luaMW_SquadronSetOldStyleBombing, so the byte is
// 0, +1Bh is 0, 009C6544's JNE is never taken, and the 210 m release clamp at
// 009C657C is UNCONDITIONAL here. See docs/DIVE_BOMB_FLYOVER_FLAGS.md.
inline constexpr bool kOldStyleBombing1b = false;
}  // namespace dive_bomb_flyabove_constant

// ---------------------------------------------------------------------------
// 009C6E10-009C6F91, the flyabove tick's ALTITUDE arm - the command this host
// did not have at all, which is why a dive bomber held its cruise height across
// the whole state and handed the turndown whatever altitude it happened to be
// flying at. Packet cc8_dive_entry.
//
// Read from the listing, with every jump sense from the branch byte:
//
//   C      = approach+0Ch ? ctl+398h : approach+ACh + approach+50h
//                                              009C6491 TEST / 009C64A6 `74` JZ
//   R      = (approach+14h)->+40h * approach+A8h            009C655F-009C6568
//   C     := R  when  C > 1.1 * R                           009C657C `76` JBE
//   base   = approach+ACh + approach+50h                    009C6E48-009C6E51
//   err    = B - min(approach+ACh, C)                       009C6E1C-009C6E44
//   target = min(base, C)                                   009C6E55-009C6E71
//   band   = min(0.15 * C, approach+B0h)                    009C6E77-009C6EA1
//   A      = the planar distance to the aim point           009C6379-009C63B1
//
//   err <  0     : 009FB800(target, min(3 * -err / max(A, 1), 1.0))
//                                                           009C6EAF `76` JBE
//   err <= band  : NO 009FB800 - cmd+2BCh = 0.0 with cmd+2D0h = 2, a dead band
//                  that holds level flight                  009C6F15 `76` JBE
//   otherwise    : 009FB800(target, min(2 * A / max(A, 1), 0.8))
//
// The two references are the second argument 009FB800 caps its pitch demand
// with, the same slot 009FBA50 fills for the run-in. On the dive arm A is the
// planar range in metres and is never under 1, so `2 * A / A` is exactly 2 and
// the cap at 00CE74F8 takes it: the reference is 0.8 at every geometry this
// mission produces. That is transcribed rather than folded, because the divisor
// is only pinned to A by the `1.0 <= A` test at 009C6F1D.
//
// UNCERTAIN, and labelled at the call site: `C`. The image prefers the control
// block's ordered cruise altitude ctl+398h and falls back to the approach's own
// base; this host models no control block, so it takes the fall-back. The image
// is not known to reach the other arm in this mission - approach+0Ch is
// unit+9D4h (009F9CFC) and nothing in this reconstruction fills it.
// ---------------------------------------------------------------------------
struct DiveBombFlyAboveAltitudeInputs {
    float height_above_aim_b = 0.0f;    // B, 009C6493
    float begin_altitude_ac = 0.0f;     // approach+ACh
    float aim_point_height_50 = 0.0f;   // approach+50h
    float alt_span_b0 = 0.0f;           // approach+B0h
    float release_altitude_a8 = 0.0f;   // approach+A8h
    float new_release_mul_40 = 0.0f;    // (approach+14h)->+40h, row+4Ch
    float planar_distance = 0.0f;       // A, 009C63A6
    float cruise_altitude_398 = 0.0f;   // ctl+398h
    bool has_control_block_0c = false;  // approach+0Ch, 009C6491
};
struct DiveBombFlyAboveAltitudeCommand {
    bool level_arm = false;        // 009C6F84: cmd+2BCh = 0, no 009FB800
    float target_altitude = 0.0f;  // 009FB800 arg1
    float reference = 0.0f;        // 009FB800 arg2
    int pitch_mode_2d0 = 2;        // EDX, 009C6DD5 MOV EDX,2
    // Kept for the run census, not commands.
    float limit_c = 0.0f;
    float dead_band = 0.0f;
    float height_error = 0.0f;
};
DiveBombFlyAboveAltitudeCommand dive_bomb_flyabove_altitude_009c6e10(
    const DiveBombFlyAboveAltitudeInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009C6DCD-009C6DEF, the flyabove tick's heading arm - the command the trace in
// docs/DIVE_BOMB_TASK.md indicts by its absence. `tick_state` dispatched
// nothing for kFlyAbove, so the aircraft held the run-in heading through 159
// ticks, overflew its target, and the turndown's split-S handed aimdive an
// aircraft pointed 179.9 degrees the wrong way.
//
// PARTIAL, and the partial part is the VALUE. The image writes
// `AddWrappedAngle(base, clamp(delta, -L, +L))` - the call at 009C6DC8 with the
// base at [ESP+54h] and the clamp built at 009C6D7E-009C6DB0 from [ESP+1Ch],
// [ESP+34h] and their negation - and neither the base nor the delta was traced.
// tools/frame_slot_census.py cannot be trusted in this body: it flags nine call
// sites it cannot account for, and the gap its own docstring names (an argument
// window opened by `SUB ESP,imm` and closed by the callee's `RET imm16`)
// already put the 009C69B1-era writes and the 009C6DC1 read four bytes apart.
// So the host commands the bearing to the aim point, which is the same quantity
// the run-in's own mode-2 command uses, and the offset is labelled here.
//
// The other three command arms are READ and NOT bound, for the same reason -
// their values need slots this body cannot resolve:
//   009C69B1/009C69B9  cmd+2C4h = 0.0 with cmd+2CCh = 1, a wings-level bank
//                      target handed to the planner's servo. It precedes the
//                      heading arm in the body, so the heading's mode 2 wins
//                      whenever both run.
//   009C6F84/009C6F89/009C6F91  cmd+2BCh with the pitch mode in EDX, on the arm
//                      that does NOT call 009FB800 at 009C6F7D. NO LONGER
//                      UNBOUND: packet cc8_dive_entry read the whole altitude
//                      arm 009C6E10-009C6F91 and this is its dead-band case,
//                      dive_bomb_flyabove_altitude_009c6e10's `level_arm`.
//   009C6FEA/009C6FF1/009C6FFB  cmd+2B0h = 0, cmd+2D8h = 1 and cmd+2B4h, the
//                      desired speed, built as `something + approach+A4h`
//                      (009C6FE1).
// ---------------------------------------------------------------------------
struct DiveBombFlyAboveCommandInputs {
    // state+1Ch at 009C6DCD, with 009C6DDA `75` JNZ skipping the write. NO
    // LONGER A CONTRACT: packet cc8_dive_flyover read the producer at 009C6919
    // and this host now keeps the latch, so it does suppress. See
    // dive_bomb_flyabove_bank_009c6857.
    bool suppress_heading_1c = false;
    // The commanded heading. BOUND by packet cc8_dive_flyover: the caller runs
    // dive_bomb_flyabove_dead_band_009c6a37 and then
    // dive_bomb_flyabove_slew_009c6d6f, which is the whole of 009C6A37-009C6DC8
    // bar the 007F0280 avoidance increment of 009C6D59. The name is kept for
    // the call sites; it is no longer a raw bearing.
    float heading_to_aim_point = 0.0f;
};
struct DiveBombFlyAboveCommand {
    bool wrote_heading = false;
    float heading_2c0 = 0.0f;
    int heading_mode_2cc = 0;   // 009C6DD5 MOV EDX,2
};
DiveBombFlyAboveCommand dive_bomb_flyabove_command_009c6dcd(
    const DiveBombFlyAboveCommandInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009C542C-009C5450, the aimglide tick's heading arm - the same shape as the
// flyabove's, and the state that owns the release site 009C5777. Its command
// census: 009C5400/009C5408 a bank target with cmd+2CCh = EBX = 1 (the servo),
// 009C5442/009C5450 a heading with cmd+2CCh = EBP = 2, 009C55D7/009C55DF an
// altitude with cmd+2D0h = EBP = 2, and 009C567F cmd+2D8h = 0. EBP is the 2 that
// 009C53DD loads and EBX the 1 that 009C53E2's LEA takes from it.
//
// PARTIAL, for the same reason as the flyabove: only the heading is bound, and
// its value is a labelled substitution. The image reads it from the frame slot
// [ESP+6Ch] at 009C5435, and this body's slots cannot be paired reliably either.
// The sibling arm at 009C5414 commands the YAW slot directly (cmd+284h, +288h
// and +2D4h = 0) and is not bound.
// ---------------------------------------------------------------------------
struct DiveBombAimGlideCommandInputs {
    // SUBSTITUTION, labelled: the bearing to the aim point in place of the
    // frame slot 009C5435 reads.
    float heading_to_aim_point = 0.0f;
};
struct DiveBombAimGlideCommand {
    bool wrote_heading = false;
    float heading_2c0 = 0.0f;
    int heading_mode_2cc = 2;   // 009C53DD MOV EBP,2, stored at 009C5450
};
DiveBombAimGlideCommand dive_bomb_aimglide_command_009c542c(
    const DiveBombAimGlideCommandInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009C4B44-009C4C06, the goaway tick's climb-out - the state that exists to get
// the aircraft out of its dive, and the one whose absence ditched it. The walk
// in local/usn04_circuit2.log is `aimdive` 318 -> `goaway` 1 tick -> water
// contact at -1.12 m; the pull-out edge fires, the state changes, and nothing
// happens because `kGoAway` dispatched nothing.
//
// Its command census, with EBX the 1 that 009C4A5C loads:
//   009C4BE0 / 009C4BE8  cmd+2BCh the pitch target, cmd+2D0h = 1
//   009C4BFE / 009C4C06  cmd+2C4h = 0.0 (XORPS), cmd+2CCh = 1
//   009C4CA7, 009C4CE7   cmd+2D8h = 0
//   009C4DF0 / 009C4DF6  a second bank-target arm, cmd+2CCh = 1
//   009C4E17 / 009C4E1D  cmd+2C0h with cmd+2CCh = 2, a heading arm
//
// Both modes are 1, and that is the whole shape: hand the planner a pitch
// target and let its own arm at 0099E490 fly it, and hand the roll servo a
// wings-level target so the aircraft rolls upright out of the inverted dive.
// The mode-1 pair is exactly what the gates bound in the last two packets pass.
//
// The pitch target is the larger of two clamped interpolations over the
// aircraft's own altitude (`[EDI+100h]`), 009C4B61 and 009C4BB3, taken by the
// `77` JA at 009C4BC8. The second one's endpoints are recovered; the first's
// `y1` and interpolant are not, so it is supplied as the same curve, labelled.
// ---------------------------------------------------------------------------
namespace dive_bomb_goaway_constant {
inline constexpr float kClimbFullAltitude = 60.0f;    // 00CEB4B0, 009C4BAA
inline constexpr float kClimbEaseAltitude = 300.0f;   // 00CE3AE8, 009C4B48/009C4B96
}  // namespace dive_bomb_goaway_constant

// 009C7F00-009C7FD6, __thiscall(goaway state) -> bool. The completion rule, now
// read whole. It was PARTIAL and only its first half was modelled, which is why
// goaway finished on its first tick in every run.
//
//   term  = state+20h * 0.9 (00D7A390), and * 0.9 AGAIN at 009C7F7A when
//           ctl+369h and 00E17BF2 and state+D1h are all set
//   ceil  = min(ctl+398h, approach+ACh + approach+50h)     009C7F38, `76` JBE
//   done  = approach+BCh > term                            009C7F8D, `76` JBE
//           AND aircraft Y > ceil - 100.0 (00D7A220)       009C7FC2, `76` JBE
//
// The second condition is the half that was missing, and it is the whole point
// of the state: goaway ends only once the aircraft has BOTH opened the range and
// climbed back to within 100 m of its cruise altitude. With approach+ACh at
// 1000.0 and the aim point at sea level that is 900 m, so an aircraft at 650 m
// coming out of a dive stays in goaway and keeps climbing.
struct DiveBombGoAwayCompleteInputs {
    float planar_distance_bc = 0.0f;  // approach+BCh
    float travel_20 = 0.0f;           // the goaway state's own +20h
    float altitude = 0.0f;            // the aircraft's world Y
    float cruise_altitude_398 = 0.0f;  // ctl+398h
    float begin_altitude_ac = 0.0f;    // approach+ACh
    float aim_point_height_50 = 0.0f;  // approach+50h
    bool has_bomb_ordnance_d1 = false;
    bool control_flag_369 = false;
    bool global_e17bf2 = false;
};
bool dive_bomb_goaway_complete_009c7f00(
    const DiveBombGoAwayCompleteInputs& in) noexcept;

struct DiveBombGoAwayInputs {
    float altitude = 0.0f;         // [EDI+100h], the unit's world Y
    float climb_angle_1ec = 0.0f;  // (approach+8h)->+1ECh, 009C4BA0
};
struct DiveBombGoAwayCommand {
    float pitch_target_2bc = 0.0f;  // 009C4BE0
    int pitch_mode_2d0 = 1;         // 009C4BE8, EBX
    float bank_target_2c4 = 0.0f;   // 009C4BFE, the XORPS zero
    int heading_mode_2cc = 1;       // 009C4C06, EBX
    int air_brake_mode_2d8 = 0;     // 009C4CA7 / 009C4CE7
};
DiveBombGoAwayCommand dive_bomb_goaway_climb_009c4b44(
    const DiveBombGoAwayInputs& in) noexcept;

struct DiveBombFlyAboveSpan {
    float floored_height = 0.0f;  // max(B, 100.0), the 009C65A9 select
    float threshold = 0.0f;       // S = floored * 0.7 + 200.0
    // x = max(R - S, 0), 009C65D5-009C65FD. R is the PLANAR RANGE, base
    // [ESP+28h], written once at 009C63A6; NOT the height. Packet
    // cc8_dive_heading; the stack walk is in the .cpp.
    float span = 0.0f;
};
// 009C658D-009C65FD. `span` is what both flags below consume. Two different
// quantities go in: the threshold is built from the height, the span is the
// range less that threshold.
DiveBombFlyAboveSpan dive_bomb_flyabove_span_009c65fd(
    float height_above_aim_point, float planar_range) noexcept;

// 009C66D5-009C66E7: leave flyabove when the bearing error beats a tolerance
// that opens from 20 degrees at span 0 to pi at span `+B4h * 0.8 - S`.
bool dive_bomb_flyabove_leave_009c66e3(float bearing_error,
                                       const DiveBombFlyAboveSpan& span,
                                       float attack_distance_b4) noexcept;

// 009C664B-009C6674: T on the paths that leave the body before the bank arm.
// InterpolateClamped(0.0, 30 deg, 200.0, 0.0, span) - both zeros are the FLDZ
// at 009C6652, whose FST (not FSTP) leaves the value on the stack for 009C666C.
float dive_bomb_flyabove_span_dead_band_009c6674(float span) noexcept;

// ---------------------------------------------------------------------------
// 009C64EE-009C6530, BL: the predicate that admits the whole bank arm. Read
// from the listing by packet cc8_dive_flyover; the x87 depths are the frame
// walk's (tools/flyabove_trace.ps1), which carries R, B and C on the stack
// across the four-way merge at 009C6532.
//
//   009C64EC  AL = vtable[5Ch](0x14) on (approach+0Ch)->+4  - UNBOUND here
//   009C64FC  AL != 0                     -> BL = 0
//   009C6510  approach+D4h  >  C          -> BL = 0   (FCOMI/JA)
//   009C651A  approach+B4h <=  R          -> BL = 1   (FCOMPI/JBE)
//   009C6522  B < approach+D4h            -> BL = 0   (FCOMI/JB), else BL = 1
//
// C is the commanded altitude of 009C64C9, i.e. the `limit_c` the altitude arm
// returns; B is the height above the aim point; R is the three-second lead
// range. `state_query_14` is the one unbound input and it is a LABELLED
// SUBSTITUTION at the call site.
bool dive_bomb_flyabove_bank_arm_009c6530(bool state_query_14,
                                          float release_range_d4,
                                          float limit_c,
                                          float attack_distance_b4,
                                          float lead_range_r,
                                          float height_above_aim_b) noexcept;

// ---------------------------------------------------------------------------
// 009C6857-009C6923, the fly-over's bank arm: the roll-in latch flyabove+1Ch
// and the dead-band half-width T. Packet cc8_dive_flyover; this closes the two
// T producers packet cc8_dive_heading left unread (009C6893 and 009C6911) by
// walking the argument window of 009C6909 with the frame base the `SUB ESP,14h`
// at 009C68DA moves to 0xAC.
//
//   Eabs = |E|, base[ESP+2Ch], the fold at 009C642F-009C6453 (-0.0 - E)
//   R    = base[ESP+28h], the lead range
//   Rt   = classDesc+268h TurnCircleRadius * 1.4, base[ESP+30h] at 009C6853
//
//   009C6861  +1Ch already set        -> skip the arm entirely (the latch)
//   009C6889  1.5 * sin(Eabs) * R > Rt-> +19h = 0, T = 0 (009C6893's XMM0)
//   009C68A0  otherwise                  approach+CCh = 3, the weapon selector
//   009C68D4  cos(Eabs) * R - 120 <= 0-> +1Ch = 1, the LATCH; T untouched
//             otherwise               -> +19h = 0 and 009C6911's
//                T = InterpolateClamped(0.0, 100 deg, Rt/1.4, 10 deg, R)
//
// The fifth argument of 009C6909 is R (the FXCH at 009C68D8 puts it in ST0
// ahead of the store at 009C68DD) and the first is the FLDZ zero of 009C68CC
// that survives the FCOMIP pop - so the dead band runs from 100 degrees at
// range 0 to 10 degrees at one turn circle, clamped.
struct DiveBombFlyAboveBankInputs {
    bool latched_1c = false;        // state+1Ch on entry, 009C6861
    bool bank_arm_bl = false;       // BL at 009C67BF; 0 leaves at 009C67C1
    float bearing_error_abs = 0.0f; // Eabs
    float lead_range_r = 0.0f;      // R
    float turn_circle_radius = 0.0f;  // classDesc+268h, NOT yet multiplied
    // T as 009C6674 left it, the value that survives when the arm is skipped.
    float span_dead_band = 0.0f;
};
struct DiveBombFlyAboveBank {
    bool latched_1c = false;      // state+1Ch after the tick
    bool clear_roll_in_19 = false;  // 009C688F / 009C68E1 write 0 to +19h
    bool weapon_select_3 = false;   // 009C68A0 approach+CCh = 3
    float dead_band_t = 0.0f;       // base[ESP+20h] as it reaches 009C6A43
    float along_track = 0.0f;       // cos(Eabs) * R, kept for the census
    float cross_track = 0.0f;       // 1.5 * sin(Eabs) * R
};
DiveBombFlyAboveBank dive_bomb_flyabove_bank_009c6857(
    const DiveBombFlyAboveBankInputs& in) noexcept;

// 009C6A37-009C6A7F, the symmetric dead band on the signed bearing error E,
// half-width T. 009C6A46's JBE skips the whole arm when T <= 0, and there the
// commanded heading IS the bearing.
//   E > 0  -> max(E - T, 0)      009C6A4D-009C6A5F
//   E <= 0 -> min(E + T, 0)      009C6A61-009C6A6F
float dive_bomb_flyabove_dead_band_009c6a37(float bearing_error,
                                            float half_width_t) noexcept;

// 009C6D6F-009C6DC8, the slew limiter on the commanded heading:
// `AddWrappedAngle(C, clamp(SubtractWrappedAngle(A, C), -L, +L))` with C the
// aircraft's own heading (009C6406) and L kHeadingSlewLimit.
float dive_bomb_flyabove_slew_009c6d6f(float heading_c, float desired_a,
                                       float limit_l) noexcept;

// ---------------------------------------------------------------------------
// 009C5C9F-009C5DB2, the aimdive tick's steering: the only thing in the whole
// chain that points the aircraft AT its aim point.
//
// 009C58D0's tick had no binding at all - `tick_state` was an empty override -
// so through 664 live aimdive ticks the host issued no roll and no pitch, the
// planner levelled the aircraft, and it flew past the target at 444 m. That is
// the whole of the 340 m the aim census reported. docs/DIVE_BOMB_TASK.md.
//
// Both writes carry the mode that survives the planner: 009C5D1C puts EBX
// (zeroed at 009C58DE) in cmd+2D0h, which is the value the pitch gate at
// 0099E3BF lets through, and 009C5DB2 puts it in cmd+2CCh, which is neither 2
// nor 1 so the planner's roll arm is skipped entirely.
//
// Jump senses from the branch bytes: 009C5CA9, 009C5CBC, 009C5CE5, 009C5D22
// and 009C5D31 are all `76` JBE.
// ---------------------------------------------------------------------------
namespace dive_bomb_constant {
// 009C5D85 / 009C5D75: the default roll band, +/- 0.4 rad of bearing error
// mapped onto the full stick. 00D1F400 is the negative endpoint.
inline constexpr float kAimDiveRollBand = 0.4000000059604645f;   // 00CE7804
// 009C5D58 / 009C5D48: the wider band the 009C5D22/009C5D31 pair selects.
inline constexpr float kAimDiveRollBandWide = 0.5f;              // 00CE3800
// 009C5D24, the 60 degrees that second test compares against.
inline constexpr float kAimDiveRollBandAngle = 1.0471975803375244f;  // 00D05AAC
// 009C5BD4 `COMISS XMM0,[00CEC728]` on pose+C64h with 009C5BDB `0f 87` JA to
// 009C5CEF. NEGATIVE thirty degrees, at its own address: 00CEC724 next door is
// the positive one kGlideDiveAngleLimit already uses. The jump is taken when
// the pitch is ABOVE it, i.e. when the dive is SHALLOWER than 30 degrees
// nose-down, and 009C5CEF writes cmd+29Ch = -1.0 without computing the aim
// error at all.
inline constexpr float kAimDiveSteepGateAngle = -0.5235987901687622f;  // 00CEC728
// 009C4FBF, the 40 degrees below which the Euler heading is abandoned for the
// body-up axis. `MOVSS XMM0,[ESI+C64h]` then `COMISS XMM0,[00CE7D1C]` with
// 009C4FC6 `76` JBE, so pitch <= this takes the body-axis arm.
inline constexpr float kAimHeadingSteepPitch = -0.6981317400932312f;  // 00CE7D1C
// 009C4FD5, the half turn of bank past which the heading is flipped, and
// 009C4FE9 the pi it is flipped by.
inline constexpr double kAimHeadingInvertedBank = 1.5707963705062866;  // 00CE3830
inline constexpr float kAimHeadingHalfTurn = 3.1415927410125732f;      // 00D7A264
// 009C5023, the body vector 0042D0D0 transforms: (0, 100, 0), the +Y row of
// the pose. atan2 is scale-invariant, so only the row matters.
inline constexpr float kAimHeadingBodyAxisLength = 100.0f;  // 00CE3D08
}  // namespace dive_bomb_constant

// ---------------------------------------------------------------------------
// 009C4F80-009C5177, `float __thiscall(state)`, called only by the aimdive tick
// (009C51A8) and the aimglide tick (009C5935) - `tools/callsite_census.py`,
// two sites, exhaustive over rel32.
//
// It is the heading the two aim states subtract the bearing to the target from
// at 009C5AA3 and 009C5AF1, and it is NOT pose+C6Ch. Two arms, picked by pitch:
//
//   pitch > -40 deg : h = pose->vtable[50h]() (the raw heading), and if the
//                     folded |bank| is past pi/2 the aircraft is inverted, so
//                     009C4FFD adds pi.
//   pitch <= -40 deg: the nose is steep enough that the Euler heading is
//                     ill-conditioned, so 009C504E transforms (0,100,0) - the
//                     body +Y axis - by the pose at +CCh and takes the bearing
//                     of its horizontal projection, pi/2 - atan2(z, x) wrapped
//                     into [0, 2pi).
//
// The two agree: rolling 180 degrees about the forward axis negates body +Y, so
// its horizontal bearing is the heading plus pi exactly when the aircraft is
// inverted. What the routine returns is the heading of the lift vector - the
// direction the aircraft turns toward - not the direction its nose points.
//
// That matters because the dive-bomb turndown ends INVERTED by construction
// (009C7EA0 needs |bank| > 2.356 on its second arm), so the aim states run
// their whole approach in the regime where this differs from pose+C6Ch by pi.
// ---------------------------------------------------------------------------
struct DiveBombAimHeadingInputs {
    float pitch_c64 = 0.0f;       // 009C4FB7
    float bank_c68 = 0.0f;        // 009C4F90, folded at 009C4FA5-009C4FB1
    // pose->vtable[50h] at 009C4FCF. SUBSTITUTION: this host passes the cached
    // Euler heading pose+C6Ch, which is what 007C1900 writes and what
    // `unit_set_heading_target_00811960` already calls `heading_virtual_0050`.
    float heading_c6c = 0.0f;
    // Pose row 1, the body +Y axis in world coordinates. 0042D0D0 is the
    // row-vector form - out.x = in.x*m[0] + in.y*m[4] + in.z*m[8], read off
    // 0042D0F2-0042D116 - so (0,100,0) selects row 1 scaled by 100.
    float body_up_x = 0.0f;
    float body_up_z = 0.0f;
};
float dive_bomb_aim_heading_009c4f80(
    const DiveBombAimHeadingInputs& in) noexcept;

struct DiveBombAimDiveSteerInputs {
    // The aim error 009C5C9B leaves in [ESP+5Ch]; its sign picks both gains.
    float aim_error = 0.0f;
    // pose+C64h, read at 009C5BCC for the gate at 009C5BD4. Above
    // kAimDiveSteepGateAngle the image jumps to 009C5CEF and never runs
    // 009C5B54-009C5C9B, so `aim_error` is not consulted at all on that arm.
    float pitch_c64 = 0.0f;
    // What [ESP+5Ch] STILL HOLDS when that jump is taken, and this is not a
    // nicety: the slot is the incoming `float dt` parameter home, reused. The
    // last writer before the gate is 009C5A4D/009C5A58, the second sqrt - the
    // planar distance from the target point to the LATCHED approach+D8h/+E0h
    // point. 009C5D08 (the roll band test) and 009C60C1 (the release window)
    // both read that slot afterwards, and on the gated arm they therefore test
    // a DISTANCE against the 25 m window, not the aim error. Proved by
    // enumerating every access to the slot over the whole body: between
    // 009C5BDB and 009C5D08 the only writers are inside the jumped-over range.
    float planar_distance_slot_5c = 0.0f;
    // The roll interpolant, and the two arms DO take different ones - walked
    // with the frame bases, so the `SUB ESP,14h` at 009C5D37/009C5D64 is not
    // read as a displacement: 009C5D33 (the wide arm) loads `[ESP+18h]`, the
    // bearing error measured from the predicted impact point approach+D8h/+E0h,
    // and 009C5D60 (the default arm) loads `[ESP+24h]`, the one measured from
    // the aircraft. Both subtract their bearing from the same 009C4F80 heading.
    float bearing_error = 0.0f;         // [ESP+24h], 009C5AA3
    float bearing_error_wide_18 = 0.0f;  // [ESP+18h], 009C5AF1
    // pose+C68h, the bank. 009C5919-009C592D folds it into the frame slot the
    // band test at 009C5D2C reads, so the two arms are picked by attitude.
    float bank_c68 = 0.0f;
    // (approach+14h)->+64h and ->+68h, read at 009C5CAB and 009C5CD0. Two more
    // fields of the same difficulty-row record whose +5Ch and +60h the aim
    // error already uses.
    float pitch_gain_positive_64 = 1.0f;
    float pitch_gain_negative_68 = 1.0f;
};
struct DiveBombAimDiveSteerResult {
    float pitch_29c = 0.0f;   // 009C5CFA, with +2A0h = 1 and +2D0h = 0
    float roll_290 = 0.0f;    // 009C5DA3, with +294h = 1 and +2CCh = 0
    bool used_wide_band = false;  // the 009C5D33 arm rather than 009C5D60
    bool steep_gate_fired = false;  // 009C5BDB JA was taken
    // The value [ESP+5Ch] carries out of this arm, which is what the release
    // window at 009C60C1 reads: the aim error normally, the latched planar
    // distance when the gate fired. Pass this to the release, not aim_error.
    float error_slot_5c = 0.0f;
};
DiveBombAimDiveSteerResult dive_bomb_aimdive_steer_009c5c9f(
    const DiveBombAimDiveSteerInputs& in) noexcept;

// 009C3FFB-009C4045, the tail of the approach constructor: approach+D4h, the
// height above the target the aircraft must have before 009C680E lets it dive.
// max(+A8h + 250.0, (+ACh + +A8h) * 0.5); the JBE at 009C4035 is the byte `76`.
float dive_bomb_dive_entry_height_009c4045(float dive_altitude_a8,
                                           float begin_altitude_ac) noexcept;

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
