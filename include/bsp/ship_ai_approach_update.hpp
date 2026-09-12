// 009F3090, the nested update that produces the attackmove approach point.
//
// Packet cc_ai_approach_update, worker agent/cc-ai-approach-update.
// Project C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra
// was READ-ONLY for this packet. Every descriptive name here is a hypothesis,
// not a recovered symbol. docs/SHIP_AI_APPROACH_UPDATE.md carries the evidence,
// address by address, and the Coverage table saying which bodies are partial.
//
// This header builds on bsp/ship_ai_attackmove_substates.hpp (ShipAiAttackMoveXZ,
// ShipAiAttackMoveRingSlot, the 60-slot ring and the approach sub-state step
// 009F3240), on bsp/unit_rudder.hpp (00419010, 00438AA0, 00438B10) and on
// bsp/ship_ai_throttle_ring.hpp (00415620). It redefines none of their types.
//
// Vocabulary. 009F3240 calls 009F3090 with ECX = sub+8h (`LEA ECX,[EDI+8]` at
// 009F3289), so the object this packet steps is the nested ring object that
// 009E5530 constructs, and every offset below is NESTED-relative:
//
//   `nested`  the object at sub+8h (= state+10h). `sub` is nested - 8, so the
//             four fields 009F3240 reads back are, in ITS numbering,
//             sub+11E8h = nested+11E0h, sub+1214h = nested+120Ch,
//             sub+1218h = nested+1210h, sub+1230h = nested+1228h and
//             sub+1238h = nested+1230h.
//   `owner`   [nested+0h], the ship AI brain (009E5540).
//   `unit`    [brain+0AA8h]; `shipclass` [brain+0AACh]; `target` [brain+0B20h].
//   `tune`    [brain+0AB0h], a float block this packet reads at +0h, +4h, +8h,
//             +0Ch, +10h, +14h, +18h and +1Ch. No producer was established.
//   `goal`    (brain+0B2Ch, brain+0B30h, brain+0B34h), the attackmove
//             destination docs/SHIP_AI_GOAL_VECTOR.md names.
//   `slot`    one of the 60 ring records; slot i is at nested + 4h + i*4Ch and
//             `slot.angle_08` is its bearing (ShipAiAttackMoveRingSlot).
#pragma once

#include <cstdint>

#include "bsp/ship_ai_attackmove_substates.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Shared 3D point. 009F1BC0 writes the approach point as three adjacent floats
// at nested+1228h/122Ch/1230h; 009F3240 reads only x and z.
// ---------------------------------------------------------------------------
struct ShipAiApproachPoint {
    float x{0.0f}; // nested+1228h = sub+1230h
    float y{0.0f}; // nested+122Ch = sub+1234h, never read by 009F3240
    float z{0.0f}; // nested+1230h = sub+1238h
};

// ---------------------------------------------------------------------------
// The approach mode latch at nested+1234h
// ---------------------------------------------------------------------------
//
// 009F1BC0 is the only writer in this packet: 0 at 009F1FF4 and 009F211A,
// 1 at 009F1FE2, 009F2022 and 009F2112 (EDI is loaded with 1 at 009F1F7F),
// 3 at 009F20B4, 4 at 009F20C0. Value 2 is compared by 009E6E80 (009E7002),
// 009E6A90 (009E6C9F) and 009F1BC0 (009F24FC, 009F282D) but is never assigned
// in any range this packet read, so its producer is unknown.
enum class ShipAiApproachMode : int {
    free_0 = 0,       // no engagement geometry; the range scan at 009E71A5 runs
    hold_1 = 1,       // 009E6E80 and 009E9190 short-circuit on it
    unassigned_2 = 2, // read by three routines, written by none that was read
    inside_3 = 3,     // range <= target radius + 2 * turn radius (009F20A4)
    standoff_4 = 4,   // the ordinary approach mode
};

// ---------------------------------------------------------------------------
// The nested object's tail state, as the seven callees read and write it
// ---------------------------------------------------------------------------
//
// Offsets are nested-relative. Only fields this packet established a reader or
// a writer for are listed; the record array at +4h..+B43h, the two curve
// objects at +12C0h and +13B0h (00954940) and the traffic list at +14A0h
// (009DF8F0) are not members here because their classes are not settled.
struct ShipAiApproachState {
    // Mode overrides. Both bytes set means "no approach geometry at all":
    // 009E6E80 (009E6E94) forces the standoff range to -1000, 009E7FC0
    // (009E80E7 reads +1208h only) and 009E76D0 (009E7C80) skip their arms,
    // and 009E6A90 (009E6B5B) skips the mode-4 speed limit.
    bool override_11d4{false}; // nested+11D4h
    bool override_11d5{false}; // nested+11D5h
    bool flag_11d6{false};     // nested+11D6h, cleared at 009F1DAA and 009F20DE
    float retarget_timer_11d8{0.0f}; // nested+11D8h, 009F1D84, reseeded in [2,3)
    float slot_scale_11dc{0.0f};     // nested+11DCh, fed to 009E6870/009E6640
    // The planar range from the unit to the attackmove goal, 009F1CDE. This is
    // the field 009F3240 reads as sub+11E8h and turns into the approach
    // throttle, so the throttle is `2 * (lookahead + 500 - range)` clamped.
    float goal_range_11e0{0.0f};
    float standoff_range_11e4{0.0f};  // nested+11E4h, chosen by 009E6E80
    int   committed_slot_11e8{0};     // nested+11E8h, read by 009E5E90
    float unit_heading_11ec{0.0f};    // nested+11ECh, unit->vtable[50h]() at 009F1C24
    float turn_radius_11f0{0.0f};     // nested+11F0h, 009F1D6D
    float avoid_refresh_11f4{0.0f};   // nested+11F4h, 009E91DC, reseeded in [2,3)
    float selected_bearing_11f8{0.0f}; // nested+11F8h, 009E7C28 / 009E7EA6
    float evade_timer_11fc{0.0f};     // nested+11FCh, 009E7534 / 009E75BC
    int   evade_slot_1200{60};        // nested+1200h, 60 is the idle sentinel
    int   speed_gate_1204{0};         // nested+1204h, 0/2 from 009E73E8/009E73F4
    bool  flag_1208{false};           // nested+1208h
    bool  flag_1209{false};           // nested+1209h
    // The two commands 009F3240 forwards to the brain.
    float commanded_heading_120c{0.0f}; // nested+120Ch = sub+1214h -> brain+1E0h
    float commanded_throttle_1210{0.0f}; // nested+1210h = sub+1218h -> brain+1D8h
    float wobble_phase_1214{0.0f};    // nested+1214h, advanced at 009E76E4
    float cleared_1218{0.0f};         // nested+1218h, zeroed at 009E7FD1, no reader
    float timer_121c{0.0f};           // nested+121Ch, 009F1E1E / 009F20ED
    float timer_1220{0.0f};           // nested+1220h, 009F1C07
    float timer_1224{0.0f};           // nested+1224h, 009F1C13
    ShipAiApproachPoint point_1228{}; // nested+1228h..1230h, the approach point
    ShipAiApproachMode mode_1234{ShipAiApproachMode::free_0}; // nested+1234h
    float clearance_12b4{0.0f};       // nested+12B4h, read at 009E7302/009E7324
    bool  clearance_valid_12ba{false}; // nested+12BAh, gates that read
    float avoid_radius_1290{0.0f};    // nested+1290h, 009E8153, fed to 009E6400
};

