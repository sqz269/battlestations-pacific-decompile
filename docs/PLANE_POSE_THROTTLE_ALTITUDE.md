# The three consumers: the published pose, the engine, and the bank cap

Addresses: 007C6500, 007D8230, 007D8252, 007D8293, 007D831C, 004134F0, 0042ED50, 007D9F60,
007C4850, 007C4978, 007C4990, 007C4992, 007C499C, 007D9050, 007DB7A6, 007DB76C, 007D9140,
007DBB5E, 007DBB0E, 007DBB23, 009D1D2E, 009D1EDD, 009D1EE5, 0099E27B, 0099B55E, 0099E3D1,
007C1966, 007C1A94, 009D1BDB, 009D1DE6, 009D2165.

Packet `cc8_plane_pose_throttle_altitude`, owner `agent/cc8-torpedo-run-in`, on
`4edec0448`.

The three things `docs/TORPEDO_RUN_IN_VELOCITY.md` named as computed-and-dropped. All three are
now bound, and one of them turned out to be a different quantity than every doc in this
reconstruction called it.

## 1. The published pose: `007C6500`, and the element this host does not have

**The deciding instructions are `007D8293` and `007D831C`.**

```
007c658c  if (unit+210h == 0) FUN_007d9f60(t) else FUN_007d8230(t)   ; 007C6500's tail
...
007d8252  LEA  ESI,[EAX + 0x674]        ; EAX = ctl+8h, the unit: the COMMITTED pose
007d8258  ...                           ; 16 dwords copied to a local
007d826b  CALL 0x004134f0               ; BSP_Matrix_Copy4x4X87 into the local
007d8278  ...                           ; translation += t * (unit+810h..818h + ctl+18h..20h)
007d828e  MOV  EBX,dword ptr [EBX + 0x8]
007d8293  LEA  ECX,[EBX + 0x74]         ; the PUBLISHED pose
007d831c  CALL 0x004134f0               ; write it
007d8327  RET  0x4
```

`unit+674h` is the committed pose and `unit+74h` is the published one. `007C6500
BSP_PlaneTickElement_AdvancePose` is tick-element slot `+4h` of all nine plane vtables, and it is a
**different element** from the fixed step `007CE040` that this host's free-flight arm stands in
for. It runs the copy every frame, then walks the node list calling `0042ED50
BSP_SceneNode_InvalidateSubtreePose` and two vtable slots. `docs/PLANE_POSE_COMMIT.md`'s "Where the
pose does advance" section narrowed the open item to exactly this routine; this closes it for the
publication half.

**This host has no `+4h` element.** `GameFixedStepHost::next_element` returns false because the
five `68h` groups at `00F876C0` are empty (`docs/PLANE_UNIT_TICK.md`), and `publish_pose` in
`src/game_hosts_units.cpp` had exactly two call sites: unit creation and the ship body path. So
**every consumer of `unit_pose` saw each aircraft frozen at its spawn placement** for the whole
life of this reconstruction - gunnery ranges and bearings, recon, the minimap, and the air-dropped
torpedo's own spawn origin.

That is what produced the release altitude every torpedo packet since
`docs/TORPEDO_RELEASE_SPAWN.md` has quoted. In one instant of one log:

```
gunnery: torpedo drop 1 by ConTBD1 at 700 m, speed -6.8 m/s, bullet 63, swim 30.9 m/s
release census: unit=ConTBD1 alt=-14556.0 m |v|=688.59 m/s angle_to_nose=90.6 deg ...
```

The gunnery host read the published pose; the census read `motion.position`. 700 m was the spawn
altitude.

The fix is one call at the end of the free-flight arm. The native interpolates by a fraction of a
frame's motion and this host has no sub-frame time, so the copy is taken at the committed pose,
which is `007D8230` with its `t` at zero.

## 2. The engine: thrust, drag, and the coefficient the loader derives

**The deciding instructions are `007C4990`-`007C499C`.**

