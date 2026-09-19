# `009FD570`, the shared fly-to-a-point solver, read whole

Packet `cc8_flyto_solver_and_goaway`, owner `agent/cc8-plane-squadron`.
Addresses: `009FD570` (whole), `009D0C10` (its torpedo call site, re-read for the frame),
`009D0D90` enter tail `009D0E3A`-`009D0F04`, `009D0F10` tick.

`009FD570` is not a bearing helper. It is a **standoff-ring controller**: given a point, a unit, a
ring radius and the unit's current range, it returns the compass heading that flies the unit
around, away from or in toward that point, with an obstacle-avoidance term and a map-edge term
folded in. The six callers are five bot-state geometry updates plus the torpedo break-off.

## 1. ABI, from the listing

| item | value | evidence |
| --- | --- | --- |
| body | `009FD570`-`009FDEDE`, 2414 bytes | `bsp.py ghidra proto` |
| return | `RET 0x18` at `009FDEDC`, the **only** `RET` in the body | `rg 'RET' ` over the whole listing: one hit |
| stack arguments | **six** | from the `RET 0x18` cleanup, not from counting pushes |
| receiver | `ECX`, saved to a local at `009FD5A2` and reloaded at `009FD749` | `MOV [ESP+0x18],EDI` / `MOV EDI,[ESP+0x1C]` |
| result | a `float` in `ST0`, already wrapped into `[0, 2pi)` | `009FDEB0`-`009FDEC4` |

```
float __thiscall solve(Cache* cache,          // ECX
                       const Vec3* point,     // arg1, E+04h
                       Unit* unit,            // arg2, E+08h
                       float standoff,        // arg3, E+0Ch
                       float* side,           // arg4, E+10h   IN AND OUT
                       float range,           // arg5, E+14h
                       float offset_scale);   // arg6, E+18h
```

The argument *widths* are settled by the loads, not by the call sites: `009FDB39 FLD float
[ESP+0xD0]` is arg5, `009FDB40 FSUB float [ESP+0xC8]` is arg3 and `009FDB5F FMUL float [ESP+0xD4]`
is arg6, all four-byte; `009FDA87 MOV EAX,[ESP+0xC8]` and `009FDBA8 MOV ESI,[ESP+0xCC]` are arg1
and arg4 as pointers. The `[ESP+n]` bases differ by region because `POP EBP` at `009FDAE8` and
`POP EBX` at `009FDAED` re-base the frame **in the middle of the body**:

| region | `ESP` | arg1 at | independent check |
| --- | --- | --- | --- |
| `009FD5E7`-`009FDAE7` | `E-0xC4` | `[ESP+0xC8]` | `009FD714 MOV [ESP+0xC4],0` and `009FD728 MOV [ESP+0xC0],-1` are the MSVC EH trylevel at `E-4`, set to 0 around a temporary and back to -1 after `0064A610` destroys it |
| `009FDAEE`-`009FDEA3` | `E-0xBC` | `[ESP+0xC0]` | the arg3/arg5/arg6 float loads above land on three consecutive slots |

## 2. `approach->vtable[0]` takes **no** arguments, and this is a retraction

`docs/TORPEDO_AFTER_THE_DROP.md` section 3.5 reads the torpedo call site as

> `009FD570(state, approach->vtable[0](&scratch, unit, state+24h, &state+2Ch, approach+90h, 0.8))`

and adds "the **side is passed by pointer** [to `vtable[0]`], which means `vtable[0]` may write it
back - a detail a by-value reconstruction would lose."

* **was**: `vtable[0]` takes a return buffer plus five arguments; `009FD570` takes one.
* **is**: `vtable[0]` takes **only** its return buffer (`RET 4`) and `009FD570` takes **six**. The
  five values are `009FD570`'s arguments 2-6, staged *before* the `vtable[0]` call and left in
  place across it.
* **evidence**, the stack balance of `009D0C10`, which has to close at its `POP ESI` / `ADD
  ESP,0x3C` at `009D0D83`. Walking backwards from there with each callee's own `RET imm`
  (`007F0280` `RET 0x18`, `00438B10` and `00438AA0` `RET 8`, `0074E260` bare `RET`) puts `ESP` at
  `S-0x18` immediately before `CALL 009FD570`, where `S` is `ESP` after the prologue's `PUSH ESI`.
  The six pushes before `CALL EAX` put `ESP` at `S-0x18` too, and `PUSH EAX` sits between them, so
  `vtable[0]` popped exactly **four** bytes. The layout that follows is arg1 `= EAX` at `S-0x18`,
  arg2 `= unit` at `S-0x14`, arg3 `= state+24h` at `S-0x10`, arg4 `= &state+2Ch` at `S-0x0C`,
  arg5 `= approach+90h` at `S-0x08`, arg6 `= 0.8` at `S-0x04`.