// ---------------------------------------------------------------------------
// The per-slot weights this packet's routines write
// ---------------------------------------------------------------------------
//
// 009E5530's first pass zeroes slot+18h..3Ch and the byte at +40h. The update
// writes them again every frame: 009E7FC0 zeroes +18h..+2Ch (009E7FE0..009E7FF8),
// the native scorers fill +18h, 009E74D0 writes +34h and +38h, 009E9190 writes
// +3Ch, 009E7FC0 writes +2Ch and 009E76D0 rewrites +2Ch, +30h, +34h, +38h, +3Ch
// or the byte at +40h. 009E76D0's winner is the slot with the largest sum of
// the five floats at +2Ch..+3Ch (009E79CA and 009E7BE0).
struct ShipAiApproachSlotScore {
    float raw_18{0.0f};      // +18h, written by the native scorer 009E5DA0
    float spare_1c{0.0f};    // +1Ch, zeroed by 009E7FC0, no reader found
    float spare_20{0.0f};    // +20h
    float spare_24{0.0f};    // +24h
    float spare_28{0.0f};    // +28h
    float normalized_2c{0.0f}; // +2Ch, 009E8292: raw_18 / max(raw_18) * tune[0]
    float penalty_30{0.0f};  // +30h, 009E784B: -tune[4] on a rejected slot
    float bearing_34{0.0f};  // +34h, 009E764B, from 009E74D0
    float evade_38{0.0f};    // +38h, 009E76B7, from 009E74D0
    float avoid_3c{0.0f};    // +3Ch, 009E96B9 / 009E9717, from 009E9190
    bool  blocked_40{false}; // +40h byte; 009E7852 clears it, 009E5E90 reads it
};

inline constexpr int kShipAiApproachSlotCount = 60; // 009E7FCC, 009E7788, 009E9677

// ---------------------------------------------------------------------------
// Constants, every one read from the image at the address in the comment
// ---------------------------------------------------------------------------

// 009F1BC0
inline constexpr float kApproachCommandUnset = 9999.0f;   // 00CE4C04, 009F1BF7
inline constexpr double kApproachRangeEpsilonSq = 1e-10;  // 00CE3820, 009F1CB0
inline constexpr double kApproachClassRadiusScale = 10.0; // 00CE3DC0, 009F1D2A
inline constexpr double kApproachTurnRadiusScale = 1.5;   // 00CE3D78, 009F1D41
inline constexpr float kApproachZoneQueryRadius = 25.0f;  // 00CE89CC, 009F1E7C
inline constexpr float kApproachHoldMargin1 = 2.1f;       // 00D0B3C8, 009F1F8C
inline constexpr float kApproachHoldMargin2 = 1.9f;       // 00D21A94, 009F1F96
inline constexpr float kApproachInsideMargin = 300.0f;    // 00CE3AE8, 009F2058

// 009E7FC0
inline constexpr double kApproachDecayWindow = 300.0;     // 00CE3CA8, 009E8061
inline constexpr float kApproachScoreSeedA = 20.0f;       // 00CE3930, 009E813D
inline constexpr float kApproachScoreSeedB = 60.0f;       // 00CEB4B0, 009E8159

