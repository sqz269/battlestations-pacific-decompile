# How a command becomes motion on a unit (packet `unit_force_commands`)

Addresses: 00825F20, 0092D300, 0092D770, 0092E8C0, 00937440, 00937630, 00811890, 00825DE0,
00825EC0, 0074F930, and read-only 0074F2E0, 009329C0, 00822C20, 00818340, 0081ED40, 0099D300,
00936DC0, 0042AC60.

## The premise this packet inherited is wrong

`docs/UNIT_CONTROLLER_UPDATE.md` recorded that the object at `unit+10D4h` is the unit's
**external force list**, drained by `0074F2E0` at `00933A52`, and called it "the channel through
which everything outside the physics library pushes force onto a unit — engine thrust, damage,
flooding". Reading both halves of that object shows it is only the last of those three.

`0074F2E0` and `0074F930` are the two halves of a **leak (flooding) model**. It has no push
routine, no lifetime field and no one-shot flag, because nothing pushes a force into it. Its
segment 41 keyword set is `sumleaks, sumforces`, and the `_ship` diagnostic dump at `00818340`
names it directly: `00818404` emits the label `"leakManager"` and hands the same sub-object to
`0074EF20`.

So there is no external force list, and the force-command contract this packet was given does not
exist in the binary. What does exist is below.

## The shape of the real path

For a surface ship the command path is mostly **kinematic**, not force-based:

| stage | routine | what it writes |
| --- | --- | --- |
| throttle -> target speed | `00825F20` tail, `008269A9..00826A6D` | a float on the stack |
| target speed -> velocity | `0092D300` | body linear velocity, `00C37E50` |
| rudder -> yaw rate | `0092E8C0` via `00811890`, `00825DE0` | body angular velocity, `00C37E20` |
| rudder -> torque | `00937440` | `00C35330`, on top of `009329C0` |
| flooding -> torque | `0074F930` then `0074F2E0` | the controller's torque staging at `+74h` |

`00C35360` (AddForce) has exactly one caller in the whole image, `00933B01` inside
`009329C0`, and `00C35330` (AddTorque) has three: `00933B38` in the same routine, `00937613` in
`00937440` and `0092BF33`. Commanded thrust is therefore never a force at all. It is a direct
rewrite of the rigid body's linear velocity, and steering is half a velocity rewrite and half a
torque.

## The unit fields

Named by the `_ship` diagnostic dump `00818340`, which walks the object emitting
`(label, type, value)` triples through its writer's vtable.

| offset | name in the dump | evidence |
| --- | --- | --- |
| `+828h` | `sinkTime` | 00818354 |
| `+980h` | `thrust` | 0081857C, already `kUnitOffThrottle` |
| `+984h` | `toTurn` | 008185EB, already `kUnitOffSteering` |
| `+9A0h` | `helmsmanControl.thrust` | 00818724 |
| `+9A4h` | `helmsmanControl.toTurn` | 00818757 |
| `+9C0h` | `maxSpeed` | 00818613, already `kUnitOffReferenceSpeedBase` |
| `+9D8h` | `thrustMod` | 00818372 |
| `+9DCh` | `turnEfficiency` | 0081835D |
| `+9E4h` | `steeringJam` | 00818369 |
| `+9E5h` | `engineJam` | 00818370 |
| `+104Ch` | `torpedoStock` | 0081856C |
| `+1130h` | `shipYardLaunch` | 00818621 |
| `+1134h..+114Bh` | `repairPreferences[6]` | 00818640 |
| `+114Ch` | `repairTimer` | 00818700 |
| `+115Ch` | `camoColor` | 00818540 |
| `+1160h` | `camoColorGun` | 00818554 |
| `+10D4h` | `leakManager` | 00818404 |

`00822C20` resets the modifier group on spawn: `maxSpeed = class[500h]` at `00822C4F`,
`thrustMod = 1.0f` and `turnEfficiency = 1.0f` at `00823714`/`0082371C`, `engineJam = 0` and
`steeringJam = 0` at `00823727`/`0082372E`. Only part of that routine was read; it also packs a
byte block that looks like network state, and it was not analysed further.

