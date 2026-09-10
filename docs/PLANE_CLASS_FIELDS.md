# Plane class descriptor field reader

Addresses: 007D1F70 0042E740 006EA910 004B3D20 007C3510 007CCCA0 007D1D30 00871BA0 009536B0

`docs/VEHICLE_CLASS_FIELDS.md` reconstructed the descriptor base reader `00960230` and tabulated,
but did not decode, the plane family's override `007D1F70` `BSP_PlaneClass_ReadLuaFields`. This
packet decodes it in full: the calling convention, the 70 key paths in body order over its 77
`GetByName` sites, the nested container layouts the earlier tabulation guessed at, the three
defaults it computes rather than reads, and the shipped-file counts for plane rows. The per-key
table, the shipped counts and both key-gap lists are in `reports/plane_class_fields.json`; the
schema, the offsets and the reader sequence are in `include/bsp/plane_class_fields.hpp` and
`src/plane_class_fields.cpp`.

## Calling convention

`007D1F70` is `__thiscall(plane descriptor, LuaObject* row)` with `RET 4` at `007D3E4F`. The
prologue takes the descriptor in `ECX` (`MOV ESI,ECX` at `007D1F99`) and the row from the stack
(`MOV EDI,[ESP+0x16C]` at `007D1F8F`), then forwards the row to `00960230` at `007D1F9F` before its
own first key. `ESI` holds the descriptor and `EDI` the row for the whole body; unlike the ship
reader, neither is reloaded, so every `BSP_LuaObject_GetByName` in the body has the row as its
receiver. The descriptor is `60Ch` bytes for every plane leaf and the base reader stops at `+134h`,
so this reader owns `+138h`..`+608h` with nothing in between.

Nine call sites reach it through the vtable slot `+8h`: `007D1E50` and `007CFA30` install the base
plane vtable at `00D05EAC`, and the eight leaf constructors `BSP_VehicleClass_ConstructReconPlane`
`00951AA0`, `..SmallReconPlane` `009536E0`, `..LargeReconPlane` `00953770`, `..Fighter` `00951AF0`,
`..DiveBomber` `00951B40`, `..TorpedoBomber` `00951C20`, `..Kamikaze` `00951D00` and
`..LevelBomber` `00951DE0` install their own.

## What the earlier tabulation got wrong or left open

The plane rows in `docs/VEHICLE_CLASS_FIELDS.md` were extracted by pairing key strings with the
next store. That is right for the flat keys, and every flat offset it listed is confirmed here. It
left five things undecided, and one of its entries is misleading:

* **`WheelHeight` and `GroundPitch` are one gate, not two keys.** `007D29B8` probes `WheelHeight`
  and `007D29E8` probes `GroundPitch`; only when both are non-nil does `007D2A5E` set the byte at
  `+1F8h` to 1 and the reader convert both values. A row with one of the two writes neither slot
  and does not clear the flag, so the descriptor keeps whatever its constructor left. The
  tabulation's `GroundPitch +1F8h` entry is that flag byte, not the value.
* `TravelSpeed` has a second, unnamed slot. `007D2406` writes `+190h` as the tuning float at `+334h`
  times the value the row gave. No Lua key reaches `+190h`.
* `Accel` is post-processed, and the post-processing writes back into shared state. See below.
* `+218h` is not part of `EngineFireEfx`. After that key's block, `007D315C` builds the literal
  `"LowPlaneAlt"` and `007D3191` resolves it into `+218h`. No data file controls that slot.
* The three `PartAnims` sub-tables are `10h`-byte records of four fields, not scalars, and the
  fourth field is computed from the first two.
* `BayDoor` at the row level defaults as a group: a missing or non-table value writes all three
  slots at `007D3550`, which is the only block in this reader that does so.

## Key table

`Slot` is the descriptor offset. `Plane rows` is how many of the 72 shipped plane rows carry the
path. `Site` is the first `GetByName`; the seven keys marked twice are fetched again to convert.

