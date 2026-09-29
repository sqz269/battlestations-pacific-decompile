# The hull aim point: what slot +100h samples, and why the "miss" was never a miss

Packet `cc8_hull_aim_point`. Addresses: `009FADA0`, `009FA260`, `009FB200`,
`00816650`, `0042D810`, `0042BB20`, `00419010`, `00BD2F10`, `0093A570`.
Reconstruction: `include/bsp/approach_target_ref.hpp`, `src/approach_target_ref.cpp`.
Ghidra was read-only for the analysis in sections 1-4. Every name is a
hypothesis, not a recovered symbol.

This continues `docs/DIVE_BOMB_AIM_POINT.md` and answers the question that
document left open: what the target-reference sub-object's `vtable[+100h]`
samples, and whether the offset it returns is big enough to explain the
10-60 m the per-bomb census measures.

The short answer is that it is far MORE than big enough, and that changes what
the census number means.

## 1. The ABI, taken from the listing

The dispatch is `009FA272`-`009FA2A0`. The receiver is `ECX`, loaded from
`sub+18h` at `009FA266`; the decompiler drops it.

```
float3* __thiscall slot100(float3* out,            // [ESP+4]  a local vec3
                           const float3* spread,   // [ESP+8]  &sub+48h
                           const float3* centre,   // [ESP+0Ch] &sub+54h
                           float section_chance,   // [ESP+10h] sub+64h
                           float w0, float w1, float w2);  // sub+68h/+6Ch/+70h
                                                   // RET 1Ch, returns `out` in EAX
```

`RET 0x1C` on both implementations confirms the seven stack dwords. The four
floats are staged by `SUB ESP,10h` at `009FA278` and the three pointers by the
pushes at `009FA296`-`009FA29F`.

**A correction to the packet brief.** `009FA260` does not only store the
returned point. `009FA2B3`-`009FA2CB` then ADDS the vector at
`sub+34h`/`+38h`/`+3Ch` to it componentwise. The offset is `pick + bias`.

## 2. Exactly two implementations, and who gets which

Found by scanning `.rdata` for each as a literal dword
(`bsp.py scan-bytes ... --limit 4000`), and each vtable base then confirmed by
its own literal store in a constructor, per the standing RTTI rule.

`0042D810` — **the origin.** `MOV EAX,[ESP+4]`, `XORPS`, three `MOVSS` zeroing
`out[0..2]`, `RET 1Ch`. Every argument ignored. Used by dozens of vtables,
including the plane unit instance: `00D05F20+100h` = `0042D810`, and `00D05F20`
is stored at `007CFD78` inside `007CFD20 BSP_PlaneUnitInstance_Construct`.
**For all of those classes, aiming at the target's origin is correct behaviour
and never was a defect.**

`00816650` — **the tapered hull box.** Used by EXACTLY NINE vtables:

| vtable base | slot +100h at | constructor that stores it |
|---|---|---|
| `00CF90B0` | `00CF91B0` | `006DFC90` |
| `00CFA778` | `00CFA878` | `006EB160` |
| `00CFB738` | `00CFB838` | `006FB300` |
| `00CFC3D0` | `00CFC4D0` | `006FE460 BSP_UnitInstance_Construct` |
| `00CFFA30` | `00CFFB30` | `0074BB00` |
| `00D01630` | `00D01730` | `00758150` |
| `00D09678` | `00D09778` | `0081ED40 BSP_UnitVehicleBase_Construct` (`0081ED80`) |
| `00D0BF80` | `00D0C080` | `00852F10 BSP_SubmarineUnit_Construct` |
| `00D0C648` | `00D0C748` | `00857CD0` |

## 3. What `00816650` computes

Two paths. `00816659` tests `section_chance` against `00D7A218` = 0.0 and
`00816687` tests it against a `Uniform(0,1)` draw; failing either goes to the
fallback.

**(a) The named-section path, `00816669`-`00816815`.** A weighted choice among
three points on the ship, a 16-byte-strided array at `+A68h`/`+A78h`/`+A88h`
with validity bytes at `+A74h`/`+A84h`/`+A94h`. Each weight survives only if it
is positive, its byte is set, and `0093A570 BSP_ShipSectionList_Contains` on the
vector at `unit+A20h` does NOT find its id (8, 6 and 5 respectively) — a
destroyed section drops out of the draw. The total must clear `00D7A268` = 1e-4.

