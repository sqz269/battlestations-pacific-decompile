# CC13_VERIFY16: objective-relevant paths of the mission and HUD hosts

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify16 on 2026-10-17 against
base 08a793150.

**Scope.** It covers `src/game_hosts_mission.cpp`, `game_hosts_mission_frame.cpp`,
`game_hosts_mission_result.cpp`, `game_hosts_hud.cpp` and `game_hosts_hud_world.cpp`, and their
headers. Within them it judges only what a mission script can read or what decides the mission end:
- objectives, win and loss;
- checkpoints, timers and counts;
- the scoring flow and the mission-end flow.

Pure presentation is skipped unless a script reads its value.

No code was changed and no ledger records were added. Line numbers are against the base.

**Sources.**
- "Image" means battlestationspacific.exe, read with `bsp.py show` and `--asm`.
- Mission scripts are this installation's `scripts/` tree, which is modded (see the memory note on
  BSPRM/AlterBSP).
- Call counts come from `local/cc13-verify16_uses.py`. It strips `--` line comments and non-nesting
  `--[[ ]]` blocks and counts calls not preceded by `.`, `:` or an identifier character.
- "SP" excludes `missions/multi/` and `missions/traininggrounds/`.

**Test rows.** Each scene's `StageScript` resolves to one script:

| Row | Script |
| --- | --- |
| JM08 | `COTP-IJN/PRCPIJN/prcpjm08.lua` |
| USN01 | `USN/usn_1_marshall.lua` |
| USN04 | `USN/usn_19_coralus.lua` |
| USN13 | `USN/usn_13_truk.lua` |
| LOMP10 | `USN/LOMP/10_san_jose.lua` |

`COTP-USN/usn_01_*` and `COTP-USN/usn_04_*` are different missions; they are not the row scripts.

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, width with no effect, ordering within a step, presentation,
  or RNG draw count only.
- MISMATCH REAL: a wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but a bound arm or another host
  already does what the image does.
- UNRESOLVED.

## What scripts can read from these five files

The five files register no Lua native. Every native is a row of `src/mission_lua_host.cpp`, answered by
`game_hosts_lua.cpp`, `game_hosts_script_orders.cpp` or `game_hosts_commands.cpp`. A row no host answers
is recorded as `MissionLuaNative::<name> UNIMPLEMENTED` and pushes nothing, so the script reads `nil`
(`game_hosts_lua.cpp:2845-2867`).

What a script reaches of the HUD and mission hosts:
- `GetSelectedUnit` (the controlled unit, `game_hosts_lua.cpp:9159-9174`) and `GetCameraState`
  (`game_hosts_script_orders.cpp:4278`). Both are answered.
- `GetDifficulty`, which reads `game+6ACh` (set by `game_hosts_mission.cpp:1127-1135`).
- `IsGUIActive`, `IsUnitSelectable`, `IsHintActive` / `IsHintCritical`, `IsPlayerControlled`,
  `GetRoleAvailable`. None of these is answered.
- The scoring and end natives (`Scoring_*`, `PlayBinkMovie`, `EndScene`).
- The trigger natives (`AddProximityTrigger`, `AddLineOfSightTrigger`, `AddSquadOrdnanceDroppedTrigger`,
  `RemoveTrigger`).

So this pass took two parts.

**Part 1** is the labelled substitutions and records in the five files. It used the VERIFY5 pattern plus
`not modelled|not reconstructed|unbound`, all `unimplemented(` and `record(` sites, and the switches.
Every switch in these files and headers is ON.

**Part 2** is a census of binding rows no game host answers, filtered to those that decide mission flow.
- `local/cc13-verify16_census.py` lists rows whose quoted name or `0x`-address appears in no
  `src/game_hosts*.cpp` or `include/bsp/game_hosts*.hpp`. That gives 207 rows with at least one
  script call.
- It was cross-checked against the `concrete` rows of 300 recent run logs, and none of the 207 appears
  there as concrete.
- The UNIMPLEMENTED rows of 810 run logs dated after 2026-10-15, across all trees, confirm which rows
  are reached.
- A named row can still be unanswered: `PlayBinkMovie` is named only by the harness injection. The
  census therefore under-counts. It does not over-count.
- `local/cc13-verify16_nilops.py` finds uses whose `nil` result raises a Lua error, through arithmetic,
  ordering, `..` or indexing.

