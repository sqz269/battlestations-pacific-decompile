#pragma once

namespace bsp {

// Packet cc13-wrap605 (docs/CC13_WRAP605.md). kShipAiHeadingStoreWrapCc13Bound:
// three host hooks cover the four ship AI sites that wrap blk+1D8h
// (desired_heading) in place with 00605070: SetterBinding covers 009E0083 (the
// setter 009E0040), HeadingHoldBinding covers 009DFF81 (009DFF40) and 009E00FA
// (009E00A0), and ApproachStepBinding covers 009F3360 (the attackmove
// approach). OFF, the host stores the value unwrapped and only counts it
// (cc9_heading_wrap_census).
//
// 00605070 is __thiscall(float* angle), RET 0: fmod by 2pi (double 00CE3828,
// CRT 00BF857A) stored as float, then +2pi when -pi (00CE3D18) >= it, else
// -2pi when it > pi (00CE3D28); bsp::ship_ai_firepower_wrap_angle_00605070.
//
// ON reach: every reader of the field, and of blk+324h after 009ED947 copies
// it, goes through 00438AA0 / 00438B10 (which loop into (-pi, pi]), sin/cos, or
// a log, so ON moves float rounding and the logged heading values only, on
// rows that store a heading outside (-pi, pi]. The 2026-10-19 census
// (cc9-lua52 l70r_n_*_main_ref.log) finds those on 14 of 24 rows, up to
// 14134 stores on j6k; the table is in the doc.
// The census bound is exact: 00CE3D18 / 00CE3D28 are float pi widened to
// double (bytes 00000060FB2109C0 / ...0940 = -/+3.1415927410125732), not
// double pi, so count_heading_wrap_store's float compare against 3.14159265f
// (the same float) matches 00605070's arms: wrap when x <= -pi
// (00605091 FCOMIP / JB) or x > pi (006050A8 FCOMI / JBE).
// No parent switch: the three hooks run on the OFF path already.
inline constexpr bool kShipAiHeadingStoreWrapCc13Bound = true;  // ON: docs/SHIP_AI_OPEN_ITEMS.md 311 (JM06 j6k/j6n, IJN18 pairs)

}  // namespace bsp
