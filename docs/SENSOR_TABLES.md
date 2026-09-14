# Sensor tables

Addresses: `00808F90`, `008082A0`, `00808CB0`, `00808EA0`, `00808C20`, `00807F60`, `00807BD0`,
`00852B90`, `004F1740`, `006DFD20`, `0074E190`, `0074DD50`, `006D1D30`, `006F57B0`, `0085EA40`,
`007ECFE0`, `008B2320`, `008B24A0`, `008B25F0`, `008B2770`, and the tail of `00444D20`.

`docs/RECON_SLOT_LISTS.md` §1(f) read the consumer, `008048A0 BSP_Recon_EvaluateSensors`, and left
everything that fills its tables unread. This document is the producer side: the record, the entry,
the script that writes them, the seven categories that index them, and the two scalars the
evaluation scales its ranges by. Every name here is a hypothesis, not a recovered symbol.

## 1. The record, `[[unit+538h]+B4h]`

`00808CB0` builds it: `operator_new(2A8h)` at `008090F1`, then

| offset | size | meaning | evidence |
| --- | --- | --- | --- |
| `+0h` | 4 | vtable, `00D08FEC` | `00808CF4` (transiently `00CEB130` at `00808CC9`) |
| `+4h` | 4 | refcount, starts at 1 | `00808CD3`; `00808FF5`/`0080917D` `InterlockedIncrement` |
| `+8h` | `2A0h` | seven groups of `60h` | `00808CDA`..`00808CFA`, the vector ctor iterator |

Each `60h` group is eight `0Ch` list headers: `00808200` runs the same iterator over **seven** `0Ch`
elements with the trivial ctor `00807B60` (three zeroed dwords) and then zeroes `+54h`, `+58h`,
`+5Ch` inline, which is the eighth. The `0Ch` header is `{Entry* begin, int count, int capacity}`;
`008048A0` reads `begin` and `count` (`00804A8E`), and `00808A37`..`00808A4E` grows on
`count == capacity` to `max(2 * capacity, 1)` through `00807BD0`. The new entry is written at
`begin + count * 1Ch` (`00808A53`..`00808A61`, `LEA ECX,[EAX*8]; SUB ECX,EAX; LEA EDI,[EDX+ECX*4]`).

So the record holds 56 lists of `1Ch` entries, addressed `+8h + (observerCategory * 8 +
subjectCategory) * 0Ch`. Both the loader (`008085AA`: `LEA EAX,[EDX+ECX*8]; LEA EAX,[EAX+EAX*2]`,
`ECX` the observer index and `EDX` the subject index) and the evaluation (`00804A79`, `EAX` the
observer and the other the target) compute the same address. The stride is 8 but only seven groups
exist, and no category value reaches 7: **column 7 of every row is allocated and unreachable**.

## 2. The `1Ch` entry, from the writer

`008085C0`..`00808A6A` fills seven dwords on the stack and `REP MOVSD`s them into the list
(`00808A5B`, `for (iVar5 = 7; ...)`).

| offset | type | Lua key | written at | read at |
| --- | --- | --- | --- | --- |
| `+0h` | float | `Dist` | `008085E5` | never |
| `+4h` | float | `Dist * Dist` | `008085F1` `FMUL ST0` | `00804AB7` |
| `+8h` | float | `Gain * 0.5` | `00808626`, the double `00D7A280` | `00804B35`, times `dt` |
| `+0Ch` | float | from `MaxLevel` | `0080866B`..`00808685` | `00804B69`, the accumulator cap |
| `+10h` | int | `RawType` | `008089F4`, stored `00808A23` | `00804ACF`, `1 << it` against the mask |
| `+14h` | float | `Angle` | `00808992`, stored `008089C6`/`008089E5` | `00804B29`, the bearing half-angle |
| `+18h` | byte | `Angle` present | `IsNil` at `008089BA`, stored `008089CF`/`008089EC` | `00804B0F`, gates the bearing test |

