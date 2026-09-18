#ifndef BSP_TORPEDO_AIM_TICK_HPP
#define BSP_TORPEDO_AIM_TICK_HPP

// 009D15F0-009D2377, the torpedo bot's `aim` state tick.
// void __thiscall BotStateTorpedoAim::Tick(float dt), RET 4 at 009D2377.
// ESI = the state, state+4h = the approach record, approach+18h = the pilot
// command block, approach+4h = the unit.
//
// The whole body is reconstructed from the listing.  The frame accounting it
// rests on is a CFG fixpoint over the function's own control-flow graph: every
// one of the 870 instructions is reached and every join agrees on the ESP
// depth, with the callee cleanups 00419010 RET 14h, 00438AA0/00438B10 RET 8,
// 00903860 RET 8, 009FA3A0 RET 4, 007F0280 RET 18h and the callee-clean
// indirect __thiscall sites.  Slot keys below are `F`, the byte offset from the
// deepest prologue ESP (SUB ESP,64h + PUSH ESI + PUSH EDI), so F=0 is the saved
// EDI, F=6Ch the return address and F=70h the `dt` argument.
//
// The aim-complete byte state+2Ch at 009D236E is the gate the torpedo task
// waits on (009D31BF reads it; see docs/TORPEDO_TASK_ARM.md).  It is produced
// by torpedo_aim_complete_009d22ff below.
//
// Uncertainty: descriptive names here are hypotheses, not recovered symbols.
// The obstacle probe 007F0280 at 009D1A94 is not reconstructed; its branch is
// modelled through TorpedoAimTickHost::sector_probe_009d1a94 and is inert when
// the host reports no hit, which is the path approach+AAh == 0 forces anyway.

