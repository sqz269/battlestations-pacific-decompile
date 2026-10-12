# Scripted wins table

Kept by the SEA/physics lane (cc9-physics3 onwards). After each batch of gameplay flips lands on main, the seven win
order files are re-run on a fresh export of current main in the reference form with `--stop-after-end 40` and
`BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1` (launcher: J:\PROG\battlestations-pacific-decompile-cc9-physics1\local\p3_runs.ps1).
A lost win is re-planned, preferring plans with margin over knife-edges. "Guard" is the exec guard's
`bsp: refused a mission script's process launch: sus_prog.exe` line, which every completion must log.

Order files (absolute paths; o-files live in the worker trees' local\ directories):

| Row | Order file |
|---|---|
| USN01 | cc9-physics1\local\p3_u1_z4.txt (since main cd761bb52; before: p3_u1_o.txt, l50_u1_u2.txt) |
| USN02 | cc9-ships36\local\s36_u2_p7.txt |
| LOMP10 | cc9-physics1\local\p3_l10_b.txt (since 2026-10-12; was cc9-ships37\local\s37_l10_p2.txt) |
| USN12 | cc9-ships38\local\s38_u12_p2.txt |
| ESMP08 | cc9-lua46\local\l46_e8_orders2.txt |
| LOMP06 | cc9-lua50\local\l50_l6_q6.txt |
| BSM01 | cc9-lua50\local\l50_b1_a.txt (the win is EndMission) |

## 2026-10-12, build fe01ecf32 (origin/main db5ab3c4e + the live-sea flip; main landed the flip as ecb1c55d1 on a later base), SHA-256 B22D402CE8ED

| Row | Result | Guard |
|---|---|---|
| USN01 p3_u1_o | completed 618.72 s | yes |
| USN02 s36_u2_p7 | completed 203.01 s | yes |
| LOMP10 p3_l10_b | completed 978.43 s | yes |
| USN12 s38_u12_p2 | completed 1554.69 s | yes |
| ESMP08 l46_e8_orders2 | completed 3714.45 s | yes |
| LOMP06 l50_l6_q6 | completed 535.24 s | yes |
| BSM01 l50_b1_a | EndMission 767.94 s | yes |

Logs: local\p3w_on_<row>.log and local\p3r_on_u1o.log / p3r_on_l10b.log in the cc9-physics1 tree;
docs/OCEAN_WAVE_FIELD.md 18 and 20 have the per-entity reasons.
## 2026-10-12, main 8e2e2fba9 (after gunnery37's dive yaw, move-start draws, draw pins, tail gunner), SHA-256 095812FC1494

Logs local\p3k1_on_<row>.log (local\p3_runs.ps1 -Set keep -Tag k1; summary local\p3_keep.py k1).

| Row | Result | Guard |
|---|---|---|
| USN01 p3_u1_o | **no completion** (22000 frames) | no |
| USN02 s36_u2_p7 | completed 203.01 s | yes |
| LOMP10 p3_l10_b | completed 1132.43 s | yes |
| USN12 s38_u12_p2 | completed 1554.69 s | yes |
| ESMP08 l46_e8_orders2 | completed 3446.89 s | yes |
| LOMP06 l50_l6_q6 | completed 535.24 s | yes |
| BSM01 l50_b1_a | EndMission 767.94 s | yes |

USN01: Convoy3, 5, 2 and 1 sink (181.61 to 194.86 s); Convoy4 and Convoy6 survive. The strike has no margin by
construction: the three TBD squadrons carry nine torpedoes, a transport has 2400 health and a torpedo takes about
1190, and this run landed six (two each on Convoy3 and Convoy5, one each on Convoy2 and Convoy1). The rest of the kills
come from gunfire on damaged ships. Any plane-AI change re-rolls which ship survives. Re-plan held until gunnery37's
torpedo-aim and roll-arm flips land (lead, 2026-10-12).
## 2026-10-12, main cd761bb52 (adds gunnery37's kFlakFlyTimeBurstBound and kFlakBlastIgnoreFalloffBound, Lua hit target as the squadron), SHA-256 16A5B0F3771F

Logs local\p3k2_on_<row>.log.

| Row | Result | Guard |
|---|---|---|
| USN01 p3_u1_o | no completion | - |
| USN01 **p3_u1_z4** (re-plan) | completed 455.41 s | yes |
| USN02 s36_u2_p7 | completed 203.01 s | yes |
| LOMP10 p3_l10_b | completed 1259.85 s | yes |
| USN12 s38_u12_p2 | completed 1554.69 s | yes |
| ESMP08 l46_e8_orders2 | **no completion** (90000 frames): Cummings survives | - |
| LOMP06 l50_l6_q6 | completed 535.24 s | yes |
| BSM01 l50_b1_a | EndMission 767.94 s | yes |

**USN01 re-plan (p3_u1_z4), with margin.** The strike's torpedo log (`torpedo from ... ordered`) shows each TBD
squadron putting all three torpedoes into one ship: ConTBD2 into Convoy3 (drops at frames 3410/3460/3470), ConTBD3
into Convoy5 (3490/3530/3600), so Convoy4 and Convoy6 get none. The plan spreads them: `3412 select ConTBD2`,
`3413 target ConTBD2 Convoy4`, `3495 select ConTBD3`, `3496 target ConTBD3 Convoy6`, and moves l50_u1_u2's
`3650 target ConTBD1 Convoy1` to `3690 select ConTBD1` / `3691 target ConTBD1 Convoy1`, so that ConTBD1|.-3's torpedo
still goes to Convoy2 first. All six transports sink by 279 s. Neighbours also win: the same plan with the TBD1
retarget at 3720 (p3_u1_z5) completes 460.36 s, and z4 plus SBD2/SBD3 retargets at 3730/3740 (p3_u1_z6) completes
490.05 s. Splits that leave the TBD1 retarget at 3650 (p3_u1_q1..q3) lose Convoy2, because TBD1|.-3 now drops after
3650. Late convoy kills do not count: in p3_u1_x3 / y5 the cruisers sink the last three transports at about 900 s
and the mission still does not complete. The phase-2 check is gated on Mission.MainAttackHere, and the squadrons
have ditched by then.
Orders in the file must stay in frame order (the generator sorts them).