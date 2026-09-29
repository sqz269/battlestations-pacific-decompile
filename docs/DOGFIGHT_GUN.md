# The dogfight task's gun controller

Addresses: 009F9980, 009FAAD0, 009FA620, 009FC7C0, 009FABE0, 007BA760, 007B96D0, 007B96F0,
0099C210, 007B4ED0, 009BECD0, 0099BEE0, 007BB6E0, 007C7800.

Packet `cc9_dogfight_gun`, after `cc9_dogfight_engaged` (docs/DOGFIGHT_ENGAGED.md). Every name is a
hypothesis, not a recovered symbol.

## 1. The controller behind approach+1Ch

`009F9980` sets `approach+1Ch = task+314h`. That object is built by `009FAAD0` from
`BSP_BotTask_ConstructBase`. It has **no vtable**: `+0h` is the task. `BSP_PilotBot_Update` ticks
it through `009FC7C0` at `00999979`, with `ECX = task+314h`, **after** the task arm
(`0099995A`), and only when `unit+C24h` is set (`00999962`). That is the same flag `007EEAEC`
requires for the dogfight class. So every task has this gun controller. The dogfight states
feed it the strafe cone at `+40h`: aim writes `Pilot/AutoStrafeAngle/Angle_Strafe`
(`tuning+678h`) and attackrun writes `tuning+670h`. Attackrun also sets the desired direction
`+68h` through `009FABE0(heading, pitch)`.

| field | value | source |
| --- | --- | --- |
| `+28h` | search arm; -1.0 after every tick | `009FAB14`, the tick's tail |
| `+2Ch` | 0.4 | `00CE7804` float, `0099C87A` |
| `+30h` | ShootDistance + 650 = 1500 | `00D1F3A0` double, `0099C888` |
| `+34h` | ShootDistance (850, SPNormal) | `0099C871` |
| `+38h` | `max(AimDistortAngle1 * 3.0, 0.08)` = 0.09 | `0099C864`; `00D7A2B0`, `00CE3E20`/`00D05B50` |
| `+3Ch` | 0 (the constructor; no writer found in the dogfight states) | `009FAB3C` |
| `+40h` | the state's strafe cone; zeroed at every tick end | aim `009A7960`, attackrun `009A7459` |
| `+48h` / `+4Bh` / `+50h` | fire / burst on / burst clock | the tick |
| `+74h` | auto target | `009FC90E` |

**`009FC7C0`**, `__thiscall(gun, float dt)`, `RET 4`, body `009FC7C0`-`009FCEE0`. Read from the
pseudocode, with the gates and burst clock checked on the listing:
1. `+50h -= dt`, and `+4Ch -= dt` while it is `>= 0`. It clears `+48h`, `+49h` and `+74h`, and
   sets `+44h = 999999.0` (`00CF87D0`).
2. **Target.** When `+28h < 1.0` and `+2Ch > 0` and `+30h > 0`, which holds every tick, it calls
   `007B96F0(max(+2Ch, +40h * 1.25), +38h, +30h, +34h * 0.3, +28h, 0, 0)`. That uses `00CF87C0`
   double 1.25 and `00CE3DC8` double 0.3. `007B96F0` forwards to `007E2090` on `unit+C50h`, and
   `007E2090` is **unread**. A live result goes to `+74h`. The lead point `+5Ch` is then the
   target's `vtable[48h]` prediction at `t = d / 007C2610()`, iterated twice.
3. **Range.** It skips to the tail unless `1 < d < max(+30h, +34h + 200)`.
4. **Envelope.** It fires only when:
   * `lateral < d * +38h`, `z > 1` and `z < +34h`, where lateral is the lead's (x, y) in the
     unit frame; and
   * `lateral > +3Ch` (divided by 1.8, `00D049A8`, with an auto target) or
     `lateral > 2 * +0Ch * +08h * d`. With `+0Ch = 0` that second arm is `lateral > 0`.
   * A hold timer `+4Ch > 0` passes it unconditionally.
5. **Steer.** When `1 - cos(lead, +68h) < +40h` and `+4Ch <= 0`, it runs `009FA7E0` (the
   distortion) and `009F9FC0`, which steers toward the lead point with the distortion scaled by
   `FighterAimMulVersusAI` (`tuning+648h`) or `...VersusPlayer` (`+64Ch`). Then `+49h = 1`.