* **second, independent evidence**: `009C47D0`, the dive bomb's geometry update, is the same
  instruction sequence with four values changed (`FLD1` for `0.8`, `approach+BCh` for
  `approach+90h`, `state+20h` for `state+24h`, `&state+18h` for `&state+2Ch`). Two call sites with
  the same shape and a `RET 0x18` agree on six.

The **conclusion** of 3.5 survives: `side` *is* an in/out pointer - just `009FD570`'s, written at
`009FDC48`. `009D3517` / `009D36E4` / `009D3DC8` already name `approach->vtable[0]` as the target
point (`TorpedoApproachHost::approach_target_point`), so arg1 at the torpedo site is **the ship
being attacked**.

## 3. What it computes

```
lead   = unit.position + 3.0 * unit->vtable[34h]()          009FD5A8 .. 009FD61F
avoid  = obstacle term over the cached list                 009FD749 .. 009FDA7D   (section 4)
d      = lead - point, in XZ only                           009FDA8E .. 009FDAA7
n      = d / |d|                                            009FDB0F .. 009FDB25
e      = range - standoff                                   009FDB39 .. 009FDB47

if (e <= -200) {                                            009FDB59 JBE
    steer = n                                               the three FSTPs at 009FDD33 discard
                                                            the live x87 values instead
} else {
    x      = e * offset_scale                               009FDB5F
    h      = bearing(n)                                     009FDB6C .. 009FDB93
    if (avoid ran) {                                        009FDBA1 COMISS best, 0.0
        delta = SubtractWrappedAngle(h, bearing(avoid))     009FDBFA
        if (delta >  0.1 && *side > 0)  *side = -1.0        009FDC11 / 009FDC1E / 009FDC48
        if (delta < -0.1 && *side < 0)  *side = +1.0        009FDC36 / 009FDC3E / 009FDC48
    }
    offset = InterpolateClamped(-200, 0.0, 300, pi,  x)     009FDC7A
    hc     = AddWrappedAngle(h, offset * *side)             009FDC97
    g      = wrap0to2pi(pi/2 - hc)                          009FDC9C .. 009FDCBE
    weight = InterpolateClamped(-200, 1.0, 300, 4.0, x)     009FDD08
    steer  = weight * (cos g, sin g)                        009FDD11 .. 009FDD2D
}
steer += avoid                                              009FDD39 .. 009FDD54
if (GGame::NearWorldEdge(lead, 500)) {                      009FDD62, 00681F40 -> AL
    q      = GGame::EdgeOverrun(lead, 500) / 60.0           009FDD80, 009FA510, 00CE3D68
    centre = 0.5 * (worldMin + worldMax), in XZ             009FDD8F .. 009FDDC5
    steer += q * normalise(centre - lead)                   009FDDC9 .. 009FDE8B
}
return wrap0to2pi(pi/2 - atan2(steer.z, steer.x))           009FDE8F .. 009FDEC4
```

`bearing(v)` is `wrap0to2pi(pi/2 - atan2(v.z, v.x))` at all three of `009FDB6C`, `009FDBBD` and
`009FDE97`; `_CIatan2` takes `y` in `ST(1)` and every one of the three pushes `z` first. With that
convention heading 0 is `+Z` and `pi/2` is `+X`, which is the same convention as `unit+C6Ch`
(`atan2(fx, fz)`), so the result can be handed straight to `SubtractWrappedAngle` against the
aircraft's heading - which is exactly what `009D0CEE` does.

**`d` points FROM the point TO the unit**, so the bare bearing `h` is the *away* bearing, and the
offset ramp is what turns it back in:

| `e = range - standoff` | `offset` | `weight` | the heading |
| --- | --- | --- | --- |
| `<= -200` | not computed | not computed | straight away from the point, weight 1 |
| `-200` | 0 | 1.0 | straight away from the point |
| `0` (on the ring) | `0.4 pi` | 2.2 | 72 degrees off the away bearing, i.e. tangential-ish |
| `>= +300` | `pi` | 4.0 | straight at the point |

`offset_scale` (arg6) pre-scales `e`, so the torpedo's `0.8` widens the band to `[-250, +375]`
metres while the dive bomb's `1.0` leaves it at `[-200, +300]`.

### 3.1 Every constant, at the width of the instruction that loads it

