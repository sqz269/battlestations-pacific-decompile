# 0095EB40, the firepower rating the ship AI asks a unit for

Addresses: 0095EB40, 0095F080, 009E6240, 00605070, 00415510, 00415690, 00419010, 006EB060, 00727D70, 0085B7D0, 0095CF80, 008386F0, 008383D0, 009F1BC0, 009E5DA0, 009E7FC0, 009E85B0, 00831840

Packet `cc_ai_bearing_rating`, worker `agent/cc-ai-bearing-rating`. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; Ghidra was **read-only** for this
packet. Every descriptive name below is a hypothesis, not a recovered symbol.
`reports/ship_ai_bearing_rating.json` carries the machine-checkable rows;
`include/bsp/ship_ai_bearing_rating.hpp` and `src/ship_ai_bearing_rating.cpp` carry the projection.

Vocabulary follows `docs/SHIP_AI_APPROACH_UPDATE.md`: `nested` is the ring object, `slot i` the
`4Ch`-byte ring record, `unit` the entity at `[brain+0AA8h]`.

## What it is

`docs/SHIP_AI_RING_SCAN.md` called `0095EB40` "the ship class's rating of a bearing" and left the
body unread, calling it "the largest single gap left in the ring ranking". Read whole, it is
neither a class routine nor an arc score. It is an **expected-damage estimate over a time window**:

- `this` is the **unit**, not the vehicle class descriptor. `009E5DB9` loads `[slot+0h]` into ECX
  and `009E635B` loads `[state+14h]`; both are entities, and the body reads the gunnery category
  index at `unit+394h`/`+430h`/`+494h` that `00956C20` builds (`docs/GUNNERY_TABLES.md`).
- The value is a sum over the unit's gun mounts of damage the mount is expected to land on a
  target described by the caller's block, inside `block[6]` seconds. Direct damage past the
  target's armour, plus flooding damage, plus fire damage.
- The **bearing** matters only through the optional per-mount test `0085B7D0`, which the ring path
  turns on (`009E8171` sets byte `+40h`) and the range-profile path turns off (`009F2ECB`). So the
  ring scan really does get a bearing rating, but by asking each mount whether it can train there,
  not by scoring an arc.

The fields that look like angle limits are not. `[projectile+ACh]`, `+B0h`, `+B4h` and `+B8h` are
`DamageMin`, `DamageMax`, `Blast.BlastDamageMin` and `Blast.BlastDamageMax`
(`docs/WEAPON_CLASS_DESCRIPTOR.md`, lines 109-120); `+BCh`, `+C0h` and `+C4h` are `WaterDamage`,
`FireDamage` and `FireChance`.

## `0095EB40` whole

`float __thiscall(Unit* this, block* b)`, `RET 4` at `0095EB99`, `0095F065` and `0095F077`, body
`0095EB40-0095F079`, result in `ST0`. **complete**.

