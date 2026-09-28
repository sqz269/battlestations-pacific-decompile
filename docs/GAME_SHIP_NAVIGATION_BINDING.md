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
