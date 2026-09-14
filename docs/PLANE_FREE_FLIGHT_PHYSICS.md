# The plane free-flight physics (`007DB680`, free-flight arm)

Addresses: 007DB680, 007D7C00, 007D8470, 007D9050, 007D9140, 007D92B0, 007D9C10, 007D99C0,
00419010, 0042D0D0, 00F872F0 (the `Dynamics/*` mirror), 00F87574 (not the mirror).

This packet re-derives the free-flight arm of `007DB680 BSP_PlaneFlight_CoreLaw` from the
assembly listing and publishes it as a pure rule in `include/bsp/plane_flight.hpp` and
`src/plane_flight.cpp`. `docs/PLANE_FLIGHT_CORE_LAW.md` recovered the law first; this document
records what held under a second reading and what did not. Every name is a hypothesis, not a
recovered symbol.

## 1. The shape of the native step, and why the rule has the shape it has

`007DB680` is **not an integrator**. It is an accumulator pass:

| address | what it does |
| --- | --- |
| `007DB690` | `007D81B0` |
| `007DB697` | `007D9C10 BSP_PlaneFlightController_RefreshBodyFrame` rebuilds `ctl+0B0h` and `ctl+3Ch` |
| `007DB6A6` | `007DA710`, the rate law, with `step` |
| `007DB6B6` | `007D7C00 BSP_PlaneDynamics_BeginStep(ctl+18h, ctl+3Ch)` |
| `007DB744`-`007DBEA8` | one term per force, each added into the accumulator its frame belongs to |
| `007DC6E6` | `007D8470` folds the world accumulators into their body partners through `ctl+0B0h` |

`007D7C00` (`RET 8`, `__fastcall(dyn, const float* world_vel, const float* body_vel)`) copies
`ctl+18h..20h` into `dyn+4Ch..54h` and `ctl+3Ch..44h` into `dyn+64h..6Ch`
(`007D7C04`-`007D7C27`), then writes the three floats at `00F87574`/`+78h`/`+7Ch` into **six**
3-float accumulators at `dyn+04h`, `+10h`, `+1Ch`, `+28h`, `+34h`, `+40h`
(`007D7C2A`-`007D7D0F`, in that listing order: `+1Ch`, `+28h`, `+04h`, `+10h`, `+40h`, `+34h`),
and finally `dyn+94h = 0` (`007D7D14` `XORPS` / `007D7D17`). The clear order is the direct
evidence for the six-accumulator layout: the offsets are exactly `dyn+04h + 0Ch*k`.

`007D8470` pairs them. `MOV ESI,[ESP+0x54]` (`007D8476`) takes the matrix argument, which
`007DC6DA PUSH EDX` with `EDX = ESI+0B0h` proves is `ctl+0B0h`; each fold calls
`0042D0D0(out, src, matrix, 0)` and adds the result into the partner:

| fold | source | destination | listing |
| --- | --- | --- | --- |
| 1 | `dyn+28h` world | `dyn+1Ch` body | `007D8480`-`007D84AF` |
| 2 | `dyn+10h` world | `dyn+04h` body | `007D8499`-`007D84DB` |
| 3 | `dyn+34h` | `dyn+40h` | `007D84C2`-`007D84FF` |

The direction is settled without assuming it: `007D9C2D`-`007D9C51` calls the same
`0042D0D0` with `EDX = ctl+18h` (the **world** velocity `007D7C00` was handed) and
`EDI = ctl+0B0h`, and stores the result into `ctl+3Ch..44h`, which `007DBD3A`/`007DBD50` pair
with `XDrag`/`YDrag` as the **body** velocity. The matrix maps world to body, so folds 1 and 2
express every force in the body frame. `007D8502`-`007D85A5` then applies a **deadband**: each
component of `dyn+1Ch` and `dyn+04h` whose magnitude is under `0.001f` (`00D7A23C`) is zeroed.

That is why the published rule is `accumulate_free_flight_007db680` (the four accumulators free
flight writes) plus `fold_world_into_body_007d8470` (the two folds and the deadband), rather
than one "advance the plane" call. **`007D8470`'s integration tail `007D8624`-`007D904E` was
not read by this packet or the previous one**, so the rule stops where the proof stops. What the
host needs to make a plane move - a body-frame acceleration for the step - is what
`fold_world_into_body_007d8470` returns.

Free flight writes four of the six accumulators:

| accumulator | frame | terms |
| --- | --- | --- |
| `dyn+04h` | body | `+04h` lateral damping, `+08h` vertical damping, `+0Ch` the ceiling's forward push |
| `dyn+10h` | world | drag along the velocity direction |
| `dyn+1Ch` | body | `+20h` lift, `+24h` thrust |
| `dyn+28h` | world | `+2Ch` gravity, the second gravity term and the ceiling |

## 2. The terms, as verified

Body axes are `(x lateral, y up, z forward)`: `007DBD3A` pairs `ctl+3Ch` with `XDrag`,
`007DBD50` pairs `ctl+40h` with `YDrag`, and `007D99C0` is built on `ctl+44h`.

### 2.1 Thrust (`007DB744`-`007DB80A`) - `dyn+24h`, body forward

```
007db76c  if (unit[0xbbc] <= 0.01f) skip the whole term        ; 00D7A238
007db7a6  a = 007D9050(desc, ctl[4], unit[0xc64] pitch, unit[0xbbc] throttle, ctl[5])
007db7bf  a *= unit[0xcc8]
007db7c7  m = (00E0C978 != 0 && [00F88C30][0xd0] != 0) ? 008E6430(6, unit) : 1.0f   ; 00D7A24C
007db80a  dyn[0x24] += m * a
```

`007D9050`'s three stack arguments are `[ESP]` = pitch, `[ESP+4]` = `unit+0BBCh` (the value
saved at `007DB755`, reached as `[ESP+0x30]` after `PUSH ECX` + `SUB ESP,8`) and `[ESP+8]` =
the byte pushed at `007DB787`. The rule takes the product as the single input
`thrust_accel`; `007D9050`'s body is not reconstructed here (`coverage: partial`).

### 2.2 Lift (`007DB875`-`007DB98D`) - `dyn+20h`, body up

```
007db875  s   = (vz > 0.0f) ? vz : (-0.0f - vz)                ; 00D7A218 = 0.0f, 00D7A208 = -0.0f
007db8a7  aoa = (0.1 <= s) ? (-vy / vz) : 0.0f                 ; 00D7A3A0, double 0.1f
007db8c1  q   = (007D99C0(ctl) / desc[0x184] StallSpd) / LevelFlight
007db8cf  q  *= ctl[0x9c]
007db8e3  qq  = (q < 1.0f) ? q * q : 1.0f
007db8f1  L   = (1.0f + aoa) * qq
007db91d  v   = (-2.0f > L) ? -2.0f : (L > 2.0f ? 2.0f : L)    ; 00CE7D7C, 00CE3958
007db951  m   = min(v * AccelCheatMul, ctl[0x90])
007db98a  dyn[0x20] += m * 9.81f                               ; 00CF9058
```

The x87 stack through `007DB8DD`-`007DB8F1` resolves as the previous packet described: both
paths leave `1 + aoa` in `ST0` and `qq` in `ST2` at `007DB8F1`, and both leave the stack one
deeper than it started, carrying a spare `1.0f` that the **gravity** block below consumes.

`007D99C0` is `ctl+44h` (`007D99CA`, the body forward velocity) when `unit+3Ch` is null, and
`ctl+44h` plus a carrier-relative correction otherwise (`007D9A09`-`007D9A5F`, through
`0042D0D0` and a `00D7A2F0`/`00CE74F8` sign selection). The rule takes it as the input
`forward_speed`.

The cap `ctl+90h` is advanced at `007DB80D`-`007DB874`, immediately before the lift reads it:
`007BBC50(unit)` true pins it to `3.0f` (`00CE3854`), otherwise `+step` below `3.0f`,
`+3*step` below `6.0f` (`00D7A2B0` is the double 3.0, `00CE6630` is 6.0f), and it stops at
`6.0f`. Construction leaves it at `99.0f`, so **the cap never binds**: `v * 1.5` is at most
`3.0` and `ctl+90h` is at least `3.0`.

### 2.3 Gravity (`007DB990`-`007DBA2F`) - `dyn+2Ch`, world up

```
007db9c4  g1 = InterpolateClamped(0.0f, 1.0f, LostDragTime, ExtraGravityMul, unit[0xc3c])
007db9db  dyn[0x2c] -= AccelCheatMul * 9.81f * g1
007db9de  if (ctl[0x94] != 0) {
007dba20    g2 = InterpolateClamped(1.0f, 0.0f, 2.5f, 3.0f, unit[0xc3c])   ; 00CF87C8, 00CE3854
007dba2f    dyn[0x2c] -= 9.81f * g2                                        ; no AccelCheatMul
            }
```