| Key path | Conversion | Slot | Site | Default | Plane rows |
| --- | --- | --- | --- | --- | --- |
| `ShortName` | NativeString | `+138h` | `007D1FB0` | - | 72 |
| `NumEngines` | NumberToInt | `+140h` | `007D204D` | - | 72 |
| `JetEngines` | BooleanOr | `+144h` | `007D208B` | false | 8 |
| `Accel` | NumberScaled | `+164h` | `007D20C6` | - | 72 |
| `YDrag` | Number | `+170h` | `007D2150` | - | 72 |
| `XDrag` | Number | `+174h` | `007D2189` | - | 72 |
| `ExtRotAccel` | Number | `+178h` | `007D21C2` | - | 72 |
| `StallRotAccel` | Number | `+17Ch` | `007D21FB` | - | 72 |
| `WaterRotAccel` | Number | `+180h` | `007D2234` | - | 72 |
| `AirBrakeDrag` | Number | `+1DCh` | `007D226D` | - | 72 |
| `WheelBrake` | Number | `+1E0h` | `007D22A6` | - | 72 |
| `BombControlLimit` | Number | `+15Ch` | `007D22DF` | - | 72 |
| `BombDelay` | Number | `+1F4h` | `007D2318` | - | 72 |
| `StallSpd` | Number | `+184h` | `007D2351` | - | 72 |
| `MaxSpd` | Number | `+188h` | `007D238A` | - | 72 |
| `TravelSpeed` | NumberAndProduct | `+18Ch`, `+190h` | `007D23C3` | - | 72 |
| `SwimHeight` | Number | `+194h` | `007D2413` | - | 72 |
| `MinWaterSpd` | Number | `+198h` | `007D244C` | - | 72 |
| `MaxWaterSpd` | Number | `+19Ch` | `007D2485` | - | 72 |
| `WaterDecel` | Number | `+1A0h` | `007D24BE` | - | 72 |
| `WaterUnSpring` | Number | `+1A4h` | `007D24F7` | - | 72 |
| `RollSpd` | Number | `+1A8h` | `007D2530` | - | 72 |
| `PitchSpd` | Number | `+1ACh` | `007D2569` | - | 72 |
| `YawSpd` | Number | `+1B0h` | `007D25A2` | - | 72 |
| `TurnRollSpd` | Number | `+1C8h` | `007D25DB` | - | 72 |
| `YawLimitAngle` | Number | `+1CCh` | `007D2614` | - | 72 |
| `PitchLimitAngle` | Number | `+1D0h` | `007D264D` | - | 72 |
| `YawRollRatio` | Number | `+1B4h` | `007D2686` | - | 72 |
| `SlideRatio` | Number | `+1B8h` | `007D26BF` | - | 72 |
| `RollAccel` | Number | `+1BCh` | `007D26F8` | - | 72 |
| `PitchAccel` | Number | `+1C0h` | `007D2731` | - | 72 |
| `YawAccel` | Number | `+1C4h` | `007D276A` | - | 72 |
| `KameraMogotte` | NumberOr | `+168h` | `007D27A3` | `DAT_00CE38B8` = 10.0f | 72 |
| `KameraFolotte` | NumberOr | `+16Ch` | `007D27E6` | `DAT_00CF87C8` = 2.0f | 72 |
| `DragPitchRatio` | Number | `+1D4h` | `007D2829` | - | 72 |
| `NegativePitchRatio` | Number | `+1D8h` | `007D2862` | - | 72 |
| `TurnRoll` | Number | `+25Ch` | `007D289B` | - | 72 |
| `TurnRollLeader` | Number | `+260h` | `007D28D4` | - | 72 |
| `RollMaxforceLimit` | Number | `+274h` | `007D290D` | - | 72 |
| `PitchMaxforceLimit` | Number | `+278h` | `007D2946` | - | 72 |
| `TurnCircleRadius` | Number | `+268h` | `007D297F` | - | 72 |
| `WheelHeight` | NumberGated | `+1FCh` | `007D29B8`, `007D2A65` | untouched | 51 |
| `GroundPitch` | NumberGated | `+1F8h`, `+200h` | `007D29E8`, `007D2A9E` | untouched | 51 |
| `WaterPitch` | Number | `+204h` | `007D2AD7` | - | 72 |
| `GlideRate` | Number | `+208h` | `007D2B10` | - | 72 |
| `CarrierBased` | BooleanOrFalse | `+160h` | `007D2B4C` | false | 42 |
| `DropAngle` | Number | `+1F0h` | `007D2B93` | - | 72 |
| `KamikazeBulletClass` | IntegerAndClass | `+20Ch`, `+210h` | `007D2BCA` | - | 72 |
| `Wreck` | WreckClassOrNull | `+21Ch` | `007D2C7A`, `007D2CB4` | 0 | 0 |
| `ExplosionEfx` | EffectHandle | `+214h` | `007D2D39`, `007D2D77` | untouched | 72 |
| `ShellsEfx` | EffectHandle | `+234h` | `007D2E27`, `007D2E65` | untouched | 53 |
| `EngineEfxes` | EngineEffectVector | `+250h` | `007D2F15` | empty | 4 |
| `EngineFireEfx` | EffectHandle | `+220h` | `007D3068`, `007D30A6` | untouched | 72 |
| `DamageSmokeEfx` | EffectHandleVector | `+224h` | `007D323E` | empty | 72 |
| `WingTipEfx` | EffectHandle | `+5A4h` | `007D3382`, `007D33C0` | untouched | 53 |
| `BayDoor` | Table | - | `007D3473` | all three below | 17 |
| `BayDoor.OpenAngle` | Number | `+5A8h` | `007D34A5` | `DAT_00CE7D1C` = -0.6981317f | 17 |
| `BayDoor.ClosedAngle` | Number | `+5ACh` | `007D34E1` | `DAT_00CE7D20` = 0.6981317f | 17 |
| `BayDoor.TimeToOpen` | Number | `+5B0h` | `007D351D` | `DAT_00D7A2F0` = 0.1f | 17 |
| `BowWaves` | EffectRecordVector | `+550h` | `007D3589` | empty | 71 |
| `ParaReload` | NumberReciprocalOrZero | `+5B4h` | `007D371A` | 0.0f | 4 |
| `GearsPullTime` | NumberOrKindDefault | `+5F8h` | `007D37A4` | 2.0f, or 4.0f by kind | 0 |
| `PartAnims` | Table | - | `007D37E8` | - | 60 |
| `PartAnims.Gears` | PartAnim | `+5C8h` | `007D381A` | untouched | 59 |
| `PartAnims.Wings` | PartAnim | `+5D8h` | `007D3990` | untouched | 10 |
| `PartAnims.BayDoor` | PartAnim | `+5E8h` | `007D3B06` | untouched | 16 |
| `TurboTime` | NumberOr | `+5FCh` | `007D3CB6` | 0.0f | 40 |
| `TurboRechargingTime` | NumberOr | `+600h` | `007D3CF6` | 0.0f | 38 |
| `TurboStrength` | NumberOr | `+604h` | `007D3D36` | 1.0f | 40 |
| `TurboControlLimit` | NumberOr | `+608h` | `007D3D73` | 1.0f | 40 |

