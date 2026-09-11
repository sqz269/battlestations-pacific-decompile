# The ship motion tick (packet `cc_controlled_unit`, part B)

Addresses: 00825F20, 0092E8C0, and read-only 0092D300, 00937440, 00813020, 0042AC60,
0042CF10, 00811890, 00825DE0, 00825EC0, 0080FC30, 0078CF20, 008E6430, 0092BE80.

Worker `agent/cc-controlled-unit`, 2026-09-11. Ghidra was read-only for this packet:
no renames, no comments, no prototypes. Every expression below comes from the assembly
listing, not from the pseudocode, because the whole routine is x87 and the decompiler
loses both the register arguments and the stack aliasing.

## What this packet adds

`docs/UNIT_FORCE_COMMANDS.md` established the shape of the command path and
reconstructed `0092D300`, `00937440`, `00825DE0` and `00825EC0`. It left three things
open that a working ship needs, and this packet closes them.

1. **`0092E8C0` is now complete.** That packet reconstructed only its call order and its
   rudder rate limiter, and recorded the body between `0092E96F` and `0092EB8F` as
   "analysed, not reconstructed". It is the whole steering model: the angular velocity is
   decomposed onto the three body-basis rows, the row-1 component is slewed toward the
   commanded yaw rate, the row-0 component takes an attitude righting term, and the three
   are recomposed.
2. **`00825F20`'s own sequence.** The order ring tick, the per-unit time scale, the keel
   sample point, the wave gate, the force-model dispatch and the boost block, with the
   call site of each.
3. **The force model's call site.** `00937440` had no recorded caller: it is reached at
   `00826A6D` as `controller->vtable[0](dt)`, and its single data reference is the vtable
   entry at `00D196F0`.

## The dispatch, and the frame delta

`00825F20` is **not** reached from `008255B0`, the per-frame unit update
(`docs/UNIT_INSTANCE_UPDATE.md`). It is a separate virtual: it appears in six vtables
(`00CF9070`, `00CFA738`, `00CFB6F8`, `00CFC394`, `00D09638`, `00D0C60C`) and has three
direct callers, two of which are subclass overrides that call it first and then add their
own work:

| site | containing routine | note |
| --- | --- | --- |
| `0085542F` | the routine starting `00855420` | no Ghidra function; forwards its own `[ESP+4]` float |
| `00749B2C` | the routine starting `00749B20` | no Ghidra function; same shape |
| `00644A38` | undefined region, nearest preceding function `006446E0` | not read |

Both wrappers take one float argument and pass it straight through, so the delta
`00825F20` receives is whatever the virtual's caller passes. The fixed simulation step is
`0.05f` at `00D0DE84` (`docs/FIXED_STEP_COUNTDOWN.md`, already
`kFixedSimulationStepFloat`), which is also the tick period the order ring is written for
(`kUnitStateMessageTickSeconds`). The probe runs at that step.

Inside the routine `EDI = unit + 310h` and `ESI = unit` (`00825F32
LEA ESI,[EDI-310h]`), which is why every field below is cited by its unit-relative
offset rather than by the decompiler's shifted `param_1`.

## The order of the motion path

| address | what happens |
| --- | --- |
| `00826121` | `00813020(unit+838h, dt)` - the order ring tick, with the **raw** delta |
| `00826126` | `if (unit+340h > 0) dt *= unit+340h` - a per-unit time scale |
| `0082614B` | `unit+1124h = max(unit+1124h - dt, 0)`, then `00825450(unit, dt)` |
| `00826866` | the keel sample point, from the pose and two class dimensions |
| `00826985` | `0078CF20(ocean, p.x, p.z)` - the wave height under that point |
| `00826994` | the throttle gate |
| `00826754` | the engine gate (hoisted far above the tail) |
| `00826A06` | the gameplay scale: `008E6430(4, unit)` or the literal `1.0f` |
| `00826A3A` | `target = ((maxSpeed * gameplayScale) * throttle) * engineGate` |
| `00826A6D` | `controller->vtable[0](dt)` - the force model, **before** either command |
| `00826A6F` | the boost block, which may replace `target` outright |
| `00826B29` | `0092D300(controller, target, dt)`, unless the gate closed |
| `00826B54` | `0092E8C0(controller, unit+984h, dt)`, unless `unit+9E4h` is set |
| `00826B6A` | `0092BE80(controller, dt)` - the controller step, owned elsewhere |
| `00826B84` | `unit->vtable[1ECh](dt)` |

