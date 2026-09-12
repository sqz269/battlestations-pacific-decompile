# The entity kind predicate at vtable slot `5Ch` (packet `cc_unit_kind_query`)

Addresses: `0042B8F0`, `004351E0`, `00435360`, `0047F190`, `00480930`, `004F1750`, `006D1610`,
`006D1650`, `006D20E0`, `006DFE50`, `006E3D50`, `006FDE40`, `006FDF20`, `006FE050`, `006FE530`,
`0074E400` (class-test bodies; all 88 are in `reports/unit_kind_query.json`), and the call sites
`004C3D38`, `004C3E05`, `004C3E32`, `004C3E49`, `006450CC`, `006450EA`, `0064332E`, `00642126`,
`0064227F`, `0072C6E1`, `0072F864`, `00730085`, `0077F2E6`, `0077F2F5`, `0077F304`, `007AC9DE`,
`007AC9F5`, `007ACA0C`, `007ACA23`, `007F1BA0`, `0085AF76`, `008FBCA1`, `0090316F`, `00922CC6`,
`00922D6A`, `00937CFD`, `0095AAFB`, `00959CC5`, `0099A26D`.

`docs/ENTITY_CLASS_IDS.md` (packet `cc2_class_id_table`) recovered the id space: the dword every
constructor stamps at `+C4h`, the id -> class -> parent tree, and the class names. This packet
reads the same slot from the other side: **every compiled body byte for byte**, so each class's
accepted-literal set is evidence rather than a derivation from the tree, and **every call site**,
so each query literal gets a meaning from the set of classes that answer it. The two readings
agree: for all 87 classes that packet listed, the accepted set decoded here equals its `ancestors`
list. The disagreements that remain are one missed body and one vtable, in `## Corrections`.

## How a call site is compiled, and how they were all found

A byte scan for `CALL dword ptr [reg+5Ch]` (`FF 5x 5C`) finds **zero** hits in `.text`. The
compiler never emits the direct form. The shape is always a load then an indirect call:

```
006bcd2f  8B 06           MOV  EAX, dword ptr [ESI]      ; the vtable
006bcd31  8B 50 5C        MOV  EDX, dword ptr [EAX + 5Ch] ; the class test
006bcd34  57              PUSH EDI                        ; a callee-save, not an argument
006bcd35  6A 09           PUSH 9                          ; the query literal
006bcd37  33 FF           XOR  EDI, EDI
006bcd39  FF D2           CALL EDX
006bcd3b  84 C0           TEST AL, AL
```

A linear sweep of `.text` (3.35M instructions) collecting every `CALL <reg>` whose register was
last defined by `MOV <reg>,[<base>+5Ch]` finds **1976** sites. The argument is the last `PUSH`
before the call; register saves can sit between the load and the call, as above, so a push count
is not a reliable argument count on its own. Of the 1976:

| | sites |
| --- | --- |
| last push is a literal in the class-id space `00h`-`60h` | 1739 |
| of those, immediately followed by `TEST AL,AL` | 1633 |
| last push is a register or a computed value (a forwarded query, e.g. `006E34A8`) | 234 |
| a different virtual that shares slot `5Ch` on a non-entity vtable | 3 |

The three excluded sites are `00ABF85C`, `00BC76A3` and `00BC78BA`. They push **three** arguments
(`PUSH EBP; PUSH 1; PUSH 92h` at `00ABF855`-`00ABF857`) through the singleton at `[00F8D394]`, and
their literals `92h` and `C8h` are outside the id space. The entity class test takes one stack
argument and returns with `RET 4`, so those are not it. Every other site in the sweep is one
argument wide.

The 88 bodies themselves were found independently of any vtable list, by anchoring on the 88
occurrences of `CMP EAX,dword ptr [ECX+0C4h]` (`3B 81 C4 00 00 00`) in `.text` and walking back to
the `INT3` padding that precedes each function. All 88 decode as the same shape and all 88 are
installed at some vtable's `+5Ch`:

```
MOV  EAX, dword ptr [ESP+4]     ; the query
CMP  EAX, imm8 / JZ  true       ; one compare per ancestor, the root as TEST EAX,EAX
CMP  EAX, dword ptr [ECX+0C4h]  ; the object's own most-derived id
JZ   true
XOR  EAX, EAX / RET 4
true: MOV EAX, 1 / RET 4
```

**No body contains a CALL.** The decoder stops at the first instruction outside that shape and
none of the 88 produced one, so there is no host method to implement: the predicate is closed over
the table, and the only run-time input beyond the query is the object's own `+C4h`.

## The class table

`accepts` is the literal set decoded from the body's compare run, ascending; the compiled order
differs (`006D20E0` puts its own id after the `+C4h` compare, `0064B720` runs `50h, 56h, 4Ch, 0`).
`extends` is the parent from `docs/ENTITY_CLASS_IDS.md`: at run time nothing is deferred, because
the whole chain is unrolled into the compare run, so the column says which class's chain this body
reproduces and adds one id to. Only the first vtable is listed when a body is installed at several
(`00`: also `00D19500`; `01`: also `00CE8D68`, `00D192E0`; `4C`: also `00CF4410`, `00CF4930`;
`56`: also `00CEC5F0`).

