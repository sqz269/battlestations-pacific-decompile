# Ship class descriptor field reader

Addresses: 00831840 00870CD0 004D9C00 0082FD20 0082F8E0 0082E1F0 0049D4C0 0049F9B0 00499030 00444BE0 00424C40

`docs/VEHICLE_CLASS_FIELDS.md` reconstructed the descriptor base reader `00960230` and tabulated,
but did not decode, the ship family's override `00831840` `BSP_ShipClass_ReadLuaFields`. This
packet decodes it in full: the calling convention, the 67 key paths in body order, the nested
container layouts the earlier tabulation guessed at, and the shipped-file counts for ship rows.
The per-key table, the shipped counts and both key-gap lists are in
`reports/ship_class_fields.json`; the schema, the offsets and the reader sequence are in
`include/bsp/ship_class_fields.hpp` and `src/ship_class_fields.cpp`.

## Calling convention

`00831840` is `__thiscall(ship descriptor, LuaObject* row)` with `RET 4` at `00834688`. The
prologue takes the descriptor in `ECX` (`MOV EDI,ECX` at `00831867`) and the row from `[EBP+8]`
(`MOV ESI,[EBP+8]` at `00831863`), then forwards the row to `00960230` at `0083186E` before its
own first key. The row is the receiver of every `BSP_LuaObject_GetByName`, the same shape
`00960230` uses.

Both `ESI` and `EDI` change meaning inside the body: they hold the descriptor, the row, a Traffic
or Idle record or a key string at different points. Two reloads settle a question the earlier
packet left open. At `00832FB5` the receiver is reloaded from `[EBP+8]`, and at `00833C08` again,
so the keys after each reload are read from the row.

## What the earlier tabulation got wrong

The ship rows in `kVehicleClassShipFieldSchema` were extracted by pairing key strings with the
next store in the listing. That pairing is right for the flat keys and wrong for every nested one.
The corrections:

* **`Idle` is a row-level sibling of `Traffic`, not nested under it.** The `Traffic` walk ends at
  `00832FAF`; `00832FB5` reloads the receiver from `[EBP+8]` and `00832FC7` asks the row for
  `Idle`. Both are independent record vectors.
* `Traffic` is a vector at `+6DCh` of `50h`-byte records, not a scalar at `+4Ch`. `pathID` lands
  at record `+4Ch` as `pathID - 1`.
* `Idle` is a vector at `+6FCh` of `1Ch`-byte records. `posID` lands at record `+18h` as
  `posID - 1`; `templates` and `anims` are two weight maps at record `+0h` and `+0Ch`.
* `HoD` is 20 records of `30h` bytes at `+138h`, and `Flag`, `Smoke` and `Idle` are its per-record
  lists, not row keys with offsets `0`, `8` and `8`.
* `Camos` is an integer-keyed array of heap camo records at `+710h`; `TextureRemaps` and
  `GunColor` are nested inside a camo, not row keys.
* `LSReload` lands at `+788h` as `1.0f / value`, not at `+78Ch`. `+78Ch` is `LandingShip`, and it
  holds a resolved class pointer, not the integer id.
* `InnerExplosionEfx` and `StructuralDamageEfx` are row-level. The receiver at `0083413A` and
  `0083430B` is the row, not the `DamagedGFXRemove` table that precedes them.
* `CapturePower` is stored as a float (`CVTSI2SS` at `0083453C`) and its default is 10, not 0.
* `DamageSmoke.Effect` appends handles to the vector at `+664h`; `+660h` is only `MaxNumber`.

## Key table

`Slot` is the descriptor offset, or the offset inside the owning record for a nested key.
`Ship rows` is how many of the 160 shipped ship rows carry the path.

