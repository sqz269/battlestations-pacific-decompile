# Sensor table data

Addresses: `008082A0`, `008085C0`, `00808734`, `00960230`, `00808F90`, `00B67C40`, and the key strings
`00D08FE4`, `00D08FD8`, `00D08FC8`, `00D08FB4`, `00D08F9C`, `00D08F88`, `00D08F70`, `00D08F64`,
`00D08F5C`, `00D08F54`, `00D08F48`, `00D08F38`, `00D08F24`, `00D08F10`, `00D08EFC`, `00D08EE4`,
`00D08ED8`, `00D08ED0`, `00D08EC8`, `00D08FF8`, `00CFA1A8`, `00CE93F8`.

`docs/GUNNERY_RECON_DETECTION.md` recovered rule (c) of the contact list and
`docs/SENSOR_TABLES.md` recovered the producer that fills its tables. Neither had the shipped
rows, so the rule executed against nothing. This document supplies them: the Lua key names as
recovered strings, the mapping from each key to the `1Ch` entry, the per-vehicle-class
association, and the dumped rows of all twelve `ReconClass` ids in both shipped variants.

Names in `include/bsp/sensor_table_data.hpp` are hypotheses. The **Lua key names are not**: each
is a literal in the image, cited below at its address.

## 1. The Lua key names are recovered strings

Every key the loader reads is a `PUSH <imm>` of a `.rdata` string, read back with
`bsp.py ghidra bytes`. These are the authors' identifiers, not descriptions of mine.

| string address | literal | pushed at | role |
| --- | --- | --- | --- |
| `00D08FE4` | `Details` | `008082DA` | the row's one required sub-table |
| `00D08FD8` | `ObserverAir` | `008083CD` | observer category 0 |
| `00D08FC8` | `ObserverSurface` | `008083ED` | observer category 1 |
| `00D08FB4` | `ObserverPeriscopeIn` | `0080840E` | observer category 2 |
| `00D08F9C` | `ObserverPeriscopeOut` | `0080842A` | observer category 3 |
| `00D08F88` | `ObserverUnderwater` | `00808446` | observer category 4 |
| `00D08F70` | `ObserverDeepUnderwater` | `00808462` | observer category 5 |
| `00D08F64` | `SubjectAir` | `00808531` | subject category 0 |
| `00D08F38` | `SubjectSurface` | `0080868A` | subject category 1 |
| `00D08F24` | `SubjectPeriscopeIn` | `008086AE` | subject category 2 |
| `00D08F10` | `SubjectPeriscopeOut` | `008086D2` | subject category 3 |
| `00D08EFC` | `SubjectUnderwater` | `008086F6` | subject category 4 |
| `00D08EE4` | `SubjectDeepUnderwater` | `00808715` | subject category 5 |
| `00D08ED8` | `GUIRange` | `00808734` | the map ring list, per observer |
| `00D08F5C` | `Dist` | `008085C0` | entry `+0h` and `+4h` |
| `00D08F54` | `Gain` | `00808604` | entry `+8h` |
| `00D08F48` | `MaxLevel` | `00808644` | entry `+0Ch` |
| `00D08ED0` | `Angle` | `00808992` (entry), `00808875` (ring) | `+14h`/`+18h` |
| `00D08EC8` | `RawType` | `008089F4` (entry), `008088C2` (ring) | `+10h` |
| `00CFA1A8` | `Range` | `008087C2` | ring `+8h` |
| `00CE93F8` | `Color` | `0080881C` | ring `+10h`..`+1Ch` |
| `00D08FF8` | `ReconClass` | `009623E4` | the vehicle-class field, and the global table |

`Dist` and `Range` are different keys: `Dist` names a detection row, `Range` a map ring. Both
occur in the shipped scripts with exactly that split.

## 2. Key to `1Ch` entry, read from the writer

`008085C0`..`00808A6A` fills seven dwords at `[ESP+74h]` and copies them with
`00808A68 MOV ECX,7` / `00808A71 REP MOVSD`, so the entry is seven dwords and nothing else.
Every row below was re-read instruction by instruction for this packet; it agrees with
`docs/SENSOR_TABLES.md` §2.