| id | class | body | vtable | accepts | extends |
| --- | --- | --- | --- | --- | --- |
| `00` | - | `0042B8F0` | `00D19120` | 00 | - |
| `01` | - | `0047F190` | `00CE8BD0` | 00, 01 | `00` |
| `02` | - | `004F1750` | `00D03E80` | 00, 01, 02 | `01` |
| `03` | - | `00888EA0` | `00D11138` | 00, 01, 03 | `01` |
| `04` | - | `006D1610` | `00D0DF70` | 00, 01, 02, 04 | `02` |
| `05` | - | `006D1650` | `00D1A698` | 00, 01, 02, 04, 05 | `04` |
| `06` | - | `006DFE50` | `00D09678` | 00, 01, 02, 04, 05, 06 | `05` |
| `07` | `MDestroyer` | `006FE530` | `00CFC3D0` | 00, 01, 02, 04, 05, 06, 07 | `06` |
| `08` | `MSubmarine` | `00853050` | `00D0BF80` | 00, 01, 02, 04, 05, 06, 08 | `06` |
| `09` | `MMothership` | `00758510` | `00D01630` | 00, 01, 02, 04, 05, 06, 09 | `06` |
| `0A` | `MCruiser` | `006FB3D0` | `00CFB738` | 00, 01, 02, 04, 05, 06, 0A | `06` |
| `0B` | `MCargo` | `006EB230` | `00CFA778` | 00, 01, 02, 04, 05, 06, 0B | `06` |
| `0C` | `MLandingShip` | `0074BC60` | `00CFFA30` | 00, 01, 02, 04, 05, 06, 0C | `06` |
| `0D` | `MBattleship` | `006DFE90` | `00CF90B0` | 00, 01, 02, 04, 05, 06, 0D | `06` |
| `0E` | `MTorpedoBoat` | `00857DC0` | `00D0C648` | 00, 01, 02, 04, 05, 06, 0E | `06` |
| `0F` | - | `0074E400` | `00D05F20` | 00, 01, 02, 04, 05, 0F | `05` |
| `10` | `MPlaneBomber` | `007D77F0` | `00D06638` | 00, 01, 02, 04, 05, 0F, 10 | `0F` |
| `11` | `MPlaneTorpedoBomber` | `009535C0` | `00D1A000` | 00, 01, 02, 04, 05, 0F, 11 | `0F` |
| `12` | `MPlaneDiveBomber` | `00953530` | `00D19D28` | 00, 01, 02, 04, 05, 0F, 12 | `0F` |
| `13` | `MPlaneFighter` | `007DDA80` | `00D06920` | 00, 01, 02, 04, 05, 0F, 13 | `0F` |
| `14` | `MReconPlane` | `0074E480` | `00D00070` | 00, 01, 02, 04, 05, 0F, 14 | `0F` |
| `15` | `MSmallReconPlane` | `0084C9F0` | `00D0BA80` | 00, 01, 02, 04, 05, 0F, 14, 15 | `14` |
| `16` | `MLargeReconPlane` | `0074E4E0` | `00D00308` | 00, 01, 02, 04, 05, 0F, 14, 16 | `14` |
| `17` | `MPlaneKamikaze` | `009534A0` | `00D1A2D8` | 00, 01, 02, 04, 05, 0F, 17 | `0F` |
| `18` | `PlaneSquadronGen` | `007EFB00` | `00D087C0` | 00, 01, 02, 18 | `02` |
| `19` | `MLandVehicle` | `0074DD90` | `00CFFDE0` | 00, 01, 02, 04, 05, 19 | `05` |
| `1A` | `LandConvoy` | `004F2560` | `00CEA570` | 00, 01, 02, 1A | `02` |
| `1B` | `MLandFort` | `006F5890` | `00CFF3F8` | 00, 01, 02, 04, 05, 1B | `05` |
| `1C` | `MCommandBuilding` | `006F58E0` | `00CFB028` | 00, 01, 02, 04, 05, 1B, 1C | `1B` |
| `1D` | `LandingPoint` | `004E9620` | `00CE90E0` | 00, 01, 1D | `01` |
| `1E` | - | `006E3D10` | `00CFDC58` | 00, 01, 02, 04, 1E | `04` |
| `20` | - | `006E3D50` | `00CFE0A8` | 00, 01, 02, 04, 1E, 20 | `1E` |
| `21` | `MRFSGun` | `00730BD0` | `00CFE308` | 00, 01, 02, 04, 1E, 20, 21 | `20` |
| `22` | - | `006FDE40` | `00CFBD20` | 00, 01, 02, 04, 1E, 20, 22 | `20` |
| `23` | `MRTGun` | `00730ED0` | `00CFE548` | 00, 01, 02, 04, 1E, 20, 22, 23 | `22` |
| `24` | `MSTGun` | `006FDF20` | `00CFBF58` | 00, 01, 02, 04, 1E, 20, 22, 24 | `22` |
| `25` | `MBombPlatform` | `006E3E10` | `00CF96A8` | 00, 01, 02, 04, 1E, 20, 25 | `20` |
| `26` | `MMultipleBombPlatform` | `006E43B0` | `00CF9918` | 00, 01, 02, 04, 1E, 20, 25, 26 | `25` |
| `27` | `MDepthChargeLauncher` | `006FE050` | `00CFC190` | 00, 01, 02, 04, 1E, 20, 22, 24, 27 | `24` |
| `28` | `MCatapult` | `006EC7B0` | `00CFAAB8` | 00, 01, 02, 04, 1E, 20, 28 | `20` |
| `29` | `MBullet` | `006E7C00` | `00CF9DF0` | 00, 29 | `00` |
| `2A` | `MBomb` | `006E2790` | `00CF9438` | 00, 01, 02, 2A | `02` |
| `2B` | `MTorpedo` | `00856260` | `00D0C3E8` | 00, 01, 02, 2A, 2B | `2A` |
| `2C` | `MDepthCharge` | `006FCA80` | `00CFBA80` | 00, 01, 02, 2A, 2C | `2A` |
| `2D` | - | `006FE9F0` | `00CFC910` | 00, 01, 02, 2A, 2D | `2A` |
| `2E` | `MDummyTarget` | `00700AC0` | `00CFD018` | 00, 01, 02, 2A, 2D, 2E | `2D` |
| `2F` | `MDummyKamikazePlane` | `006FEA30` | `00CFC698` | 00, 01, 02, 2A, 2D, 2F | `2D` |
| `30` | `MDummySubmarine` | `006FF8E0` | `00CFCB48` | 00, 01, 02, 2A, 2D, 30 | `2D` |
| `31` | `MParatrooper` | `007ABA50` | `00D05060` | 00, 01, 02, 2A, 31 | `2A` |
| `32` | `MFlakBullet` | `0070CB80` | `00CFD5D0` | 00, 29, 32 | `29` |
| `33` | `MRocket` | `0080ACB0` | `00D090E8` | 00, 01, 02, 2A, 33 | `2A` |
| `34` | `MWaterMine` | `0085EB20` | `00D0D130` | 00, 01, 02, 2A, 34 | `2A` |
| `35` | - | `007004B0` | `00CFCD60` | 00, 01, 02, 04, 05, 35 | `05` |
| `36` | `Stationary` | `00748B40` | `00CFF678` | 00, 36 | `00` |
| `37` | - | `00470BE0` | `00CE6490` | 00, 37 | `00` |
| `38` | - | `00479F60` | `00CE6130` | 00, 37, 38 | `37` |
| `39` | - | `00472870` | `00CE5B90` | 00, 37, 39 | `37` |
| `3A` | - | `004AF840` | `00CE7008` | 00, 37, 3A | `37` |
| `3B` | `Wreck` | `004B1F40` | `00CE71A8` | 00, 37, 3B | `37` |
| `3C` | - | `004740E0` | `00CE5CF0` | 00, 37, 3C | `37` |
| `3D` | `Cloud` | `00476310` | `00CE5E68` | 00, 37, 3D | `37` |
| `3E` | - | `00470C10` | `00CE5A20` | 00, 37, 3E | `37` |
| `3F` | - | `004A7BC0` | `00CE6AB0` | 00, 37, 3F | `37` |
| `40` | - | `004AC8E0` | `00CE6E10` | 00, 37, 40 | `37` |
| `41` | `NavPoint` | `004E6970` | `00CE8550` | 00, 01, 41 | `01` |
| `42` | `MovieCamPos` | `004E69C0` | `00CE86D8` | 00, 01, 42 | `01` |
| `43` | `MovieCamLookat` | `004E6A10` | `00CE8860` | 00, 01, 43 | `01` |
| `44` | `Landscape` | `004F1360` | `00CEA090` | 00, 01, 44 | `01` |
| `45` | `MAirfield` | `006D20E0` | `00CF8C08` | 00, 01, 02, 04, 05, 45 | `05` |
| `46` | `MShipyard` | `00846C00` | `00D0B770` | 00, 01, 02, 04, 05, 46 | `05` |
| `47` | `Path` | `00480930` | `00CE6290` | 00, 01, 47 | `01` |
| `48` | - | `0080F9A0` | `00D092B8` | 00, 48 | `00` |
| `49` | - | `007B3320` | `00D054D0` | 00, 01, 02, 49 | `02` |
| `4A` | `CameraPath` | `004E6920` | `00CE8390` | 00, 01, 4A | `01` |
| `4B` | - | `00A31C60` | `00D231A0` | 00, 4B | `00` |
| `4C` | - | `0042C010` | `00CE3B28` | 00, 4C | `00` |
| `4D` | `SpawnPoint` | `004F1800` | `00CEA218` | 00, 01, 02, 4D | `02` |
| `4E` | - | `004351E0` | `00CE3E60` | 00, 4C, 4E | `4C` |
| `4E` | - | `00435360` | `00CE3FD0` | 00, 4C, 4E | `4C` |
| `4F` | - | `006508B0` | `00CF6250` | 00, 4C, 4F, 56 | `56` |
| `50` | - | `0064B720` | `00CF5CE8` | 00, 4C, 50, 56 | `56` |
| `51` | - | `006051E0` | `00CF4238` | 00, 4C, 51 | `4C` |
| `52` | - | `0078FAC0` | `00D044B8` | 00, 4C, 52 | `4C` |
| `53` | - | `005177D0` | `00CEC1F8` | 00, 4C, 53 | `4C` |
| `54` | - | `0079A380` | `00D04750` | 00, 4C, 54 | `4C` |
| `55` | - | `0079D840` | `00D04B58` | 00, 4C, 55 | `4C` |
| `56` | - | `00519380` | `00CEC440` | 00, 4C, 56 | `4C` |
| `60` | - | `007810F0` | `00D040B8` | 00, 01, 02, 60 | `02` |

