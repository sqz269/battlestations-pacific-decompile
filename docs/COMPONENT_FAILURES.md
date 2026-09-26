# Component failures from hull hits: engine and steering jams, magazine explosions, fuel fires

Addresses: 0093BED0 008782A0 0093AA00 0093A9E0 0093A5D0 0093BC30 00827B90 008198A0 00818110
0081B630 0093C520 0087CA80 0083E2E3 0083E32C 0083E379 0083E547 0083E594 0083E956 00827450

Packet `cc9_component_failures`, Ghidra read-only. Every descriptive name is a hypothesis, not a
recovered symbol. `docs/UNIT_FIRE_AND_REPAIR.md` ("How a failure starts", step 3),
`docs/SHIP_HIT_RECORD.md` R8, `docs/NATIVE_DAMAGEABLE_CLASS_LUA_ORCH4.md` (Damage.Sections),
`docs/GAMEPLAY_LOOSE_ENDS_2.md` (slots 220h/224h) and `docs/SHIP_FIRE_FLOODING.md` are cited, not
restated. The sequences `roll_component_failure_0093bed0` and `repair_failures_0093c520` in
`src/unit_fire_flooding.cpp` are reused as they stand; the gunnery host supplies their call sites.

## 1. The roll, 0093BED0

`00827450` calls it with the hit record and the hull damage after R5 when that damage is above zero.
With `hit+34h != -1` and session mode 0 or 1:

1. `008782A0(hit+30h, hit+34h)` walks the unit's section pointers at `unit+344h` for the first row
   with `+4h == kind` and `+8h == index` (`008782F1..0087830F`); none, no roll.
2. `0093BF2A..0093BF7E`: `p = row+28h * damage / row+2Ch`, or `settings+3DCh * damage /
   settings+3E0h` when either row value is negative.
3. `0093BF98..0093BFB0`: a draw from `00BD2F10(0, 1)`; a draw **above** `p` ends the roll.
4. `0093AA00(hit+30h)` finds the first `settings+3E8h` row (stride 14h) whose `+0h` equals the
   kind; none, no failure.
5. `0093A9E0` / `0093A5D0` search the task's failures (`task+18h`) by the row's name; an active
   failure of that name ends the roll.
6. Message `0093A2C0(kind, index, 1)`, `0093BC30` pushes `{kind, name, row+0Ch}`, `00982C50`,
   `00913D80`, then `unit->vtable[21Ch](name, hit)` at `0093C0ED`, and `task+44h = 0`.

The section rows are `VehicleClass.Damage.Sections` (`0087CA80`): `+4h = 007149D0(MshCategory)`
with **no** 7-to-9 remap, `+8h = Index`, `+28h = FailureChance / 100.0` (default -100 / 100 =
-1), `+2Ch = FailureDamageThreshold` (default -1). No class in this installation authors either
failure field, so every roll takes the settings pair. The settings rows come from
`ShipGlobals.Failures`: `SectionName` goes through the same `00E08138` search at `0083E956` and
the index is stored at `[EDI]` (`0083E97B`), again with no remap. A geometry element of kind
`steering` is filed as `body` (`007273B1`), so a hit never produces kind 7 and **SteeringJam
cannot start from a hit**.

This installation's `ShipGlobals` gives FailureChance 100 (stored 1.0, `0083E547`),
FailureDamageThreshold 100: **p = damage / 100**. The rows:

| SectionName | kind | FailureName | FailureDuration |
| --- | --- | --- | --- |
| steering | 7 | SteeringJam | 20 |
| engineroom | 5 | EngineJam | 20 |
| runway | 11 | RunwayFailure | 20 |
| hangar | 12 | HangarFailure | 20 |
| magazine | 8 | Explosion | 240 (a cooldown) |
| fueltank | 6 | Fire | 15 (a cooldown) |

## 2. The effect, 00827B90 (ship vtable slot 21Ch)

Slot `21Ch` of the ship vtables holds `00827B90` (`00CFC5EC - 00CFC3D0 = 21Ch`).