The ring tick gets the raw delta and everything after it the scaled one, because the
rescale at `00826126..00826144` writes back into the argument slot `[ESP+0C4h]`.

## The keel sample point, `00826866..00826972`

```
along = float(class[A0h] * -0.5)          ; 00CEC9E0, a double
base  = translation + poseRow2 * along    ; poseRow2 = unit+ECh..+F4h, the forward axis
down  = float(class[A8h] * -0.5)
p     = base + poseRow1 * down            ; poseRow1 = unit+DCh..+E4h, the up axis
```

`EBP` is `unit+CCh` throughout (last written at `008267F2`), and `EBX` is the class
descriptor `[unit+538h]` (written at `00826774`); both were established by filtering the
whole listing for the register rather than by reading the nearest load. `class[A0h]` and
`class[A8h]` sit in the base vehicle-class block, below the ship reader's `+138h` HoD
records, so neither has a recovered Lua key. Half a hull length astern and half a hull
height down is the stern keel, which is what the gate below wants.

## The throttle gate, `00826994..00826A00`

```
00826985: h = 0078CF20(ocean, p.x, p.z)
0082698A: half = float(h * 0.5)                  ; 00D7A280, a double
00826998: if (p.y <= half) throttle = unit+980h
          else { throttle = 0; commandApplies = false; }
008269B1: if (unit+C4h == 8 && -3.0f > unit+100h) throttle *= settings[4B4h]
008269F4: if (unit+5Dh) throttle = 0
```

The comparison is against the keel point's **y**, not against a direction component:
`0078CF20` is `__thiscall(ocean, float, float)` with `RET 8`, so the `SUB ESP,8` at
`00826928` is undone by the callee and `[ESP+2Ch]` after the call is the same slot the
point's y was stored in at `[ESP+34h]` before it. A hull whose stern keel has risen above
half the local wave height makes no way and, uniquely among the gates here, also
suppresses the steering call (`BL` is cleared at `008269A5` and tested at `00826B0A`).

The submarine branch is class id 8 (`kUnitForceSubmarineClassId`) with the deck
reference `-3.0f` at `00CE3D50`; it scales the throttle and does not touch the gate.

`unit+5Dh` zeroes the throttle without closing the gate, so a hull with that byte set
still steers. The same byte rejects a unit from `00645060`'s selectable test
(`docs/CONTROLLED_UNIT.md`) and gates the sinking effects at steps 2 and 8 of `008255B0`,
which together read as "out of action"; that reading is provisional.

## The target speed, `00826A3A..00826A5F`

```
scaled = float(maxSpeed * gameplayScale)                 ; unit+9C0h, one store
target = float((scaled * throttle) * engineGate)         ; two multiplies, one store
```

The engine gate is `00826754..0082676E`, unchanged from `docs/UNIT_FORCE_COMMANDS.md`: a
0/1 float, open only when `unit+9E5h` is clear and `unit+9D8h` is not exactly zero.

## The boost block, `00826A6F..00826B04` (new)

Reached only when the unit answers `IsKindOf(0Eh)`.

```
00826A82: kind = unit+988h                 ; the order ring's current kind byte
00826A90: unit+118Ch = kind
          if (kind != 0) {
00826A98:     if (!(unit+1188h > 0.0f)) done;
00826AAB:     target  = float(0080FC30(unit) * class[80Ch]);   ; *replaces* the target
00826ABA:     unit+1188h = float(unit+1188h - dt);
          } else {
00826AC9:     v = float(dt * class[808h] / class[810h] + unit+1188h);
00826AF0:     unit+1188h = (v < class[808h]) ? v : class[808h];
          }
```