Items already in earlier passes are not repeated: TrulyDead, IsInBorderZone, SquadronSetReleaseAlt,
LandAndKill, the checkpoint pair, TorpedoStock, NavigatorForceTorpedo, Remove/IsUnitUntouchable,
SetCatapultStock, GetSubmarineDepthLevel, Scoring_GetPlayerShotDown, and the event channels.
`docs/CC13_WIN_READINESS.md` already lists GetCommandBuildingLevel and Scoring_SetMissionCompleted as
unbound, both as not blocking. R4 below revises the first of those.

## Summary: MISMATCH REAL (each one sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 (URGENT) | AddProximityTrigger 008C9730 and RemoveTrigger 008C5FD0 unanswered | 008C9730 -> 007505C0 BSP_ProximityTrigger_Create, callback name 00750790 | 151 calls in 42 files (89 SP). COTP-USN usn_04_defend_guadalcanal: convoy, carrier-group and evac triggers. jm13 PrisonRecon; jm02; usn_03/05/06/09/10/11/12/14. No test row | none |
| R2 (URGENT) | SetDeadMeat 008AC7B0 unanswered | plane: vt+194h("spinning"); ship: one stream-1 draw (008AC94B CALL 00BD2F10, ECX=1), then sink 008110F0; else vt+70h(1) | USN04 test row `:1689` (Shoho fleet at phase 3), `:1243-1285` (Lexington-lost path). luaSurrender. 44 calls in 21 files | image 1 draw per ship on stream 1; host none |
| R3 (URGENT class) | IsUnitSelectable 008AB580 unanswered, although `GameHudHost::unit_selectable_00645060` (`game_hosts_hud.cpp:2006`) evaluates it | 008AB580 -> 00645160 -> 00645060, PushBoolean | usn_11 / prcp_11 primary 4 (with R5); jm05 event recon; jm02 Nell/Betty targeting; 81 calls in 23 files. No test row | none |
| R4 (URGENT) | GetCommandBuildingLevel 0089B6D0 unanswered | PushInteger([entity+770h]) | JM05 (`PRCPIJN/jm05.lua:2779`): `nil >= 2` errors on every pass of luaJM5CheckObjectives. The rest of the function never runs: secondary 1 and the primary 1/2 HiddenFailed checks | none |
| R5 (URGENT) | GetEntitySpeed 008C0820 unanswered | pushes the entity's speed | usn_11 / prcp_11 CheckMontR1..R10 (primary 4); jm05 `:1341`; jm12. The `nil < 30` error is reached once R3 is fixed | none |
| R6 (URGENT) | ArtilleryEnable 0089C590 unanswered (AAEnable and TorpedoEnable are answered) | 0071DFD0(flag): session message 5Ah, sub-kind 3 | usn_09 `:9-:11` Fortress1-3 disabled at load; usn_05 (12); jm09 StageTrg; jm01/03, bsm_02. 63 SP calls. No test row | host-only shots from guns the script disabled |
| R7 | Scoring_SetMissionCompleted 008B8AD0 unanswered; stale label `game_hosts_mission_result.cpp:21-24` | 00906460: record +0h = flag, +10h = clock on the 0->1 edge, then 00927F60 for every entity with +180h <= 7 | every SP mission end (commandhelpers `:10372` failed, `:10431` completed) | none |
| R8 | PlayBinkMovie 0089A480 unanswered; stale label `game_hosts_mission_result.cpp:12-16`. The body `play_bink_movie_0089a480` (`:227`) is reached only from the harness switch (`game_hosts_mission_frame.cpp:3657-3667`) | 0089A480 reads arg 0 and arg 2 (debrief), then 0089A390 -> 004CD390, and EndScene 004D7970 when arg 2 is true | commandhelpers luaMissionEnd_Movie `:10926`, luaMissionCompleted_ComplMovie `:10111`, luaMissionFailed_FailMovie `:9854`; UNIMPLEMENTED in 31 run logs | none |

R7 and R8 do not change whether a mission completes or fails. The Lua side has already decided that
(`Mission.MissionStatus`) when they run. They change the scoring record and whether the run leaves state
0Dh.

### R1 AddProximityTrigger is unanswered

**Image.** 008C9730 reads five values: the entity (00888AA0), the trigger name (008C9350), the callback
(00750790), the target (0088A810) and the distance. It then creates the trigger through 007505C0
(seg 41). The trigger calls the named Lua function when the entity comes within range of the target.

