# InitAll order: the `thisTable` steps of passes B and C (packet `cc9_init_attach_order`)

Addresses: 009292B0, 00955420, 007F1FE0, 007C9770, 009295B0, 00928100, 009238A0, 00926317, 00925CE0, 00928630.

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

## 8. The load-time identity gaps (packet `cc9_init_identity_gaps`, `kSceneLoadThisTableIdentityBound`)

Worker cc9-init-passes, 2026-09-27, base main 1aed3c039. Ghidra was read only.

### 8.1 What the image writes at load (V)

- **Where the passes run.** The scene read's InitAll calls (0046EB4B, 0046EB88, 0046EBC6 and
  0046ED0F) run passes A, B and C over the load-time instances. This process attaches those
  instances in the mission frame, through `attach_scene_entities_00928a00`, outside the walk.
- **Units.** Each unit class's pass B reaches 009292B0 (section 1.1), which writes `ClassID`,
  `Name` and `Class`. The host wrote only the `Class` stand-in.
- **Markers of the seven default-pass-C classes** take 009295B0's 00928100 arm (section 1.3).
  It writes four fields into `thisTable[key]`:
  - `Race` = `+58h` and `Party` = `+54h`, both numbers (006B8260). The base 00925CE0 stores
    `+54h` = 2 (00925E1D) and `+58h` = -1 (00925E24). Pass A's 00927050 then copies an authored
    `Party` or `Race` from the kind-1 bag over them (`docs/ENTITY_LIFECYCLE_TAILS.md` section 2).
  - `Name` = vt+10h. It is written only when `+154h` is not 0. For Path (vtable 00CE6290), vt+10h
    is 0042E950, which returns the string pointer at `+158h`, so `+154h`/`+158h` is the name's
    length and data, and a named entity gets `Name`.
  - `Type` = `00E0CD80[+C4h]` (006B8360). The base 00928630 stores `+C4h` = 1 (009286C1). Each
    class constructor then stores its scene class id:

| class | id | constructor store | `Type` string |
| --- | --- | --- | --- |
| NavPoint | 41h | 004E59AD | `NAVPOINT` (00D18E68) |
| MovieCamPos | 42h | 004E5A0D | `MOVIECAMPOS` (00D18E5C) |
| MovieCamLookat | 43h | 004E5A6D | `MOVIECAMLOOKAT` (00D18E4C) |
| Path | 47h | 0047B6C8 | `GAMEPATH` (00D18E1C) |
| CameraPath | 4Ah | 004E58B8 | `CAMERAPATH` (00D18DFC) |
| SimpleEffect | 5Bh | 004E7F31 | `SIMPLEEFF` (00D18CD8) |
| PeriodicEffect | 5Ch | 004E800E | `PERIODEFF` (00D18CCC) |

  The other identity rows (LandingPoint 1Dh, SpawnPoint 4Dh, PlaneSquadronGen, LandConvoy) have
  their own pass C, so they get no 00928100.

### 8.2 The binding

The switch is `kSceneLoadThisTableIdentityBound` in `include/bsp/game_hosts_fixed_step.hpp`,
committed OFF.

- **The mission frame** (`src/game_hosts_mission_frame.cpp`) hands each marker's scene class id
  and its record's authored `Party` to the Lua host.
- **After the attach loop** (pass A over every instance), the Lua host does two things:
  - **Pass B:** for every unit, it calls `bind_lua_class_009292b0` with the row's class and name.
    The native row is `SceneLoad::pass B bind_lua_class` (009292b0).
  - **Pass C:** for every marker of the seven classes, it calls `mirror_identity_00928100`. This
    writes `Party` (the authored value, else 2), `Name` (when not empty) and `Type`. The native
    row is `SceneLoad::pass C mirror_identity` (00928100).
- **The attach** then writes no `Class` stand-in for units.
- **`Race` is not written.** The scene record does not carry an authored `Race`, so the value is
  unknown here. See the contract below.
- **Summary line:** `summary SceneLoad thisTable identity bound=<0|1> class_bound=N mirrored=N`.

### 8.3 Script readers in this installation

`local\cc9-init-passes-luaread.py` takes every helper function transitively called from the
mission script and lists each read of `.Name`, `.Type`, `.Race`, `.ClassID` or `.SquadronID`.
Its call graph is an over-approximation: it also follows `usage:` lines inside comments. Every
hit was then checked for a real call.

- **`luaRemoveByName`**, `commandhelpers.lua:1797`, compares `value.Name == name`. Its only
  callers are the `luaClearCheck*` family (5088..5397). Nothing in the global scripts,
  `usn_19_coralus.lua` or `usn_2_java.lua` calls that family; the hits were the `usage:` lines.
  **Not reached.** (Had it been reached, `nil == nil` on unnamed tables would remove the first
  entry of the table, so `Name` would matter there.)
- **`luaObj_AddUnit`**, `commandhelpers.lua:5830`, is called at `usn_2_java.lua:859` and `:871`.
  It reads `target.Name` only to guard a log line under `RELEASE_LOGOFF`, and the targets there
  are positions (`GetPosition(point)`, `FillPathPoints`), not entity slots. **No effect.**
- **`luaMessageHandler`**, 3526..4498, reads `unit.Name` only in its cheat and debug branches.
  It is called from `usn_19_coralus.lua:476, 1346, 2278` and `usn_2_java.lua:460`. **The
  branches are not reached without a cheat message.**
- **Not called by either mission:** `luaSurrender` (6203, 6213), `luaGenerateObjects` (6795),
  `luaStartConvoy` (12393..12403) and `luaWriteCamState` (12971, reached only from a cheat branch
  at 4399).
