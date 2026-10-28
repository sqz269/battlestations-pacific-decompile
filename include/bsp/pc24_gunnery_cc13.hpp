#pragma once

// PC24 wave 1, packet pc24w1-gunnery-1. These bodies run on the main thread, whose x87 runs
// at 24-bit precision control: every FADD/FSUB/FMUL/FDIV rounds to a 24-bit significand
// (with the extended exponent) before the next op. ON evaluates each listed chain per op
// through the cc13x87 helpers (include/bsp/cc13_x87_model.hpp) in the image's operation
// order; OFF keeps the earlier double chains rounded once.
namespace bsp {

// Gunnery: src/game_hosts_gunnery.cpp, src/gun_aim_terms.cpp, src/gun_gravity_arc.cpp and
// src/gun_dispersion.cpp. The sites are listed at each `if constexpr` in those files.
// Packet pc24w1-gunnery-2 adds: src/game_hosts_units.cpp 0099EC40 gunfire_detect (the
// cone test, the shooter-line point, the float threat sums and 004F2F40's normalize with
// its 1e-5 floor), src/gameplay_settings_tail.cpp 008383D0 (t*10 - (i+1)) and
// src/hit_narrowphase.cpp 0085CDB0 (the slab fractions' numerators). 00956C20's blast
// sum in src/gunnery_tables.cpp already rounds as the image does and is not switched: the
// double sum of two floats is not exact, but rounding it to double and then to float equals
// one float rounding (double rounding is innocuous, 53 >= 2*24 + 2), x0.5 is exact, then a
// float add.
// Expected movers when ON: plane gunfire avoidance (flag, direction and timers),
// weapon hit accuracy samples, and segment/box hit entry points and line-of-fire tests.
inline constexpr bool kPc24GunneryCc13Bound = true;  // ON (cc9-sweep2b, 2026-10-28): joint pair with the gunnery/follow_to_body switch on d3ad11c39; see commit

}  // namespace bsp
