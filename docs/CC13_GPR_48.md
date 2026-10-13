# CC13 GPR 48: segment 48 reference models (plane parts)

Packet `cc13-gpr48` (cc13 lineage, Claude). Segment 48 is `007D1E50`-`007F8390`. cc9 approved this
reference-model pass on these terms:
- new `*_cc13` files only: `src/gameplay_seg48_cc13.cpp` and `include/bsp/gameplay_seg48_cc13.hpp`,
  registered by one line in `cmake/startup.cmake`;
- ledger records;
- no host edit, no wiring, no game run.

Excluded by cc9: `007D0000`-`007D3FFF` (the kamikaze and crash path), the actuator and wing-fold chain
(`007DE1E0`, `007DE3A0`, `007EABC0`, `007B8D40`, `007C11E0`), the water law (`007DAFD0`, `007DCDD0`), the
plane bot tasks and the air operations. The lease covered `007D4000`-`007F8390`.

Method. Each comparison and float rounding was re-read from the disk bytes with Capstone `disasm-raw`,
the x87 from the opcode bytes. The models are **reconstructed and build-tested**. None of them is
ABI-compatible, wired or game-validated. Gameplay runs at 24-bit x87 precision, so the models compute in
float where the image rounds to float.

## 1. Models

### 1.1 The actuator block's part animation (`007DE7E0`, `007DE9A0`)

cc9's actuator step (`007DE3A0`, `include/bsp/torpedo_release_spawn.hpp:87`) moves the channel values.
`007DE9A0` is a different routine: it chooses which channel's position the plane's part animation shows,
and turns that position into an animation time.

- **`007DE7E0`, the channel time.** A moving channel whose stamp `+18h` is not negative is extrapolated
  from its last step to the fixed-step clock `[00F876A4]`:
  `v = float(rate * (target ? 1 : -1) * float(clock - stamp)) + value`. The result is
  `00419010(0, +10h, 1, +14h, v)`, a clamped lerp into the channel's animation range. The comparison
  is `COMISS` then `JB`, so a NaN stamp skips the extrapolation. Note that `+18h` is the float clock: the
  step `007DE274` copies `[00F876A4]` there with `MOVSS`, and `007DE7E0` subtracts it as a float.
  cc9's `PlaneActuatorChannel::frame` (`unsigned`) holds the same bits.
- **`007DE9A0`, the pick.** Nothing runs when the owner's `+5Dh` is set. Every equality test is
  `LAHF` / `TEST AH,44h` / `JP`, so a NaN compares unequal.

  | condition | time shown |
  | --- | --- |
  | `+44h` enabled and its value `!= 1.0f` | `+44h` |
  | otherwise, `+28h` enabled and its value `!= 0.0f` | `+28h` |
  | otherwise, `+60h` enabled | `+60h` |
  | otherwise, `+28h` enabled (value `0.0f`) | `+28h` |
  | otherwise | the old time |

  When the time differs from `+7Ch` (`FUCOMIP` / `JNP`), the routine stores it and calls
  `[[block+20h]]+24h(block+24h, time, 1)`.

  In cc9's names, `+44h` is the wings channel, `+28h` the gear and `+60h` the ordnance bay. Folding
  wings therefore take priority over the gear, and an extended gear over the bay. The pick is
  presentation; no reader of `+7Ch` was traced in this pass.

### 1.2 Engines and rotors (`00D08640` base, `00D0865C` rotor)

The vtables, read from the image:

| slot | base `00D08640` | rotor `00D0865C` |
| --- | --- | --- |
| 0 | `007E1BA0` (the same body as `+8h`) | `007EBF10` Init |
| `+4h` | `007DDEB0`: its own `+8h` | `007DDEC0` Restart(bool) |
| `+8h` | `007E1C00`: `+9h = 0`, stops the point effect `+4h` | `007E1C60` Reset |
| `+0Ch` | `007DDF00`: `+9h = 1` | `007DDF10` StopEngine |
| `+10h` | `007E23B0` (`RET 8`) | `007EC180` StepRpm(target, dt) |
| `+14h` | `007E23C0` (`RET 4`) | `007EC1F0` ApplySpin(dt) |
| `+18h` | `007E23D0` (`RET`) | `007EC3B0` blur blend |

`007E23B0`-`007E23D0` have no Ghidra function.

- **The speed spread (`007ECA0E`-`007ECB1C`, inside `007EC5A0`).** The routine draws `lo = U(0, 0.2)`, then
  `hi = U(8, 1)`, both on **stream 1**. The constants are `00CE54A0 = 0.2f` and `00CE3918 = 8.0f`; the
  second draw's lower bound is the 8. It fills `count + 1` values with
  `float(i / count) * (hi - lo) + lo`, then shuffles them by Fisher-Yates from `k = 1`, using
  `NextU32(1) % (k + 1)` (`00BD2FC0`). Engine `i` is initialised with `spread[i]`.
- **Init (`007EBF10`, `RET 10h`).** The call at `007ECC85` passes four arguments: `00476B90`'s model, the visual
  class `+160h` (a second model, may be null), the index (or `-1` when `class+140h < 2`) and `spread[i]`.
  - The spin sign `+2Ch` is `+1.0f` for an odd index (C remainder, so `-1` counts as odd) and `-1.0f`
    for an even one.
  - `+30h = (Rotor/SpeedRandom * spread + 1.0) * Rotor/SpeedBase`.
  - The visibility: still-damaged 0, still 1, the second model's pair 0.
  - `+38h = +3Ch = U(0, 2 pi)` on **stream 1** (`007EC10F`).
- **Reset (`007E1C60`).** The visibility of Init and the same **stream-1** draw (`007E1CCF`), then a tail
  jump to `007E1C00`. Restart (`007DDEC0`) calls Reset and sets the rpm `+34h` to 1 or 0. The engine
  list's ResetAll `007DE630` (from `007B97F0`, restore full health) also reaches it.
- **StopEngine (`007DDF10`).** Once: the still node off, the still-damaged node on (each on both models), and
  `+24h`/`+28h` now point at the damaged pair. The pseudocode's argument order is garbled; the listing
  was used.
- **StepRpm (`007EC180`).** `rpm = 0042AC60(rpm, stopped ? 0 : target, RotorSpeedChange * dt)`, then
  `angle = fmod(spin_rate * rpm * dt + angle, 2 pi)`. The modulus is the double `00CE3828`.
- **ApplySpin (`007EC1F0`).** The shown angle is `fmod(rpm * spin_rate * dt + angle, 2 pi)`, and the blur
  nodes are rotated about z by `-(angle * sign)`.
- **Blur blend (`007EC3B0`).** `s = 00419010(StillOnlySpeed, 1, BlurredOnlySpeed, 0, rpm)`. The current still
  node gets `s`, and the blur nodes get `1 - s` (`DE E1`, `FSUBRP`). So does the second model's current still
  node `+28h`: the image gives it the blur weight, not `s`. This is recorded as read.
- **Engine destruction (`007EB0B0`, `007E23E0`).** Message 101 (`"enginefire"`, `007D0B80`) reaches
  `007EB0B0(i)`, which bounds-checks `i` and calls `007E23E0`. Only while `+8h` is clear, that routine
  stops the engine (unless `+9h` is set) and spawns the class `+220h` effect for a plane with a Lua self
  `+4A4h`. It then sets `+8h = 1` (`007E24F7`). **This is the writer of the "burning" byte** that
  `007DE5B0` scans for in the death step `007CAF10` (docs/GUN_AIM_TERMS.md).
- **The engine sound (`007DE490`).** It runs only with a sound at `+10h`. Two rate-limited parameters,
  `+14h` toward `+1Ch` at `+18h * dt` and `+20h` toward `+28h` at `+24h * dt`, go to the sound's slots
  `+10h` and `+0Ch`.

### 1.3 The damage-effect switch (`007E27B0`)

`this` is `unit+E80h`. The plane's per-tick routine `007C6820` calls it with dt whenever the pointer is
non-null. The class list is `[unit+538h]+224h`.

- `d = 1 - health`. The routine computes `n = min(trunc(00419010(0, 0, 0.6, 1, d) * (count + 1)), count)`,
  with `FIMUL` and then `00BF7420` (truncation).