## Throttle to target speed, `00825F20`

`00825F20` is the ship's motion tick, `void __thiscall(unit, float dt)`, RET 4. Ghidra decompiles
it with a shifted base: its `param_1` is `unit + 310h`, so `param_1+228h` is the class block at
`unit+538h`, `param_1+D08h` is the controller at `unit+1018h`, `param_1+670h` is the throttle and
`param_1+674h` is the steering. That shift is why the field names below come from the listing.

```
00826985: 0078CF20(...)            ; ocean height
0082698A: fmul qword [00D7A280]    ; * 0.5
00826998: fcomip with [esp+2Ch]
008269A2: throttle = 0 ; BL = 0    ; the wave wins: the whole command pair is suppressed
008269A9: throttle = [EDI+670h]    ; unit->thrust
008269B1: cmp [EDI-24Ch], 8        ; unit+C4h, the class id
008269D8: comiss -3.0f, [ESI+100h] ; the pose translation's y
008269E6: throttle *= settings[4B4h]
008269F4: cmp byte [EDI-2B3h], 0   ; unit+5Dh
008269FD: throttle = 0
00826A3A: fld [ESI+9C0h]           ; maxSpeed
00826A46: fmul [esp+14h]           ; * the gameplay scale
00826A57: fmul [esp+14h]           ; * throttle
00826A5B: fmul [esp+1Ch]           ; * the engine gate
00826B29: call 0092D300(target, dt)
00826B2E: cmp byte [EDI+6D4h], 0   ; unit->steeringJam
00826B54: call 0092E8C0(unit->toTurn, dt)
```

The gameplay scale is `008E6430(4, unit)` when both `00E0C978` and `(*00F88C30)+B8h` are set, and
the literal `1.0f` at `00D7A24C` otherwise. `008E6430` was not analysed.

The engine gate is computed far earlier, at `00826754..0082676E`:

```
00826754: cmp byte [ESI+9E5h], 0   ; engineJam -> gate 0
0082675D: movss xmm0, [ESI+9D8h]   ; thrustMod
00826765: ucomiss xmm0, xmm1       ; xmm1 zeroed at 008266B4
0082676E: al = 1                   ; only when thrustMod != 0.0f
00826792: [esp+18h] = (float)al
```

So `thrustMod` acts as a boolean here, not as a scale: a ship whose `thrustMod` is exactly zero
makes no way at all. Whether it scales anything elsewhere was not established.

`target = ((maxSpeed * gameplayScale) * throttle) * engineGate`, in that order.

## Target speed to velocity, `0092D300`

`void __thiscall(controller, float commanded_speed, float dt)`, RET 8. `this` is the controller at
`unit+1018h`; `controller+1Ch` is the unit and `controller+2Ch` is the body.

```
0092D30E: 00C31F40(&v)             ; linear velocity
0092D316: 00C32000()               ; the body basis; the axis is (+18h, +1Ch, +20h)
0092D3A6: sqrt(...)                ; the CRT square root, argument not read out
0092D3B7: dir = (a.x/len, 0.0f/len, a.z/len)
0092D3CE: current = dot(v, dir)
0092D3D8: heel = |a.y|             ; the 00D7A208 form
0092D3E9: target = (1 - heel) * commanded_speed
0092D3F1: accel = 00825EC0(unit)
          current <= target && current >= 0  -> accel = class[504h]
          current >  target && |current| < 3 -> accel = 2 * class[504h]
          current >  target && current <= 0  -> accel = class[504h]
0092D43A: move = accel * dt; if (move <= |current - target|) step target by move
0092D466: v' = v - current*dir + target*dir
0092D4A8: 00C37E50(&v')
```

The y component of `dir` is a literal `0.0f` divided by the length (`0092D3B7`), so the vertical
velocity survives untouched whatever the hull's attitude is. That is the only reason a ship under
power does not fly or sink from its own throttle.

The heel scale `(1 - |a.y|)` costs a heeling ship its speed, and the same factor appears again in
the rudder torque.