## The three computed values

### `Accel`, and a write back into shared state

`007D20F3` fetches the tuning singleton `0042E740` returns and compares its float at `+31Ch`
against `DAT_00D7A24C` = 1.0f. When the factor is above 1.0f, `007D2115`..`007D2127` replaces the
slot with `tuning[+320h] * tuning[+31Ch] * Accel`. When it is not, `007D213C` stores 1.0f **into
the tuning object** and leaves `Accel` as the row gave it. That is a per-class reader mutating a
process-wide object, so the first plane class loaded with a low factor changes the factor for every
class after it. Whether that is a guard or a defect is not established here.

`TravelSpeed` uses the same singleton without the test: `007D23F5` writes `+190h` as
`tuning[+334h] * TravelSpeed`, always.

### `GearsPullTime`, whose default depends on the leaf class

`007D375C` seeds `+5F8h` with `DAT_00CF87C8` = 2.0f. `007D3773` then calls the descriptor's own
vtable slot `+18h`, `IsKindOf`, with `0x10`, and `007D3782` calls it again with `0x16`; either
answering true raises the seed to `DAT_00CE3D34` = 4.0f. `docs/VEHICLE_CLASS_DESCRIPTORS.md` maps
kind `10h` to `LevelBomber` and `16h` to `LargeReconPlane`, so those two leaves pull their gear in
four seconds and every other plane in two. The seeded value is then the `GetFloatOrDefault`
fallback at `007D37BD`, so a row that supplies the key overrides it. No shipped plane row does.