This path is **live code but dead under the seed**: `009FB2F9` stores
`00D7A260` = **-1.0** into `sub+64h`, so `00816659` sends every call straight
past it.

**(b) The fallback, `00816820`-`00816985` — the path that actually runs.**
`ESI` becomes `[unit+538h]`, the authored vehicle class descriptor. With
`c = *centre`, `s = *spread`:

```
rx = Uniform(-s.x, +s.x)      00816883
ry = Uniform( 0,   +s.y)      008168A0      <- one sided
rz = Uniform(-s.z, +s.z)      008168D5
R  = 00419010(0.6, 1.0, 1.0, 0.1, |rz|)     00816941

out.x = c.x + R * rx * 0.5  * [class+A4h]   // 0081694A, 0081695D
out.y = c.y +     ry * 0.25 * [class+A8h]   // 008168E8 (00D7A348 = 0.25)
out.z = c.z +     rz * 0.5  * [class+A0h]   // 0081684A, 008168EC
```

`00419010 BSP_Math_InterpolateClamped` is
`clamp(b + (d-b)(x-a)/(c-a), min(b,d), max(b,d))`. With `(0.6, 1.0, 1.0, 0.1)`
that is `R = 1.0` for `|rz| <= 0.6`, falling linearly to `0.1` at `|rz| = 1.0`.

`docs/VEHICLE_CLASS_FIELDS.md:86-88` names those three class fields from their
Lua readers: **`Length` -> `+A0h`** (`0096038B`), **`Width` -> `+A4h`**
(`00960354`), **`Height` -> `+A8h`** (`009603C2`).

So the along-hull axis is `z` and is scaled by half the **Length**; the
across-hull axis is `x` and is scaled by half the **Width**, tapered by how far
along the hull the point landed. **Full beam over the middle 60% of the ship,
pinching to a tenth of the beam at bow and stern. That is a ship planform, not
a box.**

### 3.1 A naming defect this corrected in an existing file

`include/bsp/gun_bot_remainder.hpp` already bound `00816650` as
`ship_lead_point_00816650` (packet `cc2_gun_bot_remainder`), but its
`LeadAimHullExtents` named `+A0h` `width` and `+A4h` `length` — swapped against
the authored keys, and against `GameVehicleClassRow` in
`include/bsp/game_hosts_lua.hpp`, which had them the right way round all along.
Only the names were wrong; the offsets each axis reads were correct, so the
sampled geometry never changed. It mattered because with the old names the
routine read as tapering the *length* by how far out the *width* draw landed,
which is geometric nonsense. Fixed in this packet, with the two use sites
swapped to match so the arithmetic is identical.

## 4. The result that decides the behaviour: the point is drawn ONCE

`009FADA0`'s cadence, `009FAE1C`-`009FAEB8`:

```
sub+60h += dt                                   009FAE26
if (sub+60h > 2.0f)                             009FAE38, 00CE3958
    sub+60h = Uniform(-0.5, +0.5)               009FAE61, 00CE69D0 / 00CE3800
    local = offset(sub+28h) - bias(sub+34h)     009FAE6C-009FAE91
    if (!target->vtable[+104h](local))          009FAEA3
        009FA260()                              009FAEAB
if (sub+41h) 009FA260()                         009FAEB0-009FAEB8
```

So the timer's period is `2.0 - Uniform(-0.5, 0.5)` = 1.5 to 2.5 s, as expected.
But the expiry does not re-pick by itself — it asks slot `+104h` whether the
current point is still acceptable, and only re-picks when the answer is **false**
(`009FAEA5 TEST AL,AL` / `JNZ` skips the re-pick on true).

`+104h` is handed the point **by value**: `009FAE69 SUB ESP,0Ch` *is* the push,
and there is no pointer push before the call. `RET 0Ch` agrees.

**All nine of the hull-sampling vtables carry `0042BB20` at `+104h`.** Read as
raw bytes, because Ghidra has no function there:

```
0042BB20  b0 01 c2 0c 00      MOV AL,1 ; RET 0Ch
```

It always answers true. **Therefore no ship class in this image ever refuses its
aim offset, and the timer never forces a re-pick.** The offset is drawn once, by
the dirty byte `sub+41h` the constructor sets at `009FB272`, and stands for the
life of the sub-object. One random hull point per attack run, not a point that
wanders every two seconds.