// 009E6E80
inline constexpr float kApproachRangeDisabled = -1000.0f; // 00D7A240, 009E6EA6
inline constexpr float kApproachRangeOverrideFloor = 0.0f; // 00D7A218, 009E6ECA
inline constexpr double kApproachLeaderBonus = 200.0;     // 00CE4D70, 009E6F7B
inline constexpr double kApproachStandoffSpread = 250.0;  // 00CF8850, 009E6F8F
inline constexpr double kApproachFallbackRadius = 1000.0; // 00CE47A0, 009E6F5C
inline constexpr float kApproachGunRangeNear = 100.0f;    // 00CE3D08, 009E7061
inline constexpr float kApproachGunRangeFar = 300.0f;     // 00CE3AE8, 009E704D
inline constexpr float kApproachGunFractionNear = 0.5f;   // 00CE3800, 009E7057
inline constexpr float kApproachGunFractionFar = 0.75f;   // 00CEE07C, 009E7043
inline constexpr double kApproachNoTargetLow = 800.0;     // 00CE3948, 009E7079
inline constexpr double kApproachNoTargetHigh = 1200.0;   // 00D1FAF8, 009E7095
inline constexpr double kApproachTargetRangeHigh = 0.85;  // 00CF0B58, 009E708D
inline constexpr double kApproachLeaderShrink = 0.8;      // 00CE3D40, 009E70B4
inline constexpr double kApproachLeaderFloor = 0.9;       // 00D7A390, 009E70C2
inline constexpr double kApproachCruiseLow = 0.92;        // 00D04308, 009E714E
inline constexpr double kApproachCruiseHigh = 0.98;       // 00D219C8, 009E715A
inline constexpr float kApproachScanStep = 50.0f;         // 00D19BDC, 009E71C3
inline constexpr double kApproachScanStepScale = 0.5;     // 00D7A280, 009E71D7
inline constexpr double kApproachSideCurveScale = 0.5;    // 00D7A280, 009E744B
inline constexpr int kApproachScanSteps = 0x77;           // 009E7206
inline constexpr float kApproachScanSeedScore = 3.4028234663852886e+38f; // 00D7A248
inline constexpr float kApproachScanWeightHigh = 2.0f;    // 00CE3958, 009E7261
inline constexpr float kApproachTurnRadiusFloor = 250.0f; // 00CE77B0, 009E6FEE
inline constexpr double kApproachTurnRadiusKnee = 250.0;  // 00CF8850, 009E6FDE
inline constexpr double kApproachSideBias = 1.25;         // 00CF87C0, 009E7367
inline constexpr float kApproachSideWeightX0 = 0.3f;      // 00CE69C8, 009E73B7
inline constexpr float kApproachSideWeightY0 = 0.4363323152065277f; // 00CEB664
inline constexpr float kApproachSideWeightX1 = 1.3f;      // 00CEB4B4, 009E73A3

// 009E9190
inline constexpr float kApproachAvoidRefreshLow = 2.0f;   // 00CE3958, 009E9200
inline constexpr float kApproachAvoidRefreshHigh = 3.0f;  // 00CE3854, 009E91EE
inline constexpr double kApproachAvoidSpeedPad = 200.0;   // 00CE4D70, 009E92D5
inline constexpr float kApproachAvoidProbeRange = 300.0f; // 00CE3AE8, 009E9492
inline constexpr float kApproachAvoidSeed = 30.0f;        // 00CE38C8, 009E941D
inline constexpr double kApproachQuarterTurn = 1.5707963705062866; // 00CE3830
inline constexpr double kApproachFullTurn = 6.2831854820251465;    // 00CE3828
inline constexpr float kApproachHalfTurn = 3.1415927410125732f;    // 00D7A264

// 009E74D0
inline constexpr float kApproachEvadeArmRange = 40.0f;    // 00CE685C, 009E7542
inline constexpr double kApproachEvadeSlotOffset = 30.0;  // 00CE7630, 009E7557
inline constexpr float kApproachEvadeGain = 5.0f;         // 00CE3850, 009E757D
inline constexpr float kApproachEvadeResetWindow = 12.0f; // 00CEB4B8, 009E7593
inline constexpr float kApproachEvadeIdle = -1.0f;        // 00D7A260, 009E75B4
inline constexpr float kApproachBearingGain = 1.0f;       // 00D7A24C, 009E74EC

// 009E76D0
inline constexpr double kApproachPhaseRate = 0.2;         // 00CE3D10, 009E76DA
inline constexpr double kApproachWobbleAmplitude = 0.0;   // 00D7A258, 009E7C4C
inline constexpr float kApproachRejectPenaltyBase = -0.0f; // 00D7A208, 009E780A
inline constexpr double kApproachSlotKeepThreshold = 0.85; // 00CF0B58, 009E7814
inline constexpr float kApproachReverseWindowX0 = 0.2f;   // 00CE54A0, 009E7E5F
inline constexpr float kApproachReverseWindowY0 = 400.0f; // 00CFD710, 009E7E55
inline constexpr float kApproachReverseWindowX1 = 1.2f;   // 00CE3814, 009E7E4B
inline constexpr float kApproachReverseWindowY1 = 50.0f;  // 00CEB4D4, 009E7E41

// 009E6A90
inline constexpr double kApproachThrottleUnsetKnee = 1000.0; // 00CE47A0, 009E6AC8
inline constexpr float kApproachThrottleSeedX0 = 0.5235987901687622f; // 00CEC724 (pi/6)
inline constexpr float kApproachThrottleSeedY0 = 1.0f;    // FLD1 at 009E6B03
inline constexpr float kApproachThrottleSeedX1 = 1.2217304706573486f; // 00CE3988 (70 deg)
inline constexpr float kApproachThrottleSeedY1 = 0.5f;    // 00CE3800, 009E6AEF
inline constexpr double kApproachStopWindow = 50.0;       // 00CE3938, 009E6BB2
inline constexpr float kApproachCreepFloor = 0.25f;       // 00CE3868, 009E6C1C
inline constexpr float kApproachCreepKnee = 200.0f;       // 00CE386C, 009E6C12
inline constexpr float kApproachLeaderStopRadius = 100.0f; // 00CE3D08, 009E6C45
inline constexpr float kApproachAcceptedThrottle = 0.1f;  // 00D7A2F0, 009E6C8C
inline constexpr float kApproachMode2X0 = 50.0f;          // 00CEB4D4, 009E6CD9
inline constexpr float kApproachMode2Y0 = 0.4f;           // 00CE7804, 009E6CCF
inline constexpr float kApproachMode2X1 = 300.0f;         // 00CE3AE8, 009E6CC5
inline constexpr double kApproachGateRange = 80.0;        // 00CF1440, 009E6D1C
inline constexpr double kApproachAsternWindow = -50.0;    // 00CE4938, 009E6D58
inline constexpr float kApproachHeadingDeadband = 0.2617993950843811f; // 00D05AA8 (pi/12)
inline constexpr float kApproachAheadCap = 2.0f;          // 00CE3958, 009E6D94
inline constexpr float kApproachAsternCap = 0.5f;         // 00CE3800, 009E6DF9
inline constexpr double kApproachAsternDivisor = 40.0;    // 00D7A378, 009E6E10

