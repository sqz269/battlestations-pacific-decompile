# The ship AI's firepower rating: binding its eight mount predicates

Addresses: 0095EB40, 0095EC46, 0095EC52, 0095EC84, 0095ED15, 0095EDC9, 0095EDFA, 0095EE07, 0095EEAD, 00729F10, 00727D70, 0085B7D0, 006EB060, 006EB0C8, 00424C40, 007313E0, 00443090, 005459B0, 007BA2E0, 0085A8B0, 0085AB50, 00955630, 00438AA0, 00438B10

Packet `cc9_ship_firepower`, 2026-09-23. The report is `reports/ship_ai_firepower.json`. All names
are hypotheses, not recovered symbols. Nothing here is ABI-compatible or game-validated.

**Status: bound, measured and landed.** The switch is `kShipFirepowerBound` in
`src/game_hosts_ship_ai.cpp`, default true. The rating itself, `0095EB40`, was already
reconstructed whole (`bsp::ship_ai_firepower_rating_0095eb40`; the formula is in
docs/SHIP_AI_BEARING_RATING.md). This packet binds the host answers it asks about each mount.

## 1. The routine and its consumers

`0095EB40` sums, over the unit's gun mounts in the categories its block enables (1-4 and 6-9), the
damage each mount is expected to land within `b[6]` seconds:

```
P    = s * h * n * k
base = P * e
w    = min(b[6], WaterDamage * P) * 100
fi   = min(b[6], base * FireChance * FireDamage / b[8]) * 40
v    = base + w + fi
```

The result is capped at `b[2] * max(1, b[6]/5)`.

| symbol | meaning |
|---|---|
| `n` | ready rounds |
| `k` | armour interpolation of `max(DamageMin, BlastDamageMin)`..`max(DamageMax, BlastDamageMax)` against `t = b[4]` for torpedoes (sub-type 0Ah), else `b[3]` |
| `h` | hit probability |
| `s` | `b[6] / ReloadTime` |
| `e` | expected penetration |

Its three consumers:
- **The ring scan** (`009E5DA0`) asks it per bearing slot, with byte +40h set so each mount is asked
  whether it can bear. `009E7FC0` then divides every slot by the frame maximum.
- **The range profile** `0095F080` samples it every 50 m for the own unit and the target. The
  standoff scan at `009E71A5` then minimises
  `max(1, them(x)) * (nested+1284h / us(x)) * interp(0, 2, 1, 1, us(x)/best)`
  (docs/SHIP_AI_APPROACH_CURVES.md).
- **The avoidance refresh** `009E6240`.

## 2. The eight predicates

| call site | native | image | host before | host now |
|---|---|---|---|---|
| 0095EC46 | `[[mount]+5Ch](22h)` | IsKindOf the turning-gun subtree: 22h, MRTGun 23h, MSTGun 24h, MDepthChargeLauncher 27h (`006FDE40`, `00730ED0`, `006FDF20`, `006FE050`). `00443090` picks the class from the device's Lua `Type`. | true, recorded | true for every category the rating counts. This installation's deviceclasses.lua maps every AAMACHINEGUN, *ARTILLERY*, FLAK, TORPEDO and DEPTHCHARGE* device to Rapid_Turning_Gun, Single_Turning_Gun or Depth_Charge_Launcher. The 12 Rapid_Fixed_Slave_Gun (MRFSGun) devices are all PLANEGUN. |
| 0095EC52 | `00729F10` | bytes `[[g+3F0h]+720h]`, `[g+3B8h]` and `[g+5Dh]` all zero (00729F16/1F/28) | true, recorded | `ship_ai_gun_is_operational_00729f10(false, false, false)`, the same three answers the gunnery host's own fire gate gives (`FireRequestBinding`) |
| 0095EC84 | `00727D70` (horizon = `b[7]`, 0 for category 7, only when byte +41h) | count of the first dword `[g+448h]` floats at `[g+414h]` that are ≤ the horizon (`FCOMI` with the horizon in ST0, `JC` skips) | `barrel_num`, recorded | `ship_ai_gun_ready_rounds_00727d70` over `fire.barrel_timers`, which 0072CF00 sets per shot and the fixed step counts down |
| 0095ED15 | `[p+B4h]` | `Blast.BlastDamageMin` | 0, recorded | the Bullets row's `blast_damage_min`, which the gunnery host now carries |
| 0095EDC9 / 006EB0C8 | `006EB060` | switch on the class's **refined** `+8h` sub-type, 1.0 for an unnamed one | the sub-type from `GameBulletClassRow::type`, which is **never filled**, so every call fell through to 1.0 | the gun row's `bullet_sub_type`, which is the refined value (006E9968). All ship ordnance now reaches `008386F0`. |
| 0095EE07 | `[a+2Ch]` | the fire record's `ReloadTime` upper value (`007313E0`); a scalar is stored to both +28h and +2Ch | gun `reload_time`, recorded | the same value, now exact: all 500 `ReloadTime` entries here are scalars |
| 0095EEAD | `00424C40`, then `+3B0h` and `+3ACh` | `WaterTickDamage`, `FireTickDamage` | 100 and 40, recorded | 100 and 40, the installed shipglobals.lua:77-78 (the `FailureDebug` override is off in scriptoptions.lua:1) |
| 0095EDFA | `0085B7D0` | the whole bear test, below | true in range, recorded | `ship_ai_gun_can_bear_0085b7d0` |