```
b[10..13] = 0                                                   0095EB4E..0095EB5D
if b[0] >= [unit+494h] return 0                                 0095EB6E FCOMIP, 0095EB90 JC
b[5] = wrap(b[5])                                               0095EB9E LEA ECX,[EDI+14h], 0095EBA1
for c in 0..11:                                                 0095EBB0, 0095EF83 CMP ESI,0Ch
  if [unit+394h + c*0Ch] == 0: continue                         0095EBB3
  if b[0] > [unit+430h + c*4]: continue                         0095EBD1 JA
  if not gate(c): continue                                      0095EBD7..0095EC1E
  for node in list [unit+398h + c*0Ch]:                         0095EC24, next at 0095EF6D
    g = [node+8h]                                               0095EC3A
    if not g->vtable[5Ch](22h): continue                        0095EC42, 0095EC46
    if not 00729F10(g): continue                                0095EC52
    if b.byte41:  n = 00727D70(g, c==7 ? 0 : b[7])              0095EC5F, 0095EC6A, 0095EC84
    else:         if [g+3B8h]: continue;  n = [g+448h]          0095EC8B, 0095EC98
    if n == 0: continue                                         0095ECA4
    f = [[g+3F4h]+80h]                                          0095ECAA
    a = [[g+354h]+74h]                                          0095ECB7, 0095ECBD
    if f == 6 and not b.byte3D and b.byte3C: 0095CF80(a, a+48h) 0095ECB0..0095ECD4
    p = [a+34h]                                                 0095ECDB
    if b[0] > [p+60h]: continue                                 0095ECED JA
    t   = (p.subtype == 0Ah) ? b[4] : b[3]                      0095ECF3
    lo  = max(p.DamageMin, p.BlastDamageMin)                    0095ED05..0095ED41
    hi  = max(p.DamageMax, p.BlastDamageMax)                    0095ED3B..0095ED71
    k   = interp(lo -> 1.0, hi -> 0.0, t)                       0095ED9D
    if not k > 0: continue                                      0095EDB0 JNC
    h   = 006EB060(p, b[0], b[1])                               0095EDC9
    if not h > 0: continue                                      0095EDDC JNC
    if b.byte40 and not 0085B7D0(g, p, b[5], b[0]): continue    0095EDE2, 0095EDFA
    s   = ([a+2Ch] > 0) ? b[6] / [a+2Ch] : 1.0                  0095EE0F, 0095EE1A
    P   = s * h * n * k                                         0095EE29..0095EE39 (FIMUL on n)
    e   = (lo > t) ? (lo+hi)*0.5 - t : (hi-t)*0.5               0095EE45 FCOMI, 0095EE4D/0095EE5D
    e   = clamp(e, 0.0, b[2])                                   0095EE63..0095EE79
    base = P * e                                                0095EE92, kept as a double
    w   = min(b[6], p.WaterDamage * P) * settings+3B0h          0095EE7E..0095EEC2
    fi  = min(b[6], base*p.FireChance*p.FireDamage / b[8])
          * settings+3ACh                                       0095EE9A..0095EEEA
    v   = base + w + fi                                         0095EEC8..0095EEFA
    total += v ; bucket(p.subtype) += v                         0095EF04, 0095EF0C..0095EF61
cap = b[2] * max(1.0, b[6] / 5.0)                               0095EF90..0095EFC8
b[11] = min(cap, small) ; b[10] = min(cap, artillery)           0095EFEF, 0095F00B
b[13] = min(cap, depth) ; b[12] = min(cap, torpedo)             0095F027, 0095F043
return min(cap, total)                                          0095F04A
```

`0.5` is the double at `00D7A280`, `5.0` the double at `00D7A370`, `1.0` the float at `00D7A24C`.
`00605070` wraps with `fmod` against `2*pi` (`00CE3828`) and the two fixups at `-pi` (`00CE3D18`)
and `pi` (`00CE3D28`).

### The four category gates

`0095EBD7..0095EC1E` is a fall-through chain, so a category no gate names is skipped:

| byte | categories it enables | `GunneryCategory` names |
| --- | --- | --- |
| `+3Ch` | 1 | `AAMACHINEGUN` |
| `+3Dh` | 2, 3, 4, 6 | `LIGHTARTILLERY`, `MEDIUMARTILLERY`, `HEAVYARTILLERY`, `LIGHTARTILLERYFLAK` |
| `+3Eh` | 7 | `TORPEDO` |
| `+3Fh` | 8, 9 | `DEPTHCHARGE`, `DEPTHCHARGELAUNCHER` |

Categories 0 (`PLANEGUN`), 5 (`FLAK`), `0Ah` (`BOMBPLATFORM`) and `0Bh` (`CATAPULT`) can never
contribute, whatever the gates say.

### The `22h` narrowing

`0095EC42` pushes `22h`, not the `20h` `00956C20` pushes at `00956C72`. `20h` is the base gun and
`22h` is the turning-gun family base (`docs/ENTITY_CLASS_IDS.md` lines 149-160), so the rating
rejects the fixed gun `21h`, the bomb platform `25h` and the catapult `28h` even inside an enabled
category. `include/bsp/gunnery_tables.hpp` records only the `20h` test; this is the narrowing.

