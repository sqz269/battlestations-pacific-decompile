#pragma once

// cc13-flybearing: the 009C62B0 flyabove lead bearing, per x87 op at PC24.
// Header-only switch.

namespace bsp {

// kFlyaboveLeadBearingPc24Cc13Bound (OFF): in update_dive_bomb_approach
// (game_hosts_units.cpp), the flyabove lead bearing B = pi/2 - atan2(z, x) follows
// 009C62B0's sequence: 009C63C4 FSTP dword stores the 00BF701A atan2 as a float,
// 009C63CC FSUBR qword [00CE3830] (pi/2 as the widened float, dive_bomb_constant::
// kHalfPi) rounds at PC24, and 009C63D2 FSTP dword stores it. OFF keeps the previous
// double expression rounded once. Split from kPc24PlaneTasksCc13Bound, which was
// already ON, so that this lands OFF and is paired before it flips.
// Residual: the CRT double atan2 vs the x87 FPATAN at the last ulp (FPATAN ignores
// PC; 00BF701A sets its own control word).
// Movers when ON: B feeds lead_bearing_error, which drives the flyabove bank and
// heading every tick of the dive approach, and the readers of
// db_flyabove_lead_bearing (last float ulp).
inline constexpr bool kFlyaboveLeadBearingPc24Cc13Bound = true;  // cc9-sweep2b: paired at fd90db962, BSM04/BSM06 h1_i/h1_j/BSM09 identical

}  // namespace bsp
