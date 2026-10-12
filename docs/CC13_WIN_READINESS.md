# Win readiness of the not-yet-won missions (cc13-winready1, 2026-10-12)

Read-only scan for cc9. No source, ledger or Ghidra state was changed, and no game was run.

**Evidence status:**
- **Script facts** are read from this installation's mission Lua. The installation is modded, so these are not
  retail claims.
- **Host binding status** comes from two places:
  - the source at origin/main as merged into this branch on 2026-10-12;
  - the `MissionLuaNative::<name> ... concrete|UNIMPLEMENTED calls=N` table that every bsp_game log prints.
- **Run facts** come from cc9's logs, which were grepped, and from the landing notes in docs/.
- Lines marked *inference* are reasoning that was not traced.

**Mission ids.** Every id below is the `missiontree.lua` `["id"]` that `--menu-select` takes. The script was
resolved through the scene's `StageScript` key.
- `missiontree.lua` itself is dated 2025-06-02.
- The tree's comment numbers differ from the ids. For example, the comment "USN15 Sibuyan Sea" is id **USN21**.
  `docs/CC13_ROCKET_KAMIKAZE_SCOPE.md` section 3 labels rows "comment / id".
- cc9's logs confirm the ids. `s13kz_usn21.log` says `id=USN21 ... scene=universe/Scenes/missions/USN/usn_sibuyan.scn`.

| id | script (this installation) | mtime | lines |
| --- | --- | --- | --- |
| USNOS | `scripts/missions/COTP-USN/us_osumi.lua` | 2024-10-29 | 2396 |
| JM08 | `scripts/missions/COTP-IJN/PRCPIJN/PRCPJM08.lua` | 2024-08-26 | 1356 |
| USN04 | `scripts/missions/USN/usn_19_coralus.lua` (Coral Sea) | 2024-08-26 | 3397 |
| USN13 | `scripts/missions/USN/usn_13_truk.lua` (Truk) | 2024-08-13 | 1918 |
| USN15 | `scripts/missions/COTP-USN/PRCPUS/usn_15_Battle_of_Surigao_Strait.lua` | 2024-08-09 | 1140 |
| USN18 | `scripts/missions/COTP-USN/PRCPUS/prcp_13_iwojima.lua` (Iwo Jima, PRCP) | 2024-10-29 | 2666 |
| USN19 | `scripts/missions/COTP-USN/PRCPUS/prcp_14_okinawa.lua` (Okinawa, PRCP) | 2024-08-26 | 2960 |
| USN21 | `scripts/missions/USN/usn_sibuyan.lua` (Sibuyan Sea) | 2024-10-29 | 1311 |
| JM05 | `scripts/missions/COTP-IJN/PRCPIJN/JM05.lua` (Port Moresby) | 2024-07-13 | 5841 |
| JM06 | `scripts/missions/COTP-IJN/PRCPIJN/JM06.lua` (Prelude to Midway, submarine) | 2024-07-13 | 2659 |
| IJN01 | `scripts/missions/Ijn/ijn_1_pearl.lua` (Pearl Harbor) | 2024-08-26 | 1488 |

The shared helpers are in `scripts/global/commandhelpers.lua` (2024-10-29, modified in this installation).
- `luaMissionCompletedNew` (10404) is the end of every win. Its first line (10411) is
  `os.execute("sus_prog.exe")`.
  - That line is absent from `commandhelpers.lua.bak` (2024-07-13).
  - The host's exec guard refuses it and logs the refusal, for example on the USNOS win (SHIP_AI_OPEN_ITEMS 210.4).
- `luaMissionFailedNew` is defined twice; the definition at 10360 is the live one.

## 0. Ranking

Closest first. A "player-order win" means a `--helm-orders` file using the existing verbs: `moveto`, `takehelm`,
`select`, `attack`, `target`, `launch`, `release`, `build`, `depthcharge`, `fire`, `order` and `torpedo`
(`src/game_hosts_mission_frame.cpp` near 2648). Sizes: **S** is a switch flip or an order-file iteration; **M** is
one bounded host packet; **L** is several packets or a missing task family.

