#pragma once

namespace bsp {

// PC24 wave 1, lane sweep1 (flight core and render). The main thread runs the x87 at 24-bit
// precision control: every FADD/FSUB/FMUL/FDIV/FSQRT result is rounded to a 24-bit
// significand (extended exponent) before the next op. The OFF arms evaluate each chain in
// double and round once at the float store.
//
// kPc24FlightRenderCc13Bound (packet pc24w1-sweep1-1, OFF): ON = per-op PC24 in the terrain
// height-field leaves of src/game_hosts_scene_contents.cpp, with each op rounded as the
// listing does (bsp/cc13_x87_model.hpp x87_*24):
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
// Movers: terrain heights and normals (00ADB3A0 grid lookups for ground contact, wakes,
// land-convoy and ground AI clearance) and the TRNV2 origin set at load.
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
// Copies still to narrow: 00ADB480 vsub_bilinear_00adb480 and 004F9B30 quad_cross
// (game_hosts_scene_contents.cpp, pc24w1-sweep1-4).
inline constexpr bool kPc24FlightRenderCc13Bound = false;

}  // namespace bsp
