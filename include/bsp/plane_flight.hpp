#pragma once
#include <cstdint>

// Projection of the plane flight controller: the object at unit+AB0h, the pilot control
// block at unit+9E4h, the fixed-step motion dispatch inside 007CE040, and the three pilot
// command helpers 009FBA50 / 009FB800 / 009F9E40.
//
// docs/PLANE_FLIGHT.md carries the evidence, the coverage table and the corrections.
// Every name here is a hypothesis, not a recovered symbol. Nothing in this header is a
// binary-compatible layout: the offset constants are the native ones, the structs are not.
//
// Contracts named but not reconstructed: the routine that writes the local player's stick or
// the bot's five planned axes into unit+9E4h (contract: unread; the candidate sites are
// 007CA509, 007D1333 and - found by packet cc7_plane_advance_pose and missing from this list
// until then - 007C6587..007C65CC, which drives the block from unit+894h, since 007CB185 and
// 007CB3CE belong to the clamp and reset in 007CAF10 and 007BB75D / 007C2B11 / 007D1676 are
// the network paths). Treat "the remaining candidates are" lists here as incomplete: this one
// said two and there are at least three;
// the matrix composition that turns the controller's state into unit+74h / unit+674h
// (docs/ENTITY_LOCAL_MATRIX.md, docs/DYN_PHYSICS_SUBSTEP.md); the 0D0h sub-object at
// controller+10h; the water arm 007DCDD0 and the ground arm 007DCCF0; the catapult and
// airbase launch of docs/AIR_OPERATIONS.md.

