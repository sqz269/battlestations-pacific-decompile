#pragma once

// Packet cc13-gunB (gun damage, packet B of the rev8 scope). Header-only switch.

namespace bsp {

// kGunDestroyedFailureCc13Bound (OFF). Every gun is a damageable device: 0087BCC0
// seeds +368h Armour (class +4Ch), +36Ch and +370h HP (class +48h) from its
// deviceclasses.lua row; 0087CA80 reads "HP" to +48h (0087CC98, default 100.0) and
// "Armour" to +4Ch (0087CCCE). At zero health the image runs 0043F7D0 (+370h <= 0)
// -> vtable[70h](1) 0077D1A0 -> 00926C80 (+60h, +70h; queued on 00F899A8) ->
// FlushPending 009273A0 -> vtable[74h] 00926390 (+5Dh = 1, +60h = 1, 00925C90) ->
// the gun's vtable[7Ch] 00729FA0 -> 00440F50 builds the "destroyed" failure ->
// 00729FD0 -> message 68h -> 0072B5A0, which sets byte +3B8h = 1 and calls
// vtable[1B4h](level) -> 00877B00 -> 00876EC0 (+358h), level = the model's damage
// rows ([[gun+360h]+170h] - [+16Ch]) >> 4, minus 1, or 0. No random draw on the
// chain. The gun is not deleted; it stays as a wreck. 0072B5A0 and 00729FA0 then
// re-run 00956C20 for the owner, which drops the gun from the category lists
// (00956C20's own +5Dh test, and 00729F10's +3B8h / +5Dh terms). Repair's clear
// (007280A0 -> 00925A00) has no host caller.
//
// GameGunneryHost::apply_gun_damage_00728080 models 00728080 (refused once +3B8h is
// set), 00442AE0 and 0087D730 (the difficulty multiplies, through the host's
// difficulty_scaled_damage), 00879070 / 00877B90 on +370h, and the chain above.
// LABELLED SUBSTITUTIONS: level = 0, since the host loads no model damage rows, so
// +358h stays 0 and +3B8h carries every gate; device+54h (0087D730's party test) is
// the owning unit's side; +150h (the invincibility floor) is 0; 0072B4C0 (stop the
// firing effects) and 00925C90 are not modelled; the pending-destroy flush runs at
// once rather than at the queue's drain later in the frame.
//
// Readers wired under ON (game_hosts_gunnery.cpp, game_hosts_ship_ai.cpp):
//   CanFire 00729A80 / 0085A830 gate.disabled (+3B8h) and gate.damage_counter (+358h),
//     in the aim-and-fire pass and torpedo_tube_can_fire_1d0;
//   Gun_Fire 00730160's +5Dh gate (0073018E) and +358h gate (007301A7), which also
//     stop the direct fire paths (NavigatorForceTorpedo 008A7200);
//   the fixed step's re-arm freeze 0072D1C1..0072D1D9 (+3B8h, +5Dh) and the trigger
//     latch 0072D2EC (+5Dh);
//   00956C20's own [dev+5Dh] test in the rebuild;
//   RateExpectedWeaponDamage 0095EC8B (+3B8h).
// IsOperational 00729F10's +3B8h and +5Dh terms are a follow-up inside packet A's
// helper (gun_is_operational_00729f10_cc13), which routes the rebuild, the seat
// filters, ship AI firepower 0095EC52 and the submarine tube walk; until then those
// sites see the gun as operational, except through the rebuild's +5Dh test.
// Not wired: Live*Barrels 0081259F / 008126CF / 0081277F feed only
// RefreshFirepowerRatios 008127E0, whose callers (Lua GetFirepower 0088DC10, bind
// 0081F980) have no host reader.
// Movers: none OFF. ON alone: none either, since nothing damages a gun until packet C
// (device hits) calls apply_gun_damage_00728080. Inert alone; pairs with C.
inline constexpr bool kGunDestroyedFailureCc13Bound = false;

}  // namespace bsp
