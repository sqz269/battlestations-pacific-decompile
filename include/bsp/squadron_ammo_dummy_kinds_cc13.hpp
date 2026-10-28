#pragma once

// Packet cc13-ammodk: rack rounds of the DummyTarget, DummySubmarine and WaterMine bullet types.
// Header-only switch.

namespace bsp {

// kSquadronAmmoDummyKindsCc13Bound (OFF).
//
// Image: 007B91C0 / 007B9320 ask each rack slot vt[220h] (006E4060 single rack). 006E4060
// takes the first child entity answering vt[5Ch](2Ah) and returns its +314h bullet
// descriptor; the caller then asks descriptor vt[8](kind). The entity test is satisfied by
// every rack round. Entity vt[5Ch], read from the PE (each also answers 2, 1, 0 and
// [ECX+C4h]):
//   MBomb               00CF9438 -> 006E2790: 2A
//   MTorpedo            00D0C3E8 -> 00856260: 2B 2A
//   MDepthCharge        00CFBA80 -> 006FCA80: 2C 2A
//   MDummyTarget        00CFD018 -> 00700AC0: 2E 2D 2A
//   MDummyKamikazePlane 00CFC698 -> 006FEA30: 2F 2D 2A
//   MDummySubmarine     00CFCB48 -> 006FF8E0: 30 2D 2A
//   MParatrooper        00D05060 -> 007ABA50: 31 2A
//   MRocket             00D090E8 -> 0080ACB0: 33 2A
//   MWaterMine          00D0D130 -> 0085EB20: 34 2A
// (006FF100, the DummyKamikazePlane descriptor's slot 24h, makes the round entity and stores
// +314h = the descriptor; the other descriptors follow the same pattern.)
// The answer is the descriptor's own hand-written test (docs/ORDNANCE_KIND_IDENTITY.md,
// "The complete census"). 2Ah is answered by Bomb, Torpedo, DepthCharge, DummyTarget,
// DummyKamikazePlane, DummySubmarine, Paratrooper, Rocket and WaterMine; 2Dh only by
// DummyKamikazePlane (006EA830). The DummyTarget (006EA6F0) and DummySubmarine (006EA8B0)
// tests omit 2Dh.
//
// Host (OFF): read_device_bullet_class_id has no id for the Types "DummyTarget",
// "DummySubmarine" and "WaterMine" (0, so every rack walk skips the rack), and the
// squadron ammo-type ancestry answers 2Ah only for 2A/2B/2C/31/33.
// ON: those Types read as 2Eh / 30h / 34h, and ancestry 2Ah follows the descriptor census
// above; 2Dh stays 2F only. 007B9320 then answers 1 for a 2E/30/34 rack, as the image does.
//
// Reach. In this installation the only plane racks with these Types are device 83
// "gunnerytarget" (bullet 142, DummyTarget, Ammo 5) on VehicleClass 110 (C47 Skytrain,
// unitclass_testbomber) and 649 (unitclass_l2d). DummySubmarine (devices 144/500) and
// WaterMine (154) are not plane platforms.
// CORRECTION (rev6): both classes ARE spawned. global.enums maps TesztBomber = 110 and
// TesztBomberJP = 649, and six live training scenes place a TesztBomber[JP]
// PlaneSquadronWNavpoint (SubType Divebomber, Equipment 1):
//   - ijn_airtraining.scn:4987 (TRN1), ijn_seatraining.scn:6420 (TRN2),
//     ijn_seatraining3.scn:6420 (TRN5);
//   - usn_airtraining.scn:4734 (TRN3), usn_seatraining.scn:6302 (TRN4),
//     usn_seatraining2.scn:6302 (TRN6).
// (ijn_seatraining2's row is commented out at missiontree.lua:278-309.) The first check
// grepped numeric ids only; scene files name classes by enum symbol.
//
// Movers when ON, in those training rows (every reader of the rack class id):
//   - 007EDAD0 squadron ammo type 0 -> 1: the 009FFED3 rtb gate, the Lua ammoType
//     (game_hosts_lua.cpp, GetProperty), cc13_seg62_seg65.cpp's reader;
//   - 007B9140 holds-a-round: the dogfight drop gate (df_can_drop_007b9140), the
//     script-orders and Lua readers;
//   - 007B92E0 any-rack-empty (script orders);
//   - the live-ordnance mask in game_hosts_units.cpp. OFF: class 0 leaves known = false and the
//     mask untouched. ON: 2Eh keeps it true, so an emptied rack clears its bit.
//     CORRECTION: ON also adds 2Eh to that block's bomb_family, so a loaded 2Eh rack keeps
//     the 2Ah bit, as the image counts it. The reader is 007B9140's rack walk (007B917B
//     PUSH 2Ah): rack vt[210h] = 006E3FE0 asks each child round entity vt[5Ch](2Ah)
//     (006E3FF2), and DummyTarget's entity test 00700AC0 answers 2Eh/2Dh/2Ah
//     (00700AC4..00700AD1). 30h/34h also answer 2Ah but are on no plane rack, so they are
//     not added. CORRECTION (rev6 NIT1): this named 007B9320 (descriptor vt[8](2Ah) at
//     007B935C, DummyTarget's 006EA6F0); that reader rejects 2Bh/2Ch/31h/33h/2Dh
//     (007B936B..007B93A7), which bomb_family counts, so it cannot be the bit's reader;
//   - DeviceWalkRack::bullet_class (the cc13_devwalk rack walk) gets 2Eh instead of 0;
//   - the single-drop test (007C0E30 arm): 2Eh is none of 2Bh/2Ch/33h, so it stays on the
//     bomb arm, as the image's DummyTarget descriptor does.
// ON (cc9-sweep3c, 2026-10-28) without a pair: reached only in TRN1-TRN6, which the
// harness cannot launch (`--menu-select TRN1`: group 0, detail=0, step=Stopped, units=0),
// and no WINS row reaches it. Compiles ON (off_switch_on_build).
inline constexpr bool kSquadronAmmoDummyKindsCc13Bound = true;

}  // namespace bsp