The ReconPlane leaf's `IsKindOf` at `009536B0` accepts `14h`, `0Fh`, `5` and `4`, so neither test
passes for it. That routine has **no Ghidra function**; its last instruction is the `RET 4` at
`009536D2` and `int3` padding starts at `009536D5`. It is reported for definition, not renamed.

### The `PartAnims` records

Each of `Gears`, `Wings` and `BayDoor` is a `10h`-byte record, and all three are built by the same
block. Taking `Gears` at `007D3838`:

| Record offset | Content | Evidence |
| --- | --- | --- |
| `+0h` | element `[1]` | `00B677E0(buf,1)` at `007D3860`, `GetNumber` at `007D386F`, store `007D38C3` |
| `+4h` | element `[2]` | `00B677E0(buf,2)` at `007D3843`, `GetNumber` at `007D387A`, store `007D38D3` |
| `+8h` | `abs([1] - [2])` | `FSUBP` at `007D3893`, the `-0.0f` negation at `007D38AD`, store `007D38D9` |
| `+Ch` | a present flag | `MOV byte ptr [ESP+0x74],1` at `007D38DF`, stored as a dword at `007D38EF` |

Element `[3]`, when `IsNumber` accepts it at `007D392D`, overwrites `+8h` at `007D3968`. The flag
at `+Ch` is written as a full dword whose upper three bytes come from the stack slot, so only its
low byte is meaningful. A missing sub-table leaves the whole record untouched.

`Wings` is the same block at `007D39AE` writing `+5D8h`..`+5E4h`, and `BayDoor` at `007D3B24`
writing `+5E8h`..`+5F4h`.

## The other nested containers

* **`EngineEfxes`**, `+250h`. `007D2F26` requires a table, then an indexed walk that stops at the
  first element failing `IsInteger`, so a gap truncates the vector. Each id becomes a handle
  through `00870CD0`, `007C3510` wraps it in a `44h`-byte record and `007CCCA0` appends that to a
  `{data, count, capacity}` vector, doubling through `007C9170`. The record's fields were not
  decoded; `007C3510` seeds several of them from 1.0f.
* **`DamageSmokeEfx`**, `+224h`. The same indexed walk, but each handle goes straight into the
  refcounted pointer vector through `004D9C00` at `007D32F9`, the same append the ship reader uses.
* **`BowWaves`**, `+550h`. This walk ends on the first **nil** rather than on the first
  non-integer, so a non-integer element is still read through `GetInteger`. `007D1D30` appends a
  `10h`-byte record whose begin, end and capacity live at `+554h`, `+558h` and `+55Ch`. Only the
  record's first dword, the handle, is initialised: `007D3608` zeroes that dword and `007CD2F0`
  copies the remaining twelve bytes from a stack slot the reader never writes.
* **`BayDoor`** at the row level is three plain numbers, defaulted as a group.

## Defaults

