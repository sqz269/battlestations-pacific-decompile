# The ship AI's own curve: rating this ship's guns against the actual target

Addresses: 009F1BC0, 009F29E0, 009F29EC, 009F2A26, 009F2A2C, 009F2A3C, 009F2A44, 009F2A54, 009F2A65, 009F2A6B, 009F2A7F, 009F2A91, 009F2A99, 009F2AA9, 009F2AB1, 009F2AC1, 009F2ECB, 009F2ED2, 009F2F11, 004407A0, 009635D0, 00D05EAC, 00810F60, 0081106E, 009E8171, 009E5DA0

Packet `cc9_own_curve_target`, 2026-09-23. The report is `reports/ship_ai_own_curve.json`. Names
are hypotheses, not recovered symbols. Nothing here is ABI-compatible or game-validated.

**Status: bound, measured and landed.** The switch is `kShipAiOwnCurveTargetBound` in
`src/game_hosts_ship_ai.cpp`, default true.

## 1. The read

**The target** is the same object as the target curve's. `009F29E0..009F2A02` loads
`[owner+0B20h]`, the raw command target, and keeps it in EBX only if `vtable[5Ch](5)` answers,
the unit subtree. The own block at `nested+127Ch` is then filled:

| word | offset | with a target (EBX) | site | without | site | width |
|---|---|---|---|---|---|---|
| range | +127Ch | `nested+11E0h` | 009F2A04 | same | | float |
| length | +1280h | `[[t+538h]+0A0h]` `Length` | 009F2A54 | 100.0f (00CE3D08) | 009F2AC1 | float |
| damage cap | +1284h | `[t+370h]` health | 009F2A44 | 10000.0f (00CE3D64) | 009F2AA9 | float |
| armour | +1288h | `[[t+538h]+4Ch]` `Armour` | 009F2A2C | 0 | 009F2A91 | float |
| torpedo armour | +128Ch | `[t+538h]->vtable[24h]()`: ship class `009635D0` (`+6B4h` `UnderwaterArmour`); plane class `004407A0` (`FLD [ECX+4Ch]`, `Armour`) | 009F2A3C | 0 | 009F2A99 | float |
| bearing | +1290h | `nested+11DCh` | 009F2A18 | same | | float |
| fire divisor | +129Ch | `[[t+538h]+6B8h]` `DamageThreshold` if `t` answers `vtable[5Ch](6)` (a ship), else 10000.0f | 009F2A65..7F | 10000.0f | 009F2AB1 | float |
| +40h, +41h | +12BCh, +12BDh | 0, 1 | 009F2ECB, 009F2ED2 | same | | bytes |

- **The plane class `vtable[24h]`** is `004407A0` in nine plane class vtables, for example
  00D05EAC+24h. It is `FLD dword ptr [ECX+4Ch]; RET`, so a plane's torpedo armour is its `Armour`.
  Ghidra has no function there (`proto --brief` reports none; FUN_00440770 ends at 00440788).
- **The host's no-target constants were the image's**, except the fire divisor. The host left it
  at 0, where `009F2AB1` stores 10000.0f.
- **The unit radius `+9C8h`** is read only by the target curve's block, not by this one. Its
  producer is corrected in docs/SHIP_AI_TARGET_CURVE.md section 7.

## 2. The binding

`fill_own_block_target_127ch()` fills the block from the target as above, falling back to
`009F2A91..009F2AC1`'s constants.
- Health comes from the gunnery host's public `unit_rows()`.
- `Length`, `Armour`, `UnderwaterArmour` and `DamageThreshold` come from the live `VehicleClass` row
  through the Lua host's reader.
- The class `vtable[24h]` is taken as `UnderwaterArmour` for a ship (IsKindOf 6) and `Armour`
  otherwise. Only the ship and plane families were read; others are LABELLED.
- The no-target fire divisor is now 10000.0f.

## 3. Prediction against measurement

The diagnostic build dumped both curves at each ship's first target refresh (`local\oc_dg_usn04.log`),
and `local\standoff_replay.py` scanned them. Every ship's first target is `D3A Val #1.1`: cap 220,
length 12, armour 5, torpedo armour 5, fire divisor 10000. Against a 220 HP target, the own curve
sits on its cap over a long span, and the long-range bias puts its peak at the far end:

| ship | replay (first choice) | measured first | measured last | heading changes, control -> treatment |
|---|---|---|---|---|
| Northampton-class01 | 2250 | **2250** | 300 | 7 -> 15 |
| Northampton-class02 | 2250 | **2250** | 300 | 110 -> 41 |
| Fletcher-class01, 02 | 1450 | **1450** | 300 | 25 -> 57, 61 -> 39 |
| Fletcher-class03 | 1550 | **1550** | 300 | 63 -> 58 |
| Fletcher-class04 | 1450 | **1450** | 300 | 68 -> 42 |
| York-class01, 02 | 1850 | **1850** | 300 | 116 -> 5, 38 -> 37 |

- **Every first standoff matches the replay.**
- **Every last standoff is 300 m.** That is the scan's seed, the target curve's effective range
  (0 here) plus 300. The scan keeps the seed when no own-curve sample is positive. That is what a
  dead target gives: cap = its health = 0, so every sample is 0 minus the bias. This is inferred
  from the scan and the cap; the last target of each ship was not dumped.
- **Heading changes move both ways.** That is consistent with section 5 of
  docs/SHIP_AI_TARGET_CURVE.md: they are not a function of the standoff alone.

## 4. Runs

Worktree root `J:\PROG\battlestations-pacific-decompile-cc9-difficulty`, base `16bd5760e`.
`BSP_GUNNERY_RNG_STREAMS=1` is set on every run. Control `build\win32\ctlA\` (switch off), treatment
`build\win32\treatA\`.

```
$env:BSP_GUNNERY_RNG_STREAMS='1'; ./tools/run_game.ps1 -Exe build\win32\<ctlA|treatA>\bsp_game.exe -Log local\oc_<ctl|trt>_usn04.log -- --frames 4700 --press-start-frame 30 --menu-select USN04 --mission-frames 4500 --mission-frame-seconds 0.05
$env:BSP_GUNNERY_RNG_STREAMS='1'; ./tools/run_game.ps1 -Exe build\win32\<ctlA|treatA>\bsp_game.exe -Log local\oc_<ctl|trt>_usn01.log -- --frames 3200 --press-start-frame 30 --menu-select USN01 --mission-frames 3000 --mission-frame-seconds 0.05
```

| run | hits | damage | deaths | Lexington | log |
|---|---|---|---|---|---|
| USN04 control | 104 | 14648.4 | 10 | sunk 223.86 s (York-class02) | `local\oc_ctl_usn04.log` |
| USN04 treatment | 103 | 15656.2 | 9 | **afloat, 142 HP** | `local\oc_trt_usn04.log` |
| USN01 control / treatment | identical | identical | identical | - | `local\oc_ctl_usn01.log`, `local\oc_trt_usn01.log` |

- **USN04's per-unit flips** are Lexington's survival, Kate #6.1 (220.86 s to 220.81 s) and
  movieval|.-2 (193.46 s to 193.86 s).
- **USN01** has no approach curve, so nothing moves there.
- The downstream moves are coupled to the new stand-off geometry and the gunnery state; they were
  not ablated. The York escort's friendly-fire kill of Lexington no longer happens.

## 5. Step 4, read-only: the ring scan and the avoidance refresh

* **The ring scan uses the same block.** 009E7FC0 hands `nested+127Ch` to 009E5DA0 after writing
  `+40h = 1` and `+41h = 1` into it (009E8171, 009E8178, `ebp+12BCh`/`+12BDh`), plus words 5-7. So
  in the image the ring's rating sees the real target fields.
  - **The host does not match.** `ring_query()` in `src/game_hosts_ship_ai.cpp` still uses the
    no-target constants and a fire divisor of 0.
  - The fix is to fill it with `fill_own_block_target_127ch()` and a 10000 divisor. Because the ring
    divides each slot by the frame maximum, only the relative mount weights would change. That is
    the natural next item; it was not done here because step 4 was read-only.
* **The avoidance refresh 009E6240** takes a block pointer from its caller and writes words 0 and 5
  and `+40h` into it (009E634F). Which block its caller passes was not traced.

## 6. Decision

**Land it.** The own block is the image's. Every first standoff is predicted by the replay, and
the last standoff follows from the image's rule for a dead target.

## 7. Correction, 2026-09-23 (packet `cc9_ring_query`, `docs/SHIP_AI_RING_QUERY.md`)

* **004407A0 is not plane-specific.** It is the base vehicle class's `vtable[24h]` (the class
  `Armour`), installed by every family read except the ship class: the plane classes, runway
  00D1A9A0, door 00D1AA58, structure 00CFF790 and wreckable 00D1AA18. The ledger name is now
  `BSP_VehicleClass_GetUnderwaterArmourDefault`, and the binding's "ship → UnderwaterArmour,
  otherwise Armour" is exact for every family, not LABELLED.
* **Section 5's ring finding is now bound** by cc9_ring_query.
