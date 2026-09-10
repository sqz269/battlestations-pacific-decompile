# Vehicle class descriptor field reader

Addresses: 00960230 0087CA80 006D0B80 00700E40 00749210 0074D4A0 007D1F70 00831840

This closes the main gap `docs/VEHICLE_CLASS_DESCRIPTORS.md` left open: `00960230`, the descriptor
base's vtable slot `+8h`, is the Lua field reader, and this packet decodes it key by key. The
per-key table, the shipped-file counts and the two key-gap lists are in
`reports/vehicle_class_fields.json`; `include/bsp/vehicle_class_fields.hpp` and
`src/vehicle_class_fields.cpp` carry the schema as data and the reader sequence over an injected
host. Names below are hypotheses, not recovered symbols.

## Calling convention

`00960230` is `__thiscall(void* descriptor /* ECX */, LuaObject* row /* [EBP+8] */)`, `RET 4`
(`00963232`). The argument is the Lua row object itself: `00960279` moves it into `ECX` as the
receiver of `BSP_LuaObject_GetByName`, which **corrects** the descriptor doc's reading of slot
`+8h` as taking a native-string out-parameter. The body opens an SEH frame with handler
`00CA9BDD` and tracks 88h unwind states, one per live `LuaObject`.

The first thing it does, before its own first key, is `PUSH ESI; CALL 0087CA80` at `00960267`:
`0087CA80` is `__thiscall(descriptor, LuaObject* row)`, `RET 4` (`0087D723`), a shared
damageable-class pre-reader also called from `00440AC0`.

All six callers of `00960230` are leaf overrides of the same vtable slot; each chains to the base
first and then reads its family's keys. Every one is `__thiscall(descriptor, LuaObject* row)`,
`RET 4`.

| Routine | Family | Keys | Proposed name |
| --- | --- | --- | --- |
| `0087CA80` | shared pre-reader | 16 | `BSP_DamageableClass_ReadLuaFields` |
| `00960230` | descriptor base | 68 sites | `BSP_VehicleClass_ReadLuaFields` |
| `00831840` | ship | 67 | `BSP_ShipClass_ReadLuaFields` |
| `007D1F70` | plane | 77 | `BSP_PlaneClass_ReadLuaFields` |
| `00749210` | structure / fort | 13 | `BSP_StructureClass_ReadLuaFields` |
| `0074D4A0` | wreckable | 3 | `BSP_WreckableClass_ReadLuaFields` |
| `00700E40` | door | 3 | `BSP_DoorClass_ReadLuaFields` |
| `006D0B80` | runway | 2 | `BSP_RunwayClass_ReadLuaFields` |

Every routine already has a Ghidra function, so `no_ghidra_function` in the report is empty.

## The Lua wrappers the reader calls

The reader never touches `lua_*` directly. It goes through the wrapper layer already named in the
ledger: `BSP_LuaObject_GetByName` `00B67800`, `GetByIndex` `00B67720`, `IsNil` `00B65FB0`,
`IsTable` `00B661B0`, `IsInteger` `00B66A60`, `IsUnbound` `00B66420`, `GetNumber` `00B66270`,
`GetInteger` `00B66290`, `GetBoolean` `00B66250`, `GetString` `00B662B0`, `IterateFirst`
`00B67080`, `IterateNext` `00B67190`, `Destruct` `00B67700`, and the four defaulting forms
`BSP_LuaReference_GetFloatOrDefault` `00B66330`, `GetIntegerOrDefault` `00B66380`,
`GetBooleanOrDefault` `00B662F0` and the string form `00B685C0`. `GetByName` returns the out
object in `EAX`, which is why the pseudocode looks argument-less: Ghidra drops the stack arguments
of the unprototyped wrappers, so every signature below comes from the listing.

The refcount idiom that appears after each resource store is
`InterlockedIncrement` at `[00CE221C]` on the new object and `InterlockedDecrement` at `[00CE2220]`
on the old, followed by `vtable[0]` at zero.

## The shared pre-reader, 0087CA80

| Key | Type | Descriptor offset | Default | Evidence |
| --- | --- | --- | --- | --- |
| `Name` | string | `+54h`, a `char*` | none | `0087CAB8` |
| `Unique` | boolean | `+34h`, byte | none | `0087CAFC` |
| `Mesh` | string | model load | `""` | `0087CB44` |
| `Comment` | string | debug name | `""` | `0087CBDB` |
| `HP` | float | `+48h` | `100.0f` (`00CE3D08`) | `0087CC72`, `FSTP [ESI+48h]` at `0087CC98` |
| `Armour` | float | `+4Ch` | `0.0f` (`FLDZ`) | `0087CCB4` |
| `ExplosionType` | integer | `+40h` | none | `0087CCEE`, integer probe first |
| `Damage.Sections[]` | table | per-section object | | `0087CD59`, `0087CD88` |