| Default source | Keys |
| --- | --- |
| `FLDZ`, so 0.0f | `TurboTime`, `TurboRechargingTime` |
| `FLD1`, so 1.0f | `TurboStrength`, `TurboControlLimit` |
| `PUSH 0`, so false | `JetEngines` |
| an `IsNil` else-branch | `CarrierBased` clears the byte, `Wreck` stores 0, `ParaReload` stores 0.0f |
| a global float | `KameraMogotte` 10.0f, `KameraFolotte` 2.0f, the three `BayDoor` angles |
| computed | `GearsPullTime`, 2.0f or 4.0f by `IsKindOf` |
| nothing | the seven twice-fetched keys, and `WheelHeight` / `GroundPitch` behind their gate |

Seven keys are read twice, once to test and once to convert: `WheelHeight`, `GroundPitch`, `Wreck`,
`ExplosionEfx`, `ShellsEfx`, `EngineFireEfx` and `WingTipEfx`. Of those, only `Wreck` clears its
slot when the test fails; the four effect keys and the two ground keys leave whatever the base
reader or the constructor put there.

`ParaReload` lands as `1.0f / value` (`FLD1` then `FDIVRP` at `007D3743`), the same shape the ship
reader's `LSReload` uses, so a shipped `0` produces an infinity while an absent key produces 0.0f.

One rule discards a value the row supplied. `007D3DA4` re-reads `TurboStrength` after all four
turbo keys are stored and, unless it is strictly above 1.0f, zeroes `TurboTime` at `007D3DB8`.
Because the `TurboStrength` default is itself 1.0f, a row that gives a `TurboTime` but no
`TurboStrength` gets no turbo. `tests/math_tests.cpp` pins that gate.

## Installed-file check

`Scripts/datatables/autoload/vehicleclasses.lua` was parsed read-only by an independent Lua-subset
parser written for this packet (`local/planecheck.py` in the worktree). It accepts `VehicleClass[n]
= {...}` statements, table constructors, `["str"]` and `[int]` keys, numbers, quoted strings,
booleans, `nil`, `NAME(...)` call expressions and comments, and treats `--[[` as nesting, which the
two earlier packets found is necessary for the file to balance. A plane row is one whose `Type` is
`ReconPlane`, `SmallReconPlane`, `LargeReconPlane`, `Fighter`, `DiveBomber`, `TorpedoBomber`,
`Kamikaze` or `LevelBomber`.

| Measure | Value |
| --- | --- |
| rows in `VehicleClass` | 633 |
| plane rows | 72 |
| distinct key paths on plane rows | 165 |
| key paths this reader consumes | 70 |
| `GetByName` sites in the body | 77 |
| this reader expects, no plane row provides | 2 |
| shipped on plane rows, no reader consumes by name | 24 |

The 633 matches the two earlier packets exactly. The 72 is a stronger check than the ship packet
had: `docs/VEHICLE_CLASS_DESCRIPTORS.md` lists per-leaf row counts of 0, 5, 4, 28, 9, 9, 6 and 11
for the eight plane constructors, which sum to 72 and match the per-`Type` breakdown this parser
produced. `ReconPlane` really has no shipped row, so its `IsKindOf` never runs on shipped data.

The file is 3,349,364 bytes in this install; `docs/VEHICLE_CLASS_FIELDS.md` recorded 3,158,242 for
the same path. The row count is identical, so the two packets read copies that differ in trailing
content rather than in data. That is recorded, not resolved.

Only two keys the reader asks for go unprovided: `Wreck`, so no plane ever resolves a wreck class
and `+21Ch` is always 0, and `GearsPullTime`, so the kind-based default above is the only value
`+5F8h` ever takes. Both agree with the earlier packet's list.

Of the 24 shipped plane keys no reader in this family consumes by name, the largest are
`ArmorIndexes`, `Race`, `TurnRollLowLimit` and `Type` (72 rows each, and `Type` and `Race` are read
by the factory `00964790` rather than by a field reader), the two `DogFight` distances and the
three `Strafe` distances (70 each), `UnitlibViewDistance` (65), `HP_Realistic` (36), `Unlock` (27),
the four `PlatformDirections` legs (22) and `UnlockID` (17). The AI distance block is the most
conspicuous: five keys shipped on 70 of 72 plane rows that no descriptor field reader names.

## Callers and callees

