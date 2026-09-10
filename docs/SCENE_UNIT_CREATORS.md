# Scene unit creators (the ten `Type`-keyed scene classes)

Addresses: 004f0520, 004f05f0, 004f06c0, 004f0790, 004f0860, 004f0930, 004f0a00, 004f0ad0,
004f0fb0, 004f10b0, 004e96d0, 004e98e0, 004e98f0, 004e9900, 004e9910, 004e6bc0, 004e6b30,
004f03c0, 00964790, 0095ba60, 00470b80, 006fe590, 009553d0, 00469610

`docs/SCENE_ENTITY_FACTORY.md` recorded ten of the 26 scene classes as `via unit`: the creator
reads the `Type` property and goes through the unit-class factory `00964790`. This packet recovers
that shared path. Every name below is a hypothesis, not a recovered symbol; the one class name that
*is* recovered from the image is `MDestroyer`, read out of the data that follows the class
descriptor's vtable at `00D1AD28`.

The headline correction to the entity-factory doc: `via unit` does **not** mean "the instance size
depends on `Type`" in the sense of one allocation. There are two objects. `00964790` returns a
**vehicle-class descriptor** (0x138..0x870 bytes, one per `VehicleClass` row, cached), and the
descriptor's vtable slot `+28h` allocates the **unit instance** (0x1188 bytes for `MDestroyer`).
The scene creator only sees the second one.

## Calling convention of a unit creator

Ghidra drops the stack arguments (the creators are unprototyped), so the signature comes from the
listing. The call site is `0046D5A4` inside `0046CF40`:

```
0046D58C: PUSH EAX        ; [ESP+1B8h]  -> stack arg 4
0046D591: PUSH EBX        ;             -> stack arg 3, the property bag
0046D596: PUSH ECX        ; LEA [ESP+50h] -> stack arg 2, the 16-float local frame
0046D59C: PUSH EDX        ; [ESP+1B4h]  -> stack arg 1, the hierarchy parent (may be null)
0046D597: MOV ECX,[EAX]   ; ECX = descriptor+0, the class id
0046D599: MOV EAX,[EAX+4] ; the creator
0046D59D: LEA EDX,[ESP+B0h] ; EDX = the entity's quoted name, a C string
0046D5A4: CALL EAX
```

so every one of the ten is

```
UnitInstance* __fastcall create(int classId /*ECX*/, const char* entityName /*EDX*/,
                                void* hierarchyParent, const float localFrame[16],
                                PropertyBag* properties, void* unused);   // RET 10h
```

`ECX` (the class id) is dead in all ten bodies; the register is loaded by the caller for the
creators that are not unit creators. The fourth stack argument is never read by any of the ten
either, but `RET 10h` still pops it. Verified per creator by the frame arithmetic: `004F0522
MOV EBP,[ESP+14h]` after two pushes is `entry+0Ch`, the third stack argument, and `004F10B1
MOV EBX,[ESP+10h]` after one push is the same slot.

## The exemplar, `004F0520` (`DestroyerGen`, 9778 authored instances)

1. `008F2260(bag, "Type")` (`00CE4780`), value dword at `+0Ch`.
2. `00964790(ECX = Type, DL = 1)` -> the vehicle-class descriptor.
3. `descriptor->vtable[28h](0)` -> the unit instance. The argument is a literal `0` in all ten.
4. `004C1130()` is the frame job pool (`docs/GAME_RENDER_FRAME.md`); its base subobject at `+4`
   answers `vtable[10h]()` with a byte. **Zero** takes the placement path, **non-zero** the
   deferred path.
   - placement: `instance->vtable[98h](hierarchyParent, *(*(00E188A8)+19CCh), localFrame)`.
     The middle argument, not the creator's own first argument, is the parent scene node.
   - deferred: `004F03C0(bag, hierarchyParent, localFrame)`.
5. The entity name is copied into the native string at **instance+154h**: `0041DD40(resize)` with
   `strlen(name)`, then `memcpy([+158h], name, [+154h])`. `+154h` is the length and `+158h` the
   buffer, so a null name resizes to 0 and copies nothing.
6. `004E6B30(instance, bag)`, the `Command` property.
7. Returns the instance in `EAX`.

