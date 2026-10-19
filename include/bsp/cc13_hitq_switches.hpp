#pragma once

// Packet cc13-hitq (cc9 lead approval 2026-10-19). docs/CC13_HIT_DEFERRED_DISPATCH.md.
//
// kHitDeferredDispatchCc13Bound: the image does not dispatch a hit where it
// happens. 00926E80 BSP_DeferredEntityEventQueue_Push (__fastcall, ECX = record,
// EDX = direction, RET) copies the 54h-byte hit record and the direction into a
// 68h-byte node of the std::list at 00F899C0 and links it at head[+4h]
// (00926F12). Its callers are 00819BA7 (the kamikaze direct arm in 00819A20),
// 0084BBF9 (each blast record of 0084BAD0) and 0084BE20 (the impact 0084BC60).
// The only consumer is 00926700 BSP_DeferredEntityEventQueue_Drain, which takes
// head[+4h] again (00926714), so the newest hit goes first (LIFO), tests the
// subject's +5Eh/+5Fh (0092673F, 00926745) and calls 009239A0 at 00926750: the
// OnHit chain (vtable[ECh] = 007BBCF0 -> 008777D0, the damage and death) and the
// attribution 0077CE60. 00926700 runs at fan-out rows 6 (00875E44) and 14
// (00875EC4) and in 00874D00 at 00874DB9. The record's damage draw (00470350
// SetShot, 006E7C60 through 00BD2F10) is made before the push, at hit time.
//
// OFF (today): the gunnery host dispatches each hit where it happens, in
// arrival order. ON: the host's three hit funnels (apply_hit for 0084BE20 and
// 0084BBF9, apply_gunless_blast_hit for the shot-0 blasts of 0084BAD0) draw the
// damage and queue the record, and the existing drain
// bsp::drain_deferred_entity_events_00926700 dispatches them, newest first, at
// the three drain points.
//
// OFF is byte-identical: no allocation, no call and no log line is added on
// the OFF path. Not yet game-validated.

namespace bsp {

inline constexpr bool kHitDeferredDispatchCc13Bound = false;

}  // namespace bsp
