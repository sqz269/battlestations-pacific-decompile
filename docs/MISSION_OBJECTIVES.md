# Binding the objective producers, and what the sets actually hold

Addresses: `008CD440`, `008CD544`, `008CD57F`, `008CD59A`, `008CD5E1`, `008CD617`, `008CD626`,
`008CD65A`, `008CD669`, `008CD6D3`, `008CD6E2`, `008CD744`, `008CD758`, `008CD7D6`, `008CD832`,
`008CD846`, `008CDAF0`, `008CDB09`, `008CDB20`, `008CDBD6`, `008CDC56`, `008CDD60`, `008CE510`,
`008E1F80`, `008DD5C0`, `008DF9B0`, `008DF5D0`, `008DF2B0`, `008DFC00`, `00B677E0`, `00888AA0`,
`00A2C450`.

Packet `cc8_mission_objectives`, read-only Ghidra analysis. Every descriptive name is a hypothesis,
not a recovered symbol. Host: `src/game_hosts_lua.cpp`, `src/game_hosts_ai.cpp`. Report:
`reports/mission_objectives.json`. Predecessor: `docs/AI_WORLD_SETS.md`, which named
`008CD440 Objectives_Add` as the unimplemented producer of the eight sets `00A2C450` walks.

**The producers are bound and the sets now fill. The world-set test still answers false, and the
reason is authored data, not the host:** on all three missions every `Objectives_Add` call carries
six arguments, which is none of its target arguments, and `Objectives_AddUnit` is never called at
all. Objectives exist; no objective holds a unit.

## 1. `008CD440`'s argument order, settled

`docs/MISSION_RESULT_DECISION.md` left this open: "It reads an integer, an optional integer, three
strings and an optional boolean ... The mapping from Lua positions to constructor arguments is not
settled."

`00B677E0 BSP_LuaObject_ArgumentAt` is `__thiscall(frame, out, index)` and the index is the **first**
of its three pushes (`008CD5D6 PUSH 1` / `LEA EAX,[ESP+24h]` / `PUSH EAX` / `LEA ECX,[ESP+5Ch]` /
`CALL`). Scanning the body for every call to it and recovering that push gives the order directly
(`local/argscan.py`):

| Lua index | site | read | role |
| --- | --- | --- | --- |
| 0 | `008CD544` `IsInteger`, `008CD57F`/`008CD59A` `GetInteger` | optional integer | party |
| 1 | `008CD5E1` `IsInteger`, `008CD617`/`008CD626` `GetInteger` | optional integer | player slot |
| 2 | `008CD65A`/`008CD669` `GetString` | string | **the objective name** |
| 3 | `008CD6D3`/`008CD6E2` `GetString` | string | text |
| 4 | `008CD744`/`008CD758` `GetString` | string | text |
| 5 | `008CD7D6` `IsBoolean`, `008CD832`/`008CD846` `GetBoolean` | optional boolean | a flag |
| 6.. | `008CD89D` onward, no immediate index | targets | units and positions |

That matches `Objectives_AddUnit(party, playerSlot, objectiveName, target, ...)`
(`docs/OBJECTIVE_UNIT_LIST.md`, `kObjectiveNameArgument = 2`,
`kObjectiveFirstTargetArgument = 3`) in its first three positions, with `008CD440` pushing the
target block out to index 6 because it carries two more strings and a boolean first.

The trailing block is walked the way `008CDD60` walks its own: `008CD8C6`/`008CD8DB`
`IsVector3Table` classifies a position, `008CD99E`/`008CD9BA` `IterateFirst` and
`008CDA2C`/`008CDA48` `IterateNext` with `IsUnbound` as the stop flatten a table argument into
several targets, and `008CD942`/`008CDA1D` push each onto a vector.

### The add loop

```
008cdaf0  MOV EAX,1
008cdaf5  SHL EAX,CL              ; the bit for this slot
008cdaf7  MOV ECX,[ESP+1Ch]       ; the slot mask arguments 0 and 1 built
008cdafb  TEST ECX,EAX
008cdafd  JZ  (next slot)
008cdb03  MOV EDX,[00E188A8]
008cdb09  MOV ECX,[EDX+EBP]       ; the set at game+21A4h + slot*4
008cdb20  CALL 008E1F80           ; four arguments: two string pointers and two values
```