`MaxLevel` is read with a default of 2 (`0080865A PUSH 2`, `00B66380`) and mapped to a cap: 0 gives
`0.0f` (`00808975 XORPS`), 1 gives `0.25f` (`0080896B`, `00CE3868`), anything else `1.0f`
(`0080867D`, `00D7A24C`). Those are exactly the blip threshold and the maximum of the detection
accumulator in `docs/RECON_SLOT_LISTS.md` §1(e), so `MaxLevel` is the highest **published level**
this entry may drive a target to: 0 never detects, 1 stops at a blip, 2 identifies.

`Dist` is stored raw at `+0h` and squared at `+4h`; only the square is ever read back, so `+0h` is
the authored range kept for nothing the executable uses.

## 3. The producer: `008082A0`, the `ReconClass` Lua reader

`__thiscall void(this = record, LuaObject* row)`, body `008082A0`-`00808C1C`, sole call site
`00809147` inside `00808F90`. It requires the row to be a table (`008082D2`) and `row.Details`
(`00D08FE4`) to be a table (`008082E9`); otherwise the record keeps its 56 empty lists.

It then walks two nested Lua key iterations. The outer keys of `Details` select the **observer**
category, the inner keys of each group select the **subject** category, and the numbered rows under
each subject key are the entries.

| key | index | compared at |
| --- | --- | --- |
| `ObserverAir` / `SubjectAir` | 0 | `008083CD` / `00808531` |
| `ObserverSurface` / `SubjectSurface` | 1 | `008083ED` / `0080868A` |
| `ObserverPeriscopeIn` / `SubjectPeriscopeIn` | 2 | `0080840E` / `008086AE` |
| `ObserverPeriscopeOut` / `SubjectPeriscopeOut` | 3 | `0080842A` / `008086D2` |
| `ObserverUnderwater` / `SubjectUnderwater` | 4 | `00808446` / `008086F6` |
| `ObserverDeepUnderwater` / `SubjectDeepUnderwater` | 5 | `00808462` / `00808715` |

Comparison is case-insensitive (`00425850`, `00BF7FBF`). **Neither index slot is initialised**:
nothing writes `[ESP+10h]` or `[ESP+14h]` before its comparison chain, and a key that matches
nothing simply falls through to the shared body with the slot unchanged. An unrecognised key
therefore appends its rows to the previous key's list, or, on the first key of a record, to a list
chosen by an uninitialised stack word. No shipped table exercises this.

One further inner key, `GUIRange` (`00D08ED8`, `00808734`), takes a different `20h` record built at
`00808791`..`00808903` and handed to `00807F60`: `{int RawType +0h, byte hasAngle +4h, float Range
+8h, float Angle +0Ch, four colour words +10h}`. It is stored per observer category and `008048A0`
never reads it; it is the map's range rings.

### Coverage

| routine | coverage |
| --- | --- |
| `008082A0` | complete for the normal path; the unwind funclets (`00808B78`..`00808C1C`) are unread |
| `00808F90` | complete |
| `00808CB0`, `00808200`, `00807B60` | complete |
| `00807F60`, `00807BD0` | partial: called as the `GUIRange` push-back and the entry-list growth; their bodies are read only far enough to establish the `20h` and `1Ch` strides and `max(2 * capacity, 1)` |
| `00852B90` | complete |
| `004F1740`, `006DFD20`, `0074E190`, `0074DD50`, `006D1D30`, `006F57B0`, `0085EA40`, `007ECFE0` | complete (each is 2 to 12 instructions) |
| `008B2320`, `008B24A0`, `008B25F0`, `008B2770` | complete |
| `00444D20` | partial: only `0044569F`..`004456B2`, the tail that sets `+74h` and `+78h`. `docs/WEATHER_CONFIG.md` owns the rest |

## 4. The resolver and its registry: `00808F90`