### The four outputs

`[p+8h]` is the projectile sub-type (`docs/PROJECTILE_KINDS.md`), and it picks the bucket:

| sub-types | block word | `009E5DA0` copies it to |
| --- | --- | --- |
| 4, 5, 6, 7 (artillery) | 10, `+28h` | `slot+24h` (`009E5DD7`) |
| 1, 2, 3, `10h` (bullet, flak) | 11, `+2Ch` | `slot+20h` (`009E5DE2`) |
| `0Ah` (torpedo) | 12, `+30h` | `slot+1Ch` (`009E5DCC`) |
| `0Bh` (depth charge) | 13, `+34h` | `slot+28h` (`009E5DED`) |

Any other sub-type is counted in the total and dropped from every bucket (`0095EF63 FSTP ST0`).

## The seventeen-dword block

`docs/SHIP_AI_RING_SCAN.md` carried this block as raw dwords because "the producer of the other
fourteen dwords is unknown". The producer is **`009F1BC0`**, which fills all seventeen at
`009F2A04..009F2ED2`; `009E7FC0` then overwrites words 5, 6, 7 and the two bytes at
`009E813D..009E8178`. `009F3090` runs them in that order (`009F309B`, `009F30A2`).

| word | offset | meaning | producer |
| --- | --- | --- | --- |
| 0 | `+00h` | range to the target, metres | `009F2A04` from `nested+11E0h` |
| 1 | `+04h` | target hull `Length` | `009F2A54` from `[target+538h]+A0h`; `009F2AB9` default `100.0f` |
| 2 | `+08h` | per-shot damage cap and the cap seed | `009F2A44` from `[target+370h]`; `009F2AA1` default `10000.0f` |
| 3 | `+0Ch` | damage a shot must beat, every sub-type but torpedo | `009F2A2C` from `[target+538h]+4Ch`; `009F2A91` default `0.0f` |
| 4 | `+10h` | the same for sub-type `0Ah` | the virtual `[[target+538h]]+24h` at `009F2A3C`; `009F2A99` default `0.0f` |
| 5 | `+14h` | bearing, radians | `009F2A18` and `009E8153` from `nested+11DCh`; `009E5DB4` makes it slot relative |
| 6 | `+18h` | damage window, seconds | `009F2EA1` and `009E814B`, both `00CE3930` = `20.0f` |
| 7 | `+1Ch` | ready-round horizon, seconds | `009F2EB1` = `00CE38C8` = `30.0f`; `009E8161` = `00CEB4B0` = `60.0f` |
| 8 | `+20h` | fire-term divisor, the class `DamageThreshold` | `009F2A6B` from `[target+538h]+6B8h`; `009F2A7F` default `10000.0f` |
| 9 | `+24h` | written by `009F1BC0`, never read here | `009F2B95`, `009F2C6B`, `009F2CF6` |
| 10-13 | `+28h..+34h` | outputs | `0095EB4E..0095EB5D`, `0095EFEF..0095F043` |
| 14 | `+38h` | written by `009F1BC0`, never read here | `009F2A10`, `009F2E9B` |
| 15 | `+3Ch..+3Fh` | the four category gates | see the table above |
| 16 | `+40h`, `+41h` | bearing test, ready-round mode | `009E8171`/`009F2ECB`, `009E8178`/`009F2ED2` |

Words 1, 2, 3, 4 and 8 come from the **target's** class descriptor and unit, not from the rating
unit's. `Length` at `+A0h` and `DamageThreshold` at `+6B8h` are named in `docs/SHIP_CLASS_FIELDS.md`;
`+4Ch` and the virtual `+24h` are not, and their producers are a follow-up.

## `0095F080`, the 60-sample range profile

`void __thiscall(Unit* this, block /*44h by value*/, float* out60, int prefer_long_range)`,
`RET 4Ch` at `0095F165`, body `0095F080-0095F167`. **complete**. `RET 4Ch` is what fixes the
argument list at `44h` of block plus two dwords (checklist rule 7).

