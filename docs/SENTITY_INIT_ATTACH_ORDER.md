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

(Filled in after the runs.)
