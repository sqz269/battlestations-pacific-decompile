#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/unit_forces.hpp"   // UnitAxialSpeedInputs/Step, unit_steering_torque_00937440
#include "bsp/unit_instance.hpp" // kUnitOffPoseBlock, kUnitOffClassBlock, kUnitOffController
#include "bsp/world_ocean.hpp"   // OceanVec3

// The ship motion tick, docs/SHIP_MOTION.md.
//
// 00825F20 is the virtual a ship class installs for "move this hull one step". It is not
// reached from the base per-frame update 008255B0 (which runs effects, timers and the
// controller); it is a separate slot, present in six vtables and wrapped by two subclass
// overrides (00855420 and 00749B20) that call it first and then add their own work.
//
// The order of the motion path inside it, with the addresses of each call site:
//
//   00826121  00813020(unit+838h, dt)      the order ring tick. This is what writes the
//                                          commanded pair at unit+980h / unit+984h.
//   00826126  dt *= unit+340h              only when that field is > 0.
//   00826866  the keel sample point from the pose and two class dimensions
//   00826985  0078CF20(ocean, p.x, p.z)    the local wave height
//   00826994  the throttle gate            p.y > height*0.5 suppresses the whole command
//   00826A3A  target = ((maxSpeed * gameplayScale) * throttle) * engineGate
//   00826A6D  controller->vtable[0](dt)    the force model; 00937440 for a surface ship
//   00826A82  the boost block              may replace the target outright
//   00826B29  0092D300(controller, target, dt)      unless the gate closed
//   00826B54  0092E8C0(controller, unit+984h, dt)   unless unit+9E4h is set
//   00826B6A  0092BE80(controller, dt)     the controller / physics step
//   00826B84  unit->vtable[1ECh](dt)
//
// 0092D300 and 00937440 are already reconstructed in bsp/unit_forces.hpp and are reused
// rather than repeated. What this header adds is 00825F20's own sequence and gates, the
// whole of 0092E8C0 including the parts that packet left as "analysed, not
// reconstructed", and a state struct the three can be stepped over without a physics
// body.
//
// The x87 blocks are modelled with double intermediates and an explicit narrowing at
// every `FSTP m32`, which is what the hardware does under the MSVC default control word
// (53-bit precision). That assumption is recorded in docs/SHIP_MOTION.md.
//
// Descriptive names are hypotheses, not recovered symbols. Nothing here is a drop-in
// binary replacement, and no rigid-body integrator is reconstructed: position and
// attitude are advanced by the external physics library (the 00C3xxxx imports).