## The literal table

Every literal any call site passes, what it selects, and how many sites pass it. `classes` is the
number of the 87 classes whose body accepts it, which is exactly the subtree rooted at that id.
`meaning` is this packet's reading of that set together with the readers below; it is not a
recovered string.

| literal | sites | classes | selects |
| --- | --- | --- | --- |
| `00` | 13 | 87 | the root: every entity |
| `01` | 23 | 61 | every entity except the effect/wreck branch under 37h and the 4Ch tool branch |
| `02` | 69 | 52 | the commandable/simulated family: units, weapon devices, ordnance, squadrons, convoys |
| `04` | 32 | 36 | a unit or a weapon device |
| `05` | 107 | 25 | a unit: every ship, plane, land vehicle, fort, command building, airfield and shipyard |
| `06` | 200 | 9 | a ship |
| `07` | 6 | 1 | a destroyer |
| `08` | 110 | 1 | a submarine; selects hull-body mode 2 at 00937CFD |
| `09` | 80 | 1 | a carrier (MMothership); the carrier arm of the air-operations split |
| `0A` | 8 | 1 | a cruiser |
| `0B` | 9 | 1 | a cargo ship |
| `0C` | 21 | 1 | a landing ship |
| `0D` | 9 | 1 | a battleship |
| `0E` | 22 | 1 | a torpedo boat |
| `0F` | 161 | 9 | a plane |
| `10` | 55 | 1 | a level bomber; the levelbomb/divebomb discriminator |
| `11` | 1 | 1 | a torpedo bomber |
| `12` | 2 | 1 | a dive bomber |
| `13` | 7 | 1 | a fighter |
| `14` | 4 | 3 | a recon plane, either size |
| `16` | 41 | 1 | a large recon plane |
| `17` | 32 | 1 | a kamikaze plane |
| `18` | 171 | 1 | a plane squadron generator |
| `19` | 8 | 1 | a land vehicle |
| `1A` | 17 | 1 | a land convoy |
| `1B` | 47 | 2 | a land structure: fort or command building |
| `1C` | 66 | 1 | a command building |
| `1E` | 13 | 10 | the weapon-device family root |
| `1F` | 3 | 0 | nothing: no class test carries 1Fh and no constructor stamps it |
| `20` | 55 | 9 | a gun or weapon platform |
| `21` | 4 | 1 | a fixed gun (MRFSGun) |
| `22` | 10 | 4 | a turning gun: RT, ST and the depth-charge launcher |
| `23` | 3 | 1 | a rotating turret gun (MRTGun) |
| `24` | 13 | 2 | an ST gun or a depth-charge launcher |
| `25` | 24 | 2 | a bomb platform, single or multiple |
| `26` | 3 | 1 | a multiple bomb platform |
| `28` | 4 | 1 | a catapult |
| `29` | 8 | 2 | a bullet, normal or flak |
| `2A` | 19 | 10 | ordnance: the MBomb family, including torpedoes, rockets, mines and paratroopers |
| `2B` | 12 | 1 | a torpedo |
| `2C` | 3 | 1 | a depth charge |
| `31` | 4 | 1 | a paratrooper |
| `32` | 1 | 1 | a flak bullet |
| `33` | 5 | 1 | a rocket |
| `34` | 5 | 1 | a water mine |
| `35` | 3 | 1 | an unnamed unit-family class, asked beside 06h in the surface-target test |
| `36` | 9 | 1 | a Stationary scene object |
| `3A` | 4 | 1 | an unnamed member of the 37h effect family |
| `3D` | 1 | 1 | a cloud |
| `41` | 5 | 1 | a nav point |
| `44` | 18 | 1 | the landscape |
| `45` | 103 | 1 | an airfield |
| `46` | 60 | 1 | a shipyard |
| `47` | 6 | 1 | a Path |
| `48` | 1 | 1 | an unnamed root-derived class asked beside Path |
| `49` | 2 | 1 | an unnamed 02h-derived class asked beside Path |
| `4A` | 3 | 1 | a CameraPath |
| `4E` | 1 | 1 | an unnamed 4Ch-family class |
| `4F` | 2 | 1 | an unnamed 56h-derived class |
| `50` | 2 | 1 | an unnamed 56h-derived class |
| `51` | 2 | 1 | an unnamed 4Ch-derived class |
| `54` | 6 | 1 | an unnamed 4Ch-derived class |
| `55` | 1 | 1 | an unnamed 4Ch-derived class |

