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
with the stage's interval draw from the shared stream. Taken by the next packet: see "Vals (packet cc9_release_issue_stage_vals)" below.

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

## The pair, measured

The logs are `local\RS_OFF_9000.log` / `RS_ON_9000.log` and `RS_OFF_4500.log` / `RS_ON_4500.log`,
in worktree cc9-circle-steer. All show the 1600x900 line and a module directory in this tree.
Both OFF logs match the device-walk runs' values.

| row | E2 9000 OFF -> ON | USN04 4500 OFF -> ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| torpedo spawns | 1 -> 1 (#4.1\|.-4, both at about 130 s) | 1 -> 1 | 1, one or two fixed steps later | held: ON drops at 73.1 m/s against 73.2 and its run is 6.80 s against 6.90 s |
| the four dead Kates' requests | refused at the request -> held in unit+C20h behind the death guard (`deferred=1 drops=0`) | same | as predicted | held |
| `dead_releases_refused` | 8 -> 4 | 8 -> 4 | 8 -> 4 | held |
| rack | `deferred=1 drops=1 ammo=0 dropping=0` on #4.1\|.-4 | same | same | held |
| cleanup walk / unit+C25h at the end | walks 0, C25h 1 on #4.1\|.-4 | same | one walk, not busy, C25h 0 | **failed**: #4.1\|.-4 dies before its cleanup interval runs out, and the stage is guard-blocked from then on (6392 ticks), as the image's death guard does |
| new natives | `Rack::tick_006e56f0` concrete 1; records `Plane::release_spawn_deferred_to_rack` 5, `Rack::can_fire_00729a80` 2, `Rack::repeat_time_descriptor_e0` 1; no `drop_roll_curve` | same | same, with `can_fire` 1 | **failed** on `can_fire`: 2. The second tick, with the ammo spent, reaches CanFire and clears `dropping` (006E58E1) |
| the torpedo's run | Fletcher-class05 at 28.6 m -> 27.6 m; hull part 1 and flood 1 on both | same | a few metres | held |
| plane deaths | 51 -> 51, the same victims; 18 death times move, the first at 151.9 s, after the drop | 41 -> 41, the same victims; 10 move | ± 2 | held |
| hit records | 836 -> 843 | 727 -> 743 | ± 40 | held |
| dive releases | 3 -> 3 | 3 -> 3 | identical | held |
| the Lexington's distance moved | 5731.91 -> 5721.98 | 3454.40 -> 3454.40 | ± 100 / ± 50 | held |

**Verdict: `kReleaseIssueStageBound` ON.**
- The torpedo now comes out of the rack the image uses, through the stage and its guards.
- A dead aircraft's request is refused where the image refuses it: by the stage's death guard,
  holding the request in unit+C20h, not by a host check at the request.
- The rack tick's `dropBombs` goes up and down as read.
- The behaviour rows hold within their bands, and every moved death is later than the drop.
- **Open:** the Val's bombs, the rack `CanFire` gates, the roll curve, the level bomber's gate,
  the descriptor's repeat time, and the fire-message route are records or stand-ins (listed
  above).

## Vals (packet cc9_release_issue_stage_vals)

2026-09-26. Ghidra read-only. Switch `kReleaseIssueStageValsBound` in `GameUnitsHost::Impl`,
committed OFF with the predictions below.

### What the image does with a Val's bomb

- **No aim point reaches the rack.** The drop `006E4D50` has a target-point block
  (`006E541F`-`006E5515`): with rack+4F8h set and the round of kind 31h, it calls `007AB230` on
  the round with rack+4FCh..+504h plus two draws, advanced by `toRepeatTime` × rack+508h..+510h.
  The only byte store to +4F8h other than the drop's own clear (`006E55FE`) is `006E3F94`, in
  the setter `006E3F90` (`__thiscall(rack)(point*, velocity*)`, `RET 8`), which stores both
  vectors. Its one caller is `007C0EC9` in `007C0D90`, on the branch taken only after the
  owner answers `IsKindOf(10h)` (`007C0E1C`, a level bomber) and the ordnance is not 2Ch, 2Bh
  or 33h. Its arguments are built by `007BCCF0` on the unit and the unit's `vtable[34h]`; their
  roles were not read. A D3A is not kind 10h,
  so its rack never has a target point and its bomb is **ballistic from the drop**. The byte
  scan was `C6 ?? F8 04 00 00` (two hits). A store through the rack+310h sub-object would be
  `+1E8h` and is not covered by that scan.
- **The drop's own state.** `006E4D50` takes the rack's first child that answers
  `IsKindOf(2Ah)` (`006E4EC0`-`006E4ED6`). It sets its orientation from two angles at
  rack+400h/+404h, drawn from the shared stream `00BD2F10` (`006E4F91`, `006E4FB1`) unless the
  first argument supplies them. It scatters the release velocity at rack+4ECh with two more
  draws (`006E513C`, `006E51AC`), and adds rack+DCh..+E4h scaled by `[rack+3F4h]+DCh` to the
  round (`006E52B4`-`006E5301`).
  Then it hands the round to the world (`009555A0`) and, with ammo left (+484h > 0), reloads
  through `vtable[224h]`, else clears +4F8h.
- **The census.** This installation's D3A Val (`VehicleClass[158]`, `vehicleclasses.lua`) has
  one equipment. Its platform 50 mounts device class 87 with `Ammo = 1`. Class 87 in
  `classtables/realistic/deviceclasses.lua` is `"Bomb platform 500kg JP"`, `Type` `BombPlatform`
  (a single rack, 25h), `RepeatTime` 0.05. Platform 50 has `UseBayDoor = true` and no
  `MainPlatform` key. The rack's round count `006E3500` is `vtable[21Ch](2Ah)` + ammo, and its
  rearm `006E3410` sets ammo to orgAmmo − 1 when a round is already loaded. So a Val holds
  **one** bomb in this installation. The path from the equipment's `Ammo` to the setter
  `006E3530` was not read, so that one link is an assumption.
