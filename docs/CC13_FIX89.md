# CC13_FIX89: the follow sight search and the kamikaze boost chain

Worker cc13-fix89 resolved the two UNRESOLVED rows U1 and U2 of `docs/CC13_VERIFY15.md` for the cc9
gameplay lead on 2026-10-17. Both are REAL mismatches on a normal path. Each is now hosted behind a
new switch in `src/game_hosts_units.cpp` ("GHU"), and both switches are OFF. With a switch OFF the
host runs exactly as before. "Image" means battlestationspacific.exe, read with `bsp.py disasm-raw`
and `bsp.py ghidra bytes`. Script values are from this installation's modded
`scripts/datatables/autoload/vehicleclasses.lua`. No game runs were made.

| Item | Verdict | Switch | Live rows |
| --- | --- | --- | --- |
| U1 follow sight search `009C211D`-`009C232A` | REAL | `kFollowSightSearchBound` | follow-state wingmen whose flight leader fires (JM08 likely, USN01 possible) |
| U2 boost request `plan+2E4h` and thrust x `desc+604h` | REAL | `kKamikazeBoostTurboBound` | Ohka kamikazes (USNOS-type rows) |

## U1: the follow sight search

### What the image does

The search is the tail of the follow tick `009C1FD0`. It runs after `009BEE30` (the command step) and
the `+85h` trail arm. The release arm `009C1FFD`-`009C2006` returns before it.

1. **Countdown** (`009C211D`-`009C213B`, `009C22BA`). While `dt` is below `+74h` (`FCOMI`/`JB`, so an
   unordered compare also takes this arm), `+74h -= dt` and the search is skipped. Otherwise
   `+74h = (+70h - dt) + +74h` and the search runs. `+70h` is `0.6f` (`00CE3D30`, stored by the
   constructor `009C2980` at `009C2A05`). The constructor sets `+74h = -U(0, 0.6)` through `00BD2F10`
   (`009C2A0A`).
2. **Gate** (`009C213E`-`009C2167`). The search runs only when `+85h` is set and either `+84h` is
   already set or `007B8EE0(squadron+3D0h)` is true. `+85h` is `009BFD70`'s in-position latch for this
   tick. `007B8EE0` is `MOV AL,[ECX+C40h]`: the flight leader is in the firing-guns list `00F87278`
   (`007C74A0` sets the byte and `007C75A0` clears it). If the gate fails, `+84h` becomes 0.
3. **Walk** (`009C216D`-`009C22E8`).
   - It sets `+78h = +7Ch = 0`.
   - It takes the leader's inverse pose (`00414E10` on `squadron+3D0h`).
   - It walks the side's recon contact list `[008053C0(unit+54h)]+DE8h`. For each entity that answers
     `IsKindOf(5)` (a unit), it transforms the position `+FCh` into the leader's frame.
   - It keeps the entity when `z > 1.0f` (`00D7A24C`, `COMISS`/`JBE`) and
     `z < gun+34h * 1.2000000476837158` (`00CEC160` is the double of `1.2f`).
   - Then `0042B2F0` takes `|(x/z, y/z, 0)|`, and the entity must also have that length below
     `gun+38h * 1.5` (`00CE3D78`).
   - The smallest such length wins, and its `z` is stored at `+80h`.
   - `gun` is `[task+1Ch]`, the task's gun controller that `009FC7C0` ticks. Its `+34h` is ShootDistance
     and its `+38h` is the aim-angle cap (docs/DOGFIGHT_GUN.md).
4. **Latch** (`009C22F5`-`009C230B`). `+84h` = found. When found, `unit+914h = 1.0f`. The plane
   constructor writes `-1.0f` there (`007CFDDF`), and nothing resets it.
5. **Hold timer** (`009C2313`-`009C232A`, every tick). `gun+4Ch` = `+84h ? 2.0f (00CE3958) : 0.0f`.
   The register `XMM2` is zeroed at `009C2083` on every path.

### Who reads the result

- The hold arm `009BF0FD`-`009BF295` takes `+78h..+80h` through the leader's pose and then the
  member's inverse. When the point is inside its cone, it steers the wingman at that point, which lies
  `z` ahead on the leader's boresight. Otherwise it clears `+84h` at `009BF295`. The host had already
  reconstructed this arm in `src/plane_follow_hold.cpp` but fed it `sight_active_84 = false`.
- `009FC7C0` reads the gun hold timer `+4Ch`. While it is positive, the fire envelope passes
  unconditionally for any finder target in range, and the fine-aim steer is skipped
  (`src/dogfight_task.cpp:286`, GHU `df_gun_tick_009fc7c0`). In other words, wingmen fire when their
  leader's sight is on a contact.
