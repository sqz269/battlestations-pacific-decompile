# Scope of the remaining host-omitted plane tasks and Lua Spawn placement (cc13-scope2)

Addresses: 0099A170 0099A24A 0099A43C 0099A462 009BDBB0 009BDC10 009BCE50 009BDA30 009BC780 009BC960 009BD6E0
009A6970 009A69E0 009A5240 009A4DC0 009A3390 009A36A0 009A3090 009A31A0 009A39C0 009A3C60 009A5330 009A66C0
009A2F40 009A2FB0 009A2730 009A2500 009A2810 009A1D60 009A1F00 009A1FC0 008A4960 008A3E70
00944680 00941FA0 009426D0 00942CB0 004BC120 00844FC0 00E08F38 00E08F80 00E08FA8

Packet `cc13-scope2` (scoping read), branch `agent/cc13-scope2`, 2026-10-12. Requested by cc9 through the cc13
lead. **Read only.** It reads this installation (`I:\SteamLibrary\steamapps\common\Battlestations Pacific`,
modded, mtimes cited) and the repository. It runs no game scripts and writes nothing to Ghidra or the ledgers. It
follows docs/CC13_ROCKET_KAMIKAZE_SCOPE.md, which covers the rocket and kamikaze tasks.

**The question.** docs/CC13_RNG_STREAM_CENSUS.md sections 10.4 and 11.1 list four more families as
HOST-OMITS-FUNCTION:
- the plane **depth-charge** bot task, 5 + 6 stream-1 sites;
- the plane **moveonpath** bot task, 1 + 4 sites;
- the plane **closetoship** bot task, 2 sites;
- the Lua **`Spawn`** placement search, 1 site.

For each family this doc gives the image entry points, the host status, what triggers it in this installation, the
missions it reaches, and its size. The families are then ranked by reach into cc9's run list: USN01, USN02, USN04,
USN12, USN13, USNOS, USNRM01, ESMP08, BSM01, JM06, JM08, LOMP06, LOMP10, IJN01, IJN11.

**Tools** (all in `local/`; each reads only the installation, the PE on disk or `local/bsp_index.sqlite`):
- `sc2b_xscan.py`: rel32 CALL/JMP and absolute-dword references in the PE on disk. The positive controls hold:
  `009BDBB0` gives its one call at `0099A24A`, and `00944680` gives its binding-table dword at `00E0BFEC`.
- `sc2b_missions.py`: the mission tree, the scenes and the stage scripts, with per-native call counts and the scene
  `Command` / `CommandTarget` counts.
- `sc2b_dc_classes.py`, `sc2b_dc_scn.py` and `sc2b_spawn_types.py`: the depth-charge classes, the scene units and
  the `SpawnNew` types.
- `sc2b_objtext.py`: objective strings from the mission tree.
- `sc2b_au10_size.py` (a copy of `au10_size.py`) and `sc2b_window.py`: the closure and window sizes.
- The call sites this doc cites are in `reports/cc13_omitted_tasks_scope_calls.json`.
  `tools/verify_report_calls.py` checked its 13 rows, and none failed.

## 0. Ranking (short answer)