// 009E5E90
inline constexpr double kApproachSlotHalfWidth = 0.05235987901687622; // 00D1A8A0 (pi/60)
inline constexpr double kApproachSlotDivisor = 60.0;      // 00CE3D68, 009E5EC4
inline constexpr double kApproachTurnDeadband = -0.1;     // 00CE3928, 009E5F1F
inline constexpr float kApproachMaxTurnStep = 2.6179940700531006f; // 00D1FED0 (150 deg)
inline constexpr int kApproachBlockedSlotCost = 60;       // 009E5F7F, 009E5FAB

// 009D68B0
inline constexpr double kApproachTangentEpsilonSq = 1e-10; // 00CE3820, 009D6946
inline constexpr float kApproachTangentMinSeparation = 1.0f; // 00D7A24C, 009D69AF
inline constexpr float kApproachTangentRandomTurn = 6.2831854820251465f; // 00CE3D9C

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 009F1C6A..009F1CDE. The planar range from the unit's world position to the
// attackmove goal. The image sums dx*dx + 0*0 + dz*dz (the FLDZ/FMUL ST0 at
// 009F1CA2 is a literal zero y term), compares the sum against 1e-10 with
// FCOMI/JBE, and takes 00BF7030 (sqrt) only on the strictly-greater side; the
// other side stores exactly 0.0f. A NaN sum therefore yields 0.0f.
float ship_ai_approach_goal_range_009f1bc0(float goal_x, float goal_z,
                                           float unit_x, float unit_z) noexcept;

// 009E7DAE..009E7DDD and 009E95FA..009E9627, the same idiom in both routines:
// a = atan2(dz, dx) through _CIatan2 (00BF701A, which takes ST(1) as y and
// ST(0) as x); h = pi/2 - a; h += 2*pi when h < 0. This is the game's from-+Z
// heading convention, the same one 009E5530 uses to build the ring.
float ship_ai_approach_heading_from_delta(float dx, float dz) noexcept;

// 009E74D0's two per-slot weights, 009E75E0..009E76B7.
//   bearing_34 = interp(0, tune[4], pi, 0, |wrap(unit_heading - slot_angle)|)
//   evade_38   = interp(0, tune[5] * gain, tune[6], 0, |wrap(evade_ref - slot_angle)|)
// `gain` is 1.0 except on the path through 009E7577, which sets it to 5.0
// (009E757D, 009E758D). `evade_ref` is `(float)state.evade_slot_1200`.
// Both arguments are made non-negative with an AND of 0x7FFFFFFF on the float
// bits (009E7619, 009E7688), not with fabsf, so a NaN difference stays NaN.
float ship_ai_approach_bearing_weight_009e74d0(float unit_heading,
                                               float slot_angle,
                                               float tune_10) noexcept;
float ship_ai_approach_evade_weight_009e74d0(float evade_reference,
                                             float slot_angle,
                                             float tune_14, float tune_18,
                                             float gain) noexcept;

// 009E75E0's evade reference and the reset test, 009E754B..009E75CA. Returns
// the value the image leaves in EAX, which is what the second weight above
// uses as an angle. NOTE: nested+11F8h is a BEARING in (-pi, pi] (009E7C28
// stores slot.angle_08 into it), so `30 + bearing` never reaches 60 and the
// `ADD EAX,-0x3C` wrap at 009E756E is unreachable; likewise the reset test
// |(float)slot - bearing| < 12 at 009E75AD is never true, because the
// difference is always about 30. Both are reproduced, not corrected.
int ship_ai_approach_evade_slot_009e74d0(float selected_bearing,
                                         int current_slot) noexcept;

// 009E81E4..009E82DA. Every slot's raw score divided by the frame maximum and
// scaled by tune[0]. The image divides by the maximum without testing it for
// zero, so a frame in which every raw score is zero produces 0/0.
float ship_ai_approach_normalized_score_009e7fc0(float raw, float maximum,
                                                 float tune_00) noexcept;

// 009E967C..009E96B9. The avoidance weight, with the wrapped difference passed
// SIGNED: unlike 009E74D0 there is no absolute-value mask, so every slot whose
// bearing is below the avoidance heading clamps to the full strength.
// The mode-1 and mode-3 arm at 009E96E0 passes strength 0 and reference 0, so
// it writes 0.0f into every slot.
float ship_ai_approach_avoid_weight_009e9190(float strength, float slot_angle,
                                             float avoid_heading) noexcept;

// 009E79CA and 009E7BE0, the ring winner's score: the five floats at +2Ch..+3Ch
// added in the image's order (+3Ch first, then +2Ch, +30h, +34h, +38h).
float ship_ai_approach_slot_total_009e76d0(const ShipAiApproachSlotScore& slot) noexcept;

// 009E6AE4..009E6B17. The throttle the image seeds when the command still holds
// the 9999.0f sentinel 009F1BC0 wrote: full ahead inside pi/6 of the commanded
// heading, falling to 0.5 at 70 degrees, clamped.
float ship_ai_approach_throttle_seed_009e6a90(float heading_error_abs) noexcept;

// 009E6BC7..009E6E78, the tail of 009E6A90. `limit` is the speed cap the mode
// machine produced (1.0f by default, 009E6B2E). The image writes `-limit` when
// -limit > command (009E6BE6, FCOMIP then JBE to the other arm), `limit` when
// command > limit (009E6E4D, the same shape) and the command otherwise. Both
// comparisons are unordered-safe in the same direction, so a NaN command falls
// through both JBEs and is stored unchanged; this is not a std::clamp.
float ship_ai_approach_command_limit_009e6a90(float command, float limit) noexcept;

// 009E5E91..009E5ECC. The ring slot a bearing falls in: the bearing is offset
// by half a slot (pi/60), lifted by 2*pi when the sum is negative, scaled by
// 60/(2*pi) and truncated toward zero by _ftol2 (00BF7420). The result is not
// range-checked by the image.
int ship_ai_approach_slot_of_bearing_009e5e90(float bearing) noexcept;

