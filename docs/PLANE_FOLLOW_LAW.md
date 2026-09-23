# The plane follow law: what a wing member is actually commanded to do

Packet `cc8_follow_law`. Program `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr`.
Everything below is read from the listing; names are hypotheses, not recovered symbols.

## 1. The correction this packet owes

`docs/PLANE_FORMATION.md` section 6, `docs/BOMBER_AFTER_TASK.md` section 6 and this packet's
own brief all describe `009BFEE0` as *"the law that FLIES a member to its station — the
throttle, the heading and the GoodPosition / WaitForHdg gates"*, and list `009BEE30` as an
unread *"third step"*. That is backwards in the part that decides behaviour.

**`009BFEE0` commands nothing.** Its fifteen distinct callees are every one of them a math or
pose primitive:

| callee | n | name |
|---|---|---|
| `00414DB0` | 13 | `BSP_EntityPose_RefreshWorld` |
| `0042CF10` | 7 | `BSP_Geometry_AsinClamped` |
| `0042BE90` | 4 | (unnamed) |
| `00BF7030` | 3 | CRT `sqrt` |
| `00438B10` | 3 | `BSP_Math_SubtractWrappedAngle` |
| `00415510` / `00415550` | 2 / 1 | `BSP_Math_Min/MaxFloatByRef` |
| `00419510` | 2 | `BSP_Vector3f_Normalize` |
| `00414C60` | 2 | `BSP_Vector2f_LengthWithCutoff` |
| `00419010` | 1 | `BSP_Math_InterpolateClamped` |
| `00419260` | 1 | `BSP_Vector2f_ReciprocalLength` |
| `0042B2F0` | 1 | `BSP_Vector3_LengthFloatThreshold` |
| `00419010`, `007D7DA0`, `004F4840`, `00BF701A` | 1 each | — |

No heading, altitude or speed helper appears anywhere in its body. It is a pure producer.
`009BEE30` is the body that issues every command.

### 1.1 A method note, because it nearly cost the map

The first write census of `009BFEE0` reported that it wrote only `state+50h..68h` and never
touched the steer point. That was a tooling defect, not a finding: Ghidra renders x87 stores as
`FSTP float ptr [...]`, and the census's size-keyword pattern listed only
`byte|word|dword|qword|xmmword`. Every x87 store — which is most of this code — was invisible.
This is the `float ptr` cousin of the Capstone x87 trap already in the project memory. Fixed
before any conclusion was drawn; every census in this document is post-fix.

## 2. The tick, and the order of the three bodies

`009C1FD0 BSP_BotStateFollow_Tick`, vtable `00D20AB8` slot `+Ch`, `RET 4` on one float (dt).
`009C7270` is a five-instruction thunk onto it and `009D2720` calls it first, so a dive bomber
and a torpedo bomber in `done` / `prepare` run this same body.

```
009C1FD7  EAX = [EBP+4]          ; the approach/owner
009C1FDA  ECX = [EAX+18h]        ; the pilot command block
009C1FE2  [ECX+26Ch] = 2         ; plan mode 2
009C1FEA  CALL 009BFD70          ; the station point
009C1FF1  JZ 009C234E            ; AL = 0 means "I am the flight leader": tick ends
009C200D  JLE 009C2066           ; the abort gate on [[EBP+4]+4]+0C20h / +0C25h
009C2068  CALL 009BFEE0          ; the GEOMETRY step
009C206D  FLD  [ESP+74h]         ; the tick's OWN incoming float argument (dt)
009C2074  FSTP [ESP]
009C2077  CALL 009BEE30          ; the COMMANDING step, handed that dt
```

`[ESP+74h]` is the tick's argument, not an out-slot: `SUB ESP,68h` plus two pushes puts the
return address at `[ESP+70h]`, and `009C2063 RET 4` confirms the single stack argument.

## 3. The contract between the two steps is one float3

`009BFEE0` writes a **steer point** at `state+44h/48h/4Ch` and `009BEE30` heads at it:

```
009BF9EA  LEA  EBP,[ESI+44h]
009BF9ED  PUSH EBP
009BF9EE  MOV  ECX,EDI           ; EDI = ESI+4, which is 009F9E40's `this`
009BF9F0  CALL 009F9E40          ; BSP_PilotBot_CommandHeadingToPoint
```

Both arms of `009BFEE0` write that same point — the hold arm at `009BFFB1/B7/BD` and again
incrementally at `009C0004`-`009C001E`, the fly-to arm at `009C10F7`, `009C11D5`, `009C1222`,
`009C1328`, `009C1552` and `009C168A`. The tail `009C16D2`-`009C1846` then overwrites its Y
(`009C1811`, `009C1827`, `009C183B`) with the leader-relative clamp section 10.8 of
`docs/BOMBER_AFTER_TASK.md` already read, and writes `state+34h` at `009C17F1`.

So the brief's "commanded altitude" at `009C17F1`/`009C1811` is not a command at all: it is the
Y of the steer point and the Y of the station point, consumed one body later.

## 4. The gate, in lockstep

`state+85h` — "in good position", written by `009BFD70` at `009BFDD4`, `009BFE95`, `009BFEA2`
and `009BFEB1` from the `Pilot/Follow` GoodPositionDist / GoodPositionDir pair. **Both** bodies
branch on it with the same polarity:

```
009BFEEE  CMP byte [ESI+85h],0 / JZ 009C0026   geometry: hold arm vs fly-to arm
009BEE49  CMP byte [ESI+85h],0 / JZ 009BF9EA   commands: hold arm vs fly-to arm
```

Body sizes: the geometry's fly-to arm is `009C0026`-`009C16D1` (~1500 instructions); the
commander's hold arm is `009BEE56`-`009BF9E5` and its fly-to arm `009BF9EA`-`009BFD38`.

## 5. The fly-to arm of `009BEE30`, read in full

This is the regime that governs a member catching up and a spent bomber in `done`.

### 5.1 The tuning block base

`state+6Ch` points into the game tuning singleton at its `+380h`, so `block+NNh` is
`singleton+(380h+NNh)`. Fixed by two offsets `docs/BOMBER_AFTER_TASK.md` already established
from their use — `block+14h` GoodPositionDir, `block+18h` GoodPositionDist — landing exactly on
`singleton+394h`/`+398h`, which `docs/GAME_TUNING_SINGLETON.md` names as those two keys.