## What the family literals select

The sixteen literals that select more than one class, in full. Each of the other 47 selects exactly
one class, so its meaning is that class's name in the table above.

- **`00`** (87 classes, 13 sites): `00`, `01`, `02`, `03`, `04`, `05`, `06`, `07` `MDestroyer`, `08` `MSubmarine`, `09` `MMothership`, `0A` `MCruiser`, `0B` `MCargo`, `0C` `MLandingShip`, `0D` `MBattleship`, `0E` `MTorpedoBoat`, `0F`, `10` `MPlaneBomber`, `11` `MPlaneTorpedoBomber`, `12` `MPlaneDiveBomber`, `13` `MPlaneFighter`, `14` `MReconPlane`, `15` `MSmallReconPlane`, `16` `MLargeReconPlane`, `17` `MPlaneKamikaze`, `18` `PlaneSquadronGen`, `19` `MLandVehicle`, `1A` `LandConvoy`, `1B` `MLandFort`, `1C` `MCommandBuilding`, `1D` `LandingPoint`, `1E`, `20`, `21` `MRFSGun`, `22`, `23` `MRTGun`, `24` `MSTGun`, `25` `MBombPlatform`, `26` `MMultipleBombPlatform`, `27` `MDepthChargeLauncher`, `28` `MCatapult`, `29` `MBullet`, `2A` `MBomb`, `2B` `MTorpedo`, `2C` `MDepthCharge`, `2D`, `2E` `MDummyTarget`, `2F` `MDummyKamikazePlane`, `30` `MDummySubmarine`, `31` `MParatrooper`, `32` `MFlakBullet`, `33` `MRocket`, `34` `MWaterMine`, `35`, `36` `Stationary`, `37`, `38`, `39`, `3A`, `3B` `Wreck`, `3C`, `3D` `Cloud`, `3E`, `3F`, `40`, `41` `NavPoint`, `42` `MovieCamPos`, `43` `MovieCamLookat`, `44` `Landscape`, `45` `MAirfield`, `46` `MShipyard`, `47` `Path`, `48`, `49`, `4A` `CameraPath`, `4B`, `4C`, `4D` `SpawnPoint`, `4E`, `4F`, `50`, `51`, `52`, `53`, `54`, `55`, `56`, `60`
- **`01`** (61 classes, 23 sites): `01`, `02`, `03`, `04`, `05`, `06`, `07` `MDestroyer`, `08` `MSubmarine`, `09` `MMothership`, `0A` `MCruiser`, `0B` `MCargo`, `0C` `MLandingShip`, `0D` `MBattleship`, `0E` `MTorpedoBoat`, `0F`, `10` `MPlaneBomber`, `11` `MPlaneTorpedoBomber`, `12` `MPlaneDiveBomber`, `13` `MPlaneFighter`, `14` `MReconPlane`, `15` `MSmallReconPlane`, `16` `MLargeReconPlane`, `17` `MPlaneKamikaze`, `18` `PlaneSquadronGen`, `19` `MLandVehicle`, `1A` `LandConvoy`, `1B` `MLandFort`, `1C` `MCommandBuilding`, `1D` `LandingPoint`, `1E`, `20`, `21` `MRFSGun`, `22`, `23` `MRTGun`, `24` `MSTGun`, `25` `MBombPlatform`, `26` `MMultipleBombPlatform`, `27` `MDepthChargeLauncher`, `28` `MCatapult`, `2A` `MBomb`, `2B` `MTorpedo`, `2C` `MDepthCharge`, `2D`, `2E` `MDummyTarget`, `2F` `MDummyKamikazePlane`, `30` `MDummySubmarine`, `31` `MParatrooper`, `33` `MRocket`, `34` `MWaterMine`, `35`, `41` `NavPoint`, `42` `MovieCamPos`, `43` `MovieCamLookat`, `44` `Landscape`, `45` `MAirfield`, `46` `MShipyard`, `47` `Path`, `49`, `4A` `CameraPath`, `4D` `SpawnPoint`, `60`
- **`02`** (52 classes, 69 sites): `02`, `04`, `05`, `06`, `07` `MDestroyer`, `08` `MSubmarine`, `09` `MMothership`, `0A` `MCruiser`, `0B` `MCargo`, `0C` `MLandingShip`, `0D` `MBattleship`, `0E` `MTorpedoBoat`, `0F`, `10` `MPlaneBomber`, `11` `MPlaneTorpedoBomber`, `12` `MPlaneDiveBomber`, `13` `MPlaneFighter`, `14` `MReconPlane`, `15` `MSmallReconPlane`, `16` `MLargeReconPlane`, `17` `MPlaneKamikaze`, `18` `PlaneSquadronGen`, `19` `MLandVehicle`, `1A` `LandConvoy`, `1B` `MLandFort`, `1C` `MCommandBuilding`, `1E`, `20`, `21` `MRFSGun`, `22`, `23` `MRTGun`, `24` `MSTGun`, `25` `MBombPlatform`, `26` `MMultipleBombPlatform`, `27` `MDepthChargeLauncher`, `28` `MCatapult`, `2A` `MBomb`, `2B` `MTorpedo`, `2C` `MDepthCharge`, `2D`, `2E` `MDummyTarget`, `2F` `MDummyKamikazePlane`, `30` `MDummySubmarine`, `31` `MParatrooper`, `33` `MRocket`, `34` `MWaterMine`, `35`, `45` `MAirfield`, `46` `MShipyard`, `49`, `4D` `SpawnPoint`, `60`
- **`04`** (36 classes, 32 sites): `04`, `05`, `06`, `07` `MDestroyer`, `08` `MSubmarine`, `09` `MMothership`, `0A` `MCruiser`, `0B` `MCargo`, `0C` `MLandingShip`, `0D` `MBattleship`, `0E` `MTorpedoBoat`, `0F`, `10` `MPlaneBomber`, `11` `MPlaneTorpedoBomber`, `12` `MPlaneDiveBomber`, `13` `MPlaneFighter`, `14` `MReconPlane`, `15` `MSmallReconPlane`, `16` `MLargeReconPlane`, `17` `MPlaneKamikaze`, `19` `MLandVehicle`, `1B` `MLandFort`, `1C` `MCommandBuilding`, `1E`, `20`, `21` `MRFSGun`, `22`, `23` `MRTGun`, `24` `MSTGun`, `25` `MBombPlatform`, `26` `MMultipleBombPlatform`, `27` `MDepthChargeLauncher`, `28` `MCatapult`, `35`, `45` `MAirfield`, `46` `MShipyard`
- **`05`** (25 classes, 107 sites): `05`, `06`, `07` `MDestroyer`, `08` `MSubmarine`, `09` `MMothership`, `0A` `MCruiser`, `0B` `MCargo`, `0C` `MLandingShip`, `0D` `MBattleship`, `0E` `MTorpedoBoat`, `0F`, `10` `MPlaneBomber`, `11` `MPlaneTorpedoBomber`, `12` `MPlaneDiveBomber`, `13` `MPlaneFighter`, `14` `MReconPlane`, `15` `MSmallReconPlane`, `16` `MLargeReconPlane`, `17` `MPlaneKamikaze`, `19` `MLandVehicle`, `1B` `MLandFort`, `1C` `MCommandBuilding`, `35`, `45` `MAirfield`, `46` `MShipyard`
- **`06`** (9 classes, 200 sites): `06`, `07` `MDestroyer`, `08` `MSubmarine`, `09` `MMothership`, `0A` `MCruiser`, `0B` `MCargo`, `0C` `MLandingShip`, `0D` `MBattleship`, `0E` `MTorpedoBoat`
- **`0F`** (9 classes, 161 sites): `0F`, `10` `MPlaneBomber`, `11` `MPlaneTorpedoBomber`, `12` `MPlaneDiveBomber`, `13` `MPlaneFighter`, `14` `MReconPlane`, `15` `MSmallReconPlane`, `16` `MLargeReconPlane`, `17` `MPlaneKamikaze`
- **`14`** (3 classes, 4 sites): `14` `MReconPlane`, `15` `MSmallReconPlane`, `16` `MLargeReconPlane`
- **`1B`** (2 classes, 47 sites): `1B` `MLandFort`, `1C` `MCommandBuilding`
- **`1E`** (10 classes, 13 sites): `1E`, `20`, `21` `MRFSGun`, `22`, `23` `MRTGun`, `24` `MSTGun`, `25` `MBombPlatform`, `26` `MMultipleBombPlatform`, `27` `MDepthChargeLauncher`, `28` `MCatapult`
- **`20`** (9 classes, 55 sites): `20`, `21` `MRFSGun`, `22`, `23` `MRTGun`, `24` `MSTGun`, `25` `MBombPlatform`, `26` `MMultipleBombPlatform`, `27` `MDepthChargeLauncher`, `28` `MCatapult`
- **`22`** (4 classes, 10 sites): `22`, `23` `MRTGun`, `24` `MSTGun`, `27` `MDepthChargeLauncher`
- **`24`** (2 classes, 13 sites): `24` `MSTGun`, `27` `MDepthChargeLauncher`
- **`25`** (2 classes, 24 sites): `25` `MBombPlatform`, `26` `MMultipleBombPlatform`
- **`29`** (2 classes, 8 sites): `29` `MBullet`, `32` `MFlakBullet`
- **`2A`** (10 classes, 19 sites): `2A` `MBomb`, `2B` `MTorpedo`, `2C` `MDepthCharge`, `2D`, `2E` `MDummyTarget`, `2F` `MDummyKamikazePlane`, `30` `MDummySubmarine`, `31` `MParatrooper`, `33` `MRocket`, `34` `MWaterMine`