namespace bsp {

// ---------------------------------------------------------------------------
// The pilot control block, unit+9E4h. Five float axes and three bytes, zeroed by
// the plane unit constructor 007CFD20 at 007CFE92..007CFEB8 except the throttle,
// which 007CFEB0 seeds with 1.0f from 00D7A24C.
// ---------------------------------------------------------------------------
namespace plane_control_off {
// CORRECTION: +9E4h is YAW and +9ECh is ROLL, not the other way round, and
// +9F4h is the air brake rather than an unnamed auxiliary. Two independent
// recoveries agree. docs/PILOT_COMMAND_PATH.md tables the Lua property readers
// (007D69BA yawInput, 007D69F1 rollInput, 007D6A5F airBrakeInput), which carry
// the original authored identifiers; docs/PLANE_AI_CONTROL.md reaches the same
// assignment from the bot's own reads. The latch permutation 007B9770 confirms
// it a third time. The offsets below were always right - only the names were
// swapped, and nothing outside this header referenced them, so no behaviour
// depended on the error.
inline constexpr int kLiveYaw = 0x9E4;       // 007CFE92 zero; 0099B476 reads it
inline constexpr int kLivePitch = 0x9E8;     // 007CFE98 zero; 0099B4B2 reads it
inline constexpr int kLiveRoll = 0x9EC;      // 007CFE9E zero; 0099B494 reads it
inline constexpr int kLiveThrottle = 0x9F0;  // 007CFEB0 = 1.0f; 0099B456 reads it
inline constexpr int kLiveAirBrake = 0x9F4;  // 007CFEA4 zero; 0099B4D0 reads it
inline constexpr int kLiveByteF8 = 0x9F8;    // 007CFEAA zero; 007DC860 copies it to ctl+5h
inline constexpr int kLiveByteF9 = 0x9F9;    // 007DC84F copies it to ctl+4h
inline constexpr int kLiveByteFA = 0x9FA;    // 007B977D latches it

// The previous-step snapshot 007B9770 writes, read by the rate law 007DA710.
// Same correction as the live block above, and the latch is what proves it: it
// copies +9E4h -> +BB0h straight through, and the Lua readers name +BB0h `yawF`
// (007D6BB2) against +9E4h `yawInput` (007D69BA). A permutation that preserved
// the order while swapping two names could not do that.
inline constexpr int kLatchedYaw = 0xBB0;       // 007B9783, from +9E4h
inline constexpr int kLatchedPitch = 0xBB4;     // 007B979C, from +9E8h
inline constexpr int kLatchedRoll = 0xBB8;      // 007B97A8, from +9ECh
inline constexpr int kLatchedThrottle = 0xBBC;  // 007B97BA, from +9F0h
inline constexpr int kLatchedAirBrake = 0xBC0;  // 007B97CC, from +9F4h
inline constexpr int kLatchedByteC8 = 0xBC8;    // 007B978F, from +9F8h
inline constexpr int kLatchedByteC9 = 0xBC9;    // 007B97B4, from +9FAh
inline constexpr int kLatchedByteCA = 0xBCA;    // 007B97C0, from +9F9h
}  // namespace plane_control_off

// The unit fields the motion dispatch and the controller read.
namespace plane_unit_off {
inline constexpr int kPoseValidByte = 0x0C8;     // 00414DB0 refreshes when clear
inline constexpr int kWorldX = 0x0FC;            // 009F9E70
inline constexpr int kWorldY = 0x100;            // 009FB84D
inline constexpr int kWorldZ = 0x104;            // 009F9E60
inline constexpr int kTickElementNode = 0x310;   // 007CFDA0 installs 00D05EDC
inline constexpr int kClassDescriptor = 0x538;   // docs/UNIT_INSTANCE_LAYOUT.md
inline constexpr int kSuppressSlot1D8 = 0x520;   // 00953D7D gates vtable[+1D8h]
inline constexpr int kControlModeGate = 0x72C;   // 007CEC30 calls its vtable[+38h]
inline constexpr int kGroundWaterMode = 0x900;   // 007CEC75, arms 4 and 5
inline constexpr int kSurfaceMode = 0x5F0;       // 007CEC99, arm 6
inline constexpr int kFlightController = 0xAB0;  // 007CFF6A / 007D7EA0
inline constexpr int kSquadron = 0x9D4;          // 007ED0E6 / 007F4B49, NOT a control block
inline constexpr int kSpawnIndex = 0x9D8;        // 007ED0EC
}  // namespace plane_unit_off

// The flight controller object, unit+AB0h, built by 007D7EA0(this, unit).
namespace plane_flight_controller_off {
inline constexpr int kControllerVtable = 0x00;         // 007D7EAD, 00D06848
inline constexpr int kModeByte4 = 0x04;      // 007D7EB3 zero; 007DC85D from unit+9F9h
inline constexpr int kModeByte5 = 0x05;      // 007D7EB6 zero; 007DC86B from unit+9F8h
inline constexpr int kControllerUnit = 0x08;           // 007D7EB9
inline constexpr int kClassDescriptor = 0x0C;// 007D7EC7, unit+538h
inline constexpr int kSubObject = 0x10;      // 007D7F4F, operator new(0D0h) at 007D7ECA
inline constexpr int kSubObjectByte = 0x14;  // 007D7F52 zero
inline constexpr int kBlock18 = 0x18;        // 007DB6B2 hands it to 007D7C00
inline constexpr int kBlock3C = 0x3C;        // 007DB6AE hands it to 007D7C00
inline constexpr int kSpeedFallback = 0x44;  // 007D99CA when unit+3Ch is null
inline constexpr int kField8C = 0x8C;        // 007D7F55 zero
inline constexpr int kField90 = 0x90;        // 007D7F3F = 99.0f from 00D059A0
inline constexpr int kField98 = 0x98;        // 007D7F5D = 1.0f
inline constexpr int kField9C = 0x9C;        // 007D7F65 = 1.0f
inline constexpr int kFieldAC = 0xAC;        // 007D7F6D zero
inline constexpr int kStateFC = 0xFC;        // 007DC841 zero; 007DB6D1 tests == 1
}  // namespace plane_flight_controller_off

// The plane class descriptor fields the flight law reads. Names and offsets from
// docs/PLANE_CLASS_FIELDS.md; +1ECh is the one the reader never writes.
namespace plane_class_flight_off {
inline constexpr int kAccel = 0x164;
inline constexpr int kStallRotAccel = 0x17C;
inline constexpr int kStallSpd = 0x184;
inline constexpr int kMaxSpd = 0x188;
inline constexpr int kTravelSpeed = 0x18C;
inline constexpr int kRollSpd = 0x1A8;             // 007DA7C2
inline constexpr int kPitchSpd = 0x1AC;            // 007DA8EB
inline constexpr int kYawSpd = 0x1B0;              // 007DA926
inline constexpr int kYawRollRatio = 0x1B4;
inline constexpr int kRollAccel = 0x1BC;
inline constexpr int kPitchAccel = 0x1C0;
inline constexpr int kYawAccel = 0x1C4;
inline constexpr int kTurnRollSpd = 0x1C8;
inline constexpr int kYawLimitAngle = 0x1CC;
inline constexpr int kPitchLimitAngle = 0x1D0;
inline constexpr int kDragPitchRatio = 0x1D4;
inline constexpr int kNegativePitchRatio = 0x1D8;  // 007DA918
inline constexpr int kClimbAngleLimit = 0x1EC;     // 009FB88D; producer unread, no Lua key
inline constexpr int kDropAngle = 0x1F0;           // 009FB979
inline constexpr int kTurnCircleRadius = 0x268;
inline constexpr int kRollMaxforceLimit = 0x274;
inline constexpr int kPitchMaxforceLimit = 0x278;
inline constexpr int kCruiseAltitudeGain = 0x518;  // 009FBADB; producer unread, no Lua key
}  // namespace plane_class_flight_off

// The bot approach controller sub-object, task+3F8h. docs/BOT_TASKS.md tabulates it
// task-relative; these are the offsets the three pilot helpers dereference.
namespace plane_pilot_approach_off {
inline constexpr int kStateVtable = 0x00;
inline constexpr int kStateUnit = 0x04;             // 009FB836, 009F9E46
inline constexpr int kClassDescriptor = 0x08;  // 009FBAD6
inline constexpr int kSquadron = 0x0C;         // 009FBA90, unit+9D4h
inline constexpr int kStateCommandBlock = 0x18;     // 009F9EAF, task+4h
}  // namespace plane_pilot_approach_off

// The command block fields the three helpers write (relative to task+4h).
namespace plane_pilot_command_off {
inline constexpr int kPitchDemand = 0x2BC;    // 009FB937 / 009FB957 / 009FBA39
inline constexpr int kHeadingDemand = 0x2C0;  // 009F9EB9
inline constexpr int kPitchMode = 0x2D0;      // 009FB93F, literal 2
inline constexpr int kStateHeadingMode = 0x2CC;    // 009F9EC1, literal 2

// The five plan slots 0099B450 seeds and 0099D300 overrides:
// {current: float, desired: float, active: BYTE} with three bytes of padding,
// stride 0Ch. The third field is a byte, not a dword - 0099B46E zeroes DL and
// stores it with MOV byte ptr, and 0099D362 sets it the same width.
inline constexpr int kThrottleCurrent = 0x274;  // 0099B466 from unit+9F0h
inline constexpr int kThrottleDesired = 0x278;  // 0099B45E, 0099D399
inline constexpr int kThrottleFlag = 0x27C;     // 0099B470 clear, 0099D3A1 set
// CORRECTED (packet cc7-recon-slot). These two slots were named roll and yaw the
// wrong way round, and each contradicted its own evidence comment: +280h is
// seeded from unit+9E4h, which this header's own kLiveYaw says is the YAW input,
// and +28Ch from unit+9ECh, which is roll. plane_ai_control.hpp's PlanSlotIndex
// had it right, so the two headers disagreed. Nothing referenced either name.
inline constexpr int kYawCurrent = 0x280;       // 0099B486 from unit+9E4h
inline constexpr int kYawDesired = 0x284;       // 0099B47E, 0099D35A
inline constexpr int kYawFlag = 0x288;          // 0099B48E clear, 0099D362 set
inline constexpr int kRollCurrent = 0x28C;      // 0099B4A4 from unit+9ECh
inline constexpr int kRollDesired = 0x290;      // 0099B49C, 0099D384
inline constexpr int kRollFlag = 0x294;         // 0099B4AC clear, 0099D38C set
inline constexpr int kPitchCurrent = 0x298;     // 0099B4C2 from unit+9E8h
inline constexpr int kPitchDesired = 0x29C;     // 0099B4BA, 0099D36F
inline constexpr int kPitchFlag = 0x2A0;        // 0099B4CA clear, 0099D377 set
// The fifth slot is the AIR BRAKE, not an unspecified aux: it is seeded from
// unit+9F4h, which this header's kLiveAirBrake names, and 007BB920's override
// writes unit+A0Ch - the same axis through the command buffer's index 4.
inline constexpr int kAirBrakeCurrent = 0x2A4;  // 0099B450's fifth slot, from unit+9F4h
inline constexpr int kAirBrakeDesired = 0x2A8;  // 0099B4D8, 0099D3A8
inline constexpr int kAirBrakeFlag = 0x2AC;     // 0099D3B0 set
}  // namespace plane_pilot_command_off

// ---------------------------------------------------------------------------
// The Dynamics/* tuning mirror. 007EAAE1 `REP MOVSD` copies 4Eh dwords from
// tuning+210h (ESI, 007E2D03 LEA ESI,[EBP+210h]) to 00F872F0 (EDI, 007EAADC),
// so 00F872F0..00F87427 is a verbatim copy of tuning+210h..tuning+347h.
// .text holds no absolute store into that window: this is the only producer.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kDynamicsMirrorBase = 0x00F872F0u;
inline constexpr int kDynamicsMirrorTuningBase = 0x210;
inline constexpr int kDynamicsMirrorDwords = 0x4E;
inline constexpr int kDynamicsMirrorBytes = kDynamicsMirrorDwords * 4;  // 0x138

// tuning_offset = mirror_address - 0x00F870E0, for an address inside the window.
int dynamics_mirror_to_tuning_offset(std::uint32_t mirror_address);
bool dynamics_mirror_contains(std::uint32_t mirror_address);

// The tuning rows the flight controller reads, by name. Values are the
// PlaneGlobals.lua defaults docs/GAME_TUNING_SINGLETON.md records.
namespace plane_dynamics_tuning_off {
inline constexpr int kCeiling = 0x210;               // 1500, 009FBA82 / 009FB809
inline constexpr int kStallRangeMin = 0x22C;         // 1.2,  00F8730C
inline constexpr int kStallRangeMax = 0x230;         // 1.6,  00F87310
inline constexpr int kStallOffPitch = 0x234;         // DEG(-15), 00F87314
inline constexpr int kDeadMeatRotationMin = 0x250;   // DEG(10), 00F87330
inline constexpr int kDeadMeatSpinRollSpd = 0x258;   // 5.0,  00F87338
inline constexpr int kDeadMeatRollMulTime = 0x25C;   // 6,    00F8733C
inline constexpr int kDeadMeatRollMul = 0x260;       // 2.0,  00F87340
inline constexpr int kDeadMeatSpinStallMul = 0x26C;  // 10,   00F8734C
inline constexpr int kClimbDist = 0x544;             // 130,  Pilot/General/ClimbDist
inline constexpr int kDropDist = 0x548;              // 200,  Pilot/General/DropDist
inline constexpr int kLandingWireRope = 0x518;       // 7.25, Pilot/Landing/WireRope
}  // namespace plane_dynamics_tuning_off

// ---------------------------------------------------------------------------
// Constants the laws use, read from the image.
// ---------------------------------------------------------------------------
inline constexpr float kPlaneCeilingMargin = 50.0f;       // 00CE3938, double
inline constexpr float kPlaneClimbAngleFloor = 0.69813174f;  // 00CE7D20, DEG(40)
inline constexpr float kPlaneDiveAngleFloor = 1.0471976f;    // 00D05AAC, DEG(60)
inline constexpr float kPlaneAngleLimitScale = 1.6f;      // 00CE3D48, double
inline constexpr float kPlaneQuarterTurn = 1.5707964f;    // 00CE3830, double
inline constexpr float kPlaneFullTurn = 6.2831855f;       // 00CE3828, double
inline constexpr float kPlaneControlQuantSteps = 127.0f;  // 00CFD408, double
inline constexpr float kPlaneControlQuantBias = 128.5f;   // 00D05998, double
// The sentinel 007D8216 parks dyn+C8h at when the owner+72Ch predicate is
// false. 00D7A260 is BF800000; the following dword is unrelated.
inline constexpr float kPlaneContactTimerExpired = -1.0f;  // 00D7A260
// The duration 007C705C hands to 007D83D0 on the BeginFlying path.
inline constexpr float kPlaneLaunchHoldSeconds = 0.8f;  // 00CE74F8, 3F4CCCCD

// ---------------------------------------------------------------------------
// The pilot control block as a value, and the latch 007B9770 performs.
// ---------------------------------------------------------------------------
struct PlaneControlInput {
    float roll{0.0f};
    float pitch{0.0f};
    float yaw{0.0f};
    float throttle{1.0f};
    float aux{0.0f};
    std::uint8_t byte_f8{0};
    std::uint8_t byte_f9{0};
    std::uint8_t byte_fa{0};
};

struct PlaneControlLatch {
    float roll{0.0f};
    float pitch{0.0f};
    float yaw{0.0f};
    float throttle{0.0f};
    float aux{0.0f};
    std::uint8_t byte_c8{0};
    std::uint8_t byte_c9{0};
    std::uint8_t byte_ca{0};
};

// 007B9770, the whole body. Note the byte permutation: +9F8h -> +BC8h,
// +9FAh -> +BC9h, +9F9h -> +BCAh.
PlaneControlLatch latch_control_input_007b9770(const PlaneControlInput& live);

// One axis of 007BB6E0: the signed-byte round trip every control value takes so a
// locally flown plane and a replicated one see the same input.
// q = ftol(v * 127 + 128.5); q >= 0FFh -> 1.0f; q <= 1 -> -1.0f; else (q - 128) / 127.
float quantize_control_axis_007bb6e0(float value);

// ---------------------------------------------------------------------------
// 009F9E40: the heading command from a world point, in the game's
// clockwise-from-north yaw convention.
// ---------------------------------------------------------------------------
float heading_command_009f9e40(float target_x, float target_z, float unit_x, float unit_z);

// ---------------------------------------------------------------------------
// 009FB800: the pitch command from an altitude error. Returns the signed pitch
// demand written to command+2BCh; mode command+2D0h is always the literal 2.
//
// alt        = min(desired_altitude, ceiling - 50)
// err        = alt - unit_world_y
// x          = err * ((reference + 1) * 0.5)
// x > 0  (climb): limit = max(class+1ECh * 1.6, DEG(40));
//                 t = clamp(x / ClimbDist, 0, reference);
//                 demand = min(class+1ECh * t, limit)
// x <= 0 (dive):  limit = max(DropAngle * 1.6, DEG(60));
//                 t = clamp(-x / DropDist, 0, reference);
//                 demand = -min(DropAngle * t, limit)
// ---------------------------------------------------------------------------
struct PlanePitchCommandInputs {
    float desired_altitude{0.0f};
    float reference{0.0f};       // the second argument; also the clamp on t
    float unit_world_y{0.0f};
    float ceiling{1500.0f};      // tuning+210h Dynamics/Ceiling
    float climb_dist{130.0f};    // tuning+544h Pilot/General/ClimbDist
    float drop_dist{200.0f};     // tuning+548h Pilot/General/DropDist
    float class_climb_angle{0.0f};  // class+1ECh, zero for every shipped row
    float class_drop_angle{0.0f};   // class+1F0h DropAngle
};

float pitch_command_009fb800(const PlanePitchCommandInputs& in);

// ---------------------------------------------------------------------------
// 009FBA50: the cruising-altitude command. It biases the altitude by a
// range-dependent term, clamps against the ceiling and the squadron limit, and
// tail-calls 009FB800. It leaves its fourth argument in ST0 at the RET, so the
// native ABI returns a float that no call site reads.
// ---------------------------------------------------------------------------
struct PlaneCruiseAltitudeInputs {
    float base_altitude{0.0f};   // arg0
    float range_low{0.0f};       // arg1
    float range_high{0.0f};      // arg2
    float scale{0.0f};           // arg3
    float ceiling{1500.0f};      // tuning+210h
    bool has_squadron{false};
    float squadron_limit{0.0f};  // squadron+394h
    float class_gain{0.0f};      // class+518h
};

struct PlaneCruiseAltitudeResult {
    float clamped_altitude{0.0f};    // 009FB800's first argument
    float unclamped_altitude{0.0f};  // 009FB800's second argument
    float ceiling_limit{0.0f};
    float returned_in_st0{0.0f};     // the leaked arg3
};

PlaneCruiseAltitudeResult cruise_altitude_command_009fba50(const PlaneCruiseAltitudeInputs& in);

// ---------------------------------------------------------------------------
// The fixed-step motion dispatch inside 007CE040, 007CEC30..007CECB4. Three
// mutually exclusive arms; the first that matches runs and the others do not.
// ---------------------------------------------------------------------------
enum class PlaneMotionArm {
    None,        // no arm matched; 007CECBF skips the 0085DC80 commit too
    FreeFlight,  // 007CEC6E 007CC2F0
    GroundRoll,  // 007CEC92 007CBFA0, unit+900h in {4, 5}
    Surface,     // 007CECAF 007CBA50, unit+5F0h == 6
};

struct PlaneMotionDispatchInputs {
    bool control_mode_gate{false};  // (*(unit+72Ch))->vtable[+38h]() at 007CEC3F
    int ground_water_mode{0};       // unit+900h
    int surface_mode{0};            // unit+5F0h; docs/PLANE_GROUND_OPS.md: the third arm branches on the same ground/water mode field
};

// Superseded by select_motion_arm_007cec30 in include/bsp/plane_ground_ops.hpp (two inputs, docs/PLANE_GROUND_OPS.md).
PlaneMotionArm select_motion_arm_007ce040(const PlaneMotionDispatchInputs& in);

// The host the step sequence drives: one method per native call site.
class PlaneFlightHost {
public:
    virtual ~PlaneFlightHost() = default;

