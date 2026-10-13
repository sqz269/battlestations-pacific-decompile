# CC13 VER 16: flak lethality in USN21, image against host (cc13-ver16)

Packet cc13-ver16 was a read-only verification for cc9, done on 2026-10-13.

- **Method.** Image bytes were read with `bsp.py disasm-raw` (disk image) and constants with
  `bsp.py ghidra bytes`. Class data come from this installation's arcade tables
  (`classtables/arcade/deviceclasses.lua` 2026-05-09 22:37, `bulletclasses.lua` 2026-05-09 23:04; both
  are locally modified). The evidence is one existing log, `J:\PROG\battlestations-pacific-decompile-cc9-ships44\local\s46_e_u21d.log`.
  Scripts: `local/v16_deaths.py`, `local/v16_blasts.py`, `local/v16_dev.py`, `local/v16_mc.py`, `local/v16_mc2.py`.
- **What was not done.** No game runs. No Ghidra writes.
- **Adoption.** `src/game_hosts_gunnery.cpp` is gunnery40's file. Every host change below is an adoption note.
- **Status.** `src/flak_ver_cc13.cpp` holds three pure fragments (D3, D5, D6). They are reconstructed and
  build-tested only. Nothing is wired into a host, and nothing is game-validated.

## 0. Short answer

- **Who kills.** 98 of the 127 cat-6 kills come from the Yamato and Musashi **main battery**, device 259
  "Yamato_1945 18'' 3X". This installation authors it as `LIGHTARTILLERYFLAK`. Its `Bullet[2]` is bullet 102,
  "Yamato 1945 Flak bullet":
  - V0 900, FlyTime 3.5, so `+60h` = 3150 m;
  - MinRange 1000;
  - BlastRange 190;
  - BlastDamage U(250, 350).
- **What one burst does.** One burst deals (U(250, 350) - 5) x 0.5556 = 136-192 HP to every plane within
  190 m. The mean is 164 against a Helldiver's 200 HP. Two bursts kill.
  - The image does the same: ignoreFalloff 1 at `0070C26E`, armour subtracted, party HP inverse 0.5556.
  - So the TTK of 3.6 s (Musashi) and 5.4 s (Yamato) is mod data, not a host defect.
- **The other 29 cat-6 kills** come from destroyer dual-purpose mounts firing "Jap Flak bullet" (49): 35 damage,
  35 m radius, 16.7 HP per burst.
- **Verdict.** Host flak is **not more lethal** than the image's.
  - On the Yamato shell the two are about equal: within about +-6 %, and the image is slightly ahead on the approach.
  - On the dual-purpose 5-inch mounts the host is **under-lethal**. The image fires them 1.2-3x faster (D4), and
    it aims them better at low planes (D6).

## 1. The USN21 census (`local/v16_deaths.py`, `local/v16_blasts.py`)

| killer | kills | kill range (m) | hits / dealt on victims | mean TTK |
| --- | --- | --- | --- | --- |
| Musashi guns 32-34 (dev 259) | 53 | 1410-2971, median 2343 | 144 / 10065 | 3.6 s |
| Yamato guns 89-91 (dev 259) | 45 | 1225-2997, median 2351 | 130 / 8607 | 5.4 s |
| destroyer DP flak (dev 279 / 281 / 49 / 51 / 252) | 29 | 252-1495 | 15-34 per hit | 4-25 s |
| cat 5 (dev 50 / 274, Suzuya, Kumano, Yahagi) | 9 | 913-1808 | 12-17 per hit | 5-11 s |

Gun rows, summed over the mission. Cat 5 and cat 6 together fired 2737 rounds, which equals `flak_rolls`.

| device | guns | rounds | plane-or-other hits | dealt |
| --- | --- | --- | --- | --- |
| 259 Yamato 18'' | 6 | 356 | 187 | 20198 |
| 281 Fubuki 5'' DP | 30 | 957 | 292 | 4634 |
| 279 Fubuki 5'' DP | 15 | 231 | 107 | 1658 |
| 51 Dual Flak (Yamato) | 12 | 382 | 68 | 1082 |
| 252 Dual Flak JP | 8 | 175 | 50 | 763 |
| 49 Dual Flak JP | 18 | 87 | 19 | 317 |
| 50 (cat 5) / 274 (cat 5) | 28 / 4 | 414 / 135 | 73 / 23 | 1139 / 383 |