`__fastcall SensorClassRecord**(ECX = out, EDX = reconClassId)`, `RET`, body `00808F90`-`008091CD`,
sole caller `00960230 BSP_VehicleClass_ReadLuaFields` at `009623F0`. The vehicle class row's
integer `ReconClass` field (`docs/VEHICLE_CLASS_FIELDS.md` line 116) is the id; the record it
returns is what the class stores at `class+B4h`.

1. `00808EA0` returns the process-wide registry, a `10h` singleton at `00F874E4` created under the
   singleton-lifetime lock: `{vtable 00D08FF4, Record** data, int capacity, int size}`.
   `00808C20` grows it to `id + 1`; `00808100` is the reserve that reference-counts the move.
2. `00808FE4`: the slot's record, if any, is returned with an added reference and **every Lua step
   is skipped** (`00809002`, the `NEG`/`SBB`/`TEST 0E1952Ch` idiom Ghidra renders as a mask). The
   table is therefore read once per recon class id per process.
3. `0080902B`/`00809041`: `globals.ReconClass` (`00D08FF8`), through the embedded Lua owner at
   `[00E188A8]+1A0Ch`.
4. `0080905E`: `ReconClass[id]`, the row.
5. `00809085`: `row.ID = id` (`00CE59B4`). `008090C9`: `row.Got = true` (`00CE452C`). The resolver
   **writes back into the script table** before reading anything out of it; the scripts can see
   which recon classes the executable has instantiated.
6. `008090F1`/`0080910D`: allocate and construct the empty record.
7. `00809147`: `008082A0` fills it.
8. `00809164`..`0080919B`: publish into the registry slot, releasing whatever was there.

A null `operator_new` leaves the record pointer null and `008082A0` is still called on it
(`00809142` passes `EBX`, which is zero on that path); the registry slot is then set to null.

## 5. The Lua producer, and one class's installed values

`scripts/datatables/autoload/reconclasses.lua` sets `ReconClass = {}` and then, from
`scripts/gamemode.lua`, loads `classtables/realistic/reconclasses.lua` (`GameMode == 1`) or
`classtables/arcade/reconclasses.lua` otherwise, aliasing `RealisticTable` or `ArcadeTable` onto the
global. Both files define twelve classes with the same shape and different base ranges. The raw
types are named there:

```lua
RRT_VISION = 0   RRT_RADAR = 1   RRT_SONAR = 2
```

which is the `RawType` at entry `+10h` and the bit `008048A0` tests; the rebuild passes `0FFh`, so
all three always pass the mask.

`docs/RECON_VALUES.md`'s `install_recon_values_00803A40` builds a **different** `recon` table
(`recon[0..2][enemy|neutral|unknown|own][19 category names]`) and is unrelated to this one.

### The twelve classes and their users

| id | comment in the script | vehicle classes carrying it |
| --- | --- | --- |
| 1 | Normal Ship (Cargo ship, Torpedoboat) | 22 |
| 2 | Radar ship (Battleship, Carrier) | 99 |
| 3 | Sonar ship (Destroyer) | 31 |
| 4 | Ship with radar and sonar (Cruiser - TONE) | 1 |
| 5 | Normal plane | 67 |
| 6 | Recon plane | 8 |
| 7 | Normal submarine | 9 |
| 8 | Ground unit w/o radar | 305 |
| 9 | Ground unit with high vision (Observation post) | 3 |
| 10 | Ground unit with radar (Radar station) | 3 |
| 11 | Ground unit with small radar (Airfield) | 6 |
| 12 | Units with no recon | 83 |

(counted from the 633 `VehicleClass[n]` rows of `scripts/datatables/autoload/vehicleclasses.lua`.)

### `ArcadeTable[3]`, the destroyer

`SonarShip_recon_SonarRange = 1000`, `NormalShip_recon_VisionRange = 4000`. Carried by 31 classes
including `VehicleClass[11]` "Allen M. Sumner 1945", `VehicleClass[23]` "Fletcher 1943" and
`VehicleClass[14]` "Akizuki 1944". Everything sits under `ObserverSurface`, so 55 of the 56 lists
stay empty. `Gain` is `1/1` and `MaxLevel` `2` on every row, so every installed entry has
`+8h = 0.5f` and `+0Ch = 1.0f`. `DEG(45)` is `45 * pi / 180` (`scripts/global/luamw_init.lua`
line 434).

