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