| rank | family | on cc9's run list? | where it matters | size (work functions / bytes) | RNG sites |
| --- | --- | --- | --- | --- | --- |
| 1 | **plane moveonpath task** | **Yes, BSM01 and JM06**, each a scripted patrol. Neither patrol is on an objective's critical path | the BSM01 P-40 patrols are targets for the Japanese fighters; the JM06 Catalina patrols from stage init; dormant on USN13 and JM08 (a shipyard Mavis launch, section 2.3) | 31 / about 5.6 KB, window `009BC690..009BDD70`. It rests on the hosted follow and moveto states and the hosted director path build | 5 |
| 2 | **plane depth-charge task** | **No.** No run-list mission has a depth-charge-armed plane and a submarine target together | 12 campaign or BSM entries. USN15 (`USN21`) is confirmed: a Jake is ordered at the player's submarine (`usn_sibuyan.lua:739`), and primary 1 is "Sink Atago and Maya with your submarines!" | 63 / about 14.3 KB, window `009A3090..009A6A60`. 18 functions are already named and its state tables exist as an uncalled library | 11 |
| 3 | **Lua `Spawn` placement** | **No** | challenge missions only: CHG01, CHG02, CHG03 (and its COOP twin MULTI931), CHG05, CHG07, CHG08 | 3 functions, about 4.7 KB (`00941FA0`, `009426D0`, `00942CB0`), plus a shared `004BC120`. The `Spawn` native itself has no host, so placement alone runs nothing | 1 |
| 4 | **plane closetoship task** | **No** | **no single-player reach at all.** The only issuer is the Lua `PilotCloseToShip`, called only by `scripts/missions/multi/competitive07.lua` | 25 / about 5.0 KB, window `009A1C30..009A3090` | 2 |

Answers to the lead's examples:
- **BSM01's depth charges are not the plane task.** BSM01 has no depth-charge-armed aircraft. Its depth-charge
  test is `HasFired(Mission.Henry, "DEPTHCHARGE", ...)` (`bsm_01_stationed_at_pearl.lua:813`): a ship's charges,
  which the gunnery lane hosts.
- **Scripted moveonpath convoys are ship orders.** The ship `moveonpath` path is hosted (ship AI,
  `NavigatorMoveOnPath`). The omitted piece is the **plane** bot task that 0099A170 builds for a plane holding a
  `moveonpath` command.

## 1. The plane moveonpath task

### 1.1 Image entry points

- **Factory.** `0099A170` (`BSP_Bot_InstallCommandTask`) has a `moveonpath` arm (`00E08F80`) that calls `009BDBB0`
  at `0099A24A`. The rel32 scan finds this as `009BDBB0`'s only caller. `009BDBB0` runs `operator_new` and then the
  constructor `009BCE50` at `009BDBE9`.
- **Load-path factory.** `009BDC10` builds through `009BDA30` (the stream variant). It has no caller in the call
  graph.
- **Constructor.**
  - `009BCE50` calls `BSP_BotTask_ConstructBase` `0099C6F0` and `BSP_BotApproach_BindToTask` `009F9980`.
  - Through `009BCC60` it builds the state registry: `009BC780` (the approach constructor, calling
    `BSP_BotApproach_ConstructSpeedReference` `009F9CE0`), `009BC960`, and the shared
    `BSP_BotStateFollow_Construct` `009C2980`.
  - It calls `BSP_Unit_IsSquadronFlightLeader` `007B8AD0`, as every task constructor does (docs/BOT_TASKS.md).
- **Vtables** (from the closure): `00D20838`, `00D208BC`, `00D208D8`, `00D208F8`, `00D20900`, `00D20910`.
  `009BD6E0` is a slot at `00D20964` and `009BD1D0` a slot at `00D20914` (absolute dwords, `sc2b_xscan.py`).
- **Kind id.** Not read. docs/BOT_TASKS.md lists `009BCE50` among `0099C6F0`'s nine non-attack callers, whose kinds
  were never read.
- **RNG sites** (census): `009BCECE` (constructor, `-U(0, x)` into `+51Ch`), `009BC7C0` (`009BC780`), `009BC9AE`
  (`009BC960`), `009BD700` (`009BD6E0`) and `009BDAB8` (`009BDA30`, load path only). So a constructed task draws
  three or four stream-1 values. If `009BD6E0` is a tick (not read), it draws more per tick.

### 1.2 Host status

- `kPilotMoveOnPathBound` is **ON** (`include/bsp/game_hosts_script_orders.hpp:256-266`).
  `PilotMoveOnPath` (`008A3E70`) runs the `NavigatorMoveOnPath` body without its speed half
  (`src/game_hosts_script_orders.cpp:3250`). The plane's director takes the `moveonpath` command and the path is
  built.
