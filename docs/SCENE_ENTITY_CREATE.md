# Scene entity creation by name (`0046D930`) and the spawn wrapper (`004C6BA0`)

Addresses: 0046d930 004c6ba0 00922e20 008f41f0 008f54f0 00925a90 00468cd0 00468fb0 00468660 0046c550 00925f20 004ba870 0046cf40 0046df00

How a mission entity instance comes to exist on the **by-name** path: the Lua spawn bindings and
the procedural cloud scatter both ask the scene database for an instance of a named record.
`0046D930` is that routine. The scene-file reader does **not** go through it; the two paths
converge one level lower, at the class descriptor's creator. See "Where the scene file enters".

`docs/SCENE_ENTITY_FACTORY.md` owns the class-id table, the class map and the `0046C550` gate;
`docs/SCENE_FILE_READER.md` owns the property bag and the tokenizer. Those are used here as
contracts and are not re-derived. Names below marked *hypothesis* are descriptive, not recovered
symbols.

## `0046D930` ABI

`__thiscall`, `RET 0Ch` (`0046D9C3` on the two failure returns, `0046DC01` on the success return).

```
Entity* 0046D930(SceneDatabase* this,   // ECX
                 const char* classKey,      // +4h  key into the record map at this+18h
                 const char* instanceName,  // +8h  the created entity's name
                 PropertyBag* overrides)    // +Ch  optional per-instance property overrides, may be 0
```

Returns the new entity, or **null** on either of two failures (record not found, gate rejected).

`this` is the scene database, established three ways:

| Evidence | Site |
| --- | --- |
| the wrapper loads `*(00E18680)` into ECX before the call | `004C6BB2` |
| the cloud scatter loads `*(00E18680)` into ECX before the call | `004BAAFC` |
| the saved `this` (stored at `0046D957` into `entry-60h`) is handed to `0046C550` as ECX, whose ECX `docs/SCENE_ENTITY_FACTORY.md` establishes as the scene database | `0046DB22` |

The routine still loads `*(00E18680)` explicitly for the class-id→name lookup at `0046DB1D`
instead of reusing `this`, so this site does not prove the two are the same object; it only
shows `this` is used where a scene database is required.

### The `classKey` argument slot is reused as a local

After `0046D95B` consumes it, the compiler reuses the incoming `classKey` slot at `entry+4h` as
scratch. It holds the resolved parent entity (`0046DA33`, `0046DA5A`, read back at `0046DA69` and
`0046DB37`) and then the `operator new(0Ch)` result (`0046DB75`, `0046DBB9`). A decompiler view
shows this as writes to `param_1`; it is not an out-parameter. Anchor for the frame arithmetic:
`entry-04h` is the SEH try level (`0046D9F6` sets 0, `0046DA0C` and `0046DB93` restore `-1`), and
`entry-0Ch` is the saved `ExceptionList` reloaded at `0046D9B1` and `0046DBED`.

## Sequence