**`0085B7D0`, `BSP_Gun_CanBearAtRange`**, `__thiscall(gun, proj, float bearing, float range)`,
`RET 0Ch`.
- Function (`[[g+3F4h]+80h]`) 8 answers true.
- A `range` greater than `[proj+60h]` answers false.
- **Function 7** asks `0085AB50` (the snap `007F6190`) with a limit of `0.7853982f` (00E0B588) ×
  `0.5` (00D7A280). Anything but the FLT_MAX sentinel bears.
- **Function 9, or Function ∈ {2,3,4,6}** (`005459B0`):
  - The aim point is `(cos a, 0, sin a) × range`, with `a = π/2 − bearing`, +2π when negative
    (`007BA2E0`).
  - The muzzle is at the zero vector `00F87574`, the speed is `[proj+50h]`, and there is no mount.
    The ballistic solve `00955630` must succeed.
  - The heading becomes **−(solved heading)** (0085B8F1, `−0.0f − h`) and the elevation the
    solved one.
- **Any other Function** keeps the bearing with elevation 0.
- Then **both** `wrap(h + 0.0523599f)` (00438AA0) and `wrap(h − 0.0523599f)` (00438B10), with
  `0.0523599f` at 00D0CBA0, must pass `0085A8B0`: both angles wrapped, then `007F5FC0` on the
  platform's arcs.

## 3. Predictions (made before the runs)

- **USN01 does not move.** Its ships issue no attackmove approach, so the rating is never called.
  That held: no `ShipAiFirepower` row appears at all.
- **USN04.** The ring scan's bearing slots change wherever a mount now fails the arc test, which
  shows as heading changes. The standoff choice changes wherever the target's or the own curve
  changes shape. A uniform scale cannot move the ring scan, which normalises by the maximum, but it
  can move the standoff scan through its `max(1, them)` floor. Downstream AA and deaths are coupled
  (docs/AA_TARGETING.md).

## 4. Runs

