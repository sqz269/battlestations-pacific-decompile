#pragma once

#include <cstddef>

// Packet cc13-gunA (gun damage packet A).

namespace bsp {

// kGunOwnerInferiorOperationalCc13Bound (OFF): the +720h term of Gun_IsOperational.
// 00729F10 (00729F10..00729F36, __fastcall gun, AL) = ![[gun+3F0h]+720h] && !gun+3B8h
// && !gun+5Dh. gun+3F0h is the owning unit: 0072D7E0 (the device init) stores the
// gun's attach parent [gun+3Ch] there at 0072D80F/0072D81A, which is the host's
// GameGunRow::unit_index. unit+720h is the owner's InferiorFailure latch: 0095AD0A sets
// it on message 7Dh (00958A30's first break at health zero, and the shipyard's
// hangars-lost rule); 0095AD7F clears it on 7Eh (00958D43, health restored). The
// +728h window does not gate it: 00729F10 reads +720h alone, so a gun stays not
// operational from the first break until the restore. The host carries the latch
// (GameUnitsHost::inferior_failure_720, under kInferiorFailureArmBound and
// kShipyardDestructionRuleBound, both ON).
// Hosted readers, all routed through gun_is_operational_00729f10_cc13:
//   00956C20 (00956C98) RebuildWeaponCategoryIndex, the gunnery rebuild binding. Its
//            callers are 0095E5B0 (Lua read), 00729FA0 (OnActivated) and the +3B8h
//            set/clear 0072B5A0/007280A0; 0095AD0A does not rebuild, so the latch only
//            reaches the lists at a rebuild, which the host runs at unit build.
//   00954210 (0095421F) the player gun seat aim accept, kinds 1/2 and 3.
//   0095EB40 (0095EC52) RateExpectedWeaponDamage, ship AI firepower; 0080DF40
//            (0080DF59) ReadyTorpedoBarrels goes through the same binding.
//   009E9910/009E9A50 and 009E99A0/009E9AE0, the sub's ready-tube counts and least
//            tube timers.
// Not hosted: the HUD sight status 00547480/00547A20 and 00812B00 IsCommandAvailable.
// OFF: every gun operational. ON: not operational while its owner's +720h is set.
// +3B8h and +5Dh stay clear (LABELLED: no producer for a gun in this host; the fire
// gate answers them false too). CanFire (00729A80) does not read +720h; the fire gate's
// own +720h test (0072D1C1, unit_fire_blocked) is unchanged.
inline constexpr bool kGunOwnerInferiorOperationalCc13Bound = false;

// 00729F10 for a gun whose [gun+3F0h] is `owner`. Called only under the switch.
template <class Units>
bool gun_is_operational_00729f10_cc13(const Units& units, std::size_t owner) {
    return !units.inferior_failure_720(owner);
}

}  // namespace bsp
