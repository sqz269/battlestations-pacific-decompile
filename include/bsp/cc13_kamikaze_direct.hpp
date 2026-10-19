#pragma once

// Packet cc13-kzdirect (cc9 approval 2026-10-19, via the cc13 lead).
// docs/CC13_KAMIKAZE_DIRECT_HIT.md.
//
// kKamikazeDirectHitCc13Bound: the direct arm of 00819A20
// BSP_UnitInstance_KamikazeDetonate (__thiscall(ECX = unit; target, const
// float3* offset), the message-70h arm 0082218D of 00821E80). OFF (today) the
// host counts the arm and records 00915F20. The image builds a hit record and
// delivers it:
//   00819A51..00819A8E  target non-null, +5Ch set, +5Dh/+60h/+5Eh clear, and
//                       classDesc+510h > 0.0f (00D7A218, COMISS / JBE);
//   00819AAB..00819AF3  s = offset * 1000.0 (double 00CE47A0), offset being
//                       the message vector contact - unit+FCh (008146A0);
//   00819B20..00819B2B  target->vtable[B0h] (006D1DF0: [target+360h], no
//                       arguments, RET) leaves its five pushes for 0098AC20
//                       BSP_CollisionNode_TraceSegment(from = unit+FCh, to = s,
//                       0, &record, 0); a miss skips to 00819BC3;
//   00819B46..00819B96  +14h = classDesc+510h, +18h..+20h = unit+FCh,
//                       +24h = +518h, +28h = +514h;
//   00819B9C            00470350 SetShot(unit): +4h = unit and +14h =
//                       unit->vtable[54h]() = 0042B8D0 FLDZ, so +14h ends 0
//                       (vtables 00CF90B0, 00CFA778, 00CFB738, 00CFC3D0,
//                       00CFFA30, 00D01630, 00D09678, 00D0BF80, 00D0C648;
//                       all also have +58h 0042B8E0 and +108h 0042BB30);
//   00819BA7            00926E80(record, EDX = offset), the delivery;
//   00819BBE            00915F20 BSP_MissionScoring_RecordHitDamage(target,
//                       &record).
// ON reproduces that and delivers through the host's immediate funnel
// apply_gunless_blast_hit. Not game-validated.

namespace bsp {

inline constexpr bool kKamikazeDirectHitCc13Bound = true;  // ON: USNOS c17 pair (docs/SHIP_AI_OPEN_ITEMS.md 306.1)

// 00CE47A0, FLD qword at 00819AB5: 1000.0.
inline constexpr double kKamikazeDirectTraceScale = 1000.0;

}  // namespace bsp