| value | address | load | use |
| --- | --- | --- | --- |
| 3.0 | `00D7A2B0` | `FLD qword` `009FD5B1`; `FDIV qword` `009FDA5D` | the lead multiplier, and the avoidance divisor |
| 100.0 | `00D7A220` | `FADD qword` `009FD6DD`, `009FD8C5` | obstacle radius pad |
| 1e-10 | `00CE3820` | `FLD qword` `009FD837`, `009FDA0C` | the square-root guard |
| 1.0 | `00D7A24C` | `COMISS` `009FD8B4`; `MOVSS` `009FDC40` | the minimum obstacle range, and `+1` for the side |
| 0.7 | `00CE3E18` | `FLD dword` `009FD918` | the falloff knee |
| -200.0 | `00D21CE8` | `FLD qword` `009FDB4B` | the early-arm threshold |
| -200.0 | `00CE77E4` | `FLD dword` `009FDC71`, `009FDCFF` | the same threshold as an interpolation abscissa |
| 300.0 | `00CE3AE8` | `FLD dword` `009FDC61`, `009FDCEF` | the far abscissa |
| pi | `00D7A264` | `FLD dword` `009FDC57` | the far offset ordinate |
| 4.0 | `00CE3D34` | `FLD dword` `009FDCE5` | the far weight ordinate |
| 1.0 | `FLD1` `009FDCF9` | - | the near weight ordinate |
| pi/2 | `00CE3830` | `FSUBR qword` `009FDB79`, `009FDBCA`, `009FDC9C`, `009FDEA5` | bearing conversion |
| 2 pi | `00CE3828` | `FADD qword` `009FDB8D`, `009FDBDE`, `009FDCB0`, `009FDEBA` | the wrap |
| 0.1 / -0.1 | `00D7A3A0` / `00CE3928` | `FLD qword` `009FDC03` / `009FDC2C` | the side latch's dead band |
| -1.0 | `00D7A260` | `MOVSS` `009FD77C`, `009FDC20` | the `best` seed, and `-1` for the side |
| 500.0 | `00CE397C` | `FLD dword` `009FDD58`, `009FDD6F` | the world-edge margin |
| 60.0 | `00CE3D68` | `FDIV qword` `009FDD85` | the world-edge gain divisor |
| 0.5 | `00D7A280` | `FLD qword` `009FDDB3` | the world centre |
| 0.0 | `00D7A218` | `COMISS` `009FDBA1`, `009FDC17` | `best > 0` and `*side > 0` |

`python tools/pe_const_read.py` (added by this packet) reads them from the on-disk image at the
width the instruction asks for; `00CE3830`, `00CE3828`, `00CE3928` and `00D7A3A0` are **floats
promoted to doubles in the image**, which is why they print as `1.5707963705062866` rather than
`1.5707963267948966`.

## 4. The obstacle term, and the cache behind it

`cache+8h` is a countdown, `cache+0Ch` a container and `cache+10h` its count. `009FD5FF ADD
[EDI+8],-1` with `JNS` at `009FD623` means the world is re-scanned **every 21st call** (`MOV
[EDI+8],0x14` at `009FD630`); in between, the cached list is reused.

The rebuild (`009FD63C`-`009FD743`) walks `[[00E188A8]+19CCh]+64h` - `00E188A8` is the GGame
singleton, `docs/APP_FRAME_GAME_STATE.md` - as a `{+4h next, +8h unit}` list and keeps every unit
whose `+54h` differs from the flying unit's `+54h` and whose horizontal distance to the *lead
point* is under `max(unit+434h, +444h, +448h) + 100`.

The per-obstacle term (`009FD801`-`009FD9C4`) is

```
dist    = |lead - obstacle.position| in XZ        the y term is an explicit FLDZ / FMUL ST0
r       = max(+434h, +444h, +448h) + 100
skip unless 1.0 < dist < r                        009FD8B4 JBE, 009FD8D1 JBE
w       = InterpolateClamped(0.7, 1.0, 1.0, 0.0, dist / r)
s       = obstacle(+474h) + obstacle(+464h) + obstacle(+478h)
avoid  += s * w * (lead - obstacle.position) / dist
best    = max(best, s)
```

and afterwards, only when `best > 0`, `avoid.x` and `avoid.z` (not `avoid.y`) are divided by
`max(best, |avoid|) / 3`. So the avoidance vector pushes the steer **away from** each obstacle,
weighted by how close it is and how big it is, and is renormalised to a magnitude of about 3 - the
same order as the direct term's weight of 1 to 4.

`InterpolateClamped` is `00419010`: `(x0, y0, x1, y1, x) -> clamp(y0 + (x - x0)/(x1 - x0) * (y1 -
y0))` into `[min(y0,y1), max(y0,y1)]`, returning `y0` when `x1 == x0` (`0041901E FUCOMPI` /
`00419022 LAHF` / `TEST AH,0x44` / `JP`). The falloff call therefore reads "1.0 inside 0.7 of the
padded radius, falling to 0 at the rim".