It wraps word 5 inline (`0095F083..0095F0E2`, the same shape as `00605070`), forces word 0 to
`50.0f` (`00D19BDC`, `0095F087`) and then calls `0095EB40` sixty times, stepping word 0 by the
`50.0` double at `00CE3938` (`0095F140`). Sample `i` therefore reports the rating at `50*(i+1)`
metres. With `prefer_long_range` the score of sample `i` is reduced by the whole number `60 - i`
(`0095F11A FISUB`), which biases the curve toward longer ranges. The store index is clamped into
`[0, 59]` (`0095F122..0095F134`), which is dead code for this loop.

`009F1BC0` is its only caller and calls it twice:

| site | `this` | output | `prefer_long_range` | block |
| --- | --- | --- | --- | --- |
| `009F2F11` | the own unit, `[owner+0AA8h]` | `nested+12C0h` | 1 (`009F2EF0`) | `nested+127Ch` |
| `009F2FB1` | the target, `[owner+0B20h]` | `nested+13B0h` | 0 (`009F2F9A`) | `nested+1238h` |

Both are rate limited by their own countdown (`nested+1220h`, re-armed to `1.5f` at `009F2F16`;
`nested+1224h`, re-armed to `2.0f` at `009F2FB6`), and the target site additionally requires the
target to answer `vtable[5Ch](5)` (`009F2F3C`). `009E6E80`, which `009F3090` calls next at
`009F30A9`, is the consumer: `009E71A5` takes `nested+13B0h` and `009E71B9` takes `nested+12C0h`.

## `009E6240`, the avoidance refresh

`void __thiscall(state, float dt, float tx, float ty, float tz, block*)`, `RET 14h` at `009E63B6`,
body `009E6240-009E63B8`. **Contract only**; the body belongs to the approach-update packets.

Its contract on `0095EB40` is: it writes the bearing it just derived into word 5
(`009E634C FSTP [EAX+14h]`, from `007B4E90` on the normalised delta), sets byte `+40h` to 1
(`009E634F`) so the mount bearing test runs, writes the distance into word 0 from `state+11Ch`
(`009E6353`), passes `ECX = [state+14h]` (`009E635B`), and stores `max(result, 1.0f)` into
`state+120h` (`009E6367..009E639B`). It then re-arms its own countdown with a uniform random draw.
It does not read the four bucket outputs.

## Host methods, in call order

Each row is a call site or a field read inside `0095EB40`.
`tools/verify_report_calls.py` checks every `address`/`native` pair in the report.

