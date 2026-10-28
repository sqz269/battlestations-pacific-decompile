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
// Movers (mechanism only): every reader of the rack class id for a rack firing those Types:
// 007EDAD0 squadron ammo type (1 instead of 0), 007B9140 holds-a-round (also the dogfight
// drop gate), 007B92E0 any-rack-empty. In this installation the only plane racks with them
// are device 83 "gunnerytarget" (bullet 142, DummyTarget, Ammo 5) on VehicleClass 110 (C47
// Skytrain, unitclass_testbomber) and 649 (unitclass_l2d). No shipped mission spawns
// either class (no PrepareClass or scene Type; mission C-47s are class 136). DummySubmarine
// (devices 144/500) and WaterMine (154) are not plane platforms. So there is no live mover.
inline constexpr bool kSquadronAmmoDummyKindsCc13Bound = false;

}  // namespace bsp