The lattice the AI actually uses is `02h` (commandable) > `04h` (unit or weapon device) > `05h`
(unit) > `06h` (ship) and `0Fh` (plane); beside it `1Eh` > `20h` (gun) > `22h` (turning gun) >
`24h`, and `2Ah` for ordnance. `05h` is the unit gate: it admits the nine-class ship family, the
nine-class plane family, `MLandVehicle`, `MLandFort`, `MCommandBuilding`, `MAirfield`, `MShipyard`
and the unnamed `35h`, and it excludes guns, ordnance, squadrons and convoys.

## Readers

Call sites read for this packet. Each row's containing function was taken from the live Ghidra
body range (`ghidra proto --brief`), not from the nearest preceding function start.

| site | function | literal | what the answer selects |
| --- | --- | --- | --- |
| `00937CFD` | `BSP_UnitController_BuildSubObjectsAndHullBody` | `08` | the hull-body mode word at `[ESP+0ACh]`: `2` when the unit is a submarine; otherwise `1`, dropped to `0` when the double at `00D7A220` is not greater than `[[unit+1Ch]+538h]+B0h` (`00937D16`-`00937D38`). This is the physics record `docs/SHIP_HULL_BODY.md` asked about: category `8` is `MSubmarine` and nothing else |
| `0099A26D`, `0099A29E`, `0099A2CF`, `0099A350`, `0099A394`, `0099A3C5` | `BSP_Bot_InstallCommandTask` | `02` | six guards on the command task's operand before it is installed: the operand must be a commandable entity, which admits units, weapon devices, ordnance and squadrons but not landscape, paths, nav points or wrecks |
| `0077F2E6`, `0077F2F5`, `0077F304` | `BSP_Unit_AssignSingleDeviceRole` (body `0077F2D0`-`0077F354`) | `05`, `1C`, `1B` | the triple that classifies a device's owner: unit, then command building, then land structure. Four sibling routines repeat the same triple with more arguments and a different helper (`0077F37D`, `0077F40D`, `0077F49D`, `0077F532`); Ghidra has no function at any of them, so they are **not** part of `BSP_Unit_AssignSingleDeviceRole`. See `## no_ghidra_function` |
| `004C3D38`, `004C3E05`, `004C3E32`, `004C3E49` | `BSP_Game_BuildLocalPlayerUnitLists` | `06`, `18`, `45`, `46` | which of the local-player lists an entity joins: ship, squadron, airfield, shipyard |
| `0064227F`, `00642126` | `BSP_InGameHudMarkers_BuildUnitMarker` | `0F`, `08` | the marker's plane arm and its submarine arm |
| `00922CC6`, `00922D6A` | `BSP_Entity_IsSurfaceTarget` | `06`, `35` | the surface-target test: a ship, or the unnamed unit class `35h` |
| `007AC9DE`, `007AC9F5`, `007ACA0C`, `007ACA23` | `BSP_Entity_PathInterfaceForKind` | `47`, `48`, `49`, `4A` | a four-way chain that picks the path interface: `Path`, then `48h`, then `49h`, then `CameraPath` |
| `0072C6E1` | `BSP_Gun_CreateAiBots` | `22` | the gun asks the turning-gun family of **itself**, after `[gun+3F4h]+80h == 1`, before allocating the aim bot |
| `0085AF76`, `0085B064` | `BSP_TurningGun_StepAim` | `23` | the `MRTGun`-only arm of the turret aim step |
| `0072F864`, `00730085` | `BSP_Gun_SpawnShotAndEffects` | `21`, `24` | the fixed-gun arm and the ST-gun/depth-charge-launcher arm of the shot spawn |
| `00959CC5`, `0095A004`, `0095A1E1`, `0095A471` | `BSP_Unit_ApplyGunAimMessage` | `20` | the weapon-device walk over `unit+48h`: each node must be a gun |
| `006450CC`, `006450EA`, `00645108` | `BSP_Unit_IsSelectableForControl` | `02`, `2A`, `45` | commandable, then not ordnance, then the airfield arm |
| `007F1BA0` | `BSP_Squadron_ReleaseFromAllAirBases` | `09` | the carrier arm of the base release |
| `0090316F` | `BSP_GunBot_BallisticAimAndFireTick` | `05` | a narrowing cast: `ECX = bot+50h` when it answers `05h`, else `ECX = 0` (`00903164`-`0090317B`), guarding the block at `0090318D` |
| `008FBCA1` | `BSP_GunBot_Attach` | `05` | the same narrowing on the value `008FBC90` stores into `bot+50h` from `[arg+28h]`: when it is a unit the bot takes `vtable[12Ch]` from it, otherwise `1` |
| `0064332E` | `BSP_InGameHudMarkers_AddUnitMarker` | `1F` | a second marker arm, reached only when the preceding test fails. `1Fh` is accepted by nothing (see Corrections) |
| `0095AAFB`, `0095AB06` | `FUN_0095A880` | `1F` | two consecutive `1Fh` tests on the same object, likewise never true |