Each section reads `MshCategory` (string), `Index` (number, into section `+4h` and `+8h`),
`FireEfx` (integer, `+24h`), `FailureChance` (float, `+28h`) and `FailureDamageThreshold`
(float, `+2Ch`). `FakeExplosionEffects` (`0087D229`) is indexed by `%d`-formatted keys and
`Emberkek` (`0087D56D`) is an iterated crew table; neither element layout was decoded.

This attributes `+34h`, `+40h`, `+48h`, `+4Ch` and `+54h`, which the descriptor doc listed as
unattributed apart from the refcount at `+4h` and the name pointer at `+54h`.

## The base reader, 00960230

Keys in body order. `float` means the value lands through `FSTP float ptr`; `sound` means the
integer is probed with `IsInteger`, wrapped by `00870CD0` and stored as a refcounted handle.

| Key | Container | Type | Offset | Default | Site |
| --- | --- | --- | --- | --- | --- |
| `SMIcon` | row | string | `+D4h` | `"gui\units\b25.tga"` | `0096027B` |
| `Width` | row | float | `+A4h` | none, a missing key stores 0 | `00960354` |
| `Length` | row | float | `+A0h` | none | `0096038B` |
| `Height` | row | float | `+A8h` | none | `009603C2` |
| `TotalHeight` | row | float | `+ACh` | the `Height` just stored | `009603F9` |
| `Mass` | row | float | `+B0h` | `1.0f` (`FLD1`) | `0096043A` |
| `Catapult` | row | table | | not a table resets `+C0h` to -1 and `+C8h`/`+CCh` to 0 | `0096047A` |
| `LaunchedClass` | `Catapult` | int | `+C0h` | `-1` | `009604AC` |
| `LaunchStock` | `Catapult` | int | `+C8h` | `0` | `009604EA` |
| `MaxLaunchedPlanes` | `Catapult` | int | `+CCh` | `0` | `00960527` |
| `Equipment` | `Catapult` | int | `+C4h` | `0` | `00960564` |
| `CockpitMesh` | row | string | `+80h` | `""` | `009605B7` |
| `CockpitCameraPos` | row | 3 floats | `+84h`..`+8Ch` | zero | `00960659` |
| `SpecRole` | row | string | `+90h` | `0` | `009606B7` |
| `SoundEfx` | row | table | | nil skips every sound key, jumping to `0096109A` | `00960741` |
| `EngineSoundEfx` | `SoundEfx` | sound | vector `+E8h` | | `0096076D` |
| `ContFireEfx` | `SoundEfx` | sound | vector `+ECh` | | `0096082E` |
| `DeadMeatEfx` | `SoundEfx` | sound | `+F0h` | | `009608EF` |
| `WindEfx` | `SoundEfx` | sound | `+F4h` | | `009609DE` |
| `TooFastEfx` | `SoundEfx` | sound | `+F8h` | | `00960ACD` |
| `HajoCsavarEfx` | `SoundEfx` | sound | `+FCh` | Hungarian for ship-propeller | `00960BBC` |
| `OrrHullamEfx` | `SoundEfx` | sound | `+100h` | Hungarian for bow-wave | `00960CAB` |
| `ElevatorEfx` | `SoundEfx` | sound | `+104h` | | `00960D9A` |
| `SonarPing` | `SoundEfx` | sound | `+108h` | | `00960E89` |
| `Ambient` | `SoundEfx` | string | `+10Ch` | resolved by name through `00A83FD0` | `00960F78` |
| `Cost` | row | int | `+124h` | `0` | `009610BA` |
| `Platforms` | row | table | vector `+94h`/`+98h` | | `009610F6` |
| `GunDelayGroups` | row | table | | list of lists of slot indices | `00961C9C` |
| `DefaultEquipment` | row | int | `+134h` | cleared to 0 first | `00961F16` |
| `Equipments` | row | table | `+12Ch`/`+130h` | | `00961F57` |
| `ReconModifier` | row | float | `+B8h` | `0.0f` | `009623A9` |
| `ReconClass` | row | int | `+B4h` | resolved through `00808F90` | `009623F0` |
| `MovieCameraPositions` | row | table | `+BCh` | | `009624A8` |
| `Repair` | row | boolean | `+D0h`, byte | | `00962DBC` |
| `Flags` | row | table | vector `+110h` | | `00962E4E` |

