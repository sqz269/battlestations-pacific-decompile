# The plane follow HOLD arm: how a wing member on station flies formation

Addresses: 009BEE30 (hold arm 009BEE56-009BF9E5, read in full), 009BFD70 (the `+85h` latch,
re-read), 009BE050, 00605070, 0042BE90, 009C1FD0 (call sites; search loop 009C216D-009C22F5
PARTIAL), 009A4940 (no Ghidra function).

Packet `cc9_follow_package`. Read-only analysis of the original image plus a pure rule,
`include/bsp/plane_follow_hold.hpp` / `src/plane_follow_hold.cpp`: reconstructed and build-tested,
**not wired** into any host seam, not fixture- or game-validated. Descriptive names are
hypotheses, not recovered symbols; the gain names are the game's own Lua keys.

## 1. Why this was read now: E2

E2 (feed `control_mode_370`, dive-bomb follow placement off) is a FAIL, not the null. It
releases 20 bombs against E1's 30 and has 24 water contacts against 16, and eight wing members
never leave `follow` and drown 11-20 m/s below their spawn seed. The numbers, the per-unit table
and the decision are in `docs/FOLLOWER_ATTACK_HANDOVER.md` section 11. That section also records
that the host's fly-to binding writes `plan+2B4h` but not the image's `009BFD15`/`009BFD1C`
stores, so its desired speed never reaches the throttle planner.

This document answers the question section 11 leaves: what the image commands for a member that
is **on** station, where the fly-to law does not run at all.

## 2. The gate and the shape of the arm

```
009BEE36  MOV ESI,ECX                ; the follow state
009BEE3F  LEA EDI,[ESI+4]            ; &approach; [EDI] = approach
009BEE42  MOV byte [[approach+18h]+2E5h],1   ; both arms
009BEE49  CMP byte [ESI+85h],0
009BEE50  JZ  009BF9EA               ; out of position -> fly-to arm (PLANE_FOLLOW_LAW.md §5)
009BEE56  ...                        ; in position -> HOLD arm
009BF9E5  JMP 009BFD39               ; shared epilogue
```