- **State 0.** When `n > 0` and `+10h != d`, it sets the timer to 10 s (`00CE38B8`), the flicker to 1 and the
  state to 1.
- **State 1.** `timer -= dt`. Then `r = U(0, 1)` on **stream 0**: `XOR ECX,ECX` at `007E2852`, the draw at
  `007E2862`. `flicker = 0042AC60(flicker, r*r, 0.5)`, and `007DE760(0, flicker)` sends the flicker to every
  sound of the effect.
  - While the timer is not negative, the held definition is `list[max(n-1, 0)]`.
  - When it goes negative, the state returns to 0 and the definition to none.
  - In either case `007E2600` rebuilds the effect when the definition changed.
- `+10h = d` on every path.

The draw happens once per tick for the 10 s after every change in a plane's damage, but only for a class
with a non-empty damage-effect list. It is presentation drawn from the gameplay stream.

### 1.4 The flight controller (`ctl = unit+AB0h`)

- **`007D8330` / `007DB1F0`, the control-state record.** The 3Ch-byte record carries:
  - the world velocity, the body velocity and the angular velocity;
  - `+4h`/`+5h`, `+ACh`;
  - the timed hold of the dyn object, `dyn+B4h..C0h` (docs/PLANE_DYN_TIMED_HOLD.md).

  On apply, `rec+26h` selects "rebuild the world velocity from the body velocity through `unit+74h`". The
  writer always clears that flag. Apply then copies `+18h` to `+30h`, refreshes the body frame (`007D9C10`) and
  calls `007D80C0(0)`.
- **`007D9EE0`, set the world velocity.** It sets the world velocity, zeroes the angular velocity (`00F87574`) and does
  the same refresh.
- **`007DB2C0`, reset the motion.** It runs once, latched by `+14h`. It zeroes:
  - the body angular `+48h` and the body linear `+3Ch`, with their copies `+60h`/`+54h`;
  - `+6Ch`, `+78h`, `+7Ch`, `+94h`, `+A0h`, `+A4h`, `+ACh`;
  - the `+B0h` matrix, to all zeros (not identity).

  It then rebuilds the world velocity (`007D9C80`). On every call, it also stores
  `float(007C47F0() * 1.05)` (`00CF2510`) into `dyn+CCh`.
- **`007D7D70`.** It sets `dyn+98h..A0h` to `(0, 1, 0)` and `dyn+A4h` to 1.

### 1.5 More of the controller, the control surfaces and the archive keys

- **`007D8020`, the speed terms.** This is the tail of `007D9C10` and `007D9C80`, which
  `src/plane_angular_velocity.cpp` leaves out. It computes `s = sqrt((vy*vy + vx*vx) + vz*vz)` over the
  world velocity, or 0 at or below `1e-10`, and stores:
  - `+6Ch = s`;
  - `+70h = +3Ch / s` and `+74h = +40h / s`, the body velocity's x and y over the speed. The divides
    are `DE FA` (`FDIVP`) and `D8 7E 40` (`FDIVR`), read from the bytes;
  - `+A8h = -0.0f - +4Ch`, `+A4h = +48h`, `+A0h = +50h`: the body angular velocity, reordered and with y
    negated.
- **`007D99C0`, the forward speed** (nine callers). The speed is `ctl+44h`, plus a term for a unit with a
  parent (`unit+3Ch`, a carrier deck): `z * (z >= 0 ? 0.1 : 0.8)`. Here `z` is the parent's velocity
  (its slot `+34h`) in the unit's frame, taken through the inverse world matrix `+110h`. The host
  (`src/game_hosts_units.cpp:17451`, `:10575`, `:30002`) takes that term as zero.
- **`007D8180`.** It sets `ctl+94h` when its argument is set, or in game mode 0 or 1. `007DB2C0` clears
  the byte.
- **`007D7C00`.** It copies the world and body velocities into `dyn+4Ch`/`+64h` and zeroes the six
  accumulators and `dyn+94h` (docs/PLANE_FREE_FLIGHT_PHYSICS.md).
- **`007D7EA0`, the controller constructor.** Besides its defaults (`+90h = 99.0f`, `+98h = +9Ch = 1.0f`), it
  draws the dying spin `ctl+F0h..F8h = U(DeadMeat RotationMin, RotationMax) * direction`. The direction
  comes from `00439950`: `z = U(-1, 1)`, `phi = U(0, 2 pi)`. All three draws are on **stream 1**, once per
  plane. docs/CC13_RNG_STREAM_CENSUS.md lists the first as NM, "not drawn".
- **`007DE2E0`, a control surface.** It runs while the deflection differs from the target and the node's
  `+ACh > 0`. It steps the deflection toward the target by a fixed `0.05` per call (`00CE7638`; no dt) and
  rotates the node about y by `-(gain * deflection)`. The four signed axes it is handed are
  `include/bsp/plane_advance_pose.hpp:141` (`ControlSurfaceDrive`).
- **The archive keys** are original identifiers, written as strings into save games:

  | owner | keys (offset) |
  | --- | --- |
  | actuator channel (`007DE880`/`007DE900`) | `targst` (`+1h`, byte), `curst` (`+4h`, float) |
  | actuator block (`007DEA60`/`007DEB70`) | `animFrame` (`+7Ch`), `gears` (`+28h`), `wings` (`+44h`), `baydoor` (`+60h`) |
  | flight controller (`007DB430`/`007D9B70`) | `V` (`+18h`), `Om` (`+24h`), `Turbo` (`+4h`, byte) |
  | lerp value (`007E0740`/`007E07E0`) | `currentValue` `+0h`, `rate` `+4h`, `targetValue` `+8h` |
  | lerp pair (`007E08B0`/`007E0960`) | `pitch` (`+14h`), `volume` (`+20h`); the sound is at `+10h` |
  | timed flag (`007E0A10`/`007E0AB0`) | `timeOut` `+0h`, `value` `+4h` (byte), `timer` `+8h` |
  | effects component (`007E0B80`/`007E0D20`) | `running` `+14h`, `flying` `+24h` (a timed flag), `RPM` `+30h`, `startStopTimer` `+40h`, `runningSound` `+48h`, `deadmeatSound` `+74h`, `windSound` `+A0h`, `fallSound` `+CCh`, `engines` |

  These keys settle three readings:
  - the actuator channels are the gear `+28h`, the wings `+44h` and the bay doors `+60h`, which matches
    cc9's naming;
  - `ctl+4h` is the turbo flag;
  - the engine sound's two parameters are pitch (sound slot `+10h`) and volume (slot `+0Ch`).

  The block loader reads `animFrame`, then overwrites `+7Ch` with -1.0 before re-running `007DE9A0`. The
  saved value is therefore never used.
- **The flying flag (`007EB4E0`, inside `007EB380`).** It debounces the plane's flying predicate
  (`unit+72Ch` slot `+38h`). When the predicate equals `value`, the timer resets. Otherwise the timer
  grows by dt, and `value` takes the new state once the timer exceeds `timeOut`. `timeOut` is GameTuning
  `+6C0h`, set by `007EC5A0`.

