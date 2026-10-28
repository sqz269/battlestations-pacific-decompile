#pragma once

// cc13-gunF1: the discarded SetShot draws of 0084BAD0's gather (rev5 CONFIRMED F1).
//
// Image. 0084BAD0(args, ignore_falloff = [ESP+8], source = [ESP+0Ch], shot = [ESP+10h])
// calls 00428800 (BSP_Explosion_NotifyBlastSites, at 0084BB00; its only RNG call is a
// stream-0 site pick, 00BD2FC0 with ECX = 0 at 00428AA4/00428AA6), then gathers through
// 00904470(&records, centre, radius, ..., shot, source):
//  * 00904470 takes exclude = source->vtable[B0h]() (00904470..00904482) for 0098C630,
//    whose 0098C510 returns at 0098C535 for that node before recursing, so the source
//    and its mounts are never gathered;
//  * its loop over the gathered records (009044B4..00904547, no exit) calls SetShot
//    00470350 at 009044F4 with the shot, which draws shot->vtable[54h] whenever the shot
//    is non-null (006E7C60: U(class+ACh, class+B0h) on stream 1; or 006E2820);
//  * 0084BAD0's push loop then calls SetShot again at 0084BBB1 for the same records, in
//    the same order, with the same shot, before each 00926E80 push (0084BBF9). Its
//    record+0h == source skip at 0084BBCF follows both draws.
// So a burst over N records makes N discarded draws, then N used ones, each just before
// its push. The burst damage itself is drawn by the caller (0084BE4D in 0084BC60;
// 0070C247 for flak), the host's Draw::blast_damage.
//
// Host (ON): apply_impact_blast, after Draw::blast_damage, makes one discarded
// draw(Draw::hull_damage, gun_row, unit, DamageMin, DamageMax) per record that its walk
// will push (blast_gathered_records: the walk's own tests, in its order: the unit's
// record, then its mounts' with kGunDeviceBlastDamageCc13Bound). The walk itself is
// unchanged, and each push makes the used draw as before. This covers every caller of
// apply_impact_blast: depth charge in water, torpedo on land, OnImpact, flak expiry and
// flak lock.
// LABELLED:
//  * N counts one record per unit (and one per mount), standing in for one per collision
//    node the grid query meets;
//  * the source is never gathered (the exclude above), so 0084BBCF's skip never fires
//    for it and the source makes no draws; rev5's sketch had the source's record drawn
//    twice and skipped, which needs a source whose vtable[B0h] node is not the node the
//    query meets (not the case for the ship owners, projectile+238h, traced here);
//  * plane crash (apply_gunless_blast_hit): the crash shot is the plane's p+72Ch
//    sub-object, not the primary (rev5 N1): 007C0910 pushes p, then p+72Ch
//    (007C0BF6..007C0C01); 0084BF00's arg2 ([ESP+E0h] at 0084C079) goes to SetShot at
//    0084C0C3, and 0084BC60's buffer+20h is that record's +4h. Slot +54h of the +72Ch
//    vtables (00CFFFE8, 00D00280, 00D065B0, 00D06898, 00D0B9F8, 00D19CA0, 00D19F78,
//    00D1A250; bytes read) is 0042BBA0 (C2 04 00: RET 4, no FLD), so neither SetShot
//    pass draws, and F1 changes nothing there. UNEXPLAINED: a RET 4 reached through
//    SetShot's argument-less vtable[54h] call would unbalance the stack, so either SetShot
//    never runs on that object or a later constructor replaces its vtable (open with
//    rev5 / cgbuildA for C2b);
//  * the HUD free-camera, water-mine and kamikaze callers of 0084BAD0 are not hosted
//    through apply_impact_blast.
// Movers when ON: stream 1 shifts by N draws per shot-bearing burst from the first such
// burst on, which moves every later draw (hits, damage, AI rolls). The order is
// push-draw-exact (N discarded, then each used draw at its push) only with
// kHitDeferredDispatchCc13Bound ON; with it OFF, apply_hit dispatches each record at
// once, so the used draws interleave with the dispatch's own draws.

namespace bsp {

inline constexpr bool kBlastGatherShotDrawCc13Bound = false;

}  // namespace bsp
