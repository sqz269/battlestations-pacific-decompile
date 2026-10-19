# CC13 kamikaze direct hit (kKamikazeDirectHitCc13Bound)

Addresses: 00819A20, 00819A51, 00819A87, 00819AAB, 00819B20, 00819B24, 00819B9C, 00819BA7, 00819BBE,
00470350, 0098AC20, 006D1DF0, 0042B8D0, 0042B8E0, 0042BB30, 00926E80, 00915F20, 008146A0, 00427C90

Packet cc13-kzdirect (cc9 approval 2026-10-19, via the cc13 lead), from the adoption note in
`docs/CC13_HIT_DEFERRED_DISPATCH.md` (packet cc13-hitq) that rev3 confirmed. Switch:
`bsp::kKamikazeDirectHitCc13Bound` in `include/bsp/cc13_kamikaze_direct.hpp`, **OFF**. Status:
reconstructed and build-tested, not game-validated. Every descriptive name is a hypothesis, not a
recovered symbol.

## The image, 00819A20's direct arm

`00819A20` `BSP_UnitInstance_KamikazeDetonate`, `__thiscall(ECX = unit; target, const float3* offset)`.
Its only caller is the message-70h arm `0082218D` of `00821E80` `BSP_UnitInstance_HandleMessage`,
which the landing-ship, mothership and submarine handlers chain to. The offset is the message vector
`008145B0` builds at `008146A0`: `00427C90` subtracts `00427EB0(unit)` (unit+FCh) from the contact,
so offset = contact - unit+FCh. Read from the raw listing, `00819A20`-`00819BD4`:

| site | step |
| --- | --- |
| `00819A44` | `unit+100Ah = 1` (both switch positions, outside this arm) |
| `00819A3E`, `00819A51`..`00819A73` | skip the arm unless the target is non-null, `+5Ch` set, `+5Dh`, `+60h` and `+5Eh` clear |
| `00819A87` | `COMISS classDesc+510h, [00D7A218] = 0.0f` / `JBE`: skip unless +510h > 0 (an unordered value skips) |
| `00819A98` | `00470470` resets the 60h record at `ESP+20h` (+30h/+34h = -1) |
| `00819AAB`..`00819AF3` | `s = offset * 1000.0` (double `00CE47A0`), each component stored to a float |
| `00819B00`..`00819B20` | five pushes (`unit+FCh`, `&s`, 0, `&record`, 0), then `target->vtable[B0h]` |
| `00819B22`..`00819B2B` | `vtable[B0h]` is `006D1DF0` (`MOV EAX,[ECX+360h]; RET`, no stack arguments), so `0098AC20` `BSP_CollisionNode_TraceSegment` on the node `[target+360h]` takes those pushes: from = unit+FCh, **to = s** (a point, not unit+FCh + s), exclude 0, the record, kind 0 (`RET 14h`). `AL == 0` jumps to `00819BC3` |
| `00819B46` | `+14h = classDesc+510h` |
| `00819B5F`..`00819B75` | `+18h..+20h = unit+FCh` |
| `00819B83`, `00819B96` | `+24h = classDesc+518h`, `+28h = classDesc+514h` |
| `00819B9C` | `00470350` SetShot(unit): `+4h = unit`, then `+14h = unit->vtable[54h]()` |
| `00819BA7` | `00926E80(ECX = &record, EDX = offset)`, the delivery |
| `00819BBE` | `00915F20` `BSP_MissionScoring_RecordHitDamage(target, &record)` |
| `00819BCF` | `004704B0` destroys the record |

**+14h ends at 0.** Every class whose handler reaches `00819A20` has `vtable[54h] = 0042B8D0`
(`FLDZ; RET`). The classes checked are the vtables whose `+164h` holds `00821E80`, `00852C60`,
`0074B570` or `00758EB0`: `00CF90B0`, `00CFA778`, `00CFB738`, `00CFC3D0`, `00CFFA30`, `00D01630`,
`00D09678`, `00D0BF80` and `00D0C648` (`00D09678` and `00D0C648` hold `00821E80` at `+164h`). So SetShot overwrites the `+510h` store, and `KamikazeDamage` decides only the gate.
In the same vtables `vtable[58h] = 0042B8E0` (`FLD1`, the weapon scale) and
`vtable[108h] = 0042BB30` (`XOR EAX,EAX`, no weapon), and `vtable[B0h] = 006D1DF0`.

