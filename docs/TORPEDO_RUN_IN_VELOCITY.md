# The run-in: why the velocity left the nose, and what commands the run

Addresses: 007DB680, 007DBD37, 007DBD3A, 007DBD41, 007DBD50, 007DBD7D, 007DBD8A, 007D92B0,
007C6340, 0092D730, 00C31F40, 00C32000, 0085E4D0, 009D15F0, 009D1BB8, 009D1BDB, 009D1C0B,
009D1C42, 009D1C51, 009D1C84, 009D1CAF, 009D1CCA, 009D1D02, 009D1D2E, 009D1EDD, 009D1EE5,
009D1F4A, 009D1F55, 009D20B4, 009D20C4, 009D2165, 009D0484, 009F9CE0, 007C1966, 007C1A94.

Packet `cc8_torpedo_run_in_velocity`, owner `agent/cc8-torpedo-run-in`, on main `ace4632df`.

The two gates `docs/TORPEDO_RELEASE_GEOMETRY.md` section 3 left open. Its measurement stands: at
the release instant `0092D730` read `-6.8 m/s` while the plane was moving at cruise, so the
velocity and the nose were roughly perpendicular.

## 1. The velocity left the nose because the only term that couples them was multiplied by zero

**The deciding instructions are `007DBD3A` and `007DBD50`.**

```
007db6ae  LEA EBX,[ESI + 0x3c]          ; EBX = ctl+3Ch, the BODY velocity x
...
007dbd37  MOV EAX,[ESI + 0xc]           ; EAX = the aircraft description
007dbd3a  FLD  float ptr [EAX + 0x174]  ; desc+174h XDrag
007dbd41  FLD  float ptr [EBX]          ; ctl+3Ch, body lateral velocity
007dbd43  FCHS
007dbd45  FMULP                         ; -vx * XDrag
007dbd4b  FLD  float ptr [ESI + 0x40]   ; ctl+40h, body vertical velocity
007dbd4e  FCHS
007dbd50  FMUL float ptr [EAX + 0x170]  ; desc+170h YDrag
007dbd61  CALL 0x007d92b0               ; r = AeroResponseCurve(forward/StallSpd)
007dbd7d  FSTP float ptr [EAX + 0x4]    ; dyn+04h += r * (-vx * XDrag)
007dbd8a  FSTP float ptr [EAX + 0x8]    ; dyn+08h += r * (-vy * YDrag)
```

`EBX` is resolved by filtering the whole listing of `007DB680` for the register: it is written
once, at `007DB6AE`, as `ESI+3Ch`, and pushed at `007DB6B1` as `007D7C00`'s body-velocity
argument. Nothing else writes it in the 768-instruction body.

This is the whole of the coupling. Nothing in the native rotates a stored velocity when the pose
turns; `0085E4D0` turns the **pose** and leaves the velocity where it is. What brings the velocity
back onto the nose is this block, which drives the body-frame lateral and vertical velocity
components toward zero at a rate of `XDrag` and `YDrag` per second, scaled by the response curve
`007D92B0`. The forward component `ctl+44h` is untouched by it. So a plane that yaws keeps its old
world velocity for a moment and then has the resulting sideslip damped out of it.

**This host declared both coefficients and never filled them.** `PlaneFreeFlightClass` in
`include/bsp/plane_flight.hpp` declares `x_drag{0.0f}` and `y_drag{0.0f}`; the free-flight arm in
`src/game_hosts_units.cpp` constructed the class with `bsp::PlaneFreeFlightClass cls;` and
assigned nothing, so `src/plane_flight.cpp:348-349` multiplied the correct rule by zero. The law
was right; both of its inputs were absent. That is why the pose could rotate 28129 times and the
velocity never followed.

It was invisible for the same reason the body/world frame error in
`docs/PLANE_FREE_FLIGHT_PHYSICS.md` was invisible: with no sideslip there is nothing to damp, and
nothing in this reconstruction made an aircraft sideslip until the pilot bot's yaw arm was wired.

