# CC13 stub census, ranked by reach (packet cc13-stubcensus)

This census reads existing logs only. No game was run for it. It uses the newest bsp_game log for each
WINS row (JM08, USN04, USN21, USN01, BSM09, BSM01, IJN08, USN15, USNOS, LOMP06, JM06) that meets
four conditions: built from a commit on main, exited 0, ran the default or OFF configuration (`_off`,
`_def` or `main_ref`), and was not run under `BSP_FRAME_JITTER`. The census aggregates every host site
that the logs report as not implemented, and it states the gameplay consequence of the top sites.
Line numbers refer to this branch after merging origin/main 81e083b63.

## 1. Logs used

The `behind` column counts commits on e73b7450f (this branch's base) that are missing from the log's
build. Every build is a clean checkout (`dirty=false` in the run manifest).

| mission | log | mtime (local) | build sha | behind origin/main e73b7450f | env | statuses (concrete / UNIMPLEMENTED rows) |
|---|---|---|---|---|---|---|
| JM08 | `...-cc9-gunnery52\local\g53_lfoff_j8.log` (30000 frames) | 2026-10-19T00:36 | aa5aaab54 | 14 | DEATH_TABLE,GUNNERY_RNG_STREAMS | 1267 / 494 |
| USN04 | `...-cc9-physics1\local\p23h_off_u04.log` (40000 frames) | 2026-10-18T23:22 | 7f9207dc6 | 158 | DEATH_TABLE,GUNNERY_RNG_STREAMS | 1082 / 476 |
| USN21 | `...-cc9-physics1\local\p23j_off_u21f_def.log` (80000 frames) | 2026-10-19T00:37 | 5eab32b9e | 28 | DEATH_TABLE | 1202 / 528 |
| USN01 | `...-cc9-physics1\local\p23j_off_u1i_def.log` (30000 frames) | 2026-10-19T00:22 | 5eab32b9e | 28 | DEATH_TABLE | 1145 / 522 |
| BSM09 | `...-cc9-physics1\local\p23j_off_b9_def.log` (36000 frames) | 2026-10-19T00:21 | 5eab32b9e | 28 | DEATH_TABLE | 1193 / 547 |
| BSM01 | `...-cc9-lua52\local\l69r_b1c_main_ref.log` (40000 frames) | 2026-10-18T22:17 | ec0df436c | 274 | DEATH_TABLE,GUNNERY_RNG_STREAMS | 1191 / 518 |
| IJN08 | `...-cc9-lua52\local\l70r_w_i08_main_ref.log` (100000 frames) | 2026-10-19T00:24 | ab2062aec | 222 | DEATH_TABLE,GUNNERY_RNG_STREAMS | 1371 / 536 |
| USN15 | `...-cc9-lua52\local\l70r_u15g_main_ref.log` (90000 frames) | 2026-10-19T00:53 | 9c2e8a30f | 221 | DEATH_TABLE,GUNNERY_RNG_STREAMS | 1198 / 488 |
| USNOS | `...-cc9-physics1\local\p23h_off_os.log` (30000 frames) | 2026-10-18T23:43 | 7f9207dc6 | 158 | DEATH_TABLE,GUNNERY_RNG_STREAMS | 1346 / 529 |
| LOMP06 | `...-cc9-lua52\local\l70r_w_l6_main_ref.log` (16000 frames) | 2026-10-18T23:50 | ab2062aec | 222 | DEATH_TABLE,GUNNERY_RNG_STREAMS | 1104 / 464 |
| JM06 | `...-cc9-physics1\local\p23c_off_j6.log` (40000 frames) | 2026-10-18T20:00 | 5b9d1b7d2 | 411 | DEATH_TABLE,GUNNERY_RNG_STREAMS | 1323 / 533 |

## 2. Method and caveats

* **Source of counts.** At exit, each log prints a host-method table,
  `  <name>  <address>  concrete|UNIMPLEMENTED  calls=N`, which `GameHostLog::record`
  (`src/game_hosts.cpp:279`) keeps. The census sums every `UNIMPLEMENTED` row across the 11 logs:
  672 distinct sites, all hit at least once. Rows whose name matches presentation, HUD, render,
  sound, front end, mission load or title code were set aside: 459 rows. The remaining 213 rows
  are gameplay candidates, listed in the appendix with missions, calls and the switches within 45
  lines above each host site.
* **A record is not always a gap.** `record()` / `unimplemented()` is often called after hosted
  code as a label: the call counts, the status stays `UNIMPLEMENTED`, and the work around it is
  done. Two cases cause this:
  1. The status is fixed on first insert by name (`src/game_hosts.cpp:281-301`). When a `done()`
     and a `record()` share a name, all calls are counted under whichever ran first. Examples:
     `BotStateFollow::station_keeping` (`done` when `kPlaneFollowLawBound`), and
     `Rack::drop_dispersion_006e4f91` (`done` at `game_hosts_gunnery.cpp:16278`).
  2. `calls` counts host invocations, often one per unit per tick, not binary effects. For example,
     `ReleaseTimer::reseed_bounds` records on every aim tick, but the skipped draw happens only on a
     timed release.

  Each top row was therefore read at its host site, and at the binary where needed, before it was
  classed.
* **Free-text census.** Lines in the logs containing "not modelled", "unimplemented", "stub",
  "record only", "substitution" or "stand-in", other than the table and its first-hit notes, come
  to under 100 distinct shapes per log. The gameplay ones are:
  the dive-bomb install note "approach+A8h (substituted)" (1131 lines, 7 missions; stale, the draw
  is bound by `kReleaseAltitudeDrawBound=ON`, `game_hosts_units.cpp:7949`); the squadron
  ordnance census "(stand-in N)" (a diagnostic that prints the old class stand-in next to the
  bound 007EDAD0 answer, `game_hosts_units.cpp:4579`); `returntobase 007F16D0 ... RECORD ONLY`
  (68 retreat resolutions, all in one mission; see row 17); and the profiler's QPC tick
  substitution (presentation). Every summary counter named `stub_*`, `*stand_in*`, `*substitut*`
  or `*unhosted*` reads 0 in all 11 logs.
* **`host_reference_unverified`** occurs in none of the 11 logs and in nothing under `src/` or
  `tools/`, so it contributes no rows.
* **RNG.** 00BD2F10 takes a stream id in ECX. `docs/CC13_RNG_STREAM_CENSUS.md` shows that the
  image's stream sequences are per thread and are reseeded by the movie camera, so the host cannot
  reproduce them. A skipped draw therefore matters for the value it would have produced, and only
  then for the host's own generator order.

## 3. Ranked sites (top of the census)

The rows are grouped by class and ranked within each class by missions x calls.

* **LIVE GAP**: the host skips or substitutes a binary effect on a normal gameplay path, and the
  effect changes unit state, a value drawn from 00BD2F10, or a scripted outcome. No switch covers
  it.
* **LIVE, SWITCH OFF**: the same, but an OFF switch that already carries the binary path exists.
* **SUBSTITUTION**: a labelled stand-in that is plausibly faithful in these runs, or whose effect is
  bounded.
* **MARKER**: the record labels hosted code, code that runs elsewhere, a field with no reader, or a
  network/presentation side effect.

| rank | binary site | host file:line | missions / calls | consequence | existing switch | LIVE |
|---|---|---|---|---|---|---|
| 1 | 009FA3A0 release timer: 009FA3EA reseed `00BD2F10(1, [t+4], [t+8])`, 009FA3FB notify `+0Dh`, 009FA3C5 `007BB110` | game_hosts_units.cpp:33112-33124 | 5 / 123,038 (BSM01 81,292; USN21 29,570) | Torpedo plane AI. After a timed release, the countdown is set to 0 instead of U(lo, hi). The stream-1 draw is not made, and `+0Dh` is never raised because `slot_byte_9c0_clear=false`. `007BB110` is replaced by "carries a torpedo and releases < 1" (SUBSTITUTION). Reported URGENT. | none | **yes** |
| 2 | 009D0E3A torpedo goaway enter: 009D0E71 `U(50,100)` climb jitter, 009D0EA3 `U(TorpFlikFlakTime1, 2)` window delay, both stream 1 | game_hosts_units.cpp:23499-23515 | 3 / 154 (BSM01 140) | Torpedo bomber GOAWAY. The climb jitter is pinned at 50, and the window delay is 0 (a HOLE, because PilotBotConfig is not loaded). The manoeuvre window opens on the first tick above +20h instead of after the authored delay. Two draws are missing per entry. Reported URGENT. | none | **yes** |
| 3 | 008BE7E0 Lua `HasFired` | mission_lua_host.cpp:452 (binding row only) | 1 / 577 (BSM01) | The script polls `HasFired(Mission.Henry, "DEPTHCHARGE", ...)` (`bsm_01_stationed_at_pearl.lua:813`, docs/CC13_OMITTED_TASKS_SCOPE.md). The unhosted native returns nothing, so the branch never sees a firing. This is objective logic. A standalone `lua_has_fired_008be7e0` exists (`include/bsp/unit_weapons.hpp:279`) but is not wired. | none | **yes** |
| 4 | 00893380 Lua `AddWaterLoad` | mission_lua_host.cpp:59 | 1 / 6 (BSM09) | Scripted flooding of a unit is not applied, which changes hull state and possibly a sinking. Standalone `lua_add_water_load` (`src/cc13_left55_seg56a.cpp:117`), unwired. | none | **yes** |
| 5 | 008914F0 Lua `BreakShip` | mission_lua_host.cpp:574 | 2 / 2 (BSM01, JM06) | The scripted break-up (00814520 when 0092BEA0 / 0092BE90 allow, docs/CC13_SEG56.md) does not happen, so the ship stays intact. Standalone `lua_break_ship` (`src/options_seg56_cc13.cpp:247`), unwired. | none | **yes** |
| 6 | 008A62A0 `SetCheatTurbo`, 008A13D0 `SetVisibility`, 008A0000 `SquadronEnableVehicleAvoidance`, 0089FAF0 `SquadronSetBaseUnsupported`, 008EE410 `AddPowerup`, 008A6C30 `UnitSetPlayerCommandsEnabled` | mission_lua_host.cpp:123 / 226 / 218 / 220 / 553 / 235 | BSM09 8+2+5+2; JM06 2; BSM01, BSM09, USN04 1 each | Unhosted scripted state changes. Each needs a per-script read; the avoidance native touches the struck cc9 range 007DF000-007DFFFF. | none | probably |
| 7 | 00729A80 `CanFire(1)` on the level-bomber rack (006E5816) | game_hosts_units.cpp:28109, 32488 | 6 / 1,445 | Level bomb, labelled SUBSTITUTION: "a rack can fire while it has ammo". Gates `+3B8h`, `+358h`, check-reload `+450h` / `+478h`, and per-barrel `+414h` are skipped. The drop cadence `rack_to_repeat` is modelled separately, so this is a gap only if one of those gates is live for a rack. Reported to the lead as possible, not proven. | none | unproven |
| 8 | 008AC2B0 Lua `RemoveUntouchableUnit` | game_hosts_lua.cpp:11249 summary; switch game_hosts_lua.hpp:806 | 1 / 5 (BSM09) | `AddUntouchableUnit` is ON (`kLuaAddUntouchableUnitBound`; BSM09 marks 5 units), but the clear is OFF, so those 5 units stay untouchable after the script releases them. This affects damage and the outcome. | `kLuaUntouchableClearBound` = OFF | **yes** |
| 9 | 009FCDB6-009FCE01 dogfight fire-branch re-aim (in 009FC7C0) | game_hosts_units.cpp:25174 note; switch cc13_fire_envelope.hpp:40 | 9 / 2,865,233 (per gun tick) | Fighter gunfire: with the switch OFF, the re-aim and the `+4Ah` clear are not modelled. | `kDogfightFireEnvelopeBound` = OFF | **yes** |
| 10 | 009C214A follow sight search | game_hosts_units.cpp:37584, 37590; switch :6118 | 4 / 25,035 | Wingman follow: the sight search is not run. | `kFollowSightSearchBound` = OFF | **yes** |
| 11 | 009C7900 / 009D3270 / 009CC400 hit-task notifies (divebomb / torpedo / strafe) | game_hosts_units.cpp:20275-20287; switch :6481 | 7 / 5,293; 5 / 4,335; 3 / 374 | The task hit clock is not run. | `kDiveHitClockBound` = OFF | **yes** |
| 12 | 007C17D0 takeoff enter, 008073C0 recon slot rebuild | game_hosts_units.cpp:16464, 16665, 36356; switch plane_land_chain_cc13.hpp:78 | 6 / 725 each | Plane flight-state notifications: `+AA0h` is carried as 0.0. | `kPlaneFlightStateNotifyCc13Bound` = OFF | **yes** |
| 13 | 008A7200 Lua `NavigatorForceTorpedo` | game_hosts_script_orders.cpp:320; switch game_hosts_script_orders.hpp:312 | 1 / 2 (JM06) | jm06.lua:2582: the two submarine shots after the convoy movie are not fired. Torpedo. | `kNavigatorForceTorpedoBound` = OFF | **yes** |
| 14 | 009F9E40 moveto steer-to-point | game_hosts_units.cpp:30010 | 7 / 247,511 | Plane moveto bearing: the body is unread, so the host writes its own bearing. LABELLED SUBSTITUTION on a hot plane path. | none | substitution |
| 15 | 009BDE80 moveto SetRanges (state+30h/+34h/+38h) at 009D48CF | game_hosts_units.cpp:22914 | 5 / 476,365 | Torpedo task: the three stores (aim altitude twice, glide distance) are not kept. 009C18C0's glide is hosted from the cruise inputs (`game_hosts_units.cpp:29984`). It is not verified that the torpedo glide uses the same values. The body is fully read (three stores, docs ledger). | none | unproven (torpedo) |
| 16 | 007F0280 near-field probe candidate walk | game_hosts_units.cpp:25327 | 7 / 148,665 | Formation near field: the probe is reconstructed, but its candidate list is the host's own unit walk (labelled). | none | substitution |
| 17 | 007F16D0 squadron return-to-base resolution | game_hosts_units.cpp:14459 | 6 / 127; 68 `retreat` (1 mission) | The squadron-level arm is computed and logged RECORD ONLY. Plane-level land and retreat tasks are installed by their own paths (`Bot::install_command_task_*`). Whether the squadron arm itself drives anything was not traced. | `kSquadronLandTaskBound` (include/bsp/game_hosts_units.hpp:211) | unproven |
| 18 | 009D0484 torpedo run profile row | game_hosts_units.cpp:31613 | 5 / 705 | Torpedo release altitude and distances use the SPNormal row because the difficulty index is unmodelled. They match at the default difficulty and differ by about 2x at other difficulties. | none | no (at default) |
| 19 | 008FB530 TorpedoBot FireTargetAccuracy | game_hosts_ship_ai.cpp:5999 | 3 / 13,476 | Values are taken from this installation's robots.lua by skill level, which is value-equivalent. | none | no |
| 20 | 009C1850 moveto desired speed | game_hosts_units.cpp:29922, 33244 | 7 / 321,246 | The speed is bound (`kMovetoSpeedBlendBound=ON`). Only the shaping is substituted. | partly bound | minor |
| 21 | 0099D300 pilot plan | game_hosts_units.cpp:39125 | 9 / 2,925,028 | Reconstructed in arms. The only labelled SUBSTITUTION is 0099DAC8's correction term (unit+B1Ch, +AE0h), which has no writer in the image and is taken as 0. The record is at the end of the hosted body. | many, ON | no |
| 22 | 0077C2A0 leak message 92h | game_hosts_units.cpp:3616 | 10 / 167 | The leak rates, including their stream-1 draws, are applied at once instead of through the session router. This is timing only. | none | minor |
| 23 | 009E465F nav-block draw `00BD2F10(0, 0, 1)` | game_hosts_ship_ai.cpp:12201-12204 | 11 / 490 (one per ship) | The record path runs because the draw source binds after the nav block is built. The draw is stream 0 (ECX=0, disk bytes 009E4644) and stores `-draw` in `blk+148h`, which has no traced reader. | `kShipAiTimerDrawsBound` = ON (inactive before bind) | no |
| 24 | 00432750 camera phase draws (3x stream 0) | game_hosts_hud.cpp:1427 | 9 / 51 | Presentation stream 0 (`xor ecx,ecx` at 0043294E / 0043296B). | none | no |
| 25 | 0090E6C0 unit-in-slot list push | game_hosts_gunnery.cpp:13083 | 10 / 891 | Pushes a unit id into a per-player-slot list (STL vector) when pending damage exceeds health. This is scoring or statistics; no gameplay reader is known. | none | no |
| 26 | 0077C7B0 submarine depth A2h message | game_hosts_units.cpp:20073 | 6 / 123 | The session message is not posted. The dive is modelled: motion reads `submarine_depth_level` at `game_hosts_units.cpp:11540` and feeds 00936DC0's band and target into the dive (008528B0's comment above :20073 is stale). | none | no |
| 27 | 00911E80 / 00912A60 / 00914390 score-record counters (named `BotScheduler::*`) | game_hosts_mission_frame.cpp:1324-1336 | 11 / 29,200 + 144,200 | MissionScoreRecord badge, achievement and category totals (ledger, packet cc9_side_ai_scheduler). Score only. Reconstructed in `src/cc13_seg59b.cpp`, unwired. | none | no (score) |
| 28 | 00758270 / 00749B20 / 00855420 / 006D2510 / 00846320 / 00953CC0 `UnitMotion::unreconstructed_*` | game_hosts_units.cpp `record_motion_phase` | 1-8 / up to 680,047 | Stale labels: their remainders run elsewhere. Air ops 006CDC70 runs from the deck tick (`game_hosts_mission_frame.cpp:1418`, cc9_deck_tick_in_step), the landing-ship tail 00749B20 is `kLandingShipLandedRemainderBound`, the submarine tail is `submarine_model`, shipyard 00846320 is `kShipyardProductionBound`, and 00953CC0's countdown is in gunnery. Not verified: 00758090 AirOpsHolder_Step. | various, ON | no |
| 29 | 00C5BB30 substep, `00cfc3d0+vtable114/50/140`, 00825F7C slot ring, 00605070, 00414DB0 pose refreshes | game_hosts_units.cpp:40628; game_hosts_ship_ai.cpp:2883, 11868, 11457, 12690, 3100, 3381 | 11 / 1-12.8 million each | MARKERS. These label hosted dispatch or hosted bodies. AutoTarget uses the recon triple (`kAutoTargetReconCandidatesBound=ON`, and `AutoTarget::party_recon_slot` never records). The slot-ring publish (`unit+0AECh` triple) has 12 readers whose callers are unknown and none on the steering path (docs/UNIT_AI_ORDER_SLOT_READER.md). | n/a | no |
| 30 | 007CE3A7 plane depth kill, 006E4F91 drop dispersion, 00923010 scene kill, 00932A16 no-gravity flag, 009BFEE0 station keeping, 0099AF53 queue release order | game_hosts_units.cpp:22294, 28161, 11314, 23614, 32073; game_hosts_mission_frame.cpp:557 | various | MARKERS. The effect is performed at the site, a same-name `done()` exists, or the "unimplemented" is the correct no-order branch. | n/a | no |