- **The attitude gate never holds a diving Val.** The rack tick passes its platform's byte
  +0Eh (`006E5837`-`006E585F`: class+94h entry, `movzx edx, byte [eax+0Eh]`). The parser names
  +0Dh `UseBayDoor` (`0096127F`, key string `00D1AB80`) and +0Eh `MainPlatform` (key
  `00D1AB70`, default 0 at `009612B3`). With 0, `007C7600` skips its bay test. Pitch unit+C64h
  is negative nose-down (`docs/PLANE_FLIGHT_CORE_LAW.md`), so every dive attitude is below
  DiveBombPitchAngleMin (60°) and passes at `007C76AB`. The gate refuses only nose-up: above
  90° always, and between 60° and 90° when |bank| exceeds the curve.
- **The roll curve** (`007C76B1`-`007C7734`): limit = `00419010`(x0 = +560h
  RollAngleMaxPitch, y0 = +558h RollAngleMax, x1 = +55Ch RollAngleMinPitch, y1 = +554h
  RollAngleMin, pitch); `00419010` is `RET 14h`, so `007C772A` reads the |bank| double stored
  before the call, and `007C7734 JBE` passes |bank| <= limit. This installation's
  `planeglobals.lua`: 90°, 90°, −80°, 60°, so the limit runs from 84.7° at 60° pitch to 90°.
- **Timing.** Each `007BBBA0` call is one request. The stage spends one per interval,
  U(0.9, 1.1) × BombDelay (0.4 s on the D3A), into `007C0D90`. The issue fires the rack with
  delay 0 (the next-rack delay `007C0E67`-`007C0E9A` is the level bomber's). The rack tick drops
  on its first tick with `toRepeatTime` < 0, which is the same fixed step if it runs after the
  stage. A second request finds no rack holding 2Ah (the child has gone and ammo is 0), so it
  issues nothing: **a Val has no second round in this installation.** The host's aimglide
  still requests two, because its carried-rounds stock is the substitute 2
  (`kDiveBombCarriedRoundsSubstitute`, `007C1DB0`); ON, the second request is spent on nothing.
- **A Val that never reaches the pitch window.** There is no window to reach. The window is an
  upper bound on nose-up pitch, so a Val's drop follows its issue by at most one rack tick in
  any dive or pull-out attitude. What stops a drop is death: a request made after death is
  held in unit+C20h behind the stage's guard `007CEA1C`, and a drop pending at death is held
  by the rack tick's live-owner test (`006E5718`-`006E5748`).

### The binding

- **The request** (`release_bomb_007bbba0`): with the switch ON and a census single rack, each
  round raises unit+C20h through `007BBC00` and is counted `deferred`; nothing spawns there,
  and the host's dead-release refusal is not applied (the stage's guard is). An aircraft with
  only multi racks keeps the request spawn: `006E4360` (never busy) is not bound.
- **The issue's device check** accepts a general bomb (2Ah) as well as a torpedo, with the same
  rack ammo and `dropping` tests. `007C1F60`'s per-unit rounds and `007B9140`'s "holds 2Ah"
  count a bomb carrier's rack rounds.
- **The rack tick** drops a bomb through `run_rack_bomb_drop_006e4d50`: the host's bomb spawn,
  with the predicted impact point and fall time computed by `009C7D71` from the aircraft's
  state at the drop tick. The gunnery host uses those two only for its scoring census.
- **The roll curve** is bound (above), under the same switch.

