# 009F3090, the nested update that produces the attackmove approach point

Addresses: 009F3090, 009F1BC0, 009E7FC0, 009E6E80, 009E9190, 009E74D0, 009E76D0, 009E6A90, 009E5E90, 009E5E00, 009D68B0

Packet `cc_ai_approach_update`, worker `agent/cc-ai-approach-update`. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; Ghidra was **read-only**.
Every descriptive name below is a hypothesis, not a recovered symbol.
Header `include/bsp/ship_ai_approach_update.hpp`, source `src/ship_ai_approach_update.cpp`,
machine-readable evidence `reports/ship_ai_approach_update.json`.

## Vocabulary and the offset shift

`009F3240` calls this update with `ECX = sub+8h` (`009F3289 LEA ECX,[EDI+8]`), so the object
stepped here is the nested ring object `009E5530` builds, and **every offset in this document is
nested-relative**. `sub = nested - 8`, which maps the four fields `009F3240` reads back:

| `009F3240` calls it | here | role |
| --- | --- | --- |
| `sub+11E8h` | `nested+11E0h` | the planar range to the attackmove destination |
| `sub+1214h` | `nested+120Ch` | the commanded heading, into `brain+1E0h` |
| `sub+1218h` | `nested+1210h` | the commanded throttle, into `brain+1D8h` |
| `sub+1230h` / `sub+1238h` | `nested+1228h` / `nested+1230h` | the approach point's x and z |

`owner` is `[nested+0h]`, the ship AI brain (`009E5540`). `unit` is `[brain+0AA8h]`, `shipclass`
`[brain+0AACh]`, `target` `[brain+0B20h]`, and the attackmove destination is
`(brain+0B2Ch, brain+0B30h, brain+0B34h)`, the goal vector `docs/SHIP_AI_GOAL_VECTOR.md` names.
`tune` is `[brain+0AB0h]`, a float block this packet reads at `+0h`, `+4h`, `+8h`, `+0Ch`, `+10h`,
`+14h`, `+18h` and `+1Ch`; it has no producer here. `slot i` is `nested + 4h + i*4Ch` and
`slot+8h` is its bearing, both from `009E5530`.

## Answer to the packet question

**(1) `009F3090` whole.** `__thiscall(nested)(float seconds)`, `RET 4`, body `009F3090-009F30E3`,
complete. The body is seven calls on the same object in a fixed order and nothing else: `009F1BC0`
(seconds), `009E7FC0`, `009E6E80`, `009E9190`(seconds), `009E74D0`(seconds), `009E76D0`(seconds),
`009E6A90`. `ESI` holds the object across all seven (`009F3099`) and the float argument is
re-pushed for the four that take one.

What the sequence does, in order:

1. `009F1BC0` writes the frame state: the unit's heading into `nested+11ECh` (from
   `unit->vtable[50h]`), the planar range to the destination into `nested+11E0h`, the turn radius
   into `nested+11F0h`, two countdowns at `nested+1220h`/`+1224h`, **the approach point** into
   `nested+1228h/122Ch/1230h` and the approach mode into `nested+1234h`. It also parks the sentinel
   `9999.0f` (`00CE4C04`) in the throttle at `nested+1210h`.
2. `009E7FC0` zeroes `slot+18h..+2Ch` for all sixty slots, then, in mode 0 only and only when the
   zone tests pass, scores every slot through `009E5DA0` and normalises `slot+2Ch` by the frame
   maximum times `tune+0h`. In mode 4 it instead decays every slot through `009E6400`.
3. `009E6E80` picks the standoff range `nested+11E4h` for the current mode, then scores every slot
   through `009E6870` with a four-float block.
4. `009E9190` refreshes a nearby-traffic list at `nested+14A0h` on a 2-3 second random timer, sums
   a weighted avoidance vector from it and writes the per-slot avoidance weight `slot+3Ch`.
5. `009E74D0` writes the two per-slot weights `slot+34h` (bearing) and `slot+38h` (evade).
6. `009E76D0` advances a wobble phase, scores every slot through `009E6640`, rejects the low
   scorers, picks the slot with the largest sum of `slot+2Ch..+3Ch`, stores its bearing in
   `nested+11F8h` and hands it to `009E5E90`, which writes the commanded heading `nested+120Ch`.
7. `009E6A90` seeds the throttle from the heading error when it still holds `9999.0f`, computes a
   speed cap from the mode machine and clamps `nested+1210h` to that cap.

**The approach point itself is the attackmove destination copied verbatim** in the ordinary case:
`009F1F2D..009F1F3D` copy `brain+0B2Ch/0B30h/0B34h` into `nested+1228h/122Ch/1230h`. Only when the
target carries a zone object (`009F1E36`, `[target+740h]`) and the unit's own avoid-zone group
(`[[unit+538h]+570h]`, `009F1E55`) is **below** the target's (`009F1E5E`, that zone object's
`vtable[2Ch]`; `009F1E60 CMP EAX,EBX` then `JGE 009F1F10`) does `00417B10` displace it.