```
007c4978  FLD  float ptr [ESI + 0x188]   ; MaxSpd
007c497f  FSTP float ptr [ESP + 0xc]
007c4983  PUSH EDI                       ; the slot becomes [ESP+0x10]
007c4984  FLD  float ptr [ESI + 0x164]   ; Accel
007c498a  FLD  float ptr [ESP + 0x10]    ; MaxSpd
007c498e  FLD  ST0
007c4990  FDIVP ST2,ST0                  ; Accel / MaxSpd
007c4992  FDIVP                          ; / MaxSpd again
007c499c  FST  float ptr [ESI + 0x50c]
```

**`desc+50Ch = Accel / MaxSpd²`.** It is derived at class load, not authored - which is why it has
no key in `src/plane_class_fields.cpp`'s table and why a `store_census` over offset `50Ch` finds
exactly one writer in the plane range. Its consequence is the whole point: the drag magnitude is
`v² · desc+50Ch · …`, the thrust is `Accel · throttle`, and at full throttle the two balance at
`v = MaxSpd` exactly. **The authored `MaxSpd` is the equilibrium airspeed, by construction.**

`007D9050 BSP_PlaneFlight_ThrustAccel`, gated at `007DB76C` on the latched throttle `unit+0BBCh`
exceeding `0.01f`:

```
a = desc+164h Accel * throttle
if (ctl+4h)  a *= tuning+330h TurboMultiplier (1.95)        ; not modelled
if (ctl+5h)  a *= desc+604h                                  ; not modelled
if (pitch < 0) a *= 1 + (AccelCheatFallMul - 1)
                      * sin(Interp(FallPitchRange/1, 0, FallPitchRange/2, pi/2, -pitch))
return clamp(a, 0, 100)
```

`007D9140 BSP_PlaneFlight_DragAccel`, five stack arguments, the call site `007DBB5E` supplying the
world speed, the live throttle `unit+9E4h+0Ch`, the pitch `unit+0C64h`, the latched air brake
`unit+0BB0h+10h` and the latched elevator `unit+0BB0h+4h`:

```
closed = clamp(1 - throttle, 0, 1)
floor  = Interp(0, MaxSpd * MinDragSpdMul, MaxDragPitch, MaxSpd * MaxDragSpdMul, pitch)
v      = max(speed, floor)
return -sgn(v) * v^2
        * (closed^2 * desc+208h GlideRate + 1 + desc+1D4h DragPitchRatio * |elevator|)
        * desc+50Ch
        * (desc+1DCh AirBrakeDrag * airbrake + 1)
```

The call site then multiplies by the pitch ramp `r2` (`007DBB23`) over `r1` (`007DBB0E`); both of
`r2`'s endpoints are `1.0` for an aircraft whose DeadMeat timer has not started, so it is inert
until one is shot up.

Two prior packets recorded `007D9140` as read-only-negative ("the body was not read"). It is read
here. Authored values for this installation's TBD Devastator: `Accel 6`, `GlideRate 1`,
`DragPitchRatio 0`, `AirBrakeDrag 0.3`, and `PlaneGlobals` gives `MaxDragSpdMul 0.1`,
`MinDragSpdMul 0.1`, `MaxDragPitch 1.0` - both speed-floor endpoints equal, so the floor is a flat
`0.1 · MaxSpd`.

**Labelled partial**: the turbo byte `ctl+4h`, the boost byte `ctl+5h` with `desc+604h`, the
call-site scales `unit+0CC8h` and `008E6430(6, unit)`, and `007D20C6`'s in-place scaling of `Accel`
by `tuning+31Ch · tuning+320h` are all unmodelled and taken as 1.0. The last of these cannot move
the equilibrium, because thrust and `desc+50Ch` both read the same `desc+164h`.

## 3. `plan+2C8h` is the bank cap, and every doc here called it a throttle

**The deciding instruction is `0099E27B`.**