`0092D770`, `void __thiscall(controller, float speed)`, RET 4, is the unlimited form: it replaces
the axial component outright, with the basis used unnormalised. Its callers are `0081F980` and
`00822C20`, both of which look like spawn or reset paths.

`00825EC0`, `float10 __fastcall(unit)`: `accel = class[508h]`, and when `unit+1038h > 0.0f` it is
scaled by `(1 + boost * (settings[228h] - 1))`.

## Rudder to yaw rate, `0092E8C0`

`void __thiscall(controller, float toTurn, float dt)`, RET 8.

```
0092E8C3: step = dt * 0.5           ; 00D7A280, a double
0092E8EF: BSP_Math_StepTowards(&controller+80h, toTurn, step)
0092E8F7: 00C32000()                ; the basis, nine floats from +0 to +20h
0092E942: 00C31F20(&w)              ; angular velocity
0092E947: rate = 00811890(unit, controller->smoothedRudder)
0092E955: fchs                      ; negated
0092E966: rate = 00825DE0(unit, rate)
...       an orientation-relative decomposition and a roll damper
0092EB8F: 00C37E20(&w')
```

The body between `0092E96F` and `0092EB8F` decomposes the current angular velocity onto the body
basis, rate-limits the yaw component and, when the unit's vtable `+5Ch` answers `0Eh`, adds a roll
correction that pulls the roll angle (`0042CF10` of basis element `+18h`) back toward
`±2.0f * 0.2618f` — `00D05AA8` is `0.2618f`, fifteen degrees in radians. That part is analysed, not
reconstructed: the decompiler loses the x87 register arguments and the listing was only read once.

`00811890`, `float10 __thiscall(unit, float rudder)`, is the rudder map:
`rate = 0082ECB0(rudder, forwardSpeed, unit->turnEfficiency)`, scaled by `008E6430(5, unit)` under
the same two globals as the throttle scale. `0082ECB0` was not analysed and is the missing curve.

`00825DE0`, `float10 __thiscall(unit, float rate)`, is the propeller turn assist: while
`unit+1030h > 0` and `|rate| < settings[220h]`, the rate is pushed away from zero by
`min(|rate|, settings[224h] * (settings[220h] - |rate|)) * unit->propellerLoad`. This is the only
recovered consumer of the propeller load, and it explains the field: a ship with the screws turning
answers the helm at low yaw rates, a ship without them does not.

## Rudder to torque, `00937440`

`void __thiscall(controller, float dt)`, RET 4. One of the four force-model overrides; it adds a
torque and then tail-calls `009329C0`.

```
00937449: if (unit+5Dh) -> skip straight to 009329C0
0093745E: 00C32000(); 0092D746-style axis at +18h/+1Ch/+20h
0093746D: 00C31F40(&v)
00937489: ratio = |dot(v, axis)| / BSP_UnitInstance_GetReferenceSpeed()
009374A1: mass = class[B0h]
009374C5: heel = |unit+D0h|             ; pose row 0, column 1
009374E8: gain = (1 - heel) * ratio * unit->toTurn * settings[588h]
009374FE: mass2 = mass * mass
0093750D: torque = mass2 * gain * (unit+ECh, +F0h, +F4h) / 10000.0
00937613: 00C35330(&torque)
```

The divisor is the double `10000.0` at `00CE4BD8`. The torque is spent on pose row 2; which basis
vector that row is has not been settled, so calling it a yaw torque is provisional.

`00937630`, the fourth override, freezes the unit instead: when `unit+1189h` is set it zeroes both
the linear and the angular velocity (`00C37E50`, `00C37E20`) and calls `0080E170(0)` without ever
reaching `009329C0`.

`00936DC0` is the third override and is a depth-holding model with a four-entry table at
`unit+1200h`; it was read but not analysed.

## The leak model at `unit+10D4h`

`0074F930`, `void __thiscall(leak, float dt)`, RET 4. Three parallel arrays, `count` long:

| offset | contents |
| --- | --- |
| `+08h` | per-leak rate ceiling |
| `+0Ch` | hull water capacity |
| `+10h` | the owning unit |
| `+14h` | count |
| `+18h` | `float rates[count]` |
| `+1Ch` | `float water[count]` |
| `+20h` | `float3 points[count]`, stride 0Ch |
| `+24h` | flooded flag |
| `+28h` | total water, **which is `unit+10FCh`** |
| `+2Ch` | total rate |

`0x10D4 + 0x28 == 0x10FC`. That settles uncertainty 3 of `docs/UNIT_CONTROLLER_UPDATE.md`:
`unit+10FCh` is not an unexplained additive mass, it is the accumulated flood water, produced by
the second pass of `0074F930` at `0074F9B2..0074F9C8`. `009329C0` reads it twice, once to raise the
effective mass at `00932A2C` and once to cut the buoyancy by `water * 10.0` at `00933A3A`.

The tick: rates under `0.001f` snap to zero; the raw rate accumulates into the total rate while the
capped rate adds `rate*dt` of water; the total water is then re-summed and, unless the unit's health
read is gated by `+5Dh`, capped at `(1 - health) * settings[404h] * capacity`, scaling every water
value and zeroing every rate when the cap bites; finally the flooded flag is `total > 0.01f`.

`0074F2E0`, `void __thiscall(leak, float* out)`, RET 4, turns the water weights into a torque
through the unit's pose block at `+CCh`. Only x and z are written; y stays zero:

```
out.x += w * (p.x*row0.z + p.y*row1.z + p.z*row2.z) * 10.0
out.z -= w * (p.x*row0.x + p.y*row1.x + p.z*row2.x) * 10.0
```

with the same `10.0` gain (`00CE3DC0`) the hydrodynamic callback uses. The caller adds the result
into the controller's torque staging at `+74h..+7Ch`.

## Constants

| address | value |
| --- | --- |
| `00D7A208` | `-0.0f`, the absolute-value subtrahend |
| `00D7A218` | `0.0f` |
| `00D7A24C` | `1.0f` |
| `00D7A280` | `0.5` (double) |
| `00CE3854` | `3.0f`, the creep threshold |
| `00CE3D50` | `-3.0f`, the deck reference |
| `00CE4BD8` | `10000.0` (double), the torque divisor |
| `00CE3DC0` | `10.0` (double), the force gain |
| `00D7A23C` | `0.001f`, the leak rate floor |
| `00D7A238` | `0.01f`, the flooded mark |
| `00D05AA8` | `0.2618f`, fifteen degrees |
| `00D19628` | `2.0f` |

## Callers and callees

`00825F20` is reached from the ship update; it calls `0092D300`, `0092E8C0`, `0092BE80`,
`0092E5B0`, `00811890`, `008E6430`, `0078CF20`, `00424C40`, `00414DB0`, `00413920`, `004134F0` and
the controller's vtable slot 0.
`0092D300` calls `00C31F40`, `00C32000`, the CRT square root, `00825EC0` and `00C37E50`; its only
caller is `00825F20`.
`0092D770` calls `00C31F40`, `00C32000` three times and `00C37E50`; callers `0081F980`, `00822C20`.
`0092E8C0` calls `0042AC60`, `00C32000`, `00C31F20`, `00811890`, `00825DE0`, `0042CF10` and
`00C37E20`; its only caller is `00825F20`.
`00937440` calls `00C32000`, `00C31F40`, `BSP_UnitInstance_GetReferenceSpeed`, `00414DB0`,
`00424C40`, `00C35330` and `009329C0`.
`0074F930` calls `00424C40` and `00923BE0`; `0074F2E0` calls `00414DB0`, `00413920` and `004134F0`.

## Uncertainties

1. **Nothing writes `unit+980h` or `unit+984h` in any encoding a displacement scan can see.** Every
   `movss`/`fstp`/`mov`/`lea` form against `[reg+980h]` and `[reg+984h]` was scanned across the
   whole image: the only stores are `mov [esi+980h], ebx` / `mov [esi+984h], ebx` at `004DA32B` in
   an unrelated scene loader, and every ship-side access is a load. The producers must use a shifted
   base register (`00825F20` itself reads them as `[EDI+670h]` with `EDI = unit+310h`), which no
   byte pattern can find. The player and AI producers are therefore **not identified**, and the
   packet's central question is open.