On the target's death (`target+5Dh`, `009FADB5`) the path `009FADAE`-`009FAE18`
takes one last sample through the same transform, unregisters the observer pair
(`006952A0`) and clears `sub+14h`/`+18h`/`+41h`, freezing the point.

## 5. The seeds, from `009FB200`

| field | seed | store |
|---|---|---|
| `sub+48h` spread | `(0.9, 0.5, 0.9)` — `00CE3860`, `00CE3800`, `00CE3860` | `009FB276`/`7B`/`80` |
| `sub+54h` centre | `00F87574`/`78`/`7C` | `009FB2C9`-`009FB2D5` |
| `sub+34h` bias | the same three | `009FB2AF`-`009FB2C3` |
| `sub+64h` chance | `-1.0` (`00D7A260`) | `009FB2F9` |
| `sub+68h`/`6Ch`/`70h` weights | `1.0` (`00D7A24C`) | `009FB2FE`-`009FB310` |
| `sub+60h` timer | `Uniform(0, 2.0)` (`00CE3958`) | `009FB2E7` |
| `sub+41h` dirty | `1` | `009FB272` |
| `sub+44h` | `0.0` | `009FB25F` |

`00F87574` is `.data` past raw size, so on disk the centre and the bias are
`(0,0,0)`. Per the standing rule that says nothing about a runtime writer; none
was found, and this is recorded as unresolved rather than as proof.

The spread means the draw reaches 90% of the half-extents in `x` and `z`, and
0 to 50% of the quarter-height in `y`. With `s.z = 0.9`, `R` ranges over
`[0.325, 1.0]`.

### 5.1 The scale, and what it does to the census number

For a hull of Length `L` and Width `W` the offset spans

```
along  the hull:  +/- 0.45 * L
across the hull:  +/- 0.45 * W  (at most; less near the ends)
up:               0 .. 0.125 * H
```

On a 180-270 m hull that is **±81 to ±121 m along the ship**. The per-bomb
census measures 11.3-25.5 m of "miss vs target at impact" on a stationary
target. That is not merely explained by a hull offset — it is an order of
magnitude SMALLER than the offset the image draws.

**So the framing in the handoff was wrong, and this packet retracts it.** The
census number was never a miss to be explained away by a hull point. A bomb
landing 25 m from the origin of a 180 m ship is a hit amidships. The image
deliberately spreads its aim over the whole hull, which is what makes a
squadron's bombs walk along a ship instead of all converging on one spot. The
mean of the draw is the centre — the sampler **scatters** the aim, it does not
**bias** it.

## 6. The binding

`include/bsp/approach_target_ref.hpp` / `src/approach_target_ref.cpp` bind the
constructor seeds, `009FA260`'s pick-plus-bias, `009FADA0`'s cadence and the
death freeze. They REUSE what was already recovered rather than restating it:

* `bsp::ship_lead_point_00816650` and `bsp::entity_lead_point_0042d810`
  (`include/bsp/gun_bot_remainder.hpp`) for slot `+100h`;
* `bsp::transform_point_004142e0` (`include/bsp/camera_affine.hpp`) for the
  `009FAEDF` transform by the target's `+CCh` matrix;
* `00419010` and `00BD2F10` are already bound inside those.

