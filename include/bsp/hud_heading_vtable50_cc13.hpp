#pragma once

// The minimap icon rotation's unit heading as the image reads it (packet cc13-hudhdg).
// Header-only switch.

namespace bsp {

// kHudHeadingVtable50Cc13Bound (OFF): 005C1CA1..005C1CAF calls the icon unit's vtable[50h]
// and adds the result to the icon heading (005C1CB1 FADD, 005C1CBC FSUBR pi/2). OFF, the
// host (GameHudMinimapHost::update_005c0f20) feeds row->heading_degrees * pi / 180, a
// recomputed atan2(row2.x, row2.z) for every class. ON feeds
// GameUnitsHost::unit_heading_vtable50, the slot body each minimap-shown class installs
// (read at vtable+50h for every vtable whose +B8h is 006D1FA0 or 0074DDF0):
//   06h..0Eh ships, submarine, torpedo boat (00D09678, 00CFC3D0, 00D0BF80, 00D01630,
//     00CFB738, 00CFA778, 00CFFA30, 00CF90B0, 00D0C648): 006DFD60 FLD [unit+1050h], the
//     host's hull_heading_1050 (00826C56, the motion tail)
//   0Fh..17h planes (00D05F20, 00D06638, 00D1A000, 00D19D28, 00D06920, 00D00070,
//     00D0BA80, 00D00308, 00D1A2D8): 0074E260 FLD [unit+0C6Ch], the host's
//     plane_heading_c6c (007C1900, kept when the nose is near vertical)
//   45h MAirfield (00CF8C08): 006D2040 atan2(+94h, +9Ch), the local frame's row 2
//   05h (00D1A698), 19h MLandVehicle (00CFFDE0), 46h MShipyard (00D0B770): 0042B8C0
//     FLDZ, so 0
//   1Ah LandConvoy (00CEA570, its +B8h the conditional 004F25B0): 0042B8C0, so 0 (the
//     accessor's default arm)
// LABELLED: the image also seeds +1050h at spawn and relocation (00818EA0
// ResetPoseDependentState, 0081F980). The host's hull_heading_1050 has no writer before the
// first motion tick, so with ON a ship icon drawn before its first motion step shows 0, for
// at most one frame.
// Movers when ON: minimap icon rotation only (render): shipyards and land vehicles stop
// turning with their pose, planes follow +C6Ch, ships and airfields change by rounding.
inline constexpr bool kHudHeadingVtable50Cc13Bound = false;

}  // namespace bsp