- `0072C128` (shot team) and `0072C5FE` (in `0072C170`) read `unit+914h > 0.0f` and substitute the
  plane's squadron head. Those readers are in the gunnery code and are not hosted. The field is stored
  only.

### Live rows and RNG

The search needs a wingman in a follow state (dive-bomb done, torpedo prepare or done-prepare, level-bomb
prepare, depth-charge done, drop-kamikaze follow, moveto follow), in position, while its flight leader
fires. In the cc9 logs read (`cc9-lua52/local/l65rt_j8p1_ref.log`, JM08), the follow hold arm ran
142740 ticks, and task-gun bursts came from bomber leaders (`B-25 #3.1`, `SBD #3.4|.-II`). In
`cc9-gunnery36/local/g51_i1_u1.log` (USN01), the hold arm ran 17670 ticks and there were 7 task-gun
bursts. Whether a leader fired while its wingmen were latched was not measured, so JM08 is "likely"
and USN01 "possible".

In the image, every extra wingman burst draws `00BD2F10` in the burst clock (`009FCED1`). The host's
burst clock uses midpoints and makes no draw, so the switch adds no RNG draws in the host. The
constructor's draw at `009C2A0A` is not hosted for these follow states; see below.

### What the host does with `kFollowSightSearchBound`

| Piece | Where |
| --- | --- |
| switch and evidence comment | next to `kFollowTurboBound` |
| `follow_sight_tick_009c211d` (countdown, gate, walk, latch, `unit+914h`, hold timer) | before `follow_trail_arm_009c207c` |
| call for the shared follow tick, behind the release test | `run_follow_tick_009c1fd0`, after the trail arm |
| call for the moveto follow tick, replacing its record-only countdown | `run_moveto_follow_tick_009c1fd0` |
| hold-arm inputs `sight_active_84` and `sight_local`, and the `009BF295` clear | `run_follow_hold_arm_009bee56` |
| the think's elapsed time as the tick's `dt` (`fs_think_dt`) | the pilot think, next to `elapsed` |
| counters, printed as `summary mission follow sight searches= found= held_ticks=` | follow-law summary |

Slot fields: `fs_sight_84`, `fs_sight_78[3]`, `fs_countdown_74`, `fs_think_dt`,
`plane_formation_sight_914` (initial `-1.0f`).

Labelled substitutions:

- **Recon contact list.** The host uses every live slot of another party. Recon levels are not
  hosted; docs/AA_TARGETING.md uses the same stand-in.
- **Leader's `+C40h`.** The host uses the leader's gunFire byte `plane_gun_fire_bc9`, which is already
  its stand-in for the firing list (`gunfire_detect_0099ec40`, CC13_VERIFY15 U3).
- **Gun fields.** `gun+34h` is `df_row.aim_shoot_distance` (850) and `gun+38h` is the controller's
  `0.09` default, as `df_gun_tick_009fc7c0` takes them.
- **Initial countdown.** For the states that go through `run_follow_tick_009c1fd0`, `+74h` starts
  at 0.0 instead of `-U(0, 0.6)`, so the first tick searches. The moveto follow keeps its existing
  drawn stagger.
- **`009BED80`'s `+84h` clear.** The clear on each re-entry is not hosted. The latch is cleared at the
  next reload whose gate fails.
- **Precision.** The leader-pose product for the hold arm is taken in double, as `follow_to_body`
  does. The search's x87 chains are modelled at 24-bit precision (one float rounding per operation);
  a 53-bit thread could differ in the last bit.

Not part of this packet: the tail's `plan+2E5h` clear (`009C232F`-`009C2347`, under `+85h` and
`plan+268h` bit 0).

**Expected pair result (ON vs OFF).** On a row where a follow-state leader fires, expect:

- non-zero `follow sight searches/found/held_ticks`;
- more task-gun `bursts` and `fire_ticks` from wingmen, and fewer `steer_ticks`;
- small changes in the wingmen's attitude while the sight is held.

On rows without leader fire in follow, the only change is the hold timer written as 0.0 each follow
tick. That timer was already at or below 0 in the host, so those rows are expected to be identical.

## U2: the boost request and the plane turbo

### What the image does

- **Request.** `plan+2E4h` has two writers: the kamikaze states `009AC74C` (gotowards, non-rocket,
  turnaround) and `009ACD66` (aim, inside TurboRange after the 3 s lock). `0099B56C` clears it in
  every think. `0099BF18`-`0099BF1E` copy it into `cmd+14h` (`unit+A10h`).
