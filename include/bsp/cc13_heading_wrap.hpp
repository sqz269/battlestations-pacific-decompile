#pragma once

namespace bsp {

// Packet cc13-wrap605 (docs/CC13_WRAP605.md). kShipAiHeadingStoreWrapCc13Bound:
// apply 00605070 to blk+1D8h (desired_heading) in place after the four ship AI
// stores the image wraps: 009DFF81 (009DFF40), 009E00FA (009E00A0), 009E0083
// (the setter 009E0040) and 009F3360 (the attackmove approach). OFF, the host
// stores the value unwrapped and only counts it (cc9_heading_wrap_census).
//
// 00605070 is __thiscall(float* angle), RET 0: fmod by 2pi (double 00CE3828,
// CRT 00BF857A) stored as float, then +2pi when -pi (00CE3D18) >= it, else
// -2pi when it > pi (00CE3D28); bsp::ship_ai_firepower_wrap_angle_00605070.
//
// ON reach: every reader of the field, and of blk+324h after 009ED947 copies
// it, goes through 00438AA0 / 00438B10 (which loop into (-pi, pi]), sin/cos, or
// a log, so ON moves float rounding and the logged heading values only, on
// rows that store a heading outside (-pi, pi] (JM06 in the census).
// No parent switch: the four hooks run on the OFF path already.
inline constexpr bool kShipAiHeadingStoreWrapCc13Bound = false;

}  // namespace bsp
