# Property and enum library (`CPropTreeLibrary::Load` `008F67B0`, `008F6FC0`, `008F7100`)

Addresses: `008F67B0`, `008F6FC0`, `008F7100`, `004D3040` call site, `008F2DB0`, `008F2C10`,
`008F29A0`, `008F2B90`, `008F31A0`, `008F4DD0`, `00469B60`, `0048E960`, `0048E840`, `0048E8D0`,
globals `00E18678` and `00E1867C`.

Packet `cc2_scene_traffic_groups`, read-only analysis. `CPropTreeLibrary_Load` is **not** a
hypothesis: the image carries the ASCII literal `CPropTreeLibrary::Load ` at `00D1653C` and
`008F6812` pushes it as the routine's scope marker. Every other name here is a hypothesis.

Continues `docs/GAME_EXECUTABLE.md` milestone 2h section 1, which established the loader's
identity, its tokenizer, its two keywords, its caller's two extensions and its VFS enumeration,
and listed `prop_tree_library` as a follow-up packet. Contracts: `docs/SCENE_PROPERTY_BAG.md`
(the 114h bag), `docs/SCENE_PROPERTY_BAG_MERGE.md` (`008F5A00`'s three arguments and the two
merge stages), `docs/VFS_*` for `00886280`.

## What the library holds

Two registries, both file-scope globals, both filled only by this loader:

| Global | Holds | Key | Entry |
| --- | --- | --- | --- |
| `00E18678` | property groups | group name | a `114h` scene property bag, the same object `docs/SCENE_PROPERTY_BAG.md` describes |
| `00E1867C` | enum tables | table name | a `19Ch` object with the name at `+11Ch` |

`00E18678` is the `CPropTreeLibrary` itself: `004D3040` constructs it at `004D30B3` (`008F5670`),
stores it with `MOV [0x00E18678],EAX` at `004D30CB`, and immediately calls
`008F7100(ECX = EAX, folder = 00CE78E0)` at `004D30D0`. The group map lives at library `+4h`
(`008F6B34 ADD ECX,0x4` before the insert), which is the same `+4h` map offset
`include/bsp/scene_property_bag.hpp` records for a bag, so the library's own storage is a bag.

`00E1867C`'s constructor was not read (`contract: unread`); it is only ever used as the `ECX` of
`008F2C10`, `008F2B90` and `0048E960`.

### On this installation

`universe\Library\` (the literal at `00CE78E0` is `universe\Library\`) holds fifteen files:
fourteen `.props` and one `.enums`.

| Keyword | Count | Examples |
| --- | --- | --- |
| `properties <Name>` | plain groups | `Common`, `MultiEntity`, `Command`, `Foam`, `MotherShipPlanes` |
| `properties <Name>(<Parent>)` | inheriting groups | `Ship(Common)`, `Stationary(Common)`, `Sub(Ship)`, `GameUnit (MultiEntity)` |
| `enum <Name>` | symbol tables | `LandVehicleClasses`, `SoldierTypes`, `ShipClasses`, `Party`, `GameMode` |

So a group is a named bag of class-default properties with an optional single parent, and an
enum table is a named symbol-to-integer map. `global.enums` alone carries the class-id tables the
rest of the game resolves names through: `docs/SCENE_TRAFFIC_BLOCK.md`'s `templates` keys land in
`LandVehicleClasses` (`Us_truck = 464`, `Jap_tank = 479`) and `SoldierTypes`
(`USMarine_shiptraffic = 4`), two disjoint id spaces, which is why `0049CF80` has two arms.

`// osszhangban kell lennie a Path.h-val` next to `enum PathCameraPointType` is an authoring
comment, so the tokenizer's comment handling covers these files as well as `.scn`.

## `008F7100` — the two passes

`__thiscall(CPropTreeLibrary* this, const char* folder)`, `RET 4` at `008F7122`.

| # | site | callee | this / args | gate |
| --- | --- | --- | --- | --- |
| 1 | `008F710E` | `008F6FC0` | ECX = this; push folder, push `00D1655C` `".enums"` | none |
| 2 | `008F711B` | `008F6FC0` | ECX = this; push folder, push `00D16554` `".props"` | none |

`.enums` is loaded first, so every `.props` file's `enum`-typed defaults resolve against tables
that already exist. Neither call has caller stack cleanup, so `008F6FC0` is `RET 8`; the
extension is argument 2 and the folder argument 1 (`008F6FC0` assigns argument 2 first, at its
own entry).