`y0 = 1.0f` in the first call is not pushed by any `FLD`: `007DB9BB FSTP [ESP+4]` pops the
`1.0f` that `007DB8DD`/`007DB8E1`'s `FLD1` left behind. Counting every push and pop from
`007DB89D` to `007DB9BB` gives a stack depth of exactly one there, and the value is that
`1.0f` - which is why the term is a plain `1.0` and not an authored row.

### 2.4 Drag (`007DBA32`-`007DBC76`) - `dyn+10h`, world

```
007dba63  s2 = |ctl[0x18..0x20]|^2
007dba75  if (s2 <= 1e-10) { speed = 0; goto 007DBD37 }                   ; 00CE3820, double
007dba7b  speed = sqrt(s2)                                                ; 00BF7030
007dba99  if (speed <= 0.001f) goto 007DBD37                              ; 00D7A23C
007dbab3  n  = ctl[0x18..0x20] / speed
007dbb0e  r1 = InterpolateClamped(0.0f, 1.0f, LostDragTime, 0.0f, unit[0xc3c])
007dbb23  r2 = InterpolateClamped(-0.3f, r1, 0.1f, 1.0f, unit[0xc64])     ; 00D06888, 00D7A2F0
007dbb5e  d  = 007D9140(desc, ...) * r2
007dbb72  if (ctl[0x44] < 0.0f) d *= 5.0                                  ; 00D7A370, double
007dbbc6  dyn[0x10..0x18] += n * d
007dbbf3  if (ctl[0x98] > 1.0f)
007dbc43    dyn[0x10..0x18] += (n.x * d, d * 0.0, n.z * d) * (ctl[0x98] - 1.0)
```

`r2`'s three trailing arguments are the slots `007DBAD3`-`007DBAE7` wrote above the first
call's frame (`[ESP+0x14]` = `0.1f`, `[ESP+0x18]` = `1.0f`, `[ESP+0x1C]` = `unit+0C64h`); the
`SUB ESP,8` at `007DBB13` lowers the frame by only the two slots the second call replaces.

The rule takes `007D9140(...) * r2` as the single input `drag_accel`. Its tail multiplies in
`-sgn`, so the value is negative for forward motion; the rule does not re-derive it
(`coverage: partial`, as `docs/PLANE_FLIGHT_CORE_LAW.md` already labelled it).

### 2.5 Body damping (`007DBD37`-`007DBE0D`) - `dyn+04h`, `dyn+08h`

```
007dbd61  r = 007D92B0(forward_speed / StallSpd)
007dbd7d  dyn[0x04] += r * (-ctl[0x3c] * desc[0x174] XDrag)
007dbd8a  dyn[0x08] += r * (-ctl[0x40] * desc[0x170] YDrag)
007dbdbd  c = InterpolateClamped(0.5f, 1.0f, 20.0f, 100.0f, unit[0x908])  ; 00CE3800/3930/3D08
007dbe0a  dyn[0x08] = (-c > dyn[0x08]) ? -c : (dyn[0x08] > c ? c : dyn[0x08])
```

`007D92B0` (`RET 4`, one float argument) is
`u = InterpolateClamped(DragRangeMin, 0, DragRangeMax, 1, ratio)`; `u == 0` returns 0
(`007D92EA` `FUCOMIP` read through `LAHF` / `TEST AH,44h`, where `JP` is the not-equal branch);
otherwise `pow(|u|, DragFuncPower)` through `FYL2X` / `F2XM1` / `FSCALE`, with `|u|` formed as
`-0.0f - u` (`007D9313`, `00D7A208`) exactly as the angle of attack forms `|vz|`.

### 2.6 The ceiling (`007DBE0E`-`007DBEA8`), free flight only

```
007dbe16  if (ctl[0xfc] != 0) goto the ground / water arms
007dbe42  f = -(unit[0x100] - Ceiling) * CeilingForce
007dbe58  if (0.0f > f) {                                      ; i.e. altitude above Ceiling
007dbe6b      dyn[0x2c] += f
007dbea2      dyn[0x0c] += InterpolateClamped(1.5f, 1.0f, 0.5f, 0.0f, spdRatio) * f
            }
```

`y1 = 0.0f` in that call is another x87 leftover: `007DBE54 FLDZ` supplies the comparison at
`007DBE56`, survives the `FADDP`/`FXCH`/`FSTP` that commit `dyn+2Ch`, and is popped into
`[ESP+0xC]` at `007DBE79`. `[ESP+0x28]` at `007DBE71` is the same stack slot `007DB773` wrote
(`ESP` is `base-0x18` there against `base-4` then), so the interpolation's `x` is the speed
ratio, not a second quantity.

