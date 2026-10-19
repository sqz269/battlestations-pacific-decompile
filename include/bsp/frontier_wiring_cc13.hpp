#pragma once

// Packet cc13-frontier (docs/CC13_FRONTIER_MERGE.md, build list item 1).

namespace bsp {

// 0080E290 BSP_Unit_AimTurningGunsToRest, __fastcall(ECX = unit), body 0080E290-0080E2B6.
// It walks the unit's device chain (unit+48h, next +44h) and, on each device that
// answers IsKindOf(22h) (vtable +5Ch), runs 0085AD00: when the platform row's
// RestAngles horizontal (+94h) is not FLT_MAX, 0085ABA0(+94h, +90h) commands the gun to
// its rest angles. There are two callers:
//  - 00780120's 4Bh arm. The player's helm role is released (+184h = 0 at 00780235, the IsKindOf(6)
//    test at 0078023C, the 0080E290 call at 00780244): game_hosts_units.cpp role_message_4b_00780162.
//  - 00645600 SetControlledUnit. The outgoing kind-6 unit (0064562D): game_hosts_hud.cpp
//    SelectControlledUnitBinding::release_unit_nodes.
// ON: both sites run GameGunneryHost::aim_unit_turning_guns_to_rest_0080e290 on that
// unit's guns. OFF: both sites only record the call, as before.
inline constexpr bool kHelmReleaseGunsToRestCc13Bound = false;

}  // namespace bsp