**What the delivered record does.** `009239A0` skips its scoring hook (the shot is not class
`29h`/`2Ah`). In the OnHit chain, `00826F10`'s hull pass runs only when the trace wrote a hull
segment, with base 0. The weapon block is skipped (`vtable[108h]` is 0 at `008270D9`, tested at
`0082733C`), so there is no fire roll and no draw. The part loop is empty: `0098AC20`'s shapes write
`+30h`, `+34h` and `+38h` (`00723F5F`..`00723F6C`) but no part array, whose only producer is
`006D2E30`, the blast gather (`docs/HIT_NARROWPHASE.md`). Damage is therefore zero for any target
armour >= 0. `0077CE60`'s damage test reads `HullDamage(0)` or `MaxPartDamage(0)`, both 0, so
there is no attribution either.

## What the host did (OFF, unchanged)

`kamikaze_detonate_00819a20` gates on `!dead && +510h > 0`, counts `kz_direct_arms` and records
`KamikazeDetonate::direct_hit_00915f20`. No record is built or delivered.

## What ON does

`GameGunneryHost::Impl::kamikaze_direct_hit_00819a20`, in place of the OFF block:

| image | host |
| --- | --- |
| target gate `00819A51`..`00819A73` | `GameUnitsHost::unit_scene_node_flags` (+5Ch, +5Dh, +5Eh) and `unit_pending_destroy_0060` (+60h); a unit with no modelled bytes falls back to `!dead` (SUBSTITUTION) |
| `+510h > 0` | `!(kr.damage_510 > 0.0f)` returns, so NaN skips as JBE does; then `++kz_direct_arms` |
| offset, `s` | `offset = contact - centre` (the host's contact point and `unit_pose` origin), `s = float(double(offset) * 1000.0)` |
| `0098AC20` on `[target+360h]` | `bsp::test_entity_0098ac20` with this host's `SegmentBinding` on the target's handle; the target's hull mesh or box stands for the node's shapes (SUBSTITUTION), with no children; the two shot counters are restored as `query_segment_units_impl` does |
| miss -> `00819BC3` | `++kz_direct_missed`, return |
| the record | `hull_damage_base = 0` (after SetShot), `part_damage_base = +514h`, `falloff_range = +518h`, `ignore_falloff = false`, `armour_selector = traced y` (+0Ch), `hull_segment` and the view's kind from the trace (with `kHullElementSegmentBound`, as `apply_hit` does), `weapon_scale = 1`, no part array |
| `00926E80` delivery | `apply_gunless_blast_hit(unit, target, traced point, record, &shot)`, the immediate funnel, with `shot_present = true`, no weapon, not a depth charge or torpedo, and direction = offset |
| `00915F20` | the existing record, now after the delivery and on a trace hit only, as at `00819BBE` |

`apply_gunless_blast_hit` gains a defaulted `const KamikazeDirectShot*`. Null (every existing
caller) gives the old view, `shot_present = false` and direction `(0, 1, 0)`.

ON adds `summary mission gunnery kamikaze direct arm traced= missed=`.

TODO (queued path): when `kHitDeferredDispatchCc13Bound` (packet cc13-hitq) lands, a follow-up
routes this delivery through that queue when that switch is ON: a `DeferredHit` with
`gunless = true`, the shot data and the offset as its direction. The code carries a
`TODO(cc13-hitq)` at the call.

## OFF identity

The direct arm is `if constexpr (kKamikazeDirectHitCc13Bound) { new } else { the old block }`. The
funnel's only OFF change is that the view's segment kind, the shot flag and the direction pointer
are chosen by `unit_shot != nullptr`. Every OFF caller passes null, so they hold the same values.
The new summary line and counters are ON-only, and OFF has no new call, draw or allocation.

## Shared RNG 00BD2F10

ON makes **no new draws**, so no RNG sub-switch is needed:
- there is no `Draw::hull_damage` draw, because SetShot's `vtable[54h]` is `FLDZ` and the funnel
  never draws one;
- there is no `Draw::hit_effect` draw, because `random_unit_float` sits behind `weapon_present`,
  which is false;
- there are no death draws, because the damage is zero.

This holds for target armour >= 0. A negative armour would make `(0 - armour) * 1 > 0` and open
the hull-pass paths.

## ON lifetime

- **Target.** With immediate delivery, the target index is the one gated at `00819A51`, used at
  the same instant. The funnel also refuses a `dead` target, as it does for the blast records.
- **Shooter.** The direct arm runs before the blast arm (`00819BDA`), as in the image, so the
  kamikaze is still alive when its direct record is dispatched; its own blast kills it afterwards.
  Under the future queued path the order reverses: LIFO dispatches the blast records first, the
  kamikaze dies to its own self-record, and then the direct record arrives. In the image the shot is
  a raw unit pointer that stays allocated through the `00903610` grace period. The record's only
  shooter readers are `00826F10`'s shooter position (`push_damage_direction`, for the local
  player's unit) and `0077CE60`, whose damage test fails first. So there is no attribution to lose,
  and the host's shooter index stays valid.

## Reach, current-main reference logs

These are the newest logs per mission from the last 30 hours across the lanes' `local\`
(`local\kz_reach.py`). In every mission not listed below, every log shows `detonations=0
direct_arms=0`: BSM01, BSM04, BSM09, ESMP08, IJN08, IJN18, JM02-JM06, JM08, JM12-JM14, LOMP01,
USN01-USN04, USN07, USN09-USN13, USN15, USN18, USN20, USN21 and USNRM01.

| mission | log | detonations | direct_arms |
| --- | --- | --- | --- |
| USNOS | cc9-ships44 `s60_pj_c21.log` | 2 | 2 |
| USNOS | cc9-ships44 `s60_p_c21.log` | 1 | 1 |
| USNOS | cc9-lua52 `l70r_vos17_main_ref.log` / `_vcon_ref` | 1 | 1 |
| USNOS | earlier runs (`l70r_vos_main_ref`, `p23h_*_os`, ...) | 0 | 0 |

The USNOS detonations are boats (`KamikazeDamage = 3000`, blast 1500, range 50), with contacts near
the world origin, e.g. `(-34.0 1.7 45.5)`. There the traced point `1000 * offset` lies roughly along
the offset, so a trace hit is likely. Far from the origin the segment from unit+FCh to `1000 * offset`
need not cross the target. That is the image's arithmetic and is reproduced as is. Kaitens arrive
once the subdive Kaiten fix lands.

## Expected movers when ON

For each detonation with a trace hit:
- one more `00826F10` dispatch: `queued_hits`, `dispatched_hits` and `ship_hit_records` +1, the
  target's `hits_taken` +1, `damage_taken` +0;
- `push_damage_direction` when the target is the local player's unit and the trace wrote a hull
  segment;
- `notify_plane_hit_007bbcf0` on the target;
- the `00915F20` record is counted only on a trace hit (`kz_direct_traced`); misses show in
  `kz_direct_missed`;
- `kz_direct_arms` is counted only after the `+5Ch`..`+60h` liveness gate (`00819A51`..`00819A73`),
  so ON and OFF can differ for a target that is pending destroy (`+60h` set) but not yet dead.

No health, death, attribution or RNG change is expected.

## Uncertainty

- The trace is the host's hull mesh or box, not the node's real shapes.
- Armour >= 0 is assumed for the zero-damage conclusion; it was not checked per class.
- `00915F20`'s own effects are not reconstructed here; it stays a record.