Which branch of step 4 the shipped game takes was not established. The predicate is a thread-pool
query on the frame job pool, and the deferred branch writes `SetHierarchy`, `HierarchyParent` and
`HierarchyMatrix` back into the property bag instead of touching the scene graph, which is what an
off-main-thread load would need. Treat "non-zero means the load is running on a job thread" as
provisional.

### `004F03C0`, the deferred hierarchy write (`__fastcall(bag, parent)`, one stack arg, `RET 4`)

Erases `SetHierarchy` (`00CEA028`), `HierarchyParent` (`00CEA018`) and `HierarchyMatrix`
(`00CEA008`) from the bag through `004F01F0`, then writes `SetHierarchy = 1` (`008F3940`); with a
non-null parent, `HierarchyParent = <parent name>` (`00926420` then `+4`, or the global at
`00E18B5D` when the name is empty) through `008F3AC0`; and finally pushes the 16 floats of the
local frame onto the stack with `REP MOVSD ECX=10h` and writes `HierarchyMatrix` through
`008F3880`. Nothing is placed in the scene graph.

### `004E6B30`, the `Command` property (`__fastcall(instance, bag)`, `RET 0`)

Looks up `Command` (`00CE8A08`) in the entity bag and requires the descriptor's type code at `+4`
to be **6**, the nested-block code. Its `+0Ch` is the sub-bag. Inside it, `Command` and
`CommandTarget` (`00CE89F8`) are looked up; if `Command` is present, the target string is taken
through `008F0DF0` (or `""` at `00CE3A0C` when `CommandTarget` is missing or empty) and

```
00469610(sceneDb = *(00E18680), instance, commandName, targetName)
```

allocates a record (`00BF681B`) and appends it through `004690D0`. This is the
`"Command" { CommandTarget = R "" ; }` sub-block the shipped `.scn` files carry on every unit.
`004F0FB0` (`LandFort`) and `004F10B0` (`CommandBuilding`) are the only two of the ten that do not
call it.

## `Type` resolution

`Type` is authored as an enum: `Type = E ShipClasses : PACK3_Pennsylvania ;` on a `DestroyerGen`,
`Type = E PlaneClasses : Warhawk ;` on a `PlaneSquadronGen`. The property descriptor declares the
type and the reader stores the resolved integer at `+0Ch`
(`docs/SCENE_FILE_READER.md`: the letter is validated, not dispatched on). That integer is a
**`VehicleClass` row index**.

`00964790` is `__fastcall(int typeId /*ECX*/, bool readRace /*DL*/)`. The singleton behind
`00437F50` holds three things it uses:

| Offset | Field |
| --- | --- |
| `+4` / `+8` | data pointer and capacity of the descriptor cache, indexed by class index; grown by `00437BD0` |
| `+10h` | `int typeToClassIndex[]`, indexed by the `Type` value; `+4010h - +10h` = 0x4000, so 4096 entries |
| `+4010h` | the per-party required-class array used by the registration pass, stride 3 |

1. `classIndex = singleton[+10h + typeId*4]`.
2. If `cache[classIndex] != 0`, return it. The test is `NEG EAX ; SBB EAX,EAX ; TEST EAX,0F8A0BDh ;
   JNZ` at `009647F9`, which is `cached != 0` with a junk immediate; there is no second condition.
3. Otherwise read the Lua globals: `VehicleClass` (`00CE5880`), index `classIndex`, field `Type`
   (the same literal `00CE4780` the scene bag uses) as a **string**, and `LandingShip` as an
   integer; a non-zero `LandingShip` recurses into `00964790` for that row first. With `DL != 0` it
   also reads `Race` (`00CE8EE0`, integer, default 0).
4. The `Type` string is compared with `00425850` against 22 literals in order and the matching
   branch allocates and constructs the descriptor (table below).
5. The descriptor is stored into `cache[classIndex]`, `vtable[10h]` and `vtable[14h]` run around a
   recursion guard on `00F8A098`, and the descriptor is returned.

The Lua table is `Scripts/datatables/autoload/vehicleclasses.lua` in the installed game, 191122
lines, **633 rows** with indices from 1 to 999 (366 indices in that span are unused). Every row
carries `Type`; the 21 distinct values present are exactly the factory's literals minus
`ReconPlane`, which has a branch but no shipped row.