| block | singleton | key | this installation |
|---|---|---|---|
| `+00h` | `+380h` | `Pilot/Follow/FollowedPointDist` | 250 |
| `+18h` | `+398h` | `Pilot/Follow/GoodPositionDist` | 100 |
| `+24h` | `+3A4h` | `Pilot/Follow/MaxFollowSpdTargetDir` | `DEG(30)` = 0.5236 |
| `+28h` | `+3A8h` | `Pilot/Follow/MinFollowSpdTargetDir` | `DEG(100)` = 1.7453 |
| `+4Ch` | `+3CCh` | **derived**, `007E908D` copies `+330h` `Dynamics/SpdMultipliers/TurboMultiplier` | — |

Authored values are this installation's own
(`scripts/datatables/planeglobals.lua`, mtime 2024-10-29 12:54:18; the install is modded, see
the project memory).

### 5.2 The commanded altitude

```
009BFA42  d_steer   = |horizontal (steerPoint - ownPos)|        ; 0042B2F0, Y zeroed first
009BFAA7  d_station = |horizontal (station   - ownPos)|         ; 0042B2F0, Y zeroed first
009BFAAC  t = d_station / d_steer
009BFAC5  t = min(t, 1.0)                                       ; [00D7A24C] = 1.0f
009BFAE1  cmdAlt = stationY + (steerY - stationY) * t           ; stationY is state+34h
009BFC03  SUB ESP,8                                            ; the two-float frame
009BFC06  FSTP [ESP+4]                                         ; arg2 = the distance
009BFC0C  FLD [ESP+40h]                                        ; starts arg1
009BFC10  FSUB [EBX+100h]                                      ; cmdAlt - own world Y
009BFC1E  FSTP [ESP]                                           ; arg1
009BFC21  CALL 009F9ED0                                        ; RET 8, two stack floats
```

**CORRECTED, packet `cc8_follow_enter`, withdrawing this section's "`009BFC0C CALL 009F9ED0`".**
`009BFC0C` is the `FLD` that begins argument 1, not the call; the `CALL` is at **`009BFC21`**. The
relay's citation was the right one and §5.9's "Unverified here" note, which recorded the discrepancy
as unresolved, is now resolved in the relay's favour. The reading of what is computed is unchanged.

`009F9ED0` is `RET 8`: two stack floats, an altitude error and a distance. The floor at
`009BFBD0` is `block+00h`.

### 5.3 The commanded speed — two ramps

```
009BFC3B  dot = ownForward . unitDirectionToStation             ; horizontal, pose+ECh/+F4h
009BFC41  cruise = 007C47F0(classDesc) * 0.9                    ; [00D7A390], read as a DOUBLE
009BFC58  leaderSpd = [[state+2Ch]]+38h ()                      ; the leader observer, 0 args
009BFC7D  s = max(leaderSpd, classDesc+188h)

009BFCD1  rampD = InterpolateClamped(0, leaderSpd,
                                     GoodPositionDist, TurboMultiplier * s,
                                     d_station)
009BFD0A  cmdSpd = InterpolateClamped(MaxFollowSpdTargetDir, rampD,
                                      MinFollowSpdTargetDir, cruise,
                                      dot)
009BFD0F  [cmdBlock+2B4h] = cmdSpd
009BFD15  [cmdBlock+2B0h] = 0
009BFD1C  [cmdBlock+2D8h] = 1
```

So a member standing **on** its station is told to fly the leader's speed; one at or beyond
`GoodPositionDist` (100 m) is told to fly the leader's speed **on turbo**. That is what makes
the first ramp a catch-up law rather than a trim.

`cmdBlock+2B4h` is the field this host already carries as
`GameUnitSlot::plane_desired_speed_2b4`.

#### 5.3.1 Two things that are easy to get wrong here, and are not guesses

**The arity.** The five floats of `00419010` (`RET 14h`) are assembled across **two** stack
windows with a zero-argument virtual call between them:

```
009BFC98  SUB ESP,0Ch    -> [B+8] = d_station, [B+4] = Turbo*s, [B+0] = GoodPositionDist
009BFCC3  CALL [..+38h]  -> the leader-speed virtual, 0 stack args
009BFCC5  SUB ESP,8      -> [B-4] = that result, [B-8] = 0.0 (FLDZ)
009BFCD1  CALL 00419010  -> RET 14h cleans all five
```

Reading the `SUB ESP,0Ch` alone gives a three-argument call and the wrong function. The arity
is settled by `00419010`'s own `RET 14h`, per the rule that callee stack effects come from the
callee's RET.

**The two virtual calls are separate.** `009BFC58` and `009BFCC3` both call slot `+38h` on
`state+2Ch`. Only the **first** result is floored at `classDesc+188h`, and that floored value is
used solely as the factor of the catch-up end. The ramp's `y0` is the **second**, unfloored
call. A leader flying slower than `classDesc+188h` therefore gives a member on its station the
leader's true speed while the catch-up end still scales the floor.

### 5.4 An asymmetry in the image's own units

The alignment ramp's input is a **cosine** (`009BFC3B` dots two horizontal unit vectors) while
its endpoints are **radians** (`DEG(30)`, `DEG(100)`). A cosine never exceeds 1 and the high
endpoint is 1.745, so the `y1` end is unreachable: even a perfectly aligned member lands at
`(1 - 0.5236) / (1.7453 - 0.5236)` = **0.39** of the way from the distance ramp toward
`cruise`. This is recorded, not corrected — the arithmetic bound in `src/plane_follow_law.cpp`
is the image's.

## 5.5 The steer point is a carrot 250 m ABEAM, on one guarded regime

> **Withdrawn 2026-09-19, packet `cc8_follow_steer`.** This section previously read *"the steer
> point is a carrot 250 m **ahead**, not the station"* and called `009C1662`-`009C16CF` the
> **fall-through** site. Both claims are withdrawn. The *form* of the point survives unchanged —
> own position plus `FollowedPointDist` times a unit direction — but that direction is
> **perpendicular** to the aircraft's nose (§5.6), and the block is reached only through one
> guarded branch, never by fall-through (§5.7). Nothing in §5.1-§5.4 is affected: those read
> `009BEE30`, which consumes the point and does not care how it was made.

The sixth of the `state+44h` store sites, `009C1662`-`009C16CF`, settles what the steer point *is*:

```
009C167B  FLD  [[ESI+6Ch]]        ; block+00h = FollowedPointDist = 250
009C1680  FMUL [ESP+4Ch]          ; * dirX
009C1684  FADD [EDI+FCh]          ; + own world X
009C168A  FSTP [ESI+44h]          ; steer X
009C16A3..009C16AF                ; the same with dirZ and own world Z -> steer Z
009C16B2  FLD [ESP+5Ch] / FMUL [ESP+28h] / FADD [ESI+34h] / FSTP [ESI+48h]
009C16C0..009C16CF                ; +60h/+64h/+68h are a straight COPY of the point
```

