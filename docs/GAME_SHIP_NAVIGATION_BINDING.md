# Ship navigation bindings

Project: `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
Descriptive names are hypotheses; process interfaces are not original object ABI.

The executable now runs complete `009E0270` before navigation consumers. It
loads actual class `+570h` from the mission Lua owner before `009E4330`, using
the actual session mode and VehicleClass Type/variant selection documented in
[GAME_SHIP_DEPTH_INPUT.md](GAME_SHIP_DEPTH_INPUT.md). The constructor callback
uses its local navigation fields before assigning the completed block.

The pre-step borrows persistent geometry, twelve sectors, reference speed and
65 profile bytes. Flags `+3E8h/+3E9h/+3EAh` and profile `+45h` now have byte
storage; any nonzero profile flag survives exactly. The constructor supplies
the native cleared bins and flag before calling the pre-step. Its stack
argument is one unused word, not a float delta.

Actual unit reference speed, turn circle at 0.5, full width and cached world
pose supply the core. The represented modifier lists are empty. Native
`009E0270` floors reference speed at the widened `1.4f` comparison while
preserving unordered values. Later obstacle and throttle-ceiling inputs now
retain the produced `+3C4h` value, including the separate explicit input read
at `009EC9AB` that review caught still using raw unit speed.

The dimension producer is documented in [UNIT_HULL_EXTENTS.md](UNIT_HULL_EXTENTS.md).
Class `+A0h/+A4h` supply actual Length/Width for the absent-model path. The
model-local box branch uses twice the maximum absolute extent about the origin,
not max-minus-min. Historical `unit_half_width_09cc` returns the native full
width. Hydro retains the separate raw class width its own producer requires.
Native extent fixtures cover both branches; runtime covers the represented
absent-model owner.

Navigation blocks are now constructed only for resolved ship descriptor kinds
7 through 14. Their actual vtable `+210h` entries reach `00810DD0`, which calls
`009F3F20` to allocate the `2268h` brain through `009F3BA0`. Abstract kind6 is
not a concrete class row. This restricts navigation to the 14 ship instances
among USN01's 77 loaded units. The other 63 retain generic command and director
updates. Original allocation prefixes and resource/session gates remain
separate from this process-owned representation.

The retained `009DE2F0` geometry supplies goal position `+184h`, sector/throttle
position and axes, and clearance shoulders `+18Ch/+194h`. Cached-pose access
rejects an unsupported dirty cache; absence of a model is established by the
represented owner and native `0087BCC0`/`006D1E30` contract. The distance constant
at `00CF1748` is widened `0.45f`, bits `3FDCCCCCC0000000`; the constructor's
`009E45F3` square root uses the recovered CRT without a positive-only guard.

Earlier corrections remain: `009ED902` adds controlled-unit `+9C8h` to braking
distance; `009E58D4` subtracts it from target `+7A0h` range. That target range
producer remains unresolved. Passing-corner diagnostics record the
`009EBF67 ->009D84E0` call only when a blocking neighbour exists.

Win32 Release and both existing CTests passed. The integrated executable
completed 120 USN01 frames, including the Enterprise MoveTo request, with 14
navigation blocks, 3,374 full pre-steps, 2,400 controls extent reads and 48
goal-position reads. All runtime depth inputs selected Single mode0 and the
shipped scalar0. All 77 rows logged loaded dimensions. Shutdown was clean,
with no FMOD errors. Commands, source/executable hashes, exact input rows and
prior verification history are in `reports/game_ship_navigation_binding.json`.

The integrator also reproduced 1,449 original-byte extent comparisons, four
existing original-byte full pre-step fixtures, and the compiled Lua reader
excerpt over 160 supported rows and 473 rejected non-ship rows. The reader
fixture checks ten Single/Multi keys and boundary/error behavior; it is not
original-byte execution of the entire settings loader. No new tracked tests
were added.

Zero corner arms occurred. These checks do not establish model-present runtime
behavior, neighbour avoidance, collision-world integration or gameplay parity.
The pre-step's `+168h` is also not the adaptive planner layer `+30Ch`.

## Follow-up packets

Bind the stateful `009ECA20` producer for `+308h/+30Ch` before changing PathPick's
layer input. It starts with owner virtual `+214h`, which reads distinct class
`+560h`; submarines use the actual `+1268h` selector into that four-value array.
Do not substitute the now-loaded `+570h` scalar. Formation and RNG owners are
under other active packets; recheck leases before expanding this work.

Main's new `unit_kind_query` core supplies native class-test behavior. Its
GameUnitsHost consumers still need a separate binding to remove the Destroyer
ancestry assumption and preserve unresolved class identity explicitly. Keep
the explicit resolved ship guard until that runtime binding is complete.

## A command aimed at a scene marker (packet `cc9_prcp03_phase_progress`)

Worker cc9-ships2, on main `72f785ab6`. Ghidra was read-only.

### Why PRCP03 never reaches phase 4

`prcp_07_tarawa.lua` advances on three escort objectives. Each is a `luaGetDistance` from a ship to a
NavPoint, and each ship is ordered there with `NavigatorMoveToRange(ship, FindEntity(point))`:

| phase | ship, from | NavPoint | done within | check |
| --- | --- | --- | --- | --- |
| 1 | Aylwin (-7400, 7800), with CV26/CV27 in formation (`:542..544`) | CarrierPoint (-2500, 3001) | 1600 m | `CheckPrimObjective1`, `:781` |
| 2 | Portland (-7600, -7600), generated in `PreparePhase2` (`:907`) | SupportPoint (-500, -2501), then SupportPoint2 (1250, -2501) | 1800 m | `CheckPrimObjective2`, `:1100` |
| 3 | Schroeder (8801, -2000), generated in `PreparePhase3` (`:1166`) | DDPoint (2000, 1501) | 1200 m | `CheckPrimObjective3`, `:1280` |

Phase 3's `FadeInAgain3` leads through `VictoryMovie3` and `luaIngameMovieEnd8` to `PreparePhase4`
(`:1359`), which generates the seven Hidden troop transports.

**The host gap.**
- A `moveto` at a NavPoint carries a descriptor with an object id and no position. On PRCP03's 9000
  run, Aylwin's is `mode=1 object id=50000`, which is CarrierPoint.
- In the image, `00521EA0` resolves the uint16 id through the handle tables `00F89A54` /
  `00F89AA8`, which hold every entity. `009E2FB0` latches that object, and `009DBCC0` puts the goal
  at the object's matrix times the zero offset: the NavPoint's position.
- This host's `00521EA0` answers units only. So nothing is latched and the goal is the zero triple,
  the world origin. Aylwin's `d32c` runs from 10559 m to 6408.59 m over 9000 frames toward (0, 0),
  not toward CarrierPoint (`local\lo_smoke_prcp03.log`).
- **Phases 1 and 2 would still complete by accident.** The line to the origin passes about 252 m
  from CarrierPoint and about 1415 m from SupportPoint.
- **Phase 3 never completes.** Schroeder's line passes about 1908 m from DDPoint, and it stops at
  the origin, about 2500 m away, against a 1200 m test. So PRCP03 stalls in phase 3 on this host.
  It is a host gap, not the image's behaviour.

**A budget question on top.** At Aylwin's measured 9.22 m/s (4151 m in 450 s):
- phase 1 needs about 5260 m, so about 570 s, about 11,400 frames;
- phase 2 needs about 7000 m for Portland's battleship formation;
- phase 3 needs about 6300 m for Schroeder, with each phase's movies and blackouts in between.

**`PreparePhase4` would need roughly 40,000 to 45,000 mission frames under lockstep 0.05, image or
host.** PRCP03 at 9200/9000 cannot show the transports either way.

### The binding

`kShipAiMarkerTargetBound`, in `include/bsp/game_hosts_ship_ai.hpp`, is committed OFF.
- The script-orders host's `register_scene_marker` now also hands each marker's authored world
  position to the units host (`register_scene_marker_position` / `scene_marker_position`).
- In the ship AI's `0071EB60` binding, a descriptor with no position whose object id resolves to no
  unit but names a scene marker supplies that marker's position as the goal.
- A new summary line counts the resolves: `summary mission ship ai marker goals`.

**Labelled substitutions:**
- The marker is handed over as a position, not as a latched object, so `raw_target_0b20` stays 0.
  The image holds the NavPoint there, and `009F1491` filters it out as not `IsKindOf(2)`.
- The weapon director's arrival test (`00836A6C`) and every other `00521EA0` consumer still do not
  resolve markers.

### Predictions, before any run

**PRCP03 9200/9000, gameplay moves (exit 3):**
- Aylwin's goal becomes CarrierPoint. Its heading changes by about 2 degrees (2.346 rad instead of
  2.383).
- It ends about 2700 m from CarrierPoint, so phase 1 is **not** completed.
- There is no `PreparePhase2`, no generated battleship and no transport.
- The marker-goal resolves are nonzero.
- Deaths and hit records may shift slightly with the carrier formation's track, around the OFF
  side's numbers.

**USN01 3200/3000, gameplay moves (exit 3).** Convoy1 (`Cruise`, then `moveto` ConvoyGoTo at
(0, -12000)) turns south toward the marker instead of east toward the origin. Its heading goes from
about 1.556 to about 2.83 rad.

**USN02 9200/9000 and USN04 4700/4500: identical (exit 0 or 1).** Neither log has a `moveto` at a
marker id.

### The two labels, read (follow-up plan)

**`00836A6C` is not what stops a ship at the point.** It is the weapon director's generic arrival
arm (docs/COMMAND_EXECUTION.md). It runs only when:
- more than one command is queued;
- the stage is below 1;
- the head is not category 1 or 2;
- the last queued command is category 1 or 2.

It then raises stage 2 within 2000 units, the double `4000000.0` at `00D09FE8`.

PRCP03's orders are single `NavigatorMoveToRange` commands, so the arm never runs for them in the
image either. The stop at the point is the ship AI's `movetopos` state, which measures against
the goal vector. With the goal at the marker, the host now measures against the same point the
image does. **No follow-up is needed for arrival.** A queued `moveto` behind another command would
need the arm to resolve the marker too. That is small: `resolve_command_target_00521ea0` plus
`command_target_facts` for a marker id. It stays out of this packet because no measured mission
queues one.

**`raw_target_0b20` does matter in one place.** In the image it holds the NavPoint (the
`00521EA0` object).
- Every consumer that asks a kind question drops a NavPoint:
  - `009F1491` filters `vtable[5Ch](2)`, so `+0B24h` is 0;
  - `009E5E00` asks `vtable[5Ch](1Ch)`;
  - `009E1A8C` asks `1Ch`;
  - `009F2F3C` asks `5`.
- The visibility gate at `009F1513` reads it only when the filtered target is nonzero.
- **But `009F2AC9` tests the raw pointer alone** (`TEST EDI,EDI`, `JE 009F2DF7`). With a NavPoint
  the image takes the **with-target** arm of the query block (section 10 of
  docs/SENTITY_INIT_PASSES.md):
  - `+12B8h = [director+221h] && 008637D0(NavPoint)`;
  - the torpedo half runs with `EBX = 0`, because a NavPoint fails `vtable[5Ch](5)`, so the gate
    stays set and `+12B4h` stays 0.
- The host, with `raw_target_0b20 = 0`, takes the no-target arm and copies the director's four
  bytes as they are.

So on a `moveto` to a marker, the image's `+12B8h` can differ: it is cleared when `008637D0`
refuses the NavPoint. The follow-up is to carry the marker id as `raw_target_0b20`, with a marker
answering every kind test false and `008637D0` answered for it. It is small, but it touches
`009F2AC9`'s host (the gate-bytes binding in `src/game_hosts_ship_ai.cpp`) and `008637D0`'s
target model. It is planned to go into the flip commit only if the pairs show the `+12B8h` gun gate
matters on a marker `moveto`. Otherwise it gets its own packet.

### The plane side has the same gap (read, packet `cc9_prcp03_phase_progress`)

**The image resolves a marker for a plane order too.**
- `PilotMoveToRange` (`008A4590`) takes its target the same way the ship orders do: `00888AA0`
  for the entity, and the command target through the handle tables `00521EA0` reads.
- The kind-7 moveto task's approach refresh `009BEBA0` reads the object at `approach+44h`
  (`009BEBBC`). When it is set, it refreshes that object's pose (`00414DB0`) and copies its world
  `+FCh`/`+100h` into the steer point `+48h`/`+4Ch` (`009BEBD3`..`009BEBE2`).
- There is no kind test, so a NavPoint's authored position is the steer point.

**This host drops the marker.**
- `GameScriptOrdersHost::run_pilot_move_to_range` turns the target into a unit index. A marker id
  (50000 and up) is past `units_.count()`, so `target_token` is 0.
- `store_unit_moveto_target` then leaves `moveto_target_plus_one` at 0.
- `moveto_refresh_009beba0` never writes the steer point, which keeps its initial `{0, 0, 0}`. The
  plane flies to the world origin, exactly like the ship goal did.

**Plane orders at a marker in the six missions**, from a read-only grep of this installation's
scripts, with the targets classed from the scenes:

| mission | order | target | reached on this host |
| --- | --- | --- | --- |
| JM06 | `PilotMoveToRange(Catalina2, FindEntity("IJNSubSpawn"))`, `jm06.lua:1423`, in `luaJM6CatalinaSpawned` | NavPoint, id 50042 at (1615, -662) | no: the Catalina spawn is not reached in `rb6_jm06`'s 3000 frames |
| JM06 | `PilotMoveOnPath(Catalina, FindEntity("CatalinaPatrolPath"))`, `:118` | Path (a marker, id 50032) | the native `008A3E70` has no host (UNIMPLEMENTED) |
| JM08 | `PilotMoveTo(MovPlane, FindEntity("MoviePoint"))`, `prcpjm08.lua:573` | NavPoint, id 50093 at (2000, 750, 7500) | the native `008A4150` has no host (UNIMPLEMENTED, 4 calls in `lo_base_jm08`) |
| USN13 | `PilotSetTarget(unit, FindEntity("CB4"/"CB2"))`, `usn_13_truk.lua:1667/1671` | CommandBuildings, created units, not markers | not a marker order |
| USN01, USN02, USN04 | every plane order targets a unit | - | - |

**Binding plan.** It comes after the marker-target pairs, as its own switch.
- In `run_pilot_move_to_range`, an unresolved target whose object id names a scene marker stores
  the marker's authored position as the task's steer point, through a new units-host setter.
- `moveto_refresh_009beba0` keeps that point while `moveto_target_plus_one` is 0.
- `PilotMoveTo` and `PilotMoveOnPath` are unbound natives and are their own packets. They should
  take the same marker rule when they are bound.

**Predictions for that binding:**
- Identity on USN01, USN02, USN04, USN13 and JM08, which have no bound plane order at a marker.
- Identity on JM06 at 3200/3000, where the Catalina spawn is not reached.
- A mission reaching `luaJM6CatalinaSpawned` would show the Catalina flying to IJNSubSpawn instead
  of the origin. JM06's frame budget for that callback was not measured.

### `PilotMoveTo`, `008A4150` (packet `cc9_pilot_move_to`, read and plan)

**The native** (`__fastcall(lua_State*)`, the `luaMW_...` failure-string prologue as its siblings):
- argument 0 goes through `00888AA0` (`008A424F`), the entity;
- argument 1 goes through `0088A810` (`008A4284`), the command target;
- then `0077D600(entity, 00E08F68, &target, 1)` (`008A42A0`..`008A42A7`), the same issue with the
  same moveto command class that `PilotMoveToRange` makes at `008A4723`..`008A472A`.

**What differs from `PilotMoveToRange` (`008A4590`):**
- `PilotMoveToRange` reads a third argument into the descriptor's `+14h` (`008A46E5`..`008A4708`,
  `bsp::pilot_move_to_range_008a46dc`). `PilotMoveTo` never does, so `+14h` keeps the 0 that
  `0088A8C7` stored.
- `PilotMoveToRange` refreshes the unit's pose up to three times after the issue
  (`008A472F`..`008A475A`). `PilotMoveTo` has no such tail.

So `PilotMoveTo(u, t)` issues exactly what `PilotMoveToRange(u, t)` with two arguments issues. The
target resolves through `0088A810`, and `00521EA0`'s handle tables hold a NavPoint like any other
entity. The kind-7 task's approach `009BEBA0` then steers to that object's world position (the
plane-side read above).

**The binding, under one switch `kPilotMoveToBound` in `include/bsp/game_hosts_script_orders.hpp`,
committed OFF:**
- a `PilotMoveTo` row (`0x008a4150`) on the script-orders host, served by the `PilotMoveToRange`
  body with the range forced to the two-argument 0;
- the plane-marker rule: an unresolved target whose object id names a scene marker stores the
  marker's authored position as the task's steer point (`moveto_point`), through a new units-host
  setter. `moveto_refresh_009beba0` keeps it, because `moveto_target_plus_one` stays 0. The rule
  applies to both natives under the same switch.

**Held.** The steer-point setter lives in `src/game_hosts_units.cpp`, which cc9-gunnery3 holds
(`cc9_plane_world_rate`, until 08:46Z). The code waits for that file.

**Predictions, before any code:**
- **JM08 3200/3000:** the four `PilotMoveTo` calls (`rb6_jm08`: UNIMPLEMENTED, calls=4) are served.
  - `PilotMoveTo(Movie Mavis, MoviePoint)` (`prcpjm08.lua:573`) installs a moveto task whose steer
    point is MoviePoint's (2000, 7500) under the marker rule, not the origin.
  - The other three are presumably `luaF4FWaveSpawned`'s `PilotMoveTo(unit, Mission.Flagship)`
    (`:722`), unit targets. The native table gives only the count, so this is to be confirmed by the
    ON log.
  - Gameplay moves (exit 3): the spawned wave flies at the flagship, and Movie Mavis flies to
    MoviePoint.
- **USN01 3200/3000, USN02 9200/9000 and USN04 4700/4500: identical (exit 0 or 1).**
  - None calls `PilotMoveTo`.
  - None has a `PilotMoveToRange` at a marker: USN04's two `moviefisher` orders target Zuikaku and
    Shoho, which are units.

**The native alone, committed OFF** (the lead's order: the marker rule follows when
`src/game_hosts_units.cpp` is free).
- `kPilotMoveToBound` adds the `PilotMoveTo` row, `0x008a4150`.
- It runs the `PilotMoveToRange` body with `+14h` forced to 0.
- The log line names the native.

**Predictions for this commit, before any run:**
- **JM08 3200/3000:** the four calls are served (UNIMPLEMENTED to concrete, 4 calls).
  - Without the marker rule, Movie Mavis's order at MoviePoint resolves to `target_token 0`. Its
    task's steer point stays `{0, 0, 0}`, so it flies toward the world origin, not (2000, 7500).
    That is the plane-side gap, now reached.
  - The other three calls install moveto tasks at a unit.
  - Gameplay moves (exit 3): Movie Mavis's track and the wave's.
- **USN01, USN02 and USN04: identical (exit 0 or 1).** None calls `PilotMoveTo`.

**The plane-marker steer point, under the same switch.**
- `GameUnitsHost::store_unit_moveto_point` writes the task's steer point x/z.
- When `run_pilot_move_to_range` finds a target that is no unit but a scene marker, it stores the
  marker's authored position there after the task is installed. It does this for both natives.
- `moveto_refresh_009beba0` keeps the point, because `moveto_target_plus_one` is 0. The height
  still comes from the cruise profile.
- A new summary line counts the resolves: `summary mission script pilot move to`.
- **Labelled:** the image re-reads the NavPoint's pose on every refresh. Here the position is
  stored once, which is the same for a NavPoint because it does not move.

**Prediction, replacing the native-alone one for JM08:**
- `marker_goals=1`.
- Movie Mavis's task steers to MoviePoint (2000, 7500) at its cruise height, not the origin.
- The other three calls install moveto tasks at a unit.
- Exit 3.

USN01, USN02 and USN04 stay identical: none calls `PilotMoveTo`, and none issues a
`PilotMoveToRange` at a marker.

### `PilotMoveOnPath`, `008A3E70` (packet `cc9_pilot_move_on_path`)

**The native.**
- Argument 0 goes through `00888AA0` (`008A3F7A`), the entity.
- Argument 2, when three or more arguments are given, goes into `EBP`. The default is 1
  (`008A3F3C`), the follow mode.
- Argument 3, when four are given, goes into `ESI`. The default is `EBP+4`, which is 5
  (`008A3F97`), the start/parameter.
- The path, argument 1, is read last through `0088A810` (`008A4031`). It is resolved through the
  handle tables `00F89A54` / `00F89AA8` (`008A4055`..`008A4086`), the same tables `00521EA0`
  reads.
- It then builds session message **5Bh `MT_GAMEUNIT_MOVEONPATH`** (`0075B430(5Bh)` at `008A4092`,
  message vtable `00D02E84`) with:
  - the path's id word `+174h` at `+20h`;
  - the follow mode at `+24h`;
  - the parameter at `+28h`.
- It routes the message on the entity through `0077C2A0` with flags 0 (`008A40E1`).

**What differs from `NavigatorMoveOnPath` (`008A3600`, the ship body this host binds).** The
argument order, the defaults (1, 5), the path resolution and the 5Bh message are the same. The
differences:
- `NavigatorMoveOnPath` reads a fifth argument, the speed, and after the message stores the class
  (or given) speed through `00890E6F`. `PilotMoveOnPath` has neither.
- `PilotMoveOnPath` is the same order without the speed half.

**Where 5Bh lands on a plane.**
- The plane's handler `007CCFA0` asks `msg->vtable[0Ch](62h)` first. The moveonpath message class
  answers only 5Bh, 59h, 49h and 46h (`0075AFF0`), so that test fails.
- Its kind switch starts at 7Ah, so 5Bh falls to `0095ABE0`, whose table sends 5Bh to its
  default.
- The delivery that acts is the session router's category-59h hop, `00780120` at `00780636`, to
  the weapon director's `00721A40`, as in docs/CRUISE_COMMAND.md.
- Its 5Bh arm (`00721AA3`..`00721ADB`) queues the `moveonpath` command class `00E08F80` with a
  descriptor built from the path entity, then stores the mode pair through `0071C1B0`.
- For a plane, the kind-specific follower is the plane bot's task for that class (`0099A170`'s
  arms). This host does not install one.

**The binding.** `kPilotMoveOnPathBound` in `include/bsp/game_hosts_script_orders.hpp` is
committed OFF.
- A `PilotMoveOnPath` row (`0x008a3e70`) runs the host's `lua_binding_navigator_move_on_path` with
  the `00890E6F` speed store suppressed.
- The host then routes the 5Bh order exactly as for a ship: the queued `moveonpath` command, the
  `0071C1B0` pair, and `0071F600`'s path build from the scene path registry.
- A new summary line counts the calls: `summary mission script pilot move on path`.
- **Labelled:** the plane bot's path task (`0099A170`'s `00E08F80` arm and its follower) is not
  installed. The plane's director holds the path command, but its flight is still driven by
  whatever task it already had.

**Callers on the measured missions** (read-only grep):

| mission | call | reached |
| --- | --- | --- |
| JM06 | `PilotMoveOnPath(Mission.Catalina, FindEntity("CatalinaPatrolPath"))`, `jm06.lua:118`, in stage init | yes, 1 call in `rb6_jm06` |
| JM06 | `PilotMoveOnPath(ent, Mission.ReconPath, PATH_FM_SIMPLE, PATH_SM_JOIN)`, `:2392`, checkpoint restore | no |
| BSM01 | `PilotMoveOnPath(P-40 Warhawk 01 / 02, PatrolPath1 / 2, PATH_FM_CIRCLE)`, `bsm_01_stationed_at_pearl.lua:1877/1880` | yes, 2 calls in `rb6_bsm01` |
| USN01, USN02, USN04, USN13, JM08, LOMP06 | none | - |

The Catalina's call is at stage init, so the image flies CatalinaPatrolPath from the first frame.
It is not a budget question. Flight along the path needs the plane-bot path task.

### Predictions, before any run

- **JM06 3200/3000:** 1 call served, UNIMPLEMENTED to concrete.
  - The Catalina's director takes a `moveonpath` command for CatalinaPatrolPath: 12 points, follow
    mode 1, parameter 5. `path_orders` rises by 1.
  - Without the plane-bot path task, the Catalina's flight does not change, so gameplay is
    identical (exit 1).
  - If the queued command displaces an authored command the plane's controller reads, the
    Catalina's row moves (exit 3). That would be the finding.
- **BSM01 3200/3000:** 2 calls served (the P-40 pair, follow mode `PATH_FM_CIRCLE`), with the same
  expectation (exit 1).
- **USN01, USN02 and USN04: identical (exit 0 or 1).** None calls the native.