| Key path | Conversion | Slot | Site | Default | Ship rows |
| --- | --- | --- | --- | --- | --- |
| `MaxRotAngle` | Number | `+4F8h` | `00831882` | - | 160 |
| `MaxRotAngleChangeRatio` | Number | `+4FCh` | `008318c4` | - | 160 |
| `MaxSpeed` | Number | `+500h` | `00831903` | - | 160 |
| `MaxAccel` | Number | `+504h` | `00831942` | - | 160 |
| `Retardation` | Number | `+508h` | `00831981` | - | 160 |
| `CollisionMaxSpeedDamage` | NumberOr | `+50Ch` | `008319c0` | 0.0f | 160 |
| `KamikazeDamage` | NumberOr | `+510h` | `00831a05` | 0.0f | 2 |
| `KamikazeBlastDamage` | NumberOr | `+514h` | `00831a4a` | 0.0f | 2 |
| `KamikazeBlastRange` | NumberOr | `+518h` | `00831a8f` | 0.0f | 2 |
| `RotorSpdTurnDiff` | NumberOr | `+69Ch` | `00831ad4` | 0.0f | 0 |
| `RotorSpd` | NumberOr | `+6A0h` | `00831b19` | 15.0f | 0 |
| `DeathEfx` | EffectHandle | `+55Ch` | `00831ba9` | 0 | 160 |
| `ExplosionEfx` | EffectHandle | `+51Ch` | `00831cb7` | - | 12 |
| `UnderwaterArmour` | NumberOr | `+6B4h` | `00831d7c` | 0.0f | 149 |
| `DamageThreshold` | NumberOr | `+6B8h` | `00831dc4` | 100.0f | 146 |
| `CaptainCameraHeight` | NumberOrSlot | `+538h` | `00831e0d` | DAT_00ce38b8 | 11 |
| `CameraDistanceFront` | NumberOrSlot | `+53Ch` | `00831e60` | descriptor+A0h (Length) | 160 |
| `CameraDistanceSide` | NumberOrSlot | `+540h` | `00831ec7` | descriptor+53Ch | 160 |
| `CameraDistanceVertical` | NumberOrSlot | `+544h` | `00831f2e` | descriptor+53Ch | 160 |
| `CameraMinHeight` | NumberOrSlot | `+548h` | `00831f95` | descriptor+538h | 151 |
| `DamageToDeath` | NumberOr | `+54Ch` | `00831ffc` | -1.0f | 0 |
| `TimeToDeath` | NumberOr | `+550h` | `00832043` | -1.0f | 73 |
| `PumpTimeToEmpty` | NumberOr | `+554h` | `0083208a` | -1.0f | 38 |
| `WaterForceMultiplier` | NumberOr | `+558h` | `008320d1` | 1.0f | 14 |
| `BowParticle` | EffectHandle | `+5ACh` | `00832114` | - | 159 |
| `BowWave` | EffectHandle | `+630h` | `008321d8` | - | 159 |
| `WaveStern` | EffectHandle | `+634h` | `0083229c` | - | 160 |
| `RotorParticle` | EffectHandle | `+65Ch` | `00832360` | - | 159 |
| `DamageSmoke` | Table | `-` | `00832445` | - | 153 |
| `DamageSmoke.MaxNumber` | Integer | `+660h` | `0083247a` | - | 153 |
| `DamageSmoke.Effect` | EffectHandleList | `+664h` | `008324c1` | 0 | 153 |
| `HoD` | Table | `+138h` | `0083266c` | - | 141 |
| `HoD[].Flag` | IntegerList | `+138h` | `0083270e` | - | 123 |
| `HoD[].Smoke` | IntegerList | `+14Ch` | `008327d5` | - | 121 |
| `HoD[].Idle` | IntegerList | `+15Ch` | `008328fe` | - | 108 |
| `ExplosionTypes` | IntegerArray | `+690h` | `00832a73` | - | 137 |
| `Smoke` | EffectHandle | `+674h` | `00832b7a` | 0 | 142 |
| `Hull` | Table | `-` | `00832d51` | - | 160 |
| `Hull.WaterLineRatio` | Number | `+71Ch` | `00832d86` | - | 160 |
| `Hull.Segments` | NumberToInt | `+720h` | `00832dc8` | - | 160 |
| `Traffic` | RecordVector | `+6DCh` | `00832e0a` | - | 134 |
| `Traffic[].pathID` | IntegerMinusOne | `+4Ch` | `00832f1c` | - | 16 |
| `Idle` | RecordVector | `+6FCh` | `00832fc7` | - | 142 |
| `Idle[].posID` | IntegerMinusOne | `+18h` | `008330c5` | - | 125 |
| `Idle[].templates` | ClassWeightMap | `+0h` | `00833107` | - | 125 |
| `Idle[].anims` | NameWeightMap | `+Ch` | `00833301` | - | 125 |
| `Camos` | CamoArray | `+710h` | `00833501` | - | 160 |
| `Camos[].TextureRemaps` | StringPairList | `-` | `0083368b` | - | 16 |
| `Camos[].GunColor` | Vector4 | `+1Ch` | `0083394a` | - | 141 |
| `LSClassId` | Integer | `+724h` | `00833a3a` | - | 0 |
| `LSPoints` | Vector3Array8 | `+728h` | `00833a7c` | - | 0 |
| `LSReload` | NumberReciprocal | `+788h` | `00833afe` | - | 0 |
| `LandingShip` | ClassPointerOr | `+78Ch` | `00833b6c` | 0 | 2 |
| `LandingShipAmount` | IntegerOr | `+790h` | `00833bcb` | 0 | 2 |
| `LandingShipCoolDown` | IntegerOr | `+794h` | `00833c0a` | 60 | 2 |
| `Fire` | Table | `-` | `00833c49` | - | 0 |
| `Fire.FireDamagePerFireTick` | NumberOrSettings | `+574h` | `00833c81` | settings+74Ch | 0 |
| `MaxTorpedoStock` | IntegerOrZero | `+7A0h` | `00833ce6` | 0 | 74 |
| `DamagedGFXRemove` | Table | `-` | `00833d2a` | - | 125 |
| `DamagedGFXRemove.Slots` | IntegerList | `+7A8h` | `00833d74` | - | 105 |
| `DamagedGFXRemove.Funnels` | IntegerListMinusOne | `+7B8h` | `00833eab` | - | 106 |
| `DamagedGFXRemove.Flags` | IntegerListMinusOne | `+7C8h` | `00833fd0` | - | 125 |
| `InnerExplosionEfx` | IndexedEffectList | `+7D8h` | `0083413c` | - | 147 |
| `StructuralDamageEfx` | EffectHandleList | `+7F4h` | `0083430d` | - | 147 |
| `HackShipRotationAdd` | NumberOrSettings | `+798h` | `00834476` | settings+238h | 112 |
| `HackShipHeightAdd` | NumberOrSettings | `+79Ch` | `008344ce` | settings+23Ch | 99 |
| `CapturePower` | IntegerOrAsFloat | `+804h` | `00834526` | 10 | 160 |