`008E1F80 BSP_ObjectiveSet_Add` is `operator new(2Ch)` at `008E1F9A` followed by
`008DD5C0` at `008E1FCA`, the record constructor `docs/MISSION_RESULT_DECISION.md` already models
(name at `+4h`/`+8h`, text at `+0Ch`/`+10h`, kind at `+18h`, state at `+1Ch`, unit list at
`+20h`..`+28h`). The tail then adds the collected targets through
`008CDBD6` `008DF9B0 BSP_ObjectiveSet_AddUnitExpandingGroups` and `008CDC56`
`008DF5D0 BSP_ObjectiveSet_AddPositionToNamedObjective`, resolving each with
`008CDBA0` `GetByName` and `008CDBAF` `ToUserdata`.

`coverage: complete` for the argument decoding, the slot loop and the add tail of `008CD440`.
`contract: unread` for `008DD5C0`'s body beyond the existing model, and for which of arguments 3
and 4 becomes the record's `+0Ch` text.

## 2. Host methods

| Site | In | Callee | Host method | this / args | ret |
| --- | --- | --- | --- | --- | --- |
| `008CD544` etc. | `008CD440` | `00B677E0` | `objective_argument_int` / `_string` / `_unit` | frame; out, index | value |
| `008CDB20` | `008CD440` | `008E1F80` | `GameObjectiveSets::add_objective` | set; name, text, kind, flag | record |
| `008CDBD6` | `008CD440` | `008DF9B0` | `GameObjectiveSets::add_unit` | set; name, unit | void |
| `008CDD60` | (row) | — | the same arm, targets from index 3 | — | 0 |
| `008CE510` | (row) | `008DFC00` | `GameObjectiveSets::remove_unit` | set; name, unit | void |
| `00A2C491` | `00A2C450` | `008DDF90` | `group_has_member_in_world_set` | set; member | bool |

### Substitutions, each labelled

* **The slot mask.** `008CDEF2`'s party loop reads `player+28h` over the eight player records at
  `game+18CCh`, and `008CDF58` gates on `player+8h`/`+9h`. This process has one player record and
  no party field on it, so `objective_slot_mask` returns the explicit slot when argument 1 names
  one and slot 0 otherwise. Every observed call resolves to mask `0x01`, which is the same answer
  the native gives a single-player session.
* **The target resolve.** `00888AA0 BSP_ObjectHandle_FromLuaTable` converts the table's `Ptr`; this
  process has no such object, so `objective_argument_unit` reads the `ID` field `00928A00` seeds
  and takes `unit = ID - 1`, the identity milestone 2l established and the same one
  `GameScriptOrdersHost::entity_from_argument` uses.
* **The record.** `GameObjectiveSets::Objective` keeps the name and the unit list only. The native
  record's text, kind and state are modelled in `include/bsp/mission_result.hpp` and are not
  duplicated here, because `008DDF90`'s membership walk reads neither.
* **Where the table lives.** The native's eight sets are a field of the world singleton at
  `game+21A4h`. The producers are in the Lua host and the reader is in the AI coordinator, so the
  host's table is a process-wide object rather than a member of either.

## 3. What the shipped scripts actually do

Every call, on every mission measured, is `Objectives_Add` with **`argc=6`**: arguments 0 through 5
and **no target block**. `Objectives_AddUnit` is called **zero** times on all three.

| Mission | `Objectives_Add` calls | objective names created | slot mask | units |
| --- | --- | --- | --- | --- |
| IJN01 | 3 | `Strafe`, `Bomba`, `Bruh` | `0x01` | 0 |
| USN01 | 2 | `CA`, `DD` | `0x01` | 0 |
| USN02 | 4 | `Sink`, `CL`, `Nav`, `Bruh` | `0x01` | 0 |

So `00A2C450`'s membership walk finds nothing because **no objective holds a unit**, not because
the sets are missing. That is a fact about the authored mission scripts, and it is the answer to
the question `docs/AI_WORLD_SETS.md` left open.

`Bruh` is not a Battlestations Pacific objective name. It appears on two of the three missions, and
this installation is modded (`docs/GAME_EXECUTABLE.md` and the memory note on the BSPRM/AlterBSP
artefacts), so the objective tables these runs read are **this installation's**, not retail's. A
retail check would need a clean install and was not done.