    // 007CE08F 00953CC0, the level-4 unit tick. Returns unit+520h after the call,
    // which 00953D7D re-reads; the sequence needs the value, not the effect.
    virtual bool unit_game_object_tick_00953cc0(float step) = 0;

    // 00953CFD unit->vtable[+1F0h]; the plane's is 0095DC40, the turbo double tap.
    virtual void class_input_poll_0095dc40(float step) = 0;

    // 00953D98 unit->vtable[+1D8h]; the plane's is 007C6C30, the respawn countdown.
    virtual void out_of_action_countdown_007c6c30(float step) = 0;

    // 007CEC3F (*(unit+72Ch))->vtable[+38h]. True selects the free-flight arm.
    virtual bool free_flight_gate_00d06130_38() = 0;

    // 007CEC4E, only when unit+9E0h is clear: unit+908h += step.
    virtual void accumulate_airborne_time(float step) = 0;

    virtual void free_flight_007cc2f0(float step) = 0;
    virtual void ground_roll_007cbfa0(float step) = 0;
    virtual void surface_007cba50(float step) = 0;

    // 007CECBA 0085DC80 on unit+674h, reached from every arm that ran.
    virtual void commit_step_pose_0085dc80() = 0;

    // 007CE96F 007B9770, the control-input latch.
    virtual void latch_control_input_007b9770() = 0;