**Host.** No game host names the row. A standalone body exists
(`lua_add_proximity_trigger`, `src/cc13_left55_seg56c.cpp`), but nothing wires it. RemoveTrigger
(008C5FD0, 46 calls) is unanswered too.

**Effect in this installation:**
- COTP-USN `usn_04_defend_guadalcanal.lua`:
  - `:754`, `:757`, `:775`, `:1858`, `:3138`: the convoy and the Japanese carrier group never get past
    their navpoints.
  - `:1938`: carrier members reaching the evac zone are never `Kill()`ed (`:1881-1885`).
  - `:2434`: PTs never head for the border (`:2464-2466`).
  - `:1550`: the recon plane never reaches home.
- jm13 `:2812` PrisonRecon (a GUI rename only) and `:3251` (the paratrooper drop).
- jm02 / prcpjm02 `:1123`.
- usn_03, 05, 06 (8), 09, 10 (9), 11, 12, 14, and prcp_06 (8).

commandhelpers' two calls are in `luaStartConvoy` `:12370`, which nothing calls.

**Same family, also unanswered:**
- AddSquadOrdnanceDroppedTrigger 008C9A10: bsm_04 `:1150`, `:1174`, `:1440` (attacker retreat) and
  USNRM `usn_1_pearl.lua:1507`. commandhelpers uses it only in the cheat reloader.
- AddLineOfSightTrigger 008C9460: bsm_03 (5, dialog) and bsm_05 `:951`.

### R2 SetDeadMeat is unanswered

**Image** (008AC8C4..008AC94B):
- `vt+5Ch(0Fh)` true: `vt+194h("spinning", 0)`.
- Otherwise, `vt+5Ch(6)` true (a ship): FLD1, then FLD [00D7A260], then `CALL 00BD2F10` with ECX = 1,
  then the sink 008110F0.
- Otherwise: `vt+70h(1)`.

**Host.** Nothing happens, so the unit stays alive. `game_hosts_gunnery.cpp:13077-13080` already records
that 008110F0's only callers are `Sink` and `SetDeadMeat`. A standalone body exists in
`src/options_seg56_cc13.cpp`, not wired.

**USN04 test row:**
- `luaZuikakuMovieEnd` `:1941-1943` calls `luaRetreatShohoFleet` `:1678`. That function puts each live
  Shoho-fleet ship 40 km north (`PutTo`, z + 40000) with invincibility 0 and calls SetDeadMeat
  (`:1689`).
- In the image those ships sink there. On the host they stay alive, keep their orders, and remain in
  every party-wide unit table.
- Secondary 1 is failed explicitly right after (`:1694-1697`), so that objective does not move. Whether
  the survivors matter to phase 3 depends on how far the run gets.
- `:1243`, `:1257`, `:1271`, `:1285` (`luaLexDeadMovie` `:1223`) do the same to all four fleets on the
  Lexington-lost path.

### R3 IsUnitSelectable is unanswered

**Image.** 008AB580 runs 00645160 (EDX = [game+18ECh]), then 00645060, then PushBoolean.

**Host.** The HUD host already models 00645060's inputs (`game_hosts_hud.cpp:1497-1535`) and the
predicate (`:2006-2011`), but the Lua row does not reach them. With `nil`, `if IsUnitSelectable(u)` is
never taken and `not IsUnitSelectable(u)` always is.

**Effects** (JM05 is `PRCPIJN/jm05.lua`):
- JM05 `:1341`: the captured-PT switch never fires.
- JM05 `:4267`, `:4302`: an event navpoint is never "reconned", so `reconned == 3` never succeeds and
  the event times out.
- jm02 `:1789`, `:1810`: every newly spawned Nell or Betty is sent at Repulse or Prince of Wales.
- usn_11 and prcp_11 `CheckMontR1..R10`: the primary-4 rescue points never clear (see R5).
- chg_5_tulagi `:967-1243`, usn_09 (tutorial), jm01, jm04, jm12.

### R4 GetCommandBuildingLevel errors inside JM05's objective check

**Image.** 0089B6D0 pushes `[entity+770h]` as an integer.

**Host.** Pushes nothing.

