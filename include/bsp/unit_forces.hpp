#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/unit_motion.hpp" // kUnitOffThrottle, kUnitOffSteering, kUnitOffPropellerLoad
#include "bsp/world_ocean.hpp" // OceanVec3

// How a commanded throttle and rudder become motion, docs/UNIT_FORCE_COMMANDS.md.
//
// The premise this packet inherited was that commanded force reaches a unit as an entry
// in a list at unit+10D4h drained by 0074F2E0. That is wrong. 0074F2E0 and 0074F930 are
// the two halves of a leak (flooding) model: 0074F930 accumulates water per leak point
// and 0074F2E0 turns those water weights into a heeling torque. The unit's commanded
// motion never passes through it.
//
// The real path for a surface ship, established from 00825F20, is:
//
//   * the throttle at unit+980h and the rudder at unit+984h are plain floats that the
//     motion tick reads;
//   * the throttle becomes a *target forward speed*, which 0092D300 approaches with an
//     acceleration limit by rewriting the rigid body's linear velocity - not a force;
//   * the rudder becomes a *target yaw rate*, which 0092E8C0 applies by rewriting the
//     body's angular velocity, again not a force;
//   * only one genuine command force exists: the ship force-model override 00937440
//     adds a rudder torque on top of the hydrodynamic callback 009329C0.
//
// So thrust is kinematic and steering is half kinematic, half torque. Everything below
// is the native expression in the native operation order, with the constants read out of
// the listing. Nothing here is a drop-in binary replacement, and the descriptive names
// are hypotheses rather than recovered symbols.

