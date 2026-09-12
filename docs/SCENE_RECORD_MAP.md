# The hidden-entity record map at `SceneDatabase+18h`

Addresses: 00468cd0 004691b0 00469480 0046dc10 0046b340 00468140 00469fc0 00469a20 004693c0
00469300 004693a0 008a9e10 00443d00 0046cf40

Packet `cc2_scene_record_map`. `docs/SCENE_ENTITY_CREATE.md` established that `0046D930` resolves a
name through a map at `SceneDatabase+18h` and reads a record out of it, and listed the record's
layout as partly provisional. This packet reads the **producer** and settles what the map is, what
its key is, what the record holds and who puts records in it.

The answer in one line: the map is the scene database's index of **hidden entities** — entities the
scene file authored with `Hidden` set, which are parsed and recorded but not instantiated, so that
a script can instantiate them later by name. The Lua binding `FindHiddenEntity` (the string at
`00D0FE10`, registered against `008A9E10` at `00E0BFF0`) is the recovered evidence for that name;
every other descriptive name below is a hypothesis.

## The container

`SceneDatabase+18h` is an MSVC `std::map` instantiation. This document names the key and mapped
types and the node offsets; the tree itself is a library contract and is not ported.

```
std::map<NativeString, SceneHiddenEntityRecord*, LessCaseInsensitive>
```

| Offset from `+18h` | Field | Evidence |
| --- | --- | --- |
| `+0h` | `_Myproxy` (checked-iterator container proxy) | never read on any path in this packet |
| `+4h` | `_Myhead`, the sentinel node | `00468CF1` compares the found node with it; `00469496` caches `[this+1Ch]` before the find; `0046B34D` starts the insert descent from `[this+4]` |
| `+8h` | `_Mysize` | not read here |

Node layout, read from the two tree walks (`00468140` lower bound, `0046B340` insert):

| Offset | Field | Evidence |
| --- | --- | --- |
| `+0h` | `_Left` | `0046B39B` takes it when the new key sorts before the node; `00468183` on the same branch |
| `+4h` | `_Parent` | `00468145` reads `_Myhead->_Parent` as the root |
| `+8h` | `_Right` | `0046B39F`, `0046817C` |
| `+0Ch` | `key.length` | `00468155`, `0046B371` |
| `+10h` | `key.data` | `0046816B`, `0046B37E` |
| `+14h` | mapped `SceneHiddenEntityRecord*` | `00469233`, `0046D9D9`, and the equivalent read in `0046DC10` just before `0046DCBA` |
| `+19h` | `_Isnil` | the loop condition at `00468148`/`0046B353` |

The `value_type` handed to `insert` is 12 bytes: the `NativeString` key at `+0h`/`+4h` and the
mapped pointer at `+8h` (`00469A5B` copies `[src+8]` into `[dst+8]`). Because the key sits at
offset 0, `&value == &value.first`, which is why `0046B3FC` can push the value pointer straight into
the comparator.

`00468CD0` returns the iterator **by value through a hidden first argument**: `{container, node}`,
8 bytes, written at `00468D2A`/`00468D2C`, `RET 8`. `0046B340` returns `{container, node, bool}`,
12 bytes, `RET 8`; the bool at `+8h` is 1 on `0046B3DE`/`0046B42C` and 0 on `0046B447`.

## The key and its comparison

The key is a `NativeString`: `{uint32 length, char* data}`, the same bare pair
`docs/SCENE_RECORD_STORAGE.md` documents, with no capacity and no small-buffer area.

The comparison is `00443D00 BSP_NativeString_LessCaseInsensitive` (an existing ledger name, and the
body confirms it):

```
less(a, b):
    if a.length == 0:  return b.length != 0          ; 00443D0D..00443D1A
    if b.length == 0:  return false                  ; 00443D24
    return _stricmp(a.data, b.data) < 0              ; 00443D3D, 00bf7fbf = __stricmp
```

**Lookups are case-insensitive.** `00468140` and `0046B340` inline exactly the same three branches
rather than calling `00443D00`, and both reach `__stricmp`; `00468D01` and `0046B3FF` call it. A
null `data` is never dereferenced because the length branch fires first, so a default-constructed
key compares equal to `""`.