- **Throttle arm `007BB79E`-`007BB834`** in `007BB6E0`, at the commit:
  - `IsKindOf(17h)` (MPlaneKamikaze) with `unit+C24h` (PilotFires) clear:
    - request and `E84h > 0`: `unit+9F8h = 1`;
    - else, when `E84h <= 0`: `9F8h = 0`;
    - else `9F8h` keeps its value.
    - The throttle `+9F0h` is then `9F8h ? 1.0f : 0.0f` (`007BB816`, or `007BB81E` with `XMM0 = 0`
      from `007BB7A8`). So this plane has no thrust before its boost or after its charge is spent.
  - Any other plane with a request: `9F8h = (E84h > 0 && !E88h)`, and the throttle is forced to `1.0f`.
  - No request: `9F8h = 0`, `E88h = 0`, and the throttle takes the usual quantisation.
- **Latch.** `007B978F` copies `9F8h` into `+BC8h`.
- **Thrust.** `007DC86B` copies `9F8h` into `ctl+5h`. `007D9077`-`007D9081` multiply the thrust by
  `desc+604h` TurboStrength. The water law clears `ctl+5h` (`007DCDEA`), and the ground-roll law stores
  `CL = 0` there (`007DCD31`).
- **Class and charge.** The class reader stores TurboTime, TurboRechargingTime and TurboStrength
  (`007D3CD0`-`007D3D50`; defaults 0, 0, 1). It zeroes TurboTime unless TurboStrength > 1
  (`007D3DB5`). `BSP_Plane_ReadPropertyBag` sets `E84h = TurboTime > 0 ? 1.0f : 0.0f` and `E88h = 0`
  (`007D5D59`-`007D5D76`).
- **Charge step** `007CEB31`-`007CEC28`, in the fixed step `007CE040` (`ESI = unit+310h`):
  - with TurboTime > 0 and `+BC8h` set: `E84h -= step / TurboTime`; at `<= 0`, `E84h = 0` and `E88h = 1`;
  - with `+BC8h` clear and the plane not the rocket-like kind: `E84h += step / TurboRechargingTime *
    f`, capped at 1.0. `f` is `008E6430(9, unit)` under `[00E0C978]` and `[[00F88C30]+F4h]`, else
    1.0.

The only direct-displacement writers of `+E84h`/`+E88h` are `007D5D20` and `007BB834`. The drain and
recharge address them as `unit+310h` + `B74h`/`B78h`.

### Live rows

In this installation, the MXY7 Ohka (`VehicleClass[156]`, Type Kamikaze) authors TurboStrength 3.5,
TurboTime 10 and TurboControlLimit 0.4. The plain kamikaze rows (A6M, D3A, Ki-43, D4Y) author no
turbo.

The cc9-ships44 USNOS logs (`s55_*`, `s56_*`) show 7 to 17 kamikaze planes per run (`Betty*_kz*`,
dropped Ohkas) and 334 to 1033 boost ticks. In those logs the host flies each Ohka at Accel x full
throttle throughout. The image instead glides it, gives it 3.5 x Accel for 10 s from the first boost,
and then glides it again.

For non-rocket kamikazes the forced 1.0 throttle duplicates their throttle slots, which is MINOR.

No `00BD2F10` draw is on this path.

### What the host does with `kKamikazeBoostTurboBound`

| Piece | Where |
| --- | --- |
| switch and evidence comment | next to `follow_turbo_steps` |
| class fields and the property-bag reset | the class read, after `BombDelay` |
| the throttle arm, replacing the analogue throttle quantisation only when ON | the commit `007BB920` block |
| the `+BC8h` latch and the charge step | `control_step_007da710`, after the throttle latch |
| `a *= TurboStrength` while `9F8h`, not on water or ground | the thrust block, after the turbo multiplier |
| `summary mission boost turbo steps=` | after the kamikaze summary |

Slot fields: `plane_boost_9f8`, `plane_boost_latched_bc8`, `plane_boost_charge_e84`,
`plane_boost_lockout_e88`, `plane_turbo_time`, `plane_turbo_recharging_time`, `plane_turbo_strength`.

Labelled substitutions:

- The charge step runs in `control_step_007da710` after the latch. Its order against
  `007CE040`'s other elements was not read.
- `008E6430(9)` is taken as 1.0, as `008E6430(6)` and `unit+CC8h` already are in the thrust.
- 24-bit precision is assumed.

Not done:

- **TurboControlLimit** (`desc+608h`) is read at `007C0FDE` inside `007C0F40`. That function belongs
  to cc9-physics16's control laws.
- **`007DB27B`** stores `[EDI+24h]` into `ctl+5h`. What that is was not read.

**Expected pair result (ON vs OFF).**

- **USNOS-type Ohka rows:**
  - `summary mission boost turbo steps` becomes non-zero;
  - Ohka speeds drop before the boost and after the charge ends, and rise sharply during it;
  - impact timing and hit counts move.
- **Rows without kamikazes:** identical, except that a plane with authored turbo now carries a charge
  that nothing consumes.
