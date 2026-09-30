# The plane wanderer (packet `cc9_plane_wanderer`)

Addresses: 007BE060, 007C4560, 007D8230, 00439950

The wanderer makes formation members drift a little around their stations. It replaces ranking
#14 (the follow trail arm, stage-only: docs/PILOT_MOVETO_TASK.md "The follow trail arm").
Worker cc9-lua20, 2026-09-29. The switch `kPlaneWandererBound` (`src/game_hosts_units.cpp`) is
committed **OFF**.

## 1. The object: `plane+810h`

**Construction.** `007C4560 BSP_PlaneWanderer_Construct` (`007C4560`-`007C47E0`,
`__thiscall(wanderer, plane)`, `RET 4`) is called at `007CFDC6` in the plane constructor.

| offset | contents | written by |
| --- | --- | --- |
| `+0h..+8h` | `offset`: the term `007D8230` adds to the translation | the step |
| `+0Ch..+14h` | `speed`, clamped to `SpeedMax` | the step |
| `+18h..+20h` | `accel`, clamped to `AccelMax` | the step |
| `+24h` | the gust timer: U(0.5, 1.5) at construction (`007C4616`, stream 1) | the step |
| `+28h` / `+2Ch` | roll and roll target | the step |
| `+30h` | `WandererMul`, 1.0; `plane+840h` | the trail arm `009C207C`; `SquadronSetWandererMul`; reset to 1.0 by every step |
| `+34h` | `WandererEnabled`, 1; `plane+844h` | the trail arm; `SquadronSetWandererEnabled`; the enter/exit of land/final, turndown, aimdive and flyabove (docs/PILOT_MOVETO_TASK.md) |
| `+38h` | the plane | the constructor |

**The tuning.** The constructor copies the `Wanderer/*` tuning into statics. It also stores
`[00E0AFFC]` = 15.0 (`00CE5380`) and `[00E0B000]` = 45.0 (`00CE3D60`). This installation's
`scripts/datatables/planeglobals.lua` (mtime 2024-10-29) sets:

| tuning | offset | static | value |
| --- | --- | --- | --- |
| SpeedRange | `+1C0h`/`+1C4h` | `[00F87268]`/`[00F8726C]` | KMH(60), KMH(100) |
| RollChangeChance | `+1C8h` | `[00E0B004]` | 0.28 |
| RollChangeMax | `+1CCh` | `[00F87274]` | 0.5 |
| RollChangeDecay | `+1D0h` | `[00F87270]` | 0.4 |
| RollChangeSpeed | `+1D4h` | `[00E0B008]` | 0.7 |
| TimeRange | `+1D8h`/`+1DCh` | `[00E0B00C]`/`[00E0B010]` | 1.0, 3.2 |
| OffsetMax | `+1E0h` | `[00E0B014]` | 3.5 |
| ChangeMul | `+1E4h` | `[00E0B018]` | 0.7 |
| AccelMax | `+1E8h` | `[00E0B01C]` | 0.7 |
| SpeedMax | `+1ECh` | `[00E0B024]` | 0.8 |
| AccelDecayTime | `+1F0h` | `[00E0B020]` = AccelMax / it | 5.0 |
| SpeedDecayTime | `+1F4h` | `[00E0B028]` = SpeedMax / it | 7.0 |
| OffsetDecayTime | `+1F8h` | `[00E0B02C]` = OffsetMax / it | 8.0 |
| SmallPlaneDecalMul | `+1FCh` | `[00E0B030]` | 1.2 |
| SmallPlaneRollDecayMul | `+200h` | `[00E0B040]` | 1.8 |
| SmallPlaneAccelMul | `+204h` | `[00E0B034]` | 1.4 |
| SmallPlaneTimeMul | `+208h` | `[00E0B038]` | 0.7 |
| SmallPlaneOffsetMul | `+20Ch` | `[00E0B03C]` | 0.8 |

The file's Hungarian comments describe `OffsetMax` as the maximum deviation, and `OffsetDecayTime`
as the time in which the offset vector dies away "and we return to the original position".

## 2. The call: `007CE0D2`

