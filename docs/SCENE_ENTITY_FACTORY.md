# Scene entity factory (`0046C550`, the class-id table)

Addresses: 0046c550 004f2800 004ee250 004ee1a0 00468660 00469690 00468fb0 0046bf20 0046bf70 0046be90 0046c450 004693c0

Packet `scene_entity_factory`, worktree `agent/entity-factory`. Ghidra was read-only for this
packet; every name below is a hypothesis, not a recovered symbol.

## What `0046C550` actually is

`docs/SCENE_FILE_READER.md` recorded `0046C550` as "the single instantiation call, reached as
`0046C550(classFactory, name, a3, frame[16], propertyBag)` after `00468660` maps the class id to
its factory". **That is wrong on both counts and this packet supersedes it.**

- `00468660` does not map a class id to a factory. It maps a class **id to its name** (below).
- `0046C550` constructs nothing. It is a **predicate**: given a parsed entity block it answers
  whether that entity should be generated at all in the current game mode, and as a side effect
  builds the deferred `Party` record and registers multiplayer stock.

The construction itself happens in `0046CF40` immediately after `0046C550` returns non-zero, by
calling the class descriptor's own creator (`descriptor[1]` on the instantiate pass,
`descriptor[2]` on the registration pass). The 26-row descriptor table is registered by
`004F2800` and is decoded in full below.

## The class registry

`*(00E18680)` is the scene database, a 0x170-byte object built by `0046F160` and published by
`0046F350` from `BSP_Game_OnInit` at `004E3E60`. Its class map is at `this+34h`: a chained hash
map with 0x40 buckets at `this+3Ch`, iterated by `00466BB0`/`00466C00`. One node is

| Offset | Field |
| --- | --- |
| +0 | key length |
| +4 | `char*` key, the class name |
| +8 | `void*` value, the class descriptor |
| +0Ch | next node in the bucket |

and one descriptor, allocated at `004EE250`, is exactly 0x0C bytes:

| Offset | Field |
| --- | --- |
| +0 | `int` class id |
| +4 | creator, run on the instantiate pass (`0046D5A4`) |
| +8 | creator, run on the registration pass (`0046D57A`, `0046D5E4`) |

Four routines work on that map:

| Address | Recovered role | ABI |
| --- | --- | --- |
| `004EE250` | `Register(name, classId, create, registerFn)` — allocates the 0Ch descriptor and inserts it | `__thiscall(map)`, four stack args, `RET 10h` |
| `004EE1A0` | map insert: find, else allocate a node, copy the key, link into the bucket, bump `this+4` | `__thiscall(map, key, value)` |
| `00468FB0` | name -> node, hashing with `00468A90` and comparing with `00438E10` (case-insensitive) | `__thiscall(map, key, int* outHash)` |
| `00468660` | **class id -> class name**: linear walk, `*(node+8)` first dword equals the id, returns `node+4` | `__thiscall(sceneDb, int id)`, `RET 4` |
| `00469690` | **class name -> class id**: linear walk, `__stricmp` on `node+4`, returns `**(node+8)` | `__thiscall(sceneDb, const char*)`, `RET 4` |

`00468660` and `00469690` both do `ADD ECX,34h` themselves, so they take the scene database, not
the map. `00468660` returns 0 when no row matches, and returns the global at `00E18560` when a
matched row has a null name pointer — `004EE250` never produces one, so that branch is unreachable
in practice. Its initialised bytes read as `Ys`, which is not a class name.

## The class table (`004F2800`)

`004F2800` is the **class registration table**, not a "resolve named objects" routine; its existing
ledger name is superseded below. It makes 26 back-to-back calls of the shape

```
MOV ECX,dword ptr [0x00e18680]
PUSH <registerFn> ; PUSH <create> ; PUSH <classId> ; PUSH <name>
CALL 0x004ee250
```

`instance size` is the `operator new` argument (`00BF55BE`) in the creator's prologue, followed by
a `memset 0` of the same size. `via` says how the creator obtains its instance: **new** allocates a
fixed engine object; **unit** reads `Type` and goes through the unit-class factory `00964790`, then
a virtual at `+28h` of the class object; **typed** reads its own key out of the property bag.

