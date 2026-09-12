# The entity class-id space (`+C4h` and vtable slot `5Ch`)

Addresses: `0042B8F0`, `0047F190`, `00480930`, `006D20E0`, `006E2790`, `006E3D50`, `006FE050`,
`0064B720`, `006508B0` (class tests read byte for byte); 87 class tests in all, one per row of the
table below and of `reports/entity_class_ids.json`. Class-descriptor tables `00CE4570`, `00CFA15C`,
`00CFF2DC`, `00D19C20`-`00D1AAC0`. Scene registration table `004F2800` (read by
`docs/SCENE_ENTITY_FACTORY.md`, not re-read here).

Seven docs left class ids unlabelled: `docs/LUA_BINDING_ENTITY_LOOKUP.md`,
`docs/UNIT_INSTANCE_LAYOUT.md`, `docs/GUN_CLASS_FAMILY.md`, `docs/UNIT_DAMAGE_AND_DEATH.md`,
`docs/UNIT_PARTS.md`, `docs/BOT_FIRE_TARGET.md`, `docs/SCENE_ENTITY_FACTORY.md`. This packet is a
survey: it settles what `+C4h` holds, what the class test computes, and which class each id names.

## What `+C4h` is

It is the **most derived class id**, not a depth.

`docs/UNIT_INSTANCE_LAYOUT.md` recorded it as "written once per level with the level's own depth,
so the final value names the most derived class", with `7` for a destroyer. The value is right and
the reading is not: the destroyer's six constructor levels write `01`, `02`, `04`, `05`, `06`,
`07` and **skip `03`**, which exists as its own class (test `00888EA0`, vtable `00D11138`, parent
`01`). The chain compiled into the destroyer's own class test `006FE530` is literally
`07, 06, 05, 04, 02, 01, 0` -- no `03`. Depth and id agree along the ship chain only because the
ids there happen to be near-consecutive; they disagree on every other branch (`MWaterMine` `34h`
is four edges from the root, `MSmallReconPlane` `15h` is seven).

Each constructor level stamps its own id over the base's, so after construction the field holds the
leaf. `MOV dword ptr [reg+0C4h], imm32` occurs 92 times in `.text`, with 74 distinct immediates.

## The class test: `vtable[5Ch](classId)`

`00480930`, the slot `5Ch` of `00CE6290` (`Path`, `47h`), in full:

```
00480930  8B 44 24 04     MOV  EAX, dword ptr [ESP + 4]    ; the queried class id
00480934  83 F8 47        CMP  EAX, 47h                    ; own id
00480937  74 16           JZ   0048094F
00480939  83 F8 01        CMP  EAX, 1                      ; parent
0048093C  74 11           JZ   0048094F
0048093E  85 C0           TEST EAX, EAX                    ; the root, 0
00480940  74 0D           JZ   0048094F
00480942  3B 81 C4 000000 CMP  EAX, dword ptr [ECX + 0C4h] ; the dynamic id
00480948  74 05           JZ   0048094F
0048094A  33 C0           XOR  EAX, EAX
0048094C  C2 04 00        RET  4
0048094F  B8 01 00 00 00  MOV  EAX, 1
00480954  C2 04 00        RET  4
```

`__thiscall`, one stack argument, `RET 4`, returns 0 or 1. Every one of the 87 tests has this
shape: a run of `CMP EAX,imm` over the owning class's **whole ancestor chain**, the root compare
emitted as `TEST EAX,EAX`, then one `CMP EAX,[ECX+0C4h]`. `006FE050` (`MDepthChargeLauncher`,
`27h`) is the deepest: `27h, 24h, 22h, 20h, 1Eh, 04h, 02h, 01h, 0`.

The rule, as a pure function:

```
IsKindOf(this, query) = query is on the parent chain of the class that owns this vtable
                        (that class itself and every ancestor up to and including 0)
                     || query == this->classId at +C4h
```

It is **not** an equality, **not** a range test and **not** a run-time chain walk: the chain is
unrolled into the compare run at compile time. The single `CMP EAX,[ECX+0C4h]` is the only
run-time part, and it exists so that a class which does **not** override slot `5Ch` is still
recognised by exact identity. Three vtables rely on it: `SimpleEffect` (`5Bh`, `00CE8BD0`) and
`PeriodicEffect` (`5Ch`, `00CE8D68`) both install class `01`'s test `0047F190`, and `00D19500`
installs the root's test `0042B8F0`, so `IsKindOf(ownId)` on those objects is answered by the
dynamic compare alone.

