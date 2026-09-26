# The release path from a bot's drop decision to the rack (packet cc9_release_issue_stage)

2026-09-26. Ghidra read-only; names are ledger hypotheses. Switch `kReleaseIssueStageBound` in
`GameUnitsHost::Impl` (`src/game_hosts_units.cpp`), landed OFF first. It covers **torpedoes**.
The dive bomber's bombs keep their spawn at the request. The reason is under "Vals" below.

## The image's path

| step | where | what |
| --- | --- | --- |
| 1. the decision | the bot task (the torpedo task's aim/drop arm; the dive-bomb task's aimdive `009C60F1` and aimglide `009C5771`-`009C5784`) | calls `007BBBA0` once per round |
| 2. the request | `007BBBA0` `BSP_Unit_RequestOrdnanceRelease` | opens the bay (channel C of the block at unit+DECh) and `unit+C20h += 1`; spawns nothing (`docs/TORPEDO_RELEASE_SPAWN.md`) |
| 3. the stage | `007CE9FD`-`007CEB31` inside `007CE040`, every plane fixed step | the five guards: session mode `007CEA02`, unit+9E0h, unit+C3Ah (death), unit+5Dh, control mode unit+900h. Then the timer unit+C28h -= dt; below zero with unit+C20h > 0, it spends one request and calls `007C0D90` (`007CEA82`-`007CEA8D`), and reloads the timer with U(0.9, 1.1) (`00BD2F10`, the shared stream) × BombDelay `class+1F4h`. With no request left and unit+C25h set, the cleanup arm and its rack walk (`docs/PLANE_DEVICE_WALK.md`) |
| 4. the issue | `007C0D90` `BSP_Plane_TickReleaseOrderIssue` | walks the plane's children; for the first `IsKindOf(25h)` rack that holds ordnance 2Ah (`vtable[210h](2Ah, 0)`) and is not busy (`vtable[1FCh]`), calls its fire slot `vtable[1F0h](delay)` = `006E3550`: `toRepeatTime += delay; dropBombs = 1`. A level bomber (`IsKindOf(10h)`) dropping plain bombs goes on to the next rack with `delay += U(...) × class+1F4h` (`007C0E67`-`007C0E9A`); anything else stops. Then unit+C25h = 1 (`007C0EE2`) and, unless the ordnance is a rocket, `007EEF30` |
| 5. the rack tick | `006E56F0`, `__thiscall(rack+310h)(float dt)` | the base gun tick `0072D130`; with a live owner (+5Ch set, +5Dh, +60h, +5Eh clear), `toRepeatTime -= dt` while above -1.0; with `dropBombs`, dt > 0 and `toRepeatTime` < 0: `CanFire(1)` (`006E3460` = `BSP_Gun_CanFire` `00729A80` and ordnance 2Ah) |
| 6. the attitude gate | `007CC8E0` (owner is a plane) | a level bomber (10h): the bay test, then \|bank C68h\| and \|pitch C64h\| <= LevelBombAngleMax `tuning+550h` (20°). Anything else: `007C7600(slot byte)`: the bay test; a rocket with rounds left passes; pitch > DiveBombPitchAngleMax `+568h` (90°) refuses; pitch < DiveBombPitchAngleMin `+564h` (60°) passes; in between, \|bank\| against a curve over `+554h`..`+560h` (`00419010`) |
| 7. the drop | `006E588B`-`006E58A5` | the fire message 0ADh (`006E3940`) through `BSP_Session_RouteMessage` `0077C2A0` to `BSP_Gun_HandleNetworkMessage` `0072D860`, which calls the rack's `vtable[1DCh]` = `006E4C10` -> `BSP_Gun_FireIfReady` `00727E30` -> `CanFire(1)` and `vtable[1D8h]` = `006E4D50`, the drop that makes the round (its body places the round at an offset `toRepeatTime` × the rack's velocity at `006E54B7`). Then `toRepeatTime` = descriptor `+E0h` (`006E58AA`) |
| 8. the end of the drop | `006E58CF`-`006E58E1` | when `CanFire(1)` fails and the rack's +3B8h is set or its ammo +484h <= 0: `dropBombs = 0`. The cleanup of step 3 then finds no busy rack and clears unit+C25h |

The rack's fields are named by its serializer `006E4A60`: +484h `ammo`, +488h `orgAmmo`, +494h
`toRepeatTime`, +498h `dropBombs` (strings at `00CF9B7C`, `00CF9B74`, `00CF9B64`, `00CF9B58`).

**Both a Kate's torpedo and a Val's bomb take this one path.** Each call to `007BBBA0` is one
request, and each request is one issue. A Val's two-round aimglide salvo is therefore two
issues, separated by the stage's reloaded interval, U(0.9, 1.1) × BombDelay. The dive attitude
gate of step 6 also applies to each of them. A Kate's torpedo leaves one or two fixed steps
(0.05-0.1 s) after its request, because pitch below 60° passes the gate at once.

## Correction to docs/PLANE_DEVICE_WALK.md

