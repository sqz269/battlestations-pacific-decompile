# The two inputs the AI command tick was running without

Addresses: 009FFD70, 009FFD80, 009FDF30, 009FDFF0, 009FDFED, 00A2D8E0, 00A371A0, 00A335D0,
009FE0B0, 009FE080, 00D7A24C, 00CE3D08, 00D7A238, 00D1AF84, 00D02F60.

Packet `cc8_ai_command_inputs`, read-only analysis. Every descriptive name here is a hypothesis,
not a recovered symbol. The key names are genuine string literals from the shipped Lua table.

`docs/AI_COMMAND_TICK.md` closed with two named missing inputs. Both are resolved here, and one of
them turned out to be named wrongly.

## 1. The member ordering key is `009FFD80`, not `009FFD70`

**Correction.** The brief and `docs/AI_COMMAND_TICK.md` both named `009FFD70` as the member
ordering key. It is not. `009FFD70` is two instructions, `MOV ECX,[ECX+0C4h]` then `JMP 009FDF30`,
so it is the class weight of the entity's own class id; its three callers are
`00A10D50 BSP_AiCommand_MergeLeaderStrengthOk`, `00A2C4C0 BSP_AiGroup_TotalLeaderWeight` and
`00A2E720 BSP_AiGroups_ComposePass`. The sorted insert `00A2D8E0` calls **`009FFD80`** instead, as
its callee list and the call at `00A2D90D` show.

### `009FDF30`, the class weight

`__fastcall(int class_id) -> float`. `LEA EAX,[ECX-6]` and `CMP EAX,0x16` bound the range, then
`JMP [EAX*4 + 009FDFF0]` picks an arm; each arm calls `00A371A0` and loads one float of the tuning
record. Resolved arm by arm:

| Class id | Record offset | Field | Class id | Record offset | Field |
| --- | --- | --- | --- | --- | --- |
| `06` | `+044h` | `OtherShip` | `11` | `+030h` | `TorpedoBomber` |
| `07` | `+014h` | `Destroyer` | `12` | `+034h` | `DiveBomber` |
| `08` | `+018h` | `Submarine` | `13` | `+038h` | `Fighter` |
| `09` | `+000h` | `MotherShip` | `15` | `+03Ch` | `ReconPlaneSmall` |
| `0A` | `+010h` | `Cruiser` | `16` | `+040h` | `ReconPlaneLarge` |
| `0B` | `+020h` | `Cargo` | `17` | `+02Ch` | `KamikazePlane` |
| `0C` | `+01Ch` | `LandingShip` | `1B` | `+00Ch` | `Landfort` |
| `0D` | `+004h` | `BattleShip` | `1C` | `+008h` | `CommandBuilding` |
| `0E` | `+024h` | `TBoat` | `14`, `18`, `19`, `1A` | — | `FLD1` at `009FDFED`, 1.0f |
| `0F` | `+048h` | `OtherPlane` | anything out of range | — | the same 1.0f |

### `009FFD80 BSP_Entity_AiLeaderWeight`

`__thiscall(entity)`, body `009FFD80`-`009FFDFB`, read in full.

```
mul = [00D7A24C] = 1.0f
if (entity->vtable[+5Ch](6))                     mul = [00CE3D08] = 100.0f
else if (vtable[+5Ch](1Bh) || (45h) || (46h))    mul = [00D7A238] = 0.01f
return 009FDF30(entity+C4h) * mul
```

The 1Bh/45h/46h trio is exactly what `009FE0B0` asks, which is the producer-side confirmation that
the two routines classify the same three entity classes.

### The sort direction

`00A2D8E0` computes the candidate's weight once at `00A2D90D` and stores it at `[ESP+0Ch]`. The
walk calls `009FFD80` on each existing member at `00A2D941`, pushes the candidate's weight with
`FLD`, compares with `FCOMPI` at `00A2D94A` and takes `JA` at `00A2D94E` to insert before that
node. `JA` needs both `CF` and `ZF` clear, so an equal weight does not stop the walk.

**The member list is ordered by descending leader weight, ties keep insertion order, and the first
member is the leader** that every tick, every merge test and every order reads.

## 2. The tuning loader was reading six keys

`00A335D0` loads a long contiguous run of `GetFloatOrDefault` keys plus the twenty `GetNumber`
per-class weights. The host's loader carried six. It now carries thirty-three: the twenty class
weights at `+000h`..`+04Ch`, the `CautionMove`/`CloseAttack` block at `+1F0h`..`+204h`,
`Formation_UnitDist` at `+210h`, and the six it already had.

The shipped values are read out of the installed `scripts/datatables/highlvlaiglobals.lua`, whose
seven sub-tables start at lines 3, 191, 377, 563, 718, 887 and 1042. Four of the new keys are not
the image default:

| Key | Record | Image default | Shipped |
| --- | --- | --- | --- |
| `CloseAttack_CollectDist` | `+1F4h` | 5000 (`00D1AF84`) | **3000**, and 6000 in Escort |
| `CloseAttack_ExistingTargetMul` | `+200h` | 1.5 | **1.8** |
| `CloseAttack_TargetGroupMemberMul` | `+204h` | 2 | **10** |
| `CautionMove_Dist` | `+1F0h` | 8000 | **40000** in IslandCaptureRookie only |