| list (`observer`, `subject`) | index | installed entries, `{+0h, +4h, +8h, +0Ch, +10h, +14h, +18h}` |
| --- | --- | --- |
| Surface, Air | 8 | `{4000, 16000000, 0.5, 1.0, 0 vision, 0, false}` |
| Surface, Surface | 9 | `{4000, 16000000, 0.5, 1.0, 0 vision, 0, false}` |
| Surface, PeriscopeIn | 10 | `{1000, 1000000, 0.5, 1.0, 2 sonar, 0, false}`, `{2500, 6250000, 0.5, 1.0, 2 sonar, 0.7853982, true}` |
| Surface, PeriscopeOut | 11 | `{1000, 1000000, 0.5, 1.0, 2 sonar, 0, false}`, `{2500, 6250000, 0.5, 1.0, 2 sonar, 0.7853982, true}` |
| Surface, Underwater | 12 | `{660, 435600, 0.5, 1.0, 2 sonar, 0, false}`, `{1660, 2755600, 0.5, 1.0, 2 sonar, 0.7853982, true}` |
| Surface, DeepUnderwater | 13 | empty: **a destroyer cannot see a deep-running submarine at all** |

Its `GUIRange` rings are `{1000, sonar}`, `{2500, sonar, 45 deg}`, `{4000, vision}`.

The realistic table is the same shape with `SonarShip_recon_SonarRange = 4000` and
`NormalShip_recon_VisionRange = 10000`.

## 6. The seven categories, `unit->[+1E4h]->vtable[1]()`

`008048A0` calls it on the target (`00804A69`, the column) and on the observer (`00804A73`, the row
times 8). `ECX` is `unit + 1E4h`, the secondary base subobject.

Twelve distinct implementations occupy slot 1 of the 51 vtables whose slot 0 is `0077B0C0` (the set
found by scanning `.rdata`/`.data` for that constant). Each vtable's class is confirmed by its
`IsKindOf` at `+88h`/`+8Ch`.

| routine | returns | vtables | class |
| --- | --- | --- | --- |
| `004F1740` | 6 | 24, including `00D03E54`, `00D0DF40`, `00D1A66C` | the base default; levels 2-4 of the unit chain |
| `0074E190` | 0 Air | 9, including `00D05EF4` | plane instances (`00D05F7C` is `BSP_PlaneInstance_IsKindOf`) |
| `006DFD20` | 1 Surface | 8, including `00CFC3A4`, `00D09648` | the ship chain, levels 5-6 (`00CFC3A4` from `006FE460 BSP_UnitInstance_Construct`) |
| `0074DD50` | 1 Surface | `00CFFDB4` | land vehicle (`00CFFE3C` is `BSP_MLandVehicle_IsKindOf`) |
| `006D1D30` | 1 Surface | `00CF8BD8` | airfield (`00CF8C64` is `BSP_MAirfield_IsKindOf`) |
| `006F57B0` | 1 Surface | `00CFAFF8`, `00CFF3CC` | land fort (`00CFF454` is `BSP_MLandFort_IsKindOf`) |
| `004F24F0`, `00846B50`, `008561E0` | 1 Surface | one each | further surface kinds, not identified here |
| `0085EA40` | 4 Underwater | `00D0D100` | constructed by `0085E9C0` in the submarine/torpedo module; the class is not identified |
| `00852B90` | 1..5, a **state** | `00D0BF54` | submarine (`00852F10`, called only by `008531A0 BSP_VehicleClassSubmarine_CreateInstance`) |
| `007ECFE0` | delegates | `00D08794` | plane squadron (`007F2C60 BSP_PlaneSquadronTickableEntity_Construct`); returns the category of the object at `+3D0h`, or 6 when that pointer is null |