`include/bsp/entity_class_ids.hpp` states both forms: `entity_is_kind_of(dynamicId, query)` for a
class that has its own test, `entity_is_kind_of_inherited(ownerId, dynamicId, query)` for one that
does not.

### Compare order is not uniform

The own id is emitted first in most tests (`006E3D50`, `00480930`, `006FE050`), **last** in others
(`006D20E0` puts `CMP EAX,45h` after the `CMP EAX,[ECX+0C4h]`), and a base can carry a larger id
than its derived class (`0064B720` is `50h, 56h, 4Ch, 0`, so `56h` is the base of `50h`). Own ids
in the table below are therefore derived by set containment -- a class's ancestor set is its
parent's plus exactly one element -- not by position and not by maximum.

## The id space is one space

Three numberings were open across the docs. They are the same space:

- The `+C4h` class id and the **scene class id** registered by `004EE250` coincide. Every scene
  class that allocates its own object stamps exactly its registered id: `18h`, `1Ah`, `1Dh`,
  `41h`, `42h`, `43h`, `44h`, `47h`, `4Ah`, `4Dh`, `5Bh`, `5Ch` -- twelve for twelve, each checked
  by decoding the registered creator and reading the stamp it reaches. The nine `unit`-via-`Type`
  scene classes stamp nothing directly, because their instance comes from the unit-class factory.
- The **entity-lookup kind** of `docs/LUA_BINDING_ENTITY_LOOKUP.md` is the class id minus six.
  `0088B1D8` loads `-6` for the first bucket and `0088B2CA` compares against `5Bh` for the last, so
  the bucket array spans class ids `0` to `60h`. `60h` is the highest id with a class test
  (`007810F0`), the independent check that the span is the whole space.

That settles `FindEntity`'s unlabelled kinds:

| kind | class id | class |
| --- | --- | --- |
| `00h` | `06h` | the ship base (unnamed) -- "units" |
| `09h` | `0Fh` | the plane base (unnamed) |
| `12h` | `18h` | `PlaneSquadronGen` |
| `13h` | `19h` | `MLandVehicle` |
| `14h` | `1Ah` | `LandConvoy` |
| `15h` | `1Bh` | `MLandFort` |
| `16h` | `1Ch` | `MCommandBuilding` |
| `3Bh` | `41h` | `NavPoint` |
| `3Ch` | `42h` | `MovieCamPos` |
| `3Dh` | `43h` | `MovieCamLookat` |
| `3Eh` | `44h` | `Landscape` |
| `3Fh` | `45h` | `MAirfield` |
| `40h` | `46h` | `MShipyard` |
| `41h` | `47h` | `Path` |
| `44h` | `4Ah` | `CameraPath` |
| `47h` | `4Dh` | `SpawnPoint` |

## Where the names come from

Two literal sources, no hypotheses:

1. The **scene registration table** `004F2800`, 26 `PUSH <name>; PUSH <classId>` pairs, already
   transcribed by `docs/SCENE_ENTITY_FACTORY.md`.
2. The **class-descriptor records**, arrays of function pointers with the class name inline. The
   creator is the dword **immediately before** the inline name; the record ends with the shared
   slot `004407A0` followed by the next record's creator.

The pairing direction was settled against the scene table, which is independent literal evidence:
under "creator before name" the records give `MLandFort` `1Bh`, `MCommandBuilding` `1Ch`,
`MWaterMine` `34h`, `MAirfield` `45h`, `MShipyard` `46h`, and the `LandConvoy` (`1Ah`) and
`PlaneSquadronGen` (`18h`) records resolve to their registered ids -- seven agreements. Under
"creator after name" all seven are wrong by one record. Two further checks agree: the descriptor
order `MReconPlane`, `MSmallReconPlane`, `MLargeReconPlane` maps to `14h` and its two children
`15h`/`16h`, and `MDestroyer`/`MSubmarine`/`MMothership`/`MLandingShip`/`MTorpedoBoat` land on
`07h`/`08h`/`09h`/`0Ch`/`0Eh`, the five ship ids the scene table registers.

