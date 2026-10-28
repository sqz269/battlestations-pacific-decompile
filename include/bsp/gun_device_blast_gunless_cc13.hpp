#pragma once

#include "bsp/gun_device_blast_damage_cc13.hpp"

// cc13-c2b (C2b): the gunless bursts' radial damage on the gun mounts' device meshes.
//
// Image. Every 0084BAD0 caller gathers the same way (00904470 -> 0098C630 -> 0098C510;
// no kind filter; exclude = source->vtable[B0h]() or 0 for a null source; 0098C510 recurses
// into the children after the node's own record whatever its test gave), so a gunless burst
// reaches the mounts exactly as C2's do (gun_device_blast_damage_cc13.hpp). What differs is
// the arguments (read from the listing):
//  * 00819A20, the kamikaze unit's blast arm (00819BFB..00819C14): damage &desc+514h
//    (KamikazeBlastDamage), radius &desc+518h (KamikazeBlastRange), ignore 0, source 0,
//    shot 0. With no source there is no exclude: the kamikaze's own node and its children
//    are walked too. With shot 0 neither SetShot (009044F4, 0084BBB1) calls vtable[54h]:
//    record+4h = 0, +14h stays 0 (00470470), no draw.
//  * 0084BC60 for a plane's crash shot (007C0910 passes unit+72Ch, so record+4h = the
//    plane) (0084BE25..0084BEE3): damage U(BlastDamageMin, BlastDamageMax) drawn once per
//    burst (0084BE4D), radius class+70h, ignore = (class+8 == 10h; the Kamikaze sub-type is
//    11h, so 0), source [buffer+4] = the plane, shot = the plane. Every plane vtable's
//    slot 54h is 0042B8D0 (FLDZ; 00D06638, 00D1A000, 00D19D28, 00D06920, 00D00070,
//    00D0BA80, 00D00308, 00D1A2D8), so +14h = 0 and no draw. The plane's node and its
//    children are excluded.
//  * 009239A0 -> 0043F7F0 on a device: [shot+0] is null (kamikaze) or a plane, neither
//    answers 5Ch(2Ch), so there is never a drop; +34h = -1, so no relay; 008777D0 runs on
//    the device (packet B's part pass, as C2).
//
// Host (ON): kamikaze_detonate_00819a20's blast loop (self included) and
// plane_crash_impact_0084bc60's (the plane skipped) gather each walked unit's mounts with
// C2's gather_device_blast_mounts, before the unit's own range and entry tests, and push a
// record per mount with entries after the unit's own record: hull_segment -1, +28h the
// burst damage, +24h the radius, ignore false, hull base 0, weapon scale 1. ON with
// kHitDeferredDispatchCc13Bound the record is queued as a gunless node and the replay runs
// device_blast_dispatch_gunless (no 2Ch drop, no relay, then 008777D0); OFF it is
// dispatched at once. No draw is made for these records.
// LABELLED: C2's labels apply (element grouping, no per-element root-box reject, a dead
// parent hides its mounts, no 0077CE60 attribution or IsKindOf(44h) notify); the host's
// walk stands for the grid-cell condition on the parent, as in C2.
// Needs kGunDeviceBlastDamageCc13Bound (the gather and the device dispatch). Movers when
// ON: guns near a kamikaze or plane-crash burst lose health and can be destroyed (carrier
// AA under kamikaze plane impacts), and the deferred queue's order and length; no draws.

namespace bsp {

inline constexpr bool kGunDeviceBlastGunlessCc13Bound = false;
static_assert(!kGunDeviceBlastGunlessCc13Bound || kGunDeviceBlastDamageCc13Bound,
              "kGunDeviceBlastGunlessCc13Bound needs kGunDeviceBlastDamageCc13Bound");

}  // namespace bsp
