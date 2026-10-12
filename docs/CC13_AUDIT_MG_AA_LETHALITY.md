# IJN01: small-calibre AA against the A7Ms (cc13-mgaa1 audit for cc9)

Packet `cc13-mgaa1`, read-only audit for cc9's gunnery/AA lane, 2026-10-12. Ghidra was only read, and no
C++ changed. No game run was made: the two logs below already answer the question. The call-site rows are
in `reports/cc13_audit_mg_aa_lethality.json` (3 rows, 0 failed).

Status labels: the image terms are **read** (listing or live decompile). The host terms are **read** from
`src/`. The run numbers are **measured** from existing logs. Nothing here is game-validated.

## Summary

**Verdict: the host's MG hits on planes are faithful. There is no AA defect and no diff.**

- From 10-06 to now, the MG-vs-plane chain gives the same result per hit and per kill.
  - The first A7M kill matches bit for bit: `A7M_6|.-4`, t=105.40, first damage 99.90, PT2 gun 454,
    29 cat-1 hits for 280.
  - The cost of a kill is unchanged: about 19-30 cat-1 hits per A7M. That is what the data predicts (section 3).
- The AA did not get more accurate. Hits on planes per AA round fell from 6.5 % to 4.6 %.
- **What changed is exposure: the PTs and LSTs now live much longer.**
  - On 10-06, all 15 PT/LST were dead by 395.18 s. Now only 6 die, the last at 286.50 s.
  - So the AA fired 54 % more rounds (14918 -> 22931).
  - The extra A7M deaths are planes that stay in range of live PT/LST guns.
- **The brief's guess is not what the logs show.** The A7Ms do not strafe *more*: strafe aims fall 105 -> 56.
- **The cascade starts in flight, before any shot is fired.**
  1. The logs first differ at 75.30 s, in a plane's altitude (`A7M_1|.-3` alt 856.7 vs 856.3). Both logs
     show `gunnery step 1500 t=75.00 shots=0`, so no gun had fired yet.
  2. At about 134 s a B-17 gunner kills the leader `A7M_2` (135.10 s). On 10-06 the same gunner killed a
     wingman, `A7M_2|.-3` (133.90 s).
  3. The host's fused-slot defect then refuses every later order to `A7M_2`
     (`CC13_AUDIT_REATTACK_AND_SKILL` A, `kFusedSquadronSlotSelectableBound`).
  4. So the LSTs that wing was ordered against survive and keep firing.
- **No gunnery commit since 10-06 touches cat-1 AA against a plane in a way this run can show.** The switches
  in this run's path report zero refusals (section 1).

## 1. What changed since 2026-10-06

**Baseline.** The baseline is `J:\PROG\battlestations-pacific-decompile-cc9-lua46\local\l46ue_off_ijn01p1.log`
(cc9-lua46, commit `7821e75b8`, written 2026-10-06 13:01 local). This is the log `CC13_AUDIT_REATTACK_AND_SKILL`
1.2 cites for 10-06. **Now** is `J:\PROG\battlestations-pacific-decompile-cc13-strafe2\local\st2_i1p1.log` (the
strafe2 OFF run, 2026-10-12 03:14 local, on the tree that produced `ba711c180`). Both runs use:
- IJN01 with `s38_i1_p1.txt`, 24200/24000 frames at 0.05 s;
- `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1` (`cc9-lua46\local\l46_lane.ps1`,
  `cc13-strafe2\local\st2_run.ps1`).

Both logs still exist; they are not in this repository.

These gunnery commits on `src/game_hosts_gunnery.cpp` between `7821e75b8` and `ba711c180` can reach AA
fire on a plane:

| commit | switch (now) | what it can move | in this run |
| --- | --- | --- | --- |
| `57c010ddf` | `kMeshlessGunPointWindowBound` ON (GUNNERY 155.1) | meshless gun fire window | `candidates=24 inserts=24`. The meshless rounds are plane guns (devices 95/98/101), not PT/LST guns |
| `c0d5741f2` | GUNNERY 138 ON, rack device | plane rack devices | none on AA |
| `acd45f8c1` | `kTailGunnerNoArmourTestBound` ON (164.4) | plane-owned kind-1 guns (B-17 tail guns) | `armour_rejects_tail=0` |
| `acd45f8c1` | `kAaLineOfFireExpiryDrawBound` ON (164.4) | one RNG draw per line-of-fire query (`Draw::line_of_fire_ttl`) | `lof_expiry_draw=1`. It is a keyed draw, and nothing reads its value |
| `90493a7c3` | `kAaKind6GateBound` ON (182.3) | cat-6 dual-purpose window gates | `kind6_gate_rejects=0`; not cat-1 |
| `ac2c85217`, `01b53daac` | health callback guard | callback timing only | per-hit damage unchanged (section 4) |

The other gunnery commits in the range move:
- torpedoes: the submerged launch, group fire, the TorpedoBot, torpedo group 4;
- depth charges;
- shipyard hiding and release;
- the magazine slice;
- bomb drift: `kBombDropDriftBound` ON, `d635effc3`. This changes where bombs land, not AA.

## 2. The chain, image against host (cat-1 MG on a plane)

| term | image (read) | host (read) | faithful? |
| --- | --- | --- | --- |
| **Fire rate / burst** | `0072D860` 0ADh arm -> FireIfReady `00727E30` -> CanFire `0085A830` over the barrel reload timers. `00730160` fires one barrel and sets that barrel's timer to `ReloadTime` | the same gate (`bsp::gun_can_fire_turning_0085a830`), one FireIfReady per gun per frame, `next_ready_barrel_007298d0`, timer = `reload_time` (`src/game_hosts_gunnery.cpp` around the `0072D860` comment) | yes in structure. See open item 1 for the reload-0 cadence |
| **Dispersion** | `00730160` throw cone = `Throw * BulletThrowMul` (`0073031D`) | bound (`cc9_bullet_throw`). Measured mean 0.411 deg (10-06: 0.438) = Throw 0.006981 rad x 1.0 | yes |
| **Aim error** | AAGunnerBot `00902920` swinging error at the shooter's skill row. US AA in IJN01 rolls on row 1, SPNormal | bound (`cc9_aa_lethality_audit`, AA_LETHALITY_AUDIT 7). Rolls `0/2940/0/0/0/0`, mean miss 19.10 m (10-06: 18.70 m) | yes |
| **Hit test** | GeomMesh element walk (`0087BCC0 -> 00712440 -> 00724510`) | `kPlaneMeshHitTestBound` ON since AA_LETHALITY_AUDIT 9.5 (2026-09-30). 26915 tests, 23879 mesh hits (10-06: 27017 / 23919) | yes. Section 9's labelled substitutions stand |
| **Per-hit damage** | the plane's vtable[ECh] `007BBCF0` -> `007BBDB1 CALL 008777D0`. The hull pass `008778A4 CALL 00470510` = `(rec+14h * modifier - armour) * weapon scale` (`00470598`-`004705B1`, x87 FMUL/FSUB/FMUL). Only if > 0 (`008778AD`-`008778B7` FCOMI) does it reach vtable[1ACh] = `0095DA00`: the role-0 cheat inverse (`0095DA7A` FMUL), then `0095DA8D CALL 0087D730`. In single player (`game+1FE4h == 0`) that scales by `1/HP[level]` only when `unit+54h` is the local player's side (`0087D743`-`0087D756`). | `rec+14h` = U(DamageMin, DamageMax) (`006E7C60`). `hull_damage_00470510`, then `difficulty_scaled_damage` (role 0, then party), then `apply_damage_00879070` (`ShipHitBinding::add_damage`) | yes. The measured rate per hit matches the formula (section 3) |
| **Kill rule** | `00879070` subtracts. The plane dies at health <= 0 (`plane death mode` lines name `007CA8A0`) | `apply_damage_00879070` / `set_health_00877b90`. The death rows sum to exactly 280 = HP, for example `A7M_5`: c1 148 + c6 132 | yes |

The host runs a plane through `apply_ship_hit_record_00826f10`, not straight to `008777D0`. The ship-only
steps in it change nothing for a plane, for two reasons:
- The measured per-hit damage is scaled once, not twice.
- No plane death row carries fire or flood damage.