So the six Japanese destroyers in `usn_2_java` get their approach point from the attackmove
command's destination; this update's real work is the *bearing* they take toward it and the
*throttle* they hold, not the point.

**No range or side is stored for `009F3240` beyond the four fields in the table above.** The ring
side comes out of the slot the winner scan picks, not out of a separate field.

**(2) `009D68B0`, the circle-tangent primitive.**
`float* __fastcall(ECX = float out[2], EDX = const Circle*)(const float point[2], float clearance,
int side)`, `RET 0Ch` at `009D6A3A` and `009D6ABF`, body `009D68B0-009D6AC1`, complete. The circle
is three floats: centre x at `+0h`, centre z at `+4h`, radius at `+8h`.

```
index = (side == 0) ? 1 : 0                009D68B7, 009D68C7 (CMP then SETZ), EBP*8 scaling
if 009D6550(circle, point, &t0, &t1):      009D68EC
    chosen = index ? t1 : t0               009D68F5, 009D6901
else:
    d2 = (circle.x-point.x)^2 + (circle.z-point.z)^2      009D6970..009D6992
    if d2 > 1e-10 and sqrt(d2) >= 1.0:     009D69A0, 009D69B7 (COMISS 1.0f then JBE)
        chosen = point                     009D68D6/009D68E6 preloaded it there
    else:
        a = random(stream 1, 0, 2*pi)      009D69DD, constant 00CE3D9C
        return (circle.x + r*sin a, circle.z + r*cos a)   009D6A22..009D6A33
sep = |chosen - point|                     009D690D..009D6967, same 1e-10 guard
if clearance <= sep: return chosen         009D6A50 FCOMIP then JBE
004F47B0({point.x, point.z, clearance}, circle, &o0, &o1)  009D6A8A
return index ? o1 : o0                     009D6A8F
```

Note the sine goes on **x** and the cosine on **z** in the degenerate arm (`009D6A12` multiplies by
the slot holding `sin` and stores into `[EBX]`), which is the same from-`+Z` convention the ring
uses. The two callees belong to the `ship_ai_nav_circle_tangent` packet and were not read.

**(3) `unit+538h`.** Its class is still open, but this packet adds evidence: `0082ADC0` is called
on it at `009F1E77` and `009F23DC` and reaches `BSP_AvoidZoneManager_GetSingleton` (`004218E0`),
and `[unit+538h]+570h` is an avoid-zone group id compared against the target zone's
`vtable[2Ch]` at `009F1E60`. So the component carries avoid-zone data as well as the armament
readiness `009F3240` tests at `vtable[2Ch]`. No constructor or vtable was read, so the class stays
with the `ship_ai_unit_armament_component` packet.

## `009F1BC0`, the frame state and the approach point (partial)

`__thiscall(nested)(float seconds)`, `RET 4` at `009F3083`, body `009F1BC0-009F3083`, SEH scope
`00CB0BA0`. Read here: `009F1BC0-009F1DBF`, `009F1E16-009F1F47`, `009F1F7F-009F2124`.

```
nested+1210h = 9999.0f                                     009F1BF7 (00CE4C04)
nested+1220h -= seconds ; nested+1224h -= seconds          009F1C07, 009F1C13
nested+11ECh = unit->vtable[50h]()                         009F1C24
refresh the unit pose when [unit+0C8h] == 0                009F1C40
nested+11E0h = hypot(goal.x - unit.x, goal.z - unit.z)     009F1C6A..009F1CDE
    the sum is dx*dx + 0*0 + dz*dz with a float store, compared against 1e-10
    (00CE3820) with FCOMI/JBE; only the greater side calls 00BF7030 sqrt
nested+11F0h = max([shipclass+500h] * 10, 00811A30(1.0) * 1.5)   009F1D1E..009F1D6D
nested+11D8h -= seconds; if negative, clear nested+11D6h and
    reseed it with random(stream 1, 2.0, 3.0)              009F1D84..009F1DB9
nested+121Ch -= seconds                                    009F1E1E
```

The mode latch at `nested+1234h`, `009F1F7F..009F2124`:

| value | set at | condition |
| --- | --- | --- |
| 1 | `009F1FE2` | the kind-8 arm: the unit does **not** answer `vtable[5Ch](8)` (`009F1F47`), the target does (`009F1F5B`), the displaced-point flag is clear (`009F1F65`), `00827F70(shipclass)` says no (`009F1F76`), and `00811A30(unit, 1.0) * (2.1 in mode 1, else 1.9) > nested+11E0h` (`009F1FD2`, `FCOMIP` then `JBE` to mode 0). It also sets `nested+11F0h = min(nested+11F0h, nested+11E0h)` at `009F1FE8`. |
| 1 | `009F2022` | `[unit+54h] == [target+54h]`, the same side. |
| 1 | `009F2112` | the same-side test again on the `009F20F7` arm. |
| 3 | `009F20B4` | the unit answers `vtable[5Ch](0Ch)` (`009F208D`), `006F2D90(target)` passes, and `(int)[target+7C4h] + max(300.0f, 2 * 00811A30(unit, 1.0)) >= nested+11E0h` (`009F20AE`, `FCOMIP` then `JC` to mode 4), so the unit is already inside. |
| 4 | `009F20C0` | otherwise, on the engageable arm. |
| 0 | `009F1FF4`, `009F211A` | every other path. |

Value 2 is compared by `009E6E80` (`009E7002`), `009E6A90` (`009E6C9F`) and `009F1BC0` itself
(`009F24FC`, `009F282D`) but is **never assigned** in any range this packet read.

## `009E7FC0`, the per-frame score reset

`__thiscall(nested)()`, `RET 0`, body `009E7FC0-009E82F8`, complete.

`nested+1218h = 0.0f` (`009E7FD1`, no reader anywhere in this packet), then six floats per slot at
`+18h..+2Ch` are zeroed (`009E7FE0..009E8003`, `EAX` starting at `nested+2Ch` and advancing `4Ch`).
Either override byte at `nested+11D4h`/`+11D5h` clears `nested+12BAh`; both together end the pass
(`009E8005..009E8029`). Modes 1, 2 and 3 return (`009E8035..009E804A`). Mode 4 runs
`009E6400(slot, nested+1290h)` for all sixty slots, but only while
`nested+11E0h - nested+11E4h < 300.0` (`009E8061`, `00CE3CA8`).

Mode 0 is the scoring arm. It probes `target->vtable[5Ch](5)` and throws the answer away
(`009E80AB`, overwritten at `009E80AD`), requires the byte at `brain+0B28h`, requires
`nested+127Ch <= [unit+494h]` (and exact equality when `nested+1208h` is set and `nested+1209h` is
not, `009E80E7..009E80FF`), and requires the zone test `00864FD0` or, with no target, `009E6120`
followed by `00864BA0`. Then it seeds `nested+1290h = nested+11DCh`, `nested+1294h = 20.0f`,
`nested+1298h = 60.0f`, sets `nested+12BCh`/`+12BDh`, scores every slot through `009E5DA0` keeping
a running maximum seeded to `1.0f`, and writes `slot+2Ch = slot+18h / maximum * tune+0h` in a
10-way unrolled loop (`009E81E4..009E82DA`, `EAX` advancing `0x2F8 = 10 * 4Ch` six times). The
image never tests the divisor.

## `009E6E80`, the standoff-range choice

`__thiscall(nested)()`, `RET 0`, body `009E6E80-009E74CD`, complete. It writes `nested+11E4h` and
then scores every slot through `009E6870`.

| arm | condition | `nested+11E4h` |
| --- | --- | --- |
| `009E6E94` | both override bytes set | `-1000.0f` (`00D7A240`) |
| `009E6ECA` | `tune+1Ch >= 0.0f` | `tune+1Ch` |
| `009E6EE8` | mode 1 or 3 | `-1000.0f` |
| `009E6EFC` | target answers `vtable[5Ch](8)` and `00827F70(shipclass)` says no | `-1000.0f` |
| `009E6F29` | mode 4 | keep if it is already in `[base, base+250]`, else redraw uniformly there. `base` is `(int)[target+7C4h] - 300.0` when the target answers `vtable[5Ch](1Ch)`, else `1000.0`, minus `200.0` for a group leader. |
| `009E700B` | mode 2 | keep if it is in `[lo, hi]`, else redraw. With a kind-1Ch target `lo = interp(100, 0.5, 300, 0.75, [unit+9C8h]) * (int)[target+7A0h]` and `hi = (int)[target+7A0h] * 0.85`; without one `lo = 800.0`, `hi = 1200.0`. A group leader shrinks `hi` by `0.8` and raises `lo` to `0.9 * hi`. |
| `009E7138` | mode 0 with `nested+1208h` set | keep if it is in `[0.92 * [unit+490h], 0.98 * [unit+490h]]`, else redraw |
| `009E71A5` | mode 0 otherwise | `00952530(nested+13B0h) + 300.0`, then a 119-step scan from `x = 50.0` in steps of `25.0` keeping the x with the **smallest** score `max(1.0, q(x)) * (nested+1284h / p(x)) * interp(0, 2, 1, 1, p(x)/ref)`, where `p` and `q` are `00955A40` on `nested+12C0h` and `nested+13B0h` and `ref` is `009523C0(nested+12C0h)`. The seed is `FLT_MAX` (`00D7A248`) and the update is `FCOMIP` then `JC` at `009E72AE`. |