- `0099A170`'s host (`ScriptOrderAttackCommandHost::create_bot_task`, `src/game_hosts_script_orders.cpp:1197` ff.)
  records a `(unit, command, target)` triple for any class. It builds no task.
- `src/game_hosts_units.cpp` has no moveonpath task flag. The installed-task flags are dive bomb, dogfight, land,
  level bomb, moveto, retreat, strafe, takeoff, torpedo, and the never-set `torpedo_closetoship_task_installed`.
- docs/GAME_SHIP_NAVIGATION_BINDING.md ("Where 5Bh lands on a plane", and the 2026-09-27 pairs) labels and
  measures the gap. JM06 and BSM01 were exit 1 with the binding ON: "Flight is unchanged, as labelled". **The
  plane keeps flying whatever task it already had.**
- **Status: a contract and a factory pointer (`make_moveonpath_009bdbb0`,
  `include/bsp/native_pilot_bot_command_producer.hpp:50`). Nothing is reconstructed.**

### 1.3 What issues `moveonpath` to a plane (image)

The absolute-dword scan of `00E08F80` finds 42 sites. The issuers among them:
- the Lua natives `PilotMoveOnPath` and `NavigatorMoveOnPath`: the 5Bh message, applied by
  `BSP_WeaponDirector_ApplyGameUnitMessage` `00721A40`;
- scene `Command = E CommandType : MoveOnPath` blocks, through `BSP_UnitInstance_ApplyEntityCommand` `00816E30`
  and `BSP_PlaneSquadronCommandBlock_Step` `0084E010`;
- `BSP_Shipyard_CreateLaunchedUnit` `00844FC0` (`008454BF`): **every shipyard-launched unit, a plane included,
  gets `moveonpath` on its hangar path.**

The rest of the 42 read or compare the class: the director, the HUD and the suppression predicate.

### 1.4 Reach in this installation

| mission | call | what it moves | critical path? |
| --- | --- | --- | --- |
| **BSM01** (`bsm_01_stationed_at_pearl.lua` 2024-07-13) | `PilotMoveOnPath(P-40 Warhawk 01 / 02, PatrolPath1 / 2, PATH_FM_CIRCLE)` at `:1877` and `:1880`, in `luaGenerateHarborTrafic` (`:1818`) | two P-40s circling over the harbour. They are added to `Mission.JapFighterTrgGang` (`:1879`, `:1882`), the pool `:1514` picks the Japanese fighters' targets from | **Incidental.** No objective names them. They change which units the Japanese fighters chase, and therefore the dogfight and AA numbers |
| **JM06** (`JM06.lua` 2024-07-13) | `PilotMoveOnPath(Mission.Catalina, CatalinaPatrolPath)` at `:118`, stage init. `:2392` is checkpoint restore only | `PBY Catalina 01` patrols a 12-point path. Nothing else reads `Mission.Catalina` | **Incidental.** Secondary 2 is a different Catalina (`Mission.Catalina2`, spawned by `luaJM6SpawnCatalina`) |
| **USN13**, **JM08** | none in the scripts. The scene shipyard stocks a `Mavis` (`Shipyard Entity CB3` in `usn_13_truk.scn` 2024-10-29; `Shipyard 01` in `prcpijn_08_defend_guadalcanal.scn` 2024-08-09) | a built Mavis would leave on `moveonpath` | **Dormant.** In single player nothing sends the A9h build order (docs/GUNNERY_OPEN_ITEMS.md 136.3 item 1, 146: "No `build` line, and nothing sends A9h"). Only a harness `build` line naming the Mavis reaches it. The same holds for LOMP10's `CB4_SY` (the 146 pair built Elcos, which are boats) |
| USN01, USN02, USN04, USN12, USNOS, USNRM01, ESMP08, LOMP06, LOMP10, IJN01, IJN11 | no `PilotMoveOnPath`. Every `NavigatorMoveOnPath` first argument in these scripts is a ship, PT boat or submarine (read one by one). No scene `MoveOnPath` command | - | - |