namespace bsp {

// ---------------------------------------------------------------------------
// Class-descriptor fields the motion path reads
// ---------------------------------------------------------------------------

// The two hull dimensions the keel sample point uses. Both are in the base vehicle-class
// block (below the ship reader's +138h HoD records), so neither has a Lua key in
// bsp/ship_class_fields.hpp.
inline constexpr std::size_t kShipMotionClassHullLength = 0x0A0; // 00826866
inline constexpr std::size_t kShipMotionClassHullHeight = 0x0A8; // 00826910

// The boost block at 00826A82. These three sit above the ship reader's last field
// (kCapturePower, +804h), so they belong to a derived descriptor and have no recovered
// Lua key either.
inline constexpr std::size_t kShipMotionClassBoostCapacity = 0x808; // 00826AD0, 00826AF0
inline constexpr std::size_t kShipMotionClassBoostSpeedScale = 0x80C; // 00826AB0
inline constexpr std::size_t kShipMotionClassBoostRefillTime = 0x810; // 00826AD6

// ---------------------------------------------------------------------------
// Unit fields the motion path reads that no other header declares
// ---------------------------------------------------------------------------

inline constexpr std::size_t kUnitOffMotionTimeScale = 0x340;  // 00826126, EDI+30h
inline constexpr std::size_t kUnitOffOrderKindCurrent = 0x988; // 00826A82, EDI+678h
inline constexpr std::size_t kUnitOffOrderKindMirror = 0x118C; // 00826A90
inline constexpr std::size_t kUnitOffBoostReserve = 0x1188;    // 00826A98
inline constexpr std::size_t kUnitControllerOffSmoothedRudder = 0x80; // 0092E8D8

// Pose rows inside the 4x4 at kUnitOffPoseBlock (+CCh), stride 10h. +F0h is already
// kUnitOffPoseLateral and +100h kUnitOffPoseBase in bsp/unit_instance.hpp.
inline constexpr std::size_t kUnitPoseRowStride = 0x10;
inline constexpr std::size_t kUnitPoseRowRight = 0x00;   // +CCh, the row 00937440 heels on
inline constexpr std::size_t kUnitPoseRowUp = 0x10;      // +DCh, used by the keel point
inline constexpr std::size_t kUnitPoseRowForward = 0x20; // +ECh, the torque axis
inline constexpr std::size_t kUnitPoseRowTranslation = 0x30; // +FCh

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

// 00CEC9E0, a double. The keel point is placed at minus half of each dimension.
inline constexpr double kShipMotionHalfNegative = -0.5;
// 00D7A280, a double. Halves the sampled wave height at 0082698A and scales the rudder
// slew at 0092E8C3.
inline constexpr double kShipMotionHalf = 0.5;
// 00D19628 read as a *double*: 0.2617991 radians, fifteen degrees. The float at the same
// address is 2.0f, which is what a four-byte read of it yields; the roll damper's FMUL
// is an eight-byte read and gets the angle. docs/UNIT_FORCE_COMMANDS.md recorded the
// float reading; see the Corrections section of docs/SHIP_MOTION.md.
inline constexpr double kShipMotionRighting = 0.2617991025660515;
// 00D05AA8, a float: the same angle to float precision, the damper's threshold.
inline constexpr float kShipMotionRightingThreshold = 0.2617843747f;
// 00CE3DC0, a double. The damper's gain, per second.
inline constexpr double kShipMotionRightingGain = 10.0;

// ---------------------------------------------------------------------------
// 0092E8C0, the steering half, complete
// ---------------------------------------------------------------------------

// The body basis 00C32000 returns, read as three rows of three floats at +0h, +0Ch and
// +18h. 0092D300 and 00937440 both take row 2 as the forward axis, and 0092E8C0 puts the
// commanded yaw rate on row 1, so row 0 is the remaining lateral axis. Which handedness
// the three make was not established.
struct ShipBodyBasis {
    float row0[3]{}; // +00h, +04h, +08h
    float row1[3]{}; // +0Ch, +10h, +14h
    float row2[3]{}; // +18h, +1Ch, +20h
};

struct ShipSteeringInputs {
    float smoothed_rudder{0.0f};  // controller+80h, before the slew
    float to_turn{0.0f};          // unit+984h, the commanded rudder
    float dt{0.0f};
    ShipBodyBasis basis{};        // 00C32000 at 0092E8F7
    OceanVec3 angular_velocity{}; // 00C31F20 at 0092E942
    // 0092E950 negated at 0092E955, then 0092E966. The host supplies the composition of
    // 00811890 and 00825DE0 because both read unit and settings state this routine does
    // not see.
    float yaw_rate_target{0.0f};
    // 0092E9D1: unit->IsKindOf(0Eh). False for a destroyer, whose IsKindOf answers
    // {0,1,2,4,5,6,7} plus its own class id (docs/UNIT_INSTANCE_UPDATE.md). When it is
    // true the routine evaluates 0042CF10(basis.row2[1]) at 0092E9E9 itself, through the
    // already reconstructed camera_asin_clamped_0042cf10.
    bool righting_active{false};
};

struct ShipSteeringStep {
    float smoothed_rudder{0.0f};  // the value written back to controller+80h
    float rate_row0{0.0f};        // the row-0 component, after the righting term
    float rate_row1{0.0f};        // the row-1 component, after the slew limiter
    float rate_row2{0.0f};        // the row-2 component, untouched
    OceanVec3 angular_velocity{}; // what reaches 00C37E20
};

// 0092E9E1..0092EA73. Active only above the threshold; the rate is pushed by the gap
// between the current angle and +/-15 degrees, at ten per second.
//
//   sign = angle > 0 ? +1 : angle < 0 ? -1 : 0
//   rate -= (sign * 0.2617991 - angle) * (dt * 10.0)
//
// Both constants are doubles in the listing and the result is stored back as a float.
// Whether this rights the hull or capsizes it depends on the sign convention linking
// row 0 to the angle asin(row2.y), which was not established; the arithmetic is exact
// and the interpretation is provisional.
float ship_righting_rate_0092e9e1(float rate, float angle, float dt) noexcept;

// 0092E8C0, void __thiscall(controller, float to_turn, float dt), RET 8, body
// 0092E8C0..0092EB9B. Complete: every branch of the body is covered.
//
// The rudder slews toward the command at dt*0.5 per step (0042AC60 at 0092E8EF), the
// angular velocity is decomposed onto the three basis rows, the row-1 component is
// slewed toward the commanded yaw rate at 2*dt per step, the row-0 component takes the
// righting term when the unit answers IsKindOf(0Eh), and the three are recomposed as
// ((row1*rate1 + row2*rate2) + row0*rate0) before the write.
ShipSteeringStep ship_apply_steering_0092e8c0(const ShipSteeringInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 00825F20's own rules
// ---------------------------------------------------------------------------

struct ShipKeelPointInputs {
    float translation[3]{}; // pose row 3, unit+FCh..+104h
    float row_up[3]{};      // pose row 1, unit+DCh..+E4h
    float row_forward[3]{}; // pose row 2, unit+ECh..+F4h
    float hull_length{0.0f}; // class+A0h
    float hull_height{0.0f}; // class+A8h
};

// 00826866..00826972. Two displacements, each minus half a hull dimension:
//   p = translation + row_forward * (-0.5 * class[A0h]) + row_up * (-0.5 * class[A8h])
// The two scales are computed as floats (FSTP at 00826872 and 0082692D) before the
// per-component multiplies, and each component is summed base-first.
OceanVec3 ship_keel_point_00826866(const ShipKeelPointInputs& in) noexcept;

struct ShipThrottleGateInputs {
    float keel_y{0.0f};       // the sampled point's y
    float wave_height{0.0f};  // 0078CF20's result at 00826985
    float throttle{0.0f};     // unit+980h
    bool out_of_action_5d{false}; // unit+5Dh
    int class_id{0};          // unit+C4h
    float pose_base_y{0.0f};  // unit+100h
    float submerged_scale{1.0f}; // settings+4B4h
};

struct ShipThrottleGate {
    bool command_applies{true}; // BL; false suppresses both 0092D300 and 0092E8C0
    float throttle{0.0f};
};

// 00826994..00826A00, in listing order:
//   if (keel_y > wave_height * 0.5) { throttle = 0; command_applies = false; }
//   else                              throttle = unit+980h
//   if (class_id == 8 && -3.0f > unit+100h) throttle *= settings+4B4h
//   if (unit+5Dh)                            throttle = 0
// The submerged scale is a submarine path (class id 8 at 008269B1, the deck reference
// -3.0f at 00CE3D50); it does not clear command_applies.
ShipThrottleGate ship_throttle_gate_00826994(const ShipThrottleGateInputs& in) noexcept;

// 00826A3A..00826A5F. ((max_speed * gameplay_scale) * throttle) * engine_gate, each
// product narrowed to float where the listing stores it.
float ship_target_speed_00826a3a(float max_speed, float gameplay_scale, float throttle,
                                 float engine_gate) noexcept;

// 00826754..0082676E, already described in docs/UNIT_FORCE_COMMANDS.md: a 0/1 float,
// open only when the jam byte is clear and the modifier is not exactly zero.
float ship_engine_gate_00826754(bool engine_jam, float thrust_mod) noexcept;

// The descriptor floats the motion path reads, by their native offsets. Only max_accel
// and retardation have Lua keys in bsp/ship_class_fields.hpp; the other four sit outside
// the range that reader covers.
struct ShipMotionClass {
    float max_accel{0.0f};       // class+504h, Lua "MaxAccel"
    float retardation{0.0f};     // class+508h, Lua "Retardation"
    float hull_mass{0.0f};       // class+B0h, read only by 00937440
    float hull_length{0.0f};     // class+A0h
    float hull_height{0.0f};     // class+A8h
    float boost_capacity{0.0f};   // class+808h
    float boost_speed_scale{0.0f}; // class+80Ch
    float boost_refill_time{1.0f}; // class+810h, a divisor with no zero guard at 00826AD6
};

struct ShipBoostInputs {
    bool trait_0e{false};        // unit->IsKindOf(0Eh) at 00826A78
    std::uint8_t order_kind{0};  // unit+988h at 00826A82
    float reserve{0.0f};         // unit+1188h
    float reference_speed{0.0f}; // 0080FC30(unit) at 00826AAB
    float boost_speed_scale{0.0f}; // class+80Ch
    float boost_capacity{0.0f};    // class+808h
    float boost_refill_time{0.0f}; // class+810h
    float dt{0.0f};
    float target_speed{0.0f};    // the product chain's result, which this may replace
};

struct ShipBoostStep {
    bool applied{false};      // the block ran at all (trait 0Eh)
    float reserve{0.0f};      // the new unit+1188h
    float target_speed{0.0f}; // the commanded speed after the block
    std::uint8_t kind_mirror{0}; // the byte copied into unit+118Ch
};

// 00826A6F..00826B04.
//
//   if (!IsKindOf(0Eh)) return unchanged;
//   unit+118Ch = unit+988h;
//   if (unit+988h != 0) {                       a boost order is standing
//       if (unit+1188h <= 0.0f) return unchanged;   the reserve is spent
//       target  = 0080FC30(unit) * class[80Ch];     the target is *replaced*
//       reserve = reserve - dt;
//   } else {                                    no boost order: refill
//       reserve = dt * class[808h] / class[810h] + reserve;
//       if (class[808h] < reserve) reserve = class[808h];
//   }
//
// The replacement at 00826AB6 writes the same stack slot the product chain wrote at
// 00826A5F, which is why it overrides rather than scales. docs/UNIT_FORCE_COMMANDS.md
// did not record this block; see the Corrections section of docs/SHIP_MOTION.md.
ShipBoostStep ship_boost_step_00826a6f(const ShipBoostInputs& in) noexcept;

// ---------------------------------------------------------------------------
// The state the tick reads and writes
// ---------------------------------------------------------------------------

// Only the fields the reconstructed path touches. This is a working state, not the
// native 0x1188-byte layout; bsp/unit_instance_layout.hpp holds that.
struct ShipMotionState {
    // Pose, unit+CCh..+108h. Rows 0..2 are the body axes, row 3 the world position.
    float pose_row0[3]{1.0f, 0.0f, 0.0f};
    float pose_row1[3]{0.0f, 1.0f, 0.0f};
    float pose_row2[3]{0.0f, 0.0f, 1.0f};
    float position[3]{};

    // Rigid body, as the physics imports see it.
    OceanVec3 linear_velocity{};
    OceanVec3 angular_velocity{};

    // The commanded pair the ring tick writes.
    float throttle{0.0f};      // unit+980h
    float to_turn{0.0f};       // unit+984h
    std::uint8_t order_kind{0}; // unit+988h

    // Controller and modifiers.
    float smoothed_rudder{0.0f}; // controller+80h
    float max_speed{0.0f};       // unit+9C0h
    float thrust_mod{1.0f};      // unit+9D8h
    float turn_efficiency{1.0f}; // unit+9DCh
    bool steering_jam{false};    // unit+9E4h
    bool engine_jam{false};      // unit+9E5h
    bool out_of_action_5d{false}; // unit+5Dh
    float propeller_load{0.0f};  // unit+1030h
    float acceleration_boost{0.0f}; // unit+1038h
    float motion_time_scale{0.0f};  // unit+340h, applied only when > 0
    float boost_reserve{0.0f};      // unit+1188h
    std::uint8_t order_kind_mirror{0}; // unit+118Ch
    int class_id{0};                // unit+C4h
};

// What one tick did, so a caller can watch it without a physics body.
struct ShipMotionStepResult {
    float dt_raw{0.0f};        // what the ring tick got
    float dt_scaled{0.0f};     // what everything after it got
    OceanVec3 keel_point{};
    float wave_height{0.0f};
    ShipThrottleGate gate{};
    float engine_gate{0.0f};
    float target_speed{0.0f};  // after the product chain and the boost block
    ShipBoostStep boost{};
    bool speed_applied{false};
    bool steering_applied{false};
    UnitAxialSpeedStep axial{};
    ShipSteeringStep steering{};
    OceanVec3 force_model_torque{}; // 00937440's contribution, when the host reports it
};

// One method per native call site on 00825F20's motion path that this reconstruction
// does not itself compute, in call order. Nothing has a default.
struct ShipMotionHost {
    virtual ~ShipMotionHost() = default;

    // 00826121: 00813020(unit+838h, dt). The ring tick is reconstructed in
    // bsp/unit_state_message.hpp; the host runs it because it owns the ring.
    virtual void tick_order_ring(float dt) = 0;

    // 00826985: 0078CF20(ocean, x, z), the local wave height.
    virtual float ocean_height(float x, float z) = 0;

    // 00826A21: 008E6430(4, unit) when DAT_00E0C978 and [[00F88C30]+B8h] are both set,
    // and the literal 1.0f otherwise. 008E6430 was not analysed by any packet.
    virtual float gameplay_scale() = 0;

    // 00826A6D: controller->vtable[0](dt). For a surface ship that slot is 00937440,
    // whose torque is reconstructed as unit_steering_torque_00937440 in
    // bsp/unit_forces.hpp; the hydrodynamic tail 009329C0 is not.
    virtual OceanVec3 run_force_model(float dt) = 0;

    // 00826AAB: 0080FC30(unit), the reference speed, read only by the boost block.
    virtual float reference_speed() = 0;

    // 0092D300's own reads, at 0092D30E and 0092D316.
    virtual OceanVec3 body_linear_velocity() = 0;
    virtual ShipBodyBasis body_basis() = 0;
    // 0092D444: 00825EC0(unit).
    virtual float forward_acceleration() = 0;
    // 0092D588: 00C37E50.
    virtual void body_set_linear_velocity(const OceanVec3& v) = 0;

    // 0092E942: 00C31F20.
    virtual OceanVec3 body_angular_velocity() = 0;
    // 0092E950 then 0092E966: 00811890 negated, then 00825DE0.
    virtual float yaw_rate_target(float smoothed_rudder) = 0;
    // unit->IsKindOf(0Eh). Queried twice per tick, at 00826A78 for the boost block and
    // at 0092E9D1 for the righting term, so one method answers both.
    virtual bool unit_trait_0e() = 0;
    // 0092EB8F: 00C37E20.
    virtual void body_set_angular_velocity(const OceanVec3& w) = 0;

    // 00826B6A and 00826B84, both outside this packet: the controller step 0092BE80
    // (bsp/unit_controller.hpp) and the unit's vtable +1ECh.
    virtual void controller_step(float dt) = 0;
    virtual void unit_post_motion(float dt) = 0;
};

// 00825F20's motion path, void __thiscall(unit, float dt), RET 4.
//
// Coverage is partial by construction and the ranges left out are listed in
// docs/SHIP_MOTION.md: the two jam ramps at 00825FB1..008260F6, the wake, anchor and
// attachment blocks at 00826187..00826866, the inlined ring setters at
// 008266CE..0082674C and the tail after 00826B84. The command path itself -
// 00826121..00826B84 minus those inlined setters - is complete.
ShipMotionStepResult ship_motion_step_00825f20(ShipMotionState& state,
                                               const ShipMotionClass& cls,
                                               ShipMotionHost& host, float dt);

// ---------------------------------------------------------------------------
// Integration, for a caller with no physics library
// ---------------------------------------------------------------------------

// Advances the position and the three pose rows from the body velocities with one
// explicit Euler step and re-orthonormalises the rows. This is NOT a reconstruction:
// the game integrates in the external physics library reached through 00C32000 /
// 00C37E20 / 00C37E50, none of which was read. It exists so a probe can plot a
// trajectory, and any trajectory it produces is the reconstructed command math driving
// a stand-in integrator.
void ship_integrate_stand_in(ShipMotionState& state, float dt) noexcept;

} // namespace bsp