- **The mission scripts themselves** read none of these fields of an entity table. Their `.Name`
  reads are `this.Name` (the mission's) and `Class.Name`.
- **Prediction: identity.**

### 8.4 Contracts

- **cc9-ships, `src/game_hosts_scene_contents.cpp`, `GameSceneEntityRecord`:** carry the
  authored `Race` beside `Party`, as 00927050's kind-1 arm reads it into `+58h`. With it, the
  marker mirror writes `Race` (authored, else -1), and 00928F50's `Race` for units can be written
  too. It is not written today.
- **The script-orders owner (`src/game_hosts_script_orders.cpp`, now in cc9-ships's lease):**
  CreateScript's script entity is class 3 (`+C4h` = 3, stored after the vtable writes at 00898874..00898888 per the 00898750 ledger entry). The image pushes it on the
  pending list through 00928630, so the next InitAll's pass C runs 00928100 on its slot. That
  writes `Race` -1, `Party` 2, `Type` `SCRIPTENTITY` (00D19034), and `Name` if it is named;
  whether it is named is **contract: unread**. The host's script-entity slot
  (`push_self_table_slot_008989f6`, 2286) writes none of them.
- **Landscape (00CEA090)** uses 009295B0 too, but it has no identity row, so no `thisTable` key
  is known for it. 00928100 writes nothing without `+178h`. Whether a Landscape has a key is
  **unread**.

### 8.5 Predictions (written before the pairs; the same tree, switch only, both variables set)

- **USN04 4700/4500:**
  - `class_bound` 0 -> 21 (the 21 load-time units) and `mirrored` 0 -> 5 (four Paths and the
    NavPoint `IJNRetreat`).
  - Two new native rows: `SceneLoad::pass B bind_lua_class` with 21 calls, and
    `SceneLoad::pass C mirror_identity` with 5.
  - The `thisTable: 26 per-entity slot(s) ... 21 of them with ... Class` line is unchanged.
  - The runtime-walk rows of sections 6 and 7 are unchanged.
  - Gameplay identical, with the death rows and the unit table.
- **USN02 9200/9000:**
  - `class_bound` 0 -> 28 and `mirrored` 0 -> 2 (the NavPoints `DRGoTo` and `EscapePoint`).
  - The same two rows, with 28 and 2 calls.
  - Gameplay identical, and the Game Over at 39.65 s on both sides.
- **If a gameplay row moves,** a script reached one of these fields. The switch stays OFF, and
  the reader is traced.

### 8.6 Pairs and verdict

- **The runs.** The binaries are `local\bin\ig_off` (the committed OFF, 4c3864299) and
  `local\bin\ig_on` (flipped locally, then reverted). `BSP_GUNNERY_RNG_STREAMS=1` and
  `BSP_DEATH_TABLE=1` were set. All four logs have the fit line, the immediate present interval
  and the final COM release, and each module directory is under `local\bin\ig_*` in this tree.
- **What `tools/pair_diff.py` reports.** It exits 1 on both pairs, and the clock offset is
  +0.00 s. The only changes are the two added rows and the summary line. The masked multiset of
  other lines shows 0 lines only OFF and 0 only ON.

| row | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: |
| `SceneLoad::pass B bind_lua_class` 009292b0 | - | concrete 21 | - | concrete 28 |
| `SceneLoad::pass C mirror_identity` 00928100 | - | concrete 5 | - | concrete 2 |
| `class_bound` / `mirrored` | 0 / 0 | 21 / 5 | 0 / 0 | 28 / 2 |
| deaths, hit records | 43, 788 | identical | 20, 329 | identical |
| death rows, plane death modes, unit table | 43, 43, 81 | identical | 20, 0, 32 | identical |

**Every prediction held. Verdict: ON.**

## 9. One node per entity on the pending list (packet `cc9_pending_list_dedup`, `kPendingListDedupBound`)

Worker cc9-init-passes, 2026-09-27, base main af680b4c0. There were no Ghidra reads beyond
section 17's: 00926BE0 is a locked `push_back` and does not deduplicate. The image never needs
to, because only the base constructor pushes, once, at 00928760.

### 9.1 Why the host needs it

The host will have two pushers for one construction:
- the Lua routes (section 17 of `docs/CONSTRUCT_WORLD.md`), which push after `create_units`
  returns;
- `create_units` itself, once cc9-units2 lands its push (`docs/RELEASE_ISSUE_STAGE.md` part 2).

This causes two problems:
- **Doubled nodes.** Without a rule, every route instance would be on the list twice.
- **Wrong wing order.** Wing planes would be pushed in the `create_units` batch, before their
  squadron's pass A. The image pushes them from 007F4580's constructions, at the tail, during
  that pass A.

### 9.2 The rules (`src/game_hosts_lua.cpp`)

- **A plain push** (`push_pending_entity_00926be0`) is skipped in two cases:
  - its id is pending (`skipped_pending`);
  - its id was already attached by a pass A or by the load attach (`skipped_attached`).
- **A squadron push** (`push_pending_squadron_00926be0`):
  - first drops the pending plain nodes of its own wing, the ids in
    `[units_before + 1, units_end]` other than its own (`wing_deferred`), so that its pass A
    appends them at the tail. It never does this during a walk (`init_active_00f899a5_`).
  - then upgrades a pending plain node of its own id in place (`squadron_upgrades`), or pushes a
    new node.
- **Pass A's wing append** skips an id that is already pending (`wing_append_skipped`).
- **The load attach** (`attach_scene_entities_00928a00`) records its ids as attached and drops
  any pending node of those ids (`load_dropped`). The load-time instances stay outside the list,
  as today.
- **Summary line:** `summary SEntity::InitAll pending dedup bound=<0|1> skipped_pending=N
  skipped_attached=N squadron_upgrades=N wing_deferred=N wing_append_skipped=N
  load_dropped=N`.

**Consequence.** Once `create_units` pushes, the list keeps the image's order without moving
construction: a squadron node, then its wing appended by its pass A. The route pushes can then
be retired, which is the third step, after cc9-units2's push lands.

### 9.3 Contract: wing construction in pass A (cc9-units2 and the script-orders owner)

- **Where it lives.** The wing units are built by the creator batch:
  - `src/game_hosts_script_orders.cpp` near 400..403 for SpawnNew and near 571 for GenerateObject
    (the `wing_record` copies);
  - `create_units` in `src/game_hosts_units.cpp`.
  Both run before InitAll. In the image, 007F4580 (the squadron's pass A) constructs each plane,
  which pushes it, after every node already pending.
- **The contract.** Hold the squadron's wing records on the squadron (its `PendingEntity` or its
  registry record), and construct them from the squadron's pass A through a units-host call.
  The Lua host would call it at `entity_attach_lua_self_vcall_9c`, before its wing append.
  `create_units` then pushes each plane as it constructs it.
- **What moves.** Construction time only: unit ids and anything a constructor does. List order is
  already the image's under 9.2. Until this lands, the pass A append remains the stand-in for
  007F4580's constructions.

### 9.4 Predictions (written before the pairs; the same tree, switch only, both variables set)

With the Lua routes as the only pusher, no rule fires.
- **USN04 4700/4500:**
  - Every dedup counter is 0.
  - Unchanged: `pushes=20`, `wing_appended=40`, `entities=60`, `self_table_entities=86` and
    `wing_member_tables=40`.
  - The InitAll rows (calls 4,512, with work 12) and every native row are unchanged. The only
    moved line is the summary's `bound 0 -> 1`.
  - Gameplay identical.
- **USN02 9200/9000:** the same, with `pushes=4`, `entities=4`, `self_table_entities=34` and
  InitAll 9,008 / 4. Gameplay identical.

### 9.5 Pairs and verdict

- **The runs.** The binaries are `local\bin\dd_off` (the committed OFF, cbaf29bc9) and
  `local\bin\dd_on` (flipped locally, then reverted). `BSP_GUNNERY_RNG_STREAMS=1` and
  `BSP_DEATH_TABLE=1` were set. All four logs have the fit line, the immediate present interval
  and the final COM release, and each module directory is under `local\bin\dd_*` in this tree.
- **What `tools/pair_diff.py` reports.** It exits 1 on both pairs, and the clock offset is
  +0.00 s. The native table is identical in both. The only moved line is the dedup summary's
  `bound 0 -> 1`. The masked multiset of other lines shows 0 lines only OFF and 0 only ON.

| row | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: |
| InitAll calls / with work / entities | 4,512 / 12 / 60 | identical | 9,008 / 4 / 4 | identical |
| pushes / wing appended | 20 / 40 | identical | 4 / 0 | identical |
| `self_table_entities` | 86 | 86 | 34 | 34 |
| every dedup counter | 0 | 0 | 0 | 0 |
| deaths, hit records | 43, 788 | identical | 20, 329 | identical |
| death rows, plane death modes, unit table | 43, 43, 81 | identical | 20, 0, 32 | identical |

**Every prediction held. Verdict: ON.**

**The rules are not exercised yet:** no counter fired, because the Lua routes are still the only
pusher. The pair that exercises them is cc9-units2's `create_units` push. With that push in
place, the counters should read as follows, on the same missions:
- **USN04:** `create_units` pushes the 20 squadrons' leader units and their 40 wing planes as
  plain nodes. The routes' 20 squadron pushes then drop the 40 wing nodes (`wing_deferred=40`)
  and upgrade the 20 leader nodes (`squadron_upgrades=20`). Pass A appends the 40 again
  (`wing_appended=40`, `wing_append_skipped=0`). So `entities=60` and `self_table_entities=86`
  are unchanged. The load-time scene units, pushed by `create_units` at load, are dropped by the
  load attach (`load_dropped` = the scene unit count, 21). Gameplay identical.
- **USN02:** the 4 GenerateObject ships are pushed by `create_units`, and the route's plain
  push is skipped (`skipped_pending=4`). `load_dropped=28`, and gameplay identical.
- **If a count differs,** the push order in `create_units` differs from the one assumed here,
  which is squadron before its wing, both before the route's push.

## 10. The load-time InitAll through the walk (packet `cc9_load_time_init_all`, `kLoadTimeInitAllBound`)

Worker cc9-init-passes, 2026-09-27, base main 338c8b4e3. Ghidra was read only.

### 10.1 Where the scene read calls InitAll (V)

- **The loop.** `BSP_SceneFile_Read` 0046DF00..0046EF62 is a token loop, whose head is at
  0046E9F5. An `entity` block is read by `BSP_SceneFile_ReadEntityBlock` 0046CF40 (0046EB0F);
  its creators construct the instances, and each construction pushes through 00928760.
- **The four sites.** The first block that is not an entity block, or the end of the file, runs
  InitAll:

| site | before | guard |
| --- | --- | --- |
| 0046EB4B | the traffic block 009514B0 | `TEST BL,BL` (the instantiate pass), `CMP [ESP+13h],BL` |
| 0046EB88 | the groups block 00467E10 | the same |
| 0046EBC6 | the browser-groups block 00469E40 | the same |
| 0046ED0F | the end of the file, before the holder release at 0046ED1E and 0046AAB0 `ResolveDeferredReferences` | `CMP [ESP+13h],0` |

- **One call per pass.** `[ESP+13h]` is cleared at 0046E9F0 and set after each call (0046EB50,
  0046EB8D). So a scene read runs **one** InitAll, and it runs over every instance the entity
  blocks made, before the traffic, groups and deferred references.
- **The host.** `run_load_scene_contents_004d4df0` reads the records, `create_units` builds the
  units, and the mission frame collects the markers. The load attach
  `attach_scene_entities_00928a00` then stood in for this InitAll with pass A alone. Section 8
  added the pass B and C identity writes.

### 10.2 The binding

- **Under the switch,** the mission frame calls `run_scene_load_init_all_0046eb4b(entities)` in
  place of the load attach, at the same point and with the same list: the units in unit order,
  then the markers. It pushes one node per instance, standing in for each constructor's 00928760
  push. The node carries `load_scene`, `findable`, the marker's class id and its authored `Party`.
  The dedup rules skip an id that `create_units` has already pushed, once that lands. It then
  runs one InitAll, logged at 0046EB4B.
- **Pass A**, for a load node:
  - binds the air-ops deck id (`air_ops_decks().bind_entity_id`), as the load attach did;
  - records the id as load-attached;
  - attaches with `findable`, so a marker that is not findable stays out of the name index;
  - writes `Party`/`Race` when the SceneEntity carries them. No caller does today.
- **Pass B** skips markers, which have no 009292B0. Units get `ClassID`, `Name` and `Class`
  (section 1.1).
- **Pass C** runs 00928100 for markers of the seven default-pass-C classes (section 8). The row is
  `SEntity::InitAll pass C mirror_identity` (00928100).
- **Passes D and E, the start branch and loading progress** run per node, as for runtime
  entities.
- **Retired when ON:** the load attach and its section 8 block. Their rows go: `MissionLua::
  entity_lua_attach`, `SceneLoad::pass B bind_lua_class` and `SceneLoad::pass C
  mirror_identity`, and so do the two `thisTable:` note lines.
- **Summary line:** `summary SEntity::InitAll load walk bound=<0|1> pushes=N mirrored=N`.
- **Not reproduced:**
  - **The call's position.** The host calls it after `create_units`, `issue_authored_commands`
    and the scoring reset, where the load attach was. The image calls it inside the scene read,
    before the traffic and groups blocks and 0046AAB0. Nothing between the two points reads or
    writes `thisTable`. The scene-contents half, an InitAll inside `run_load_scene_contents`, is
    the contract below.
  - **The site.** The call is logged at 0046EB4B whichever block the scene reaches first.

**Contract for cc9-ships, `src/game_hosts_scene_contents.cpp`.** The instantiate pass
constructs, so push each marker instance there (00928760). Then run the one InitAll at the
entity-blocks boundary through the Lua host's runner (`run_sentity_init_all_00925f20`, site
0046EB4B), once the units host's `create_units` push exists and the unit instances are built
inside that pass. Until then, the mission frame's push-and-walk is the stand-in.

### 10.3 Predictions (written before the pairs; the same tree, switch only, both variables set)

- **USN04 4700/4500.** The load list holds 26 entities: 21 units and 5 markers, which are four
  Paths and the NavPoint `IJNRetreat`.
  - Load walk: `pushes` 0 -> 26 and `mirrored` 0 -> 5.
  - InitAll: calls 4,512 -> 4,513, with work 12 -> 13, entities 60 -> 86, pushes 20 -> 46.
    `wing_appended` stays 40.
  - Section 6's `class_bound` goes 60 -> 81 and `squadron_ids` stays 40. Section 8's
    `class_bound` and `mirrored` go 21 -> 0 and 5 -> 0.
  - Native rows, calls OFF -> ON:
    - `SEntity::InitAll` 4,512 -> 4,513;
    - pass A, pass B record, pass C record, start branch, pass D and pass E each 60 -> 86;
    - loading progress 180 -> 258;
    - pass B `bind_lua_class` 60 -> 81;
    - `pass C mirror_identity` added, with 5;
    - `pass C plane squadron_id` stays 40.
  - Removed: `MissionLua::entity_lua_attach`, and the two `SceneLoad::` rows (21 and 5).
  - Unchanged: `self_table_entities=86`, `wing_member_tables=40`, and every dedup counter at 0.
    `load_dropped` is 0 on both sides, because nothing pushes at load today.
  - Other lines:
    - OFF only: the two `thisTable:` load notes.
    - ON only: the `at 0046eb4b: 26 pending` and `INIT,ENUM:26` notes.
    - The log prints only the first 8 walks that find work. The load walk is now the first, so
      the eighth runtime walk's two note lines become OFF only.
  - Gameplay identical: deaths, hit records, releases, death rows, plane death modes and the unit
    table.
- **USN02 9200/9000.** The load list holds 30 entities: 28 units and 2 NavPoints.
  - Load walk: `pushes` 30 and `mirrored` 2.
  - InitAll: calls 9,008 -> 9,009, with work 4 -> 5, entities 4 -> 34, pushes 4 -> 34.
  - Pass B `bind_lua_class` goes 4 -> 32, and the per-pass rows go 4 -> 34.
  - Loading progress goes 12 -> 102.
  - Unchanged: `self_table_entities=34`.
  - Other lines: the same removals, and the notes as above; with only 5 walks, none is pushed out.
  - Gameplay identical.
- **If a gameplay row moves,** the call's later position or a pass the load attach never ran is
  the cause, and the switch stays OFF.

### 10.4 Pairs and verdict

- **The runs.** The binaries are `local\bin\lt_off` (the committed OFF, 86074aa4b) and
  `local\bin\lt_on` (flipped locally, then reverted). `BSP_GUNNERY_RNG_STREAMS=1` and
  `BSP_DEATH_TABLE=1` were set. All four logs have the fit line, the immediate present interval
  and the final COM release, and each module directory is under `local\bin\lt_*` in this tree.
- **What `tools/pair_diff.py` reports.** It exits 1 on both pairs, and the clock offset is
  +0.00 s. Every gameplay and per-entity row is identical, and the other changes are the ones
  predicted in 10.3:

| row | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: |
| InitAll calls / with work / entities / pushes | 4,512 / 12 / 60 / 20 | 4,513 / 13 / 86 / 46 | 9,008 / 4 / 4 / 4 | 9,009 / 5 / 34 / 34 |
| load walk pushes / mirrored | 0 / 0 | 26 / 5 | 0 / 0 | 30 / 2 |
| pass A..E and start-branch rows | 60 each | 86 each | 4 each | 34 each |
| pass B `bind_lua_class` | 60 | 81 | 4 | 32 |
| `pass C mirror_identity` (walk) | - | 5 | - | 2 |
| `SceneLoad::` rows, `MissionLua::entity_lua_attach` | 21, 5, 1 | removed | 28, 2, 1 | removed |
| loading progress | 180 | 258 | 12 | 102 |
| `self_table_entities` | 86 | 86 | 34 | 34 |
| deaths, hit records | 43, 788 | identical | 20, 329 | identical |
| death rows, plane death modes, unit table | 43, 43, 81 | identical | 20, 0, 32 | identical |

- **Other lines.** The two `thisTable:` load notes are OFF only. The `26 pending`/`INIT,ENUM:26`
  notes (30 on USN02) are ON only. On USN04 the eighth runtime walk's two notes (`1 pending`,
  `INIT,ENUM:3`) are OFF only, as predicted.