| id | Class | create | register | size | via | key | .scn count |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 07 | `DestroyerGen` | 004F0520 | 004E98E0 | — | unit | `Type` | 9778 |
| 08 | `SubmarineGen` | 004F05F0 | 004E98E0 | — | unit | `Type` | 330 |
| 09 | `MotherShipGen` | 004F0860 | 004E98F0 | — | unit | `Type` | 1089 |
| 0C | `LandingShipGen` | 004F0790 | 004E98E0 | — | unit | `Type` | 390 |
| 0E | `TBoatGen` | 004F06C0 | 004E98E0 | — | unit | `Type` | 652 |
| 18 | `PlaneSquadronGen` | 004F0AD0 | 004E6BC0 | 0x414 | unit | `Type` | 2131 |
| 1A | `LandConvoy` | 004F2700 | 004EE300 | 0x3CC | new | — | 31 |
| 1B | `LandFort` | 004F0FB0 | 004E5BA0 | — | unit | `Stationary`, `Type` | 72615 |
| 1C | `CommandBuilding` | 004F10B0 | 004E5BA0 | — | unit | `Type` | 649 |
| 1D | `LandingPoint` | 004E9D40 | 004E5B00 | 0x224 | new | — | 3582 |
| 34 | `WaterMine` | 004E9F80 | 004E5B00 | — | typed | `Type`, `Party` | 1013 |
| 36 | `Stationary` | 004F0BE0 | 004E5B00 | — | typed | `Type` | 2233 |
| 3B | `Wreck` | 004F0CC0 | 004E5B00 | — | typed | `WreckType` | 0 |
| 3D | `Cloud` | 004E9E40 | 004E9920 | — | typed | `CloudType` | 7231 |
| 41 | `NavPoint` | 004E99B0 | 004E5B00 | 0x1E4 | new | — | 3515 |
| 42 | `MovieCamPos` | 004E9AE0 | 004E5B00 | 0x1E4 | new | — | 78 |
| 43 | `MovieCamLookat` | 004E9C10 | 004E5B00 | 0x1E4 | new | — | 78 |
| 44 | `Landscape` | 004F1460 | 004E5B00 | 0x430 | new | — | 763 |
| 45 | `AirField` | 004F0930 | 004E9900 | — | unit | `Type` | 256 |
| 46 | `Shipyard` | 004F0A00 | 004E9910 | — | unit | `Type` | 243 |
| 47 | `Path` | 004EA650 | 004E5B00 | 0x210 | new | — | 25292 |
| 4A | `CameraPath` | 004EA760 | 004E5B00 | 0x210 | new | — | 0 |
| 4D | `SpawnPoint` | 004F1A60 | 004EA080 | 0x348 | new | — | 1704 |
| 5B | `SimpleEffect` | 004F0DB0 | 004E5B00 | 0x1EC | new | — | 11 |
| 5C | `PeriodicEffect` | 004F0EB0 | 004E5B00 | 0x228 | new | — | 0 |
| 5D | `FreeCamPos` | 004E5BB0 | 004E5B00 | — | typed | — | 0 |

`004E5B00` is the shared no-op registration function used by 15 classes. The `.scn count` column is
this packet's own lexical scan of the 259 installed files (below); `LandFort` includes the three
`Landfort` spellings, which resolve because `00468FB0` and `00469690` compare case-insensitively.

**The seven ids `docs/SCENE_FILE_READER.md` left unidentified are now named:** `44` = `Landscape`,
`47` = `Path`, `1B` = `LandFort`, `1C` = `CommandBuilding`, `34` = `WaterMine`, `4D` = `SpawnPoint`.
The seventh, **`19`, is not a registered class id at all**. `004EE250` has exactly one caller, so
there is no other registration site in the binary, and the `CMP EAX,19h` at `0046D492` inside the
registration pass can never match. Treat it as a leftover of a class that was removed before
release, between `PlaneSquadronGen` (18h) and `LandConvoy` (1Ah).

Only the fixed-size classes have an instance size; the nine `unit` classes are data-driven
spawners whose instance comes from the unit-class registry and whose size therefore depends on the
`Type` value, not on the scene class. Vtables were only read directly out of three creators
(`NavPoint` `00CE8550`, `MovieCamPos` `00CE86D8`, `MovieCamLookat` `00CE8860`); the rest are set
inside a constructor the creator calls, and were not chased — the per-class bodies are out of scope
for this packet.

### No unknown-class handling

`0046CF40` does `piVar1 = *(int **)(node + 8)` on the result of `00468FB0` with no null check
(`0046D0xx`). A class token that is not in the table dereferences address 8 and faults. There is no
skip, no default class and no diagnostic. Every class token in the 259 shipped files is one of the
22 names above, so the path is never taken in shipped content.

## `0046C550` in full

`__thiscall`, `RET 5Ch`. `ECX` is the scene database (`this+164h` and `this+24h` are used), which is
why `0046D421` loads it from a local and not from the descriptor.