#include "bsp/torpedo_approach_update.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Constants, each the exact widening of the float the listing loads.  Every
// `double ptr` operand in this function was checked to be the widening of a
// float, so the reconstruction computes in float and widens only where the
// listing widens.
// ---------------------------------------------------------------------------
namespace torpedo_aim {

inline constexpr float kAltFloorOverLand = 30.0f;   // 00CE38C8, 009D1635
inline constexpr float kAltFloorOverSea = 5.0f;     // 00CE3850, 009D163F
inline constexpr float kGroundClearance = 5.0f;     // 00D7A370, 009D16DA
inline constexpr float kTightenScale = 0.9f;        // 00D7A390, 009D177E
inline constexpr int kTightenKindA = 0x10;          // 009D175B, 009D1E04
inline constexpr int kTightenKindB = 0x16;          // 009D176A, 009D1E13
inline constexpr int kTargetKindProbe = 6;          // 009D1710

// The sector-offset gain chain, 009D17D7 and 009D182C.
inline constexpr float kSectorGainX0 = 1.8f;        // 00CF4848
inline constexpr float kSectorGainY0 = 3.0f;        // 00CE3854
inline constexpr float kSectorGainX1 = 5.0f;        // 00CE3850
inline constexpr float kSectorGainY1 = 0.8f;        // 00CE74F8
inline constexpr float kSectorClampLo = -1.2f;      // 00D05EA4
inline constexpr float kSectorClampHi = 1.2f;       // 00CE3814
inline constexpr float kLeadGainX0 = 1.35f;         // 00D2131C
inline constexpr float kLeadGainX1 = 1.6f;          // 00D06BB4
inline constexpr float kTurnClampLo = -1.4f;        // 00CFD46C
inline constexpr float kTurnClampHi = 1.4f;         // 00D06874

// The turn-radius escape, 009D190A-009D19A0.
inline constexpr float kTurnSinCap = 1.2f;          // 00CEC160 / 00CE3814
inline constexpr float kTurnRadiusBias = 200.0f;    // 00CE4D70, 009D198C

// The sector-probe arm, 009D19B9-009D1B64.
inline constexpr float kProbeTimeScale = 72.0f;     // 00CF0058, 009D19B9
inline constexpr float kProbeGateX0 = 1.5f;         // 00CE380C
inline constexpr float kProbeGateY0 = 0.872665f;    // 00D05B40
inline constexpr float kProbeGateX1 = 3.0f;         // 00CE3854
inline constexpr float kProbeGateY1 = 0.349066f;    // 00CE398C

// The release threshold, 009D1D54-009D1DED.
inline constexpr float kTurnFactorSlope = 1.6f;     // 00CE3D48, 009D1D66
inline constexpr float kAltMarginFloor = -0.5f;     // 00CEC9E0 / 00CE69D0
inline constexpr float kTurnFactorFloor = 0.1f;     // 00D7A3A0 / 00D7A2F0

// The commanded pitch, 009D1E53-009D1EDD.
inline constexpr float kPitchRangeBias = 1200.0f;   // 00D1FAF8, 009D1E5A
inline constexpr float kPitchClampLo = 0.05625f;    // 00D21318
inline constexpr float kPitchClampHi = 0.872665f;   // 00D057E0 / 00D05B40

// The throttle chain, 009D1BDB-009D1CD7.
inline constexpr float kBankFoldX0 = -1.0f;         // 00D7A260
inline constexpr float kBankFoldY0 = 2.2f;          // 00CE89D8
inline constexpr float kBankFoldX1 = -0.2f;         // 00CE69CC
inline constexpr float kBankFoldY1 = 1.0f;          // FLD1
inline constexpr float kTimeFoldX0 = 1.5f;          // 00CE380C
inline constexpr float kTimeFoldY0 = 0.1f;          // 00D7A2F0
inline constexpr float kTimeFoldX1 = 2.0f;          // 00CE3958
inline constexpr float kAltFoldY0 = 0.1f;           // 00D7A2F0
inline constexpr float kAltFoldY1 = 1.6f;           // 00D06BB4
inline constexpr float kAltFoldX1Scale = 3.0f;      // 00D7A2B0
inline constexpr float kSpeedFoldScale = 1.2f;      // 00CEC160, 009D1C57
inline constexpr float kThrottleCap = 1.2f;         // 00CE3814
inline constexpr float kRollLimit = 1.4f;           // 00D06874, 009D1D02

// The aim-solution byte approach+130h, 009D1F99-009D2021.
inline constexpr float kAspectX0 = 0.5f;            // 00CE3800
inline constexpr float kAspectY0 = 1.0f;            // FLD1
inline constexpr float kAspectX1 = 1.0f;            // FLD1
inline constexpr float kSolutionBias = 200.0f;      // 00CE4D70, 009D2002

// The release gate chain, 009D202C-009D2233.
inline constexpr float kLeadSlack = 80.0f;          // 00CF1440, 009D204C
inline constexpr float kAltGateX0 = 0.4f;           // 00CE7804
inline constexpr float kAltGateY0 = 40.0f;          // 00CE685C
inline constexpr float kAltGateX1 = 1.0f;           // FLD1
inline constexpr float kAltGateY1 = 25.0f;          // 00CE89CC
inline constexpr float kBankRateScale = 0.006f;     // 00CEC390, 009D211C
inline constexpr float kBankRateBias = 0.8f;        // 00CE3D40, 009D2104
inline constexpr float kBankRateCap = -0.1f;        // 00CE3928 / 00CE3CB4
inline constexpr float kConeSplit = 0.8f;           // 00CE74F8, 009D2175
inline constexpr float kConeNearX0 = 0.3f;          // 00CE69C8
inline constexpr float kConeNearY0 = 80.0f;         // 00CE5444
inline constexpr float kConeNearY1 = 50.0f;         // 00CEB4D4
inline constexpr float kConeFarX1 = 1.6f;           // 00D06BB4
inline constexpr float kConeFarY1 = 15.0f;          // 00CE5380
inline constexpr float kDegToRadNum = 3.14159265358979f;  // 00CE3D28
inline constexpr float kDegToRadDen = 180.0f;       // 00CE3D20
inline constexpr float kGroundArmHeight = 1.0f;     // 00D7A24C, 009D226A
inline constexpr float kCountdownGate = 3.0f;       // 00CE3854, 009D228F
inline constexpr float kCountdownSplit = 2.0f;      // 00CE3958, 009D22B0

// The aim-complete test, 009D22FF-009D236E.
inline constexpr float kRampHalf = 0.5f;            // 00D7A280, 009D2324

}  // namespace torpedo_aim