This installation's `scripts/datatables/autoload/vehicleclasses.lua` authors `XDrag = 7` and
`YDrag = 7` on `VehicleClass[112]` TBD Devastator, so the damping is strong: at a 0.05 s step the
sideslip loses 35 percent of itself per step and the velocity is on the nose within a second.

### Fixed at the producer

`src/game_hosts_lua.cpp`'s vehicle-class row reader now reads `XDrag`, `YDrag`, `MaxSpd` and
`TravelSpeed` in the spellings `007D1F70`'s own reader uses (`src/plane_class_fields.cpp:279-292`),
the unit creation path copies them onto the slot, and the free-flight arm fills `cls.x_drag`,
`cls.y_drag` and `cls.stall_spd` from them. A ship row carries none of the four keys and reads
zero, which is the right answer for a ship and leaves every ship result untouched.

## 2. The run-in speed: the seed was one row's TravelSpeed standing in for every row's

**The deciding fact is the authored row, against `007C6340`'s seed field `desc+18Ch`.**

`docs/PLANE_FLIGHT_CORE_LAW.md` already named the producer: the spawn seeds the airspeed from
`desc+18Ch TravelSpeed` through `007C6340`, and it quoted `TravelSpeed = 141.666672 m/s` as the
class value it had in hand. This host then hard-coded that number for every aircraft.

In this installation's `vehicleclasses.lua`, `141.666672` is the `TravelSpeed` of four rows out of
seventy-four. The torpedo bombers author a different one:

| row | TravelSpeed | MaxSpd | StallSpd | XDrag | YDrag |
| --- | --- | --- | --- | --- | --- |
| `VehicleClass[112]` TBD Devastator | **61.111111** | 69.444443 | 19.444445 | 7 | 7 |
| `VehicleClass[113]` TBF Avenger | **61.111111** | 72.222221 | 18.611111 | 7 | 7 |

61.111111 m/s is 220 km/h; 141.666672 is 510 km/h. **The host was flying its torpedo run-in at
2.3 times the authored cruise**, which is on its own enough to put a dropped torpedo into the
water above `MaxWaterHitVel`.

The seed now takes the row's `TravelSpeed`, keeping `141.666672` as the fallback for a row that
carries none. The lift/gravity balance is unaffected: `007DB8E3` caps `qq` at 1.0 once the forward
speed reaches `LevelFlight * StallSpd`, which is 35.0 m/s for the Devastator, so 61.1 m/s is as
level as 141.7 m/s was. The acceptance test in `tests/math_tests.cpp` pins the law at fixed
values and is untouched.

`MaxSpd` is loaded for its own reason: `009F9CE0` (`bot_task_speed_ratio`) divides `desc+188h`
by `Pilot/Torpedo/ReferenceSpeed` and floors the result at 1.0, and that ratio is `approach+24h`,
the scale on the run profile's release distances at `009D0484`. `docs/TORPEDO_RUN_PROFILE.md`
recorded it as an unloadable blocker. The numerator is now real. The divisor is still a labelled
substitution, the authored `KMH(300) = 83.333336` recorded against PilotBot tuning offset `+440h`
in `include/bsp/bot_tasks.hpp:266`, because the registry is still unreachable from the units host.
For both torpedo bombers `MaxSpd` is below the reference, so the 1.0 floor wins and the ratio is
1.0 either way - the substitution that was there was numerically right, and is now right for a
reason.

## 3. The throttle command, read to its end

`009D1BB8`-`009D1D39`. The product is four factors and a ceiling, and every constant below is the
image's own bytes:

```
009d1bdb  FLD [EDI+0xc64]                        ; unit+C64h, the PITCH angle
009d1c0b  CALL 00419010  ; A = Interp(x0=-1.0,   y0=2.2,  x1=-0.2, y1=1.0, x=unit+C64h)
009d1c42  CALL 00419010  ; B = Interp(x0= 1.5,   y0=0.1,  x1= 2.0, y1=1.0, x=[ESP+14h])
009d1c51  FLD [EDX+0x25c] FMUL double 1.2        ; C = desc+25Ch * 1.2
009d1c84  FLD [EAX+0xa4]                         ; desc+A4h
009d1caf  FMUL ST1                               ; bytes D8 C9 -> ST0 = 3.0 * desc+A4h
009d1cca  CALL 00419010  ; D = Interp(x0=desc+A4h, y0=0.1, x1=3*desc+A4h, y1=1.6, x=unit+100h)
009d1cd7  FLD [0x00ce3814]                       ; 1.2, the ceiling
009d1d2e  MOVSS [ECX+0x2c8],XMM1                 ; cmd+2C8h = min(1.2, A*B*C*D)
```

| constant | address | value |
| --- | --- | --- |
| `A` x0 | `00D7A260` | -1.0 |
| `A` y0 | `00CE89D8` | 2.2 |
| `A` x1 | `00CE69CC` | -0.2 |
| `B` x0 | `00CE380C` | 1.5 |
| `B` y0 / `D` y0 | `00D7A2F0` | 0.1 |
| `B` x1 | `00CE3958` | 2.0 |
| `C` scale | `00CEC160` (double) | 1.2 |
| `D` x1 scale | `00D7A2B0` (double) | 3.0 |
| `D` y1 | `00D06BB4` | 1.6 |
| ceiling | `00CE3814` | 1.2 |
| `cmd+2E8h` | `00D06874` | 1.4 |

`009D1CAF` is `D8 C9`, `FMUL ST0,ST1`, not `DC C9`. That is what decides the `D` fold's shape: the
product `3.0 * desc+A4h` is stored and popped at `009D1CB1`, leaving the bare `desc+A4h` on the
stack for the `x0` slot at `009D1CC7`. With `DC C9` the two endpoints would be the other way round
and the fold would run backwards. Ghidra prints both encodings as `FMUL ST1`.

**What the native therefore commands.** `A` is the pitch fold and it is a *boost*: level flight
(`unit+C64h` near 0) is past `x1` and takes `y1 = 1.0`, while a nose-down 1.0 rad takes `y0 = 2.2`.
`D` is the altitude fold and it is the *cut*: at or below `desc+A4h` it is 0.1, at or above three
times that it is 1.6. `B` cuts the same way on the slot `[ESP+14h]` the tick's time-to-target
occupies. `C` is `desc+25Ch`, which `src/plane_class_fields.cpp:131` names `TurnRoll` from its
writer `007D289B`; the Devastator authors 1.047198, so `C` is 1.2566 and the ceiling at
`00CE3814` binds whenever the two folds are near 1.

So the native runs in at the ceiling until the aircraft comes down below three times `desc+A4h`,
then the throttle collapses toward a tenth. **The descent drives the slowdown, not the other way
round.**

## 4. Why this host neither slows nor descends

Three separate reasons, each an absent binding rather than a wrong rule.

1. **The throttle has no consumer.** `src/game_hosts_units.cpp` stores
   `r.commanded_throttle_2c8` into `unit_.torpedo_aim_throttle_last` and nothing reads that field.
   It is the only write and there is no other reference in `src/`.
2. **The throttle has nothing to act on even if it were published.**
   `PlaneFreeFlightState::thrust_accel` and `drag_accel` in `include/bsp/plane_flight.hpp` are the
   two host-supplied inputs for `007D9050 * unit+CC8h` and `007D9140`, and the free-flight arm
   leaves both at their `0.0f` declaration. There is no thrust term and no forward drag term in
   this host at all, so a plane holds whatever speed it was seeded with. That is also why the
   **seed** is the whole of the run-in speed.