Worktree root `J:\PROG\battlestations-pacific-decompile-cc9-difficulty`, base `95472d9b5`.
- Control `build\win32\ctl4\` (switch off).
- Treatment `build\win32\treat4\`, identical in behaviour to the committed `treat5`. treat5 differs
  only in labelling the tick-damage call concrete.
- Ablations: A = without the sub-type fix, B = without the blast minimum, C = without the arc test.

```
./tools/run_game.ps1 -Exe build\win32\<bin>\bsp_game.exe -Log local\fp_<run>_usn04.log -- --frames 4700 --press-start-frame 30 --menu-select USN04 --mission-frames 4500 --mission-frame-seconds 0.05
./tools/run_game.ps1 -Exe build\win32\<ctl4|treat5>\bsp_game.exe -Log local\fp_<ctl|trt>_usn01.log -- --frames 3200 --press-start-frame 30 --menu-select USN01 --mission-frames 3000 --mission-frame-seconds 0.05
```

**USN04, standoff distance / heading changes per attackmove ship.**

| ship | control | A (no sub-type) | B (no blast) | C (no arc) | treatment |
|---|---|---|---|---|---|
| Northampton-class01 | 1450 / 11 | 1450 / 6 | = treatment | 950 / 9 | **950 / 7** |
| Northampton-class02 | 1450 / 148 | 1450 / 118 | = treatment | 950 / 30 | **950 / 12** |
| Fletcher-class01..04 | 200 / 13, 55, 65, 70 | 200 / 12, 53, 63, 70 | = treatment | = control | 200 / 12, 53, 63, 70 |
| York-class01, 02 | 950 / 61, 9 | 950 / 57, 4 | = treatment | = control | 950 / 57, 4 |

| run | queued_hits | damage | deaths | Lexington sunk | log |
|---|---|---|---|---|---|
| control | 116 | 15970.7 | 9 | 220.36 s (York-class02) | `local\fp_ctl_usn04.log` |
| A | 94 | 14615.7 | 9 | failure at 222.06 s | `local\fp_abA_usn04.log` |
| B | 92 | 14733.9 | 9 | failure at 183.06 s | `local\fp_abB_usn04.log` |
| C | 111 | 15928.9 | 9 | 220.41 s (York-class02) | `local\fp_abC_usn04.log` |
| treatment | 92 | 14733.9 | 9 | **182.86 s (movieval\|.-2)** | `local\fp_trt_usn04.log` |
| USN01 control / treatment | 35 / 35 | 2892.0 / 2892.0 | 0 / 0 | - | `local\fp_ctl_usn01.log`, `local\fp_trt_usn01.log` |

**Term by term.**
1. **The standoff move, 1450 m to 950 m, for the two Northamptons only, comes from the sub-type
   term.** A (everything but the sub-type) keeps 1450, and C (everything but the arc) moves.
   - Classified, the hit probability is the image's default `WeaponHitAccuracy` profile (0.5 in
     every slot) instead of the 1.0 fall-through. That halves both curves.
   - The standoff objective's `max(1, them(x))` floor makes it sensitive to that scale, and the
     argmin moves.
   - This mechanism is inferred from the scan formula; the attribution is proven by the ablation.
   - Ships whose standoff was already 200 m or 950 m do not move.
2. **The fewer heading changes come from the arc test.** Mounts that cannot train toward a ring
   slot no longer add to its score.
   - C, which lacks only the arc test, reproduces the control's Fletcher and York counts exactly.
   - A, which has it, reproduces the treatment's.
   - Northampton-class02's 148 to 12 is both terms: 118 with the arc alone, 30 with the standoff
     alone.
3. **The blast minimum, ready rounds, operational, turning-gun, cycle-period and tick-damage terms
   move nothing on USN04.**
   - B equals the treatment on every row.
   - C equals the control wherever the standoff did not move.
   - Ready rounds apply only when the caller's block sets byte +41h. Which of the three callers set
     it was not traced; on this run the term changes no row.
4. **Downstream, coupled.**
   - With the Northamptons standing off 500 m closer and turning less, the AA picture over the
     carriers changes. Hits go from 116 to 92.
   - `movieval|.-2`'s bomb now finishes Lexington at 182.86 s instead of York's fire at 220.36 s,
     and the failure path runs at 183.06 s.
   - `D3A Val #1.1|.-3` survives (175 taken), and `B5N Kate #4.1|.-2` dies at 148.65 s.
   - These follow the approach change and are not separate decisions.
   - Neither term alone does it. A and C both keep Lexington afloat to about 220 s, credited to
     York-class02. Only the two together, the Northamptons at 950 m and turning less, give the
     183 s loss.

**The native table.** The host's UNIMPLEMENTED call total on USN04 falls from **18,125,296** to
**9,607,363**, a drop of 8,517,933. The seven bound firepower rows are concrete, and
`hit_probability_unclassified` no longer occurs. One row remains:
`ShipAiFirepower::hit_accuracy_profile_008386f0` (1,168,042 calls), the substituted default
profile.

## 5. Decision and open items

* **Decision: land it.** Every decision change is attributed to a term by ablation, and each term
  is the image's rule over produced state.
* **Open: the accuracy profile.** `ShipGlobals["WeaponHitAccuracy"]` is not loaded, and
  `008386F0`'s small/large interpolation is unread. The whole standoff shift rests on the
  profile's absolute level, so loading the authored table will move it again.
* **Open, for the gunnery owner:** `GameBulletClassRow::type` is never filled
  (`src/game_hosts_gunnery.cpp`, the row built near line 730). This packet routes around it through
  `GameGunRow::bullet_sub_type`. Any other reader of `type` has the same fall-through.
* **Open: the Function 7 arm** uses the snap `007F6190` with the platform's arcs. The per-platform
  bounds check in `0085AB50` itself is not modelled.
* No correction to docs/SHIP_AI_BEARING_RATING.md. One to docs/SHIP_AI_FIREPOWER_INPUTS.md is
  appended there.
