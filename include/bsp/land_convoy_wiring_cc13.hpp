#pragma once
// Packet cc13-convoy: the switch that wires cc15's land convoy bodies
// (include/bsp/cc15_land_convoy.hpp, docs/CC15_LAND_CONVOY.md) and cc13's
// 00742210 / 0074CD10 (include/bsp/cc13_seg41_land.hpp) into the game host.

namespace bsp::game {

// True:
//  - the roster keeps one member vector (+398h, nulls in place) and the live count
//    +3BCh per convoy, shared with the motion state's slots, and each member's
//    +738h back pointer;
//  - 007420B0 runs at bind (members take the convoy's 00927A80 skill through
//    vtable +128h, 006D1EB0);
//  - each 009273A0 flush runs 0074CD10 -> 007422A0 for every member that died
//    since the last flush; 007422A0's 00926D90(convoy, 2) at exactly zero live
//    members queues the convoy, and its 00742180 runs in the same flush, because
//    009273A0 loops until both queues are empty (009275BA JMP 009273C0, the head
//    re-testing 00F899B0 / 00F899BC);
//  - Lua Kill on a convoy runs 00742210 (members killed with cause 2 and nulled,
//    +738h kept, the convoy not killed).
// False: the convoy is a marker with a fixed roster; a member's death touches
// nothing, and Lua Kill on a convoy kills nobody (no units-host slot).
inline constexpr bool kLandConvoyBodiesBound = false;

}  // namespace bsp::game
