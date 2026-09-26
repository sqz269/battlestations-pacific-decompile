# Ship fire, flooding and hull repair in the gunnery host

Addresses: 00723E90 00723D60 00724510 00826F10 00826F62 0082733C 0082738E 0080FA50 0082203D
0093A470 0093A4F0 0093BCC0 0093CA20 0093C770 0093C120 0093C210 008160B0 00825F20 00826B84
0083E1B3 0083E1F5 0083E23E 0083E243 0083E4AA 00962DBC 0090E6C0

Packet `cc9_ship_fire_flooding`, Ghidra read-only. Every descriptive name is a hypothesis, not a
recovered symbol. `docs/SHIP_HIT_RECORD.md` (rules R1..R12), `docs/HIT_HULL_SEGMENT.md`,
`docs/PART_DAMAGE_REACHABILITY.md`, `docs/UNIT_FIRE_AND_REPAIR.md` and `docs/UNIT_MESSAGE_ARMS.md`
are cited, not restated. This installation is modded; "this installation" means
`I:/SteamLibrary/steamapps/common/Battlestations Pacific` as read on 2026-09-25.

## 1. Why every reference run reports `part=0 fires=0 floods=0`

The shell hull trace (`kShellHullHitTestBound`) already tests a round against the ship model's
GeomMesh triangles, but it writes `record+34h = -1` and `record+30h = 0Ah` whatever it hits. The
image's shape `00724510 -> 00723E90` copies both out of the element it hit
(`00723F62 MOV [EAX+30h],ECX` = element+4h, `00723F6C MOV [EAX+34h],EDX` = element+8h). R1
(`00826F62`) skips R2..R10 when `+34h == -1`, so the host never reached R7b (flood, every hit with a
weapon), R7c (fire, on a roll), R4 (fizika part damage) or R8 (component failure).

`00723D60` walks the elements in index order over each element's own triangle list, shortening the
far end on every hit, so the last acceptor of an equal distance wins; triangles that belong to no
element are never tested.

## 2. What a flood and a fire do

Message `9Eh` (`0080FA50`, add flag 1 from both R7b and R7c) reaches `0093A4F0` (water, selector 1)
or `0093A470` (fire, selector 0). Each **adds seconds**: water to `task+34h`, fire to `task+38h`
(`task = unit+A20h`). Each then compares `water_s * task+2Ch + fire_s * task+30h` with the health at
`unit+370h` and, when the sum is larger, tail-calls `0090E6C0(game+21A0h, unit)`.

`0093BCC0` (the task constructor, called at `0081EF47`) copies `settings+3B0h` WaterTickDamage into
`task+2Ch` and `settings+3ACh` FireTickDamage into `task+30h`, sets `task+28h = 1.0`, `+45h = +46h
= 1` and priority `+24h = 0`.

The tick `0093CA20` runs from `008160B0` (vtable slot 1ECh, called unconditionally at `00826B84`
inside `00825F20` with the ship-motion dt) when the class byte `+D0h` is set and the entity flags
allow it. `+D0h` is `VehicleClass.Repair`, and `00962DCF..00962DDA` stores **1 when the key is
nil**: every class except an explicit `Repair = false` (this installation: Kaiten) runs damage
control. Its five steps, in order:

| step | rule |
| --- | --- |
| `0093C770` hull | `heal = task+28h * dt * settings+3B4h * max_health * (priority 0 ? settings+3D4h : 1) * modifier`, then clamp to max |
| `0093C860`, `0093C520` | subobjects and failures; this host has neither, so both are no-ops |
| `0093C120` water | `before = +34h; +34h -= dt; floor 0 (and +40h = 0); damage = (before - +34h) * +2Ch / ((priority 4 ? settings+3C8h : 1) * modifier)` through `vtable[1ACh]` |
| `0093C210` fire | the same over `+38h`, `+30h`, `+3Ch`, `settings+3CCh`, priority 3 |

`0083E23E` reads BodyRepairTickPercentage and `0083E243 FDIV qword [00D7A220]` divides it by
100.0; `src/gameplay_settings.cpp` stores the raw value. WaterTickDamage (`0083E1B3`) and
FireTickDamage (`0083E1F5`) are stored raw. This installation's `ShipGlobals` gives 100, 40, 0.1
and BodyRepairMultiplier 2, so a flood second costs 100 hit points, a fire second 40, and a damaged
hull heals 0.2% of its maximum per second. `FireChance` is stored divided by 100
(`docs/WEAPON_CLASS_DESCRIPTOR.md`, `00D7A220`), and the host's bullet row keeps the raw value.