The ship descriptors at `00D1AD28`-`00D1AF30` put the creator two slots before the name rather than
one, and the three large plane descriptors at `00D19C98`-`00D1A520` put it `298h` before. The
offset was resolved per table by probing every `.text` pointer in the record for a `+C4h` stamp;
only one candidate per record stamps at all.

### `docs/GUN_CLASS_FAMILY.md` needs a one-record shift

That doc paired each gun name with the creator **after** it. Correcting the direction moves its
rows:

| name | doc's id | corrected id | corrected parent |
| --- | --- | --- | --- |
| `MRFSGun` | `23h` | `21h` | `20h`, the base gun |
| `MRTGun` | `24h` | `23h` | `22h` |
| `MSTGun` | `27h` | `24h` | `22h` |
| `MMultipleBombPlatform` | `21h` | `26h` | `25h`, `MBombPlatform` |
| `MBombPlatform` | unread | `25h` | `20h` |
| `MDepthChargeLauncher` | unread | `27h` | `24h`, `MSTGun` |
| `MCatapult` | unread | `28h` | `20h` |

The corrected tree also reads better: `RFS` is a **fixed** gun and sits directly under the base gun
rather than under the turning-gun base `22h`; the two turning guns (`RT`, `ST`) are the two
children of `22h`; and `MMultipleBombPlatform` derives from `MBombPlatform`. `vtable[5Ch](20h)`,
the test `008CF350` and `0081F8B0` select on, is unaffected: `20h` is still the base gun and still
the ancestor of all eight.

## The table, `00h`-`47h`

`class test` is the vtable slot `5Ch` body; Ghidra has no function at any of them, so they were
read with `bsp.py ghidra bytes` and decoded linearly. `name evidence` is the descriptor record's
creator address, the scene table's creator, or both.