| `VehicleClass.Type` | rows | descriptor size | constructor | notes |
| --- | --- | --- | --- | --- |
| `Destroyer` | 31 | 0x808 | `00963380` inline | vtable `00D1ACF8`, second vptr at `+6Ch` = `00D1ACF4`; class name `MDestroyer` at `00D1AD28` |
| `Cruiser` | 41 | 0x80C | inline | vtable `00D1AD38`, `+6Ch` = `00D1AD34` |
| `LandingShip` | 6 | 0x81C | `00963C20` | |
| `Cargo` | 13 | 0x80C | `00963CB0` | |
| `BattleShip` | 39 | 0x80C | `00963D30` | |
| `Submarine` | 9 | 0x840 | `00963DB0` | |
| `TorpedoBoat` | 3 | 0x814 | `00963E40` | |
| `MotherShip` | 18 | 0x870 | `00963EC0` | |
| `ReconPlane` | 0 | 0x60C | `00951AA0` | no shipped row |
| `SmallReconPlane` | 5 | 0x60C | `009536E0` | |
| `LargeReconPlane` | 4 | 0x60C | `00953770` | |
| `Fighter` | 28 | 0x60C | `00951AF0` | |
| `DiveBomber` | 9 | 0x60C | `00951B40` | |
| `TorpedoBomber` | 9 | 0x60C | `00951C20` | |
| `Kamikaze` | 6 | 0x60C | `00951D00` | |
| `LevelBomber` | 11 | 0x60C | `00951DE0` | |
| `AirField` | 2 | 0x140 | `0095FEE0` | |
| `Shipyard` | 2 | 0x138 | `0095FF50` | |
| `LandVehicle` | 21 | 0x150 | `0095FFC0` | |
| `LandFort` | 368 | 0x180 | `00749180` | |
| `CommandBuilding` | 7 | 0x1AC | `00951E30` | |
| `DummyTargetVehicle` | 1 | 0x144 | `00960050` | |

Sizes come from `MOV ECX,<size> ; CALL 00470B80` (`00470B80` is `operator new` plus a `memset 0` of
the same size, `__fastcall(size /*ECX*/)`), except the first three, which the compiler inlined as
`PUSH <size> ; CALL 00BF55BE`. The name-to-branch pairing is confirmed twice: by address order of
the branch bodies, and by the SEH state byte, which runs `0Ah` for `Destroyer` up to `1Fh` for
`DummyTargetVehicle`.

Row counts are from `local/vehicleclass_scan.py` over the installed file.

### From descriptor to instance: `descriptor->vtable[28h]`

For `MDestroyer` the slot is `006FE590`, `__thiscall(descriptor, int flag)`, `RET 4`:

```
operator new(1188h) + memset 0
006FE460(instance, flag)          ; the instance constructor; sets *instance = 00CFC3D0
009553D0(instance, descriptor)    ; refcounted setter, instance+538h
instance+354h = descriptor        ; a second, non-owning back-pointer
return instance
```

So a `Destroyer`-typed `DestroyerGen` entity produces a **0x1188-byte** unit instance whose vtable
is `00CFC3D0`. The name string at `+154h` and the two class pointers at `+354h` / `+538h` are the
only fields this packet established.

`instance->vtable[98h]` is `00CFC468` = `006DFE40`, a five-byte `JMP 00928860`. `00928860` is
`__thiscall(instance, arg1, newParent, matrix)`: it addrefs the object behind `00928240()+4`,
compares `newParent` with the current parent at `instance+30h`, calls `vtable[134h]` on the
instance when it is leaving an old parent, runs `009258F0(instance, arg1, newParent, matrix)` and
calls `vtable[130h]` when it has gained one. For this instance `vtable[130h]` is `006FE620`, which
is the world registration: it calls `00928560` and then pushes the instance onto **five** intrusive
lists of the parent node, at parent `+30h`, `+48h`, `+54h`, `+60h` and `+6Ch`, through `00484540`.
The parent node is the global `*(*(00E188A8)+19CCh)`.

## `004F0AD0` (`PlaneSquadronGen`, 2131 authored instances) against the exemplar

The squadron creator is the one real divergence. It has an SEH frame (`00C680B8`) and:

```
004F0AEA: PUSH 414h ; CALL 00BF55BE ; PUSH 414h ; PUSH 0 ; PUSH ESI ; CALL 00BF79F0
004F0B18: CALL 007F2C60          ; __thiscall(squadron, 0) -- the squadron constructor
004F0B32: CALL 008F2260          ; bag "Type"
004F0B3C: CALL 00964790          ; ECX = Type, DL = 1 -- RESULT DISCARDED
```