This is the existing labelled path. It is not new since 10-06.

## 3. This installation's data (the install is modded; cite mtimes)

**The class tables.** `gamemode.lua` (2024-07-13) sets `GameMode = 0`. So `scripts/datatables/autoload/
bulletclasses.lua` and `deviceclasses.lua` load the **arcade** tables. Both arcade tables are modded:
`classtables/arcade/deviceclasses.lua` 2026-05-09 22:37, `bulletclasses.lua` 2026-05-09 23:04.

| unit / row | file:line (this installation) | values |
| --- | --- | --- |
| PT (`TBoatGen`, `Type = ShipClasses : Elco`, `ijn_1_pearl.scn` 9534) | `autoload/vehicleclasses.lua` (2026-05-09 21:52) 17667 `VehicleClass[27]` | HP 400 (17753), Armour 4 (17679). AA 1 = device 286 (17836), AA 2 / AA 3 = device 44 (17866, 17896) |
| device 44 "Elco gunner" | arcade `deviceclasses.lua` 2500 | AAMACHINEGUN, Bullet 43, **ReloadTime 0**, Throw 0.006981, one Bullet entry. The realistic table (2024-07-13) line 2393 has ReloadTime 0.1 |
| device 286 "AA cannon 20mm" | arcade `deviceclasses.lua` 11797 | Bullet 40, ReloadTime 0.24 |
| LST (`LandingShipGen`, `LST_Strafeable`, scn 9365) | `vehicleclasses.lua` 143668 `VehicleClass[345]` | HP 1500, Armour 10. Device 40 x2 (144092, 144122), device 18 (144152, artillery), device 42 (144183) |
| device 40 / 42 | arcade `deviceclasses.lua` 2304 / 2402 | 20 mm Oerlikon: Bullet 40, reload 0.24. 40 mm Bofors: Bullet 42, reload 0.5, BarrelDelayTime 0.25 |
| bullets 43 / 40 / 42 | arcade `bulletclasses.lua` 1266 / 1209 / (row 42) | .50: 20-25. 20 mm: 25-30. 40 mm: 40-45. No `Range`, so no falloff |
| A7M (`PlaneSquadronGen`, `PlaneClasses : Zero`, scn 7997; log `type_id=150`) | `vehicleclasses.lua` 52416 `VehicleClass[150]` "A6M Zero" | HP 280 (52509), Armour 5 (52426) |
| AAGunnerBot SPNormal | `scripts/datatables/robots.lua` (2025-06-01) 164 | ConstAngleError 4, AngleDiffErrorRatio 5, BulletThrowMul 1.0 |
| party HP inverse, level 1 | log `summary mission gunnery difficulty` | `hp_inverse=0.5556`; the player is Japan, so the A7Ms are scaled |

**Expected damage per hit on an A7M:** (U(min, max) - 5) x 0.5556.

| round | damage per hit (mean) | hits per 280 HP |
| --- | --- | --- |
| .50 | 8.33-11.11 (9.72) | 28.8 |
| 20 mm | 11.11-13.89 (12.5) | 22.4 |
| 40 mm | 19.44-22.22 (20.8) | 13.5 |

**Measured:**
- PT2's .50 kill: 29 hits for 280 (9.66 per hit).
- PT3's kills: 28-29 hits for 280.
- LST kills: 16-22 hits, from a 20 mm / 40 mm mix.
- Mean cat-1 damage per hit on the dead A7Ms: 10.17 on 10-06 and 11.09 now. The rise is the larger LST
  share of the kills.

All of this matches the expected values.

## 4. The runs, side by side

Summaries are from `local\mgaa_deaths.py`, `mgaa_cmp.py`, `mgaa_orders.py` and `mgaa_firstdiff.py` in this
tree. They read the logs and do not run anything.