```
0046C550(this = SceneDatabase*,
         a1  = const char* className,     // 00468660(descriptor->classId)
         a2  = const char* entityName,    // the quoted name
         a3  = Entity* parent,            // null at the top level
         a4  = const float localFrame[16],
         a5  = PropertyBag* properties,
         a6  = <not read on any path>,
         a7..a22 = float parentFrame[16] by value,
         a23 = void*& outDeferredRecord)  // the caller always passes 0
```

Return is `AL`: non-zero means the caller may construct the entity.

1. `0046C57A` takes `X = a4[+30h]` and `Z = a4[+38h]`, the translation of the entity's own frame.
2. `0046C58C` looks up `MultiType` (`00CE580C`) in the property bag through `008F2260`.
   **Absent** takes the deferred-record branch; **present** takes the game-mode gate.
3. Deferred-record branch (`0046C5AC..0046C695`): read `Party` (`00CE5804`), take the parent's name
   through its vtable slot `+10h` (or `""` at `00CE3A0C` with no parent), then, when `a23` is still
   null, `operator new(5Ch)`, fill it through `008F41F0` into `004693C0` with
   `(className, entityName, party, localFrame, parentFrame)`, store it into `a23` and append it to
   the list at `this+24h` via `0046C450`. **Returns 1 unconditionally.**
4. Game-mode gate: `MultiType`'s value is the nested sub-bag, and the argument slot `a5` is
   overwritten with it at `0046C6AE`, so the per-mode keys are read out of the sub-bag while
   `Party`, `GenerateInGame` and `GenerateInEngineMovie` stay entity-level.
5. World position (`0046C6B7`): with a parent, refresh it through `00414DB0` when byte `+C8h` is
   clear, then add float `+FCh` to `X` and `+104h` to `Z`. Without a parent, `00413920` multiplies
   the local frame by `parentFrame` and `X`/`Z` come from `+30h`/`+38h` of the product.
6. `0046C716` sends `className` back through `00469690` and `0046C741` searches the `std::set` at
   `this+164h` with `00468DB0`. **A hit returns 1 immediately**, bypassing the area test.
7. `004BCA50` (`BSP_Game_GetEffectiveGameMode`) selects one of 11 cases through the jump table at
   `0046CCE4`; anything above 10 returns 0.

### The game-mode cases

The play-area rows are six floats at `*(00E188A8)+705Ch` with an 18h stride. Only four are read:
`f[0] < X < f[3]` and `f[5] < Z < f[2]`. Every compare is `FCOMIP`/`JBE`, so all four bounds are
strict and a point exactly on an edge is rejected. The Z pair is stored high first. The slot order
in the array is **not** the mode order.

| Mode | Case at | Slot | Row offset | Behaviour |
| --- | --- | --- | --- | --- |
| 0 | 0046C95E | 0 | +705Ch | inside the area -> `MultiType.MultiIslandCapture1v1`, else 0 |
| 1 | 0046C9D5 | 1 | +7074h | -> `MultiIslandCapture2v2` |
| 2 | 0046CA4C | 2 | +708Ch | -> `MultiIslandCapture3v3` |
| 3 | 0046CAC3 | 3 | +70A4h | -> `MultiIslandCapture4v4` |
| 4 | 0046C870 | 6 | +70ECh | -> `MultiDuel` |
| 5 | 0046C8E7 | 7 | +7104h | -> `MultiEscort` |
| 6 | 0046C782 | 4 | +70BCh | -> `MultiSiege` |
| 7 | 0046C7F9 | 5 | +70D4h | -> `MultiCompetitive` |
| 8 | 0046CC5B | — | — | `GenerateInGame` set -> `0046BF20` stock registration, return 1; else 0 |
| 9 | 0046CB43 | — | — | `GenerateInGame` set -> build the deferred record, `0046BF20`, return 1; else `GenerateInEngineMovie` |
| 10 | 0046CCCA | — | — | return 1 unconditionally |

The returned byte is the property's value at `+0Ch`, read without a null check, so a scene whose
`MultiType` block omits the current mode's key would fault. Every `MultiType` block in the shipped
files that reaches a mode's case carries that mode's key.

## What the reader does with the answer (`0046CF40`)

| Pass | After `0046C550` returns non-zero |
| --- | --- |
| Instantiate | `descriptor[1](ECX = classId, EDX = &frameBlock, bag, ...)` creates the instance; `*(instance+0C0h)` then takes an object built at `0046D5B0` from a 0Ch allocation through `00922E20` |
| Registration | six `CMP` against `47h`, `19h`, `1Bh`, `1Ch`, `34h`, `4Dh` gate `008F2260` + `0095C640` + `0046BF70` + `descriptor[2]`; the fallback runs `descriptor[2]` for `4Dh` and `44h` |