Then the common tail at `009E6FBD`: `r = max(250.0f, 00811A30(unit, 1.0) * 1.5)`; when
`nested+12BAh` is set, `nested+12B4h > 0` and `0080DF40(unit) > 0`, clamp
`nested+11E4h` down to `nested+12B4h - r`; form `t = (nested+11E0h - nested+11E4h) / r`, shifted by
`±1.25` when `nested+1208h` is set; the span weight is `interp(0.3, 0.43633232, 1.3, 0, |t|)`; and
the side weight is `pi` with `nested+1204h = 0` when `t < 0` in mode 0 with `nested+1208h` clear,
`0` with `nested+1204h = 2` otherwise, or, on the other branch and only for `t < 0`,
`(0.5 * q) * (q + pi * q)` with `q = c + c*c` float-stored and `c = min(|t|, 1)`
(`009E7441..009E7469`). The four floats `{side, span, nested+11DCh, tune+4h}` go to `009E6870` for
each of the sixty slots.

## `009E9190`, the nearby-traffic list and the avoidance weights

`__thiscall(nested)(float seconds)`, `RET 4`, body `009E9190-009E9736`, complete, SEH scope
`00CB0AAB`. Modes 1 and 3 take `009E96D8`, which writes `0.0f` into every `slot+3Ch` through an
interpolation whose two ordinates are both zero.

Otherwise `nested+11F4h -= seconds`; when it goes negative it is reseeded from
`random(stream 1, 2.0, 3.0)` and the list at `nested+14A0h` is refilled: for every entity in
`008053C0([unit+54h])`'s list at `+0DE8h` that answers `vtable[5Ch](5)`, is not the attack target,
and whose squared planar distance from the unit is below `([entity+494h] + 200.0)^2`
(`009E92CF..009E92E9`), a `0x124`-byte record is allocated and spliced in.

The accumulation pass walks the list, drops records that fail `009E6170(record, unit position,
300.0f)`, advances the rest through `009E6240(record, seconds, unit position, &nested+1238h)` and
sums `[record+120h] * ([record+10Ch], [record+110h], [record+114h])` from a seed of the zero vector
at `00F87574`. When the accumulated squared length exceeds `1.0` the heading is
`pi/2 - atan2(acc.z, acc.x)` lifted by `2*pi` when negative, and the strength is
`interp(0, 0, tune+0Ch, tune+8h, |acc|)` with `|acc|` from `0042B2F0`; otherwise both are zero.
Each slot then gets `slot+3Ch = interp(0, strength, pi, 0, wrap(slot.angle - heading))`.

**The wrapped difference is passed signed here** (`009E968D` then `009E96B4`, no sign mask), unlike
`009E74D0`, so every slot whose bearing is below the avoidance heading clamps to the full strength.

## `009E74D0`, the bearing and evade weights

`__thiscall(nested)(float seconds)`, `RET 4`, body `009E74D0-009E76CE`, complete.

```
gain = 1.0f                                                009E74EC (00D7A24C)
if nested+11FCh > 0:                                       009E7502 COMISS then JBE
    if [unit+1128h] > 0: nested+11FCh += seconds           009E7527, 009E7534
    if nested+11FCh > 40.0f:                               009E7542 (00CE685C)
        if nested+1200h == 60:                             009E754B, the idle sentinel
            nested+1200h = (int)(30.0 + nested+11F8h)      009E7557..009E7566
            if that is >= 60, subtract 60                  009E756E (unreachable, see below)
        gain = 5.0f                                        009E757D, 009E758D (00CE3850)
        if |(float)nested+1200h - nested+11F8h| < 12.0f:   009E75AD (unreachable, see below)
            nested+11FCh = -1.0f ; nested+1200h = 60       009E75BC, 009E75C4
for each slot:
    slot+34h = interp(0, tune+10h, pi, 0, |wrap(nested+11ECh - slot.angle)|)     009E7646
    slot+38h = interp(0, tune+14h * gain, tune+18h, 0, |wrap((float)nested+1200h - slot.angle)|)
                                                                                 009E76B2
```

Both absolute values are an `AND` of `0x7FFFFFFF` on the float bits (`009E7619`, `009E7688`), not
`fabsf`, so a NaN difference stays a NaN.

