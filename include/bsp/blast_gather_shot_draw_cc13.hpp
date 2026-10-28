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
//  * plane crash (apply_gunless_blast_hit): 0084BF00 hands SetShot [arg2+0CCh], not
//    arg2 (0084C079 MOV EAX,[ESP+0E0h]; 0084C084 MOV EAX,[EAX+0CCh]; PUSH; 0084C0C3).
//    For the crash arg2 is p+72Ch and [p+72Ch+0CCh] is the plane's primary object (the
//    one 0084BF56 tests with IsKindOf(0Fh)), so the shot is the plane: its vtable[54h] is
//    0042B8D0 (D9 EE C3: FLDZ; RET) on all eight plane vtables (00D06638, 00D1A000,
//    00D19D28, 00D06920, 00D00070, 00D0BA80, 00D00308, 00D1A2D8; slot +54h read from the
//    bytes), so neither of its SetShot passes draws, and F1 changes nothing there;
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