| | 10-06 (`l46ue_off_ijn01p1`) | now (`st2_i1p1`) |
| --- | --- | --- |
| A7M deaths, all squadrons (one row per victim) | 19 | 26 |
| ordered A7M_1..5 deaths (of 25) | 15 | 24 |
| ... by PT / LST | 7 | 19 |
| ... by B-17 gunners | 6 | 3 |
| PT/LST deaths (of 15) | 15, last LST11 at 395.18 s | 6, last PT4 at 286.50 s |
| AA seat rounds (`bullet throw seats aa`) | 14918 | 22931 |
| plane hit records (`plane hit notice`) | 966 | 1063 |
| hit records per AA round | 6.5 % | 4.6 % |
| SPNormal gunner rolls, mean miss | 1682, 18.70 m | 2940, 19.10 m |
| cat-1 hits / damage on the dead A7Ms | 508 / 5166 | 611 / 6774 |
| strafe aims, strafe gun-point fires | 105, 2466 | 56, 1343 |
| plane gun rounds | 10155 | 6310 |

On the counts:
- The strafe2 doc's "13 on 10-06, 21 now" were counted another way.
- The numbers above count one death row per victim, over the squadrons `A7M_1`..`A7M_5`, and include the
  B-17 kills.
- The direction and the size of the rise are the same either way.

**Where the outcomes split.**
- **Before any shot.** The timestamped lines agree up to `gunnery step 1500 t=75.00 shots=0`. The first
  difference is the strafe edge at 75.30 s: `A7M_1|.-3` alt 856.7 vs 856.3, and `A7M_1` h 3210.9 vs 3211.2.
  This is a flight difference. The plane, squadron and terrain-avoid paths changed in the same range. One
  sign: `vehicle/terrain avoid A7M_1|.-3` reports `terrain=0` on 10-06 and `terrain=19 bands=30
  min_margin=101.0` now. Which commit causes it is not bisected here.
- **About 134 s.** On 10-06, B-17 01|.-2 kills the wingman `A7M_2|.-3` (133.90 s), and the leader `A7M_2`
  lives to 325.44 s. Now B-17 01 kills the leader `A7M_2` (135.10 s). From frame 2802 every `select A7M_2`
  is refused (order lines 70, 71, 81, 82, ...).
- **150-172 s.** On 10-06 the B-17s wipe out the whole of `A7M_5` (150.90-172.51 s), so its later orders
  are refused. Now `A7M_5` survives them and its orders are applied (lines 87, 88, 98, 99, 109, 110).
  Its five planes then die at 246-434 s to PT3, LST3 and LST5, so their deaths move from "B-17" to "PT/LST".
- **After that.** Every A7M that dies to PT/LST fire after 395 s (`A7M_5` 434.57, `A7M_2|.-5` 497.60,
  `A7M_2|.-2` 597.38) dies to a ship that on 10-06 was already sunk.

## 5. Verdict

**Faithful; no diff.** The losses are expected under this run's events, but not for the reason the brief
offered. The A7Ms are not strafing more. They are flying for longer near PTs and LSTs that are not being
sunk. One reason is that a leader's early death strands its wing, which is the host defect in
`CC13_AUDIT_REATTACK_AND_SKILL` A (`kFusedSquadronSlotSelectableBound`, OFF). The other is that the run
diverged in flight before 75 s. Landing that switch is the lever for IJN01. The AA chain is not.

## 6. Open

1. **The cadence of a reload-0 gun.** This installation's Elco gunner (device 44) has `ReloadTime 0`, a mod
   value; the realistic row has 0.1.
   - Both image and host fire at most one round per FireIfReady, so a reload-0 gun fires once per
     gunnery tick.
   - The host's tick is 0.05 s, which gives 20 rounds/s.
   - The image's AA-bot tick rate is not read here. `docs/APP_INIT_PLATFORM.md` records a 50 ms fixed
     clock, enabled when `DAT_00e1ae81` is set.
   - If the image ticks the bots every render frame, it fires these guns **faster** than the host, not
     slower. Either way the host is not too lethal.
2. **The flight divergence before 75.30 s.** It is not attributed. It belongs to the plane/terrain lanes,
   not AA. A same-tree pair on the candidate flight switches would settle it. The candidates are:
   - the terrain/avoid path;
   - `kDiveAimHeadingBlendBound` (`ea7983dbb`, ON);
   - the land-command switches (SHIP_AI 208/210).