| # | Site | Callee | Name (hypothesis unless noted) | this / args / ret | Gate |
| --- | --- | --- | --- | --- | --- |
| 1 | `0046D95B` | `0041E870` | `BSP_NativeString_Assign` (existing) | ECX = `&key` (`entry-5Ch`), arg `classKey`; `RET 4` | — |
| 2 | `0046D96F` | `00468CD0` | `scene_record_map_find` | ECX = `this+18h`, args `&it` (`entry-54h`), `&key`; `RET 8`; `it = {container, node}` | — |
| 3 | `0046D987` | `00419CC0` | `BSP_SizedStoragePool_GetSingleton` (existing) | args `key.data`, `key.length+1`, `1`; `__cdecl`, args left for step 4 | `key.data != 0` (`0046D97A`) |
| 4 | `0046D98E` | `00BD1510` | `BSP_SizedStoragePool_ReturnBlock` (existing) | ECX = the pool; consumes step 3's three args; releases the temporary string | same |
| 5 | `0046D9AB` | — | end check | `node == *(this+18h+4)` (`_Myhead`) → **return 0** | — |
| 6 | `0046D9D9` | — | `record = *(node+14h)` | MSVC `std::map` node: `_Left/_Parent/_Right` at 0/4/8, the 8-byte `NativeString` key at `0Ch`, the mapped pointer at `14h` | — |
| 7 | `0046D9E4` | `0041E870` | `BSP_NativeString_Assign` (existing) | ECX = `&key2`, arg `record->className` (`record+0Ch`) | — |
| 8 | `0046D9FE` | `00468FB0` | `BSP_SceneClassMap_FindNode` (existing) | ECX = `this+34h`, args `&key2`, `&entry+4h`; `RET 8`; `classRow = *(ret+8)` at `0046DA03` | — |
| 9 | `0046DA21` / `0046DA28` | `00419CC0` / `00BD1510` | pool release of `key2` | as steps 3-4 | `key2.data != 0` (`0046DA14`) |
| 10 | `0046DA55` | `00925A90` | `scene_node_find_child_by_name` (not entered in the ledger, see below) | ECX = `*(*(00E188A8)+19CCh)`, arg `record->parentName.data` (`record+58h`) or `00E18560`; `RET 4` | `record->parentName.length != 0` (`0046DA2D`), else parent = 0 (`0046DA33`) |
| 11 | `0046DA5E`..`0046DB00` | — | identity frame | 16 floats built on the stack, `1.0f` from `00D7A24C` at `+0/+14h/+28h/+3Ch`, `0.0f` elsewhere, then `REP MOVSD` of 16 dwords into the 40h-byte by-value slot of step 13 | — |
| 12 | `0046DB1D` | `00468660` | `BSP_SceneDatabase_ClassIdToName` (existing) | ECX = `*(00E18680)`, arg `classRow->id` (`*classRow`); `RET 4` | — |
| 13 | `0046DB27` | `0046C550` | `BSP_SceneEntity_ShouldGenerate` (existing) | ECX = `this`; `RET 5Ch`; **`AL == 0` → return 0** (`0046DB2E` jumps to the `0046D9AF` return-null tail) | the generation gate |
| 14 | `0046DB4B` | `*(classRow+4)` | the class creator (indirect) | `__fastcall` ECX = `classRow->id`, EDX = `instanceName`, stack `parent`, `&record->localFrame`, `record->properties`, `0`; returns the entity into EBP | — |
| 15 | `0046DB5A` | `008F41F0` | `BSP_ScenePropertyBag_Clone` | ECX = `record->properties`; no stack args; returns a fresh 114h bag | `overrides != 0` (`0046DB55`) |
| 16 | `0046DB66` | `008F54F0` | `BSP_ScenePropertyBag_Apply` | ECX = the clone, args `overrides`, `1` | same |
| 17 | `0046DB6D` | `00BF681B` | `operator new` (existing) | arg `0Ch`; `__cdecl` | same |
| 18 | `0046DB88` | `00922E20` | `BSP_ScenePropertyBagRef_Construct` | ECX = the 0Ch block, arg the clone; `RET 4` | holder allocation succeeded (`0046DB83`) |
| 19 | `0046DBAB` | `*(*tmp)` | the clone's deleting destructor | ECX = the clone, arg `1` | `clone != 0` (`0046DBA1`) |
| 20 | `0046DBB1` | `00BF681B` | `operator new` (existing) | arg `0Ch` | `overrides == 0` (`0046DB55`) |
| 21 | `0046DBCF` | `00922E20` | `BSP_ScenePropertyBagRef_Construct` | ECX = the 0Ch block, arg `record->properties`; `RET 4` | holder allocation succeeded (`0046DBC7`) |
| 22 | `0046DB9B` / `0046DBE0` | — | `entity->+C0h = holder` | 0 when `operator new` returned null | — |
| 23 | `0046DBE8` | `00925F20` | `BSP_SEntity_InitAll` (existing) | CL = 0, no stack args | — |
| 24 | `0046DBF3` | — | `return entity` | EAX = EBP | — |

Steps 15-19 and steps 20-21 are the two arms of one branch: with overrides the record's bag is
cloned, the overrides merged in, wrapped, and the temporary clone destroyed; without overrides the
record's bag is wrapped directly. `00922E20` clones again internally, so the wrapped bag is always
a private copy and the record's own bag is never handed to the entity.

### The `0046C550` argument block

The argument setup for step 13 is interleaved with step 12's, which is why the block looks larger
than the call. `0046C550` is `__thiscall`, `RET 5Ch`, and the cleanup closes the frame exactly:
from `entry-CCh` back to `entry-70h`. Mapped onto the signature in `docs/SCENE_ENTITY_FACTORY.md`:

| Slot | Value at this site | Pushed at |
| --- | --- | --- |
| a1 `className` | `00468660(*(00E18680), classRow->id)` | `0046DB26` |
| a2 `entityName` | `instanceName` (arg `+8h`, held in ESI from `0046DB05`) | `0046DB1B` |
| a3 `parent` | the resolved parent or 0 | `0046DB1A` |
| a4 `localFrame[16]` | `&record->localFrame` (`record+14h`) | `0046DB16` |
| a5 `properties` | `record->properties` (`record+4`) | `0046DB0C` |
| a6 unused | `0` | `0046DA74` |
| a7..a22 `parentFrame[16]` | the identity matrix, by value | `0046DA6F` + `0046DB00` |
| a23 `outDeferredRecord` | `0` | `0046DA6D` |

That the caller passes `0` for a23 matches the factory doc's "the caller always passes 0", so the
deferred-record branch of `0046C550` can fire from this path too.

## The record in the map at `SceneDatabase+18h`

The map is keyed by `NativeString` and holds a pointer per entry. The record's fields are read
here only where the routine touches them; nothing else about it is asserted.

| Offset | Field | Evidence |
| --- | --- | --- |
| +4 | `PropertyBag* properties` | ECX of `008F41F0` at `0046DB57`, a5 of `0046C550`, the argument of `00922E20` at `0046DBCC` |
| +8 / +0Ch | `NativeString className` — length at +8 (**provisional**, not read here), data at +0Ch | `0046D9DC` feeds `+0Ch` to `BSP_NativeString_Assign`, and the result keys the class map |
| +14h..+53h | `float localFrame[16]` | a4 of `0046C550`, which reads `a4[+30h]` and `a4[+38h]` as X and Z; the 40h span ends exactly where +54h begins |
| +54h / +58h | `NativeString parentName` — length at +54h, data at +58h | `0046DA2D` tests the length, `0046DA3D` takes the data and substitutes the empty string `00E18560` when it is null |

The `{length, data}` shape is the same one the temporary strings use at `0046D97C`/`0046D9D9`
(the pool release reads the length at +0 and the buffer at +4), and `00E18560` is the same default
`00468660` returns when no class row matches.

## What `0046D930` writes on the new entity

| Offset | Value | Site | Note |
| --- | --- | --- | --- |
| +C0h | the 0Ch property-bag reference holder, or 0 when `operator new(0Ch)` failed | `0046DB9B` (override arm), `0046DBE0` (plain arm) | the **only** entity field this routine writes |

Everything else an instance carries — its name, its class, its party, and the u16 at `+174h` that
`docs/MISSION_LUA_SELF_TABLE.md` shows keying the Lua self table — is written **inside the class
creator** at step 14, not here. The creator receives `instanceName` in EDX and the record's
property bag as its third stack argument, which is where those values must come from. The 26
creator bodies are out of this packet's scope; `docs/SCENE_ENTITY_FACTORY.md` lists them and
records that 23 of the 26 were not opened. **Partial coverage:** this document does not establish
the name, class, party or `+174h` writes, only that `0046D930` is not the writer.

## The property-bag reference holder (`00922E20`)

Body `00922E20`-`00922E41`, complete, `__thiscall(void* this, PropertyBag* src)`, `RET 4`.

```
this->vtable = 00D03D94     // 00922E27
this->refs   = 1            // 00922E35
this->bag    = 008F41F0(src) // 00922E2D, __thiscall with ECX = src and no stack args
return this                  // 00922E3C
```

A 12-byte intrusively counted holder `{vtable +0, refs +4, bag +8}` that takes a **private clone**,
not a reference to the argument. It has nine callers; only the two inside `0046D930` are attributed
here.

`008F41F0` is leased by the peer packet `cc2_scene_property_bag` and is an **external contract**
here: the body was read but the routine is deliberately **not** entered in the ledger by this
packet. `008F41F0` (`008F41F0`-`008F4289`, `__thiscall(PropertyBag*)`, no stack args) is the clone:
`operator new(114h)`, vtable `00D16504` with `00D162C4` at +4, +8 cleared, 100h bytes zeroed from
+0Ch, +10Ch and +110h cleared, then a walk of the source's entry list at `src+4` (`00480690` /
`0047E480`) inserting each entry through `008F33F0` with `008F4F60` applied to the value and
`00F89450` substituted for a null name. `008F54F0` is the merge used for the overrides; its body
was not decoded beyond the entry walk, so it is a contract here.