## Coverage

| item | coverage |
| --- | --- |
| class-test bodies decoded from the image | complete: 88 of 88, each from `MOV EAX,[ESP+4]` to `RET 4`, no unexpected instruction |
| accepted-literal set per class | complete: decoded from the compare run, not derived from the tree |
| agreement with `docs/ENTITY_CLASS_IDS.md` ancestors | complete: 87 of 87 class rows agree exactly |
| slot-`5Ch` call sites located | complete for the compiled form `MOV <reg>,[<base>+5Ch]` + `CALL <reg>`: 1976 sites |
| query literal per call site | 1739 of 1976 are a compile-time literal in `00h`-`60h`; 234 pass a computed value and were not resolved; 3 are a different virtual |
| literal -> accepting class set | complete for all 63 literals that appear at a call site |
| literal meaning | partial: read from the accepting set for all 63, plus the 17 reader rows above. The remaining callers were counted, not read |
| containing function per call site | partial: verified against live Ghidra bodies only for the reader rows. The per-literal caller counts use the nearest preceding function start, which over-reaches into Ghidra gaps (`009E18FC` is past the end of `BSP_ShipAi_FollowStateStep`, body `009E1610`-`009E18C2`) |
| host methods the predicate needs | complete: none. No body among the 88 contains a CALL |
| `+C4h` stamps | partial: the 92 immediate stamps (`C7 8x C4 00 00 00`) were decoded; the 35 register-form stores to `+C4h` were checked for an immediate source but not traced to their producers |