| rank | id | furthest point reached (evidence) | blockers before a legal player-order win |
| --- | --- | --- | --- |
| 1 | **USNOS** | **Completes** at 7475.82 s (variant x) and 7687.83 s (variant y) on branch cc9-ships43 at `794e37ae7` (SHIP_AI_OPEN_ITEMS 210.4) | land ships43's 210 bounds on main and re-tune x on the main build (S-M). Plans are build-specific: AG lost f17 to the soft roll |
| 2 | **JM06** | Phase 3 posts idle at 24000 frames, after `GetSubmarineOnSurface` was bound (LUA_BINDING_MISSION, cc9-lua44) | hit-callback args, `kLuaListenerCallbackArgsBound` OFF, so `arg[4]` is nil (S once paired); a torpedo order that actually hits Lexington (S-M, order-file geometry); Lexington plus escorts sunk by the Yamato (M) |
| 3 | **JM08** | HQ held for 2400-4800 s in r5/r7/w1-w3, but 10 of 11 LandShips dead, so primary 1 is open (SQUADRON_LAND_TASK 14968-15017) | bombs cannot finish TT04 against repair (GUNNERY 175, M); then phase 2's three C-47 waves with the HQ still alive (M) |
| 4 | **USN15** | **no host run on record** | first run needed. A pure surface action with hosted features. Risk: the escape fail at JapanGoTo (M, unknown) |
| 5 | **USN04** | Phase 2 with order file `s39_u4_p1`, under frame jitter only (SHIP_AI 196, SQUADRON_LAND_TASK 14571ff) | US dive bombers never release: the SPVeteran row in this installation's `robots.lua` zeroes the dive-aim gains (S lever: difficulty 0; otherwise M); phase 1 under lockstep (M); Lexington slots 1-2; then the carrier-HP and scuttle phases (L overall) |
| 6 | **USN13** | Phase 3 idle (SHIP_AI 14780ff); never attempted with orders | about 60 targets including AA and coastal forts; same SPVeteran dive-release problem; the carrier-loss fail is a knife edge (L) |
| 7 | **IJN01** | Phase 1 stuck: each A7M dives once and never strafes; only PT1 dies (SHIP_AI 14716ff) | fighter after its one bomb (plane lane, M); then Judy and Jill phases (M) |
| 8 | **JM05** | Phase 1, no end on idle rows (AG); no order run | three captures by transports, sinking Lexington, and keeping both carriers alive; long mission (L) |
| 9 | **USN19** | 3000-frame kamikaze pair only (2026-09-29) | kamikaze/Ohka tasks not hosted (cc13-kamikaze1); three HQ captures; sinking a 17500-HP Yamato with the dive-release problem (L) |
| 10 | **USN18** | 3000-frame smoke, phase 1 | kill the Ohka group (drop-kamikaze/kamikaze, not hosted), 39 shore guns, HQ capture; four phases (L) |
| 11 | **USN21** | 3000-frame pair, phase 1 | submarine torpedoes in phase 1, strike damage (dive release), kamikaze wave (not hosted). The script has **no fail path**, so it can only stall (L) |

**Already won with order files (AG/AH, not in scope):** USN01, USN02, USN12, LOMP06, LOMP10, ESMP08 and BSM01
(GAME_EXECUTABLE "Scripted-win rows (AG)" and the AH plan).

No other stock mission was scanned in depth. *Inference:* USN15-like pure surface actions are the cheapest new
candidates, because the gunnery, torpedo and damage-control lanes are the most mature.

## 1. Cross-cutting host status

### 1.1 Natives on the critical paths

The binding table is `src/mission_lua_host.cpp` (560 rows, image table 00E0B7B8). A row with no host arm falls
through `game_hosts_lua.cpp` near line 2685: it is recorded UNIMPLEMENTED and returns nothing, so Lua sees `nil`.

The runs column aggregates the 30 AG logs (`cc9-gunnery35\local\g35_ag*.log`) with `local/wr_natives.py`.

