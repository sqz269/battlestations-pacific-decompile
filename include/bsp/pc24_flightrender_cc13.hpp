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
// pc24w1-sweep1-3, ocean spectrum, plane pose copies, bearings and turn rate:
//  * Ocean spectrum (src/ocean_spectrum.cpp): 00BF0DF0 Box-Muller s and result, 00B94CA0
//    Phillips arg/dot/tail and l2, 00B95750 / 00B952D0 FFT butterflies and twiddle step,
//    00B95D30 h(k, t) complex step.
//  * 007DB4D0 plane class turn rate (src/game_hosts_units.cpp): c, dd, e and
//    (ps * e) * sr + dd, 007DB566..007DB60A.
//  * 0085DAD0 (src/plane_advance_pose.cpp): this file's 00419440 length and 004F9B30 cross
//    copies and the inlined dot 0085DB43..0085DB52.
//  * 00B63F10 look-at translation dots used by 0085E4D0 (src/plane_angular_velocity.cpp
//    dot3), 00B6426A..00B642CC.
//  * 009AC190 bearing (src/plane_attitude_angles.cpp), 009F9ED0 climb angle and 009F9E40
//    heading command (src/plane_flight.cpp): atan2 stored through float before the
//    rounded FSUBR (and 009F9E40's FADD 2pi); 009F9E40 ON calls the double atan2.
// The plane pose chains run from frame-job pool work (24 or 53 bits by claiming thread) and
// are modelled at 24 per the standing policy (model 24, record 53).
// Movers: ocean wave heights, plane pose rows, plane turn rates, bearings, climb angles and
// heading commands.
//
// pc24w1-sweep1-4, the terrain segment walk in src/game_hosts_scene_contents.cpp:
//  * 00ADC5F0 block_sample: sample * scale + offset, 00ADC62E..00ADC630.
//  * 004F9B30 quad_cross (00ADEB80 line-quad test): products rounded before the FSUBRP,
//    004F9B54..004F9BA0.
//  * 00AE9BD0 leaf: the inlined 004193E0 squared length (xx + yy) + zz, 00419401..00419429.
//  * 00AE9D80 node walk: the mirrors (max + min) - s, y_in / y_out and the leaf end points
//    s + d * t, 00AE9E8B..00AEA019.
//  * 00AEA2B0 ray walk: the 00419440 length, direction (q - p) / length, the mirrors and the
//    slab t = (bound - o) / d, 00AEA2CB..00AEA465.
//  * 00ADB480 vsub_bilinear_00adb480 (VSUB vertical probe): the bilinear chain as in
//    grid_height_arm_00adb480.
//  00ADA240 landscape_entry_segment_hit's march is not live (kTerrainSegmentQuadtreeBound and
//  kTerrainVerticalSubwalkBound ON) and carries lint allows.
// Movers: terrain segment hits (line of sight and ground probes through 00ADA240) and the
// VSUB vertical probe height; not the Dyn hull ground contact.
//
// pc24w1-sweep1-5, the rest of the same walk (src/game_hosts_scene_contents.cpp):
//  * quad_dot / 00ADEB40 scalar_triple: (x*x' + y*y') + z*z', 00ADEB55..00ADEB6B.
//  * 00ADEB80 line_quad: the inlined dots, 1 / ((v + u) + w) and 1 / ((-v + u) + w) (the
//    OFF arms sum (w + v) + u), the stored weights and (cu*d + cv*{c,a}) + cw*b,
//    00ADEC6F..00ADF003.
//  * 00ADF1B0 tile_walk: the entry times and the cell corners index * 9.375 + base,
//    00ADF2ED..00ADF514.
// Movers: the same terrain segment hit points.
inline constexpr bool kPc24FlightRenderCc13Bound = false;

}  // namespace bsp
