#pragma once

// Packet cc13-ammort: 007EDAD0 BSP_PlaneSquadron_AmmoType's per-rack round test.

namespace bsp {

// kSquadronAmmoTypeRackRoundCc13Bound (OFF): squadron_ammo_type_007edad0
// (game_hosts_units.cpp) asks plane_rack_holds_round for each rack instead of
// rack_holds_round.
//
// Why: 007EDAD0's predicates (007B93F0, 007B94F0, 007B9400, 007B9500, 007B93E0,
// 007B9320) each run 007B91C0's walk: the plane's +994h rack slots at +974h,
// slot->vtable[220h](loadout 1), then the descriptor's vtable[8] kind. 007B9140
// (plane_holds_rack_round_007b9140) reads the same rack objects through the same
// slots, and the host already models those with plane_rack_holds_round. Its two
// corrections over rack_holds_round therefore apply here too:
//   - Constructor round (kRackConstructorRoundBound): 006E3350 skips the loadout
//     Ammo when +C54h <= 0 (006E3386 JLE 006E33E3) and tail-jumps vtable[224h]
//     006E3E70, which attaches one round from the constructor's ammo +484h = 1
//     (006E3C20..006E3C37). So a bag-0 single rack with no authored Ammo holds a
//     round in the image. Image fact.
//   - Unseeded Equipments[1] fallback: a scene squadron with no authored
//     Equipment flies Equipments[1] (0094BD34), so before the release issue
//     seeds the host count, a rack the DefaultEquipment leaves empty takes that
//     loadout's Ammo. LABELLED: this is a host modelling choice (the host keeps
//     authored Ammo in place of the image's per-rack +514h/+484h state), not a
//     read of the image.
// Observed difference: JM08 type 152 bag 0 and ESMP08 type 150 bag 0 answer 0
// with rack_holds_round, 1 in the image.
//
// Readers of 007EDAD0 (byte scan: six rel32 calls):
//   00692ED5 / 00692EEF  award/hint pass class 7, via the hint seam
//                        controlled_payload_007edad0 (game_hosts_mission_frame.cpp),
//                        which returns constant 0; does not move
//   007EF1F4             Lua GetProperty "ammoType" (game_hosts_lua.cpp,
//                        kGetPropertySquadronAmmoTypeBound)
//   00984BF8             drop warning channel (cc13_bombdrop.cpp; runs after the
//                        seed)
//   009FFED3             RTB exclusion, heads of class 10h/11h/12h
//                        (game_hosts_ai.cpp, kAiSquadronRtbExclusionBound)
//   00A2456C             StrategicGain; not routed to this reader in the host
// Not readers: free attack 009F8470 and the chooser 007EEC50 / 007EE8F0 test
// each plane with 007ED7E0 -> 007B9320 instead, and the chooser fills its
// general-bomb count from unit_ordnance (game_hosts_script_orders.cpp).
// Expected movers when ON: Lua ammoType and the RTB exclusion for bag-0 bomber
// squadrons with no authored rack Ammo (JM08, ESMP08). The census line in
// squadron_ordnance_census is unchanged.
// ON (cc9-sweep3c, 2026-10-28) by pairs on becab26e5, ESMP08 esmp08_p12_e8_rb and
// JM08 s63_j8_v3, 20000 frames each: gameplay identical; the GetProperty ammoType
// (ESMP08 asked=859 zero=8) and the rtb exclusion (calls=0) lines are equal.
inline constexpr bool kSquadronAmmoTypeRackRoundCc13Bound = true;

}  // namespace bsp