`docs/PILOT_PLANNER_PITCH_ROLL.md` section (2) transcribes it:
`if (plan+2C8h < pi) plan+2C4h = ClampInPlace(plan+2C4h, -plan+2C8h, +plan+2C8h)`, where
`plan+2C4h` is the bank target. Its note 6 gives the reset, `0099B55E`'s `20.0f` - above `pi` on
purpose, so the clamp is inert until a task opts in - and lists "the seven task-side `plan+2C8h`
writers" as open. `009D1D2E` is one of them.

Three things agree and nothing disagrees:

* the product's base factor is `desc+25Ch`, which `src/plane_class_fields.cpp:131` names `TurnRoll`
  from its writer `007D289B` and that doc's note 7 reads as a maximum bank angle - 1.047198 rad on
  the TBD Devastator;
* the ceiling `[00CE3814]` is `1.2`, and 1.2 **radians** is 68.8 degrees of bank, while 1.2 of full
  throttle would be meaningless;
* the consumer already exists in this host at `src/plane_ai_control.cpp:517-519`, reading
  `PilotBotRollInputs::bank_limit_2c8`, and nothing wrote it.

So the four folds schedule how hard the aircraft may bank on its run-in: the `unit+C64h` fold opens
the cap when the nose is down, and the time and altitude folds close it to a tenth as the aircraft
gets low and close in. Corrections are appended to `docs/TORPEDO_AIM_TICK.md` and
`docs/TORPEDO_RUN_IN_VELOCITY.md`.

## 4. `plan+2BCh` is a nose-up floor, not a descent

`009D1EDD` writes `plan+2BCh`, the pitch target, and `009D1EE5` writes `plan+2D0h = 1`, the mode
`0099E3D1` gates the entire pitch law on. The value is `clamp(-f34 / den, 0.05625, 0.872665)`,
where `f34` is the aircraft's height **above** the altitude floor. A high aircraft makes the
quotient negative and the clamp floors it at `0.05625` rad - 3.2 degrees nose up. Only an aircraft
**below** the floor produces anything larger, and then it is a pull-up of up to 50 degrees.

`009D1EAE`'s comparison settles the direction: `009D1EA6 FLD [00D21318]` loads `0.05625` and
`009D1EAC FCOMIP ST0,ST1` against the quotient, with `JBE` taking the upper-clamp path, so the
low branch writes `0.05625`.

**Nothing in the aim tick brings a torpedo bomber down.** Whatever descends a torpedo run is
upstream of the aim state, and this packet did not find it. That is the honest answer to "find the
altitude command": there is not one here.

Measured consequence, and it is the reason section 2 is in this packet rather than a later one: the
pitch command alone **stalls the aircraft**. Bound without an engine, the 3.2-degree nose-up floor
made `Mav4` climb from 806 m to 895 m while its airspeed bled from 63.20 to 34.46 m/s. `1.8 ·
StallSpd` is `1.8 · 19.4 = 35.0`, so lift collapsed between the tick-251 sample (41.73 m/s) and the
tick-301 sample (34.46), the aircraft banked to -0.78 rad and fell, and the velocity ended 64.5
degrees off the nose. With thrust and drag bound the same aircraft holds 60 to 69 m/s for the whole
run - `MaxSpd` is 69.44 - and the angle stays between 0.1 and 1.1 degrees.

## 5. `unit+C64h` and `unit+C68h`, applied

`unit+C64h` is the **pitch** angle, written at `007C1966 FSTP [ESI+0C64h]`, and `unit+C68h` is the
**bank** angle, written at `007C1A94` (`docs/PLANE_ATTITUDE_ANGLES.md` sections 1 and 3). The aim
tick contract names both for bank, and the host binding fed `plane_latched_controls[0]` - the
latched **yaw control axis**, a number in `[-1, 1]` - where the native reads an attitude angle in
radians, and a literal `0.0f` where it reads the bank. Three consumers depend on it: the bank cap's
pitch fold at `009D1BDB`, the pull-up denominator at `009D1DE6` and the release gate at `009D2165`.
Applied at the producer. The contract's field names are left to the aim tick's owner.