6. **Fire** (`009FCDB6`-`009FCE1F`):
   * `envelope && !007BA760(unit) && !007B96D0(unit)` is required.
   * `007BA760` is true when `unit+C3Ah` or `unit+5Dh` is set.
   * When `007B96D0` is true, the tick re-aims at +0.25 through `009F9FC0` instead of firing.
   * Then `+4Bh != 0 || +50h < 0` sets `+48h = 1` and `task+2E0h = 1`.
7. **Burst clock** (`009FCE4F`-`009FCED9`):
   * With `+4Bh` clear, `+50h < 0` and `+48h` set: `+4Bh = 1` and
     `+50h = U(row+234h, row+238h)`, which is AimShootTime {4.5, 3.0}.
   * With `+4Bh` set and `+50h < 0`: `+4Bh = 0` and `+50h = U(row+23Ch, row+240h)`, which is
     AimShootDelayTime {1.6, 0.7}.
   * The row comes from `0099C210`.

**Which guns.** The controller never names a gun. Its only output is the request byte
`task+2E0h`.

## 2. The fire path into the projectile system: not established

* `task+2E0h` is **plan+2DCh**. `0099BEE0` is called at `0099B0A0` with `ECX = EBX+4`, the plan
  base, and copies `plan+2DCh` to `cmd+16h`.
* `007BB6E0` carries that to the control byte `unit+9FAh`, whose Lua name is `gunFire`
  (`007D6A90`). It is forced to 0 when `unit+5Dh` is set. `007B9770` latches it to `+BC9h`.
* **No weapon reader was found downstream.** The census scans:
  * `+BC9h`: only the latch writer, `007B97B6`.
  * `+BC8h`: `CMP byte` readers of the bomb byte and float fields of other classes.
  * `+9FAh`: the quantiser, the latch, `007C7800` (a player-camera effect in segment 45, gated on
    `unit+C24h`), the property-bag writer at `007C9B2B`, and `BSP_Plane_ReadPropertyBag`.
  * `+A12h`: no `.text` hit.
  * `LEA reg,[reg+9E4h]`: five sites, none a weapon.
