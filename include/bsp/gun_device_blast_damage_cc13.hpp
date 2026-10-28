#pragma once

// cc13-gunC C2: a burst's radial damage on the gun mounts' own device meshes.
//
// Image. 0084BAD0 (Projectile::blast_radial_damage) gathers through 00904470 with
// exclude = source->vtable[B0h](), the firing unit's collision node, and 0098C630's grid
// sphere query calls 0098C510 for each node it meets:
//  * 0098C535: node == exclude returns before anything else, so neither the firing unit
//    nor its children (its own mounts) are tested;
//  * otherwise 0098C510 resets the record (00470470: +30h/+34h = -1), sets +0h =
//    node+4Ch (the node's entity), runs vtable[0Ch] = 0070F720 (-> 00723F80 into node
//    space -> 00723B70 per element: root-box reject, closest triangle 0085DF60, an entry
//    when it is strictly inside the radius), appends the record (0098C460) when it has
//    entries, and then ALWAYS recurses over the children at node+FCh/+100h.
//  * A device node is only ever a child (00710AD0 -> 0070F7D0 for an owner answering
//    5Ch(1Eh) -> 0098BA10 -> 0098B920; never in a grid cell; see
//    gun_device_hit_damage_cc13.hpp), so a mount is met only through its parent's
//    recursion, after the parent's own record, and whether or not the parent had one.
//  * 0084BBF9 pushes each record (00926E80) after 00470350 SetShot, so each device
//    record makes its own shot->vtable[54h] (006E7C60) draw at the push, like a hull
//    record. The record keeps +34h = -1 (00470470; 0084BAD0 never writes +34h).
//  * 009239A0 hands a device record to 0043F7F0: a depth charge (shot 5Ch(2Ch)) whose
//    parent is not 5Ch(8) returns AL = 0 (0043F822) and the parent's own vtable[ECh]
//    takes it: on a ship, 00826F10's 2Ch gate at 00826F44 refuses a non-submarine, so
//    nothing moves. Otherwise there is no relay (+34h == -1, 0043F827..0043F841) and
//    008777D0 runs on the device: no hull pass (+34h == -1), then the part pass
//    008778E6..00877A37 over the entries with index != -1 (armour = kind 4 ?
//    [dev+354h]->vtable[24h]() : dev+368h, times ProductForUnit(2, [dev+3F0h]) for an
//    IsKindOf(20h) device; 004705C0's 1 - d / r falloff; the strict maximum; > 0 ->
//    vtable[1ACh] = 00728080).
//
// Host (ON): apply_impact_blast gathers, for every unit it walks (after the shooter,
// dead and detached skips, before the hull's own range and entry tests), the unit's
// device_children_of rows: the burst centre into the device's node-0 frame
// (device_hit_world_frame), a sphere prune on the mesh radius, then the closest
// triangle per run of triangles with the same (kind, index) with the 00723B70 rule.
// Each mount with entries gets a record (hull_segment -1, +28h the burst damage, +24h
// the radius, +2Ch and the owner modifier as the hull record's) pushed through
// apply_hit after the unit's own record, which queues it (and makes its draw) as any
// blast record. The dispatch runs device_hit_split_0043f7f0 and then packet B's
// apply_gun_hit_record_008777d0 (whose part pass is 008778E6..00877A37, power-up
// product included); a drop is counted only.
// LABELLED:
//  * the per-element grouping is by runs of equal (kind, index) in the mesh's element
//    order; two adjacent elements with the same pair become one entry with the nearer
//    distance, which 008777D0's maximum over entries cannot tell apart;
//  * no per-element root-box reject: result-exact, since any triangle within the radius
//    puts the element's box within it too, so the reject only skips elements with no
//    entry;
//  * the device draw is Draw::hull_damage keyed like the parent's (gun_row, unit);
//  * (A) (cgbuildC, verdict PARTIAL) mount children are walked in ascending platform
//    key, the order 0095A880 creates them over the key-indexed [class+94h] (ships:
//    00810F60 at 0081108D) and 0098B920 appends them; exact for ships until 00819CC0
//    (damaged-gfx level > 0) detaches the DamagedGFXRemove.Slots mounts (00710B80 ->
//    0098A2C0 swap-remove). That removal, and the reorder it causes, are not modelled.
//    Planes, forts and airfields are not checked. The host walks device_children_of's
//    gun rows. Each mount record makes a stream-1 draw (00470350 -> 006E7C60 at the
//    0084BBB1 push), so an order mismatch reorders the draws among one unit's mounts;
//  * a drop does not run the parent's 00826F10 (its 2Ch gate refuses a non-submarine
//    ship; a plane or land parent's own handler is not traced), and the 0077CE60
//    attribution and 009239A0's IsKindOf(44h) notify are not modelled for device
//    records;
//  * the grid-cell condition on the parent is approximated by the host's walk (every
//    live, attached unit other than the shooter); a dead parent hides its mounts;
//  * the other 0084BAD0 callers the host routes elsewhere (apply_gunless_blast_hit:
//    plane crash, kamikaze and other shot-0 bursts) do not gather mounts. The depth
//    charge, torpedo, impact and flak bursts all go through apply_impact_blast.
//  * the pose gaps are C1's (gun_device_hit_damage_cc13.hpp).
// Needs kGunDeviceHitDamageCc13Bound's mesh and pose helpers and, for the gun to take the
// damage, kGunDestroyedFailureCc13Bound. Movers when ON: guns near a burst lose health
// and can be destroyed (fire control, AA and gun counts), one extra hull_damage draw per
// mount record, and the deferred queue's order and length. A destroyed mount is still
// gathered and still draws while 00728080 refuses its damage (+3B8h), as in the image.
// With the switch OFF the host's log is unchanged (the summary line is ON-only).

namespace bsp {

inline constexpr bool kGunDeviceBlastDamageCc13Bound = true;   // ON: SHIP_AI 330.2

}  // namespace bsp