## 3. Provenance of every constant

| value | where it comes from | address | read at |
| --- | --- | --- | --- |
| `0.0f`, `-0.0f` | `.rdata` float literals (the `fabs` idiom) | `00D7A218`, `00D7A208` | `007DB87A`, `007DB88B` |
| `0.1f` (aoa gate) | `.rdata` **double** `3FB99999A0000000` = `(double)0.1f` | `00D7A3A0` | `007DB8A1` |
| `-2.0f`, `+2.0f` | `.rdata` float literals | `00CE7D7C`, `00CE3958` | `007DB913`, `007DB8F5` |
| `9.81f` | `.rdata` **double** `40239EB860000000` = `(double)9.81f` | `00CF9058` | `007DB981`, `007DB9CF`, `007DBA25` |
| `3.0f`, `6.0f`, `3.0` | `.rdata` literals (lift-ramp timer) | `00CE3854`, `00CE6630`, `00D7A2B0` | `007DB81D`, `007DB854`, `007DB865` |
| `1.0f`, `0.0f` in the interpolations | **immediates** `FLD1` / `FLDZ`, or x87 leftovers | - | `007DB9BB`, `007DB9BF`, `007DBA15`, `007DBA1B`, `007DBE79`, `007DBE87` |
| `2.5f`, `3.0f` (second gravity) | `.rdata` float literals | `00CF87C8`, `00CE3854` | `007DBA0B`, `007DBA01` |
| `1e-10`, `0.001f` | `.rdata` double / float | `00CE3820`, `00D7A23C` | `007DBA67`, `007DBA92`, `007D850E` |
| `-0.3f`, `0.1f` (drag ramp) | `.rdata` float literals | `00D06888`, `00D7A2F0` | `007DBB1A`, `007DBAE1` |
| `5.0` (flying backwards) | `.rdata` double | `00D7A370` | `007DBB78` |
| `0.0`, `1.0` (the `ctl+98h` term) | `.rdata` doubles | `00D7A258`, `00D7A210` | `007DBC03`, `007DBC15` |
| `0.5f`, `20.0f`, `100.0f` | `.rdata` float literals (damping clamp) | `00CE3800`, `00CE3930`, `00CE3D08` | `007DBDB4`, `007DBDA4`, `007DBD9A` |
| `1.5f` (ceiling fade) | `.rdata` float literal | `00CE380C` | `007DBE8D` |
| `0.01f` (thrust gate), `1.0f` (thrust default) | `.rdata` float literals | `00D7A238`, `00D7A24C` | `007DB76C`, `007DB7E6` |
| `StallSpd` **17.5** | **authored class field** `desc+184h`, written by `007D1F70` | - | `007DB760` |
| `XDrag`, `YDrag` | **authored class fields** `desc+174h`, `+170h`, written by `007D1F70` | - | `007DBD3A`, `007DBD50` |
| `Ceiling` 1500, `CeilingForce` 0.1 | **authored** `Dynamics/*`, mirror | `00F872F0`, `00F872F4` | `007DBE34`, `007DBE42` |
| `DragFuncPower` 1.8 | **authored** `Dynamics/DragFuncPower` | `00F87308` | `007D9325` |
| `DragRangeMin` 1.0, `DragRangeMax` 2.0 | **authored** `Dynamics/SpdMultipliers/*` | `00F87324`, `00F87328` | `007D92D2`, `007D92C2` |
| `LevelFlight` 1.8 | **authored** `Dynamics/SpdMultipliers/LevelFlight` | `00F8732C` | `007DB8C1` |
| `LostDragTime` 5, `ExtraGravityMul` 1.5 | **authored** `Dynamics/DeadMeat/*` | `00F87344`, `00F87348` | `007DB9B1`, `007DB9A7` |
| `AccelCheatMul` 1.5 | **authored** `Dynamics/AccelCheatMul` | `00F873FC` | `007DB931`, `007DB9C9` |

Every mirror address above reads **zero** in the PE on disk, because `007EAAE1`'s `REP MOVSD`
fills the window at load. The mapping `tuning_offset = mirror - 00F870E0` is not assumed here:
`007D92B0` alone reads `00F87324`, `00F87328` and `00F87308`, and the offsets that mapping
predicts (`+244h`, `+248h`, `+228h`) are exactly `DragRangeMin`, `DragRangeMax` and
`DragFuncPower` in `docs/GAME_TUNING_SINGLETON.md`'s key table - a three-of-three hit on one
routine, which is what makes the other six rows credible.