* Category-0 guns (the fighters' `0:6`) get **no gun bot** in `0072C6A0`, which handles
  sub-types 1, 5/6, 2/3/4/6 and 7.
* So either the forward guns read `gunFire` through a pointer this census cannot see, or a
  weapon path outside the bot fires them. **Neither is established**, so no rounds are spawned.

## 3. Host against image (kDogfightGunBound = true)

**Bound:**
* `009FC7C0`'s range, envelope, fire and burst logic, as the pure rule `dogfight_gun_tick_009fc7c0`.
* Its `+48h` output feeds the aim tick's boredom rate (-4/s while firing).
* `007B4ED0` as the rule `dogfight_throttle_007b4ed0`. Its wiring into the head-on arm and the
  maneuver tail (`009A9270`) sits behind `kDogfightThrottleBound`, which is **off** after run F1
  (section 6).

**Labelled substitutions:**
* `+74h` is the task's own target, not the `007E2090` finder's choice, unled.
* `007BA760` and `007B96D0` are false.
* Draws are at their midpoints: burst 3.75 s, gap 1.15 s.
* The steering `009F9FC0` and the distortion `009FA7E0` are not bound.

**Not bound:**
* Rounds. No hook in `src/game_hosts_gunnery.cpp` is needed or requested until the gunFire
  consumer is found.

**Also read:**
* `009BECD0`, `__thiscall(state, a, b, sep)`, `RET 0Ch`. It returns
  `b + (a - b) * max(interp(tuning+5CCh, 0, tuning+5D0h, 0.5, sep), 007EF2C0())` when
  `approach+0Ch` and `+8h` are non-null, else `a`. `007EF2C0` is unread.
* The moveto stand-in stays: its tick `009C18C0`'s separation input is not modelled in the
  dogfight arm.

**Still substituted:** `009AAA80`'s early edge, because it needs `007E2090`.

## 5. Predictions, written before runs F0/F1

F0 is this tree with `kDogfightGunBound = false` (main's engaged binding). F1 turns on the gun
controller as a census, the head-on throttle (`007B4ED0`) and the maneuver tail's full throttle.
No rounds are spawned (section 3).

1. **Bursts** come only from the three `Yorktown-class01_sqn02` fighters, while they are in aim
   on a Val inside 850 m and within `0.09 * d` laterally. That gate is tighter than E1's
   `gun_window_ticks`, which used a 0.25 tangent. Expect one to three bursts per fighter. The
   Lexington flight has none.
2. **No hits and no kills by fighters**, because nothing is fired into the projectile system.
3. **Behaviour changes are small and confined to the Yorktown flight.**
   * Aim boredom now runs down at -4/s while the fire flag is up. No aim exit was boredom in
     E1, so the state sequence should not change.
   * Maneuver flies full throttle. Only `.-3` spent real time in maneuver (28 ticks in E1).
   * Head-on throttle applies only on head-on aim ticks, and E1 had no too-close entries.
4. **Downstream.** Any change in other units' rows comes through the same recon and assignment
   mechanism as E1. Dive-bomb rows and water contacts should be unchanged.

## 6. Runs

All runs are USN04 at the E2 parameters (`--frames 9200 --press-start-frame 30 --menu-select
USN04 --mission-frames 9000 --mission-frame-seconds 0.05`). Each ran from its own copied binary
through `tools/run_game.ps1 -Exe`, and each log ends with the renderer's final COM release line.

| run | configuration | binary | log |
| --- | --- | --- | --- |
| F0 | this tree (main `80f19575f` plus this packet), `kDogfightGunBound = false` | `local\binF0` | `local\F0_usn04.log` |
| F1 | gun census, plus `007B4ED0` wired into the head-on arm and the maneuver tail | `local\binF1` | `local\F1_usn04.log` |
| F2 | gun census only (`kDogfightThrottleBound = false`) | `local\binF2` | `local\F2_usn04.log` |

**F1 is rejected.** Two new water contacts appear: `Yorktown-class01_sqn02` and its `.-2`, both
at |v| 51. Each had spent exactly one tick in maneuver, whose tail leaves the throttle slot
active in direct mode (`+2D8h = 0`). Both then stalled and drowned after aim handed them back to
moveto and follow. `.-3` spent 20 ticks in maneuver and survived. The image rebuilds the command
block every think (`0099B4E8`) and this host does not, so the write outlives the state. The
throttle wiring is now behind `kDogfightThrottleBound`, which is off. `dogfight_throttle_007b4ed0`
stays as a reconstructed rule.

**F2 against F0, term by term:**
* **Fighter gun.** One burst, from `Yorktown-class01_sqn02|.-3` on `D3A Val #3.1|.-3`, in aim at
  845.7 m with a lateral offset of 59.07 m. The envelope limit there is 0.09 × 845.7 = 76.1 m.
  The burst gave 5 fire ticks: the Val stayed under 850 m for only 5 ticks before the range
  opened. The other five fighters raise no request. The prediction of one to three bursts per
  Yorktown fighter was wrong: the Vals pull away before most chases close inside 850 m.
* **Everything else is identical.** All `dogfight` rows (states, latches, targets) match. Water
  contacts are 8 and 8, the killed_by table matches, and the dive-bomb rows match (364 lines
  each). The only differing summary row is `fighter gun`. The one input the census feeds back,
  the aim boredom rate while `+48h` is up, did not change any state sequence.
* **Rounds: 0.** No fighter damages anything, so there is nothing to trace into the dive-bomb
  rows.

## 7. Decision

**The census lands on this branch** (`kDogfightGunBound = true`, `kDogfightThrottleBound =
false`). F2 matches F0 on every per-entity row, and the image's fire decision and burst clock are
now visible per fighter.

**Fighter gunfire is not bound.** Two things block it, and both need reading:
1. **The consumer of `gunFire`.** `plan+2DCh` reaches `unit+9FAh` and is latched to `+BC9h`, and
   no weapon reader of either was found by the literal scans (section 2). Candidates are a
   pointer read of the control block, or the plane's weapon update reached through the unit
   vtable.
2. **The finder `007E2090` on `unit+C50h`.** It supplies the auto target `+74h`, and through
   `007B96F0` it also gates `009AAA80`'s early edge.

**Also open:**
* The host needs a per-think re-seed of the plan slots (`0099B450` / `0099B4E8`) before
  `007B4ED0` can be wired.
* `009F9FC0` (the aim assist) and `009FA7E0` (the distortion) are unread.

## Correction, 2026-09-23 (packet cc9_plane_gunfire)

* Section 6's explanation of run F1 is **wrong**. It said a one-tick maneuver write of
  `+2D8h = 0` outlived the state because the host did not re-seed the command block. The host
  now re-seeds `+2B4h`, `+2B0h` and `+2D8h` every think (`kPilotPlanReseedBound`), and with the
  throttle wiring on top (run T1) the same two fighters still drown at the same speeds. The
  likely cause is the head-on throttle cut in aim. That is untested. See
  `docs/PLANE_GUNFIRE.md` section 6.
* Section 2's "no weapon reader" still holds after a wider search: the latched block
  `+BB0h`-`+BCAh`, the command buffer and the gun vtable slot `+1DCh`. See
  `docs/PLANE_GUNFIRE.md` section 2.

## 8. The gun controller in every task (packet `cc9_task_gun_controller_all_tasks`, cc9-lua18, 2026-09-29)

**The image.** `BSP_PilotBot_Update` `009998A0`, read from the listing:
- **Head** (`009998A4`-`009998C6`): with `bot+270h == 2` and the published byte of `bot+274h` set,
  it skips everything.
- **Fast path:** `0099C270` (body `0099C270`-`0099C293`) returns
  `unit[word[00F876B8]*8 + 9C2h] != 0`. That byte is the previous step's published copy of
  `unit+520h` (`007CDC70`, at `007CDCD0`). A set `+520h` means the plane is not AI-flown; the host
  holds it as `generic_suppress_520`, and `009A17F8` reads "clear" as "an AI plane".
  - When the test is true, the task arm runs at `009998FB` (with `+2E4h = 0`), then `0099D300`:
    no `0099B740` and no gun tick.
- **Slow path:** the `+308h`/`+304h` accumulator gates only `0099B740` (`0099993C`). Then:
  - the task arm runs at `0099995A` (with `+2E4h = FFh`);
  - **`009FC7C0(task+314h)` runs at `00999979` whenever `unit+C24h` is set** (`00999962`), for
    every task class;
  - then comes the `+38Ch` sub-object (`00999983`, unread).
- `unit+C24h` is `PilotFires` (docs/ATTACK_GATE_TAILS.md). This installation's
  `vehicleclasses.lua` (mtime 2026-05-09, modded) authors `PilotFires = true` on 201 platform
  rows and `false` on 50.

**The cone.** The host passes `+40h` directly rather than storing it:
- The target cone is `max(+2Ch 0.4, 1.25 x +40h)`. Every authored cone is below 0.32 rad
  (`planeglobals.lua`, mtime 2024-10-29: Prepare 0, MoveTo 2, GoAway 10, Strafe 15 degrees), so
  the finder's cone is 0.4 in every state.
- Only the fine-aim steer gate (`009FCCA4`) depends on `+40h`.
- This binding delivers no non-dogfight cone. So outside dogfight the steer never runs.
  - That is exact for Prepare and the land states, whose authored cone is 0.
  - It is a labelled gap for moveto (2 degrees, `009C1B2D`, `009C282C`, `009C257F`) and goaway
    (10 degrees, `009C4E27`, `009D111C`).

**Item c, whether a bomber strafes its ship target: no.** `009FC7C0`'s target is the finder
`007E2090`, which scores only the `+50h` list (docs/PLANE_GUNFIRE.md). `007E11D0` fills that list
with aircraft (`vtable[5Ch](0Fh)`) of another party within the enemy radius. So a bomber's
forward guns fire only at enemy aircraft ahead of it, never at its ship.

**The binding** (`kTaskGunControllerAllTasksBound`, OFF when committed) calls the existing
`df_gun_tick_009fc7c0` after the task arms. It runs only for a plane that meets all of these:
- no dogfight task (the dogfight arm already runs the tick);
- a modelled torpedo, dive-bomb, moveto or land task;
- `generic_suppress_520` clear;
- `PilotFires` set.

The fire request feeds `plane_gun_fire_bc9`, which the gunnery host turns into rounds and the
evasion scan turns into the firing list.

Labelled substitutions:
- the fast-path test reads the current `+520h`, not the previous step's published copy;
- a plane with no modelled task gets no tick;
- the non-dogfight cones are not delivered.

**Predictions (written before the pairs):**
- **Every row with enemy aircraft crossing ahead of AI bombers or escorts** (JM05, USN04, USN13,
  IJN01):
  - `summary mission task gun (all tasks)` shows ticks on every PilotFires plane with a task, and
    bursts > 0 on some;
  - `trigger_rises` grows;
  - the gunnery host's plane-gun shots grow;
  - the firing-list evasion (`summary mission gunfire avoidance ... flagged`) becomes non-zero
    for the first time.
- **Deaths:** new aircraft kills with a plane as the killer are possible; `pair_diff` 3.
  - Through the shared generator, ship AA draws shift, so aircraft deaths move in time and
    identity. Judge the per-entity table, not the totals.
- **No ship takes forward-gun damage from a bomber** (item c).
- **USN01** (five Mav torpedo aircraft, no enemy aircraft): 0 bursts; `pair_diff` 1 unless the
  RNG draw count moves.

**Pairs** (OFF = `b0322e072` tree build, ON = its export with the flip; `local\l18_f0_*`,
`local\l18_f1_*`):

| row | pair_diff | task gun census (ON) |
| --- | --- | --- |
| JM05 9200/9000 | 1, gameplay identical | 45 planes, 85973 ticks, 0 bursts |
| USN04 9200/9000 | 1 | 51 planes, 52177 ticks, 0 bursts |
| USN13 3200/3000 | 1 | 24 planes, 24061 ticks, 0 bursts |
| IJN01 3200/3000 | 1 | no eligible plane |
| USN01 3200/3000 | 1 | 2 planes, 1296 ticks, 0 bursts |

**Reach census** (ON only, `e845cbb01`, which adds `enemy_list_ticks`, the ticks with a non-empty
`+50h` list; `local\l18_f2_*`):

| row | planes | ticks | ticks with enemy aircraft listed | bursts |
| --- | --- | --- | --- | --- |
| JM05 9200/9000 | 45 | 85973 | 0 | 0 |
| USN04 9200/9000 | 51 | 52177 | 690 | 0 |
| USN13 3200/3000 | 24 | 24061 | 0 | 0 |
| JM08 3200/3000 | 9 | 9801 | 420 | 0 |
| LOMP10 3200/3000 | 10 | 14926 | 0 | 0 |
| USN02, JM06, BSM01, LOMP06, USN12, USNOS | 0 | 0 | 0 | 0 |

**Verdict.** The mechanism matches: the tick runs on every eligible plane, and the enemy list fills
on USN04 and JM08 when enemy aircraft come within 1200 m. The prediction of bursts on the mixed-air
rows is a **spread miss**. No listed enemy ever enters the fire envelope
(`lateral < 0.09 d`, `1 < z < 850`), so no row fires, and the firing list stays empty (`flagged=0`).
Every pair is gameplay identical. **`kTaskGunControllerAllTasksBound` ON**, recorded as inert on
the reference rows.

**The land-state records, relabelled with the switch on:**
- `009B1DDA` (begin's `direction_40`): **done**. It stores a literal 0.0, and the host's tick
  delivers cone 0 outside dogfight.
- `009FABE0` in standby and line: **done only while the authored `Angle_Prepare` is 0**, which
  it is in this installation. The code tests `pilot_auto_strafe_angle_angle_prepare == 0` and
  records otherwise.
- The moveto `009FABE0` records (Angle_MoveTo 2 degrees) stay recorded; that cone is the open gap.

## 9. The states' cones outside dogfight, and the untasked planes (packet `cc9_task_gun_cones`, cc9-lua18, 2026-09-29)

**Every store to the cone in the bot segment.** A displacement scan of `00996000`-`00A00000` for
the four tuning fields, `+66Ch` to `+678h`, found these sites. Each `FLD` is followed within
16 bytes by `FSTP [reg+40h]` through `[approach]+1Ch`, checked with `disasm-raw`.

| site | state (Ghidra function) | value | host |
| --- | --- | --- | --- |
| `009C1B2D` | moveto `009C18C0`, the with-target arm | Angle_MoveTo | torpedo, dive-bomb and land moveto ticks |
| `009C2584` | moveto task `009C2430` | MoveTo | moveto task tick |
| `009C2831` | moveto circle `009C26D0` | MoveTo | moveto task circle |
| `009BFD2B` | follow `009BEE30`, the fly-to arm | MoveTo | `run_follow_tick_009c1fd0`'s fly-to arm |
| `009BF9D7` | follow, the hold arm | Prepare | not delivered (0; exact while Prepare is 0) |
| `009C4443` | dive-bomb attackrun `009C4220` | MoveTo | dive attackrun tick |
| `009C4E2C` | dive-bomb goaway `009C4A40` | GoAway | dive goaway tick |
| `009D0AD4` | torpedo attackrun `009D07B0` | MoveTo | torpedo attackrun dispatch |
| `009D1121` | torpedo goaway `009D0F10`, the window arms | GoAway | goaway arms 1, 2 |
| `009D11BA` | torpedo goaway, after the window, high | MoveTo | goaway arm 3 (arm 4 stores nothing) |
| `009C7032`, `009D2AD4`, `009B1212`, `009B08C3` | flyabove, torpedo done-prepare, land standby, land line | Prepare | not delivered (0) |

Branch checks, from the listings:
- `009D07B0` and `009C4220` have one `RET` each, and no jump targets an address past their store
  (the last targets are `009D0A1F` and `009C438B`). The store is on every path.
- `009C18C0`'s store is only in the with-target arm; each host tick sets it after its target
  guard.

The sites outside the host's modelled tasks, `009A38CE` (depth charge), `009A744E` and `009A7952`
(dogfight attackrun and aim), `009CAC37` (strafe) and the rest, are not delivered. The dogfight
arm keeps its own path, which covers aim only; its attackrun MoveTo cone stays a gap.

**The consumer.** `009FC7C0` reads the cone at `009FC8B5` (the finder cone
`max(0.4, 1.25 x +40h)`) and at `009FCCA4` (the steer gate). The host now gives both the state's
value outside dogfight, and the gun tail zeroes it (`009FCE69`) every think.
- SUBSTITUTION, labelled: the steer gate measures the lead against the unit's forward, not against
  `009FABE0`'s `+68h` (the commanded heading and pitch).

**Untasked planes.** `0099A170` builds a task for every command class, `009C3C40` for none
(docs/ATTACK_COMMANDS.md). So every AI bot has a task, and `009998A0` ticks its gun.
`kTaskGunUntaskedPlanesBound` gives the tick, with cone 0, to an eligible plane whose command has
no host task (for example `returntobase` when the land task was refused). The census counts
those planes' thinks in both builds.

**Switches:** `kTaskGunConeBound` and `kTaskGunUntaskedPlanesBound`, both OFF when committed. The
patch compiles with both ON (a scratch export of the tree at `02d593a6e`).

**Predictions (written before the pairs):**
- **Values:** Angle_MoveTo = 2 degrees = 0.0349 rad, so the steer gate opens only when
  `1 - cos(lead) < 0.0349`, a lead within about 15 degrees of the nose. Angle_GoAway = 10 degrees
  opens it within about 34 degrees. The finder cone stays 0.4 in every state (1.25 x 0.1745 < 0.4).
- **Cone ticks:** `cone_ticks` > 0 on every row with a PilotFires plane in moveto, attackrun or
  goaway: JM05, USN04, USN13, LOMP10, JM08.
- **Steer ticks:** the steer needs a target from the finder, that is, an enemy aircraft within
  1200 m and ahead. Section 8 found the list non-empty only on USN04 (690 ticks) and JM08 (420),
  with no enemy ever inside the fire envelope. Expect `steer_ticks` 0 on all rows, or a handful
  on USN04 or JM08; in either case `pair_diff` 1.
- **Untasked:** `untasked_planes` counts the refused-RTB and unmodelled-command planes (JM05 has
  the refused SecondaryAirfieldEntity 01 returns). With the switch on, they tick with cone 0 and
  the same finder. Expect no bursts, so `pair_diff` 1.