| offset | stack slot | Lua key | transform | written at |
| --- | --- | --- | --- | --- |
| `+0h` | `[ESP+74h]` | `Dist` | `float(Dist)` | `008085E5` |
| `+4h` | `[ESP+78h]` | `Dist` | `float(float(Dist)^2)`, `008085E9 FLD` / `008085F1 FMUL ST0` | `008085FB` |
| `+8h` | `[ESP+7Ch]` | `Gain` | `float(Gain * 0.5)`, the double `0.5` at `00D7A280` | `0080863B` |
| `+0Ch` | `[ESP+80h]` | `MaxLevel` | `0` -> `0.0f` (`00808975 XORPS`), `1` -> `0.25f` (`00CE3868`), else `1.0f` (`00D7A24C`) | `0080897C` |
| `+10h` | `[ESP+84h]` | `RawType` | `int`, `00B66290 GetInteger` | `00808A23` |
| `+14h` | `[ESP+88h]` | `Angle` | `float(Angle)`, or `0.0f` when nil | `008089E5` / `008089C6` |
| `+18h` | `[ESP+8Ch]` | `Angle` | `1` when the key is not nil, else `0` (`008089BA` `IsNil`) | `008089EC` / `008089CF` |

`MaxLevel` is read with `00B66380 GetIntegerOrDefault` and the default `2` pushed at `0080865A`,
so a row that omits the key behaves as `2`. `Dist`, `Gain` and `RawType` have no default: the
shipped rows always carry them.

Verified constants: `00D7A280` is `00 00 00 00 00 00 E0 3F` = `0.5` (double); `00D7A24C` is
`00 00 80 3F` = `1.0f`; `00CE3868` is `00 00 80 3E` = `0.25f`.

The list the entry lands in is `record + 8 + (observer * 8 + subject) * 0Ch`, from
`008085AA MOV ECX,[ESP+10h]` / `008085AE MOV EDX,[ESP+14h]` / `LEA EAX,[EDX+ECX*8]` /
`LEA EAX,[EAX+EAX*2]` / `LEA EBX,[ECX+EAX*4+8]`. On `count == capacity` (`00808A37`) the list
grows to `max(2 * capacity, 1)` through `00807BD0`, and the entry is written at
`begin + count * 1Ch` (`00808A58 LEA ECX,[EAX*8]` / `SUB ECX,EAX` / `LEA EDI,[EDX+ECX*4]`).

### The `GUIRange` ring, `20h`

`00808734` matches the key, `008087A4`..`008087B2` picks the destination and `00808903` pushes
through `00807F60`. The record base is `[ESP+54h]` (`008088FC LEA EAX,[ESP+54h]`):

| offset | Lua key | transform | written at |
| --- | --- | --- | --- |
| `+0h` | `RawType` | `int` | `008088EB` |
| `+4h` | `Angle` present | `1`/`0` | `008088BD` / `008088A9` |
| `+8h` | `Range` | `float(Range)`, unsquared | `00808807` |
| `+0Ch` | `Angle` | `float(Angle)` or `0.0f` | `008088B9` / `008088A3` |
| `+10h`..`+1Ch` | `Color` | four floats, Lua array indices 1..4 | `0080884E`..`00808864` |

`00808844` calls `00B67C40`, named `BSP_LuaObject_GetFloat4` by this packet: it indexes the Lua
array at 1, 2, 3 and 4 through `00B67720` (`00B67C45`, `00B67C82`, `00B67CBC`, `00B67CF6`),
converts each with `00A67770`, stores `FSTP float ptr [EDI + k*4]` (`00B67C69`, `00B67C9D`,
`00B67CD7`, `00B67D11`) and returns the caller's buffer (`00B67D30 MOV EAX,EDI`). It has three
call sites - `00808844`, `00833980` in `BSP_ShipClass_ReadLuaFields` and `00B1BA00` - of which
only the first was read here. The four
words at ring `+10h` are therefore **floats**, and the three shipped constants decode:
`COL_RED {1,0,0,1}`, `COL_GREEN {0,1,0,1}`, `COL_BLUE {0,0,1,1}`
(`scripts/global/luamw_init.lua` lines 25-27).

## 3. The per-vehicle-class association

