# The run-in descent: the one command that brings a torpedo bomber down

Addresses: 009D07B0, 009FBA50, 009FBAE9, 009FBAF1, 009FBB03, 009FBB06, 009FBB0C, 009FBB13,
009FB800, 009FB849, 009FB858, 009FB88D, 009FB96E, 009FB9B8, 009D1631, 009D20B4, 009D20C4,
009D48F6, 0099E490, 0099DD35, 00CE7D20, 00D05AAC, 00CE3D48, 00CE3938.

Packet `cc8_torpedo_run_in_descent`, owner `agent/cc8-torpedo-run-in`, on `5407abbbb`.

`docs/PLANE_POSE_THROTTLE_ALTITUDE.md` section 4 established that the aim tick's `plan+2BCh` is a
nose-up floor and that nothing in that tick descends an aircraft. This finds what does.

## 1. The chain, with addresses

**`009D07B0` step 4 is the only descent command in the torpedo chain.**
`docs/BOT_TASK_STATES.md` "The torpedo run" records it: *altitude `approach->+78h +
approach->+74h` through `009FBA50`*. The chain from there to the pitch axis:

| stage | address | what it does |
| --- | --- | --- |
| attackrun step 4 | `009D07B0` | calls `009FBA50` with the altitude pair as the base |
| cruise altitude | `009FBA50` | biases by `span · scale · class+518h` when `span = max(rangeHigh - rangeLow, 0)` is positive, clamps against `Dynamics/Ceiling - 50` and the squadron limit, then calls `009FB800` |
| pitch from altitude | `009FB800` | `cmd+2BCh` = the signed pitch demand, `cmd+2D0h` = 2 |
| the plan | `plan+2BCh` | `0099E490`'s pitch arm floors it; `0099DD35`'s slew applies in mode 2 |

**The argument order at the join, checked against the push order** rather than taken from the
decompiler, because a reversed pair here would be silent:

```
009fbae9  FLD [ESP+0xc]      ; the biased altitude
009fbaed  FLD [ESP+0x10]     ; the ceiling limit
009fbaf1  FCOMIP ST0,ST1     ; ceilingLimit vs altitude
009fbaf5  JA 009fbb03        ; ceilingLimit > altitude -> keep the altitude
009fbaf7  MOVSS [ESP+0xc] <- [ESP+0x10]     ; else clamp
009fbb03  SUB ESP,0x8
009fbb06  FSTP [ESP+4]       ; second argument: the x87 value that survives = the UNCLAMPED altitude
009fbb0c  FLD [ESP+0x14]     ; = old [ESP+0xc], the CLAMPED altitude
009fbb10  FSTP [ESP]         ; first argument
009fbb13  CALL 009fb800
```

So `009FB800(clamped, unclamped)`: the first argument is the altitude to fly, the second is the
same value **before** the ceiling clamp, and that second value is used twice - as the weight
`(reference + 1) · 0.5` at `009FB858` and as the upper clamp on `t`. With the torpedo run's base of
12 m the ceiling never binds, so both are 12.

`009FB800`'s two arms, and which one can act:

```
009fb849  err = min(desired, Ceiling - 50) - unit+100h
009fb858  x   = err * ((reference + 1.0) * 0.5)
009fb88d  x > 0, climb: limit = max(class+1ECh * 1.6, DEG(40) [00CE7D20]);
                        t = clamp(x / ClimbDist, 0, reference);
                        cmd+2BCh = min(class+1ECh * t, limit)
009fb96e  x <= 0, dive: limit = max(class+1F0h DropAngle * 1.6, DEG(60) [00D05AAC]);
                        t = clamp(-x / DropDist, 0, reference);
                        cmd+2BCh = -min(DropAngle * t, limit)
```

`class+1ECh` has **no producer in any shipped row** - `docs/PLANE_FLIGHT.md` scanned `.text` for
every store at that displacement and found none, and `ClimbAngle` appears zero times in the
installed `vehicleclasses.lua`. So `min(0 · t, DEG(40))` is zero and **the climb arm commands
nothing**. This routine can only ever push an aircraft down. That is the image's behaviour, not a
host gap, and section 4 shows what it costs.

## 2. What this host was not running

`009D48F6` is the state machine's "run the current state's own tick". This host ran it for
**`aim` only**. The `attackrun` state - where the descent lives - reached its tick never, so no
aircraft in this reconstruction was ever told to come down. The other half of the gap is that the
binding `command_altitude_and_throttle`, which `attack_run_tick` in `src/bot_task_states.cpp`
already calls, was an empty override.

Both are fixed. The `kAttackRun` branch now runs step 4's altitude half through a real
`command_altitude_and_throttle` that drives `cruise_altitude_command_009fba50` and
`pitch_command_009fb800`, and `desc+1F0h DropAngle` is loaded from the vehicle-class row.