* `"Explosion"` (`__stricmp`): message `6Ah` with `+24h/+28h` the hit's kind and index, `+2Ch =
  10.0` (`00CE38B8`) and `+30h = unit+36Ch * settings+3C4h`. `008198A0` builds a 38h-byte object
  (`00818110`) with `+2Ch = msg+30h`; its step `0081B630` spawns the effect, calls
  `unit->vtable[1ACh](+2Ch)` at `0081B729` and retires itself through `vtable[10h]`
  (`0071C4A0` sets `+10h = 1`). ExplosionDamagePercentage 35 is stored as 0.35 (`0083E379`).
* `"SteeringJam"`: message `6Bh`, `unit+9E4h = 1`. `"EngineJam"`: its own message, `unit+9E5h = 1`
  and the engine effects stop (`008199C7`).
* `"Fire"` (`00CE7310`): message `9Eh`, selector 0, add flag 1, FireFailureDamageDuration seconds
  (`settings+3C0h`, 10 here, read raw at `0083E32C`).
* Any other name, for example RunwayFailure or HangarFailure, returns 1 with no effect.

## 3. Retirement, 0093C520

At priority 0 with `+46h` set, every failure loses `dt` seconds per tick; at zero or below it is
swap-erased after `vtable[19Ch]` and a clear message (`6Dh` / `6Eh` clear the two jam flags,
`008137B0`).

## 4. The settings loader

`0083B5E0..00842951` has exactly five `FDIV qword [00D7A220]` sites: `0083E243`, `0083E295`,
`0083E2E3`, `0083E379` and `0083E547`. FireFailureChance, ExplosionDamagePercentage and
FailureChance now go through `store_repair_tick_percentage_0083e243` in
`src/gameplay_settings.cpp` like the two repair percentages. Nothing in the process runs that
loader yet, so no run moves.

## 5. Switch

`kComponentFailureBound` in `src/game_hosts_gunnery.cpp`. The jam flags are published through
`GameGunneryHost::unit_failure_active(unit, "EngineJam" / "SteeringJam")`; the ship motion in
`src/game_hosts_units.cpp` is not touched.

## 6. Predictions (written before any run)

Same-tree pair, `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`, USN04 4700/4500 and USN02 1400/1200.

1. USN04: both of its direct element hits are `fizika`, and no Damage.Sections row has that
   kind, so `rolls` is 0 to 2 (a hit must do hull damage) and `resolved`, `started` are 0. Death table identical.
2. USN02: `rolls` is 19 or fewer (the element hits whose hull damage is above zero). `resolved`
   counts only the non-`fizika` hits whose element index matches an authored `Index` (0 in every
   row read), so between 0 and 12. With artillery hull damage of tens to a few hundred points, p
   is 0.3 to 1, so most resolved rolls start a failure: 0 to 5 in total, at most one per name
   and ship because of the active-name test.
3. A magazine hit on USN02 costs the ship 35% of its maximum at once. A destroyer already under
   flood damage then sinks earlier, so any `Explosion` moves one death earlier or adds one. A
   `Fire` adds 400 points over 10 s through the fire timer. EngineJam changes nothing in this
   host.
4. The USN02 failure at 44.6 s is not expected to move, because no player-ship death is predicted.

## 7. Results

Two builds of the same tree (local `cf_off`, `cf_on`), window line and module directory checked,
logs deleted first. ShipGlobals read as FailureChance 1.000, FailureDamageThreshold 100.0,
ExplosionDamagePercentage 0.350, FireFailureDamageDuration 10.0, 6 Failures rows.

| run | rolls | resolved | started | effect | death table | USN02 end |
| --- | --- | --- | --- | --- | --- | --- |
| USN04 OFF / ON | 0 / 0 | 0 / 0 | 0 / 0 | none | identical (31) | - |
| USN02 OFF / ON | 0 / 19 | 0 / 12 | 0 / 2 | two Explosions | identical (5) | failed at 44.60 s both |

The two USN02 failures:

* Exeter at 43.45 s, from a direct torpedo hit on its magazine. The same impact's blast
  (bullet 67, 6090 taken) sank it before the next pass, so its scheduled explosion found a dead
  unit and did nothing.
* Kortenaer at 54.00 s, at 1892 of 2500: the explosion took 875 (35%), leaving it afloat.

1. Held: USN04 rolls 0 and nothing else moves.
2. Held: 19 rolls, 12 resolved (the top of the band), 2 started.
3. **Did not hold.** No death moved or was added: Exeter was already sinking from the same hit,
   and Kortenaer survived the 35%. The 10 other resolved rolls either drew above p or hit a kind
   with no Failures row (`body`), so no EngineJam or Fire started.
4. Held: the USN02 failure stays at 44.60 s.

`kComponentFailureBound` is ON. No reference count moves; Kortenaer ends the USN02 run 875 points
lower.