`00960230 BSP_VehicleClass_ReadLuaFields` reads the vehicle-class row's `ReconClass`
(`009623E4` pushes the key, `00962401` `IsNil` skips an absent one, `0096240E` `GetInteger`
gives the id), passes it to `00808F90` at `00962419`, and stores the returned `2A8h` record at
`class+B4h` (`00962420` reads the previous one to release it). `00808F90` keeps one record per
id in the registry at `00F874E4`, so all classes sharing an id share one record and the Lua
table is read once per id per process. `008048A0` then reaches it as `[[unit+538h]+B4h]` and
indexes it with `sensor_list_index_008085aa(observerCategory, subjectCategory)`.

`00960230` and the VFS loader sit inside a peer orchestrator's territory. Both were **read
only**: nothing here is leased, renamed or annotated in that range.

Counted from this installation's `scripts/datatables/autoload/vehicleclasses.lua`
(**locally modified**, see §5), 633 `VehicleClass` rows carry 633 top-level `ReconClass` keys:

| id | script comment | vehicle classes | examples |
| --- | --- | --- | --- |
| 1 | Normal Ship (Cargo ship, Torpedoboat) | 22 | LSM, Elco PT Boat, US Tanker |
| 2 | Radar ship (Battleship, Carrier) | 97 | Lexington 1944, Yorktown 1944, Hermes 1941 |
| 3 | Sonar ship (Destroyer) | 31 | Allen M. Sumner 1945, Akizuki 1944, Fletcher 1943 |
| 4 | Ship with radar and sonar (Cruiser - TONE) | 1 | Tone 1944 |
| 5 | Normal plane | 65 | P-80 Shooting Star, TBM Avenger, F6F Hellcat |
| 6 | Recon plane | 8 | M6A Seiran, OS2U Kingfisher, PBY Catalina |
| 7 | Normal submarine | 9 | Kaiten, I400, Gato |
| 8 | Ground unit w/o radar | 305 | commandstation, house |
| 9 | Ground unit with high vision (Observation post) | 3 | watchtower, radiotower, fort |
| 10 | Ground unit with radar (Radar station) | 3 | radarstation |
| 11 | Ground unit with small radar (Airfield) | 6 | ctrltower |
| 12 | Units with no recon | 83 | obstacle, mavis, debrish |

The full 633-row map is in `reports/cc7_sensor_table_data.json` under
`vehicle_class_association.per_vehicle_class`.

## 4. What the shipped data does not contain

Three consequences follow from the dump and are worth stating because they bound what rule (c)
can do at all:

* **No shipped row names `DeepUnderwater`** - neither `ObserverDeepUnderwater` nor
  `SubjectDeepUnderwater` appears in either variant. A submarine that `00852B90` classifies as
  deep-running is invisible to every sensor in the game and detects nothing while it is there.
* **No shipped row can reach category 6** (`unclassified`, what `004F1740` returns), because no
  Lua key addresses it. A unit that keeps the base accessor neither detects nor is detected.
* **Only five of the seven observer categories are ever authored**: `Surface` (ten classes),
  `Air` (two: `ReconClass[5]` and `[6]`), and `PeriscopeIn` / `PeriscopeOut` / `Underwater`
  (`ReconClass[7]`, the submarine, alone).
* **`ReconClass[12]` authors `Dist = 0`, `Gain = 0`, `MaxLevel = 0` on all four rows**, so its
  entries convert to `{0, 0, 0, 0.0f, ...}` - the cap is `0.0f` and the gain `0.0f`. It is a
  deliberate "never detects" class, not an absence of rows.

The `Gain` values are all `1/1` except one row: `ObserverPeriscopeOut` -> `SubjectUnderwater` of
the submarine class, authored `1/5` (line 278 of the arcade file), giving `+8h = 0.1f`.

## 5. Provenance: what was read, and the modded-installation caveat

The rows come from the installed game, read only, never written:

| file | mtime | role |
| --- | --- | --- |
| `scripts/datatables/autoload/reconclasses.lua` | 2024-07-13 08:26 | picks the variant by `GameMode`, aliases it onto the global `ReconClass` |
| `scripts/datatables/classtables/arcade/reconclasses.lua` | 2024-07-13 08:26 | `ArcadeTable`, 467 lines |
| `scripts/datatables/classtables/realistic/reconclasses.lua` | 2024-07-13 08:26 | `RealisticTable`, 467 lines |
| `scripts/global/luamw_init.lua` | - | `COL_*` (lines 22-28) and `DEG` (line 434) |
| `gamemode.lua` (install root) | 2024-07-13 11:27 | `GameMode = 0`, so this install loads the arcade table |
| `scripts/datatables/autoload/vehicleclasses.lua` | 2026-05-09 21:52 | the id per vehicle class |