Each of those six comparisons is written as `00469690(this, 00468660(*(00E18680), descriptor->id))`
— an id round-trip through the name. Both sides are the same scene database, so it is an identity
and the constants are plain class ids.

The creator's ABI, read at the `0046D59D` call site: `ECX` = the class id, `EDX` = a pointer to the
frame block, then four stack arguments of which the first is the property bag. It is not
`__thiscall`; `ECX` carries data, not an object.

`0046BF20` -> `0046BF70` is the multiplayer stock walk: for `AirField`/`MotherShipGen` it iterates
`PlaneStock %d` and reads `Type`, `Count` and `Hidden`, dispatching to `0095C4D0` or `0046BE90`;
for `SpawnPoint`/`Shipyard` it reads `NumSlots`, `Slot %d`, `Stock %d`, `AlliedList` and
`JapanList`. `0046BE90` rejects the id `2ACh` outright and otherwise compares the class name to
`SpawnPoint`; `2ACh` is not a scene class id, it is a `Type` (unit-class) value.

## Installed-file evidence

A lexical scan of all 259 `.scn` files under
`I:/SteamLibrary/steamapps/common/Battlestations Pacific` (`local/scn_scan2.py`, matching
`"<name>" ( <Class> )` and `"MultiType" {`):

| Measure | Value |
| --- | --- |
| files | 259 |
| entity headers | 133664 |
| distinct class tokens | 23 (22 classes; `Landfort` is `LandFort` re-spelled) |
| entities carrying a `MultiType` block | 126098 (94.3%) |
| files carrying at least one | 253 |

So the game-mode gate is the common path and the deferred-record branch covers the remaining 7566.
The split is per class: `NavPoint`, `Stationary`, `MovieCamPos`, `MovieCamLookat`, `SimpleEffect`
and `Landfort` never carry `MultiType` at all, while `Cloud`, `LandingPoint`, `WaterMine`,
`CommandBuilding` and `LandConvoy` always do.

| Class | entities | with `MultiType` |
| --- | --- | --- |
| LandFort | 72612 | 72455 |
| Path | 25292 | 24964 |
| DestroyerGen | 9778 | 9123 |
| Cloud | 7231 | 7231 |
| LandingPoint | 3582 | 3582 |
| NavPoint | 3515 | 0 |
| Stationary | 2233 | 0 |
| PlaneSquadronGen | 2131 | 1846 |
| SpawnPoint | 1704 | 1697 |
| MotherShipGen | 1089 | 1060 |
| WaterMine | 1013 | 1013 |
| Landscape | 763 | 749 |
| TBoatGen | 652 | 626 |
| CommandBuilding | 649 | 649 |
| LandingShipGen | 390 | 289 |
| SubmarineGen | 330 | 302 |
| AirField | 256 | 245 |
| Shipyard | 243 | 236 |
| MovieCamPos | 78 | 0 |
| MovieCamLookat | 78 | 0 |
| LandConvoy | 31 | 31 |
| SimpleEffect | 11 | 0 |
| Landfort | 3 | 0 |

`Wreck`, `CameraPath`, `PeriodicEffect` and `FreeCamPos` are registered but never authored.

These totals sit 2183 above the per-class totals in `reports/scene_file_reader.json` (133655 total,
131481 across classes), because this scan is lexical and also counts the entity headers inside the
regions where the earlier packet's recovering parser bailed out in the nine non-conforming files.
The entity-header total agrees to within nine.

`MultiType` is always authored as a nested block, `"MultiType" { MultiDuel = B false ; ... }`, never
as a scalar, which is why a key-level grep for it finds nothing.

## Reconstruction

`include/bsp/scene_entity_factory.hpp` and `src/scene_entity_factory.cpp` hold the class table as
data (id, name, both creator addresses, instance size, creator kind, type key), the lookups in both
directions, the registration-pass id sets, the mode-to-slot and mode-to-key tables, the strict
area test, and `scene_entity_generation_gate_0046c550` over a `SceneEntityGateHost` with one method
per native call site. The parsed entity block comes in as `bsp::ScenePropertyBlock` from
`include/bsp/scene_file.hpp`; no type there is duplicated. The `std::set` at `this+164h` is an
input rather than a table because what fills it was not recovered.

Build-tested under `scripts/build.ps1` (Win32, `/W4 /WX`). One case was added to
`tests/math_tests.cpp` for the two properties that decide whether an entity dispatches: the
case-insensitive round trip that makes `Landfort` resolve, and the absence of any unknown-class
handling.