| native (address) | status | runs (AG) | where it matters |
| --- | --- | --- | --- |
| FindEntity 00898E30, GetDifficulty 008AE030, GetPosition 008A7B00, Kill 008AC5C0, SetParty 008A8930, SetInvincible 00897A50, StartDialog 008B0540, Blackout 008D1340, MissionNarrative 008B0C10, CreateScript 00898750, SetThink 00897FB0, Countdown 008B16E0, CountdownCancel 008B19A0, GameTime 008A9320, EndScene 008B01B0 | bound | concrete in every log that calls them (FindEntity also shows UNIMPLEMENTED in 2 logs for unresolved names) | every mission |
| GetHpPercentage 0088E9D0 | bound | concrete, 10 logs | USN04 phase 3 (carrier HP), USN21 phase 2 (average HP) |
| AddListener 008C6760, RemoveListener 008C6990, IsListenerActive 008C6BB0 | bound, with target/attacker/attackType filters | concrete | USN04 Lexington hits, JM06 cvhit, USN21 Takao |
| **hit/kill callback arguments** | **record-only**: both dispatchers call `lua_pcall(state,0,0,0)`. `kLuaListenerCallbackArgsBound` and `kLuaKillAttackerBound` are **false** (`include/bsp/game_hosts_lua.hpp:222,232`; SQUADRON_LAND_TASK 5fu) | - | **JM06 primary 2** (`arg[4] == "TORPEDO"`); USN21 secondary 1 |
| LaunchSquadron 0089E3C0, IsReadyToSendPlanes 00895D20, SetAirBaseSlotCount 008963E0, AddAirBaseStock 00896A90, UnitGetAttackTarget 008A6DE0, SquadronSetTravelAlt 0089F550 | bound | concrete | air ops in USN04, USN13, USN18, USN19, USNOS (`luaAirfieldManager`, commandhelpers 7872) |
| harness `launch` slot choice | host divergence: slot 0 refuses "no such slot" where the image's 006C7210 grows the deck (docs/CC13_AIR_OPS_SLOT_GROWTH.md 11-23, diff reverted) | - | USNOS strikes; any order file that launches from a full deck |
| GenerateObject 00944FD0, SpawnNew 0094C480, OverrideHP 008C1930, JoinFormation 00899D10 | bound | concrete | spawned waves in USN18, USN19, IJN01, JM06 |
| PilotSetTarget 008A4C90, NavigatorAttackMove 008A30D0, NavigatorMoveToRange 008A2F20, NavigatorMoveToPos 008A2BC0, NavigatorMoveOnPath 008A3600, UnitSetFireStance 008A6490 | bound (the task the target picks may not be: section 1.2) | concrete | script attack orders everywhere |
| GetSubmarineOnSurface 008942C0 | bound (`game_hosts_lua.cpp:5560`) | not called in AG (JM06 never reaches phase 2 in 3000 frames) | JM06 phase 2: 285 calls, 51 true on the cc9-lua44 24000-frame pair |
| SetSubmarineDepthLevel 00893F40 | bound | concrete, 2 logs | JM06, USN21 boats |
| RepairEnable 008AD330 | bound; reaches task+45h (`kHullRepairEnableRouteBound` true) | concrete | **works against kills**: the USNOS Hakus and JM08's TT04 regenerate |
| GetProperty 0088BF80 | bound | concrete | `"slots"` and `"ammoType"` (IJN01's backup spawn) |
| TrulyDead 00899500 | **unbound** | UNIMPLEMENTED, 7 logs | JM05 2687/2828 (fail-entity choice only); JM06 954 (`not nil` makes the Yamato branch run). Not a blocker |
| IsInBorderZone 008AF020 | **unbound** | not called | JM06 996, the "Hornet left the area" fail. `nil` disarms that fail, which helps the win |
| SetFailure 0088EFF0, GetFailure 0088F1D0 | **unbound** | GetFailure UNIMPLEMENTED in 2 logs | JM06 1992-1993: Lexington's engine and steering jam are never applied (*inference*: Lexington keeps moving in phase 4); USN19 hidden objective |
| SquadronForceGunnerMode 0089F950 | **unbound** | UNIMPLEMENTED in `s30_cap_usn18.log` (12 calls) | USN18 phase 1, player gunner seat only |
| GetCommandBuildingLevel 0089B6D0 | **unbound** | not called | JM05 secondary (2779) |
| Scoring_SetMissionCompleted 008B8AD0, BannSupportmanager 0088C750, EnableInput 008AFBD0 | **unbound** | UNIMPLEMENTED | inside the end helpers. The wins above complete without them, so not a blocker |

### 1.2 Gameplay features

| feature | host status | evidence | missions whose win or fail depends on it |
| --- | --- | --- | --- |
| Command-building capture (party flip) | **hosted**; `kCaptureStatePartyFromUnitBound = true` (`src/game_hosts_ship_ai.cpp:678`) | USNOS x captures HQ1, HQ2 and CB2 (SHIP_AI 210.4) | USNOS, USN18, USN19, JM05; JM08's fail (HQ goes neutral) |
| Kamikaze, drop-kamikaze (Ohka) and rocket plane tasks | **not hosted**. The chooser and factory run; there is no tick (CC13_ROCKET_KAMIKAZE_SCOPE section 1). cc13-kamikaze1 and cc13-rocket1 are reconstructing | the scope doc | USN18 primary 2 (OkhaGang), USN19 primary 1 (waves), USN21 primary 4. Each needs the planes **dead**, not their attacks. *Inference:* without a tick the planes may never close to AA range, so killing them may need fighters ordered onto them |
| Dive-bomb release | hosted, but US carrier strikes almost never release at difficulty 1. The cause is **this installation's** `robots.lua` (2025-06-01): its SPVeteran PilotBot row (lines 693ff) sets `DiveBombAimPrecPullPlus`/`Minus` = 0 (720-721), and Elite does the same (1134). SPNormal (554ff) has 0.018 | SQUADRON_LAND_TASK 14571-14612; GUNNERY 181 table (USN04 1 of 33) | USN04 (Shoho, carriers), USN13, USN19 (Yamato), USN21 phase 2. *Inference:* USN04 maps difficulty 0 to a non-zero row; check each script's `SkillLevelOwn` table first |
| Bomb accuracy against a ship under repair | open; three systematic errors identified (GUNNERY 175) | JM08 TT04: 9 blasts in 4800 s | JM08 primary 1 |
| Submarine: surface test, submerged launch | hosted: `GetSubmarineOnSurface` is bound, and `kSubmergedTorpedoLaunchSwimBound = true` (`src/game_hosts_gunnery.cpp:469`, flipped ON in 00d768ba3). The order file has a `torpedo` verb but no dive or surface verb | GUNNERY 170 and 182 | JM06; USN21 phase 1 |
| Plane depth-charge task | not hosted (docs/CC13_OMITTED_TASKS_SCOPE.md, rank 2) | `usn_sibuyan.lua:739` | USN21 only, and incidental: no fail path |
| Fighter strafing after a bomb drop | open: the squadron stays in `done` after one dive bomb (SHIP_AI 14716-14727) | `s38_i1_p1` | IJN01 primary 1 |
| Hull repair | hosted | - | works against JM08 and USNOS kills (see RepairEnable above) |

## 2. Per mission

### 2.1 USNOS (us_osumi.lua)

**Script (read):**
- **Win.** The win needs two flags:
  - `Mission.BaseCapped`: HQ1, HQ2 and CB2 all `PARTY_ALLIED` (lines 1080-1136);
  - `Mission.ReinDead`: every `Mission.FinalTrgs` entry dead (1181-1187). That is Yamato, Musashi, Haku1-4 and the
    Betty1-6 that `luaFuntime` adds (1795-1813).
- Then the chain runs `luaBombFadeOut` (977, defined at 1823) -> the nuke movie -> `luaNukeMovieEnd` (1922-1924) -> `luaMissionComplete`
  (2065) -> `luaMissionCompletedNew(... "The Japanese Empire is finished!")` (2115).
- **Fail.** While primary 1 is active, the mission fails if `Mission.Troops` are all dead or `Mission.BigE.Dead`
  (985-987). That calls `luaMissionFailedNew(Mission.JapHQs[1], "Game Over")` (2171).

**Features:** troop capture (hosted), surface gunnery (hosted), air ops (slot-0 refusal), and drop-kamikaze Bettys.
The Bettys are only kill targets and die in every log (CC13_USNOS_HAKU).

**Runs:**
- **Completed:** x at 7475.82 s and y at 7687.83 s, on cc9-ships43 at `794e37ae7` with both 210 bounds ON. Logs
  `cc9-ships43\local\s43_p9_osxf.log` and `s43_p9_osyf.log` (SHIP_AI_OPEN_ITEMS 16608-16642).
- **AG** (`g35_agw_os.log`) **failed at 1554.69 s** at HQ1: the soft roll moved ships41's f17 plan
  (GAME_EXECUTABLE "Results (AG)").

**Blockers:**
1. Land 210.4's build on main (S).
2. Re-run x on main. If it diverges, re-tune from x, not from older variants (S-M). Every gameplay change moves USNOS
   from about 288 s on.
3. Optional: the slot-growth host fix (CC13_AIR_OPS_SLOT_GROWTH), so strikes stop being refused (M). The x plan wins
   with surface ships instead.

### 2.2 JM06 (JM06.lua, the player is a submarine)

**Script (read).** Phases run in order:
1. **Primary 1, sink the convoy.** All `CargoTargets` dead, with none having `KillReason == "exitzone"`
   (851-860). Then a movie and `StepPhase`.
2. **Primary 3, radio message.** Needs `Mission.MessageTransmitted` (889). `luaJM6CheckSurface` (1944) runs
   `Countdown("ijn06.obj_s3",1,24,...)` (2121) while `GetSubmarineOnSurface(PlayerUnit)` holds, so the boat must stay
   surfaced for 24 s.
3. **Primary 2, torpedo the carrier.** Needs `Mission.CVHitByPlayer` (905). That is set only by the `"hit"`
   listener callback `luaJM6CVHitByPlayer` when `arg[4] == "TORPEDO"` (1505-1525, attacker filter
   `{Mission.PlayerUnit}`).
4. **Primary 4, sink the carrier group.** All `Mission.Escorts` dead and `Mission.Lex.Dead` (926). Then
   `luaJM6FinishMission` (953-959).

**Fail (read):**
- all three `PlayerSub` dead (2414-2419);
- a convoy target escaped (854-870);
- Hornet in the border zone (996-1001), which is dead because `IsInBorderZone` is unbound.

**Difficulty.** `ShipsToSink` is 2, 3 or 5 (100-106).

**Runs:**
- Idle phases 1-2 complete, and phase 3 posts at 24000 frames (`l44_sub_j6l.log`; LUA_BINDING_MISSION
  3740-3760).
- The player-torpedo pair (GUNNERY 170) fires two torpedoes. Both snap to bow-dead-ahead and end in land. GUNNERY
  attributes this to the order file's geometry.

**Blockers:**
1. **`kLuaListenerCallbackArgsBound` is OFF.** Without it the callback gets no arguments and primary 2 can never
   complete, even with a hit. Pair it and flip it (S).
2. An order file that brings the boat bow-on to Lexington before firing (S-M).
3. *Inference:* `SetFailure` is unbound, so Lexington's engine and steering jams (1992-1993) never apply and it
   keeps manoeuvring. Phase 4 needs the Yamato's guns on Lexington and every escort (M).
4. The phase-1 -> 2 step depends on the movie callback (2485-2486, 2590-2617). The idle runs pass it.

### 2.3 JM08 (PRCPJM08.lua)

Full read: docs/CC13_JM08_HQ.md.

**Script.**
- **Win.** Primary 1: all 11 `LandShips` dead (727-734). Primary 2: `Mission.Flagship.Dead` (747-755). Then
  `CheckCompletion` (790-803) -> phase 2 -> primary 3: every C-47 dead (1300-1310) -> `Victory`.
- **Fail.** `Mission.CommandBuilding.Party == 2` (1320-1324). The engine neutralizes the HQ at health 0.

**Features:** hit and repair, dive and level bombing, shipyard boats (`build`), HQ health. `RemoveAllAirBasePlanes`
008975B0 is unbound, so Hosho is stronger than in the image (CC13_JM08_HQ 265).

**Runs:**
- **AG JM08 long fails at 1026.93 s** ("The HQ has been destroyed!"; `g35_ag_jm08l.log`).
- cc9-lua50's r5/r7/w1-w3 hold the HQ but leave TT04 or LST03 alive. TT04 is repaired to 4000/4000, and only 9
  blasts land in 4800 s (SQUADRON_LAND_TASK 14995-15017).
- The status is "parked until the dive-bomb fix lands, then rerun r7" (15224-15225).
- After `kAimDiveBrakeRatioBound`, r7 releases 20 of 33 dive bombs (GUNNERY 181).

**Blockers:**
1. Bomb accuracy on TT04 (GUNNERY 175's three errors) (M).
2. Phase 2's C-47 waves have never been reached. The HQ must survive the B-17, SBD and C-47 attacks (M).

### 2.4 USN15 (usn_15_Battle_of_Surigao_Strait.lua)

**Script (read).** `luaCheckObjs` polls every 5 s (700, 889).
- **Win.** The four `IJNKeyTrgs` (Fuso, Yamashiro, Nachi, Ashigara; 388-392) dead (862), or all 17 `IJNShips`
  dead (868). Then the dialogs -> `luaMissionCompletedNew` (965).
- **Fail.** Any enemy ship within 3000 of `JapanGoTo` (119, 315, 850-852), or all `USShips` dead (856-858). No timer.
- **Difficulty** changes only Japanese evasion, repair and torpedo use (398-425).

**Features:**
- surface gunnery and ship torpedoes (hosted; `kPlayerTorpedoGroupFireBound` ON);
- shipyard PT boats (`build` verb);
- no aircraft, kamikaze or rockets on the path.

**Runs:** none found under any log name.

**Blockers:**
1. A first idle run, to see how far the Japanese line gets toward JapanGoTo and how fast US gunnery kills (S).
2. An order file that concentrates fire on the four key ships (M).

*Inference:* this is the cheapest of the never-run missions.

### 2.5 USN04 (usn_19_coralus.lua, Coral Sea)

**Script (read).**
- **Primary 1** completes after the bomber waves, or once the bombers or fighters are all dead (569-585).
- **Primary 2** is `Shoho.Dead` (599), which leads to `luaMoveToPh3` (1702).
- **Primary 3** is carrier HP:
  - Easy: `Zuikaku or Shokaku <= 50` (676).
  - Normal: both at or below 50 (725).
  - Hard: both at or below 20 (774).
- **Phase 4.** Lexington is held at 5% (962). The "Lexkiller" bombers' hits are counted by
  `AddListener("hit", ...)` with attackType `{"TORPEDO","BOMB","ROCKET"}` (1201-1206). At `LexHits >= 15` (815)
  Lexington explodes.
- **Primary 5:** scuttle Lexington (828), then `luaMissionCompletedNew` (913).
- **Fail:** Lexington dead in phase 1 (589-590), or one `USCVs` left while primary 4 is active (523-527).

**Features:** carrier air ops, US dive and torpedo bombing, the hit listener (filters hosted, no arguments needed),
and torpedoes from an escort to scuttle Lexington. The rocket appears only in the listener's type list. No kamikaze.

**Runs:**
- `s39_u4_p1` (45000 frames, 0.0222 s, jitter `20,3`) reaches phase 2 (SHIP_AI 196).
- Dive-bomb releases are 0 of 33 at first and 1 of 33 with the brake fix (GUNNERY 181). Torpedo releases are 3 of
  36, all Kates.
- Phase 1 passes only under frame jitter (SHIP_AI 196).

**Blockers:**
1. The SPVeteran zero row. Running at difficulty 0 is a harness lever (S; `chosen_difficulty()`,
   CC13_DIFFICULTY_VALUES), but check `SkillLevelOwn` at 66-82 first. Otherwise there is no in-host fix: the row is
   this installation's data.
2. Phase 1 under lockstep (M).
3. Lexington slots 1-2 (SHIP_AI 196) (M).
4. Primary 3 needs both carriers at or below 50% at difficulty 1 (M).
5. Primary 5 needs a scuttling torpedo order (S).

### 2.6 USN13 (usn_13_truk.lua, Truk)

**Script (read).**
- **Phase 1:** the `AttackWave` bombers all dead (799-803).
- **Phase 2:** all `JapFighta` dead (827-829).
- **Phase 3:** all `TotalTrgs` dead (897), then `luaMissionCompletedNew(FindEntity("Enterprise"))` (1753).
  - `TotalTrgs` is the Marus, plus Airfield2, Airfield5, JapAF and "Shipyard Entity CB3", plus every live Japanese
    landfort named "Heavy AA", "Light AA" or "Coastal Gun" (534-545, 637).
- **Fail:**
  - `#USCV <= 7`, i.e. two of nine carriers lost (787-789);
  - in phase 2, all `USBomba` or all `USFighta` dead (815-823).

**Features:** carrier strikes on ships and land forts (dive release), Japanese airfield ops, and dogfighting. The
one Raiden with rockets is incidental.

**Runs:**
- Idle reaches phase 3 (`s38_u13idle.log`, SHIP_AI 14780-14788).
- The AG long row runs out its 9000 frames in phase 1 (`g35_ag_usn13l.log`: `MissionPhase=1`, deaths 19 in the
  3000-frame row).
- With the bomb-drift switch ON, Cowpens burns out and the mission fails at 366.33 s (GUNNERY_OPEN_ITEMS 12046, 12336), so the
  fail is a knife edge.

**Blockers:**
1. Dive release at SPVeteran (as USN04).
2. About 60 targets.
3. The slot return. It was fixed for the case of lost squadrons (AIROPS_LAUNCH_TICK 898-911), and ESMP08's 3204 s win
   shows sustained launches, but USN13 has not been retried.
4. Keeping carriers alive.

Size: L.

### 2.7 IJN01 (ijn_1_pearl.lua)

**Script (read).**
- **Primary 1:** all 25 `StrafeShips` dead (785-791).
- **Primary 2:** all 6 `PearlObjOne` dead (815-819).
- **Primary 3:** all 6 `PearlObjTwo` dead (851-859). This starts `luaStartDialog("END")` (857) before
  `EndMission = true` (859), then `luaMissionCompletedNew` (1205).
- **Fail:** only in phase 1, when all `ReppuTable` A7Ms are dead (795-797).

**Features:** fighter strafing of ships (MG damage against hulls), dive bombs, aerial torpedoes. No kamikaze.

**Runs:** `s38_i1_p1` (24000 frames). The chooser picks `divebomb` for 228 of 233 targets. Each squadron makes one
dive, then stays `done`, so only PT1 dies (SHIP_AI 14716-14727).

**Blockers:**
1. What the image does with an A7M after its bombs are spent: re-choose a gun pass (plane lane, M).
2. Phase 2-3 release rates for Japanese Judy and Jill waves, untested (M).

### 2.8 JM05 (JM05.lua)

**Script (read).**
- **Win.** Only `MissionPhase = 99` with `MissionCompleted` (2954-2968) ends the mission. It is reached from 2871:
  `Airfields[1].Party == PARTY_JAPANESE and MissionPhase < 3`, plus `luaJM5AllPriCompleted()`. The primaries are:
  - P4: the radar station captured (2760);
  - P1: the secondary airfield captured (2838-2843);
  - P2: the main airfield captured;
  - P3: Shokaku and Zuikaku alive;
  - P5: `Lexington.Dead` (2718).
- **Fail:**
  - all `PlayerUnits` dead (2682-2685);
  - Shokaku or Zuikaku dead (2699-2704);
  - all transports dead (2823-2833).

**Features:** troop capture three times (hosted), escort and surface combat, sinking Lexington, AI air groups.

**Runs:**
- AG rows reach no end in 3000 or 9000 frames.
- On the anchor the long row failed at 312.05 s when Shokaku died; `thr` cures it (GAME_EXECUTABLE AG).
- No order file.

**Blockers:** the whole plan is untested. Size: L.

### 2.9 USN19 (prcp_14_okinawa.lua)

**Script (read).**
- **Primary 1:** all `Mission.Bomber` and `Mission.PT` dead (1410-1412). These are three kamikaze waves (classes
  100, 45 and 46, 978ff) and Shinyo or PT boats.
- **Primary 2:** HQ1-3 `Party == PARTY_ALLIED` (1767-1789, 1808-1810).
- **Primary 3:** `Yamato.Dead` (1993). Its HP is 15400, 17500 or 21000 by difficulty (593-632). Then the
  "YamatoDead" dialog callback `Victory` (582-583) -> `luaMissionCompletedNew` (2883).
- **Fail:**
  - all 8 transports dead (964-972 -> `FailureAP`);
  - TaskForce3 all dead while Yamato lives (2006-2009).
- Kamikaze waves repeat every 210 s until the bases fall (1708, 1762-1763).
- **Script bugs:**
  - Line 2007 compares a boolean with a number, so the poller dies with a runtime error after Yamato's death. The
    effect is harmless.
  - Line 1956 indexes the undefined `Mission.HQ[3]` if Yamato beaches.

**Runs:** only a 3000-frame pair (`s13zon_usn19.log`, 2026-09-29): 86 deaths, `MissionPhase=nil`, first wave. It
is older than `AddAirBaseStock`'s binding.

**Blockers:**
1. Kamikaze task (cc13-kamikaze1). It decides whether the waves close on the fleet, and so the fail pressure.
2. Three captures.
3. Yamato kill under the dive-release problem.

Size: L.

### 2.10 USN18 (prcp_13_iwojima.lua)

**Script (read).**
- **Primary 1:** a B-29 within 1000 of `BmbNav` (1096-1100). The AI flies the bombers.
- **Primary 2:** `OkhaGang` all dead (1138-1142). These are class 32 Betty/Funryu and class 103 Kamikaze Judy.
- **Primary 3:** two flying-boat waves killed (1150-1164).
- **Primary 4:** all 39 `Mission.Guns` dead (1176-1178).
- **Win:** `JapHQ.Party == PARTY_ALLIED` (1217-1219) -> `luaMissionCompletedNew` (2483).
- **Fail:**
  - all B-29s dead (1088-1090);
  - BB group or carriers wiped out in phase 2 (1122-1124);
  - all transports dead in phase 5 (1036-1040).
- **Script bug:** a `nil` `bomba` at 1094-1096 raises an error after the failure.

**Runs:** `s30_cap_usn18.log` (3000 frames, phase 1). `SquadronForceGunnerMode` is unbound (12 calls, player seat
only).

**Blockers:**
1. Ohka and kamikaze tasks.
2. 39 shore guns.
3. HQ capture.
4. Four phases and no run past phase 1.

Size: L.

### 2.11 USN21 (usn_sibuyan.lua)

**Script (read).**
- **Primary 1:** Atago and Maya dead, by the player's submarines (529-543).
- **Primary 2:** the Centre Force average HP at or below 60% (557-569). The ships are floored at 10% (368), and
  Musashi and Myoko at 33% (364).
- **Primary 3:** `Musashi.Dead` (597).
- **Primary 4:** all `KamikazeWave` dead (619), then `luaMissionComplete()` (633) ->
  `luaMissionCompletedNew(Mission.Yamato, ...)` (1041).
- **No fail path.** `luaMissionFailed` (1045) is never called.

**Features:**
- submarine torpedoes (hosted);
- carrier strikes (dive release);
- kamikaze planes as kill targets (task not hosted);
- a Jake ordered at the player's submarine (739; plane depth-charge task not hosted, incidental).

**Runs:** `s13kz_usn21.log` only (3000 frames, phase 1, one death).

**Blockers:** submarine attack orders, strike damage, the kamikaze wave. Size: L. It cannot fail, so it can only
stall.

## 3. Method and tools

The tools are in this branch's `local/` and are not committed:
- `wr_ids.py`: the id -> scene map from `missiontree.lua`;
- `wr_scn.py`: scene -> `StageScript` -> script mtime and size;
- `wr_log.py`: per-log objectives, end lines and the native status table;
- `wr_natives.py`: per-native source arms plus aggregated AG statuses;
- `wr_spot.py` / `wr_grep.py`: line checks.

Three read-only subagents extracted the script chains. Their line numbers were spot-checked against the files. Where
a cited line was off by a few, the line given here is the re-read one (for example USN04 easy at 676, Shoho at 599,
the scuttle at 828).

`tools/verify_report_calls.py` does not apply: this doc makes no call-site claims. The native addresses are
`src/mission_lua_host.cpp` table rows.

## 4. Open questions for cc9

- Is `chosen_difficulty()` worth a harness option? Difficulty 0 likely selects a non-zero dive-aim row for the US
  side in USN04, USN13 and USN19. Check each script's skill table before relying on it.
- Should `kLuaListenerCallbackArgsBound` be paired on JM06 24000 next? It is the only hard host gate left on JM06's
  primary 2.
- Should USN15 get a first idle row? It is the only mission in the list with no run at all.
