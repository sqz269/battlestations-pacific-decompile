#pragma once

// PC24 wave switch, lane "units host" (game_hosts_units.cpp). Header-only; the sites are
// listed in .cpp comments next to each `if constexpr`.

namespace bsp {

// kPc24UnitsHostCc13Bound: the main thread runs the x87 at 24-bit precision control, so every
// FADD/FSUB/FMUL/FDIV result rounds to 24 bits (extended exponent). OFF keeps the host bodies'
// double chains that round once; ON narrows each live chain per x87 op with the
// include/bsp/cc13_x87_model.hpp helpers.
inline constexpr bool kPc24UnitsHostCc13Bound = false;

}  // namespace bsp