## 3. Switches

| switch | what it binds |
| --- | --- |
| `kHullElementSegmentBound` | the element trace: kind and index into `+30h`/`+34h`, the view's segment kind from `+30h`, FireChance / 100 |
| `kShipDamageControlTickBound` | `9Eh` add-seconds and the water/fire steps of `0093CA20`, per ship, after the projectile pass |
| `kShipHullRepairBound` | the hull step `0093C770` |

## 4. Predictions (written before any run)

Pairs are same-tree, `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`, USN04 4500 frames and USN02 900
frames at 0.05 s.

1. `kHullElementSegmentBound` alone: `floods` becomes roughly the number of direct mesh hits by
   artillery and torpedoes (every such bullet row authors `WaterDamage`), `fires` about a tenth of
   the artillery ones, `part > 0` only for `fizika` elements. No unit's health changes, so the death
   table and hit counts match the OFF run exactly; the new draws are on the keyed hit-effect stream.
2. Adding `kShipDamageControlTickBound`: ships take `100 * WaterDamage + 40 * FireDamage` extra
   per flood or fire over the following seconds. Ship deaths rise or come earlier on both missions;
   plane deaths are unchanged until a carrier or its escorts sink.
3. Adding `kShipHullRepairBound`: every damaged ship heals 0.2% of its maximum per second, so a
   ship that survives the first strikes regains health; some ship deaths from (2) move later or
   disappear. Planes are untouched.

## 5. Results

Four builds of the same tree (local `ff_off`, `ff_seg`, `ff_tick`, `ff_all`), window line checked,
logs deleted before each run. Every ship model read covers all of its triangles with elements
(`element_triangles == triangles` on every class line), so the element walk tests the same set as
before; ShipGlobals was loaded when the classes were flattened (100, 40, 0.00200 of max per second).

| run | USN02 floods / fires / part | water / fire damage | repaired | USN02 deaths | USN04 |
| --- | --- | --- | --- | --- | --- |
| all OFF | 0 / 0 / 0 | 0 / 0 | 0 | 4 | 31 deaths, 528 hits |
| element | 19 / 2 / 7 | 0 / 0 | 0 | 4, table identical | identical |
| + tick | 19 / 2 / 7 | 3313 / 324 | 0 | 5: Yamakaze at 57.70 s | identical |
| + repair | 19 / 2 / 7 | 3401 / 324 | 382 | 5: Yamakaze at 58.60 s | identical |

1. Held. All 19 USN02 direct mesh hits carried a real index and each flooded; 7 were `fizika`
   elements; 2 rolled a fire. Both death tables are line-for-line identical to OFF.
2. Held on USN02: one more ship death, Yamakaze, from 1503 flood and 200 fire damage.
   On USN04 the tick changed nothing. Its 4500 frames put only 2 direct mesh hits on ships, both
   from rounds with no `WaterDamage`, and no ship lost health in this host inside the window.
3. Held. The repair heals 382 points on USN02 and moves Yamakaze's death 0.9 s later. USN04 has
   nothing to heal.

All three switches are ON. USN02's reference moves from 4 to 5 deaths. USN04 does not move.

## 6. Not modelled, labelled

* `0093BED0` (R8, component failure) stays a record, so the failure list and `0093C520` stay empty.
* `0092D1F0` (R4) is counted, not applied: the host keeps no per-segment health vector, so a
  destroyed `fizika` segment has no consequence here.
* The tick runs after the gunnery projectile pass, not inside `00825F20`. The entity flag tests at
  `008160BF..008160D1` are read as "not dead".
* Blast records keep `+34h = -1` as the image's do, so a bomb or torpedo burst never floods; only
  the round's own direct impact does.
* Packet `cc9_repair_percentage_scale`: `src/gameplay_settings.cpp` now stores BodyRepairTickPercentage
  and GunRepairTickPercentage through `store_repair_tick_percentage_0083e243`, the `/ 100.0` of
  `0083E243` / `0083E295` (`FDIV qword [00D7A220]`, bytes `00 00 00 00 00 00 59 40`, then `FSTP` to
  `+3B4h` at `0083E258` and `+3B8h` at `0083E2AA`). The host takes its repair fraction through the
  same rule; the result is bit-identical to its former own division (`0x3B03126F`, 0.002 of max per
  second), so no pair was run.