**Effect.** In `luaJM5CheckObjectives` (`PRCPIJN/jm05.lua:2779`; COTP-IJN `:2596`; ijn/JM `:2582`),
once primary 4 has succeeded and the RadarStation is Japanese, `GetCommandBuildingLevel(...) >= 2`
raises "attempt to compare nil with number" on every pass. Everything after it in the function is
skipped:
- secondary 1 (`luaJM5DisabledBunkers` -> `luaObj_Completed`);
- the primary 1 and 2 `HiddenFailed` checks.

`docs/CC13_WIN_READINESS.md:96` counted only the secondary.

**Related, same mission.** SetCommandBuildingOwnerPlayer (008A8D90, unanswered) hands the radar station
and both command buildings to PLAYER_1 (`:1157`, `:1158`, `:1178` in COTP-IJN).

### R5 GetEntitySpeed is unanswered

**Pattern.** usn_11 and prcp_11 `CheckMontR1..R10` (`:2295-2415`) clear a primary-4 rescue point only
when all of these hold:
- `luaGetDistance(Monteray, Points[i]) < 100`
- `IsUnitSelectable(Monteray)`
- `GetPosition(Monteray).y < 10`
- `GetEntitySpeed(Monteray) < 30`

**Effect today.** R3's `nil` short-circuits the condition, so it is never true.

**Effect with R3 fixed.** `nil < 30` raises an error.

Hammerhead `:1363`, JM05 `:1341` and jm12 / prcpjm12 `:702` / `:721` follow the same pattern. R3 and R5
have to be answered together.

### R6 ArtilleryEnable is unanswered

**Image.** 0089C590 reads the entity and the flag, gates on the weapon director, and calls 0071DFD0(flag).
0071DFD0 builds session message 5Ah with sub-kind 3 and routes it with `BSP_Session_RouteMessage(msg, 7, 0)`.
It is the twin of AAEnable's 0071E050 (sub-kind 4, director+221h; `game_hosts_lua.cpp:7322-7328`). The
artillery handler's director field is unread (docs/UNIT_WEAPON_DEVICES.md).

> **Erratum (2026-10-17, cc13-fix17).** This section first said AAEnable's 0071E050 sends sub-kind 2. It
> sends sub-kind 4: `0071E09A MOV dword ptr [ESP+24h],4` (ArtilleryEnable's 0071DFD0 stores 3 at
> 0071E01A). The message 5Ah sub-kind map in 0071C1E0 is: 0 -> vtable[64h], 1 -> vtable[68h],
> 2 -> +1CCh, 3 -> +220h, 4 -> +221h, 5 -> +222h, 6 -> +223h. The director+221h field above was
> already right; only the sub-kind number was wrong.

**Host.** Nothing happens, so guns the script switched off keep firing.

**Single-player uses:**
- usn_09_leyte `:9-:11` disables Fortress1-3 at load; also `:589` and 15 more.
- usn_05 (12 calls).
- jm09 `:182`, `:194` (StageTrg) and `:1589`.
- bsm_02 (5), jm01, jm03, usn_03, usn_11, usn_16, and COTP usn_04_defend_guadalcanal.
- commandhelpers `luaShutUp` `:6865` (called only by chg/ac_escape) and the cheat handler.

### R7 Scoring_SetMissionCompleted is unanswered

**Image.** 008B8AD0 calls 00906460 on the local slot. In single player the argument is the flag, which
defaults to true. 00906460:
- writes record +0h, and +10h (the clock) on the 0->1 edge only;
- walks `[[game+19CCh]+28h]` and calls 00927F60 for every entity with +180h <= 7. That routine stamps
  the clock into entity+1D0h (and into up to five flight members) and may call 0090ED40
  (docs/MISSION_RESULT_DECISION.md).

**Host.** The row is unanswered; the reconstruction `set_slot_mission_completed_00906460`
(`src/mission_result.cpp:108`) is not reached.

**Stale label.** `game_hosts_mission_result.cpp:21-24` says the mission's stage init never reached the
native. That premise is stale: luaMissionFailedNew (`:10372`, false) and luaMissionCompletedNew
(`:10431`, true) call it in every single-player mission end. The committed record therefore says "not
completed" for every win.

The neighbouring scoring rows are unanswered too. They are scoring-only:
- Scoring_SetRanking (row 008B8CA0, 20 calls);
- Scoring_GrantUnlock (row 008CB920);
- Scoring_GrantBonus and Scoring_SetConditionMessage;
- Scoring_IgnoreEntityKill (USN04 `:1183-1186`, `:2181`; it inserts into `[game+21A0h]+1488h`);
- SetAchievements.