## Nested container layouts

### `HoD`, 20 records of `30h` bytes at `+138h`

`00832687` skips the block unless the value is a table, `0083268D` starts the index at 0 and
`00832695` sets the record cursor to `descriptor+15Ch`, which is record `+24h`. The loop runs
while the index is below 20 and advances the cursor by `30h`. The Lua index selects the record, so
a gap in the table leaves a record untouched rather than shifting the rest.

| Record offset | Content | Evidence |
| --- | --- | --- |
| `+0h` | `Flag` list, appended by `00442190` | `LEA ESI,[EBX-0x24]` at `0083275D`, call at `0083277A` |
| `+14h`, `+18h`, `+1Ch` | `Smoke` integer vector, begin / end / capacity | the push at `008327D5`'s walk |
| `+24h`, `+28h`, `+2Ch` | `Idle` integer vector, begin / end / capacity | the push at `008328FE`'s walk |
| `+0Ch`..`+13h`, `+20h` | not written here | - |

Each of the three is a key/value iteration that keeps only the integer value; a non-table value
skips that list and leaves the record's vector empty.

### `Traffic`, a vector at `+6DCh` of `50h`-byte records

`00832E1E` tests `IsNil` on the row's `Traffic`; a nil skips straight to the `Idle` block. The
walk then reads index 1, and while the element is not nil: `0049F9B0` default-constructs a
`50h`-byte prototype on the stack, `0082FD20` resizes the vector at `+6DCh` to the loop index with
that prototype, and the new last element is `end - 50h` read through the begin and end pointers at
`+6E0h` and `+6E4h`. `pathID` is read with `GetInteger`, decremented at `00832F30` and stored at
record `+4Ch`. `0049D4C0` at `00832F54` then fills the rest of the record from the same Lua
element; that routine was not analysed, which is why only 16 of the 134 shipped rows with a
`Traffic` table carry `pathID` at all.

