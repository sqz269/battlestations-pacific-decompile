# WeaponHitAccuracy: the getter, the authored profiles, and what they do to the standoff

Addresses: 008386F0, 008383D0, 00419010, 00836EF0, 00836F80, 0083C795, 0083C81A, 0083C86F, 0083C8C4, 0083C919, 006EB060, 009E8171, 009E8178, 009F29CA, 009F29D1, 009F2ECB, 009F2ED2, 009E634F, 0085AB50, 005471B0

Packet `cc9_hit_accuracy`, 2026-09-23. The report is `reports/weapon_hit_accuracy.json`. Names are
hypotheses, not recovered symbols. Nothing here is ABI-compatible or game-validated.

**Status: bound, measured and landed.** The switch is `kWeaponHitAccuracyBound` in
`src/game_hosts_ship_ai.cpp`, default true.

## 1. The getter

**008386F0, `BSP_GameSettings_WeaponHitAccuracy`**:
`__thiscall(ECX = settings, int function, float target_length, float t)`, `RET 0Ch`, read whole.

| function | profile | site |
|---|---|---|
| 2, 3, 4, 6 | `settings+240h`, Artillery | 00838713 |
| 1, 5 | `+298h`, AA | 0083873D |
| 7 | `+2F0h`, Torpedo | 00838762 |
| 8, 9 | `+348h`, DepthCharge | 0083878C |
| anything else | `FLD1`, 1.0 | 008387A1 |

Its only caller is `006EB060`, which passes function 3 for artillery sub-types (4-7), 7 for
torpedoes (0Ah), 8 for depth charges and mines (0Bh, 13h), and 1 for bullets and flak
(1, 2, 3, 10h). It passes `t = range / [proj+60h]`.

**008383D0, `BSP_WeaponHitAccuracy_Sample`**: `__thiscall(profile, float target_length, float t)`,
`RET 8`, body `008383D0-0083851E` read whole.
- **Size weight.** `w = 00419010(x0 = [+0h], y0 = 1.0, x1 = [+4h], y1 = 0.0, x = target_length)`,
  clamped. It is 1 at the small reference size and 0 at the large.
- **Buckets.** Bucket k is centred at `t = (k+1)/10`.
  - `t ≤ 0.1` (the double at 00D7A3A0) uses bucket 0 alone.
  - `t ≥ 1.0` (the float at 00D7A24C) uses bucket 9 alone.
  - Otherwise `i = clamp(−1 − _ftol(t × −10.0), 0, 8)`, with −10.0 at 00D0A198 and 00BF7420 as the
    truncating `_ftol`. Then `d = 10t − (i+1)` (10.0 at 00CE3DC0), clamped to [0, 1].
- **Result.**
  `w·(small[i]·(1−d) + small[i+1]·d) + (1−w)·(large[i]·(1−d) + large[i+1]·d)`, with small at
  `+8h..+2Ch` and large at `+30h..+54h`.

**The default** when nothing loads is `00836EF0`'s: sizes 100 and 200, every slot `0.5f`
(00CE3800).

**Other readers.** None found. Ghidra's call graph gives `006EB060` as 008386F0's only caller (a
direct CALL; docs/GAMEPLAY_SETTINGS_TAIL.md found no literal-offset reader of the profiles either),
and 006EB060's only caller is `0095EB40`, the ship-AI rating. The gunnery hit resolution
does not read these profiles.

## 2. The profiles as loaded

`0083C795..0083C919` hands each sub-table of `ShipGlobals["WeaponHitAccuracy"]` to `00836F80`:
Artillery (0083C81A), AA (0083C86F), Torpedo (0083C8C4), DepthCharge (0083C919). The host now does
the same, inside its existing ShipGlobals load (`GameMissionLuaHost::load_weapon_hit_accuracy_0083c795`,
run from `load_ship_globals_0083b6e6`). The authored values are in this installation's
`scripts/datatables/shipglobals.lua`, lines 148-209. Each cell below is small / large:

| % of max range | Artillery {100, 250} :150 | AA {80, 200} :164 | Torpedo {100, 300} :178 | DepthCharge {100, 200} :192 |
|---|---|---|---|---|
| 10-40 | 1.0 / 1.0 | 1.0 / 1.0 | 0.6/0.8, 0.6/0.8, 0.5/0.7, 0.4/0.6 | 1.0 / 1.0 |
| 50 | 1.0 / 1.0 | 1.0 / 1.0 | 0.3 / 0.45 | 0.9 / 0.9 |
| 60 | 1.0 / 1.0 | 0.9 / 1.0 | 0.22 / 0.3 | 0.8 / 0.8 |
| 70 | 0.9 / 1.0 | 0.8 / 1.0 | 0.15 / 0.22 | 0.7 / 0.7 |
| 80 | 0.7 / 0.9 | 0.7 / 0.9 | 0.1 / 0.15 | 0.6 / 0.6 |
| 90 | 0.5 / 0.8 | 0.5 / 0.7 | 0.05 / 0.1 | 0.5 / 0.5 |
| 100 | 0.3 / 0.5 | 0.3 / 0.5 | 0.02 / 0.05 | 0.4 / 0.4 |

The run log prints the four loaded profiles (`weapon hit accuracy ...`), and they match the file.

## 3. Prediction against measurement

**Predicted** (before the run): the Northamptons back at about 1450 m, because the AA and artillery
curves sit near 1.0 over most of the range, like the old fallback. Heading changes and hits near
the arc-only ablation of docs/SHIP_AI_FIREPOWER.md.

**Measured.** The prediction was wrong:

| ship | control (0.5 default) | treatment (authored) |
|---|---|---|
| Northampton-class01, 02 | 950 m / 7, 12 heading changes | **1050 m** / 7, **62** |
| Fletcher-class01..03 | 200 m / 12, 53, 63 | 200 m / 12, 53, 63 |
| Fletcher-class04 | 200 m / 70 | **50 m** / 68 |
| York-class01, 02 | 950 m / 57, 4 | **350 m** / **116**, **36** |

**Why, with the curves.** A temporary one-shot dump of each ship's first own curve, in both
builds (`local\ha_dgC_usn04.log`, `local\ha_dgT_usn04.log`, 3000 frames), and an offline replay of
the 009E71A5 scan (`local\standoff_replay.py`) reproduce **every** measured standoff exactly. In
every case the standoff equals the peak of the ship's own biased curve:
- **The host's "target" curve is the ship's own rating.** `src/game_hosts_ship_ai.cpp` builds
  `approach_curve_target` with the same `FirepowerBinding(owner_, index_)` and the same query block
  as the own curve. It only drops the long-range bias, so `max(1, them)/us` is minimised at the
  own curve's peak. In the image, 009F2FB1 rates the TARGET unit with block `nested+1238h`. This is
  a host defect in the approach seam, reported in section 5 and not fixed here.
- **The authored profile moves that peak.**
  - The Northamptons' curve now sits on the rating cap (`b[2]·max(1, b[6]/5)`) from 500 m to
    1050 m: 39949 at 500 m and 39959 at 1000 m, the difference being the bias. The long-range bias
    then puts the peak at the plateau's far end, 1050 m.
  - Under the flat 0.5 default the curve was below the cap and peaked at 950 m.
  - Under the 1.0 fallback it was presumably capped over a longer span, which would give 1450 m.
    That build was not dumped, so this is an inference.
  - The Yorks' and Fletcher-class04's weapons reach the high-accuracy band only at short range, so
    their peaks move in, to 350 m and 50 m.
- **Heading changes follow the new stand-off geometry.** The Yorks now hold 350 m instead of 950 m,
  so the bearing to a crossing aircraft sweeps faster and the ring scan changes slot more often.
  The ring scores also reweight per mount, because `h` now depends on each mount's own fraction of
  its range. This part is inferred; it was not ablated.

## 4. Runs

