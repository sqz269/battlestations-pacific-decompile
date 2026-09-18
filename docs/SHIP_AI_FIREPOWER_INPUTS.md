# The inputs the ship AI's standoff scan chooses on

Addresses: 00956C20, 0095EB40, 0095F080, 006EB060, 008386F0, 009F1BC0, 009E6E80, 009E7284,
00CE3D64, 00CE3D08, 00836EF0

Packet `cc8_ship_ai_firepower_inputs`, worker `agent/cc8-ship-approach-curves`. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; Ghidra was **read-only**. Every
descriptive name below is a hypothesis, not a recovered symbol.
`reports/ship_ai_firepower_inputs.json` carries the rows (4 call rows, 0 failures under
`tools/verify_report_calls.py`). It continues `docs/SHIP_AI_APPROACH_CURVES.md`, whose first two
follow-ups it closes.

## What was blocking the scan

`docs/SHIP_AI_APPROACH_CURVES.md` left the 119-step standoff scan running on two empty curves. Two
inputs were missing, and both turned out to be already produced somewhere in the process.

### 1. `unit+494h`, the gate the rating returns at

`0095EB40` returns zero immediately when the query range is at or past `[unit+494h]`
(`0095EB6E FCOMIP`, `0095EB90 JC`). `00956C20` writes that field at `00956E59` as the maximum of
`00731020`'s range over every category's live guns, seeded to `10.0f` at `00956D51`
(`docs/GUNNERY_TABLES.md`, "The engagement ranges"). Beside it, `unit+430h + cat*4` is the same
maximum within one category, floored at `10.0f` by `00956D63`, and `unit+490h` is the maximum
restricted to Functions 2, 3, 4 and 6.

The gunnery host **already ran that rebuild** for every unit
(`bsp::rebuild_weapon_category_index_00956c20` in `src/game_hosts_gunnery.cpp`) and was throwing
two of its three outputs away: `store_artillery_max_range` and `store_any_weapon_max_range` were
empty overrides. Capturing them, and exposing the per-category gun lists the same rebuild built, is
the whole of the fix on that side.

### 2. `nested+1284h`, which is not a scan constant

The previous packet called it "the numerator of the scan's middle factor" with no producer. A
displacement scan settles what it is. `scan-bytes "84 12 00 00"` finds the pattern in thirty-odd
places across `.text` (the positive control), of which three matter: `009E7286`, the reader inside
`009E6E80`, and two writers, `009F2A4E` and `009F2AAD`, both inside `009F1BC0`.

Reading them shows `nested+1284h` is **word 2 of the own unit's firepower query block at
`nested+127Ch`**, which is the per-shot damage cap:

| offset | word | writer, with a target | writer, without |
| --- | --- | --- | --- |
| `+1280h` | 1, target length | `009F2A54` from `[[target+538h]+0A0h]` | `009F2AC1`, the `100.0f` at `00CE3D08` |
| `+1284h` | 2, damage cap | `009F2A44` from `[target+370h]` | `009F2AA9`, the `10000.0f` at `00CE3D64` |
| `+1288h` | 3, armour | `009F2A2C` from `[[target+538h]+4Ch]` | `009F2A91`, zero |
| `+128Ch` | 4, torpedo armour | `009F2A3C`, that object's `vtable[24h]()` | `009F2A99`, zero |

`EBP` is `nested`: `009F2AD1 MOV ECX,[EBP]` then `009F2AD4 MOV ECX,[ECX+0AA8h]` is the brain's own
unit. `docs/SHIP_AI_BEARING_RATING.md` had already named this producer in the
`ShipAiFirepowerQuery::damage_cap` field comment; this packet confirmed it from the listing
independently.

That matters twice over, because the damage cap is also what
`ship_ai_firepower_output_cap(damage_cap, window)` caps every profile sample with. A zero there
caps the whole curve at zero, whatever else is bound.

## `006EB060`, the hit probability

`float __thiscall(ProjectileClass* this, float range, float target_length)`. **complete**. It is
the factor every mount's contribution is multiplied by, so nothing can be rated without it.