## 4. What this packet corrected in the recovered summary

1. **The `ctl+98h` extra drag term has a forward component.**
   **Was** (`docs/PLANE_FLIGHT_CORE_LAW.md` §4.7): `dyn[0x10..0x18] += (d*n.x, 0, 0) * (ctl[0x98] - 1)`.
   **Is:** `dyn[0x10..0x18] += (n.x*d, d*0.0, n.z*d) * (ctl[0x98] - 1.0)`.
   **Evidence:** the x87 stack at `007DBBF5` holds `ST0 = n.x*d`, `ST1 = n.z*d`, `ST2 = d`
   (built at `007DBB95`-`007DBBC2`). `007DBC03 FMUL double [00D7A258]` multiplies **`d`**, not
   `n.y*d`, by the double `0.0` at `00D7A258` and stores it as the middle component;
   `007DBC0D FSTP [ESP+0x38]` keeps `n.z*d`, `007DBC3B FMUL [ESP+0x38]` scales it by
   `ctl[0x98]-1`, and `007DBC55`-`007DBC5C` adds it into `dyn+18h`. Only the middle component
   is zeroed. The term is therefore the horizontal-plane projection of the drag, not the world
   x axis alone. (It is inert in practice: `ctl+98h` is `1.0f` from construction at `007D7F5D`.)

2. **`00F87574`-`00F8757C` is not the `Dynamics/*` mirror.** The mirror declared in
   `include/bsp/plane_flight.hpp` is `00F872F0`-`00F87427`, 0x138 bytes, so `00F87574` is
   0x14Ch past its end and the `+494h Pilot/DepthCharge/AimAltRange/1` row of
   `docs/GAME_TUNING_SINGLETON.md` cannot be at `mirror_base + 494h`. A byte scan finds **no**
   writer of `00F87574` under either store encoding (`D9 1D` `FSTP` and `F3 0F 11 05` `MOVSS`)
   and **161** readers under `F3 0F 10 05` `MOVSS`; the image bytes are zero. It is a read-only
   `Vector3` zero constant, which is what makes `007D7C00` a clear rather than a preset. This
   matters: if the three floats were an authored triple, every accumulator would start at a
   non-zero value and nothing below would hold.

3. **The three doubles that look like authored numbers are not.** `00D7A3A0` (the angle-of-attack
   gate) and `00CF9058` (gravity) are `(double)0.1f` and `(double)9.81f` bit-for-bit -
   `3FB99999A0000000` and `40239EB860000000`, the float mantissas shifted left 29 - so they are
   source literals promoted by the x87 path, not data. Read as doubles they would otherwise be
   easy to mistake for a tuning row.

4. **The second gravity term's endpoints are now named.** §4.6 recorded that `ctl+94h` gates a
   second application without stating its arguments. They are
   `InterpolateClamped(1.0f, 0.0f, 2.5f, 3.0f, unit[0xc3c])` - `007DBA1B` `FLD1`, `007DBA15`
   `FLDZ`, `00CF87C8` = 2.5f, `00CE3854` = 3.0f - and the result is multiplied by `9.81f`
   **without** `AccelCheatMul` (`007DBA25` goes straight to `00CF9058`), unlike the first term.

Everything else in §4.2, §4.4-§4.9 held instruction by instruction, including the two x87
leftovers that supply `y0 = 1.0f` to the gravity ramp and `y1 = 0.0f` to the ceiling fade, the
`-0.0f - x` absolute-value idiom in three places, and the `dyn+94h = desc[0x1fc] - unit[0xbfc]`
sign (`007DB702` loads `unit+0BFCh` and subtracts `desc+1FCh`, then `007DB724 FCHS`).

## 5. The rule's shape, and the seam

`PlaneFlightHost::free_flight_007cc2f0(float step)` stays the virtual the host calls from
`run_plane_fixed_step_007ce040`. Behind it:

```
PlaneFreeFlightClass    stall_spd, x_drag, y_drag                 (authored, desc+184h/174h/170h)
PlaneFreeFlightTuning   nine Dynamics/* rows                      (authored, the mirror)
PlaneFreeFlightState    world_velocity, body_velocity, world_to_body[9],
                        forward_speed, world_altitude, lost_drag_timer,
                        airborne_time, pitch, lift_ramp, roll_drag_scale,
                        lift_scale, extra_gravity, thrust_accel, drag_accel

accumulate_free_flight_007db680(state, cls, tuning, step, ramp_reset) -> PlaneDynAccumulators
fold_world_into_body_007d8470(acc, world_to_body)                    -> PlaneBodyAcceleration
free_flight_world_up_acceleration(state, cls, tuning, step)          -> float
```

No host, no globals, no state held between calls: every input the native reads from `ctl`,
`unit`, `desc` or the mirror is a field of one of the three structs, and the two helpers whose
bodies are not reconstructed (`007D9050`, `007D9140`) enter as the scalars `thrust_accel` and
`drag_accel`. The step argument is used only where the native uses it - `007DB80D`'s ramp -
which is why the rule returns accelerations and the caller owns the integration.

## 6. The acceptance-test numbers

Measured with `local/probe_free_flight.cpp` against the installed Fighter row
(`StallSpd` 17.5, `TravelSpeed` 141.666672), `XDrag`/`YDrag` zero, altitude 500 m,
`unit+0C3Ch` zero, the mirror at its `PlaneGlobals.lua` defaults, `step` = 1/30:

| forward speed | lift (`dyn+20h`) | gravity (`dyn+2Ch`) | net world-up |
| --- | --- | --- | --- |
| 141.666672 (spawn) | +14.71500 | -14.71500 | **0.00000000** |
| 31.500 (`1.8 * StallSpd`) | +14.71500 | -14.71500 | **0.00000000** |
| 31.400 | +14.62172 | -14.71500 | -0.09327984 |
| 20.000 | +5.93197 | -14.71500 | -8.78302670 |
| 10.000 | +1.48299 | -14.71500 | -13.23200703 |
| 0.000 | 0.00000 | -14.71500 | -14.71500015 |

Lift cancels gravity **exactly**, not approximately, and the threshold is exactly
`1.8 * StallSpd`: both sides are the same float `1.5f * 9.81f`, because `q >= 1` pins `qq` to
`1.0f`, `aoa` is zero, the `±2` clamp does not bite, and `min(1.5, ctl+90h >= 3)` is `1.5`.
Below the threshold the plane sinks, and at rest it falls at `1.5 g`, not `g` - the
`AccelCheatMul` scaling applies to gravity as well as lift.

The self-correction, at the spawn airspeed, perturbing the body vertical velocity:

| `ctl+40h` | angle of attack | net world-up |
| --- | --- | --- |
| 0 | -0.0000000 | 0.00000000 |
| -1 (sinking) | +0.0070588 | +0.10387135 |
| -10 | +0.0705882 | +1.03870678 |
| -30 | +0.2117647 | +3.11611748 |
| +10 (climbing) | -0.0705882 | -1.03870583 |

Sinking raises the angle of attack, which raises lift above gravity; climbing does the reverse,
symmetrically. At 1700 m the ceiling adds exactly `-(1700 - 1500) * 0.1 = -20.0` on top of a
still-cancelling lift/gravity pair, measured `-20.00000000`.

**The model needed nothing the host cannot supply.** The only authored value the cancellation
depends on is `StallSpd`, which `007D1F70` already parses and `docs/PLANE_FLIGHT.md` records
for the installed rows; everything else is either an image constant or a `Dynamics/*` row whose
`PlaneGlobals.lua` default is recorded. No constant was tuned to reach the expected answer -
`level_flight` and `accel_cheat_mul` came from `docs/GAME_TUNING_SINGLETON.md` before the
arithmetic was run, and the exact zero is a consequence of `AccelCheatMul` appearing on both
sides, not of fitting.

One case was added to `tests/math_tests.cpp` covering rows 1, 4 and the `-10` perturbation.

## 7. Proven versus assumed

**Proven here, from the listing or the bytes on disk.**
- §2.1-§2.6 instruction by instruction, including the three x87 stack traces (the two-path
  square, the `1.0f` leftover at `007DB9BB`, the `0.0f` leftover at `007DBE79`) and the
  `007DBBF5`-`007DBC5C` component ordering that correction 1 rests on.
- The six-accumulator layout and the clear, from `007D7C00`'s complete 53-instruction listing.
- The world-to-body direction of `ctl+0B0h`, from `007D9C2D`-`007D9C51` writing `ctl+3Ch` and
  `007DC6DA` handing the same matrix to `007D8470`.