## ABI

* `007C6500` `BSP_PlaneTickElement_AdvancePose`, `void __thiscall(elem, float t)`, `RET 4`, Ghidra
  body `007C6500`-`007C675D`.
* `007D8230`, `void __thiscall(ctl, float t)`, `RET 4`, body `007D8230`-`007D8329`.
* `004134F0` `BSP_Matrix_Copy4x4X87`, body `004134F0`-`00413556`.
* `007D9050` `BSP_PlaneFlight_ThrustAccel`, `__fastcall(desc, int turbo, float pitch, float
  throttle, int boost)`, `RET 0Ch`, body `007D9050`-`007D9137`.
* `007D9140` `BSP_PlaneFlight_DragAccel`, `__fastcall(desc, float speed, float throttle, float
  pitch, float airbrake, float elevator)`, `RET 14h`, body `007D9140`-`007D92A4`.
* `007C4850`, body `007C4850`-`007C4D86`; the `desc+50Ch` derivation is `007C4978`-`007C499C`.

## Uncertainty

* `007D9F60`, the other arm of `007C6500`'s tail (`unit+210h != 0`), is **unread**. 272
  instructions. This host always takes the `007D8230` shape.
* The five unmodelled thrust and drag scales listed in section 2.
* `007D9140`'s argument order past the first is still the call site's, as two prior packets
  recorded; this packet read the body but did not re-derive the order from `007DBB5E`'s pushes.
* `desc+A4h`, the bank cap's altitude knee, is still unread and still not in the class field table.
* The pitch mode `plan+2D0h` gate at `0099E3D1` is not modelled in this host; the pitch law always
  runs. Publishing the aim tick's mode would need that gate first.

## Host methods

| host method | file | native | kind |
| --- | --- | --- | --- |
| `publish_pose` at the end of the free-flight arm | `src/game_hosts_units.cpp` | `007C6500` / `007D8230` | binding, interpolation dropped |
| `state.thrust_accel` | `src/game_hosts_units.cpp` | `007D9050` | binding, five scales **substituted** as 1.0 |
| `state.drag_accel` | `src/game_hosts_units.cpp` | `007D9140` | binding |
| `Accel`, `GlideRate`, `DragPitchRatio`, `AirBrakeDrag` row keys | `src/game_hosts_lua.cpp` | `007D20C6`, `007D2B10`, `007D2829`, `007D226D` | binding |
| `desc+50Ch` as `Accel / MaxSpd^2` | `src/game_hosts_units.cpp` | `007C4990`-`007C499C` | binding |
| latched throttle and air brake | `src/game_hosts_units.cpp` | `unit+0BBCh`, `unit+0BB0h+10h` | binding |
| `plan_state.bank_limit_2c8` from the aim tick | `src/game_hosts_units.cpp` | `009D1D2E` | binding |
| `plan_state.pitch_target_2bc` from the aim tick | `src/game_hosts_units.cpp` | `009D1EDD` | binding, mode `2D0h` **not** modelled |
| `unit+C64h` / `unit+C68h` fed the pitch and bank angles | `src/game_hosts_units.cpp` | `007C1966`, `007C1A94` | correction |

## Corrections

Appended to the docs they amend, and verified present there.

* `docs/TORPEDO_AIM_TICK.md` - `009D1BB8`-`009D1D39` is the bank cap, not the throttle; `cmd+2E8h`
  is 1.4; `cmd+2BCh` is a nose-up floor.
* `docs/TORPEDO_RUN_IN_VELOCITY.md` - the same, against its section 3, plus the status of its four
  follow-ups.
* `docs/PLANE_POSE_COMMIT.md`'s "Where the pose does advance" named `007C6500` as a candidate and
  said its body was unread. It is read here, and the publication half is established:
  `007D8293`/`007D831C`. Its integration half, what writes `unit+674h`, is untouched.