### `Idle`, a vector at `+6FCh` of `1Ch`-byte records

The same shape with `0082E1F0` as the constructor and `0082F8E0` as the resize.

| Record offset | Content |
| --- | --- |
| `+0h` | `templates`, a weight map keyed by a resolved class pointer; `00499030` returns the float slot |
| `+0Ch` | `anims`, a weight map keyed by the raw animation name; `00444BE0` returns the float slot |
| `+18h` | `posID - 1`, stored at `008330E3` |

`templates` is the only place the reader resolves a name against another data table. For each key
it asks the manager at `[00E1867C]` whether `LandVehicleclasses` (`00CE6710`) holds it, through
`0048E960` then `0048E8D0` at `008331F2` and `008331F9`. When it does, `0048E840` fetches the
entry and `BSP_VehicleClass_GetOrCreate` `00964790` resolves it with `DL = 1`. When it does not,
the same pair runs against `SoldierTypes` (`00CE6700`) and `004B1400` resolves it instead. The Lua
number becomes the float in the map slot.

### `Camos`, an integer-keyed array at `+710h`

`00833515` requires a table, then the walk is a key/value iteration. The integer key indexes the
array whose begin and end are `+710h` and `+714h`, checked against the bounds trap at `0083360A`.
Each value allocates a refcounted camo record with the vtable at `00D099B0`, releases whatever the
slot held and stores the new record. Inside a camo:

* `TextureRemaps` is a list of two-element string arrays; both strings are read by `GetString` at
  `0083374E` and `008337BB`, copied into native strings and appended through `0082B170`,
  `0082E160` and `0082AD20`. Only 16 ship rows use it.
* `GunColor` sets the byte at camo `+18h` to 1 and copies four floats from `00B67C40` into
  `+1Ch`..`+28h`; an absent key sets the byte to 0. 141 ship rows use it.

### `DamagedGFXRemove` and the two row-level effect lists

`DamagedGFXRemove` holds three integer vectors, each `{begin, end, capacity}`:

| Key | Vector | Value stored |
| --- | --- | --- |
| `Slots` | `+7A8h` | unchanged |
| `Funnels` | `+7B8h` | value minus one |
| `Flags` | `+7C8h` | value minus one |

`InnerExplosionEfx` and `StructuralDamageEfx` follow but are read from the row. `InnerExplosionEfx`
puts the integer key into the vector at `+7D8h` and the value's handle into the one at `+7E4h`
(`LEA EBX,[EAX+0x7e4]` at `008341DC`). `StructuralDamageEfx` keeps only the value's handle, in the
vector at `+7F4h` (`LEA EBX,[ESI+0x7f4]` at `008343A3`).

## Defaults

Eleven keys carry a literal default, five chain to another slot and three come from the settings
object `00424C40` returns.