So the categories are fixed per unit kind except for the submarine, and the Lua key names name them
exactly: Air 0, Surface 1, PeriscopeIn 2, PeriscopeOut 3, Underwater 4, DeepUnderwater 5. **Value 6
has no Lua key**: a unit that keeps the base `004F1740` neither detects nor is detected, because row
6 and column 6 of every shipped record are empty.

### The submarine, `00852B90`

`ECX = unit + 1E4h`, so `ESI = unit` and `EDI = unit + 1E4h`. It refreshes the world pose through
`00414DB0` when `unit+C8h` is clear, then reads the hull's world Y at `unit+100h` and compares:

1. `00852BAB`: `y > (unit[1204h] + unit[1200h]) / 3.0` (the double at `00D7A2B0`) gives **Surface**.
2. `00852BE4`: else `unit[1204h] - 5.0` (the double at `00D7A370`) `<= y` gives **PeriscopeOut** when
   the byte at `unit+1234h` is set and **PeriscopeIn** when it is clear (`00852C4C`, `SETNE` plus 2).
3. `00852C14`: else `(unit[120Ch] + unit[1208h]) * 0.5` (`00D7A280`) `<= y` gives **Underwater**,
   otherwise **DeepUnderwater**.

The four depth words and the periscope byte are produced elsewhere and are not read here.

## 7. The two tuning scalars on `[game+21C4h]`

`game+21C4h` is the `7Ch` weather config of `docs/WEATHER_CONFIG.md` (`00445B10` constructs,
`00444D20 BSP_WeatherConfig_LoadGlobals` loads). Its `+74h` and `+78h` are the two floats
`008048A0` scales its ranges by, and they are not weather values at all:

| offset | Lua name | getter | setter |
| --- | --- | --- | --- |
| `+74h` | `SimplifiedReconMultiplier` | `008B24A0` | `008B2320` |
| `+78h` | `SimplifiedSonarMultiplier` | `008B2770` | `008B25F0` |

The four are rows of the binding table at `00E0C248`..`00E0C264`, each `{const char* name, int
(*fn)(lua_State*)}`, with the names at `00D0F8E4`, `00D0F8C4`, `00D0F8A4`, `00D0F884`. Each setter
reads Lua argument 0 as a number and stores it straight into the live config
(`008B243C FSTP [EAX+74h]`, `008B270C FSTP [EAX+78h]`).

`00444D20` sets both to `1.0f` at `004456AD`/`004456B2`, unconditionally and **after** every Lua
read and every map iteration it does, so no data table can change them. Only a mission script can.

Because `008048A0` divides the squared horizontal distance by `env^2 * signature * m74^2` (and, for
a submerged submarine target, by `m78^2` as well) before comparing against `Dist^2`, each multiplier
scales sensor range linearly: `SetSimplifiedReconMultiplier(10.0)` is a ten-fold recon range.

The shipped missions call `SetSimplifiedReconMultiplier` about 60 times with values from `0.1` to
`10.5`; the commonest live values are `0.5` (9 missions), `10.0` (8) and `1.0` (7), and many
missions carry the call commented out. **No shipped script calls `SetSimplifiedSonarMultiplier`**,
so the submerged-submarine factor is `1.0` in all shipped content and the sonar penalty of
`docs/RECON_SLOT_LISTS.md` §1(f) step 5 never fires there.

## 8. `[[game+19CCh]+13Ch]`, the world list

Not resolved. What is established: `00807462`..`0080749A` walks a singly linked list whose head is
`world+13Ch`, node `{?, next +4h, unit +8h}` — the same node shape as the zone list at `world+16Ch`
in `docs/GAME_AWARD_TRACKERS.md` — and calls `00805BE0` on `node[8h] + 1E8h + slot * 34h`, one
detection record per unit. `[game+19CCh]` is the world/scene registry that also carries the
per-class-id unit lists at `+18h + id * 0Ch` and the carrier list at `+88h`. No writer of a
`+13Ch` list head was found: scanning `.text` for `MOV [reg+13Ch], reg` yields nothing in the world,
scene or recon modules, so the head is written through a register-held address by a helper this
packet did not find.

