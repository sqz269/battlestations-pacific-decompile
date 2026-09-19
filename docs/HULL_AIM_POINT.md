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