**The installation is modded.** `xlive.dll` is AlterBSP, and the BSPRM mod is present;
`gamemode.lua` carries a modder's note. Two things follow.

1. The three `reconclasses.lua` files carry `2024-07-13 08:26`, the timestamp shared by the
   untouched bulk of `scripts/datatables` (`animclasses`, `effects`, `stationaryclasses`, ...).
   Files that were changed locally carry visibly later stamps: `vehicleclasses.lua`
   2026-05-09, `classtables/arcade/{bulletclasses,deviceclasses}.lua` 2026-05-09,
   `classtables/realistic/bulletclasses.lua` 2025-06-02, `shipnames.lua` 2025-06-02.
   `mod_restore.bat` restores `interface`, `movies`, `mpak`, `sound` and the exe, and never
   touches `scripts/`. No pristine retail copy exists on this machine to diff against, so
   "install baseline" here means *same timestamp as the untouched bulk*, not *byte-identical to
   the retail release*. **The sensor rows themselves are install baseline on that evidence.**
2. **`vehicleclasses.lua` is not.** The §3 counts therefore describe this installation. The
   mechanism (`ReconClass` -> `00808F90` -> `class+B4h`) is independent of the file's contents.

The extraction is `local/extract_reconclasses.py` (ignored, not committed): a parser for the Lua
subset these files use - scalar assignments, table constructors, `+ - * /`, `DEG()` - evaluated
with the `COL_*` and `DEG` definitions above, then `008082A0`'s arithmetic applied in `float32`.
Its output for `ArcadeTable[3]` reproduces, value for value, the table `docs/SENSOR_TABLES.md`
§5 derived independently by hand.

**The two variant files are identical except for the table name and fifteen base-range
scalars.** Every row, key, nesting and comment is the same. The scalars:

| scalar | arcade | realistic |
| --- | --- | --- |
| `NormalShip_recon_VisionRange` | 4000 | 10000 |
| `RadarShip_recon_RadarRange` | 6000 | 20000 |
| `SonarShip_recon_SonarRange` | 1000 | 4000 |
| `RadarAndSonarShip_recon_RadarRange` | 5000 | 10000 |
| `RadarAndSonarShip_recon_SonarRange` | 1000 | 4000 |
| `NormalPlane_recon_VisionRange` | 4000 | 10000 |
| `ReconPlane_recon_VisionRange` | 5000 | 10000 |
| `ReconPlane_recon_SonarRange` | 750 | 1000 |
| `NormalSub_recon_VisionRange` | 4000 | 9000 |
| `NormalSub_recon_SonarRange` | 1800 | 4000 |
| `GroundUnit_recon_VisionRange` | 4000 | 10000 |
| `GroundObserverUnit_recon_VisionRange` | 6000 | 10000 |
| `GroundRadarUnit_recon_RadarRange` | 10000 | 20000 |
| `GroundRadarUnit_recon_VisionRange` | 4000 | 10000 |
| `SmallRadarUnit_recon_RadarRange` | 5000 | 20000 |

`RRT_VISION = 0`, `RRT_RADAR = 1`, `RRT_SONAR = 2` are defined at the top of both files; they
are the `RawType` at entry `+10h` and the bit `008048A0` tests against the mask.

## 6. Proven versus assumed

**Read from shipped data** (the tables in §7, and every value in
`reports/cc7_sensor_table_data.json` under `authored_tables`): the `Dist`, `Gain`, `MaxLevel`,
`RawType` and `Angle` of all 67 detection rows and the `Range`, `Color`, `RawType` and `Angle`
of all 27 ring rows, per variant; the twelve ids; the observer/subject key each row sits under.

**Derived by the producer, byte-proven** (§2): the squaring, the `* 0.5`, the `MaxLevel` -> cap
mapping, the nil-`Angle` rule and the list index. These are computations, not data.

**Producer default, never exercised by shipped data**: the `MaxLevel` default of `2`
(`0080865A`). Every shipped row carries the key explicitly, so the default never fires here.

**Assumed**: that these script files are the retail ones. See §5; the evidence is timestamps,
not a diff.