## 4. Every other reader of the sets, which the walk-ended arm was hiding

`008DDF90 BSP_SzurkeNyil_ContainsUnit` has fifteen call sites
(`tools/callsite_census.py 008ddf90`). Thirteen of them build `this` as
`[game + 21A4h + [00E188A8 + 18ECh]*4]`, the **local player's** slot, and only `00A2C450` indexes
by the brain's own slot:

| Caller | What the answer gates |
| --- | --- |
| `004C3E90` in `BSP_Game_BuildLocalPlayerUnitLists` | which of the eight local-player lists a unit joins (`docs/LOCAL_PLAYER_UNIT_LISTS.md`) |
| `004F25A6`, `004F25C6` `BSP_Unit_IsLocalPlayerObjective` | two identical `__thiscall bool(unit)` thunks with no Ghidra function |
| `006399E6` `BSP_InGameHudMarkers_ResolveMarkerColourIndex` | the marker colour |
| `00643183` `BSP_InGameHudMarkers_AddUnitMarker` | whether a unit gets a HUD marker |
| `00922D54` `BSP_Entity_IsSurfaceTarget` | an attack-capability input (`docs/ATTACK_CAPABILITY_INPUTS.md`) |
| `00A0F8B5` `BSP_AiCommand_CandidateTargetWeight` | the AI's target weight |
| `00A2C491` `BSP_AiGroup_HasMemberInWorldSet` | the planner split |
| `00A2DF43` in `FUN_00A2DEF0` | unread |
| `005226F5`, `005227E5`, `00526E86`, `00527059`, `0059D973`, `005B2F6C` | unread |

With the sets empty of units, **all fifteen take their negative arm**. Binding the producers does
not change that here, for the reason in section 3, but it is worth recording that the AI's own
target weight (`00A0F810`) and the HUD's marker colour hang off the same bit.

## 5. Validation

"Before" is the world-sets commit `9c1da1b2c` (`local/ws_<m>_after.log`); "after" adds
this packet (`local/obj_<m>_after.log`). Same three missions, 3000 mission frames.

| Mission | | objectives created | objective units | world-set queries | hits | `served` | `attackmove` | commands | hits (gunnery) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| IJN01 | before | 0 | 0 | 1 | 0 | 2450 | 2250 | 2515 | 17 |
| IJN01 | after | 3 | 0 | 1 | 0 | 2450 | 2250 | 2515 | 17 |
| USN01 | before | 0 | 0 | 1 | 0 | 0 | 0 | 475 | 10 |
| USN01 | after | 2 | 0 | 1 | 0 | 0 | 0 | 475 | 10 |
| USN02 | before | 0 | 0 | 1 | 0 | 616 | 559 | 635 | 168 |
| USN02 | after | 4 | 0 | 1 | 0 | 616 | 559 | 635 | 168 |

What the bindings created, per mission, every call at slot mask `0x01`:

| Mission | objectives | units |
| --- | --- | --- |
| IJN01 | `Strafe`, `Bomba`, `Bruh` | 0 |
| USN01 | `CA`, `DD` | 0 |
| USN02 | `Sink`, `CL`, `Nav`, `Bruh` | 0 |

### Attribution

The sets went from empty to holding three, two and four objective records. **Every AI
number is unchanged**, including `served`, `attackmove`, the world-set query count and
its hit count, and so is the gunnery census. That is the correct result and not a null
one: `00A2C450` walks objective **units**, every `Objectives_Add` call on these missions
carries `argc=6` and therefore no target block, and `Objectives_AddUnit` is never called,
so the membership walk still finds nothing. Binding the producer was necessary to learn
that, and it could not have been learned from the listing alone.


## 6. Corrections

* `docs/AI_WORLD_SETS.md` section 2 says the sets are empty "because their Lua producer is not
  bound". The producer is now bound; the sets hold objective **records** on all three missions and
  still hold no **units**, because every `Objectives_Add` call carries no target block and
  `Objectives_AddUnit` is never called. Appended there as a correction.
* `docs/MISSION_RESULT_DECISION.md`'s uncertainty "`008cd440`'s Lua argument order ... is not
  settled" is **settled** by section 1.

## 7. Follow-up packets

* Which of arguments 3 and 4 becomes the record's `+0Ch` text, and what argument 5's boolean sets.
* `008DF9B0 BSP_ObjectiveSet_AddUnitExpandingGroups`: its group expansion is unread, and it is the
  path a target that names a formation would take.