The store at `00826AB6` writes `[ESP+14h]`, which is the same stack slot the product
chain wrote at `00826A5F` (the `PUSH ECX` at `00826A4E` is undone by the `RET 4` of the
force-model virtual and the `RET 4` of the `IsKindOf` call), so the boost speed replaces
the commanded speed rather than scaling it. `class[808h]`, `+80Ch` and `+810h` lie above
the ship reader's last field (`kCapturePower`, `+804h`), so they belong to a derived
descriptor and have no recovered Lua key. `class[810h]` is a divisor with no zero guard.

## `0092E8C0`, complete

`void __thiscall(controller, float toTurn, float dt)`, `RET 8`, body
`0092E8C0..0092EB9B`. `ESI` is the controller, `[ESI+1Ch]` the unit, `[ESI+2Ch]` the
rigid body, `[ESI+80h]` the smoothed rudder.

```
0092E8C3: step = float(dt * 0.5)                       ; 00D7A280, a double
0092E8EF: 0042AC60(&controller+80h, toTurn, step)
0092E8F7: M = 00C32000()                               ; rows at +0h, +0Ch, +18h
0092E942: 00C31F20(&w)                                 ; the angular velocity
0092E950: rate = -00811890(unit, controller+80h)       ; FCHS at 0092E955
0092E966: yawTarget = 00825DE0(unit, rate)
0092E96F: rate1 = float((M11*w.y + M10*w.x) + M12*w.z)     ; the listing's operand order
0092E99B: rate2 = float((M21*w.y + M20*w.x) + M22*w.z)
0092E9B5: rate0 = float((M01*w.y + M00*w.x) + M02*w.z)
0092E9D1: if (unit->IsKindOf(0Eh)) {
0092E9E9:     a = 0042CF10(M21);
0092EA15:     if (|a| > 0.2617844f) {                  ; 00D05AA8, a float
0092EA21:         s = (0 > a) ? -1 : (a > 0) ? 1 : 0;
0092EA49:         e = float(s * 0.2617991 - a);        ; 00D19628 read as a double
0092EA5B:         rate0 = float(rate0 - e * (dt * 10.0));   ; 00CE3DC0, a double
              }
          }
0092EA75: maxStep = float(dt + dt)
0092EA7F: gap = |float(rate1 - yawTarget)|              ; the 0x7FFFFFFF mask form
0092EAAD: rate1 = (maxStep > gap) ? yawTarget
                : (yawTarget > rate1) ? rate1 + maxStep : rate1 - maxStep
0092EAD3: w' = (rate1*row1 + rate2*row2) + rate0*row0
0092EB8F: 00C37E20(&w')
```

The basis rows are the body axes: `0092D300` and `00937440` both take row 2 as the
forward axis, and the commanded yaw rate lands on row 1, so row 1 is up and row 0 the
remaining lateral axis. Which handedness the three make is not established, so whether
the row-0 term rights the hull or capsizes it is provisional; the arithmetic is exact
either way.

The rudder slews at `0.5` per second toward the command, and the yaw rate slews at
`2` per second toward the commanded rate. Both limits are hard-coded, not class data.

## x87 precision

Every block above is x87. Values live in the register stack at the control word's
precision and are rounded only at an `FSTP m32`. The reconstruction carries register
values as `double` and narrows with an explicit cast at exactly those instructions, which
matches the MSVC CRT default of 53-bit precision. If the shipped executable runs with the
x87 control word at 64-bit precision - Direct3D's `D3DCREATE_FPU_PRESERVE` is the usual
reason it would - the intermediates are wider than modelled. Nothing in this packet
establishes which; it is the first thing a differential run against the game would settle.

## Routines and coverage