## 2. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs and where the host gets them | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `actuator_block_anim_007de9a0`, `actuator_channel_anim_time_007de7e0` | none. It sits beside the host's block `src/game_hosts_units.cpp:1600` (`bsp::PlaneActuatorBlock`, stepped by `plane_actuator_block_step_007de3a0`) | the host's three channels; each channel's animation range `+10h`/`+14h`, which `007EABC0` fills from the class part animations (host: `wing_fold_class_007eabc0`, `src/game_hosts_units.cpp:13398`); the fixed-step clock and the float stamp `+18h` | none | OFF: none. ON: presentation only. The part animation time follows wings, then gear, then bay. No gameplay field changes. |
| `rotor_*`, `engine_base_*`, `engine_list_*` (spin, blur, stop, reset), `rotor_speed_spread_007eca0e` | none. `007EC5A0` is HOST-OMITS in docs/CC13_RNG_STREAM_CENSUS.md 10.4 | GameTuning Rotor/* (`+160h`..`+17Ch`); class `+140h` (engine count) and `+144h` (no rotors); visual class `+160h`; the scene nodes | stream 1: 2 uniform draws, then `count` NextU32 draws per plane load (`007ECA23`, `007ECA41`, `007ECADD`); 1 per rotor Init (`007EC10F`); 1 per rotor Reset (`007E1CCF`) | OFF: none. ON: the visuals are invisible to the host. Every stream-1 consumer after a plane spawn shifts by `2 + count + rotors` draws, and by one per rotor on each full-health restore. |
| `engine_list_any_burning_007de5b0`, `engine_on_destroyed_007e23e0`, `engine_list_destroy_at_007eb0b0` | none. The host's death step `bsp::plane_death_step_007caf10` (`include/bsp/plane_death_modes.hpp:81`, called at `src/game_hosts_units.cpp:18038`) models no engines and no `"enginefire"` | the engine list (class `+140h` entries), message 101 (`"enginefire"`) | none here. `007BA170` picks the engine (index `% desc+7Ch`, segment 47); whether it draws was not read in this pass | OFF: none. ON, with the death step's engine block also bound: a dead plane raises `"enginefire"` until one engine is burning. The burning byte then stops the repeat, and one engine is stopped and its fire effect spawned. |
| `engine_sound_step_007de490` | none | the sound parameter rates and targets, which `007EB380` writes | none | presentation only |
| `damage_effect_switch_007e27b0` | none (`007C6820` is only partly hosted, `docs/TORPEDO_RELEASE_SPAWN.md`) | unit health (`00923BE0`), the class damage-effect list count `[unit+538h]+228h..22Ch`, dt | **stream 0**, `U(0, 1)` at `007E2862`, once per tick for 10 s after each change in a plane's damage | Bind it on a presentation stream (RNG rule). If bound on stream 0, every stream-0 consumer shifts from the first hit on a plane whose class has a damage-effect list. |
| `flight_controller_write_record_007d8330`, `flight_controller_apply_record_007db1f0` | none (the multiplayer control-state message; no host sends it) | the controller and the dyn hold | none | none in single player |
| `flight_controller_set_world_velocity_007d9ee0`, `flight_controller_reset_motion_007db2c0`, `plane_dyn_set_ground_contact_up_007d7d70` | the spawn seeding around `place_on_launch_spot_007c5f60` (`src/game_hosts_units.cpp:13696`) and `set_plane_forward_speed_007d9e80` (`:16380`) | the controller fields and `007C47F0` (the class level-flight speed) | none | OFF: none. ON: a spawned plane starts with zero body rates and its `dyn+CCh` = 1.05 x level speed. `dyn+CCh` has no host field; its reader was not traced. |
| `flight_controller_construct_007d7ea0` | none (the host's controller has no dying spin; docs/CC13_RNG_STREAM_CENSUS.md row `007D7F90`, NM) | GameTuning DeadMeat RotationMin/Max (`00F87330`/`00F87334`, `include/bsp/game_tuning_singleton.hpp:192`) | **stream 1**, three draws per plane at construction (`007D7F90`, then `0043996D` and `0043998D` in `00439950`) | OFF: none. ON: each plane spawn shifts every later stream-1 consumer by 3 draws, before the effects component's `2 + count + rotors`. The spin `ctl+F0h` reaches nothing in the host until a dying-spin reader is bound. |
| `flight_controller_update_speed_terms_007d8020` | the missing tail of `world_to_body_007d9c10` / `body_to_world_007d9c80` (`src/plane_angular_velocity.cpp:221`, `:228`) | the controller's world and body velocities and body angular velocity | none | OFF: none. ON: `ctl+6Ch`, `+70h`, `+74h` and `+A0h..A8h` get values. Their readers were not traced in this pass. |
| `control_surface_step_007de2e0` | none (`include/bsp/plane_advance_pose.hpp:138` models the drive values only) | `ControlSurfaceDrive` per surface, the node's `+ACh` | none | presentation only |
| `actuator_block_save_007deb70`, `actuator_block_load_007dea60`, `timed_flag_debounce_007eb4e0` | none (no save games; no effects component) | the actuator block; the flying predicate and GameTuning `+6C0h` | none | none in a mission run |
| `flight_forward_speed_007d99c0` | the dot products `v . forward` at `src/game_hosts_units.cpp:10575`, `:10656`, `:30002` and `ctl+44h` at `:17451` | `ctl+44h`; the deck parent's velocity in the plane's frame | none | OFF: none. ON: changes only planes parented to a moving carrier deck (take-off roll and deck taxi), whose forward speed gains 0.1 x the carrier's forward velocity (0.8 x when the carrier moves backward in the plane's frame). Every reader sees the change: the stall ratio, the control authority, the drag. |

## 3. For cc9 (no host edits made)

- **`src/plane_ai_control.cpp:294`, `plane_speed_factor_007d9a70`, is a stale copy.** It treats
  `00F8731C`/`00F87320` as never written (0.0), which docs/PLANE_CONTROL_AUTHORITY.md corrects:
  they are ControlRangeMin/Max of the tuning block. Nothing calls it. The live copy,
  `control_authority` at `src/game_hosts_units.cpp:33170`, reads the tuning, and that is the one the
  ledger record cites.
- **`include/bsp/torpedo_release_spawn.hpp:57`.** The channel's `+18h` is the float clock `[00F876A4]`.
  It is bit-copied with `MOVSS` and used as a float by `007DE7E0`, so `unsigned frame` is a representation
  choice only.
- **`src/plane_flight.cpp:604`, `max_sustainable_climb_angle_007d98f0`.** It keeps the image's bisection,
  with a substituted per-angle predicate (`007D9360` integrates ten 0.04 s steps). The ledger record
  marks it as a substitution.
- **`src/game_hosts_units.cpp:8173`, `plane_class_turn_rate_270_007db4d0`.** It returns the image's rate.
  Note that the clamped arm of the image (`e > 0.6`) also rewrites the class's TurnRollLeader `+260h` to
  `atan(0.6 * PitchSpd / c)`. That is a write to the shared class record, so every later call for that
  class (any plane of the class) starts from the new angle. The host comment records the omission. The
  first call already returns the clamped rate, and later calls see a different `sr`/`cr`.
- **`007D7DA0` and `007DB4D0` in the host** keep double intermediates where the image rounds every
  step to 24 bits (`docs/CC13_FPU_PRECISION.md`).
- **The forward speed `007D99C0`** has a carrier-deck term the host drops (section 1.5).

## 4. Ledger

Function records (`config/reconstruction`), models in `src/gameplay_seg48_cc13.cpp`:
- the actuator animation: `007DE7E0`, `007DE9A0`;
- the rotor: `007EBF10` (partial: node lookups are the caller's), `007E1C60`, `007DDEC0`, `007DDF10`,
  `007EC180`, `007EC1F0` (partial: the matrix writes), `007EC3B0`;
- the engine list: `007DE5B0`, `007DE540`, `007DE580`, `007DE5E0`, `007DE600`, `007DE630`;
- the base engine: `007E1C00`, `007E1BA0`, `007DDEB0`, `007DDF00`;
- engine destruction: `007E23E0` (partial: effect construction), `007EB0B0`;
- the engine sound: `007DE490`;
- the damage-effect switch: `007E27B0`;
- the flight controller: `007D8330`, `007DB1F0`, `007D9EE0` (partial), `007DB2C0`, `007D7D70`.

One fragment: `007ECA0E` (the speed spread in `007EC5A0`).

Records for existing reconstructions, checked against the listing, with no host edit:
- `007D81B0`, `007D83D0` and `007D92B0`, in `src/plane_flight.cpp`;
- `007D98F0`, a substitution;
- `007D9A70`, `control_authority` at `src/game_hosts_units.cpp:33170`;
- `007D9C10` and `007D9C80`, in `src/plane_angular_velocity.cpp` (partial: the tail `007D8020`);
- `007D9E80`, a substitution.

- batch 2: `007D8020`, `007D8180`, `007D7C00`, `007D7EA0` (partial: the dyn defaults), `007DE2E0` (partial:
  the matrix), `007DEB70`, `007DEA60`; the fragment `007EB4E0` (the flying flag in `007EB380`); and
  `007D80C0`, the existing `src/band_007a_cc13.cpp:128`, checked against the listing. Its record notes one
  difference: that model subtracts and divides in double.

- batch 3: `007D99C0` (model); and, checked against the listing, `007D7DA0`
  (`src/game_hosts_units.cpp:5727`), `007DB4D0` (`:8173`, partial: the `+260h` rewrite) and `007DB630` (the inline
  wire seed at `:14474`); `007EF8F0` and `007E1D80` (`src/band_007a_cc13.cpp:111`, `:149`).

Renames (`config/names`, applied to Ghidra with `tools/ghidra_annotate.py --apply`; the old evidence is kept
in each record, and the prior annotations are in the tool's local JSON):

| address | was | now |
| --- | --- | --- |
| `007DE9A0` | `BSP_ActuatorBlock_Step` | `BSP_ActuatorBlock_PushPartAnimFrame` (the step is `007DE3A0`) |
| `007DE7E0` | `BSP_ActuatorChannel_ComputeBlend` | `BSP_ActuatorChannel_AnimTime` |
| `007DE540` | `BSP_PlaneEngineList_StepAll` | `BSP_PlaneEngineList_SpinAll` (it calls slot `+14h`) |
| `007DE580` | `BSP_PlaneEngineList_UpdateAll` | `BSP_PlaneEngineList_BlurBlendAll` |

Evidence was appended to `007E27B0` (stream 0), `007D7EA0` (stream 1), `007D9B70` (`Turbo`), `007EBF10`
(stream 1) and `007E23E0` (the burning byte).

## 5. Verification

- `./scripts/build.ps1` passed in this tree: the build and ctest (2 of 2). No test was added.
- `tools/verify_report_calls.py local/gpr48_calls2.json` checked 31 call-site rows, with 0 failures. The
  rows cover:
  - the five stream draws (`007E2862`, `007E1CCF`, `007EC10F`, `007ECA23`, `007ECA41`) and the NextU32 at
    `007ECADD`;
  - the tail jump `007E1CE8`;
  - the interpolate, step-towards and truncation calls of the models;
  - the constructor's `007D7F90`/`007D7F9D`, the archive calls `007DEAC8`/`007DEBE6`/`007DEB3D`, and `007D8051`
    (sqrt).

  The row file is not committed.
- `tools/x87_dc_form_scan.py --range 007D7D70-007ECB20` finds 11 DC-form sites, in six functions. None
  is in a function modelled here. `007D9335` (in the checked host function `007D92B0`) is the `x - frndint(x)`
  step of the pow idiom.

## 6. Open and handoff

Not done in this packet, by priority:

1. **Host reconstructions in range still without a record**, because each needs a full listing check:
   - `007D8230` (`src/plane_advance_pose.cpp:308`) and `007D9F60` (`:334`);
   - `007D8470` (`src/plane_flight.cpp:436`);
   - `007DA710` (`src/plane_control_rate.cpp:40`/`:104`);
   - `007DB680`, the core law (`src/plane_flight.cpp:319`);
   - `007D9050`/`007D9140`/`007DA380` (inline in the hosts);
   - `007D9CE0` (the deck reparent).
2. **`007D9360`.** The climb-angle trial integration behind `007D98F0`: the host substitutes its predicate.
3. **`007EB380`, the effects-component step**, beyond the flying flag:
   - the power filter (`+34h`/`+38h`/`+3Ch`);
   - the start/stop state (`running`, `RPM`, `startStopTimer`);
   - the rotor rpm target passed to every engine's slot `+10h`;
   - the camera-relative sound part (doppler from `GGame+19FCh`).

   It is presentation with no RNG.
4. **`007EC5A0`'s other fields** (the Falling/Dying effects, sound loops) and the `007EB0E0`/`007EB160`/
   `007EB270`/`007EB350` effect helpers. Presentation.
5. **The squadron functions `007ECF00`-`007F3E50`**, left to cc9's squadron lanes (excluded as air ops and
   bot tasks). The gun-platform functions `007F6xxx` are cc9's gunnery.
6. **Infrastructure** (constructors, destructors, pod arrays, STL wrappers) between `007D5AC0` and
   `007F8280`. Their names stand and they have no records.

The scripts are in this tree's `local/gpr48_*.py`: the function list, the grep and definition finders, the
record writers and the doc edits. They are not committed.

## 7. gpr48b: listing verification of the host flight functions (cc13-gpr48b)

This was a verification pass over cc9's flight reconstructions, plus one new model. I made no host edits, no
wiring and no runs. Every check below is against the disk bytes (Capstone listing), with the x87 stack
tracked by hand. `tools/x87_dc_form_scan.py` finds DC-form sites in `007D9360` (`007D954D`, `007D9555`,
`007D9663`) and `007DB680` (`007DC2AE`, `007DC7E1`); each was read as `st(1) op= st(0)`.

### 7.1 Results

| function | host | verdict |
| --- | --- | --- |
| `007D8230` | `translate_only_007d8230` (src/plane_advance_pose.cpp:308) | **matches.** The order is `f(f(unit+810h*t) + f(pos + f(t*ctl+18h)))` |
| `007D9F60` | `rotate_and_translate_007d9f60` (:334) | **the model matches**, except for the attached clamp (D1). The running host differs (D2-D4) |
| `007D9050` | inline, src/game_hosts_units.cpp:17550-17580 | **partial.** The body matches. The boost arm and the call-site scale are missing (D5) |
| `007D9140` | inline, src/game_hosts_units.cpp:17587-17632 | **partial.** The formula and the two call-site ramps match. The body's rounding order was not re-read |
| `007DB680` | `accumulate_free_flight_007db680` (src/plane_flight.cpp:319) | **partial.** `007DB744-007DBBDF` matches (D6). The rest was not re-read |
| `007D9360` | not hosted (the host substitutes `007D98F0`'s predicate) | **modelled** in `gameplay_seg48_cc13.*` as `trial_integrate_at_climb_angle_007d9360` |

**Not done:** `007D8470` (3 KB), `007DA710`, `007DA380` and `007D9CE0`. Their host stand-ins are
`fold_world_into_body_007d8470` and `integrate_body_velocity_007d8611` (plane_flight.cpp:404-472),
src/plane_control_rate.cpp, and the inlines in game_hosts_units.cpp. None of them was re-read, and
they have no new records.

### 7.2 Divergences, with predicted effects

- **D1, `007DA2B1-007DA338`, the deck/runway clamp, is not modelled.** The host sets
  `attached_branch_unmodelled` (src/plane_advance_pose.cpp:388-392). What the image does:
  - the clamp runs only when ctl+FCh == 1, unit+BF4h is set and there is a parent (unit+3Ch);
  - it asks `006BC530` for the runway height h;
  - it computes `d = float(h - (desc+1FCh + 0.01))`, where 0.01 is the double `00D7A358`
    (`3F847AE140000000`);
  - when `0 > d` it lifts y by `-d` and calls `007D7D70` on ctl+10h.
  
  Effect: in the image, a plane on a deck or runway is held at desc+1FCh above the surface every
  step, and the ground-contact flag is set.
- **D2, `007DA080-007DA117`, the bank yaw, is missing from the running host.** The host
  (game_hosts_units.cpp:33203-33206) says "the bank-yaw ... is not carried". What the image does:
  - every wave-1 pose step yaws by `-sin(bank)*cos(pitch)*desc+1C8h*Interp(3.0, 0, 5.0, 1, ctl+7Ch)*difficulty*t`;
  - the result is committed by slot +0Ch, so it persists (docs/TICK_ELEMENT_OVERRIDES.md: wave 1 runs
    +4h with 0.05f, then commits).
  
  Effect: in the image, a banked aircraft whose ctl+7Ch is at least 5 gets an extra yaw rate in
  proportion to desc+1C8h and sin(bank). The term ramps in between 3 and 5. I did not establish its sign
  relative to the turn. Without it, the host's banked-flight heading drifts from the image's. A bound version should call `bank_yaw_angle_007da0ae` from the running path.
- **D3, the rotation argument form.** The image passes `w = omega*t` with the factor 1.0f (`FLD1` at
  `007D9FDC`). The running host passes `(omega, step)` (game_hosts_units.cpp:33270). The math is equal,
  but the rotation angle rounds differently (`|omega*t|` against `|omega|*t`): last-ulp drift only.
- **D4, the order within a step.** The image runs the +4h pose slot (`007D9F60`: rotate, bank yaw,
  level, translate with ctl+18h and unit+810h) in wave 1, before the +8h physics in wave 3. The running
  host integrates its velocity first and then moves the pose with the new velocity
  (game_hosts_units.cpp:18025-18056).
  
  Effect: the host's position and attitude run one fixed step (50 ms) ahead of the image for the same
  velocity history. Each step uses v(n+1) where the image uses v(n). That is about one step of travel,
  roughly 5-10 m at cruise.
- **D5, the thrust call site (`007DB7B9-007DB7FF`).**
  - The image scales `007D9050` by `unit+0CC8h` and then by `008E6430(6, unit)` (or 1.0f). The product
    order is `f(f(thr*CC8h)*mod)`.
  - The host takes both as 1.0 (game_hosts_units.cpp:17542-17549). It also skips the boost arm,
    ctl+5h x desc+604h (`007D9050`).
  
  Effect: where unit+CC8h is not 1 (an engine-damage or power scale) or modifier 6 applies, image thrust
  differs. Damaged planes keep full thrust in the host.
- **D6, the drag gate constant.** `007DBA67` compares the float speed squared with the **double**
  1e-10 (`00CE3820`). The host compares it with `1.0e-10f` (plane_flight.cpp:341). Those differ
  only for a speed squared in `(1e-10, 1.0000000134e-10]`. Effect: none in practice.

### 7.3 The `007D9360` model

`trial_step_forces_007d9360` / `trial_integrate_at_climb_angle_007d9360`
(include/bsp/gameplay_seg48_cc13.hpp). The header gives the per-step formula.

How it differs from the core law, which it otherwise mirrors:
- the thrust is `desc+164h` at full throttle;
- the angle-of-attack threshold is **1.0f** (`00D7A24C`), not 0.1;
- the drag is taken as `v * (1/|v|)` and gets no backward x5;
- there is no lift-ramp cap and no extra gravity.

The pose ops, `007D7C00`/`007D8470`, `007D9140`, `007D92B0` and `00419010` are host callbacks.

**Adoption.**
- It would replace the host's substituted predicate for the `007D98F0` bisection, which calls it as
  `(angle, probeSpeed, 0.04f, 10)`.
- It draws no RNG.
- OFF: nothing changes.
- ON: the max-climb angle comes from a 10-step x 0.04 s trial of the real force law, not the
  substitute.

**Status.** Build-tested (`./scripts/build.ps1`, CTest 2 of 2). RET 10h is confirmed (`C2 10 00` at
`007D9890`).

### 7.4 Ledger

| address | record | status |
| --- | --- | --- |
| `007D9360` | model | `semantic_build_tested`, complete |
| `007D8230` | pointer | `reconstructed_listing_checked` |
| `007D9F60` | pointer | partial |
| `007D9050` | pointer | partial |
| `007D9140` | pointer | partial |
| `007DB680` | pointer | partial |

## 8. ver1: byte verification of the rest of the flight step (cc13-ver1)

This is a verification pass, approved by cc9. I made no host edits, no wiring and no runs. Every check
below was made against the disk bytes (`bsp.py disasm-raw`, and `sc2_x87.py` for the long blocks).
The x87 stack was tracked by hand, and the pop forms were read from the opcode bytes
(`DE E1`, `DE E2`, `DE E9`, `DE FA`, `DE F9`). `tools/x87_dc_form_scan.py --range 007D8470-007DD000`
finds three DC-form sites in these targets:
- `007DA417`, in `007DA380`'s water arm, which is out of scope;
- `007DC2AE`, in the water arm of `007DB680`, out of scope;
- `007DC7E1`, the ctl+30h low-pass. It reads as `st(1) = st(1) * st(0)`, so each sum is multiplied by 4.0.

Gameplay runs at 24-bit precision. So the only rounding that can differ is association order, and a
double product that the image rounds twice where the host rounds once. I mark those cases "last ulp".

### 8.1 Results

| function | host | verdict |
| --- | --- | --- |
| `007D8470` | `fold_world_into_body_007d8470`, `integrate_body_velocity_007d8611`, `blend_direction_hold_007d88cb` (src/plane_flight.cpp:404-584), and the `kFlightIntegratorBound` inline (src/game_hosts_units.cpp:17858-18030) | **partial.** The arithmetic matches. Two blocks are missing (V1, V2) |
| `007DA710` | `plane_control_targets_007da710`, `plane_control_axis_step_007da710` (src/plane_control_rate.cpp), `control_step_007da710` (game_hosts_units.cpp:33356) | **partial.** The targets and the polynomial match. The crossing floor diverges (R1), and so does the use of BC4h (R2) |
| `007DA380` | inline in `control_step_007da710` | **matches** for modes 0 and 1. The water arm was not checked |
| `007D9CE0` | inline at game_hosts_units.cpp:14081 and :14495, translation only | **modelled** in `gameplay_seg48_cc13` as `reparent_velocity_007d9ce0`. The host differs (P1) |
| `007D9140` body | inline at game_hosts_units.cpp:17587-17632 | **formula matches**, with a different product association (last ulp) |
| `007DB680`, `007DBBDF` to the end | `accumulate_free_flight_007db680` (src/plane_flight.cpp:319-400), and the ground band at game_hosts_units.cpp:17660-17755 | **partial.** The roll-drag extra, the damping, the ceiling and the ground band match. Two blocks are missing (C1, C2) |

### 8.2 What matches

**`007D8470`:**
- **The folds:** dyn+28h into dyn+1Ch, and dyn+10h into dyn+04h, then the 0.001f deadband.
- **The per-axis step (`007D8611`-`007D874B`):**
  - `w = a*t + v`;
  - `q = |r40|` by the `-0.0f - x` idiom;
  - for `w >= 0`: `max(0, (min(b, 0) + w) - q*t)`;
  - for `w < 0`: `min(0, (max(b, 0) + w) + q*t)`;
  - every compare direction was checked, including equality.
- **The 0.01f deadband.** Equality keeps the value.
- **The hold blend (`007D88CB`-`007D8C5E`):**
  - every three-term sum is `(x2+y2)+z2`;
  - `1 - C0h` comes from `DE E1`;
  - the blend is `s*d + (1-s)*vhat`, then `(b*inv)*speed`.
- **The world rotation (`007D8C6B`):** `(vx*m0 + vy*m4) + vz*m8` through unit+74h.
- **The +C4h sink (`007D8CE9`-`007D8E1B`):**
  - world vy >= -0.001f (`00D05E10`);
  - the speed squared is gated against the double 1e-10;
  - the speed must exceed 3.0f (`00CE3854`);
  - `vy - sp*0.1`, where 0.1 is the double `00D7A3A0` (the float 0.1 widened);
  - then `00419510` (`v * (1/|v|)`), times the speed, the 0.01f deadband, and the rotation back.
- **The contact projection (`007D8E28`-`007D8F16`).** It is the host's projection with n = (0, 1, 0).
- **The acceleration:** `(v - dyn+70h) / t`.

The core law reseeds dyn+64h from ctl+3Ch every step (`007D9C10` then `007D7C00`). So the host
recomputing the body velocity from the world velocity is faithful.

**`007DA710`:**
- **The roll target:** `((RollSpd*f1)*bank)*roll`. In state 6, `bank = Interp(0, 1, 0.10471976, -0.1, |C68h|)`.
- **The pitch target:** `((PitchSpd*f1)*BB4h)`, times desc+1D8h when negative.
- **The yaw target:** `(YawSpd*BB0h)*BC4h*f2`, and the BC4h relax by `0.5*t` toward 1.
- **The slide:** `((1B8h*YawSpd)*sin(bank))*cos(pitch)`.
- **The accels.**
- **The polynomial:** `((A*d)*d + B*|d|) + C`, times |accel|. Pitch builds it as `(A*d)*d` (`007DABC4`-`007DABC8`), not `(d*d)*A`.
- **The sign step and the three-way clamp.**
- **The flat 0.6f floor.**

**`007DA380`, mode 1:**
- `r = max(YawSpd*RunwayYawTurnSpdMul, 0.87266/YawSpd)`;
- r is halved for a landed 10h/16h class;
- `blend = Interp(+2A8h, r, +2ACh, 1, v)`;
- a and b are taken over 3.6;
- the output is `c * blend`.

The image reads tuning +2A8h and +2ACh where the host reads one field. Both hold
`RunwayYawTurnSpdLimit/1` in this build (docs/GAME_TUNING_SINGLETON.md:541), so the values are equal.

**`007DB680`:**
- **The roll-drag extra (`007DBBE2`-`007DBC5C`):**
  - the gate is ctl+98h > 1.0f;
  - the extra is `(n_i*d)*(98h - 1)`;
  - the middle component is `d*0.0`.
- **The damping (`007DBD37`-`007DBE0A`):**
  - desc+174h applies to vx and +170h to vy;
  - the bound is `Interp(0.5, 1, 20, 100, unit+908h)`.
- **The ceiling (`007DBE2E`-`007DBEA2`).**
- **The ground band (`007DBEB3`-`007DC200`):**
  - the brake `1E0h * max(airbrake, 0.6 - thr*26)`;
  - the friction `Interp(StallSpd*F87374, F87370, StallSpd*F87378, 0, speed) * (clamp*surface)`;
  - the sum `(rf + brake) + dyn48`;
  - the wire, and `|2vx|`.

### 8.3 Divergences, with predicted effects

- **V1, the dyn+C8h speed floor, is absent (`007D87C7`-`007D87F1`).**
  - Host: game_hosts_units.cpp:17872 integrates with no floor. plane_flight.hpp:604 calls the floor "not in
    free flight", and game_hosts_units.cpp:7584 leaves `007D7A40` unmodelled.
  - Image: `if (dyn+C8h > 0 && dyn+CCh > vz) vz = dyn+CCh`.
    - The floor dyn+CCh is `float(LevelFlightSpeed * 1.05)` (`007DB2C0`), which is 1.05 x 1.8 x StallSpd,
      about 1.89 x StallSpd.
    - The timer is set to 1.2f (`00CE3814`) by `007D7A40`. `0099BF30` calls `007D7A40` whenever its band
      repair rewrites the pitch command (`0099C032`) or command[0] (`0099C0ED`) of a plane whose +72Ch
      predicate holds, which is free flight.
    - `007D81B0` counts the timer down.
  - Host: no floor.
  - Effect:
    - In the image, a bot plane whose command band is repaired cannot fly slower than about 1.89 x StallSpd
      for 1.2 s afterwards. The forward body speed is snapped up to that value in one step.
    - At cruise (usually above that speed) nothing changes.
    - It matters in slow flight: climbs, dive pull-outs, the terrain and ceiling bands, and slow
      approaches. In the image those planes cannot sit near the stall while their commands are being
      repaired. In the host they can.
- **V2, the third fold and its deadband, are absent (`007D84C2`-`007D84FF`, `007D85A8`-`007D85E7`).**
  - Host: plane_flight.cpp:436-470 folds two pairs only, and game_hosts_units.cpp:17872 passes
    `acc.body_resist_40` raw.
  - Image: `dyn+40h += R*dyn+34h`, and each component under 0.001f is zeroed.
  - Effect: dyn+34h is zero in modes 0 and 1, so only the deadband differs. A ground-band friction, brake
    or lateral term under 0.001 acts in the host and not in the image. That is under 5e-5 m/s per step:
    negligible.
- **V3, the sink rounding (`007D8D6D`-`007D8D75`).**
  - Host: game_hosts_units.cpp:17985 computes `b1 - sp*0.1` in double and rounds once.
  - Image: under 24-bit precision it rounds `sp*0.1` and then the difference.
  - Effect: last ulp, landing and takeoff rolls only.
- **V4, dyn+A4h is not carried.** `007D7D70` sets it (for example the deck clamp, D1). The image then
  zeroes vy at `|vz| < 1e-5` and projects onto the normal in the next integrate, even with dyn+94h = 0.
  The host keys both on `pen > 0`. The effect belongs with D1.
- **R1, the rate-law crossing floor (`007DAABD`-`007DAB36`, `007DAC08`-`007DAC40`, `007DAD90`-`007DADBC`, `007DAEC7`-`007DAF00`).**
  - Image. When 007DA380's byte is set (free flight) and the sign test passes:
    - pitch: `rate >= (|w|*1.5)*sp`;
    - yaw: `rate >= |w|*sp`, with no 1.5;
    - roll: no deflection floor at all;
    - here `sp = Interp(StallRangeMin 1.2, 1, StallRangeMax 1.6, 0, fwd/StallSpd) * Interp(StallOffPitch -15deg, 1, StallOnPitch 20deg, 0, pitch)`.
  - Host: plane_control_rate.cpp:67, called from game_hosts_units.cpp:33511, uses `rate >= 1.5*|w|` on all
    three axes.
  - Effect: in the image, sp is 0 above 1.6 x StallSpd or above a 20 deg pitch, so a cruising plane gets
    no crossing floor on any axis. Its stick reversals follow the polynomial alone. In the host every
    reversal is at least 1.5|w| per second on every axis.
    - That makes the host's reversals faster wherever `(0.5 d^2 + 1.5|d| + 0.1)*|accel| < 1.5|w|`:
      low-accel classes, and roll on every class.
    - So bots roll out of a bank and reverse yaw more briskly than the image. It shows in turn-reversal
      timing in dogfights and on the torpedo and dive approach lines.
    - Near the stall the host's yaw floor is 1.5/sp times the image's.
    - The size depends on the class accels, and I have not measured it.
- **R2, BC4h, the yaw gain.**
  - Host: game_hosts_units.cpp:33486 sets `target_yaw = -(yaw_raw*f2 - slide) * g` and builds the coupling
    from `yaw_raw` without g (plane_control_rate.cpp:150).
  - Image: `yaw_raw = (YawSpd*BB0h)*BC4h` (`007DA926`-`007DA94B`) feeds both
    `target_yaw = -(yaw_raw*f2 - slide)` and `roll += (yaw_raw*f1)*1B4h`.
  - Effect: only while the terrain arm holds g away from 1.
    - The host scales the slide term by g, which the image does not.
    - The host leaves g out of the yaw-to-roll coupling. With g = 2 the host's coordinated-turn roll is
      half the image's.
- **R3, the coupling association.** The image forms `(yaw_raw*f1)*1B4h`. The host
  (plane_control_rate.cpp:150) forms `(1B4h*f1)*yaw_raw`. Last ulp.
- **R4, the DeadMeat roll terms (`007DA7E4`-`007DA8D5`), are not modelled.** The host already labels this.
  The roll ramp is `Interp(0, 1, RollMulTime, RollMul, C3Ch)`, and the spin term uses C36h/C37h.
  Effect: crippled planes do not spiral down in the host.
- **C1, the wreck drag (`007DBC77`-`007DBD34`), is not hosted.**
  - Image: when ctl+94h is set and unit+C3Ch > 1.0f (XMM0 still holds the 1.0f from `007DBB8D`):
    `dyn+0Ch -= Interp(-8, 10, -1, 1, alt) * fwd * min(1, C3Ch - 1)`.
    - `00415510` (`BSP_Math_MinFloatByRef`) returns the smaller of its two floats.
    - It is reached only from the drag path, where the speed is above 0.001.
  - Host: absent. plane_flight.cpp:367 goes straight from the drag to the damping. The host also feeds
    ctl+94h only in water mode (game_hosts_units.cpp:17536).
  - Effect: in the image, a dying plane with ctl+94h set loses forward speed with a time constant of
    about 1 s in the air. Below 1 m depth the rate rises to ten times that. Wrecks stop sliding on the
    water. In the host they coast.
  - The in-air producer of ctl+94h, `007CE1D9` (inside the free-flight arm `007CE040`, a loop that
    compares a `004142E0` result with -1.0), is unread.
- **C2, the ctl+30h low-pass (`007DC743`-`007DC80B`), is not carried.** When `t < 0.25`, the image sets
  `ctl+30h = ((0.25 - t)*prev + t*v)*4`, which is `0.8*prev + 0.2*v` at 50 ms. The host has no field
  for it (game_hosts_units.cpp:16455). I did not trace who reads it.
- **P1, `007D9CE0`.**
  - Host: game_hosts_units.cpp:14081/14495 adds and subtracts the parent's velocity in world axes.
  - Image: it rotates ctl+18h and ctl+24h through the parent's matrices (old+CCh, then new+110h) and copies
    ctl+30h.
  - Effect: a parented plane's relative velocity rotates with a turning carrier in the image, and not in
    the host. Over a deck run of about 10 s on a carrier turning at 1-2 deg/s, the heading of the
    relative velocity differs by 10-20 deg. A non-turning parent gives the same result.
- **D-9140, rounding order (last ulp).**
  - Image: `((-sgn*v)*v) * ((K*coef)*B)` (`007D928D`-`007D9295`), with `coef = (Accel/Max)/Max` (`007C4990`
    `DE FA`, `DE F9`).
  - Host: `((((-sgn*v)*v)*K)*coef)*B` with `coef = Accel/(Max*Max)`.
  - The ground-band brake `0.6 - thr*26` rounds twice in the image (`007DBEBB`) and once in the host.

### 8.4 Models (`include/bsp/gameplay_seg48_cc13.hpp`)

These cover the pieces above that the host has no reconstruction for. None is wired.

| model | covers |
| --- | --- |
| `takeoff_speed_floor_007d87c7`, `takeoff_floor_speed_007db2c0`, `kRepairFloorSeconds_007d7a40` | V1 |
| `rate_law_floor_factor_007da710`, `rate_law_deflection_floor_007da710` | R1 |
| `dead_meat_forward_drag_007dbc77` | C1 |
| `velocity_lowpass_007dc762` | C2 |
| `reparent_velocity_007d9ce0` | P1, the whole function |

**Adoption.**
- R1 replaces the `floor_from_deflection` arm of `plane_control_axis_step_007da710`. It needs the forward
  speed, StallSpd, the pitch, and tuning +22Ch..+238h.
- V1 is a step in the integrate inline, after the 0.01 deadband. It also needs `007D7A40` at the host's
  band-repair site, game_hosts_units.cpp:7584.
- None of the models draws RNG.

**Status.** Build-tested (`./scripts/build.ps1`). Not game-run.

### 8.5 Ledger

| address | record | status |
| --- | --- | --- |
| `007D8470` | pointer `fold_world_into_body_007d8470` | partial, listing-checked |
| `007DA710` | pointer `plane_control_axis_step_007da710` | partial, listing-checked |
| `007DA380` | pointer, the inline | partial: modes 0 and 1 match, the water arm is unchecked |
| `007D9CE0` | model `reparent_velocity_007d9ce0` | `semantic_build_tested`, complete |
| `007D9140` | pointer, replaced, with gpr48b's evidence kept | partial: the formula matches, the order differs in the last ulp |
| `007DB680` | pointer, replaced, with gpr48b's evidence kept | partial: C1 and C2 |
| `007D87C7`, `007DAABD`, `007DBC77`, `007DC743` | fragments (the models) | `semantic_build_tested` |

### 8.6 Open

- **The in-air producer of ctl+94h, `007CE1D9`.** It decides whether C1 touches airborne wrecks.
- **The readers of ctl+30h (C2).**
- **The class accel magnitudes (desc+1BCh/1C0h/1C4h).** They are needed to size R1.
- **The water arms.** `007DA380` mode 2 and `007DB680` `007DC205`+ were excluded by the packet.

## 9. V1 and R1 bound in the host, OFF (cc9-physics6, 2026-10-13)

Packet cc9_plane_rate_floor. Two switches in `src/game_hosts_units.cpp`, both committed OFF, both using the 8.4
models.
- `kPlaneSpeedFloorBound` (V1):
  - Arming. `gunfire_repair_0099bf30`'s `finish()` arms dyn+C8h = 1.2f. It does so inside the changed arm of
    command[1] and command[0], and only in free flight (unit+900h == 7), as 0099C011-0099C032 and
    0099C0CC-0099C0ED do.
  - The other caller is 0099B1B0, a JMP to 007D7A40. It has no reference at all: no rel32, and the absolute
    dword `B0 B1 99 00` does not occur in the image.
  - Countdown and floor. In the integrate, after `integrate_body_velocity_007d8611`, 007D81B0's countdown runs
    first (subtract the step from a positive timer; park it at -1.0f outside free flight). Then 007D87C7 raises
    the body forward velocity to float(007C47F0() x 1.05).
  - SUBSTITUTION, labelled: dyn+CCh is taken from `bot_desired_speed_007c47f0` at each step rather than stored at
    spawn by 007DB2C0. Its inputs are per class.
- `kPlaneRateLawCrossingFloorBound` (R1): when 007DA380's byte is set, the axis step takes the overload in
  `src/plane_control_rate.cpp`:
  - pitch: (|w| x 1.5) x sp
  - yaw: |w| x sp
  - roll: no floor
  - sp: the 007DAABD-007DAB36 stall factor. Its inputs are the step's forward speed, StallSpd, unit+C64h and
    tuning +22Ch..+238h.
- Summary line: `summary mission plane rate floor`.

**Predictions, before the pairs** (the rows: USN12 `usn12_s38_u12_p2`, LOMP10 `p3_l10_b`, USN01 `usn01_p6_u1_g`,
which has carrier launches):
- OFF against the tree build: gameplay identical; the counts are 0, and the summary line shows bound=0/0.
- V1 ON:
  - `speed_floor_arms` > 0 on every row with bot planes under the gunfire-repair bands (attack runs into flak).
  - `speed_floor_raises` > 0 only where an armed plane is below about 1.89 x StallSpd: dive pull-outs, climbs and
    slow approaches.
  - Gameplay moves (verdict 3) on the plane-heavy rows. It may be identical where no plane flies slowly while
    repaired.
- R1 ON:
  - `rate_floor_steps` counts every flight-control step.
  - Roll reversals lose their floor, and pitch and yaw lose theirs above 1.6 x StallSpd or a 20 deg pitch.
  - So turn reversals get slower and every plane path moves: verdict 3 on every row.
  - Torpedo and dive approach lines change, so the knife-edge wins (USN01) may be lost.

**The pairs** (scratch exports of 9d9854241 with `local\p6_v1r1_apply.py`: OFF `local\p6_rf`, V1 only `p6_rfv`,
R1 only `p6_rfr`; reference env; logs `local\p6rf_{off,v,r}_{u12,l10,u1}.log`; OFF against the tree build on USN01:
gameplay identical):
- V1: USN12 arms 1509, raises 0, gameplay identical (1524.96 s); LOMP10 arms 39, raises 0, identical (1006.93 s);
  USN01 `usn01_p6_u1_g` arms 2693, raises 3408, gameplay moved, completed 1118.67 s both sides (83 deaths both).
- R1: rate_floor_steps on every flight step; USN12 moved, completed 1524.96 s; LOMP10 moved, completed 977.48 s
  (was 1006.93); USN01 g LOST (deaths 83 -> 57: ConTBD1's three torpedoes miss Convoy3, which ConTBD2 sinks first;
  Convoy1 and Convoy6 survive).
- As predicted on both. **Flipped ON, both** (cc9-physics6, 2026-10-13). USN01 is re-planned as
  `config/win_orders/usn01_p6_u1_k6.txt` (ConTBD1 at Convoy6 at frame 3306). cc13-ver17 independently found V1 to be
  the cause of USN04's Shoho/Zuiho/Yorktown climb-out crashes (planes under continuous band repair bleeding to
  17-19 m/s); the USN04 pair with V1 is open (handoff).

## 10. The rest of the plane-flight packet (cc9-physics7, 2026-10-13)

### 10.1 D2, the bank yaw (`kPlaneBankYawBound`, packet cc9_plane_bank_yaw)

- Image, 007DA00C-007DA117 inside 007D9F60 (re-read with `disasm-raw`): after the axis-angle rotation, whether or
  not it applied, the live pose is post-multiplied (00413920) by RotY (00B646E0) of
  `-(sin(unit+C68h) * cos(unit+C64h) * desc+1C8h * Interp(3.0 [00CE3854], 0, 5.0 [00CE3850], 1, ctl+7Ch) * d * t)`.
  - ctl+7Ch is the body forward speed (007D80C0 copies ctl+44h after each core step; SQUADRON_LAND_TASK 5da).
  - desc+1C8h is TurnRollSpd: 0.087266 or 0.10472 rad/s (5-6 deg/s) on this installation's plane rows
    (vehicleclasses.lua, mtime 2026-05-09).
  - d is 008E6430(0Dh, unit) when the byte [00E0C978] and [[00F88C30]+124h] are set, else 1.0f (00D7A24C). The
    host's modifier list is empty, so 1.0f (as for every other 008E6430 site).
- Host: `advance_pose_0085e4d0` calls `bank_yaw_angle_007da0ae` between `rotate_pose_0085e4d0` and the ground
  levelling, with C64h/C68h from `refresh_attitude_007c1900` and the step's forward speed (the value the image's
  next wave-1 pose reads). Summary line `summary mission plane bank yaw`.
- **Predictions, before the pair** (OFF: main ad68156b1, `local\p7_m`; ON: the same plus the switch, `local\p7_d2`;
  rows USN12, LOMP10, USN01 k6, JM08 f):
  - OFF: steps=0. ON: steps on every plane pose step; abs_angle grows with time spent banked.
  - A plane banked 60 deg gains about 5 deg/s of heading rate. Every banked turn changes radius, so every plane
    path moves: verdict 3 on all four rows.
  - The sign relative to the turn is not established. If it adds to the turn, attack approaches line up sooner; if
    it opposes, the pilot's yaw and roll loops must work harder and turn reversals stretch.
  - Wins: USN12 and LOMP10 should hold (ship-dominated). USN01 k6 is a knife-edge on torpedo geometry and may be
    lost. JM08 f has margin.

### 10.2 R2, where BC4h enters (`kPlaneYawGainRawBound`, packet cc9_plane_yaw_gain_raw)

- Image, 007DA926-007DA94B: `yaw_raw = (YawSpd * latched yaw) * unit+BC4h`, read before this step's relax
  (007DA967-007DA9E3). yaw_raw feeds `target = -(yaw_raw*f2 - slide)` and the coupling `roll += (yaw_raw*f1)*1B4h`.
- Host before: `targets.target[1] * g` after the rule, which scales the slide term by g and leaves g out of the
  coupling. ON: `state.yaw_scale_bc4 = g` before `plane_control_targets_007da710` (the rule already multiplies it
  in), and no post-multiply. The relax is unchanged.
- **Predictions, before the pair** (OFF `local\p7_m`, ON `local\p7_r2`; same rows as 10.1):
  - Only steps where g != 1 differ: g is raised by 0099BB00 (`max(g, 2(m - 0.4) sin|bank| + 1)`, the terrain arm)
    and relaxes back at 0.5/s. Planes near terrain while banked: few on the sea rows.
  - Where it acts, the coordinated-turn roll grows by g and the slide term no longer does.
  - Verdict 3 (moved) on rows with low terrain flying (LOMP10's island, USN12), possibly 1 on USN01 and JM08.
    Win times move little.

### 10.3 D4, pose before physics (`kPlanePoseFirstBound`, packet cc9_plane_pose_first)

- Image: wave 1 runs the +4h slot (007D9F60: rotate with ctl+24h, the bank yaw, the levelling with ctl+8Ch,
  translate with ctl+18h and unit+810h), then wave 3 runs the physics 007DB680 on the moved pose
  (docs/TICK_ELEMENT_OVERRIDES.md). Host before: physics first, then translate and rotate with the new velocities.
- ON: the core law starts with the pose step, using the world velocity, ctl+7Ch and the levelling mode the previous
  core law left; its own translation and the trailing `advance_pose_0085e4d0` are skipped. SUBSTITUTION, labelled:
  the step sits at the head of the core law, so the arm's attitude refresh, think and water sample still read the
  pre-move pose (one step stale against the image).
- **Predictions, before the pair** (OFF `local\p7_m`, ON `local\p7_d4` with D2 OFF; rows as 10.1):
  - steps = core law steps on ON, 0 on OFF.
  - Each plane's pose runs one 50 ms step behind the OFF host for the same velocity history (about 5-10 m at
    cruise), so every plane path moves a little: verdict 3 on all rows. Effects on wins are noise-sized (RNG
    coupling through the shared stream), except where a knife-edge sits on a torpedo drop point (USN01 k6).

**R2 pair** (OFF `local\p7_m` C277B01AE8BE, ON `local\p7_r2` C0B3DF5C46E6, reference env; logs `local\p7{w1,r2}_<row>.log`):
USN12 moved, 1534.87 s both; LOMP10 gameplay identical (1244.64 s); USN01 k6 moved, 1118.67 s both; JM08 f moved,
2318.84 -> 2147.67 s. Guard logged on all. As predicted (moves only where the terrain arm raised g; no win lost).

**D2 pair** (ON `local\p7_d2` D13F73EE6F6C; logs `local\p7d2_<row>.log`): verdict 3 on all four rows.
- USN12 1534.87 s both. LOMP10 1244.64 -> 1605.04 s. USN01 k6 LOST (no end). JM08 f LOST: failed at 1574.66 s,
  "The HQ has been destroyed!".
- Sign: the yaw goes with the bank. With the forward row on +Z and the right wing down, 007C1900's residual gives
  bank = -phi, so the angle is +sin(phi)*K and RotY turns the forward row toward +X (right). The result does not
  depend on 00B646E0's convention, because 007C1900 reads the bank through the same RotY.
- The pilot's own rotation shows it. On USN01 the heading change per rotation step falls from 0.0069 rad (3199 rad
  over 465495 steps) to 0.0036 rad (1400 rad over 386133 steps), plus 305 rad of bank yaw. The bank term supplies
  part of the coordinated-turn yaw that the control loop was producing alone.

### 10.4 D1 + V4, the deck/runway clamp (`kPlaneDeckClampBound`, packet cc9_plane_deck_clamp)

- Image, 007DA2B1-007DA338 (re-read with `disasm-raw`): when ctl+FCh == 1, unit+BF4h is set and unit+3Ch is
  non-null, the translated position goes through the parent's world matrix (00414D10 on parent+CCh). 006BC530
  (ECX = unit+BF4h) then returns h, the local height over the holder. `d = float(h - (desc+1FCh + 0.01))` (the double
  `00D7A358`). When `0 > d` the local y becomes `y - d`, and 007D7D70 runs on dyn. V4: with dyn+A4h set the next
  integrate zeroes body vy at `|vz| < 1e-5` and projects the world velocity off (0,1,0), then clears it.
- Host ON: at the end of `advance_pose_0085e4d0`, with `ground` and `plane_contact_deck_bf4`. The two contact terms
  key on `pen > 0 || a4`, and the projection clears a4. SUBSTITUTIONS, labelled: the parent is taken as present
  whenever the holder is, and the lift applies to world y.
- **Predictions, before the pair** (ON `local\p7_d1` with D2 OFF):
  - lifts > 0 only on rows with ground-law planes on a deck or runway (USN01's carriers, the JM08 airfield,
    LOMP10 if any plane lands). USN12 has none: gameplay identical, or moved only through RNG coupling.
  - lift_total small (centimetres per step): the host's contact projection already stops downward velocity once
    below the wheels, so the clamp mostly removes the residual sink.
  - Takeoff runs start from wheel height instead of slightly below it. Launch times move by a step or so; win
    times move little.

**D4 pair** (ON `local\p7_d4` 120CFB5B149E, D2 OFF; logs `local\p7d4_<row>.log`): verdict 3 on all four, as predicted.
USN12 1534.87 s both; LOMP10 1244.64 -> 1016.43 s; JM08 f 2318.84 -> 1926.10 s; USN01 k6 LOST (no end: 69 deaths
against 81; 36 attack orders against 54; plane steps 468576 -> 249886, with more landings, 9 against 5).
`pose first: steps` equals the core-law steps. No sign of a broken arm: takeoffs and landings occur, and the two
ship-heavy rows finish sooner.

**D1 pair** (ON `local\p7_d1` ED7F859DCA28, D2 OFF; logs `local\p7d1_<row>.log`): as predicted.
- USN12: lifts 0, gameplay identical (1534.87 s).
- LOMP10: lifts 45592 (2.34 m in all), moved, 1244.64 -> 1147.64 s.
- USN01 k6: lifts 926 (21.3 m), moved, 1118.67 s both.
- JM08 f: lifts 29970 (988 m, about 3.3 cm each), moved, 2318.84 -> 2118.19 s.
- No win lost. Guard logged on all.