The blast lines against planes:

| bullet | radius | lines | took/(base - armour) |
| --- | --- | --- | --- |
| 102, via gun row 23 | 190 | 180 | median distance 118 m, max 186 m |
| 49, via gun rows 31 / 49 | 35 | 598 | the ratio sits at 0.557 = 0.5556 |
| 14 | 30 | 23 | the ratio sits at 0.557 = 0.5556 |

- For bullets 49 and 14 the ratio is 0.5556 at every distance, so falloff is ignored, as in the image.
- For bullet 102, `took` is often capped by the health that remains, so its ratio is lower.

The flak bursts: 1548 proximity bursts and 1127 expiry bursts. The `aa bot error` line records 2737 flak rolls,
all at level 1, with a mean angle of 2.392 deg.

Elevation of the cat-6 kills: 105 were above 6 deg, mostly dive bombers at about 1460 m altitude. 15 were in
1-6 deg, and 7 were below 1 deg.

## 2. Verified equal (bytes)

| term | image | host |
| --- | --- | --- |
| lock radius | `0070C50F`-`0070C526`, `0070C5A8`-`0070C62F`: min(90000 (`00CFD508`), (0.5 L + 2 BlastRange)^2); Yamato 300 m, 5-inch 90 m | `:9540`-`:9541` |
| lock candidates and the winner | `0070C553`-`0070C58C` kinds 5 + (0Fh / 0Eh / 0Ch), no side test; the nearest wins (`0070C635`-`0070C63D`) | `:9543`-`:9556` |
| burst distance | `0070C685`-`0070C6E3`: dot(A - prev, dir) + [+290h], floored at 0 | `:9577`-`:9581` |
| locked step and burst | `0070C70F`-`0070C7AF`: L < rem -> rem -= L, else move to prev + dir x rem and `0070C210(1)`; this also runs on the lock tick | `:9588`-`:9602` |
| burst damage and falloff | `0070C226`-`0070C280`: U(+B4h, +B8h) on stream 1, `0084BAD0(..., 1 = ignoreFalloff, source, round)` | `:11237`-`:11238`, `kFlakBlastIgnoreFalloffBound` ON |
| per-plane damage | `004705C0` max(0, f x b - armour), then `0087D730` 1/HP[level] for the player's side (`docs/AA_LETHALITY_AUDIT.md` 10.2; `docs/CC13_AUDIT_MG_AA_LETHALITY.md` 2) | log `hp_inverse=0.5556`, measured ratio 0.557 |
| expiry burst | `0070C3AA`-`0070C3C8` (`docs/CC13_FLAK_FLYTIME_BURST.md`) | `:9507`-`:9524` ON |
| aim, no gravity term | `009031CF` / `00903219` 00901C20 lead, then `0090327B` 008FDAF0 angles; there is no arc solve | `:7039`-`:7119`, pitch 0 |
| error roll | `008FDBE0` row 1 {0.4, 0-2, 2-4.5 deg; 0.4-1.5, 1.5-3 m} | `:7329`-`:7346`. The timing is D2 of `docs/CC13_VER_14.md` |
| cat-6 air minimum range | `00729B90`: kind 6 reads `[desc+74h]+7Ch`, Bullet[2]'s +58h | `:5775`-`:5779` |
| the Bullet table read | `00732C69`-`00732CD6`: a table iterator appends each entry in iteration order (device 274 authors only `[2]`) | `:4004`-`:4013`, the first present index |
| rounds of the Yamato 18'' | reload 18 / delay 0.6 in both records, so no change | the same |

**Precision.** The burst distance (`sc2_x87.py 0070C685 0070C6E8`) is
`f(f(dirY x f(Ay - py) + f(Ax - px) x dirX) + dirZ x f(Az - pz))`, then `f(+290h + that)`. The host sums
x + y + z (`:9577`-`:9579`). At 24-bit precision control the two orders can differ by one float ulp. That is
far below any gameplay effect. `tools/x87_dc_form_scan.py --include-st0` over `009030C0`-`00903420` and
`0070C370`-`0070CAD4` finds five DC-form sites, all in `0070C370`. All five are read here as Capstone prints them:
`0070C4E1`, `0070C518`, `0070C524`, `0070C60D` and `0070C9BA`.

