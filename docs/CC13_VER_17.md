# CC13 VER 17: why USN04's carrier launches fly into the sea (cc13-ver17)

2026-10-13. Static reading of the image and of existing logs only; no new runs. Names are
hypotheses. Follows docs/CC13_VER_14.md section 1 (the symptom) and docs/CC13_VER_15.md (the
launch side up to lift-off, verified).

## Verdict

**Root cause: the host does not model the image's speed floor during avoidance band repair**
(cc13-ver1's V1, docs/CC13_GPR_48.md 8.3). The terrain arm and the takeoff task correctly pull
the nose up, but without the floor the aircraft stall and fall.

- In the image, the band repair `0099BF30` calls `007D7A40` whenever it changes a plane's pitch or
  yaw command in free flight. That sets a 1.2 s timer.
- While the timer runs, the integrator holds the body forward speed at no less than
  1.05 x LevelFlightSpeed, about 1.89 x StallSpd. For the Zero that is about 35 m/s.
- So a plane whose command is repaired every think cannot fall below that speed. It zooms out of
  the layer.
- The host has neither the timer nor the floor. The planes bleed from 40 to 17-19 m/s at full
  up-elevator, stall, and hit the water.

The fix is committed OFF as `kAvoidRepairSpeedFloorBound`, in `src/game_hosts_units.cpp`. Status:
reconstructed and build-tested, **not game-validated**.

## 1. The trajectory, from the logs

Logs used:
- `cc13-strafe2\local\st2_u4base.log` (all line numbers below are from this log);
- `cc9-ships44\local\s44_gpon_u4.log` and `s44_gpoff_u4.log`;
- `cc9-gunnery36\local\g37_a9allon_u4p1.log` and `g37_a9alloff_u4p1.log`.

They tell the same story, but carry no per-tick plane trace. The sqn07 leader (class 150, Zero):

| t (s) | event | line |
|---|---|---|
| 256.81 | takeoff task installed, floor 28.19 (15 + deck y 13.19) | 83385 |
| 258.41 / 258.51 | prepare -> SlowTakeoff -> Takeoff | 83922, 83970 |
| 264.16 | ground contact lost at the deck edge, then lift-off 4 -> 7, y 13.34, \|v\| 40.33 | 85951-85952 |
| (formation tick 0) | leader speed **17.22** | 88896 |
| 290.55 | surface probe: alt 4.58, vy -13.97, spd 19.24, **pitch_c64 1.5628, live_pitch 1.000**, throttle 1.0 | 94791 |
| after 290.55 | water contact, \|v\| 18.78, dead | 94864 |
| end of run | takeoff summary: still in state 470, never done; `no_holder=267` free-flight steps | 327680 |
| end of run | terrain avoid: 267 terrain ticks, 536 bands, 70 throttle sets, **min_margin -206.0** | 330347 |

A census over every lift-off in the log (script `local/v17_census.py`) splits the launches into two
groups:

- **Fine.** Yorktown sqn02/04/08/09/11/13/15 and Lexington sqn01/03/05/06/23.
  - The takeoff task retires 5.0-6.0 s after lift-off.
  - At that point the plane has |v| 25-66 and y 34-102.
  - The terrain-avoid min_margin is 9.4 to -0.4.
- **Crash.** All 28 Zuiho aircraft (sqn07-18) and the 12 Yorktown aircraft of sqn19-22.
  - The task never retires, or retires late (13-15 s) at 35 m/s.
  - Water contact comes at 18.7-29 m/s.
  - The terrain-avoid min_margin is **-108 to -313 m**, with 40-70 throttle sets per plane.

The crash group is exactly the set of planes that took off below the avoid-zone layer. It is not a
class, a deck-heading or a launch-speed effect. Yorktown sqn19 is class 112, the same class as the
fine sqn09, and lift-off headings and speeds match across the two groups.

## 2. Why the planes are under the layer (image-faithful)

- The script places the Shoho with `PutTo(Zuiho-class01, (11000, 0, 14000))` (log line 75605).
  That is past the .nav grid's +12000 edge, so `00417FA0` clamps it to the bottom row near the
  east end.
- That is where the generic `.nav` (all USN missions share it, docs/AVOID_ZONE_REGISTRY.md) holds
  200-300 m cells.
- The Yorktown's sqn19-22 climb-outs (652-750 s) sample the layer at -108 to -272 m too
  (log summary lines). I did not trace the carrier's track to name the cells.
- The host samples the layer with `kAvoidZoneLayerSampleBound` (ON). Its `0041BC20` /
  `00417FA0` / `0041BAE0` were verified against the bytes.
- So this is what the image sees too. **Not a divergence.**

## 3. The hand-off after lift-off (verified, no divergence)

| image | what | host | verdict |
|---|---|---|---|
| `009CFC70` | retire when free flight, +908h > 5.0 (`00CE3850`), and (y > task+424h = sub+2Ch, or \|v\| > `007C4810` MinControlSpeed) | `takeoff_rule_009cfc70`, game_hosts_units.cpp (search the name) | MATCH |
| `009CFA80` | sub+2Ch = max(layer350(x,z) + 30h, 30h + 34h); ECX = `[[ESI+0Ch]+350h]` at `009CFAA3` | `takeoff_altitude_floor_009cfa80` | MATCH apart from the labelled +34Ch-for-+350h layer substitution (see 6) |
| `009CE2C0` free-flight arm | P = Interp(2.0, P0, 6.0, class+1ECh, f*5 + 908h), then P = max(P, c64 - 3 deg) (`009CE9xx`, `piVar1[0x319]` - `00D1A8A0`) | `run_takeoff_run_step_009ce2c0`, branch F (no holder): throttle 1.0, level, pitch target P | MATCH |

Note that the c64 - 3 deg lower bound ratchets the pitch target up with the nose. Once the terrain
bands drive the nose past the climb angle, the takeoff task holds it there. This is image
behaviour.

## 4. The divergence: the repair speed floor

**Image.**
- **`0099BF30` (the band repair).** When the pitch command `cmd[1]` changes it calls `007D7A40`
  with `ECX = unit+AB0h` (`0099C02C`/`0099C032`). It does the same when the yaw command `cmd[0]`
  changes (`0099C0E7`/`0099C0ED`). Each call is gated on the unit's +72Ch `vtable[38h]` (free
  flight), asked again after the store.