So `steerPoint = ownPosition + FollowedPointDist * direction`, with its Y built as an offset
above the **station's** Y. The name `FollowedPointDist` means what it says about the *distance*.
The *direction* is read in §5.6 and it is not the nose.

Two consequences that matter, and that a station-point substitution would have got wrong:

* `d_steer` in section 5.2 is therefore **about 250 whatever the geometry**, not the range to
  the station. The altitude blend `t = min(d_station / d_steer, 1)` consequently only reaches
  `stationY` once the member is 250 m or more from its station; inside that it sits
  proportionally between `stationY` and the carrot's Y.
* Substituting the station point for the steer point — the obvious cheap binding — is not a
  weaker version of this law, it is a different one: it would make `t` collapse to 1 and put the
  aircraft's commanded heading on the station rather than on this point.

### 5.5.1 Slot names, and why the raw `[ESP+NNh]` in the old text was unsafe

`009BFEE0` opens `PUSH EBP / MOV EBP,ESP / AND ESP,0xFFFFFFF8 / SUB ESP,78h` and then pushes
four registers, so every local is addressed off a **moving** ESP. A raw `[ESP+4Ch]` is therefore
not a slot name. All slot identities below are the canonical frame offsets produced by
`tools/x87trace.py` (its `base [ESP+NNh]` annotation), which are comparable across the whole
body:

| raw, at `fb=8Ch` | canonical | role |
|---|---|---|
| `[ESP+4Ch]` | `base+18h` | steer direction X |
| `[ESP+50h]` | `base+1Ch` | steer direction Z |
| `[ESP+5Ch]` | `base+28h` | carrot altitude offset, factor A |
| `[ESP+28h]` | `base-0Ch` | carrot altitude offset, factor B |

In this body the frame happens to sit at `fb=8Ch` almost everywhere, so the raw displacements
are stable; but `base-0Ch` really does appear as `[ESP+2Ch]` at `009C0786` and `009C0D25`, where
`fb=90h`. The walk is only trustworthy once every callee's stack effect is supplied — see §5.8.

## 5.6 The steer direction is the PERPENDICULAR of the nose

`009C15C0`-`009C1661` is the only producer of the two direction slots that `009C1662` consumes.

```
009C15D6  base+18h = pose[+ECh]            ; forward.X, raw
009C15EA  base+1Ch = pose[+F4h]            ; forward.Z, raw
009C15F1  CALL 00419260                    ; invLen = 1 / |(forward.X, forward.Z)|
009C1616  base+44h = ux = forward.X * invLen
009C161E  base+48h = uz = forward.Z * invLen
009C1622  base+18h = uz   and  base+1Ch = ux      ; the components are SWAPPED
009C1646  JE, on TEST byte [00E0E2F9],BL at 009C15F6:
            bit clear -> base+1Ch = -ux    =>  direction ( uz, -ux)
            bit set   -> base+18h = -uz    =>  direction (-uz,  ux)
```

Three things make this reading tight rather than a guess:

* **The swap is explicit.** `009C1622`-`009C1642` is MSVC's add-then-recover idiom: it stores
  `ux+uz` into `base+18h`, recovers `ux` by `FSUBRP ST(2)` into `base+1Ch`, then recovers `uz`
  by `FSUB` back into `base+18h`. Net: `base+18h = uz`, `base+1Ch = ux`.
* **The negation is exact.** `[00D7A208] = -0.0f`, so `SUBSS xmm0, v` with `xmm0 = -0.0` is
  MSVC's float negate, not an offset subtraction. `0042BE90` negates through the same constant.
* **Which component is X is not in doubt.** `009C1680` multiplies `base+18h` into steer X and
  adds `pose[+FCh]`; `009C16A5` multiplies `base+1Ch` into steer Z and adds `pose[+104h]`. Row 3
  of the pose matrix at `+FCh/+100h/+104h` is the translation (used as own world position right
  there), so row 2 at `+ECh/+F0h/+F4h` is X/Y/Z of the forward basis — and §5.3's dot product at
  `009BFC3B`, read independently in another body, already treats `pose+ECh/+F4h` as
  `ownForward`.

`(uz, -ux)` and `(-uz, ux)` are precisely the two horizontal perpendiculars of the unit forward
vector. So on this path the member is steered at a point **250 m abeam** — to its left or its
right, the side chosen by a global bit — not 250 m ahead. Heading at a point fixed abeam of your
own nose is a turn command that renews itself every tick, which is why the old "ahead" reading
made the law look like a straight-line chase when it is not.

### 5.6.1 The four `00E0E2Fx` globals are bit constants, and `BL` is the variable

An earlier draft of this section left open whether `00E0E2F8`-`00E0E2FB` were a mask table,
scratch or tuning. Settled, and the answer inverts which operand of the `TEST` is the variable:

* **Every reference in the whole image is inside `009BFEE0` itself** — 11 sites, found by
  scanning the image for each address dword (the pattern is known to occur, so this is not a
  vacuous negative). **None of them is a write.**
* They are in **`.data` and initialised**, not loader-zero BSS: the bytes at
  `00E0E2F8..00E0E2FB` are `08 02 01 04`, i.e. `[F8]=8`, `[F9]=2`, `[FA]=1`, `[FB]=4`.
* `009C0ED9` does `OR AL, byte [00E0E2F8]`, which only makes sense on a bit.

So they are four never-written single-bit constants, and `TEST byte [00E0E2F9], BL` is
`BL & 2` — the *mask* is in memory and `BL` is the value under test. `BL` is built by this
function and never leaves it.

### 5.6.2 `BL` is a quadrant classifier, and it picks the side

`009C01D3`-`009C024F` classifies two quantities into `BL ∈ {1,2,3,4}`:

* `A` = `base-1Ch`, the wrapped angle returned by `00438B10 SubtractWrappedAngle` at
  `009C01CE` (proved by adjacency: the call returns in ST0 and the next x87 op stores it);
* `V` = `base+0Ch`, tested only for sign at `009C01E7`. Its sign is also saved to the byte
  `base-21h` (0 for `V >= 0` at `009C01F0`, 1 for `V < 0` at `009C021E`), which later sites
  reload into `BL`.

The thresholds are exactly a quadrant split: `[00D7A218] = 0.0f`, `[00CE3830] = +π/2`,
`[00CF48A0] = -π/2` (`1.5707963705062866`, the `double` form).

