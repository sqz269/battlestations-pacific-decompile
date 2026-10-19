#pragma once

// Packet cc13-ver3 (docs/CC13_VER_3.md): byte verification of cc9's ship motion host
// (00825F20's command tail, 0092D300, 0092E8C0, 00811890, 00937440, 009329C0, 00834E90).
// These are reference models of what the host substitutes or omits. They are
// reconstructed and build-tested only: not ABI-compatible, not wired, not game-validated.
// Descriptive names are hypotheses.

#include "bsp/world_ocean.hpp"  // OceanVec3

namespace bsp {

// 00810010 ECX owner, EAX=[owner+4F8h], RET. Meaning of this word remains provisional.
std::uint32_t ship_motion_word_4f8_00810010(const void*) noexcept;
// 00815590 ECX two floats, stack writer, RET4: writer slot0C receives
// key {0,"thrust"}/{0,"toTurn"}, tagged value {2,float}, in that order.
struct ShipMotionFloatArchiveCalls {
    virtual ~ShipMotionFloatArchiveCalls() = default;
    virtual void write_float_slot0c(void* writer, const char* key, float value) = 0;
};
void ship_motion_write_controls_00815590(const float* controls, void* writer, ShipMotionFloatArchiveCalls&);

// ---------------------------------------------------------------------------
// Settings this installation feeds the motion tail (00424C40 singleton), read from
// scripts/datatables/shipglobals.lua on disk. The host passes 1.0 / 0 / 0 for the
// first three (src/ship_motion.cpp:307, src/game_hosts_units.cpp:9595 and :9989).
// ---------------------------------------------------------------------------
struct ShipMotionSettingsCc13 {
    float submarine_depth_speed_mul_4b4{0.834f};  // Submarine.SubmarineDepthSpeedMul, 008269E6
    float tboat_torque_mul_588{1500.0f};          // Physics.TBoatNyomatekSzorzo, 00937592
    float avoidance_turn_spd_limit_220{4.0f};     // AvoidanceCheat.TurnSpdLimit, 00825E1B
    float avoidance_turn_spd_mul_224{0.5f};       // AvoidanceCheat.TurnSpdMul, 00825E44
    float avoidance_stop_spd_mul_228{2.0f};       // AvoidanceCheat.StopSpdMul, 00825EEE
};

// 008269B1..008269F0. A unit with class id 8 (submarine) whose pose base Y lies strictly
// below the -3.0f at 00CE3D50 has its gated throttle multiplied by settings+4B4h. The
// product is one FMUL and one FSTP float.
float ship_submerged_throttle_008269b1(int class_id, float pose_base_y, float throttle,
                                       float depth_speed_mul) noexcept;

// 00857F40, the TorpedoBoat class reader after the ship reader 00831840: three keys and
// their reader defaults. 00857C80 later seeds the unit's reserve +1188h from +808h
// (bsp::torpedo_boat_sentity_init_00857c80 in gameplay_seg53_cc13.hpp).
struct TorpedoBoatTurboClass {
    float turbo_time_808{10.0f};             // "TurboTime" (00D061F0), default 00CE38B8
    float turbo_strength_80c{3.0f};          // "TurboStrength" (00D061CC), default 00CE3854
    float turbo_recharging_time_810{10.0f};  // "TurboRechargingTime" (00D061DC), default 00CE38B8
};
struct TorpedoBoatTurboRow {
    bool has_time{false};
    float time{0.0f};
    bool has_strength{false};
    float strength{0.0f};
    bool has_recharging_time{false};
    float recharging_time{0.0f};
};
TorpedoBoatTurboClass torpedo_boat_turbo_class_00857f40(const TorpedoBoatTurboRow& row) noexcept;

// 00937440, the force callback of controller variant C only (vtable 00D196F0, built by
// 00939F20, whose only caller 00857C20 is the TorpedoBoat vtable's controller creator).
// The base controller vtable 00D19630 has 009329C0 itself in slot 0, so other surface
// ships get no rudder torque. Skipped when unit+5Dh is set (00937449).
struct TorpedoBoatRudderTorqueInputs {
    OceanVec3 velocity{};         // 00C31F40
    float row2[3]{};              // basis +18h..+20h for the dot; pose unit+ECh..F4h for the torque
    float reference_speed{1.0f};  // 0080FC30
    float pose_row0_y{0.0f};      // unit+D0h
    float to_turn{0.0f};          // unit+984h
    float torque_mul{0.0f};       // settings+588h
    float hull_mass{0.0f};        // class+B0h
};
// The image's grouping: along = (vy*fy + vx*fx) + vz*fz (one float store), ratio =
// |float(along / ref)|, g = float((((1 - |r0y|) * ratio) * to_turn) * mul),
// p_i = float(g * f_i), m2 = float(mass*mass), torque_i = float(float(p_i * m2) / 10000.0).
OceanVec3 tboat_rudder_torque_00937440(const TorpedoBoatRudderTorqueInputs& in) noexcept;

// 0092E9E1..0092EA6D with the image's constants. 00D19628 is the double
// 0x3FD0C15240000000 and 00D05AA8 the float 0x3E860A92: both are float(pi/12) =
// 0x1.0c1524p-2. include/bsp/ship_motion.hpp carries 0.2617991025660515 and
// 0.2617843747f. Every intermediate is rounded to float, the main thread's 24-bit
// precision (docs/CC13_FPU_PRECISION.md); a pool-worker run at 53 bits keeps dt*10 and
// error*(dt*10) unrounded and can differ in the last place.
inline constexpr double kShipRightingAngle00D19628 = 0x1.0c1524p-2;
inline constexpr float kShipRightingThreshold00D05AA8 = 0x1.0c1524p-2f;
float ship_righting_rate_0092e9e1_cc13(float rate, float angle, float dt) noexcept;

}  // namespace bsp
