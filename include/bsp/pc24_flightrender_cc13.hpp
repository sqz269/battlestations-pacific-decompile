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
// Other copies of the same addresses, not narrowed here: 00ADB480 vsub_bilinear_00adb480
// (game_hosts_scene_contents.cpp, packet pc24w1-sweep1-4); 004F9B30 quad_cross (same file,
// pc24w1-sweep1-4) and plane_angular_velocity.cpp / plane_pose_commit.cpp /
// plane_advance_pose.cpp (pc24w1-sweep1-2, -3).
// Movers: terrain heights and normals (00ADB3A0 grid lookups for ground contact, wakes,
// land-convoy and ground AI clearance) and the TRNV2 origin set at load.
inline constexpr bool kPc24FlightRenderCc13Bound = false;

}  // namespace bsp