**Two branches are unreachable**, and the reason is a producer mismatch rather than a reading
error: `nested+11F8h` is a bearing in `(-pi, pi]`, not a slot index. Its only two producers are
`009E7C28`, which stores the winning slot's `slot+8h` bearing, and `009E7EA6`, which stores an
`atan2` heading. Therefore `(int)(30.0 + bearing)` is always in `[26, 33]`, the wrap at `009E756E`
never runs, and `|(float)nested+1200h - nested+11F8h|` is always about 30, so the 12.0f reset
window at `009E75AD` is never entered. Once `nested+1200h` leaves 60 it never returns to it. The
reconstruction reproduces the arithmetic as written; it does not correct it.

## `009E76D0`, the ring scan that picks the approach bearing (partial)

`__thiscall(nested)(float seconds)`, `RET 4`, body `009E76D0-009E7EDC`.

`nested+1214h += seconds * 0.2` then `00605070` wraps it in place (`009E76DA..009E76F4`), the same
helper `009F3240` uses on `brain+1E0h`. Mode 3 clears every `slot+40h` byte and skips the scoring
(`009E79B4`). Otherwise every slot is scored by
`009E6640(slot, seconds, nested+11DCh, nested+11D4h, nested+11D5h, nested+11F0h)` with a running
maximum, the scores are divided by that maximum when it is below `1.0` (`009E7795`), and each slot
is accepted or rejected against `0.85` (`00CF0B58`, `009E7814`): an accepted slot only has its
`+40h` byte cleared (`009E7852`), a rejected one has `+2Ch`, `+38h` and `+3Ch` zeroed and
`+30h = -0.0f - tune+4h` (`009E7843..009E784B`). **Nothing in this packet ever sets that byte to
non-zero**; `009E6640` or `009E6870` must.

The winner is the slot with the largest `slot+3Ch + slot+2Ch + slot+30h + slot+34h + slot+38h`
(`009E79CA` for slot 0, `009E7BE0` for the tail), and its bearing goes to `nested+11F8h`
(`009E7C28`).

`009E7C3E..009E7C78` computes `wrap(bearing + sin(phase) * 00D7A258)` and discards it: the
amplitude at `00D7A258` is the double `0.0` and the result is popped by the `FSTP ST0` at
`009E7C78`. The wobble is dead twice over.

When `nested+1209h` is set and both override bytes are clear, the bearing is replaced outright
(`009E7C98..009E7EAE`): the heading toward `(brain+0B2Ch, brain+0B34h)` from the unit's refreshed
position, reversed by `pi` when
`(double)[unit+490h] - interp(0.2, 400, 1.2, 50, |wrap(nested+11ECh - heading)|) > (double)nested+11E0h`
(`009E7E68`, `009E7E7D`).

Either way `009E5E90(bearing, seconds)` runs at `009E7ECB`.

## `009E5E90`, the commanded heading

`__thiscall(nested)(float bearing, float seconds)`, `RET 8`, body `009E5E90-009E605A`, complete.
`seconds` is pushed by `009E76D0` and never read.

```
slot = (int)(((bearing + pi/60) [+2*pi if negative]) * 60 / (2*pi))    009E5E91..009E5ECC
if slot == nested+11E8h:                                              009E5ED7
    add_arm = wrap(nested+11ECh - bearing) < 0                        009E5EEF, 009E5F00 JNC
else:
    walk the ring both ways from nested+11E8h to slot; an unblocked slot
    (slot+40h == 0) costs 1 and latches BL, a blocked one costs 60 once
    BL is set and nothing before it; BL is seeded from the committed
    slot's own byte and is NEVER cleared between the two walks
                                                                      009E5F59..009E5FC1
    add_arm = backward_cost > forward_cost                            009E5FC3 CMP then SETLE
d = add_arm ? wrap(bearing - nested+11ECh) : wrap(nested+11ECh - bearing)
nested+120Ch = (-0.1 < d < 150 degrees) ? bearing
             : add_arm ? wrap(nested+11ECh + 150 deg) : wrap(nested+11ECh - 150 deg)
                                                                      009E5F1F..009E6050
```

`pi/60` is `00D1A8A0` (half a slot), `-0.1` is `00CE3928`, `150 degrees` is `00D1FED0`.
`nested+11E8h` itself is never written by any routine in this packet; `009E5530` zeroes it at
`009E561F`.

## `009E6A90`, the throttle limiter

`__thiscall(nested)()`, `RET 0`, body `009E6A90-009E6E78`, complete.