| id | class | parent | class test | vtable(s) | name evidence | cited by |
| --- | --- | --- | --- | --- | --- | --- |
| `00` | unnamed | root | `0042B8F0` | `00D19120`, `00D19500` | - | - |
| `01` | unnamed | `00` | `0047F190` | `00CE8BD0`, `00CE8D68`, `00D192E0` | - | - |
| `02` | unnamed | `01` | `004F1750` | `00D03E80` | - | - |
| `03` | unnamed | `01` | `00888EA0` | `00D11138` | - | - |
| `04` | unnamed | `02` | `006D1610` | `00D0DF70` | - | - |
| `05` | unnamed | `04` | `006D1650` | `00D1A698` | - | UNIT_INSTANCE_LAYOUT |
| `06` | unnamed | `05` | `006DFE50` | `00D09678` | - | UNIT_INSTANCE_LAYOUT |
| `07` | `MDestroyer` | `06` | `006FE530` | `00CFC3D0` | 006FE590 / DestroyerGen | UNIT_INSTANCE_LAYOUT, SCENE_ENTITY_FACTORY |
| `08` | `MSubmarine` | `06` | `00853050` | `00D0BF80` | 008531A0 / SubmarineGen | SCENE_ENTITY_FACTORY |
| `09` | `MMothership` | `06` | `00758510` | `00D01630` | 00758D30 / MotherShipGen | SCENE_ENTITY_FACTORY |
| `0A` | `MCruiser` | `06` | `006FB3D0` | `00CFB738` | 006FB430 | - |
| `0B` | `MCargo` | `06` | `006EB230` | `00CFA778` | 006EB290 | - |
| `0C` | `MLandingShip` | `06` | `0074BC60` | `00CFFA30` | 0074BE00 / LandingShipGen | SCENE_ENTITY_FACTORY |
| `0D` | `MBattleship` | `06` | `006DFE90` | `00CF90B0` | 006DFEF0 | - |
| `0E` | `MTorpedoBoat` | `06` | `00857DC0` | `00D0C648` | 00857E20 / TBoatGen | SCENE_ENTITY_FACTORY |
| `0F` | unnamed | `05` | `0074E400` | `00D05F20` | - | - |
| `10` | `MPlaneBomber` | `0F` | `007D77F0` | `00D06638` | 007D7850 | - |
| `11` | `MPlaneTorpedoBomber` | `0F` | `009535C0` | `00D1A000` | 009564E0 | - |
| `12` | `MPlaneDiveBomber` | `0F` | `00953530` | `00D19D28` | 00956390 | - |
| `13` | `MPlaneFighter` | `0F` | `007DDA80` | `00D06920` | 007DDAE0 | - |
| `14` | `MReconPlane` | `0F` | `0074E480` | `00D00070` | 008091D0 | - |
| `15` | `MSmallReconPlane` | `14` | `0084C9F0` | `00D0BA80` | 0084CA50 | - |
| `16` | `MLargeReconPlane` | `14` | `0074E4E0` | `00D00308` | 0074E540 | - |
| `17` | `MPlaneKamikaze` | `0F` | `009534A0` | `00D1A2D8` | 00956240 | - |
| `18` | `PlaneSquadronGen` | `02` | `007EFB00` | `00D087C0` | 004F0AD0 | SCENE_ENTITY_FACTORY, LUA_BINDING_ENTITY_LOOKUP |
| `19` | `MLandVehicle` | `05` | `0074DD90` | `00CFFDE0` | 0074DF10 | - |
| `1A` | `LandConvoy` | `02` | `004F2560` | `00CEA570` | 004F2700 | SCENE_ENTITY_FACTORY, LUA_BINDING_ENTITY_LOOKUP |
| `1B` | `MLandFort` | `05` | `006F5890` | `00CFF3F8` | 00747000 / LandFort | UNIT_PARTS, UNIT_DAMAGE_AND_DEATH, SCENE_ENTITY_FACTORY |
| `1C` | `MCommandBuilding` | `1B` | `006F58E0` | `00CFB028` | 006F5C10 / CommandBuilding | UNIT_PARTS, SCENE_ENTITY_FACTORY |
| `1D` | `LandingPoint` | `01` | `004E9620` | `00CE90E0` | 004E9D40 | SCENE_ENTITY_FACTORY |
| `1E` | unnamed | `04` | `006E3D10` | `00CFDC58` | - | - |
| `1F` | *no class test in the image* | - | - | - | - | - |
| `20` | unnamed | `1E` | `006E3D50` | `00CFE0A8` | - | GUN_CLASS_FAMILY, GUN_AIMING, UNIT_WEAPON_DEVICES |
| `21` | `MRFSGun` | `20` | `00730BD0` | `00CFE308` | 00731C80 | GUN_CLASS_FAMILY |
| `22` | unnamed | `20` | `006FDE40` | `00CFBD20` | - | GUN_CLASS_FAMILY |
| `23` | `MRTGun` | `22` | `00730ED0` | `00CFE548` | 00731D50 | GUN_CLASS_FAMILY |
| `24` | `MSTGun` | `22` | `006FDF20` | `00CFBF58` | 00731E20 | GUN_CLASS_FAMILY |
| `25` | `MBombPlatform` | `20` | `006E3E10` | `00CF96A8` | 006E08F0 | GUN_CLASS_FAMILY |
| `26` | `MMultipleBombPlatform` | `25` | `006E43B0` | `00CF9918` | 006E0970 | GUN_CLASS_FAMILY |
| `27` | `MDepthChargeLauncher` | `24` | `006FE050` | `00CFC190` | 006FE390 | GUN_CLASS_FAMILY |
| `28` | `MCatapult` | `20` | `006EC7B0` | `00CFAAB8` | 006EC860 | GUN_CLASS_FAMILY |
| `29` | `MBullet` | `00` | `006E7C00` | `00CF9DF0` | 006E8430, also the MArtilleryBullet record | - |
| `2A` | `MBomb` | `02` | `006E2790` | `00CF9438` | 006E2C00 | - |
| `2B` | `MTorpedo` | `2A` | `00856260` | `00D0C3E8` | 00856420 | - |
| `2C` | `MDepthCharge` | `2A` | `006FCA80` | `00CFBA80` | 006FD210 | - |
| `2D` | unnamed | `2A` | `006FE9F0` | `00CFC910` | - | - |
| `2E` | `MDummyTarget` | `2D` | `00700AC0` | `00CFD018` | 00700DD0 | - |
| `2F` | `MDummyKamikazePlane` | `2D` | `006FEA30` | `00CFC698` | 006FF100 | - |
| `30` | `MDummySubmarine` | `2D` | `006FF8E0` | `00CFCB48` | 006FFFE0 | - |
| `31` | `MParatrooper` | `2A` | `007ABA50` | `00D05060` | 007AC4C0 | - |
| `32` | `MFlakBullet` | `29` | `0070CB80` | `00CFD5D0` | 0070CC30 | - |
| `33` | `MRocket` | `2A` | `0080ACB0` | `00D090E8` | 0080ADE0 | - |
| `34` | `MWaterMine` | `2A` | `0085EB20` | `00D0D130` | 0085F310 / WaterMine | SCENE_ENTITY_FACTORY |
| `35` | unnamed | `05` | `007004B0` | `00CFCD60` | - | - |
| `36` | `Stationary` | `00` | `00748B40` | `00CFF678` | 004F0BE0 | SCENE_ENTITY_FACTORY |
| `37` | unnamed | `00` | `00470BE0` | `00CE6490` | - | - |
| `38` | unnamed | `37` | `00479F60` | `00CE6130` | - | - |
| `39` | unnamed | `37` | `00472870` | `00CE5B90` | - | - |
| `3A` | unnamed | `37` | `004AF840` | `00CE7008` | - | - |
| `3B` | `Wreck` | `37` | `004B1F40` | `00CE71A8` | 004F0CC0 | SCENE_ENTITY_FACTORY |
| `3C` | unnamed | `37` | `004740E0` | `00CE5CF0` | - | - |
| `3D` | `Cloud` | `37` | `00476310` | `00CE5E68` | 004E9E40 | SCENE_ENTITY_FACTORY |
| `3E` | unnamed | `37` | `00470C10` | `00CE5A20` | - | - |
| `3F` | unnamed | `37` | `004A7BC0` | `00CE6AB0` | - | - |
| `40` | unnamed | `37` | `004AC8E0` | `00CE6E10` | - | - |
| `41` | `NavPoint` | `01` | `004E6970` | `00CE8550` | 004E99B0 | SCENE_ENTITY_FACTORY, LUA_BINDING_ENTITY_LOOKUP |
| `42` | `MovieCamPos` | `01` | `004E69C0` | `00CE86D8` | 004E9AE0 | SCENE_ENTITY_FACTORY, LUA_BINDING_ENTITY_LOOKUP |
| `43` | `MovieCamLookat` | `01` | `004E6A10` | `00CE8860` | 004E9C10 | SCENE_ENTITY_FACTORY, LUA_BINDING_ENTITY_LOOKUP |
| `44` | `Landscape` | `01` | `004F1360` | `00CEA090` | 004F1460 | SCENE_ENTITY_FACTORY, LUA_BINDING_ENTITY_LOOKUP |
| `45` | `MAirfield` | `05` | `006D20E0` | `00CF8C08` | 006D3110 / AirField | UNIT_PARTS, UNIT_DAMAGE_AND_DEATH, SCENE_ENTITY_FACTORY |
| `46` | `MShipyard` | `05` | `00846C00` | `00D0B770` | 00848380 / Shipyard | UNIT_PARTS, UNIT_DAMAGE_AND_DEATH, SCENE_ENTITY_FACTORY |
| `47` | `Path` | `01` | `00480930` | `00CE6290` | 004EA650 | SCENE_ENTITY_FACTORY, LUA_BINDING_ENTITY_LOOKUP |