| # | method | site | callee | what the callee's body says |
| --- | --- | --- | --- | --- |
| 1 | `unit_max_weapon_range` | `0095EB62` | field `[unit+494h]` | `kUnitOffAnyWeaponMaxRange`, the max over every category (`00956E59`) |
| 2 | wrap the bearing (reimplemented) | `0095EBA1` | `00605070` | `fmod` by `2*pi` then the `+-pi` fixups, in place on `ECX` |
| 3 | `category_device_count` | `0095EBB3` | field `[unit+394h+c*0Ch]` | the list count `00956C20` maintains |
| 4 | `category_max_range` | `0095EBC4` | field `[unit+430h+c*4]` | `kUnitOffCategoryRanges` |
| 5 | `category_list_head` | `0095EC24` | field `[unit+398h+c*0Ch]` | the list head |
| 6 | `list_device` | `0095EC3A` | field `[node+8h]` | `kUnitGunneryListNodeOffPayload` |
| 7 | `device_is_turning_gun` | `0095EC46` | virtual `[[g]+5Ch](22h)` | the class test; `22h` is the turning-gun base |
| 8 | `device_is_operational` | `0095EC52` | `00729F10` | `[[g+3F0h]+720h]`, `[g+3B8h]`, `[g+5Dh]` all clear |
| 9 | `device_ready_rounds` | `0095EC84` | `00727D70` | counts the `[g+448h]` floats at `[g+414h]` that are `<= horizon` |
| 10 | `device_is_destroyed` | `0095EC8B` | field `[g+3B8h]` | the other arm of byte `+41h` |
| 11 | `device_barrel_count` | `0095EC98` | field `[g+448h]` | the same count `00727D70` bounds its loop with |
| 12 | `device_weapon_function` | `0095ECAA` | field `[[g+3F4h]+80h]` | the `Function` `007327B0` writes, `bsp::GunneryCategory` |
| 13 | `device_ammo_record` | `0095ECB7` | field `[[g+354h]+74h]` | the ammunition record |
| 14 | `ammo_select_flak_alternate` | `0095ECD4` | `0095CF80` | a refcounting copy-assign over `+4h..+40h` and the nested object at `+3Ch`, called with `src = this+48h` |
| 15 | `ammo_projectile_class` | `0095ECDB` | field `[a+34h]` | the projectile class; its fields are read at `0095ECE2`, `0095ECF3`, `0095ED05`, `0095ED15`, `0095ED3B`, `0095ED4B`, `0095EE7E`, `0095EE9A`, `0095EEA0` |
| 16 | clamped interpolation (reimplemented) | `0095ED9D` | `00419010` | returns `y0` when `x0 == x1` exactly, else clamps the lerp into `[min(y0,y1), max(y0,y1)]` |
| 17 | `weapon_hit_probability` | `0095EDC9` | `006EB060` | 0 when `range >= [p+60h]`; else the `WeaponHitAccuracy` profile for the class's `Function` at `(Length, range/[p+60h])` |
| 18 | `device_can_bear` | `0095EDFA` | `0085B7D0` | `Function 8` always true; out of range false; `Function 7` uses the traverse filter `0085AB50`; otherwise the gravity arc `00955630` plus `0085A8B0` at both spread edges |
| 19 | `ammo_cycle_period` | `0095EE07` | field `[a+2Ch]` | a period in seconds: `b[6]` is divided by it |
| 20 | clamp the excess (reimplemented) | `0095EE79` | `00415690` | `ECX` value, `EDX` low, stack high; low wins outright |
| 21 | `gameplay_tick_damage` | `0095EEAD` | `00424C40` | `settings+3B0h` `WaterTickDamage` (default 100) |
| 22 | min against the window (reimplemented) | `0095EEBD` | `00415510` | `min(*ECX, *EDX)` in `ST0`, both by reference |
| 23 | `gameplay_tick_damage`, second site | `0095EED8` | `00424C40` | `settings+3ACh` `FireTickDamage` (default 40) |
| 24 | min against the window, second site | `0095EEE5` | `00415510` | the fire argument |
| 25 | `list_next` | `0095EF6D` | field `[node+4h]` | `kUnitGunneryListNodeOffNext` |

`006EB060` maps the projectile sub-type to a weapon `Function` and `008386F0` maps that to the
`58h`-byte sub-object at `settings+240h` (`Artillery`), `+298h` (`AA`), `+2F0h` (`Torpedo`) or
`+348h` (`DepthCharge`) (`docs/GAMEPLAY_SETTINGS_TAIL.md` lines 165-168). This answers the
`weapon_hit_accuracy_consumer` follow-up in that doc: the reader is `0095EB40`.

## The class fields `+510h` and `+514h`

They are not armament fields. `00831840` reads them from the vehicle class Lua table under the keys
**`KamikazeDamage`** (string `00D09CF4`, pushed at `008319F6`, stored at `00831A22`) and
**`KamikazeBlastDamage`** (string `00D09CE0`, pushed at `00831A3B`, stored at `00831A67`). The
neighbouring `KamikazeBlastRange` (`00D09CCC`) lands at `+518h` (`00831AAC`). All three use the
defaulted float getter `00B66330` with an `FLDZ` default, so an absent key leaves `0.0f`.
No other function in the export tree writes either field.

`009E85B0` is already named `BSP_ShipAi_AttackMoveEngageGate` in the ledger; the name survives, but
what it gates is a ramming attack, not a gun engagement. Its first conjunct:

```
009E85B6  MOV ECX,[EDI+4]                    the AI context
009E85B9  MOV EAX,[ECX+0AA8h]                the unit; zero returns 0 at 009E85C1
009E85C7  MOV EAX,[EAX+538h]                 the vehicle class descriptor
009E85CD  MOVSS XMM1,[EAX+510h]
009E85D8  COMISS XMM1,XMM0 (XORPS zero)
009E85DB  JA  009E85EE                       +510h > 0.0f: pass
009E85DD  MOVSS XMM1,[EAX+514h]
009E85E5  COMISS XMM1,XMM0
009E85E8  JBE 009E86AF                       +514h <= 0.0f: bail, AL = 0
```

The conjunct is true iff `desc+510h > 0.0f` **or** `desc+514h > 0.0f`; both fields NaN is false.
`00936DC0` performs the identical pair test on `[[+1Ch]+538h]`.

**Installed values for `usn_2_java`.** The scene
`universe/scenes/missions/usn/usn_2_java.scn` spawns Alden and John1..John3 as `Clemson`, the RN
destroyers as `PACK3_Icarus`, and the enemy destroyers as `Shiratsuyu`, `Kagero` and `Fubuki`. In
`scripts/datatables/autoload/vehicleclasses.lua` none of those class tables contains
`KamikazeDamage` or `KamikazeBlastDamage`; in the whole file the pair appears only for
`VehicleClass[4]` Kaiten (lines 2106-2108) and `VehicleClass[43]` Shinyo (lines 22981-22983).

So every `usn_2_java` destroyer has `desc+510h == desc+514h == 0.0f` and `009E85B0`'s first
conjunct is **false by design**. The gate is a suicide-craft test, and the sub-state behind it is
a ramming attack, not an engagement. `docs/GAME_EXECUTABLE.md` milestone 2p follow-up 7 asked how
the executable could select the engage sub-state "when the real values say so": for a destroyer the
real values never say so, and a run that wants the sub-state must load a Kaiten or a Shinyo class.

## Coverage

| routine | body | coverage |
| --- | --- | --- |
| `0095EB40` | `0095EB40-0095F079` | complete |
| `0095F080` | `0095F080-0095F167` | complete |
| `009E6240` | `009E6240-009E63B8` | not reconstructed: contract on `0095EB40` only |
| `00605070` | `00605070-006050BD` | complete, reimplemented as a pure rule |
| `00419010` | `00419010-004190CE` | complete, reimplemented as a pure rule |
| `00415510` | `00415510-0041554D` | complete, reimplemented as a pure rule |
| `00415690` | `00415690-004156EF` | complete, reimplemented as a pure rule |
| `006EB060` | `006EB060-006EB151` | contract read from the body; host method, not reconstructed |
| `0085B7D0` | `0085B7D0-0085B97B` | contract read from the body; host method, not reconstructed |
| `00727D70` | `00727D70-00727E0D` | contract read from the body; host method, not reconstructed |
| `0095CF80` | `0095CF80-0095D11B` | contract read from the body; host method, not reconstructed |
| `00729F10` | `00729F10-00729F36` | already named; contract re-read, host method |
| `008386F0` | `008386F0-008387A5` | read for the sub-object mapping only |
| `008383D0` | `008383D0-00838526` | not read past the two-row, nine-bucket shape |
| `009F1BC0` | `009F1BC0-009F3083` | read only for the block producer and the two `0095F080` sites |
| `009E85B0` | `009E85B0-009E86B5` | read only for the first conjunct |
| `00831840` | large | read only for the `+510h`/`+514h`/`+518h` stores |

## Corrections

