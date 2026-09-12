# The four spawn bindings and the request record

Addresses: 00944680 0094c480 00946380 00946390 00949750 00945850 00945a20 00949530 00948cc0 004c6ba0 0046d930 00f89b3c 00e0cf74

Packet `cc_lua_core`, worktree `agent/cc-lua-core`. Ghidra was read-only. Every name here is a
hypothesis, not a recovered symbol.

`docs/LUA_BINDING_TABLE.md` listed these four among the twenty-four binding handlers that had no
Ghidra function; the integrator defined and named them at 39F9528F. The reconstruction is
`include/bsp/lua_binding_spawn.hpp` and `src/lua_binding_spawn.cpp`.

They split in two. `Spawn` creates a scene object on the spot and hands the script the entity
table. The other three are a request queue on a separate singleton, and none of them creates
anything.

## The three thunks

`SpawnNew`, `SpawnNewIDIsRequested` and `SpawnNewIDRemove` are three instructions each:

```
0094c480: PUSH ECX                      ; the lua_State the binding received in ECX
0094c481: MOV ECX,dword ptr [0x00f89b3c] ; the spawn manager singleton
0094c487: CALL 0x00949750
0094c48c: RET
```

So the lua_CFunction is `__fastcall(lua_State*)` on the outside and
`__thiscall(manager, lua_State*)` inside, and the callee cleans the one stack argument (`RET`
with no immediate here, `RET 4` in the callee).

| Thunk | Global | Target | Body |
| --- | --- | --- | --- |
| `0094C480` | `SpawnNew` | `00949750` | reads a twelve-field table, queues a record |
| `00946380` | `SpawnNewIDIsRequested` | `00945850` | scans the queue for an id, pushes a boolean |
| `00946390` | `SpawnNewIDRemove` | `00945A20` | removes every record with an id |

## The manager

`*(00F89B3C)`. `include/bsp/world_construct.hpp`'s `kTailConstructions` already records the
producer: the last row is `{0x0000, 0x10, 0x00945820, 0x00941310}` at `004DFA92`, "stored to
DAT_00F89B3C". So the object is 10h bytes with constructor `00945820`, which agrees with the
two fields the three methods touch.

| Offset | Meaning | Evidence |
| --- | --- | --- |
| +4h | list sentinel node | `00945934`, `00945B02` |
| +8h | element count | `00945BAA` `ADD dword ptr [EDI+8],-1` |

Nodes are `{next, prev, record}` at +0h, +4h and +8h, walked by both scan methods.

## The request record

`00949530` allocates DCh bytes through `operator_new`, constructs them with `00948CC0` and
links the node. The layout below is the producer's, `00948CC0`, not a consumer's.

| Offset | Field | Evidence |
| --- | --- | --- |
| +4h / +8h | group-member vector begin and end, 10h-byte elements | `00948EE7` computes `(end - begin) >> 4` |
| +68h, +6Ch, +70h | three scalars | `00948D2C`, `00948D32`, `00948D38` |
| +74h, +78h | two floats | `00948D41`, `00948D47` |
| +7Ch | the request serial | `00948EC9` |
| +80h | one dword | `00948D4D` |
| +84h / +88h | a NativeString, by elimination the `callback` name | `00948D70`, `00948D76` |
| +9Ch..+B7h | seven dwords copied verbatim from a caller block | `00948DB2` |
| +ACh | a float inside that block, clamped to zero-or-greater and used as a radius | `00948F1x` |
| +B8h / +BCh | the request id NativeString | `00948DFE`, `00948E04` |
| +C0h | a byte, always zero | `00948E3B` |
| +C4h, +C8h | two dwords | `00948E42`, `00948E48` |
| +D0h, +D4h, +D8h | zeroed | `00948E5x` |

The id pair is the one field two independent routines agree on. `00945850` reads
`[record+B8h]` at `0094594B` and `00945A20` reads it at `00945B31`, each to match a script's
string, which is what makes B8h/BCh the id rather than a guess. The callback attribution is
provisional: `SpawnNew` reads exactly two strings out of its table, `id` and `callback`, and
`id` is accounted for, but nothing reads +84h back.

## The serial

```
00949f2b: MOV EAX,[0x00e0cf74]
00949f30: CMP EAX,0x3e80          ; 16000
00949f35: JBE 0x00949f3c
00949f37: MOV EAX,0x1
00949f41: MOV ECX,EAX             ; the serial this request gets
00949f44: ADD EAX,0x1
00949f47: MOV [0x00e0cf74],EAX
```

So the sequence runs 1 through 16000 and restarts at 1, 16001 is never issued, and the counter
is a process global rather than a per-mission one. Two missions in one session do not restart it.

## What `SpawnNew` reads

Twelve named fields, in the order `00949750` reads them. Every read goes through an
or-default helper, so a misspelt field gives the default rather than an error.

