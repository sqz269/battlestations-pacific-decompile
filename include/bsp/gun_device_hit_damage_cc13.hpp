#pragma once

// cc13-gunC C1: a shell's direct hit on a gun mount's own device mesh.
//
// Image. Each weapon device builds its own collision at setup: 0072E6D0 -> 0043F7B0
// (BSP_DeviceEntity_InitHealthAndParts) -> 0087BCC0, which allocates the 1ACh part
// instance (00BF681B, 007135C0) when the device class's Mesh (desc+50h) is set.
//  * 007135C0 runs 00712440 (BuildShapes, the GeomMesh of the device .mmod) at 007136F2,
//    then 00710AD0 (AttachSpatialNode) at 00713701.
//  * 00710AD0 picks the parent node with 0070F7D0: only for an owner that answers
//    vtable[5Ch](1Eh), which every gun class does (20h's chain has 1Eh,
//    docs/ENTITY_CLASS_IDS.md), it walks owner+3Ch up to the first ancestor whose
//    vtable[B0h] node (+360h) has +158h set, which is the ship's (or plane's, or land
//    unit's) part node.
//  * 0098BA10 with that parent goes to 0098B920, so the device node is a CHILD in the
//    parent's child array (+FCh/+100h), and 0098B530 grows the parent's local AABB over
//    it. 0098BDB0/0098BC70 refresh a moving root's children every step from the
//    device entity's own world matrix (+CCh), so the mesh follows the device pose.
//  * 0098AC20 walks a root's shapes, then (kind filter 0 only, 0098ACC0) its children,
//    shortening the segment after every accepted hit (0098AC88), so the nearest triangle
//    over hulls and mounts wins; a tie goes to the last acceptor (00723D60's `t <= best`).
//    A root the query excludes (the round's owner, 0098AEE4) is skipped with its children.
//  * The device GeomMesh is all ('body', index 0) in node-0 space for the 379 device
//    models that have one (rev8, local\rev8_gmscan.py; 4 also carry engineroom/magazine/
//    fueltank). A mount with no GeomMesh is unhittable; there is no fallback shape.
//  * The record's entity is the device, so 009239A0 delivers it to the device's
//    vtable[ECh], 0043F7F0 (ECX = device, [ESP+4] = record, RET 4):
//      shot answers 5Ch(2Ch) (MDepthCharge): no parent -> straight to 008777D0;
//                                            parent not 5Ch(8) (MSubmarine) -> RET with
//                                            AL = 0 (0043F822), so 009239A0's loop
//                                            (00923A70) hands the record to the parent's
//                                            own vtable[ECh]; the gun takes nothing;
//      parent answers 5Ch(6) and record+34h != -1: parent->vtable[ECh](record), the SAME
//                                            record (00826F10 on a ship: its hull pass on
//                                            element 0 with the ship's armour, R7b/R7c);
//      always then: 008777D0 on the device, with the device's armour (unit+368h =
//                                            desc+4Ch, 0087BCF4); it returns 1, so the
//                                            loop stops at the device.
//    So a mount hit damages BOTH the ship and the gun; it is not divided between them.
//    009239A0's attribution walks from the device up to its parent and credits it.
//
// Host (ON):
//  * SegmentBinding offers, as children of a unit in every kind-0 query, the guns of
//    that unit whose device class has a GeomMesh and whose mount is known; a child
//    traces the device triangles with the element rule and fills the record with the
//    element's kind and index (9, 0). The parent's broad-phase box grows over them.
//  * The direct-hit dispatch (run_projectiles -> apply_hit, queued under
//    kHitDeferredDispatchCc13Bound like any direct hit) runs device_hit_split_0043f7f0:
//    drop -> the parent's path as before; relay -> the parent's path (00826F10,
//    unchanged) and then the gun's 008777D0; neither -> the gun's 008777D0 only.
//    gun_device_hit_damage_008777d0 is the seam; the gun-health side is packet B's.
//    CORRECTION (follow-up): the seam now calls B's apply_gun_hit_record_008777d0(g, hit)
//    (g = the guns() row) when kGunDestroyedFailureCc13Bound is also ON; with B OFF the
//    mount still stops the shell and relays it, and the gun takes nothing.
//  * LABELLED: with neither, the host returns before its attribution tail, where the
//    image still writes the parent's 0077CE60 block (a plane or land parent).
// LABELLED:
//  * pose (rev8 (d)): node 0's world is [RotY(-horz), and RotX(-vert) when the model has
//    no barrel; T = the slot translation] x the owner's world (00859550 every update
//    with dt > 0 drops the slot frame's 3x3). The host takes it from
//    device_hierarchy_node_world (pick 0), the muzzle path's own pose. Gaps: the
//    parent's pose stands in for [gun+3Ch]->vtable[18h] (hull-oriented, unit scale);
//    0072DD20's kind 46h/44h parent re-expression, recoil and the slot +94h override
//    are not applied; before the first update tick the image's root still carries the
//    full slot frame. have_barrel is "barrel" Note only where the image's +3C8h also
//    needs "base"; rev8's census has no barrel-only device model, so they agree.
//  * a gun whose mount the host does not know (no platform attachment) is not offered;
//    a child is offered only while its parent offers its own shape (dead, detached and
//    excluded parents hide their mounts; the image's 008263E2 / sinking teardown path
//    for children is not traced).
//  * a destroyed gun keeps its mesh, FAITHFUL as far as traced: the destroy chain
//    0072B5A0 calls the gun's own vtable[1B4h] (0072B604), which on the gun vtables is
//    00877B00 (MRTGun 00CFE548 + 1B4h = 00877B00; no spatial call), not the ship's
//    00819CC0 that detaches (00710B80); 0072B4C0 is effects and 00956C20 the category
//    index; neither is one of 0098A500's five callers (deeper callees not swept).
//    009239A0's gate reads only +5Eh/+5Fh, so a dead gun's
//    mesh still stops shells and relays them to the ship, and 00728080 refuses the gun
//    damage at +3B8h.
//  * the children's broad phase is a sphere around the node-0 origin (the image's is the
//    child's AABB); it is a filter only, so no hit changes.
//  * the shot-kind gate reads the round's class (is_depth_charge_class), as 00826F44's
//    view does.
//  * LABELLED (rev8 C1 review, correcting its own (d)): 00859550 runs only for the
//    TurningGun classes (vtable[DCh] = 0085A270: 00CFBD20, 00CFBF58, 00CFC190, 00CFE548).
//    For the non-turning classes - the bomb racks 00CF96A8 / 00CF9918, the base gun
//    00CFE0A8 and 00CFE308 (vtable[DCh] = 0072B2D0), and MCatapult 00CFAAB8 (006EC620) -
//    node 0's local keeps 0072DD20's FULL slot frame (3x3 and T), while the host's pose
//    (device_hierarchy_node_world) uses RotY(-horz) with T only. The hit moves only
//    where an authored slot frame is rotated; the kMuzzleOffsetsBound muzzle path
//    carries the same gap.
//  * ordering (relay arm): the host runs the gun's 008777D0 from DeviceGunAfterRelay's
//    destructor in apply_hit (game_hosts_gunnery.cpp), after the parent path's
//    attribution tail; the image runs it at 0043F854, before control returns to
//    009239A0's attribution. Neither reads the other's state, so the order is harmless.
//  * LABELLED triangle test: device_trace_segment (game_hosts_gunnery.cpp) is the
//    house Moller-Trumbore (double-sided, |det| < 1e-12 cutoff, no 007238E0 AABB tree),
//    the same substitution kShellHullHitTestBound labels, not 00723AA0's own arithmetic.
//    The nearest-hit rule (`t <= best`, last acceptor on a tie) is 00723D60's.
// Movers when ON: shells that meet a mount before the hull stop at the mount (nearer
// point; record kind 9 / segment 0 instead of the hull element's), so the ship's hull
// pass, flooding and fire rolls run on its element 0, and the gun takes damage; hull
// hits shadowed by a turret move; the kamikaze trace (0098AC20 with kind 0 at 00819B24)
// and the HUD pick / gun seat queries can stop on a mount too; a plane's or land
// unit's mounts take hits that no longer reach the plane or land unit (0043F7F0 relays
// only to a kind-6 parent). No random draw is added.

namespace bsp {

inline constexpr bool kGunDeviceHitDamageCc13Bound = false;

// 0043F7F0's two decisions, in its order.
struct DeviceHitSplit {
    bool drop{false};    // 0043F822: RET 4 (AL = 0) before 008777D0; the parent handles it
    bool relay{false};   // 0043F843..: parent->vtable[ECh](record)
};

inline DeviceHitSplit device_hit_split_0043f7f0(bool shot_is_depth_charge, bool has_parent,
    bool parent_is_submarine, bool parent_is_ship, int hull_segment) noexcept {
    DeviceHitSplit out;
    if (shot_is_depth_charge) {
        if (!has_parent) return out;                   // 0043F810 JZ 0043F851
        if (!parent_is_submarine) { out.drop = true; return out; }
    }
    out.relay = has_parent && parent_is_ship && hull_segment != -1;   // 0043F827..0043F841
    return out;
}

}  // namespace bsp