```
e = wrap(unit->vtable[50h]() - nested+120Ch)               009E6AB5, 009E6ABB
if nested+1210h > 1000.0:                                  009E6ADE, the 9999.0f sentinel
    nested+1210h = interp(pi/6, 1.0, 70 deg, 0.5, |e|)     009E6B12
limit = 1.0f                                               009E6B2E
mode 4:  [unit+1128h] > 0                    -> limit = 0
         both override bytes clear:
             ref = 009E5E00() ? (int)[target+7C4h] - 300.0 : nested+11E4h
             d   = nested+11E0h - ref
             d < 50.0                        -> limit = 0                009E6BB8
             else limit = interp(0, 0.25, 200, 1.0, d)                   009E6C2B
             and, with r = leader ? 100.0f : 200.0f, when r + ref > nested+11E0h,
             the target exists and unit->vtable[234h](target) passes,
                                             -> limit = 0.1f             009E6C8C
mode 2:  limit = interp(50, 0.4, 300, 1.0, nested+11E0h - nested+11E4h)  009E6CE2
if limit > 0:                                                            009E6CF4
    nested+1204h == 1 and nested+11E4h > nested+11E0h - 80.0 -> limit = 0   009E6D24
    nested+1204h == 2:
        d = nested+11E0h - nested+11E4h
        d >= -50.0 and |e| < pi/12 -> limit = min(limit, clamp((d-50)/80, 0, 2))   009E6DBF
        d <  -50.0 and |e| < pi/12 -> nested+1210h = -min(limit, clamp((-d-50)/40, 0, 0.5))
                                                                                   009E6E2A
nested+1210h = clamp(nested+1210h, -limit, +limit)                       009E6BC7..009E6E78
```

The final clamp is written out with `FCOMIP` then `JBE` twice in the same direction, so a NaN
command falls through both and is stored unchanged; it is not a `std::clamp`.

## `009E5E00`, the engagement-target accessor

`__thiscall(nested)()`, `RET 0`, body `009E5E00-009E5E29`, complete. Returns `[brain+0B20h]` when
it is non-null and answers `vtable[5Ch](1Ch)`, else 0. `009E6A90` is its only caller.

## Host methods the executable must implement, in call order

| routine | order |
| --- | --- |
| `009F3090` | `009F309B` 009F1BC0, `009F30A2` 009E7FC0, `009F30A9` 009E6E80, `009F30B8` 009E9190, `009F30C7` 009E74D0, `009F30D6` 009E76D0, `009F30DD` 009E6A90 |
| `009F1BC0` (read part) | `009F1C24` vtable[50h], `009F1C40` 00414DB0, `009F1CC0` 00BF7030, `009F1D0F` vtable[5Ch], `009F1D3C` 00811A30, `009F1DB4` 00BD2F10, `009F1E5E` vtable[2Ch], `009F1E77` 0082ADC0, `009F1E94` 00417B10, `009F1F5B` vtable[5Ch], `009F1F76` 00827F70, `009F1FC3` 00811A30, `009F1FE8` 00415510, `009F2053` 00811A30, `009F2074` 00415550, `009F208D` vtable[5Ch], `009F2095` 006F2D90 |
| `009E7FC0` | `009E8080` 009E6400 (mode-4 arm), then `009E80AB` vtable[5Ch], `009E8116` 00864FD0 or `009E8129` 009E6120 and `009E8130` 00864BA0, `009E81A7` 009E5DA0 |
| `009E6E80` | `009E6E8F` 00954940, `009E6F03` vtable[5Ch], `009E6F11` 00827F70, `009E6F3C` vtable[5Ch], `009E6F6E` 00778890, `009E701E` vtable[5Ch], `009E706A` 00419010, `009E70A7` 00778890, `009E711D`/`009E7199` 00BD2F10, `009E71AE` 00952530, `009E71EF` 009523C0, `009E721A`/`009E722D` 00955A40, `009E726E` 00419010, `009E727F` 00415550, `009E6FCB` 00811A30, `009E731B` 0080DF40, `009E7338` 00415510, `009E73C0` 00419010, `009E74B7` 009E6870 |
| `009E9190` | `009E9209` 00BD2F10, `009E9220` 008053C0, `009E9253` vtable[5Ch], `009E9280`/`009E9290` 00414DB0, `009E9342` 00BF681B, `009E935D` 009E8360, `009E9381` 009E7F60, `009E938C` 009E8DC0, `009E93EE` 00414DB0, `009E94C4` 009E6170, `009E950F` 009E6240, `009E9598` 00BF65AC, `009E95FA` 00BF701A, `009E9642` 0042B2F0, `009E9665` 00419010, `009E968D` 00438B10, `009E96B4` 00419010 |
| `009E74D0` | `009E7609` 00438B10, `009E7646` 00419010, `009E7678` 00438B10, `009E76B2` 00419010 |
| `009E76D0` | `009E76F4` 00605070, `009E7755` 009E6640, `009E7C6D` 00438AA0 (discarded), `009E7CB4` 00414DB0, `009E7CD1` 00413920, `009E7DB2` 00BF701A, `009E7E0D` 00438B10, `009E7E68` 00419010, `009E7E97` 00438AA0, `009E7ECB` 009E5E90 |
| `009E6A90` | `009E6AB5` vtable[50h], `009E6ABB` 00438B10, `009E6B12` 00419010, `009E6B85` 009E5E00, `009E6C2B` 00419010, `009E6C3C` 00778890, `009E6C86` vtable[234h], `009E6CE2` 00419010, `009E6DBF`/`009E6E2A` 00415620, `009E6DD0`/`009E6E3B` 00415510 |
| `009E5E90` | `009E5ECC` 00BF7420, `009E5EEF`/`009E5F16`/`009E5FE3`/`009E6025` 00438B10, `009E604B` 00438AA0 |
| `009E5E00` | `009E5E16` vtable[5Ch] |
| `009D68B0` | `009D68EC` 009D6550, `009D695A`/`009D69A6` 00BF7030, `009D69DD` 00BD2F10, `009D6A8A` 004F47B0 |