* `docs/PLANE_FREE_FLIGHT_PHYSICS.md`'s "It is not gone for uncommanded aircraft" - the banked
  spiral for aircraft the planner never runs. It **is** gone, and it was not a scope limit: the
  missing forward drag was. See the Validation table's `distance_moved` row.

## no_ghidra_function

None. `007C6500`, `007D8230`, `007D9F60`, `007C4850`, `007D9050`, `007D9140` and `004134F0` all
have Ghidra functions; the "no Ghidra function" note in `007C6500`'s own ledger record is stale and
is left for its owner.

## Validation

`tools/run_game.ps1`, 3200 frames, `--mission-frames 3000` at `0.05` s, from this worktree. The
before column is this branch at `4edec0448` with the four bindings switched off in a temporary
compile-time constant, removed before the commit; it reproduces that commit's numbers exactly
(23 hits, 220.0 damage), which is the check that the switch is faithful.

The middle column is the same build **without** section 2, and it is kept because it is the
measurement that proves section 2 is a prerequisite rather than an extra.

### USN01, `Mav4` through the run-in

| aim tick | before, `\|v\|` / alt / angle | pose+pitch only | with thrust and drag |
| --- | --- | --- | --- |
| 1 | 63.20 / 806.3 / 0.2 | 63.20 / 806.3 / 0.2 | **69.41** / 807.1 / 0.2 |
| 51 | - | 59.83 / 819.8 / 0.2 | 66.87 / 822.0 / 0.2 |
| 151 | - | 51.55 / 851.0 / 0.3 | 64.95 / 858.8 / 0.2 |
| 251 | - | 41.73 / 881.9 / 0.7 | 62.94 / 908.6 / 0.9 |
| 301 | - | **34.46** / 895.5 / 3.8 | 60.36 / 960.2 / 1.0 |
| 351 | 39.27 / 876.6 / 0.5 | **43.37 / 830.8 / 64.5** | 65.11 / 978.9 / **1.1** |

`1.8 · StallSpd` is `1.8 · 19.4 = 35.0 m/s`. The middle column crosses it between tick 251 and
tick 301, lift collapses, the aircraft banks to -0.788 rad and falls, and the velocity ends 64.5
degrees off the nose. The right-hand column holds 60 to 69 m/s for the whole run against a
`MaxSpd` of 69.44, which is the equilibrium `desc+50Ch = Accel / MaxSpd²` puts it at. Its slow
climb from 807 m to 979 m is the aim tick's own 0.0563 rad nose-up floor, faithfully applied.

### USN01, the mission

| | before | with all four |
| --- | --- | --- |
| `distance_moved`, all aircraft | **2180805 m** | **178295 m** |
| implied mean airspeed, 20 aircraft over 150 s | **727 m/s** | **59.4 m/s** |
| pilot attack, `range_last_mean` | 148870 m | **5535 m** |
| pilot attack, `closed_mean` | **-142090 m** | **+1250 m** |
| pilot attack, `worst_closed` | -249444 m | -3322 m |
| `pose_rotations` | 11907 | 11107 |
| torpedo drops / breakups / `swims_started` | 0 / 0 / 0 | 0 / 0 / 0 |
| torpedo hits, torpedo damage | 0, 0 | 0, 0 |
| mission hits / kills / damage | 23 / 1 / 220.0 | **1 / 0 / 9.3** |

**The two numbers that matter are the first and the fourth.** `distance_moved` falls by a factor
of twelve to exactly cruise speed, and the ordered aircraft go from opening 142 km on their targets
to **closing 1.25 km**. Before this packet twenty aircraft were averaging 727 m/s and flying away
from the mission; now they fly a real approach at the airspeed their row authors.

**The mission damage falls, and it should be read as a loss of an artefact.** Those 23 hits and
that one kill were produced by aircraft that (a) were pinned at their spawn coordinates for every
range and bearing a gun computes, because the pose was never published, and (b) were physically
travelling at hundreds of metres per second in the banked spiral. Nothing in either column is
torpedo damage. The remaining single hit at 25.10 s is ship gunnery. What this packet costs is a
number that was never earned; what it buys is aircraft that close on their targets.