Scoring_GetTotalMissionScore and GetPlayerDetails are read only on multiplayer branches.

### R8 PlayBinkMovie is unanswered

**Image.** 0089A480 reads arg 0 (the name) and, when there are more than two arguments, arg 2 (the
debrief byte). It then tails into 0089A390, which calls 004CD390 and, with the byte set, EndScene
004D7970.

**Host.** The reconstruction (`game_hosts_mission_result.cpp:227-253`) is called only from
`--mission-complete-frame` (`game_hosts_mission_frame.cpp:3657-3667`). The Lua row is unanswered.

**Stale label.** `game_hosts_mission_result.cpp:12-16` says no script calls PlayBinkMovie in a headless
run. That is stale: the commandhelpers end helpers call `PlayBinkMovie(Mission.*Params.Movie, "", true)`
(`:9854`, `:10111`, `:10926`), and the row is UNIMPLEMENTED in 31 run logs. So a scripted end that goes
through the movie never builds the result or runs EndScene. That matches VERIFY4's harness-boundary
note on EndScene 008B01B0.

## Part 1: labels in the five files

| Host | Label | Verdict | Note |
|---|---|---|---|
| mission `:1117-1124` | chosen difficulty 1, no profile | MATCH (as a fresh profile) | read only when the record's difficulty is 3; GetDifficulty reads game+6ACh |
| mission `:1136-1143` | checkpoint_differs false, write_checkpoint record | MISMATCH MINOR | profile write at start; a fresh run has no checkpoint (VERIFY5 covers Save/LoadCheckpoint) |
| mission `:621-1215`, `:1503-2051` | front-end page, movie and menu records | MISMATCH MINOR | presentation; no script reads them |
| mission_result `:12-16` | "no script calls PlayBinkMovie" | STALE LABEL, see R8 | |
| mission_result `:21-24` | "stage init never reached SetMissionCompleted" | STALE LABEL, see R7 | |
| mission_result `:376-414` | 009064B5 notify, 00904BF0 zero-delta tick, 004C3CB0 flush, end-movie and GUI records | MISMATCH MINOR | after the end is decided; reached only through R8's injected path |
| mission_result `:422-464` | debrief bring-up, end-scene records | MISMATCH MINOR | after the end; the debrief is not built |
| mission_result `:483-505` | objective sound, markers, replication, HUD byte records | MISMATCH MINOR | presentation and network; VERIFY14 shows the scripts decide on `obj.Target`, not the native sets |
| mission_frame `:103-107` | profiler slots | MATCH | not game state |
| mission_frame `:160-190` | scan-proximity and traffic substitutions | MISMATCH MINOR | switches ON; the effect is a point effect; the traffic walk is exact for scenes with no traffic item (labelled) |
| mission_frame `:598-640` | the OFF arms of ScanProximity | not live | switch ON |
| mission_frame `:1356` | engine-movie latch, no checkpoint | MATCH | fresh run |
| mission_frame `:1760`, `:1795` | torpedo warning owner; loss-warning text by side | MISMATCH MINOR | voice and HUD warnings only |
| mission_frame `:2159-2165` | first-unit controlled stand-in | not live | kInitialControlledUnitBound ON |
| mission_frame `:3657-3667` | `--mission-complete-frame` injection | harness | see R8 |
| hud `:1504-1508` | "local player's side record is party 0" | STALE LABEL | `local_player_party()` is `scene_slot_party(0, 0)` (`game_hosts_ai.hpp:484-486`, ON) |
| hud `:1531-1534` | +2194h not modelled, spectate shut | MATCH | the comment itself notes +2194h is clear in single player |
| hud `:2112-2114` | local player index 0 | MATCH | game+18ECh is 0 in single player |
| hud `:3801-3830` | destruction notice from the destroyed-unit record at the interface update | UNRESOLVED (ordering) | drives the controlled-unit change GetSelectedUnit reads; the image delivers it in the kill dispatch, the host at the next interface update. Whether a script think can fall between them in one frame was not traced |
| hud `:2966-2969` | power-up manager not built | MISMATCH MINOR | AddPowerup (unanswered) is called on the JM08, USN01, USN04 and USN13 rows (`usn_19_coralus :625` on secondary 1 completion, `:804` after luaRetreatTransportFleet), but a power-up needs the player to activate it |
| hud `:376`, `:971`, `:1363-1387`, `:1771-1926`, `:2136-2827`, `:2940-3446`; hud_world `:246-1242` | HUD screen, gauge, input, camera and marker substitutions | MISMATCH MINOR | presentation; scripts read none of them |