| Order | Field | Literal | Read at |
| --- | --- | --- | --- |
| 1 | `party` | `00D14754` | `0094981B` |
| 2 | `player` | `00D199D0` | `0094985F` |
| 3 | `callback` | `00CE49C0` | `0094989E` |
| 4 | `excludeRadiusOverride` | `00D199B8` | `00949954` |
| 5 | `id` | `00CF16BC` | `00949972` |
| 6 | `groupMembers` | `00D199A8` | `009499B5` |
| 7 | `CamoColor` | `00D09980` | `00949AD6` |
| 8 | `area` | `00D199A0` | `00949B1A` |
| 9 | `refPos` | `00D19998` | `00949B30` |
| 10 | `angleRange` | `00D1998C` | `00949CC1` |
| 11 | `distRange` | `00D19980` | `00949D50` |
| 12 | `lookAt` | `00D19978` | `00949E2B` |

`id` and `callback` default to the empty string at `00CE3A0C`. `refPos` accepts either an
entity table or a bare position: `00949B51` asks `008889C0` first and takes `00888AA0` on yes,
`00888760` on no. `distRange` and `angleRange` are each two indexed numbers, slot 0 then slot 1;
`angleRange` keeps the defaults 200.0 and 2500.0 from `00D19908`/`00D1990C` when the field is
nil, and its first element is then clamped to at least 10.0 (`00CE38B8`).

## What `Spawn` does

`Spawn` `00944680` is the only one of the four that creates anything, and the only one of the
twenty-four newly defined handlers that returns an entity table.

Arguments, in slot order:

| Slot | Read as | At |
| --- | --- | --- |
| 0 | string, the class name | `00944746` |
| 1 | integer, the placement mode | `009447A0` |
| 2 | entity table or bare position | `009447D4` asks, `0094480A` / `0094487A` take |
| 3, 4, 5 | numbers, the span | `009448CD`, `00944907`, `00944941` |
| 6, 7 | numbers, two angles in degrees | `0094497B`, `009449BE` |
| 8 | string, the instance name, when it is one | `00944AC2` asks, `00944AF8` takes |
| 8 or 9 | boolean, direct placement | `00944B5C`..`00944BB7` |

The trailing boolean's slot is decided from the count and the shape of slot 8: `009449E7` seeds
it with 8, and `00944B4E` raises it to 9 when slot 8 was a string or nil. `00944B61` then reads
it only when the count exceeds it, and `00944B57` defaults it to false.

The two angles are converted from degrees with `FMUL double [00CE3D28]` then
`FDIV double [00CE3D20]`, which are 3.141592653589793 and 180.0 (`00944980`/`00944992` and
`009449C3`/`009449D5`). When the mode is 1 the routine refreshes the origin entity's world pose,
reads entity+ECh and entity+F4h, derives a yaw through the CRT helper at `00BF701A` less
1.5707963267948966 (`00CE3830`) and adds that yaw to both angles. The helper was not identified
beyond its operand shape, so the yaw derivation is provisional; the addition is not.

The creation itself:

| Host method | address | native | Contract |
| --- | --- | --- | --- |
| `entity_refresh_world_pose` | `0094482D` | `00414DB0` | already `BSP_EntityPose_RefreshWorld`; also at `009449F9` and `00944A11` |
| `find_spawn_position` | `00944C44` | `00941FA0` | unread; `009426D0` at `00944C64`, `004BC120` at `00944CC4`, `009426D0` again at `00944CEF` and `00944D36` |
| `place_spawn_directly` | `00944EF6` | `00942CB0` | unread; fed by `00438E10` at `00944EAD` |
| `create_scene_object` | `00944D8A` | `004C6BA0` | body read in full |
| `after_scene_object_created` | `00944D93` | `00874D00` | partial |
| `object_vcall_118` | `00944DDE` | vtable+118h | unread: a virtual with no resolved concrete vtable |
| `object_vcall_11c` | `00944DF2` | vtable+11Ch | unread, same reason |
| `scene_resolve_deferred_references` | `00944E9B` | `0046AAB0` | already `BSP_SceneDatabase_ResolveDeferredReferences` |

`004C6BA0` is eight instructions and is the answer to "which unit creator does it reach":
`__thiscall(*(00E188A8), class name, instance name, 0)`, forwarding to `0046D930` and then
calling `BSP_Game_AssignPartyPlayerSlots(0)` when game+1FE4h is non-zero. `0046D930` is the
scene-database creator that resolves a class by name; `00468660
BSP_SceneDatabase_ClassIdToName` is in its callee set. `004C6BA0` has exactly three callers:
`Spawn`, `GenerateObject` `00944FD0` and `00945450`, so the two spawn-shaped bindings share one
creator. An absent name is passed as the empty NativeString data at `00F89B40`, not as null.

The tail is the entity convention of `docs/LUA_BINDING_ENTITY.md`: `00944DF4` reads the low 16
bits at object+174h, `00944E03` formats the decimal key, `00944E23` takes the `thisTable`
global and `00944E48` pushes `thisTable[key]`. The failure exit at `00944F20` pushes nil
(`00944F24`) and still reports one result.

## Coverage