// 009E5F59..009E5FC9. The cost of walking the ring from `from_slot` (the
// committed slot at nested+11E8h) round to `to_slot`, both ways. An unblocked
// slot costs 1 and sets the latch the image keeps in BL; a blocked slot costs
// 60 once that latch is set and nothing before it. The two walks are one
// routine here because the image runs them back to back over the SAME BL: the
// forward walk at 009E5F9B starts from the latch the backward walk at
// 009E5F6F left, and BL is never cleared. `blocked` is slot+40h for all 60.
void ship_ai_approach_turn_costs_009e5e90(const bool blocked[kShipAiApproachSlotCount],
                                          int from_slot, int to_slot,
                                          int& backward_cost,
                                          int& forward_cost) noexcept;

// 009E5EDF..009E6058. The commanded heading nested+120Ch. `reference` is the
// unit's current heading at nested+11ECh. The requested bearing is taken
// verbatim only when the turn is inside the band (-0.1, 150 degrees); outside
// it the heading is the reference stepped by 150 degrees toward the chosen
// side. `add_arm` selects which: the image takes the adding arm (009E6035)
// when wrap(reference - bearing) < 0 with the slot unchanged (009E5F00, JNC)
// or when the backward ring cost exceeds the forward one (009E5FC6, SETLE).
float ship_ai_approach_commanded_heading_009e5e90(float reference, float bearing,
                                                  bool add_arm) noexcept;

// ---------------------------------------------------------------------------
// 009D68B0, the circle-tangent primitive
// ---------------------------------------------------------------------------
//
// float* __fastcall(ECX = float out[2], EDX = const Circle* circle)
//                  (const float point[2], float clearance, int side),
// RET 0Ch at 009D6A3A and 009D6ABF, body 009D68B0-009D6AC1, complete. The
// circle is three floats: centre x at +0h, centre z at +4h, radius at +8h.
// `side` picks which of the two tangents 009D6550 returns: the index is
// `side == 0 ? 1 : 0` (the SETZ at 009D68C7 feeding the EBP*8 scaling).
struct ShipAiCircleTangentCircle {
    float x{0.0f};      // circle+0h
    float z{0.0f};      // circle+4h
    float radius{0.0f}; // circle+8h, read only by the degenerate arm
};

struct ShipAiCircleTangentHost {
    virtual ~ShipAiCircleTangentHost() = default;
    // 009D68EC, 009D6550 with ECX = circle and three stack arguments: the
    // point and two out pointers. Returns false when no tangent exists.
    // Body unread (it belongs to the ship_ai_nav_circle_tangent packet):
    // contract unread beyond "fills both out points and returns a flag".
    virtual bool tangent_points_009d6550(const ShipAiAttackMoveXZ& point,
                                         ShipAiAttackMoveXZ& first,
                                         ShipAiAttackMoveXZ& second) = 0;
    // 009D6A8A, 004F47B0 with ECX = a three-float block {point.x, point.z,
    // clearance} and EDX = circle, plus two out pointers. Body unread:
    // contract unread.
    virtual void offset_points_004f47b0(const ShipAiAttackMoveXZ& point,
                                        float clearance,
                                        ShipAiAttackMoveXZ& first,
                                        ShipAiAttackMoveXZ& second) = 0;
    // 009D69DD, 00BD2F10(stream 1, low, high): the shared random draw
    // 009E5530 also uses. Body unread.
    virtual float random_stream1_00bd2f10(float low, float high) = 0;
    // 009D695A and 009D69A6, 00BF7030, the CRT square root.
    virtual float sqrt_00bf7030(float value) = 0;
};

ShipAiAttackMoveXZ ship_ai_circle_tangent_009d68b0(
    const ShipAiCircleTangentCircle& circle, const ShipAiAttackMoveXZ& point,
    float clearance, int side, ShipAiCircleTangentHost& host);

// ---------------------------------------------------------------------------
// 009F3090, the nested update
// ---------------------------------------------------------------------------
//
// __thiscall(nested)(float seconds), RET 4 at 009F30E3, body 009F3090-009F30E3,
// complete. Seven calls in a fixed order on the same object; ESI holds the
// object across all of them (009F3099) and the float argument is re-pushed for
// the four that take one. Nothing else happens in the body.
struct ShipAiApproachUpdateHost {
    virtual ~ShipAiApproachUpdateHost() = default;
    virtual void frame_state_009f1bc0(float seconds) = 0; // 009F309B
    virtual void reset_scores_009e7fc0() = 0;             // 009F30A2
    virtual void choose_standoff_range_009e6e80() = 0;    // 009F30A9
    virtual void refresh_avoidance_009e9190(float seconds) = 0; // 009F30B8
    virtual void score_evade_009e74d0(float seconds) = 0; // 009F30C7
    virtual void select_slot_009e76d0(float seconds) = 0; // 009F30D6
    virtual void limit_throttle_009e6a90() = 0;           // 009F30DD
};

void ship_ai_approach_update_009f3090(ShipAiApproachUpdateHost& host,
                                      float seconds);

