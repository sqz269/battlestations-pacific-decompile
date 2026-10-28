#pragma once

// PC24 wave switch for the units host (cc13 pc24w1, packets pc24w1-units-*):
// the bodies inlined into GameUnitsHost's motion step (game_hosts_units.cpp).
// The game's main thread runs the x87 with 24-bit precision control, so every
// FADD/FSUB/FMUL/FDIV/FSQRT result is rounded to a 24-bit significand
// (docs/CC13_FPU_PRECISION.md). The host bodies evaluated multi-op chains in
// double and rounded once; the ON arms round per op through the cc13x87
// helpers in the image's operation order. Each OFF arm is the previous code,
// unchanged. None of these bodies runs under DynWorkerPrecisionScope.

#include "bsp/cc13_x87_model.hpp"

namespace bsp {

// kPc24UnitsHostCc13Bound (OFF): per-op PC24 arithmetic in
//   units-A (cc13-pc24w1-units-A), the dogfight host bodies:
//     004142E0 df_local (the host's d . row / |row| stand-in for the vector
//              times unit+110h; pc24w2-units made the ON arm take the image
//              path, the 00B63D50 inverse at +110h then
//              Impl::to_body_image_004142e0)
//     007B4E90 pattern df_heading_of (pi/2 - atan2, +2pi wrap; 009A8873-009A889A)
//     009A87F0 df_maneuver_enter_009a87f0 (009A89BD-009A89F5)
//     009A8B20 df_maneuver_tick_009a8b20 (009A8CB3-009A8CC7 ceiling; the speed
//              is vtable[38h] 007B8E60 = unit+B1Ch, 007D8020's inline
//              0042B2F0 of ctl+18h)
//     009A80E0 df_avoid_turn_tick_009a80e0 (009A82CD-009A8312)
//     00954650 df_predict_00954650 with 0042B2F0, 004F9B30, 00952340, 0085C3F0
//     009FC7C0 df_set_lead_bound's dist (009FC931-009FC9F6, 0042B2F0) and
//              df_fine_aim_009f9fc0 (009FCB25 length; the +68h world rebuild is
//              a host stand-in)
//     009AAA80 df_early_edge_009aaa80 (009AAB96-009AAC31; speed via 007B8E60
//              / 007D8020 as above)
//     009A76E0 df_arm_body_009ab1c0 (tan_len 00414C60 at 009A77F4; target
//              speed, probably 007B8E60 / 007D8020 as above)
//   units-1 (cc13-pc24w1-units-1):
//     00A010F0 tick_danger_cost_00a010f0, 00A29FD0 capture_plan_00a29fd0 and
//              00A1A660 in capture_defenders_00a24870 (game_hosts_ai.cpp)
//     00417FA0 avoid_zone_world_to_cell_00417fa0 (game_hosts_avoid_zones.cpp)
//     00836920 director_step_00836920 arrival radius (game_hosts_commands.cpp)
//     006F5CC0 ensure_parent_link_71c_006f5cc0 (game_hosts_units.cpp)
//     007B11F0 start_index_for_position_007b11f0 (gameplay_seg46_cc13.cpp)
//     00743060, 007410C0 and 00742400 (group arc, lateral offset) land convoy
//              arithmetic (land_and_structures.cpp)
// Expected movers when ON: dogfight lead points and fine-aim angles, the arc
// predictor's arc/straight verdicts, maneuver headings and pitch, the avoid
// turn's bank decay, and early-edge target pickups; AI danger costs, capture
// planning and defender counts, avoid-zone cells, arrival-radius stage raises,
// command-building adoption, path start indices and land convoy arcs.
//   units-DE (cc13-pc24w1-units-DE), the strafe/ordnance and flight-core host
//   bodies (PlaneBinding methods unless named otherwise):
//     009CA870 st_gotowards_tick (3-D and planar squared distances)
//     009CB1B0 st_aim_tick (|l|^2, stored as float at 009CB349)
//     009CADB0 st_attackrun_tick (ceiling * 0.9 - y)
//     007C1FB0 lb_rack_timing (BombDelay * (n - 1) + longest)
//     009C58D0 run_dive_bomb_aimdive_tick (stand-in d . row / |row|^2, per op as
//              df_local)
//     009BEBA0 moveto_refresh (planar distance dx*dx + dz*dz)
//     009FBB20 circle_steer (d2 and w = (ratio - 1) / 0.2)
//     007D9A70 control_authority (hold * 0.6 + r)
//     007DC830 free_flight_wreck_lift (|bank| * 0.8 + 0.6 and (q * u) * h)
//     007CE040 motion_step fixed-step velocity / acceleration (FDIV by the step,
//              not * 1/dt; 007CED84-007CEE3F)
//     007B8E70 kz_forward_speed_b2c (stand-in dot in 0042D0D0's order, the ctl+44h
//              writer at 007D9C34; PROVISIONAL: the 0085DEA0 matrix vs pose_row2)
// Movers (units-DE): strafe range/height and aim, level-bomb release delay,
// dive-bomb aim point, moveto distance, circle steer, control authority, wreck
// lift, the fighter lead's target acceleration, kamikaze forward speed.
//   units-BC (cc13-pc24w1-units-BC), the land and takeoff host bodies:
//     009B0300 run_land_line_tick_009b0300 (s2 009B04E7-04EF, near-field ratio
//              009B0667-06A1, upper/lower 009B0766-07CE, +2B4h 009B0899-08B6)
//     009AFAF0 land_geometry_009afaf0 (dx dx + dz dz 009AFC03-FC0B)
//     007C07A0 arm_direction_hold_007c07a0 (pi/2 - float(atan2) 007C0843-084B)
//     009B22C0 run_land_park_tick_009b22c0 (retarget 009B2423-2433, heading
//              009B29A0-29AE, speed scale 009B2A5E-2ABD, deadband 009B2B97-2BA7)
//     009B1300 land_min_speed_009b1300 (0042B2F0 |v|, inside the
//              kLandMinSpeedOwnerVelocityBound ON arm)
//     009C18C0 run_land_moveto_tick_009c18c0 (the 009F9E40 bearing 009F9E5D-9E91)
//     009CDE50 run_takeoff_prepare_step_009cde50 (wobble angle 009CDF3E-DF46,
//              00419260 length, member distance via 0042B2F0 at 009CE083)
//     009CE2C0 run_takeoff_run_step_009ce2c0 (ten chains, 009CE5C0-F41E)
//     009B3900 run_land_task_tick_009b3eb0 (altitude decay 009B3A91-3A9F, request
//              pacing 009B3AFA-3B06)
// Expected movers when ON (units-BC): landing approach speeds, line and park
// steering, the park yaw and speed scale, the land moveto bearing, the takeoff
// wobble and squadron spacing, takeoff pitch, yaw and throttle, and the land
// request pacing and altitude decay.
// ON (cc9-sweep3b, 2026-10-28), paired alone and judged by plausibility on six short rows
// (OFF/ON = origin/main 64af020e2): USN01 415.02 -> 430.02 s, BSM09 450.01 both, LOMP10
// 1037.44 -> 863.46, PRCP01 689.96 -> 839.22, USN12 1560.40 -> 1555.39, BSM06 h1_i completed
// 1149.00 -> failed 1079.93. Flooding (USN12 407/406 floods) and deck recovery stay plausible.
inline constexpr bool kPc24UnitsHostCc13Bound = true;

namespace pc24_unitshost {

// 0042B2F0: (y*y + x*x) + z*z, stored as a float, sqrt when above 1e-10
// [00CE3820], else 0; the result is stored as a float.
inline float length3_0042b2f0(float x, float y, float z) noexcept {
    using namespace bsp::cc13x87;
    const float s = static_cast<float>(
        x87_add24(x87_add24(x87_mul24(y, y), x87_mul24(x, x)), x87_mul24(z, z)));
    if (!(static_cast<double>(s) > 1e-10)) return 0.0f;
    return static_cast<float>(x87_sqrt24(s));
}

// 00414C60: x*x + y*y, the same cutoff and stores.
inline float length2_00414c60(float x, float y) noexcept {
    using namespace bsp::cc13x87;
    const float s = static_cast<float>(x87_add24(x87_mul24(x, x), x87_mul24(y, y)));
    if (!(static_cast<double>(s) > 1e-10)) return 0.0f;
    return static_cast<float>(x87_sqrt24(s));
}

// (a0*b0 + a1*b1) + a2*b2, each op rounded, the sum stored as a float.
inline float dot3(const float a[3], const float b[3]) noexcept {
    using namespace bsp::cc13x87;
    return static_cast<float>(x87_add24(x87_add24(x87_mul24(a[0], b[0]), x87_mul24(a[1], b[1])),
                                        x87_mul24(a[2], b[2])));
}

// 004F9B30: out = a x b, out_i = a_j*b_k - a_k*b_j, each component stored.
inline void cross_004f9b30(const float a[3], const float b[3], float out[3]) noexcept {
    using namespace bsp::cc13x87;
    out[0] = static_cast<float>(x87_sub24(x87_mul24(a[1], b[2]), x87_mul24(a[2], b[1])));
    out[1] = static_cast<float>(x87_sub24(x87_mul24(a[2], b[0]), x87_mul24(a[0], b[2])));
    out[2] = static_cast<float>(x87_sub24(x87_mul24(a[0], b[1]), x87_mul24(a[1], b[0])));
}

}  // namespace pc24_unitshost

}  // namespace bsp
