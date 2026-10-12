# Scripted wins table

Kept by the SEA/physics lane (cc9-physics3 onwards). After each batch of gameplay flips lands on main, the seven win
order files are re-run on a fresh export of current main in the reference form with `--stop-after-end 40` and
`BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1` (launcher: J:\PROG\battlestations-pacific-decompile-cc9-physics1\local\p3_runs.ps1).
A lost win is re-planned, preferring plans with margin over knife-edges. "Guard" is the exec guard's
`bsp: refused a mission script's process launch: sus_prog.exe` line, which every completion must log.

Order files (absolute paths; o-files live in the worker trees' local\ directories):

| Row | Order file |
|---|---|
| USN01 | cc9-physics1\local\p3_u1_o.txt (since 2026-10-12; was cc9-lua50\local\l50_u1_u2.txt) |
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