namespace bsp {

// ---------------------------------------------------------------------------
// Unit fields this packet establishes. The throttle (980h), the steering (984h)
// and the reference-speed base (9C0h) already live in include/bsp/unit_motion.hpp
// as kUnitOffThrottle / kUnitOffSteering / kUnitOffReferenceSpeedBase and are not
// redeclared here.
// ---------------------------------------------------------------------------

// The helmsman override pair, named by the "_ship" diagnostic dump at 00818340
// (00818724 "helmsmanControl"/"thrust", 00818757 "toTurn"). The ship AI reads both at
// 0099D5A4 / 0099D694 and, when either is non-zero, clamps it and uses it in place of
// its own plan. Nothing this packet read writes them.
inline constexpr std::size_t kUnitOffHelmsmanThrust = 0x9A0;  // 00818724
inline constexpr std::size_t kUnitOffHelmsmanToTurn = 0x9A4;  // 00818757

// Per-tick performance modifiers, reset to 1.0f / 0 at 00823704..0082372E.
inline constexpr std::size_t kUnitOffThrustMod = 0x9D8;       // 0082675D, 00823714
inline constexpr std::size_t kUnitOffTurnEfficiency = 0x9DC;  // 008118CE, 0082371C
inline constexpr std::size_t kUnitOffSteeringJam = 0x9E4;     // 00826B2E, 0082372E
inline constexpr std::size_t kUnitOffEngineJam = 0x9E5;       // 00826754, 00823727

// Four floats zeroed together by the ship constructor at 0081F2EC..0081F304. Only two
// have a recovered consumer: +1030h scales the low-rate turn assist in 00825DE0 and
// +1038h scales the forward acceleration in 00825EC0. +102Ch and +1034h are read only
// by the AI (009D4E30, 009DE5B0, 009F3F80). No producer was found for any of them.
// +1030h itself is kUnitOffPropellerLoad in include/bsp/unit_motion.hpp.
inline constexpr std::size_t kUnitOffTurnAssistLoad = 0x102C;  // 009D4FB0
inline constexpr std::size_t kUnitOffTurnAssistLoad2 = 0x1034; // 009D4FE0
inline constexpr std::size_t kUnitOffAccelerationBoost = 0x1038; // 00825ED8

// The leak model, not an external force list. 0074F930 ticks it and writes the total
// accumulated water at leak+28h, which is the unit+10FCh that 009329C0 adds to the hull
// mass at 00932A2C and subtracts from the buoyancy at 00933A3A.
inline constexpr std::size_t kUnitOffLeakModel = 0x10D4;   // 00933A52
inline constexpr std::size_t kUnitOffLeakWaterMass = 0x10FC; // 0074F930 + 28h

// Class-block (descriptor) fields the command path scales by.
inline constexpr std::size_t kShipClassOffHullMass = 0x0B0;     // 00937489
inline constexpr std::size_t kShipClassOffMaxSpeed = 0x500;     // 00822C4F
inline constexpr std::size_t kShipClassOffBrakeAccel = 0x504;   // 0092D3F1
inline constexpr std::size_t kShipClassOffDriveAccel = 0x508;   // 00825EC6

// Settings-singleton (00424C40) fields the command path scales by.
inline constexpr std::size_t kSettingsOffTurnAssistLimit = 0x220; // 00825DFB
inline constexpr std::size_t kSettingsOffTurnAssistGain = 0x224;  // 00825E14
inline constexpr std::size_t kSettingsOffAccelBoostTop = 0x228;   // 00825EE9
inline constexpr std::size_t kSettingsOffSubmergedThrust = 0x4B4; // 008269E6
inline constexpr std::size_t kSettingsOffRudderTorque = 0x588;    // 009374F4

// ---------------------------------------------------------------------------
// Constants, all read with `bsp.py ghidra bytes`.
// ---------------------------------------------------------------------------

// 00D7A208 is -0.0f and 00D7A218 is 0.0f, so `if (x <= 0.0f) x = -0.0f - x` is the
// listing's absolute value. It is spelled out rather than folded to fabsf because the
// native form keeps the sign of a negative zero.
inline constexpr float kUnitForceNegativeZero = -0.0f; // 00D7A208
inline constexpr float kUnitForceZero = 0.0f;          // 00D7A218

inline constexpr float kUnitForceUnitScale = 1.0f;   // 00D7A24C
inline constexpr float kUnitForceHalf = 0.5f;        // 00D7A280, a double in the listing
inline constexpr float kUnitForceCreepSpeed = 3.0f;  // 00CE3854
inline constexpr float kUnitForceDeckDepth = -3.0f;  // 00CE3D50
inline constexpr float kUnitForceTorqueDivisor = 10000.0f; // 00CE4BD8, a double
inline constexpr float kUnitForceLeakRateFloor = 0.001f;   // 00D7A23C
inline constexpr float kUnitForceLeakFloodedMark = 0.01f;  // 00D7A238

// 008269B1 tests unit+C4h, the class id kUnitOffClassId, against this before applying
// the submerged-throttle scale.
inline constexpr int kUnitForceSubmarineClassId = 8; // 008269B1

// ---------------------------------------------------------------------------
// The leak model at unit+10D4h.
// ---------------------------------------------------------------------------

// Field offsets on the leak model itself, from 0074F930 and 0074F2E0. The three arrays
// are parallel and `count` long; the model does not own a std::vector-shaped pair.
struct UnitLeakModelLayout {
    static constexpr std::size_t kRateCap = 0x08;    // 0074F97D, per-leak rate ceiling
    static constexpr std::size_t kCapacity = 0x0C;   // 0074F9F9, hull water capacity
    static constexpr std::size_t kOwnerUnit = 0x10;  // 0074F2F8
    static constexpr std::size_t kCount = 0x14;      // 0074F2F0
    static constexpr std::size_t kRates = 0x18;      // 0074F959, float[count]
    static constexpr std::size_t kWater = 0x1C;      // 0074F98D, float[count]
    static constexpr std::size_t kPoints = 0x20;     // 0074F334, float3[count]
    static constexpr std::size_t kFloodedFlag = 0x24; // 0074FA5F
    static constexpr std::size_t kTotalWater = 0x28; // 0074F9C4, = unit+10FCh
    static constexpr std::size_t kTotalRate = 0x2C;  // 0074F984
};

// One leak: an inflow rate, the water already taken, and the hull-space point the
// weight hangs from. The point stride is 0Ch (0074F32B adds 0Ch per iteration).
struct UnitLeakEntry {
    float rate{0.0f};
    float water{0.0f};
    OceanVec3 point{};
};

// What one tick of 0074F930 produced.
struct UnitLeakTickResult {
    float total_water{0.0f}; // -> leak+28h, the unit's extra mass
    float total_rate{0.0f};  // -> leak+2Ch
    bool flooded{false};     // -> leak+24h, total_water > 0.01f
};

// 0074F930, void __thiscall(leak_model, float dt), RET 4.
//
// Pass 1 (0074F94A..0074F9AF): a rate under 0.001f is snapped to zero in place; the raw
// rate accumulates into total_rate; the rate actually applied is min(rate, rate_cap) and
// adds rate*dt to that leak's water.
// Pass 2 (0074F9B2..0074F9C8): total_water = sum of the per-leak water.
// The cap (0074F9CB..0074FA4F): when total_water > 0 and the unit is not gated by its
// +5Dh byte, ratio = (1 - health) * settings[404h] * capacity / total_water; if the
// ratio is under 1 every water value is scaled by it, every rate is zeroed and
// total_water is scaled too.
// Finally 0074FA52: flooded = total_water > 0.01f.
//
// `entries` is updated in place, exactly as the native pass order leaves it.
UnitLeakTickResult unit_leak_tick_0074f930(UnitLeakEntry* entries, std::uint32_t count,
                                           float rate_cap, float capacity,
                                           float settings_flood_scale, float unit_health,
                                           bool health_gate_5d, float dt) noexcept;

// 0074F2E0, void __thiscall(leak_model, float* out), RET 4. Read-only for this packet:
// the address stays outside the lease.
//
// The out vector is zeroed and only x and z are ever written; y stays 0. For every leak
// the water weight is turned into a torque through the unit's pose rows at +CCh (row 0),
// +DCh (row 1) and +ECh (row 2), each read at column 0 and column 2:
//
//   out.x += w * (p.x*row0.z + p.y*row1.z + p.z*row2.z) * 10.0
//   out.z -= w * (p.x*row0.x + p.y*row1.x + p.z*row2.x) * 10.0
//
// The 10.0 is the same 00CE3DC0 gain the hydrodynamic callback uses. The caller adds the
// result into the controller's torque staging at +74h..+7Ch (00933A52).
OceanVec3 unit_leak_torque_0074f2e0(const UnitLeakEntry* entries, std::uint32_t count,
                                    const float* pose_rows_ccb) noexcept;

// ---------------------------------------------------------------------------
// Throttle -> target speed. 00825F20's tail, 008269A9..00826B29.
// ---------------------------------------------------------------------------

struct UnitSpeedCommandInputs {
    float throttle{0.0f};      // unit+980h
    float max_speed{0.0f};     // unit+9C0h
    float thrust_mod{0.0f};    // unit+9D8h
    bool engine_jam{false};    // unit+9E5h
    bool dead_5d{false};       // unit+5Dh, forces the throttle to zero at 008269FD
    int class_id{0};           // unit+C4h
    float pose_base_y{0.0f};   // unit+100h, the pose translation's y
    float wave_height{0.0f};   // 0078CF20's result at 00826985
    float deck_reference{0.0f}; // the value it is compared against at 00826998
    float submerged_scale{1.0f}; // settings+4B4h
    float gameplay_scale{1.0f};  // 008E6430(4, unit), or 1.0f when the gate is closed
};

struct UnitSpeedCommand {
    bool apply{true};       // BL at 008269A5 / 00826B0A: false suppresses the whole command
    float throttle{0.0f};   // the gated throttle, slot [ESP+10h]
    float engine_gate{0.0f}; // 1.0f or 0.0f, slot [ESP+18h]
    float target_speed{0.0f}; // what 0092D300 receives
};

// The native order is ((max_speed * gameplay_scale) * throttle) * engine_gate, taken from
// the FMUL chain at 00826A46..00826A5F. The engine gate is
// `engine_jam == 0 && thrust_mod != 0.0f` (00826754..0082676E, with XMM1 zeroed at
// 008266B4); it is a 0/1 float, not the modifier itself.
UnitSpeedCommand unit_speed_command_00826985(const UnitSpeedCommandInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Target speed -> body linear velocity.
// ---------------------------------------------------------------------------

// 00825EC0, float10 __fastcall(unit). The drive acceleration, boosted by unit+1038h:
// accel = class[508h] * (1 + boost * (settings[228h] - 1)) when boost > 0.0f.
float unit_forward_acceleration_00825ec0(float class_drive_accel, float acceleration_boost,
                                         float settings_boost_top) noexcept;

struct UnitAxialSpeedInputs {
    OceanVec3 velocity{};   // 00C31F40
    OceanVec3 axis{};       // (body+18h, body+1Ch, body+20h) from 00C32000
    float commanded_speed{0.0f};
    float drive_accel{0.0f}; // unit_forward_acceleration_00825ec0
    float brake_accel{0.0f}; // class[504h]
    float dt{0.0f};
};

struct UnitAxialSpeedStep {
    OceanVec3 velocity{};    // what reaches 00C37E50
    float current_speed{0.0f};
    float target_speed{0.0f}; // after the heel scale and the rate limit
    float accel_used{0.0f};
};

// 0092D300, void __thiscall(controller, float commanded_speed, float dt), RET 8.
//
// The forward axis is flattened before use: only x and z survive, and both are divided by
// the length the listing computes at 0092D3A6, so the y component of the direction is a
// literal zero (0.0f / len at 0092D3B7). The commanded speed is scaled by (1 - |axis.y|)
// - the ship loses speed as it heels. The acceleration is the drive value unless the
// listing's three special cases fire, all reading class[504h]:
//   * moving forward but under the target  -> class[504h]
//   * over the target and nearly stopped   -> 2 * class[504h]
//   * over the target and moving astern    -> class[504h]
// The step is accel*dt and only bites when it is no larger than the remaining gap. The
// new velocity replaces the axial component and leaves everything else, y included.
UnitAxialSpeedStep unit_approach_axial_speed_0092d300(const UnitAxialSpeedInputs& in) noexcept;

// 0092D770, void __thiscall(controller, float speed), RET 4. Replaces the component of
// the velocity along the body axis outright, with no limit and no heel scale. The axis
// is used unnormalised, exactly as 00C32000 returns it.
OceanVec3 unit_set_axial_speed_0092d770(const OceanVec3& velocity, const OceanVec3& axis,
                                        float speed) noexcept;

// ---------------------------------------------------------------------------
// Rudder -> yaw rate and rudder -> torque.
// ---------------------------------------------------------------------------

// 00825DE0, float10 __thiscall(unit, float yaw_rate), RET 4. A low-rate turn assist paid
// for by the propeller load at unit+1030h: while the load is positive and |rate| is under
// settings[220h], the rate is pushed away from zero by
// min(|rate|, settings[224h] * (settings[220h] - |rate|)) * load, keeping the sign of the
// input (00825E36 tests rate < 0.0f). Outside that window the rate is returned unchanged.
float unit_propeller_turn_assist_00825de0(float yaw_rate, float propeller_load,
                                          float settings_assist_limit,
                                          float settings_assist_gain) noexcept;

struct UnitSteeringTorqueInputs {
    OceanVec3 velocity{};      // 00C31F40
    OceanVec3 axis{};          // (body+18h, body+1Ch, body+20h)
    float reference_speed{1.0f}; // BSP_UnitInstance_GetReferenceSpeed
    float hull_mass{0.0f};     // class[B0h]
    float steering{0.0f};      // unit+984h
    float pose_row0_y{0.0f};   // unit+D0h, the heel term
    OceanVec3 pose_row2{};     // unit+ECh..+F4h, the axis the torque is spent on
    float settings_rudder_torque{0.0f}; // settings[588h]
};

// 00937440, void __thiscall(controller, float dt), RET 4. The ship force-model override:
// it adds one torque and then tail-calls the hydrodynamic callback 009329C0. The whole
// body is skipped when the unit's +5Dh byte is set (00937449).
//
//   speed_ratio = |dot(velocity, axis)| / reference_speed
//   heel        = |pose_row0_y|
//   gain        = (1 - heel) * speed_ratio * steering * settings[588h]
//   torque      = (hull_mass * hull_mass) * gain * pose_row2 / 10000.0f
//
// Both absolute values are the `if (x <= 0.0f) x = -0.0f - x` form. The divisor is the
// double 10000.0 at 00CE4BD8, narrowed to float in the z slot before the stores.
//
// Which basis vector pose row 2 is has not been settled, so calling this a yaw torque is
// provisional; the listing only says it is the third row of the pose block at +CCh.
OceanVec3 unit_steering_torque_00937440(const UnitSteeringTorqueInputs& in) noexcept;

// ---------------------------------------------------------------------------
// The tick that issues both commands.
// ---------------------------------------------------------------------------

// One method per native call site reached from the command tail of 00825F20 and from
// 0092E8C0, in call order. Nothing here has a default: the parts of the native routine
// that were not recovered are absent rather than guessed.
struct UnitForceCommandHost {
    virtual ~UnitForceCommandHost() = default;

    // 00825F20's tail, in order.
    virtual UnitSpeedCommandInputs read_speed_inputs() = 0;
    virtual float unit_dead_gate() = 0; // unit+5Dh as read at 008269F4

    // 0092D300 and 0092E8C0 both take the controller as `this`.
    virtual UnitAxialSpeedInputs read_axial_inputs(float commanded_speed, float dt) = 0;
    virtual void body_set_linear_velocity(const OceanVec3& v) = 0;

    // unit+9E4h at 00826B2E: a jammed rudder skips the steering call entirely.
    virtual bool steering_jam() = 0;
    virtual float steering_command() = 0; // unit+984h at 00826B4B

    // 0092E8C0's own call sites, in order: the rate limiter on controller+80h
    // (0042AC60 at 0092E8EF, with dt*0.5f), the rudder-to-yaw-rate map (00811890 at
    // 0092E950, negated at 0092E955) and the propeller assist (00825DE0 at 0092E966).
    virtual float step_smoothed_rudder(float target, float max_step) = 0;
    virtual float yaw_rate_from_rudder_00811890(float smoothed_rudder) = 0;
    virtual float propeller_turn_assist(float yaw_rate) = 0;
    virtual void body_set_angular_velocity_from_yaw_rate(float yaw_rate) = 0;
};

// What one command tick did, so a caller can observe it without a physics body.
struct UnitForceCommandResult {
    bool speed_applied{false};
    bool steering_applied{false};
    UnitSpeedCommand speed{};
    UnitAxialSpeedStep axial{};
    float smoothed_rudder{0.0f};
    float yaw_rate{0.0f};
};

// The command tail of 00825F20, 008269A9..00826B59, void __thiscall(unit, float dt).
//
// It is not the whole routine: 00825F20 also runs the wake, the anchor sink and the pose
// refresh. Only the two command calls and their gates are reconstructed here.
//
// The steering half stops at the angular-velocity write. 0092E8C0's own body - the
// orientation-relative decomposition at 0092E96F..0092EB8F and its roll damper - is
// analysed but not reconstructed; the host supplies its result.
UnitForceCommandResult unit_apply_motion_commands_00825f20(UnitForceCommandHost& host,
                                                           float dt) noexcept;

} // namespace bsp
