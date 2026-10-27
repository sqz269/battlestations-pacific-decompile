#pragma once

namespace bsp {

// PC24 wave 1, lane sweep1 (flight core and render). The main thread runs the x87 at 24-bit
// precision control: every FADD/FSUB/FMUL/FDIV/FSQRT result is rounded to a 24-bit
// significand (extended exponent) before the next op. The OFF arms evaluate each chain in
// double and round once at the float store.
//
// kPc24FlightRenderCc13Bound (OFF): ON = per-op PC24, each op rounded as the listing does
// (bsp/cc13_x87_model.hpp x87_*24). Sites by packet:
//
// pc24w1-sweep1-1, the terrain height-field leaves of src/game_hosts_scene_contents.cpp:
//  * 00ADDA60 TRNV2 origin, 00ADDB65..00ADDB81 and 00ADDBBF..00ADDBCC: (box / 300) - 1.0.
//  * 00ADA900 height and 00ADABA0 normal: ((world - node) - origin) * float(1 / 9.375),
//    00ADA922..00ADA93D / 00ADA94A..00ADA968 and 00ADABC4..00ADABDE / 00ADABEB..00ADAC01.
//  * 00ADA160 local height and 00ADA1C0 local normal: (local - origin) * inv,
//    00ADA174..00ADA196 and 00ADA1D3..00ADA201.
//  * 00ADB480 bilinear: a = h00 + fu * (h10 - h00), b = h01 + fu * (h11 - h01) and
//    a + fv * (b - a), each product and sum rounded (00ADB4EC..00ADB542).
//  * 00ADAA40 cell normal: the inlined 004F9B30 cross (each product rounded before its
//    difference, 004F9B54..004F9BA0) and the 00419440 length sum (xx + yy) + zz, the
//    partial sum rounded but not stored (0041947D..00419485).
// 00ADB480 and 00ADAA40 are switched only through the main-thread entries
// (grid_height_arm_00adb480 / cell_normal_arm_00adaa40 with this switch). The Dyn hull terrain
// contact (src/hull_terrain_contact.cpp, under DynWorkerPrecisionScope) calls the
// grid_height_pc53_00adb480 / cell_normal_pc53_00adaa40 entries, which stay 53-bit either way.
// Movers: main-thread terrain heights and normals only (00ADB3A0 grid lookups for wakes,
// land-convoy and ground AI clearance), and the TRNV2 origin set at load; not the Dyn hull
// ground contact.
//
// pc24w1-sweep1-2, ocean grid/coverage and plane pose leaves:
//  * 00B960C0 ocean grid (src/ocean_wave_field.cpp): u * 63, 63 * v and the three lerps
//    x + (y - x) * t, 00B9627A..00B962C7.
//  * 00B9CF50 ocean mask (src/ocean_wave_field.cpp): (1 - (z - z0) * sz) * h, w - 1,
//    ((x - x0) * sx) * w, h - 1 and byte / 255, 00B9CFB3..00B9D0C5.
//  * 00419440 length copies (src/world_ocean.cpp, plane_pose_commit.cpp,
//    plane_angular_velocity.cpp length3): squares and (xx + yy) + zz, 00419461..00419489.
//  * 004F9B30 cross copies (same three files, cross3): products rounded before the FSUBRP,
//    004F9B54..004F9BA0.
//  * 00413920 multiply_00413920 (plane_angular_velocity.cpp): ((l0r0 + l1r1) + l2r2) + l3r3.
//  * 0042D0D0 transform_direction_0042d0d0 (plane_angular_velocity.cpp): (v0c0 + v1c1) + v2c2.
//  * 0085DC80 dot3_x87 (plane_pose_commit.cpp): (y*y' + x*x') + z*z'.
// Movers: ocean wave grid and coverage mask, plane pose normalisation, angular velocity and
// body-frame transforms.
//
// Copies still to narrow: 00ADB480 vsub_bilinear_00adb480 and 004F9B30 quad_cross
// (game_hosts_scene_contents.cpp, pc24w1-sweep1-4); plane_advance_pose.cpp and the
// plane_angular_velocity.cpp look-at dot3 (pc24w1-sweep1-3, sites in those .cpp comments).
inline constexpr bool kPc24FlightRenderCc13Bound = false;

}  // namespace bsp