## `004C6BA0`, the spawn-side wrapper

Body `004C6BA0`-`004C6BD6`, complete. `__thiscall`, `RET 0Ch`.

```
Entity* 004C6BA0(Game* this, const char* classKey, const char* instanceName, PropertyBag* overrides)
```

| # | Site | Callee | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| 1 | `004C6BB9` | `0046D930` | ECX = `*(00E18680)` (`004C6BB2`), args forwarded unchanged in order | — |
| 2 | `004C6BCD` | `004C3840` `BSP_Game_AssignPartyPlayerSlots` (existing) | ECX = `this` (the game), arg `0` | `*(this+1FE4h) != 0` (`004C6BBE`) |

It substitutes the global scene database for its own `this` and adds **nothing to the entity**.
The single thing it adds for a script-spawned entity is the party/player-slot reassignment on the
game object, and only when `game+1FE4h` is non-zero. The entity is preserved in EDI across that
call (`004C6BC5`, `004C6BD2`) so both paths return it, including the null a failed creation
returns. Its three callers are `BSP_LuaBinding_Spawn` `00944680`, `BSP_LuaBinding_GenerateObject`
`00944FD0` and `00945450`, per `docs/LUA_BINDING_SPAWN.md`.

## Where the scene file enters

**The Instantiate pass does not reach `0046D930`.** `0046D930` has exactly two callers,
`004BAB12` in `BSP_SceneRecord_ScatterClouds` and `004C6BB9` in the wrapper (`ghidra xrefs`).
Neither `0046DF00 BSP_SceneFile_Read` nor `0046CF40 BSP_SceneFile_ReadEntityBlock` is among them,
and `0046D930` is absent from both callee sets.

The two paths converge one level lower, at the class descriptor's creator:

| Path | Creator call | How the descriptor is reached |
| --- | --- | --- |
| scene file | `0046D5A4 CALL EAX` with `EAX = *(descriptor+4)` (`0046D599`) | the reader already holds the descriptor |
| by name | `0046DB4B CALL EAX` with `EAX = *(classRow+4)` (`0046DB45`) | record `+0Ch` class name → `BSP_SceneClassMap_FindNode` → `*(node+8)` |

So the by-name path is a parallel front end that resolves a record and a class before calling the
same `descriptor+4` creator the scene file calls directly. The gate `0046C550` runs on both.

## The cloud scatter call site

`004BAB12`, inside `BSP_SceneRecord_ScatterClouds` (body `004BA870`-`004BAC12`):

```
004baafc  MOV ECX,[00E18680]        ; this = the scene database
004bab04  PUSH 0x0                  ; overrides = 0
004bab06  PUSH 0xCE7524             ; instanceName = "Cloud"
004bab0b  LEA EAX,[EDI + 0xE081C8]  ; classKey = a row of the 18h-stride kind table
004bab11  PUSH EAX
004bab12  CALL 0x0046D930
```

`classKey` is the address of a row of the kind table at `00E081C8`, whose rows begin with the
inline name characters (`CloudSmall`, `CloudMedium`, `CloudBig` per
`docs/MISSION_SCENE_CONTENTS.md` line 187), so it is an ordinary C string. The instance name is the
literal `"Cloud"`, and `Cloud` is class id `3Dh` in the factory doc's table — the record keyed by
`CloudSmall` names the class `Cloud`, which is the two-level indirection this routine exists for.

## Correction to `docs/SCENE_ENTITY_FACTORY.md`

That file is owned by another packet and was **not edited**; the correction is recorded here.

**was** (line 204): "The creator's ABI, read at the `0046D59D` call site: `ECX` = the class id,
`EDX` = a pointer to the frame block, then four stack arguments of which the first is the property
bag."

**is**: `ECX` = the class id (unchanged). `EDX` = the **entity name**. The frame block is the
**second** stack argument and the property bag the **third**; the first is the parent.