Operand order differs between the two walks and both are consistent with a strict weak ordering:
`00468140` computes `less(node.key, k)` (`00468170` pushes `node->key.data` first), `0046B340`
computes `less(newKey, node.key)` (`0046B385` pushes the new key's data first) and therefore
descends left where the lower bound descends right.

## The record

`operator new(5Ch)` at `0046D68C`, constructed by `004693C0`, destroyed by `00469300` with the
deleting destructor `004693A0` (`RET 4`, `operator delete` at `004693B0` when the flag bit is set).
The single-slot vftable is `00CE5640`; the only slot holds `004693A0`.

| Offset | Field | Writer | Readers |
| --- | --- | --- | --- |
| `+0h` | `void** vftable` = `00CE5640` | `004693DE`, rewritten by the destructor at `0046931E` | none |
| `+4h` | `PropertyBag* properties`, **owned** | `00469405` from ctor a1 | `0046DB57` (clone), `0046DBCC`/`0046DEC1` (wrap), a5 of `0046C550`; deleted through its own vtable at `00469337` |
| `+8h` | `char* name`, owned, `strdup` | `00469411` from `00438E40(a3)` | freed at `00469344`; no creator reads it |
| `+0Ch` | `char* className`, owned, `strdup` | `00469428` from `00438E40(a2)` | `0046D9DC` and `0046DCB6` feed it to the class-map lookup; freed at `00469357` |
| `+10h` | `int party` | `00469421` from ctor a4 | no reader found on any path in this packet |
| `+14h`..`+53h` | `float frame[16]` | `0046942B` `BSP_Matrix_Copy4x4X87` from ctor a5 | a4 of `0046C550` and of the class creator at `0046DB4B`/`0046DE9A` |
| `+54h`/`+58h` | `NativeString parentName` | zeroed at `004693E8`, then `0041DD40` + `memcpy` at `0046943F`/`00469454` from ctor a6 | `0046DA2D`/`0046DA3D` and `0046DD09`/`0046DD19` resolve the parent |

Size `0x5C`. `00438E40 BSP_NativeString_Duplicate` is `strdup` in ECX: it returns 0 for a null
input, otherwise `strlen`, allocate `len+1`, `memcpy`. The destructor frees `+8h` and `+0Ch` with
the CRT `free` at `00BF6989`, which is the ownership proof for both.

### Correction to the provisional layout in `docs/SCENE_ENTITY_CREATE.md`

That document read `+8h`/`+0Ch` as one `NativeString className` with the length at `+8h`, flagged
provisional because `+8h` is never read by `0046D930`. The producer says otherwise: `+8h` and
`+0Ch` are two independent owned `char*` fields, written from two different constructor arguments
and freed separately. `+8h` is the entity's **name**; `+0Ch` is the class name. The consequence for
the reconstruction is only that there is no class-name length field; `include/bsp/scene_entity_create.hpp`
declares no constant for `+8h`, so nothing there is wrong, and that file is not edited from here.

Second correction, same file: `SceneCreateRequest::class_key` names `0046D930`'s first argument a
class key. It is the authored **entity** name, the key of this map, exactly as
`docs/LUA_BINDING_GENERATE_OBJECT.md` line 67 has it. The field is not renamed from this packet
because the file is leased elsewhere.

### Relationship to `docs/SCENE_RECORD_STORAGE.md`

Unrelated objects that share the word "record". That document's record is the **mission** record
built by `004DA2A0` with the scene path at `+90Ch`/`+910h` and script names out past `+980h`; this
one is `0x5C` bytes and describes a single authored entity. The two share only the `NativeString`
representation and the sized storage pool. The `+C24h..` region of the mission record has no
counterpart here.

## What fills the map, and when

`0046CF40 BSP_SceneFile_ReadEntityBlock`, at the tail of a parsed entity block, after the property
bag is complete. Four gates, in order:

| Order | Gate | Site | Meaning |
| --- | --- | --- | --- |
| 1 | `Hidden` property (`00CE5708`) byte at `+0Ch` non-zero | `0046D5EB` read, `0046D5F4` branch | the entity is authored hidden |
| 2 | argument `a6` at `entry+54h` non-zero | `0046D5FA` | the caller enabled hidden recording; `0046EB0F` passes 1 |
| 3 | `0046C550` returns true | `0046D655` | the per-entity generation predicate, called here with `a23 = 1` (`0046D60F`) instead of the 0 every other site passes |
| 4 | `registration` argument at `entry+58h` **zero** | `0046D679` | the instantiate pass only; the registration pass never records |

Gate 2's operand was read as `this+4h` in `docs/SCENE_FILE_READER.md`; the frame arithmetic says
argument `a6`. The prologue is `PUSH -1; PUSH handler; PUSH EAX; SUB ESP,190h; PUSH EBX; PUSH EBP;
PUSH ESI; PUSH EDI`, so ESP is `entry-1ACh` at `0046D5FA` and `[ESP+200h]` is `entry+54h`; the same
anchor makes `[ESP+204h]` at `0046D679` the last argument, and `RET 58h` puts the last argument at
`entry+58h`.

The sequence, once the gates pass:

| # | Site | Callee | Name (hypothesis unless noted) | this / args / ret |
| --- | --- | --- | --- | --- |
| 1 | `0046D674` | `008F2260` | property lookup (existing) | ECX = the bag, arg `00CE5804` `"Party"`; the value is `[EAX+0Ch]` at `0046D681` |
| 2 | `0046D68C` | `00BF681B` | `operator new` (existing) | arg `5Ch`; null tolerated, the record is then 0 (`0046D6FD`) |
| 3 | `0046D6B6` | `*(*parent+10h)` | the parent object's name accessor | ECX = the parent from `entry+0Ch`; `00CE3A0C` substituted when the parent is null (`0046D6BA`) |
| 4 | `0046D6C4` | `0041E870` | `BSP_NativeString_Assign` (existing) | ECX = `&parentName` temp, arg the name from step 3 |
| 5 | `0046D6F4` | `004693C0` | `BSP_SceneHiddenEntityRecord_Construct` | ECX = the block; a1 bag, a2 class name, a3 the 256-byte entity-name buffer at `entry-10Ch`, a4 party, a5 the frame at `entry-164h`, a6 `&parentName`; `RET 18h` |
| 6 | `0046D71B` | `0041E870` | `BSP_NativeString_Assign` (existing) | ECX = an 8-byte slot reserved by `SUB ESP,8` at `0046D6FF`, arg the **same** buffer `entry-10Ch` |
| 7 | `0046D729` | `00469FC0` | `STL_PairNativeStringPtr_Construct` | ECX = the pair, EDX = the record, the key **by value**; `RET 8`; deep-copies the key and releases the caller's buffer at `0046A03E`/`0046A045` |
| 8 | `0046D73E` | `00469A20` | `STL_PairNativeStringPtr_CopyConstruct` | ECX = a second pair, arg the first; `RET 4` |
| 9 | `0046D75F` | `0046B340` | `STL_MapNativeStringPtr_Insert` | ECX = `this+18h` (`0046D750`..`0046D754`), args `&out` and the pair copy; `RET 8` |
| 10 | `0046D77F`/`0046D786`, `0046D7AC`/`0046D7B3`, `0046D7DD`/`0046D7E4` | `00419CC0` / `00BD1510` | pool release (existing) | the pair copy's key, the first pair's key, and the parent-name temp; each gated on a non-null `data` |

**The key is the entity name.** Steps 5 and 6 read the same address, `entry-10Ch`: the 256-byte
stack buffer `008D9980` fills with the quoted entity name at the head of the block, and the same
buffer is handed to `0046C550` as `entityName` at `0046D63C`. So the map key and `record+8h` are
byte-identical by construction, and `record+8h` needs no separate argument about what it holds.

**Duplicates are rejected.** `0046B340` is `std::map::insert(const value_type&)`: when the descent
lands on a node whose key does not compare less than the new key (`0046B3FF`, `0046B406`), it
writes `{existing, false}` at `0046B439`..`0046B44B` and inserts nothing. The first authored entity
with a given name wins; later ones keep their record object, which then leaks, since the caller
never inspects the returned bool.

## What the three other callers do

| Address | Name (hypothesis) | ABI | Behaviour |
| --- | --- | --- | --- |
| `004691B0` | `BSP_SceneDatabase_FindHiddenEntityRecord` | `__thiscall(SceneDatabase*, const char*)`, `RET 4` | builds a temporary key, finds, releases the key, returns `node+14h` or 0 |
| `00469480` | `BSP_SceneDatabase_HasHiddenEntityRecord` | `__thiscall(SceneDatabase*, const char*)`, `RET 4`, result in AL | the same find, but only `SETNZ` on `node != _Myhead` at `004694BE` |
| `0046DC10` | `BSP_SceneDatabase_CreateHiddenEntityAt` | `__thiscall(SceneDatabase*, const char* name, void* second, const float pos[3], float heading)`, `RET 10h` | `0046D930` with a placement override |

`008A9E10 BSP_LuaBinding_FindHiddenEntity` is `00469480`'s only caller: it opens a Lua call frame,
takes argument 0 as a string, calls `00469480` and pushes the boolean. `00944680
BSP_LuaBinding_Spawn` is `004691B0`'s only caller: at `0094477D` it resolves the request's template
name (`00F89B40`, the empty string, when the Lua argument is absent) and stores the **record
pointer** at `[ESP+58h]` in the spawn request it is building. So `Spawn` keeps a record, while
`GenerateObject` keeps an entity.

### `0046DC10` in detail

The same body as `0046D930` up to the creator call, with three differences:

| Site | Difference |
| --- | --- |
| `0046DD54`..`0046DD7E` | after `REP MOVSD` copies `record+14h`..`+53h` into the working frame, the caller's three floats overwrite elements 12, 13, 14 (`+30h`, `+34h`, `+38h`): the translation row is replaced, the basis is kept |
| `0046DD41`..`0046DD9F` | when `heading < 2*pi` (the double at `00CE3828` is `0x401921FB60000000`, `(float)6.2831855` widened), `00467050` rebuilds the frame's 3x3 part from the Euler triple `(0, heading, 0)`; the value is pushed at `0046DD98` between two zeros stored at `0046DD8E` and `0046DD9C`. A heading of a full turn or more leaves the authored basis alone |
| `0046DE82`..`0046DEC1` | no override arm: the record's own bag goes straight into `00922E20`, where `0046D930` clones and merges first |

`0046DC10` also never reads its second argument, matching the open question in
`docs/LUA_BINDING_GENERATE_OBJECT.md`; the argument is consumed by the class creator, which
receives it in EDX at `0046DE98`.

## Host table, one row per native call site

Routine `004691B0`:

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `004691C0` | `0041E870` | (inside `find_record`) | ECX = `&key` at `entry-10h`, arg `name`; `RET 4` | — |
| `004691D4` | `00468CD0` | `map_find` | ECX = `this+18h`, args `&it` at `entry-8`, `&key`; `RET 8` | — |
| `004691EC` | `00419CC0` | `release_temp_key` | args `key.data`, `key.length+1`, `1`; `__cdecl`, args left for the next call | `key.data != 0` (`004691DF`) |
| `004691F3` | `00BD1510` | `release_temp_key` | ECX = the pool | same |
| `00469211` | — | end check | `it.node == [this+1Ch]` -> return 0 | — |
| `00469233` | — | mapped read | `return *(it.node+14h)` | — |

Routine `00469480`:

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `00469491` | `0041E870` | (inside `has_record`) | ECX = `&key` at `entry-10h`, arg `name` | — |
| `004694A8` | `00468CD0` | `map_find` | ECX = `this+18h`, args `&it`, `&key`; `RET 8`, EAX = `&it` | — |
| `004694BE` | — | end check | `SETNZ` on `it.node != [this+1Ch]`, cached at `00469496` | — |
| `004694D7` / `004694DE` | `00419CC0` / `00BD1510` | `release_temp_key` | as above | `key.data != 0` (`004694CA`) |

Routine `0046DC10`:

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `0046DC41` | `0041E870` | (inside `map_find`) | ECX = `&key` at `entry-...`, arg `name` | — |
| `0046DC55` | `00468CD0` | `map_find` | ECX = `EBP+18h`, args `&it`, `&key`; `RET 8` | — |
| `0046DC6D` / `0046DC74` | `00419CC0` / `00BD1510` | `release_temp_key` | as above | `key.data != 0` |
| `0046DCBA` | `0041E870` | (inside `find_class_row`) | ECX = `&key2`, arg `record+0Ch` | record found |
| `0046DCD7` | `00468FB0` | `find_class_row` | ECX = `this+34h`; `classRow = *(ret+8)` at `0046DCDC` | same |
| `0046DCFD` / `0046DD04` | `00419CC0` / `00BD1510` | `release_temp_key` | as above | `key2.data != 0` |
| `0046DD31` | `00925A90` | `find_parent_entity` | ECX = `*(*(00E188A8)+19CCh)`, arg `record+58h` or `00E18560` | `record+54h != 0` (`0046DD0D`) |
| `0046DD9F` | `00467050` | `set_frame_rotation` | ECX = the working frame, args `0.0f`, `heading`, `0.0f` | `heading < 2*pi` (`0046DD87`) |
| `0046DE6B` | `00468660` | `class_id_to_name` | ECX = `*(00E18680)`, arg `*classRow` | — |
| `0046DE75` | `0046C550` | `should_generate` | ECX = `this`; `RET 5Ch`; `AL == 0` -> return 0 | the generation gate |
| `0046DE9A` | `*(classRow+4)` | `create_instance` | ECX = `*classRow`, EDX = the second argument, stack `parent`, `&frame`, `record+4`, `0` | — |
| `0046DEA0` | `00BF681B` | `alloc_bag_ref` | arg `0Ch` | — |
| `0046DEC1` | `00922E20` | `construct_bag_ref` | ECX = the block, arg `record+4`; `RET 4` | `operator new` returned non-null (`0046DEAE`) |
| `0046DEDD` | `00925F20` | `init_all_entities` | CL = 0 | — |

## Lookup paths compared

| Path | Container | Key | Result |
| --- | --- | --- | --- |
| `GenerateObject` -> `004C6BA0` -> `0046D930` | this map | `NativeString` from the Lua string, case-insensitive | a newly created entity, or 0 when the name is absent |
| `GenerateObject` with a placement -> `004C6BE0` -> `0046DC10` | this map | same | the same, placed at the caller's position and heading |
| `Spawn` -> `00944680` | this map, through `004691B0` | same | the **record pointer**, stored in the spawn request |
| `FindHiddenEntity` -> `008A9E10` | this map, through `00469480` | same | a boolean |
| the deferred command queue -> `0046AAB0` | **not this map**: the entity registry at `[[00E188A8]+19CCh]` | the queued target name, through `00925A90` at `0046AB48` | a live entity |

That last row is the distinction worth keeping: queued entity commands resolve **live entities**
after instantiation, while this map resolves **authored records** that were deliberately not
instantiated. `docs/SCENE_DEFERRED_REFS.md` has the queue; the two containers never meet.

## Coverage

| Routine | Coverage |
| --- | --- |
| `00468CD0`, `00468140`, `00469A20` | complete |
| `0046B340` | complete for both outcomes; `0046AD70` (`_Insert`) and `004678F0` (`--iterator`) are contracts |
| `00469FC0` | complete |
| `004691B0`, `00469480` | complete |
| `004693C0`, `00469300`, `004693A0` | complete; `00469362` onward of the destructor (the `+58h` release) was read only as far as the pool call |
| `0046DC10` | partial: the lookup, the class resolution, the placement override, the heading gate, the creator call and the bag wrap were read. The SEH unwind funclets were not |
| `0046CF40` | partial: only the hidden-record branch `0046D5E4`..`0046D7E9` and the gates that reach it |
| `008A9E10` | partial: the argument read and the `00469480` call; the Lua frame plumbing was not |

## Open questions

- The indirect call at `0046D66B` (`EAX = *(local+8)` where the local is at `entry-188h`, ECX = the
  property bag, result discarded) sits between the generation gate and the `Party` read and was not
  identified. It is not on the path that fills the map, but it runs immediately before it.
- Whether nested hidden entities are recorded. The recursive call at `0046D8A9` passes a local at
  `entry-188h` as `a6`, not the caller's `a6`, so gate 2 for a child is not established here. The
  256 shipped `.scn` files were not scanned for a nested `Hidden` entity.
- `record+10h` (party) has no reader in this packet's routines. `0046D930` and `0046DC10` never
  touch it, so whichever consumer reads a hidden entity's party is elsewhere.
- The frame at `record+14h` is the frame the reader hands `0046C550` as `a4` (the same address,
  `entry-164h`), with the parent's world frame travelling separately as the by-value argument.
  Both creators later pass identity for that parent frame, so a nested hidden entity would be
  instantiated at its parent-relative pose. No shipped case was checked.
- No run-time evidence. `bsp_game.exe` reaches the scene reader, but nothing here was confirmed
  against a run log.