## 9. `no_ghidra_function`

| address | name | end address |
| --- | --- | --- |
| `00808200` | `sensor_class_group_ctor_00808200` | `00808225` |
| `00807B60` | `sensor_list_header_ctor_00807b60` | `00807B6C` |
| `00852B90` | `sensor_category_submarine_00852b90` | `00852C5D` |
| `004F1740` | `sensor_category_unclassified_004f1740` | `004F1745` |
| `006DFD20` | `sensor_category_surface_006dfd20` | `006DFD25` |
| `0074E190` | `sensor_category_air_0074e190` | `0074E192` |
| `0074DD50` | `sensor_category_surface_0074dd50` | `0074DD55` |
| `006D1D30` | `sensor_category_surface_006d1d30` | `006D1D35` |
| `006F57B0` | `sensor_category_surface_006f57b0` | `006F57B5` |
| `0085EA40` | `sensor_category_underwater_0085ea40` | `0085EA45` |
| `007ECFE0` | `sensor_category_delegated_007ecfe0` | `007ECFFC` |
| `004F24F0` | `sensor_category_surface_004f24f0` | `004F24F5` |
| `00846B50` | `sensor_category_surface_00846b50` | `00846B55` |
| `008561E0` | `sensor_category_surface_008561e0` | `008561E5` |

None of these has a Ghidra function. The first two are reached only through the vector-constructor
iterator `00BF7CD1`; the eleven category accessors are reached only through vtable slot 1 and are
two to twelve instructions each. All were read with `bsp.py disasm-raw`.

`00852BA6` (the pose refresh `00414DB0` inside `00852B90`) is therefore a call site that lies in no
Ghidra function, which is why it is not a `host_steps` row of `reports/sensor_tables.json`.

## 10. Open questions

* The list at `[[game+19CCh]+13Ch]` (§8): which units are on it, and what puts them there.
* `0085EA40`'s owning class (returns Underwater, constructed by `0085E9C0`), and the three further
  Surface implementations `004F24F0`, `00846B50`, `008561E0`.
* `00807BD0` and `00807F60` are read only for their strides and growth rule; their bodies and the
  `GUIRange` consumer are unread.
* The `Color` values in `GUIRange` are Lua constants (`COL_BLUE`, `COL_RED`, `COL_GREEN`) that this
  packet did not decode, so the four colour words are left zero in the reconstruction.
* No run-time evidence: `docs/RECON_SLOT_LISTS.md` §10 records that `bsp_game.exe` fakes the recon
  lists, so no path it runs reaches `008048A0`, `00808F90` or `008082A0`.

## 11. Corrections to earlier documents

* `docs/WEATHER_CONFIG.md` line 24 and `include/bsp/weather_config.hpp` name `+74h` and `+78h`
  provisionally as amplitude multipliers (`current_view_mode_amp_multiplier_74`,
  `current_weather_amp_multiplier_78`). The Lua bindings name them
  `SimplifiedReconMultiplier` and `SimplifiedSonarMultiplier`, and `008048A0` uses them as sensor
  range scalars. Neither file is owned by this packet and neither was edited.
* `docs/RECON_SLOT_LISTS.md` §1(f) step 8 and §10 describe the categories as `0..7` and the table as
  `observerCategory * 8 + targetCategory`. The stride is 8 but the record holds seven groups, the
  categories run `0..6`, and column 7 is unreachable.
* `docs/RECON_SLOT_LISTS.md` §1(f) step 10 reads entry `+0Ch` as a free clamp. It is one of exactly
  three values derived from the script's `MaxLevel`.