## Corrections

- **`docs/ENTITY_CLASS_IDS.md`: the image holds 88 class tests, not 87.** Class `4Eh` has two
  byte-identical bodies, `004351E0` at vtable `00CE3E60+5Ch` and `00435360` at `00CE3FD0+5Ch`.
  That doc lists both vtables under `004351E0`; `00435360` is absent from it and has no Ghidra
  function. Evidence: the 88 occurrences of `CMP EAX,[ECX+0C4h]`, and reading `00CE3FD0+5Ch`
  directly, which holds `00435360`. Nothing about the id space changes: both bodies accept
  `{0, 4Ch, 4Eh}`. Named `BSP_ClassId4E_IsKindOfSecondBody` by this packet.
- **`IsKindOf(1Fh)` is false everywhere in the shipped build.** Three sites ask it (`0064332E`,
  `0095AAFB`, `0095AB06`). No class test in the image carries `1Fh` in its compare run, and no
  immediate stamp writes `1Fh` to `+C4h` (all 92 `MOV [reg+0C4h],imm32` decoded). The branches are
  unreachable unless one of the 35 register-form stores to `+C4h` can carry `1Fh`; none of those
  has an immediate source, and the two that copy a value copy it from another entity's `+C4h`
  (`004E8DF3`) or from an unrelated field.
