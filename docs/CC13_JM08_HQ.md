# JM08: what makes the HQ fall, primary 1, and the natives the logic uses (cc13-mission2, 2026-10-12)

This is a read-only investigation for cc9. No source, ledger or Ghidra state was changed. It builds on
docs/SQUADRON_LAND_TASK.md sections 5fg and 5fh.

**Status of the evidence:**
- The script quotes are read from this installation's files.
- The host behaviour is read from the source at origin/main `fa36636cd`.
- The outcome table comes from cc9's existing run logs, which were grepped. No game was run.
- One image routine was decompiled (008975B0 and its callee 006C8840).

## 1. The script and its helpers

The mission file is `scripts/missions/COTP-IJN/PRCPIJN/prcpjm08.lua` (mtime 2024-08-26 16:10:32, 1356 lines).
- It differs from the sibling `PRCPIJN/jm08.lua` (2024-07-13) in three lines only: `Mission.ScriptFile`, `this.Name`
  and the message map.
- The helpers are in `scripts/global/commandhelpers.lua` (mtime 2024-10-29, so it is modified in this installation).
- The party constants are in `scripts/global/luamw_init.lua` (2024-07-13): `PARTY_ALLIED = 0`, `PARTY_JAPANESE = 1`,
  `PARTY_NEUTRAL = 2` (lines 73-75).

All line numbers below are from prcpjm08.lua unless another file is named.

### 1.1 The fail condition

```
1320: function CheckHQ()
1321: 	if Mission.CommandBuilding.Party == 2 then
1322: 		Fail()
1323: 	else
1324: 		luaDelay(CheckHQ,1)
1316: function Fail()
1317: 	luaMissionFailedNew(Mission.CommandBuilding, "Mission Failed - The HQ has been destroyed!")
```

- `Mission.CommandBuilding = FindEntity("Headquarter 01")` (line 109). `Mission.HQ` is the same entity (line 493).
- `CheckHQ` is started once by `luaIntroMovieEnd` (line 642) and polls every second for the rest of the mission,
  including phase 2.
- **The HQ falls when its Lua `Party` field becomes 2 (neutral).** No timer, arrival count or script capture rule is
  involved, and no script function writes the HQ's party.
- That is the only fail path in the file. `luaObj_Failed` is never called, and nothing fails on Missouri, the
  landships or the Hosho.

### 1.2 How the party becomes 2 (engine side, not script)

The HQ is a CommandBuilding (kind 1Ch). include/bsp/command_building_capture.hpp and the `kCommandBuildingCaptureBound`
comment in src/game_hosts_ship_ai.cpp cite docs/SHIP_AI_OPEN_ITEMS.md sections 78-81 for this chain:
- The health-changed slot 006F3270, at health <= 0 on an owned building, runs the neutralize route 006F2940 and then
  006F4D10 (D3h, slot 9). That sets party 2 instead of killing the building.
- 006F4D10 then calls `vtable[2Ch]`. For CommandBuilding (vtable 00CFB028, slot 00CFB054) that is 00951F30, a JMP
  to 00928F50, which rewrites the Lua `Party` and `Race` fields. The host mirrors this behind
  `kCommandBuildingPartyLuaMirrorBound = true`.

So the HQ falls when its health reaches 0. A capture is not needed:
- the neutral building's capture tick (006F6760, which needs ships or landed craft) would only matter after
  neutralization;
- `CheckHQ` has already failed the mission by then.