```
if [this+60h] <= range: return 0
t = range / [this+60h]
switch [this+8h]:                 the projectile sub-type
  4, 5, 6, 7   -> 008386F0(3, target_length, t)
  0Ah          -> 008386F0(7, target_length, t)
  0Bh, 13h     -> 008386F0(8, target_length, t)
  1, 2, 3, 10h -> 008386F0(1, target_length, t)
  anything else-> return 1.0
```

The `1.0` fall-through is a trap for a host: a caller that cannot classify its ammunition gets a
certain hit rather than a plausible one. The binding therefore takes the sub-type from the image's
own `Type` name chain, `006EA910`, through `bsp::projectile_class_for_lua_type`.

`008386F0` maps the function code to one of the four `58h` `WeaponHitAccuracy` profiles at
`settings+240h`, `+298h`, `+2F0h` and `+348h`. Nothing in this process loads
`ShipGlobals["WeaponHitAccuracy"]`, so the binding uses the image's own defaults from `00836EF0`,
whose twenty accuracy slots are all `0.5f`. **Partial**: because every slot of that default is the
same value, the answer does not depend on `008386F0`'s own interpolation, which is how the target
length picks between the small-target and large-target curves. That interpolation was not read and
will matter the moment the authored table is loaded.

## Host methods

`src/game_hosts_ship_ai.cpp`, `FirepowerBinding`, wired to the gunnery host through
`GameGunneryHost::set_ship_ai`, which now also calls `GameShipAiHost::bind_gunnery`. Both hosts
index units by the same `GameUnitsHost` index.

| method | site | source |
| --- | --- | --- |
| `unit_max_weapon_range` | `0095EB62` | `GameGunneryUnitRow::any_weapon_max_range`, captured from `00956C20`'s own store |
| `category_device_count` | `0095EBB3` | the per-category list sizes that rebuild built |
| `category_max_range` | `0095EBC4` | `GameGunneryUnitRow::category_ranges` |
| `category_list_head` / `list_next` / `list_device` | `0095EC24`, `0095EF6D`, `0095EC3A` | the gun indices in that category, walked by position |
| `device_barrel_count` | `0095EC98` | `GameGunRow::barrel_num` |
| `device_weapon_function` | `0095ECAA` | `GameGunRow::category` |
| `ammo_projectile_class` | `0095ECDB` | the authored `Bullets` row, sub-type through `006EA910`'s chain |
| `ammo_cycle_period` | `0095EE07` | `GameGunRow::reload_time` |
| `weapon_hit_probability` | `0095EDC9` | `006EB060`'s rule with the `00836EF0` default profile |
| `gameplay_tick_damage` | `0095EEAD`, `0095EED8` | the authored defaults, `WaterTickDamage` 100 and `FireTickDamage` 40 |
| `nested_scan_scale_1284` | `009E7284` | the `10000.0f` of `009F2AA9` |

### Labelled substitutions

Each is recorded so the run log names it, and each makes the rating optimistic rather than wrong in
form:

- `device_is_turning_gun` (`0095EC46`) and `device_is_operational` (`0095EC52`): the class test and
  `00729F10`'s three flags have no producer here, so every mount passes.
- `device_ready_rounds` (`0095EC84`): `use_ready_rounds` is set on this path (`009F2ED2`), but the
  gunnery host's reload-timer list has no producer, so every barrel counts as ready.
- `ammo_cycle_period`: the gun's reload time stands in for `[ammo+2Ch]`.
- the four allow bytes (`009F2AE7`, `009F2B20`, `009F2BC2`, `009F2B6D`, from
  `[0080E160(unit)+220h..+223h]`): with all four clear `0095EBD7` skips every category and the
  rating is always zero, so every category is allowed here.
- the four target fields of the query block: the no-target constants of `009F2A91..009F2AC1` stand
  in on both arms.
- `blast_damage_min` (`+B4h`) is the one projectile field the gunnery host does not carry.

## Validation