## Coverage

| routine | coverage |
| --- | --- |
| `009F3090` nested update | complete (`009F3090-009F30E3`, every instruction) |
| `009D68B0` circle tangent | complete (`009D68B0-009D6AC1`) |
| `009E7FC0` score reset | complete (`009E7FC0-009E82F8`) |
| `009E74D0` bearing and evade | complete (`009E74D0-009E76CE`) |
| `009E6A90` throttle limiter | complete (`009E6A90-009E6E78`) |
| `009E6E80` standoff range | complete (`009E6E80-009E74CD`) |
| `009E9190` avoidance | complete (`009E9190-009E9736`) |
| `009E5E90` commanded heading | complete (`009E5E90-009E605A`) |
| `009E5E00` target accessor | complete (`009E5E00-009E5E29`) |
| `009E76D0` ring scan | partial: the 6-way unrolled accept/reject body `009E78CC-009E79AA` and the 8-way unrolled winner scan `009E79E5-009E7B7E` were read in their first and last unrolled step only. The stride `4Ch` and the loop control at `009E799E` and `009E7BBD` were checked, and the non-unrolled tail at `009E7BE0` matches. |
| `009F1BC0` frame state | partial: read `009F1BC0-009F1DBF`, `009F1E16-009F1F47`, `009F1F7F-009F2124`. **Not read**: `009F1DBF-009F1E16`, `009F2124-009F2216`, `009F221C-009F237B`, `009F2395-009F26EC`, `009F270A-009F3083`. Four further stores to the approach point at `009F2216` (from `006AC5D0`), `009F237D` (from `006F3AF0`), `009F23B5` (the goal vector again) and `009F26F0` (an accumulated point) were located by their call sites but their arms were not read. |
| `009E6400`, `009E5DA0`, `009E6870`, `009E6640` slot scorers | not read; modelled as host calls. They are the only candidates for the producer of `slot+40h`. |
| `009E6120`, `00864FD0`, `00864BA0` zone tests | not read; modelled as host calls |
| `009E8360`, `009E6170`, `009E6240`, `009E7F60`, `009E8DC0` traffic records | not read; modelled as host calls |
| `00952530`, `009523C0`, `00955A40`, `00954940` curve objects | not read; modelled as host calls |
| `009D6550`, `004F47B0` tangent helpers | not read; modelled as host calls |
| `00811A30`, `00827F70`, `0080DF40`, `008053C0`, `006F2D90`, `0082ADC0`, `00417B10` | not read; modelled as host calls |

## Corrections