    // Observation points the sequence reads rather than calls.
    virtual bool airborne_time_frozen() = 0;  // unit+9E0h
    virtual int ground_water_mode() = 0;      // unit+900h
    virtual int surface_mode() = 0;           // unit+5F0h
};

// The ordered part of 007CE040 this packet read: the level-4 tick and its two
// virtual arms, then the motion dispatch, then the pose commit and the latch.
// Everything between 007CE094 and 007CEC30 (damage, effects, collision, sound)
// is not in the sequence: coverage is partial by construction.
PlaneMotionArm run_plane_fixed_step_007ce040(PlaneFlightHost& host, float step);

// ---------------------------------------------------------------------------
// 007DB680 BSP_PlaneFlight_CoreLaw, the free-flight arm's physics.
//
// The native routine is an *accumulator* pass, not an integrator. 007DB6B6
// 007D7C00 BSP_PlaneDynamics_BeginStep zeroes six 3-float accumulators in the
// dynamics block at controller+10h (dyn+04h, +10h, +1Ch, +28h, +34h, +40h; the
// stores are at 007D7C2A..007D7D0F, all from the read-only zero vector
// 00F87574..7C) and copies the controller's world velocity ctl+18h into
// dyn+4Ch and its body velocity ctl+3Ch into dyn+64h. 007DB680 then adds one
// term per force into the accumulator that force is expressed in, and
// 007DC6E6 007D8470 folds each *world* accumulator into its *body* partner
// through ctl+0B0h before the unread tail integrates.
//
// Free flight writes four of the six:
//   dyn+04h  body   lateral / vertical damping        (007DBD7D, 007DBD8A)
//   dyn+10h  world  drag along the velocity direction (007DBBC6 onward)
//   dyn+1Ch  body   +20h lift (007DB98D), +24h thrust (007DB80A),
//                   +0Ch of the pair-1 partner takes the ceiling's push
//   dyn+28h  world  +2Ch gravity (007DB9DB, 007DBA2F) and the ceiling (007DBE6B)
//
// docs/PLANE_FREE_FLIGHT_PHYSICS.md carries the per-term derivation, the
// listing ranges and the provenance of every constant. Every name here is a
// hypothesis, not a recovered symbol.
// ---------------------------------------------------------------------------

// The class-descriptor fields the free-flight law reads (unit+538h, also
// controller+0Ch). 007D1F70 BSP_PlaneClass_ReadLuaFields is the producer; the
// key strings it pushes before each FSTP name the two drag fields.
struct PlaneFreeFlightClass {
    float stall_spd{17.5f};  // desc+184h StallSpd, 007DB760 divides by it
    float x_drag{0.0f};      // desc+174h XDrag, 007DBD3A lateral damping
    float y_drag{0.0f};      // desc+170h YDrag, 007DBD50 vertical damping
};

// The Dynamics/* rows the law reads, every one inside the mirror window
// 00F872F0..00F87427 that kDynamicsMirrorBase declares. The defaults are the
// PlaneGlobals.lua values docs/GAME_TUNING_SINGLETON.md records; the image
// bytes are zero because 007EAAE1 fills the mirror at load.
struct PlaneFreeFlightTuning {
    float ceiling{1500.0f};         // +210h, mirror 00F872F0, read 007DBE34
    float ceiling_force{0.1f};      // +214h, mirror 00F872F4, read 007DBE42
    float drag_func_power{1.8f};    // +228h, mirror 00F87308, read 007D9325
    float drag_range_min{1.0f};     // +244h, mirror 00F87324, read 007D92D2
    float drag_range_max{2.0f};     // +248h, mirror 00F87328, read 007D92C2
    float level_flight{1.8f};       // +24Ch, mirror 00F8732C, read 007DB8C1
    float lost_drag_time{5.0f};     // +264h, mirror 00F87344, read 007DB9B1
    float extra_gravity_mul{1.5f};  // +268h, mirror 00F87348, read 007DB9A7
    float accel_cheat_mul{1.5f};    // +31Ch, mirror 00F873FC, read 007DB931
};

// The plane's state as the law reads it. Body axes are (x lateral, y up,
// z forward): 007DBD3A pairs ctl+3Ch with XDrag, 007DBD50 pairs ctl+40h with
// YDrag, and 007DB760 divides 007D99C0's ctl+44h-derived speed by StallSpd.
struct PlaneFreeFlightState {
    float world_velocity[3]{};  // ctl+18h..20h, handed to 007D7C00 at 007DB6B5
    float body_velocity[3]{};   // ctl+3Ch..44h, 007D9C10 rebuilds it each step
    // ctl+0B0h, row-major, body = M * world. 007D9C39 writes ctl+3Ch from
    // 0042D0D0(ctl+18h, ctl+0B0h), and 007DC6DA hands the same matrix to the
    // fold, so the two uses share one convention.
    float world_to_body[9]{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float forward_speed{0.0f};    // 007D99C0's return; ctl+44h plus a carrier term
    float world_altitude{0.0f};   // unit+100h, the ceiling's input
    float lost_drag_timer{0.0f};  // unit+0C3Ch, the DeadMeat ramp's input
    float airborne_time{3600.0f}; // unit+908h, the vertical-damping clamp's input
    float pitch{0.0f};            // unit+0C64h, the drag ramp's input
    float lift_ramp{99.0f};       // ctl+90h, the cap on the lift term
    float roll_drag_scale{1.0f};  // ctl+98h, the extra world-x drag above 1.0
    float lift_scale{1.0f};       // ctl+9Ch, the multiplier on q
    bool extra_gravity{false};    // ctl+94h, gates the second gravity term
    // Supplied by the host, not reconstructed here: see the doc's coverage table.
    float thrust_accel{0.0f};  // 007D9050 * unit+0CC8h * the 008E6430 multiplier
    float drag_accel{0.0f};    // 007D9140 * the 007DBB23 pitch ramp; negative forward
};

// The four accumulators free flight writes, in the order 007D7C00 clears them.
struct PlaneDynAccumulators {
    float body_damping[3]{};   // dyn+04h..0Ch
    float world_drag[3]{};     // dyn+10h..18h
    float body_lift[3]{};      // dyn+1Ch..24h
    float world_gravity[3]{};  // dyn+28h..30h
};

// What 007D8470's first two folds leave in the body accumulators, and their sum.
struct PlaneBodyAcceleration {
    float pair_04[3]{};  // dyn+04h after 007D84B2 folds dyn+10h into it
    float pair_1c[3]{};  // dyn+1Ch after 007D8487 folds dyn+28h into it
    float total[3]{};    // pair_04 + pair_1c, the step's body-frame acceleration
};

// 007D92B0 BSP_PlaneFlight_AeroResponseCurve (RET 4, one float argument).
// u = InterpolateClamped(DragRangeMin, 0, DragRangeMax, 1, ratio);
// u == 0 -> 0, else pow(|u|, DragFuncPower) through FYL2X / F2XM1 / FSCALE.
float aero_response_curve_007d92b0(float speed_ratio, const PlaneFreeFlightTuning& tuning);

// 007DB8A7..007DB8B9. |vz| is formed as -0.0f - vz (00D7A208), so a zero vz
// with either sign fails the >= 0.1f test (00D7A3A0, a double) and the angle is
// exactly zero; otherwise -vy / vz, positive when the plane is sinking.
float angle_of_attack_007db8b1(float body_vy, float body_vz);

// 007DB875..007DB98D, the whole lift term in m/s^2 along body up, before it is
// added into dyn+20h.
float lift_accel_007db875(const PlaneFreeFlightState& state, const PlaneFreeFlightClass& cls,
                          const PlaneFreeFlightTuning& tuning);

// 007DB990..007DBA2F, the signed world-up gravity term (negative), including the
// second application 007DB9E7 that ctl+94h gates.
float gravity_accel_007db990(const PlaneFreeFlightState& state,
                             const PlaneFreeFlightTuning& tuning);

// 007DB80D..007DB874. ramp_reset is 007DB819's AL, the result of 007BBC50 on
// the unit; it pins the cap at 3.0f. Otherwise the cap creeps: +step below 3,
// +3*step below 6 (00D7A2B0, a double), and stops at 6 (00CE6630).
float advance_lift_ramp_007db80d(float lift_ramp, float step, bool ramp_reset);

// The free-flight arm as a pure rule: one native step of 007DB680 with ctl+FCh
// clear, expressed as the four accumulators it leaves for the fold. The step
// argument is used only where the native uses it, by 007DB80D's ramp.
PlaneDynAccumulators accumulate_free_flight_007db680(const PlaneFreeFlightState& state,
                                                     const PlaneFreeFlightClass& cls,
                                                     const PlaneFreeFlightTuning& tuning,
                                                     float step, bool ramp_reset = false);

// 007D8470's first two folds and the 0.001f deadband at 007D8502..007D85A5.
PlaneBodyAcceleration fold_world_into_body_007d8470(const PlaneDynAccumulators& acc,
                                                    const float world_to_body[9]);

// The quantity the acceptance test pins: the world-up component of one free
// flight step's acceleration, with the body result rotated back through the
// transpose of ctl+0B0h. At level flight the two frames agree and this is just
// lift + gravity.
float free_flight_world_up_acceleration(const PlaneFreeFlightState& state,
                                        const PlaneFreeFlightClass& cls,
                                        const PlaneFreeFlightTuning& tuning, float step,
                                        bool ramp_reset = false);

// The dyn+B4h/+C0h pair (docs/PLANE_DYN_TIMED_HOLD.md). 007D83D0 and 007DB2A4
// write the two together, so they are one group. "Seconds" is proved, not
// guessed: 007D902F decrements +C0h by the integrator's own step argument, the
// same scalar 007D8F39 divides a position delta by to get a velocity. The name
// "direction hold" is a hypothesis from the two call sites, not a symbol.
struct PlaneTimedDirectionHold {
    float direction[3]{};  // dyn+B4h..BCh; normalised at the 007C705C call site
    float seconds{0.0f};   // dyn+C0h; the yaw law reads it as 0.6f * seconds
};

// 007D83D0, whole body. Sets both halves at once; no clamping, no validation.
// 007C705C passes a normalised direction and the literal 0.8f at 00CE74F8;
// 007C08F7 passes a computed duration.
PlaneTimedDirectionHold arm_timed_direction_hold_007d83d0(const float direction[3],
                                                          float seconds);

// 007D8FFE..007D902F, the integrator's tail. Strictly positive values decay by
// one step and stop at zero; anything at or below zero is left untouched (the
// native takes the 007D900F JBE exit without storing).
float decay_direction_hold_007d902f(float seconds, float step);

// 007DC692..007DC6C5, the core law's commit tail. Both reaching paths store
// zero: no owner takes the 007DC6A8 JZ with XMM0 already cleared at 007DC692,
// and a false predicate re-clears XMM0 at 007DC6BF because the virtual call
// clobbers it. A true predicate skips the store, preserving the field.
// `hold` is the unresolved boolean: owner+72Ch's vtable slot +38h.
float commit_direction_hold_007dc6c5(float seconds, bool owner_present, bool hold);

// 007D81B0..007D81C7. The clear runs only when GGame+1FE4h equals 2; the
// caller supplies that comparison rather than the state value, because what
// state 2 means is not established here.
float gate_direction_hold_007d81c7(float seconds, bool game_state_is_two);

// 007D81CF..007D8216, the sibling timer at dyn+C8h, included because it is the
// same shape under the same predicate and settles the polarity. Positive values
// decay by one step; a false predicate then parks the timer at the -1.0f
// sentinel from 00D7A260. Non-positive values are left untouched.
float tick_contact_timer_007d81b0(float seconds, float step, bool hold);

}  // namespace bsp