3. **Nothing commands an altitude.** The aim tick's altitude output `cmd+2BCh` (`009D1EDD`) and
   its mode `cmd+2D0h` (`009D1EE5`) are dropped by the same binding that publishes the heading,
   and the host's `command_altitude_and_throttle` binding is an empty override. The plane holds
   its spawn altitude for the whole mission, which in USN01 is 700 m.

`cmd+278h`/`+27Ch` are **not** the altitude pair. `009D1F4A` writes the constant `1.0` from
`00D7A24C` and `009D1F55` writes the byte `1`; they are two of the five fixed command bits, and
`docs/TORPEDO_APPROACH_UPDATE.md` line 195 already records them that way.

### The release at 700 m did not come from the aim tick

`009D20B4` computes `Interp(0.4, 40.0, 1.0, 25.0, time_to_target)`, so the gate value lies in
`[25, 40]` metres, and `009D20C4` passes only when that exceeds the aircraft's altitude. At 700 m
the altitude gate is false and the aim tick's five-flag chain cannot arm the release timer at
`009D2287`. The drop this host observes therefore comes from the other release path -
`request_ordnance_release` from `009D4956`, `009D26F8`, `009D2938` or `009D29CB`, the task arm's
own release - and not from the aim tick. `docs/TORPEDO_FIRST_RELEASE.md` and
`docs/TORPEDO_GOAWAY_RELEASE.md` own those sites.

## ABI

* `007DB680` `BSP_PlaneFlight_CoreLaw`, `__thiscall(controller, float step, float, float, int)`,
  Ghidra body `007DB680`-`007DC82A`. The damping block is `007DBD37`-`007DBE0D`.
* `007D92B0` `BSP_PlaneFlight_AeroResponseCurve`, one float on the stack, `RET 4`, body
  `007D92B0`-`007D9358`.
* `009D15F0` `BSP_BotStateTorpedoAim_Tick`, `void __thiscall(BotStateTorpedoAim*, float dt)`,
  `RET 4`, Ghidra body `009D15F0`-`009D2379`. The throttle block is `009D1BB8`-`009D1D39`.
* `007C6340` `BSP_Plane_ChooseSpawnFlightState`, `__fastcall(unit)`, body `007C6340`-`007C64F5`.
* `0092D730` `BSP_UnitController_GetBodyAxisSpeed`, `float __thiscall(controller)`, `RET 0`, body
  `0092D730`-`0092D76E`.

## Uncertainty

* `desc+A4h`, the altitude fold's knee, is not in `src/plane_class_fields.cpp`'s key table. Its
  value decides where the native's throttle cut begins and it is **unread**.
* `[ESP+14h]`, the `B` fold's input, is the time-to-target slot by position in the frame; it was
  not traced back through every `PUSH`/`POP` in this packet. Labelled partial.
* `desc+25Ch` is `TurnRoll` by `src/plane_class_fields.cpp:131` and its writer `007D289B`. The
  aim-tick host contract calls the same slot `cruise_speed_25c`. One of the two names is wrong and
  this packet did not settle which; the arithmetic above uses the authored `TurnRoll` value.
* The `SPNormal` row choice for `TorpReleaseAlt`/`TorpReleaseDist*` is still a substitution: the
  difficulty index at `[[unit+DF4h]+34h]` is unmodelled. Unchanged from
  `docs/TORPEDO_RELEASE_GEOMETRY.md`.

## Host methods

| host method | file | native | kind |
| --- | --- | --- | --- |
| `XDrag`/`YDrag`/`MaxSpd`/`TravelSpeed` row keys | `src/game_hosts_lua.cpp` | `007D2189`, `007D2150`, `007D238A`, `007D23C3` | binding |
| `cls.x_drag`, `cls.y_drag`, `cls.stall_spd` in the free-flight arm | `src/game_hosts_units.cpp` | `007DBD3A`, `007DBD50`, `007DB760` | binding |
| spawn airspeed from the row's `TravelSpeed` | `src/game_hosts_units.cpp` | `007C6340`, `desc+18Ch` | binding |
| `approach+24h` from `MaxSpd / ReferenceSpeed` | `src/game_hosts_units.cpp` | `009F9CE0`, `009D0484` | binding, divisor **substituted** |
| velocity-versus-nose census, per tick and at the release | `src/game_hosts_units.cpp` | observation of `0092D730` | census |