The instance is the squadron's own **0x414-byte** object, built by `007F2C60`, not anything the
class descriptor allocates. `00964790`'s return value is never stored: `EDI` still holds the
squadron when the placement call runs at `004F0B75`. The call is made for its cache side effect,
so the plane class the squadron will spawn exists before the squadron is placed. Everything after
that point is the exemplar's tail, byte for byte: the same frame-job-pool predicate, the same
`vtable[98h]` / `004F03C0` pair, the same name string at `+154h` and the same `004E6B30`.

That means the squadron object shares the `+154h` name-string layout with the unit instances even
though it is a different class. The plane stock itself is not read here: it is the registration
pass (`004E6BC0` below) and `0046BF20` that walk `PlaneStock %d`.

The second caller of `004F0AD0` is `006C5050`, outside the scene reader; that path was not read.

## The other eight, against the exemplar

| Class | create | divergence |
| --- | --- | --- |
| `SubmarineGen` | `004F05F0` | none: identical instruction sequence, `+154h`, `vtable[98h]`, `004E6B30`, `RET 10h` |
| `TBoatGen` | `004F06C0` | none |
| `LandingShipGen` | `004F0790` | none |
| `MotherShipGen` | `004F0860` | none |
| `AirField` | `004F0930` | none |
| `Shipyard` | `004F0A00` | none |
| `LandFort` | `004F0FB0` | reads `Stationary` (`00CEA044`) first: `0048E9F0(bag, "Stationary")` returns a byte, and when it is set **and** the property's own byte at `+0Ch` is non-zero the instance comes from `00748C40(descriptor)` instead of `descriptor->vtable[28h](0)`. Otherwise identical, except that it never calls `004E6B30` |
| `CommandBuilding` | `004F10B0` | the exemplar minus the `004E6B30` call |

The six "none" rows were checked instruction by instruction against `004F0520`: same `008F2260`
key, same `MOV DL,1`, same `vtable[28h]` with a literal `0`, same `[EDX+98h]` placement slot, same
`LEA <reg>,[<instance>+154h]`, same `RET 10h`. They differ only in register allocation. `LandFort`
and `CommandBuilding` also use `[EDX+98h]` and `+154h`.

`00748C40` (the `Stationary` `LandFort` variant) allocates and memsets its own object and calls
`00748A40` and `0084F7F0`; its size was not read. `LandFort` is the highest-count scene class in
the shipped files (72615 entities), so the `Stationary` split is the hot path of the whole scene
loader.

## The registration pass

`0046CF40` runs `descriptor[+8]` on the registration pass. For the ten unit classes:

| Scene class | register | body |
| --- | --- | --- |
| `DestroyerGen`, `SubmarineGen`, `LandingShipGen`, `TBoatGen` | `004E98E0` | `JMP 004E96D0`, a five-byte thunk |
| `MotherShipGen` | `004E98F0` | `004E96D0`, then tail-`JMP 006BCF40` |
| `AirField` | `004E9900` | `004E96D0`, then tail-`JMP 006BCF40` |
| `Shipyard` | `004E9910` | `004E96D0`, then tail-`JMP 008433C0` |
| `PlaneSquadronGen` | `004E6BC0` | its own body, below |
| `LandFort`, `CommandBuilding` | `004E5BA0` | shared with the typed classes; not read by this packet |

### `004E96D0`, `__thiscall(PropertyBag* bag)`, `RET 0`

1. `Type` (`00CE4780`) into `EDI`, `Party` (`00CE5804`) into `EBP`, both from `+0Ch`.
2. `0095BA60(ECX = Type, EDX = Party)`.
3. `LaunchClassID` (`00CE9270`): present, type code 0, value >= 0 -> use it directly.
4. Otherwise build the Lua path `"VehicleClass." .. tostring(Type) .. ".Catapult.LaunchedClass"`
   by string concatenation and read it through `00B68D70` with a default of `-1`.
5. If the result is `> -1`: `0095BA60(launchClass, Party)` and
   `0046BE90(*(00E18680), launchClass)`, the already-named
   `BSP_SceneEntity_RegisterHiddenStockUnit`.