## `008F6FC0` — enumerate one extension

`__thiscall(CPropTreeLibrary* this, const char* folder, const char* extension)`.

| # | site | callee | name | this / args / ret | gate |
| --- | --- | --- | --- | --- | --- |
| 1 | entry | `0041E870` | `BSP_NativeString_Assign` | the extension, then the folder, into two stack strings | none |
| 2 | - | `00886280` | `BSP_Vfs_EnumerateResourceList` | `(out, &folder, &extension, 1)` | none |
| 3 | loop | `00557A90` | `pooled_string_list_next` | `(&list)`, entry in EAX | list non-empty |
| 4 | loop | `008F67B0` | `CPropTreeLibrary_Load` | ECX = this, push `entry+4h` (the name char*, or `00F89450`, the shared empty string, when null) | none |
| 5 | exit | `BSP_PooledStringList_ClearStorage` + `_free` | - | - | none |

The VFS enumeration is the only file discovery; there is no manifest and no fixed file list.

## `008F67B0` — parse one library file

`__thiscall(CPropTreeLibrary* this, const char* path)`, `RET 4` at `008F6B95`. `this` is saved to
`[ESP+28h]` by the entry prologue (`008F67D6 MOV dword ptr [ESP+28h],ECX`) and reused twice: as
`008F5A00`'s third argument and as the base of the group-map insert.

Prologue: the path is turned into a full name (`BSP_NativeString_Assign`, `_Resize`, `_Concat`),
then `operator new(838h)` at `008F68A2` and `BSP_SceneTokenizer_Construct` at `008F68DA` over the
delimiter set `;{}=:()` at `00D16534`. That set differs from the `.scn` set `;{}=:(,)` at
`00CE4F40` by one character: the library tokenizer does **not** treat `,` as a delimiter.
`008F6812` pushes `00D1653C` `CPropTreeLibrary::Load ` as the scope marker for diagnostics.

Top-level loop, until the tokenizer's end flag at `tok+80Ah` is set or the token is empty:

### `enum` branch (`008F693D MOV EDX,0xD1652C`)

| # | site | callee | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| 1 | `008F6944` | `00438E10` | ECX = peeked token, EDX = `00D1652C` `"enum"` | none |
| 2 | `008F6953` | `008D8960` | `ConsumeToken` | match |
| 3 | `008F695A` | `008D9980` | `ReadToken`; the name is copied byte by byte into a 124-byte stack buffer at `[ESP+2Ch]` | match |
| 4 | `008F697C` | `008F2C10` | ECX = `[00E1867C]`, push `&name`; BL = table already present | match |
| 5 | `008F698A` | `008D9930` | `ExpectToken` `00CE4D38` `"{"` | match |
| 6 | `008F699E` | `0048E960` | ECX = `[00E1867C]`, push `&name`; ESI = the existing table | BL != 0 |
| 7 | `008F69AC` | `00BF681B` | `operator new(19Ch)`, then `008F4DD0` constructs it and the name is copied to `+11Ch` | BL == 0 |
| 8 | `008F69FF` | `008F31A0` | parse the table body | none |
| 9 | `008F6A18` | `008F2B90` | register the new table | BL == 0 |

A second `enum` block with a name already present reopens the existing table rather than
replacing it, so declarations across files accumulate.

### `properties` branch (`008F6A27 MOV EDX,0xCE568C`)

| # | site | callee | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| 1 | `008F6A2E` | `00438E10` | ECX = peeked token, EDX = `00CE568C` `"properties"` | enum compare failed |
| 2 | `008F6A3D` | `008D8960` | `ConsumeToken` | match |
| 3 | `008F6A44` | `008D9980` | `ReadToken`, name copied to the same stack buffer | match |
| 4 | `008F6A69` | `008F2DB0` | ECX = `[00E18678]`, push `&name`; BL = group already present | match |
| 5 | `008F6A7F` | `00469B60` | ECX = `[00E18678]`, push `&name`; EDI = the existing bag | BL != 0 |
| 6 | `008F6A8D` | `00BF681B` | `operator new(114h)`; `+0h = 00D16504`, `+4h = 00D162C4`, `+8h = 0`, `40h` dwords from `+0Ch` zeroed by `REP STOSD`, `+10Ch = 0`, `+110h = 0` | BL == 0 |
| 7 | `008F6AD7` | `008F5A00` | ECX = the bag; push tok, push `0`, push `[ESP+28h]` = `this` | none |
| 8 | `008F6B42` | `008F29A0` | ECX = `this + 4h`, push `&name`, push EDI | BL == 0 |