`007CE040` (the plane fixed step, ECX = `unit+310h`) calls `007BE060(unit+810h, step)` at `007CE0D2`.
The call is reached only in these conditions:
- after the unit tick `00953CC0`;
- only when `unit+520h` is clear (`007CE09C`, `JNE 007CE55B`; the `007CE55B` path has no other
  call, per a rel32 census that finds the single caller);
- only when `game+1FE4h == 0` (`007CE0C0`).

## 3. The step: `007BE060` (`007BE060`-`007BE9A0`, `RET 4`)

The step was read from the listing, `local\l20_wlist.txt` in the cc9-lua20 tree. The stack frame
is `SUB ESP,34h`, then `PUSH ESI`, `PUSH EDI`, so `[ESP+40h]` is dt. Every x87 value that stays on
the FPU stack across a branch was traced by hand.

**Multiplayer arm** (`game+1FE4h != 0`, `007BE074`-`007BE0B4`):
- with `+34h` set, it clears `+34h`, zeroes `offset` and zeroes `+28h`;
- it returns in either case.

**The gate** (`007BE0B7`-`007BE0FF`):
- `len = |accel|` (`0042B2F0`).
- Active when (`+34h`, or `plane+520h` and `plane+9D8h`) and `(plane+72Ch)->vtable[38h]` (the
  free-flight gate) answers true.

**Active arm:**
1. **The timer.** `timer -= dt` (`007BE105`-`007BE114`). Below zero (`007BE11D`):
   - The multipliers `am` and `tm` are 1.0 for a large plane (`0047B850`: kinds 10h/16h), else
     `SmallPlaneAccelMul` and `SmallPlaneTimeMul`.
   - `timer = U1(TimeRange1, TimeRange2) x tm` (`007BE186`).
   - `d = 00439950() - offset / OffsetMax`. When `|d| > 1.5` (`00CE380C`), `d` is scaled to 1.5
     (`00CE3D78`).
   - `accel += d x ChangeMul x am`, then clamped to `AccelMax` (`0042AE50`).
   - **The roll kick:**
     - `u = U1(0, 1)` and `c = +30h x RollChangeChance`;
     - `w = c > u ? interp(0 -> 1.0, c -> 0.2; u) : interp(c -> 0.2, 0.8 -> 0; u)` (`00419010`,
       `00CE54A0` 0.2, `00CE74F8` 0.8);
     - when `w > 0`: `+2Ch += RollChangeMax x w x s`. The sign `s` is +1 when
       `_ftol2(timer x 9999.0)` (`00D05A88`, `00BF7420` CVTTSD2SI) is odd, else -1.
2. **Accel decay** (`007BE38F`-`007BE49D`):
   - `|accel| < 0.001` (`00CF3F30`): `accel = 0`;
   - else `accel -= accel/|accel| x min(AccelMax/AccelDecayTime x dt, |accel|)`.
3. **Speed** (`007BE4A0`-`007BE54F`): `speed += accel x dt x +30h`, clamped to `SpeedMax`.
4. **The fade** (`007BE551`-`007BE5F3`):
   - `f = interp(SR1 -> 0, SR2 -> 1; plane speed vtable[38h]) x interp(15 -> 0, 45 -> 1;
     plane+9B4h)`, the height above ground;
   - for a small plane, `f /= SmallPlaneDecalMul`.
   - `g = 1.5 / (f + 0.5)` when `+34h` is set, else 3.0 (`00D7A280`, `00CE3D78`, `00D7A2B0`).
5. **Speed decay** (`007BE618`-`007BE6EB`): as step 2, with `SpeedMax/SpeedDecayTime x dt x +30h x g`.
6. **Offset** (`007BE6EE`-`007BE741`): `offset += speed x f x dt`.
7. **Offset decay** (`007BE744`-`007BE8B3`):
   - `L = |offset|`, divided by `SmallPlaneOffsetMul` for a small plane.
   - Below 0.001: `offset = 0`.
   - Else `offset -= offset/L x min(interp(0.7 OffsetMax -> 1, 3 OffsetMax -> 5; L) x g x
     OffsetMax/OffsetDecayTime, L) x dt` (`00CEFFA0` 0.7, `00D7A2B0` 3.0, `00CE3850` 5.0).
