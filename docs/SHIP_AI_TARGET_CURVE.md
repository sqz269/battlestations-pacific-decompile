# The ship AI's target curve: the target's rating of this ship

Addresses: 009F1BC0, 009F28FA, 009F2906, 009F2920, 009F2931, 009F294B, 009F2963, 009F2969, 009F2985, 009F299B, 009F29A1, 009F29B2, 009F29CA, 009F29D1, 009F29D8, 009F29E0, 009F2733, 009F2FB1, 009635D0, 00D1ACC4, 009E71A5

Packet `cc9_target_curve`, 2026-09-23. The report is `reports/ship_ai_target_curve.json`. Names are
hypotheses, not recovered symbols. Nothing here is ABI-compatible or game-validated.

**Status: bound, measured and landed.** The switch is `kShipAiTargetCurveBound` in
`src/game_hosts_ship_ai.cpp`, default true. The own curve is untouched.

## 1. The read

**What "the target" is.** `009F29E0..009F29FE` takes `EDI = [owner+0B20h]`, the approach's raw
command target, and keeps it only if it answers `vtable[5Ch](5)`, the unit subtree. In the host that
is `goal_vector.raw_target_0b20`, a one-based unit handle (`resolve_command_target_00521ea0`).
`009F2FB1` then runs `0095F080` with `this` = that target and the block at `nested+1238h`, with
`prefer_long_range = 0`, so no bias. It writes `nested+13B0h`.

**The block at nested+1238h describes THIS ship** (`[owner+0AA8h]`), because it is the target's
rating of this ship. It is filled at `009F28FA..009F29D8`:

| word | offset | source | site | width |
|---|---|---|---|---|
| range | +1238h | `nested+11E0h`, then stepped by 0095F080 | 009F2995 | float |
| target length | +123Ch | `[unit+9C8h]`, the unit radius. It has no producer in the image (docs/GAME_EXECUTABLE.md); host 0, so the small-target curve weighs 1. | 009F294B | float |
| damage cap | +1240h | `[unit+370h]` health | 009F2931 | float |
| armour | +1244h | `[[unit+538h]+4Ch]` `Armour` | 009F2906 | float |
| torpedo armour | +1248h | `[unit+538h]->vtable[24h]()` = `009635D0`, `FLD [class+6B4h]`, `UnderwaterArmour` | 009F2920 | float |
| bearing | +124Ch | `nested+11DCh`, unused because +40h is clear | 009F299B | float |
| window | +1250h | 20.0f (00CE3930) | 009F2985 | float |
| ready horizon | +1254h | 30.0f (00CE38C8), unused because +41h is clear | 009F29A1 | float |
| fire divisor | +1258h | `[[unit+538h]+6B8h]` `DamageThreshold` | 009F2963 | float |
| word 9 | +125Ch | 5.0f (00CE3850), never read | 009F2969 | float |
| word 14 | +1270h | 0.0f, never read | 009F29D8 | float |
| gates | +1274h..+1277h | `BL` = 1 from `009F2733 MOV EBX,1`; EBX survives the calls to 009F29B2 | 009F29B2..C4 | bytes |
| +40h, +41h | +1278h, +1279h | 0, 0 | 009F29CA, 009F29D1 | bytes |

**Into the scan.** The standoff scan at 009E71A5 samples the target curve as `secondary(x)` and
scores `max(1, secondary) × nested+1284h / primary × interp(primary/peak)` (docs/SHIP_AI_APPROACH_CURVES.md).
With `them = 0` the floor makes the score `w / primary`, minimised at the own curve's peak.

**Host against image, before this packet.** The host built the target curve from this ship's own
`FirepowerBinding` and its own query: the no-target constants (cap 10000, length 100, armour 0),
and only the bias dropped. So `them(x)` was this ship's own rating.

## 2. The binding

`target_query_1238h()` fills the block as the table above.
- Health comes from the gunnery host's public `unit_rows()`.
- `Armour`, `UnderwaterArmour` and `DamageThreshold` come from the live `VehicleClass[type_id]` row
  through the Lua host's existing `read_vehicle_class_number`. The loader defaults are 0, 0 and 100.
- `FirepowerBinding(owner_, target)` then rates the target's gun rows.

No gunnery accessor was needed.

## 3. Prediction against measurement

**Prediction.** Every USN04 attackmove target is an aircraft. Its only rated mount is the category-1
rear gun, whose `hi = max(DamageMax, BlastDamageMax)` sits far below a ship's `Armour`, so
`k = 0` and `them(x) = 0` at every range. The standoff therefore stays at the own curve's peak, and
**every standoff is unchanged**. USN01 builds no approach curve and is unchanged.

**The replay.** A diagnostic build dumped each ship's first own and target curves
(`local\tc_dg2_usn04.log`), and `local\standoff_replay.py` scanned them:

| ship | Armour / UnderwaterArmour / DamageThreshold / HP | them max | replay | measured |
|---|---|---|---|---|
| Northampton-class01, 02 | 90 / 160 / 80 / 6700 | 0 | 1050 | 1050 |
| Fletcher-class01..03 | 50 / 50 / 50 / 2500 | 0 | 200 | 200 |
| Fletcher-class04 | 50 / 50 / 50 / 2500 | 0 | 50 | 50 |
| York-class01, 02 | 85 / 150 / 80 / 6000 | 0 | 350 | 350 |

Every standoff matches the replay. A first diagnostic build read the class rows with a raw table
index and got armour 0 (`local\tc_dg_usn04.log`). There the Val's gun did reach, and the replay
moved the Fletchers and Yorks to 800 m. The armour term decides this outcome.