**evidence**: at `0046DB49` `MOV EDX,ESI` with ESI loaded from the `instanceName` argument at
`0046DB05` and preserved across two calls that both save ESI, so EDX is a name string, while the
frame block `&record->localFrame` is pushed separately at `0046DB44`. The push order at `0046DB3B`
is `0`, `record->properties`, `&record->localFrame`, `parent`, so ascending the stack args are
parent, localFrame, properties, 0. At the other site the same reading holds: `LEA EDX,[ESP+0B0h]`
at `0046D59D` resolves to the same frame slot as `LEA EDX,[ESP+0A0h]` at `0046CF97`, the
destination of the byte-copy loop that copies the tokenizer's name string, and the frame block
there is `LEA ECX,[ESP+50h]` pushed at `0046D596` as the second stack argument.

**caveat**: the `0046D5A4` half of the evidence rests on ESP being unchanged between `0046CF97`
and `0046D57E`, which was taken from the fixed 190h frame and the accounted-for argument pushes,
not verified instruction by instruction across the whole reader. The `0046DB4B` half is exact.
The identity of `EBX` at `0046D591` inside `0046CF40` was not established, so "the property bag is
the third stack argument" is asserted from `0046DB4B` alone.

## Reconstruction

`include/bsp/scene_entity_create.hpp`, `src/scene_entity_create.cpp`. The creation is a sequence
over `SceneEntityCreateHost`, one virtual per native callee, plus pure rules for the branch
decisions (the record-missing return, the gate return, the override arm, the identity frame, the
party-slot gate). Nothing here is a binary-compatible replacement: the native records stay opaque
pointers because their layouts are not reconstructed, and Lua, the STL, the CRT and the scene-file
reader are contracts, not ports.

| Routine | Coverage |
| --- | --- |
| `0046D930` | complete: every instruction of `0046D930`-`0046DC03` is accounted for |
| `004C6BA0` | complete: `004C6BA0`-`004C6BD6` |
| `00922E20` | complete: `00922E20`-`00922E41` |
| `008F41F0` | analyzed, not reconstructed: the clone's shape and allocation size, not its entry insert |
| `008F54F0`, `00925A90`, `0046C550`, `00468660`, `00468FB0`, `00468CD0`, `00925F20` | contracts |
| the class creator at `classRow+4` | not opened; 26 bodies, see `docs/SCENE_ENTITY_FACTORY.md` |

## Uncertainties

- Whether `this` of `0046D930` is always `*(00E18680)`: both call sites pass it, but the routine
  reloads the global at `0046DB1D` rather than reusing `this`, which a same-object assumption
  would not need. Provisional.
- The record's class-name length at `record+8` is inferred from the `{length, data}` shape of the
  other two strings, not read.
- `008F54F0`'s merge semantics (which side wins on a duplicate key, what the `1` argument selects)
  were not decoded. The override arm is modelled as "apply and win" only because the clone is the
  destination.
- The u16 at `entity+174h`, the entity name, class and party are not written by `0046D930`; which
  creator instruction writes each is unread.
- `00922E20` has nine callers; the other seven were not read, so the holder's role elsewhere in
  the engine is not asserted.

## Follow-up packets

| Packet | Addresses | Contract |
| --- | --- | --- |
| `scene_entity_creator_fields` | 004e9e40 004e9920 | Which creator instruction writes the entity name, class, party and the u16 at `+174h`, for the `Cloud` exemplar |
| `scene_property_bag_merge` | 008f54f0 008f33f0 008f4f60 | The merge rule and the meaning of the `1` argument |
| `scene_record_map` | 00468cd0 | What fills the record map at `SceneDatabase+18h` and the full record layout |

## Correction from docs/SCENE_RECORD_MAP.md (packet cc2_scene_record_map)

The map at `SceneDatabase+18h` that `0046D930` resolves the name in is the **hidden-entity
record map**: `std::map<NativeString, SceneHiddenEntityRecord*, LessCaseInsensitive>`, filled by
`BSP_SceneFile_ReadEntityBlock` only for entities authored with the `Hidden` property (parsed,
never instantiated during the read), and read by the `FindHiddenEntity` binding `008A9E10`.
So `BSP_SceneDatabase_CreateEntityByName` instantiates an authored *hidden* entity by name, which
is what `Spawn` and `GenerateObject` do; `record+8h` is the strdup'd entity name, not a class-name
length, and the record is `5Ch` bytes (vftable `00CE5640`, bag `+4h`, name `+8h`, class name
`+0Ch`, party `+10h`, sixteen floats `+14h`, parent name `+54h/+58h`).