- **`docs/RECON_SLOT_LISTS.md`: `IsKindOf(02h)` is not "the base of every scene entity".**
  `02h` is accepted by 52 of the 87 classes. The scene classes `LandingPoint` `1Dh`, `Stationary`
  `36h`, `Wreck` `3Bh`, `Cloud` `3Dh`, `NavPoint` `41h`, `MovieCamPos` `42h`, `MovieCamLookat`
  `43h`, `Landscape` `44h`, `Path` `47h` and `CameraPath` `4Ah` all answer it false, as does
  `MBullet` `29h`. What `02h` does admit beyond units is the weapon-device family `1Eh`-`28h`,
  the ordnance family `2Ah`-`34h`, `PlaneSquadronGen` `18h`, `LandConvoy` `1Ah`, `SpawnPoint`
  `4Dh`, `49h` and `60h`. The gate that means "a unit" is `05h`.
- **`docs/GUN_BOT_TICKS.md` row 1 of the `009030C0` table.** The row reads "`piVar8 = bot+50h`
  unless `gun->IsKindOf(5)` fails, in which case null". No class in the gun family (`1Eh`, `20h`
  through `28h`) carries `05h` in its compare run, so if `bot+50h` is the gun for this bot class
  the test cannot succeed and the guarded block at `0090318D` is dead. Provisional: this packet
  did not enumerate every attach path into that bot class, and the sibling sites that walk
  `bot+50h`'s `+3Ch` parent chain to the first `IsKindOf(5)` (`008FFFCE`, `006DFA60`) are a
  correct use of the same literal, because the walk ends at the owning unit.

## Follow-up packets

- `unit_kind_computed_query`: the 234 sites whose query is a register. `006E34A8`, `006E34E8` and
  `006E3FF8` are thin forwarders, but the rest may include a table-driven kind that would name new
  categories.
- `gun_bot_owner_slot`: settle what `bot+50h` holds for each of the six attach flavours
  (`006DF1F0`, `008FBDC0`, `008FBEC0`, `008FC060`, `00864BD0`, `008FF040`), which decides whether
  the `0090318D` block is dead. Owns `docs/GUN_BOT_TICKS.md`.
- `class_ids_above_47h`: `4Eh`-`56h` have bodies and call sites but no `+C4h` immediate stamp, so
  their instances are constructed some other way. `58h`, `5Bh` and `5Ch` are stamped but have no
  body of their own and are recognised only by the `+C4h` compare.
- `entity_kind_1f_history`: whether `1Fh` is a class cut from the shipped build, which would
  explain two live call sites that can never be true.

## no_ghidra_function

| start | end (inclusive) | evidence for the boundaries |
| --- | --- | --- |
| `00435360` | `00435386` | `INT3` padding `00435358`-`0043535F` ends the preceding function (`00435350 SUB ECX,24h` / `00435353 JMP 004353F0`); the body decodes linearly from `MOV EAX,[ESP+4]` at `00435360` to `RET 4` at `00435384`-`00435386`, and `INT3` padding resumes at `00435387`. `ghidra proto 00435360` reports no function |
| `0077F360` | `0077F3EB` | `INT3` padding `0077F355`-`0077F35F` after the end of `BSP_Unit_AssignSingleDeviceRole` (body `0077F2D0`-`0077F354`), and `0077F3EC`-`0077F3EF` before the next start. Prologue `MOV EAX,[ESP+8]` at `0077F360`, helper `00927D20`, the `05h`/`1Ch`/`1Bh` triple at `0077F37D`, `0077F38C`, `0077F39B` |
| `0077F3F0` | `0077F47B` | `INT3` padding `0077F3EC`-`0077F3EF` and `0077F47C`-`0077F47F`; the same triple at `0077F40D`, `0077F41C`, `0077F42B` |
| `0077F480` | `0077F50B` | `INT3` padding `0077F47C`-`0077F47F` and `0077F50C`-`0077F50F`; the same triple at `0077F49D`, `0077F4AC`, `0077F4BB` |
| `0077F510` | `0077F5A0` | `INT3` padding `0077F50C`-`0077F50F` and `0077F5A1`-`0077F5AF`; the same triple at `0077F532`, `0077F541`, `0077F550` |

The other 87 class-test bodies are Ghidra functions, named by packet `cc2_class_id_table`. The four
`0077Fxxx` rows are not class tests; they are readers found while attributing call sites, and they
are listed because the per-literal caller counts would otherwise credit their sites to
`BSP_Unit_AssignSingleDeviceRole`.