**PARTIAL, and labelled as such.** Only step 4's altitude runs. Steps 1, 2 and 3 keep a countdown,
a period and a lateral offset at `state+18h`/`+1Ch`/`+20h` that this host does not carry; step 4's
throttle half and step 5's four command bits are not run. The heading those steps write is the same
raw bearing the yaw arm already uses here, so nothing is lost by their absence in this mission.

## 3. It works, and five torpedoes leave the aircraft

The census, `Mav3`, one line per fifty commands:

| n | commanded | live altitude | pitch demand | measured pitch |
| --- | --- | --- | --- | --- |
| 1 | 12.00 | 800.0 | **-1.0472** | 0.0000 |
| 51 | 12.00 | 686.6 | -1.0472 | -0.7440 |
| 101 | 12.00 | 194.0 | -1.0472 | -1.0695 |
| 151 | 12.00 | -320.2 | 0.0000 | -0.4426 |
| 201 | 12.00 | -400.5 | 0.0000 | -0.0007 |
| 251 | 12.00 | -400.6 | 0.0000 | -0.0002 |

The commanded altitude is **12.00 m**, which is `approach+78h + approach+74h` with `+74h` at zero
and `+78h` carrying the authored `TorpReleaseAlt` that `docs/TORPEDO_RELEASE_GEOMETRY.md` bound.
The demand is `-1.0472` rad, the `DEG(60)` floor, because `DropAngle · t` exceeds it by a factor of
four at that altitude error. The aircraft dives, and **five torpedoes are released** where the
before column released none.

## 4. And they go straight through the sea: the next gate

The release census:

```
gunnery: torpedo drop 1 by Mav3 at -230 m, speed 55.7 m/s, bullet 69, swim 30.9 m/s
release census: unit=Mav3 alt=-230.3 m |v|=55.68 m/s angle_to_nose=0.1 deg
                body_fwd_0092d730=55.68 m/s
```

**Everything about the release is now right except its altitude.** The velocity is 0.1 degrees off
the nose, the airspeed is 55.7 m/s against a `MaxWaterHitVel` of 100, and if the drop happened at
the commanded 12 m the entry speed would be `sqrt(55.7² + 2·9.81·12)` = **57.8 m/s**, comfortably
inside the limit. The torpedo would swim.

It does not, because the aircraft **overshoots to -400 m and holds there**. Two things combine and
both are image behaviour that this host does not yet carry:

1. **The flare is far too late to arrest a 60-degree dive.** The demand only falls below the
   `DEG(60)` cap once `DropAngle · t` does, which for `Mav`'s `DropAngle` of 0.4014 needs
   `t < 2.609`, i.e. the aircraft within 80 m of its target. At 55 m/s that is 1.5 seconds of flare
   against a sink rate near 48 m/s.
2. **Nothing catches the aircraft at the surface, and nothing can lift it back.** The climb arm of
   `009FB800` commands zero for every shipped class (section 1), so once the aircraft is below the
   commanded altitude the demand is `0.0000` and it simply levels off wherever it is. In the image
   an aircraft at the water is not on the free-flight arm at all: `007DB680` selects its arm on
   `ctl+FCh`, and `0` is free flight, `1` the ground arm `007DBEAA`-`007DC204` and `2` the water arm
   `007DC205`-`007DC68C`. This host's `free_flight_gate_00d06130_38` answers true for every plane
   forever, so neither contact arm exists here, and `docs/PLANE_FLIGHT_CORE_LAW.md` records the
   water arm as unread.

**The next gate, by address and value: `ctl+FCh` and the water arm `007DC205`-`007DC68C`.** Until a
plane at the surface leaves free flight, a commanded descent has no floor. No ad-hoc altitude clamp
was added here, because inventing one would be inventing behaviour the image does not have.

A second, smaller contributor worth naming: these aircraft enter `attackrun` at 800 m. In the image
they would already be near `Pilot/Torpedo/CruisingAlt`, authored 500, from the `moveto` state's own
`009C18C0` altitude command, so the descent they flare from is 500 m rather than 788. That halves
the overshoot without removing it.

## 5. The aim tick's own altitude handling, for completeness

`009D1631` builds the floor `max(over_land ? 30.0 : 5.0, approach+78h + approach+74h,
ground + 5.0)`, and `009D20B4` computes the release gate `Interp(0.4, 40.0, 1.0, 25.0,
time_to_target)`, between 25 and 40 m, which `009D20C4` passes only below. So the aim state expects
an aircraft that is already low: it never descends one, it only refuses to release a high one and
pulls up one that has sunk under its floor. The two states divide the work, and this host was
running only the second of them.