**It also closes an open question in `docs/PLANE_FREE_FLIGHT_PHYSICS.md`.** That doc's "And the
banked-plane divergence, resolved for commanded aircraft only" concluded that uncommanded aircraft
stay in the spiral and called it "a host scope limit rather than a physics one", needing the
unread task kinds to close. It was a physics one. The missing forward drag was the whole of it:
with `007D9140` bound, every aircraft in USN01 - commanded or not - holds cruise, which is what
takes `distance_moved` from 2.18 million metres to 178 thousand.

### USN02

**Identical to both of the previous packet's columns**: 161 hits, 3 kills, 19061.0 damage, first
hit at 31.80 s, `swims_started = 44`, `torpedo_drop drops = 0`. Its own summary reads
`distance_moved=0.00 m pose_rotations=0 thinks=0`, so the mission carries no flying aircraft and
nothing in this packet can reach it. That is a three-way identity across two packets.

One USN02 run was discarded before this one: it died with an access violation at fixed step 2168
of 3000, before its summary. The rerun on the same binary completed normally and the machine had a
second agent's run finishing against the same lock at the time, so it is recorded as a transient
rather than attributed to this packet. It is not evidence either way.

### Still no swim, and the next gate by address and value

No aircraft released. The release chain needs the aim tick's altitude gate `009D20B4`, which is
`Interp(0.4, 40.0, 1.0, 25.0, time_to_target)` and therefore between **25 and 40 metres**, and
`009D20C4` passes only below it. The aircraft now fly at 807 to 979 m, and section 4 shows the aim
tick cannot bring them down: its `plan+2BCh` is a nose-up floor. The descent is upstream.
`kTorpedoCruiseProfile` (`src/bot_tasks.cpp:66`) points at `Pilot/Torpedo/CruisingAlt`, authored
500, while `TorpReleaseAlt` is 5 to 12 m. **Whatever loses that 490 m is the next gate**, and it is
follow-up 1.

## Follow-up packets

1. **What descends a torpedo run-in.** Not the aim tick. `Pilot/Torpedo/CruisingAlt` is 500 in
   `kTorpedoCruiseProfile` and the authored release altitude is 5 to 12 m, so something between the
   approach state and the aim state loses 490 m. It is the last gate before a swim.
2. **`007D9F60`**, the other arm of `007C6500`, and `unit+210h` which selects between them.
3. **The pitch mode `plan+2D0h`** and `0099E3D1`'s gate, which would let a task turn the pitch law
   off as the native does.
4. **The five unmodelled thrust scales**, most usefully `unit+0CC8h` and `007D20C6`'s `Accel`
   scaling, which set how quickly a plane reaches `MaxSpd` rather than what `MaxSpd` is.

## Correction from packet `cc8_torpedo_release_timer`: section 4's "nose-up floor" is a dive command

Section 4 reads the torpedo aim tick's `009D1EDD` write to `plan+2BCh` as a nose-up floor and a
pull-up, on the strength of a clamp whose lower bound was taken as `0.05625`.

* **was**: `plan+2BCh = clamp(-F34 / den, 0.05625, 0.872665)`, always positive, a pull-up.
* **is**: `clamp(-F34 / den, -1.3962634, +0.8726646)`, i.e. `[-DEG(80), +DEG(50)]`. `00D21318` holds
  the float `0xBFB2B8C3` = `-1.3962634`; `0.05625` is the **double** at those same eight bytes, and
  `009D1EA6 FLD float ptr [00D21318]` and `009D1EB0 MOVSS XMM0,dword ptr [00D21318]` are both
  four-byte loads. Since `009D1E98 FCHS` negates a quantity that is positive whenever the aircraft
  is above its release floor, the command is a **dive** that eases to zero at the floor.
* **evidence**: `docs/TORPEDO_RELEASE_TIMER.md` section 1.
