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

## 5. Verification

- `./scripts/build.ps1` passed in this tree: the build and ctest (2 of 2). No test was added.
- `tools/verify_report_calls.py local/gpr48_calls.json` checked 24 call-site rows, with 0 failures. The
  rows cover:
  - the five stream draws (`007E2862`, `007E1CCF`, `007EC10F`, `007ECA23`, `007ECA41`) and the NextU32 at
    `007ECADD`;
  - the tail jump `007E1CE8`;
  - the interpolate, step-towards and truncation calls of the models.

  The row file is not committed.
- `tools/x87_dc_form_scan.py --range 007D7D70-007ECB20` finds 11 DC-form sites, in six functions. None
  is in a function modelled here. `007D9335` (in the checked host function `007D92B0`) is the `x - frndint(x)`
  step of the pow idiom.
