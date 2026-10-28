#pragma once

// Non-PC24 deviations in src/game_hosts_units.cpp (packet cc13-unitsdev). Header-only.

namespace bsp {

// kUnitsHostDeviationsCc13Bound (OFF): the image's own guards and steps where the host
// body differed in kind, not in rounding. OFF keeps the previous code unchanged.
//   009BEBA0 moveto_refresh: distance is 0 unless d2 > 1e-10 (FCOMI against [00CE3820],
//            JBE so NaN also gives 0); the host used 1e-6.
//   0099F1C0 terrain avoidance c0 floor: compared against double 0.10000000149011612
//            ([00D7A3A0]) and replaced with float 0.1f ([00D7A2F0]); no float lies
//            between the two, so this is identity.
//   0099F1C0 pass-1 pitch: reads unit+C7Ch, the velocity elevation 007C1900 writes only
//            when |v| > 0.1 (007C1C55) and otherwise keeps; the host recomputed atan2 when
//            |v| > 1 and fell back to unit+C64h. ON adds the C7Ch slot field and its writer
//            in refresh_attitude_007c1900, which all three host arms (free flight, ground
//            roll/deck, surface) call before the think. The image writes C7Ch at the end
//            of the previous step (007CEE9C, after think and motion); the two agree in
//            steady free flight and differ after a collision response, a deck carry or an
//            out-of-step velocity write (labelled; see the comment at the reader).
//   Not a deviation: 0099FBED and 0099FCA0 call 00419510 with EDX = the pass-1 direction
//            but ECX = an unused temporary ([ESP+468h] / [ESP+45Ch]), so the direction is
//            never normalized; the host matches and nothing is modelled.
// Movers when ON: plane moveto distances near zero, and the pass-1 pitch (so terrain
// probe points, avoidance bands and the pilot pitch commands) below |v| = 1.
inline constexpr bool kUnitsHostDeviationsCc13Bound = false;

}  // namespace bsp
