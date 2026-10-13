# Scripted wins table

Kept by the SEA/physics lane (cc9-physics3 onwards). After each batch of gameplay flips lands on main, the seven win
order files are re-run on a fresh export of current main in the reference form with `--stop-after-end 40` and
`BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1` (launcher: J:\PROG\battlestations-pacific-decompile-cc9-physics1\local\p3_runs.ps1).
A lost win is re-planned, preferring plans with margin over knife-edges. "Guard" is the exec guard's
`bsp: refused a mission script's process launch: sus_prog.exe` line, which every completion must log.

Win order files live in config/win_orders; local\ copies are scratch. (Committed 2026-10-12 after a worktree
cleanup removed cc9-ships36, cc9-ships38 and cc9-lua46; USN02, USN12, LOMP06 and BSM01 are the
cc9-gunnery36\local\g38_ord_*.txt copies that reference AH reproduced every win time from; USN01, LOMP10, LOMP06
and BSM01 are byte-identical to the files the last wins used.)

| Row | Order file | Was |
|---|---|---|
| USN01 | config/win_orders/usn01_p3_u1_z4.txt | cc9-physics1\local\p3_u1_z4.txt (since main cd761bb52; before: p3_u1_o.txt, l50_u1_u2.txt) |
| USN02 | config/win_orders/usn02_s36_u2_p7.txt | cc9-ships36\local\s36_u2_p7.txt |
| LOMP10 | config/win_orders/lomp10_p3_l10_b.txt | cc9-physics1\local\p3_l10_b.txt (was cc9-ships37\local\s37_l10_p2.txt) |
| USN12 | config/win_orders/usn12_s38_u12_p2.txt | cc9-ships38\local\s38_u12_p2.txt |
| ESMP08 | not committed (2.3 MB, not a win now): cc9-gunnery36\local\g38_ord_e8.txt | cc9-lua46\local\l46_e8_orders2.txt |
| LOMP06 | config/win_orders/lomp06_l50_l6_q6.txt | cc9-lua50\local\l50_l6_q6.txt |
| BSM01 | config/win_orders/bsm01_l50_b1_a.txt (the win is EndMission) | cc9-lua50\local\l50_b1_a.txt |
| USN15 | config/win_orders/usn15_s45_u15_l2.txt (90000 frames; the win is EndMission) | cc9-ships44\local\s45_u15_l2.txt (replaces s44_u15_h of SHIP_AI 219) |

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
and the mission still does not complete within 1600 s. Why is unexplained: on 355b60167 (below) z4 completes 178 s after a
late Convoy6 kill at 836 s, so late kills can count.
Orders in the file must stay in frame order (the generator sorts them).
**ESMP08 on cd761bb52, not yet re-planned.** The win is `getn(alive USNFleet) == 0` (08_engano.lua). Torpedoes,
not bombs, sink the fleet. B6N Jill is class 163 and B5N Kate 162 (this installation's vehicleclasses.lua);
D4Y Judy 159 releases its bombs 1.3 to 1.7 km off. The last two or three US ships die only to torpedoes before
about 2400 s. After that, Jill squadrons launched at the survivor (Cummings, stopped `state=stop`) start about
27 km away, cruise at the commanded 1450 m (`torpedo <sqn> glide census ... range=27749`), close at 40 to 66 m/s,
and are shot down by the target's flak at about 1440 m before their descent. Logs: local\p3k2_on_e8.log
(Chitose_sqn55, Zuiho_sqn50). So the winning run (k1, 3446.89 s) depended on Cummings dying at 2814.57 s to a
torpedo strike launched while it was still in reach. Plans that try Cummings first in every launch group from frame
24000 (local\p3_e8_p24.txt) or 36000 (p3_e8_p36.txt) sink Cummings (1586 s / 2236 s) and then leave San Diego +
Woodworth, or Woodworth, for the same reason. Extending the launch cycle (p3_e8_e1.txt) adds nothing. The late-strike
failure is plane-AI behaviour (cruise altitude, descent, torpedo approach), which gunnery37's pending batch
(aimglide/fly-over, torpedo aim) changes. Re-check ESMP08 after that batch, before re-planning it.

## Keeper notes (cc9-physics3 handoff, 2026-10-12, at about 70% context)

- Tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 (branch agent/cc9-physics1). Lease cc9_physics3_wins
  covers docs/OCEAN_WAVE_FIELD.md and this file.
- To re-check: `python tools/pair_export.py --commit <origin/main sha> --out local\p3_on` (an incremental rebuild of the
  same export directory), then `powershell -File local\p3_runs.ps1 -Set keep -Sides on -Tag k<N>` (in the background;
  it waits on all seven), then `python local\p3_keep.py k<N>`. Update the order-file table above when a re-plan
  replaces a file.
- Helpers in local\: p3_ends.py (end, guard, convoy deaths over a glob), p3_deathdiff.py (one-sided deaths),
  p3_runs.ps1 sets `one` (-Key -Mission -Mf -Pat <order file>), `sweepw` (-Pat <prefix>, USN01 at 32000 frames),
  p3_e8_prio.py (ESMP08 target-priority generator).
- USN01 re-planning: read each `torpedo from <plane> ... ordered <ship>` line and map drops to frames through the
  preceding `ship ai step` line. A `target` order needs the squadron selected (`select` first); `select` changes
  the controlled unit, so re-select before any later `target` of another squadron. Keep files frame-sorted.
- Open: ESMP08 (above), after gunnery37's next batch. The state-6 seaplane law (OCEAN_WAVE_FIELD 21) and the
  scaled-fort pruning (DYN_COLLISION_PASS) stay unbound by the lead's decision.
## 2026-10-12, main 355b60167 (contains gunnery37's batch 088103ce0 / ba92657b9 and the audit9 dive fly-above trio), SHA-256 4EF43E5C88B4

Logs local\p3k3_on_<row>.log.

| Row | Result | Guard |
|---|---|---|
| USN01 p3_u1_z4 | completed 1014.63 s (Convoy1 at 330 s, Convoy6 at 836 s, so less margin than on cd761bb52) | yes |
| USN02 s36_u2_p7 | completed 203.01 s | yes |
| LOMP10 p3_l10_b | completed 1002.18 s | yes |
| USN12 s38_u12_p2 | completed 1539.83 s | yes |
| ESMP08 l46_e8_orders2 | **no completion** (90000 frames) | - |
| LOMP06 l50_l6_q6 | completed 535.24 s | yes |
| BSM01 l50_b1_a | EndMission 767.94 s | yes |

ESMP08 against the k1 win: Cummings, Dunlap, Grayson and Woodworth now survive (in k1 they died between 1353 and
2814 s, all to torpedoes). The batch did not restore it. Next keeper: re-plan ESMP08 from its torpedo budget (the
mechanism is in the cd761bb52 section above).
## 2026-10-12, keeper notes (cc9-physics4)

- **New row USN15 (Surigao)**, reported by cc9-ships45: `s45_u15_l2.txt`, 90000 frames, reference form,
  EndMission 2775.58 s on main 1de67cfb9 with the guard (log cc9-ships44\local\s45_v_l2.log); it also wins with
  `BSP_FRAME_JITTER 20,3` at 2671.53 s. Replaces s44_u15_h.
- **gunnery38's AH batch** (reported): the six wins hold at identical times. ESMP08 stays lost: late Jill/Kate
  squadrons loiter at 1450 m over a dead target (Washington) and the flak kills them; with the flak pair OFF it
  completes at 2594.66 s. The flak stays ON; gunnery38 is binding the unhosted command retest 009F8160.
- **cc9-physics4's flips, checked on their pairs:** `kPlaneWaterSurfaceLawBound` (25d13aa63) and
  `kGunAimDegreePrecisionBound` (f435022e8) leave USN01 (1014.63 s) and USN12 (1539.83 s) at identical times;
  ESMP08 is unchanged (lost).
- **Retracted: "USN12 lost on 8b0e7bfce".** That run (local\p4l_off_u12.log) logged `helm orders refused: cannot open` its order file: cc9-ships38 had been removed minutes before. USN12 ran with no orders; it says nothing about main.
- **kPlaneLeaderTurnRollBound flipped ON** (cc9-physics5, 2026-10-13, ruling (a)): faithful, loses USN01 `p3_u1_z4` (deaths 90 -> 68 on the 8b0e7bfce pair). USN01 stays a win row pending the re-plan on current main; until then treat `p3_u1_z4` as expected to fail.
- **New row USNOS** (reported by cc9-ships45 through the lead, 2026-10-13): `config/win_orders/usnos_s45_os_k.txt` (source cc9-ships44\local\s45_os_k.txt), 200000 frames at 0.05 s, reference env, `--stop-after-end 40`; mission completed at 7332.83 s on 1de67cfb9 with the sus_prog.exe guard line logged (SHIP_AI_OPEN_ITEMS 221, 975b45791). Knife-edge on one transport (TroopTrans6), not jitter-checked. **Pending re-check on main with kamikaze/drop/sweep/n1 ON (ships45).**