// ---------------------------------------------------------------------------
// 009E7FC0, the per-frame score reset and normalisation
// ---------------------------------------------------------------------------
//
// __thiscall(nested)(), RET 0 at 009E8094, 009E82EC and 009E82F8, body
// 009E7FC0-009E82F8, complete.
struct ShipAiApproachScoreResetHost {
    virtual ~ShipAiApproachScoreResetHost() = default;
    // 009E8080, 009E6400 with ECX = slot i and the float at nested+1290h.
    // Body unread: contract unread.
    virtual void decay_slot_009e6400(int slot, float radius) = 0;
    // 009E80AB, target->vtable[5Ch](5). The result is loaded into EAX and then
    // overwritten at 009E80AD without being tested, so the call is made for
    // its effect only. Callee body unread: contract unread.
    virtual void target_kind_probe_vtable_005c(int kind) = 0;
    // 009E8098, [brain+0B20h]: the target the zone test is asked about.
    virtual std::uint32_t brain_target_0b20() = 0;
    // 009E80B0, the byte at brain+0B28h. 009E80BD and 009E80CD, nested+127Ch
    // and [unit+494h].
    virtual bool brain_flag_0b28() = 0;
    virtual float nested_reference_127c() = 0;
    virtual float unit_lookahead_0494() = 0;
    // 009E8116, 00864FD0 with ECX = [unit+6DCh] and the target; or, with no
    // target, 009E6120 filling a three-float block followed by 009E8130,
    // 00864BA0 on the same object. Both bodies unread: contract unread.
    virtual bool zone_allows_target_00864fd0(std::uint32_t target) = 0;
    virtual ShipAiApproachPoint probe_point_009e6120() = 0;
    virtual bool zone_allows_point_00864ba0(const ShipAiApproachPoint& point) = 0;
    // 009E81A7, 009E5DA0 with ECX = slot i and a 44h-byte block copied from
    // nested+127Ch by a REP MOVSD of 0x11 dwords (009E819C). It is what fills
    // slot+18h. Body unread: contract unread.
    virtual float score_slot_009e5da0(int slot) = 0;
    // 009E81FA, the float at tune+0h.
    virtual float tune_scale_00() = 0;
};

// `has_target` is [brain+0B20h] != 0 (009E8098). `mode` is nested+1234h.
// `decay_elapsed` is nested+11E0h - nested+11E4h (009E8055), the mode-4 arm's
// only input.
void ship_ai_approach_reset_scores_009e7fc0(ShipAiApproachState& state,
                                            ShipAiApproachSlotScore slots[],
                                            bool has_target,
                                            ShipAiApproachScoreResetHost& host);

// ---------------------------------------------------------------------------
// 009E74D0, the bearing and evade weights
// ---------------------------------------------------------------------------
//
// __thiscall(nested)(float seconds), RET 4 at 009E76CC, body
// 009E74D0-009E76CE, complete. It has no callee other than the two library
// helpers, so the host carries only the two values it reads off other objects.
struct ShipAiApproachEvadeHost {
    virtual ~ShipAiApproachEvadeHost() = default;
    // 009E751F, the float at [unit+1128h]; the evade timer only advances while
    // it is strictly positive.
    virtual float unit_evade_flag_1128() = 0;
    // 009E75F2, 009E7656 and 009E7660: tune+10h, tune+18h and tune+14h.
    virtual float tune_bearing_10() = 0;
    virtual float tune_evade_14() = 0;
    virtual float tune_evade_span_18() = 0;
};

void ship_ai_approach_score_evade_009e74d0(ShipAiApproachState& state,
                                           const ShipAiAttackMoveRingSlot ring[],
                                           ShipAiApproachSlotScore slots[],
                                           float seconds,
                                           ShipAiApproachEvadeHost& host);

// ---------------------------------------------------------------------------
// 009E6A90, the throttle limiter
// ---------------------------------------------------------------------------
//
// __thiscall(nested)(), RET 0 at 009E6C04, 009E6E65 and 009E6E78, body
// 009E6A90-009E6E78, complete.
struct ShipAiApproachThrottleHost {
    virtual ~ShipAiApproachThrottleHost() = default;
    // 009E6AB5, unit->vtable[50h](): the unit's current heading. Called with no
    // argument; the PUSH at 009E6AB1 is the second stack slot of the 00438B10
    // that follows. Callee body unread: contract unread.
    virtual float unit_heading_vtable_0050() = 0;
    // 009E6B46, the float at [unit+1128h].
    virtual float unit_evade_flag_1128() = 0;
    // 009E6B85, 009E5E00 with ECX = nested: returns [brain+0B20h] when it is
    // non-null and answers vtable[5Ch](1Ch), else 0. Body read; see the doc.
    virtual std::uint32_t engagement_target_009e5e00() = 0;
    // 009E6B90, the INTEGER field at [target+7C4h], loaded with FILD.
    virtual std::int32_t target_radius_07c4(std::uint32_t target) = 0;
    // 009E6C3C, 00778890 with ECX = unit: true when the unit leads its group.
    virtual bool unit_is_group_leader_00778890() = 0;
    // 009E6C86, unit->vtable[234h](target). Callee body unread: contract unread.
    virtual bool target_accepted_vtable_0234(std::uint32_t target) = 0;
};

void ship_ai_approach_limit_throttle_009e6a90(ShipAiApproachState& state,
                                              ShipAiApproachThrottleHost& host);