Build: `scripts/build.ps1`, Win32 Release, `/W4 /WX`, clean. Tests: 2 of 2 passing.

USN02, 3000 mission frames at 0.05 s, against the previous packet's after-run:

| | before | after |
| --- | --- | --- |
| curve samples filled | 0 of 60, both curves | 60 of 60, both curves, all fourteen ships |
| `unit+494h` | 0.0 | 6136.0 |
| Haguro | 300.0 | 1450.0 |
| Jintsu | 300.0 | 1550.0 |
| the twelve destroyers | 300.0 | 200.0 |
| gunnery | `shots=734 hull=180 deaths=2 total_damage=18525.6` | identical |

**The standoff range moved; nothing downstream did.** The attribution is exact on both halves.

The range moved because `unit+494h` stopped being zero. `0095EB40` no longer returns at
`0095EB6E`, all sixty samples of both profiles are positive, and the scan's minimum is a real one
rather than a seed that survived because every step was skipped.

Nothing downstream moved because the standoff range reaches the slot weights and stops there. The
gunnery aim line is identical to the character (`angle_sets=1169733 refusals=185667 shots=734`),
the three ship AI summary lines are identical, and every per-unit ring-scan row is identical: the
winner is slot 0 in both runs with the same commanded heading of `-3.1415` and the same throttle.
The reason is named below.

The run exited 1 from `present failed hr=0x88760868`, `D3DERR_DEVICELOST`, during shutdown after
3187 presents. The mission ran all 3000 frames and printed every census, so the run is judged by
the log.

USN01 was not re-run for this packet. The previous packet recorded `hull=23 deaths=1
total_damage=220.0` with `standoff choices=0`, and nothing here changes a mission whose ships never
enter the approach sub-state.

## Corrections

Appended to `docs/SHIP_AI_APPROACH_CURVES.md`, which is this packet's own predecessor. Both of its
first two follow-ups are closed, and one of them was stated wrongly: `nested+1284h` is not a scan
constant without a producer, it is the firepower query block's damage cap and `009F1BC0` writes it.

## no_ghidra_function

none. `006EB060` and `00956C20` are both existing Ghidra functions, and the two `009F1BC0` writers
are inside that function's body.

## Corrections from packet cc8_ship_ai_approach_slot_tune

Appended, not a rewrite.

| was | is | evidence |
| --- | --- | --- |
| Follow-up `ship_ai_approach_slot_tune`: "the next blocking input ... with the tune block zero every slot scores alike, `009E76D0` keeps slot 0, the commanded heading never changes and no gunnery number can move." | Half right. The block is found and bound with its real values, and the commanded heading does move during the run, but the winning slot is still 0 for every ship and the gunnery census is unchanged. The tune block was not the last blocker; the four per-slot scorers behind `slot+18h` are. | `docs/SHIP_AI_APPROACH_SLOT_TUNE.md`, Validation, with a control build whose tune values were zeroed. `009E7FC0` writes `slot+2Ch = slot+18h / running_max * tune+0h`, and multiplying a tie by `10.0` leaves a tie. |

## Follow-up packets

- `ship_ai_approach_slot_tune`: **the next blocking input.** `009E7489` (`tune+4h`, the slot
  scorer's weight) and `009E784B` (`tune+4h` again, the selection's reject threshold) are both
  unimplemented with 8400 calls each. The standoff range reaches `009E6870`'s four floats, but with
  the tune block zero every slot scores alike, `009E76D0` keeps slot 0, the commanded heading never
  changes and no gunnery number can move. Until that block has a producer, a better standoff range
  cannot show up in a census.
- `weapon_hit_accuracy_load`: nothing loads `ShipGlobals["WeaponHitAccuracy"]`, and `008386F0`'s
  own sampling was not read.
- `ship_ai_firepower_allow_bytes`: `[0080E160(unit)+220h..+223h]`, the four category permissions.
- `ship_ai_approach_target_fields`: `[target+370h]`, `[[target+538h]+4Ch]` and
  `[[target+538h]+0A0h]`, the damage cap, armour and length of the ship being rated.
