#pragma once

// Packet cc9_plane_water_law (docs/WATER_SURFACE_LAW.md section 10). The plane's
// state-6 arm on the water, read whole from the listing:
//   0078D3D0  the water-surface normal at (x, z)
//   007DCDD0  BSP_PlaneFlight_WaterSurfaceLaw, Ghidra body 007DCDD0-007DD9AC
//   007DC205  the water band of 007DB680 BSP_PlaneFlight_CoreLaw (007DC205-007DC68C)
//   007DAFD0  the angular kick the law applies twice (body 007DAFD0-007DB1C9)
//   007DA3D0  007DA380's mode-2 factor arm (007DA3D0-007DA53F)
//   007CBC36  007CBA50's lift-off flag (007CBC36-007CBD6D)
// Every descriptive name is a hypothesis, not a recovered symbol. The main thread runs
// these at the 24-bit x87 precision control, so every term below is float arithmetic
// with the listing's float stores (docs/X87_CONTROL_WORD.md).

#include "bsp/plane_flight.hpp"

namespace bsp {

// 0078CF20 BSP_GameWorld_SampleWaterHeight as the host supplies it.
struct PlaneWaterHeightSampler {
    float (*fn)(void* ctx, float x, float z) = nullptr;
    void* ctx = nullptr;
    float operator()(float x, float z) const { return fn(ctx, x, z); }
};

// The pose as the plane carries it at unit+0CCh: row i is body axis i in world
// (0042D0D0 and 004142E0 are row-vector products v * M), plus the translation.
struct PlaneWaterPose {
    float rows[3][3]{};
    float position[3]{};
};

// 0078D3D0, __thiscall(world, float* out, float x, float z), RET 0Ch. Four samples at
// x +- 1.0 and z +- 1.0 (double 00D7A210), a = (1, h(x+1) - h(x-1), 0) and
// b = (0, h(z+1) - h(z-1), 1) (float 00D7A24C), out = normalize(b x a) through
// 004F9B30 (ECX out, EDX b, stack a) and 00419440, with a zero length giving zero.
// So n = normalize(-dhx, 1, -dhz), the differences taken over 2 m.
void water_surface_normal_0078d3d0(float out[3], float x, float z,
                                   const PlaneWaterHeightSampler& height);

// 007DCDD0's head, 007DCDD0-007DD45F: the depth below the swim line and the unit
// surface normal it hands 007DB680 and the LiftRotate kick.
struct PlaneWaterLawSample {
    float depth{0.0f};      // SwimHeight + h - y (FCHS at 007DD401); > 0 is under
    float normal[3]{};      // normalised at 007DD40B-007DD473
};

struct PlaneWaterLawClass {
    float min_water_spd_198{0.0f};   // desc+198h MinWaterSpd
    float swim_height_194{0.0f};     // desc+194h SwimHeight
    float derived_508{0.0f};         // desc+508h = -model min y - SwimHeight (007C4CF8)
    float water_rot_accel_180{0.0f}; // desc+180h WaterRotAccel
    float stall_spd_184{0.0f};       // desc+184h StallSpd
    float water_decel_1a0{0.0f};     // desc+1A0h WaterDecel
    float water_pitch_204{0.0f};     // desc+204h WaterPitch
};

struct PlaneWaterLawTuning {      // the 00F872F0 mirror of tuning+210h..347h
    float lift_depth_ratio{0.0f};        // +2D8h, 00F873B8
    float lift_max{0.0f};                // +2DCh, 00F873BC
    float decel_speed{0.0f};             // +2E0h, 00F873C0
    float side_drag_ratio{0.0f};         // +2E4h, 00F873C4
    float max_side_drag{0.0f};           // +2E8h, 00F873C8
    float min_drag_spd{0.0f};            // +2FCh, 00F873DC
    float lift_begin_dive_mul{0.0f};     // +300h, 00F873E0
    float lift_rotate_max{0.0f};         // +304h, 00F873E4
    float lift_rotate_depth_ratio{0.0f}; // +308h, 00F873E8
    float lift_rotate_angle_ratio{0.0f}; // +30Ch, 00F873EC
    float accel_cheat_mul{0.0f};         // +31Ch, 00F873FC
};

// MinWaterSpd zero (007DCDFE JP not taken): the origin and the body points (8, 0, 0)
// and (-8, 0, 0) (00CE3918, 00CE3CC8) are sampled; a wing tip standing higher than the
// origin, c = clamp((d_tip - d0) * 4.0, 0, 1) (double 00D7A328), tilts the normal's
// horizontal part away from that tip by cos(heading difference) * |n_xz| * c.
// MinWaterSpd non-zero (007DD349): the origin alone.
PlaneWaterLawSample water_law_sample_007dcdd0(const PlaneWaterLawClass& cls,
                                              const PlaneWaterPose& pose,
                                              const PlaneWaterHeightSampler& height);

// 007DC205-007DC68C, the core law's ctl+FCh == 2 band, in full (007DC426-007DC64A
// included). Its five products go into four accumulators.
struct PlaneWaterBandInputs {
    float lost_drag_timer_c3c{0.0f};  // unit+C3Ch, faded at 007DC208 by 1 - t * 0.5
    float depth{0.0f};                // the fourth argument, 007DCDD0's depth
    float accumulator_90c{0.0f};      // the third argument, unit+90Ch
    float normal[3]{};                // the fifth argument
    float world_vy_acc{0.0f};         // unit+ACCh
    float world_velocity[3]{};        // ctl+18h..20h, the drag block's [ESP+4Ch..54h]
    float body_vx{0.0f};              // ctl+3Ch, EBX at 007DC617
};

struct PlaneWaterBandTerms {
    float world_gravity[3]{};    // dyn+28h += B * n, 007DC448-007DC45B
    float world_drag[3]{};       // dyn+10h += E * n, 007DC4AC-007DC4BF
    float world_resist_34[3]{};  // dyn+34h += F * (dx, 0, dz), 007DC601-007DC614
    float body_damping_x{0.0f};  // dyn+04h += side drag, 007DC689
};

PlaneWaterBandTerms water_band_007dc205(const PlaneWaterBandInputs& in,
                                        const PlaneWaterLawClass& cls,
                                        const PlaneWaterLawTuning& tuning);

// Adds the band to the accumulators. dyn+34h has no slot in PlaneDynAccumulators; its
// only consumer is 007D8470's third fold (007D84C2-007D84FF), 0042D0D0 through ctl+0B0h
// into dyn+40h, which is done here.
void add_water_band(PlaneDynAccumulators& acc, const PlaneWaterBandTerms& terms,
                    const float world_to_body[9]);

// 007DAFD0, __thiscall(ctl, const float* body_point, const float* world_dir,
// float accel, float step, float limit), RET 14h. axis = normalize(dir x R(point))
// (004F9B30 with EDX = dir), with unit+674h standing in when |axis| < 0.001
// (double 00CF3F30); then per world component, c = accel * axis_i, and
// w_i += step * c only while w_i has not yet reached limit * c in c's direction.
void water_angular_kick_007dafd0(float angular_world[3], const PlaneWaterPose& pose,
                                 const float body_point[3], const float world_dir[3],
                                 float accel, float step, float limit,
                                 const float fallback_axis_674[3]);

// 007DD493-007DD9AA, 007DCDD0's tail after the core law: the LiftRotate kick toward
// the normal, the ctl+94h nose-down kick, ctl+8Ch = 0, and the angular damping below
// StallSpd. Returns nothing; the law's ST0 result is the sample's depth.
struct PlaneWaterTailInputs {
    float depth{0.0f};
    float normal[3]{};
    float step{0.0f};
    bool nose_down_94{false};      // ctl+94h, set by 007D8180 (007CBC02 for a dead plane)
    float horizontal_speed{0.0f};  // |(vtable[+34h]() with y = 0)|, 0042B2F0
    float pitch_c64{0.0f};
    float roll_c68{0.0f};
    float forward_speed_44{0.0f};  // ctl+44h
    float fallback_axis_674[3]{};  // unit+674h..67Ch
};

void water_law_tail_007dd493(float angular_world[3], const PlaneWaterTailInputs& in,
                             const PlaneWaterPose& pose, const PlaneWaterLawClass& cls,
                             const PlaneWaterLawTuning& tuning);

// 007DA3D0-007DA53F, 007DA380's mode-2 arm. outA scales the roll and pitch rate,
// outB the yaw rate; the byte is 0 (007DA53A).
struct PlaneWaterRateInputs {
    float authority{0.0f};       // 007D9A70's result
    float up_y{0.0f};            // unit+E0h, the body up axis' world y
    float speed_6c{0.0f};        // ctl+6Ch
    float throttle_9f0{0.0f};    // unit+9F0h
    float yaw_control_factor{0.0f};      // +2C4h, 00F873A4
    float normal_yaw_control_spd{0.0f};  // +2C8h, 00F873A8
    float max_yaw_control_spd{0.0f};     // +2CCh, 00F873AC
    float max_yaw_control{0.0f};         // +2D0h, 00F873B0
    float max_ctrl_angle{0.0f};          // +2ECh, 00F873CC
    float min_ctrl_angle{0.0f};          // +2F0h, 00F873D0
};

struct PlaneWaterRateFactors {
    float out_a{1.0f};
    float out_b{1.0f};
};

PlaneWaterRateFactors water_rate_factors_007da3d0(const PlaneWaterRateInputs& in);

// 007CBC36-007CBD6D: the byte [ESP+68h] that keeps the plane on the water. Set when
// -depth < desc+508h + 1.5 (double 00CE3D78); otherwise, with unit+770h set, set when
// any hull point of desc+5BCh..5C0h is under the water, and when none is, the arm
// calls (unit+72Ch)->vtable[+24h](0). Clear means 007CBED9 requests state 7.
struct PlaneWaterLiftOff {
    bool stay_on_water{true};
    bool hull_clear_call{false};  // (unit+72Ch)->vtable[+24h](0) at 007CBD6B
};

PlaneWaterLiftOff water_lift_off_007cbc36(float depth, float derived_508, bool hull_check_770,
                                          bool any_hull_point_under);

}  // namespace bsp