- **`007D7A40`.** `if (ctl+10h) dyn+C8h = [00CE3814] = 1.2f`.
- **`007D81B0`, the core law's first call.** It does `dyn+C8h -= step` while the timer is
  positive. It parks the timer at -1.0 (`00D7A260`) when the free-flight predicate is false.
- **`007D87C7`-`007D87F1`, in `007D8470` after the 0.01 deadband.** `ESI = EDI+64h` at
  `007D85F7`, so dyn+64h..6Ch is the body velocity and dyn+6Ch its forward component. The code is
  `if (dyn+C8h > 0.0 && dyn+CCh > dyn+6Ch) dyn+6Ch = dyn+CCh`. The 0.0 comes from XMM0, which the
  deadband stores at `007D878F`/`007D87A8`/`007D87C2`.
- **`007DB2C0`** stores dyn+CCh = float(`007C47F0` x 1.05 [`00CF2510`]) at placement
  (`007C5F60`) and at spawn.

**Host before this packet.**
- `gunfire_repair_0099bf30` ended with "007D7A40 after a changed pitch or yaw: unread, not
  modelled".
- The free-flight integrator (`integrate_body_velocity_007d8611` in `run_core_law_007db680`) had
  no floor.
- include/bsp/plane_flight.hpp:604 says the floor "does not run in free flight". That is wrong:
  the timer is only ever set in free flight.

**Effect.**
- In the image, a plane under continuous band repair keeps at least about 1.89 x StallSpd (Zero:
  1.05 x 1.8 x 18.6, about 35.2 m/s) on its body axis. At full up-elevator it climbs out of the
  phantom layer.
- In the host, the same plane decays below the stall, goes vertical (pitch_c64 1.56 with the
  elevator still at +1.0), and falls. The takeoff task's c64 - 3 deg bound keeps the target up.
- This explains 40 of 40 crashes: 28 Zuiho and 12 Yorktown sqn19-22.

**Fix (committed OFF).** `kAvoidRepairSpeedFloorBound`, src/game_hosts_units.cpp:
- **The switch and its evidence** are next to `kAvoidZoneLayerSampleBound`.
- **New per-unit fields** are `plane_speed_floor_timer_c8`, `speed_floor_resets` and
  `speed_floor_snaps`.
