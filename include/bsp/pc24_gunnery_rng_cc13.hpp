#pragma once

// PC24 wave 2, packet pc24w2-gunrng. These gunnery sites run on the main thread, whose x87
// runs at 24-bit precision control, and each decides a value or a draw count on the shared
// random stream (00BD2F10), so they have their own switch: cc9 can pair them apart from
// kPc24GunneryCc13Bound. ON evaluates each chain per op in the image's order through the
// cc13x87 helpers (include/bsp/cc13_x87_model.hpp); OFF keeps the earlier double chains.
namespace bsp {

// kPc24GunneryRngCc13Bound (ON), src/game_hosts_gunnery.cpp:
//   00BF0DF0 (through 00BD2F90), the polar normal draw in draw_normal: the CRT log of s is
//            rounded to float before -2 * L / s (00BF0F60..00BF0F77), as the image stores it.
//   00933DA0 the sink grid counts (int)(extent * 1.5 / 15 + 1), each op rounded before
//            _ftol (00933DC2..00933E1C); the count sets how many draws the grid takes.
// Movers: the AA gunner aim error (00BD2F90 draws) and, rarely, the number of sink-effect
// draws on the shared stream, so every later draw on it.
// Flipped ON (cc9-nondet1, 2026-10-28): paired on the 10 WINS rows that call 00BF0DF0,
// gameplay identical on every one (deaths, hits, death rows, unit tables). About 13% of
// the draws move k by an ulp without changing an outcome; the grid count never differed.
inline constexpr bool kPc24GunneryRngCc13Bound = true;

}  // namespace bsp