| routine | state | coverage |
| --- | --- | --- |
| `00825F20` | analysed; the motion path reconstructed and build-tested | partial: the two jam ramps `00825FB1..008260F6`, the sub-updates `00826187..00826866` (wake, anchors, attachments, the inlined ring setters at `008266CE..0082674C`) and the tail after `00826B84` are not reconstructed |
| `0092E8C0` | reconstructed, build-tested, probe-exercised | complete |
| `0092D300` | read and reused from `docs/UNIT_FORCE_COMMANDS.md`; the accel selection re-derived from the listing and confirmed | complete |
| `00937440` | read and reused; its call site established | complete as reconstructed there |
| `00813020` | reused from `docs/UNIT_STATE_MESSAGE.md` | complete as reconstructed there |
| `00811890`, `00825DE0`, `00825EC0`, `0080FC30`, `0042AC60`, `0042CF10` | reused | as reconstructed in their packets |
| `0078CF20`, `008E6430`, `0092BE80` | not read here; host methods | none |

## Corrections

**To `docs/UNIT_FORCE_COMMANDS.md`, the constant at `00D19628`.** Its table records
`2.0f`. The float at that address is `2.0f`, but the roll damper's instruction at
`0092EA4D` is `FMUL double ptr [00D19628]`, an eight-byte read of
`00 00 00 40 52 C1 D0 3F` = `0.2617991`, fifteen degrees in radians. The float `2.0f` is
the low half of that double. The damper therefore pulls the attitude angle back toward
+/-15 degrees, not toward +/-30. The threshold `00D05AA8` is a separate float holding the
same angle to float precision, `0.2617844f`.

**To `docs/UNIT_FORCE_COMMANDS.md`, the two acceleration fields.** It names `class[504h]`
`brakeAccel` and `class[508h]` `driveAccel` (and
`include/bsp/unit_forces.hpp` carries `kShipClassOffBrakeAccel` / `kShipClassOffDriveAccel`
and the `brake_accel` / `drive_accel` members of `UnitAxialSpeedInputs`). The vehicle-class
reader settles it the other way: `ShipClassDescriptorOffsets::kMaxAccel` is `0x504` and
`kRetardation` is `0x508` (`docs/SHIP_CLASS_FIELDS.md`, loader `00831882..00831998`). The
selection in `0092D300` is consistent with the reader, not with the names: a hull below
its target and moving ahead accelerates at `class[504h]`, and one above its target and
moving ahead slows at `class[508h]`. The arithmetic in `unit_forces.cpp` is right; only
the two names are inverted. `src/ship_motion.cpp` maps by offset and says so at the call.

**To `docs/UNIT_FORCE_COMMANDS.md`, the acceleration selection's branch addresses.** Its
listing extract cites `0092D3F1`, `0092D43A` and `0092D466`. The stored listing has the
`00825EC0` call at `0092D444`, the selection at `0092D455..0092D4DF`, the rate limiter at
`0092D4E5..0092D534` and the recomposition at `0092D538..0092D588`. The reconstruction it
shipped matches the listing branch for branch; only the cited addresses are off.

**To `docs/UNIT_FORCE_COMMANDS.md`, `00937440`'s callers.** It records none. The routine
is `controller` vtable slot 0 and is reached at `00826A6D` inside `00825F20`; its only
data reference is `00D196F0`.

**To the packet brief.** It described the motion side as gated by "sinking, docked,
paused". There is no docked gate and no pause gate inside `00825F20`: the gates are the
wave gate at `00826998`, the out-of-action byte `unit+5Dh` at `008269F4`, the engine jam
`unit+9E5h`, the rudder jam `unit+9E4h` at `00826B2E`, and the per-unit time scale
`unit+340h` at `00826126`. A paused game stops the fixed-step driver upstream.

## The probe

`src/ship_motion_probe.cpp`, target `bsp_ship_motion_probe`. It reads
`vehicleclasses.lua` from the installed game, picks a destroyer, issues one
throttle-and-rudder order through `00815440` / `00816A40` / `0080DAD0`, and then steps the
ring tick, the motion tick and a stand-in integrator at `0.05 s`.

The run for `VehicleClass[11]`, Allen M. Sumner class 1945 (`MaxSpeed 17.4911`,
`MaxAccel 3`, `Retardation 2`, `MaxRotAngle 0.15708`), throttle 1, rudder 1:

* forward speed rises at `3 m/s^2` (the `class[504h]` branch of `0092D300`) and peaks at
  `17.4428`, ratio `0.9972` against the reference speed `17.4911`; it then settles back to
  `17.3631` because a sustained turn bleeds speed - `0092D300` rewrites only the axial
  component and leaves the lateral one alone, so the hull carries a sideslip;
* heading turns at exactly `0.15708 rad/s`, `9 deg/s`, once the rudder has finished its
  own `0.5/s` slew at about `t = 2 s`.

`--class 23`, the Fletcher 1943 (`MaxSpeed 18.7772`, `MaxRotAngle 0.139626`), behaves the
same way: peak `18.7331`, ratio `0.9977`, yaw rate exactly `0.13963 rad/s`. Its tighter
turn bleeds more speed, down to `17.69` after 15 seconds of full rudder, and the yaw rate
falls with it because `0082ECB0` scales the rate by the speed ratio.

**What the second check does and does not show.** The settings block at `+438h..+44Ch`
that `0082E890` interpolates was never recovered, from the image or from the installed
data. The probe forces the three denominator knots to `1.0`, which makes the curve return
`1.0` at every speed, so `0082ECB0` reduces to
`MaxRotAngle * speedRatio * rudder * efficiency`. That the probe then measures
`MaxRotAngle` at full speed and full rudder confirms the chain composes correctly
end to end; it does not confirm the game's turn rate, which is that value divided by the
real curve. The other stand-ins are the ocean sampler (`0078CF20`, flat sea at `y = 0`),
the gameplay scale (`008E6430`, `1.0`) and the rigid-body integrator, and the probe prints
all four in its header.

## no_ghidra_function

none. Every routine named or reconstructed here has a Ghidra function. The two subclass
wrappers at `00855420` and `00749B20` were read from the raw listing for their call sites
only; neither was named, bounded or reconstructed, and the `00644A38` call site was not
read at all.

## Uncertainties

1. The handedness of the body basis, and therefore whether the row-0 term in `0092E8C0`
   rights or worsens the attitude.
2. The x87 control-word precision the shipped executable runs with.
3. `0078CF20`, `008E6430` and `0082ECB0`'s settings block remain unrecovered; the first
   two were not read here and the third has no source in the image or the game data that
   this packet found.
4. `class[808h]`, `+80Ch`, `+810h`, `+A0h` and `+A8h` have no recovered Lua key; the
   readings "boost capacity, boost speed scale, refill time, hull length, hull height" are
   from their use, not from a producer.
5. `unit+340h`, the per-unit time scale, has no located writer.
6. Whether the trait id `0Eh` that gates both the boost block and the righting term names
   a class or a capability is unsettled; `008252C0` queries the same id for an audio
   settings block (`docs/UNIT_TIMED_SUBUPDATES.md`).

## Follow-up packets

* `ship_rudder_curve_settings` - addresses `0082E890`, `00424C40` and the settings
  object's constructor; files `docs/UNIT_RUDDER_CURVE.md`. Find what fills `+438h..+44Ch`
  so the probe's turn rate becomes the game's turn rate.
* `ocean_height_sampler` - address `0078CF20` and the ocean object at `[game+19F0h]`;
  files `docs/OCEAN_HEIGHT.md`. The last unreconstructed input to the throttle gate.
* `ship_boost_descriptor` - addresses `00826A6F` (read), the derived ship descriptor's
  reader, `class[808h]..+810h`; files `docs/SHIP_BOOST.md`. Which subclass owns those
  three floats and what their Lua keys are.
* `unit_motion_time_scale` - address `unit+340h` and its writers; files
  `docs/UNIT_TIME_SCALE.md`.
* `ship_motion_subupdates` - addresses `00825450`, `00826187..00826866`; files
  `docs/SHIP_MOTION_SUBUPDATES.md`. The wake, anchor and attachment blocks this packet
  labelled partial.
