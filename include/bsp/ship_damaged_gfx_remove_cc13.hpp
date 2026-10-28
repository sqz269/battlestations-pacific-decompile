#pragma once

#include "bsp/gun_destroyed_failure_cc13.hpp"
#include "bsp/gun_device_hit_damage_cc13.hpp"

// cc13-gunC C1b: a ship's damaged-gfx level and its DamagedGFXRemove mount detach.
//
// Image.
//  * The only single-player raiser is the Lua native SetDamagedGFXLevel 0088F710:
//    00888AA0(arg 1) is the entity (no null test), 00B66290(arg 2) the level, then the
//    entity's vtable[1B4h](level) at 0088F849..0088F851. Shipped scripts make 30 such
//    calls, all with level 1 (USN01, BSM01, JM01, JM02, JM09).
//  * Ships' vtable[1B4h] is 00819CC0 (slot 109 of the ship vtables, 00CF90B0 and
//    others). 00819CE0..00819CF0: equal to +358h, nothing. Else 00877B00(level) (the
//    base: +358h and the model variant), and with level > 0 (00819CFC JLE):
//      for each int of class+7A8h..+7ACh in order (00819D10..00819DC8), walk the ship's
//      entity child chain (head +48h, next +44h; 00924F90 appends, so creation order)
//      and, for EVERY child answering vtable[5Ch](1Eh) with +38Ch equal to the key
//      (0095A880 stores the platform key there, 0095A92C), with no dead test:
//        vtable[70h](1) = 0077D1A0 -> 00926C80 (00819D7C), the entity destroy;
//        00B6DA70(node, 0.0, 0), the visibility factor (00819D98..00819DA9);
//        00710B80 (00819DB9) -> 0098A500 -> 0098A2C0, out of the parent's spatial child
//        array by swap-remove (the last child moves into the hole). A part with +184h
//        already 0 is inert, so a second pass detaches nothing.
//      then for each int of class+7B8h..+7BCh (Funnels.Slots) below ship+A18h, the
//      funnel effect at ship+A14h[i]: 00867B10, +9 = 1, released, nulled.
//  * class+7A4h is the DamagedGFXRemove.Slots vector: 00831840 reads the table
//    "DamagedGFXRemove" (00D09A80, 00833D1B), its "Slots" (00D09A78, 00833D66) and
//    pushes each value of a next() walk (00B67080 / 00B67190 / 00B66420) through
//    00B66290 at 00833E07..00833E63; begin +7A8h, end +7ACh. "Funnels" (00D09A70,
//    00833E9A) -> "Slots" fills class+7B8h. 40 of the 129 vehicle classes of this
//    installation list slots (e.g. Mahan 1941 {20,21,50}); the Pearl Harbor battleship
//    classes list none.
//  * The gun's destroy is packet B's chain: 00926C80 (+60h once, cause +70h, queued on
//    00F899A8) -> 009273A0 -> 00926390 (+5Dh) -> 00729FA0 -> 00440F50, which with +70h
//    == 1 calls vtable[194h]("destroyed") = 00729FD0 (00440F8A..00440FA1) -> message 68h
//    -> 0072B5A0 (+3B8h, vtable[1B4h], 00956C20).
//
// Host (ON). GameMissionLuaHost dispatches 0088F710 to
// GameGunneryHost::set_damaged_gfx_level_00819cc0 for an entity with a units-host slot.
// The gunnery host keeps a per-unit level (0 at spawn) with the equal-level early-out;
// for a ship-kind unit (5Ch(6)) with level > 0 it walks the flattened Slots keys and, in
// gun build order, every gun row with that platform key: packet B's
// gun_destroyed_failure_0072b5a0(row) (idempotent on +60h, as 00926C80 is), then the
// row leaves the unit's device child list by swap-remove. The removals are logged per
// unit and replayed whenever the list is rebuilt, so the order survives a rebuild.
// The C1 shell trace and the C2 blast gather read that list, so a removed mount takes no
// shell (the round reaches the hull behind it) and no blast record.
//
// LABELLED (not modelled or substituted):
//  * The cause gate: a gun's +70h is 1 unless its parent is already destroyed, when it
//    takes the parent's +70h; a cause other than 1 skips 00729FD0 in the image (no
//    +3B8h, no 0072B5A0). The host runs packet B's chain whatever the owner's cause.
//  * The flush runs at once (packet B's early-flush label); 00925C90's observer notify,
//    the WarningManager killed telemetry (00986480) and the MP 4Eh broadcast of
//    0077D1A0 are not modelled. 0072B4C0 (the firing effects) is not run on this path;
//    +5Dh already stops the gun firing.
//  * The visibility factor and the funnel loop are presentation only; not modelled.
//  * MT_DAMAGEDGFXLEVEL (00878350's arm, vtable[1B4h]([msg+20h])) is MP only.
//  * The image's spatial child array also holds kind-1Eh devices the host has no gun
//    row for (a platform with no gunnery category); they take part in its swap-removes
//    and not in the host's. A gun row added to a unit after a removal is placed as if it
//    had been there before it (no ship gains guns after spawn).
//  * Non-ship units keep the level only (00877B00's +358h); their own vtable[1B4h]
//    overrides are not modelled. Nothing re-attaches a removed mount (level 0 included).
//  * The Slots values are read with pairs() and tonumber in the host's interpreter, the
//    image's next() walk and 00B66290.
//
// Movers when ON: USN01 Cassin (usn_1_pearl.scn `Type = E ShipClasses : Mahan`; Mahan
// 1941 lists slots 20, 21, 50): at luaCassinDie
// (luaDelay 1.5 s after luaBombingMovie) its slot guns are destroyed and detached.
// luaBlowUp only runs ExplodeToParts (0088E1B0 -> 00935C70, the part detach; it does not
// destroy the unit), so the owner is live and the image takes the live arm (00956C20).
// JM02's FakePoW / FakeRepulse cut-scene props and JM09's StageTrg, for whatever slots
// their classes list. The swap-remove only reorders the device trace, which moves a
// result on an exact tie. Pearl Harbor battleships: no slots, no change.

namespace bsp {

inline constexpr bool kShipDamagedGfxRemoveCc13Bound = false;

static_assert(!kShipDamagedGfxRemoveCc13Bound
        || (kGunDestroyedFailureCc13Bound && kGunDeviceHitDamageCc13Bound),
    "kShipDamagedGfxRemoveCc13Bound needs packet B (kGunDestroyedFailureCc13Bound) and C1 "
    "(kGunDeviceHitDamageCc13Bound)");

}  // namespace bsp