- The fold pairing and the `0.001f` deadband, from `007D8470`-`007D85A5`.
- `007D92B0` complete (`RET 4`, `007D92B0`-`007D9358`).
- Every constant in §3, read with `ghidra bytes` from the PE on disk.
- `00F87574`'s writer census: zero writers under two store encodings, 161 readers.

**Assumed or partial.**
- `coverage: partial` for `007DB680`: this packet read the free-flight path
  (`007DB680`-`007DBEA8`) and left the ground arm `007DBEAA`-`007DC204` and the water arm
  `007DC205`-`007DC68C` to `docs/PLANE_FLIGHT_CORE_LAW.md`, which labels the water arm partial.
- `coverage: partial` for `007D8470`: only `007D8470`-`007D85A5` was read. **The integration
  tail `007D8624`-`007D904E` is unread**, so the published rule does not integrate and no
  velocity or position update in this repository is attributable to it.
- `007D9050` `coverage: none` beyond its call site; `007D9140` `coverage: none` beyond its call
  site and the previous packet's reading of `007D9287`-`007D929B`. Both enter as inputs.
- `007D99C0`'s carrier-relative branch (`007D99DA`-`007D9A5F`) was read for its shape but the
  two constants `00D7A2F0`/`00CE74F8` that select the sign were not decoded; the rule takes the
  routine's result as an input.
- `007D81B0`, `007DA710`, `007BBC50`, `008E6430`, `00414DB0`, `0085DEA0` were not read.
- `0042D0D0`'s row/column convention was not re-read here. `fold_world_into_body_007d8470`
  declares its own: `world_to_body` is row-major with `body = M * world`. The acceptance test
  drives the identity case, where the convention cannot matter; a non-level pose would depend
  on it and is **not** covered.
- `XDrag`/`YDrag`'s installed values were not read from `vehicleclasses.lua`; the test leaves
  them zero and the damping term is consequently untested against authored numbers.
- **No run-time evidence.** `bsp_game.exe` builds and the unit test passes, but the arm is not
  yet wired into the host (the integrator owns `src/game_hosts_units.cpp`), so checklist item 6
  is satisfied vacuously. The first run that can test any number in §6 is the one that binds
  `free_flight_007cc2f0` to this rule.
- The installation this evidence came from is **modded** (BSPRM/AlterBSP): `vehicleclasses.lua`
  has mtime 2026-05-09 against an untouched bulk of 2024-07-13. No file in it was modified. The
  `StallSpd` 17.5 and `TravelSpeed` 141.666672 quoted here are `docs/PLANE_FLIGHT.md`'s
  installed row, not a fresh read of the script.

## 8. Follow-up packets

1. **`007D8470`'s integration tail** (`007D8624`-`007D904E`). The single highest-value
   remaining read: until it is known, the host must either invent an Euler step or leave the
   plane still. Everything above it is now proven.
2. **`007D9050 BSP_PlaneFlight_ThrustAccel`** (`007D9050`-`007D9137`): `desc+164h Accel`,
   `tuning+330h TurboMultiplier`, the `AccelCheatFallPitchRange` interpolation and
   `AccelCheatFallMul`. Small, bounded, and it closes `thrust_accel`.
3. **`007D9140 BSP_PlaneFlight_DragAccel`** (`007D9140`-`007D92A4`) and the exact argument
   ordering at `007DBB30`-`007DBB5E`, which the previous packet took from the call site.
4. **`ctl+94h`'s producer**, still not found. It gates the second gravity term and the
   `007DBC77` branch; until it is found, `extra_gravity` is a host input with no known writer.
5. **`unit+0CC8h` and `unit+0C64h`'s producers** - the thrust scale and the pitch the drag ramp
   and `007D9050` both read.
6. **`00F87574`'s identity.** Naming it (a `Vector3` zero constant with 161 readers) would
   retire a whole class of mirror-offset confusion, and the `+494h` row of
   `docs/GAME_TUNING_SINGLETON.md` should be re-derived from its writer rather than from
   `mirror_base + offset`.

## Integration result: the planes fly, on the heading the mission authored

The rule is wired into `src/game_hosts_units.cpp`'s free-flight arm. Measured on `IJN01`, 500
mission ticks:

```
plane step:   steps=16500 free_flight=16500 ground_roll=0 surface=0 none=0
plane motion: distance_moved=116833.55 m
```

The planes hold the seeded airspeed for the whole mission, so lift cancels gravity in the running
process as the acceptance test pins it. The physics is confirmed end to end, from the listing
through the pure rule to a live run.