| Default source | Keys |
| --- | --- |
| `FLDZ`, so 0.0 | `CollisionMaxSpeedDamage`, `KamikazeDamage`, `KamikazeBlastDamage`, `KamikazeBlastRange`, `RotorSpdTurnDiff`, `UnderwaterArmour` |
| `FLD1`, so 1.0 | `WaterForceMultiplier` |
| `00CE5380` = 15.0 | `RotorSpd` |
| `00CE3D08` = 100.0 | `DamageThreshold` |
| `00D7A260` = -1.0 | `DamageToDeath`, `TimeToDeath`, `PumpTimeToEmpty` |
| an integer immediate | `DeathEfx` 0, `Smoke` 0, `LandingShip` 0, `LandingShipAmount` 0, `LandingShipCoolDown` 60, `CapturePower` 10 |
| another slot | `CameraDistanceFront` from the base `Length` at `+A0h`, `CameraDistanceSide` and `CameraDistanceVertical` from `+53Ch`, `CameraMinHeight` from `+538h`, `CaptainCameraHeight` from `DAT_00CE38B8` |
| the settings object | `HackShipRotationAdd` from `+238h`, `HackShipHeightAdd` from `+23Ch`, `Fire.FireDamagePerFireTick` from `+74Ch` |

The camera block is a chain, not five independent fallbacks: `CameraDistanceSide` and
`CameraDistanceVertical` both fall back to `CameraDistanceFront`, which may itself already be the
base `Length`. Each guard is `IsNil`, so a present but non-numeric value goes through `00B66270`
rather than taking the fallback. `tests/math_tests.cpp` pins the chain.

Six keys are read twice, once to test and once to convert: `DeathEfx`, `ExplosionEfx`,
`BowParticle`, `BowWave`, `WaveStern` and `RotorParticle` each run `IsInteger` on a first fetch and
leave the slot untouched when it fails, so an absent or non-integer value keeps whatever the base
reader left there rather than clearing it.

## Installed-file check

`Scripts/datatables/autoload/vehicleclasses.lua` was parsed read-only by an independent Lua-subset
parser written for this packet (`local/shipcheck.py` in the worktree). It accepts table
constructors, `["str"]` and `[int]` keys, numbers, quoted strings, booleans, `nil`, `NAME(...)`
call expressions and comments, and treats `--[[` as nesting, which the earlier packet found is
necessary for the file to balance and which excludes the five commented-out trailing records. A
ship row is one whose `Type` is `BattleShip`, `Cargo`, `Cruiser`, `Destroyer`, `LandingShip`,
`MotherShip`, `Submarine` or `TorpedoBoat`; that classification is taken from the `Type` strings,
not from the factory's dispatch, and is provisional.

| Measure | Value |
| --- | --- |
| rows in `VehicleClass` | 633 |
| ship rows | 160 |
| distinct key paths on ship rows | 211 |
| key paths this reader consumes | 67 |
| this reader expects, no ship row provides | 8 |
| shipped on ship rows, the base or shared reader consumes | 61 |
| shipped on ship rows, no reader consumes by name | 74 |
| `Idle[].templates` and `Idle[].anims` leaves, which are data | 17 |

The eight keys `00831840` asks for that no ship row provides are `DamageToDeath`, `Fire`,
`Fire.FireDamagePerFireTick`, `LSClassId`, `LSPoints`, `LSReload`, `RotorSpd` and
`RotorSpdTurnDiff`. So the whole landing-ship point block never runs on the shipped data, the fire
tick always takes the settings default, and the two rotor keys always take theirs. This agrees
with the earlier packet's list for the rows it covered.

Of the 74 shipped ship keys no reader consumes by name, the largest are `ArmorIndexes` and
`Damage.Sections[].HPBlack` (160 rows each), `Type` and `Race` (160, read by the factory
`00964790` rather than by a field reader), `HP_Realistic` (158), `MaxSpeed_Realistic` (154),
`MovieCameraPositions.SnittPositions.IncomingAttack` (143), the four `PlatformDirections` legs
(117), `Unlock` (74), `MaxTorpedoStock_Realistic` (71), `UnitlibViewDistance` (68), the five
`SmokeScreen` keys (47) and `DeckCamera` (25). The realistic-difficulty variants and the whole
smoke-screen block are the two most conspicuous groups: they are shipped for ships and no reader
in this family reads them, so whatever consumes them is outside the descriptor field readers.