| was | is | evidence |
| --- | --- | --- |
| docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md Uncertainties: "`sub+11E8h`, which sets the approach throttle's bias, is seeded to zero by `009E5530` and written by the unread nested update. Its play-time value is unknown." | `sub+11E8h` is `nested+11E0h`, the planar range from the unit to the attackmove destination, rewritten every frame by `009F1BC0`. `009F3240`'s throttle is therefore `2 * ([unit+494h] + 500 - range)` clamped to `[0, 1000]`. | `009F1C45` reads the unit world x/z at `+0FCh`/`+104h`; `009F1C6A`/`009F1C74` read `brain+0B2Ch`/`brain+0B34h`; `009F1C7E..009F1CAC` forms `dx*dx + 0*0 + dz*dz` with a float store; `009F1CBA` compares against `1e-10` (`00CE3820`) with `FCOMI`/`JBE`; `009F1CDE` stores the `00BF7030` result to `nested+11E0h`. |
| docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md, the `009F3240` section: the follow-up packet is "where the approach sub-state's goal, heading and throttle at `sub+1214h`/`+1218h`/`+1230h`/`+1238h` are produced" | Three of the four are, but the commanded heading `sub+1214h` is not produced by any of `009F3090`'s seven callees: `009E5E90` writes it, and `009E76D0` calls that at `009E7ECB`. | Every store to a nested field in the seven callees was enumerated; none targets `nested+120Ch`. `009E5E90` writes it at `009E5F4C`, `009E602A` and `009E6050`; `009E6A90` reads it back at `009E6A98`. |
| docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md Follow-up packets: "Without it `009F3240` has no approach point of its own." | The approach point is normally the attackmove destination copied verbatim; the nested update's own product is the bearing and the throttle. | `009F1F10..009F1F3D` copies `brain+0B2Ch/0B30h/0B34h` into `nested+1228h/122Ch/1230h`. The displaced arm at `009F1E6B` runs only when `[target+740h]` exists and `[[unit+538h]+570h] < targetzone->vtable[2Ch]()` (`009F1E60 CMP EAX,EBX` then `JGE 009F1F10`). |
| This packet's own first reading of `009E74D0`: "`nested+11F8h` is a ring slot index, so the wrap at `009E756E` and the reset at `009E75AD` are live." | `nested+11F8h` is a bearing in `(-pi, pi]`. Both branches are unreachable. | `009E7C19 IMUL ESI,ESI,0x4C` then `009E7C1C MOVSS XMM1,[ESI + EBP + 0xC]`: `nested + i*4Ch + 0Ch` is `slot+8h`, the bearing `009E5530` writes at `009E56F9`. `009E7C28` stores it into `nested+11F8h`; `009E7EA6` stores an `atan2` heading there on the other arm. |

## Follow-up packets

- `ship_ai_approach_slot_scorers`: `009E5DA0`, `009E6640`, `009E6870` and `009E6400`, the four
  per-slot routines that fill `slot+18h`. One of them must be the producer of the blocked byte at
  `slot+40h`, which `009E5E90`'s ring walk reads and which nothing in this packet ever sets.
- `ship_ai_approach_frame_state_tail`: the unread ranges of `009F1BC0` listed in Coverage,
  including the four other producers of the approach point and the writer of mode 2.
- `ship_ai_approach_curve_objects`: `00954940`, `00952530`, `009523C0` and `00955A40`, the two
  curve objects at `nested+12C0h` and `nested+13B0h` that `009E6E80`'s 119-step scan samples and
  that `009E5530` also builds.
- `ship_ai_approach_traffic_records`: `009DF8F0`, `009E8360`, `009E6170` and `009E6240`, the list
  at `nested+14A0h` and the `0x124`-byte record `009E9190` allocates.
- `ship_ai_nav_circle_tangent` (already open): `009D6550` and `004F47B0`, the two callees of
  `009D68B0` this packet modelled as host calls.

## no_ghidra_function

none. Every address this packet named is the start of an existing Ghidra function, and every call
site in `reports/ship_ai_approach_update.json` was checked against the live bodies by
`python tools/verify_report_calls.py` (99 rows, 0 failures).

## Uncertainties

- Every float expression here was transcribed from the listing, but the image computes in x87
  80-bit registers with double memory operands. The C++ projection uses `double` and rounds to
  float at the same stores the image does; the two agree to float precision on ordinary inputs and
  are not bit-identical in general. Nothing in this packet is fixture-tested against the image.
- `tune = [brain+0AB0h]` is read at eight offsets by five routines here and has no producer. The
  header's names for those fields are roles inferred from use, not recovered meanings.
- `unit+538h`'s class is still open. This packet adds the avoid-zone evidence above but read
  neither its constructor nor its vtable.
- `nested+1218h` is zeroed by `009E7FC0` and has no reader anywhere in this packet.
  `nested+1254h`, `nested+1274h..1279h`, `nested+127Ch..12A0h` and `nested+12B4h..12BDh` are
  written by routines here and read mostly by the unread slot scorers.
- `009E9190`'s candidate walk is a `std::list` traversal with debug checks (`00BF6713`). The
  projection models it as an indexed walk over a host; the order is the list's, not an array's.
- No run-time evidence was gathered. `bsp_game.exe` implements no host from this packet, so none of
  these paths was exercised in a run log.

## Correction from docs/SHIP_AI_RING_SCAN.md

Packet `cc_ai_ring_scan` read `009E76D0` whole (both unrolled bodies) and its four slot scorers:
`009E6640` is a planar obstacle probe owning the blocked byte at `slot+40h` and the probe fields
`slot+44h`/`+48h`, `009E6870` writes `slot+30h` as the distance from the nearer edge of the
standoff arc, `009E6400` writes `slot+2Ch` on the mode-4 arm as a beam preference, and `009E5DA0`
asks the ship class to rate a bearing through `0095EB40` (unread). The accept/reject body starts
at `009E7822`, not `009E78CC`, and three field meanings this doc had seen from one side are
corrected in that doc's Corrections table; the two accessors `009E5E70` / `009E5E80` read the
commanded heading and throttle.