- **Noise.** The pretranslate row, 18 -> 17 and 18 -> 19, is on the noise list.

**Every prediction held. Verdict: ON.** The load attach and the section 8 block are retired while
the switch is on. Both stay in the code as the OFF path.

## 11. RunExtraFixedStep's other rows (packet `cc9_run_extra_fixed_step`, `kRunExtraFixedStepBound`)

Worker cc9-init-passes, 2026-09-27, base main 9db5c5290. Ghidra was read only.

### 11.1 00874D00 `BSP_Game_RunExtraFixedStep` (V)

- **Callers** (Ghidra xrefs, 19 sites):
  - GenerateObject, 00945311 (`MOV CL,1` at 0094530F);
  - Spawn, 00944D93 (`MOV CL,1` at 00944D8F);
  - LaunchAirBaseSlot, 00896976 and 0089698A;
  - 0046B730, ten sites;
  - 00904C40, five sites.
  Only GenerateObject is reached in the two reference missions: USN02 calls it four times, and
  USN04 not at all.
- **The body.** BL = CL throughout.

| step | site | CL = 1 | CL = 0 |
| --- | --- | --- | --- |
| queued-call drain 00888230 (`[game+1A08h]`) | 00874D0F | yes | yes |
| due think 00929460 with `[00D0DE84]` = 0.05 | 00874D22 | no | yes |
| world gate `[[game+19CCh]+4ACh]` | 00874D2E..00874D41 | - | - |
| session pump 00778450 | 00874D68 | dt 0.0 | dt 0.05 |
| 0077EC20, 00874C90 | 00874D6D, 00874D72 | yes | yes |
| InitAll 00925F20 (CL = 0) | 00874D79 | yes | yes |
| outbound flush 0076FFC0 (dt, `SETZ DL` mode) | 00874DAF | 0.0, mode 0 | 0.05, mode 1 |
| 00926700, 009273A0, then `JMP 00903610` | 00874DB9..00874DD1 | no | yes |

The pump, the creation apply, the registrations flush, InitAll and the outbound flush all sit
under the world gate.

### 11.2 The binding

- **The method.** `GameFixedStepHost::run_extra_fixed_step_00874d00(flag, site)` runs these
  steps through the fixed-step host's own row methods: rows 7, 8, 9, 10, 11, 12 and 13, and the
  tail rows. Each row keeps its current single-player body.
- **The world gate** is the one the last fixed step saw.
- **Wiring.** The Lua host reaches the method through `GameExtraFixedStepRunner`, which the
  mission frame attaches.
- **Under the switch,** GenerateObject calls it with CL = 1 at 00945311, in place of its lone
  InitAll row.
- **Not bound:** Spawn and LaunchAirBaseSlot. They are not host routes that reach the call.

### 11.3 Predictions (written before the pairs; the same tree, switch only, both variables set)

- **USN04 4700/4500.** No GenerateObject, so the logs are identical. `pair_diff` exits 0, or 1
  on the noise list alone.