**Off the run list:** CHG09 (`chg_9_coup.lua`, 2 calls), BSM03 (1 call, plus 4 scene `MoveOnPath` commands whose
units were not resolved), JM01 (3 calls) and JM12 (`JM11.lua`, 1 call).

### 1.5 Size

- **Window** `009BC690..009BDD70`: 38 functions, 5856 bytes; 31 excluding CG, unwind and STL, 5632 bytes; none
  named, none reconstructed.
- **Largest:** `009BD6E0` 752, `009BD4D0` 528, `009BDC10` 352 (load path), `009BD1D0` 304 and `009BCE50` 288 bytes.
- **Closure** from `009BDBB0`/`009BDC10`/`009BCE50`/`009BDA30`: 21 functions, 2944 bytes, 6 vtables, 31 outside
  callees. Many of those callees are already hosted or named:
  - `009C2980` follow;
  - `0099C6F0` base;
  - `009F9980` and `009F9CE0` approach;
  - `0071E430`, from the director path family.
- The closure misses some vtable slots (`009BD6E0` among them), so read the window figure as the bound.

## 2. The plane depth-charge task

### 2.1 Image entry points

- **Factory.**
  - `0099A170`'s `depthcharge` arm (`00E08F38`, gate: target `IsKindOf(8)`, a submarine; docs/ATTACK_COMMANDS.md)
    calls `BSP_BotTask_MakeDepthCharge` `009A6970` at `0099A462`. That is its only caller.
  - `009A69E0` is the load-path factory and has no caller.
- **Task.**
  - `BSP_BotTaskDepthCharge_Construct` `009A5240`: kind 1, size `7BCh`, vtables `00D1F738` / `00D1F734` /
    `00D1F730`.
  - `BSP_BotApproachDepthCharge_Construct` `009A3390`: the approach, reached through `009A4DC0`.
  - `BSP_BotTaskDepthCharge_RegisterStateNames` `009A3090`: nine states (docs/BOT_TASKS.md).
- **Ticks and states:** `BSP_BotTaskDepthCharge_Tick` `009A66C0`, `_UpdateState` `009A5870`,
  `BSP_BotApproachDepthCharge_Update` `009A5F50`, `BSP_BotStateDepthChargeAim_Tick` `009A4010`,
  `BSP_BotStateDepthChargeAttackRun_Tick` `009A3770`, `_UpdateCruiseProfile` `009A6500` and `_ShouldBreakOff`
  `009A65F0`.
- **Chooser side.** `007EEC50` and `007EE8F0` pick the class when the plane carries ordnance kind `2Ch` (bullet
  `Type "Depthcharge"`). `009F8160` revalidates it. These are hosted in `src/attack_commands.cpp`.
- **RNG sites** (census):
  - `009A33F8`, `009A3440`, `009A346F`, `009A34AA` (approach constructor);
  - `009A36FA` (attackrun constructor);
  - `009A31D1`, `009A3201`, `009A3281` (`009A31A0`, called from `009A3390` at `009A3573` and from two more
    approach helpers);
  - `009A39DE` (`009A39C0`, vtable `00D1F66C`);
  - `009A3CD3` (`009A3C60`, vtable `00D1F6A4`);
  - `009A5360` (`009A5330`, from `009A5D00`).

### 2.2 Host status

- **Records and an uncalled library:**
  - the class row in `src/bot_tasks.cpp:12` and `kDepthChargeCruiseProfile` (`:53`);
  - the state names, offsets and entry-state rules in `src/bot_task_states.cpp:16-121`;
  - the chooser arm and `make_depthcharge_009a6970` in `src/attack_commands.cpp:615-617`.
  `git grep` finds no caller of the `depth_charge_*` helpers outside their own file.