## 5. The six callers, and how each host stands in for it today

| call site | containing function | arg3 standoff | arg4 side | arg5 range | arg6 scale | host today |
| --- | --- | --- | --- | --- | --- | --- |
| `009D0C54` | `009D0C10` `BSP_BotStateTorpedoGoAway_UpdateGeometry` | `state+24h` | `&state+2Ch` | `approach+90h` | `0.8` (`00CE74F8`, `FLD dword`) | **bound by this packet** |
| `009C4810` | `009C47D0`, the dive bomb's geometry update | `state+20h` | `&state+18h` | `approach+BCh` | `1.0` (`FLD1`) | unbound; `agent/cc8-dive-bomb` owns it |
| `007B5B2D` | `007B5AF0`, called only from `007B6240` | `state+4Ch` | `&state+48h` | `this+54h` | `1.0` (`FLD1`) | unbound, unread |
| `009A3D2D` | `009A3CF0`, a vtable-only geometry update | `state+1Ch` | `&state+20h` | `this+48h` | `1.0` (`FLD1`) | unbound, unread |
| `009B57A8` | `009B5760`, a vtable-only geometry update | not read | `&state+18h` | `this+BCh` | not read | unbound, unread |
| `009AD54C` | `009AD480`, a vtable-only geometry update | not read | not read | not read | `1.0` (`FLD1`) | unbound, unread |

The last three rows are read only far enough to confirm the shape (`SUB ESP,8` / scale / `LEA EDX,
[state+S]` / `CALL EAX` / `PUSH EAX` / `CALL 009FD570`); a cell marked "not read" is a cell this
packet did not open, not a cell that is absent. `coverage: partial` for those three; the solver
itself is `complete`.

## 6. Reconstruction

`include/bsp/plane_fly_to_solver.hpp` and `src/plane_fly_to_solver.cpp`, pure, no host interface:
the world scan is the caller's, because the cached obstacle list is a world query rather than
arithmetic. `fly_to_point_heading_009fd570` takes `side` by value and returns the possibly-updated
value plus a `side_written` flag, which is the honest shape of an in/out pointer in a pure
function. `fly_to_avoidance_009fd749` is the obstacle term on its own.

Three facts a caller must not lose:

1. `side` is read **after** the latch may have rewritten it (`009FDC7F FMUL [ESI]`), so a caller
   that copies `side` out before the call and multiplies itself gets the previous tick's hand.
2. On the `e <= -200` arm nothing reads `side`, `offset_scale` or either interpolation - the
   heading is the bare away bearing. On this installation's USN01 the torpedo goaway never leaves
   that arm: the run reports `break_off_24h=700.0` and `range_peak_in_goaway=382.2`, so
   `e <= 382.2 - 700 = -317.8` throughout.
3. The lead is `position + 3 * vtable[34h]()`. If `vtable[34h]` is the velocity the solver is
   aiming three seconds ahead of the unit; `vtable[34h]`'s own contract is **not** read here and
   the reconstruction carries the vector as an input rather than naming it.

## Coverage

| routine | coverage |
| --- | --- |
| `009FD570` | complete: every instruction of `009FD570`-`009FDEDE` accounted for, both frame regions, all 24 calls |
| `009D0C10` | complete for the frame and the argument layout (this packet); its arithmetic is `docs/TORPEDO_AFTER_THE_DROP.md` section 3.4 |
| `00419010` | complete |
| `00681F40` | complete: `NearWorldEdge(pos, margin)` over `GGame+711Ch/7124h/7128h/7130h` |
| `009FA510` | complete for the shape: the largest of the four margin overruns, seeded at 0 (`009FA545 FLDZ`) |
| `007B5AF0`, `009A3CF0`, `009AD480`, `009B5760` | partial: call-site shape only, bodies unread |

## Uncertainty

* `unit->vtable[34h]` is called with a return buffer and its result is scaled by 3.0. "Velocity" is
  the reading that makes 3.0 a lead time; it is **not proved** here.
* `unit+434h/444h/448h` and `+464h/474h/478h` are used as extents and as a size sum. Which is the
  bounding-box min and which the max is not established; the solver only needs `max` of the first
  triple and the sum of `+474h + +464h + +478h`.
* `cache+0Ch`'s container type is inferred from `007B4500(0)` (`RET 4`) clearing it and `009FCFD0`
  (`RET 4`) appending, with a `00CF5C94` vtable temporary destroyed by `0064A610
  BSP_ObservedOwnerBase_Destroy` under an EH trylevel. The element stride is `0x18` from `ADD
  EBP,0x18` and the unit pointer is at `+14h` within it; the other five dwords are unread.
* `009FA510` is read from `009FA53D` onward. The three instructions before that are not in this
  document.