* `include/bsp/recon_slot_lists.hpp`'s `ReconSensorEntry` omits `+0h`. The producer writes the
  authored `Dist` there and its square at `+4h`.

## Correction from docs/GAMEPLAY_LOOSE_ENDS_1.md (packet cc2_gameplay_loose_ends_1)

- **Was:** [[game+19CCh]+13Ch] is the world list; no writer of a +13Ch list head was found, so the head is written through a register-held address by a helper this packet did not find.
  **Is:** There is no separate world list and no missing helper. registry+13Ch is the head dword of the per-class-id list triple for class id 24, in the same registry+18h+id*0Ch array the recon scan's own step 5 walks; plane squadrons are pushed onto it through BSP_UnitList_PushBack.
  **Evidence:** 0x13C = 0x18 + 24*0xC + 4, and the reader itself uses the array at 008074C7 LEA EAX,[EBX+EBX*0x2] / 008074CA MOV EBP,[EDX+EAX*0x4+0x1C]. The array is built by the 004CB076 vector constructor iterator with PUSH 0x61 count and PUSH 0xC stride. 00484540 BSP_UnitList_PushBack writes the head at list+4h. 007F10C8 ADD ECX,0x138 then 007F10CE CALL 0x00484540 in FUN_007f10b0, at 00D087C0+130h, is the insert; the squadron's primary vptr 00D087C0 is installed at 007F2CAD.

## Correction from docs/SUBMARINE_MODEL.md (packet cc2_submarine_model)

- **Was:** The four depth words and the periscope byte are produced elsewhere and are not read here.
  **Is:** The producers are now named. The four depth words at unit+1200h..+120Ch are built once at scene attach by 00853630's loop at 00853A90-00853AC5 from the class keys PeriscopeDepth, SwimDepth2 and SwimDepth3 against a default table of 0.0, -20.0, -40.0, -80.0 at 00E0B578; band 0 is always the default. The periscope byte at unit+1234h is cleared every frame at 00854b00 and set at 00855057 only when the extending mast reaches class.PeriscopeMoveRange + periscopeY - 1.0.
  **Evidence:** 00853630 008539e0-00853a7c for the band loop, 00854b00 and 00855057 for the byte, and the save schema binding at 00854008 which names the byte periscopeOut.

## Correction from docs/SENSOR_TABLE_DATA.md (packet cc7_sensor_table_data)

- **Was:** "no category value reaches 7: **column 7 of every row is allocated and unreachable**".
  **Is:** column 7 of each observer group is the `GUIRange` list. It is reached at `008087B2`, which
  is the push that fills it, so the column is allocated *and used* - just not by the same key that
  fills columns 1 through 6.

- **Was:** the vehicle counts for `recon-2` and `recon-5` given as 99 and 67.
  **Is:** 97 and 65 in this installation. Four `ReconClass` keys sit inside `--[[ ]]` comment
  blocks and were counted as live.

- **Was:** "the `Color` values in `GUIRange` are Lua constants (`COL_BLUE`, `COL_RED`, `COL_GREEN`)
  that this packet did not decode".
  **Is:** each `Color` word is four floats, and `COL_RED`, `COL_GREEN` and `COL_BLUE` decode.

- **Provenance, which applies to every authored value in this document and in the new one.** The
  installation under `I:/SteamLibrary/steamapps/common/Battlestations Pacific` is **modded** - it
  carries BSPRM/AlterBSP artefacts. The three `reconclasses.lua` files carry `2024-07-13`, the
  timestamp of the untouched datatables bulk, but `scripts/datatables/autoload/vehicleclasses.lua`
  carries `2026-05-09` and is **locally modified**. Verified at integration with
  `find ./scripts -name "<file>" -printf "%TY-%Tm-%Td %p\n"`. So the detection rows dumped from
  `reconclasses.lua` are very likely retail, while any per-class association taken from
  `vehicleclasses.lua` describes *this installation*. No pristine copy exists on this machine to
  diff against. Prefer "this installation" over "retail" for authored values until one does.