**Not established**: anything at run time. `docs/RECON_SLOT_LISTS.md` §10 records that
`bsp_game.exe` fakes the recon lists, so no path the executable runs reaches `008082A0`,
`00808F90` or `008048A0`. Nothing in this document is game-validated.

## 7. The dumped tables

`Dist` and ring `Range` are shown as *arcade / realistic*; every other column is identical
between the two variants. `+0Ch` and `+8h` are what `008082A0` stores, not what the script
authors. Colours are the ring's four floats.

### `ReconClass[1]` - Normal Ship (Cargo ship, Torpedoboat)

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Surface` -> `Air` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `PeriscopeOut` | 1200 / 3000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |

Rings: `Surface` 4000/10000 vision rgba(0,0,1,1)

### `ReconClass[2]` - Radar ship (Battleship, Carrier)

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Surface` -> `Air` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Air` | 6000 / 20000 | 1 radar | 1 -> 0.25 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 6000 / 20000 | 1 radar | 1 -> 0.25 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `PeriscopeOut` | 1200 / 3000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |

Rings: `Surface` 4000/10000 vision rgba(0,0,1,1); `Surface` 6000/20000 radar rgba(1,0,0,1)

### `ReconClass[3]` - Sonar ship (Destroyer)

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Surface` -> `Air` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `PeriscopeIn` | 1000 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `PeriscopeIn` | 2500 / 10000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | 0.785398 (`DEG(45)`), true |
| `Surface` -> `PeriscopeOut` | 1000 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `PeriscopeOut` | 2500 / 10000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | 0.785398 (`DEG(45)`), true |
| `Surface` -> `Underwater` | 660 / 2640 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Underwater` | 1660 / 6640 | 2 sonar | 2 -> 1 | 1 -> 0.5 | 0.785398 (`DEG(45)`), true |

Rings: `Surface` 1000/4000 sonar rgba(0,1,0,1); `Surface` 2500/10000 sonar cone rgba(0,1,0,1); `Surface` 4000/10000 vision rgba(0,0,1,1)

### `ReconClass[4]` - Ship with radar and sonar (Cruiser - TONE)

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Surface` -> `Air` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Air` | 5000 / 10000 | 1 radar | 1 -> 0.25 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 5000 / 10000 | 1 radar | 1 -> 0.25 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `PeriscopeIn` | 1000 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `PeriscopeIn` | 2500 / 10000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | 0.785398 (`DEG(45)`), true |
| `Surface` -> `PeriscopeOut` | 1000 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `PeriscopeOut` | 2500 / 10000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | 0.785398 (`DEG(45)`), true |
| `Surface` -> `Underwater` | 660 / 2640 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Underwater` | 1660 / 6640 | 2 sonar | 2 -> 1 | 1 -> 0.5 | 0.785398 (`DEG(45)`), true |

Rings: `Surface` 1000/4000 sonar rgba(0,1,0,1); `Surface` 2500/10000 sonar cone rgba(0,1,0,1); `Surface` 4000/10000 vision rgba(0,0,1,1); `Surface` 5000/10000 radar rgba(1,0,0,1)

### `ReconClass[5]` - Normal plane

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Air` -> `Air` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Air` -> `Surface` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |

Rings: `Air` 4000/10000 vision rgba(0,0,1,1)

### `ReconClass[6]` - Recon plane

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Air` -> `Air` | 5000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Air` -> `Surface` | 5000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Air` -> `PeriscopeIn` | 750 / 1000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Air` -> `PeriscopeOut` | 750 / 1000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Air` -> `Underwater` | 495 / 660 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |

Rings: `Air` 750/1000 sonar rgba(0,1,0,1); `Air` 5000/10000 vision rgba(0,0,1,1)

### `ReconClass[7]` - Normal submarine

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Surface` -> `Air` | 4000 / 9000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 4000 / 9000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `PeriscopeIn` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `PeriscopeOut` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Underwater` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `PeriscopeIn` -> `Surface` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `PeriscopeIn` -> `PeriscopeIn` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `PeriscopeIn` -> `PeriscopeOut` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `PeriscopeIn` -> `Underwater` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `PeriscopeOut` -> `Air` | 4000 / 9000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `PeriscopeOut` -> `Surface` | 4000 / 9000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `PeriscopeOut` -> `PeriscopeIn` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `PeriscopeOut` -> `PeriscopeOut` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `PeriscopeOut` -> `Underwater` | 1800 / 4000 | 2 sonar | 2 -> 1 | 0.2 -> 0.1 | nil -> 0, false |
| `Underwater` -> `Surface` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Underwater` -> `PeriscopeIn` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Underwater` -> `PeriscopeOut` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Underwater` -> `Underwater` | 1800 / 4000 | 2 sonar | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |

Rings: `Surface` 1800/4000 sonar rgba(0,1,0,1); `Surface` 4000/9000 vision rgba(0,0,1,1); `PeriscopeIn` 1800/4000 sonar rgba(0,1,0,1); `PeriscopeOut` 1800/4000 sonar rgba(0,1,0,1); `PeriscopeOut` 4000/9000 vision rgba(0,0,1,1); `Underwater` 1800/4000 sonar rgba(0,1,0,1)

### `ReconClass[8]` - Ground unit w/o radar

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Surface` -> `Air` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |

Rings: `Surface` 4000/10000 vision rgba(0,0,1,1)

### `ReconClass[9]` - Ground unit with high vision (Observation post)

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Surface` -> `Air` | 6000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 6000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |

Rings: `Surface` 6000/10000 vision rgba(0,0,1,1)

### `ReconClass[10]` - Ground unit with radar (Radar station)

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Surface` -> `Air` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Air` | 10000 / 20000 | 1 radar | 1 -> 0.25 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 10000 / 20000 | 1 radar | 1 -> 0.25 | 1 -> 0.5 | nil -> 0, false |

Rings: `Surface` 4000/10000 vision rgba(0,0,1,1); `Surface` 10000/20000 radar rgba(1,0,0,1)

### `ReconClass[11]` - Ground unit with small radar (Airfield)

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Surface` -> `Air` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Air` | 5000 / 20000 | 1 radar | 1 -> 0.25 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 4000 / 10000 | 0 vision | 2 -> 1 | 1 -> 0.5 | nil -> 0, false |
| `Surface` -> `Surface` | 5000 / 20000 | 1 radar | 1 -> 0.25 | 1 -> 0.5 | nil -> 0, false |

Rings: `Surface` 4000/10000 vision rgba(0,0,1,1); `Surface` 5000/20000 radar rgba(1,0,0,1)

### `ReconClass[12]` - Units with no recon

| list | `Dist` arcade / realistic | `RawType` | `MaxLevel` -> `+0Ch` | `Gain` -> `+8h` | `Angle` -> `+14h`/`+18h` |
| --- | --- | --- | --- | --- | --- |
| `Surface` -> `Air` | 0 / 0 | 0 vision | 0 -> 0 | 0 -> 0 | nil -> 0, false |
| `Surface` -> `Air` | 0 / 0 | 0 vision | 0 -> 0 | 0 -> 0 | nil -> 0, false |
| `Surface` -> `Surface` | 0 / 0 | 0 vision | 0 -> 0 | 0 -> 0 | nil -> 0, false |
| `Surface` -> `Surface` | 0 / 0 | 0 vision | 0 -> 0 | 0 -> 0 | nil -> 0, false |

Rings: `Surface` 0/0 vision rgba(0,0,1,1); `Surface` 0/0 vision rgba(1,0,0,1)


## 8. The published module

`include/bsp/sensor_table_data.hpp` and `src/sensor_table_data.cpp` publish the rows as **data**,
not as a fixture:

* `ReconAuthoredEntry` / `ReconAuthoredRing` hold the Lua row verbatim, reusing
  `SensorLuaEntryRow` and `SensorLuaRingRow` from `bsp/sensor_tables.hpp`. Nothing in §2 is
  recomputed here.
* `recon_class_source(variant, id)` returns the authored rows of one class.
* `build_sensor_class_table_008082a0(variant, id)` files each row through the existing
  `sensor_entry_from_lua_row_008085c0` / `sensor_ring_from_lua_row_00808791` and
  `sensor_list_index_008085aa`, producing the `SensorClassTable` the loader would produce. An id
  outside 1..12 gives the empty record the loader leaves when `row.Details` is not a table
  (`008082FE`/`00808305`).
* `kReconBaseRangesArcade` / `kReconBaseRangesRealistic` are the fifteen scalars of §5.