// ---------------------------------------------------------------------------
// The pure rules.  Each is one contiguous listing region and is named for what
// the bytes do, not for a recovered symbol.
// ---------------------------------------------------------------------------

// 009D1613-009D16F8.  Slot F=28h.  The commanded altitude floor.
float torpedo_aim_altitude_floor_009d1613(bool over_land_a9, float alt_74,
                                          float alt_78,
                                          float ground_height) noexcept;

// 009D1D54-009D1DED.  Slot F=34h, the single write that reaches the
// aim-complete test's clause-1 comparand (009D22FF).
float torpedo_aim_release_threshold_009d1ded(float unit_altitude,
                                             float altitude_floor,
                                             float turn_magnitude) noexcept;

// 009D22FF-009D236E.  THE GATE.  Returns the value written to state+2Ch.
//   release_threshold  slot F=34h, 009D1DED
//   range              slot F=14h, approach+90h from 009D1604, x 0.9 at 009D1782
//   turn_magnitude     slot F=18h, |turn| 009D18F5 or |heading error| 009D1BB0
//   time_to_target     slot F=0Ch, 009D1500 from 009D160F, x 0.9 at 009D178A
//   commanded_speed    009D22DC, the +134h >= 15.0 switch between +7Ch and +80h
bool torpedo_aim_complete_009d22ff(float release_threshold, float range,
                                   float turn_magnitude, float time_to_target,
                                   float commanded_speed) noexcept;

// 009D1F99-009D2021.  The aim-solution byte approach+130h.
bool torpedo_aim_solution_009d2021(float target_aspect, float approach_84,
                                   float commanded_speed, float range) noexcept;

// ---------------------------------------------------------------------------
// The injected host: one method per native call site the tick makes.
// ---------------------------------------------------------------------------
struct TorpedoAimSectorProbe {
    bool hit{false};        // 009D1AC9, |out+54h| > 0.05
    float lateral{0.0f};    // out+54h,  009D1ACF
    float scale_a{1.0f};    // out+64h,  009D1AD5
    float scale_b{1.0f};    // out+68h,  009D1AD9
};

class TorpedoAimTickHost {
  public:
    virtual ~TorpedoAimTickHost() = default;

    // 009D160A, 009D1500(approach) -> the time-to-target metric, slot F=0Ch.
    virtual float time_to_target_009d1500() = 0;

    // 009D1679 / 009D1721, unit->vtable[50h], the world heading in radians.
    virtual float unit_heading_vtable50() = 0;
    virtual float target_heading_vtable50() = 0;

    // 009D1714 / 009D175F / 009D176E / 009D1E08 / 009D1E17, vtable[5Ch].
    virtual bool target_is_kind_vtable5c(int kind) = 0;
    virtual bool unit_is_kind_vtable5c(int kind) = 0;

    // 009D16FE, approach+CCh: is there a target entity at all.
    virtual bool has_target_cc() = 0;

    // 009D16B4 / 009D1C79 / 009D1D4F / 009D2077, 00414DB0 when unit+C8h == 0.
    virtual void refresh_world_pose_00414db0(bool pose_dirty_c8) = 0;

    // 009D16D1 and 009D2265, 00903860(world+19CCh, unit+FCh) -> ground height.
    virtual float ground_height_00903860() = 0;

    // 009D19A4, 009D1360, the committed-run-time hook.
    virtual void update_run_time_009d1360() = 0;

    // 009D1A94, 007F0280(ctl = approach+Ch, unit, extents, out...).
    virtual TorpedoAimSectorProbe sector_probe_009d1a94(float extent_x,
                                                        float extent_y,
                                                        float extent_z) = 0;

    // 009D2027, 009FA3A0(&state+18h, dt), the state's own timer accumulator.
    virtual void accumulate_timer_009fa3a0(float dt) = 0;

    // 009D2282/009D2287, the state+18h arm when the ground is close enough.
    virtual void arm_release_timer_009d2287() = 0;
};