// ---------------------------------------------------------------------------
// 009E6E80, the standoff-range choice
// ---------------------------------------------------------------------------
//
// __thiscall(nested)(), RET 0 at 009E74CD, body 009E6E80-009E74CD, complete.
// The 119-step curve scan at 009E71A5 is projected as one host call because
// the two curve objects at nested+12C0h and nested+13B0h belong to 00954940,
// whose class this packet did not read.
struct ShipAiApproachStandoffHost {
    virtual ~ShipAiApproachStandoffHost() = default;
    // 009E6E8F, 00954940 on a 124h-byte stack object that is built, never read
    // and never destroyed in this body. Body unread: contract unread.
    virtual void construct_scratch_00954940() = 0;
    // 009E6EC5, the float at tune+1Ch: a non-negative value is an explicit
    // standoff-range override.
    virtual float tune_range_override_1c() = 0;
    // 009E6EFC and 009E6F3A, target->vtable[5Ch](kind) with kind 8 then 1Ch.
    // Callee body unread: contract unread.
    virtual bool target_is_kind_vtable_005c(int kind) = 0;
    // 009E6F11, 00827F70 with ECX = shipclass. Body unread: contract unread.
    virtual bool shipclass_allows_close_00827f70() = 0;
    // 009E6F4E, the INTEGER radius at [target+7C4h]; 009E706F and 009E7087, the
    // INTEGER gun range at [target+7A0h].
    virtual std::int32_t target_radius_07c4() = 0;
    virtual std::int32_t target_gun_range_07a0() = 0;
    // 009E6F6E, 009E70A7, 00778890 with ECX = unit.
    virtual bool unit_is_group_leader_00778890() = 0;
    // 009E7036 and 009E7140, [unit+9C8h] and [unit+490h].
    virtual float unit_gun_reference_09c8() = 0;
    virtual float unit_cruise_speed_0490() = 0;
    // 009E718A, 009E711D, 009E7199, 00BD2F10(stream 1, low, high).
    virtual float random_stream1_00bd2f10(float low, float high) = 0;
    // 009E71AE and 009E71EF, 00952530 on nested+13B0h and 009523C0 on
    // nested+12C0h; 009E721A and 009E722D, 00955A40 on each at a sample point.
    // All four bodies unread: contract unread.
    virtual float curve_base_00952530() = 0;
    virtual float curve_reference_009523c0() = 0;
    virtual float curve_primary_00955a40(float x) = 0;
    virtual float curve_secondary_00955a40(float x) = 0;
    // 009E7284, the float at nested+1284h.
    virtual float nested_scan_scale_1284() = 0;
    // 009E6FCB, 00811A30 with ECX = unit and the float 1.0: the turn radius at
    // full helm. Body unread: contract unread.
    virtual float unit_turn_radius_00811a30(float rudder) = 0;
    // 009E731B, 0080DF40 with ECX = unit. Body unread: contract unread.
    virtual int unit_clearance_count_0080df40() = 0;
    // 009E74B7, 009E6870 with ECX = slot i and a four-float block
    // {side_weight, span_weight, nested+11DCh, tune+4h}. Body unread:
    // contract unread.
    virtual void score_slot_009e6870(int slot, float side_weight,
                                     float span_weight, float slot_scale,
                                     float tune_04) = 0;
    // 009E7489, the float at tune+4h, the fourth member of that block.
    virtual float tune_slot_04() = 0;
};

void ship_ai_approach_choose_standoff_009e6e80(ShipAiApproachState& state,
                                               bool has_target,
                                               ShipAiApproachStandoffHost& host);

// ---------------------------------------------------------------------------
// 009E9190, the nearby-traffic refresh and the avoidance weights
// ---------------------------------------------------------------------------
//
// __thiscall(nested)(float seconds), RET 4 at 009E96D5 and 009E9734, body
// 009E9190-009E9736, complete. It carries an SEH frame (scope 00CB0AAB) around
// the record allocation at 009E9342.
struct ShipAiApproachAvoidHost {
    virtual ~ShipAiApproachAvoidHost() = default;
    // 009E9209 and 009E91EE, 00BD2F10(stream 1, 2.0, 3.0).
    virtual float random_stream1_00bd2f10(float low, float high) = 0;
    // 009E9220, 008053C0 with ECX = [unit+54h], then the list head at +0DE8h.
    // Body unread: contract unread. The walk hands back one entity per step.
    virtual int candidate_count_008053c0() = 0;
    virtual std::uint32_t candidate_at(int index) = 0;
    // 009E9253, candidate->vtable[5Ch](5). Callee body unread: contract unread.
    virtual bool candidate_is_kind_vtable_005c(std::uint32_t candidate) = 0;
    // 009E9263, [brain+0B20h]: the attack target is never added to the list.
    virtual std::uint32_t brain_target_0b20() = 0;
    // 009E9280 and 009E9290, 00414DB0 on the unit and on the candidate when the
    // pose byte at +0C8h is clear.
    virtual void refresh_pose_00414db0(std::uint32_t entity) = 0;
    // 009E9295..009E92CF, the candidate's world x/z at +0FCh/+104h and its
    // speed at +494h; 009E93F3, the unit's own world position.
    virtual ShipAiApproachPoint entity_world_position(std::uint32_t entity) = 0;
    virtual float entity_speed_0494(std::uint32_t entity) = 0;
    virtual ShipAiApproachPoint unit_world_position() = 0;
    // 009E9342 and 009E935D, operator new(0x124) then 009E8360(record)(entity),
    // spliced into the list at nested+14A0h by 009E7F60 and 009E8DC0. Bodies
    // unread: contract unread.
    virtual void insert_traffic_record(std::uint32_t entity) = 0;
    virtual int traffic_record_count() = 0;
    // 009E94C4, 009E6170 with ECX = record and the unit position plus 300.0f;
    // false erases the record (009E9588..009E9598). Body unread.
    virtual bool traffic_record_active_009e6170(int record,
                                                const ShipAiApproachPoint& unit_pos,
                                                float range) = 0;
    virtual void erase_traffic_record(int record) = 0;
    // 009E950F, 009E6240 with ECX = record and (seconds, unit position,
    // &nested+1238h). Body unread: contract unread.
    virtual void advance_traffic_record_009e6240(int record, float seconds,
                                                 const ShipAiApproachPoint& unit_pos) = 0;
    // 009E9514..009E9542, the record's weight at +120h and its direction at
    // +10Ch/+110h/+114h.
    virtual float traffic_record_weight_0120(int record) = 0;
    virtual ShipAiApproachPoint traffic_record_direction_010c(int record) = 0;
    // 009E9642, 0042B2F0 BSP_Vector3_LengthFloatThreshold on the accumulator.
    virtual float vector_length_0042b2f0(const ShipAiApproachPoint& v) = 0;
    // 009E964E and 009E9655, tune+8h and tune+0Ch.
    virtual float tune_avoid_strength_08() = 0;
    virtual float tune_avoid_span_0c() = 0;
};

void ship_ai_approach_refresh_avoidance_009e9190(ShipAiApproachState& state,
                                                 const ShipAiAttackMoveRingSlot ring[],
                                                 ShipAiApproachSlotScore slots[],
                                                 float seconds,
                                                 ShipAiApproachAvoidHost& host);