**Substitutions and records** (labelled in the code):
- `Rack::drop_dispersion_006e4f91`: the four scatter draws of `006E4D50` are not taken.
- The stage's interval draw keeps the host's low end 0.9 (the torpedo binding's convention),
  not a draw from the shared stream.
- One round per single rack, as for torpedoes. It agrees with this installation's D3A `Ammo`.
- `kDiveBombCarriedRoundsSubstitute` (2) is unchanged. It is the next gap: the image's
  `007C1DB0` would latch 1 for a D3A, and the aimglide would then request one round.
- Unchanged from the torpedo packet: `CanFire` as "has ammo", the level bomber's gate, the bay
  test (not reached for a Val: `MainPlatform` is 0), the fire-message route and the descriptor
  repeat time.

### Predictions (written before the runs)

The pairs are `local\rv_off` against `local\rv_on`, built from this branch with the switch
only, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`, one run at a time. The previous pairs
(`RS_*`) show 3 dive releases on both missions, all made by Vals that were already dead
(#1.1|.-4 two rounds, #1.1|.-2 and #5.1|.-2 one each), and 0 bombs spawned.

| row | USN04 9200/9000 | USN04 4700/4500 |
| --- | --- | --- |
| Val requests (`val bomb request` lines) and their times | identical: 3 requests, 4 rounds, same instants | same |
| bombs spawned (`bombs_spawned`, gunnery `bomb_drops`) | 0 -> 0 | 0 -> 0 |
| `dead_releases_refused` | 4 -> 0; the three `dead release refused ... bombs=` lines go | same |
| the three dead Vals' rack lines | OFF `deferred=0 C20h=0`; ON `deferred=2/1/1`, `C20h=2/1/1`, `drops=0`, `issues=0` | same |
| rack census | every D3A `single=1 multi=0` on both sides | same |
| rack drops, gate refusals, roll curve | 0, 0, and `Plane::drop_roll_curve_007c76b1` absent on both | same |
| new natives | ON: `Plane::release_spawn_deferred_to_rack` +4 over OFF; `Rack::drop_dispersion_006e4f91` absent | same |
| second-round timing | not observable: no live Val releases on either mission | same |
| plane deaths, hit records, torpedo rows, the Lexington's movement | identical, the same victims at the same times | identical |
| identical rows | everything except the lines above; the follow state's release arm (`BotStateFollow::release_arm`, reads C20h) stays as OFF unless a dead Val is still in follow | same |

A failed prediction of "identical" would mean something reads unit+C20h on a dead Val.

### The pairs, measured

Logs in worktree cc9-plane-release: `local\RV_OFF_9000.log` / `RV_ON_9000.log` and
`RV_OFF_4500.log` / `RV_ON_4500.log`. All four show the 1600x900 line and a module directory in
that tree. The OFF runs use the binary of `dadc05b84`. The first ON run of 4500
(`RV_ON1_4500.log`, same commit) raised unit+C20h twice per round; the fix is `a1de87bda`, and
both ON logs above are from it. Apart from pointers, the harness slot lines and the ignored
`ship avoidance search refills` counter, each pair differs in exactly the lines in this table.

| row | USN04 9000 OFF -> ON | USN04 4500 OFF -> ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| Val requests and times | identical: #1.1\|.-4 2 rounds at 128.10 s, #1.1\|.-2 1 at 129.60 s, #5.1\|.-2 1 at 205.66 s, all nose-down (pitch −0.91, −0.85, −0.76 rad) | same | identical | held |
| bombs spawned, gunnery `bomb_drops` | 0 -> 0 | 0 -> 0 | 0 -> 0 | held |
| `dead_releases_refused` and the three refusal lines | 4 -> 0, the lines go | same | same | held |
| the three dead Vals' rack lines | `deferred` 0 -> 2/1/1; `C20h` 2/1/1 on both sides; `issues=0`, `drops=0` | same | OFF C20h 0 | **failed** for OFF C20h: the dive task's request hook already raises C20h per call through the torpedo binding. The first ON run doubled it to 4/2/2; fixed in `a1de87bda` |
| rack census | 19 D3A `single=1 multi=0` on both | same | same | held |
| rack drops, gate refusals, roll curve | 0, 0, absent | same | same | held |
| natives | `release_spawn_deferred_to_rack` 5 -> 9; `Plane::issue_block_c3a` 326734 -> 326731 (the request-side refusal's three calls go); no `drop_dispersion` | 5 -> 9; 183224 -> 183221 | +4; the c3a move not predicted | held; the c3a move is the refusal leaving the request |
| deaths, hit records, death table, torpedo rows, the Lexington's line | identical | identical | identical | held |
| second-round timing | not observable | same | not observable | no live Val releases on either mission, nor in any recent USN01 log |

**Verdict: `kReleaseIssueStageValsBound` ON.** The dead Vals' requests are now refused where the
image refuses them, by the stage's death guard. Every behaviour row is identical. **Not
run-time evidenced:** the live Val's issue, rack drop, roll curve and the one-round consequence.
No mission in this tree currently has a live Val release. The first mission that does is the
test, and it should show one bomb per Val where OFF spawned two from a two-round glide.

## Carried rounds (packet cc9_dive_bomb_carried_rounds)

2026-09-26. Switch `kDiveBombCarriedRoundsBound`, committed OFF with the predictions below.

**What the image reads.** `007C1DB0` `BSP_Unit_CountRemainingOrdnanceRounds` walks unit+48h and
sums `006E3500` over every child answering `IsKindOf(25h)`. `006E3500` is `vtable[21Ch](2Ah)`,
the loaded round, plus ammo +484h. Its callers are the aimglide enter `009C4F00` and tick
`009C5180`, the HUD's unit rows `00648C20`, `00609BD0` and `009A3290`. For a D3A the value is 1
until the rack drops its round and 0 after. The spend at the request (`approach+2Ch`, the host's
`spend_round`) is the task's own latched count, not this one.

**The authored count.** This installation's `scripts/datatables/autoload/vehicleclasses.lua`
(modified 2026-05-09 21:52) gives `VehicleClass[158]` "D3A Val" one equipment,
`Equipments[1][50] = { Ammo = 1, Platform = 87, ReloadTime = 60 }`, and `DefaultEquipment = 1`.
Platform 50 mounts device class 87, a single `BombPlatform`. The equipment reader is `00961F57`
(`docs/VEHICLE_CLASS_FIELDS.md`). The step from the equipment's `Ammo` to the rack's setter
`006E3530` was not read, so it is an assumption.

**The binding.** ON, the census adds each single rack's authored `Ammo` (through its platform
key `p<n>_key`) into `rack_rounds_authored`. `dive_bomb_rounds_remaining` returns it until the
issue's first check. After that it returns the rack's ammo, which only the rack drop spends. The
issue's first check seeds the rack's ammo with the same number, and `007C1F60`'s rounds and
`007B9140`'s holds-2Ah read it too. An aircraft with no single rack or no authored `Ammo` keeps
the substitute 2. **Assumption:** the aircraft carries its `DefaultEquipment`.

The glide's travel seed does not move: max((rounds − 1) × 0.07 × approach+A4h, 5.0) is 5.0 for
both 1 and 2 rounds.

### Predictions (written before the runs)

Same-tree pairs `local\cr_off` against `local\cr_on`, the switch only, streams and death table
on, one run at a time. On main, the only Val releases on both missions come from three dead Vals.
#1.1|.-4 made a two-round glide at 128.10 s. #1.1|.-2 (129.60 s) and #5.1|.-2 (205.66 s) each
made a one-round aimdive release.

| row | USN04 9200/9000 | USN04 4700/4500 |
| --- | --- | --- |
| census | every D3A `authored=1` on both sides (the line prints in both builds) | same |
| #1.1\|.-4's glide at 128.10 s | `rounds=2` -> `rounds=1`: the second glide request goes; `deferred` 2 -> 1, `C20h` 2 -> 1 at the first release | same |
| #1.1\|.-4 afterwards | OFF its count reaches 0 and approach+D1h drops, so the task goes to `done` (18 ticks). ON its count stays 1, because a dead aircraft's rack never drops, so D1h stays set: its state counts move, and it may request again | same |
| #1.1\|.-2 and #5.1\|.-2 | identical: one-round aimdive requests at the same times | same |
| `Plane::release_spawn_deferred_to_rack` | 9 -> 8, plus any new request by #1.1\|.-4 | same |
| live Val rows, bombs spawned | identical; 0 bombs on both | same |
| deaths | the same victims; times identical, unless #1.1\|.-4's own flight moves | same |
| hit records | ± 40 | ± 40 |
| the Lexington's movement line | identical | identical |

### The pairs, measured

Logs in worktree cc9-plane-release: `local\CR_OFF_9000.log` / `CR_ON_9000.log` and
`CR_OFF_4500.log` / `CR_ON_4500.log`, all from the binary of `37d74858f` with the switch only.
All four show the 1600x900 line and a module directory in that tree. Apart from pointers, the
harness slot lines and the ignored refill counter, both pairs differ in the same lines, all of
them #1.1|.-4's or the census they sum into.

| row | 9000 and 4500 (identical pattern) | prediction | verdict |
| --- | --- | --- | --- |
| census | 19 D3A `authored=1` on both sides | same | held |
| #1.1\|.-4's glide at 128.10 s | `rounds=2` -> `rounds=1`; `deferred` 2 -> 1; `C20h` 2 -> 1; `007BBBA0` calls 12 -> 11; the second release census line goes | the second request goes | held |
| #1.1\|.-4 afterwards | OFF `aimglide -> done` at tick 1030 (D1h 0, 18 done ticks). ON D1h stays 1, the glide runs 17 calls (blocked by rearm, bearing and lead) and leaves `aimglide -> goaway` at tick 1045 at 23.3 m. It meets the water one fixed step later (surface steps +1). No further request | states move, it may request again | held; no new request |
| #1.1\|.-2, #5.1\|.-2 | identical | identical | held |
| live Val rows, bombs | identical; 0 bombs | identical | held |
| deaths, death table, hit records, the Lexington's line | identical | same victims, ± 40 | held |
| natives | `count_remaining_rounds_007c1db0` concrete (23756 / 26270 calls); `release_spawn_deferred_to_rack` 9 -> 8; the done state's two records go; one fewer free-flight step | as predicted | held |

**Verdict: `kDiveBombCarriedRoundsBound` ON.** The count is the rack's, as `007C1DB0` reads it.
The only moved rows belong to the one dead Val whose two-round glide became one.

## Mavis rack drops (packet cc9_mavis_rack_drops)

2026-09-27, worker cc9-units2. Switch `kRackRoundsPerRackBound`, committed OFF with the
predictions below, ON since the pairs.

**The symptom.** In the layer-OFF USN01 runs of `docs/GAME_EXECUTABLE.md` (reference c,
`local\ud_az_off_usn01.log` in worktree cc9-gunnery2) Mav1 reports `issues=1`, `fired=1`,
`drops=4`, `ammo=0` for one request, and only one torpedo spawns. Mav4 is the same.

**This installation's data.** `scripts/datatables/autoload/vehicleclasses.lua` (modified
2026-05-09 21:52), `VehicleClass[174]` "H6K Mavis" (line 57020): `DefaultEquipment = 1`
(line 57095), and `Equipments[1]` mounts four platforms, keys 50 to 53, each
`{ Ammo = 1, Platform = 85, ReloadTime = 80 }` (lines 57103-57126). Device class 85 in
`classtables/realistic/deviceclasses.lua` (line 4374, 2024-07-13) is "Torpedo platform Japan
weak", `Function = "BOMBPLATFORM"`, `Type = "BombPlatform"`: a single rack. So a Mavis carries
four single racks of one torpedo each. Equipments 2 and 3 (devices 88 and 86) are not the default.

**What the image does per issue.**
- `007C0D90` walks the plane's children and stops at the first `IsKindOf(25h)` rack that holds
  2Ah (`vtable[210h]`) and is not busy (`vtable[1FCh]`). It fires that rack through
  `vtable[1F0h]` = `006E3550` (`dropBombs` +498h = 1, `toRepeatTime` += delay). For an aircraft
  that is not a level bomber (`IsKindOf(10h)` false) it then sets unit+C25h and returns, so one
  issue fires one rack. `callsite_census.py` finds `006E3550` only as the two vtable entries
  `00CF9898` and `00CF9B08`.
- The rack's tick `006E56F0` runs with ESI = rack+310h. Its end test `006E58D8`
  `CMP [ESI+174h], 0` reads the rack's own ammo +484h, and `006E58E1` clears its own `dropBombs`.
  So a rack of one round drops once and stops. The other three racks stay loaded.

**The host gap.** The host keeps one rack state per aircraft: `rack_ammo` is the sum of the
single racks' rounds (4 for a Mavis), and one issue sets one `rack_dropping`. The tick then drops
on every fixed step (`toRepeatTime` stays 0) until the pool is empty: four drops for one issue.
The first drop spawns the torpedo. The next three call `release_ordnance_drop`, which fails
because kind 2Bh was cleared by the first, yet `rack_drops` still counts them. The image drops one
round per issue and the host drops four. That is a host divergence, not the image's behaviour.

**The binding.** ON: the census keeps each single rack's authored Ammo in census order. The
child-list order is unread, so that is an assumption. The issue's first check seeds one ammo per
rack: its authored Ammo, or one round per rack where the census found none (the pool's own
fallback). The issue fires the first rack with a round left, and the tick tests and spends that
rack's ammo only. The pool `rack_ammo` is still spent by each drop, so `007C1DB0`'s count and the
walk's "holds 2Ah" test read the same sum as before. A new line `rack rounds:` prints the per-rack
ammo, the fired rack and `drops_unspawned`, the drops whose host torpedo spawn returned false. It
prints in both builds.

Every aircraft on USN04 and USN02 with a rack has one single rack (the D3A Vals, movievals and
B5N Kates), where one rack and the pool are the same thing. The Mavis and USN01's ScoutDauntless
(`single=3 authored=1`) are the multi-rack aircraft in the measured missions.

### The squadron's layer, re-checked

- `007F1D90` stores `0041DF40(1.5, true)` at squadron+34Ch. `007F1DB7` loads `[00CE380C]`,
  bytes `00 00 C0 3F` = 1.5f, and `007F1DBD` pushes 1. Its three call sites (rel32, by
  `tools/callsite_census.py`) are `007F2114` in `007F1FE0` and `007F4699` / `007F46F3` in
  `007F4580`. None passes a squadron value to the +34Ch query, so the Mavis squadrons and the
  Devastators get the same layer.
- `0041DF40` primes the result with the first layer, then keeps the largest slope limit strictly
  below tan(1.5 rad) = 14.10. The registry holds the constructor's tan(10°) layer and then the
  `.nav`'s tan(10°), tan(20°) and tan(70°) layers, so the answer is index 3, tan(70°) = 2.747478.
  That is the host's `squadron_34c=3 ... slope=2.747478` on USN01 (`local\GH_OFF_USN01.log` in
  worktree cc9-plane-release). **Held.**
- All 13 `.nav` files under `universe/Scenes/missions/USN/` are 172,939 bytes with md5 prefix
  `ec09b4bbff20`. Eleven are dated 2024-07-13. `usn_ormoc.nav` and `usn_sibuyan.nav` are dated
  2024-10-29 but hold the same bytes. So Marshall (USN01) samples the generic layer, not its atolls.

### Predictions (written before the runs)

Same-tree pairs `local\mr_off` against `local\mr_on`, the switch only, streams and death table
on, one run at a time. On main the layer sample is ON, and with it no Mavis issues a release on
USN01 (`issues=0` on all five), so the main pairs cannot reach the fix. A third pair,
`local\mrl_off` against `local\mrl_on`, builds both sides with `kAvoidZoneLayerSampleBound`
false (not committed) to reach the Mavis drops.

| row | USN01 3200/3000 | USN04 4700/4500 | USN01, layer OFF both sides |
| --- | --- | --- | --- |
| `rack rounds:` line | `bound=0` -> `bound=1`; `per_rack` empty on both unless a rack's issue check runs | same; a Kate that reaches its check shows `[1]` ON | Mav1, Mav4: OFF `[]` `drops_unspawned=3`; ON `[0,1,1,1]` `active=0` `drops_unspawned=0` |
| rack line `drops` | identical (0) | identical | Mav1, Mav4: 4 -> 1; `ammo` 0 -> 3 |
| torpedo drops (spawned) | identical | identical | 2 -> 2, at the same times |
| cleanup of unit+C25h | identical | identical | Mav1, Mav4: 3 fixed steps earlier ON (busy clears after one drop) |
| releases, deaths, death rows, hit records | identical | identical | identical; band ± 5 hit records if the earlier C25h cleanup moves a Mavis |
| pair_diff exit | 1 | 1 | 1 (3 only if the band is used) |

### The pairs, measured

Logs in worktree cc9-units2: `local\MR_OFF_USN01.log` / `MR_ON_USN01.log`,
`local\MR_OFF_USN04.log` / `MR_ON_USN04.log` (binaries of `b33ddee5d`, switch only) and
`local\MRL_OFF_USN01.log` / `MRL_ON_USN01.log` (the same, with `kAvoidZoneLayerSampleBound`
false on both sides, not committed). All six show the 1600x900 fit line, the immediate present
interval, a module directory in this tree and the final COM release. `tools/pair_diff.py` exits 1
on all three: gameplay, death rows, plane death modes and the unit table identical.

| row | USN01 3200/3000 | USN04 4700/4500 | USN01, layer OFF both sides | prediction | verdict |
| --- | --- | --- | --- | --- | --- |
| `rack rounds:` line | only `bound=0` -> `1`; `per_rack` empty on all five | only `bound`; Kate #4.1\|.-4 `[0] active=0` ON | Mav1, Mav4 `[0,1,1,1] active=0` ON | as written | held |
| rack line `drops` | 0 both | identical | Mav1, Mav4: 4 -> 1, `ammo` 0 -> 3 | 4 -> 1 | held |
| torpedo drops | 0 / 0 | 1 / 1 | 2 / 2 | unchanged | held |
| deaths, hit records, releases | 7, 150, 3 of 5 both | 43, 788, 4 of 16 both | 7, 141, 4 of 5 both | identical | held |
| natives | no change | no change | rack tick 8 -> 2, `can_fire` 10 -> 4, repeat time 8 -> 2; `Rack::drop_dispersion_006e4f91` (6 calls) gone | not predicted | see below |
| `drops_unspawned` OFF | 0 | 0 | **0, not 3** | 3 per Mavis | **failed** |
| cleanup of unit+C25h | identical | identical | `walks=1 busy=0` both sides; the step of the cleanup is not printed | 3 steps earlier ON | not measured |

**The failed prediction.** OFF, the three surplus drops of each Mavis did not reach the torpedo
arm. The first drop's `release_ordnance_drop` clears kind 2Bh from the ordnance mask, so the next
tick's `drop_is_bomb` test finds 2Ah alone and routes the drop to `run_rack_bomb_drop_006e4d50`.
That is the six `mission gunnery bomb_drops` refusals OFF (`refusals 6 -> 0`) and the six
dispersion records. So OFF a Mavis was also "dropping bombs" it does not carry; ON it does not.

**Verdict: `kRackRoundsPerRackBound` ON.** One issue is one rack's drop, as in `007C0D90` and
`006E56F0`. Nothing that reaches gameplay moved on any pair. The surplus drops were host bookkeeping.

## Handoff: the rest of cc9-plane-release's queue (2026-09-27)

Worker cc9-plane-release stopped at about 80 % context after `cc9_ground_height_hunks`. The
queue below is in the lead's order. Every item is in `src/game_hosts_units.cpp`, the units file.

**Before starting any item:**
- Run `python tools/bsp.py sync` and `python tools/bsp.py brief`.
- Keep edit scripts under your own tree's `local\`, named with your worker name, and re-read each
  before running it. The shared scratchpad collides between sessions (memory
  `shared-scratchpad-script-collisions`).
- Diff every pair with `python tools/pair_diff.py <off.log> <on.log>`.
- Each switch needs both variants built. MSVC `/WX` turns an unreachable `return` after an
  `if constexpr` arm into error C4702, so put the OFF body in an `else`.
- Stage the binaries into `local\<tag>_off` and `local\<tag>_on`, and run with
  `./tools/run_game.ps1 -Exe local\<tag>_<v>\bsp_game.exe -Log local\<TAG>_<V>_<M>.log -- ...`.

### 1. `cc9_mavis_rack_drops`

- **The symptom.** On USN01 3200/3000 with the layer sample OFF, the IJN Mavis flying boats Mav1
  and Mav4 each report `drops=4` on their racks: eight rack drops but only two torpedoes spawn.
  The gunnery successor's drift pairs at 435e8bb87 found it (docs/GAME_EXECUTABLE.md,
  reference c).
- **Where to look.** The rack route is `run_rack_tick_006e56f0`. The census is
  `plane_rack_census()`, which counts only `"BombPlatform"` single racks and lists
  `"MultiBombPlatform"` separately; the issue binding is `TorpedoReleaseOrderBinding`. The torpedo
  arm of the tick calls `gunnery->release_ordnance_drop(index)`, whose failures (no torpedo row,
  or kind 2Bh already cleared after the first drop) return false while `rack_drops` still
  increments. That is the likely host gap: `--rack_ammo; ++rack_drops` run whatever the spawn
  returned.
- **What to read.** This installation's Mavis class row in
  `scripts/datatables/autoload/vehicleclasses.lua` (modified 2026-05-09 21:52): its
  `Equipments[DefaultEquipment]` Ammo per torpedo platform, and the platform device's `Type`
  (single or multi) in `classtables/realistic/deviceclasses.lua`. Then the image per drop: the
  drop `006E4D50` takes one loaded child (`IsKindOf(2Ah)`, `006E4EC0`), and with ammo left it
  reloads through `vtable[224h]` (`006E55F0`), so one drop is one round. Also `006E3500` (rounds)
  and `006E3410` (rearm), in `docs/RELEASE_ISSUE_STAGE.md`'s Vals section.
- **The second item.** Check that squadron+34Ch = 3 is 0041DF40(1.5, true)'s choice on USN01. All
  13 USN `.nav` files are one file (md5 prefix ec09b4bbff20); say so in
  `docs/AVOID_ZONE_REGISTRY.md` where it describes the layer.
- **Pairs.** USN01 3200/3000, and USN04 4700/4500 for no change.

### 2. The pending-list push from `create_units` (the lead's third queued item)

- **The contract.** docs/CONSTRUCT_WORLD.md section 17 (world-init, 114dd603b):
  `create_units` calls `push_pending_entity_00926be0` for every constructed instance, as the
  image's base constructor 00928760 pushes onto 00F899D0 through 00926BE0.
- **The switch.** Use the existing `kSEntityInitAllBound` or a sibling.
- **Predictions.** Pending-list pushes per creation route, with `self_table_entities` 86 / 34
  unchanged, the InitAll row counts unchanged, and identical gameplay.
- **Pairs.** USN04 4700/4500 and USN02 9200/9000.
- **Out of scope.** Retiring the Lua host's own pushes and the wing-append substitution at pass A
  is cc9-world-init's (mission-frame, fixed-step and Lua files). Write it as a contract.

### 3. `cc9_sunk_ship_kill_depth`

- **What the image does.** A dead ship keeps sinking while +5Dh is set: 00825F20 advances
  sinkTime +828h through the sub-object base (ECX = unit+310h, `[EDI+518h]`). Once the hull is
  wholly below GameSettings+3F4h, `VizbeomlesDolgok.KillDepth` (−200 in this installation's
  shipglobals.lua line 377), `00826628` calls Kill. The 009263C0 removal follows, and three passes
  later the free unlinks the ship from the world lists through the +130h registrar.
- **The binding.** Put it in the ship motion path. The tail binding is `ShipMotionTailBinding`;
  the sinking step is inside `ship_motion_step_00825f20`'s host `ShipMotionBinding`.
- **The contract.** docs/CONSTRUCT_WORLD.md section 21 (the flush landed ON at bcaee249b).
- **Pairs.** USN02 9200/9000, where twenty ships sink, so predict each wreck's time to −200 m and
  its leaving list 6. Also E2.
- **Note.** 8c's `post_motion_gate` census will then fall below the tick count.

### 4. `cc9_units_init_pass_gaps` (docs/SENTITY_INIT_PASSES.md section 4)

- **The order change comes first.** It is being written by cc9-init-passes: the thisTable attach
  (pass A) runs over the whole pending list before any pass B or C.
- **Then the per-class gaps, largest gameplay effect first:**
  - ship pass C's immediate rudder 0080DA00 and torpedo stock 0081F8B0;
  - the squadron's avoid-zone layers 007F1D90 / 0041DF40 (the registry's
    `select_layer_by_slope_0041df40` already exists; squadron+350h needs the squadron record's
    slope value);
  - whether 007F4BA0's initial command 0077D600 at 007F4E9E duplicates the host's
    authored-command path;
  - plane pass B's body 00C5D580, the flight-controller setters, actuators, neighbours, the
    firing-gun registration 007C74A0, and a 00BD2F10 draw on the shared generator. Check
    `src/plane_flight.cpp` first, since some may exist under other addresses.

### State left by this worker

- **On main:** every switch this worker added is ON.
  - `kReleaseIssueStageValsBound`, `kDiveBombCarriedRoundsBound`;
  - `kAvoidZoneLayerSampleBound`, `kUnitsContractsBound`;
  - `kShipMotionTailBound`, `kFormationSlotSwapBound`, `kShipPostMotionRepairOrder`.
  - `kGroundHeightHunksBound` too (docs/SCENE_CONTENTS_HOSTS.md 7a; identical gameplay, 0 blocked probes).
- **Open run-time evidence:**
  - a live Val's rack drop (no mission here has a live Val release);
  - the slot swap's distance test (both missions gate every call);
  - unit+9B8h has no host reader.

## Handoff: cc9-units2's queue (2026-09-27)

Worker cc9-units2 stopped at about 80 % context after `cc9_squadron_initial_command` (ON at
88b41ee34). Everything below is in `src/game_hosts_units.cpp` unless named.

**Before starting any item:** run `python tools/bsp.py sync` and `brief`. Claim the lease before
the first edit. Keep scripts under your tree's `local\`. Write multi-line Python edits to a file:
Git Bash heredocs break on an apostrophe in the body. Diff every pair with `tools/pair_diff.py`.

### Landed by this worker, all ON

| switch | doc | what |
| --- | --- | --- |
| `kRackRoundsPerRackBound` | this file, "Mavis rack drops" | one ammo per single rack; one issue is one drop |
| `kSunkShipKillDepthBound` | docs/CONSTRUCT_WORLD.md 24 | sinkTime, the KillDepth kill, the removal and the on-killed world-list unlink |
| `kShipSinkDescentBound` | docs/CONSTRUCT_WORLD.md 25 | the leak manager and the wreck handler's sink block: wrecks flood and sink |
| `kUnitsPendingPushBound` | docs/CONSTRUCT_WORLD.md 26 | `create_units` pushes each instance (00928630 at 00928760) |
| `kSquadronInitialCommandBound` | docs/CONSTRUCT_WORLD.md 27 | the squadron's pass C default order (007F4E9E) |

The squadron hooks `on_squadron_pass_a_construct_wing` / `on_squadron_pass_c_initial_command`
are called by the Lua host since 27ab3a4b4 (20 / 20 on USN04, 0 on USN02).

### 1. The wing construction in pass A (next; three files, one switch)

- **Units half:** `on_squadron_pass_a_construct_wing(leader)` constructs the squadron's wing
  planes through `create_units`, which pushes each one while the walk is inside pass A.
- **Where the wing records live today:** the creator batches in `src/game_hosts_script_orders.cpp`
  build them, near line 400 for the air-ops launch and near 571 for GenerateObject/SpawnNew
  (`wing_record` copies of the leader's record, class `PlaneUnitInstance`, `class_id -1`, names
  from `plane_squadron_plan_members_007f4580`). They also fill the registry record's
  `member_units` as `before + wing`.
- **What must move:**
  - the script-orders host builds the leader only and holds the plan's wing records, for example
    on the `PlaneSquadronHostRecord`;
  - the hook builds them and appends their unit indices to `member_units`;
  - the air-ops `squadron_unit` / `member_units` pre-registration (script_orders.cpp around
    line 425) must then take the indices the hook makes, not `before + wing`.
- **The Lua half** is written in docs/SENTITY_INIT_ATTACH_ORDER.md section 15.4, with its
  predictions: `wing_appended` 40 -> 0, `wing_deferred` 40 -> 0, construction pushes 81 unchanged,
  entities / `self_table_entities` / `wing_member_tables` unchanged.
- **One switch across three owners:** units (this file), script orders (cc9-ships at the time of
  writing) and the Lua host (cc9-init-passes). The lead coordinates the joint flip.
- **Watch:**
  - unit ids: planes will be constructed after the squadron's pass A starts, not in the creator
    batch. On a SpawnNew group, member B's leader is then built before member A's wing, so every
    wing plane's id changes;
  - anything keyed by unit index (gunnery rows, logs by name) must follow. Expect identical
    gameplay only if nothing orders by id.

### 2. Open items this worker found, by owner

- **Gunnery (cc9-ships):** the torpedo stock contract (docs/CONSTRUCT_WORLD.md 27). Spare
  unit+104Ch = MaxTorpedoStock − loaded; a fired tube waits at the FLT_MAX sentinel until the
  spare re-arms it. The host's tubes re-arm on their own clock.
- **Gunnery (cc9-ships):** the live-hull leak, message 90h (docs/CONSTRUCT_WORLD.md 25). A
  units-host `add_leak_0074f440` entry is still to be written when the hit that sends it is bound.
- **Units:** the buoyancy element producer. The descent's rate rests on controller+84h over the
  eight stand-in elements (capacity = Mass / 3, about 97 s to −200 m).
- **Units:** the scene `HomeBase` key at squadron pass C (`00CF8820`) is not read. Only air-ops
  launches have a home base in the host.
- **Not isolated:** on the sink-descent pair, Kawakaze's first damage moved 2 s before any kill.
  If it recurs, check the heeling wrecks' hull geometry against rounds first.

## The sink-descent pair's pre-kill move, isolated (packet `cc9_sink_pair_prekill_move_read`, docs only)

2026-09-27, worker cc9-units3, on main 2872fab7b. This belongs in docs/CONSTRUCT_WORLD.md
section 25. That file is leased to cc9-ships (`cc9_wing_construction`) until 21:04Z, so it is
recorded here. Section 25 needs one pointer line to this section when the file is free.

Section 25's pair (`2e58c646c`, `SD_OFF_USN02` / `SD_ON_USN02` in worktree cc9-units2) moved
Kawakaze's first damage from 104.05 to 102.10 s, before any kill. Its cause was not isolated.

**It is not hull geometry against rounds.** The first divergence is ship AI reacting to how a
wreck moves.

### What differs between the builds

- **OFF, a wreck keeps cruising under its last throttle.** In neither build does the host cut a
  dead ship's engine: the wreck handler's pre-sink part `00824B60..00824FE4` is unbound. At
  52.50 s the controlled DeRuyter (dead at 30.25 s) runs at fwd 16.45 m/s, x = 119.2,
  throttle 1.000.
- **ON, the sink block damps it.** The block sets inertia × 2 (`00825044`), angular damping 2.5
  (`0082505C`) and linear damping 0.5 (`00825074`, `00C37E00`). The same wreck is at 2.86 m/s,
  x = 245.7. (`+828h`, zeroed by the same block, is `sinkTime`, not a throttle.)

### How a wreck's speed reaches live ships

Each live ship sees a neighbour through its obstacle node, and the node's boxes are projected by
the neighbour's own speed:
- the near box's half-length is 0.55 × Length + |advance|, with the lookahead advance from
  `009EAECA` (`src/ship_ai_neighbour_box.cpp`);
- the avoid box is advanced by the observed body-axis speed × the projection time (`009EB1FE`).

A cruising wreck therefore throws a long box ahead of itself. A damped one does not.

### The first differences, in order

1. **Step 1015 (50.75 s), the first host divergence.** The sector scan `009EB660` of Kortenaer
   finds DeRuyter's node blocking. The OFF box is 102.3 m: 0.55 × 171 plus 8.25 m of speed
   lookahead. The scan runs `009D84E0` at `009EBF67` and raises the node's lifetime
   (`009EBEDE`). A diagnostic run of the OFF build logs this block 60 times from step 1015 to
   step 1064, always Kortenaer against DeRuyter. In the logs of record, the natives
   `settings_neighbour_memory_194`, `raise_node_lifetime_78`, `passing_corner_009d84e0` and
   `settings_blocked_margin_1d8` first fire at mission frame 1014 OFF and at frame 7540 ON.
   Kortenaer's printed step lines stay identical through step 1060.
2. **Step 1060 (53.0 s), the first visible gameplay divergence.** Minegumo is slot 1 in a
   formation behind Yamakaze (slot 0): the same heading −0.6283 and the same fire target Alden.
   Yamakaze dies at 51.65 s. ON its sink block damps it; OFF it cruises on at throttle 0.5.
   - Minegumo's heading target first reads 4.4941 OFF against −1.7891 ON, exactly 2π apart.
   - At step 1070 the targets differ by 0.018 rad and the rudders by −0.138 against −0.284.
   - No other ship's step line differs before this (a keyed comparison of every
     `ship ai step` line).
3. From there the engagement drifts. Kawakaze's first damage moves 104.05 → 102.10 s and the
   hull hits 167 → 179.

### Evidence

- **The logs of record:** `SD_OFF_USN02.log` and `SD_ON_USN02.log` in worktree cc9-units2,
  read only.
- **The diagnostic:** `local\PK_OFF_USN02.log` in worktree cc9-units3, one run.
  - The binary is an export of `2e58c646c` (the pair's own OFF commit, `local\pk_off`) with a
    single diagnostic `notef` at the `009EBF67` site. That edit is not committed.
  - It ran 1200 mission frames at 0.05 s.
  - It reproduces the OFF log: the controlled-unit line at frame 1050 matches exactly.
  - The log shows the fit line, the immediate present interval, a module directory in
    worktree cc9-units3 and the final COM release.
- **Tools:** `local\cc9-units3-firstdiff.py` (keyed step-line comparison) and
  `local\cc9-units3-linediff.py` (a masked sequence diff), both in worktree cc9-units3.

### Consequence

The OFF side of any pair where ships die carries wrecks that cruise under their last throttle.
The engine cut belongs to the handler's pre-sink part `00824B60..00824FE4`, which is the next
thing to read. Until it is bound, a wreck's speed on the OFF side of a pair is a host artefact.