### SpecRole

`009606BC` reads the string with an empty default, then `009606F7` compares it with `strcmp`
(`00BF7FBF`) against `"FlyingControll"` and stores `0`; failing that, `00960716` uses the
case-insensitive helper `00425850` against `"BomberPilot"` and stores `22h`. The asymmetry is
real: only the second comparison tolerates a case change. Anything else leaves the `0` written at
`009606B1`.

### Platforms

Each entry allocates `98h` bytes (`operator new` at `00961170`), constructs it with `007F7110`,
and fills it. The Lua **key** is the slot index, not a running counter: `00961B69` reads it back
out of platform `+8h`, `00961B72` compares it with the vector's count at `descriptor+98h`, grows
through `005471B0` when short, and writes the pointer into `*(base + index*4)` with the
increment/decrement pair. A table with a gap therefore leaves a null slot. This is the same
`+94h`/`+98h` pointer vector the descriptor doc records vtable slot `+10h` walking.

| Platform field | Offset | Key | Site |
| --- | --- | --- | --- |
| slot index | `+8h` | the Lua key | `009611B7` |
| `PilotFires` | `+Ch`, byte | | `009611CB` |
| `UseBayDoor` | `+Dh`, byte | | `00961250` |
| `MainPlatform` | `+Eh`, byte | | `00961296` |
| `DefaultGun` | `+38h` | resolved object | `0096146A` |
| `ForwardAim` | `+48h`, float | | `00961211` |
| `Name` | `+8Ch`, duplicated `char*` | | `009612D9` |
| `RestAngles[2]` | `+90h`, float | default `0.0f` | `00961B3A` |
| `RestAngles[1]` | `+94h`, float | default `FLT_MAX` (`00D7A248`) | `00961ADF` |

`Gun` (`00961335`) is an integer list of weapon class ids. `Windows` (`0096160D`) is a list of
firing arcs, each built by `007F6B10` from `Nofire`, `MinHorzAngle`, `MaxHorzAngle`,
`MinVertAngle` and `MaxVertAngle`. `DirectorFollowPlatforms` (`009618FC`) is an integer list.

### Flags

`00962EC9` takes `descriptor+110h` as a `std::vector` with `_Myfirst` at `+114h` and `_Mylast` at
`+118h`; the `0x66666667` / `SAR 3` divide at `00962F06` gives a stride of `14h` bytes. The Lua
key is one-based and `00962EFA` subtracts one for the element index, resizing through `0095D850`
with a default element of `{0.0f, 0.0f, 0, 0, 0}`.

| Flag field | Offset | Key | Default |
| --- | --- | --- | --- |
| size horizontal | `+0h`, float | `SizeHorizontal` | `2.5f` (`00CF87C8`) |
| size vertical | `+4h`, float | `SizeVertical` | `1.5f` (`00CE380C`) |
| texture path | `+8h`, `{length, buffer}` | `ForcedTexture` | `""` |
| texture handle | `+10h` | loaded by `(*00F8D394)->vtable[64h](path, 0)` | null |

The path is resized with `BSP_NativeString_Resize` `0041DD40` and copied with `memcpy` `00BF7680`;
the load only runs when the resolved path differs from the empty default (`009630DD`).

### MovieCameraPositions and Equipments

`MovieCameraPositions` (`009624A8`) is walked only when it is a table and builds the container at
`descriptor+BCh`. Under it, `Positions` (`0096253D`) is a list of `5Ch`-byte camera records taking
`Name`, `PureSnitt` and the four angle keys, with `MinHorzAngle`/`MaxHorzAngle` read as a pair and
`MinVertAngle`/`MaxVertAngle` as another, both halves required. `SnittPositions` (`00962A3B`) is
string-keyed and `00962D8F` clears `+BCh` afterwards.

`Equipments` (`00961F57`) has two iterator levels; each innermost entry reads `Platform`, `Ammo`
and `ReloadTime` into a 10h-byte record, and the built range lands in `descriptor+12Ch` and
`+130h` at `009622CF`.

## Leaf readers

The full per-key tables are in the report. The larger two:

* `00831840`, the ship family, reads 67 keys into ship-descriptor offsets `+4F8h`..`+804h`:
  the motion block (`MaxRotAngle` `+4F8h` through `Retardation` `+508h`), the kamikaze block
  (`+50Ch`..`+518h`), the camera block (`CaptainCameraHeight` `+538h`, then
  `CameraDistanceFront` `+53Ch` defaulting to the base's `Length`, `CameraDistanceSide` `+540h`
  and `CameraDistanceVertical` `+544h` both defaulting to `CameraDistanceFront`, and
  `CameraMinHeight` `+548h` defaulting to `CaptainCameraHeight`), the sinking block
  (`DamageToDeath` `+54Ch`, `TimeToDeath` `+550h`, `PumpTimeToEmpty` `+554h`,
  `WaterForceMultiplier` `+558h`), the wake particles (`BowParticle` `+5ACh`, `BowWave` `+630h`,
  `WaveStern` `+634h`, `RotorParticle` `+65Ch`), `Hull.WaterLineRatio` `+71Ch` and
  `Hull.Segments` `+720h`, the landing-ship block (`LSClassId` `+724h`, `LandingShip` `+78Ch`,
  `LandingShipAmount` `+790h`, `LandingShipCoolDown` `+794h`), `MaxTorpedoStock` `+7A0h`,
  `HackShipRotationAdd` `+798h`, `HackShipHeightAdd` `+79Ch` and `CapturePower` `+804h`.
* `007D1F70`, the plane family, reads 77 keys into `+140h`..`+608h`: the aerodynamic block
  (`Accel` `+164h`, `YDrag` `+170h`, `XDrag` `+174h`, the rotation accelerations `+178h`..`+180h`,
  the speed block `+184h`..`+1A4h`, the rate block `+1A8h`..`+1D8h`), the ground block
  (`WheelHeight` `+1FCh`, `GroundPitch` `+200h`, `WaterPitch` `+204h`, `GlideRate` `+208h`), the
  effect handles (`ExplosionEfx` `+214h`, `EngineFireEfx` `+218h`/`+220h`, `ShellsEfx` `+234h`,
  `WingTipEfx` `+5A4h`), `BayDoor` (`OpenAngle` `+5A8h`, `ClosedAngle` `+5ACh`, `TimeToOpen`
  `+5B0h`), `PartAnims` (`Gears` `+5C8h`, `Wings` `+5D8h`, `BayDoor` `+5E8h`) and the turbo block
  `+5FCh`..`+608h`.

The three small ones: `00749210` adds the fire and secondary-explosion block plus `MapIcon`
`+17Ch`, `Fortress` `+175h` and `DamageModelChangeDelay` `+138h`; `0074D4A0` adds `ExplosionEfx`
`+138h`, `Wreck` `+13Ch` and `Smoke` `+14Ch`; `00700E40` adds `ExplosionEfx` `+140h`, `OpenTime`
and `SlowingFactor`; `006D0B80` adds `RunwayWidth` `+138h` and `RunwayLength` `+13Ch`.

## Installed-file check

`Scripts/datatables/autoload/vehicleclasses.lua` (3,158,242 bytes) was parsed read-only by an
independent Lua-subset parser written for this packet (`local/luaparse.py` in the worktree; it
accepts only table constructors, `["str"]`/`[int]` keys, numbers, quoted strings, booleans,
`nil`, `NAME(...)` call expressions and comments). Results:

| Measure | Value |
| --- | --- |
| rows in `VehicleClass` | 633 |
| distinct top-level keys | 198 |
| distinct key paths | 342 |
| key paths some reader consumes | 231 |
| reader expects, no row provides | 17 |
| shipped, no reader consumes by name | 108 |

The file only balances if `--[[` block comments **nest**, which Lua 5.1.1 does not do: the last
five records are commented out and the closing `]]` sits at end of file. The parser was made
nesting-aware and those records were excluded; the game presumably loads a preprocessed or
precompiled chunk. This is recorded as an uncertainty rather than resolved.

Keys the readers ask for that no shipped row provides: `CockpitMesh`, `Unique`, `ExplosionType`,
`Emberkek`, `Damage.Sections[].FailureChance`, `Damage.Sections[].FailureDamageThreshold`,
`Platforms[].DirectorFollowPlatforms`, `FakedType`, `GearsPullTime`, `DamageToDeath`, `RotorSpd`,
`RotorSpdTurnDiff`, `LSClassId`, `LSPoints`, `LSReload`, `Fire` and `Fire.FireDamagePerFireTick`.
So the cockpit-mesh load, the platform director-follow list and the whole landing-ship point block
never run on the shipped data.