The per-class weights vary by mode too: `BattleShip` is 12 in IslandCaptureRookie against 20
everywhere else, and `TorpedoBomber` and `DiveBomber` drop from 8 to 1.5 in Duel, Siege and
Competitive. A campaign mission runs IslandCaptureRookie, so `CloseAttack_CollectDist` is 3000 and
`BattleShip` is 12.

## Host methods

| Host method | Native | Note |
| --- | --- | --- |
| `unit_leader_weight` | `009FFD80` | `009FDF30` on `unit_class_id`, times the ship or trio multiplier |
| the member insert | `00A2D8E0` | `std::stable_sort` on descending leader weight |
| `ai_tuning_load_00a335d0` | `00A335D0` | thirty-three keys instead of six |

## Corrections

Appended to `docs/AI_COMMAND_TICK.md` and `docs/AI_TUNING_GLOBALS.md`, never rewriting them. The
`009FFD70` naming correction above is the substantive one.

## no_ghidra_function

None. Every routine read here has a Ghidra function.

## Validation

Three campaign missions, 3000 mission frames at 0.05 s. Before is this tree at `41385525a`, whose
own after-runs are reused unchanged. After is this change.

| Mission | Run | Leader | `collect_dist` | Tick `moveto` | Promotions | Gunnery hits | Damage |
| --- | --- | --- | --- | --- | --- | --- | --- |
| IJN01 | before | first by unit index | 0.0 | 0 | 1 | 4 | 500.0 |
| IJN01 | after | `Dauntless1`, weight 8 | 3000.0 | 0 | 1 | 4 | 500.0 |
| USN01 | before | first by unit index | 0.0 | 0 | 1 | 25 | 229.5 |
| USN01 | after | `ScoutDauntless`, weight 8 | 3000.0 | 0 | 1 | 23 | 220.0 |
| USN02 | before | first by unit index | 0.0 | 49 | 0 | 161 | 19061.0 |
| USN02 | after | `DeRuyter`, weight 1200 | 3000.0 | 5 | 1 | 161 | 19061.0 |

Each mission's groups now lead with:

| Mission | Group | Leader | Weight | Why |
| --- | --- | --- | --- | --- |
| IJN01 | team 0, 34 members | `Dauntless1` | 8 | `DiveBomber` 8 times the 1.0 default multiplier |
| IJN01 | team 0, 45 members | `Enterprise` | 2500 | `MotherShip` 25 times the ship multiplier 100 |
| IJN01 | team 1, 29 members | `JudySpawn1` | 8 | `DiveBomber` |
| IJN01 | team 1, 2 members | `No18` | 400 | `Submarine` 4 times 100 |
| USN01 | team 1, 20 members | `Nell1` | 2 | `LevelBomber` |
| USN01 | team 1, 7 members | `Katori` | 1200 | `Cruiser` 12 times 100 |
| USN01 | team 0, 17 members | `ScoutDauntless` | 8 | `DiveBomber` |
| USN01 | team 0, 7 members | `Enterprise` | 2500 | `MotherShip` |
| USN02 | team 0, 14 members | `DeRuyter` | 1200 | `Cruiser` |
| USN02 | team 1, 18 members | `Haguro` | 1200 | `Cruiser` |

Attribution.

- **The ordering is the visible change.** Before, every group led with whichever member had the
  lowest unit index, and the reported weights were all zero because the class-weight fields were
  unloaded. After, the leaders are the heaviest units and the weights are the shipped numbers:
  a carrier at 2500, a cruiser at 1200, a submarine at 400, a dive bomber at 8.
- **`CloseAttack_CollectDist` now reads 3000 instead of 0.0**, which changes USN02. Its group
  closes to 2995.3 and then promotes, where before a zero threshold meant it never promoted and
  kept ordering. Tick `moveto` falls from 49 to 5 and one promotion appears. That is the native
  behaviour: 2995.3 is inside 3000.
- **IJN01 and USN01 still issue nothing, and the reason is now a native one.** Their attack groups
  lead with a dive bomber, and `009FE080` admits only a ship base or a plane squadron, not an
  individual plane, so the closing arm's first test fails. The distances at promotion moved from
  4756.1 to 5894.4 and from 75.7 to 1875.2, because the leader changed.
- **Gunnery.** IJN01 and USN02 are unchanged. USN01 moves from 25 hits and 229.5 damage to 23 and
  220.0, which follows from its groups leading with different units and one fewer formation
  request. No gunnery code changed.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ai_tuning_remaining_keys` | `00A335D0`-`00A3692F` | the rest of the loader's key list, so no future packet reads a zero |
| `ai_command_engage_pass` | `00A13B60`-`00A14D9F` | still the largest unread body, and now the only thing between a promoted CLOSEATTACK and its members |
| `ai_group_total_leader_weight` | `00A2C4C0`, `00A2E720` | the two other consumers of `009FFD70`, which the compose pass sums |
