#pragma once

#include "bsp/gun_destroyed_failure_cc13.hpp"
#include "bsp/gun_device_hit_damage_cc13.hpp"

// Packet cc13-gunD (gun damage, packet D of the rev8 scope). Header-only switch.

namespace bsp {

// kGunSubobjectRepairCc13Bound (OFF): gun health comes back, and a destroyed gun is
// revived at full health. Two image callers heal a unit's guns (the unit+48h children,
// next +44h; LABELLED: the host walks the unit's gun rows in build order):
//   0093C860, step 2 of the ship repair tick 0093CA20 (008160B0, slot 1ECh), every tick
//     between the hull step 0093C770 and the failure list 0093C520. A child is kept when
//     IsKindOf(4), not IsKindOf(0Fh) and byte +378h is set: every gun class passes, and
//     +378h is 1 from 0087BD19 (LABELLED: Lua RepairEnable 008AD4E8 on a gun has no
//     host route, so it stays 1). The heal is ((settings+3B8h GunRepairTickPercentage
//     (/ 100) * dt) * gun+36Ch) * rate (0093C938..0093C94C), rate = the 0093C8C5
//     modifier product (3) times 1.0, or settings+3D8h GunRepairMultiplier when task+24h
//     is 2 (the host's task priority is the constructor's 0), into 00879810. Then, when
//     +36Ch <= +370h, 00877B90(max) (0093C99A).
//   006F47F0 CommandBuilding_Repair's second loop (006F48BC..006F49E3), after the hull
//     arm and whatever it did: f2 = Repair[level] * a / [004C1D10()+40h] (GunRepairTime,
//     20 in this installation's commandbuildingglobals.lua, 2024-07-13), taken when f2
//     > 0; every IsKindOf(4) child (no 0Fh or +378h test) gets min(max, max * f2 +
//     health) through 00877B90, then the same full-health test.
// Both then send a gun with +5Dh set down vtable[19Ch]("destroyed", 0) (0093C9CC /
// 006F49B3) = 0072A0D0 -> message 69h (0077C2A0, local receive queue) -> 0072D830 ->
// slot 1CCh 007280A0: +3B8h = 0 (007280C0), vtable[1B4h](0) = 00877B00 (+358h = 0),
// 008781F0(1) (scene nodes visible, presentation, not modelled), 00925A00, then
// 00956C20 on [gun+3F0h] (007280DF). 00925A00 runs only when the parent [gun+3Ch] has
// +5Ch set and +5Dh/+60h/+5Eh clear (LABELLED: the owner's UnitState is not dead, which
// misses an inactive owner and a live owner with +60h or +5Eh set); it clears +5Fh
// (dropping the F899B4 entry), +5Eh, +60h (dropping the F899A8 entry) and +5Dh. The
// rebuild runs whatever the gate did. LABELLED: the 69h chain applies inline, an early
// flush, as packet B's 68h chain does; +5Eh and +5Fh have no gun fields (B's destroy
// path never sets them); 00876EC0's presentation calls (00710BB0, 0049C940) and
// 008781F0 are not modelled. No random draw anywhere on either path.
// Requires packet B's gun fields and destroy chain. Movers: none until packet C1's
// device hits take gun health down; then ship guns heal at GunRepairTickPercentage per
// second (2 here, x GunRepairMultiplier only at priority 2) and garrison and fort guns
// at Repair[level] / GunRepairTime per second of the capture step (a flip adds one
// Repair[level] / GunRepairTime step), and destroyed guns return to service at full
// health.
inline constexpr bool kGunSubobjectRepairCc13Bound = true;   // ON: SHIP_AI 330.2

static_assert(!kGunSubobjectRepairCc13Bound
                  || (kGunDestroyedFailureCc13Bound && kGunDeviceHitDamageCc13Bound),
              "packet D repairs the gun state packet B models, damaged by packet C1");

// 006F47F0's gun repair time ([004C1D10()+40h], GunRepairTime). LABELLED: this
// installation's commandbuildingglobals.lua (2024-07-13), as the host's other
// CommandBuildingGlobals constants are.
inline constexpr float kCommandBuildingGunRepairTime = 20.0f;

}  // namespace bsp