Coverage: 71 of the 72 ids in `00h`-`47h` have a class test in the image; `1Fh` has none and no
constructor stamps it, so nothing in the shipped binary is of that class. 50 of the 72 are named.
The 21 unnamed ids are the family bases (`00h`-`06h`, `0Fh`, `1Eh`, `20h`, `22h`, `2Dh`, `35h`,
`37h`) and eight members of the `37h` family (`38h`, `39h`, `3Ah`, `3Ch`, `3Eh`, `3Fh`, `40h`);
abstract bases have no factory record, which is why they have no literal.

## Above `47h`

Included because the bucket array reaches `60h`. Not part of this packet's brief, and not named
beyond what the scene table gives.

| id | class | parent | class test | vtable(s) | name evidence |
| --- | --- | --- | --- | --- | --- |
| `48` | unnamed | `00` | `0080F9A0` | `00D092B8` | - |
| `49` | unnamed | `02` | `007B3320` | `00D054D0` | - |
| `4A` | `CameraPath` | `01` | `004E6920` | `00CE8390` | 004EA760 |
| `4B` | unnamed | `00` | `00A31C60` | `00D231A0` | - |
| `4C` | unnamed | `00` | `0042C010` | `00CE3B28`, `00CF4410`, `00CF4930` | - |
| `4D` | `SpawnPoint` | `02` | `004F1800` | `00CEA218` | 004F1A60 |
| `4E` | unnamed | `4C` | `004351E0` | `00CE3E60`, `00CE3FD0` | - |
| `4F` | unnamed | `56` | `006508B0` | `00CF6250` | - |
| `50` | unnamed | `56` | `0064B720` | `00CF5CE8` | - |
| `51` | unnamed | `4C` | `006051E0` | `00CF4238` | - |
| `52` | unnamed | `4C` | `0078FAC0` | `00D044B8` | - |
| `53` | unnamed | `4C` | `005177D0` | `00CEC1F8` | - |
| `54` | unnamed | `4C` | `0079A380` | `00D04750` | - |
| `55` | unnamed | `4C` | `0079D840` | `00D04B58` | - |
| `56` | unnamed | `4C` | `00519380` | `00CEC440`, `00CEC5F0` | - |
| `60` | unnamed | `02` | `007810F0` | `00D040B8` | - |