That packet said `dropBombs` is cleared only by the constructor. It is also cleared by the rack
tick, at `006E5880` (a non-plane owner's drop) and `006E58E1` (the ammo is spent). Both stores
address it as `[ESI+188h]` with ESI = rack+310h, which a byte scan for disp32 498h cannot see
(the sub-object trap). "Busy" therefore means "still dropping". A torpedo bomber's cleanup is
held off only until the tick after its drop, not until a re-equip. The host's walk now answers
`dropping`.

## The host's two paths, before this packet

- **The request spawns.** `release_ordnance_007bbba0` (torpedo) and `release_bomb_007bbba0`
  (bomb) call the gunnery host's spawn at the request itself. This is a labelled substitution
  from the time the rack was unread. A dead aircraft's request is refused there
  (`dead release refused`).
- **The stage runs, but finds nothing.** The release-issue stage (`007CE9FD`, reconstructed)
  also runs. Its issue's device check read the torpedo ordnance bit, which the spawn had already
  cleared. So `007C0D90` never armed a rack and unit+C25h was never raised. On E2 9000, 5
  torpedo requests gave 1 spawn: the other 4 came from dead Kates and were refused at the
  request.

## The binding (torpedoes)

- **The request.** With the switch ON, the torpedo request no longer spawns. It is counted,
  `rack_requests_deferred`, and the image's stage decides.
- **The issue.** The device check becomes "the aircraft still holds its torpedo, its single
  rack has ammo, and it is not dropping". The census and the ammo come from `BSPGun` and
  `DeviceClass Type` (`docs/PLANE_DEVICE_WALK.md`); the ammo is one round per single rack. The
  fire slot sets `dropping`.
- **The rack tick** runs after the stage in the same fixed step: `run_rack_tick_006e56f0`. It
  does the countdown, the ammo test, the pitch half of `007C7600` and the spawn, and clears
  `dropping` once the ammo is spent.
- **The walk** answers `dropping`.

**Substitutions and records** (labelled in the code):
- `CanFire` `00729A80` for a rack is "has ammo": `Rack::can_fire_00729a80`.
- The roll curve of `007C7600` is the record `Plane::drop_roll_curve_007c76b1`, and the drop is
  allowed there. A torpedo run never reaches it: pitch is below 60°.
- The level bomber's gate in `007CC8E0` is not bound; no level bomber flies in these missions.
- The bay-door test is not bound: the per-slot byte unit+9C3h does not exist in this host.
- The fire message route is collapsed into a direct spawn, the host's stand-in for `006E4D50`.
- `toRepeatTime`'s reload from descriptor `+E0h` is 0: `Rack::repeat_time_descriptor_e0`.
- The order of the rack's tick against the plane's within one fixed step is not read.

**Vals, not bound here.** Moving the bomb spawn onto the rack means carrying the dive task's
release-tick aim point and fall time (`db_run_in_origin`, `db_impact_fall_time`) to the rack's
tick. Those are what the gunnery host scores a bomb against. The Val's rack census (single or
multiple racks, rounds per rack) has not been printed either. The next part should take both,
with the stage's interval draw from the shared stream.

## Bodies with no Ghidra function

`ghidra proto --brief` shows none for each. INT3 padding sits on both sides unless noted, and
each is referenced only from the rack vtables `00CF96A8` and `00CF9918`.

| start | inclusive end | evidence |
| --- | --- | --- |
| `006E3460` | `006E3488` | rack `CanFire`, slot `+1D0h` (`00CF9878`, `00CF9AE8`); `RET 4` at 006E3486; INT3 006E345B-006E345F and from 006E3489 |
| `006E4C10` | `006E4D29` | rack `FireIfReady`, slot `+1DCh` (`00CF9884`, `00CF9AF4`); `RET` at 006E4D29; INT3 006E4C06-006E4C0F and from 006E4D2A |
| `006E4D50` | `006E56EF` | rack drop, slot `+1D8h` (`00CF9880`, `00CF9AF0`); `RET 0Ch` (`C2 0C 00`) at 006E56ED; INT3 006E4D4C-006E4D4F before; the next routine `006E56F0` begins at once |

## Predictions (written before the runs)

The pair is `local\rs_off` against `local\rs_on`, built from main `1cdc219ca` plus this packet.
It uses the switch only, streams on, and one run at a time.

| row | E2 9000 (OFF: DW_ON_9000's values) | USN04 4700/4500 |
| --- | --- | --- |
| torpedo spawns (`torpedo_drop drops`) | 1 on both sides; ON one or two fixed steps (0.05-0.1 s) later than OFF, from Kate #4.1\|.-4 | 1 on both |
| the four dead Kates' requests | OFF: refused at the request (4 `dead release refused` torpedo lines). ON: no such line; each request stays in unit+C20h behind the death guard | same |
| `dead_releases_refused` | 8 -> 4 (only the bomb refusals remain) | same pattern |
| rack armed / dropped / walks | ON: `rack ... drops=1 ammo=0 dropping=0` for #4.1\|.-4, `deferred=1` on it and on the four dead Kates; one cleanup walk, not busy (`walks=1 busy=0`), unit+C25h 0 at the end | same |
| new natives | `Rack::tick_006e56f0` concrete (1); `Plane::device_busy_1fc` concrete (1); records `Plane::release_spawn_deferred_to_rack` (5), `Rack::can_fire_00729a80` (1), `Rack::repeat_time_descriptor_e0` (1); `Plane::drop_roll_curve_007c76b1` absent | same |
| the one torpedo's run | its trace line (Fletcher-class05 at 28.6 m, hull part 1, flood 1) moves by a few metres; the flood may flip | same |
| dive releases, Val deaths | identical: bombs are not bound | identical |
| plane deaths / hit records | 51 ± 2 / 836 ± 40 | 41 ± 2 / 727 ± 40 |
| the Lexington's distance moved | 5731.91 ± 100 | 3454.40 ± 50 |
| identical rows | everything before #4.1\|.-4's request | same |