8. **Reset and roll** (`007BE8B6`-`007BE994`):
   - `+30h = 1.0`;
   - `00927F30(plane, 1)` (the local player's role 1) clears `+2Ch`;
   - `+28h += (+2Ch - +28h) x dt x RollChangeSpeed` when they differ;
   - `+2Ch` decays toward 0 by `RollChangeDecay x dt`, x `SmallPlaneRollDecayMul` for a small
     plane.

**Inactive arm** (`007BE3CE`):
- `|accel| < 0.1` (`00D7A3A0`): `offset = 0`, `+28h = 0`, and return (`+30h` is not reset).
- Otherwise steps 2-8 run: the drift dies away.

**`00439950`** (`00439950`-, stream 1 twice) returns a uniform direction: `z = U(-1, 1)`,
`phi = U(0, 2pi)` (`00CE3D9C`), `r = sqrt(1 - z^2)`, `(r cos phi, r sin phi, z)`.

## 4. The consumer: `007D8230`

`007D8230` (`007D8278`-`007D8318`) publishes the translation as `committed + t x (unit+810h..818h
+ ctl+18h..20h)`. The commit `006D1FC0` copies `74h` back to `674h` (docs/PLANE_ADVANCE_POSE.md),
so the `offset` acts as an **extra velocity** in world axes.

With `SpeedMax` 0.8 and the fades, `offset` stays within about `OffsetMax` (3.5), so a member
drifts at up to about 3.5 m/s, and the follow law (`009BFEE0`/`009BEE30`) pulls it back.

The roll `+28h` has no reader found in this packet; its consumer is unread.

## 5. The binding

- **`src/plane_wanderer.cpp`**: `plane_wanderer_construct_007c4560`,
  `plane_wanderer_fixed_step_007be060` and `plane_wanderer_random_direction_00439950`. Coverage:
  complete for the single-player arm.
- **`PlaneFlightHost::wanderer_fixed_step_007be060`**: called in `run_plane_fixed_step_007ce040`
  inside the `unit+520h`-clear block.
- **The host override** (`src/game_hosts_units.cpp`):
  - fills the tuning from `plane_globals()`;
  - the gate inputs are `landing_airborne`, `park_bomber_class_0047b850`, `|plane_world_velocity|`
    and the height `y - avoid_surface_height` (the host's `unit+9B4h`, as elsewhere);
  - draws with key `<plane>#wander` through `release_altitude_draw_00bd2f10`. That is the shared
    stream stand-in, keyed per plane under `BSP_GUNNERY_RNG_STREAMS=1`.
- **`wanderer_publish_007d8230`**: adds `offset x step` to the position where the host integrates
  the velocity.

LABELLED:
- The construction draw is taken at the first step.
- The `+844h` writers of the four states are observed as state transitions at the step.
- The `+520h`/`+9D8h` arm and `00927F30(1)` read false, because the call is reached only with
  `unit+520h` clear.
- There is no sub-frame publish (`t` is the fixed step).
- x87 extended precision is not reproduced.

The census lines:
- `summary plane wanderer planes=... steps=... active=... timer_draws=... off_transitions=...
  offset_max=... speed_max=... accel_max=... displacement=...`;
- on both sides, `summary follow station error n=... mean=... max=...` (the 009BFD70 distance each
  follow tick).

## 6. Predictions, written before any ON run

The pairs are USN04 3000, E2 (USN04 9000), JM05 9000, JM08 3000 and LOMP10 3000: OFF against
`--flip kPlaneWandererBound=true`.

- **Mechanism, every plane row:**
  - `planes` > 0;
  - `accel_max` <= 0.7 and `speed_max` <= 0.8 (the clamps);
  - `offset_max` <= about 3.5 (OffsetMax);
  - `timer_draws` about `active steps x step / 1.5` (the mean gust period for small planes is
    2.1 x 0.7 = 1.47 s);
  - `displacement` > 0.
- **Station error:**
  - the follow `mean` rises by at most a few metres (the drift is a few m/s, and the follow law
    corrects it);
  - `max` may rise more.
- **Gameplay:** every row with aircraft in flight moves (exit 3). The positions change, and so do
  every later distance, sighting and AA solution. The per-plane draws are keyed under the pair env,
  so the coupling is physical, not through a shared stream. The lead asked for judgement on
  mechanism, not death counts, and so will the verdict.
- **LOMP10** moves too (its B-25s fly), unless no plane is airborne in the window.