## Corrections

Appended, not rewriting the other docs' text.

* `docs/TORPEDO_RELEASE_GEOMETRY.md`, "The next gate is that the aircraft's velocity and its nose
  have come apart", and its follow-up 1. Answered: the first of its two readings is the right one.
  The magnitude was at cruise and the fault was the free-flight arm's coupling, specifically that
  `PlaneFreeFlightClass::x_drag` and `y_drag` were never filled. Its follow-up 2, the run-in speed,
  is answered by the `TravelSpeed` seed rather than by `MaxSpd`: `MaxSpd` scales release
  *distances*, not the airspeed. Its follow-up 3, what holds the run-in at 700 m, is answered:
  nothing commands an altitude at all, and the aim tick's own altitude output is dropped.
* `docs/PLANE_FREE_FLIGHT_PHYSICS.md` section 2.5 and section 7. The rule was transcribed
  correctly and was never exercised: both class coefficients were zero for the whole life of this
  reconstruction. Section 2.5 should be read as reconstructed-but-unexercised before this packet.
* `docs/TORPEDO_AIM_TICK.md` section 4, the `009D1BB8`-`009D1D39` row. The four folds are now
  transcribed with their endpoints and the `D8 C9` at `009D1CAF` that fixes the altitude fold's
  direction. The row's description stands.
* `src/torpedo_approach_update.cpp`'s `torpedo_aim_tick_009d15f0` is an earlier, cruder
  reconstruction of the same tick and it has **no caller** in this host. Two of its writes are
  refuted by the listing: `cmd+2BCh` is not the altitude floor (`009D1EDD` stores the clamped
  quotient the `009D1E9A` `FDIV` produces, bounded by `00D21318` = 0.05625 and `00D057E0` =
  0.872665), and `cmd+2E8h` is not `0.0` (`009D1CFA` loads `00D06874` = **1.4**). Left in place
  because it is unreachable; the full reconstruction in `src/torpedo_aim_tick.cpp` is correct on
  both.
* **A mis-bound pair, not fixed here.** `unit+C64h` is the **pitch** angle, written at `007C1966`
  `FSTP [ESI+0C64h]`, and `unit+C68h` is the **bank** angle, written at `007C1A94`
  (`docs/PLANE_ATTITUDE_ANGLES.md` sections 1 and 3). `include/bsp/torpedo_aim_tick.hpp:226-227`
  names them `unit_bank_c64` and `unit_bank_rate_c68`, and `src/game_hosts_units.cpp:3833-3834`
  feeds `unit_.plane_latched_controls[0]` - the latched **yaw control axis**, a number in
  `[-1, 1]` - where the native reads a pitch angle in radians, and a literal `0.0f` where it reads
  the bank angle. Three consumers are affected: the throttle's `A` fold at `009D1BDB`, the pitch
  command's `denom` branch at `009D1DE6`, and the release gate at `009D2165`. The one-line fix is
  `plane_pitch_angle_c64` and `plane_bank_angle_c68`. It is left to the aim-tick owner rather than
  applied unmeasured here, because it moves the release gates.

## no_ghidra_function

None. Every routine this packet read has a Ghidra function: `007DB680`, `007D92B0`, `007C6340`,
`009D15F0`, `0092D730`, `009F9CE0`.

## Validation

`tools/run_game.ps1`, 3200 frames, `--mission-frames 3000` at `0.05` s, both columns from this
worktree. The before column is this branch with the three bindings switched off in a temporary
compile-time constant, so both columns come from the same source tree and the same census.