2. `unit+9A0h`/`+9A4h`, the helmsman override pair, are read by the ship AI at `0099D5A4` and
   `0099D694`: when either is non-zero it is clamped and used in place of the AI's own plan
   (`0099D679`, `0099D6B7`, each with a validity byte). Their writers were not found either. The
   plane classes have a control pair at the same two offsets written by `luaMW_PlaneSetRollCtrl`
   (`0089DE45`) and `luaMW_PlaneSetPowerCtrl` (`0089DFE5`); no ship equivalent exists in the Lua API.
3. `unit+102Ch`, `+1030h`, `+1034h` and `+1038h` are zeroed together by the ship constructor at
   `0081F2EC..0081F304` and are read by `00825DE0`, `00825EC0` and three AI routines
   (`009D4E30`, `009DE5B0`, `009F3F80`). No writer was found, by the same shifted-base problem. The
   propeller load's producer therefore remains open, as `docs/UNIT_TIMED_SUBUPDATES.md` recorded.
4. Which basis vector pose row 2 is, and therefore whether `00937440` is a yaw torque, is unsettled.
5. The square-root argument at `0092D3A6` was not read out of the listing; the flattened norm is the
   reading that matches the divides.
6. `0082ECB0`, the rudder-to-yaw-rate curve, and `008E6430`, the gameplay scale hook, were not
   analysed.
7. `thrustMod` is used only as a non-zero test on the command path; whether it scales anything is
   unknown.
8. The roll damper inside `0092E8C0` is described from one reading of the listing and is provisional.

## What remains

* The writers of `unit+980h`, `+984h`, `+9A0h`, `+9A4h` and `+102Ch..+1038h`.
* `0082ECB0` and `008E6430`.
* `00936DC0`, the depth-holding force model, and its `unit+1200h` table.
* The rest of `00822C20` and of `0081F980`.

## Follow-up packets

* `unit_command_producers` — addresses `00825F20` (read), `0099D300`, `009998A0`, `009D4E30`,
  `009F3F80`; files `docs/UNIT_COMMAND_PRODUCERS.md`. Find the writers of `unit+980h`/`+984h` by
  decompiling the ship AI's output stage rather than by scanning displacements, and settle whether
  the player writes them directly or through the same AI object.
* `unit_rudder_curve` — addresses `0082ECB0`, `00811890`, `00811940`, `00811960`, `00811AB0`;
  files `docs/UNIT_RUDDER_CURVE.md`. The rudder-to-yaw-rate curve and the small ship accessors
  around it.
* `unit_leak_model` — addresses `0074F930`, `0074EF20`, `0074F2E0`, and the leak's push site;
  files `docs/UNIT_LEAK_MODEL.md`. Who opens a leak, and the `leakManager` dump's field names.
* `unit_depth_force_model` — address `00936DC0`; files `docs/UNIT_DEPTH_FORCE_MODEL.md`. The
  submarine depth controller and the `unit+1200h` table.

## State reached

| address | state |
| --- | --- |
| `00825F20` | analysed; the command tail reconstructed and build-tested |
| `0092D300` | reconstructed, build-tested, one focused test |
| `0092D770` | reconstructed, build-tested |
| `0092E8C0` | analysed; only its call order and its rate limiter reconstructed |
| `00937440` | reconstructed, build-tested |
| `00937630` | analysed |
| `00811890` | analysed |
| `00825DE0` | reconstructed, build-tested |
| `00825EC0` | reconstructed, build-tested |
| `0074F930` | reconstructed, build-tested |
| `0074F2E0` | reconstructed, build-tested (read-only, outside the lease) |
| `00822C20`, `0081F980`, `00936DC0`, `0099D300`, `00818340` | exported and read |

Every routine here has a Ghidra function; there is no listing-only routine in this packet.
`bsp.py ghidra flow` was not run, and no `_free` fall-through gap appeared in the listings read.