## ABI

* `009D07B0`, the torpedo attackrun tick, `void __thiscall(state, float dt)`; step 4 is the
  altitude and throttle command.
* `009FBA50`, `float __thiscall(this, float base, float rangeLow, float rangeHigh, float scale)`,
  `RET 10h`, body `009FBA50`-`009FBB1C`. The float in `ST0` at the `RET` is `scale`, never popped;
  every call site discards it.
* `009FB800`, `void __thiscall(this, float desiredAltitude, float reference)`, `RET 8`, body
  `009FB800`-`009FBA4F`. Writes `cmd+2BCh` and `cmd+2D0h = 2` on every path.

## Uncertainty

* `class+518h`, `009FBA50`'s per-class gain, and the squadron limit `squadron+394h` are unmodelled.
  Neither is reachable on this path: the attackrun tick passes `rangeLow = rangeHigh = 0`, so
  `span` is zero and the gain term is skipped, and no aircraft here has a squadron.
* `cmd+2D0h = 2` is written but not acted on. `0099DCE0`'s mode-2 arm holds `unit+C84h` and applies
  the `0099DD35` increment; this host models neither, so the demand reaches the planner as a plain
  target. Unchanged from `docs/PLANE_POSE_THROTTLE_ALTITUDE.md`.
* Steps 1, 2, 3 and 5 of `009D07B0` are not run. Labelled partial.
* `class+1ECh` having no producer is quoted from `docs/PLANE_FLIGHT.md`'s own `.text` scan; this
  packet did not repeat the scan.

## Host methods

| host method | file | native | kind |
| --- | --- | --- | --- |
| the `kAttackRun` branch of the state tick dispatch | `src/game_hosts_units.cpp` | `009D48F6`, `009D07B0` step 4 | binding, **partial** |
| `command_altitude_and_throttle` | `src/game_hosts_units.cpp` | `009FBA50` then `009FB800` | binding |
| `DropAngle` row key | `src/game_hosts_lua.cpp` | `desc+1F0h`, `007D2B93` | binding |
| descent census | `src/game_hosts_units.cpp` | observation | census |

## Corrections

Appended to the docs they amend, and verified present there.

* `docs/BOT_TASK_STATES.md`, "The torpedo run". Its step 4 is correct and is now exercised; what
  this packet adds is that step 4's altitude is the **only** descent command in the chain, and the
  `009FBA50` -> `009FB800` argument order at `009FBB03`-`009FBB13`.
* `docs/PLANE_FLIGHT.md`, "`009FBA50`, the cruising-altitude command" and "`009FB800`". Both rules
  are confirmed against the listing and are now bound and measured. Its observation that the climb
  arm commands nothing is confirmed at run time: once below the commanded altitude the demand is
  exactly `0.0000` and the aircraft never recovers.

## no_ghidra_function

None. `009D07B0`, `009FBA50` and `009FB800` all have Ghidra functions.

## Validation

`tools/run_game.ps1`, 3200 frames, `--mission-frames 3000` at `0.05` s, from this worktree at
`5407abbbb`, which is `4f86eaf4a` merged with main. The before column is the same source with the
`kAttackRun` branch switched off by a temporary compile-time constant, removed before the commit.

Those baselines are **not** comparable with `docs/PLANE_POSE_THROTTLE_ALTITUDE.md`'s: the merge
brought in other workers' close-attack, command-input and collision-pass work, which moved USN01
from 1 hit and 9.3 damage to 68 hits and 660.0 before anything in this packet ran.

### USN01

| | before | after |
| --- | --- | --- |
| **torpedo drops** | **0** | **5** |
| torpedo task releases | 0 | 5 |
| water-entry breakups | 0 | 0 |
| `swims_started` | 0 | 0 |
| torpedo hits, torpedo damage | 0, 0 | 0, 0 |
| commanded run-in altitude | none commanded | **12.00 m** |
| release altitude | no release | **-230.3 m** |
| release airspeed | no release | **55.68 m/s** |
| release angle, velocity to nose | no release | **0.1 deg** |
| `MaxWaterHitVel` | 100.0 | 100.0 |
| mission hits / kills / damage | 68 / 3 / 660.0 | 68 / 3 / 660.0 |
| pilot attack `range_last_mean` | 3011.3 m | 2804.2 m |
| pilot attack `closed_mean` | 1105.6 m | 1312.7 m |
| `distance_moved` | 195347 m | 202515 m |
| `pose_rotations` | 26586 | 26633 |

**The descent is the whole change and it produced the drops.** Nothing else in the mission moved:
the same 68 hits, the same three kills, the same 660.0 damage to the tenth. The ordered aircraft
close 207 m more on their targets because five of them now fly a real attack run.