| what | was | is | evidence |
| --- | --- | --- | --- |
| what `0095EB40` rates | "the ship class's rating of a bearing", body unread (`docs/SHIP_AI_RING_SCAN.md`) | an expected-damage estimate over `b[6]` seconds; `this` is the unit; the bearing enters only through `0085B7D0` | the damage product `0095EE29..0095EEFA`; `+ACh..+B8h` are damage fields, not angles |
| the `vtable[5Ch]` argument | `is_gun = vtable[5Ch](20h)` (`include/bsp/gunnery_tables.hpp`) | the rating narrows it to `22h`, the turning-gun base | `0095EC42 PUSH 22h` against `00956C72 PUSH 20h`; `docs/ENTITY_CLASS_IDS.md` 149-160 |
| the seventeen dwords | "the producer of the other fourteen dwords is unknown" | `009F1BC0` writes all seventeen; `009E7FC0` overwrites words 5, 6, 7 and two bytes | `009F2A04..009F2ED2`, `009E813D..009E8178`, ordered by `009F309B`/`009F30A2` |
| word 7 on the ring path | "`nested+1298h = 60.0f` (`009E8161`)" | correct value, but sourced from the float at `00CEB4B0`; the profile path writes `30.0f` from `00CE38C8` | `009E8159`, `009F2EB1` |
| the decompiler output | `FUN_00605070()`, `BSP_Math_MinFloatByRef()`, `BSP_Math_ClampInPlace(pfVar4+2)`, no `b[8]` | `00605070` takes `ECX = &b[5]`; `00415510` and `00415690` take by-reference arguments; the fire term divides by `b[8]` | `0095EB9E`, `0095EEB2`/`0095EEB5`, `0095EE63`/`0095EE6B`/`0095EE6F`, `0095EEA6` |
| `weapon_hit_accuracy_consumer` | open follow-up in `docs/GAMEPLAY_SETTINGS_TAIL.md` | `0095EB40` is the reader, through `006EB060` and `008386F0` | `0095EDC9`; `00838713`/`0083873D`/`00838762`/`0083878C` |
| `+510h`/`+514h` | "the object at `unit+538h`. Its `+510h`/`+514h` pair is ... why the engage sub-state is never selected" (`docs/GAME_EXECUTABLE.md` 2p follow-up 7) | the pair is `KamikazeDamage`/`KamikazeBlastDamage`; it is zero for every destroyer class by design, so the sub-state is a ramming attack the class list never enables | `008319F6`/`00831A22`, `00831A3B`/`00831A67`, `vehicleclasses.lua` |

## no_ghidra_function

none. Every address named, documented or reconstructed by this packet is the start of a Ghidra
function; `python tools/bsp.py ghidra proto <addr> --brief` returns a body range for each.

## Follow-up packets

- `ship_ai_firepower_query_target_fields` - the producer of the class descriptor field `+4Ch`
  (block word 3) and of the virtual `[[desc]]+24h` (block word 4). `00831840` writes neither, and
  both are consumed in damage units at `0095ED9D`. `UnderwaterArmour` at `+6B4h` is the obvious
  candidate for word 4 and is **not** established here.
- `gun_ammo_record_layout` - whether `device+354h` and `device+3F4h` hold the same class descriptor
  (`00731020` and `00956C20` reach the same shape through `+3F4h`, `0095EB40` through `+354h`), and
  the producer of the ammunition record fields `+2Ch`, `+34h` and the nested alternate at `+48h`.
  `0095CF80` copy-assigns that nested record over the shared record, which looks destructive to a
  descriptor other mounts also point at; nothing here establishes that it is not.
- `ship_ai_standoff_range_009e6e80` - the consumer of both 60-sample curves.
- `weapon_hit_accuracy_profile_008383d0` - the nine range buckets, the `0.1` floor at `00D7A3A0`
  and the two nine-entry rows of the `58h`-byte sub-objects at `settings+240h`/`+298h`/`+2F0h`/`+348h`.

## What this packet did not do

- No Ghidra mutation: the project stayed read-only, so the three ledger names below are repository
  records only until an integrator applies them.
- No run log. `bsp_game.exe` does not reach `0095EB40`; the whole ship-AI approach update is native.
  Checklist rule 6 does not apply.
- No new test case. The pure rules are arithmetic transcriptions and `tests/math_tests.cpp` already
  covers `00419010`-shaped interpolation; a second copy would mirror the implementation.