- **`gunfire_repair_0099bf30`.** `finish` now reports a change, and `reset_floor_timer` sets
  1.2f when the plane is in free flight.
- **`run_core_law_007db680`.** On entry it calls `bsp::tick_contact_timer_007d81b0` (the existing
  plane_flight model) with the predicate `+900h == 7`.
- **After `integrate_body_velocity_007d8611`.** It calls
  `gameplay_seg48_cc13::takeoff_speed_floor_007d87c7` on `vs.velocity[2]`, with
  `takeoff_floor_speed_007db2c0(bot_desired_speed_007c47f0(unit))` as dyn+CCh. That is labelled:
  the image stores the value at placement, while the host forms it from the same class and tuning
  each step.
- **A census line** `repair speed floor <unit> resets= snaps=` prints only when the switch is ON.

While OFF, the build is byte-for-byte unchanged in behaviour: every new statement sits under
`if constexpr`.

**Predicted effect when ON** (for cc9's pair):
- **USN04.** The Shoho's CAP and strikes (sqn07-18, 28 aircraft) and the Yorktown's sqn19-22
  (12) should climb out instead of ditching:
  - water contacts at 18-29 m/s should disappear;
  - the takeoff task should retire, by speed above MinControlSpeed or by altitude;
  - the Japanese strikes reach the US fleet. Expect large downstream changes: AA kills, ship
    damage, and possibly sinkings and the mission outcome.
- **Other planes.** Any plane whose gunfire, vehicle or terrain arm repairs a command will hold
  at least about 1.89 x StallSpd for 1.2 s afterwards. Slow-flight phases change: dive pull-outs,
  approaches, and the Yorktown fighters over the eastern block.
- **RNG.** The change draws no random numbers, but it moves positions and so changes the order
  and count of later stream-0 draws (AA fire-stagger, kills). It is RNG-coupled, so judge it from
  the per-entity tables, not the aggregates.

## 5. What differs between the failing and the working launches (brief item 2)

Nothing on the launch side: task, deck run, lift-off speed (37-41 m/s), the takeoff task's floor
at install (15 + deck y) and the deck velocity carry-in are the same. The one discriminator in the
logs is the avoid-zone layer height under the climb-out path:
- the Shoho's (11000, 14000), clamped;
- the Yorktown's position after about 600 s.

The two crash shapes follow from the class:
- **The Zero (class 150)** never reaches MinControlSpeed again, so the takeoff task never
  retires.
- **The Val/Kate classes (158/162/112/108)** sometimes retire at 13-15 s at about 35 m/s by
  speed. They then fall under the next task, which still has the bands and still has no floor.

## 6. Not established / open

- **The image's actual layer for `009CFA80`.** It is squadron+350h (`0041DF40(squadron slope,
  false)`). The host substitutes +34Ch (tan 70 deg). If +350h is a shallower-slope layer, the
  image's floor is higher still. That does not change the conclusion, since the floor arm only
  delays the takeoff task's retirement.
- **Think cadence.** `0099BF30` runs once per pilot think. The 1.2 s timer covers the gap between
  thinks, so the floor stays continuous while bands persist. I did not measure the band
  persistence per think in these logs (there is no per-tick trace).
- **The +20h byte of the C6h lift-off message** (docs/CC13_VER_15.md, open point) is unrelated
  here: the planes are fine for the first seconds.
- **Pairing.** cc9 needs to pair OFF/ON on USN04 (and one mission without phantom terrain) before
  flipping.

## Handoff

- Commit: see `git log agent/cc13-ver17`. The switch is OFF. Build: scripts/build.ps1.
- Ledger: no new names. Every function cited already has a ledger name (`007D7A40`
  BSP_PilotPlan_ResetCommandTimer, `0099BF30` BSP_PilotBot_RepairCommandBands, `007D81B0`
  BSP_PlaneFlight_TickContactTimers, `009CFC70` BSP_TakeoffTask_Rule, `009CFA80`
  BSP_TakeoffTask_UpdateAltitudeFloor).
- **Correction for cc9.** The comment at include/bsp/plane_flight.hpp:604-606 ("The takeoff floor
  (dyn+C8h) ... do not run in free flight") is wrong for dyn+C8h. That header is cc9's; it is
  not edited here.