`LaunchedClass` is real data: `vehicleclasses.lua` row 261 (`Pennsylvania 1941`, `Type` =
`BattleShip`) carries `["LaunchedClass"] = 121`, its catapult float plane.

### `0095BA60`, `__fastcall(int typeId, int party)`, `RET 0`

Returns immediately unless `party` is 0 or 1. Otherwise `classIndex = singleton[+10h + typeId*4]`,
grows the array at `singleton+4010h` through `004359E0` if `classIndex >= [+4014h]`, and sets
`byte[base + classIndex*3 + party] = 1`. So the whole registration pass produces one bitmap:
**which vehicle classes each of the two parties needs**, keyed by class index, three bytes per row
(the third byte is never written here). That is what pass 2 records before instantiation.

### `004E6BC0`, `PlaneSquadronGen`, `__thiscall(PropertyBag* bag)`, `RET 0`

Six instructions of work: `Type` and `Party` out of the bag, `0095BA60(Type, Party)`, then
`0046BE90(*(00E18680), Type)`. No `LaunchClassID`, no Lua. A squadron marks its own plane class as
required for its party and registers it as a hidden stock unit.

`006BCF40` (`MotherShipGen`, `AirField`) walks `PlaneStock %d` with `Party` and `008F2260`, calling
`0095BA60` and `0046BE90` per slot; `008433C0` (`Shipyard`) does the same over `Stock %d`. Both are
the per-base stock half of `docs/SCENE_ENTITY_FACTORY.md`'s proposed `scene_multiplayer_stock`
packet and were not read further.

## Routines with no Ghidra function

| Address | End (inclusive) | Role |
| --- | --- | --- |
| `004E98E0` | `004E98E0` | `JMP 004E96D0`; the registration creator of `DestroyerGen`, `SubmarineGen`, `LandingShipGen`, `TBoatGen` |
| `004E6BC0` | `004E6BF5` | the `PlaneSquadronGen` registration creator |
| `006DFE40` | `006DFE40` | `JMP 00928860`; unit-instance `vtable[98h]` |

`004E98E0` and `006DFE40` are single `JMP` instructions followed by `INT3` padding; `004E6BC0` runs
to a `RET` at `004E6BF5`, also followed by `INT3` padding. The orchestrator has to define these
before a name can be applied. No `bsp.py ghidra flow` gap was seen in any routine read here.

## Reconstruction

`include/bsp/scene_unit_creators.hpp` / `src/scene_unit_creators.cpp`:

- `kSceneUnitCreatorTable`, the ten scene classes as data (class id, name, create and register
  addresses, whether the creator allocates its own instance, whether it applies the `Command`
  property, the extra key it reads).
- `kVehicleClassKindTable`, the 22 `VehicleClass.Type` literals with descriptor size and
  constructor address, in the factory's comparison order.
- `resolve_vehicle_class_00964790`, the pure `Type` -> class index -> cache -> Lua row rule,
  including the `LandingShip` recursion and the `Race` gate.
- `mark_party_requires_class_0095ba60`, the party bitmap rule.
- `scene_unit_launch_class_004e96d0`, the `LaunchClassID` / `Catapult.LaunchedClass` rule.
- `create_scene_unit_004f0520` and `create_plane_squadron_004f0ad0` over a
  `SceneUnitCreatorHost` with one method per native call site.

Nothing here is a drop-in binary replacement: the host is an injected interface, the property bag
is the repository's `bsp::ScenePropertyBlock`, and the native objects are opaque handles.

## Uncertainties

- The frame-job-pool predicate at `004C1130()+4`, `vtable[10h]`. Which branch a shipped load takes
  is unproven; the deferred branch's meaning is inferred from what it writes.
- `00964790`'s recursion guard `00F8A098` and the `00BF681B(1Ch)` / `00E0CFC0` block around
  `vtable[18h](19h)` and `vtable[18h](1Bh)` were not decoded.
- What fills `typeToClassIndex` at `singleton+10h`. `00438020` calls both `00437F50` and
  `00964790` and is the likely filler; not read.
- The third byte of each `singleton+4010h` row is never written by `0095BA60`. Its reader is
  unknown.
- `00748C40`'s object size and vtable (`LandFort` + `Stationary`).
- `004E5BA0`, the registration creator `LandFort` and `CommandBuilding` share with the typed
  classes.