Of the 108 shipped paths no reader names, 18 are the leaves of `Idle[].templates` and
`Idle[].anims`, which `00831840` iterates generically, so they are data rather than schema. The
rest are genuinely untouched here; the largest are `ArmorIndexes` (550 rows),
`Damage.Sections[].HPBlack` (1382 occurrences), `HP_Realistic` (595), `PlatformDirections` (139),
`UnitlibViewDistance` (133), `Unlock` (101) and the whole `Traffic[]` movement block. `Type` (633)
and `Race` (232) are read by the factory `00964790`, not by the field reader.

One shipped row spells `HoD[].smoke` in lower case where the reader asks for `Smoke`, so that
entry is silently dropped.

## Consumers

`docs/VEHICLE_CLASS_DESCRIPTORS.md` already records the fields the factory `00964790` writes and
reads (`+70h` class index, `+74h`/`+78h` the `Type` string, `+7Ch` `NumEngines`, `+C0h` the linked
class index) and the vtable slot `+10h` activate walking the pointer vector at `+94h`/`+98h` when
the byte at `+44h` is clear. That vector is exactly the platform array this reader fills, which
ties the two documents together. `docs/SCENE_UNIT_CREATORS.md` covers the creators that consume
the descriptor and `docs/UNIT_INSTANCE_UPDATE.md`, when it lands, will consume the stats
attributed here. A full read-site map for the remaining offsets was not established in this packet
and is proposed as a follow-up below.

## Uncertainties

* The two flow gaps `bsp.py ghidra flow 00960230` reports, both after `CALL 00BF6989`
  (`00962C9C..00962CAB`, 15 bytes and `00962CEF..00962D51`, 98 bytes), sit inside the
  `SnittPositions` block. They are reported, not repaired, so the `SnittPositions` walk is decoded
  only as far as its first two entries and its element layout is unknown.
* `00831840`'s `Traffic` / `Idle` nesting: the reader reads `Traffic[i].pathID` and
  `Idle[i].posID`, `templates` and `anims`, but whether that `Idle` is the row's own table or one
  nested under `Traffic` was not settled from the listing.
* `0087CA80`'s `Emberkek` and `FakeExplosionEffects` element layouts were not decoded.
* The `Equipments` outer/inner nesting is taken from the listing's two iterators and corroborated
  by the shipped data's `Equipments[i][j]` shape, not from a decoded container type.
* `00B685C0` and `00B680A0` were read only for their signatures, not decoded.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `vehicle_class_snitt_positions` | 00962a3b 00bf6989 007a4590 00799350 | `docs/VEHICLE_CLASS_SNITT_POSITIONS.md` | Repair the two `00BF6989` flow gaps and decode the string-keyed cut-camera table at `descriptor+BCh` |
| `vehicle_class_field_consumers` | 00749010 0095f500 004407a0 and the read sites of `+80h`..`+134h` | `docs/VEHICLE_CLASS_FIELD_CONSUMERS.md` | Which subsystem reads each descriptor offset this packet attributed, as xrefs |
| `vehicle_class_damage_sections` | 0087ca80 0087cd88 0087d229 0087d56d | `docs/VEHICLE_CLASS_DAMAGE_SECTIONS.md` | The section, fake-explosion and `Emberkek` element layouts, and where `ArmorIndexes` and `HPBlack` are read |
| `vehicle_class_ship_hod` | 00831840 0083266c 0083270e 008327d5 008328fe | `docs/VEHICLE_CLASS_SHIP_HOD.md` | The ship's `HoD` / `Traffic` / `Idle` crew-and-traffic tables and the `Camos` / `TextureRemaps` walk |

## State

| Routine | State |
| --- | --- |
| `00960230` | exported, analysed, reconstructed, build-tested, installed-file-checked |
| `0087CA80` | exported, analysed, installed-file-checked |
| `00831840` | exported, analysed (key table only), installed-file-checked |
| `007D1F70` | exported, analysed (key table only), installed-file-checked |
| `00749210`, `0074D4A0`, `00700E40`, `006D0B80` | exported, analysed, installed-file-checked |

`bsp::read_vehicle_class_base_fields_00960230` is reconstructed and build-tested; it is not a
binary-compatible replacement and reproduces neither the refcount traffic nor the pooled string
storage. One focused case was added to `tests/math_tests.cpp` for the `SpecRole` comparison
asymmetry.