### USN01

| | before | after |
| --- | --- | --- |
| `ConTBD1` at the release instant, `\|v\|` | **688.59 m/s** | no release |
| `ConTBD1` at the release instant, angle to `pose_row2` | **90.6 deg** | no release |
| `ConTBD1` at the release instant, `0092D730` | -6.76 m/s | no release |
| `ConTBD1` at the release instant, `motion.position[1]` | **-14556.0 m** | no release |
| `KatTBD` at aim tick 51, `\|v\|` / angle / altitude | 504.37 m/s / 82.5 deg / -7484.2 m | - |
| every torpedo bomber at aim tick 1, angle to nose | 84 to 91 deg | **0.2 deg** |
| every torpedo bomber at aim tick 351, angle to nose | - | **0.5 to 0.6 deg** |
| worst airspeed seen in the census | **688.59 m/s** | **64.50 m/s** |
| altitude at aim tick 351 | - | 873 to 877 m |
| torpedo drops | 1 | 0 |
| water-entry breakups | 1 | 0 |
| `swims_started` | 0 | 0 |
| torpedo hits, torpedo damage | 0, 0 | 0, 0 |
| mission hits / kills / damage | 25 / 1 / 229.5 | 23 / 1 / 220.0 |
| `distance_moved` | 2132014 m | 2180805 m |
| `pose_rotations` | 28129 | 11907 |

**The velocity is on the nose.** 84 to 91 degrees before, 0.2 degrees after, and the body-axis
speed `0092D730` now equals the magnitude instead of being a fraction of it with the wrong sign.
That is the gate this packet was given, and it is closed.

**The second thing the census shows was not asked for and matters more.** Before the fix
`ConTBD1` was moving at **688 m/s** and its own `motion.position[1]` was **-14556 m** - fourteen
kilometres below the sea. `KatTBD` was at 504 m/s and -7484 m at four seconds in. These are the
aircraft `docs/PLANE_FREE_FLIGHT_PHYSICS.md` predicted would blow up: "nothing in this
reconstruction rolls a plane level, so an aircraft that banks stays banked indefinitely". With
`XDrag`/`YDrag` at zero there was nothing to terminate the spiral. With them wired at the authored
values no aircraft in the mission exceeds 64.50 m/s and every one holds 800 to 880 m. The doc's own
open question - whether the reconstructed drag is too weak to terminate the spiral - is answered:
the drag was not too weak, it was switched off.

**The drop that disappeared was an artefact of that spiral.** The single before-run drop came from
`ConTBD1` while the aircraft was at -14556 m at 688 m/s. It is not a capability that was lost.
Torpedo hits and torpedo damage were zero in both columns.

**The 9.5 points of mission damage** (229.5 to 220.0, 25 hits to 23, one kill in both) is the
aircraft path moving. Every plane in USN01 now flies a different, slower and far more plausible
trajectory, so the strafing gunnery that produced those hits has different geometry. No torpedo
damage exists in either column, so none of the difference is torpedoes.

### USN02

**Bit-identical, before and after**: 161 hits, 3 kills, 19061.0 damage, first hit at 31.80 s,
`swims_started = 44`, `torpedo_drop drops = 0`, `snaps = 54920`. USN02's own summary line reads
`distance_moved=0.00 m pose_rotations=0 thinks=0`: the mission carries no aircraft at all, so
nothing this packet changed can reach it. The 44 ship-tube swims are unchanged, which is the
no-regression evidence the packet needed.

## The release altitude was never a command, and it was never 700 m either

`docs/TORPEDO_RELEASE_GEOMETRY.md` recorded the release as happening at 700 m and asked what holds
the run-in there. The census answers it in a way the packet did not expect. In the **same instant**
of the same before-run log:

```
gunnery: torpedo drop 1 by ConTBD1 at 700 m, speed -6.8 m/s, bullet 63, swim 30.9 m/s
release census: unit=ConTBD1 alt=-14556.0 m |v|=688.59 m/s angle_to_nose=90.6 deg ...
```

The gunnery host reads `unit_pose`, which reads the **published** pose `slot.world`; the census
reads `slot.motion.position`. `publish_pose` has two call sites in `src/game_hosts_units.cpp`, unit
creation and the ship body path. **The plane path never republishes.** So every consumer of
`unit_pose` - the drop origin among them - has seen each aircraft frozen at its spawn placement for
the whole life of this reconstruction, and the "700 m release altitude" was that spawn altitude,
not a flying aircraft's.

Nothing commands a run-in altitude in this host (section 4), and the altitude the drop was spawned
at was not the aircraft's. Both halves of the release-altitude question are therefore host gaps,
and the native's authored 5 to 12 m from `TorpReleaseAlt` has never been reachable. This is the
next gate and it is named precisely: a `publish_pose` call at the end of the free-flight arm, and
the measurement of what it moves across the gunnery and recon paths, which is a packet of its own.

## Follow-up packets

0. **The plane pose is never published.** `publish_pose` at the end of the free-flight arm, and a
   measured pass over what it moves in the gunnery and recon hosts. Until it exists no air-dropped
   round leaves from where the aircraft is, and no release altitude means anything. This is the
   gate.
1. **The `unit+C64h` / `unit+C68h` mis-binding** in the aim tick's host contract, with the release
   gates re-measured. One line, three consumers, and it is in the pilot area the brief warns about.
2. **`thrust_accel` and `drag_accel`.** `007D9050`, `unit+CC8h` and `007D9140` are the missing
   forward terms. Until they exist the commanded throttle cannot change a plane's speed, and the
   airspeed is whatever the spawn seeded.
3. **The commanded altitude.** Publish `cmd+2BCh`/`cmd+2D0h` from the aim tick and give
   `command_altitude_and_throttle` a body, then find where the plane control law consumes an
   altitude command (`docs/PLANE_FLIGHT.md`, "`009FBA50`, the cruising-altitude command").
4. **`desc+A4h`**, the altitude fold's knee, which decides where the native cuts the throttle.


## Correction from packet `cc8_plane_pose_throttle_altitude`: section 3's "throttle" is a bank cap

Appended, not rewriting section 3.

Section 3 reads `009D1BB8`-`009D1D39` to its end and every constant in its table is right, but it
inherits the wrong name for what the product is. `plan+2C8h` is the per-task **bank-angle cap**:
`0099E27B` clamps the bank target `plan+2C4h` into `+-plan+2C8h`, `0099B55E` resets it to `20.0f`
so the clamp is inert until a task opts in, and the product's base factor `desc+25Ch` is `TurnRoll`,
an authored maximum bank angle in radians. The ceiling `00CE3814` = 1.2 is 1.2 radians.
`docs/PILOT_PLANNER_PITCH_ROLL.md` section (2) and note 6 carry the evidence; the same correction
is appended to `docs/TORPEDO_AIM_TICK.md`.

Section 4's first two reasons therefore describe a bank cap that was dropped, not a throttle. Its
third reason stands unchanged, and its second is now closed: `thrust_accel` and `drag_accel` are
bound in `docs/PLANE_POSE_THROTTLE_ALTITUDE.md`, from `desc+164h Accel` and the derived coefficient
`desc+50Ch = Accel / MaxSpd^2` that `007C4990`-`007C499C` computes.

Follow-up 1 of this doc (the `unit+C64h` / `unit+C68h` mis-binding) and follow-up 0 (the plane pose
is never published) are both applied and measured in that packet. Follow-up 2 (thrust and drag) is
applied. Follow-up 3, the commanded altitude, is answered rather than applied: there is no
commanded altitude in this chain, only the nose-up floor above.