// ---------------------------------------------------------------------------
// The sequence routine.  Everything the tick reads, in listing order.
// ---------------------------------------------------------------------------
struct TorpedoAimTickState {
    // approach record reads
    float range_90{0.0f};            // 009D15FA
    float bearing_94{0.0f};          // 009D161A
    float turn_offset_5c{0.0f};      // 009D1628
    float alt_floor_74{0.0f};        // 009D1650
    float alt_margin_78{0.0f};       // 009D1647
    bool over_land_a9{false};        // 009D1613
    bool sector_probe_enabled_aa{false};  // 009D19AE
    bool has_ordnance_132{false};    // 009D1EF2
    float elapsed_134{0.0f};         // 009D18D6, 009D1F78, 009D22DC
    float speed_late_7c{0.0f};
    float speed_early_80{0.0f};
    float aspect_scale_84{0.0f};     // 009D1FD0
    float fall_lead_a0{0.0f};        // 009D2044
    // unit reads
    float unit_altitude{0.0f};       // unit+100h, 009D1C91 / 009D1D54 / 009D207C
    float unit_bank_c64{0.0f};       // 009D1BDB, 009D1DDE, 009D2114
    float unit_bank_rate_c68{0.0f};  // 009D20D5
    bool pose_dirty_c8{false};       // 009D16A0, 009D1C47, 009D1D44, 009D206C
    // config object reads, approach+8h
    float turn_radius_268{0.0f};     // 009D1950
    float turn_radius_26c{0.0f};     // 009D1946
    float cruise_speed_25c{0.0f};    // 009D1C51
    float alt_fold_a4{0.0f};         // 009D1C84
    float pitch_scale_188{0.0f};     // 009D1E64
    float pitch_div_1ac{0.0f};       // 009D1E39
    // state reads
    bool state_flag_24{false};       // 009D2030
};

struct TorpedoAimTickResult {
    // the command block, approach+18h
    float commanded_heading_2c0{0.0f};   // 009D1D16, also approach+60h 009D1D39
    int heading_mode_2cc{2};             // 009D1D1E
    float commanded_throttle_2c8{0.0f};  // 009D1D2E
    float commanded_altitude_2bc{0.0f};  // 009D1EDD
    int altitude_mode_2d0{1};            // 009D1EE5
    float roll_limit_2e8{0.0f};          // 009D1D02
    float fade_264{0.0f};                // 009D1F36, only when t < 1.5
    bool fade_264_written{false};
    float flag_278{1.0f};                // 009D1F4A
    unsigned char flag_27c{1};           // 009D1F55
    float flag_2a8{0.0f};                // 009D1F5C
    unsigned char flag_2ac{1};           // 009D1F64
    int flag_2d8{0};                     // 009D1F6B

    bool aim_solution_130{false};        // 009D2021
    int countdown_a4{-1};                // 009D22C0 / 009D22CF, -1 = untouched
    bool clear_slot_1c_40{false};        // 009D22AB
    bool run_time_updated{false};        // 009D19A4

    // the aim-complete test's own operands, for the census
    float release_threshold_f34{0.0f};   // 009D1DED
    float range_f14{0.0f};               // 009D1604 / 009D1782
    float turn_magnitude_f18{0.0f};      // 009D18F5 / 009D1BB0
    float time_to_target_f0c{0.0f};      // 009D160F / 009D178A
    float altitude_floor_f28{0.0f};      // 009D1673 / 009D16F8
    float ramp{0.0f};                    // 009D2345
    bool clause_range{false};            // 009D235A
    bool clause_turn{false};             // 009D2368
    bool aim_complete_2c{false};         // 009D236E

    // the five-flag release chain, 009D2215-009D2233
    bool gate_state_24{false};           // F=0Bh, 009D2038
    bool gate_lead{false};               // F=08h, 009D2058
    bool gate_altitude{false};           // F=09h, 009D20BF
    bool gate_bank{false};               // F=0Ah, 009D215C
    bool gate_cone{false};               // BL,    009D220F
};

// 009D15F0.  The whole tick.  `dt` is the RET 4 stack argument, slot F=70h.
TorpedoAimTickResult torpedo_aim_tick_full_009d15f0(TorpedoAimTickHost& host,
                                                    const TorpedoAimTickState& in,
                                                    float dt) noexcept;

}  // namespace bsp

#endif  // BSP_TORPEDO_AIM_TICK_HPP