| sign of `V` | sign of `A` | `|A|` vs π/2 | `BL` |
|---|---|---|---|
| `>= 0` | `>= 0` | `<= π/2` | 2 |
| `>= 0` | `>= 0` | `> π/2` | 4 |
| `>= 0` | `< 0` | `<= π/2` | 1 |
| `>= 0` | `< 0` | `> π/2` | 3 |
| `< 0` | `>= 0` | `<= π/2` | 1 |
| `< 0` | `>= 0` | `> π/2` | 3 |
| `< 0` | `< 0` | `<= π/2` | 2 |
| `< 0` | `< 0` | `> π/2` | 4 |

Equivalently, with `s = (V >= 0)` and `a = (A >= 0)`: `BL = 2` when `s == a` and `|A| <= π/2`,
`4` when `s == a` and `|A| > π/2`, `1` when `s != a` and `|A| <= π/2`, `3` otherwise. The two
paths that share the comparison at `009C0247` deliberately push their operands in opposite
orders so one `FCOMPI` serves both.

Two consequences follow directly, and they close §5.7's open guard:

* **The side of the abeam point.** `009C15F6` tests `BL & 2`, which is set exactly for
  `BL ∈ {2,3}`. So the direction is `(-uz, ux)` when `BL ∈ {2,3}` and `(uz, -ux)` when
  `BL ∈ {1,4}` — i.e. the turn side is `(s == a) == (|A| <= π/2)`.
* **The guard at `009C1247`.** It tests `BL & 8`, and the classifier never sets bit 8: the
  *only* writer of that bit is `009C0EDF` (`BL = (BL ? 2 : 4) | 8`). So any path that reaches
  `009C1247` without passing `009C0ED9` has `BL ∈ {1,2,3,4}`, the test yields zero and the
  `JE` is **taken** into the abeam regime. The abeam block is therefore the default for that
  branch, which is what the old "fall-through" wording was groping at — but it is a bit test on
  a value this function computed, not a fall-through, and a path through `009C0ED9` skips it.

**Still open here:** what `A` and `V` mean geometrically — `A` is a bearing error in radians and
`V` is sign-tested only, but neither has been traced to its own producer, so the *name* of the
quadrant (relative to the leader? to the station? to a threat?) is not established. Names are
hypotheses.

## 5.7 `009C1662` is one guarded regime, not the fall-through

The old text assumed the block was reached when nothing else fired. It is not:

* the **only** entry to `009C1662` is `JMP 0x9C1662` at `009C1654`, inside the `009C15C0` block;
* `009C15C0` itself is entered **only** by `JE 0x9C15C0` at `009C1247`, under
  `TEST byte [00E0E2F8],BL`;
* the third store site (`009C1222`) ends `JMP 0x9C16C0`, i.e. into the `+60h/+64h/+68h` copy;
* the fifth store site (`009C1552`) ends `JMP 0x9C16D2`, into the tail;
* the hold arm reaches the tail from `009C0021`.

So the six `state+44h` sites are six *regimes*, each with its own exit, and the abeam one is the
regime selected by `009C1247`. What remains unread in the fly-to arm is the guard and direction
of the four store sites at `009C10F7`, `009C11D5`, `009C1222` and `009C1328`, and the altitude
offset factors `base+28h` / `base-0Ch`.

## 5.8 The frame walk, and the callee table without which it lies

Reading any ESP-relative slot in this body depends on knowing ESP at every instruction, and
`tools/x87trace.py` **starts with an empty call table**: with no `--call` / `--icall` it assumes
every call is `esp+0`, which silently mis-tracks the frame. Run that way it reported 15 join
conflicts here, several with genuinely different ESP on the two sides — all of them artefacts.
With every callee's effect supplied the walk is globally ESP-consistent over
`009BFEE0`-`009C1846`: the five remaining conflicts all read `esp -140` on **both** sides, and
only the x87 depth still disagrees.

The table is committed as `tools/callee_effects_009bfee0.json`, with the ready-made argv. Four
entries were wrong under the obvious reading and cost real time:

| callee | effect | what the obvious reading gets wrong |
|---|---|---|
| `0042CF10` `AsinClamped` | **RET 4** | `calleefx` swept past the body end into the next function and offered `ret 0`. Ghidra's body ends `0042CF9E`; the bytes there are `c2 04 00`. **Seven** call sites, so this alone shifted the frame. |
| `00419010` `InterpolateClamped` | **RET 14h** | same overrun offered `ret 10h`; `004190CE` is `c2 14 00`. Confirms §5.3.1 independently. |
| `00BF701A` | `LIBCRT_atan2` | it is *not* `sqrt`; the `sqrt` span at `00BF7030` swallowed this separate entry. An atan2 in the direction code is a semantic clue, not just a stack effect. |
| `0042E740` | `GetSingleton`, RET 0 | overrun offered `ret 4`. |

Two rules this body pays for:

* **A tail `POP ECX` is MSVC freeing a 4-byte local allocation, not cleaning an argument.**
  `0042BE90`, `00414C60`, `0042B2F0` and `00419510` all end that way and all clean nothing. The
  caller-side effect of a call is exactly the callee's own `RET` imm.
* **`calleefx.py` sweeps linearly to the next INT3 padding**, so where padding is absent it
  reports the *next* function's `ret` too. Any callee it reports with two RET imms must have its
  body end taken from Ghidra and the imm read as bytes there.

The five indirect calls resolve from their dispatch and their call sites: `009C00C8` and
`009C01B6` are vtable slot `+50h` with no stack args and a float return (an `FSTP` consumes the
result immediately); `009C025E` is slot `+34h` with one pushed out-pointer, returning that
pointer in EAX; `009C02CF` / `009C02DE` are slot `+5Ch` taking `push 10h` / `push 16h` and
returning a bool in AL.

## 5.9 The tail clamps BOTH Y values into one leader-relative band

This is the `done`-state floor, and it is the measurable consequence of the whole chain, so it is
read here from the listing rather than taken from `docs/BOMBER_AFTER_TASK.md` 10.8 secondhand.

The band is built in two halves:

```
009C16E8  FLD [[ESI+6Ch]+4]          ; tuning block+04h  (= singleton+384h)
009C16EB  FADD [EDI+100h]            ; + leader world Y      (EDI = [ESI+2Ch], the leader)
009C16F1  base+00h = leaderY + block[+4]
009C16F5  base+04h = state+88h
009C16FF-009C1725   base+04h = L = min(leaderY + block[+4], state+88h)     ; FCOMIP + select

009C1779  FLD [EDI+100h] / FADD qword [00D1F3F8]   ; leaderY + 120.0  (the double is 120.0)
009C1789  CALL 0042E740 / FLD [EAX+210h]           ; the tuning singleton's +210h
009C17A5-009C17B9   cap = min(singleton[+210h], leaderY + 120.0)
```

