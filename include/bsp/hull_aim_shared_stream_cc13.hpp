#pragma once

// The hull aim point's random draws on the image's shared stream (packet cc13-hullaimrng).
// Header-only switch.

namespace bsp {

// kHullAimSharedStreamCc13Bound (OFF): 009FA260 BSP_BotApproachTargetRef_PickBodyFrameAimOffset
// calls the target's vtable[100h] (009FA2A0). For the nine hull-sampling ship vtables that is
// 00816650, which draws from 00BD2F10 with ECX = 1, the shared stream; for every other class
// it is 0042D810, which draws nothing. OFF, the host's pick (hull_aim_world_point in
// src/game_hosts_units.cpp) takes four counter-hash values per pick
// (approach_target_ref_unit_draws_substitute), which consume nothing. ON, the pick draws
// through GameGunneryHost::hull_aim_point_draw_00816650, the generator ship_ai_draw and every
// gunnery draw share, in 00816650's count and order:
//   section chance (sub+64h) > 0 (00816659 COMISS, JBE): roll U(0, 1) at 0081667C;
//   chance > roll (00816685 FCOMIP, JBE) and the surviving weight total > double 1e-4
//     [00D7A268] (00816773 FCOMI, JBE): pick U(0, f32(total - 1e-4)) at 00816796, and no more;
//   otherwise the box: U(-box.x, box.x) at 00816883, U(0, box.y) at 008168A0,
//     U(-box.z, box.z) at 008168D5, box = sub+48h (0081685A).
// So a pick draws 0 (non-ship target), 2 (section) or 3 / 4 (box, without / with the roll).
// Callers of 009FA260: 009FADA0 (009FAEAB, 009FAEB8), 009FB190 BindTarget (009FB1D9),
// 009A31A0 depth charge (009A3252), 009CA780 strafe rearm (009CA7D7), 007B48E0 rocket
// gotowards Enter (007B497A), 009ABF60 kamikaze RedrawAimError (009AC09E), 007B4810
// (undefined code, 007B4866). LABELLED: the host picks lazily, at the next aim-point query
// after the per-shooter copy goes dirty (a retarget, 009C3DA0 / 009D02A0's aim error, the
// strafe rearm, the kamikaze dirty carry); so the draws land where the host's pick runs, not
// at those call sites, and a path whose host binding does not dirty the copy (the depth
// charge pick_target_ref_009fa260 is empty) draws nothing. Any later change to the pick time
// moves these draws within the stream with it.
// STREAM-ORDER MOVER: every draw here advances the process-wide stream, so with ON every
// later 00BD2F10 consumer (ship AI torpedo draws, gunnery reloads, aim errors, flak, breakups,
// damage rolls) shifts. Pairs on that stream move as a whole, not only the aim point.
// Direct movers when ON: where dive, torpedo, strafe, rocket and kamikaze attacks aim on a
// ship's hull (box point or named section), hence hit location and section damage.
inline constexpr bool kHullAimSharedStreamCc13Bound = false;

}  // namespace bsp