## 4. Runs

Worktree root `J:\PROG\battlestations-pacific-decompile-cc9-difficulty`, base `e26ed8cf9`. `BSP_GUNNERY_RNG_STREAMS=1` is set on every run. Control `build\win32\ctl7\`
(switch off), treatment `build\win32\treat8\`.

```
$env:BSP_GUNNERY_RNG_STREAMS='1'; ./tools/run_game.ps1 -Exe build\win32\<ctl7|treat8>\bsp_game.exe -Log local\tc_<ctl|trt>_usn04.log -- --frames 4700 --press-start-frame 30 --menu-select USN04 --mission-frames 4500 --mission-frame-seconds 0.05
$env:BSP_GUNNERY_RNG_STREAMS='1'; ./tools/run_game.ps1 -Exe build\win32\<ctl7|treat8>\bsp_game.exe -Log local\tc_<ctl|trt>_usn01.log -- --frames 3200 --press-start-frame 30 --menu-select USN01 --mission-frames 3000 --mission-frame-seconds 0.05
```

| run | standoffs (N01, N02, F01-F04, Y01, Y02) | heading changes | hits | damage | log |
|---|---|---|---|---|---|
| USN04 control | 1050, 1050, 200, 200, 200, 50, 350, 350 | 7, 110, 25, 61, 63, 68, 116, 38 | 104 | 14648.4 | `local\tc_ctl_usn04.log` |
| USN04 treatment | identical | identical | 104 | 14648.4 | `local\tc_trt_usn04.log` |
| USN01 control / treatment | no approach | - | identical | identical | `local\tc_ctl_usn01.log`, `local\tc_trt_usn01.log` |

- **USN04's per-unit table is identical.** The only diffs are `curve_target_nonzero` (45 to 0, the
  prediction), the refills counter, and a census count, `curve_refreshes`. The treatment binary
  did not count the new branch; that is fixed in the committed source and was re-run as
  `local\tc_trt2_usn04.log`.
- **The durable measurement is the standoffs per ship.** The reference rows will be re-taken by the
  gunnery worker after its packet. docs/GAME_EXECUTABLE.md is leased to that worker, so the dated
  note for it is in the report, not in that file.

## 5. Step 4: the heading-change attribution does not hold

docs/WEAPON_HIT_ACCURACY.md attributed the heading-change rise to the closer standoffs. An
ablation pinned every standoff back to its pre-profile figure (Northamptons and Yorks 950 m,
Fletchers 200 m) with the profile still loaded (`local\tc_pin_usn04.log`). The pin was applied after
009E6E80 returns, so that routine's own tail still saw the unpinned value.

| ship | pre-profile (950/200) | profile, free | profile, pinned |
|---|---|---|---|
| Northampton-class02 | 12 | 110 | **110** |
| York-class01 | 57 | 116 | 80 |
| York-class02 | 4 | 38 | 50 |

Pinning the geometry back barely moves the counts, and moves York-class02 the other way. The rise
comes mostly from the ring scan's per-mount reweighting and from gunnery coupling, not from the
stand-off distance. The previous packet's treatment, on an older main without RNG streams, gave Northampton-class02
62 heading changes. Current main with RNG streams gives 110. The counts move with the gunnery
state. A correction to docs/WEAPON_HIT_ACCURACY.md is appended there.

## 6. Decision and open items

* **Decision: land it.** The target curve is the image's construction. Every standoff matches the
  replay, and USN04 and USN01 are otherwise identical.
* **Open:** the unit radius `unit+9C8h` has no producer, so the target length stays 0. It weighs
  only the small-target accuracy curve, and on this mission that curve is multiplied by `k = 0`.
* **Open:** the own curve still uses the no-target constants (cap 10000, length 100, armour 0)
  where the image reads the target's `+370h`, `+A0h`, `+4Ch`, `vtable[24h]` and `+6B8h`
  (009F2A26..009F2A77). Binding it the same way will move the standoffs. That is the natural next
  packet.

## 7. Correction, 2026-09-23 (packet `cc9_own_curve_target`, `docs/SHIP_AI_OWN_CURVE.md`)

* **The unit radius `+9C8h` has a producer.** Section 1 repeated docs/GAME_EXECUTABLE.md's "no
  producer anywhere". A scan of the disp32 store forms finds three writers:
  - `0081106E` in `BSP_Unit_InitializeDirectorAndHullDimensions` (00810F60), the ship's unit
    init. With model bounds at `[class+50h]` it stores `+9CCh` = the larger x half-extent, and
    `+9C8h` = 2.0 (00D7A308) × that half-extent × the larger z half-extent, as traced. That
    product wants a second read. Without bounds it stores class `+A4h` and `+A0h` (Length).
  - `0081FA4D` in 0081F980, a save and load path.
  - `004EC28E` in 004EB9B0, a scene-header property store.

  The host still feeds 0. That only matters where the target curve is non-zero, which it is not
  on USN04.

## 8. Correction, 2026-09-23 (packet `cc9_ring_query`, `docs/SHIP_AI_RING_QUERY.md`)

* **Section 7's formula for `+9C8h` was wrong.** The FPU trace took `FMUL ST1` at 0081100C as writing
  ST0. Its bytes are `DC C9`, `FMUL ST(1), ST(0)`, which writes ST1. So:
  - `+9CCh` = 2 × max(xmax, −xmin), the unit's full width.
  - `+9C8h` = 2 × max(zmax, −zmin), the unit's full length.

  This is the same quantity as the no-bounds arm's class `+A0h` (Length) and `+A4h`. `+9C8h` is the
  unit's length, not a radius or a product.