## 3. Divergences

### D3: the flak range gate measures to the lead point

- **Image.** `009031F4` (`lea ecx,[esp+34h]` with two registers pushed) and `0090319A` both hand 00901C20 the
  tick's dt slot `[S+2Ch]` as `outDist`. `0090332C`-`00903344` then requires `[class+58h] < outDist < [class+60h]`.
  Both comparisons are strict, and an unordered result fails.
- **Host.** `src/game_hosts_gunnery.cpp:7648`-`7654` measures `|unit_aim_point - muzzle|`, and `:7703` gates on that.
- **Effect.** This is the same shape as ver14 D1.
  - Inbound, the image opens fire at a current distance of about `max x (1 + vr/V0)`. For the Yamato that is
    3300-3550 m against 3150 m.
  - The image ceases fire at about 1050-1120 m against 1000 m.
  - At the 40-45 m/s closing speed of the level leg, the image's window is about 5 % longer. So the image is
    slightly more lethal inbound and less outbound.
  - No RNG change.
- **Model.** `bsp::flak_bot_range_gate_0090332c`.

### D4: in mode 1 the shot reads Bullet[2]'s Throw, ReloadTime and BarrelDelayTime

- **Image.**
  - `0072D5B0` sets `gun+3F8h = [desc+74h] + mode x 48h` (`0072D600`-`0072D61C`). `008FF240` selects mode 1 for
    a kind-6 gun with an AI-held role slot.
  - Throw comes from `[gun+3F8h]+4` (`007302DF`). The reload is U(+28h, +2Ch) on stream 1
    (`0072D55C`-`0072D579`). The barrel delay is +30h (`00730A05`).
  - The throw seat comes from `[[gun+3F8h]+34h]+8h` (`0073031D`). For Flak that is 10h, so the AAFlakBot
    multiplier applies: 0.6 at level 1.
- **Host.**
  - `:4024`-`:4028` reads reload, bdelay and throw from b1 only. `SecondAmmo` (`:4584`-`:4622`) carries none of them.
  - `:3519` picks the seat from the primary bullet's sub-type. The log's `seats ... flak=549` counts only the cat-5
    rounds. The 2188 cat-6 flak rounds took the ArtillerySubDirector 0.9.

| device | image (Bullet[2]) reload / delay / throw x mul | host (Bullet[1]) |
| --- | --- | --- |
| 259 Yamato 18'' | 18 / 0.6 / 0.07 x 0.6 = 0.042 rad | 18 / 0.6 / 0.020944 x 0.9 = 0.0189 rad |
| 281 Fubuki DP | 4.5 / 0.5 / 0.006 | 6 / 0.4 / 0.009 |
| 279 Fubuki DP | 6 / 0.5 / 0.006 | 7 / 0.4 / 0.009 |
| 49 Dual Flak JP | 5 / 1.5 / 0.006 | 7.5 / 0.4 / 0.009 |
| 51 Dual Flak (Yamato) | 3 / 1.5 / 0.006 | 7.5 / 0.4 / 0.009 |
| 252 Dual Flak JP | 3 / 1.5 / 0.006 | 9 / 0.4 / 0.009 |

- **Effect.**
  - **Throw.** It is swamped by the row-1 aim error (`local/v16_mc.py`). For the Yamato, P(burst within 190 m)
    is 0.827 in the image against 0.885 in the host at 2000 m, and equal at 2400 m and beyond. For the 5-inch the
    difference is not measurable.
  - **Reload.** It dominates. Weighted by the host's rounds, the image fires about 1.7x as many dual-purpose flak
    rounds: 1832 -> about 3150.
  - The reload draw stays on stream 1, so the order of the stream-1 draws changes.

### D5: the proximity fuse arms only after MinRange / V0

- **Image.**
  - `0070C0B0` (the MFlakBullet finalise, vtable `00CFA54C`) stores `+D8h = MinRange / V0`
    (`0070C0BC`-`0070C0C2`).
  - `0070C3DA`-`0070C3E6` skips the lock search unless the flight time is strictly greater than `+D8h`.
  - `0070C3EC`-`0070C400` also requires the scaled step to be non-zero.
  - `docs/FLAK_PROXIMITY_BURST.md` step 3 says "no reader writes +D8h, so 0". **That is wrong.** The value is
    already reconstructed as `WeaponClassFinaliseResult::flak_fuse_time` (`src/bullet_engagement_range.cpp`), but
    the host never reads it.