In cc9's logs the neutralize line is `command building level: unit=Headquarter 01 3 -> 0 (neutralize D4h 006F5362) at
t=...`, at max hp 12000.

### 1.3 Who brings the health to 0 (script orders)

Three script orders point damage at the HQ:

1. **Nevada.**
   ```
   919: function SpawnNevadaFleet()
   921: 	Mission.Nevada = GenerateObject(Mission.NevadaFleet[1].Name)
   926: 	NavigatorAttackMove(Mission.Nevada, Mission.HQ, {})
   ```
   This is reached through the chain intro end -> `luaDelay(StartTimer,60)` (632) -> `luaStartDialog("SOS")` (886).
   The SOS dialog's callback is `HoshoTime` (lines 274-285), and `HoshoTime` runs
   `luaDelay(SpawnNevadaFleet,20)` (891).
   - Nevada (with San Francisco in formation, line 924) is **not** in `Mission.InvasionForce`, so the retreat in
     `CheckCompletion` never recalls it.
2. **The invasion force.**
   ```
   757: function CheckInvasion()
   758: 	local Invasion = luaGetShipsAroundCoordinate({["x"] = 0, ["y"] = 0, ["z"] = 0}, 300, PARTY_ALLIED, "own")
   760: 		if Invasion ~= nil then
   761: 			luaDelay(StartInvasion,1)
   767: function StartInvasion()
   769: 	for idx,unit in pairs(Mission.InvasionForce) do
   771: 			NavigatorAttackMove(unit, Mission.HQ, {})
   775: 	for i = 1,6 do
   777: 			NavigatorMoveToPos(Mission.APs[i], Mission.LandPoints[i])
   ```
   - The invasion starts when any living allied ship (destroyer, cruiser, battleship, cargo, landingship, ...) is
     within 300 m (horizontal) of the world origin.
   - `luaGetShipsAroundCoordinate` (commandhelpers.lua 408-465) reads the engine-filled global `recon[party][allegiance]`
     and returns `nil` when nothing matches (lines 460-461).
   - The force is Missouri plus 17 ships (lines 497-515). It is in formation on Missouri (lines 542-544), which starts
     at (0, 6000) in the host log.
   - In cc9's `l50_w3_j8.log`, `StartInvasion`'s first `NavigatorMoveToPos` falls near t = 705 s.
3. **The six troop transports (APs).**
   ```
   825: function CheckAP1()
   827: 		if luaGetDistance(Mission.APs[1], Mission.LandPoints[1]) < 200 then
   828: 			NavigatorAttackMove(Mission.APs[1], Mission.HQ, {})
   ```
   - The radius is 200 m for APs 1-4 and 300 m for APs 5-6 (lines 825-883). The landing navpoints are at z = -300,
     x = ±500/1000/1500 (host scene markers).
   - The image's transport logic then launches its landing craft (`LandingShip`). They take the HQ's pads, lower their
     ramps and bleed the HQ: `LandedDamage` minus armour per craft-second, which is 40 - 36 = 4 hp/s per craft in the
     logs (00749B20 -> 006F1F20 -> 007470B0 -> AddDamage). This is engine behaviour that the host binds
     (`kLandingShipLandedRemainderBound`, sections 161 and 169). It is not script.

The script uses no HQ health threshold. `GetHpPercentage` is read only for Missouri (`CheckFlagship50/20`, lines
809-823, dialogue only) and in `GetHPPercents` (651-662, a display value).

### 1.4 Primary 1, primary 2, and the win chain

```
631: 	luaObj_Add("primary",1,Mission.LandShips)
727: function CheckPrim1()
728: 	if table.getn(luaRemoveDeadsFromTable(Mission.LandShips)) == 0 then
729: 		luaStartDialog("ShipsSunk")
730: 		luaObj_Completed("primary",1,true)
734: 		luaDelay(CheckPrim1,1)
```

- **Primary 1 ("landingships", text `ijn08.obj_p2`)** completes when every entry of `Mission.LandShips` has
  `.Dead` true. `luaRemoveDeadsFromTable` (commandhelpers.lua 1582-1605) keeps entries with `not value.Dead`. There
  are 11 entries: USTroopTransport 01-06, LST 01-03 and LSM 01-02 (lines 520-531).
  - The landing craft the transports launch are not in the table and do not count.
  - There is no fail condition for primary 1.
  - `luaObj_Completed` (5898-5932) does nothing unless the objective is active. `luaObj_Add` sets `Active = true`
    (5780/5808), and the intro end calls it at line 631.
- **Primary 2 ("Missouri")** is `Mission.Flagship.Dead` (`CheckPrim2`, 747-755).
- **The win chain:**
  - `CheckCompletion` (790-803) waits for both successes. `luaObj_GetSuccess` (6007-6019) is pure Lua over
    `Mission.Objectives`.
  - It then sends every live invasion-force unit to `RetreatPoint` with `NavigatorMoveToRange` (795) and, 2 s later,
    starts `luaStartDialog("Run")`, whose callback is `FadeIn` (lines 332-343).
  - `FadeIn` runs `PreparePhase2` 5 s later, which starts bomber wave 1 and `Phase2Movie`, which adds primary 3.
  - Waves 2 and 3 follow 180 s apart (1080, 1183).
  - `CheckPrim3` (1300-1310) wins when every C-47 (`Mission.Dakota`) is dead, and calls `Victory` 2 s later.
- **Phase 2 still threatens the HQ:**
  - the B-17s (1077), the SBDs (1178) and the C-47s (1272) get `PilotSetTarget(unit, Mission.CommandBuilding)`;
  - Nevada and San Francisco, if alive, keep their attack-move on the HQ;
  - `CheckHQ` is still polling.

## 2. What the runs show (cc9's logs, grepped by `local/ms2_j8tab.py`)

Columns:
- **HQ neutral** is the time of the neutralize line.
- **Missouri** and **Nevada** are their death-row times.
- **Landships dead** counts the 11 `LandShips`.
- **Ramps** are the `landing ship ramp: ... lowered` lines, which are mostly the transports' `LandingShip` craft.

| run (cc9-lua50) | HQ neutral | Missouri | Nevada | landships dead | ramps lowered |
| --- | --- | --- | --- | --- | --- |
| w1, w2, w3, x4, r7 (identical) | never (2400 s) | 740 | 997 | 10/11 (TT 04 alive) | LST 02 at 1204 only |
| r5 | never | 756 | 980 | 10/11 (LST 03 alive) | none |
| t5 / t5o | **1363.5** | 702 | 529 | 6/11 | 7, from 1116 |
| t5r | **1248.0** | 704 | 563 | 6/11 | 8, from 1072 |
| y1 | 1137.8 | 688 | alive | 5/11 | 6, from 987 |
| y2 | 1031.0 | alive | 981 | 9/11 | 3, from 1035 |
| x1 | 990.2 | alive | alive | 9/11 | 1, at 1000 |
| r2 | 995.9 | alive | alive | 8/11 | 8, from 987 |

The other 32 cc9-lua50 JM08 logs all fall between 993.7 and 1257.1 s, and each has Missouri, Nevada or both alive.

**"Both Missouri and Nevada die" is not sufficient.** t5, t5o and t5r lose both by about 700 s and the HQ still
falls at 1248-1363 s:
- Three to five transports survive and their craft land from 1072-1116 s.
- `summary mission landing ship landed remainder` gives `bleed_total=6929.70` in t5 against the HQ's 12000 hp. In y1
  it gives 2619.60.

**Every survivor also has 10 of the 11 landships dead and at most one ramp lowered.** The rule the data supports is
cumulative damage:
- Nevada from about 660 s;
- Missouri and the escorts after `StartInvasion`;
- the craft bleed, from the first craft's ramp at about 990-1130 s.

The HQ holds only when the heavy shellers die early enough **and** the transports die before their craft land.

## 3. Natives the deciding logic uses, and whether the host binds them

Each address is the native's row in the registration table at 00E0B7B8 (src/mission_lua_host.cpp). "Bound" means the
host dispatches it, either in `binding_trampoline` (src/game_hosts_lua.cpp, by address, with its gate flag) or in the
ScriptOrderBinding table (src/game_hosts_script_orders.cpp, by name). The check was made with `local/ms2_bound.py`
and the run logs' `host MissionLuaNative::X [...] concrete|UNIMPLEMENTED` lines.

**Bound, and on the deciding path:**

| native | address | how it is dispatched |
| --- | --- | --- |
| FindEntity | 00898E30 | game_hosts_lua.cpp 7472, concrete in the logs |
| NavigatorAttackMove | 008A30D0 | orders |
| NavigatorMoveToPos | 008A2BC0 | orders |
| NavigatorMoveToRange | 008A2F20 | orders, reached in g35_blon at 987 s |
| JoinFormation | 00899D10 | orders |
| GenerateObject | 00944FD0 | trampoline |
| Countdown | 008B16E0 | orders |
| StartDialog | 008B0540 | orders; the SOS and Run callbacks run |
| GetHpPercentage | 0088E9D0 | orders |
| GetPosition | 008A7B00 | orders |
| SetSkillLevel | 00895250 | orders |
| SetShipSpeed | 00890D30 | `kLuaSetShipSpeedBound` |
| AddAirBaseStock | 00896A90 | `kLuaAddAirBaseStockBound` |
| Objectives_Add | 008CD440 | trampoline |
| Objectives_Completed | 008BD340 | `kObjectiveStatusBound` |
| HideScoreDisplay | 008C24B0 | orders |
| SpawnNew | 0094C480 | trampoline |
| PilotMoveTo | 008A4150 | orders |
| PilotSetTarget | 008A4C90 | orders |
| SquadronSetTravelAlt | 0089F550 | trampoline |
| SquadronSetAttackAlt | 008A22B0 | trampoline |
| SetForcedReconLevel | 008AA8F0 | trampoline |
| Kill | 008AC5C0 | trampoline |
| SetInvincible | 00897A50 | trampoline |

The Lua fields the script reads are `.Dead` (game_hosts_lua.cpp and game_hosts_script_orders.cpp write it), `.Party`
(the capture mirror, above) and the `recon` table (`publish_recon_slot_tables_00806b10`). In cc9's g35_blon_j8r7 run
all 11 landships die at 987.03 s. The log then shows:
- `Objectives_Completed name="landingships"`;
- `HideScoreDisplay(2, 0)`;
- the `NavigatorMoveToRange ... RetreatPoint` orders;
- `StartDialog("Run")`.

So primary 1, `CheckCompletion` and the retreat work in the host. That run is 20000 frames, so it ends at 1000 s,
before `FadeIn`.

**Unbound or partly stubbed:**

1. **RemoveAllAirBasePlanes 008975B0: unbound, and gameplay-relevant.** Line 908 calls it on the Hosho before
   `AddAirBaseStock(Hosho, 150/158/162, 21/12/18)`.
   - The image reads the entity's air-ops block and calls 006C8840 (decompiled live). 006C8840 walks the list at
     block+44h, calls 006C7B10(type, count) for each node, empties the list, zeroes +48h and frees the nodes. So the
     image's stock afterwards is only the 21/12/18 that the script adds.
   - LABELLED: that +44h is the same list `AddAirBaseStock` extends is inferred from the pairing, not traced.
   - The host records the call (`RemoveAllAirBasePlanes ... UNIMPLEMENTED calls=1`) and keeps the authored stock:
     `AddAirBaseStock 00896a90: ... class 150 +21 -> 31`, `158 +12 -> 22`, `162 +18 -> 28` (l50_w3_j8.log).
   - **The host's Hosho therefore carries about 10 more of each type than the image's**, including 10 more Kates,
     the torpedo bombers that did about 5600 of Nevada's 10000 hp in 5fg's g12.
   - The deck slots loaded at spawn (`slots=150x3,162x3,158x3`) are a separate structure and are not touched by
     006C8840.
2. **SetRoleAvailable 008AB850: the binding is concrete, but its inner `RoleOwner::set_role_available` (008ABA51) is
   an UNIMPLEMENTED record.**
   - The script uses it at lines 112, 904-906 and 982-984: it makes the airfield, the Hosho group and the movie plane
     AI-only, then hands the Hosho group to the player (`PLAYER_ANY`) when the Hawaii fleet arrives.
   - Whether this changes AI control in the host is not verified here.
3. **DisplayUnitHP 008C1B10: unbound.** It is the HUD bar only (line 629) and decides nothing.
4. **End-of-mission natives: unbound, and they do not decide the outcome.** Scoring_SetMissionCompleted 008B8AD0 and
   BannSupportmanager 0088C750 are reached only inside `luaMissionFailedNew` / `luaMissionCompletedNew`, after the
   decision.
5. **Not reached in single player:** Objectives_ClientRefresh 008BDEC0 (multiplayer only, lines 5929-5930) and
   AddPowerup 008EE410 (`luaJM8AddPowerup` is defined but never called).

## 4. Answers for cc9

- **The HQ falls** when its health reaches 0. The engine then neutralizes it (party 2, 006F3270 -> 006F2940 ->
  006F4D10), and `CheckHQ` (1320-1325) fails the mission on `Party == 2`. There is no timer, arrival count, script
  capture rule or health threshold.
- **The damage comes from three sources, all ordered onto the HQ by the script:**
  - Nevada: `SpawnNevadaFleet`, 20 s after the SOS dialog's `HoshoTime` callback;
  - Missouri and its 17-ship force: `StartInvasion`, once an allied ship is within 300 m of the origin;
  - the six troop transports' landing craft, after `CheckAPn`, bleeding about 4 hp/s each once landed.
- **Survival in the logs needs the transports dead as well,** because the both-dead runs t5 and t5r still fall.
- **Primary 1** is all 11 `LandShips` `.Dead` (USTroopTransport 01-06, LST 01-03, LSM 01-02). It has no fail
  condition. The only HQ-tied fail is `CheckHQ`.
- **A win** also needs Missouri dead (primary 2). After that comes phase 2: the HQ must survive the B-17, SBD and
  C-47 strikes (and Nevada, if alive, because the retreat does not recall it) until every C-47 is dead.
- **Unbound natives:** the one that matters is RemoveAllAirBasePlanes 008975B0, which makes the host's Hosho about 10
  planes per type stronger than the image's. The others are SetRoleAvailable's inner record, DisplayUnitHP, and the
  end-of-mission scoring natives, none of which decides the outcome.

## 5. Handoff

- Everything is on `agent/cc13-mission2`.
- Scripts, none of them for commit: `local/ms2_j8tab.py` (the run table), `local/ms2_bound.py` (the dispatch audit)
  and `local/ms2_calls.py` (the Lua callee lister).
- **Open items:**
  1. Bind RemoveAllAirBasePlanes. This is cc9's lane, because the Lua binding hosts are cc9's.
  2. Read 006C7B10 to confirm what each node release does.
  3. Check whether the RoleOwner record changes who drives the Hosho group.
