# Scene `traffic` block (`009514B0`) and the vehicle-class preload census

Addresses: `009514B0`, `009512F0`, `00951220`, `00951160`, `00950FC0`, `0049CF80`, `004A5620`,
`0095CA10`, `0095C640`, `0095C550`, `0095C4D0`, `0046BF70` (contract), `0049F910`, `0049F9B0`,
`00499030`, `00499320`, `004A50D0`, `00964790` (contract), `00925A90` (contract), `007AC9D0`,
`008F5A00`/`008F41A0`/`008F5410` (contracts), `0046DF00` and `0046CF40` call sites.

Packet `cc2_scene_traffic_groups`, read-only analysis. Every name here is a hypothesis, not a
recovered symbol. Closes the two "not decoded" rows of `docs/SCENE_FILE_READER.md`'s tail-block
table for `traffic`, and the pass-2 half of the same row. Contracts:
`docs/SCENE_PROPERTY_BAG.md` (the 114h bag and the 38h property record),
`docs/MISSION_SCENE_CONTENTS.md` (the census array and the pass bracket),
`docs/VEHICLE_CLASS_DESCRIPTORS.md` (the registry's `+10h` forward index map),
`docs/FIXED_STEP_FANOUT.md` (`00925F20`).

## Grammar

```
traffic  := "traffic" "{" ( item | <token> )* "}"
item     := "item" TOKEN "{" "path" "=" TOKEN propsection "}"
```

`009514B0` does not parse `traffic`'s body as a fixed production. It runs a brace counter: after
`"traffic" "{"` it peeks, and only the literal `item` (`00D19B80`) hands control to `009512F0`.
Every other token is counted (`{` at `00951518` increments, `}` at `00951534` decrements) and
consumed, so an unknown child block is skipped rather than rejected. The loop ends when the
counter reaches zero (`0095153E TEST EDI,EDI ; JG 009514E0`).

`009512F0` *is* a fixed production: five `BSP_SceneTokenizer_ExpectToken` calls in a row
(`item`, `{`, `path` `00D099C0`, `=` `00CE55BC`, then `}` at the end) with one optional
`properties` branch gated on a peek against `00CE568C`.

### Installed-file check

`local/census_traffic_groups.py` tokenizes all 259 `.scn` files under
`universe/scenes` with the delimiter set the tokenizer uses and walks both blocks.

| Measure | Value |
| --- | --- |
| files with a `traffic` block | 251 |
| `item` records | 172 |
| `item` children other than `path` and `properties` | 0 |
| items whose first child is not `path` | 0 |
| items with no `properties` section | 0 |
| distinct `templates` entries | 12 |
| grammar errors | 0 |

Every one of the 172 items authors the same 14 property keys. Thirteen are the ones `0049CF80`
reads; the fourteenth, `anims`, is parsed into the bag and never read by the applier.

## `009514B0` — the block reader

`__thiscall(void* this, SceneTokenizer* tok)`, `RET 4` at `00951555`. `this` is the static
object `00F8A084`, pushed as `MOV ECX,0xF8A084` at the only call site `0046EB56`. That object is
a vector of record pointers with the MSVC iterator-debug layout: proxy `+0h`, first `+4h`
(`00F8A088`), last `+8h` (`00F8A08C`), end `+0Ch` (`00F8A090`).

| # | site | callee | name | this / args / ret | gate |
| --- | --- | --- | --- | --- | --- |
| 1 | `009514B5` | `00951160` | `traffic_records_clear` | `__thiscall(vector)`, ECX still the entry `this` | none |
| 2 | `009514C5` | `008D9930` | `BSP_SceneTokenizer_ExpectToken` | ECX = tok, push `00CE5890` `"traffic"` | none |
| 3 | `009514D1` | `008D9930` | same | push `00CE4D38` `"{"` | none |
| 4 | `009514E2` | `008D8A70` | `BSP_SceneTokenizer_PeekToken` | ECX = tok, token in EAX | loop head |
| 5 | `009514EE` | `00438E10` | `BSP_CString_CompareInsensitive` | ECX = token, EDX = `00D19B80` `"item"` | none |
| 6 | `009514FA` | `009512F0` | `traffic_item_read` | ECX = vector, push tok | compare == 0 |
| 7 | `00951503`/`0095151F` | `008D8A70` + `00438E10` | brace probes | EDX = `00CE4D38` `"{"`, then `00CE4CD4` `"}"` | item compare != 0 |
| 8 | `00951539` | `008D8960` | `BSP_SceneTokenizer_ConsumeToken` | ECX = tok | item compare != 0 |
| 9 | `0095154D` | `004A5620` | `traffic_records_commit` | ECX = `[[00E188A8]+21D0h]` | after the loop |

Step 6 does not consume: `009514FF JMP 0095153E` jumps past the consume at `00951539`, because
`009512F0` consumes `item` itself through its first `ExpectToken`.

Step 9's `this` is the traffic manager at game `+21D0h`, the owner `docs/GLOBAL_SUBSYSTEMS.md`
records. Loaded as `MOV EAX,[00E188A8] ; MOV ECX,[EAX+21D0h]`.

`00951160` is `vector::clear` with element ownership: it walks `[this+4h]` to `[this+8h]`, and for
each non-null element calls the destructor `0049F910` (`009511AB`) then `_free` (`009511B1`),
then erases the range and stores the new last at `[this+8h]` (`00951209`). The same destructor
`0049F910` is the element destructor of `BSP_ShipClass_ResizeTrafficRecords` (`0082FD20`), so the
scene's traffic records and a ship class's `Traffic` records are the same class.

`00951160` is also called at `0046E9EB` with `ECX = 0xF8A084`, before the entity loop of
`0046DF00`, under the gate at `0046E9D5` that admits passes 2 and 3. So the vector is emptied at
the start of both passes, and the `traffic` block of pass 3 always fills an empty vector.

## `009512F0` — the `item` reader

`__thiscall(vector* this, SceneTokenizer* tok)`, `RET 4` at `00951449`.

| # | site | callee | name | this / args / ret | gate |
| --- | --- | --- | --- | --- | --- |
| 1 | `0095131D` | `008D9930` | `ExpectToken` | push `00D19B80` `"item"` | none |
| 2 | `00951324` | `008D9980` | `BSP_SceneTokenizer_ReadToken` | ECX = tok, char* in EAX | none |
| 3 | `0095132E` | `0041E870` | `BSP_NativeString_Assign` | the item name, into a stack string | none |
| 4 | `00951345` | `008D9930` | `ExpectToken` | push `00CE4D38` `"{"` | none |
| 5 | `00951351` | `008D9930` | `ExpectToken` | push `00D099C0` `"path"` | none |
| 6 | `0095135D` | `008D9930` | `ExpectToken` | push `00CE55BC` `"="` | none |
| 7 | `00951364` | `008D9980` | `ReadToken` | the path value, char* in EAX | none |
| 8 | `0095137E` | `00925A90` | `BSP_EntityRegistry_FindEntityByName` | ECX = `[[00E188A8]+19CCh]`, push the path name | `EAX != 0 && *EAX != 0` (`00951369`, `0095136D`) |
| 9 | `00951385` | `007AC9D0` | `path_entity_cast` | ECX = the found entity, result in EAX | same gate |
| 10 | `00951396` | `00951220` | `traffic_record_create` | ECX = vector, push `&name`, push the path object (or 0 from `0095138C XOR EAX,EAX`); record in EAX | none |
| 11 | `0095139F`/`009513AB` | `008D8A70` + `00438E10` | peek vs `00CE568C` `"properties"` | | none |
| 12 | `009513BB` | `008D9930` | `ExpectToken` | push `00CE568C` | compare == 0 |
| 13 | `009513C6` | `008F41A0` | `BSP_ScenePropertyBag_Construct` | ECX = a stack bag, push 0 | compare == 0 |
| 14 | `009513DC` | `008F5A00` | `BSP_ScenePropertyBag_Parse` | ECX = bag; push tok, 1, 0 (arg1 tok, arg2 = 1, arg3 registry = 0) | compare == 0 |
| 15 | `009513E8` | `0049CF80` | `traffic_record_apply_properties` | ECX = the record from step 10, push `&bag` | compare == 0 |
| 16 | `009513F9` | `008F5410` | `BSP_ScenePropertyBag_Destruct` | ECX = the stack bag | compare == 0 |
| 17 | `00951405` | `008D9930` | `ExpectToken` | push `00CE4CD4` `"}"` | none |

Step 14's third argument is 0, so the library group registry is absent and a parenthesised parent
inside an item's `properties` would not resolve. No installed file authors one.

**The item name is parsed and discarded.** Step 10 pushes `&name` as argument 1, and `00951220`
never reads its argument-1 slot `[entry_ESP+4]`; the full 51-instruction listing reads only
`[entry_ESP+8]`. Nothing else in `009512F0` uses the string either: it is destroyed at
`00951429`/`00951430`. The `"soldierTR01"`-style labels in the shipped files reach no field.

## The traffic record (4Ch bytes)

Producer: `00951220` writes the constructor defaults, `0049CF80` overwrites twelve of them from
the property bag. Both were read in full; the offsets below are from the writers, per
`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 4.

`00951220` is `__thiscall(vector* this, NativeString* name, void* path_entity)`, `RET 8` at
`009512EA`: `operator new(4Ch)` at `0095123C`, base constructor `0049F9B0` at `00951258`, the
field stores below, then `00950FC0(this, &record)` at `009512D3` appends the pointer to the
vector. The record pointer is returned in EAX.

| Offset | Type | Property key | `00951220` default | `0049CF80` default when the key is absent |
| --- | --- | --- | --- | --- |
| `+00h` | `CPathEntity*` | the `path =` value | the resolved entity, or 0 | not written |
| `+04h` | `int32` | `startPt` | `-1` (`00951288`) | `0` |
| `+08h` | `int32` | `endPt` | `-1` (`0095128F`) | `0` |
| `+0Ch` | `bool` | `cyclic` | `0` (`009512B8`) | `0` |
| `+10h` | `int32` | `rowCount` | `0` (`0095129D`) | `0` |
| `+14h` | `int32` | `columns` | `1` (`00951296`) | `1` |
| `+18h` | `float` | `rowGap` | `40.0` (`00CE685C`) | `40.0` |
| `+1Ch` | `float` | `columnGap` | `10.0` (`00CE38B8`) | `10.0` |
| `+20h` | `float` | `HP` (`00CE6750`) | not written by the constructor | `0.0` |
| `+24h` | `float` | `speed` | `5.0` (`00CE3850`) | `5.0` |
| `+28h` | `float` | `randomFactor` | `0.0` | `0.0` |
| `+2Ch` | `float` | `rowDev` | `0.0` | `0.0` |
| `+30h` | `float` | `columnDev` | `0.0` | `0.0` |
| `+34h`..`+4Bh` | class-id keyed float map, 18h bytes | `templates` | built by `0049F9B0` | filled entry by entry |

`+18h`, `+1Ch` and `+24h` are the only non-zero constructor defaults, and `0049CF80` reloads the
same three constants (`DAT_00CE685C`, `DAT_00CE38B8`, `DAT_00CE3850`) as its own absent-key
values, so the two producers agree. `+04h` and `+08h` do **not** agree: the constructor writes
`-1` and the applier writes `0`. Since every installed item has a `properties` section, `-1`
never survives in a shipped scene, but a hand-authored item without one would keep it.

### Numeric coercion

For each of the eight float fields the applier reads the 38h property record's type code at
`+04h`: `== 1` takes the float at `+0Ch` directly, anything else runs `CVTSI2SS` over the same
dword. The five integer and boolean fields take `+0Ch` raw with no type check. Presence is
probed by `0048E9F0` and the record fetched by `BSP_ScenePropertyBag_Find` (`008F2260`), one
probe and one fetch per key.

### `templates`

`0049CF80` fetches `templates` through `008F2260` and requires the record's type code to be `6`
(the sub-block code); anything else is treated as absent. It then iterates the sub-block
(`00480690` builds the iterator over `value+4`, `0047E480` advances, the loop tests
`iterator[1] != 0`). Per child, with `00484D20` returning the child's key:

1. `0048E960(ECX = [00E1867C], "LandVehicleclasses")` fetches the enum table named
   `00CE6710`, and `0048E8D0(table, key)` tests membership.
2. Member: `0048E840(table, key)` yields the class id, and `00964790(ECX = id, DL = 1)`
   (`BSP_VehicleClass_GetOrCreate`) materialises the class.
3. Not a member: the same pair against `"SoldierTypes"` (`00CE6700`), then a `strlen` +
   `BSP_NativeString_Resize` + `004B1400` arm whose callee was not opened
   (`contract: unread`).
4. Either way the child's own value is read as the weight, with the same type-code-`1` float
   test, and `00499030(ECX = record + 34h, &id)` returns the `float*` slot that receives it
   (`0049D364`, `0049D371`).

`ADD EBX,0x34` at `0049D244`, with `EBX = [ESP+14h]` the record, is what fixes the map at `+34h`.

Both arms are exercised by shipped content. The twelve names the installed files use resolve in
`universe\Library\global.enums` (`docs/SCENE_PROPERTY_LIBRARY.md`): the land vehicles in
`enum LandVehicleClasses` (`Us_truck = 464`, `Us_jeep = 466`, `Us_ambulance = 465`,
`Us_apc = 471`, `Jap_jeep = 474`, `Jap_ambulance = 472`, `Jap_tank = 479`) and
`USMarine_shiptraffic = 4` in `enum SoldierTypes`. The two tables are disjoint id spaces, six
small ids against the vehicle-class range, which is why the applier needs two arms and why only
the first reaches `BSP_VehicleClass_GetOrCreate`.

## `004A5620` — the commit

`__fastcall(void* manager)` where `manager = [[00E188A8]+21D0h]`. It walks the same vector
(`DAT_00F8A088` to `DAT_00F8A08C`) and, per record:

| # | site | callee | name | this / args / ret | gate |
| --- | --- | --- | --- | --- | --- |
| 1 | `004A5695` | `00499320` | `traffic_runtime_slot` | ECX = `manager + 10h` (`004A568B LEA ESI,[EBP+10h]`), push `&record`; slot in EAX | none |
| 2 | `004A56A5` | `00BF681B` | `operator_new` | `170h` | `*slot == 0` |
| 3 | `004A56C3` | `004A50D0` | `traffic_runtime_construct` | ECX = the 170h block, push `[manager+8h]`, push the record | allocation non-null |
| 4 | `004A56DD` | `00499320` | same slot accessor | ECX = `manager + 10h`, push `&record`; the constructed object is stored into `*slot` | `*slot == 0` |

So the parsed records are inputs to a per-record runtime spawner of `170h` bytes, keyed by the
record pointer in a map at manager `+10h`. `004A50D0`'s body was not opened
(`contract: unread`), and neither was `[manager+8h]`.

## Why pass 2 stops at `traffic`, and what runs instead

`0046DF00`'s `traffic` arm tests the registration flag first:
`0046EB3B TEST BL,BL ; 0046EB3D JNZ 0046EBDD`. `BL` is `[EBP+1Ch]`, the pass-2 flag
(`docs/SCENE_FILE_READER.md`'s pass table). The registration pass therefore never reaches
`009514B0` at all, and the `traffic` body is not even brace-skipped — the reader leaves the
tokenizer mid-file and falls into its stop epilogue.

`0095CA10`, which `docs/SCENE_FILE_READER.md` recorded as "an STL instantiation the reader calls
to skip it", **does not skip anything**. It is 14 instructions and touches no tokenizer:

```
0095ca10: MOV EAX,[0x00f8a0b0]        ; the list sentinel
0095ca16: MOV ESI,dword ptr [EAX]     ; sentinel->next
0095ca18: CMP ESI,dword ptr [0x00f8a0b0]
0095ca20: MOV ECX,dword ptr [ESI + 0x8]   ; node value
0095ca23: CALL 0x0095c640
0095ca35: MOV ESI,dword ptr [ESI]     ; ->next
```

It drains the intrusive list whose sentinel pointer lives at `00F8A0B0` (nodes are next `+0h`,
prev `+4h`, value `+8h`) into `0095C640`, one preload registration per queued class id.

The only writer of that list is `0095C4D0`, and `0095C4D0`'s only caller is `0046BF70`
`BSP_SceneEntity_RegisterMultiplayerStock` (see `docs/SCENE_BROWSER_GROUPS.md`'s stock section
and `docs/SCENE_ENTITY_FACTORY.md`). So the pass-2 `traffic` keyword is the point where the
multiplayer stock classes the entity pass queued become census entries. `004C8AA0`
`BSP_Scene_ClearPendingClassIds` clears both the list and the census array before pass 2
(`docs/MISSION_SCENE_CONTENTS.md` step 1), so the queue only ever holds this scene's stock.

After `0095CA10` the reader continues at `0046EBE2` into the stop epilogue, which collects
strings into a stack vector and then, at `0046EC6F JNZ 0046ED5D`, walks them: per name it reads a
Lua object under `[00E188A8]`, fetches its `Type` field (`00CE4780`), and unless that type is
`CommandBuilding` (`00CE5870`), `LandFort` (`00CE5864`) or `LandVehicle` (`00CE5858`), resolves
the name through `00418140` and calls `0095C640` with `*EAX` (`0046EEFD`). What fills that string
vector was not established (`contract: unread`).

## `0095C640` — register a vehicle class for preload

`__fastcall(int class_id)`, no stack arguments, `RET` at `0095CA00`-range epilogue. Three call
sites, each verified against its containing function with `ghidra proto`:

| # | site | containing function | ECX | gate |
| --- | --- | --- | --- | --- |
| 1 | `0046D51A` | `0046CF40` `BSP_SceneFile_ReadEntityBlock` | `[008F2260(bag, "Type") + 0Ch]`, the entity's own class id | the six class-id comparisons of `docs/SCENE_ENTITY_FACTORY.md` |
| 2 | `0046EEFD` | `0046DF00` `BSP_SceneFile_Read` | `*(00418140(EBX, EDI))` | the three name rejections above |
| 3 | `0095CA23` | `0095CA10` | `[node+8h]` | none |

Body:

1. `id2 = *(BSP_VehicleClassRegistry_GetSingleton() + 10h + class_id*4)` (`0095C665`,
   `0095C66A`). That is the forward index map `docs/VEHICLE_CLASS_DESCRIPTORS.md` documents as
   `800h` ints keyed by the `Type` enum value, and as the identity for anything not remapped.
   Site 3 therefore maps a value `0095C4D0` had already mapped once; the double mapping is
   harmless for exactly that reason.
2. `if (id2 == 2ACh) return` (`0095C66E`). `2ACh` is the same "no class" sentinel `0046BE90`
   rejects outright.
3. Lua: globals (`00B67980` on `[00E188A8]+1A0Ch`), `VehicleClass` (`00CE5880`), element `id2`,
   field `Type` (`00CE4780`) as a native string.
4. Field `LandingShip` as an integer, default 0; non-zero values are appended by `0095C550`.
5. `0095C550(id2)` appends the class itself.
6. If the `Type` string equals any of `Destroyer`, `Cruiser`, `LandingShip`, `Cargo`,
   `BattleShip`, `Submarine`, `TorpedoBoat`, `MotherShip` (case-insensitive,
   `BSP_NativeString_EqualsCStringInsensitive`), it builds the dotted path
   `"VehicleClass." + <id2> + ".Catapult.LaunchedClass"`, resolves it, reads an integer with
   default `-1`, and appends any non-negative result with `0095C550`.

So a carrier's catapult load-out and a landing ship's carried class are pulled into the census
alongside the class itself.

`0095C550(int id)` is the census append: a linear scan of `[DAT_00F8A0A0, DAT_00F8A0A4)` that
returns on a hit and otherwise grows the vector. The vector's four words are `00F8A09C` (proxy),
`00F8A0A0` (first), `00F8A0A4` (last), `00F8A0A8` (end). `docs/MISSION_SCENE_CONTENTS.md` already
records its two readers, `004C8AA0` and `004D4720`.

## Routine coverage

| Address | Name | Coverage |
| --- | --- | --- |
| `009514B0` | `BSP_SceneFile_ReadTrafficBlock` | complete (listing read in full, 51 instructions) |
| `009512F0` | `BSP_SceneFile_ReadTrafficItem` | complete (listing read in full) |
| `00951220` | `BSP_TrafficRecordVector_CreateRecord` | complete (listing read in full) |
| `00951160` | `BSP_TrafficRecordVector_Clear` | complete |
| `0049CF80` | `BSP_TrafficRecord_ApplyProperties` | complete for the thirteen keys; the `SoldierTypes` arm's `004B1400` is a contract |
| `004A5620` | `BSP_TrafficManager_BuildRuntime` | complete for the walk; `004A50D0` and the `170h` object are contracts |
| `0095CA10` | `BSP_SceneFile_DrainStockClassQueue` | complete |
| `0095C640` | `BSP_VehicleClass_RegisterPreload` | complete |
| `0095C550` | `BSP_VehicleClassCensus_Append` | complete |
| `0095C4D0` | `BSP_VehicleClassStockQueue_Append` | complete |
| `00950FC0`, `0049F9B0`, `0049F910`, `00499030`, `00499320`, `004A50D0`, `004B1400`, `00418140` | - | `contract: unread` |

## Open questions

1. `004A50D0` and the `170h` runtime object are the whole simulation side of traffic; nothing
   here says what spawns or moves.
2. `anims` is authored by all 172 items and read by no code this packet opened.
3. The producer of the string vector `0046DF00`'s stop epilogue walks at `0046ED5D` is unknown,
   so call site 2 of `0095C640` has a gate but no established source.
4. The `SoldierTypes` arm ends in `004B1400`, which was not opened, so what a soldier template
   materialises is unknown where the vehicle arm's `00964790` is documented.
5. `00F8A084`'s vector and `00F8A0B0`'s list are both file-scope statics with no constructor read
   here; `004C8AA0` is their only other writer.