The nine call sites are listed under the calling convention above. `007D1F70` calls 31 routines.
Game-side: `00960230` (the base reader), `0042E740`, `006EA910`, `004B3D20`, `007C3510`,
`007CCCA0`, `007D1D30`, `00870CD0`, `00871BA0`, `004D9C00`, `00419CC0`, `0041DD40` and `0041E870`.
The rest are the `BSP_LuaObject_*` wrappers and two CRT helpers, read but not renamed.

`python tools/bsp.py ghidra flow 007D1F70` reports 1853 listed instructions and **0 gaps**.

## Reconstruction

`include/bsp/plane_class_fields.hpp` and `src/plane_class_fields.cpp` carry the 70-key schema as
data in the same shape `ship_class_fields.hpp` uses, the descriptor and record offsets as
documented structs, the five post-processing rules as pure functions, and the reader sequence over
`bsp::VehicleClassFieldHost` for the Lua half plus a new `bsp::PlaneClassFieldHost` with one method
per remaining native call site. Nothing from `vehicle_class_fields.hpp` or `ship_class_fields.hpp`
is redefined.

The header is a semantic reconstruction. It is not ABI compatible with the descriptor, it does not
reproduce the refcount traffic or the SEH states, and it has not been run against the game.

## Uncertainties

* The `44h`-byte `EngineEfxes` record `007C3510` builds was not decoded, only that it wraps one
  effect handle and seeds several floats from 1.0f.
* The `BowWaves` record's trailing twelve bytes come from an uninitialised stack slot. Either
  `007CD2F0` is copying fields the caller is expected to have set and this caller does not, or
  those fields are dead. Not settled.
* The tuning object `0042E740` returns is `6D0h` bytes loaded from `Scripts\global\luaMW_init.lua`
  by `007E2A20` and has 141 call sites. Only `+31Ch`, `+320h` and `+334h` are touched here and none
  of the three is named.
* The `Accel` path writes 1.0f back into `tuning[+31Ch]`. It is a per-class read mutating shared
  state and its intent is not established.
* `+218h` is filled from the `"LowPlaneAlt"` literal, so no data file controls it and nothing here
  says what reads it.
* The plane-row rule comes from the `Type` strings, though the 72 it selects matches the descriptor
  doc's per-leaf counts exactly.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `plane_engine_effect_record` | 007c3510 007ccca0 007c9170 007c34b0 | `docs/PLANE_ENGINE_EFFECT_RECORD.md` | Decode the `44h`-byte EngineEfxes record and find which subsystem walks the vector at plane descriptor `+250h` |
| `plane_bow_wave_record` | 007d1d30 007cd2f0 007d1b50 | `docs/PLANE_BOW_WAVE_RECORD.md` | Decode the `10h`-byte BowWaves record, say what its three trailing dwords mean and where `+550h` is read |
| `game_tuning_singleton` | 0042e740 007e2a20 | `docs/GAME_TUNING_SINGLETON.md` | Decode the `6D0h`-byte luaMW_init tuning object, name the offsets its 141 call sites use and settle whether the `+31Ch` write-back is a guard |
| `plane_ai_distance_keys` | 007d1f70 | `docs/PLANE_AI_DISTANCE_KEYS.md` | Find the reader that consumes `DogFightAimDistance`, `DogFightShootDistance`, the three `Strafe` distances and `TurnRollLowLimit`, shipped on 70 of 72 plane rows and named by no descriptor field reader |

## State

| Routine | State |
| --- | --- |
| `007D1F70` | exported, analysed, reconstructed, build-tested, installed-file-checked |
| `0042E740`, `006EA910`, `004B3D20`, `00871BA0` | exported, analysed for their role, name recorded |
| `007C3510`, `007CCCA0`, `007D1D30` | exported, analysed from their call sites, name recorded |
| `009536B0` | read only, no Ghidra function; end `009536D2`, reported for definition |
| `00870CD0`, `004D9C00`, `00960230`, `007E2A20`, `007CD2F0` | read only, not renamed here |