### What to build first (proposal; cc9 ranks)

1. Rows 1 and 2, the torpedo release timer and the torpedo goaway draws. They are values on the
   torpedo bomber path in 5 and 3 of the 11 missions. Both need the PilotBotParameters rows (the
   `[t+4]/[t+8]` bounds and TorpFlikFlakTime 1/2) loaded from this installation's
   `PilotBotConfig`. Put both behind a new OFF switch.
2. Row 8, `kLuaUntouchableClearBound`. It is already built. The asymmetry with the ON add-switch
   leaves BSM09 units invulnerable.
3. Rows 3 to 5, `HasFired`, `AddWaterLoad` and `BreakShip`. Standalone cc13 bodies exist; they
   need host wiring behind OFF switches. BSM01 and JM06 objectives are the affected surface.
4. Rows 9 to 13. The switches exist; they need pair runs on the missions listed.
5. Row 7, the level-bomb rack `CanFire`. First establish whether any `00729A80` gate is live for a
   rack.

## Appendix A. All 213 gameplay-candidate UNIMPLEMENTED rows

The switch column lists `k...Bound` identifiers found within 45 lines above the host site, with their
current values. These are hints only: proximity does not mean the switch covers that site.

| # | binary site | host record name | host file:line | missions / calls | switches seen within 45 lines above |
|---|---|---|---|---|---|
| 1 | 00c5bb30 | ShipMotion::rigid_body_substep_schedule | game_hosts_units.cpp:40628 | 11 / 12836261 |  |
| 2 | 00cfc3d0+vtable114 | ShipAi::unit_weapon_director | game_hosts_ship_ai.cpp:2883 | 11 / 12473865 |  |
| 3 | 00825f7c | ShipAiOrder::slot_to_order_ring | game_hosts_ship_ai.cpp:12690 | 11 / 12473848 |  |
| 4 | 00cfc3d0+vtable50 | ShipAi::drive_heading_vtable50 | game_hosts_ship_ai.cpp:11868 | 11 / 12469995 | kShipAiObstacleBackoffCountdownBound=ON |
| 5 | 00cfc3d0+vtable140 | AutoTarget::candidate_owner_vtable140 | game_hosts_ship_ai.cpp:11457 | 11 / 3723150 | kAutoTargetCommandAcceptBound=ON |
| 6 | 0099d300 | PilotBot::plan_controls | game_hosts_units.cpp:39125 | 9 / 2925028 |  |
| 7 | 009fc7c0 | BotTaskGun::tick | game_hosts_units.cpp:25273 | 9 / 2865233 | kDogfightFireEnvelopeBound=OFF kFighterAimTermsBound=ON kFighterGunLeadBound=ON kPlaneGunfireBound=ON |
| 8 | 00605070 | ShipAiControls::after_heading_stored | game_hosts_ship_ai.cpp:3100, game_hosts_ship_ai.cpp:3181 | 11 / 1302689 | kShipAiSetterPathResetBound=ON |
| 9 | 00414db0 | ShipAiFollow::refresh_world_pose | game_hosts_ship_ai.cpp:3381 | 10 / 1057639 | kShipFormationSpeedBound=ON |
| 10 | 00758270 | UnitMotion::unreconstructed_override_remainder_00758270 | game_hosts_units.cpp:39799/40037 (record_motion_phase) | 8 / 680047 |  |
| 11 | 00749b20 | UnitMotion::unreconstructed_override_remainder_00749b20 | game_hosts_units.cpp:39799/40037 (record_motion_phase) | 2 / 542083 |  |
| 12 | 009bde80 | BotStateMoveTo::refresh_ranges | game_hosts_units.cpp:22914 | 5 / 476365 |  |
| 13 | 009d4923 | Unit::device_requests_release | game_hosts_units.cpp:22933 | 5 / 476365 |  |
| 14 | 0099af53 | PilotBot::queue_release_order | game_hosts_units.cpp:32073 | 5 / 471372 |  |
| 15 | 009bfee0 | BotStateFollow::station_keeping | game_hosts_units.cpp:23614, game_hosts_units.cpp:23616 (+5) | 7 / 437367 | kPlaneFollowLawBound=ON kTorpedoDoneFormationBound=ON |
| 16 | 009c1850 | BotStateMoveTo::set_desired_speed | game_hosts_units.cpp:29922, game_hosts_units.cpp:33244 (+1) | 7 / 321246 | kDiveBombMoveToOriginBound=OFF kMovetoSpeedBlendBound=ON |
| 17 | 009c18c0 | BotStateMoveTo::glide_slope | game_hosts_units.cpp:29984, game_hosts_units.cpp:33304 (+1) | 7 / 321246 | kPitchCommandCallersBound=ON kTaskGunConeBound=ON |
| 18 | 006d2510 | UnitMotion::unreconstructed_phase_006d2510 | game_hosts_units.cpp:39799/40037 (record_motion_phase) | 5 / 313591 |  |
| 19 | 00846320 | UnitMotion::unreconstructed_phase_00846320 | game_hosts_units.cpp:39799/40037 (record_motion_phase) | 3 / 293591 |  |
| 20 | 00855420 | UnitMotion::unreconstructed_override_remainder_00855420 | game_hosts_units.cpp:39799/40037 (record_motion_phase) | 6 / 258382 |  |
| 21 | 009f9e40 | BotStateMoveTo::steer_to_point | game_hosts_units.cpp:30010, game_hosts_units.cpp:30012 (+1) | 7 / 247511 | kDiveBombMoveToOriginBound=OFF kPitchCommandCallersBound=ON kTaskGunConeBound=ON |
| 22 | 009fabe0 | BotStateMoveTo::direction_009fabe0 | game_hosts_units.cpp:27699, game_hosts_units.cpp:37014 (+3) | 8 / 226316 | kFlyToObstacleListBound=ON kNearFieldProbeBound=ON kTaskGunConeBound=ON |
| 23 | 00414db0 | ShipAiApproachPoint::refresh_unit_pose | game_hosts_ship_ai.cpp:4502 | 5 / 201171 |  |
| 24 | 009f1dbf | ShipAiApproach::frame_state_unread_spans | game_hosts_ship_ai.cpp:6508 | 5 / 201171 | kGunneryPassByte7dBound=ON kShipAiApproachLanderTermsBound=ON kShipAiApproachTargetLayerPushBound=ON kShipAiQueryGateBytesBound=ON |
| 25 | 00954940 | ShipAiApproach::scratch_00954940 | game_hosts_ship_ai.cpp:5300 | 5 / 200881 | kShipAiApproachLanderTermsBound=ON |
| 26 | 009f3314 | ShipAiApproach::sub_heading_command | game_hosts_ship_ai.cpp:7140 | 5 / 200881 |  |
| 27 | 00605070 | ShipAiApproach::wrap_brain_heading | game_hosts_ship_ai.cpp:7187 | 5 / 200881 |  |
| 28 | 009f339a | ShipAiApproach::sub_throttle_command | game_hosts_ship_ai.cpp:7151 | 5 / 200881 |  |
| 29 | 00932a16 | ShipHydro::set_body_no_gravity_flag | game_hosts_units.cpp:11314 | 2 / 195732 | kShipSinkDescentBound=ON |
| 30 | 009e80ab | ShipAiApproach::target_kind_probe_005c | game_hosts_ship_ai.cpp:5138 | 5 / 190042 | kAttackMoveApproachImageBound=ON |
| 31 | 00d09ec0+vtable140 | EntityCommand::endpoint_subject | game_hosts_commands.cpp:1000 | 11 / 176718 | kDirectorQueueCc13BodiesBound=ON |
| 32 | 0077c2a0 | WeaponDirector::route_set_command_message | game_hosts_commands.cpp:1750 | 11 / 176718 | kSetCommandMessageCc13Bound=ON kSetCommandQueueDelayBound=ON |
| 33 | 00cfc530+vtable114 | Session::dispatch_gameunit_message | game_hosts_commands.cpp:1751 | 11 / 176718 | kSetCommandMessageCc13Bound=ON kSetCommandQueueDelayBound=ON |
| 34 | 009e5aca | ShipAiMoveOnPath::brain_leg_scale_0308 | game_hosts_ship_ai.cpp:3685 | 6 / 170298 |  |
| 35 | 007f0280 | NearFieldProbe::probe | game_hosts_units.cpp:25327 | 7 / 148665 |  |
| 36 | 00914390 | BotScheduler::slot_prepare | game_hosts_mission_frame.cpp:1327 | 11 / 144200 | kWarningManagerTickBound=ON |
| 37 | 00836b95 | WeaponDirector::attackmove_arm_building_moveto_00836b95 | game_hosts_commands.cpp:3371 | 2 / 137194 |  |
| 38 | 009cfb04 | TakeoffTask::end_moveto_command_0071e430 | game_hosts_units.cpp:36303 | 6 / 128145 | kTakeoffTaskHeadBound=ON |
| 39 | 007bb110 | Unit::can_release_007bb110 | game_hosts_units.cpp:5906, game_hosts_units.cpp:33112 | 5 / 123038 | kGroundHeightHunksBound=ON kTorpedoRunTimeUpdateBound=ON |
| 40 | 009fa3ea | ReleaseTimer::reseed_bounds | game_hosts_units.cpp:33120 | 5 / 123038 | kTorpedoRunTimeUpdateBound=ON |
| 41 | 009fa3fb | Unit::slot_byte_9c0 | game_hosts_units.cpp:33123 | 5 / 123038 | kTorpedoRunTimeUpdateBound=ON |
| 42 | 009c1fd0 | BotTaskDogfight::follow | game_hosts_units.cpp:25789 | 4 / 77475 | kPlaneFollowLawBound=ON |
| 43 | 009fd050 | BotApproach::refresh_path_leg_009fd050 | game_hosts_units.cpp:34115 | 7 / 72577 | kLegAltitudeManagerBound=ON kMoveToLegAltitudeBound=ON kSquadronTravelAltBound=ON kTaskProfileActivationOnlyBound=ON |
| 44 | 00521ea0 | GameUnitMessage::resolve_target_object | game_hosts_commands.cpp:1069 | 11 / 55666 | kSetCommandClearAllMessageBound=ON |
| 45 | 009c18c0 | BotTaskDogfight::moveto_standin | game_hosts_units.cpp:26023 | 5 / 42699 | kDogfightMovetoSpeedBound=ON |
| 46 | 007cb7f0 | Plane::water_contact_007cb7f0 | game_hosts_units.cpp:22183, game_hosts_units.cpp:22231 | 8 / 38833 | kPlaneWaterContactGateBound=ON kPlaneWaterPowerLostBound=ON |
| 47 | 007ce3a7 | Plane::depth_kill_007ce3a7 | game_hosts_units.cpp:22294 | 1 / 37889 | kPlaneCrashSweepBound=ON kPlaneDepthKillBound=ON |
| 48 | 00911e80 | BotScheduler::think_a | cc13_seg59b.cpp:3751, game_hosts_mission_frame.cpp:1333 | 11 / 29200 | kWarningManagerTickBound=ON |
| 49 | 00912a60 | BotScheduler::think_b | cc13_seg59b.cpp:4028, game_hosts_mission_frame.cpp:1336 | 11 / 29200 | kWarningManagerTickBound=ON |
| 50 | 009a76e0 | BotStateDogfightAim::tick | game_hosts_units.cpp:25869 | 5 / 28041 | kDogfightAimAdoptsGunTargetBound=ON kDogfightThrottleBound=ON kSeg67bDogfightAimSpeedCorrOffBound=OFF |
| 51 | 007208a3 | WeaponDirector::session_trace_value | game_hosts_commands.cpp:2789 | 11 / 26383 |  |
| 52 | 009c214a | BotStateFollow::sight_search | game_hosts_units.cpp:37584, game_hosts_units.cpp:37590 | 4 / 25035 | kFollowReleaseArmBound=ON kFollowSightSearchBound=OFF kFollowTrailArmBound=ON |
| 53 | 0071fb90 | WeaponDirector::create_path_object | game_hosts_commands.cpp:2779 | 11 / 21732 |  |
| 54 | 006952a0 | WeaponDirector::unregister_target_observer | game_hosts_commands.cpp:2776 | 11 / 20905 |  |
| 55 | 00953cc0 | UnitMotion::unreconstructed_phase_00953cc0 | game_hosts_units.cpp:39799/40037 (record_motion_phase) | 1 / 19980 |  |
| 56 | 0075b430 | BotScheduler::retarget_begin | game_hosts_mission_frame.cpp:1324 | 11 / 18258 | kWarningManagerTickBound=ON |
| 57 | 00805680 | Recon::group_members_under_convoy_00805680 | game_hosts_gunnery.cpp:7730 | 11 / 18228 | kReconAggregatesBound=ON |
| 58 | 008069a0 | Recon::publish_group_level_008069a0 | game_hosts_gunnery.cpp:7731 | 11 / 18228 | kReconAggregatesBound=ON |
| 59 | 00414db0 | ShipAiLand::refresh_world_pose | game_hosts_ship_ai.cpp:10064 | 1 / 17302 |  |
| 60 | 0071c830 | WeaponDirector::build_set_command_message | game_hosts_commands.cpp:1748, game_hosts_commands.cpp:1785 | 1 / 15549 | kSetCommandMessageCc13Bound=ON kSetCommandQueueDelayBound=ON |
| 61 | 0087f9a0 | Landscape::on_hit_0087f9a0 | game_hosts_gunnery.cpp:10861 | 6 / 14437 | kLandscapeSpatialAttachBound=ON kTraceSceneDamageDrawBound=ON |
| 62 | 0089892c | ScriptEntity::place_vtable_98 | game_hosts_script_orders.cpp:5217 | 11 / 14103 | kScriptEntityPoolUnboundedBound=ON |
| 63 | 00898932 | ScriptEntity::call_00927610 | game_hosts_script_orders.cpp:5222 | 11 / 14103 | kScriptEntityPoolUnboundedBound=ON |
| 64 | 008fb530 | ShipAiTorpedoStandoff::torpedo_bot_accuracy_008fb530 | game_hosts_ship_ai.cpp:5999 | 3 / 13476 | kForeignTorpedoThreatHeadBound=ON |
| 65 | 00d214f4+vtable00 | ShipAiPlanner::release_node_list | game_hosts_ship_ai.cpp:8464 | 11 / 11270 |  |
| 66 | 00521ea0 | EntityOrder::resolve_target_object | game_hosts_commands.cpp:836 | 11 / 10725 | kSceneCommandFindCaseInsensitiveBound=ON |
| 67 | 0077c2a0 | EntityOrder::route_message | game_hosts_commands.cpp:1678 | 11 / 9387 |  |
| 68 | 00cfc530+vtable160 | Session::dispatch_entity_command | game_hosts_commands.cpp:1681 | 11 / 9387 | kSetCommandQueueDelayBound=ON |
| 69 | 006fe530 | EntityOrder::target_is_kind_of | game_hosts_commands.cpp:842 | 11 / 8871 |  |
| 70 | 009e1ac2 | ShipAiLand::brain_field_308 | game_hosts_ship_ai.cpp:10121 | 1 / 8432 |  |
| 71 | 00414db0 | WeaponDirector::refresh_target_pose | game_hosts_commands.cpp:2758 | 11 / 7305 |  |
| 72 | 008654ac | Gunnery::target_sub_entities_slot0fc | game_hosts_gunnery.cpp:6482 | 9 / 6394 | kAaSlotRangeOriginBound=ON |
| 73 | 00926110 | SEntity::InitAll pass B init_slot_a0 | game_hosts_lua.cpp:4851 | 11 / 6133 | kCarrierLaunchSkillBound=ON kGeneratedWingPartyBound=ON kPendingListDedupBound=ON |
| 74 | 009261a1 | SEntity::InitAll pass C init_slot_a4 | game_hosts_lua.cpp:4886 | 11 / 6133 | kCarrierLaunchSkillBound=ON kSEntityInitThisTableStepsBound=ON |
| 75 | 0109cefc+vtable0c | EntityThink::gc_gate_predicate | game_hosts_ready.cpp:202 | 11 / 6092 |  |
| 76 | 009fc1c0 | BotPathLegManager::remove | game_hosts_units.cpp:37366 | 2 / 5741 | kLegAltitudeManagerBound=ON kMoveToFollowBound=ON |
| 77 | 009c7900 | Bot::hit_task_notify_divebomb_009c7900 | game_hosts_units.cpp:20281 | 7 / 5293 | kDiveHitClockBound=OFF kStrafeTaskBound=ON |
| 78 | 006e00e3 | Entity::local_matrix_notice_310_vtable0c | game_hosts_units.cpp:41154 | 1 / 5032 |  |
| 79 | 0088a88e | LuaBinding::entity_object_id | game_hosts_script_orders.cpp:1450 | 11 / 4797 |  |
| 80 | 009bfee0 | BotStateDiveBombDone::station_keeping | game_hosts_units.cpp:30645 | 5 / 4483 | kPlaneFollowLawBound=ON |
| 81 | 009d3270 | Bot::hit_task_notify_torpedo_009d3270 | game_hosts_units.cpp:20275 | 5 / 4335 | kDiveHitClockBound=OFF kStrafeTaskBound=ON |
| 82 | 00835c70 | CommandController::begin_command | game_hosts_ship_ai.cpp:11268 | 11 / 4297 |  |
| 83 | 0099b650 | BotApproachLand::set_owner_0099b650 | game_hosts_units.cpp:37137 | 3 / 3651 | kLandAbortStateBound=ON kLandBeginStateBound=ON kPlaneTouchdownBound=ON |
| 84 | 009fc000 | BotPathLegManager::register | game_hosts_units.cpp:37360 | 1 / 3137 | kLegAltitudeManagerBound=ON kMoveToCircleSteerBound=ON kMoveToFollowBound=ON |
| 85 | 009fc260 | BotPathLegManager::altitude | game_hosts_units.cpp:37373 | 1 / 3136 | kLegAltitudeManagerBound=ON kMoveToFollowBound=ON |
| 86 | 00964790 | SceneUnit::vehicle_class_descriptor | game_hosts_scene_contents.cpp:1260 | 11 / 2835 | kVehicleClassPreloadCc15Bound=OFF |
| 87 | 004c1130 | SceneUnit::placement_deferred | game_hosts_scene_contents.cpp:1296 | 11 / 2835 | kVehicleClassPreloadCc15Bound=OFF |
| 88 | 004de610 | SceneUnit::world_parent_node | game_hosts_scene_contents.cpp:1302 | 11 / 2835 | kVehicleClassPreloadCc15Bound=OFF |
| 89 | 00928860 | SceneUnit::place_instance | game_hosts_scene_contents.cpp:1313 | 11 / 2835 |  |
| 90 | 00923010 | Entities::kill_vtable84 | game_hosts_mission_frame.cpp:557 | 10 / 2802 |  |
| 91 | 0090364e | WorldExpiry::destroy_entity | game_hosts_ready.cpp:349 | 10 / 2802 |  |
| 92 | 00748c40 | SceneUnit::create_stationary_instance | game_hosts_scene_contents.cpp:1282 | 9 / 2237 | kVehicleClassPreloadCc15Bound=OFF |
| 93 | 009e13b4 | CruiseState::helm_drive_arm | game_hosts_commands.cpp:4174 | 1 / 1889 |  |
| 94 | 006e4f91 | Rack::drop_dispersion_006e4f91 | game_hosts_gunnery.cpp:16278, game_hosts_units.cpp:28161 (+1) | 6 / 1848 | kLevelBombParatrooperDropBound=OFF kParatrooperRoundBound=OFF |
| 95 | 00465080 | AutoTarget::build_command_target | game_hosts_ship_ai.cpp:11417 | 8 / 1722 | kAutoTargetOverrideBound=ON kScriptFireTargetBound=ON |
| 96 | 00925a90 | SceneCommand::find_entity_by_name | game_hosts_commands.cpp:781 | 7 / 1675 |  |
| 97 | 009aa630 | BotTaskDogfight::select_target | game_hosts_units.cpp:24183 | 5 / 1480 | kOpportunisticRocketBound=ON |
| 98 | 00729a80 | Rack::can_fire_00729a80 | game_hosts_units.cpp:28109, game_hosts_units.cpp:32488 | 6 / 1445 | kLevelBombParatrooperDropBound=OFF kPlaneDeathModesBound=ON kPlaneLevelBombTaskBound=ON kRackRoundsPerRackBound=ON |
| 99 | 00d214f4+vtable00 | ShipAiSearch::release_path_node | game_hosts_ship_ai.cpp:8531 | 6 / 1352 |  |
| 100 | 00694a60 | ShipAiGoal::observer_register | game_hosts_ship_ai.cpp:7994 | 11 / 1246 | kShipAiMarkerTargetBound=ON |
| 101 | 00982120 | Plane::out_of_action_notice_00982120 | game_hosts_units.cpp:37073 | 1 / 1208 | kTaskGunConeBound=ON |
| 102 | 007c07a0 | BotStateLandBegin::direction_hold_007c07a0 | game_hosts_units.cpp:34955 | 3 / 904 | kPlaneDirectionHoldBound=ON |
| 103 | 0090e6c0 | DamageControl::pending_exceeds_health_0090e6c0 | game_hosts_gunnery.cpp:13083 | 10 / 891 |  |
| 104 | 00d21598+vtable08 | ShipAiState::exit_vtable08 | game_hosts_ship_ai.cpp:2965, game_hosts_ship_ai.cpp:4415 | 11 / 880 | kShipAiApproachEnterReseedBound=ON |
| 105 | 006952a0 | ShipAiGoal::observer_unregister | game_hosts_ship_ai.cpp:7991 | 10 / 865 | kShipAiMarkerTargetBound=ON |
| 106 | 007ac9d0 | EntityCommandArm::path_interface | game_hosts_commands.cpp:1542 | 10 / 790 | kEntityCommandSelfKindBound=ON kSceneFollowJoinBound=ON |
| 107 | 008073c0 | Recon::rebuild_slot_lists_008073c0 | game_hosts_units.cpp:16464 | 6 / 725 | kPlaneFlightStateNotifyCc13Bound=OFF |
| 108 | 007c17d0 | Plane::takeoff_enter_007c17d0 | game_hosts_units.cpp:16665, game_hosts_units.cpp:36356 | 6 / 725 | kGroundRetakeoffBound=ON |
| 109 | 008be7e0 | MissionLuaNative::HasFired | - | 1 / 577 |  |
| 110 | 006fe590 | SceneUnit::create_instance_from_descriptor | game_hosts_scene_contents.cpp:1276 | 11 / 576 | kVehicleClassPreloadCc15Bound=OFF |
| 111 | 00d21598+vtable04 | ShipAiState::enter_vtable04 | game_hosts_ship_ai.cpp:3032, game_hosts_ship_ai.cpp:3035 (+1) | 11 / 554 | kShipAiStateEnterBytesBound=ON |
| 112 | 0080e290 | RoleMessage::kind6_release_0080e290 | game_hosts_units.cpp:12985 | 7 / 497 |  |
| 113 | 009dfcb0 | ShipAiNavBlock::seed_steering_unprojected | game_hosts_ship_ai.cpp:12169 | 11 / 490 |  |
| 114 | 00bd2f10 | ShipAiNavBlock::uniform_00bd2f10 | game_hosts_ship_ai.cpp:12201, game_hosts_ship_ai.cpp:12204 | 11 / 490 | kShipAiTimerDrawsBound=ON |
| 115 | 0079cd36 | PlaneClass::issue_threshold_a0 | game_hosts_units.cpp:22521 | 6 / 480 | kDiveBombCarriedRoundsBound=ON kReleaseIssueStageValsBound=ON |
| 116 | 006e4d50 | Plane::release_spawn_deferred_to_rack | game_hosts_units.cpp:3768, game_hosts_units.cpp:5766 | 6 / 477 | kBayRequestNoForceEnableBound=ON kReleaseIssueStageBound=ON kReleaseIssueStageValsBound=ON |
| 117 | 00694a60 | WeaponDirector::register_target_observer | game_hosts_commands.cpp:2773 | 10 / 411 |  |
| 118 | 009cc400 | Bot::hit_task_notify_strafe_009cc400 | game_hosts_units.cpp:20287 | 3 / 374 | kDiveHitClockBound=OFF kStrafeTaskBound=ON |
| 119 | 007c0d90 | Plane::droppable_device_walk | game_hosts_units.cpp:22391, game_hosts_units.cpp:32741 | 2 / 341 | kLevelBombParatrooperDropBound=OFF kPlaneDeviceWalkPerRackBound=ON kPlaneLevelBombTaskBound=ON kReleaseIssueStageBound=ON |
| 120 | 0046c450 | SceneGate::append_deferred_entity_record | game_hosts_scene_contents.cpp:1203 | 11 / 336 |  |
| 121 | 0077c904 | Formation::entity_route_slot | game_hosts_script_orders.cpp:3561 | 10 / 327 |  |
| 122 | 00469610 | SceneUnit::queue_entity_command | game_hosts_scene_contents.cpp:1340 | 11 / 320 |  |
| 123 | 0077bde4 | UnitGroup::leave_director_notice_vtable114 | game_hosts_units.cpp:41194 | 8 / 207 |  |
| 124 | 0070e548 | UnitGroup::observer_unregister_00694a60 | game_hosts_units.cpp:41228 | 8 / 207 |  |
| 125 | 009c7240 | BotStateDiveBombDone::enter | game_hosts_units.cpp:30592 | 5 / 205 |  |
| 126 | 006e6670 | WarningManager::loss_handle_006e6670 | game_hosts_mission_frame.cpp:2446 | 10 / 183 |  |
| 127 | 0097b8c0 | WarningManager::kill_channel_0097b8c0 | game_hosts_mission_frame.cpp:2450 | 10 / 183 | kWarningMessageBankCc15Bound=ON |
| 128 | 0077edf0 | WarningManager::loss_teardown | game_hosts_mission_frame.cpp:2453 | 10 / 183 | kWarningMessageBankCc15Bound=ON |
| 129 | 0092ced0 | Parts::detach_part_message_99h_0092ced0 | game_hosts_gunnery.cpp:12287 | 9 / 182 |  |
| 130 | 00928c80 | Entity::on_killed_lua_self_00928c80 | game_hosts_units.cpp:40500, game_hosts_units.cpp:42505 | 10 / 175 | kShipTorpedoBoatKindBound=ON kSunkHullTerrainMaskBound=ON |
| 131 | 0092bd30 | UnitController::clear_hull_shape_fields | game_hosts_units.cpp:40516, game_hosts_units.cpp:42513 | 10 / 175 |  |
| 132 | 00824b60 | EntityQueues::wreck_handler_vtable7c | game_hosts_ready.cpp:595 | 10 / 167 | kDeadMemberLeavesGroupBound=ON kSunkShipFlushBound=ON |
| 133 | 0077c2a0 | LeakManager::route_ship_sink_92h | game_hosts_units.cpp:3616 | 10 / 167 |  |
| 134 | 00cfc3d0+vtable50 | CruiseCommand::unit_heading | game_hosts_commands.cpp:1364 | 10 / 157 | kAttackPushEndsOverrideBound=ON |
| 135 | 008ab070 | MissionLuaNative::GetSelectedUnit | - | 1 / 157 |  |
| 136 | 00997c68 | PilotBotParameters::TorpFlikFlakTime | game_hosts_units.cpp:23513 | 3 / 154 |  |
| 137 | 0093a2c0 | Failure::section_state_message_0093a2c0 | cc13_segmix.cpp:184, game_hosts_gunnery.cpp:12857 | 9 / 130 |  |
| 138 | 00982c50 | Failure::warning_manager_00982c50 | game_hosts_gunnery.cpp:12860 | 9 / 130 |  |
| 139 | 00913d80 | Failure::statistics_00913d80 | cc13_seg59b.cpp:1813, game_hosts_gunnery.cpp:12863 | 9 / 130 |  |
| 140 | 009db590 | ShipAiAttack::initial_enter_009db590 | game_hosts_ship_ai.cpp:4458 | 6 / 129 | kShipAiApproachEnterReseedBound=ON kShipAiEngageSubStateBound=ON |
| 141 | 007f16d0 | Squadron::resolve_return_to_base_007f16d0 | game_hosts_units.cpp:14459 | 6 / 127 | kSquadronLandTaskBound=ON |
| 142 | 0077c7b0 | SubmarineUnit::depth_message_a2 | game_hosts_units.cpp:20073 | 6 / 123 |  |
| 143 | 0099a45b | Bot::install_command_task_retreat_arm | game_hosts_units.cpp:9732 | 1 / 118 |  |
| 144 | 009f3061 | ShipAiApproach::mode3_brain_field_308 | game_hosts_ship_ai.cpp:6703 | 1 / 109 |  |
| 145 | 0099a3dd | Bot::install_command_task_land_arm | game_hosts_units.cpp:14593 | 5 / 106 |  |
| 146 | 006c4790 | AirOpsBlock::squadron_not_excluded_006c4790 | game_hosts_units.cpp:14594 | 5 / 106 |  |
| 147 | 008aba51 | RoleOwner::set_role_available | game_hosts_script_orders.cpp:3702 | 6 / 102 | kHullRepairEnableRouteBound=ON kPlayerRoleBookkeepingBound=ON |
| 148 | 009a7650 | BotTaskDogfight::set_target | game_hosts_units.cpp:24100 | 4 / 79 | kDogfightAimAdoptsGunTargetBound=ON kOpportunisticRocketBound=ON |
| 149 | 00e188d8 | Game::local_player_unit | game_hosts_script_orders.cpp:5412 | 11 / 68 | kAfterRow9OrderQueueBound=ON |
| 150 | 0077c2a0 | Failure::clear_message_0077c2a0 | game_hosts_gunnery.cpp:12887 | 9 / 66 |  |
| 151 | 006cfa41 | Plane::tick_node_advance_zero_006cfa41 | game_hosts_units.cpp:16326 | 1 / 66 |  |
| 152 | 007c1680 | Plane::five_to_four_007c1680 | game_hosts_units.cpp:15621, game_hosts_units.cpp:15623 | 1 / 63 | kPlaneFlightStateNotifyCc13Bound=OFF |
| 153 | 008c8f70 | MissionLuaNative::PrepareClass | - | 10 / 55 |  |
| 154 | 00959dcc | PlayerGunSeat::flak_0084c500 | game_hosts_gunnery.cpp:13890 | 1 / 40 |  |
| 155 | 0088fe30 | MissionLuaNative::SetNumbering | - | 2 / 36 |  |
| 156 | 008199c7 | Failure::engine_jam_flag_008199c7 | game_hosts_gunnery.cpp:12979 | 8 / 32 | kShipDamageControlTickBound=ON |
| 157 | 0097e360 | MissionEvents::command_event_block | game_hosts_commands.cpp:3138 | 6 / 32 |  |
| 158 | 00882ac0 | Landscape::load_terrain | game_hosts_scene_contents.cpp:1893 | 9 / 28 | kSceneLandscapeTerrainBound=ON |
| 159 | 00883bb0 | Landscape::attach_terrain_vcall_9c | game_hosts_scene_contents.cpp:1894 | 9 / 28 | kSceneLandscapeTerrainBound=ON |
| 160 | 009c1ffd | BotStateFollow::release_arm | game_hosts_units.cpp:37560 | 1 / 24 | kFollowReleaseArmBound=ON kFollowSightSearchBound=OFF |
| 161 | 007f2c60 | SceneUnit::create_squadron_instance | game_hosts_scene_contents.cpp:1288 | 7 / 22 | kVehicleClassPreloadCc15Bound=OFF |
| 162 | 00815e7c | UnitGroup::leader_offset_fa0_00815e20 | game_hosts_units.cpp:3317 | 6 / 20 |  |
| 163 | 009064b5 | MissionScoring::notify_entities | game_hosts_mission_result.cpp:449 | 6 / 20 |  |
| 164 | 0088c750 | MissionLuaNative::BannSupportmanager | - | 6 / 20 |  |
| 165 | 008b8ca0 | MissionLuaNative::Scoring_SetRanking | - | 4 / 18 |  |
| 166 | 008cb920 | MissionLuaNative::Scoring_GrantUnlock | - | 4 / 18 |  |
| 167 | 0081beb0 | ShipUnit::exit_zone_party_lua_0081beb0 | game_hosts_units.cpp:20877 | 2 / 16 | kShipExitZoneRoleGateBound=ON |
| 168 | 009b1e84 | BotStateLandFinal::enter_844 | game_hosts_units.cpp:35053 | 3 / 14 | kTaskGunControllerAllTasksBound=ON |
| 169 | 007cb5f0 | Plane::touchdown_007cb5f0 | game_hosts_units.cpp:15778 | 2 / 13 | kCarrierDeckParentBound=ON kPlaneFlightStateNotifyCc13Bound=OFF |
| 170 | 004c1130 | MissionLua::on_frame_job_thread | game_hosts_lua.cpp:11579 | 11 / 12 | kEntryPointIsFunctionBound=ON |
| 171 | 006b8720 | MissionLua::set_panic_function | game_hosts_lua.cpp:11375 | 11 / 11 | kGetPropertySquadronStateBound=ON kGetPropertyTorpedoStockBound=ON |
| 172 | 004f0ad0 | SceneUnit::create_plane_squadron_gen | game_hosts_ai.cpp:6600 | 11 / 11 |  |
| 173 | 007f2c60 | PlaneSquadron::construct | game_hosts_ai.cpp:6601 | 11 / 11 |  |
| 174 | 007f4580 | PlaneSquadron::attach_planes | game_hosts_ai.cpp:6602 | 11 / 11 |  |
| 175 | 00a32350 | AiController::create | game_hosts_ai.cpp:6449 | 11 / 11 | kAiOwnerPlayerSlotBound=ON |
| 176 | 00a31730 | AiController::construct | game_hosts_ai.cpp:6450 | 11 / 11 | kAiOwnerPlayerSlotBound=ON |
| 177 | 0090fa40 | MissionScoring::reset_tail_0090fa40 | game_hosts_mission_frame.cpp:2912 | 11 / 11 | kBotSchedulerWritersBound=ON |
| 178 | 008a8a83 | LuaBindingCore::entity_party_query_vtable_5c | game_hosts_script_orders.cpp:467 | 11 / 11 | kScriptEntityPoolUnboundedBound=ON kSkillLevelBound=ON |
| 179 | 00761310 | ShipDeath::breakup_message_98_00761310 | game_hosts_gunnery.cpp:13496 | 5 / 10 | kLandMasterDamageBound=ON kShipBreakInTwoDrawsBound=ON kShipDeathMessageBound=ON |
| 180 | 0072d5b0 | PlayerGunSeat::role_take_fire_mode | game_hosts_gunnery.cpp:14307 | 2 / 10 | kFlakSecondAmmoCadenceBound=ON |
| 181 | 007c71e0 | Plane::touchdown_reparent_007c71e0 | game_hosts_units.cpp:17215 | 1 / 9 |  |
| 182 | 007cc7a0 | Plane::enter_state_two_007cc7a0 | game_hosts_units.cpp:16169, game_hosts_units.cpp:16171 | 1 / 9 | kHitIndexDetachBound=ON kPlaneFlightStateNotifyCc13Bound=OFF kPlaneWingFoldElevatorBound=ON |
| 183 | 008ad4e8 | UnitInstance::set_repair_enabled_field | game_hosts_script_orders.cpp:3654 | 1 / 9 | kSkillLevelBound=ON |
| 184 | 004a4520 | TrafficConfig::launch_pad_troops | game_hosts_ship_ai.cpp:6330, game_hosts_ship_ai.cpp:13807 (+1) | 1 / 8 |  |
| 185 | 0080e490 | ShipAiLand::enter_tail_0080e490 | game_hosts_ship_ai.cpp:3004 | 1 / 8 | kShipAiKamikazeAttackStepBound=ON kShipAiLandStepBound=ON |
| 186 | 008a62a0 | MissionLuaNative::SetCheatTurbo | - | 1 / 8 |  |
| 187 | 0064a8e0 | EntityCommandArm::follow_tail | game_hosts_commands.cpp:1527 | 1 / 7 | kEntityCommandSelfKindBound=ON kSceneFollowJoinBound=ON |
| 188 | 00746566 | LandFort::fire_sync_message_d1 | game_hosts_ship_ai.cpp:13165 | 2 / 6 |  |
| 189 | 00893380 | MissionLuaNative::AddWaterLoad | - | 1 / 6 |  |
| 190 | 00904bf0 | MissionCompletion::world_final_tick | game_hosts_mission_frame.cpp:1748 | 1 / 6 | kSunkShipFlushBound=ON |
| 191 | 008a0000 | MissionLuaNative::SquadronEnableVehicleAvoidance | - | 1 / 5 |  |
| 192 | 008ac2b0 | MissionLuaNative::RemoveUntouchableUnit | game_hosts_lua.cpp:7900 | 1 / 5 |  |
| 193 | 00984eb0 | AirOps::stock_notice | game_hosts_lua.cpp:7395, game_hosts_lua.cpp:7442 | 1 / 4 |  |
| 194 | 007f31a0 | PlaneSquadron::on_plane_left_map_007f31a0 | game_hosts_units.cpp:9856 | 1 / 4 | kFollowSightSearchBound=OFF |
| 195 | 007c16f0 | Plane::four_to_five_007c16f0 | game_hosts_units.cpp:15608 | 1 / 4 | kPlaneFlightStateNotifyCc13Bound=OFF |
| 196 | 0095ba60 | VehicleClass::mark_party_preload | game_hosts_lua.cpp:7385 | 1 / 3 |  |
| 197 | 008a6c30 | MissionLuaNative::UnitSetPlayerCommandsEnabled | - | 3 / 3 |  |
| 198 | 008affc0 | MissionLuaNative::EndSCore | - | 3 / 3 |  |
| 199 | 0074b0a0 | LandingShip::route_ramp_message_0a6 | game_hosts_ship_ai.cpp:13423 | 1 / 2 | kLandingShipRampBound=ON kLandingShipRampHullContactBound=ON |
| 200 | 006ac370 | LandingShip::pad_troop_paths_006ac370 | game_hosts_ship_ai.cpp:13436 | 1 / 2 | kLandingShipRampBound=ON kLandingShipRampHullContactBound=ON kLandingShipUnloadBound=ON |
| 201 | 00986820 | MissionEvents::ship_landed_00986820 | game_hosts_ship_ai.cpp:13437 | 1 / 2 | kLandingShipRampBound=ON kLandingShipUnloadBound=ON |
| 202 | 0089faf0 | MissionLuaNative::SquadronSetBaseUnsupported | - | 1 / 2 |  |
| 203 | 008a13d0 | MissionLuaNative::SetVisibility | - | 1 / 2 |  |
| 204 | 00835860 | EntityCommandArm::set_fire_target | game_hosts_commands.cpp:1549 | 1 / 2 | kSceneFollowJoinBound=ON |
| 205 | 008914f0 | MissionLuaNative::BreakShip | - | 2 / 2 |  |
| 206 | 008ee410 | MissionLuaNative::AddPowerup | - | 1 / 2 |  |
| 207 | 008a7200 | MissionLuaNative::NavigatorForceTorpedo | - | 1 / 2 |  |
| 208 | 006bd520 | AirOps::slot_cleared_notify_and_message | game_hosts_lua.cpp:7440 | 1 / 1 |  |
| 209 | 008cc330 | MissionLuaNative::Scoring_IgnoreEntityKill | - | 1 / 1 |  |
| 210 | 008c3fe0 | MissionLuaNative::Scoring_AddPartyLoss | - | 1 / 1 |  |
| 211 | 008aca30 | MissionLuaNative::LoadCheckpoint | game_hosts_lua.cpp:10460 | 1 / 1 |  |
| 212 | 008eb350 | MissionLuaNative::GetAvailablePowerups | - | 1 / 1 |  |
| 213 | 008acb70 | MissionLuaNative::SaveCheckpoint | game_hosts_lua.cpp:10496 | 1 / 1 |  |