### The spawn orientation is authored, and an earlier revision threw it away

The first revision of this wiring seeded the airspeed along world `+Z` and left
`PlaneFreeFlightState::world_to_body` at the identity, on the stated ground that "a real orientation
needs the pose the model supplies, which this process does not build". **That was wrong, and it was
never checked.** The pose is already in the slot: unit creation copies the instance's world 4x4 -
the frame `0046cf40` composes from the authored placement - straight into
`motion.pose_row0/1/2`, and `src/system_camera_axes.cpp:387` reads `world[8..10]` of that same frame
as forward. `IJN01` authors two distinct orientations:

```
plane spawn: unit=A7M_1      heading=180.00 deg forward=(0.0000 -0.0003 -1.0000)
plane spawn: unit=JudySpawn1 heading=180.00 deg forward=(0.0000 -0.0003 -1.0000)
                             forward=(0.7324  0.0000  0.6808)   # heading ~47.1 deg
```

So the world-`+Z` seed was **180 degrees wrong** for the A7M and Judy groups. Both defects are
fixed together, because they are the same fact:

- The airspeed is seeded along `pose_row2`, the plane's own forward axis, normalised. A degenerate
  authored frame falls back to the old world-`+Z` seed rather than propagating a NaN through every
  lift term.
- `world_to_body` is the pose rows laid out row-major. The header records the convention as
  `body = M * world` (`007D9C39` writes ctl+3Ch from `0042D0D0(ctl+18h, ctl+0B0h)`), and row `i` of
  the pose is body axis `i` expressed in world, so `dot(row_i, v)` is exactly the body component.
  Nothing is invented; the matrix is the authored placement.
- `forward_speed` becomes `body_velocity[2]`, which is what the header already said it was
  (ctl+44h). Under an identity frame that is the world `+Z` the old code took, so the acceptance
  test is unmoved - and it still passes.

`distance_moved` moves 116875.00 -> 116833.55 m, down 0.035%. That is the expected consequence of a
pose that is not exactly level: at a forward `y` of -0.0003 the lift no longer cancels gravity to
the last bit, so the speed decays very slightly. The level-flight ideal of
`141.666672 m/s x 25 s x 33 planes = 116875 m` is the number the acceptance test pins, and the run
sits just under it.

### Correction: what the `nearest` column can and cannot show

The previous commit reported that the aircraft "now report `nearest` distances of 4582 to 5068 m
where the closest aircraft was previously 2950 m: the planes are flying **away** from the fleet."
**The conclusion was right but the evidence was misread.** `row.nearest_enemy`
(`src/game_hosts_gunnery.cpp:856`) is a running minimum that never resets, so it reports the closest
approach over the whole mission. Flipping the seed 180 degrees leaves it byte-identical at 4582 m,
which it could not do if it tracked live position - both runs are simply reporting the distance at
t=0.

Read correctly, that pinned minimum is the stronger statement: **the planes never get closer than
their spawn distance, on either heading.** The arithmetic agrees - 4582 m of separation against
3541 m of travel in 25 s, so even a plane aimed straight at the fleet would not reach the 800 m
`AAMACHINEGUN` range inside this window.

### Air combat is still unreachable, and this is not the reason

The per-category table is byte-identical to the previous run: `AAMACHINEGUN` 295 assignments / 26 /
0 shots, `FLAK` 60 / 0 / 0. Correcting the orientation did not make a single new gun fire, and it
was never going to: the planes fly the heading the mission authored, which does not close on the
fleet, and no recovered code steers them off it.

What is missing is the plane AI. `docs/PLANE_UNIT_TICK.md` records that nothing in the ledger names
one. Until it is recovered, aircraft in a reconstructed mission fly straight lines from their
authored placements, and **gameplay validation of air combat remains unsatisfied.**

**The integration step is the host's own, not recovered code.** `007DB680` is an accumulator pass:
it leaves four accumulators for `007D8470` to fold and the caller applies them. The
`v += a*dt; p += v*dt` in the binding is the host's, and is commented as such so it is not mistaken
for native behaviour.

### Follow-up packets

1. **`plane_ai`** - the steering. Nothing in the ledger names one; it is what stands between a
   reconstructed mission and any air combat at all.
2. **`plane_pose_integration`** - the pose is read at spawn but never updated afterwards, so a
   plane's orientation is frozen at its authored value for the whole mission. A turning plane needs
   the angular half of the flight law folded back into `pose_row0/1/2`.
