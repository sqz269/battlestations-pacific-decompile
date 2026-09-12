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
// the bot's five planned axes into unit+9E4h (contract: unread; the remaining candidate sites
// are 007CA509 and 007D1333, since 007CB185 and 007CB3CE belong to the clamp and reset in
// 007CAF10 and 007BB75D / 007C2B11 / 007D1676 are the network paths);
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
inline constexpr int kLiveRoll = 0x9E4;      // 007CFE92 zero; 0099B476 reads it
inline constexpr int kLivePitch = 0x9E8;     // 007CFE98 zero; 0099B4B2 reads it
inline constexpr int kLiveYaw = 0x9EC;       // 007CFE9E zero; 0099B494 reads it
inline constexpr int kLiveThrottle = 0x9F0;  // 007CFEB0 = 1.0f; 0099B456 reads it
inline constexpr int kLiveAux = 0x9F4;       // 007CFEA4 zero; 0099B4D0 reads it
inline constexpr int kLiveByteF8 = 0x9F8;    // 007CFEAA zero; 007DC860 copies it to ctl+5h
inline constexpr int kLiveByteF9 = 0x9F9;    // 007DC84F copies it to ctl+4h
inline constexpr int kLiveByteFA = 0x9FA;    // 007B977D latches it

// The previous-step snapshot 007B9770 writes, read by the rate law 007DA710.
inline constexpr int kLatchedRoll = 0xBB0;      // 007B9783
inline constexpr int kLatchedPitch = 0xBB4;     // 007B979C
inline constexpr int kLatchedYaw = 0xBB8;       // 007B97A8
inline constexpr int kLatchedThrottle = 0xBBC;  // 007B97BA
inline constexpr int kLatchedAux = 0xBC0;       // 007B97CC
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

// The five plan slots 0099B450 seeds and 0099D300 overrides: {current, desired, flag}.
inline constexpr int kThrottleCurrent = 0x274;  // 0099B466 from unit+9F0h
inline constexpr int kThrottleDesired = 0x278;  // 0099B45E, 0099D399
inline constexpr int kThrottleFlag = 0x27C;     // 0099B470 clear, 0099D3A1 set
inline constexpr int kRollCurrent = 0x280;      // 0099B486 from unit+9E4h
inline constexpr int kRollDesired = 0x284;      // 0099B47E, 0099D35A
inline constexpr int kRollFlag = 0x288;         // 0099B48E clear, 0099D362 set
inline constexpr int kYawCurrent = 0x28C;       // 0099B4A4 from unit+9ECh
inline constexpr int kYawDesired = 0x290;       // 0099B49C, 0099D384
inline constexpr int kYawFlag = 0x294;          // 0099B4AC clear, 0099D38C set
inline constexpr int kPitchCurrent = 0x298;     // 0099B4C2 from unit+9E8h
inline constexpr int kPitchDesired = 0x29C;     // 0099B4BA, 0099D36F
inline constexpr int kPitchFlag = 0x2A0;        // 0099B4CA clear, 0099D377 set
inline constexpr int kAuxCurrent = 0x2A4;       // 0099B450's fifth slot, from unit+9F4h
inline constexpr int kAuxDesired = 0x2A8;       // 0099B4D8, 0099D3A8
inline constexpr int kAuxFlag = 0x2AC;          // 0099D3B0 set
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
    int surface_mode{0};            // unit+5F0h
};

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

}  // namespace bsp