- **USN02 9200/9000.** Four GenerateObject calls. At each one the world gate is open, because
  the calls come mid-mission after fixed steps have run.
  - New row: `Game::run_extra_fixed_step` 00874d00, concrete with 4 calls.
  - These rows gain 4 calls each: `FixedStepFanout::drain_queued_lua_calls` (and the
    subsystems' own drain row, if it logs one), `pump_session`, the two `Session::` records,
    `apply_pending_entity_creates`, `flush_tick_registrations` and `flush_outbound_session`.
  - `SEntity::InitAll` stays at 9,009 calls: the same call is now made from inside 00874D00.
  - The fixed-step summary's fan-out count rises by 4 × 7 = 28, if it prints one.
  - ON only: four `RunExtraFixedStep 00874d00 from 00945311: CL=1, world gate open` notes.
  - **Gameplay identical.** The queued-call list is empty in this process (its producer 00887560
    runs only off the main thread). Every other row's single-player body is a no-op, and the
    session countdown is held at -1.0, so dt 0.0 does nothing.
- **If the gate is closed** at a call, only the drain row gains that call. That would mean a call
  came before the first fixed step.
- **If any gameplay row moves,** the switch stays OFF.

### 11.4 Pairs and verdict

- **The runs.** The binaries are `local\bin\rx_off` (the committed OFF, 7b6f70b25) and
  `local\bin\rx_on` (flipped locally, then reverted). `BSP_GUNNERY_RNG_STREAMS=1` and
  `BSP_DEATH_TABLE=1` were set. All four logs have the fit line, the immediate present interval
  and the final COM release, and each module directory is under `local\bin\rx_*` in this tree.
- **USN04 4700/4500.** `pair_diff` exits 0: identical apart from noise.
- **USN02 9200/9000.** `pair_diff` exits 1, and every gameplay and per-entity row is identical
  (20 deaths, 329 hit records, 20 death rows, 32 unit rows). The clock offset is +0.00 s.
  - `Game::run_extra_fixed_step` 00874d00 is added, concrete with 4 calls.
  - The drain, `pump_session`, both `Session::` records, `apply_pending_entity_creates`,
    `flush_tick_registrations` and `flush_outbound_session` go from 9,000 to 9,004 calls.
  - InitAll is unchanged.
  - Summary lines:
    - `fixed step body: fanout_sites 144000 -> 144020, concrete 126000 -> 126024`;
    - `fixed step subsystems: lua_calls 9000/0 -> 9004/0`, where the second field, the queued
      calls actually run, stays 0.
  - ON only: four `RunExtraFixedStep ... CL=1, world gate open` notes.
  - The pretranslate row is noise.

**One sub-prediction failed:** the fan-out count. I predicted +28 (4 × 7). It rose by 20
(4 × 5), because only five of the rows count as fan-out sites: InitAll is called directly
through the runner, and the two `Session::` rows are records inside the pump. The rows
themselves moved exactly as predicted.

**Verdict: ON.**

## 12. The deck-tick launch lag (packet `cc9_deck_tick_in_step`, `kDeckTickInFixedStepBound`)

Worker cc9-init-passes, 2026-09-27. Ghidra was read only.

### 12.1 Where the image ticks the decks (V)

- **Callers of 006CDC70** (Ghidra xrefs):
  - 0075828E in 00758270 `BSP_MotherShipUnit_UpdateMotion`, the carrier;
  - 006D254B in 006D2510 `BSP_AirField_TickAdvance`, the airfield;
  - 00896983 in `LaunchAirBaseSlot`.
- **Where motion runs.** Unit motion runs from the step's job waves: 00875CDD, 00875D4D and
  00875DBD (`docs/IN_MISSION_SUBSYSTEM_TICK.md`, the step loop, item 2). The waves come before
  the fan-out 00875E0C..00875EDF, whose row 12 at 00875EA2 runs InitAll. A launch the deck tick
  starts pushes its squadron in the waves, and the same step's row 12 attaches it.
- **The host** ran the tick from `GameScriptOrdersHost::run_script_timers`. The mission frame
  calls that after `run_mission_frame`, so after the frame's fixed step. Such a launch was
  attached at the next step's row 12, one step later. At the harness's lockstep that is one
  frame.

### 12.2 The binding

- **Under the switch,** `FixedStepBinding::run_step_job_waves` (`src/game_hosts_mission_frame.cpp`)
  calls `run_air_ops_update_006cdc70(0.05f)` right after the job waves, once per fixed step.
- **`run_script_timers`** no longer runs it.
- **Not reproduced: the order among units.** The tick runs once for all decks, not inside each
  owner's motion. The ship motion 00825F20 still runs after the fan-out in this host.
- **LaunchAirBaseSlot's own call** (00896983) is not a host route.

### 12.3 How to show the lag, since neither reference mission queues a launch

- **Why the reference missions cannot show it.** USN04's four LaunchSquadron calls all log
  `(started)`: they are started inside the call and attached at 0089E613. USN02 has no deck.
- **What a demonstration needs.** A launch the deck tick starts later, which is a `LaunchSquadron`
  that returns queued (the slot not ready: state 3 or 4, being refilled).
- **What to compare.** In such a run, compare the frame of the tick's `LaunchSquadron`/
  `air ops tick` note with the `SEntity::InitAll 00925f20 at 00875ea2: N pending` note:
  - OFF: the next frame;
  - ON: the same frame.
- **No mission is picked here.** A mission script that launches twice from one slot within a
  refill would do; none was searched for in this packet.

### 12.4 Predictions (written before the pairs; the same tree, switch only, both variables set)

- **USN04 4700/4500:**
  - `AirOps::update` 006cdc70, `update_slots` 006c0da0 and `slot_tick` 006c0510 keep their
    call counts within ±2. The count moves from simulated frames with a positive delta to fixed
    steps, and at lockstep the two agree.
  - No launch is queued, so no InitAll row moves.
  - Gameplay identical: deaths, hit records, releases, death rows, plane death modes and the unit
    table.
  - The `air ops tick 006c0510` refill notes keep their text. The tick now runs before the fan-out
    and before ship motion in the same frame, and it reads nothing either writes.
- **USN02 9200/9000.** There are no decks. The tick walks an empty registry, the counts are within
  ±2, and gameplay is identical.
- **If a gameplay row moves,** the likely cause is `resolve_plane_squadron_members`, which now
  runs before ship motion in the frame instead of after it. The switch stays OFF.

### 12.5 Pairs and verdict

- **The runs.** The binaries are `local\bin\dt_off` (the committed OFF, a909d98dd) and
  `local\bin\dt_on` (flipped locally, then reverted). Their SHA-256 prefixes differ:
  `6ec7c4a94dbe` and `5256240b14b9`. `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1` were
  set. All four logs have the fit line, the immediate present interval and the final COM release,
  and each module directory is under `local\bin\dt_*` in this tree.
- **What `tools/pair_diff.py` reports.** It exits 0 on both pairs: identical apart from noise.
  The native tables are equal row for row. `AirOps::update` is 4,500 calls on both sides of
  USN04, so the ON tick ran once per fixed step from the job waves, where OFF ran it once per
  frame from the timers.
- **The lag itself is not shown** by these pairs. See 12.3 for the run that would show it.

**Every prediction held. Verdict: ON.**

## 13. The session pump's two records (packet `cc9_session_pump_records_read`, docs only, no switch)

Worker cc9-init-passes, 2026-09-27. Ghidra was read only. The records are
`Session::global_object_step_00f8a2fc` (007784F6) and `Session::drain_loopback_queue_0076c600`
(00778542). They are in `GameFixedStepHost::pump_session_00778450`. They stay records, and the
reasons follow.

### 13.1 `[00F8A2FC]->vtable[5Ch](step)` (V)

- **What the object is.** `BSP_Game_OnInit` calls 00992A50 at 004E3F0F whenever `[00F8A2FC]`
  is null. That routine allocates 2218h bytes, constructs them through 00A47EE0 (vtable 00D24878,
  stored at 00A47F1D), and stores the object at `00F8A300` and `00F8A2FC` (00992A9A, 00992A9F).
  The object is the XLive system (`BSP_XLiveSystem_Initialize` 00A40DF0 writes its fields).
  **It exists in single player,** so the pump calls its slot `5Ch` every fixed step.
- **Slot `5Ch` = 00A42A50** (`XLiveSystem_StepCountdown`). It has **no Ghidra function**; its
  body is 00A42A50..00A42A97, with `RET 4` at 00A42A95.
  - While byte `+85h` is set and float `+88h` >= `[00D7A218]` = 0.0, it subtracts the step from
    `+88h`.
  - When the result drops below 0.0, it calls `vtable[58h]`.
- **Who arms it:**
  - `+85h` = 1 comes only from slot `54h`, 00A43650 (`XLiveSystem_ArmCountdown`, 00A436B5).
    That same routine sets `+88h` to `[00D7A260]` = -1.0 (00A436BC), so arming alone does not
    start the countdown.
  - A non-negative `+88h` comes from slot `184h`, 00A47B80: `[00CEB4B0]` at 00A47BD2, unless its
    `vtable[19Ch]` answers true.
  - The constructor stores both fields at 00A47F23 and 00A47F29.
  - The other `+88h` writers in the class are at 00A40E69, 00A4A3B3, 00A4A3EC and 00A4AFF1. They
    are not read here.
  - The byte scan found no other `+85h` writer in 00A3xxxx..00A4xxxx.
- **Expiry, slot `58h` = 00A435E0** (`XLiveSystem_OnCountdownExpired`). It clears `+85h`
  (00A435EC). It then returns when `[00E188A8]+1FE4h` is 0 (00A435F8/00A435FF), which is single
  player.
- **Single-player content.** At most, a timer runs down and clears its own flag. That happens
  only after slot `54h` and slot `184h` have run. Their callers go through the vtable and were not
  found. `BSP_Game_LoadMissionScene` touches the object only at 004E184F (`+48h`). **No gameplay
  state is reached in single player**, so the record is not replaced by a binding. The unread parts
  are the callers of slots `54h` and `184h`.

### 13.2 `0076C600` the loopback drain

- **What it carries.** `docs/SESSION_MESSAGE_DISPATCH.md` read it complete: with `+F4h` = 0,
  the pump calls it every step. It carries every local order the image routes through the
  session, in single player too:
  - `MT_COMMAND` 58h;
  - fire target 5Eh, built by 00835740;
  - director 5Ah;
  - the pass-side 8Fh;
  - the entity-create category 47h.
  Delivery follows that doc's "same-step answer": a message routed before row 9 of the step
  arrives in the same step, one routed after row 9 arrives in the next.
- **What the host does.** The posters deliver each kind directly:
  - the command host issues commands;
  - the gunnery host sets fire targets;
  - the ship-AI host delivers 8Fh at the start of its controller step (`kShipPassSideMessageBound`);
  - nothing posts 47h.
  So **the drain has single-player content, but no queue exists to drain.**
- **Binding it would need** a host-wide loopback queue that every poster enqueues to, drained by
  the pump. The posters live in the units, gunnery, ship-AI and commands files. The observable
  difference is only the step a message arrives in, for posters that run after row 9.
- **Contract, for the lead to route per poster.** For each local order kind, name the image's
  enqueue site and its fixed-step position:
  - before row 9: direct delivery is exact;
  - after row 9: delivery belongs in the next step's pump.
  Only the posts after row 9 need the queue.

### 13.3 Coverage and bodies without a Ghidra function

| routine | coverage |
| --- | --- |
| 00A42A50 | complete, **no Ghidra function**: 00A42A50..00A42A97 inclusive |
| 00A43650 | 00A436A2..00A436C5 (the arming stores) |
| 00A435E0 | 00A435E0..00A4360D (the flag clear and the single-player return) |
| 00992A50 | complete |
| 0076C600 | as `docs/SESSION_MESSAGE_DISPATCH.md` (complete) |

Names added (hypotheses): `XLiveSystem_StepCountdown` 00A42A50, `XLiveSystem_ArmCountdown`
00A43650, `XLiveSystem_OnCountdownExpired` 00A435E0.

## 14. Step 3: the route and load-walk pushes retired (packet `cc9_pending_list_dedup` step 3, `kRoutePushesRetiredBound`)

Worker cc9-init-passes, 2026-09-27, base main 4a1d8e428. This follows cc9-units2's construction
push (`docs/CONSTRUCT_WORLD.md` section 26, ON at 34191e927): `create_units` pushes every
constructed unit, as 00928630 does at 00928760.

### 14.1 What is retired, and what stays

- **GenerateObject's plain push** is a lookup. It pushes nothing when `create_units`'s node is
  pending. So is **the load walk's push of each scene unit**, which then labels that node
  (`load_scene`, `findable` and the rest), as before.
- **The squadron routes keep their call:** GenerateObject of a `PlaneSquadronGen`, SpawnNew,
  LaunchSquadron and the air-ops creation. It is no longer a push. It marks the pending leader
  node as a squadron and records its wing range (the dedup's upgrade), which pass A's wing append
  needs. In the image this knowledge is the squadron object itself, 007F4580's `this`.
- **The markers' load pushes stay.** Their constructor is in the scene-contents host, which does
  not push yet (section 10.2's contract).
- **A fallback.** If a route finds no pending node, it pushes one and counts it
  (`fallback_pushes`). None is expected while `create_units` pushes.
- **Summary line:** `summary SEntity::InitAll route pushes retired bound=<0|1> retired=N
  squadron_annotations=N fallback_pushes=N`.

### 14.2 The two squadron hooks (not in this landing)

cc9-units2's `on_squadron_pass_a_construct_wing` and `on_squadron_pass_c_initial_command`
(`docs/CONSTRUCT_WORLD.md` section 27) were not on main at 4a1d8e428. As the lead directed, the
retirement lands first and the hooks follow in the next landing.

- **The Kingfisher.** USN02's Kingfisher is a load-time unit, and the load walk marks no node as
  a squadron. So the pass C hook needs a squadron test for load nodes, such as the row's scene
  class 18h. That is checked when the hooks land.

### 14.3 Predictions (written before the pairs; the same tree, switch only, both variables set)

- **USN04 4700/4500:**
  - `retired` 0 -> 21 (the load walk's scene units), `squadron_annotations` 0 -> 20 (16 SpawnNew
    and 4 air-ops), `fallback_pushes` 0 on both sides.
  - Dedup: `skipped_pending` 21 -> 0. Unchanged: `squadron_upgrades=20`, `wing_deferred=40`,
    `wing_append_skipped=0`, `load_dropped=0` and `skipped_attached=0`.
  - Unchanged: InitAll calls / with work / entities / pushes 4,513 / 13 / 86 / 86, and
    `wing_appended=40`. Load walk pushes and mirrors stay 26 / 5, and `self_table_entities`
    stays 86.
  - The native table is identical: no row is added and no count moves.
  - Gameplay identical.
- **USN02 9200/9000:**
  - `retired` 0 -> 32 (28 at load and 4 at GenerateObject), `squadron_annotations` 0,
    `fallback_pushes` 0.
  - `skipped_pending` 32 -> 0.
  - InitAll 9,009 / 5 / 34 / 34, load walk 30 / 2 and `self_table_entities=34`, all unchanged.
  - Native table identical, gameplay identical.
- **If `fallback_pushes` is not 0,** a route runs without `create_units`'s push first, and that
  route's order must be read before ON.

### 14.4 Pairs and verdict

- **The runs.** The binaries are `local\bin\rp_off` (the committed OFF, 886959c2e) and
  `local\bin\rp_on` (flipped locally, then reverted). `BSP_GUNNERY_RNG_STREAMS=1` and
  `BSP_DEATH_TABLE=1` were set. All four logs have the fit line, the immediate present interval
  and the final COM release, and each module directory is under `local\bin\rp_*` in this tree.
- **What `tools/pair_diff.py` reports.** It exits 1 on both pairs. The native tables are
  identical, and so are gameplay, death rows, plane death modes and the unit table. The clock
  offset is +0.00 s. The masked multiset of other lines is empty.

| row | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: |
| `retired` / `squadron_annotations` / `fallback_pushes` | 0 / 0 / 0 | 21 / 20 / 0 | 0 / 0 / 0 | 32 / 0 / 0 |
| `skipped_pending` | 21 | 0 | 32 | 0 |
| `squadron_upgrades` / `wing_deferred` | 20 / 40 | 20 / 40 | 0 / 0 | 0 / 0 |
| InitAll calls / with work / entities / pushes | 4,513 / 13 / 86 / 86 | identical | 9,009 / 5 / 34 / 34 | identical |
| deaths, hit records | 43, 788 | identical | 21, 566 | identical |

**Every prediction held. Verdict: ON.** `create_units` is the one push for units. The routes and
the load walk look its node up. The squadron routes annotate it, and the markers still push at
load.

## 15. The squadron pass hooks called (packet `cc9_squadron_pass_hooks_calls`, `kSquadronPassHooksCalled`)

Worker cc9-init-passes, 2026-09-27, base main 13ccb5215. cc9-units2's entries
`GameUnitsHost::on_squadron_pass_a_construct_wing(squadron_index)` and
`on_squadron_pass_c_initial_command(squadron_index)` landed at 1ac30b2b0
(`docs/CONSTRUCT_WORLD.md` section 27). Both are counted no-ops, and the index is the unit index
of the squadron's fused leader, its entity id - 1.

### 15.1 The binding

- **Pass A** (`entity_attach_lua_self_vcall_9c`): for a squadron node, the walk calls the pass A
  entry after the slot attach and before the wing append, at 007F4580's plane constructions. The
  native row is `SEntity::InitAll pass A squadron_construct_wing hook` (007f4580).
- **Pass C** (`entity_init_third_vcall_a4`): for a squadron node, the walk calls the pass C entry,
  007F4BA0's initial command at 007F4E9E. The native row is `SEntity::InitAll pass C
  squadron_initial_command hook` (007f4e9e).
- **Access.** The mission frame hands the Lua host the units host, writable, when it creates it
  (`attach_units_hooks`), and clears it with the orders host.
- **Which nodes are squadrons.** Only the route squadrons (GenerateObject of a PlaneSquadronGen,
  SpawnNew and air-ops creation) are squadron nodes. cc9-units2's read (section 27, b9d3f64da)
  settles USN02: the host builds no squadron there. So no load-node squadron test is added.

### 15.2 Predictions (written before the pairs; the same tree, switch only, both variables set)

- **USN04 4700/4500:**
  - `summary squadron pass hooks` goes from `pass_a=0 pass_c=0` to `pass_a=20 pass_c=20`, the 16
    SpawnNew and 4 air-ops squadrons.
  - The two new native rows have 20 calls each.
  - Nothing else moves, and gameplay is identical.
- **E2 (USN04 9200/9000):** the same 20 and 20. Not run in this packet.
- **USN02 9200/9000:** no squadron node, so both counts stay 0, no row is added, and `pair_diff`
  exits 0.

### 15.3 Pairs and verdict

- **The runs.** The binaries are `local\bin\sh_off` (the committed OFF, 20e3d4387) and
  `local\bin\sh_on` (flipped locally, then reverted). `BSP_GUNNERY_RNG_STREAMS=1` and
  `BSP_DEATH_TABLE=1` were set. All four logs have the fit line, the immediate present interval
  and the final COM release, and each module directory is under `local\bin\sh_*` in this tree.
- **USN04 4700/4500.** `pair_diff` exits 1.
  - The two hook rows are added, 20 calls each.
  - `squadron pass hooks` goes from `pass_a=0 pass_c=0` to `pass_a=20 pass_c=20`.
  - Nothing else moves: 43 deaths and 788 hit records on both sides, and the death rows, plane
    death modes and 81 unit rows are identical.
- **USN02 9200/9000.** `pair_diff` exits 0: identical apart from noise, with no squadron node.
- **The report's pass A call site** is 00926054 (`CALL EAX` after the `+9Ch` load at 0092604E),
  corrected from the OFF commit's 0092604E.

**Every prediction held. Verdict: ON.**

### 15.4 The Lua half of the wing construction (section 9.3), for when units2 builds the wing in the pass A hook

When `on_squadron_pass_a_construct_wing` constructs the wing, `create_units` pushes each plane
while the walk is inside pass A. The walk re-reads the list's size, so it reaches those planes in
this same pass A, as 007F4580's constructions are reached in the image. The Lua host then changes
in five places, all in `src/game_hosts_lua.cpp`, under one switch landed with units2's.

1. **Retire the wing append in pass A.** The loop over `[units_before, units_end)` in
   `entity_attach_lua_self_vcall_9c` goes. The planes arrive as construction pushes instead.
2. **Mark the hook's pushes as this squadron's wing.** The append set three fields on each wing
   node, and a plain construction push has none of them:
   - `wing_member`, which the `wing_member_tables` count and pass C's `SquadronID` need;
   - `squadron_id` = the leader's id (007F4B49's `+9D4h`);
   - `class_index` = the squadron's class.
   So pass A records the list size before the hook call, and marks every node appended during the
   call with them.
3. **Retire the wing-range annotation and the wing deferral.** Once the creator batch builds no
   wing, `route_push_squadron`'s `units_before`/`units_end` has nothing to describe, and the
   dedup's `wing_deferred` rule (section 9.2) finds nothing to drop. Both go.
4. **The squadron flag stays** until `create_units` can mark a construction push as a squadron:
   the route's annotation is what makes pass A call the hook and pass C call the initial command.
   Moving that flag to the units push is the step after.
5. **`wing_append_skipped` and the load-walk path are unaffected.** No load-time squadron exists
   in the reference missions, and a load squadron's wing would be built by the same hook.

**Predictions for that landing:**
- `wing_appended` 40 -> 0;
- the units host's construction pushes are unchanged in total (81 on USN04), with 40 of them now
  made during pass A;
- `wing_deferred` 40 -> 0;
- `entities=86`, `self_table_entities=86` and `wing_member_tables=40` are unchanged;
- `squadron_ids` stays 40;
- gameplay identical, **unless** the wing's construction time moves something. The planes would
  now exist from pass A of the InitAll that follows their squadron's creation, instead of from
  the creator batch. On USN04 both are inside the same Lua call, so identity is expected.

## 16. Handoff (cc9-init-passes retires after this landing)

Worker cc9-init-passes, 2026-09-27, main 92f7cad2f. Owners are from `bsp.py lease list` just
before this commit: cc9-ships holds `docs/GAME_EXECUTABLE.md` (cc9_reference_rebaseline_4), and
cc9-units2 holds `docs/CONSTRUCT_WORLD.md` and `src/game_hosts_units.cpp`
(cc9_squadron_initial_command). Every other file named below is unleased.

### 16.1 Switches this line set ON (all in `include/bsp/game_hosts_fixed_step.hpp`)

| switch | section | what it does |
| --- | --- | --- |
| `kSEntityInitThisTableStepsBound` | 1..6 | pass B 009292B0 (`ClassID`, `Name`, `Class`) and pass C `SquadronID`, after every pass A |
| `kSEntityInitPassEReleaseBound` | 7 | pass E's holder release, exact (the host's per-entity copy is a creator temporary) |
| `kSceneLoadThisTableIdentityBound` | 8 | the load attach's identity writes: units' 009292B0, markers' 00928100 `Party`/`Name`/`Type`. Superseded while section 10's switch is ON |
| `kPendingListDedupBound` | 9 | one pending node per entity id |
| `kLoadTimeInitAllBound` | 10 | the load-time instances go through the pending list and one InitAll walk (0046EB4B) in place of `attach_scene_entities_00928a00` |
| `kRunExtraFixedStepBound` | 11 | GenerateObject runs 00874D00's whole body |
| `kDeckTickInFixedStepBound` | 12 | the air-ops deck tick 006CDC70 runs after the job waves, before row 12 |
| `kRoutePushesRetiredBound` | 14 | `create_units` is the one unit pusher. The routes and the load walk look its node up, the squadron routes annotate it, and markers still push |
| `kSquadronPassHooksCalled` | 15 | pass A and pass C call the units host's squadron entries |

### 16.2 Open, with owners

1. **The wing construction, a joint flip with cc9-units2** (section 15.4). When units2 constructs
   the wing inside `on_squadron_pass_a_construct_wing`, the Lua side (`src/game_hosts_lua.cpp`,
   unleased) changes in exactly four places, under one switch flipped together with units2's:
   - `entity_attach_lua_self_vcall_9c` loses the wing-append loop.
   - It records the pending-list size before the hook call, and marks every node appended during
     the call with `wing_member = true`, `squadron_id` = the leader's id and `class_index` = the
     squadron's.
   - `route_push_squadron` stops passing `units_before`/`units_end`.
   - The dedup's `wing_deferred` rule is retired.
   - The squadron flag annotation stays until `create_units` can mark a squadron itself.

   Predictions for that pair:
   - `wing_appended` 40 -> 0 and `wing_deferred` 40 -> 0;
   - construction pushes stay 81, with 40 of them during pass A;
   - `entities`, `self_table_entities` (86) and `wing_member_tables` (40) unchanged, and
     `squadron_ids` 40;
   - gameplay identical on USN04.
2. **The movie camera binding** (`docs/HUD_PICK_SEGMENT_QUERY.md` section 8.4). About 11 KB of
   x87 to read (007A0EB0, 00798130, 00797DA0, 0078FAF0, 007A0860, 007A2CD0), then the bind in the
   HUD host (`src/game_hosts_hud.cpp`, unleased). There is one open question: when screen 38h's
   slot `+18h` (005CDC50, the null mover) runs. It is a fresh x87-capable worker's packet.
3. **For the scene-contents owner** (`src/game_hosts_scene_contents.cpp`, unleased now):
   - **The load-time InitAll's position** (section 10.2). Run the one InitAll inside the scene
     read, at the entity-blocks boundary (0046EB4B, before the traffic, groups and deferred
     references 0046AAB0), through the Lua host's runner. Today the mission frame calls it after
     `create_units`, `issue_authored_commands` and the scoring reset.
   - **The marker pushes** (sections 10.2, 14.1). Push each marker instance at its construction
     (00928760). Then the load walk's marker pushes in `run_scene_load_init_all_0046eb4b` retire
     like the units' did.
   - **`Race`** (section 8.4). Carry the authored `Race` on `GameSceneEntityRecord`, as
     00927050's kind-1 arm reads it into `+58h`. The marker mirror 00928100 and 00928F50's unit
     mirror then write it; ships and units have no `Race` today.
4. **For the script-orders owner** (`src/game_hosts_script_orders.cpp`, unleased): the
   CreateScript script entity's 00928100 mirror at its InitAll pass C (section 8.4): `Race` -1,
   `Party` 2, `Type` `SCRIPTENTITY` (00D19034), and `Name` if named. Whether it is named is unread.
5. **For the gunnery owner** (`docs/CONSTRUCT_WORLD.md` section 27, cc9-units2's contract): the
   ship pass C torpedo stock 0081F8B0.
6. **The loopback drain's per-poster contract** (section 13.2). For each local order kind (58h
   MT_COMMAND, 5Eh fire target, 5Ah director, 8Fh pass-side), name the image's enqueue site and
   its fixed-step position. Delivery before fan-out row 9 is exact as direct. Delivery after row 9
   belongs in the next step's pump, which needs a host-wide loopback queue. The posters are in
   the commands, gunnery, ship-AI and units files. None is leased now except
   `src/game_hosts_units.cpp` (cc9-units2).
7. **Unread, noted where they arose:**
   - the callers of the XLive system's slots `54h` and `184h` (section 13.1);
   - the deck-tick launch lag's demonstration run, a mission with a queued `LaunchSquadron`
     (section 12.3);
   - Landscape's `thisTable` key (section 8.4);
   - the 004E5xxx and 004E7xxx default-pass-C classes not in the identity table (section 1.3).

### 16.3 State of this worker

- **Tree:** `J:\PROG\battlestations-pacific-decompile-cc9-init-passes`, branch
  `agent/cc9-init-passes`. It is kept, per the lead.
- **Local files:** the pair binaries are in `local\bin\{ao,pe,ig,dd,lt,rx,dt,rp,sh}_{off,on}`, with
  their logs beside them in `local\`, and the scripts are the `local\cc9-init-passes-*` files.
- **Leases:** none after this landing.

## 17. Race on the scene record, and the script entity's identity (packet `cc9_scene_race_and_script_identity`, `kSceneRaceAndScriptIdentityBound`)

Worker cc9-ships, 2026-09-27, base main `df7f875c7`. Ghidra was read only. This packet takes the
two contracts of section 8.4. The Lua host files (`src/game_hosts_lua.cpp` and its header) are
leased to cc9-units3, so the binding feeds the Lua host's existing `race` field and does not edit
the mirror.

### 17.1 Race (V)

- **00927050's kind-1 arm** reads two keys with no presence test:
  - `Race` (00CE8EE0, pushed at 0092708F) is stored at `+58h` (0092709C);
  - `Party` (00CE5804, at 009270A8) is stored at `+54h`.
  - The find, 008F2260, returns 0 on a miss (008F2348), so an absent key would fault at
    00927099. Every kind-1 bag therefore carries both keys.
- **Where they come from.** This installation's `universe/library/global.enums` declares them in
  `properties Common`: `Party = E Party:Allied` and `Race = E Races : Neutral` (lines 1755-1756,
  `enum Races` at 1712, Neutral 0 .. French 6).
  - Every class group that reaches the arm derives from `Common`: `Ship(Common)`, `Path(Common)`,
    `PlaneSquadronWNavpoint(Common)`, `LandConvoy(Common)` and the rest.
  - The scenes author Race per object: usn_2_java.scn has 40 `Race` lines (Japan 18, USA 13,
    GB 5, Dutch 4), usn_19_coralus.scn 61 and usn_1_marshall.scn 51.
- **Correction to 8.4.** A marker's `Race` is the bag value, authored or `Common`'s Neutral 0. It
  is not "authored, else -1". The same holds for `Party`: `Common` makes it Allied 0 by default,
  and the base's 2 (00925E1D) survives only on an entity with no kind-1 descriptor.
- **The two writers.** 00928F50 (vtable `+2Ch`, the party-set mirror) writes `Race = +58h` at
  00928FD9 for a unit. 00928100 (pass C of the seven marker classes) writes it at 0092814B.

### 17.2 The script entity (V)

- **CreateScript 00898750:**
  - allocates 1E4h zeroed bytes (00898834, 00BF79F0);
  - constructs through 00928630;
  - writes the vtable 00D11138 and friends (00898874..00898888) and `+C4h` = 3 (00898892).
- **Pass A,** slot `+9Ch` = 00928A00 (00D111D4), builds the self object at `+178h` (00928A36).
  00928100 tests it at 00928103, so the mirror does write.
- **Pass C,** slot `+A4h` = 009295B0 (00D111DC). With `+C0h` null (009295D2..009295DC) it calls
  00928100 at 009297E4.
- **00928100** writes:
  - `Race = +58h` (0092814B) and `Party = +54h` (00928162) as numbers. No spawn descriptor
    reaches 00927050's kind-1 arm, so these are the base 00925CE0's -1 and 2.
  - `Type = 00E0CD80[3]` = 00D19034 `SCRIPTENTITY`.
- **The name gate.** `Name` is written only when `+154h` is set (00928179).
  - The allocation is zeroed. 00928630, 00927610 and 009290A0 write neither `+154h` nor `+158h`.
  - CreateScript never calls the name setter slot `+0Ch` (004313D0), and 00927050's kind-1 arm,
    which does, does not run for it.
  - `vtable[10h]` is 0042E950, the same `+158h` getter Path uses.
  - **So the script entity is unnamed at its InitAll, and no `Name` is written.** This closes
    8.4's "contract: unread".

### 17.3 The binding

`kSceneRaceAndScriptIdentityBound` (`include/bsp/game_hosts_scene_contents.hpp`), committed OFF.
- **The record.** `GameSceneEntityRecord::race` resolves the bag's `Race` enum like `Party`. It is
  filled whatever the switch says; it has no reader when the switch is off.
- **Scene units.** Under the switch, the mission frame looks each unit's record up by name and
  sets `SceneEntity::race`. The marker seeds take their record's race too.
  - The Lua host already writes `thisTable[key].Race` for any `race >= 0`: at pass A through
    `write_party_race_fields` on the load-time InitAll, or at the attach when that switch is off.
  - The image writes a unit's Race at 00928F50 and a marker's at pass C; both run before the
    first Think.
- **Script entities.** Under the switch, each CreateScript entity gets Race -1, Party 2 and Type
  `SCRIPTENTITY` on the first mission frame after its creation.
  - **LABELLED SUBSTITUTION:** the image writes them at the next fixed step's InitAll, row 12,
    pass C.
  - The host's CreateScript runs the named global inside the call, as the image does (009290A0).
    So in both, the global's first body sees no Type.
- **The census:**
  - `summary SceneLoad race identity bound=.. races_fed=..`;
  - `summary mission script entity identity bound=.. written=..`.

### 17.4 Script readers

Section 8.3's reader census covers `.Race`, `.Type` and `.Name`. No reached script path reads
them from an entity table. The `["Race"] = Japan` rows in usn_19_coralus.lua (1061..1141) and
commandhelpers.lua 16796 are spawn parameter tables, not reads.

### 17.5 Predictions (written before the pairs; the same tree, switch only, both variables set)

- **USN04 4700/4500:**
  - `races_fed` 19..24: 19 created scene units, plus up to 5 markers whose group carries `Race`;
  - `written=13`, one per script entity created (13 in reference d).
- **USN02 9200/9000:** `races_fed` 28..30 (28 units, 2 markers) and `written=16`.
- **Both missions:**
  - one new native row, `ScriptEntity::InitAll pass C mirror_identity` (00928100), with calls
    equal to `written`;
  - pair_diff exit 1: every gameplay row, the death, plane and unit tables and every other
    summary line identical, the two census lines apart.

### 17.6 The first pair, and what it caught

The first pair was run on `81318497d`:
- exports `local\ri_off` (SHA-256 prefix `8021B739520A`) and `local\ri_on` (`D76E077E7FEE`);
- logs `local\ri_{off,on}_{usn04,usn02}.log`.

It held on gameplay: pair_diff exit 1 on both, with identical death, plane and unit tables. But
the scripts moved:
- CreateScript 13 -> 10 on USN04 and 16 -> 13 on USN02, three `luaDoTimeTable` entities fewer;
- `mission script state: Party 0 -> 2` on USN02.

**The cause was the binding, not the image.**
- `luaInit(this)` runs `this.Party = SetParty(this, PARTY_ALLIED)` (usn_2_java.lua 54,
  usn_19_coralus.lua 60) inside CreateScript's own call.
- The first binding then wrote the constant Party 2 on the next frame, over the 0.
- In the image, SetParty on the script entity reaches its `vtable[2Ch]`, which is 00928F50
  (00D11164). It works in two steps:
  - **The store.** Its base 00923B80 stores the first argument at `+54h` (00923B92). The second
    argument goes to `+58h` (00923B95), and 008A8ADF pushes the entity's own `+58h` there, so the
    race comes back unchanged.
  - **The mirror.** 00928F50 then mirrors Race and Party.
- So pass C's 00928100 later writes the Party that SetParty left, 0, not the base's 2.

**The fix** (still under the switch):
- each script entity keeps its `+54h` and `+58h` (2 and -1 from the base);
- SetParty on a script entity stores the party and mirrors both fields now (00928F50);
- pass C writes the entity's current values.

SetParty on any other entity stays the unimplemented record it was.

**Added predictions for the re-run** (written before it):
- CreateScript, SetThink and the timetable counts are identical OFF and ON (13 on USN04, 16 on
  USN02);
- `mission script state` keeps `Party=0`;
- `party_sets=1` on both missions (`luaInit`'s own call);
- `races_fed` 24 on USN04 and 30 on USN02, as the first pair measured;
- `written` 13 on USN04 and 16 on USN02;
- pair_diff exit 1, with the two census lines and the 00928100 / 00928F50 native rows the only
  changes.

### 17.7 The re-run pair, and the verdict

- **Builds.** `tools/pair_export.py` of `9e5aaf488`: `local\ri_off` (SHA-256 prefix
  `52F7C5EB76B4`) and `local\ri_on` (`4CBB89D2C0A3`, the switch flipped).
- **Logs.** `local\ri2_{off,on}_{usn04,usn02}.log`. Each shows the 1600x900 fit, the immediate
  present interval, its own module directory and the final COM release.
- **`tools/pair_diff.py`: exit 1 on both, gameplay identical.**
  - The death, plane and unit tables are identical: USN04 43 / 43 / 81 rows, USN02 21 / 0 / 32.
  - The masked multiset of other lines shows 0 lines only OFF and 0 only ON.

| row | USN04 4700/4500 | USN02 9200/9000 | predicted | held |
| --- | --- | --- | --- | --- |
| `races_fed` | 0 -> 24 | 0 -> 30 | 24 / 30 | yes |
| `written` | 0 -> 13 | 0 -> 16 | 13 / 16 | yes |
| `party_sets` | 0 -> 1 | 0 -> 1 | 1 / 1 | yes |
| native rows | + `ScriptEntity::InitAll pass C mirror_identity` 13, + `ScriptEntity::set_party_race_lua_mirror` 1, - `LuaBindingCore::entity_set_party_vtable_2c` 1 | the same, with 16 | as predicted | yes |
| CreateScript, SetThink, timetable counts | identical | identical | identical | yes |
| `mission script state` Party | 0 both | 0 both | 0 | yes |

**Verdict: `kSceneRaceAndScriptIdentityBound` ON.**
- Scene units and markers carry the bag's Race into `thisTable`.
- A script entity carries 00928100's Race, Party and Type, with SetParty's value kept as the image
  keeps it.
- No reference mission reads the fields, so nothing else moves.
- **Open:**
  - the marker mirror in `src/game_hosts_lua.cpp` still writes Party only; Race reaches markers
    through the pass-A write instead, on a file this packet could not lease;
  - SetParty on a non-script entity (`vtable[2Ch]` for units) is still a record.

## 18. The marker Race mirror: already carried (packet `cc9_marker_race_mirror`, read only, no switch)

2026-09-27, worker cc9-units3, on main 9d7e418eb. The item came from two handoffs:
docs/UNIT_WEAPON_DEVICES.md, cc9-ships's item 5, and section 8.4 above, "the marker mirror should
write Race itself (authored, else -1)".

**It is closed by section 17.**
- **The value.** 17.1 corrected 8.4: a marker's `Race` is its bag value, authored or `properties
  Common`'s Neutral 0, from 00927050's kind-1 arm (`+58h` at 0092709C). It is not "authored, else
  -1". Every kind-1 bag carries the key.
- **The write.** Under `kSceneRaceAndScriptIdentityBound` (ON), the marker seeds take their
  record's race, and the Lua host writes `thisTable[key].Race` for every `race >= 0`. The
  reference e logs show it:
  - `races_fed=30` on USN02 (28 units and 2 markers);
  - `races_fed=24` on USN04 (19 units and 5 markers);
  - `local\rb5_usn02.log` / `rb5_usn04.log`, worktree cc9-units3.
- **What differs from the image, and why it is left.**
  - **When.** The host writes the marker's Race at the load attach, or at pass A of the load-time
    InitAll (`write_party_race_fields`). The image writes it at pass C (00928100, 0092814B).
    Both run before the mission's first Think, and no script reads it in between.
  - **The −1 case.** 00928100 writes `+58h` even when it is the base's −1 (00925CE0), which
    happens only for an entity with no kind-1 descriptor. `mirror_identity_00928100` in
    `src/game_hosts_lua.cpp` skips Race, so such an entity would read nil, not −1. No scene
    marker lacks a kind-1 bag. The script entities, the one −1 case, are written by section 17's
    script-entity identity.
- **Readers.** No mission Lua of this installation reads an entity's `.Race`:
  - the only `.Race` read is chg_2_java.lua 442..443 on a local `template` table;
  - `luaGetNmiRace` (commandhelpers.lua 16772) reads `Mission.Party`;
  - the `["Race"] = ...` rows are spawn parameter tables (17.4).

**Prediction for any binding of the −1 write: identity on every mission,** with no census line
moving. No switch is added, and `src/game_hosts_lua.cpp` (cc9-hud2's lease) is not edited. A
one-line edit writing `Race = -1` in `mirror_identity_00928100` when no race is carried remains
available if a scene ever authors a marker outside `Common`.

## 19. The squadron pass hooks at load time (packet `cc9_load_time_squadron_hooks`, `kLoadTimeSquadronHooksBound`)

Worker cc9-init2, 2026-09-27, base main 222a80033. Ghidra was read only. This closes the gap
docs/CONSTRUCT_WORLD.md section 32 measured on JM08: the load walk (section 10) never calls the
units host's squadron hooks, so a squadron built from a scene row never reads its `HomeBase`.

### 19.1 The image

- **The same walk.** All four scene-read sites load `CL` = 0 and call 00925F20: `XOR CL,CL` at
  0046EB49 before 0046EB4B, and at 0046ED0D before 0046ED0F. That is the flag the runtime InitAll
  passes too, so the passes a node runs do not depend on the call site.
- **A squadron's two slots.** The PlaneSquadron vtable is 00D087C0:
  - `+9Ch` holds 007F4580 (the DATA xref at 00D0885C): pass A, which constructs the wing;
  - `+A4h` holds 007F4BA0 (the DATA xref at 00D08864): pass C, which reads `HomeBase` at
    007F4C43, calls 007F1C00 at 007F4CFA, and issues the initial command at 007F4E9E.
- **So every squadron the scene read constructs runs both,** in the one load InitAll, as every
  mission-time squadron does.
- **Order against the authored commands.** The end-of-file site 0046ED0F runs before
  `ResolveDeferredReferences` 0046AAB0 (0046ED1E), which issues each unit's authored command. In
  the image a load squadron's pass C therefore sees no current command. The host issues the
  authored commands before its load walk (section 10.2, "Not reproduced"), so a row that authors a
  `Command` reaches pass C with one and takes 007F4E0C's skip. **Not reproduced, labelled.**
  Where the authored command replaces the initial one, the end state is the same.

### 19.2 The host today

- **Which nodes are squadrons.** Section 15.1 marks only route squadrons (GenerateObject, SpawnNew,
  air-ops) and adds no load-node test. So the load walk's squadron nodes carry `squadron = false`,
  and passes A and C skip both hooks. JM08 reads `pass_a=0 pass_c=0`.
- **The wing is already built.** `create_units` builds a scene row's wing at load, from the
  records that the `plane squadrons: ... wing record(s) appended` line counts, and stages nothing.
  So the pass A hook finds nothing staged and constructs nothing. **SUBSTITUTION, labelled
  (existing):** the image constructs these planes inside pass A.
- **The member array is resolved before the walk.** `create_units` ends with
  `resolve_plane_squadron_members(..., only_unresolved=true)`, and the mission frame calls the load
  walk after it. So the registry answers for a scene squadron's leader when the walk runs.

### 19.3 The binding (planned; `src/game_hosts_lua.cpp` is cc9-hud2's lease at the time of writing)

- **Under `kLoadTimeSquadronHooksBound`,** `run_scene_load_init_all_0046eb4b` marks a load node
  `squadron = true` when the plane-squadron registry holds a record that did not come from an
  air-ops launch and whose `squadron_unit` (the fused leader, slot 0) is the node's unit index.
  Passes A and C then call the two hooks exactly as for a route squadron.
- **No wing range.** The node records none. With `kWingConstructionLuaActive` the pass A loop marks
  only nodes the hook pushes, and a scene squadron's hook pushes none.
- **Summary line:** `summary SEntity::InitAll load squadron hooks bound=<0|1> squadrons=N`.
- **Left open:** the load wing planes (the `|.-2` and `|.-3` units) are plain load nodes. They
  carry no `wing_member` or `squadron_id`, so pass C writes no `SquadronID` for them (007C9770
  through plane+9D4h). That is a separate gap, not bound here.

### 19.4 Predictions (written before any pair; both variables set, lockstep 0.05, idle player)

**Step 1, `kLoadTimeSquadronHooksBound` alone** (`kSceneHomeBaseQualifiedNameBound` OFF):

- **JM08 3200/3000.** Six scene squadrons: Movie Mavis, H6K Mavis 01..03, Ki-43 Oscar 01 and
  Gekko 01.
  - `load squadron hooks squadrons` 0 -> 6; `squadron pass hooks` 0/0 -> `pass_a=6 pass_c=6`; the
    two native hook rows are added with 6 calls each.
  - Wing construction unchanged: `staged=0 builds=0 planes=0 left_staged=0`. `wing_marked` and
    `squadron_ids` stay 0.
  - Home base: `keys=2`, `resolved` stays 0, `unresolved` 0 -> 2, `queue_pushes` stays 0.
  - Initial command, for squadrons whose leader is active at load: the three H6K Mavis rows author
    `Command = Stop`, so `skipped_current` 0 -> 3. Oscar 01 and Gekko 01 have no home, so
    `no_home` 0 -> 2. Movie Mavis authors no command and no home, so it adds 1 to `no_home`, or 1
    to `stops` if its leader is in water mode (+900h = 6). An inactive leader returns silently and
    adds to none of these.
  - `movetos` stays 0. **Gameplay identical, unless Movie Mavis takes the water stop.**
- **USN04 4700/4500.** One scene squadron, `movieval` (usn_19_coralus.scn: no command, empty
  `HomeBase`, 700 m up). The brief expected identity; the scene row makes it one more squadron.
  - `pass_a` 20 -> 21 and `pass_c` 20 -> 21, and both native hook rows 20 -> 21;
  - `keys=0`, so the home-base census does not move;
  - `no_home` 16 -> 17 (unchanged if the leader is inactive at load); `movetos` stays 4;
  - no command is issued, so **gameplay identical** (`pair_diff` exits 1).
- **USN02 9200/9000.** No scene row builds a squadron (the log has no `plane squadrons:` line), so
  nothing moves and `pair_diff` exits 0.

**Step 2, `kSceneHomeBaseQualifiedNameBound` ON** (its own commit and pair, on top of step 1 ON):

- **JM08.** `unresolved` 2 -> 0, `resolved` 0 -> 2, `queue_pushes` 0 -> 2, both keys matching the
  deck "MainAirFieldEntity 01". Oscar 01 and Gekko 01 then find a home unit, so `no_home` falls by
  2 and `movetos` rises by 2 if both leaders are active. **Gameplay moves:** the two squadrons are
  ordered to their airfield, and two entries join the deck's assign queue (006CC7B0).
- **USN04 and USN02.** No key, so identity (`pair_diff` exits 0, or 1 on noise only).

### 19.5 Pairs and verdict (step 1)

- **The runs.** The OFF binary is `local\bin\lsh_off` (a build of 913a9061b). The ON binary is
  `pair_export` of a04aa3998 with the switch flipped (`local\lsh_on`, SHA-256 45F7BD959143).
  a04aa3998 changes nothing with the switch OFF, so the OFF logs stand. `BSP_GUNNERY_RNG_STREAMS=1`
  and `BSP_DEATH_TABLE=1` were set, lockstep 0.05, idle player, and each log has its module
  directory under this tree and the final native table.
- **The first JM08 pair caught a gap.** The ON export of 913a9061b ran pass C six times but pass A
  never. The Lua pass A returned when the script-orders host was absent, and it is absent during
  the scene read's walk. a04aa3998 limits that guard to the legacy wing append, the only code that
  reads that host. **Failed prediction, recorded.**

| row | JM08 OFF | JM08 ON | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| load squadron hooks `squadrons` | 0 | 6 | 0 | 1 | 0 | 0 |
| `pass_a` / `pass_c` (and the two native rows) | 0 / 0 | 6 / 6 | 20 / 20 | 21 / 21 | 0 / 0 | 0 / 0 |
| wing construction `staged builds planes` | 0 0 0 | 0 0 0 | unchanged | unchanged | 0 | 0 |
| `skipped_current` | 0 | 3 | 0 | 0 | 0 | 0 |
| `no_home` | 0 | 3 | 16 | 17 | 0 | 0 |
| `movetos` / `stops` | 0 / 0 | 0 / 0 | 4 / 0 | 4 / 0 | 0 / 0 | 0 / 0 |
| home base `unresolved` | 0 | 2 | 0 | 0 | 0 | 0 |
| deaths, hit records | 2, 127 | identical | 44, 789 | identical | 21, 652 | identical |
| death rows, plane death modes, unit table | 2, 1, 43 | identical | 44, 44, 81 | identical | 21, 0, 32 | identical |
| `pair_diff` exit | | 1 | | 1 | | 1 |

- **JM08.** Every row moved as predicted. Movie Mavis took `no_home`, not the water stop. The two
  `names no entity` lines for Ki-43 Oscar 01 and Gekko 01 are ON only.
- **USN04.** `movieval` is the one load squadron, and only the predicted rows moved.
- **USN02.** Only the new summary line's `bound` field differs, so `pair_diff` exits 1 where 0 was
  predicted. **Failed prediction, recorded;** nothing else moved.
- **Noise found on JM08.** The same OFF binary ran twice gave `ShipAiSectorScan::clip_arc_zones`
  calls of 12000 and 0 (and `zone_segment_crossing` 6000 and 0). The first ON run gave 6000.
  Gameplay was identical in all three, so these two rows vary between identical JM08 runs and are
  not this switch's.

**Verdict: ON.**

### 19.6 Open: the load wing planes carry no squadron id

A scene squadron's wing planes (for example `Ki-43 Oscar 01|.-2`, `movieval|.-2`) are plain load
nodes. The pass A hook pushes nothing for them, so `kWingConstructionLuaActive`'s marking never
sets their `wing_member`, `squadron_id` (007F4B49's plane+9D4h) or `class_index`. So pass C writes
no `SquadronID` into their `thisTable` (007C9770), and JM08 reads `squadron_ids=0`. In the image
the planes are constructed inside the squadron's pass A and carry +9D4h. The follow-up: in the load
walk, mark the registry record's other `member_units` as that squadron's wing.