- There is no task flag or tick in `src/game_hosts_units.cpp`.
- **The munition is hosted.** The depth-charge projectile's scan and burst are in `src/bomb_torpedo_tick.cpp:132-136`
  and the gunnery blast. Only the plane's attack run is missing.

### 2.3 Classes (this installation)

Sources: `vehicleclasses.lua`, and the arcade `bulletclasses.lua` / `deviceclasses.lua`, all three with mtime
2026-05-09.

- **Bullets of `Type "Depthcharge"`:** 2, 12, 32, 54, 55, 57 and 58.
- **Aerial platforms:** device 77 "DC platform US" (bullet 55) and device 86 "DC platform Japan" (bullet 58).

| class | VehicleClass | DC set | DefaultEquipment | so a default unit is |
| --- | --- | --- | --- | --- |
| `Kingfisher` | 121 OS2U | 1 | 1 | **armed** |
| `Pete` | 171 F1M | 1 | 1 | **armed** |
| `Jake` | 172 E13A | 2 | 2 | **armed** |
| `Seiran` | 34 M6A | 4 | 2 | unarmed |
| `Helldiver` | 38 SB2C | 2 | 1 | unarmed |
| `Swordfish` | 109 | 3 | 1 | unarmed |
| `Catalina` | 125 PBY | 2 | 1 | unarmed |
| `Judy` | 159 D4Y | 2 | 1 | unarmed |
| `Mavis` | 174 H6K | 3 | 1 | unarmed |
| `Emily` | 175 H8K | 3 | 1 | unarmed |
| `BTD_Destroyer` | 331 | 4 | 1 | unarmed |
| `BlackCat` | 343 | 2 | 1 | unarmed |
| `D4Y3` | 361 | 2 | 1 | unarmed |

**The three float planes carry depth charges by default.** Every other class needs an explicit `Equipment`.

### 2.4 Reach

The task needs both a depth-charge-armed plane and a submarine target.

**Run list: none.**