Then the same band is applied twice, by two ordered selects of identical shape:

```
009C17C6-009C17F1   state+34h = clamp(stationY, L, cap)     ; the STATION point's Y
009C17F6-009C183B   state+48h = clamp(steerY,   L, cap)     ; the STEER point's Y
                    floor at 009C1811, ceiling at 009C1827, pass-through at 009C183B
```

Both clamps are a true band: below `L` the bound wins, above `cap` the cap wins, otherwise the
value passes. `FCOMI` (`db f1`, no pop) at `009C17CE` versus `FCOMIP` (`df f1`, pops) at
`009C17A5`/`009C1803` is what keeps `L` and `cap` live across the selects — read the opcode
byte, not the mnemonic spelling, or the stack tracking goes wrong here.

**Why it matters, and it is the first acceptance row for any host switch:** §5.2's commanded
altitude is `lerp(stationY, steerY, t) - ownY` fed to `009F9ED0`, and *both* of its inputs have
just been floored at `L`, which is tied to the leader's own Y every tick. A spent **wing member**
in `done` therefore cannot descend to the water while its leader flies. This host's
`D3A Val #1.1|.-2` sinking 270 m -> 0 m in `done` is exactly this command going missing. A
finished flight **leader** is commanded nothing at all (`009C1FF1 JZ 009C234E` ends the tick), so
a leader sitting low in `done` is faithful-by-omission and is not a defect to fix.

**Two corrections to how this was relayed**, both from the listing above:

* The band is **not** `leaderY ± 120.0`. `120.0` at `00D1F3F8` is loaded **once**, and only into
  the *ceiling*. The *floor* is `min(leaderY + block[+4], state+88h)` — a different tuning key
  (`singleton+384h`, not yet named in §5.1's table) and a state field, not a symmetric offset.
* `009C1789` loads `[EAX+210h]`, which is a *candidate* ceiling; the effective cap is the **min**
  of it and `leaderY + 120.0`, taken at `009C17A5`-`009C17B9`. Quoting `[EAX+210h]` alone
  overstates the ceiling whenever the leader is low.

~~Unverified here: §5.2 cites the altitude command as `009BFC0C CALL 009F9ED0` and the relay cites
`009BFC21`.~~ **RESOLVED, packet `cc8_follow_enter`: the `CALL` is at `009BFC21`** and `009BFC0C` is
the `FLD [ESP+40h]` that begins argument 1. §5.2 is corrected and now carries the whole
`009BFC03`-`009BFC21` sequence.

## 5.10 The reference direction: the leader's heading, LAGGED by distance

Packet `cc8_follow_regimes`. This closes §5.6.2's "still open" on what `A` and `V` mean.

`009C00C8` calls unit virtual slot `+50h` on the **leader** (`state+2Ch`); `009C01B6` calls the
**same slot** on the own unit. That slot is the heading getter — `src/game_hosts_units.cpp`
already records it, at the `00835AC0` latch, as "a RET 0 getter … atan2 over pose row 2", which
is independent of this packet. Then:

```
009C00F7  T   = InterpolateClamped(block+44h, block+3Ch, block+48h, block+40h, R)
009C0109  G   = 007D7DA0(leader+0AB0h) * T
009C0128  ref = SubtractWrappedAngle(leaderHeading, G)     ; = leaderHeading - G
```

`00438B10`'s own entry is `FLD [ESP+4] / FSUB [ESP+8]`, so the order is `wrap(arg0 - arg1)`.
The four block offsets are `singleton+3C4h/+3BCh/+3C8h/+3C0h`, which
`docs/GAME_TUNING_SINGLETON.md` names:

| block | singleton | key | this installation |
|---|---|---|---|
| `+04h` | `+384h` | `Pilot/Follow/LeaderFollowAlt` | — (§5.9 left this one unnamed) |
| `+3Ch` | `+3BCh` | `Pilot/Follow/LeaderHeadingSpdTime/1` | 0.5 |
| `+40h` | `+3C0h` | `Pilot/Follow/LeaderHeadingSpdTime/2` | 4.0 |
| `+44h` | `+3C4h` | `Pilot/Follow/LeaderHeadingSpdDist/1` | 100 |
| `+48h` | `+3C8h` | `Pilot/Follow/LeaderHeadingSpdDist/2` | 500 |

So `T` is a **time**, ramped by the member's range to its station: 0.5 s at 100 m, 4.0 s at
500 m. `ref` is the leader's heading lagged by `rate * T` — a member well out of position steers
on the track its leader held seconds ago, not on where the leader points now. The tuning key's
own name, `LeaderHeadingSpdTime`, says the same thing, which is what makes this a reading.

`007D7DA0` (one caller, this body) is the rate. Its body reads `[obj+0C68h]` through `FSIN` and
`FCOS`, `[obj+0C64h]` through `FCOS` of its absolute value, and `[desc+1C8h]`. This host carries
those three as `plane_bank_angle_c68`, `plane_pitch_angle_c64` and `plane_class_turn_roll_spd`
(`desc+1C8h TurnRollSpd`), so it is a **coordinated-turn rate from bank, pitch and speed**, in
rad/s. The body itself is not reconstructed and the name is a hypothesis.

### 5.10.1 `A` and `V` are the heading error and the cross-track offset

`009C0139` is `_CIatan2` in its **x87-argument** form — the arguments arrive in `ST(1)`/`ST(0)`
and there is no stack adjustment, which is why the call has no `SUB ESP` beside it. It gives
`atan2(dz, dx)` over `own - station`, and `009C0142`-`009C0164` rewrites that as
`wrap(pi/2 - it)` into `[0, 2pi)`: the **compass bearing station->aircraft**, this image's
heading convention being `direction = (sin h, cos h)`. With `A0 = wrap(bearing - ref)`:

```
009C018D  V     = R * sin(A0)     the CROSS-TRACK offset, +ve to the RIGHT of the lagged track
009C01A9  along = R * cos(A0)     the ALONG-TRACK offset
009C01CE  A     = wrap(ownHeading - ref)          the heading error against that track
```

That is the whole frame: both quantities are the member's displacement and attitude expressed
about the leader's lagged track through the station.

## 5.11 `BL` is REWRITTEN in Phase A, so the quadrant does not select the regime

> **Withdrawn 2026-09-19, packet `cc8_follow_regimes`.** §5.6.2 concluded that "any path that
> reaches `009C1247` without passing `009C0ED9` has `BL` in {1,2,3,4}", i.e. that the dispatch
> tests the quadrant. It does not. The classifier and the bit constants are unaffected; what is
> withdrawn is that the classifier's value survives to the dispatch.

A census of every write to `BL`/`EBX` over the whole body gives **nine**, not five:

```
009C0213/0235 BL=1   009C0239 BL=3   009C024B BL=2   009C024F BL=4   the quadrant classifier
009C032D  MOV BL,[ESP+13h]        base-21h, the saved SIGN OF V
009C0814  MOV BL,[00E0E2FB] = 4
009C08CD  MOV BL,[00E0E2FA] = 1
009C08F5  MOV BL,[00E0E2F9] = 2
009C0909  MOV BL,[ESP+13h]        the V-sign byte again
009C0BC7  MOV BL,[00E0E2FA] = 1
009C0BD9  MOV BL,[ESP+13h]
009C0EDF  MOV BL,AL             = (BL ? 2 : 4) | 8
```

The quadrant value is consumed **inside** Phase A by thirteen `TEST BL,BL` booleans. The five
blocks that reach the convergence point `009C0EE1` are `009C0814`, `009C08CD`, `009C08F5`,
`009C0BC7` and `009C0ED9`, so the `BL` the dispatch tests has exactly five possible values:

| `BL` at dispatch | set at | regime |
|---|---|---|
| 1 | `009C08CD`, `009C0BC7` | lead pursuit (`009C1059` tests `BL&1`) |
| 2 | `009C08F5` | abeam, `(-uz, ux)` = LEFT |
| 4 | `009C0814` | abeam, `(uz, -ux)` = RIGHT |
| 10 / 12 | `009C0EDF` | the `009C1328` regime (`009C1241` tests `BL&8`) |

Two leaf guards are read. `009C08B5`-`009C08C7` is `JA` on `e > 0.05 * p` (`[00D7A270]`, a
**double**, read at the `FMUL`'s own width); its twin is `009C0B82`/`009C0B9C`. Failing it gives
`BL=1`; passing it gives the bit-8 regime. The abeam **side** is decided at `009C08E7` by
`TEST BL,BL` where `BL` is the V-sign byte, so `BL=4` (V >= 0, the aircraft **right** of the
track) selects the abeam **RIGHT** direction: the member is steered 250 m to the side it is
already on. On that branch the manoeuvre is a **break-away, not a rejoin**.

## 5.12 The lead-pursuit regime, read end to end

This is the one a member converging on its station flies, and the three "store sites"
`009C10F7`, `009C11D5` and `009C1222` are not three regimes but three **stages** of one point;
the block ends `009C123C JMP 009C16C0`.

```
D  = |ownPos - station|3D                         009C10A9 CALL 0042B2F0 (Y NOT zeroed here)
Lh = max(|leader horizontal forward|, 0.01)       009C0F0D-009C0F80, floor [00D7A238]
Tdir = wrap(pi/2 - ref)                           009C0F86-009C0FAE
U  = normalize(Lh*cos Tdir, leaderFwd.Y, Lh*sin Tdir)          009C0FB2-009C104C
     = the unit vector along `ref` carrying the leader's own vertical slope
P  = station + D * U                              009C10D2-009C1105
n  = normalize(ownPos - P)                        009C111E-009C117D
P += 0.20 * D * n                                 009C1184-009C11EC   [00CE3D10] = 0.2, double
P += FollowedPointDist * U     (250 m)            009C11EF-009C1239   block+00h
state+44h/48h/4Ch = P
```

In words: the station pushed **(D + 250) m ahead along the leader's lagged track**, then pulled
**0.2 x D back toward the aircraft**. `FollowedPointDist` is the same 250 the abeam regime
spends, spent along-track here instead of abeam.

**A register trap that costs a plausible wrong answer.** At `009C0F13` and `009C100F` MSVC
emits `LEA EAX,[EDI+0CCh]` — the leader's pose row 0 — and then calls `00419510`, which is
`__fastcall(out = ECX, v = EDX)` and **returns `out` in EAX** (`00419545 MOV EAX,EDI`). The
`LEA` is therefore dead, and the `[EAX]`/`[EAX+4]`/`[EAX+8]` copy that follows reads the
**normalized local**, not the pose row. Taking the `LEA` at face value gives
"steer point = station + D x leaderRight", which is wrong and reads perfectly well.

### 5.12.1 The other two regimes, and what Phase A still owes

* **Abeam**, `009C15C0`-`009C16CF` (§5.5, §5.6): `ownPos + 250 * perpendicular(own nose)`, with
  `steerY = stationY + U.y * base-0Ch`.
* **`009C1328`**, `009C12DD`-`009C1336`: `station + base-0Ch * U` — the same lead construction
  with `base-0Ch` in place of `D` and without the 0.2 pull-back or the 250 push. The
  perpendicular this block also builds (`009C124D`-`009C12D9`, swapped components with one
  negated on the sign of V, scaled by `base-20h`) feeds computation past `009C1365` that is not
  part of the steer point.

Phase A (`009C0251`-`009C0EE0`, ~1200 instructions) is still unread, but it now reaches the
dispatch through **exactly two channels**, which is what makes the remaining substitution
bounded rather than open-ended:

1. the **regime selector** (§5.11);
2. **`base-0Ch`**, last written at `009C0EE1`-`009C0F00` as `base-0Ch *= base-8h` and never
   written again before `009C16B2` — so the abeam regime's altitude offset is
   `stationY + U.y * base-0Ch`, the same slope-times-distance shape the lead regime uses.

Also settled in passing: `009C0EF4` writes **`state+5Ch = -1.0f`** (`[00D7A260]`), which this
host already carries as `kPlaneContactTimerExpired`, so that auxiliary field is a timer reset.

## 6. What is bound, and what is not

Bound, pure, no globals: `include/bsp/plane_follow_law.hpp` + `src/plane_follow_law.cpp`,
`plane_follow_flyto_command_009bee30` and `plane_follow_blended_altitude_009bfaac`. Builds
clean (`./scripts/build.ps1`, exit 0). Reuses `dive_bomb_interpolate_clamped_00419010` rather
than adding a second copy of `00419010`.

**Not yet bound, and the honest blocker for switching the host off placement:** the steer
direction in the regimes that are not §5.6's. One of the six is now read — the abeam regime
(§5.6, §5.7) — and reading it made the blocker *sharper*, not smaller: the direction on that
path is perpendicular to the aircraft's nose, so a station-point substitution is further from
the law than the old "carrot ahead" reading suggested, and the cheap binding stays refused.
Still open: the guard and direction of the store sites at `009C10F7`, `009C11D5`, `009C1222`
and `009C1328`; the carrot's altitude offset (`base+28h` × `base-0Ch`); and the writers of the
`00E0E2F8`-`00E0E2FB` globals that pick the abeam side.

Therefore `kPlaneFormationPlacementEnabled` in `src/game_hosts_units.cpp` **stays true** and the
placement hole in `docs/PLANE_FORMATION.md` section 6 stands, narrowed: it is no longer "2900
instructions of station-keeping law are unread" but "the ~1500-instruction steer-point producer
is unread; the ~1000-instruction commander that consumes it is read and bound".

## 7. Block-map state, for whoever takes this next

| body | range | state |
|---|---|---|
| `009C1FD0` tick | whole | **read** (section 2) |
| `009BFD70` station point | whole | read previously; writes `+85h`, `+14h` |
| `009BFEE0` hold arm | `009BFEE0`-`009C0025` | **read** — writes the steer point, no commands |
| `009BFEE0` fly-to arm, abeam regime | `009C1662`-`009C16CF` | **read** (§5.5); reached ONLY from `009C1654`, not by fall-through |
| `009BFEE0` fly-to arm, abeam direction | `009C15C0`-`009C1661` | **read** (§5.6) — perpendicular of the nose; entered only from `JE` at `009C1247` |
| `009BFEE0` fly-to arm, `BL` classifier | `009C01D3`-`009C024F` | **read** (§5.6.2) — quadrant of `(V, A)` about `±π/2`; sets the abeam side and the `009C1247` guard |
| `009BFEE0` fly-to arm, the frame | `009C0026`-`009C0250` | **read** (§5.10) — R, the lagged reference heading, `A`, `V`, the quadrant classifier |
| `009BFEE0` fly-to arm, PHASE A | `009C0251`-`009C0EE0` | **OPEN** — the remaining blocker, but it reaches the dispatch through only two channels (§5.12.1): the regime selector and `base-0Ch` |
| `009BFEE0` fly-to arm, `U` producer | `009C0EE1`-`009C1058` | **read** (§5.12) — the unit vector along the lagged track with the leader's slope |
| `009BFEE0` fly-to arm, lead pursuit | `009C1059`-`009C123C` | **read and bound** (§5.12) — three stages of ONE point, not three regimes |
| `009BFEE0` fly-to arm, `009C1328` | `009C1241`-`009C1336` | **read** (§5.12.1); the tail past `009C1365` is not part of the steer point and is unread |
| `009BFEE0` fly-to arm, `009C1552` site | `009C1455`-`009C1560` | **OPEN** — reached from a subtree this packet did not enter; ends `JMP 009C16D2` |
| `009BFEE0` tail | `009C16D2`-`009C1846` | read previously (BOMBER_AFTER_TASK 10.8) |
| `009BEE30` fly-to arm | `009BF9EA`-`009BFD38` | **read and bound** (section 5) |
| `009BEE30` hold arm | `009BEE56`-`009BF9E5` | **OPEN** — the larger arm; writes cmd `+278h`-`+29Ch` |
| `009BECD0` | whole | not reached; the law calls `007C47F0` directly at `009BFC41` |

Auxiliary state fields `009BFEE0`'s fly-to arm also writes: `+60h`/`+64h`/`+68h` are a straight
copy of the steer point (`009C16C0`-`009C16CF`, section 5.5). `+50h`, `+54h`, `+58h` and `+5Ch`
are written at `009C0EF4`, `009C1262`, `009C1463`-`009C15B6` and their consumer is not yet
identified — they are not read by `009BEE30`'s fly-to arm.

Listings and the census tool used are in this worktree's ignored `local/`:
`arm_009bfee0.lst`, `step3_009bee30.lst`, `follow_tick_009c1fd0.lst`, `station_009bfd70.lst`,
`blockmap.py` (`--mode blocks|calls|writes`, `--lo`/`--hi`).

## 14. All three seams on the image's body (packet cc9_plane_follow_law)

`kPlaneFollowLawBound` (`src/game_hosts_units.cpp`) runs 009C1FD0's body at the three seams where
the host placed the member on its station each tick:
- the torpedo follow tick, reached through 009D2731;
- the dive-bomb follow tick, which ran the fly-to law but also kept the placement;
- the dive-bomb done tick, reached through 009C7278.

Per tick at each seam:
1. `place_wing_member_on_station_007f23a0(..., apply_position = false)` produces 009BFD70's
   station from the live slot 0, without moving the member. The first-step seeding at the
   member's first step stays as it was.
2. **The +85h latch.** It is recomputed with no hysteresis, as 009BFDD4-009BFEA2 do:
   distance to the station below `Pilot/Follow/GoodPositionDist` (100) AND leader-forward dot
   member-forward above `GoodPositionDir` (0.5). Source: `docs/PLANE_FOLLOW_HOLD_ARM.md`
   section 10.
3. **Latched: the HOLD arm, 009BEE56-009BF9E5.** It runs through
   `bsp::plane_follow_hold_command_009bee56`, wired for the first time.
   - It writes the pitch, yaw and roll slots with modes 0; yaw mode 0 skips the planner's yaw
     arm at 0099E756.
   - It writes the bank target with mode 1 when |leader bank| >= 0.75.
   - It writes the throttle and air-brake slots with plan+2D8h = 0.
   - Its gains are the `yf_*`, `pf_*`, `rf_*` and `pwr_*` keys at singleton+3ECh..420h.
   - The power floor uses 007C47F0's level-flight speed.
   - Everything is sourced in `docs/PLANE_FOLLOW_HOLD_ARM.md` sections 3-9.
4. **Otherwise: the fly-to arm,** the host's existing `run_follow_law_009bfee0_009bee30`
   (009BFEE0 lead pursuit, then 009BEE30's 009BF9EA-009BFD38).

**Substitutions, labelled in the code:**
- **Controller rates.** ctl+A0h/A4h/A8h are replaced by the body angular velocity ctl+50h/48h/4Ch.
  Only the pitch rate has a non-zero gain, pf_pitchV = 1/DEG(100).
- **Turn rate.** unit+C70h is taken as 0; its gain rf_hdgV is authored 0.
- **The steer point in the hold arm.** 009BFEE0's own latched geometry path at 009BFEEE is unread,
  so the fly-to steer point stands in. The blend uses it only while |leader bank| > 0.5.
- **The sight correction.** state+84h stays inactive; the search loop that writes it is only
  partly read.
- **The lock** is never taken, because an AI leader's +520h byte is clear.

### 14.1 Predictions, written before the pair

Taken from `local\M1_9000.log`, main plus the leave-at-death tree, E2 9000 with the stream option.
The pair is F0 (switch off) against F1 (on).

**Who is at the seams.**
- Every Kate wingman spends 1421-1615 ticks in the torpedo follow state BEFORE its aim. This is
  the whole approach from spawn, and it is placement today.
- Kates #8.1|.-2 and .-4 add about 600 done ticks after release.
- The only dive-bomb member placed in done is Val #7.1|.-2, for 841 ticks.
- No member is in the dive follow state; every Val hands over at arm tick 0.

**The first leader change at a seam.** Kate #8.1 dies at 338.59 s and #8.1|.-2 becomes leader.
#8.1|.-4, already in follow after its release, is placed on .-2's station from the next tick.
That jump is about one formation offset, 90-180 m in the pairwise rows. The law closes it
instead: at the hold arm's power law (+1.0 throttle at 10 m behind the station) and a 10-20 m/s
speed surplus, it takes 5-15 s. The done-state Val #7.1|.-2 has no leader change in M1, because
#7.1 survives.

| row | F0 (expected = M1) | F1 prediction |
| --- | --- | --- |
| hold / fly-to ticks | 0 / 0 (switch off) | both non-zero; hold dominates for the Kate wingmen in formation |
| Kate #8.1 wing pairwise distance (placed about 88-116 m) | as M1 | 60-250 m, varying, no longer pinned |
| torpedo drops | 8 | 8 ± 3: the wingmen now fly their approach, so their aim-entry geometry and times move |
| bomb drops | 4 | 4 ± 2: only one Val is at a seam, and after its release |
| Kate deaths | 16 | 16 ± 2; death times move, since they are path-coupled |
| plane water contacts | 14 | 14 ± 4. The earlier E2 failure (members drowning in follow) had no hold arm and no speed stores; both are now in place |
| fighter bursts / hits | 10 / 59 | ±25%, path-coupled |
| Lexington | alive | alive |

### 14.2 The first pair was void, and a fourth seam

**The first treatment (`local\F1_9000.log`) did not test the torpedo follow.** The Kate wingmen's
follow state never reached 009C1FD0 in the host:
- The torpedo arm dispatches `state->vtable[0Ch]` (009D48E7-009D48F6).
- For the follow state that is 009C1FD0 (vtable 00D20AB8 slot +0Ch).
- The host instead ran the moveto tick 009C18C0 for `kFollow` as well as `kMoveTo`. The wingmen
  flew their own moveto at the target, in parallel with the leader. That is why the Kate #4.1
  pairwise rows were identical to the digit in F0 and F1.

The 20606 hold ticks came from the dogfight follow state, a fourth caller of the same body (the
generic follow tick). It ran the law AND still placed the fighter each tick.

**The fix, same switch:**
- With `kPlaneFollowLawBound` the torpedo follow state runs `run_follow_tick_009c1fd0`: station
  without placement, then the law.
- The dogfight follow site stops placing. Its call to 007F23A0 now passes
  `apply_position = !kPlaneFollowLawBound`.

The switch-off path is unchanged, so F0 (`local\F0_9000.log`, which reproduces M1 exactly) stays
the control. First-treatment numbers, for the record: bombs 4 to 2, torpedoes 8 = 8, fighter
bursts 10 to 17, hits 59 to 79, Val deaths 13 to 15, US fighter deaths 3 to 1, water contacts 16 =
16.

**Predictions for F1b** (`local\F1b_9000.log`), written before the run:
- **The Kate wingmen are now flown by the law for their 1400-1600 follow ticks.** Their pairwise
  rows stop being parallel copies: 60-250 m, varying, with most ticks in the hold arm while
  within 100 m and 60 degrees.
- **The Kate follow-to-aim timing moves.** Aim entry for wingmen shifts by up to ±10 s, because
  their range to the target now follows the leader's station, not their own moveto.
- **Torpedo drops 8 ± 3, and Kate deaths 16 ± 2.** The #2.1 and #6.1 wings keep dying to the
  Lexington's AA before 450 m.
- **Bomb drops 4 ± 2, fighter hits ±30%, water contacts 16 ± 5, Lexington alive.**

### 14.3 The pair, measured (F0 against F1b)

Both runs are E2 9000 with `BSP_GUNNERY_RNG_STREAMS=1`, from the same tree with only the switch
different. F0 reproduces M1 exactly: 4 bombs, 8 torpedoes, 59 fighter hits, 35 deaths.

| row | F0 (off) | F1b (on) | prediction | verdict |
| --- | --- | --- | --- | --- |
| hold / fly-to ticks | 0 / 0 | 8771 / 30866 | hold dominant | **missed**: fly-to dominates |
| torpedo follow ticks on 009C1FD0 | 0 (moveto ran) | 13491 | - | - |
| dive-bomb done law ticks / placed ticks | 0 / 841 | 706 / 0 | - | - |
| Kate #4.1 pairwise, ticks 400-1600 | 70-193 m (parallel moveto) | 0-1 pair 91-147 m; the other wingmen at 95-1477 m, growing | 60-250 m | **missed**: the wingmen fall behind |
| Kate wingman follow ticks | 1421-1615 | 869-1600 | aim entry ±10 s | follow ends 9-37 s earlier for most |
| torpedo drops | 8 | 8 | 8 ± 3 | held |
| Kate #6.1\|.-2 (Lexington) | no release | releases | - | first Lexington-side release |
| bomb drops | 4 | 6 | 4 ± 2 | held, at the edge |
| Kate deaths | 16 | 16 | 16 ± 2 | held |
| fighter bursts / hits | 10 / 59 | 20 / 61 | hits ±30% | held |
| water contacts | 16 | 14 | 16 ± 5 | held |
| damage total | 8355.2 | 10851.9 | - | - |
| Lexington | alive | alive | alive | held |

**Why the formation opens.** The wingmen start on station and fall behind the leader.
- The leader flies moveto at full throttle through the descent: Kate #4.1 at 89.8-95.5 m/s over
  its first 100 aim ticks.
- The fly-to arm's catch-up has no such speed. Its level-flight term is a labelled substitution,
  max speed x 0.9 (section 6), and its floor is the class minimum.
- Once a member is beyond GoodPositionDist (100 m) it stays on the fly-to arm, and the gap grows
  to 1.0-1.5 km by tick 1600.

This is the next term. Is the leader's moveto speed in the descent the image's? Does the fly-to
arm's catch-up (Dynamics/SpdMultipliers/Turbo, 007C47F0) reach it with the real level-flight
speed? It is not bound here.

**Switch state landed: `kPlaneFollowLawBound` ON.** At all four seams the members now fly the
image's 009C1FD0 body instead of being written onto their stations, and the torpedo follow state
no longer runs the moveto tick. The releases and the Lexington's fate held. The open item is the
catch-up speed above.