| Routine | Coverage |
| --- | --- |
| `00944680` `Spawn` | partial: argument marshalling, the mode-1 yaw, the creator call, the entity tail and the failure exit were read. The placement helpers `00941FA0`, `009426D0`, `004BC120` and `00942CB0` were not. |
| `00945850` `SpawnNewIDIsRequested` | complete |
| `00945A20` `SpawnNewIDRemove` | complete for the routine; the record destructor `009442A0` was not read |
| `00949750` `SpawnNew` | partial: the field set, the defaults, the two-element ranges, the serial rule and the enqueue were read. The `groupMembers` element pass (`00945C30`, `00949610`, `008F3710`), the `area` and `lookAt` geometry (`008F84D0`, `008F8530`, `008F8610`, `008F8680`, `008F84A0`, `0085DC80`) and the mapping from the staged floats to the record's scalar block were not. |
| `00949530` / `00948CC0` | complete for the allocation, the link and the field writes |

## Corrections

**`00945A20`'s decompilation drops a reachable block, and the contract changes.**
`python tools/bsp.py ghidra flow 00945a20` reports one gap: after the `_free` at `00945BA2`,
twelve bytes at `00945BA7..00945BB3` are undisassembled.

- was: the pseudocode ends the routine at the first removal (`_free(_Memory); return extraout_EAX;`),
  which reads as "remove the first record with this id"
- is: the routine removes every record with this id and keeps the manager's count in step
- evidence: the disk bytes at `00945BA7` are `ADD ESP,4` / `ADD dword ptr [EDI+8],-1` /
  `JMP 0x945B07`, and `00945B07` is the top of the scan loop; `EDI` is `ECX` from `00945A46`,
  so `[EDI+8]` is the manager's element count

`python tools/ghidra_flow_repair.py 00945a20 --apply` clears it. `00949750` has a three-byte
gap of the same kind at `0094A076` (`ADD ESP,4`), which is cosmetic.

**`Spawn` is the twentieth entity-returning binding.** Recorded in `docs/LUA_BINDING_CORE.md`
under Corrections, because it corrects that document's parent, `docs/LUA_BINDING_TABLE.md`.

## Uncertainties

- No mapping was established from the twelve Lua fields to the record's scalar offsets beyond
  `id` and the serial. `SpawnNew` stages four floats through x87 stores into the outgoing frame
  at `00949F67`..`00949F8C`, and this packet did not follow each store to its slot.
- Nothing in this packet reads the queue back. The consumers of `*(00F89B3C)` are a wide set
  (`00777850`, `0077EC20`, `0066A8A0`, `00673A10`, `00A28A60` and others), so what a queued
  request eventually does is open.
- `00874D00` was read only as far as its first two calls; whether it matters to the spawn is
  not established.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `spawn_request_consumers` | 00f89b3c 00777850 0077ec20 0066a8a0 00673a10 | docs/SPAWN_REQUEST_QUEUE.md | Who drains the queue and what a record's scalar block means to them, which settles the offsets this packet left provisional |
| `spawn_placement_search` | 00941fa0 009426d0 004bc120 00942cb0 | docs/SPAWN_PLACEMENT.md | The four placement helpers `Spawn` uses, and the meaning of its three span arguments |
| `spawn_new_group_members` | 00945c30 00949610 008f3710 | docs/SPAWN_GROUP_MEMBERS.md | The `groupMembers` element pass and the 10h-byte vector element |
| `lua_binding_generate_object` | 00944fd0 004c6ba0 0046d930 | docs/LUA_BINDING_ENTITY.md | `GenerateObject`, the other caller of the shared creator, already known to return an entity |

## no_ghidra_function

None. All four handlers, all three thunk targets and both record routines have Ghidra
functions, confirmed with `python tools/bsp.py ghidra proto <addr> --brief`:

| Address | Body range |
| --- | --- |
| `00944680` | `00944680` - `00944FC0` |
| `0094C480` | `0094C480` - `0094C48C` |
| `00946380` | `00946380` - `0094638C` |
| `00946390` | `00946390` - `0094639C` |
| `00949750` | `00949750` - `0094A13F` |
| `00945850` | `00945850` - `00945A1C` |
| `00945A20` | `00945A20` - `00945C25` |
| `00949530` | `00949530` - `0094960D` |
| `00948CC0` | `00948CC0` - `009492FE` |

## Correction from docs/SCENE_RECORD_MAP.md

Packet `cc2-scene-record-map` (main 91b309c7) proved that the map at `SceneDatabase+18h` which
`0046D930` (now `BSP_SceneDatabase_CreateEntityByName`) resolves names in is the hidden-entity record
map, `std::map<NativeString, SceneHiddenEntityRecord*, LessCaseInsensitive>`, filled by
`BSP_SceneFile_ReadEntityBlock` only for entities authored with the `Hidden` property. So `Spawn` and
`GenerateObject` instantiate an authored hidden entity by name through `004C6BA0`
(`BSP_Game_SpawnEntityByName`), not an arbitrary class, and an unknown name is null into the unguarded
dereference; `004691B0` returns the record for `Spawn`, `0046DC10` is `0046D930` with a placement
override. The `create_scene_object` host method's comment in `include/bsp/lua_binding_spawn.hpp` said
"class name"; the first argument is the hidden entity's name.
