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
009BFC0C  009F9ED0(cmdAlt - ownY, max(d_station, FollowedPointDist))
```

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

## 5.5 The steer point is a carrot 250 m ahead, not the station

The last of the six `state+44h` store sites, `009C1662`-`009C16CF`, is the fall-through one and
it settles what the steer point *is*:

```
009C167B  FLD  [[ESI+6Ch]]        ; block+00h = FollowedPointDist = 250
009C1680  FMUL [ESP+4Ch]          ; * dirX
009C1684  FADD [EDI+FCh]          ; + own world X
009C168A  FSTP [ESI+44h]          ; steer X
009C16A3..009C16AF                ; the same with dirZ and own world Z -> steer Z
009C16B2  FLD [ESP+5Ch] / FMUL [ESP+28h] / FADD [ESI+34h] / FSTP [ESI+48h]
009C16C0..009C16CF                ; +60h/+64h/+68h are a straight COPY of the point
```

So `steerPoint = ownPosition + FollowedPointDist * direction` — a carrot held 250 m ahead of
the aircraft along a computed unit direction (`00419260 Vector2f_ReciprocalLength` normalises
it at `009C15F1`), with its Y built as an offset above the **station's** Y. The name
`FollowedPointDist` means exactly what it says.

Two consequences that matter, and that a station-point substitution would have got wrong:

* `d_steer` in section 5.2 is therefore **about 250 whatever the geometry**, not the range to
  the station. The altitude blend `t = min(d_station / d_steer, 1)` consequently only reaches
  `stationY` once the member is 250 m or more from its station; inside that it sits
  proportionally between `stationY` and the carrot's Y.
* Substituting the station point for the steer point — the obvious cheap binding — is not a
  weaker version of this law, it is a different one: it would make `t` collapse to 1 and put the
  aircraft's commanded heading on the station rather than on a point ahead of its own nose.

What remains unread in the fly-to arm is therefore narrower than "where does the member head":
it is **the unit direction at `[ESP+4Ch]`/`[ESP+50h]` and the altitude offset at
`[ESP+5Ch]`/`[ESP+28h]`**, plus the five earlier store sites that can pre-empt this one.

## 6. What is bound, and what is not

Bound, pure, no globals: `include/bsp/plane_follow_law.hpp` + `src/plane_follow_law.cpp`,
`plane_follow_flyto_command_009bee30` and `plane_follow_blended_altitude_009bfaac`. Builds
clean (`./scripts/build.ps1`, exit 0). Reuses `dive_bomb_interpolate_clamped_00419010` rather
than adding a second copy of `00419010`.

**Not yet bound, and the honest blocker for switching the host off placement:** the steer
direction. Section 5.5 settles the point's *form* — own position plus 250 m along a unit
direction — so what is missing is that direction and the carrot's altitude offset, computed
somewhere in `009C0026`-`009C1661` (~1500 instructions, five further `state+44h` store sites
that can pre-empt the fall-through one). The commanding law above consumes the point; it cannot
manufacture it, and section 5.5 shows the cheap station-point substitution would be a
*different* law rather than a weaker one, so it is not taken.

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
| `009BFEE0` fly-to arm, fall-through | `009C1662`-`009C16CF` | **read** (section 5.5) |
| `009BFEE0` fly-to arm, the rest | `009C0026`-`009C1661` | **OPEN** — the blocker; the steer DIRECTION and five earlier `+44h` sites |
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