- The instance layout beyond `+154h`, `+354h` and `+538h`, and the meaning of the five parent-node
  lists `006FE620` links into.
- The `ShipClasses` / `PlaneClasses` enum symbol tables that turn `PACK3_Pennsylvania` into a row
  index live in the property-descriptor system, not in any shipped Lua or data file; a lexical
  search of `Scripts/` found no definition.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `vehicle_class_descriptor` | 00964790 00963380 00963c20 00749180 00437f50 00437bd0 | docs/VEHICLE_CLASS_DESCRIPTOR.md | The 0x808 `MDestroyer` descriptor's layout, the twelve-slot vtable at `00D1ACF8`, and which `vehicleclasses.lua` fields each constructor reads |
| `vehicle_class_index_table` | 00438020 00437f50 00437bd0 004359e0 | docs/VEHICLE_CLASS_INDEX_TABLE.md | What fills `typeToClassIndex` at `singleton+10h`, and the third byte of the `+4010h` party rows |
| `unit_instance_layout` | 006fe460 006fe590 009553d0 00928860 009258f0 006fe620 00484540 | docs/UNIT_INSTANCE_LAYOUT.md | The 0x1188 instance: constructor, vtable `00CFC3D0`, the five parent-node lists and the world node `*(*(00E188A8)+19CCh)` |
| `scene_stationary_fort` | 004f0fb0 00748c40 00748a40 0084f7f0 0048e9f0 | docs/SCENE_STATIONARY_FORT.md | The `Stationary` `LandFort` variant, 72615 authored entities |
| `scene_entity_command_queue` | 00469610 004690d0 00468350 004e6b30 | docs/SCENE_ENTITY_COMMAND_QUEUE.md | The record `004E6B30` queues on the scene database and who drains it |

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 004F0520 | BSP_SceneUnit_CreateDestroyerGen | exported, analyzed, reconstructed, build-tested |
| 004F05F0 | BSP_SceneUnit_CreateSubmarineGen | analyzed (listing), tabulated |
| 004F06C0 | BSP_SceneUnit_CreateTBoatGen | analyzed (listing), tabulated |
| 004F0790 | BSP_SceneUnit_CreateLandingShipGen | analyzed (listing), tabulated |
| 004F0860 | BSP_SceneUnit_CreateMotherShipGen | analyzed (listing), tabulated |
| 004F0930 | BSP_SceneUnit_CreateAirField | analyzed (listing), tabulated |
| 004F0A00 | BSP_SceneUnit_CreateShipyard | analyzed (listing), tabulated |
| 004F0AD0 | BSP_SceneUnit_CreatePlaneSquadronGen | exported, analyzed, reconstructed, build-tested |
| 004F0FB0 | BSP_SceneUnit_CreateLandFort | analyzed (listing), tabulated |
| 004F10B0 | BSP_SceneUnit_CreateCommandBuilding | analyzed (listing), tabulated |
| 004E96D0 | BSP_SceneUnit_RegisterLaunchStock | exported, analyzed, reconstructed, build-tested |
| 004E98E0 | BSP_SceneUnit_RegisterStockThunk | analyzed (listing only, no Ghidra function) |
| 004E98F0 | BSP_SceneMotherShip_RegisterStock | analyzed (listing) |
| 004E9900 | BSP_SceneAirField_RegisterStock | analyzed (listing) |
| 004E9910 | BSP_SceneShipyard_RegisterStock | analyzed (listing) |
| 004E6BC0 | BSP_SceneSquadron_RegisterStock | analyzed (listing only, no Ghidra function) |
| 004E6B30 | BSP_SceneUnit_ApplyCommandProperty | exported, analyzed, reconstructed |
| 004F03C0 | BSP_SceneUnit_DeferHierarchyToProperties | exported, analyzed, reconstructed |
| 00964790 | BSP_VehicleClass_GetOrCreate | exported, analyzed, reconstructed, installed-file-checked |
| 0095BA60 | BSP_VehicleClass_MarkPartyRequired | analyzed (listing), reconstructed |
| 00470B80 | BSP_Memory_AllocZeroed | analyzed (listing) |
| 006FE590 | BSP_VehicleClassDestroyer_CreateInstance | analyzed (listing) |
| 009553D0 | BSP_Unit_SetVehicleClass | analyzed (listing) |
| 00469610 | BSP_SceneDatabase_QueueEntityCommand | not read past its call site |