`src/sensor_table_data.cpp` is generated, not hand-written; the header says so. No existing
module is edited: `bsp/sensor_tables.hpp`, `bsp/recon_slot_lists.hpp`,
`bsp/gunnery_recon_detection.hpp` and their sources are untouched. Build-tested only
(`scripts/build.ps1`, `src/sensor_table_data.cpp` -> `bsp_core`); no new test was added.

`bsp/sensor_tables.hpp` already publishes `sonar_ship_sensor_table_arcade()`, the hand-built
`ArcadeTable[3]` fixture. It is left alone. Its eight detection entries agree row for row with
the dump; its three rings differ only in `color`, which it leaves zero because that packet had
not decoded the `COL_*` constants. The new module supersedes it in scope, not in correctness.

## 9. Corrections to earlier documents

* **`docs/SENSOR_TABLES.md` §1 and `include/bsp/sensor_tables.hpp`'s `kSensorListCount` comment.**
  - **Was:** "column 7 of every row is allocated and unreachable" / "Columns 7 stay empty".
  - **Is:** column 7 of each observer group *is* the `GUIRange` ring list.
  - **Evidence:** `008087A4 MOV EAX,[ESP+10h]` (the observer index) / `008087AC LEA EAX,[EAX+EAX*2]`
    / `008087AF SHL EAX,5` / `008087B2 LEA EDI,[EAX+ECX*1+5Ch]` is `record + 8 + observer*60h +
    7*0Ch`, exactly the eighth `0Ch` header of the group, and `00808903` pushes the `20h` ring into
    it through `00807F60`. What is true is that `008048A0` never reads column 7. Neither file is
    owned by this packet and neither was edited.
* **`docs/SENSOR_TABLES.md` §5, the per-class vehicle-class counts.**
  - **Was:** recon 2 carries 99 vehicle classes and recon 5 carries 67 (total 637).
  - **Is:** 97 and 65 (total 633).
  - **Evidence:** `vehicleclasses.lua` has 633 `VehicleClass[n]` rows and 637 `["ReconClass"]`
    occurrences. The four extra ones, at lines 188118, 188395, 189548 and 191096, lie inside the
    `--[[` blocks that comment out `VehicleClass[342]`, `[425]`, `[250]` and `[251]` at lines
    187892, 188169, 188446 and 189580. (Both counts describe a locally modified file; see §5.)
* **`docs/SENSOR_TABLES.md` §10, fourth open question.**
  - **Was:** "the `Color` values in `GUIRange` are Lua constants that this packet did not decode,
    so the four colour words are left zero in the reconstruction."
  - **Is:** decoded. `COL_RED {1,0,0,1}`, `COL_GREEN {0,1,0,1}`, `COL_BLUE {0,0,1,1}`, stored as
    four floats.
  - **Evidence:** `scripts/global/luamw_init.lua` lines 25-27, and `00B67C40`'s
    `FSTP float ptr [EDI+k*4]` at `00B67C69`/`00B67C9D`.

Nothing in this document contradicts `docs/GUNNERY_RECON_DETECTION.md`'s rule (c).

## 10. Follow-up packets

* **Drive rule (c) from this data in `bsp_game.exe`.** The contact list still runs rules (a) and
  (b) only (2004 candidates -> 4989 assigns on the 3200-frame USN02 run). Wiring
  `build_sensor_class_table_008082a0` into the observer runtime is the first use of these rows,
  and the first chance at run-time evidence for anything in §6.
* **`ReconModifier` (`class+B8h`) and the two range scalars.** `008048A0` divides by
  `env^2 * signature * SimplifiedReconMultiplier^2`. The signature is the per-class
  `ReconModifier`, which is authored in the same (modified) `vehicleclasses.lua` and was not
  dumped here. The multipliers are mission-scripted (`docs/SENSOR_TABLES.md` §7).
* **A pristine script diff.** Obtaining an unmodded copy of `scripts/datatables` would turn §5's
  timestamp argument into a byte-for-byte one, and would settle the §3 counts.
* **The `GUIRange` consumer.** `00807F60`'s list is read by the map/minimap ring drawing, which
  is unidentified. `docs/SENSOR_TABLES.md` §10 leaves `00807F60` and `00807BD0` read only for
  their strides.