## State reached

| Routine | State |
| --- | --- |
| `0046C550` | exported, analyzed, reconstructed, build-tested, installed-file-checked |
| `004F2800` | analyzed (disassembled in full), reconstructed as data, installed-file-checked |
| `004EE250`, `004EE1A0` | exported, analyzed |
| `00468660`, `00469690`, `00468FB0` | exported, analyzed |
| `0046BF70`, `0046BE90` | exported, analyzed (not reconstructed; contract only) |
| `0046BF20`, `0046C450`, `004693C0` | analyzed as call sites only |
| the 26 creators | identified (address, size, creator kind, type key); bodies not analyzed |

## Uncertainties and what remains

- The set at `this+164h` that exempts a class from the area test: read-only evidence shows the
  `std::set<int>` and the `00468DB0` find, but nothing was found that inserts into it. Until that
  is recovered the exemption list is unknown, so the gate's behaviour for any given class cannot be
  predicted from the binary alone.
- Argument `a6` of `0046C550` is not referenced on any path that was read. It is pushed from `EDX`
  at `0046D3E7` and may be dead.
- The six floats of a play-area row: indices 1 and 4 are never read. `{minX, minY, minZ, maxX,
  maxY, maxZ}` fits the reads only if the Z pair is authored inverted, so the field names are a
  guess and only the four read slots are asserted.
- `008F41F0` and `004693C0` build the 5Ch deferred record; its layout was not decoded beyond the
  five inputs. `docs/SCENE_FILE_READER.md` saw the same size at `0046D68A`.
- The three vtables above are the only ones read; the other 23 classes set theirs inside a
  constructor that was not opened.
- Whether `EDI` at `0046D467` is literally `*(00E18680)` was inferred from the identical
  `ADD ECX,34h` in both helpers and from five of the six compared constants matching registered
  ids, not proved by tracking the slot. Provisional.
- `0046C550` is reached from three more sites (`0046D655`, `0046DB27`, `0046DE75`) whose argument
  setup was not read; only the `0046D426` site was decoded.

No `no_ghidra_function` cases: every routine named here already has a Ghidra function. No
fall-through gap after a `_free` call was seen in any of them.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `scene_class_exempt_set` | 0046c550+0x1c4 00468db0 004682b0 0046f160 | docs/SCENE_CLASS_EXEMPT_SET.md | What inserts into the `std::set` at scene database `+164h`, and the sibling containers at `+18h`, `+158h` |
| `scene_unit_class_factory` | 00964790 004f0520 004f0860 004f0fb0 | docs/SCENE_UNIT_CLASS_FACTORY.md | The `Type` value to unit-class path the nine data-driven scene classes share, and the virtual at `+28h` |
| `scene_fixed_entity_classes` | 004e99b0 004ea650 004f1a60 004f1460 | docs/SCENE_FIXED_ENTITY_CLASSES.md | The 12 fixed-size creators: constructor, vtable, and which `localframe` and property fields each stores |
| `scene_multiplayer_stock` | 0046bf20 0046bf70 0046be90 0095c4d0 0095c640 | docs/SCENE_MULTIPLAYER_STOCK.md | `PlaneStock %d` / `Slot %d` / `Stock %d` / `AlliedList` / `JapanList` and the `2ACh` unit-class id |
| `scene_deferred_entity_record` | 008f41f0 004693c0 0046c450 0046a9f0 | docs/SCENE_DEFERRED_ENTITY_RECORD.md | The 5Ch record's layout and how the list at scene database `+24h` is drained |

## Corrections from docs/SCENE_UNIT_CREATORS.md

The `via unit` classes do not have a `Type`-dependent instance size: there are two objects. `00964790` returns a vehicle-class descriptor (0x138 to 0x870 bytes, one per row of the installed `VehicleClass` Lua table, cached by class index), and the descriptor's vtable slot +28h allocates the unit instance (0x1188 bytes for `MDestroyer`, the one genuine symbol, ASCII at `00d1ad28`). All ten unit creators take the class id in ECX and never read it, and `RET 10h` pops a fourth stack argument none of them reads; six of the eight siblings are instruction-for-instruction the DestroyerGen exemplar, `LandFort` splits on `Stationary`, and `PlaneSquadronGen` allocates its own 0x414 object and calls `00964790` only for the cache side effect. The parent node in the placement call is `*(*(00e188a8)+19CCh)`, not the creator's parent argument, and world registration is `006fe620` pushing the instance onto five intrusive lists of that node.