Step 6's field writes match `include/bsp/scene_property_bag.hpp` exactly: `kScenePropertyBagSize`
`0x114`, `kScenePropertyBagMapOffset` `0x04`, `kScenePropertyBagCountOffset` `0x08`,
`kScenePropertyBagBucketsOffset` `0x0C` with `kScenePropertyBagBucketCount` `0x40`,
`kScenePropertyBagOrdinalOffset` `0x10C`, `kScenePropertyBagOwnerOffset` `0x110`. The two
pointers written at `+0h` and `+4h` are `00D16504` and `00D162C4`, the same vtable pair
`008F5A00` and `008F4170` belong to.

**Step 7 is the answer to `docs/SCENE_PROPERTY_BAG_MERGE.md`'s open question.** That doc found
that `008F5A00`'s third argument is a group registry, that `0046D34E` passes 0 so an entity block
can never use the group-merge branch at `008F5AB3`, and that "the `.props` library loader
`008F67B0` is what supplies a registry". It does, and the registry it supplies is the library
object itself. That is how `properties Ship(Common)` resolves `Common`: the parenthesised parent
is looked up in the library that is mid-load, and the parent's bag is merged into the child with
the later-wins flag, since argument 2 here is `0` where an entity block passes `1`.

The loader therefore depends on declaration order within a file for inheritance, and `.enums`
being loaded before `.props` matters only for enum-typed values, not for parents.

## Who consults the library

| Consumer | Site | What it asks for |
| --- | --- | --- |
| `008F5A00` `BSP_ScenePropertyBag_Parse` | its own argument 3 | the parent group of a parenthesised declaration |
| `00469BF0` `BSP_SceneFile_ReadHeaderBlock` | `00469D0E` | `00469B60([00E18678], name)`, a group bag by name |
| `0046CF40` `BSP_SceneFile_ReadEntityBlock` | `0046D2FD` | the same, for each name of an entity's `properties ( ... )` list; stage 1 of `docs/SCENE_PROPERTY_BAG_MERGE.md` |
| `0049CF80` traffic `templates` | `0049D25A`, `0049D2AC` | `0048E960([00E1867C], "LandVehicleClasses" / "SoldierTypes")`, then `0048E8D0` and `0048E840` on the returned table |
| `004CCCC0`, `004DCF90`, `004DC664` | - | lifetime: re-publish and teardown of `00E18678` |

`0048E960(registry, table_name)` is `RET 4` and `0048E840`/`0048E8D0(table, symbol)` are also
`RET 4`; at `0049D24E` the two pushes belong to two different calls, the symbol staying on the
stack across `0048E960` for the `0048E8D0` that follows.

## Routine coverage

| Address | Name | Coverage |
| --- | --- | --- |
| `008F7100` | `BSP_PropTreeLibrary_LoadFolder` | complete (14 instructions) |
| `008F6FC0` | `BSP_PropTreeLibrary_LoadExtension` | complete |
| `008F67B0` | `CPropTreeLibrary_Load` (recovered) | complete for the dispatch, both branches and the registry argument; the file-name prologue was read as pseudocode only |
| `008F2DB0`, `008F2C10`, `008F29A0`, `008F2B90`, `008F31A0`, `008F4DD0`, `008F5670`, `00469B60`, `0048E960`, `0048E840`, `0048E8D0`, `00557A90`, `00886280` | - | `contract: unread` |

## Open questions

1. The `19Ch` enum-table object's layout is unread beyond the name at `+11Ch`; `008F31A0` is
   where the symbol-to-integer pairs are parsed.
2. `00E1867C`'s construction and publication site were not found, so the enum registry's
   lifetime is unknown while `00E18678`'s is in `004D3040` and `004DC664`.
3. `008F67B0` silently ignores any top-level token that is neither `enum` nor `properties`; it
   consumes nothing in that case, so a stray token is an infinite loop. No shipped file has one.
4. Whether a group's parent can be a group declared in a later file was not tested; the loader's
   order dependence says it cannot.