## Callers and callees

Eight constructors reach `00831840` through the descriptor vtable: `006E00F0`, `006EB4A0`,
`006FB550`, `006FE6A0`, `0074C630`, `00759590`, `00854230` and `00857F40`. It calls 51 routines;
the ones this packet decoded far enough to name are in the ledger.

`bsp.py ghidra flow 00831840` reports seven gaps. All seven follow an unconditional `JMP` and none
follows a `CALL`, so they are alignment padding rather than the `_free` fall-through the repair
tool targets. They are reported, not repaired.

## Reconstruction

`include/bsp/ship_class_fields.hpp` and `src/ship_class_fields.cpp` carry the 67-key schema as
data in the same shape `vehicle_class_fields.hpp` uses, the descriptor and record offsets as
documented structs, the three post-processing rules as pure functions, and the reader sequence over
`bsp::VehicleClassFieldHost` for the Lua half plus a new `bsp::ShipClassFieldHost` with one method
per remaining native call site. Nothing from `vehicle_class_fields.hpp` is redefined; the
provisional `kVehicleClassShipFieldSchema` there is superseded but left alone, because that file
belongs to another packet.

The header is a semantic reconstruction. It is not ABI compatible with the descriptor, it does not
reproduce the refcount traffic or the SEH states, and it has not been run against the game.

## Uncertainties

* `0049D4C0` fills most of the `50h`-byte Traffic record and was not analysed, so only `pathID` is
  attributed.
* The `HoD` record's `+0Ch`..`+13h` and `+20h` are not written by this reader, and the `Flag`
  container at record `+0h` is known only through `00442190`, whose element type was not decoded.
* `Smoke` builds three handles from one id, into `+674h`, `+6A4h` and `+6A8h`. Why three was not
  established.
* The camo record beyond `+18h`..`+28h`, and the `TextureRemaps` container built by `0082B170`,
  `0082E160` and `0082AD20`, were not decoded.
* `DAT_00CE38B8` was read as an address only; nothing here says what writes it.
* The ship-row rule for the installed-file check comes from the `Type` strings, not from the
  factory's dispatch table, so the 160 is a close estimate rather than the exact set of rows that
  reach this reader.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `ship_traffic_record` | 0049d4c0 0049f9b0 0082fd20 | `docs/SHIP_TRAFFIC_RECORD.md` | Decode the `50h`-byte Traffic record `0049D4C0` fills, and say which subsystem walks `descriptor+6DCh` |
| `ship_hod_record` | 00442190 0083270e 008327d5 008328fe | `docs/SHIP_HOD_RECORD.md` | Decode the `30h`-byte HoD record at `descriptor+138h`, its `Flag` container and the 20-slot indexing, and find its read sites |
| `ship_camo_record` | 00833501 0082b170 0082ad20 0082e160 | `docs/SHIP_CAMO_RECORD.md` | Decode the camo record allocated at `008335BD` and the `TextureRemaps` container, and find where the renderer reads `+710h` |
| `plane_class_fields` | 007d1f70 | `docs/PLANE_CLASS_FIELDS.md` | The same treatment for the 77-key plane reader, whose nested `BayDoor` and `PartAnims` blocks are also only tabulated |

The plane reader `007D1F70` was left as a comparison this packet did not have budget for; its
77-key tabulation in `docs/VEHICLE_CLASS_FIELDS.md` stands, with the same caveat that every nested
key's offset there is unverified.

## State

| Routine | State |
| --- | --- |
| `00831840` | exported, analysed, reconstructed, build-tested, installed-file-checked |
| `00870CD0`, `004D9C00` | exported, analysed, name recorded |
| `0082FD20`, `0082F8E0`, `0082E1F0` | exported, analysed from their call sites, name recorded |
| `0049D4C0`, `0049F9B0`, `00499030`, `00444BE0`, `00424C40` | exported only, read for their role, not named |
| `007D1F70` | unchanged from the earlier packet: exported, analysed (key table only) |