Worktree root `J:\PROG\battlestations-pacific-decompile-cc9-difficulty`, base `d3f7b057b`. Control `build\win32\ctl6\` (switch off), treatment `build\win32\treat6\`:

```
./tools/run_game.ps1 -Exe build\win32\<ctl6|treat6>\bsp_game.exe -Log local\ha_<ctl|trt>_usn04.log -- --frames 4700 --press-start-frame 30 --menu-select USN04 --mission-frames 4500 --mission-frame-seconds 0.05
```

| run | queued_hits | damage | deaths | Lexington | UNIMPLEMENTED calls | log |
|---|---|---|---|---|---|---|
| control | 92 | 14733.9 | 9 | sunk 182.86 s (movieval\|.-2), failure at 183.06 s | 9,607,363 | `local\ha_ctl_usn04.log` |
| treatment | 98 | 14695.8 | 9 | sunk **223.81 s** (York-class02), no failure inside 225 s | **8,456,131** | `local\ha_trt_usn04.log` |

- The per-entity table shows two flips: Lexington's time and credit, and `B5N Kate #6.1`, which dies
  at 220.71 s to Northampton-class01 against 221.01 s to York-class02.
- The unimplemented total falls by 1,151,232. `ShipAiFirepower::hit_accuracy_profile_008386f0` is now
  concrete, and `GameSettings::load_weapon_hit_accuracy` is concrete.

## 5. Decision, the read-only items, and open questions

* **Decision: land it.** The getter and the loader are the image's. Every standoff row is
  reproduced by replaying the scan on the dumped curves. The standoff numbers stand on the host's
  target-curve defect, so they are not the image's until that is fixed.
* **Finding: the host's target curve.** `approach_curve_target` must be the TARGET unit's rating
  of this ship, with the block at `nested+1238h`: `+40h = 0` and `+41h = 0` (009F29CA, 009F29D1),
  and its own armour and weights. It must not be the own unit's rating without the bias. Fixing it
  will move every standoff again. It is a separate packet: the target's gun rows and the
  `nested+1238h` fields.
* **Block +41h, read-only.** The ring path sets `+40h` and `+41h` (009E8171, 009E8178; block at
  `ebp+127Ch`). The own profile clears `+40h` and sets `+41h` (009F2ECB, 009F2ED2). The target
  profile clears both (009F29CA, 009F29D1). The avoidance refresh 009E6240 writes only `+40h`
  (009E634F). So ready rounds count on the ring and own-curve paths and not on the target curve.
  The host's own query sets `use_ready_rounds = 1`, which matches.
* **0085AB50, read-only.** Its "bounds check" is a lazy grow, not a refusal. If the gun's platform
  index `[gun+38Ch]` is at or past the class's platform vector count (`[[gun+3F0h]+538h]+94h`,
  count at `+98h`), it grows the vector to index+1 through 005471B0, then calls 007F6190 on
  `array[index]`. For an authored platform that is exactly the host's use of `gun.arcs`, so there
  is no gap.

## 6. Correction, 2026-09-23 (packet `cc9_target_curve`, `docs/SHIP_AI_TARGET_CURVE.md`)

* **The heading-change rise was not the closer standoffs.** Section 3 said the rise followed the new
  stand-off geometry, and called that inferred. An ablation with the profile loaded and every
  standoff pinned back to its pre-profile figure gives:
  - Northampton-class02: 110, the same as unpinned.
  - York-class01: 80 against 116 unpinned.
  - York-class02: 50 against 38 unpinned.

  So the geometry explains little of it. The per-mount reweighting of the ring scores, and the
  gunnery state the counts are coupled to, carry most of it.
  - The pre-profile figures (12, 57, 4) came from an older main without RNG streams, so the
    comparison across that column is itself only indicative.
* **Section 3's "target curve" description still holds for its binary,** and cc9_target_curve now
  binds the image's construction. On USN04 that yields the same standoffs, because no aircraft's
  rear gun penetrates a ship's armour.