* The six unread `008DDF90` callers in section 4.
* `008BD340 Objectives_Completed` and `008BD900 Objectives_Failed`, still unimplemented: with
  records now in the sets they have something to act on.
* A retail-install check of the three missions' objective tables, since this installation is modded.

## 8. `Objectives_Completed` and `Objectives_Failed` (packet `cc9_objectives_completed`, `kObjectiveStatusBound`)

Worker cc9-hud3, 2026-09-27, base d9d55c3f1. Ghidra was read only.

**What the natives do** (V; `docs/MISSION_RESULT_DECISION.md`, `docs/OBJECTIVE_UNIT_LIST.md`):
- **The call.** 008BD340 `Objectives_Completed` and 008BD900 `Objectives_Failed` take
  `(party, slot, name, text, quiet)`. `luaObj_Completed` in `global/commandhelpers.lua` 5915-5920
  passes `obj.Party`, `obj.PlayerIndex`, `obj.ID`, `obj.TextCompleted` and `quiet`. For each slot
  the mask selects (the same party/slot rule as `Objectives_Add`), the native runs 008E20D0
  (completed) or 008E2200 (failed) on that slot's set.
- **The first objective whose name matches:**
  - is announced by 008E1D30 (sound, unless quiet);
  - goes through 008DFE50, which hands every live unit of a non-hidden objective to the removal
    008DFC00;
  - takes `+1Ch` = 1 or 2.
- **`+1Ch` is the only field written.**

**What reads the mark:**
- 008DF2B0's hidden-objective marker gate (`+18h == 2 && +1Ch != 1`, at a later unit add);
- 008DFE50's marker walk;
- the HUD's objective pages.

**Nothing native ends the mission or scores from it.** The mission end is script-driven:
`Mission.EndMission`, `Scoring_SetMissionCompleted` 008B8AD0 and the end-movie request
(`docs/MISSION_RESULT_DECISION.md`). `luaObj_Completed` also sets the script's own
`obj.Success = true` (5923), which `luaObj_GetSuccess` reads, whatever the native did.

**Callers in this installation's scripts** (read-only grep of `luaObj_Completed(` call sites):
usn_1_marshall 10, usn_2_java 7, usn_19_coralus (USN04) 12, usn_13_truk (USN13) 9. BSM01
(`bsm/bsm_01_stationed_at_pearl.lua`), JM06 and JM08 (`jm06.lua`, `jm08.lua`) exist and were not
counted. On the measured idle runs the natives are reached:

| mission | calls on the OFF logs (d9d55c3f1 tree) | objectives added |
| --- | --- | --- |
| USN01 3200/3000 | `Objectives_Completed` 2 | CA, DD |
| USN04 4700/4500 | none | Bombers |
| USN02 9200/9000 | `Objectives_Failed` 2 (the Houston loss path at 39.65 s) | Sink, CL |

**The binding.**
- **Switch:** `kObjectiveStatusBound` in `include/bsp/game_hosts_lua.hpp`, committed OFF.
- The two rows join the objective rows of `src/game_hosts_lua.cpp` and call
  `GameObjectiveSets::set_status` per selected slot (state and the unit drop).
- **The census:** `summary mission objective status ... sets= misses= unit_drops=`, plus the
  existing `objective binding` lines.
- **SUBSTITUTIONS, labelled:**
  - The kind (`+18h`) is not recorded by this host, so 008DFE50's early return for a hidden
    objective is not applied. No objective on these missions holds a unit.
  - The announcement sound and the HUD marker refresh are not modelled.

**Predictions** (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player):
- **USN01:** `sets=2 misses=0 unit_drops=0`, if the two keys are CA and DD. A key that names no
  added objective counts as a miss. Mission end stays `none`. **Gameplay identical, exit 1.**