| mission | depth-charge-armed planes | submarines | verdict |
| --- | --- | --- | --- |
| IJN11 | 3 Jake, 6 Kingfisher, 3 Pete, all USA, as **airfield stock** inside a `Landscape` entity | none in the scene. No `SpawnNew`. The player's classes are cruisers and destroyers (270, 357, 359, 14, 290) | unreachable |
| USN04 | 1 hidden `moviefisher` Kingfisher, a movie prop | none | unreachable |
| LOMP06 | the scene's hidden Pete template is armed. The script's seaplanes are `SpawnNew` Type 171 with `Equipment = 2` (`06_crucial_cargo.lua:376-385`, 2024-07-13), which is **not** the depth-charge set. The script expects `divebomb` against the surfaced Narwhal (`:757`) | Narwhal, the player's | unreachable as authored |
| JM06 | `SpawnNew` Catalina, set 1 (unarmed) | I400 (spawned), 6+6 Japanese, 2 USA | unreachable |
| USNOS | `SpawnNew` D4Y Judy, set 1 (unarmed) | 6 Kaiten (spawned), 1 hidden USA | unreachable by scene or script. Player-chosen loadouts were not considered (cc9's runs use an idle player) |
| BSM01, IJN01, USNRM01, USN13 | none | present | unreachable |

**Off the run list** (`sc2b_dc_scn.py`; scene co-presence of an armed plane and a submarine, plus script orders
where they were read):
- **USN21, "USN15 Battle of the Sibuyan Sea"** (`usn_sibuyan.lua` 2024-10-29). Confirmed. `luaJakeTime` generates
  `JakeTmpl` (set 2, armed) every 60 s in phase 1 and calls `PilotSetTarget(jake, subm)` on a random player
  submarine (`:721-739`). The player's primary is "Sink Atago and Maya with your submarines!" **This is the threat
  to the objective.**
- **IJN12 Attack on the Panama Canal** (`ijn_12_panama.scn` 2024-10-29): 3 Kingfisher on the default set (armed)
  and the player's I400 / I501 / I601. The Catalinas (set 2) are armed too. The hidden objective is "Destroy the
  patrol planes!" No script order was found (`ijn_12_panama.lua` has no `PilotSetTarget`), so the attack would come
  from the AI's own targeting. Not traced.
- **BSM11 Endgame at Midway** (`bsm_11_endgame_at_midway.scn`): 2 Catalina on set 2 and the Japanese I-25. The
  secondary is "Destroy the attacking Japanese submarine!" The script's `PilotSetTarget` (`:748`) is for the
  Emilys against carriers.
- **The rest, scene co-presence only:** BSM03, CHG07, CHG08, JM01, JM14, PRCP04, PRCP07, USN11 and IJN09.

### 2.5 Size

- **Window** `009A3090..009A6A60`: 75 functions, 14800 bytes; 63 of them work functions, 14320 bytes; 18 named;
  none reconstructed.
- **Largest:** `009A4010` 1632, `009A5F50` 1456, `009A4940` 928, `009A5870` 640 and `009A4DC0` 576 bytes.
- **Closure** from the factory, the constructor, the approach constructors, the state-name registration and the two
  overrides: 40 functions, 10688 bytes, 9 vtables, 65 outside callees.
- This is the largest of the four families. It is also the best documented one: docs/BOT_TASKS.md,
  docs/BOT_TASK_STATES.md and docs/PILOT_TASK_HEADING_ARM.md read its states, and the torpedo task shares its shape.

## 3. The Lua `Spawn` placement

### 3.1 Image entry points

- **The native.** `Spawn` = `BSP_LuaBinding_Spawn` `00944680`, binding-table dword at `00E0BFEC`.
- **The placement.** It calls:
  - `00941FA0`, the placement search with the RNG site `00941FC0`, at `00944C44`;
  - `009426D0`, at `00944C64`, `00944CEF` and `00944D36`;
  - `004BC120`, at `00944CC4`.
- **Direct placement.** With the trailing boolean set, the native calls `00942CB0` at `00944EF6`. That routine
  calls `00941FA0` again at `00942D25`, so both branches draw.
- **Callers.** The rel32 scan gives `00941FA0` two callers (`00944680`, `00942CB0`). `00942CB0` and `009426D0` are
  each called only from `00944680`. `004BC120` is shared with `008AF700`.
- `SpawnNew` (`0094C480` -> `00949750`, drained by `0094C490`) does not use this search.

### 3.2 Host status

- `include/bsp/lua_binding_spawn.hpp` and `src/lua_binding_spawn.cpp` (packet cc_lua_core) reconstruct the
  argument parse and the creation tail of `lua_binding_spawn`. They do this against an abstract
  `LuaBindingSpawnHost`.
- `find_spawn_position` and `place_spawn_directly` are pure virtuals marked `contract: unread` (`:225-234`).
- **No class in `src/` implements `LuaBindingSpawnHost`.** The `Spawn` row exists in the native table
  (`src/mission_lua_host.cpp:278`), but no game host dispatches it. `SpawnNew` is hosted
  (`src/game_hosts_lua.cpp:3250`, `run_spawn_new_00949750`).
- **Reconstructing the placement therefore requires the native's host as well:**
  - the creation seam `004C6BA0`;
  - `00874D00`;
  - the entity tail.
  The SpawnNew lane already has most of these.

### 3.3 Reach

- **Run list:** no stage script calls `Spawn`, `luaSpawn`, `luaSpawnDelayed` or `luaSquadronSupply`.
- **Callers in this installation** (non-comment lines). `commandhelpers.lua` (2024-10-29, modded) wraps the native:
  - `luaSpawn` (`:8947`) calls `Spawn(..., true)`, the direct-placement branch;
  - `luaSpawnDelayed` / `luaSpawnDelayedTimer` (`:8686`, `:8768`) and `luaSquadronSupply` (`:13751`) call
    `luaSpawn`;
  - `luaTCReconSaver_think` (`:5500`) calls `Spawn` directly, but nothing in `scripts/` or the scenes names
    `luaTCReconSaver`.
- **Missions:**
  - CHG01 (`chg_1_monster.lua` 2024-10-29: 4 `Spawn` and 4 `luaSpawn` calls; the PT boats, destroyers and cruisers
    around Fuso);
  - CHG02 (`chg_2_java.lua` 2024-08-03, `luaSpawn`);
  - CHG03 and MULTI931 (`chg_3_hunt.lua` 2024-08-03);
  - CHG05 (`chg_5_tulagi.lua` 2024-08-26, `luaSquadronSupply`);
  - CHG07 (`chg_7_cargo.lua`, `PC_cargo.lua`, both 2024-07-13, `luaSpawnDelayed`);
  - CHG08 (`chg_8_periscope.lua` 2024-07-13, `luaSpawnDelayed`).
- **Critical path.** In the challenge missions the spawned units are the opposition. Without the native they come
  back as nil.

### 3.4 Size

- The three routines: `00941FA0` 256 bytes, `009426D0` 1504 and `00942CB0` up to 2912 (to the next function at
  `00943810`; an upper bound), about 4.7 KB in all. The closure from the three agrees: 4672 bytes, 11 outside
  callees.
- **Outside callees:** `00413920`, `00414DB0`, `00419440`, `004BC120`, `0071C4F0`, `00903860`, `00941C20`,
  `00941D30`, `00BD2F10`, `00BF7030` and `00BF79F0`.
- The window `00941FA0..00942D80` is mostly STL instantiations: 17 functions, 5 of them work.

## 4. The plane closetoship task

### 4.1 Image entry points

- **Factory.**
  - `0099A170`'s `closetoship` arm (`00E08FA8`, gate `009229F0`) calls `BSP_BotTask_MakeCloseToShip` `009A2F40`
    at `0099A43C`. That is its only caller.
  - `009A2FB0` is the load-path factory and has no caller.
- **Task.** `BSP_BotTaskCloseToShip_Construct` `009A2730`: kind 0, size `554h`. The approach is
  `BSP_BotApproachCloseToShip_Construct` `009A1D60`. `009A2500` registers the states `moveto`, `follow` and
  `stayclose`.
- **Other functions:** `_UpdateCruiseProfile` `009A2D60` and `_AttackModeCountdown` `009A2810` (called by the
  vtable `+64h` arm `009A2B00`; docs/TORPEDO_ATTACK_MODE.md).
- **RNG sites:** `009A1F54` (`009A1F00`, called at `009A25A3` and `009A26B8`) and `009A20FA` (`009A1FC0`, called
  at `009A232E`).
- **Who issues the class.** The absolute-dword scan of `00E08FA8` finds 9 sites:
  - `BSP_LuaBinding_PilotCloseToShip` `008A4960`, which is **the only one that issues it**;
  - `BSP_LuaBinding_PilotStopCloseToShip` `008A4B10`, which only compares;
  - `0099A170`;
  - `009A28B0` and `009A2920`, inside the task;
  - two static-init and two class-object functions.
- **No AI path builds it.** `007EEC50` never produces `closetoship` (`include/bsp/attack_commands.hpp:223`).

### 4.2 Host status

- `torpedo_closetoship_task_installed`, `_timer_550` and `_latch_43c` are held in `src/game_hosts_units.cpp:1521-1531`
  and consumed at `:25364`. The flag is **never set**.
- `closetoship_attack_mode_countdown_009a2810` is reconstructed and build-tested (`src/torpedo_release_orders.cpp:183`).
- The class row is in `src/bot_tasks.cpp:10`.
- **The `PilotCloseToShip` native is not bound.** It appears only as a name row in `src/mission_lua_host.cpp:176`.

### 4.3 Reach

- **Callers.** `PilotCloseToShip` is called only in `scripts/missions/multi/competitive07.lua` (2024-07-13), at
  `:893` and `:1147` (`PilotCloseToShip(unit, Mission.Yorktown)`). That is a multiplayer competitive script.
- No stage script in the mission tree, and no global helper, calls `PilotCloseToShip` or `PilotCloseToShipDrop`.
- **Single-player reach: none.**

### 4.4 Size

- **Window** `009A1C30..009A3090`: 30 functions, 5216 bytes; 25 of them work, 5008 bytes; 5 named; 1 reconstructed
  (`009A2810`).
- **Largest:** `009A2170` 704, `009A29F0` 480, `009A1FC0` 432, `009A1DC0` 320 and `009A2C50` 272 bytes.
- The closure is 17 functions and 2864 bytes. It includes two out-of-family vtables, `00CF5C20` and `00CF5C94`,
  reached through `009A1CA0`, and the window's lower edge `009A1C30` is not established. Read the window figure as
  the bound.

## 5. Caveats

- **Scene parsing.**
  - Equipment `0` or absent is read as the class `DefaultEquipment`, as in au10.
  - Airfield and hangar stock (`PlaneClasses` inside a `Landscape` or a ship) is counted with its holder's type
    (`Jake@Landscape`).
  - Ship catapult planes that come from the vehicle class rather than from the scene were not enumerated. Some
    run-list cruiser or battleship could launch a default Kingfisher, Pete or Jake. Even then a submarine target is
    still required, and **only JM06, USNOS, BSM01, IJN01, USNRM01 and USN13 have submarines among the run list.**
    JM06's and USNOS's are Japanese. A catapult float plane of the opposing side, in one of those missions,
    attacking unordered through the AI's own targeting was not traced.
- **Player-chosen loadouts and player orders** (hangar equipment cycling, manual attack orders) are outside an
  idle-player run and were not considered.
- **`009BD6E0`.** Ghidra has a function at this address, but its body does not cover the call at `009BD9A1`, which
  `verify_report_calls.py` reports as "in no Ghidra function". The row was left out of the report. This is a
  boundary repair to make before reconstructing the moveonpath task. It is the same section 1.3 pattern the census
  records for `009BD6E0`.
- **Not read.**
  - The moveonpath task's kind id and its tick.
  - Whether `009BD6E0` is the per-tick update.
  - BSM03's four scene `MoveOnPath` command units.
  - The AI's own submarine targeting in IJN12 and BSM11.
  - `009BBAD0` / `009BBF60`, the census's unreferenced manoeuvre factory. A rel32 and absolute scan finds no
    reference to `009BBF60`, so it is dead in the image and outside this scope.
- **Status words.** Nothing here is reconstructed, build-tested or game-validated. This doc is a static scope read
  of the installation and the image.

## 6. Handoff

- **The recommended first packet is the plane moveonpath task.**
  - It is the only family with run-list reach (BSM01, JM06).
  - It is small: about 31 functions.
  - It rests on states the host already runs (follow `009C2980`; the moveto task `009C3000`; the director path
    build `0071F600`).
  - The expected pair effect: BSM01's two P-40s and JM06's Catalina start flying their paths, and their stream-1
    draws add three or four values per construction. That shifts every later stream-1 draw. Land it behind an OFF
    switch first (memory: shared RNG stream couples pairs).
- **The depth-charge task is next** if cc9 adds USN21 (USN15 Sibuyan Sea) or IJN12 to the run list. It is the
  largest family, but most of its states are named and documented.
- **Spawn placement only matters together with a `Spawn` native host,** and only for the challenge missions.
- **Closetoship has no single-player value.**
