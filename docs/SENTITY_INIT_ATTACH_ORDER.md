# InitAll order: the `thisTable` steps of passes B and C (packet `cc9_init_attach_order`)

Addresses: 009292B0, 00955420, 007F1FE0, 007C9770, 009295B0, 00928100, 009238A0.

Worker cc9-init-passes, 2026-09-27, base main 0942b3cd9. Ghidra was read only. This follows
`docs/SENTITY_INIT_PASSES.md` section 2 (the order difference) and `docs/CONSTRUCT_WORLD.md`
section 17 (the InitAll binding). `docs/SENTITY_INIT_PASSES.md` is leased to cc9-scene-entities
right now, so this is a separate file. The switch is `kSEntityInitThisTableStepsBound` in
`include/bsp/game_hosts_fixed_step.hpp`, committed OFF.

## 1. The three steps, read

### 1.1 `BSP_Unit_BindLuaClass` 009292B0 (V)

- **What it writes.** ECX is the entity, then two stack arguments: the class id and a name
  (`RET 8`). Through the entity's own slot (00927B40) it sets three fields of
  `thisTable[entity]`:
  - `ClassID`, an integer (00B67460);
  - `Name`, the string argument (00B66790, key 00CE8ED0 `"Name"`);
  - `Class` = `VehicleClass[ClassID]` (00B67980, 00B67800, 00B67720, then 00B675D0).
- **Its two callers** (Ghidra xrefs):
  - 007F218E in the squadron's pass B 007F1FE0. The class id is the first plane's descriptor
    class: 007F2177 `MOV ESI,[EBP+3D0h]`, 007F2181 `MOV ECX,[ESI+538h]`, 007F2187
    `MOV EDX,[ECX+70h]`. The name is the squadron's vt+10h, at 007F217F `CALL EAX`.
  - 00955498 in 00955420. That is the scene-bindings routine every unit pass B calls: ships and
    carriers from 00822C20 at 00822CDB, planes from 007D5D20 at 007D5DAC, airfields from
    006D3C10 at 006D3C2D. The class id is the entity's own descriptor `+70h` and the name is its
    vt+10h (`src/native_unit_scene_initialization.cpp`).
- **The squadron's gate.** 007F208D reads the holder kind at `[+C0h]+4h`. Kind 1 jumps to
  007F2101, and that arm reaches 007F218E. Kind 2 takes 007F20A0 and returns without the bind.
  Every single-player squadron holds kind 1. The scene holder is 00922E20. An air-ops launch's
  holder is also 00922E20: 006C5050 builds it at 006C530B and stores it at 006C5324.
- **Correction.** `docs/SENTITY_INIT_PASSES.md` row 00822CDB marks 00955420 as "host, at
  creation". That holds for the scene handle part. Its 009292B0 call has no host: nothing in the
  game hosts writes `ClassID` or `Name`. The load-time attach and pass A's attach write `Class`
  alone, as a stand-in for 009292B0's third field.

### 1.2 The plane's `SquadronID`, 007C9770 (V)

- 007C97BD tests `plane+9D4h`. The squadron's pass A 007F4580 stores itself there for each plane
  it constructs: 007F4B49 `MOV [EBX+9D4h],ESI`, where ESI is the squadron, written once at
  007F45A5 `MOV ESI,ECX`.
- 007C97C6..007C9805 then store `thisTable[plane].SquadronID` through 00927B40 and 00B67460. The
  key is 00D05B80 `"SquadronID"`. The value is the u16 `squadron+174h`, the squadron's entity id
  (007C97EE `MOVZX ECX,word ptr [ECX+174h]`).

### 1.3 The default pass C 009295B0 and the think-script name (V)

- **Which classes use it.** The dword 009295B0 sits at slot `+A4h` of ten vtables. Each was found
  by scanning for its store:
  - 00CE6290, `BSP_Path_Construct` 0047B660;
  - 00CEA090, `BSP_Landscape_Construct` 004F11C0;
  - 00D192E0, `BSP_GameEntity_Construct` 00928630, the base;
  - 00D11138, `CreateScript`'s script entity (00898876);
  - 00CE8390, 00CE8550, 00CE86D8, 00CE8860, 00CE8BD0 and 00CE8D68, built in 004E5850..004E7F90.
    These are unread.
  No class this process pushes on the pending list uses it: not ships, carriers, squadrons or
  planes.