## Part 2: other unanswered rows

### Query rows

| Native | Scripts' use | Verdict |
|---|---|---|
| IsGUIActive 008CA010 (always pushes a boolean) | 23 calls. Only `usn_01_battle_of_eastern_solomons :1762` compares `== false`, and it gates a map hint. The rest test truthiness, where nil and false agree headless | MISMATCH MINOR |
| IsHintActive / IsHintCritical | hint chains; ShowHint is unanswered too, so the two stay consistent | MISMATCH MINOR |
| IsPlayerControlled 008A5EF0 | 20 calls, all in commandhelpers `luaPlayerAirbaseManager`, which nothing calls | not live |
| GetRoleAvailable 008ABAC0 | 59 calls, all in multiplayer scripts | not live in SP |
| GetPlayerDetails, Scoring_GetTotalMissionScore, Multi_GetPlayers | multiplayer branches | not live in SP |
| Scoring_IsUnlocked 008BBC40 | 68 calls: medal-gated content (jm03 `:985`, ...) | MATCH as a fresh profile (nil and false agree) |
| GetShipMaxSpeed 00890BB0 | usn_02 `:821`, `:844` `GetShipMaxSpeed(unit)*1.5` errors; jm12 (5), usn_10, chg_*, 07_Komandorski | MISMATCH REAL, error, no test row |
| GetNumCatapulted 00892AA0 | jm02, jm03, jm04, jm14, 01_saving_force_z: arithmetic or compare | MISMATCH REAL, error, no test row |
| GetDirection 008A7CB0 | jm01 (4), jm02 | MISMATCH REAL, error, no test row |
| GetGuiName 0088DAA0 | jm08 (COTP) `:1487`, jm10 `:739`: stored into tables, no error | MISMATCH MINOR |

### Action rows used in SP missions

These are census rows, not per-row traces; none is on a test row. They were sent to team-lead as one
note (URGENT class under the gameplay terms).

| Class | Rows (SP calls / files) |
|---|---|
| Torpedo | ReloadTorpedoes 008C3CE0 (14/12); GetSquadTorpedoes 008BEF80; GetShipTorpedoes 008BED80 |
| Submarine | SetSubmarineAirSupply 00893A60; ForceFireDepthCharge 0089B9D0 |
| Bomb release | PlaneForceRelease 0089EBA0; SquadronForceRelease 0089E850; PilotBomb 008A4F00; PlaneBombFallingTime 008A54C0; PlaneReloadBombPlatforms 0089F080; GetPlaneIsReloading 008BEBE0 (21/12) |
| Placement and air ops | PutRelTo 008AA560 (71/29); SquadronSetTravelSpeed 008A0630 (31/23); SquadronSetHomeBase 0089E220 (20/10); ShipUseCatapult 00891D50 (19/11); SetAirBaseSlot 00895ED0 |
| Unit state | SetDeviceReloadTimeMul 008C14B0 (21/18); SetCrewLevel 00894F20 (54/8, chg_*); SetRace 008A8720 (14/11); ClearForcedReconLevel 008AAB90 (26/18; SetForcedReconLevel is answered, so a forced level never clears) |

The rest of the 207 rows are presentation, input, camera or multiplayer:
- Effect, SetGuiName, ShowHint, DisplayUnitHP / HideUnitHP, BlackBars, Loading_*, PrepareClass;
- EnableInput / ForceEnableInput / HackPressInput, SetSkipMovie, MovCam_*;
- MissionNarrative*, SetNumbering, the support-manager rows;
- spawnpoints (multiplayer only), and the AI planner weights (multiplayer only).

The full list with counts is reproducible with `local/cc13-verify16_census.py` and
`local/cc13-verify16_sp.py`.

## Not covered

- The HUD substitutions were classed by reading their labels against what scripts can reach. They were
  not each traced to the image.
- The R-items in Part 2's action table were not traced instruction by instruction.
- The `hud :3801` ordering question is open.
- The census keys on names and `0x` addresses. A row named only in a comment or a harness path (as
  PlayBinkMovie is) counts as answered, so the census can miss rows. The run-log union (810 logs)
  covers only the missions those logs ran.