Two vtables, `00CE3E60` and `00CE3FD0`, carry the same chain `4Eh, 4Ch, 0` through two distinct
class tests `004351E0` and `00435360`. Either one class has two vtables or two classes share `4Eh`;
this packet did not read the constructors to tell them apart.

`SimpleEffect` (`5Bh`), `PeriodicEffect` (`5Ch`) and `FreeCamPos` (`5Dh`) are named by the scene
table and have no class test of their own; the first two install class `01`'s.

## Reconstruction

`include/bsp/entity_class_ids.hpp` and `src/entity_class_ids.cpp` carry the table as data and the
rule as two pure functions. There is no host table: the class test takes no native calls, and the
reconstruction is a projection over recovered data, not over an injected host. Status: exported,
analyzed, reconstructed, build-tested. Not ABI-compatible -- the native routine is a `__thiscall`
virtual reading `+C4h` off a real instance.

## What this does not say

- The 21 unnamed ids in range have no class-name literal anywhere in the image that this packet's
  scan could reach. Their parents and their class tests are settled; only the names are open.
- `1Fh` is not attested at all: no class test, no constructor stamp. `docs/SCENE_ENTITY_FACTORY.md`
  reached the same conclusion for `19h` from the scene side, but `19h` **is** a real class
  (`MLandVehicle`, test `0074DD90`); it is simply not scene-registered. `1Fh` may be the same kind
  of gap one step further, or a class removed before release.
- The three large plane descriptors (`MPlaneDiveBomber` `12h`, `MPlaneTorpedoBomber` `11h`,
  `MPlaneKamikaze` `17h`) rest on a per-record creator offset of `298h`, confirmed only by the
  record stride being uniform at `2D8h` and by each record holding exactly one stamping pointer.
  The set `{11h, 12h, 17h}` is certain, the assignment within it is provisional.
- `MKamikazePlane` (`00CFA4A0`) and `MArtilleryBullet` (`00CFA464`) are class-name literals whose
  record slot before the name is not a `.text` pointer, so neither was paired with an id.
  `MBullet` (`00CFA15C`) and `MArtilleryBullet` both sit before the same creator `006E8430`, which
  stamps `29h`.
- Which classes actually reach each branch at run time was not measured; this is a static survey.
- `docs/BOT_FIRE_TARGET.md`'s per-owner-kind lists over kinds `8`, `9`, `0Bh`, `0Ch` were not
  re-read. If those are class ids they are `MSubmarine`, `MMothership`, `MCargo` and
  `MLandingShip`; if they are entity-lookup buckets they are `0Eh`, `0Fh`, `11h` and `12h`. The
  call sites were not examined, so neither reading is asserted here.