// ---------------------------------------------------------------------------
// 009E76D0, the ring scan that picks the approach bearing
// ---------------------------------------------------------------------------
//
// __thiscall(nested)(float seconds), RET 4 at 009E7EDA, body
// 009E76D0-009E7EDC. Partial: the 6-way and 8-way unrolled bodies were read in
// their first and last step only; see the doc's Coverage table.
struct ShipAiApproachSelectHost {
    virtual ~ShipAiApproachSelectHost() = default;
    // 009E76F4, 00605070 with ECX = &nested+1214h: wraps the phase in place
    // into (-pi, pi], the same helper 009F3240 uses on brain+1E0h.
    virtual float wrap_angle_00605070(float value) = 0;
    // 009E7755, 009E6640 with ECX = slot i and five stack values: seconds,
    // nested+11DCh, the two override bytes and nested+11F0h. It returns the
    // slot's raw score. Body unread: contract unread.
    virtual float score_slot_009e6640(int slot, float seconds, float slot_scale,
                                      bool override_a, bool override_b,
                                      float turn_radius) = 0;
    // 009E784B, the float at tune+4h, subtracted from -0.0f for a rejected slot.
    virtual float tune_reject_penalty_04() = 0;
    // 009E7CB4 and 009E7CD1, the unit pose refresh 009E76D0 open-codes
    // (00414DB0 then a 16-float copy through 00413920). Bodies unread.
    virtual void refresh_unit_pose() = 0;
    // 009E7D7A and 009E7D84, brain+0B2Ch and brain+0B34h; 009E7D92 and
    // 009E7DA4, the unit's world z and x.
    virtual ShipAiAttackMoveXZ brain_goal_0b2c() = 0;
    virtual ShipAiAttackMoveXZ unit_world_xz() = 0;
    // 009E7DE7, the float at [unit+490h].
    virtual float unit_cruise_speed_0490() = 0;
    // 009E7ECB, 009E5E90 with ECX = nested and (bearing, seconds): the routine
    // that turns the chosen bearing into nested+120Ch.
    virtual void commit_bearing_009e5e90(float bearing, float seconds) = 0;
};

void ship_ai_approach_select_slot_009e76d0(ShipAiApproachState& state,
                                           const ShipAiAttackMoveRingSlot ring[],
                                           ShipAiApproachSlotScore slots[],
                                           float seconds,
                                           ShipAiApproachSelectHost& host);

// ---------------------------------------------------------------------------
// 009E5E90, the commanded-heading commit
// ---------------------------------------------------------------------------
//
// __thiscall(nested)(float bearing, float seconds), RET 8 at 009E5F56,
// 009E6032 and 009E6058, body 009E5E90-009E605A, complete. `seconds` is pushed
// by 009E76D0 at 009E7EBE and never read by the body.
void ship_ai_approach_commit_bearing_009e5e90(
    ShipAiApproachState& state,
    const bool blocked[kShipAiApproachSlotCount], float bearing);

// ---------------------------------------------------------------------------
// 009F1BC0, the frame state and the approach point
// ---------------------------------------------------------------------------
//
// __thiscall(nested)(float seconds), RET 4 at 009F3083, body
// 009F1BC0-009F3083, with an SEH frame (scope 00CB0BA0). PARTIAL: this packet
// read 009F1BC0-009F1DBF (the frame state), 009F1E60-009F1F47 (the approach
// point) and 009F1F7F-009F2124 (the mode latch), plus the four other stores to
// nested+1228h identified by their call sites. Everything else is unread; the
// doc's Coverage table lists the ranges.
struct ShipAiApproachPointHost {
    virtual ~ShipAiApproachPointHost() = default;
    // 009F1C24, unit->vtable[50h](): the unit's current heading.
    virtual float unit_heading_vtable_0050() = 0;
    // 009F1C40, 00414DB0 on the unit when the pose byte at +0C8h is clear.
    virtual void refresh_unit_pose_00414db0() = 0;
    // 009F1C45, the unit's world position at +0FCh/+100h/+104h.
    virtual ShipAiApproachPoint unit_world_position() = 0;
    // 009F1C6A, brain+0B2Ch/+0B30h/+0B34h.
    virtual ShipAiApproachPoint brain_goal_0b2c() = 0;
    // 009F1D0F, unit->vtable[5Ch](8). Callee body unread: contract unread.
    virtual bool unit_is_kind_vtable_005c(int kind) = 0;
    // 009F1D1E, the float at [shipclass+500h].
    virtual float shipclass_radius_0500() = 0;
    // 009F1D3C, 00811A30 with ECX = unit and 1.0.
    virtual float unit_turn_radius_00811a30(float rudder) = 0;
    // 009F1DB4, 00BD2F10(stream 1, 2.0, 3.0).
    virtual float random_stream1_00bd2f10(float low, float high) = 0;
    // 009F1E5E, [[target+740h]]->vtable[2Ch]() compared against the avoid-zone
    // group id at [[unit+538h]+570h] (009F1E55). Callee body unread.
    virtual int target_zone_group_vtable_002c() = 0;
    virtual int unit_zone_group_0570() = 0;
    // 009F1E77, 0082ADC0 with ECX = [unit+538h]: it reaches the avoid-zone
    // manager (004218E0) and its ST0 result is left on the x87 stack, unused,
    // across the 00417B10 call at 009F1E94. Body unread: contract unread.
    virtual float unit_avoid_radius_0082adc0() = 0;
    // 009F1E94, 00417B10 with ECX = brain and (out, unit position, 25.0f, 1).
    // Body unread (it belongs to the obstacle-tables packet): contract unread.
    virtual ShipAiAttackMoveXZ zone_exit_point_00417b10(
        const ShipAiApproachPoint& from, float radius) = 0;
};

// Covers 009F1BC0-009F1DBF and 009F1E60-009F1F47 only: the frame timers, the
// goal range, the turn radius, the retarget timer and the approach point.
// `has_zone` is [target+740h] != 0 (009F1E36).
void ship_ai_approach_frame_state_009f1bc0(ShipAiApproachState& state,
                                           bool has_target, bool has_zone,
                                           float seconds,
                                           ShipAiApproachPointHost& host);

} // namespace bsp