- **Host.** `:9528`-`:9529` arms at `life > 0`.
- **Effect.** The Yamato shell arms at 1.11 s (1000 m) and the 5-inch at 0.25 s (200 m). Before that, a host round
  can lock onto any plane within 300 m (Yamato) or 90 m (5-inch) of its track; the image's round cannot.
  - USN21 hardly moves: every Yamato kill is at 1225 m or more.
  - A host that adopts it is slightly less lethal.
- **Model.** `bsp::flak_fuse_armed_0070c3da`.

### D6: a negative elevation against a plane is fired flat

- **Image.** After the error add, `00903297`-`009032A1` tests v < 0.
  - For a plane (`IsKindOf(0Fh)` at `009032B3`) it sets v = 0 (`009032BB`).
  - For anything else below -0.02f (`00D7A320`) it sets v = v - (v + 0.02) x 0.5 (`009032D5`-`009032E9`;
    `00D7A2F8` is (double)0.02f and `00D7A280` is 0.5).
- **Host.** `:7326` adds `fe.v`. Nothing clamps it for cat 5 or 6; the `:7349` halving is cat 1 only.
- **Effect.** This applies to low targets only. `local/v16_mc2.py` gives P(burst within the radius) with the row-1
  error and a solved elevation of 0:

| range | 5-inch (35 m) host / image | Yamato (190 m) host / image |
| --- | --- | --- |
| 1500 m | 0.141 / 0.203 | 1.000 / 1.000 |
| 2000 m | 0.080 / 0.140 | 0.911 / 0.957 |
| 2500 m | 0.050 / 0.105 | 0.600 / 0.783 |

  So the host is under-lethal against torpedo planes and low legs, by up to 2x per 5-inch round. No RNG change.
- **Model.** `bsp::flak_bot_vertical_adjust_00903297`.

## 4. Expected image against host, USN21 (s46_e_u21d)

| term | host (measured) | image (expected) |
| --- | --- | --- |
| Yamato burst, per plane | 164 HP mean (136-192) | the same |
| Yamato TTK | 3.6 / 5.4 s | the same per burst; about 5 % more salvos inbound (D3); low planes +5-30 % per round (D6); throw -0-6 % (D4) |
| Yamato kills | 98 | about 95-105 |
| 5-inch burst, per plane | 16.7 HP (12 bursts per Helldiver) | the same |
| DP flak rounds | 1832 | about 3150 (D4) |
| DP + cat-5 flak damage on planes | 1x | about 1.7-2x (D4 x D6 x D3) |
| DP kills (29) + cat 5 (9) | 38 | about 50-65 (rough; kills are shared with the Yamato's) |

**Effect on strikes.** In the image, strike survival is **lower** than in the host, not higher. Releases would fall
slightly. The main losses (Sanshiki bursts at 2-3 km) are equal in both.

**Order to adopt, each behind its own OFF switch and pair:**
1. D4 (reload / delay / throw / seat from Bullet[2]); this is the biggest.
2. D6.
3. D3, shared with ver14 D1.
4. D5, which makes the host slightly less lethal.

## Handoff

- **gunnery40.** Adopt D3-D6 as above. D2 (`docs/CC13_VER_14.md`) stays separate.
- **Docs.** `docs/FLAK_PROXIMITY_BURST.md` step 3 (+D8h) needs the D5 correction. `docs/AA_TARGETING.md` line 72
  ("kind 6's second ammunition is not loaded") is stale since `kDualPurposeSecondAmmoBound`.
- **Not done.**
  - The AAFlakBot target pick, through the shared 2 s assignment `00864FE0`, was not re-read. It is covered by
    `docs/AA_TARGETING.md`.
  - The `[gunclass+95h]` lead branch at `00903184` (the shooter velocity) was not re-read.
  - The barrel reset that `0072D5B0(1, 0)` makes on a mode change (a turret switching between a ship target and a
    plane) was not quantified.