The whole function is 997 instructions (the handover's "997" is the function, not the arm);
the hold arm is 741 of them. `[approach+4]` is the member (EBX), `state+2Ch` the leader (EBP),
`[approach+18h]` the pilot plan, `state+6Ch` the tuning block at singleton+380h.

In order, the arm does:

| range | what |
| --- | --- |
| `009BEE56`-`009BEEF1` | frame setup: station and leader forward into the member's body frame |
| `009BEEF2`-`009BF0E8` | the **lock**: copy the leader's velocity and return (human-led formations only) |
| `009BF0EB`-`009BF295` | the **sight** correction from `state+78h`/`+84h` |
| `009BF29C`-`009BF50D` | yaw, pitch and roll commands from the `yf_`/`pf_`/`rf_` gains |
| `009BF50D`-`009BF6E5` | blend toward the steer point when the leader banks past 0.5 rad |
| `009BF6E9`-`009BF789` | the three attitude stores |
| `009BF793`-`009BF9C8` | the **power** law: throttle and air brake |
| `009BF9D2`-`009BF9E2` | `singleton+66Ch` (`Pilot/AutoStrafeAngle/Angle_Prepare`) into `[approach+1Ch]+40h` |

Stack offsets below are body-frame offsets (`B+nn`, ESP after the four prologue pushes),
normalised by a script over the listing with the callee cleanups `004142E0` RET 8, `0042D0D0`
RET 8, `00438B10` RET 8, `00415620` RET 4, `00419010` RET 14h, vtable `+34h` one argument; the
walk found no depth mismatch at any join. x87 register forms were checked against Capstone,
because Ghidra prints `DC C9` (`fmul st(1),st(0)`, at `009BF5E3`) as `FMUL ST1`, which reads as
the opposite destination.

## 3. What the arm reads

**Frame setup.** Twice the lazy refresh (`CMP [unit+10Ch],0` / `00414DB0` / `00B63D50` building
the inverse at `unit+110h` from the pose at `unit+CCh`), then:

* `009BEE92 004142E0(this=state+30h, out=B+54, m=member+110h)`: the **station** (`state+30h`,
  written by `009BFD70`'s `007F23A0` call) in the member's body frame, `S = (B+54, B+58, B+5C)`.
* `009BEEED 0042D0D0(dst=B+60, src=leader+ECh, m=member+110h, normalize=0)`: the leader's
  **forward** row as a direction in the member's frame, `Lf = (B+60, B+64, B+68)`.

**Per-aircraft scalars**, read for both leader and member:

| field | meaning (evidence) |
| --- | --- |
| `unit+C64h` | pitch (`007C1966 FSTP [ESI+0C64h]`, host `game_hosts_units.cpp`) |
| `unit+C68h` | bank |
| vtable `+50h` | heading, `0074E260` `FLD [ECX+0C6Ch]` |
| `unit+C70h` | turn rate; the arm pairs it with `rf_hdgV_radPerSec`, "turn speed difference" |
| vtable `+38h` | speed, `007B8E60 BSP_Unit_GetCachedSpeed` (plane vtable `00D05F20` slot `+38h` = `00D05F58`) |
| `unit+B50h/B54h/B58h` | flight controller `ctl+A0h/A4h/A8h` (`ctl = unit+AB0h`); paired with `rf_rollV`, `pf_pitchV`, `yf_yawV`, so roll, pitch and yaw rate |

Every difference is **leader minus member**, wrapped: `00438B10` is `FLD [ESP+4] / FSUB
[ESP+8]` then a wrap into (-pi, pi], and the leader's value is always the lower-address argument.

**The gains.** `state+6Ch` + `6Ch`..`A0h` = singleton `+3ECh`..`+420h`, all loaded by the host
already (`include/bsp/game_tuning_singleton.hpp`). This installation's
`scripts/datatables/planeglobals.lua` (mtime 2024-10-29) authors them with comments:

| block | key | value | comment in the Lua |
| --- | --- | --- | --- |
| `+6Ch` | `yf_hdg_rad` | 1/DEG(10) | heading difference multiplier |
| `+70h` | `yf_yawV_radPerSec` | 0/DEG(10) | yaw speed difference multiplier |
| `+74h` | `yf_sidepos_meter` | 1/25 | side position difference multiplier |
| `+78h` | `yf_sidedir` | 1/25 | side position difference multiplier |
| `+7Ch` | `pf_pitch_rad` | 1/DEG(10) | pitch difference multiplier |
| `+80h` | `pf_pitchV_radPerSec` | 1/DEG(100) | pitch speed difference multiplier |
| `+84h` | `pf_vertpos_meter` | 1/25 | vertical position difference multiplier |
| `+88h` | `pf_vertdir` | 1/25 | vertical position difference multiplier |
| `+8Ch` | `rf_roll_rad` | -1/DEG(30) | roll difference multiplier |
| `+90h` | `rf_rollV_radPerSec` | -0/DEG(80) | roll speed difference multiplier |
| `+94h` | `rf_hdg_rad` | 0/DEG(40) | heading difference multiplier |
| `+98h` | `rf_hdgV_radPerSec` | 0/DEG(400) | turn speed difference multiplier |
| `+9Ch` | `pwr_back_meter` | 1/10 | lagging behind multiplier |
| `+A0h` | `pwr_spd_meterPerSec` | 1/KMH(10) | velocity difference multiplier |

## 4. The lock, `009BEEF2`-`009BF0E8`

Taken only when the leader exists and its published `+520h` byte is set
(`009BEF09 CMP byte [EBP+EAX*8+9C2h],0`, `EAX = word [00F876B8]`). `docs/BOMBER_AFTER_TASK.md`
6c reads that byte, provisionally, as "under human control". Then all of:

| test | constant (width, address) |
| --- | --- |
| `|S|^2 < 180` (`009BEF33`) | float `[00D049FC]` = 180.0 |
| `|d pitch| < 5 deg` | dword `[00CEDF5C]` = 0.0872665 |
| `|d bank| < 8 deg` | dword `[00D20A18]` = 0.1396263 |
| `|d heading| < 5 deg` | the same `XMM1` |
| `|d speed| < 1.5` | dword `[00CE380C]` = 1.5 |

and it sets `member+520h = 1` (`009BF0B8`), `plan+270h = leader` (`009BF0C4`), calls
`009BE050(member, leader->vtable[34h](&tmp))` and **returns from the step**. `009BE050`
(`__thiscall`, RET 4) copies the vector `007BBB70 BSP_Unit_CopyVectorAC8` returned into
`member+AC8h..AD0h` (`ctl+18h`, the world linear velocity), copies the globals
`[00F87574..7C]` into `ctl+24h..2Ch` (BSS, zero at load; writers not checked), and calls
`007D9C10 BSP_PlaneFlightController_RefreshBodyFrame`. So a member of a human-led formation
that is within 13.4 m of its station and matched in attitude and speed is snapped to the
leader's velocity and stops commanding. If the provisional reading of the byte holds, this never
runs for an AI leader.
On every other path `009BF0F3` clears `plan+270h`.

## 5. The sight correction, `009BF0EB`-`009BF295`

`B+1C` and `B+28` start at zero. If `state+84h` is set and there is a leader, `state+78h` is
taken through the leader's pose (`009BF13D`) then the member's inverse (`009BF177`,
`00414E10`) into `P`. With `P.z > 1.0` (dword `[00D7A24C]`):

```
x/z, y/z                                                   009BF1B7-009BF1C3
|x/z| < 0.15  and  |y/z| < 0.15   (float [00CE7818]; 0042BE90 is |*ECX| on ST0)
    B+1C = clamp(20 * y/z, -1, 1)      double [00CE3D88]; ClampFloatByRef 00415620
    B+28 = clamp(30 * x/z, -1.2, 1.2)  double [00CE7630]; dwords [00CE3814]/[00D05EA4]
else  state+84h = 0                                        009BF295
```

**Producer, PARTIAL.** `state+84h`/`+78h..80h` are written by `009C1FD0`'s loop
`009C216D`-`009C22F5`: it zeroes `+78h`/`+7Ch`, walks a list from `008053C0(...)+DE8h`, keeps
entries whose vtable `+5Ch(5)` is true, projects their positions, keeps a best candidate into
`+80h`, and stores the found flag at `009C22F5`; on success `009C230B` sets
`member+914h = 1.0f`. `009BEDC3` (enter) and `009C2A30` (construct) clear `+84h`. Together
with the `Angle_Prepare` store at the end of the arm, this reads as an opportunistic
auto-strafe aim while holding formation. That identity is **not established**.

## 6. The attitude commands, `009BF29C`-`009BF50D`

With `dH`, `dP`, `dB`, `dT` the wrapped leader-minus-member heading, pitch, bank and `C70h`
differences, `rA0/rA4/rA8` the controller rates, and

```
f = InterpolateClamped(0.8, 0, 0.2, 1, |leader bank|)     009BF35E; floats [00CE74F8], [00CE54A0]
```

(1 up to 0.2 rad of leader bank, 0 from 0.8 rad), the arm forms:

```
yaw   = yf_hdg*(f*dH) + yf_yawV*(L.rA8 - M.rA8)
      + yf_sidepos*S.x + yf_sidedir*Lf.x                          -> B+34 after + B+28 (009BF3C5)
pitch = pf_pitch*(f*dP) + pf_pitchV*(f*L.rA4 - M.rA4)
      + (0.6f + 0.4)*(pf_vertpos*S.y + pf_vertdir*Lf.y)           doubles [00CEFF98], [00CE65D0]
                                                                  -> B+28 after + B+1C (009BF476)
roll  = rf_roll*dB + rf_rollV*(L.rA0 - M.rA0)
      + rf_hdg*(f*dH) + rf_hdgV*(f*dT)                            -> B+3C (009BF50D)
```

`f*dH` is kept as a double at `B+48` (`009BF37D FST`) and reused by the roll. With this
installation's gains the rate terms of yaw and roll are zero, the heading term of roll is zero,
and `rf_roll` is negative.

## 7. The blend, `009BF50D`-`009BF6E5`

Taken only when `|leader bank| > 0.5` (dword `[00CE3800]`; `009BF531 JBE 009BF6E9`). The steer
point `state+44h` (from `009BFEE0`'s own hold arm) goes into the member's frame
(`009BF57F`), its `z` is raised to at least 80 (dword `[00CE5444]`), it is normalised
(`00419510`), and

```
yaw   = InterpolateClamped(0.5, yaw,   0.8, clamp(8*n.x, -1, 1), |leader bank|)   009BF6A3
pitch = InterpolateClamped(0.5, pitch, 0.8, clamp(8*n.y, -1, 1), |leader bank|)   009BF6DA
```

(double `[00CE3DB0]` = 8.0). So a member follows the leader's attitude in gentle turns and
points at the steer point in steep ones.

## 8. What the arm writes

| site | field | value |
| --- | --- | --- |
| `009BF6F4`/`009BF702`/`009BF70B` | `plan+29Ch` / `+2A0h` / `+2D0h` | pitch desired, slot 3 active, mode 0 |
| `009BF716`/`009BF729`/`009BF730` | `plan+284h` / `+288h` / `+2D4h` | yaw desired, slot 1 active, mode 0 |
| `009BF743`/`009BF74B`/`009BF752` | `plan+290h` / `+294h` / `+2CCh` | roll desired, slot 2 active, mode 0, when `|leader bank| < 0.75` (dword `[00CEE07C]`) |
| `009BF783`/`009BF789` | `plan+2C4h` / `+2CCh` | otherwise: bank target `00605070(L.bank - L.rA0)`, mode 1 |
| `009BF9AA`/`009BF9B2` | `plan+278h` / `+27Ch` | throttle desired, slot 0 active |
| `009BF9B9`/`009BF9C1` | `plan+2A8h` / `+2ACh` | air brake desired, slot 4 active |
| `009BF9C8` | `plan+2D8h` | 0 |
| `009BF8C4` / `009BF8DE` | `state+90h` / `+94h` | the two power terms |
| `009BF9E2` | `[approach+1Ch]+40h` | `singleton+66Ch` |

Slots are `plan+274h + 0Ch*i`, `{current, desired, active}`, in `0099B450`'s order: 0 throttle,
1 yaw, 2 roll, 3 pitch, 4 air brake (`include/bsp/pilot_plan_slots.hpp`). The mode words
`+2CCh/+2D0h/+2D4h` are the roll/pitch/yaw modes `0099D300` reads
(`include/bsp/plane_ai_control.hpp`). So the hold arm **drives the stick slots directly**,
where the fly-to arm hands the planner a heading point (`009F9E40`), an altitude
(`009F9ED0`) and a desired speed with `plan+2D8h = 1`. The yaw, pitch and roll values are not
clamped by the arm except through the blend endpoints.

`00605070` (`__thiscall(float*)`, RET) wraps in place: `fmod` by double `[00CE3828]` = 2pi (CRT
`00BF857A`), then one `+2pi` if not above double `[00CE3D18]` = -pi, or one `-2pi` if above
double `[00CE3D28]` = pi. The bank-target expression subtracts a rate from an angle; it is
recorded as read.

## 9. The speed law is a throttle law

There is **no commanded speed** in the hold arm. It sets throttle and air brake directly.

```
dv   = L.speed - M.speed        (L.speed kept as a double, 009BF79D / 009BF7AA)
back = S.z                      (station distance ahead of the member, body frame)
p    = pwr_spd * shape(dv) + pwr_back * shape(back)     state+90h, state+94h, 009BF8E7
floor = InterpolateClamped(LFS - 5.5556, +1, LFS + 8.3333, -1, M.speed)   009BF945
u    = p > floor ? p : floor                             009BF956
throttle = clamp(u, 0, 1)       -> plan+278h
brake    = clamp(-u, 0, 1)      -> plan+2A8h
```

`LFS` is `007C47F0 BSP_PlaneClass_LevelFlightSpeed` on `[approach+8]`, called twice with no
stack arguments (`009BF90D`, `009BF92E`); the offsets are doubles `[00D20A08]` = 5.5556
(KMH(20)) and `[00D20A10]` = 8.3333 (KMH(30)). `shape` (`009BF7CA`-`009BF834` for `dv`,
`009BF836`-`009BF8B9` for `back`, the same code twice) is continuous and odd:

```
|x| >= 10          x                    float [00CE38B8], dwords [00CE38B8]/[00CE6848]
5 < x < 10         x - 0.6*(10 - x)     dword [00CE3850], double [00CEFF98]
-5 < x <= 5        0.4*x                dword [00CFBC84], double [00CE65D0]
-10 < x <= -5      x + 0.6*(x + 10)
```

With this installation's gains a member 10 m behind its station (`back = +10`) gets +1.0 of
throttle from position alone, and one 10 km/h slower than its leader gets +0.4 from speed.
The **floor** is the part that answers E2: a member slower than its class level-flight speed
minus 20 km/h gets full throttle regardless of position, and full throttle fades to full brake
across the 50 km/h window above that.

So the image's answer to "how is the member's commanded speed derived from the leader's" is:
it is not. The leader's speed enters only as a difference, the station enters as a distance
ahead, and the class's level-flight speed sets a floor. There is no `TravelSpeed` or
`desc+18Ch` clamp in this arm; the fly-to arm's `classDesc+188h` floor does not appear here.

## 10. The `+85h` latch, `009BFD70`

Recomputed every tick with **no hysteresis**:

```
009BFDD4  MOV byte [ESI+85h],0                         cleared first
009BFE19  d = |memberPos(+FCh..104h) - station|        0042B2F0, 3-D
009BFE28  JBE 009BFEA8   unless block+18h > d          GoodPositionDist, 100 here
009BFE62-7B  c = leaderFwd(+ECh..F4h) . memberFwd      3-D dot
009BFE8C  JBE 009BFEA0   unless c > block+14h          GoodPositionDir, 0.5 here
009BFE95  MOV byte [ESI+85h],AL   (AL = 1)
009BFEA2  MOV byte [ESI+85h],AL   (AL = 0)
009BFEB1  MOV byte [ESI+85h],0    the path with no station (not re-read here)
```

A member is "in good position" exactly while it is within 100 m of its station and heading
within 60 degrees of its leader. `009BFEE0` branches on the same byte one call earlier
(`009BFEEE`), so geometry and command always take matching arms. The arms therefore alternate
freely at the 100 m boundary; nothing in the image smooths the switch.

## 11. Call sites

`tools/callsite_census.py`:

* `009BEE30` has one reference, `CALL` at `009C2077` in `009C1FD0 BSP_BotStateFollow_Tick`.
  `scan-bytes '30 ee 9b 00'` finds no pointer.
* `009C1FD0` has six references: vtable `00D20AB8` slot `+0Ch` (`00D20AC4`), and `CALL`s at
  `009C7278` (`009C7270`, dive-bomb done), `009D2731` (`009D2720`, torpedo done/prepare),
  `009AD371` (`009AD1F0`, referenced only from `.rdata 00D1FC14`), `009B6A75` (`009B6670`,
  only from `.rdata 00D201AC`) and `009A4950`.
* `009A4950` lies in a routine Ghidra never defined: `009A4940`-`009A4CDA`, bounded by INT3 at
  `009A493F` and `009A4CDB`, with exits at `009A4A83`, `009A4AE2`, `009A4B55` and `009A4CD8`
  and one reference, `.rdata 00D1F708`. Ghidra's `009A4900` body ends at `009A493E`, so the
  census tool's label for that call is wrong.

**The torpedo follow seam reaches the hold arm.** The host's `follow_base_tick_009c1fd0` models
`009D2731`, which calls `009C1FD0`, which calls `009BEE30` at `009C2077`. So in the image a
torpedo bomber in `done`/`prepare` runs the same hold and fly-to arms as a dive-bomb wing
member, gated by the same `+85h`.

## 12. What is reconstructed, and what is not

* `bsp::plane_follow_hold_command_009bee56` and `bsp::plane_follow_hold_shape_009bf7ca`:
  the arithmetic of sections 4-9, with the frame quantities as explicit inputs. It reuses
  `wrapped_angle_subtract_00438b10` and `dive_bomb_interpolate_clamped_00419010`.
  Build-tested only.
* **Not wired.** Wiring needs, per member, the controller rates `ctl+A0h/A4h/A8h`, `unit+C70h`,
  the leader's forward row, a body-frame transform, and the `+85h` gate the host does not
  model. The host runs its fly-to binding on every tick regardless of position.
* **Not read:** the identity of `009C1FD0`'s search list and of `member+914h`; the consumer of
  `[approach+1Ch]+40h`; the writers of `[00F87574..7C]`; what `009A4940`, `009AD1F0` and
  `009B6670` are (three more users of the follow tick); `009BFEB1`'s path in `009BFD70`.

## 13. What this means for the host

1. The image's follow is two controllers switched by a 100 m / 60 degree gate. The host has
   one, the fly-to binding, running at all ranges, and it drops the fly-to arm's `plan+2D8h = 1`
   so no speed reaches the throttle. A member that is on station in the host is flown by a law
   the image never applies on station, and the follow law issues no throttle command.
2. The cheapest faithful step is the two missing fly-to stores (`009BFD15`, `009BFD1C`).
   The larger step is the hold arm, which additionally needs the rate inputs listed above.
   Either needs a same-binary control run before it lands.