Both tasks are fed from it in `src/game_hosts_units.cpp` (edited under the
integrator's hunk arbitration of 2026-09-19): the dive-bomb `tp` and the
torpedo `approach_target_point`. A target whose class is not
`kVehicleClassIsShipKind` keeps the origin, which is what `0042D810` does.

### 6.1 The one labelled substitution

`00816650` draws from `00BD2F10` on stream `ECX=1`, whose state this process
does not carry. The host's usual stand-in — `random_between(lo, _)` returns
`lo` — is **wrong here**: it would peg every attack at the stern-port corner of
the hull rather than sampling it. The mean is wrong the other way: it collapses
to the centre, which is the behaviour the binding exists to replace.

`approach_target_ref_unit_draws_substitute` is therefore a counter-based integer
hash giving a reproducible draw per approach. Distributionally faithful,
identical run to run so the census lines stay comparable, and **not** the
image's sequence. Labelled as such in the header.

## 7. Predictions, written before the runs

Base `ec14870c3`, same-binary before/after pairs on this tree.

1. **The dive-bomb per-bomb `miss vs target AT IMPACT` will grow by roughly an
   order of magnitude**, from the 11.3-25.5 m of the before run to tens of
   metres, bounded by `0.45 x Length` of the bombed hull.
2. **The growth will be overwhelmingly in the ALONG-course component.** The
   across component is bounded by `0.45 x Width` and tapers near the ends, so it
   should stay in the low tens of metres at most.
3. **Each aircraft's offset is constant for the whole run.** Nothing should
   destabilise the release geometry: release range and fall time should move
   only as far as the shifted aim point moves them, not wander tick to tick.
4. **The USN01 torpedoes should still hit the Northampton.** Their 8.5 / 9.1 m
   centre-to-centre closest approach should move ALONG the hull by the drawn
   offset and stay a hull hit, because the draw is inside the hull by
   construction.
5. **Null result to watch for:** if a target's authored `Length` is 0 the offset
   collapses to the bias and nothing moves at all. A run where no census number
   changes means the class row was not read, not that the binding is inert.

## 8. Measurements

Recorded in section 9 as each pair lands.

## 9. Re-pair on the faithful dive set (packet `cc9_dive_aim_hull_point`, cc9-lua16, 2026-09-29)

Every earlier verdict on `kHullAimOffsetEnabled` (23 -> 8 releases, then 29 -> 0 and 23 -> 0 in
docs/DIVE_THROTTLE.md and docs/FLYOVER_SPEED.md) was taken before the rest of the dive was bound.
Since `e1a95859b` the aimdive tail, the goaway and aimglide throttles, the aimglide pitch and yaw
and the fly-over speed are all ON, the host throttle moves, and cc9-lua15 measured the profile as
faithful: on USN04 4700/4500 every Val closes the in-range latch and passes the turndown, and the
releases are lost to anti-aircraft fire in the dive (7 of 16), row length (3), overshoot (2) and
three wingmen leaving the fly-over by 0.1-0.2 degrees at `009C66E3` (docs/DIVE_BOMB_TASK.md, the
last two subsections). The origin feed is now the only labelled divergence on that path. So the
switch is re-paired unchanged: same code, same labelled draw substitute (section 6.1), flipped on a
clean export of this branch.

### 9.1 Predictions, written before the runs

USN04 4700/4500, reference environment, same-binary OFF/ON pair.

1. **Mechanism.** One `hull_aim draw` line per (attacker, target) pair, never a second one for the
   same pair (the `+104h` slot is `0042BB20`, always true). Offsets along the hull up to
   `0.45 x Length`, across up to `0.45 x Width`, up 0 to `0.125 x Height`.
2. **The fly-over leavers.** The leave is a 0.1-0.2 degree margin against the bearing to the
   three-second lead point, and at spans of 0-30 m a tens-of-metres move of the point swings that
   bearing by far more than 0.2 degrees. Prediction: the leaver set is NOT {#1.1|.-2, #1.1|.-4,
   #5.1|.-4}; which way each goes is not predicted. Leavers that remain still miss by under one
   degree.
3. **Overshoot at the drop floor.** The aim error is measured against the fed point, so the two
   overshooters (#3.1, #3.1|.-2) take a different final error. Not predicted to fall under 25 m.
4. **Releases.** Anti-aircraft deaths in the dive are the dominant loss and the switch does not
   touch them, so the Val releases stay in 0-4 (OFF: cc9-lua15 measured 1). The earlier collapse
   to 0 is NOT predicted, since the profile it came from is gone. A collapse again (ON below OFF
   with the same aircraft alive at the drop floor) is a mechanism failure and keeps the switch OFF.
5. **Hits.** A released bomb lands within the hull by construction, so every release that hit OFF
   hits ON, give or take the ship's motion over the fall.
6. **Torpedoes.** The torpedo `approach_target_point` is fed from the same point; the 2 of 16
   torpedo releases may move by one either way.

### 9.2 Measured, and the verdict

One commit, `25236960b`, exported twice by `tools/pair_export.py` (OFF `4EA0C9D0FEEC`, ON with
`kHullAimOffsetEnabled=true` `96394B734AE6`). Reference environment
(`BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`, lockstep 0.05, `--press-start-frame 30`). A 300-frame
USN04 smoke of the ON binary ran first and finished clean. Logs `local\l16_{off,on}_{usn04,usn01,usn13}.log`
in the cc9-lua16 tree; the per-aircraft census is cc9-lua15's `local\l15_dive.py`.

| row | pair_diff | deaths | dive-bomb releases | torpedo releases | damage |
| --- | --- | --- | --- | --- | --- |
| USN04 4700/4500 | 3 | 46 -> 45 | 1 of 19 -> 1 of 19 | 4 of 16 -> 5 of 16 | 13297.8 -> 14292.0 |
| USN01 3200/3000 | 3 | 5 -> 5, same set | 2 of 2 -> 2 of 2 | 0 of 5 -> 0 of 5 | 2786.4 -> 2786.4 |
| USN13 3200/3000 | 3 | 31 -> 32 (+ Kate `bruh #1.5|.-4`) | - | 0 of 60 -> 0 of 60 | 9669.9 -> 9680.0 |

Against the predictions:

1. **Mechanism: held.** USN04 prints 19 `hull_aim draw` lines, exactly one per (Val, target) pair,
   none repeated. Both targets are 250 x 30 m hulls (Lexington-class01, Yorktown-class01). The
   along-hull offsets run from -111.8 to +109.0 m, inside the 112.5 m bound (`0.45 x 250`). The
   across offsets run from -12.9 to +12.0 m, inside 13.5 m. OFF prints none.
2. **The leaver set: held.** OFF leavers are #1.1|.-2, #1.1|.-4 and #5.1|.-4, the same three
   cc9-lua15 found. ON leavers are #1.1|.-3, #1.1|.-4 and #5.1|.-2. The count stays three, with two
   of the three changed. The miss margin of the ON leavers was not measured, because it needs the
   `kHullAimTrace` build.
3. **Overshoot: missed, and in the favourable direction.** The two OFF overshooters, #3.1 (final
   error 116.0 m) and #3.1|.-2 (64.6 m), end ON at -18.3 m and -15.3 m, inside the 25 m gate. They
   are then shot down above the drop floor, at 439 m and 550 m.
4. **Releases: held, no collapse.** USN04 still has one dive release, from a different aircraft
   (#3.1|.-3 OFF, #7.1 ON). The earlier 23 -> 8 and 29 -> 0 collapses do not recur on the bound
   dive.
5. **Hits: unresolved on USN04, held on USN01.**
   - On USN04, the OFF bomb lands at the sea surface 45.3 m from York-class02, so it misses. The
     ON bomb is released late in the row (its drop line is log line 62075 of about 64,600) and
     no impact line follows before the row ends, so neither side hits.
   - USN01's two releases give identical damage in both builds.
6. **Torpedoes: held.** USN04 torpedo releases go from 4 to 5, and the other rows do not move.

The moved death rows are timing and killer changes on aircraft whose paths changed. The only
set changes are the ones that follow from the leaver swap:
- #5.1|.-2 leaves the fly-over ON and survives;
- #5.1|.-4 dives ON and is shot down;
- Zero #7.2 is shot down OFF only.

**Verdict: `kHullAimOffsetEnabled` flips ON.** The mechanism matches the image. The one missed
prediction (3) is a spread effect, not a failure of the mechanism. The origin feed was the last
labelled divergence on the dive path, apart from the labelled draw substitute (6.1). That
substitute stays.

## Correction from packet `cc9_approach_target_lead`: `sub+44h` is "projtime", and the point leads

cc9-planes1, 2026-09-29. This document's `sub+44h = 0.0` row (seeded at `009FB25F`) is true only at
construction.
- `009FB3E0` registers the field as "projtime".
- The torpedo approach writes it as `approach+F8h`, its engagement estimate (`009D3D2F`/`52`/`65`).
- The dive-bomb approach writes it as `approach+74h`, `tf + approach+C8h` (`009C7D65`/`7E72`/`7E85`).

When it is positive, `009FADA0`'s tail (`009FAF05`-`009FAF7D`) adds the target's predicted
displacement, `008120E0` over projtime, to the hull point. So the aim point is the hull point plus a
lead. The details and the switch are in `docs/TORPEDO_AIM_LEAD.md` section 11.