- **USN02:** `sets` + `misses` = 2, `unit_drops=0`. The mission still fails at 39.65 s ("Game
  Over", Houston). **Identity, exit 1.**
- **USN04:** census all 0. The mission still ends without an end (phase 1). **Identity, exit 1.**
- **No mission completes or scores differently:** nothing reads `+1Ch` toward the result.

**Pairs and verdict.**
- **The pairs.** The OFF side is this tree's build at f637c824d (`local\ob2_off_*`). The ON side is
  `local\ob2_on`, an export of the same commit with only the switch flipped.
- **Run parameters:** streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle player.

| pair | `pair_diff` | result |
| --- | --- | --- |
| USN01 3200/3000 | exit 1 | `Objectives_Completed` "DD" then "CA", `sets=2 misses=0 unit_drops=0`. Mission end `none`. Gameplay, 7 death rows and 28 unit rows identical |
| USN02 9200/9000 | exit 1 | `Objectives_Failed` "Sink" and "CL", `sets=2`. Still fails at 39.65 s ("Game Over", Houston). Gameplay, 19 death rows and 28 unit rows identical |
| USN04 4700/4500 | **exit 0** | no call; everything identical |

**Every prediction held. No mission completes or scores differently.** **Verdict:
`kObjectiveStatusBound = true`.**

## 9. The objective kind `+18h` (packet `cc9_objective_kind`, `kObjectiveKindBound`)

Worker cc9-lua2, 2026-09-27. This is item 7 of the cc9-hud3 handoff. Ghidra was read only.

### 9.1 Where the kind comes from (V)

- **The record constructor.** `008DD5C0` stores its third stack argument at `+18h`
  (`008DD678`/`008DD684`). Its frame is 20h deep: three SEH pushes and five registers. So
  `[ESP+2Ch]` there is argument 3, and it returns through `RET 10h`.
- **Add.** `008E1F80 BSP_ObjectiveSet_Add` hands its own argument 3 to that slot. Its four pushes
  at `008E1FB4`..`008E1FC7` are read with the 48h-deep frame.
- **Objectives_Add.** `008CD440` pushes `[ESP+70h]` as Add's argument 3 (`008CDB10`/`008CDB15`).
  The one write to that slot is `008CD7A7`, the result of `008DBF40` on the string that
  `008CD744`/`008CD758` read, Lua argument 4 (section 1).
- **`008DBF40`** returns the case-insensitive (`__stricmp`) index of that string in the pointer
  table `00E0C948`:

| index | string |
| --- | --- |
| 0 | `primary` |
| 1 | `secondary` |
| 2 | `hidden` |
| 3, 4, 5 | `marker1`, `marker2`, `marker3` |
| 6 | no match: the loop ends after the sixth entry and returns 6 |

- The shared helper passes the objective's level there:
  `Objectives_Add(obj.Party, nil, obj.ID, obj.Text, level[, true])`
  (`global/commandhelpers.lua` 5763-5769).

### 9.2 What reads it

- **`008DFE50`** returns before its unit walk when `+18h` is 2 (`008DFE6E`). A status change on a
  hidden objective therefore keeps its units.
- **`008DF2B0`'s marker gate** (section 8). The host models no marker, so this reader stays a
  record.

### 9.3 The binding

- **Switch:** `kObjectiveKindBound` in `include/bsp/game_hosts_ai.hpp`, committed OFF.
- While true:
  - `Objectives_Add` records the kind: `objective_kind_008dbf40` over Lua argument 4, stored on
    the newly made `GameObjectiveSets::Objective::kind`;
  - `set_status` keeps a hidden objective's units and counts `hidden_status_holds`.
- **The census:** `summary mission objective kind bound=.. hidden_objectives=..
  hidden_status_holds=..`.
- **SUBSTITUTION, labelled.** An objective the host finds already listed keeps its first kind. The
  image's Add constructs a new record on every call.

### 9.4 Predictions (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player)

The measured adds are:
- USN01: `CA`, `DD`;
- USN02: `Sink`, `CL`;
- USN04: `Bombers`.

None is a hidden objective. In this installation the hidden IDs are `Bruh` (USN01 and USN02) and
`Trans` (USN04), and none of them is added on the idle runs. No measured objective holds a unit.

| row | prediction |
| --- | --- |
| USN01 3200/3000 | `hidden_objectives=0 hidden_status_holds=0`; identity, exit 1 |
| USN02 9200/9000 | the same; identity, exit 1 |
| USN04 4700/4500 | the same; identity, exit 1 |

The switch changes nothing measured. It is bound so that a run which adds a hidden objective with
units (USN02 adds `Mission.HiddenTrgs` at `usn_2_java.lua` 777 on a later stage) keeps those units
at completion, as the image does.
