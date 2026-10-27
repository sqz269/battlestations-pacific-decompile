#pragma once

// PC24 wave 1, packet pc24w1-gunnery-1. These bodies run on the main thread, whose x87 runs
// at 24-bit precision control: every FADD/FSUB/FMUL/FDIV rounds to a 24-bit significand
// (with the extended exponent) before the next op. ON evaluates each listed chain per op
// through the cc13x87 helpers (include/bsp/cc13_x87_model.hpp) in the image's operation
// order; OFF keeps the earlier double chains rounded once.
namespace bsp {

// Gunnery: src/game_hosts_gunnery.cpp, src/gun_aim_terms.cpp, src/gun_gravity_arc.cpp and
// src/gun_dispersion.cpp. The sites are listed at each `if constexpr` in those files.
inline constexpr bool kPc24GunneryCc13Bound = false;

}  // namespace bsp
