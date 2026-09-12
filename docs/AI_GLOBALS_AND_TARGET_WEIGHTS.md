# AI globals and target weights

Addresses: 00A335D0, 00A371A0, 00A371C0, 00A08460, 00A31DB0, 00A32500, 00A2EEE0, 009FFC80,
00F8A8BC, 00F8A8C8, 00F8A734, 00F8AB08, 00F8AB5C, 00F8AB60

This packet answers the three follow-ups of `docs/LUA_BINDING_AI.md`: the `00F8A8BC` / `00F8A8C8`
overlap, the target-weight rule layout and the target-weight model, and whether `AIGetGroupInfo`
returns its table. Names below are hypotheses, not recovered symbols.

## Routines

| Address | Role | Coverage |
| --- | --- | --- |
| `00A335D0` | The AI globals loader. `__thiscall(ECX = AI coordinator)`, no arguments. | complete for the seven-mode field loop (00A335D0-00A3692F); `partial: 00A36930-00A37127` (the `SupportValues` and `ForcedTargetWeightValues` tails) |
| `00A371A0` | The active mode's tuning record. Body read in full, eight instructions. | complete |
| `00A371C0` | `AIReloadGlobals`' three instructions; `CALL 004C1C50`, `MOV ECX,EAX`, `JMP 00A335D0`. | complete |
| `009FFC80` | The game-mode index, 0..6. Read in full for this packet. | complete |
| `00A08460` | The target-weight model. | `partial: 00A0861F-00A09222` (the attacker-is-type-0Fh branch) |
| `00A31DB0` | The forced-target-weight lookup. Body read in full (00A31DB0-00A32048). | complete |
| `00A32500` | The forced-target-weight insert and remove. | complete for the identity comparison and the growth path (00A32500-00A325E0); `partial: 00A325E0-00A325F0` (the remove arm's tail) |
| `00A2EEE0` | The `AIGetGroupInfo` fill routine. Callee set enumerated, body not read. | `partial: body unread, 551 instructions` |

## 1. What the loader reads

`00A335D0` opens a Lua state (`BSP_LuaStateOwner_Open(0x41)`) and runs two scripts by path, in
order: `Scripts\global\luaMW_init.lua` (00A33600) then `Scripts\datatables\HighLvlAIGlobals.lua`
(00A3365C). Both strings are built with `BSP_NativeString_Resize` plus `memcpy`, lengths `0x1D`
and `0x27`. It then takes the global `HighLvlAIGlobals` (00A338xx) and loops.

The installed `scripts\datatables\highlvlaiglobals.lua` (1194 lines) is the shipped table and is
the producer for every key below; its Hungarian comments supply the semantics.

The loop is `XOR EDI,EDI` at `00A3374B`, `ADD EDI,1` / `CMP EDI,7` / `JL` at `00A370D4`, so seven
iterations. Each selects one sub-table through the `CMP EDI,n` chain at `00A3379D`..`00A3385C`:

| Mode | Sub-table | `009FFC80` arm |
| --- | --- | --- |
| 0 | `IslandCaptureParams_Rookie` | `009FFC9E`, difficulty clamped into 0..2 |
| 1 | `IslandCaptureParams_Regular` | same arm |
| 2 | `IslandCaptureParams_Veteran` | same arm |
| 3 | `DuelParams` | `009FFCD0`, literal 3 |
| 4 | `EscortParams` | `009FFCD9`, literal 4 |
| 5 | `SiegeParams` | `009FFCE2`, literal 5 |
| 6 | `CompetitiveParams` | `009FFCEB`, literal 6 |

The two index spaces are the same one, which is what makes section 3 decidable.

### The record

`IMUL EDX,EDX,0x23C` at `00A338DD` and `LEA ESI,[EDX + ECX]` at `00A338E3` give the store base,
with `ECX` from `[ESP+0F4h]`, the coordinator pointer passed in `ECX`. `00A371A0` is the reader
side and settles the base:

```
009FFC80()            ; mode, 0..6
004C1C50()            ; the AI coordinator
LEA EAX,[EAX + ESI*1 + 4]   ; 00A371B3
```

So the array starts at `coordinator + 4`, the stride is `23Ch`, and the loader's first store
(`+004h`) is the record's `+000h`. The record is exactly `23Ch` bytes and consecutive records do
not overlap: the last store, `ComposeGroup_TargetAgainstAttackerMul` at loader `+23Ch`
(`FSTP [ESI+23Ch]`, `00A36592`), is record `+238h`, the final float. `ESI` is written once for the
record, at `00A338E3`, and not again until `00A366BF`, after that store.

### Getters

Two shapes. `BSP_LuaObject_GetNumber` (`00B66270`) is the plain read and answers 0 for a missing
key. `BSP_LuaReference_GetFloatOrDefault` (`00B66330`) takes a float by value and answers it when
the key is absent; the `default` column below is that value, read out of the image.
Array-valued fields are reached with `BSP_LuaObject_GetByIndex` (`00B67720`), 1-based.

Six of the 143 record slots have no store in `00A335D0`: `+098h`, `+0A4h`, `+0B0h`, `+0BCh`,
`+0C8h` and `+0D4h`. Each is the third slot of a two-element weapon-params group, so the group is
laid out three wide and only `DogfightParams` and `StrafeParams` fill all three.

`AttackerReferenceSpeed` (`+064h`) is read even though the shipped script comments the key out, so
every mode gets its default of 1000.

### Key, slot, destination and default

| record | loader | key | slot | getter | default |
| --- | --- | --- | --- | --- | --- |
| `+000h` | `+004h` | `MotherShip` | - | `GetNumber` | n/a (plain read) |
| `+004h` | `+008h` | `BattleShip` | - | `GetNumber` | n/a (plain read) |
| `+008h` | `+00Ch` | `CommandBuilding` | - | `GetNumber` | n/a (plain read) |
| `+00Ch` | `+010h` | `Landfort` | - | `GetNumber` | n/a (plain read) |
| `+010h` | `+014h` | `Cruiser` | - | `GetNumber` | n/a (plain read) |
| `+014h` | `+018h` | `Destroyer` | - | `GetNumber` | n/a (plain read) |
| `+018h` | `+01Ch` | `Submarine` | - | `GetNumber` | n/a (plain read) |
| `+01Ch` | `+020h` | `LandingShip` | - | `GetNumber` | n/a (plain read) |
| `+020h` | `+024h` | `Cargo` | - | `GetNumber` | n/a (plain read) |
| `+024h` | `+028h` | `TBoat` | - | `GetNumber` | n/a (plain read) |
| `+028h` | `+02Ch` | `LevelBomber` | - | `GetNumber` | n/a (plain read) |
| `+02Ch` | `+030h` | `KamikazePlane` | - | `GetNumber` | n/a (plain read) |
| `+030h` | `+034h` | `TorpedoBomber` | - | `GetNumber` | n/a (plain read) |
| `+034h` | `+038h` | `DiveBomber` | - | `GetNumber` | n/a (plain read) |
| `+038h` | `+03Ch` | `Fighter` | - | `GetNumber` | n/a (plain read) |
| `+03Ch` | `+040h` | `ReconPlaneSmall` | - | `GetNumber` | n/a (plain read) |
| `+040h` | `+044h` | `ReconPlaneLarge` | - | `GetNumber` | n/a (plain read) |
| `+044h` | `+048h` | `OtherShip` | - | `GetNumber` | n/a (plain read) |
| `+048h` | `+04Ch` | `OtherPlane` | - | `GetNumber` | n/a (plain read) |
| `+04Ch` | `+050h` | `Other` | - | `GetNumber` | n/a (plain read) |
| `+050h` | `+054h` | `ValueRandomMul` | [1] | `GetNumber` | n/a (plain read) |
| `+054h` | `+058h` | `ValueRandomMul` | [2] | `GetNumber` | n/a (plain read) |
| `+058h` | `+05Ch` | `DogfightEquipmentPenalty` | - | `GetNumber` | n/a (plain read) |
| `+05Ch` | `+060h` | `MaxTargetKillRatio` | - | `GetNumber` | n/a (plain read) |
| `+060h` | `+064h` | `DamageCalcTime` | - | `GetNumber` | n/a (plain read) |
| `+064h` | `+068h` | `AttackerReferenceSpeed` | - | `GetFloatOrDefault` | `00ce3804` = 1000 |
| `+068h` | `+06Ch` | `Capture_SmallLandingShipSurviveMul` | - | `GetFloatOrDefault` | `00cee07c` = 0.75 |
| `+06Ch` | `+070h` | `Capture_InRangeCaptureMul` | - | `GetFloatOrDefault` | `00ce3800` = 0.5 |
| `+070h` | `+074h` | `Capture_LandedCaptureMul` | - | `GetFloatOrDefault` | `00ce74f8` = 0.8 |
| `+074h` | `+078h` | `Capture_LandedDamageMul` | - | `GetFloatOrDefault` | `00ce74f8` = 0.8 |
| `+078h` | `+07Ch` | `DogfightParams` | [1] | `GetNumber` | n/a (plain read) |
| `+07Ch` | `+080h` | `DogfightParams` | [2] | `GetNumber` | n/a (plain read) |
| `+080h` | `+084h` | `DogfightParams` | [3] | `GetNumber` | n/a (plain read) |
| `+084h` | `+088h` | `StrafeParams` | [1] | `GetNumber` | n/a (plain read) |
| `+088h` | `+08Ch` | `StrafeParams` | [2] | `GetNumber` | n/a (plain read) |
| `+08Ch` | `+090h` | `StrafeParams` | [3] | `GetNumber` | n/a (plain read) |
| `+090h` | `+094h` | `TailGunParams` | [1] | `GetNumber` | n/a (plain read) |
| `+094h` | `+098h` | `TailGunParams` | [2] | `GetNumber` | n/a (plain read) |
| `+09Ch` | `+0A0h` | `DivebombParams` | [1] | `GetNumber` | n/a (plain read) |
| `+0A0h` | `+0A4h` | `DivebombParams` | [2] | `GetNumber` | n/a (plain read) |
| `+0A8h` | `+0ACh` | `LevelbombParams` | [1] | `GetNumber` | n/a (plain read) |
| `+0ACh` | `+0B0h` | `LevelbombParams` | [2] | `GetNumber` | n/a (plain read) |
| `+0B4h` | `+0B8h` | `TorpedoParams` | [1] | `GetNumber` | n/a (plain read) |
| `+0B8h` | `+0BCh` | `TorpedoParams` | [2] | `GetNumber` | n/a (plain read) |
| `+0C0h` | `+0C4h` | `DCParams` | [1] | `GetNumber` | n/a (plain read) |
| `+0C4h` | `+0C8h` | `DCParams` | [2] | `GetNumber` | n/a (plain read) |
| `+0CCh` | `+0D0h` | `BigRocketParams` | [1] | `GetNumber` | n/a (plain read) |
| `+0D0h` | `+0D4h` | `BigRocketParams` | [2] | `GetNumber` | n/a (plain read) |
| `+0D8h` | `+0DCh` | `ShipDistWeight_AriveDist` | - | `GetFloatOrDefault` | `00d1af84` = 5000 |
| `+0DCh` | `+0E0h` | `ShipDistWeight_TravelTime` | [1] | `GetFloatOrDefault` | `00ceb4b0` = 60 |
| `+0E0h` | `+0E4h` | `ShipDistWeight_TravelTime` | [2] | `GetFloatOrDefault` | `00ce3ae8` = 300 |
| `+0E4h` | `+0E8h` | `ShipDistWeight_WeightMul` | [2] | `GetFloatOrDefault` | `00ce54a0` = 0.2 |
| `+0E8h` | `+0ECh` | `ShipDistWeight_WeightMul` | [1] | `GetFloatOrDefault` | `inline 3F800000h` = 1.0 |
| `+0ECh` | `+0F0h` | `PlaneDistWeight_AriveDist` | - | `GetFloatOrDefault` | `00d0946c` = 4000 |
| `+0F0h` | `+0F4h` | `PlaneDistWeight_TravelTime` | [1] | `GetFloatOrDefault` | `inline 3F800000h` = 1.0 |
| `+0F4h` | `+0F8h` | `PlaneDistWeight_TravelTime` | [2] | `GetFloatOrDefault` | `00d1a918` = 90 |
| `+0F8h` | `+0FCh` | `PlaneDistWeight_WeightMul` | [2] | `GetFloatOrDefault` | `00ce54a0` = 0.2 |
| `+0FCh` | `+100h` | `PlaneDistWeight_WeightMul` | [1] | `GetFloatOrDefault` | `inline 3F800000h` = 1.0 |
| `+100h` | `+104h` | `TravelTimeValue` | [1] | `GetNumber` | n/a (plain read) |
| `+104h` | `+108h` | `TravelTimeValue` | [2] | `GetNumber` | n/a (plain read) |
| `+108h` | `+10Ch` | `TravelTimeMul` | [2] | `GetNumber` | n/a (plain read) |
| `+10Ch` | `+110h` | `TravelTimeMul` | [1] | `GetNumber` | n/a (plain read) |
| `+110h` | `+114h` | `MachineGun` | [1] | `GetNumber` | n/a (plain read) |
| `+114h` | `+118h` | `MachineGun` | [2] | `GetNumber` | n/a (plain read) |
| `+118h` | `+11Ch` | `MachineGun` | [3] | `GetNumber` | n/a (plain read) |
| `+11Ch` | `+120h` | `MachineGun` | [4] | `GetNumber` | n/a (plain read) |
| `+120h` | `+124h` | `Artillery` | [1] | `GetNumber` | n/a (plain read) |
| `+124h` | `+128h` | `Artillery` | [2] | `GetNumber` | n/a (plain read) |
| `+128h` | `+12Ch` | `Artillery` | [3] | `GetNumber` | n/a (plain read) |
| `+12Ch` | `+130h` | `Artillery` | [4] | `GetNumber` | n/a (plain read) |
| `+130h` | `+134h` | `Bomb` | [1] | `GetNumber` | n/a (plain read) |
| `+134h` | `+138h` | `Bomb` | [2] | `GetNumber` | n/a (plain read) |
| `+138h` | `+13Ch` | `Bomb` | [3] | `GetNumber` | n/a (plain read) |
| `+13Ch` | `+140h` | `Bomb` | [4] | `GetNumber` | n/a (plain read) |
| `+140h` | `+144h` | `Torpedo` | [1] | `GetNumber` | n/a (plain read) |
| `+144h` | `+148h` | `Torpedo` | [2] | `GetNumber` | n/a (plain read) |
| `+148h` | `+14Ch` | `DepthCharge` | - | `GetNumber` | n/a (plain read) |
| `+14Ch` | `+150h` | `Paratroopers` | - | `GetNumber` | n/a (plain read) |
| `+150h` | `+154h` | `Kamikaze` | [1] | `GetNumber` | n/a (plain read) |
| `+154h` | `+158h` | `Kamikaze` | [2] | `GetNumber` | n/a (plain read) |
| `+158h` | `+15Ch` | `Kamikaze` | [3] | `GetNumber` | n/a (plain read) |
| `+15Ch` | `+160h` | `Kamikaze` | [4] | `GetNumber` | n/a (plain read) |
| `+160h` | `+164h` | `SmallRocket` | [1] | `GetNumber` | n/a (plain read) |
| `+164h` | `+168h` | `SmallRocket` | [2] | `GetNumber` | n/a (plain read) |
| `+168h` | `+16Ch` | `SmallRocket` | [3] | `GetNumber` | n/a (plain read) |
| `+16Ch` | `+170h` | `SmallRocket` | [4] | `GetNumber` | n/a (plain read) |
| `+170h` | `+174h` | `BigRocket` | [1] | `GetNumber` | n/a (plain read) |
| `+174h` | `+178h` | `BigRocket` | [2] | `GetNumber` | n/a (plain read) |
| `+178h` | `+17Ch` | `BigRocket` | [3] | `GetNumber` | n/a (plain read) |
| `+17Ch` | `+180h` | `BigRocket` | [4] | `GetNumber` | n/a (plain read) |
| `+180h` | `+184h` | `Flak` | [1] | `GetNumber` | n/a (plain read) |
| `+184h` | `+188h` | `Flak` | [2] | `GetNumber` | n/a (plain read) |
| `+188h` | `+18Ch` | `Flak` | [3] | `GetNumber` | n/a (plain read) |
| `+18Ch` | `+190h` | `Flak` | [4] | `GetNumber` | n/a (plain read) |
| `+190h` | `+194h` | `PartyPresence_DistanceMin` | - | `GetFloatOrDefault` | `00d0946c` = 4000 |
| `+194h` | `+198h` | `PartyPresence_DistanceMax` | - | `GetFloatOrDefault` | `00ce3d64` = 10000 |
| `+198h` | `+19Ch` | `Capture_ArriveToRangeTime` | - | `GetFloatOrDefault` | `00ce38c8` = 30 |
| `+19Ch` | `+1A0h` | `Capture_CapturePointResourceValue` | - | `GetFloatOrDefault` | `00ce4d90` = 35 |
| `+1A0h` | `+1A4h` | `Capture_MinimalResource` | - | `GetFloatOrDefault` | `00ce3808` = 150 |
| `+1A4h` | `+1A8h` | `Capture_CollectDefendersDist` | - | `GetFloatOrDefault` | `00d1af84` = 5000 |
| `+1A8h` | `+1ACh` | `Capture_ActAttackTargetWeightMul` | [2] | `GetFloatOrDefault` | `00ce380c` = 1.5 |
| `+1ACh` | `+1B0h` | `Capture_ActAttackTargetWeightMul` | [1] | `GetFloatOrDefault` | `00ce3d08` = 100 |
| `+1B0h` | `+1B4h` | `Capture_ActAttackTargetWeightMulDist` | [1] | `GetFloatOrDefault` | `00cfa424` = 3000 |
| `+1B4h` | `+1B8h` | `Capture_ActAttackTargetWeightMulDist` | [2] | `GetFloatOrDefault` | `00ceffe0` = 4500 |
| `+1B8h` | `+1BCh` | `Capture_MinimalCBTargetWeight` | - | `GetFloatOrDefault` | `00ce3800` = 0.5 |
| `+1BCh` | `+1C0h` | `Capture_CommandBuildingStrategicWeightMul` | - | `GetFloatOrDefault` | `00ce74f8` = 0.8 |
| `+1C0h` | `+1C4h` | `Capture_SpawnDelay` | [1] | `GetFloatOrDefault` | `00ce38c8` = 30 |
| `+1C4h` | `+1C8h` | `Capture_SpawnDelay` | [2] | `GetFloatOrDefault` | `00ce3850` = 5 |
| `+1C8h` | `+1CCh` | `Capture_SpawnDelayTime` | - | `GetFloatOrDefault` | `00d05804` = 120 |
| `+1CCh` | `+1D0h` | `FreeAttack_ObjectiveTargetMul` | - | `GetFloatOrDefault` | `00ce3958` = 2 |
| `+1D0h` | `+1D4h` | `FreeAttack_NearDist` | - | `GetFloatOrDefault` | `00d1af84` = 5000 |
| `+1D4h` | `+1D8h` | `FreeAttack_FarDist` | - | `GetFloatOrDefault` | `00ce3968` = 12000 |
| `+1D8h` | `+1DCh` | `FreeAttack_ExistingTargetMul` | - | `GetFloatOrDefault` | `00ce380c` = 1.5 |
| `+1DCh` | `+1E0h` | `Defend_MergeTargetDist` | - | `GetFloatOrDefault` | `00cffd60` = 2000 |
| `+1E0h` | `+1E4h` | `Defend_MergeGroupsDist` | - | `GetFloatOrDefault` | `00ce397c` = 500 |
| `+1E4h` | `+1E8h` | `Defend_CollectEnemiesDist` | - | `GetFloatOrDefault` | `00d23550` = 6000 |
| `+1E8h` | `+1ECh` | `Defend_AgainstEnemyResourceMul` | - | `GetFloatOrDefault` | `inline 3F800000h` = 1.0 |
| `+1ECh` | `+1F0h` | `Defend_MinimalResource` | - | `GetFloatOrDefault` | `00ce386c` = 200 |
| `+1F0h` | `+1F4h` | `CautionMove_Dist` | - | `GetFloatOrDefault` | `00d02f60` = 8000 |
| `+1F4h` | `+1F8h` | `CloseAttack_CollectDist` | - | `GetFloatOrDefault` | `00d1af84` = 5000 |
| `+1F8h` | `+1FCh` | `CloseAttack_NearDist` | - | `GetFloatOrDefault` | `00cfa424` = 3000 |
| `+1FCh` | `+200h` | `CloseAttack_FarDist` | - | `GetFloatOrDefault` | `00d02f60` = 8000 |
| `+200h` | `+204h` | `CloseAttack_ExistingTargetMul` | - | `GetFloatOrDefault` | `00ce380c` = 1.5 |
| `+204h` | `+208h` | `CloseAttack_TargetGroupMemberMul` | - | `GetFloatOrDefault` | `00ce3958` = 2 |
| `+208h` | `+20Ch` | `AutoMerge_MergeDist` | - | `GetFloatOrDefault` | `00d20180` = 650 |
| `+20Ch` | `+210h` | `AutoMerge_LeaveDist` | - | `GetFloatOrDefault` | `00cfd714` = 1200 |
| `+210h` | `+214h` | `Formation_UnitDist` | - | `GetFloatOrDefault` | `00ce3ae8` = 300 |
| `+214h` | `+218h` | `ComposeGroup_ReferenceWeight` | - | `GetFloatOrDefault` | `00ce38b8` = 10 |
| `+218h` | `+21Ch` | `ComposeGroup_AttackSumMul` | - | `GetFloatOrDefault` | `00ce3868` = 0.25 |
| `+21Ch` | `+220h` | `ComposeGroup_AttackerDontAttackPenalty` | - | `GetFloatOrDefault` | `00ce3958` = 2 |
| `+220h` | `+224h` | `ComposeGroup_TargetDontAttackedPenalty` | - | `GetFloatOrDefault` | `00ce3958` = 2 |
| `+224h` | `+228h` | `ComposeGroup_RepeatPenalty` | - | `GetFloatOrDefault` | `inline 3F800000h` = 1.0 |
| `+228h` | `+22Ch` | `ComposeGroup_SpeedBonusWeightRatio` | - | `GetFloatOrDefault` | `00ce3800` = 0.5 |
| `+22Ch` | `+230h` | `ComposeGroup_SpeedBonus` | - | `GetFloatOrDefault` | `inline 3F800000h` = 1.0 |
| `+230h` | `+234h` | `ComposeGroup_GroupCostModifier` | - | `GetFloatOrDefault` | `00ce7804` = 0.4 |
| `+234h` | `+238h` | `ComposeGroup_AttackerAgainstTargetMul` | - | `GetFloatOrDefault` | `00ce74f8` = 0.8 |
| `+238h` | `+23Ch` | `ComposeGroup_TargetAgainstAttackerMul` | - | `GetFloatOrDefault` | `00ce7804` = 0.4 |
| n/a | `00F8A8BC[mode]` | `Defend_ResourcePercent` | - | `de] GetFloatOrDefa` | `lt  00cf6560` = 0.35 |


## 2. The twelve-byte verdict

`docs/LUA_BINDING_AI.md` left the question open: `00F8A8BC` and `00F8A8C8` are twelve bytes apart,
`009FFC80` answers 0..6, so either slots 3..6 are unreachable or the shipped build writes out of
bounds. The loader settles it, and the answer is the second one.

**The array at `00F8A8BC` is three floats wide.** The store is guarded:

```
00A360F5: CMP EDI,0x3
00A360F8: JGE 0x00A3613E          ; modes 3..6 skip the read entirely
00A360FA: PUSH 0xD23500           ; "Defend_ResourcePercent"
00A36121: CALL 0x00B66330         ; GetFloatOrDefault(00CF6560 = 0.35)
00A36126: FSTP float ptr [EDI*0x4 + 0xF8A8BC]
```

`EDI` is 0, 1 or 2 there, so the loader touches `00F8A8BC`, `00F8A8C0` and `00F8A8C4` and nothing
further. This is not a case of the key being absent: all seven sub-tables of the shipped script
carry `Defend_ResourcePercent`, and the four non-island tables set it to `0.10`. The loader drops
those four values on purpose.

**`00F8A8C8` is the base of the 1Ch-stride per-party array**, independently:

```
00A25C06: MOV ECX,dword ptr [0x00E0E344]   ; the party index
00A25C0C: LEA EAX,[ECX*0x8 + 0x0]
00A25C13: SUB EAX,ECX
00A25C15: ADD EAX,EAX
00A25C17: ADD EAX,EAX                      ; EAX = party * 1Ch
00A25C1B: CMP byte ptr [EAX + 0xF8A8D4],BL ; the +0Ch valid byte
00A25C23: LEA EDX,[EAX + 0xF8A8D8]         ; the +10h vector 3
```

`0F8A8D4h - 0F8A8C8h` is `0Ch` and `0F8A8D8h - 0F8A8C8h` is `10h`, matching the offsets
`lua_binding_ai.hpp` already records for `AISetQuickSpawnTargetPos`.

**Two consumers index the three-float array with the 0..6 mode id.** `AISetDefendResourcePercent`
writes `MOVSS [ESI*4 + 0F8A8BCh],XMM0` at `00A37E4B` with `ESI` straight from `009FFC80`
(`00A37DF6`), and `00A29C38` reads `FLD dword ptr [EAX*4 + 0F8A8BCh]` with `EAX` from the same
call at `00A29C33`. For modes 3, 4, 5 and 6 both land on `00F8A8C8`, `00F8A8CC`, `00F8A8D0` and
`00F8A8D4`, that is party 0's first three floats and its quick-spawn valid byte.

So: **a latent out-of-bounds access in the shipped build**, on both the write and the read side,
reachable whenever the effective game mode is Duel, Escort, Siege or Competitive. A mission script
calling `AISetDefendResourcePercent` in those modes clobbers party 0's quick-spawn record instead
of setting a defend percentage, and `00A29C38` reads that record back as a percentage.
`islandcapture01.lua:1744` is the only shipped call and it runs in mode 0..2, so the shipped
missions do not reach it. No run-time evidence: `bsp_game.exe` does not reach these paths, which
need a loaded mission script.

## 3. The forced-target-weight rule

Two tables of `20h`-stride records: a global one (`00F8AB5C` base, `00F8AB60` count) and one per
game mode (`00F8AB08 + mode*0Ch` base, `00F8AB0C + mode*0Ch` count, `00F8AB10 + mode*0Ch`
capacity). `00A31DB0` scans the global table first and returns as soon as either table yields any
match (`00A31EEB`, `00A32024`). `00A32500` inserts and, with `DL` clear, first removes an identical
record.

| Offset | Field | Evidence |
| --- | --- | --- |
| `+00h` | attacker selector | `00A31E3A` / `00A31E4C`; compared against `attacker+70h` or passed to the vtable `+18h` query |
| `+04h` | target selector | `00A31E64` / `00A31E7C`, same two shapes against the target |
| `+08h` | byte, target-is-neutral | `00A31E20` / `00A31E2C`, matched against the query's fourth argument |
| `+0Ch` | float, the weight | `FLD float ptr [EDI+0Ch]` at `00A31EC2` |
| `+10h` | byte, relative form | `CMP byte ptr [EDI+10h],0` at `00A31EA3` |
| `+14h` | reference attacker selector | `00A32500`'s identity test compares `[5]` only when `[4]` is set |
| `+18h` | reference target selector | same test, `[6]` |
| `+1Ch` | byte, reference target-is-neutral | same test, `[7]` low byte |
| `+1Dh` | byte, attacker selector is an exact class id | `CMP byte ptr [EDI+1Dh],0` at `00A31E34` |
| `+1Eh` | byte, target selector is an exact class id | `CMP byte ptr [EDI+1Eh],0` at `00A31E64` |

This is the byte layout `docs/LUA_BINDING_AI.md` could not establish; its `AiTargetWeightRule`
carries the same ten values in write order, and the four- and six-argument forms of
`AISetTargetWeight` map onto `+00h/+04h/+08h/+0Ch` and `+00h/+04h/+08h/+14h/+18h/+1Ch/+0Ch`.
The shipped script's own comment confirms the meaning of the boolean: `neutral-e a tamadott`, is
the target neutral.

Matching is scored. A selector scores 2 on an exact class-id comparison and 1 on a type-group
query; a zero on either side rejects the rule; the rule's score is the sum, 2 to 4, the highest
scoring rule wins, and a 4 stops the scan (`00A31EBF`). Selector strings such as `"PLANE"` or
`"SHIP"` are the group form and names such as `"BATTLESHIP"` the exact form; both come through the
97-entry `char*` table at `00E0CD80` that the loader walks at `00A36BE4` and `00A36D84`.

**The relative form does not work.** When `+10h` is set, `00A31DB0` evaluates a base value by
calling `00A08460` again:

```
00A31EA9: MOV EAX,dword ptr [ESP + 0x28]   ; the query's flag
00A31EAD: MOV EDX,dword ptr [ESP + 0x18]   ; the query's EDX
00A31EB1: MOV ECX,dword ptr [ESP + 0x10]   ; the query's attacker
00A31EB5: PUSH EAX
00A31EB6: PUSH EBX                          ; the query's target
00A31EB7: CALL 0x00A08460
```

Those four stack slots are the lookup's own arguments, written once each in the prologue
(`00A31DB6`, `00A31DBA`, `00A31DE8`, `00A31DED`) and never again; `00A31DB0` never reads `+14h`,
`+18h` or `+1Ch`. So the call is the identical query that reached `00A31DB0` in the first place,
and `00A08460` writes its memo entry only at the end, so nothing breaks the cycle. A rule with the
relative flag recurses until the stack is exhausted. The shipped `ForcedTargetWeightValues` tables
use only the four-element form and the seven-element example is commented out, and no shipped
mission script uses the six-argument `AISetTargetWeight`, so the path is unreachable as shipped.

## 4. The target-weight model

`float __fastcall FUN_00a08460(void* attacker /*ECX*/, int attacker_class /*EDX*/,
void* target /*+4*/, int target_is_neutral /*+8*/)`, `RET 8` at `00A0980C`, weight in ST0. `EDI`
takes the first stack argument at `00A08486` and `EBX` the second at `00A0847C`, both before the
callee-saved pushes.

The four values are copied verbatim into a 16-byte key at `00A084A3`..`00A084BC` and that key
addresses a memoisation map at `00F8A734` with end sentinel `00F8A738`.

```
weight(attacker, attacker_class, target, neutral):
  it = memo_find(00F8A734, key)                     ; 00A084C3 -> 00A03B90
  if it != end:  return *(float*)(it + 1Ch)          ; 00A0851A
  if attacker == 0 or target == 0: return 0.0        ; 00A08522, 00A0852A -> FLDZ 00A097F2
  if forced_rule(key, &w):                           ; 00A08540 -> 00A31DB0
      memo_insert(key) = w;  return w                ; 00A08556 -> 00A079B0
  tuning = 00A371A0()                                ; the current mode's record
  hp     = *(float*)(target + 48h)                   ; 00A08593
  if attacker->is_type(0Fh):                         ; 00A085AD, branch at 00A08619
      total = <00A0861F..00A09222, not projected>
  else:
      total = barrel_model(...)                      ; 00A09228..00A09733, below
  if total > 0:                                      ; 00A09740
      if attacker->is_type(0Fh) and not attacker->is_type(14h):
          total *= 3.0                               ; 00A09771, double at 00D7A2B0
      r = total / hp                                 ; 00A09783
      result = r < 0 ? 0 : min(r, tuning.MaxTargetKillRatio)   ; 00A0979C, 00A097AD
  else:
      result = 0
  memo_insert(key) = result                          ; 00A097CC
  return result
```

The result is a **ratio**, the damage the attacker can do to the target in `DamageCalcTime`
seconds divided by the target's hit points, capped at `MaxTargetKillRatio`. That is exactly what
the shipped script's comment on `MaxTargetKillRatio` describes.

### The barrel model, `00A09228`..`00A09733`

`ESI` holds `00A371A0() + 50h` throughout (`00A08574`..`00A0857C`), so `ESI+0Ch` is record `+05Ch`
= `MaxTargetKillRatio`, `ESI+10h` is `+060h` = `DamageCalcTime`, `ESI+18h` is `+068h` =
`Capture_SmallLandingShipSurviveMul`, `ESI+1Ch` is `+06Ch` = `Capture_InRangeCaptureMul`, `ESI+20h`
is `+070h` = `Capture_LandedCaptureMul` and `ESI+24h` is `+074h` = `Capture_LandedDamageMul`.

```
total = 0 ; capture = 0
for each subsystem s of attacker[+94h], attacker[+98h] entries:          ; 00A095E3
    best = 0
    for each barrel b of s[+74h], s[+78h] entries, stride 48h:           ; 00A095C3
        acc = 009FE270(target)                                           ; 00A094E6
        if not acc > 0: continue                                         ; 00A094F5
        shots  = 0072AB80(b)                                             ; 00A09501
        reload = b[+2Ch]                                                 ; 00A09506
        f = reload > 1e-30 (00D7A218) ? DamageCalcTime / reload : 1.0     ; 00A0950B..00A09533
        dmg = f * acc * (float)shots                                      ; 00A09544, 00A09548
        w   = dmg * 009FE200(b[+B8h], b[+B0h], limit, hp)                 ; 00A09578
        best = max(best, w)                                               ; 00A09593
        capture += dmg * b[+BCh]                                          ; 00A095A9
    total += best                                                         ; 00A095D8
capture = min(capture, DamageCalcTime)                                    ; 00A09602..00A0961E
total  += 00424C40()[+3B0h] * capture                                     ; 00A09624..00A09637
if total / hp > MaxTargetKillRatio: total = hp * MaxTargetKillRatio        ; 00A09645..00A09660
if target_kind == 1Ch and attacker->is_type(6):                           ; 00A0966A, 00A09675
    if attacker->is_type(0Ch) and attacker[+809h] == 0:
        land = 1.0 (00D7A24C) ; cb = attacker                             ; 00A09698
    else:
        cb = attacker[+78Ch]
        if cb: land = (attacker[+790h] / (attacker[+794h] / DamageCalcTime))
                      * Capture_SmallLandingShipSurviveMul                ; 00A096B4..00A096CA
        if attacker != cb:
            total += attacker[+804h] * DamageCalcTime
                     * Capture_InRangeCaptureMul                          ; 00A096D2..00A096E2
    if cb:
        if not (flag == 1):                                               ; 00A096EA
            v = (float)cb[+80Ch]
            if v > target[+4Ch]:
                total += (v - target[+4Ch]) * land * DamageCalcTime
                         * Capture_LandedDamageMul                        ; 00A0970B..00A09719
        total += land * (float)cb[+810h] * DamageCalcTime
                 * Capture_LandedCaptureMul                               ; 00A09723..00A09733
```

`target_kind` is the target's vtable `+1Ch` result, saved at `00A08598`. The fourth argument
reaches the branch as `SETZ` of `CMP EBX,1` at `00A085D7`, stored at `[ESP+1Eh]`.

Not read further: `009FE270` (the per-barrel accuracy), `009FE200` (the four-argument distance
falloff), `0072AB80` (the shot count), `00424C40` (the object holding the capture scale at `+3B0h`)
and `00A03B00`/`00A001D0` (the STL accessors). They are contracts in the host table below.

### Call sites

Eight callers. Each passes the same four values; the argument contract is uniform.

| Site | Containing function | `this`/args | Read |
| --- | --- | --- | --- |
| `00A383C4` | `BSP_LuaBinding_AIGetTargetWeight` | `ECX` and `EDX` from `00964790` class descriptors, target and flag from the frame | yes, and in `docs/LUA_BINDING_AI.md` |
| `00A0C2D1` | `FUN_00a0c1f0` | `ECX = EBP`, `EDX = EBX`, target from `[ECX+EDI*4]`, flag literal 0 | yes |
| `00A0C2EE` | `FUN_00a0c1f0` | same shape, second arm | site not read further |
| `00A0C345` | `FUN_00a0c330` | `ECX = A[0]`, `EDX = A[10h]`, target `B[0]`, flag `B[1Ch]`, from two records in `ECX`/`EDX` | yes |
| `00A0F843` | `FUN_00a0f810` | same two-record shape after two `00A04560` calls | yes |
| `00A0AE80` | `FUN_00a0a280` | `ECX` from `[ESP+2Ch]`, `EDX` from `[ESP+EDI*4+94h]`, target `EBX`, flag `EBP` | yes |
| `00A09F19`, `00A09FFB` | `FUN_00a09810` | not read further | no |
| `00A0BD03` | `FUN_00a0b420` | not read further | no |
| `00A31EB7`, `00A31FDF` | `FUN_00a31db0` | the relative-rule re-entry, section 3 | yes |

## 5. Does `AIGetGroupInfo` return its table?

**No. It returns nothing.** `00A378C0` creates the table with `00B67930` at `00A3798B`, fills it
with `00A2EEE0(ECX = group)(&table)` at `00A3799F`, and takes its result count from
`00B66400(ECX = [ESP+0Ch])` at `00A379A8`, whose whole body is

```
00B66403: MOV EAX,dword ptr [ESI]
00B66405: MOV ECX,dword ptr [EAX + 0x4]
00B66408: CALL 0x00A673D0          ; lua_gettop
00B6640D: SUB EAX,dword ptr [ESI + 0xC]   ; minus the saved top
```

so the count is the live stack delta. `00B67930` takes a registry reference, which pops the table
it creates, and every Lua callee of `00A2EEE0` is a `LuaObject` accessor rather than a push:
`BSP_LuaObject_GetByName` (`00B67800`), `BSP_LuaObject_SetBoolean` (`00B673A0`),
`BSP_LuaObject_SetNewTable` (`00B67580`), `BSP_LuaObject_SetObject` (`00B675D0`),
`BSP_LuaObject_Destruct` (`00B67700`), plus `00B666C0` and `00B67460`. `00B673A0` was read: it
pushes the key and the value and settles them into the table, net zero. The table LuaObject is then
destructed at `00A379AD` before the handler returns, so the table is unreachable from the script.

Coverage: this rests on the callee set and on `00B66400`, not on `00A2EEE0`'s 551 instructions,
which were not read. `00B666C0`, `00B67460`, `00B67580` and `00B675D0` were not read either. If
one of those four pushed without popping the answer would change; all four are `LuaObject`
setters by name and by their position in `docs/LUA_OBJECT_API.md`'s family.

## 6. Host table

One row per native call site the reconstruction crosses. `gate` is the condition under which the
site runs.

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `00A33600` | `00B65EE0` | `run_script` | `(path)` / none | always |
| `00A3365C` | `00B65EE0` | `run_script` | `(path)` / none | always |
| `00A338xx` | `00B67800` | `push_field` | `ECX = parent` `(key)` / object | always |
| `00A338F0` | `00B66270` | `read_number` | `ECX = object` / ST0 | plain-read fields |
| `00A33xxx` | `00B66330` | `read_number_or` | `ECX = object` `(float)` / ST0 | defaulted fields |
| `00A337xx` | `00B67720` | `push_index` | `ECX = object` `(1-based)` / object | array-valued fields |
| `00A33904` | `00B67700` | `pop_field` | `ECX = object` / none | after every field |
| `00A371A8` | `004C1C50` | `mode_record` | none / coordinator | `00A371A0` and `00A371C0` |
| `00A36126` | none (inline `FSTP`) | `store_defend_resource_percent` | `[EDI*4+0F8A8BCh]` | `EDI < 3` |
| `00A36FD5` | `00A32500` | `insert_forced_rule` | `ECX = record` `DL = 1` `(-1)` | one per `ForcedTargetWeightValues` entry |
| `00A084C3` | `00A03B90` | `memo_lookup` | `ECX = 00F8A734` `(out, key)` / iterator | always |
| `00A08556`, `00A097CC` | `00A079B0` | `memo_store` | `ECX = 00F8A734` `(key)` / `float*` | on the override return and on the normal return |
| `00A08540` | `00A31DB0` | `forced_rule_weight` | `ECX = attacker` `EDX = class` `(target, flag, out)` / `AL` | after the memo miss |
| `00A08574` | `00A371A0` | `mode_tuning` | none / record + 4 | after the override miss |
| `00A08588`, `00A08591` | vtable `+1Ch` | `entity_kind` | `ECX = entity` / int | always; the target's value gates `00A0966A` |
| `00A085AD` | vtable `+18h` | `entity_is_type` | `ECX = entity` `(code)` / `AL` | always; result is the top-level branch |
| `00A08593`, `00A085A8` | none (inline `MOVSS`) | `target_hit_points`, `target_capture_state` | `target+48h`, `target+4Ch` | always |
| `00A09379` | `00A03B00` | (STL accessor, contract: unread) | `ECX = list` / node | per subsystem |
| `00A094E6` | `009FE270` | `barrel_accuracy` | `(target)` / ST0 | per barrel |
| `00A09501` | `0072AB80` | `barrel_shots` | `ECX = barrel` / `EAX` | accuracy above zero |
| `00A09578` | `009FE200` | `distance_falloff` | four floats / ST0 | accuracy above zero |
| `00A09624` | `00424C40` | `capture_scale` | none / object, `+3B0h` read | after the subsystem loop |
| `00A0975E` | vtable `+18h` | `entity_is_type` | `ECX = attacker` `(14h)` / `AL` | `total > 0` and type `0Fh` |
| `00A3798B` | `00B67930` | (table creator, contract: read) | `ECX = state` `(out)` / registry object kind 2 | `AIGetGroupInfo` |
| `00A3799F` | `00A2EEE0` | (group fill, contract: partial) | `ECX = group` `(table)` | `AIGetGroupInfo` |
| `00A379A8` | `00B66400` | (result count, contract: read) | `ECX = frame` / `lua_gettop - saved` | `AIGetGroupInfo` |

## 7. Open questions

- The attacker-is-type-`0Fh` branch, `00A0861F`..`00A09222`. Ghidra's recovery of it is poor
  (`unaff_EBP`, a float local used as a pointer, byte-packed flag words) and it was not
  transcribed. It reaches `009552E0` with the attacker class, walks a list, and picks one of six
  `PTR_PTR_00E08F..` descriptors before joining the common epilogue at `00A09737`.
- `009FE200`'s four arguments. The transcription above names them positionally from the pushes at
  `00A0955E`..`00A09575`; the callee was not read, so their meaning rests on the
  `ShipDistWeight_*` / `PlaneDistWeight_*` keys rather than on the body.
- The `SupportValues` and `ForcedTargetWeightValues` tails of the loader, `00A36930`..`00A37127`.
  Their shape is visible (two `00E0CD80` name walks and a `00A32500` insert at `00A36FD5`) but the
  stack record they assemble was not transcribed field by field.
- `00A2EEE0`'s body, and the four unread `LuaObject` setters named in section 5.