The per-aircraft trace is section 3's table. Commanded 12.00 m throughout, a `-1.0472` rad demand
that is the `DEG(60)` floor, and an 800 m to -400 m dive that overshoots by 412 m and then holds.

**What the numbers say about the remaining gate.** The release itself is correct in every respect
but altitude: 55.68 m/s with the velocity 0.1 degrees off the nose. At the commanded 12 m the
entry speed would be `sqrt(55.68² + 2·9.81·12)` = **57.8 m/s** against a 100.0 limit, so the
torpedo would survive entry and swim. It is released at -230 m instead, below the surface, which is
why `water_entry_breakups` and `swims_started` are both zero: the round never crosses the water
downward.

### USN02

168 hits, 3 kills, 20721.4 damage, first hit 31.80 s, `swims_started = 44`, `distance_moved 0.00 m`
with `pose_rotations 0`.

**Identity is established directly rather than by a second run.** The mission's own summary line
reads *"no ordered aircraft carries torpedo ordnance (kind 2Bh), so 0099A170 builds no kind Eh
task"*, and the run's log contains **zero** `descent census` lines, so the `kAttackRun` branch this
packet adds never executed. Its numbers differ from
`docs/PLANE_POSE_THROTTLE_ALTITUDE.md`'s 161 hits and 19061.0 damage because of the merge of main,
not because of anything here.

## Follow-up packets

1. **`ctl+FCh` and the water arm `007DC205`-`007DC68C`**, with the ground arm
   `007DBEAA`-`007DC204` alongside it. This is the gate: a commanded descent has no floor until a
   plane at the surface leaves free flight. `docs/PLANE_FLIGHT_CORE_LAW.md` records both as unread.
2. **`009C18C0`, the moveto altitude command**, so aircraft reach `attackrun` at
   `Pilot/Torpedo/CruisingAlt` 500 rather than at their spawn altitude.
3. **The rest of `009D07B0`**: the countdown, period and lateral offset at `state+18h`/`+1Ch`/`+20h`
   and step 4's throttle half.
4. **`0099DCE0`'s mode-2 pitch arm** and `unit+C84h`, which is what `cmd+2D0h = 2` selects.


## Correction from packet `cc8_plane_altitude_hold_and_surface`: the climb arm is not silent

Appended, not rewriting sections 1 and 4.

Section 1 repeats `docs/PLANE_FLIGHT.md`'s "`class+1ECh` has **no producer in any shipped row**...
So `min(0 · t, DEG(40))` is zero and **the climb arm commands nothing**", and section 4 makes that
the first of its two reasons for the -400 m overshoot. **Both are refuted.** `desc+1ECh` is derived
at class load by `007C4C08`-`007C4C14` as `desc+1E4h · 0.6`, with `desc+1E4h` the steepest
sustainable climb angle that the `007D98F0` bisection solves for at `007C4BE9`.

Section 4's second reason stands and is now exact. The surface is not gated by `ctl+FCh`, which a
store census shows each of the three law entry points writing **on entry** as a tag; the gate is
`unit+900h` through `0074E210`, and the free-flight arm carries its own water line at
`007CC523`-`007CC562`, handing a contact to `007CB7F0`, whose tail at `007CB92C` calls
`BSP_Plane_SetFlightState(6)`. `docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md`.


## Correction from packet `cc8_torpedo_throttle_cut`: there is no throttle half to bind

Appended, not rewriting the sections above.

Section 2 labels the binding partial in part because "step 4's throttle half ... [is] not run", and
follow-up 3 lists it as work. **There is no throttle half.** `009D0A6B` stores the interpolation's
result as `009FBA50`'s fourth argument, `scale`, and both of the range arguments this tick passes
are `approach+90h` (`009D07E4`, `009D0A08`), so `span` is zero, `009FBAD2`'s term is skipped and the
value is discarded at the `RET`.

That also means the binding's `command_altitude_and_throttle(approach, base, 0.0f, 0.0f, 0.0f)` is
**exactly right**: passing a zero range pair gives the same commanded altitude as the native's own
arguments do. It was right for a reason it did not know.

Section 4's remaining question, why the aircraft dives at the `DEG(60)` cap the whole way down, is
answered in `docs/TORPEDO_THROTTLE_CUT.md` section 4: the commanded 12 m is the image's own number
for a torpedo bomber, because `approach+74h` comes from a field constructed zero at `00939E83` whose
only gameplay writer is the dive-bomb task's `009C89CE`. What is missing is the `moveto` tick
`009C18C0`, which would bring the aircraft to `Pilot/Torpedo/CruisingAlt` before the run begins.