- **The think-script arm needs a kind-3 holder.** 009295D2..009295E6 test `[+C0h]` for non-null
  and its `+4h` for 3; anything else jumps to 009297E2. The kind-3 arm starts with 009238A0,
  named here `SEntity_GetSavedEntityLuaData`, which returns
  `globals._savedata._entities[[+C0h]+8h]`. From that it reads:
  - `_gameEntity.thinkFunction`, handed to 0088A330 at 00929687;
  - `timing`, into `+1E0h` and the byte `+1DCh`;
  - nine keyed values.
- **Kind 3 is not built on a fresh mission.** The holder vtable 00D03D94 is stored at five
  sites: 00922DE9 (the copy constructor 00922DE0, which copies the source's kind), 00922E29
  (kind 1), 00922E59 (the destructor), 00779556 (kind 2) and 0077ECDB (kind 2). None stores
  kind 3. Every routine whose kind-3 arm calls 009238A0 reads `_savedata`.
- **Conclusion.** The think-script name step cannot run on a fresh single-player mission start.
  The holder that carries it belongs to a restored save, and the constructor that makes kind 3
  was not found. **Uncertainty:** it could store the kind through a register, which this byte
  scan would miss.
- **The arm every fresh entity takes** is 00928100, named here
  `SEntity_MirrorIdentityToThisTable`. When the entity has a `thisTable` key (`+178h`), it
  writes four fields of `thisTable[key]`:
  - `Race` = `+58h`;
  - `Party` = `+54h`;
  - `Name` = vt+10h, when `+154h` is set;
  - `Type` = the string table `00E0CD80[+C4h]`.
  It applies to Path, Landscape and script entities, which this process attaches at load (the
  mission frame's `attach_scene_entities_00928a00`) or not at all. It is not in the InitAll walk
  this packet binds. Section 4 lists it as a gap.

## 2. The binding (`kSEntityInitThisTableStepsBound`)

- **Pass A.** Under the switch, pass A's attach seeds only `ID`, `Dead` and `Ptr`, the three
  fields 00928A00 seeds. Its `Class` stand-in is skipped.
- **Pass B.** For every node, pass B runs `bind_lua_class_009292b0`, which writes `ClassID`,
  `Name` and `Class` into the slot pass A made. The native table row is
  `SEntity::InitAll pass B bind_lua_class` (009292b0). The pass B record itself stays, because
  the rest of each class's pass B body is still unbound.
- **Pass C.** For a wing plane, pass C runs `set_plane_squadron_id_007c97e3`. The row is
  `SEntity::InitAll pass C plane squadron_id` (007c97e3). The pass C record stays.
- **Order.** Both steps now run after every pass A of the walk. In the image, pass A covers the
  whole list and appends each squadron's wing, and only then does pass B start.
- **Substitutions, labelled in the code:**
  - **The squadron's `ClassID`** is the node's VehicleClass row. The image reads the first
    plane's descriptor `+70h`. The host creates a squadron's planes from its one type, so the
    two rows are the same.
  - **The squadron and its leader plane share one node and one slot here,** keyed by the leader
    unit's id. The image has a squadron entity and a separate leader plane. So the leader's
    `SquadronID` is not written: writing it would put the field on the table that stands for the
    squadron, whose image table has none. Each wing plane's `SquadronID` is the leader's id,
    because that is the key the squadron's slot has in this process.
  - **The name** is the host's unit row name, not the entity's vt+10h.
- **OFF** is today's behaviour: pass A writes `Class`, and nothing writes `ClassID`, `Name` or
  `SquadronID`.
- **Summary line:** `summary SEntity::InitAll thisTable steps bound=<0|1> class_bound=N
  squadron_ids=N think_names=0`.

**create_units needs no change for these three steps.** None of them runs in `create_units`
today. The early B and C work `create_units` does (`docs/SENTITY_INIT_PASSES.md` section 2)
reads no `thisTable` slot. So no contract is written for cc9-plane-release.

## 3. Predictions (written before the pairs; the same tree, switch only, `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`)

**Script readers.** No reader in this installation's `usn_19_coralus.lua` or `usn_2_java.lua`
touches `ClassID`, `Name` or `SquadronID` of an entity table. The global helpers read an
entity's `ClassID` only in `luaSetUnlockName`, which neither mission calls. They read `.Name`
in helpers neither script calls directly: the `luaClearCheck*` family, cheat narratives, and a
log line under `RELEASE_LOGOFF` in `luaObj_AddUnit`. Their indirect use was not traced.

- **USN04 4700/4500:**
  - `class_bound` 0 -> 60: 20 squadrons (16 SpawnNew, 4 air-ops) and 40 wing planes.
  - `squadron_ids` 0 -> 40, and `think_names` 0 on both sides.
  - Two new native rows, concrete: `pass B bind_lua_class` with 60 calls and
    `pass C plane squadron_id` with 40.
  - The pass B and pass C records stay at 60 each.
  - Unchanged: `self_table_entities=86` and `wing_member_tables=40`.
  - **No think script runs that did not.** The script-call and think-pass summary lines are
    identical.
  - **Gameplay identical:** deaths, the per-entity death table, hit records, and dive-bomb and
    torpedo releases. The band is zero, because `Class` holds the same row at the first Lua read
    on both sides.
- **E2 = USN04 9200/9000.** The same counts as the USN04 pair, and gameplay identical.
- **USN02 9200/9000:**
  - `class_bound` 0 -> 4 (the four GenerateObject destroyers), `squadron_ids` 0.
  - One new row with 4 calls. The SquadronID row does not appear, because no wing plane is
    pushed.
  - Gameplay identical.
- **If any gameplay row moves,** a helper reached one of the new fields. The switch stays OFF,
  and the reader is traced before any flip.

## 4. Gaps and contracts

- **Load-time units have no `ClassID` or `Name`.** Scene units are attached by the mission
  frame's `attach_scene_entities_00928a00` in `src/game_hosts_lua.cpp`, outside this walk. In
  the image, the scene read's four InitAll calls (0046EB4B, 0046EB88, 0046EBC6, 0046ED0F) run
  pass B over them, so they carry `ClassID` and `Name` from 00955420 -> 009292B0 as well.
- **Path, Landscape and script entities** lack 00928100's `Name` and `Type`. Their `Race` and
  `Party` are written by the load attach's 00928F50 mirror.
- **Next packet.** Both gaps belong to the load-time attach, which this packet did not change.
  They need a class test per scene entity: 009292B0 for unit classes, 00928100 for the
  default-pass-C classes.
- **The kind-3 arm of 009295B0** (the saved-game restore) is outside single-player new-mission
  runs.

## 5. Coverage

| routine | coverage |
| --- | --- |
| 009292B0 | complete (existing reconstruction `src/native_unit_class_lua.cpp`) |
| 007F1FE0 | 007F1FE0..007F21A3: the kind gate and the 009292B0 call's arguments |
| 007C9770 | complete |
| 009295B0 | the kind gate, the kind-3 arm's reads (not their stores beyond `+1DCh`/`+1E0h`), and the else call |
| 00928100 | complete |
| 009238A0 | complete |
| 00955420 | the 009292B0 call's arguments, from the existing reconstruction |

No body without a Ghidra function was found. 009295B0, 00928100, 009238A0 and 007C9770 are
Ghidra functions (`ghidra proto --brief`).

## 6. Pairs and verdict

**The runs.**
- **Binaries.** Both were built from one tree at 19c9c6998 and differ only by the switch:
  `local\bin\ao_off` (committed OFF) and `local\bin\ao_on` (flipped locally, then reverted).
- **Settings.** `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1` were set for every run,
  launched through `tools/run_game.ps1`.
- **Log checks.** All six logs have the fit line, the immediate present interval and the final
  COM release. Each module directory is under `local\bin\ao_*` in this tree.

| row | USN04 OFF | USN04 ON | E2 OFF | E2 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| native table rows | 1,591 | 1,593 | 1,591 | 1,593 | 1,498 | 1,499 |
| `pass B bind_lua_class` 009292b0 | - | concrete 60 | - | concrete 60 | - | concrete 4 |
| `pass C plane squadron_id` 007c97e3 | - | concrete 40 | - | concrete 40 | - | - |
| `class_bound` / `squadron_ids` / `think_names` | 0 / 0 / 0 | 60 / 40 / 0 | 0 / 0 / 0 | 60 / 40 / 0 | 0 / 0 / 0 | 4 / 0 / 0 |
| InitAll calls / with work / entities | 4,512 / 12 / 60 | identical | 9,012 / 12 / 60 | identical | 9,008 / 4 / 4 | identical |
| deaths, hit records | 43, 788 | identical | 52, 875 | identical | 20, 329 | identical |
| torpedo / dive-bomb task releases | 4 of 16 / 1 of 19 | identical | 4 of 16 / 1 of 19 | identical | - | - |
| death rows, plane death modes, unit table | 43, 43, 81 | identical | 52, 52, 81 | identical | 20, 0, 32 | identical |

**What `tools/pair_diff.py` reports.** It exits 1 on all three pairs: every gameplay and
per-entity row is identical. The clock offset is +0.00 s. In each pair, the only other changes
are the added native rows above and the one summary line. The masked multiset of other lines
shows 0 lines only OFF and 0 only ON. So every script-call and think-pass summary line is
identical, and no think script runs that did not run before. The USN04 gameplay section:

```
GAMEPLAY: identical
  deaths 43 / 43; hit records 788 / 788; hull hits 331 / 331; damage 11917.1 / 11917.1
  shots 5075 / 5075; first hit 93.00 s / 93.00 s; torpedo-task releases 4 of 16 / 4 of 16
  dive-bomb-task releases 1 of 19 / 1 of 19; torpedo drops 1 / 1; plane water contacts 16 / 16
  controlled moved Lexington-class01 3514.72 / 3514.72; units 81 / 81
DEATH ROWS: identical (43 rows)   PLANE DEATH MODES: identical (43 rows)   UNIT TABLE: identical (81 rows)
```

**Every prediction held.**

**Also seen.** USN02 ends at 39.65 s with `Mission.EndMission` "Game Over" on both sides. This
switch does not cause it: the OFF run shows the same end. The world-init worker's USN02 pair at
70b4afc41 recorded 22 deaths and 439 hits, so the control has moved since that tree.

**Verdict: ON.** `kSEntityInitThisTableStepsBound` is set true.

## 7. Pass E and the property bag (packet `cc9_init_pass_e_property_bag`, `kSEntityInitPassEReleaseBound`)

Worker cc9-init-passes, 2026-09-27, base main 0c86989b7. Ghidra was read only.

### 7.1 What pass E frees (V)

- **The release.** 009262FE..00926319 load the holder at `entity+C0h`. When it is not null,
  they call its slot 0 with 1 (00926315 `PUSH 1`, 00926317 `CALL EAX`) and store 0 at `+C0h`.
- **The holder class.** The scene holder's vtable is 00D03D94. Its slot 0 is 00779570, the scalar deleting destructor, which calls 00922E50.
  For kind 1 the destructor frees the bag at `+8h` through the bag's own slot 0.
- **The bag is a clone.** 00922E20, the kind-1 constructor, stores at `+8h` the result of
  00922E2D `CALL 008F41F0`, the bag clone that `docs/SENTITY_INIT_PASSES.md` also names at
  00955420 (`clone_bag_008f41f0`). So pass E frees the entity's own copy. The authored bag in the
  scene database stays: that is `sceneDb+18h`'s named-object map, which 0046D930 instantiates
  from again on each GenerateObject call.

### 7.2 The host's copies of authored values, and every later reader

The host keeps no bag object on an entity. The scene reader copies authored keys into
`GameSceneEntityRecord` fields (`include/bsp/game_hosts_scene_contents.hpp` lines 52..104).
Records live in three places:

| holder | image counterpart | lifetime |
| --- | --- | --- |
| `GameSceneContentsHost::entities()` | the scene read's own objects | whole run |
| `scene_spawn_pool()` entries | the scene database's named-object map (`sceneDb+18h`) | whole run |
| the creator's record copy (`GenerateObject`'s local `record`, `script_orders` `copy` and `batch`) | the entity's cloned bag at `+C0h` | ends with the creator call |

Every read of an authored field outside the scene reader, with when it runs:

| reader (file:line) | field | when | image has it where by then |
| --- | --- | --- | --- |
| `src/game_hosts_units.cpp:6500..6512` (SceneStartSpeed binding) | `StartSpeed`, `ShipYardLaunch` | inside `create_units`, the creator | pass B 00822C20 reads the bag before pass E |
| `src/game_hosts_units.cpp:6690` | `Command`, `CommandTarget` into the unit row | `create_units` | the scene read queues both strings through 00469610 into the scene database's list at `database+14Ch` (`include/bsp/scene_deferred_refs.hpp`), not the entity's bag; later reads use the unit row |
| `src/game_hosts_script_orders.cpp:565..567` | `WingCount` of a held-back squadron | the SpawnNew or GenerateObject call, before the entity exists | the scene database's bag (still alive); the entity's pass A 007F4747 reads it before pass E, and later readers use the resolved `+3C8h` (`src/game_hosts_ai.cpp:2431`, `owner->wing_count`) |
| `src/game_hosts_lua.cpp:2197` | the held-back carrier's deck | GenerateObject, the creator | 006CADD0, in the carrier's pass B, copies it into the air-ops block |
| `src/game_hosts_lua.cpp:2096..2112` | `spawned` and `entity_id` only | any later GenerateObject | host bookkeeping, not an authored key |
| `src/game_avoid_zone_runtime.cpp:153..184` | Path `Point%02i.Pos` | load (`load_avoid_zone_geometry`, `src/game_hosts_mission_frame.cpp:1781`) | the Path entity's pass B 007B38D0 copies the points (`docs/SENTITY_INIT_PASSES.md`, Path row) |
| `src/game_hosts_mission_frame.cpp:1967` (`collect_scene_markers`) | name, class, frame | load | object headers, not bag keys |
| `src/game_hosts_scene_contents.cpp:2840..2993` | `impl.entities` | inside `run_load_scene_contents_004d4df0` only | load |

- **The result.** No host read takes an authored value from an entity's copy after that entity's
  InitAll. The one copy that would count is the creator's temporary record, and no reader holds
  it past the creator call. The long-lived records are the scene-database side, which the image
  keeps too.
- **No reader breaks.** The drop needs no contract to another owner.
- **The method, and its limit.** The field names of `GameSceneEntityRecord` were grepped across
  `src`. A reader that takes the record's fields under another name, through a copy made outside
  those three holders, would be missed.

### 7.3 The binding

- **ON.** Under `kSEntityInitPassEReleaseBound`, the pass E row
  `SEntity::InitAll pass E release_spawn_holder` (00926317) is logged as implemented and
  counted. The release is exact, because the entity's copy is already gone by then.
- **OFF.** The named record.
- **Summary line:** `summary SEntity::InitAll pass E bound=<0|1> released=N`.
- **The fold-in.** 009295B0's think-script name was settled by packet 1 (section 1.3): it is not
  reachable on a fresh mission. Its 00928100 arm is packet 3.

### 7.4 Predictions (written before the pairs; the same tree, switch only, both variables set)

- **USN04 4700/4500:**
  - The native row `pass E release_spawn_holder` goes from UNIMPLEMENTED 60 to concrete 60.
  - The summary goes from `released=0` to `released=60`. Nothing else moves.
  - Gameplay identical: deaths, hit records, releases, death rows, unit table.
- **USN02 9200/9000:** the row goes from UNIMPLEMENTED 4 to concrete 4, `released` from 0 to 4,
  and gameplay is identical.
- **If anything else moves,** the switch stays OFF.

### 7.5 Pairs and verdict

- **The runs.** The binaries are `local\bin\pe_off` (the committed OFF, 53ee0ad6a) and
  `local\bin\pe_on` (flipped locally, then reverted). `BSP_GUNNERY_RNG_STREAMS=1` and
  `BSP_DEATH_TABLE=1` were set. All four logs have the fit line, the immediate present interval
  and the final COM release, and each module directory is under `local\bin\pe_*` in this tree.
- **What `tools/pair_diff.py` reports.** It exits 1 on both pairs, and the clock offset is
  +0.00 s. The only changes are the pass E row, from UNIMPLEMENTED to concrete with the same call
  count, and the pass E summary line.

| row | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: |
| `pass E release_spawn_holder` 00926317 | UNIMPLEMENTED 60 | concrete 60 | UNIMPLEMENTED 4 | concrete 4 |
| `released` | 0 | 60 | 0 | 4 |
| deaths, hit records | 43, 788 | identical | 20, 329 | identical |
| death rows, plane death modes, unit table | 43, 43, 81 | identical | 20, 0, 32 | identical |
| other lines (masked multiset) | - | 0 / 0 | - | 0 / 0 |

**Every prediction held. Verdict: ON.**
