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
| USN01 | config/win_orders/usn01_p8_u1_i.txt (since main c9a8b1344; the newest file per mission is the row, see the notes below) | before: usn01_p8_u1_c, usn01_p7_u1_a, p6_u1_k6, p6_u1_g, p5_u1_e, p3_u1_z4 |
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
- **USN01 re-planned (cc9-physics5, 2026-10-13): `config/win_orders/usn01_p5_u1_e.txt`**, replaces `p3_u1_z4`. Tree agent/cc9-physics1 at origin/main 3eac809d6 (leader turn roll, O1, avoid pick, gun burst, kamikaze/drop/sweep/n1 ON), bsp_game.exe SHA-256 A449FC8E71B1, 26000 mission frames at 0.05 s, reference env, `--stop-after-end 40`: **completed 1183.08 s** with the sus_prog.exe guard line (`local\p5r_e26.log`; the 36000-frame run `p5r_e` ends identically). The first divergence from z4: `ConTBD1|.-3` flies into Convoy3 at 179.71 s (plane crash sweep, 150.6 dealt), so Convoy3 survived; `3306 target ConTBD1 Convoy3` gives Convoy3 to ConTBD1|.-3's guns at 188.71 s, and Convoy2/1/6 fall to Northampton and SaltLakeCity by 934.20 s. z4's Enterprise Wildcat launches (frames 4400+) stop phase 3 (escort to NavEscort1) from completing by 1800 s on main, so they are removed (`p5_u1_d` with them: no completion; `p5_u1_f`, launches only after 22000: 1183.08 s). **Knife-edge:** with `BSP_FRAME_JITTER 20,3` it fails (Convoy1 and Convoy6 survive; `p5r_ej`, `p5r_fj`).
- **LOMP10 `p3_l10_b`** holds on the same tree: completed 1158.10 s with the guard (`local\p5wf_off_l10.log`).
- **New row JM08** (reported by cc9-ships45 through the lead, 2026-10-13): `config/win_orders/jm08_s45_j8_m10.txt` (source cc9-ships44\local\s45_j8_m10.txt), 72000 frames at 0.05 s, reference env, `--stop-after-end 40`; completed 2645.96 s on 1de67cfb9 (3062.46 s with land-mesh ON), guard line logged. Fails under `BSP_FRAME_JITTER 20,3` (the phase-2 B-17 interception). **USNOS and JM08: pending re-check on current main (ships45).**
- **USN12 `s38_u12_p2`** completes at 1534.87 s on main (cc9-lua54's pair; 1539.83 s before n1 and the sweep flips).
- **USN01 `usn01_p5_u1_e` LOST again on main 64675c644** (cc9-physics5, tree e97058a57's base; `local\p5so_on_u1e.log`, 26000 frames): Convoy5/3/4 die as before, Convoy1 now at 282.75 s (ConTBD1, cat 0), and Convoy2 and Convoy6 survive (on 3eac809d6 Northampton and SaltLakeCity sank them at 888/934 s). Main commits since: gunnery38's dead-target retest ON (395e1fec0, 518193e10), lua52 and ships44 merges (630f7ae95, 0b274d016, d69822767). Re-plan needed (handoff OCEAN_WAVE_FIELD 24).
- **Keeper updates, current main (reported through the lead, 2026-10-13):** JM08 `jm08_s45_j8_m10` completes at 3528.42 s on bd3b0c333 (ships45, guard logged, cc9-ships44\local\s45_rc_j8m10.log); USNOS `k` re-check still running. gunnery39's reference on 02c6b592f: USN02 203.01 s, LOMP06 535.24 s, USN12 1534.87 s, LOMP10 1158.10 s (was 1002.18), USN15 2641.80 s. **BSM01 `l50_b1_a` LOST** (luaMissionFailed 1521.36 s; HenryPT never rams MiniSub); it won at 767.94 s on 518193e10, and gunnery39 is bisecting the five switches since.

- **JM08 `jm08_s45_j8_m10` LOST by the wing fold flip** (cc9-physics6, 2026-10-13; docs/PLANE_INPUT_CHECKS_CC9.md, the
  JM08 pair): on origin/main 168cb72a9 it completes at 3528.42 s with the switches OFF (guard logged,
  `local\p6wf_off_j8.log`) and does not complete in 72000 frames with `kPlaneWingFoldElevatorBound` and
  `kPlaneWingSpreadTakeoffBound` ON (`local\p6wf_on_j8.log`): every Hosho Kate launch waits 3.0 s for its wings in the
  takeoff prepare (009CDEED). Faithful, flipped ON; JM08 needs a re-plan.

- **New row JM06** (reported by cc9-ships45 through the lead, 2026-10-13): `config/win_orders/jm06_s45_j6_k.txt`,
  100000 frames at 0.05 s, reference env, `--stop-after-end 40`; completed at 2880.63 s on bd3b0c333
  (`ijn06.obj_missioncompleted`), guard line logged (SHIP_AI 225). Not jitter-checked.
- **USNOS `usnos_s45_os_k` does NOT complete on current main** (reported through the lead, 2026-10-13): every
  FinalTrgs unit dies, but ReinDead / `HideScoreDisplay(4,0)` never fires and the mission stays in phase 2.
  cc9-ships46 is investigating with Lua error logging.
- **JM08 re-plan assigned to cc9-physics6** (2026-10-13). On 6a7062611 (wing fold ON) `m10` sinks only 4 of 11
  landing ships: Grayson kills Isokaze (655 s) and Fubuki (682 s) before they reach the transports (in the win
  Fubuki sank LSM 01, TT02 and TT04 by 720 s). Hosho still dies at 509 s (Hawaii) either way.
- **JM08 re-planned (cc9-physics6, 2026-10-13): `config/win_orders/jm08_p6_j8_f.txt`**, replaces `jm08_s45_j8_m10`
  (kept). Tree agent/cc9-physics1 at 974089548's merge (origin/main with the wing fold ON), bsp_game.exe SHA-256
  6C400EF57F47, **110000 mission frames** at 0.05 s, reference env, `--stop-after-end 40`: **completed 2974.03 s**,
  guard line logged (`local\p6j8_f.log`). The plan is m10 (SHIP_AI 223) with its late generator part regenerated to
  110000 frames (`local\p6_j8gen.py` = s45_j8gen.py, same arguments with `--end 110000 --air-until 110000`, spliced
  at 24000; the splice at 72000 reproduces m10 byte for byte) and the airfield's launch groups between frames 6000
  and 24000 given Grayson, then Helena, as first targets (`local\p6_prepend.py`). Grayson had sunk Isokaze and
  Fubuki before they reached the transports; with it engaged the destroyers survive to 661/714 s and the airfield
  finishes the landing ships by 2590 s, then the B-17 interception completes at 2974 s. Variants: Hosho at Grayson
  (a, and a+airfield c): HQ destroyed 1706.84 s; airfield at Grayson only to frame 14000 (b, 72000 frames): 10 of
  11 landing ships, no end.
- **JM08 `jm08_p6_j8_f` holds under `BSP_FRAME_JITTER 20,3`** (cc9-physics6, `local\p6j8_fj.log`, same exe): completed 2697.31 s, guard logged. Variant d (Grayson only to frame 14000, 110000 frames): USTroopTransport 01 survives, no end. Not a knife-edge on this evidence.
- **Reference AI times on current main** (gunnery39, reported through the lead, 2026-10-13): USN02 203.01 s,
  LOMP06 535.24 s, USN12 1534.87 s, LOMP10 1158.10 s, USN15 2641.80 s.
- **BSM01 `l50_b1_a`: plan casualty, not a defect** (gunnery39, df1343735): the loss needs both
  `kPlaneCrashSweepBound` and `kLandStructureMeshHitTestBound`; crashing planes hit Pearl's buildings on their meshes
  from 133 s, the battle shifts, and HenryPT never rams MiniSub (the win's phase-3 end was at 437.77 s). Re-plan
  queued with cc9-physics6, base `config/win_orders/bsm01_l50_b1_a.txt`.
- **BSM01 re-planned (cc9-physics6, 2026-10-13): `config/win_orders/bsm01_p6_b1_a.txt`**, replaces `bsm01_l50_b1_a`
  (kept). Tree agent/cc9-physics1 at the merge of origin/main 6846c0620, exe A351D9B2F093, 40000 frames, reference env,
  `--stop-after-end 40`: **EndMission 752.94 s**, guard logged, MiniSub sunk by HenryPT's depth charge at 424.07 s
  (`local\p6b1_a.log`). Only change: the last takehelm leg at frame 7000 instead of 7250. On current main HenryPT is
  further south by frame 7000 than in the win (1921,-2955 vs 2027,-2687); with the leg at 7250 it turned west at
  z = -3150 and stalled at (1624,-3170) from 395 s with throttle 0.5 and no speed until it died at 1519.8 s
  (`local\p6b1_base.log`). Variant b (waypoint 6600 at z -2990 instead of -3084): MiniSub survives.
- **BSM01 `bsm01_p6_b1_a` checks** (cc9-physics6): on 8c0dc221a (exe 751CFE0BE15A, warning walk ON) EndMission 752.94 s again (`local\p6b1_a2.log`); with `BSP_FRAME_JITTER 20,3` EndMission 764.94 s, MiniSub at 436.17 s, guard logged (`local\p6b1_aj.log`).
- **JM06 row is now `config/win_orders/jm06_s46_j6_m.txt`** (cc9-ships46, SHIP_AI 227, reported through the lead, 2026-10-13): completes at 2880.63 s, and at 3421.16 s under `BSP_FRAME_JITTER 20,3` (seed 3), guard logged both times. `jm06_s45_j6_k` (kept) fails under jitter.
- **USN01 re-planned (cc9-physics6, 2026-10-13): `config/win_orders/usn01_p6_u1_g.txt`**, replaces `usn01_p5_u1_e`
  (kept). Tree agent/cc9-physics1 at the merge of origin/main 09a095e45's base (gunnery39's verdict 72e64feac in),
  exe 629B244E3C28, 26000 frames, reference env, `--stop-after-end 40`: **completed 1197.94 s**, guard logged
  (`local\p6u1_g.log`). Plan e plus `3700 select ConSBD3` / `3701 target ConSBD3 Convoy2` (both applied) after
  the retest ends ConSBD3's command on Convoy5's death (183.76 s). ConSBD3 still releases nothing (every aimglide
  blocked at the ceiling); the kills are Convoy5/3/4 to torpedoes by 189 s and Convoy2/1/6 to Northampton at 888,
  920 and 941 s, as on 3eac809d6. Plan e on the same exe: no completion (`local\p6u1_e.log`, Convoy2 and Convoy6
  survive); ConSBD3 at Convoy6 (h): no completion. **Knife-edge:** with `BSP_FRAME_JITTER 20,3` it fails (Convoy1 and
  Convoy6 survive, `local\p6u1_gj.log`), like e. Northampton and SaltLakeCity cannot be ordered directly:
  `select Northampton` is refused by 00645060 (`local\p6u1_i.log`).
- **ESMP08 re-plan (cc9-physics6, 2026-10-13), pending the host squadron cap.** Plan `local\p6_e8_Br3L.txt` in the
  cc9-physics1 tree (4.5 MB; not committed until it completes on main): g38_ord_e8's launch lines without its order
  lines, every slot's target list re-ordered big ships first (South Dakota, Washington, Iowa, then Mobile, Santa Fe,
  San Diego, Salt Lake City, then the destroyers; `local\p6_e8_alloc.py`), the 600-frame launch cycle extended to
  140000 frames (`local\p6_e8_extend.py`), and three rounds of `g38_e8_retarget.py` re-targets. On a scratch export
  of 09a095e45 with `kSquadronCreateLimit` 64 -> 512 (src/game_hosts_script_orders.cpp, exe 5A90E0943AE7), 140000
  frames, reference env: **completed 2619.43 s**, guard logged (`local\p6e8_Br3Lc.log`), and **3283.38 s under
  `BSP_FRAME_JITTER 20,3`** (`local\p6e8_Br3Lcj.log`). On the current exe it cannot complete: the host's 64-squadron
  ceiling (a safety net counting every squadron ever created) stops launches near frame 45300, and the plan needs
  about 70-80 squadrons. Other allocations (90000 frames, current exe): baseline 5 of 15 sunk, destroyers first 1,
  mixed 6, big ships first 4 (9 with one re-target round, 11 with two and 140000 frames).
- **New row IJN18** (cc9-ships46, SHIP_AI 228, reported through the lead, 2026-10-13):
  `config/win_orders/ijn18_s46_i18_a.txt`, 60000+ frames: EndMission 2208.58 s, and 2370.14 s under
  `BSP_FRAME_JITTER 20,3`, guard logged both.
- **New row PRCP01 (Santa Cruz)** (cc9-lua55/56, SQUADRON_LAND_TASK 5go, reported through the lead): order file
  `config/win_orders/prcp01_l55_p01_c.txt` (source cc9-lua52\local\l55_p01_c.txt), 30000 frames, completed 733.59 s on
  main 458f9efbe (kRackDeviceReloadBound + kDiveBreakOffReloadFeedBound ON), guard logged. Not jitter-checked.
- **JM08 `jm08_p6_j8_f` with device reload ON** (lua56, cc9-lua52\local\l56_M_j8.log): completed 2141.01 s, 29
  reloads, guard logged.
- **Navigator group ON (428f05881, cc13-ver7 D2/D3/D5/D6 + back-off countdown)**, reported through the lead: USN15
  2730.99 s (was 2641.80), JM08 f 2693.50 s (was 2141.01), LOMP10 1006.93 s (was 1158.10), IJN18 2317.59 s (was
  2208.58), guard logged. USN02, LOMP06, BSM01, USN01, PRCP01 and USN12 not re-run on it yet.
- **ESMP08 is a win again: `config/win_orders/esmp08_p6_e8_br3l.txt`** (cc9-physics6, 2026-10-13), the Br3L plan above, on agent/cc9-physics1 at d2bb05c0e (origin/main with `kAirOpsSquadronCeilingRaisedBound` ON, cddc07577), exe D764DF0C5DBE, 140000 frames, reference env: **completed 2619.43 s** (`local\p6e8_m3.log`), **3283.38 s under `BSP_FRAME_JITTER 20,3`** (`local\p6e8_m3j.log`), guard logged both; Br2L completes at 3788.77 s (`local\p6e8_m2.log`).
- **Times on main before V1/R1** (cc9-physics6, scratch export `local\p6_rf` of 9d9854241, both switches OFF):
  USN12 `s38_u12_p2` 1524.96 s, LOMP10 `p3_l10_b` 1006.93 s, USN01 `usn01_p6_u1_g` 1118.67 s, guard logged.
- **V1 + R1 ON (57fce131f, CC13_GPR_48 9)**: USN12 still completes at 1524.96 s, LOMP10 977.48 s. R1 loses USN01 g,
  so USN01 is re-planned: **`config/win_orders/usn01_p6_u1_k6.txt`** (g with `3306 target ConTBD1 Convoy6`)
  completes at 514.80 s on 57fce131f (exe 344896BA8503, 26000 frames, guard logged, `local\p6m7_k6.log`). All six
  convoys sink to torpedoes by 245 s. **Knife-edge:** it fails under jitter 20,3 and 20,7, and with V1/R1 OFF.
- **JM06 is a win again** (cc9-ships47, SHIP_AI 231, reported through the lead): `config/win_orders/jm06_s47_j6_n1.txt`,
  120000 frames: 4108.77 s, 2667.48 s under jitter 20,3, 5901.74 s under 20,5 (tight), guard logged, on main with
  kShipSubmergedThrottleBound and kShipRightingImageConstantsBound ON.
- **PRCP01 is a win again** (reported through the lead): `prcp01_l55_p01_c` completes at 720.25 s on main 2c0421b51
  with cc13-fix5's eight natives ON, guard logged.- **Re-time on main ad68156b1** (cc9-physics7, 2026-10-13). Export `local\p7_m`, exe C277B01AE8BE, reference env,
  `--stop-after-end 40`, logs `local\p7w1_<row>.log` (launcher `local\p7_wins.ps1`, summary `local\p7_keep.py w1`):

  | Row | Frames | Result | Guard |
  |---|---|---|---|
  | USN02 `s36_u2_p7` | 16000 | completed 203.01 s | yes |
  | LOMP06 `l50_l6_q6` | 16000 | completed 372.83 s (was 535.24) | yes |
  | USN12 `s38_u12_p2` | 36000 | completed 1534.87 s | yes |
  | LOMP10 `p3_l10_b` | 36000 | completed 1244.64 s (977.48 on V1/R1) | yes |
  | USN15 `s45_u15_l2` | 90000 | EndMission 2730.99 s | yes |
  | USN01 `p6_u1_k6` | 26000 | completed 1118.67 s (was 514.80; Northampton's gunfire finishes it, not the strike) | yes |
  | BSM01 `p6_b1_a` | 40000 | EndMission 764.94 s | yes |
  | JM08 `p6_j8_f` | 110000 | completed 2318.84 s | yes |
  | JM06 `s47_j6_n1` | 120000 | completed 2733.49 s | yes |
  | IJN18 `s46_i18_a` | 64000 | EndMission 2193.71 s | yes |
  | PRCP01 `l55_p01_c` | 30000 | **no end** | - |
  | ESMP08 `p6_e8_br3l` | 140000 | **no end**: Cummings survives; the other 14 die by 6475 s (in the win, all 15 by 2615 s) | - |

  - PRCP01: cc9-lua52's `l57_d0_p01w.log` has the same size as this run, and its `l57_d1` (lua57's think-delay fix)
    completes at 865.06 s; the lead reports that fix is now on main, so PRCP01 is a win again there (not re-run here).
  - ESMP08: the kill rate halves from the first big-ship deaths on (Washington 846 -> 1812 s, Iowa 1265 -> 2096 s).
    Re-plan after the plane-flight flips (CC13_GPR_48 10).
- **The four plane-flight switches ON together** (cc9-physics7, scratch `local\p7_all` = ad68156b1 plus D2, R2, D4 and
  D1 ON, exe 24B400E520EF; docs/CC13_GPR_48.md 10; logs `local\p7a1_<row>.log`). The four switches are committed ON by
  cc9-physics7 in the commit that adds this note. Results: USN02 203.01 s, LOMP06 372.83 s, USN12 1534.87 s, LOMP10 985.08 s,
  USN15 EndMission 2730.99 s, BSM01 EndMission 761.94 s, JM08 f 1603.74 s, JM06 n1 2697.46 s, IJN18 EndMission
  2124.35 s, PRCP01 586.53 s, guard logged on all. **USN01 k6 lost** (Convoy1 survives). ESMP08 was stopped at the
  pause, before it ended.
  - USN01 re-plan for that build: **`config/win_orders/usn01_p7_u1_a.txt`**, which is k6 with `3413 target ConTBD2
    Convoy1` (Convoy4 now dies first). It completes at 366.33 s (`local\p7u1_a.log`). Jitter status (20,3 and 20,7): pending; the
    checks were stopped at the pause.
  - Variants b-f (ConSBD3 or ConSBD1/2 at Convoy1, ConTBD1's retarget at 3600) did not complete.
- **Re-time on main f83e91891** (cc9-physics8, 2026-10-14). Export `local\p7_m` re-exported at f83e91891, exe
  04087FFFB22D, reference env, `--stop-after-end 40`, logs `local\p8w1_<row>.log` with `.manifest.json`
  (launcher `local\p8_wins.ps1`, summary `local\p8_keep.py w1`, death diff `local\p8_deaths.py <a> <b>`):

  | Row | Frames | Result | Guard |
  |---|---|---|---|
  | USN02 `s36_u2_p7` | 16000 | completed 200.06 s (was 203.01) | yes |
  | LOMP06 `l50_l6_q6` | 16000 | completed 189.86 s (was 372.83) | yes |
  | USN12 `s38_u12_p2` | 36000 | completed 1510.35 s | yes |
  | LOMP10 `p3_l10_b` | 36000 | completed 1176.57 s | yes |
  | USN15 `s45_u15_l2` | 90000 | EndMission 2793.65 s | yes |
  | BSM01 `p6_b1_a` | 40000 | EndMission 770.94 s | yes |
  | JM08 `p6_j8_f` | 110000 | completed 2181.70 s | yes |
  | IJN18 `s46_i18_a` | 64000 | EndMission 2116.94 s | yes |
  | PRCP01 `l55_p01_c` | 30000 | completed 670.81 s | yes |
  | USN01 `p7_u1_a` | 26000 | **no end** | - |
  | JM06 `s47_j6_n1` | 120000 | **no end** (6000 s) | - |
  | ESMP08 `p6_e8_br3l` | 140000 | **no end** | - |
  | USNOS `s45_os_k` | 200000 | **failed** at 2721.53 s ("Troop": TroopTrans5 sunk by Shimotsuke at 2717.13 s) | - |

  - USN01 `a` lost. First divergence against `local\p7u1_a.log`: Mav5 dies at 61.85 s instead of 63.50 s
    (SaltLakeCity; the flak/AA flips 1120ac370 and 6f1c8edac moved the opening AA). All nine TBDs and twelve SBDs now
    release, but Convoy3 and Convoy5 are never hit. **Re-planned: `config/win_orders/usn01_p8_u1_c.txt`** (a with
    `3413 target ConTBD2 Convoy3`): **completed 1094.94 s** (`local\p8u1_c.log`), **1094.94 s under
    `BSP_FRAME_JITTER 20,3`** and **1089.94 s under 20,7** (`local\p8u1_cj3.log`, `local\p8u1_cj7.log`), guard logged
    on all three. Convoy6/3/2 (and 4 in two of three) sink to the strike by 210 s; Convoy5 and Convoy1 fall to
    SaltLakeCity and Northampton at 907-926 s. Variant b (also keeping ConTBD3 on Convoy5) and e (ConTBD1's last
    retarget to Convoy5) give the same 1094.94 s.
  - JM06 `n1` lost. First divergence against `local\p7a1_j6.log`: the first hit (USTroopTransport 02) at 82.40 s
    instead of 82.35 s; Fletcher-class 09 is no longer sunk by PlayerSub 03 at 132.70 s (it dies at 1616.10 s with no
    killer), and the rest of the convoy kills slip by 40-2500 s; ship shots 16594 -> 8706. Suspects on the range:
    the torpedo swim-depth homing (8572ddf97) and the AA/flak flips (1120ac370, 30a9b3991). For the ships lane.
  - ESMP08 `br3l` still lost: Cummings and Woodworth survive to 7000 s (Salt Lake City at 3894.72 s); the other
    targets die between 431 and 3434 s.
  - USNOS `k` still lost (known, the Betty1 startDist stall).
- **USN01 re-planned again for C2 (cc9-physics8, 2026-10-14): `config/win_orders/usn01_p8_u1_i.txt`**, replacing
  `usn01_p8_u1_c` (kept). With `kPlaneVelocityLowpassBound` ON (c9a8b1344) plan c loses Convoy2. Plan i is c with
  ConTBD1's last retarget to Convoy2 (3691) and ConSBD1 at Convoy5, ConSBD2 at Convoy3 from frame 3300 (the SBDs
  release before 3700, so the old 3700 slot was too late for them). On c9a8b1344 (export `local\p7_m`, exe
  BFD82222278F, reference env, 26000 frames): **completed 1185.03 s** (`local\p8u1m_i.log`), **1145.04 s under
  jitter 20,3**, **1210.06 s under 20,5**, guard logged on all three; **fails under 20,7** (Convoy2 and Convoy4
  survive, `local\p8u1m_ij7.log`). Convoy6/3/5 sink to the strike by 190 s; the cruisers finish the rest at 880-930 s.
  Variants f, g (complete without jitter, fail under 20,3) and h (fails under 20,3) are in local\.
- **PRCP01 `l55_p01_c` lost on main a103f270a** (not C2, R4 or C1: identical with all three OFF): failed 1491.83 s,
  "usn03.obj_p2_fail" (Enterprise). First divergence: Kate1's strike dies 30 s later (147.20 -> 177.46 s). Reported
  to the lead for the owning lane (range f83e91891..a103f270a). LOMP10 `p3_l10_b` moved to 1396.79 s on that range,
  and to 1631.02 s with C2 ON (still completes).
- **Re-time on agent/cc9-physics1 a2ad905f1** (= origin/main after C2 ON, c9a8b1344, plus cc13-seg19-21; cc9-physics8,
  2026-10-14). Export `local\p7_m`, exe 0AA17CC7F908, reference env, logs `local\p8w2_<row>.log`. ESMP08 and USNOS not
  re-run (lost, see above).

  | Row | Result | Guard |
  |---|---|---|
  | USN02 `s36_u2_p7` | completed 200.06 s | yes |
  | LOMP06 `l50_l6_q6` | completed 189.86 s | yes |
  | USN12 `s38_u12_p2` | completed 1510.35 s | yes |
  | LOMP10 `p3_l10_b` | completed 1581.97 s | yes |
  | USN15 `s45_u15_l2` | EndMission 2793.65 s | yes |
  | BSM01 `p6_b1_a` | EndMission 833.92 s | yes |
  | IJN18 `s46_i18_a` | EndMission 2111.93 s | yes |
  | USN01 `p8_u1_i` | completed 1185.03 s | yes |
  | JM08 `p6_j8_f` | **no end** (110000 frames) | - |
  | PRCP01 `l55_p01_c` | **no end** (failed at 1491.83 s on a103f270a) | - |
  | JM06 `s47_j6_n1` | **no end** | - |

  - JM08 f is lost to C2: on a103f270a (C2 OFF, `local\p8c2off_j8.log`) it completes at 1776.31 s; with C2 ON the
    "landingships" and "bombing" objectives never complete. First divergence: the second land kill, Tent01 16, at
    85.65 s instead of 66.55 s (Gyoraitei #Y3 instead of #Y2); the first, at 53.90 -> 54.30 s, already moves, so
    the planes' throttle/pitch change re-rolls the shared stream from the first seconds. Needs a re-plan (the plan
    is 6 MB of generated orders; `jm08_s45_j8_m10` is the older alternative).
  - JM08 alternatives on the same exe also fail: `jm08_s45_j8_m10` at 72000 frames (`local\p8w2_j8m10.log`) and
    `p6_j8_f` at 140000 frames (`local\p8w2_j8f9.log`, only Missouri and Nevada complete).

- **PRCP01 re-planned on main 25b98b1fa (cc9-physics9, 2026-10-14): `config/win_orders/prcp01_p9_p01_k.txt`**,
  replacing `l55_p01_c` (kept). Export `local\p9_m`, exe 073805D36E49, reference env, 30000 frames, logs with
  `.manifest.json`. On this main `c` no longer fails (Enterprise survives) but never ends: the first wave is repelled
  at 75.05 s (was 48.05 s), so Junyo is generated after frames 1600/1620 and the first strike leaves at 2600; Junyo
  dies at 287.20 s, and after Zuiho is generated (~330 s) every launch line finds "no such slot" (all four Enterprise
  slots hold airborne squadrons; nothing lands in time to free one). Plan `k` is `c` plus, at 6650/6660, Enterprise
  slot 3 (the Avengers launched at 5600 on Junyo, still armed when Junyo died) told `order 1` (moveto self) and then
  `order 3 Zuiho`: 006CCDA0 accepts an attack order from state 3 only while the squadron holds at its moveto. The same
  pair repeats at 12650 as a fallback; the slot-4 lines are refused. Enterprise_sqn08|.-2 sinks Zuiho.
  **Completed 536.54 s** (`local\p9a_p01k.log`), **675.26 s under jitter 20,3**, **536.09 s under 20,5**
  (`local\p9a_p01kj3.log`, `local\p9a_p01kj5.log`), guard logged on all three.
  - Variants that fail on the same exe (`local\p9_p01_<x>.txt`, logs `local\p9a_p01<x>.log`): `d` (extra Junyo lines
    at 1700-1820), `e` (d plus a CAP recall at 1690), `f`/`g`/`h` (order 3 on slots 3/4 with no moveto first: refused,
    "006CCDA0 refused the order in this slot state"), `i`/`j` (recall, order 2, then order 3: both are accepted, but the
    retargeted squadron does not sink Zuiho by 1500 s; not chased).
- **Re-time on main 25b98b1fa** (cc9-physics9, 2026-10-14). Export `local\p9_m`, exe 073805D36E49, reference env,
  `--stop-after-end 40`, logs `local\p9w1_<row>.log` with `.manifest.json` (launcher `local\p9_wins.ps1`, summary
  `local\p9_keep.py w1`). JM06, USNOS, JM08, USN12, USN15 and ESMP08 are being re-planned by other lanes and were not
  re-run.

  | Row | Frames | Result | Guard |
  |---|---|---|---|
  | USN02 `s36_u2_p7` | 16000 | completed 200.06 s | yes |
  | LOMP06 `l50_l6_q6` | 16000 | completed 376.83 s (was 189.86) | yes |
  | USN01 `p8_u1_i` | 26000 | completed 360.04 s (was 1185.03); 350.09 s under 20,3, 355.04 s under 20,5 | yes |
  | PRCP01 `p9_p01_k` | 30000 | completed 536.54 s (new plan, above) | yes |
  | LOMP10 `p3_l10_b` | 36000 | completed 1187.58 s (was 1581.97) | yes |
  | BSM01 `p6_b1_a` | 40000 | EndMission 833.92 s | yes |
  | IJN18 `s46_i18_a` | 64000 | EndMission 2156.98 s (was 2111.93) | yes |
  | IJN08 `s48_i08_h5` | 70000 | **no end** | - |

  - USN01 `i` was expected to lose to 81397688d's dive flips; it does not. The opening ScoutDauntless release line is
    still refused (sight 47.3 m from Convoy1), yet ScoutDauntless sinks Convoy1 at 171.41 s (Northampton did, at
    931.45 s, on c9a8b1344), and the strike sinks the other five by 207.66 s (Convoy4 at 203.36 s instead of 880.21 s
    by Northampton).
- **JM08 `p6_j8_f` is a win again** (cc9-lua59, reported 2026-10-14): **completes at 1464.00 s** on main fcee16d53
  (export cc9-lua52\local\l59_m4, 80000 frames, reference env, guard logged; log local\l59_m4s_j8.log with its
  manifest). It was lost on 453ed7e51 (no end in 110000 frames: LST 01/02, USTroopTransport 02 and LSM 02 survive).
  A re-plan on 1ae160e44 (filters on the Grayson/Helena/Missouri launch lines plus late PT-boat assignments)
  completed at 2150.67 s, but later gunnery flips (91d6a9d22 and others) restored the original plan, so no new order
  file. **Knife-edge** under the stock flips. Removing the Grayson/Helena/Nevada launch lines loses the HQ, because
  those ships shell it. LOMP10 1581.97 s on 453ed7e51 stands.
- **IJN08 `s48_i08_h5` lost on main 25b98b1fa; re-planned as `config/win_orders/ijn08_p9_i08_w.txt`** (cc9-physics9,
  2026-10-14; generator `local\p9_i08plan.py`, from cc9-ships48's `s48_i08gen.py`). Frame budget 100000.
  - Loss, cause 1 (phase 1: the subs must sink the six Cargos; h5 has no order before frame 12490): on main the
    JapSubs never launch a torpedo (`summary mission gunnery torpedo bot range gate_a_drops=71576 ... mean_attack_range=756`).
    Responsible: **91d6a9d22** (kTorpedoBotAttackRangeBound and its companions). Same-tree check on 25b98b1fa:
    kTorpedoBotAttackRangeBound, kTorpedoBotStaleFlagBound and kTorpedoSubmergedTargetDropBound false reproduces the
    win's phase-1 death rows exactly (`local\p9tb0_i08.log`); kSubAttackOwnHalfLengthBound (fd6d92617) false alone
    changes nothing (`local\p9sub0_i08.log`).
  - Loss, cause 2 (phase 2): with those switches false the run still fails, at 1680.51 s (Game Over: both Marus sunk
    by US_Airfield_03 squadrons at 1142 and 1678 s; no Maru died in the win). The air kills diverge from ~692 s; the
    script's airfield manager (ijn_8_north_sol.lua:543-565) targets only the Marus (Mission.JapTrans), so this is
    the plane flips since 181196e51 (C2 and later); not bisected further.
  - Plan `w`: phase 1, every 200 frames each JapSub is selected and given one `attack` line per Cargo, least
    preferred first; all six Cargos sink by ~414 s. CORRECTION (cc9-physics9, same day): an `attack` line on a dead
    target is applied, not refused (`local\p9i8_w.log` frame 6101: Cargo6, dead at 286.40 s, is applied), so in every
    cycle the LAST line, the sub's first preference, sticks even when that target is dead. The same holds for the
    phase-2 `A|B` lists: `A` is issued last and sticks, so the US_Airfield_03 entries below are issued and overridden
    a frame later, never a real fallback. Why `w` wins where `n` (the same plan without those lines) does not is not
    established; the extra order traffic may act only through the shared random stream. Treat `w` as a knife-edge.
    Phase 2 is h5 shifted by -3500 frames, with Kongo/Hiei on Jap_Airfield and Kirishima/Haruna on US_Airfield_01
    first, and every later target list ending in US_Airfield_03 (in h5 nothing attacks it after frame 40500).
    **Completed 4551.09 s** (`local\p9i8_w.log`), **4406.65 s under jitter 20,3**, guard logged on both; no Maru
    dies. **Fails under 20,5** (1365.21 s: Maru1 and Maru2 sunk by US_Airfield_03/02 squadrons at 1233/1365 s), so
    it is a knife-edge on the early Maru survival. Variant `x` (Kongo attacks Coastal Gun 03 instead of its h5
    moveto) completes at 4580.97 s.
  - Failing variants (`local\p9_i08_<v>.txt`, logs `local\p9i8_<v>.log`): a/b (attack without select: refused for
    the uncontrolled subs), c/d (5 of 6 Cargos), e/f (phase 1 done; Marus lost by 1440/1010 s with h5 unshifted),
    g/h/i/q/r/s/t/u (Marus lost 1060-1415 s), m (Marus held at the spawn with a cruiser screen: 2171 s), n (w
    without the US_Airfield_03 fallbacks: 3067 s; 4103 s under 20,3 with everything but US_Airfield_03 dead).
- **ESMP08 on main 25b98b1fa (cc9-physics9, parked item, not won).** `esmp08_p6_e8_br3l`, 140000 frames: no end,
  10/15 US ships sunk; Zuikaku sails south-west on its own at ~10 m/s from the start (trajectory CSV
  `local\p9e8_traj.csv`) and Iowa sinks it at 1388 s. Variants (`local\p9_e8gen.py`, `local\p9_e8gen_f.py`, logs
  `local\p9e8_<v>.log`, summary `local\p9_e8sum.py`, launches `local\p9_e8launch.py`):
  a (select+moveto every 2000 frames holding the four carriers ~500 m north-east of their start) 11/15;
  b (a plus Ise/Hyuga moved to ~(8000, 7700) as the US attack-move's nearest target) 12/15, three carriers alive;
  e (b plus surface attack lists from 2000 s) failed 5406.88 s; f (b plus one live target per ship per cycle from
  frame 44100, from b's death times) **13/15** (Fanning, Woodworth survive) but failed 6527.49 s; g (light ships only)
  12/15, failed 6955.81 s. Limits: from ~2500 s every launch line is refused with "no stock of the class" (stock is
  consumed since 74aae670d, all classes including the Zeros), and the script's endless US bomber waves (every 180 s)
  then sink the carriers and the surface ships. An attack line on a dead target is applied, so per-cycle target
  lists must be resolved from a reference run's death times.
- IJN08 variant `x` also completes under jitter 20,3 (4361.83 s, `local\p9i8_xj3.log`).
- **ESMP08 re-planned: `config/win_orders/esmp08_l61_e8_f.txt`** (cc9-lua61, 2026-10-14). This is br3l with every launch
  group (frame, carrier, slot) re-ordered by a (class, target) rank, from `local\l61_e8_alloc.py f` in the cc9-lua52 tree:
  Jills/Kates (163/162) take the battleships first, Judys (159) and then Zeros (150) take the cruisers, and destroyers
  come after. Exe = tree build of main 8612ef146, copied to `local\l61_m\`, 140000 frames, reference env. Logs are
  `local\l61_<v>_e8.log`; census with `local\l61_e8_launches.py <log> 1200`.
  | plan | reference | 20,3 | 20,5 |
  |---|---|---|---|
  | br3l (unchanged) | 14/15, Woodworth survives | 7/15 | - |
  | **f** | **completed 4196.63 s** | 13/15 (Washington, Fanning survive) | **completed 5703.77 s** |
  | h = f + physics9's carrier hold (p9_e8_a) | completed 2965.92 s | 14/15 (Cummings) | 14/15 (Woodworth) |
  | j = h + held-slot `order 3` every 1200 fr | completed 2600.56 s | identical to h | - |
  | e / g / m (destroyers earlier) | 11/15, 9/15, 13/15 | - | - |

  The exec guard refused sus_prog.exe on every completion. Surface `attack` lists for the IJN ships (variants c/d)
  apply, but no US ship takes surface damage (US cruisers 0 taken at 1200 s), so they are dropped. The binding limit
  on this exe: a slot whose squadron has landed stays in state 3, and the host's stowed planes cannot fly a re-send
  (5ep). Launches therefore stop near 2400 s (~80 launches; about 30k "not in state 1 or 5" refusals against 2k "no
  stock"); routed to cc9-physics10. **f is committed as the most robust plan: 2 of 3 conditions complete, and it is a
  knife-edge under 20,3.** It still needs a re-time on current main.
- **Re-time on main 9228d13fe** (cc9-lua61, 2026-10-14). This main includes 610dc9bfd, 3f25f26ce, 222 ON, the accept
  gate, the blast mesh, the director leader fire target, the torpedo done formation, the level-bomb prepare, the
  wanderer float trig and the harness dead-target refusal 0f70e9dee. Exe: tree build copied to
  `cc9-lua52\local\l61_rt\` (A6266C343C18). Reference env, `--stop-after-end 40`. Logs are
  `local\l61rt_<row>_<ref|j3>.log` with manifests; queue `local\l61_retime.ps1`, summary `local\l61_rt_sum.py`, helm
  divergence `local\l61_helmdiff.py <old> <new>`.

  | Row | Frames | Reference | 20,3 | Guard |
  |---|---|---|---|---|
  | USN02 `s36_u2_p7` | 16000 | completed 200.06 s | completed 200.06 s | yes |
  | LOMP06 `l50_l6_q6` | 16000 | completed 383.83 s (was 376.83) | completed 355.84 s | yes |
  | USN01 `p8_u1_i` | 26000 | completed 365.03 s (was 360.04) | completed 360.04 s | yes |
  | PRCP01 `p9_p01_k` | 30000 | completed 536.29 s | completed 800.68 s | yes |
  | USN12 `g45_u12_e7` | 36000 | completed 1675.51 s | completed 1675.51 s (jitter confirmed on) | yes |
  | LOMP10 `p3_l10_b` | 36000 | completed 1003.43 s (was 1187.58) | completed 1245.64 s | yes |
  | BSM01 `p6_b1_a` | 40000 | EndMission 833.92 s | EndMission 875.91 s | yes |
  | IJN18 `s46_i18_a` | 64000 | EndMission 2162.98 s (was 2156.98) | EndMission 2166.99 s | yes |
  | JM08 `p6_j8_f` | 110000 | **failed 940.94 s** ("The HQ has been destroyed!") | **no end** | - |
  | JM06 `s49_j6_s` | 120000 | completed 2123.95 s (was 1991.82) | **no end** (was 2901.71) | ref yes |
  | IJN08 `p9_i08_w` | 100000 | **failed 789.93 s** (Game Over) | **failed 1530.37 s** (Game Over) | - |
  | ESMP08 `l61_e8_f` | 140000 | completed 6532.47 s (was 4196.63 on 8612ef146) | 13/15 (Washington, Cummings) | ref yes |

  Losses, first divergence and suspect range (none bisected):
  - **JM08 f.** Diffed against the last win `cc9-lua52\local\l59_m4s_j8.log` (fcee16d53, 1464.00 s). The first hit
    moves from 5.00 to 4.65 s, so the run diverges from the opening salvo. The first helm divergence is at frame 6600:
    `build Shipyard 01 class 77` is now built instead of being refused, and at 6800 `select Gyoraitei #Y1` is refused
    (the byte filters), so its `attack` lines are refused. Headquarter 01 falls at 940.94 s. Range fcee16d53..9228d13fe;
    the first-hit move points at the gunnery flips 01dcecf2e (accept gate / 222), 4aa1a549a (blast mesh) and
    9428d8b8f (director leader fire target).
  - **JM06 s, 20,3 only.** Diffed against `cc9-ships44\local\s49_j3_s.log` (fd6d92617). The first moved death is US
    Cargo Transport 02 at 171.96 -> 172.01 s, then the run drifts. The decisive change: PlayerSub 01 is sunk at
    1384.53 s by Lexington-class 01_sqn02|.-2 (cat 6, damaged from 726.65 s near US Tanker 02). In the win that
    squadron was shot down, and the sub sank Tanker 02 at 1744.18 s. Range fd6d92617..9228d13fe; most likely the plane
    lane (attack and formation flips), but not shown.
  - **IJN08 w.** Diffed against `cc9-physics1\local\p9i8_w.log`. The first moved death is DLBarrak 03 at 8.30 -> 8.70 s
    (killer DamagedBB -> HQ), so the exe moved it before any order line differs. The first helm divergence is at frame
    5725: `attack JapSub2 -> Cargo6` is now `refused: target dead` (0f70e9dee). w was built on dead-target lines being
    applied (the last line sticks), so the harness fix changes the plan's meaning. Maru2 dies at 684.06 s (bombed by
    Jap_Airfield_sqn15, a friendly squadron) and Maru1 at 787.53 s (US_Airfield_03_sqn10); under 20,3 both die to
    US_Airfield_03_sqn22 at 1476/1526 s. Range 25b98b1fa..9228d13fe, with 0f70e9dee a certain contributor. w needs a
    re-plan against the refusal; variant x was not run.
  - **ESMP08 f** holds the reference on this main and stays at 13/15 under 20,3: knife-edge (state-3 slot limit, with
    cc9-physics10).
- **ESMP08: the Zero recall does not help on this host** (cc9-lua61, 2026-10-14, main 9228d13fe). The test was f
  plus `order <carrier> <slot> 2` every 4800 frames (slot 4 always, slots 1-3 from frame 30000; plan
  `local\l61_e8_n.txt`, log `local\l61_n_e8.log`). Result: 6/15 sunk, and 6/15 again under 20,3.
  - 35 recalls were applied, but the summary shows `stowed plane stock return ... slot_returns=2`.
  - A recalled leader lands, stows and returns its stock. Wingmen still in `divebomb ai:close_attack` keep flying
    (Zuiho_sqn15, Chiyoda_sqn13), so the squadron never empties and its slot stays in state 4.
  - Routed to cc9-physics10 to settle whether this is a host gap. The ESMP08 row stays `esmp08_l61_e8_f`.
  - Physics10's own r2 (on 610dc9bfd, `cc9-physics1\local\p10e8_r2*.log`): reference completed 5018.40 s, 20,5
    completed 4500.43 s, and 20,3 reached 14/15 (Fanning survives; it never leaves its spawn). Not committed.
- **IJN08 (cc9-physics10, exe 610dc9bfd, logs `cc9-physics1\local\p10i8_<tag>.log`)**:
  `config/win_orders/ijn08_p10_i08_z.txt` (7d613369d) puts US_Airfield_03 first for three battleships and relies on
  the dead-target refusal 0f70e9dee. Results: completed 3006.81 s; 20,3 completed 3947.73 s; 20,5 no end within
  100000 frames (no fail); 20,11 failed at 1220 s. On the same exe, w completes 20,5 at 4566.03 s and 20,7 at
  4441.52 s, and fails 20,3 at 1530 s and 20,11 at 1615 s. z is the IJN08 row from now on (plain and 20,3). Not
  re-timed on 9228d13fe.
- **New row USN15 `usn15_g46_u15_tl`** (cc9-gunnery46, GUNNERY 258; recorded by cc9-lua61 from its report).
  - Plan `config/win_orders/usn15_g46_u15_tl.txt`, dc1c7efc8 on agent/cc9-gunnery36.
  - Exe 9b25d503, an export of a65c06700 (includes 3b30ed84a), reference env, 144000 mission frames (more than
    90000 are needed). Log `cc9-gunnery36\local\g46_p2_u15tl.log`.
  - Reference: EndMission 6331.16 s with FLAGWIN; the guard is logged. Deaths: Nachi 781 s, Yamashiro 1171 s, Fuso
    3840 s, Ashigara 6326 s.
  - **Reference-only win**: 20,3 fails at 1422.31 s and 20,5 fails at 1437.33 s. Yamashiro survives under both,
    because its 1171 s kill is a deterministic Fubuki-07 friendly torpedo.
- **eaf50b067 (kTorpedoBotCanFireReloadBound ON, GUNNERY 260.1)** (cc9-gunnery47's ON runs, recorded by cc9-lua62).
  Exe 47A1F5820F8E, a tree build of agent/cc9-gunnery36 d4e7a58f1, reference env, `--stop-after-end 40`. Logs
  `cc9-gunnery36\local\g47_on_<u15|j6|i8>.log` with manifests.
  | Row | Result | Was |
  |---|---|---|
  | USN15 `g46_u15_tl` (144000) | **failed 1417.31 s** (Game Over) | EndMission 6331.16 s (reference-only win) |
  | JM06 `s49_j6_s` | completed 2144.97 s, guard logged | completed 2123.95 s |
  | IJN08 `p9_i08_w` | failed 789.93 s (Game Over, entity HQ) | failed 789.93 s (unchanged) |

  The flip is faithful and stays ON; USN15 tl needs a re-plan (see the next re-time).
- **Re-time on origin/main 25fbdcba0** (cc9-lua62, 2026-10-16). This main contains f9d36bb0e, eaf50b067 and the cc13
  mission-picture wiring 7e2ac0fec (never paired before), plus the Codex font/text/frontend batches. Exe: tree build
  (full `scripts/build.ps1`, tests pass) copied to `cc9-lua52\local\l62_rt\` (3ED7E2E69EE6). Reference env,
  `--stop-after-end 40`. Logs `local\l62rt_<row>_<ref|j3>.log` with manifests; queue `local\l62_retime.ps1`, summary
  `local\l62_rt_sum.py`, first moved death `local\l62_deathdiff.py <old> <new>`. "Was" is the 9228d13fe re-time
  unless named.

  | Row | Frames | Reference | 20,3 | Guard |
  |---|---|---|---|---|
  | USN02 `s36_u2_p7` | 16000 | completed 200.06 s | completed 205.06 s (was 200.06) | yes |
  | LOMP06 `l50_l6_q6` | 16000 | completed 374.83 s (was 383.83) | completed 373.83 s (was 355.84) | yes |
  | USN01 `p8_u1_i` | 26000 | completed 365.03 s | completed 360.04 s | yes |
  | PRCP01 `p9_p01_k` | 30000 | completed 536.29 s | completed 800.68 s | yes |
  | USN12 `g45_u12_e7` | 36000 | completed 1685.52 s (was 1675.51) | completed 1675.51 s | yes |
  | LOMP10 `p3_l10_b` | 36000 | completed 1293.69 s (was 1003.43) | completed 1276.67 s (was 1245.64) | yes |
  | BSM01 `p6_b1_a` | 40000 | EndMission 833.92 s | EndMission 875.91 s | yes |
  | IJN18 `s46_i18_a` | 64000 | EndMission 2183.00 s (was 2162.98) | EndMission 2166.99 s | yes |
  | IJN08 `p10_i08_z` | 100000 | completed 3006.81 s | completed 3947.73 s | yes |
  | JM06 `s49_j6_s` | 120000 | completed 2144.97 s (= eaf50b067) | **no end** (as on 9228d13fe) | ref yes |
  | JM08 `p6_j8_f` | 110000 | **failed 949.94 s** (was 940.94) | **no end** | - |
  | ESMP08 `l61_e8_f` | 140000 | completed 6532.47 s (identical deaths) | 13/15 (identical deaths) | ref yes |
  | USN15 `g46_u15_tl` | 144000 | **failed 1417.31 s** (Game Over) | **failed 1417.31 s** | - |

  - IJN08 z reproduces physics10's 610dc9bfd times exactly on both conditions; it replaces w as the IJN08 row.
  - The only new loss against the last re-time is USN15 tl, already recorded under eaf50b067. Against its win log
    `cc9-gunnery36\local\g46_p2_u15tl.log`, the first moved death is Fletcher-class02 at 760.89 -> 761.94 s (killer
    Nachi in both); Denver then dies at 1311.95 s instead of 779.93 s, and Yamashiro, Fuso and Ashigara survive. The
    Japanese torpedo reload bound moved the Japanese torpedo timing, so the plan needs a re-plan (gunnery lane).
  - JM08 f moves earlier than before: the first moved death is MainAirFieldEntity 01_sqn03|.-3 at 426.97 -> 429.87 s,
    after the slot-4 launch at frame 8903 is now applied (was refused). It is being re-planned (next entries).
  - JM06 s under 20,3 diverges from the 9228d13fe run at 168.11 s (Fletcher-class 09 sunk by PlayerSub 03 instead of
    surviving); still no end (cc9-physics lane item).
  - The other moves (LOMP10, LOMP06, USN12, IJN18) are timing only; every completion logged the guard.
- **JM08 re-planned: `config/win_orders/jm08_l62_j8_w1.txt`** (cc9-lua62, 2026-10-16), replacing `jm08_p6_j8_f`
  (kept). Exe `local\l62_rt` (25fbdcba0, 3ED7E2E69EE6), reference env, 110000 frames, `--stop-after-end 40`; the
  gunnery flips stay ON. Logs `cc9-lua52\local\l62_j8<v>.log` with manifests.

  | Condition | Result | Guard |
  |---|---|---|
  | reference | **completed 1954.48 s** | yes |
  | 20,3 | **completed 1464.00 s** | yes |
  | 20,5 | completed 1517.05 s | yes |
  | 20,7 | failed 1138.03 s (HQ destroyed, 1 landing ship left) | - |

  The plan is f post-processed by `cc9-lua52\local\l62_j8_mod.py` (arguments on its first line):
  - Gyoraitei boats take Nevada as their top attack target from frame 8000. Nevada attack-moves the HQ and its main
    battery is what destroyed the HQ in f (about 21000 damage dealt). The boats kill it in every seed tried
    (535-581 s).
  - The airfield's 152/166/167 launch lines keep only landing ships and raiders. Class 154 never killed a ship.
  - Fubuki then Isokaze alternate `torpedo <ship> at <landing ship>` presses from frame 10600. The rotation is
    every 400 frames, offset 290, released after 89 frames.
  - From frame 16000 the player's own idle squadrons get a `target A | B | ...` pick every 400 frames, each after a
    `select`. These are H6K Mavis 02/03, Ki-43 Oscar 01 and Gekko 01, which no earlier plan used. The picks list
    the phase-2 raiders first, then the landing ships with the toughest first. In the reference run Gekko 01 sinks
    the last one (LST 02, 1570 s).
  - Slots 1-2 get `order 3 <raider>` retargets, and every launch group first offers the raiders. Plan f named the
    B-17s only from frame 24000; wave 1 spawns at 18668-31810.

  Mission mechanics behind the choices (prcpjm08.lua, this installation):
  - The HQ loss is `CommandBuilding.Party == 2`. The host neutralizes the HQ when its health reaches 0.
  - StartInvasion fires when a US ship comes within 300 m of the origin, which is the bay between the shipyard and
    the US formation. It sends every escort and landing ship at the HQ.
  - Under 20,3 the invasion fires at 603.85 s in every variant that drops the bombers' warship targets. Once it
    fires, phase 1 usually completes, because the HQ guns sink the LSTs, but the HQ enters phase 2 damaged.
  - Without the invasion, the landing ships idle about 3.5 km north of the origin after Missouri dies. Phase 1 is
    then limited by firepower.
  - Friendly torpedo kills between Fubuki and Isokaze decided several runs. They happened in i (reference), j3, l3, n
    and n3; the presses stay because no plan without them completed.
  - About 30 variants are recorded in `local\l62_j8_<v>.txt` and their logs. Reference-only wins: i1/s1 (1735.27 s) and
    w2/w3 (1946.47 s / 1416.96 s). The only 20,3-only win is i (1295.84 s).
  - **Possible host gap (Lua lane):** each completion comes about 4 s after wave 3 (8 C47s, class 136) spawns, with
    no C47 shot down (w1: wave 3 at frame 39012 = 1950.6 s, completed 1954.48 s). CheckPrim3 needs every class-136
    unit in `Mission.Dakota` dead, and luaBomberWaveSpawned3 inserts only units whose `unit.Class.ID == 136`. An
    empty table therefore suggests `unit.Class.ID` does not read 136 on this host. This is not yet checked; if it
    is fixed, JM08 also has to shoot down the C47s.
  - Confirmed afterwards in the w1 log: PilotSetTarget ran only for wave 1, whose handler calls it unconditionally.
    Waves 2 (B-25 type 118, SBD type 108) and 3 (C47 type 136) never matched their `Class.ID` branches.
  - **Stale exe.** w1 was timed on the 25fbdcba0 exe. Main has since turned ON the CommandBuilding step repair
    (4285975cc), the path-cursor step (82041f5c3), AngleErr per evaluation (6c900f1e0), the auto-target phase draw
    (fad263ffb) and the sink and sub-wake draws, and all of them move JM08. w1 must be re-timed on current main.
    There, ships53 reports that `cc9-ships44\local\s52_j8_b3.txt` completes at 1329.87 s (reference, 40000 frames,
    log `cc9-ships44\local\s53_p1_j8.log`; 20,3 not run).
- **New row BSM09: a no-orders win** (cc9-ships53, recorded by cc9-lua62).
  - Tree agent/cc9-ships44 at 2edfb11a3 (origin/main), reference env, 36000 frames, `--menu-select BSM09`, no
    `--helm-orders`. Log `cc9-ships44\local\s53_rf1_b9.log` with its manifest.
  - Zuikaku is driven below 10% HP by the US AI, which completes primary 1 (bsm_09_battle_of_the_coral_sea.lua:521-536).
    EndMission comes at 917.90 s, then luaMissionCompletedNew "Enemy carriers are retreating - Mission Complete" at
    921.85 s. The guard is logged.
  - It runs at all only since kGenerateObjectRecordOnlyCreatorBound (b510a54fe). 20,3 has not been run.
- **ESMP08 row is now `config/win_orders/esmp08_p12_e8_rb.txt`** (cc9-physics12, 2f1480151), replacing
  `esmp08_l61_e8_f`.
  - On 135f8ae26 (kAirOpsQueueCompactBound ON): reference 4684.71 s, 20,3 3594.58 s, 20,5 3251.20 s; 20,7 no end
    (12/15).
  - On b46d56a35 (kSquadronLandTargetGoneBound ON): reference 3334.33 s, 20,3 3594.58 s, 20,5 3251.20 s.
  - The guard is logged in all; logs are `cc9-physics1\local\p12e8{b,c}_rb_<cond>.log`.
- **JM06 row is now `config/win_orders/jm06_p12_j6_a4.txt`** (cc9-physics12, 8652858c8), replacing `s49_j6_s`.
  - Plan `s` lost 20,3 to kTorpedoBotThinkTimerBound (0cc305ee0, faithful) and lost the reference under 6c900f1e0.
  - a4 on 25fbdcba0: reference 2226.05 s, 20,3 2343.16 s, 20,5 2175.00 s, 20,7 2033.86 s, 20,11 3180.98 s; the guard
    is logged in all.
- **Open (ships53 report):** BSM01 `bsm01_l50_b1_a` reaches no EndMission within 20000 frames on current main
  (`cc9-ships44\local\s53_q0_b1.log` / `s53_q1_b1.log`). The next re-time must check it at its full 40000 frames.
- **Current main also needs a re-time** for IJN08 z, USN12, JM08, ESMP08 and PRCP01, the torpedo-sensitive rows.
- **ESMP08 lost under kFighterSpentEndBound ON** (cc9-physics12, 45ffa86bf on agent/cc9-physics1, faithful):
  `esmp08_p12_e8_rb` reference no end with 5/15 sunk (`cc9-physics1\local\p12e8d_rb_ref.log`); `esmp08_l61_e8_f` no
  end with 4/15 (`p12e8d_f_ref.log`). The plans' air-ops Zeros have equipment 0, which is a null loadout in the image
  (no bomb). They get divebomb, and 009F8160/009F7C90 end it at launch; the old host dropped phantom bombs from the
  release path's default. The re-plan waits on a lead decision (OCEAN_WAVE_FIELD 43.1, 44).
- **JM08 `cc9-ships44\local\s52_j8_b3.txt` still wins under kPathFollowFinishBound ON** (cc9-ships54, 56734e5ec):
  completed 1329.87 -> 1336.88 s, guard logged (`cc9-ships44\local\s54_p1_j8.log`).
- **Re-time on 7a31a4fef** (cc9-lua63, 2026-10-17 UTC; origin/main b64fd347d plus the level-bomb flips f84b3e537, so
  kVehicleClassRowIdBound and the 5hv flips are ON; kEntryPointIsFunctionBound / kRomanMemberNamesBound still OFF).
  Exe `cc9-lua52\local\l63_rt` (9CCCFF2E9EF3), reference env, `--stop-after-end 40`; logs
  `cc9-lua52\local\l63rt_<row>_<ref|j3>.log` with manifests. "Was" is lua62's 25fbdcba0 re-time or the row's own record.

  | Row (plan) | frames | reference | 20,3 | was (ref / 20,3) |
  |---|---|---|---|---|
  | JM08 `l62_j8_w1` | 110000 | **no end** | **no end** | 1954.48 / 1464.00 |
  | JM06 `p12_j6_a4` | 120000 | **no end** | completed 2000.83 s | 2226.05 / 2343.16 |
  | IJN08 `p10_i08_z` | 100000 | completed 3937.72 s | **failed 959.89 s** (Game Over) | 3006.81 / 3947.73 |
  | USN12 `g45_u12_e7` | 36000 | completed 1685.52 s | completed 1675.51 s | identical |
  | ESMP08 `p12_e8_rb` | 140000 | **no end** | **no end** | 3334.33 / 3594.58 |
  | PRCP01 `p9_p01_k` | 30000 | completed 877.26 s | completed 572.28 s | 536.29 / 800.68 |
  | BSM09 (no orders) | 36000 | EndMission 914.90 s, narrative complete 920.90 s | **failed 1016.88 s** (Game Over) | 917.90 / not run |
  | USN15 `g46_u15_tl` | 144000 | no end (survives) | no end (survives) | failed 1417.31 / failed |
  | IJN18 `s46_i18_a` | 64000 | EndMission 2032.86 s | EndMission 2216.04 s | 2183.00 / 2166.99 |
  | BSM01 `p6_b1_a` | 40000 | EndMission 779.93 s | EndMission 1515.40 s, **no guard** | 833.92 / 875.91 |
  | LOMP10 `p3_l10_b` | 36000 | completed 1476.87 s | completed 1433.82 s | 1293.69 / 1276.67 |
  | USN01 `p8_u1_i` | 26000 | completed 699.95 s | completed 749.94 s | 365.03 / 360.04 |
  | USN02 `s36_u2_p7` | 16000 | completed 200.06 s | completed 200.06 s | 200.06 / 205.06 |
  | LOMP06 `l50_l6_q6` | 16000 | **failed 744.09 s** | **failed 611.12 s** | 374.83 / 373.83 |

  Every completion and every EndMission row except BSM01 20,3 logged the guard. Losses and their first moved death
  (`l62_deathdiff.py` against the win log):
  - JM08 (both): Medium Bunker, Concrete 02 at 65.45 s (AA truck 07) -> 63.95 s (Patrolboat 01) against
    `l62_j8w1.log`. kVehicleClassRowIdBound (a96d382c5) also makes CheckPrim3 wait for the C47 wave
    (Mission.Dakota), so the plan must now shoot the C47s down; re-plan.
  - JM06 reference: Fletcher-class 09, 168.11 s (PlayerSub 03) -> survives to 1616.8 s (PlayerSub 01); first hit
    +3.85 s against `cc9-physics1\local\p12j6_a4_ref.log`.
  - IJN08 20,3: Cargo1, 324.14 s (JapSub1) -> 296.9 s (JapSub2) against `l62rt_i08_j3.log`.
  - ESMP08 (both): identical to cc9-physics12's `p12e8d_rb_ref.log` (1321 deaths each); against the `c` win
    the first moved death is Zuiho_sqn03|.-3 at 382.23 s (Cummings) -> 382.78 s (Case).
  - LOMP06 (both): Toba Maru (191.81 s, Narwhal) survives; Kongosan Maru moves 184.86 -> 184.76 s first.
  - BSM01 20,3: EndMission is set in phase 3 but `lua_Think` fails at commandhelpers.lua:5991
    (`luaObj_IsActive` on a missing objective), and no completion or guard line follows.
  - BSM09 20,3 had never been run; it fails.
- **Then flipped on the same tree (cc9-lua63, SQUADRON_LAND_TASK 5hv.8):** kEntryPointIsFunctionBound (identical on
  USN02 and LOMP06) and kRomanMemberNamesBound. The roman flip keeps LOMP10 (completed 1559.95 s, guard) but **loses
  USN01** (no end in 26000 frames; KatTBD's second plane dies 1.15 s later and Convoy1 survives), through the
  per-name keyed stand-in streams. USN01 `p8_u1_i` needs a re-plan on the flipped tree, the plane-heavy rows' times above
  move with the flip, and order files must name wing planes `|.-II` etc.
- **JM06 row: `config/win_orders/jm06_p12_j6_c1.txt`** (cc9-physics12, OCEAN_WAVE_FIELD 45; recorded by cc9-lua64,
  2026-10-17). Completed 1967.79 s (reference), 2259.08 s (20,3), 1901.73 s (20,5), 2102.93 s (20,7), guard logged in
  all four (`cc9-physics1\local\p12j6c1_<ref|j3|j5|j7>.log`, exe `cc9-physics1\local\p12_m5`, tree fedbf3c92). That
  tree predates the roman flip, so the row still needs a run on current main.
- **BSM01 `p6_b1_a` under 20,3 is a loss, not a host gap** (cc9-lua64, `cc9-lua52\local\l63rt_b1_j3.log`). HenryPT dies
  at 1514.05 s (`death row`, killer Sacramento, killer_cat 6 blast, nearest ship MiniSub 319 m), so lua_Think calls
  `luaMissionFailed` (bsm_01_stationed_at_pearl.lua 2367, this installation, mtime 2024-07-13). It sets
  `Mission.EndMission = true` and then calls `luaObj_IsActive("secondary",1)` (line 2401), but the script's own
  `Mission.Objectives.secondary` table is empty (lines 109-110), so commandhelpers.lua:5991 indexes nil and the
  function stops before `luaMissionFailedNew`. The same script fails the same way on any engine; the logged
  "EndMission" for this row is a failure, and the missing guard line is correct (no completion). The friendly-blast
  kill is a gunnery-lane question.
- **USN01 re-planned: `config/win_orders/usn01_l64_u1_b.txt`** (cc9-lua64, 2026-10-17, tree 1c43edf3b = origin/main
  f4d7658e6 plus an OFF switch; exe `cc9-lua52\local\l64_off`, 43F25CF839C4; roman names ON). p8_u1_i lost Convoy1
  (ScoutDauntless misses after the roman flip moved the per-name streams). ConSBD1 and ConSBD2 still carry bombs
  when the dead-target retest ends their dives (Convoy5 189.21 s, Convoy3 193.76 s), so b adds
  `3791 target ConSBD1 Convoy1 | Convoy4 | Convoy2 | Convoy3 | Convoy5 | Convoy6` and the same for ConSBD2 at 3881
  (the first live pick). Completed 714.95 s (reference), 730.00 s (20,3), 699.95 s (20,5), 739.94 s (20,7), guard
  in all four (`cc9-lua52\local\l64_u1_b[_j3|_j5|_j7].log`). Plan a (Convoy1 only) lost 20,3: Convoy1 died early
  there and Convoy4 survived.
- **LOMP06 re-planned: `config/win_orders/lomp06_l64_l6_a.txt`** (cc9-lua64, same exe). q6 kept attacking
  Kongosan Maru after it sank at 184.76 s, and Toba Maru (sunk at 191.81 s by the Kongosan salvo in the win) now
  survives; a sends the 3800-5800 attack lines at Toba Maru. Completed 345.84 s (reference) and 354.84 s (20,3),
  guard in both (`cc9-lua52\local\l64_l6_a[_j3].log`).
- **After the kAttackTaskTargetRetireBound flip (8b1dd41ae; runs on the flipped export `cc9-lua52\local\l64_on`).**
  - USN01 `usn01_l64_u1_b` loses its reference on the flip (Convoy4 survives: the retarget rebuilds ConSBD2's dive
    task while it is diving on Convoy3). **USN01 row: `config/win_orders/usn01_l64_u1_f.txt`**: p8_u1_i with
    ConTBD2 on Convoy4 (not Convoy3), ConSBD1 retargeted to `Convoy1 | Convoy4 | ...` after its retest, ConSBD2
    left on Convoy3, plus usn01_p3_u1_z4's 3792 Enterprise F4F launch lines on the Nells (phase 3 needs all 18 Nell
    planes down; d without them loses 20,3 and 20,5 to one or more surviving Nells). Completed 410.02 s (reference),
    370.03 s (20,3), 430.02 s (20,5), 400.03 s (20,7), guard in all four (`local\l64_u1_f_on[_j3|_j5|_j7].log`).
  - LOMP06 `lomp06_l64_l6_a`: identical on the flipped exe (completed 345.84 s, 0 retires, `local\l64_l6_a_on.log`).
  - **JM08 row: `config/win_orders/jm08_l64_j8_e.txt`** (cc9-lua64, flipped exe). On current main the C47 wave
    (PRCPJM08.lua 1186-1278, this installation, mtime 2024-08-26: five C47 squadrons of four with
    `PilotSetTarget(CommandBuilding)`; CheckPrim3 needs all of them dead) and the bomber waves before it wear
    Headquarter 01 down to zero health, which neutralizes it (party 2) and fires CheckHQ's Fail ("The HQ has been
    destroyed!", w1 on main: failed 2247.12 s). e is lua62's generator (`cc9-lua52\local\l62_j8_mod.py`, w1's options
    with `--torp-start 6000 --torp-period 300`), then `local\l64_j8_c47.py --from 38800 --launch-classes 150,154` (every
    C47 squadron plane first in the Ki-43 Oscar 01 / Gekko 01 target lists, and a launch line per C47 squadron ahead
    of each class 150/154 launch) and `local\l64_j8_nols.py` (no landing ships in those two fighters' lists, so they
    stop dying to landing-ship AA around 1570-1680 s and are alive for the waves). Completed 2142.66 s (reference,
    all 20 C47s down) and 2881.39 s (20,3), guard in both; **20,5 and 20,7 lose the HQ** (failed 1995.87 s and
    1242.14 s). Logs `cc9-lua52\local\l64_j8_e_on[_j3|_j5|_j7].log`. Variants b, c, d (same logs dir) each won one
    seed only: the HQ's survival against the waves and Fubuki/Isokaze living long enough to torpedo the landing
    ships are the two knife-edges.
- **Re-time on 654daf00e** (cc9-lua64, 2026-10-17; origin/main e2b768d00 with this lane's
  kAttackTaskTargetRetireBound flip and physics13's kNeighbourClockDrawsBound ON). Export `cc9-lua52\local\l64_rt`
  (5CDE4804F2F3), reference env, `--stop-after-end 40`; logs `cc9-lua52\local\l64rt_<row>_<ref|j3>.log` with
  manifests, summary `local\l64_rt_sum.py`. "Was" is the 7a31a4fef re-time above or the row's own record.

  | Row (plan) | frames | reference | 20,3 | was (ref / 20,3) |
  |---|---|---|---|---|
  | USN01 `l64_u1_f` | 26000 | completed 410.02 s | completed 395.03 s | 410.02 / 370.03 (flipped exe) |
  | JM08 `l64_j8_e` | 110000 | **no end** | **no end** | 2142.66 / 2881.39 (flipped exe) |
  | LOMP06 `l64_l6_a` | 16000 | completed 345.84 s | completed 354.84 s | identical |
  | USN02 `s36_u2_p7` | 16000 | completed 200.06 s | completed 200.06 s | identical |
  | LOMP10 `p3_l10_b` | 36000 | completed 1048.45 s | completed 1007.43 s | 1476.87 / 1433.82 |
  | USN12 `g45_u12_e7` | 36000 | **no end** | completed 1675.51 s | 1685.52 / 1675.51 |
  | BSM01 `p6_b1_a` | 40000 | EndMission 791.93 s, guard | loss (HenryPT 1503.09 s) | 779.93 / loss |
  | PRCP01 `p9_p01_k` | 30000 | completed 796.28 s | **failed 1111.41 s** (usn03.obj_p2_fail) | 877.26 / 572.28 |
  | JM06 `p12_j6_c1` | 120000 | completed 1967.79 s | completed 2259.08 s | identical to physics12's record |
  | IJN08 `p10_i08_z` | 100000 | completed 3892.67 s | **failed 1170.02 s** (Game Over) | 3937.72 / failed 959.89 |
  | IJN18 `s46_i18_a` | 64000 | EndMission 2002.83 s, guard | EndMission 2356.17 s, guard | 2032.86 / 2216.04 |
  | BSM09 (no orders) | 36000 | EndMission 911.90 s, guard | EndMission 905.90 s, guard | 914.90 / failed |
  | ESMP08 `p12_e8_rb` | 140000 | **completed 5808.36 s**, guard | no end | physics13: 20,7 3261.21 (OCEAN_WAVE_FIELD 47.2) |

  Every completion and every winning EndMission logged the guard. New losses against the last re-time:
  - JM08 e (both seeds): landing ships survive (ref LSM 02 and LST 01; 20,3 USTroopTransport 01 and three LSTs) and
    the C47 wave never spawns; the neighbour-clock flip moves the plane fights. JM08 needs another re-plan; the two
    knife-edges are in the JM08 row note above.
  - USN12 reference: first moved death MovieBetty at 897.85 -> 897.55 s (Columbia) against `l63rt_u12_ref.log`
    (names after the roman flip differ, so `|.-2` / `|.-III` rows appear as moved). Ships/gunnery plan.
  - PRCP01 20,3: first moved death Kate1 at 186.41 -> 182.61 s (Cushing) against `l63rt_p01_j3.log`.
  - BSM01 20,3 is again HenryPT killed by Sacramento's killer_cat 6 blast (1503.09 s), then the script's own nil
    index at commandhelpers.lua:5991 (see the BSM01 note above). Two seeds now show a US ship's blast killing the
    player's PT boat near MiniSub: a gunnery question.
- **ESMP08 row: `esmp08_p12_e8_rb`** (cc9-physics13, OCEAN_WAVE_FIELD 47.2; recorded by cc9-lua64). On
  agent/cc9-physics1 57ecc76a4 (kRackConstructorRoundBound and kNeighbourClockDrawsBound ON): completed 3984.96 s
  (reference), 4707.67 s (20,3), 3966.89 s (20,5), 3261.21 s (20,7), guard on all four
  (`cc9-physics1\local\p13e8c_rb_<ref|j3|j5|j7>.log`). The 654daf00e re-time above (retire flip ON, rack round
  not yet merged) completes the reference only (5808.36 s).
- **USN15 `usn15_g50_u15_nn` is not the reference row** (cc9-gunnery50, GUNNERY_OPEN_ITEMS 269, exe f4d7658e6):
  EndMission and guard at 20,3 3229.07 s, 20,5 3289.13 s, 20,7 3068.92 s, but the reference seed and 20,11 do not
  win (Fuso, torpedo-only, survives); variant nm wins ref / 20,3 / 20,7 but not 20,5. `s45_u15_l2` stays the row.
- **Re-time on fe9811ba8** (cc9-lua65, 2026-10-17; main after physics14's kStockReturnDestroyBytesBound,
  kRotateAboutAxisImageTailBound, kRackPositiveBagRoundBound and kFreeFlightStallForceBound flips). Export
  `cc9-lua52\local\l65_cs_off` (6BABC70AC0A9: 1ebb5f062, fe9811ba8 plus this lane's switches committed OFF, gameplay
  identical to main), reference env, `--stop-after-end 40`; logs `cc9-lua52\local\l65rt_<row>_<ref|j3>.log` with
  manifests, summary `local\l65_rt_sum.py`, queue `local\l65_retime.ps1`. "Was" is the 654daf00e re-time above.

  | Row (plan) | frames | reference | 20,3 | was (ref / 20,3) |
  |---|---|---|---|---|
  | USN01 `l64_u1_f` | 26000 | **no end** | completed 420.02 s | 410.02 / 395.03 |
  | USN01 **`l65_u1_g`** (new) | 26000 | completed 435.02 s | completed 390.03 s | - |
  | JM08 `l64_j8_e` | 110000 | **failed 1988.86 s** (HQ) | **no end** | no end / no end |
  | JM08 **`l65_j8_p2`** (new) | 110000 | completed 2688.20 s | **no end** | - |
  | LOMP06 `l64_l6_a` | 16000 | completed 345.84 s | completed 354.84 s | identical |
  | USN02 `s36_u2_p7` | 16000 | completed 200.06 s | completed 200.06 s | identical |
  | LOMP10 `p3_l10_b` | 36000 | completed 1073.47 s | completed 1293.69 s | 1048.45 / 1007.43 |
  | USN12 `g45_u12_e7` | 36000 | completed 1680.51 s | completed 1670.50 s | no end / 1675.51 |
  | BSM01 `p6_b1_a` | 40000 | **no end** | **no end** | EndMission 791.93 / loss |
  | PRCP01 `p9_p01_k` | 30000 | completed 540.39 s | **no end** | 796.28 / failed 1111.41 |
  | JM06 `p12_j6_c1` | 120000 | completed 1967.79 s | completed 2256.08 s | identical |
  | IJN08 `p10_i08_z` | 100000 | completed 3006.81 s | completed 3026.83 s | 3892.67 / failed 1170.02 |
  | IJN18 `s46_i18_a` | 64000 | EndMission 2152.97 s, guard | EndMission 2306.12 s, guard | 2002.83 / 2356.17 |
  | BSM09 (no orders) | 36000 | **failed 920.90 s** (Game Over) | **failed 917.90 s** | EndMission 911.90 / 905.90 |
  | BSM09 **`l65_b9_c`** (new) | 36000 | EndMission 483.01 s, guard | EndMission 360.04 s, guard | - |
  | ESMP08 `p12_e8_rb` | 140000 | completed 3876.81 s | completed 3634.62 s | 5808.36 / no end |

  Every completion and every winning EndMission logged the guard. New plans and losses:
  - **USN01 `usn01_l65_u1_g`**: f with ConSBD1 on Convoy1 from 3301. The rack flip makes a dive spend both bombs
    (`val bomb request ... rounds=2`), so f's ConSBD1 emptied its racks on Convoy5 and had nothing left for Convoy1,
    which survived (no end).
  - **BSM09 `bsm09_l65_b9_c`**: with no orders the controlled Yorktown steams at heading 142 into the Japanese
    screen and Asagiri's torpedoes sink it at 872.26 s (Game Over). c launches Yorktown's SBDs (113) and TBDs (108) at
    Zuikaku, then Shokaku, every 600 frames from 300 and turns Yorktown back at frame 14000; a carrier at or under
    the script's hplimit fires RETREATING and EndMission (bsm_09_battle_of_the_coral_sea.lua 531-560, mtime
    2024-07-13), as in the old no-orders win. Variant d (no turn-back) ends at the same 483.01 s on the reference seed.
  - **JM08 `jm08_l65_j8_p2`**: e with the class 152/166/167 airfield launch groups from frame 12000 putting LST 01,
    LST 02, LST 03, LSM 02 first (`local\l65_j8_prio.py`), and e's lines in frames [20090, 44000) copied 5320 frames
    later (`local\l65_j8_multi.py`). Phase 1 (every landing ship dead, then PreparePhase2 -> luaBomberWave1) ends at
    1566.60 s with e on the reference seed (983 s on 654daf00e), so e's frame-scheduled intercepts missed the B-17 #2
    wave and it destroyed Headquarter 01. The LST priority brings phase 1 to 1249.74 s (wave at frame 25410), and the
    copies move the intercepts there. 20,3 still never ends:
    Fubuki and Isokaze die at 666 / 707 s, LST 01, LST 02 and LSM 02 survive, and the bombers sent at LST 01 put no
    bomb on it. Variants that lost the reference seed: s2 (copies at +11660 only), p3 (copies at +2660..+10640),
    p4 (p3 plus Mavis 02/03 at the landing ships from frame 6000; both Mavis die at 523 s).
  - **BSM01** (both seeds, same in `l65rt_b1t_ref` built from this lane's tree): HenryPT's depth charge detonates
    at 413.22 s by depth at (1465, -3007) and never hits MiniSub, whose hit listener (luaMiniSubHit) is the scripted
    kill; on 654daf00e the charge hit at 461 s. physics-lane plan: routed to the lead.
  - **PRCP01 20,3**: no end at 30000 frames (physics-lane plan `p9_p01_k`): routed to the lead.
- **Re-time on fcb6c6fa5** (cc9-lua67, 2026-10-18; main after lua66's SetDeadMeat / PutTo / DepthChargeTask / issue
  draw flips, physics17's four air switches / GenerateObject / band floor, ships57's SHIP_AI 274-283 and cc13 fix10-21,
  all OFF switches of fix13-20 still OFF). Export `cc9-lua52\local\l67_rt` (1F0D9DD17135), reference env,
  `--stop-after-end 40`; logs `cc9-lua52\local\l67r_<row>_rt_<ref|j3>.log` with manifests, summary `local\l67_sum.py`,
  queue `local\l67_runs.ps1` / `l67_queue.ps1` (at most two runs, 65 s apart). "Was" is the fe9811ba8 re-time above.

  | Row (plan) | frames | reference | 20,3 | was (ref / 20,3) |
  |---|---|---|---|---|
  | USN01 `l65_u1_g` | 26000 | completed 405.02 s | completed 415.02 s | 435.02 / 390.03 |
  | JM08 `l65_j8_p2` | 110000 | **failed 1259.15 s** (HQ) | **failed 1165.06 s** (HQ) | 2688.20 / no end |
  | LOMP06 `l64_l6_a` | 16000 | **failed 635.12 s** (Narwhal sunk) | completed 346.84 s | 345.84 / 354.84 |
  | LOMP06 **`l67_l6_b`** (new) | 16000 | completed 269.86 s | completed 269.86 s | - |
  | USN02 `s36_u2_p7` | 16000 | **failed 195.06 s** | **failed 634.97 s** | 200.06 / 200.06 |
  | LOMP10 `p3_l10_b` | 36000 | completed 987.43 s | completed 991.43 s | 1073.47 / 1293.69 |
  | USN12 `g45_u12_e7` | 36000 | completed 1395.24 s | **no end** | 1680.51 / 1670.50 |
  | BSM01 `p6_b1_a` | 40000 | no end | no end | no end / no end |
  | PRCP01 `p9_p01_k` | 30000 | completed 921.85 s | completed 854.86 s | 540.39 / no end |
  | JM06 `p12_j6_c1` | 120000 | **no end** | **no end** | 1967.79 / 2256.08 |
  | IJN08 `p10_i08_z` | 100000 | **no end** | completed 3076.88 s | 3006.81 / 3026.83 |
  | IJN18 `s46_i18_a` | 64000 | **failed 2035.86 s** (Game Over) | **failed 789.93 s** | EndMission 2152.97 / 2306.12 |
  | BSM09 `l65_b9_c` | 36000 | **failed 183.06 s** (Donald) | EndMission 327.04 s, guard | 483.01 / 360.04 |
  | ESMP08 `p12_e8_rb` | 140000 | completed 1367.41 s | completed 1372.41 s | 3876.81 / 3634.62 |
  | USN15 `s45_u15_l2` | 90000 | **failed 1257.15 s** (Game Over) | **failed 1297.19 s** | row record: EndMission |

  Every completion and every winning EndMission logged the guard. Notes:
  - **USN02** (both seeds): the AutoTarget override (SHIP_AI 274, `override 0071e7f0`) puts Houston into `attackmove`
    after every kill, so `540 moveto Houston EscapePoint` never holds; phase 2 starts at 148 s with Houston 3.5 km out
    and Exeter is torpedoed (Ushio, 190.31 s) before Houston arrives. Tried (`cc9-lua52\local\l67_u2_<v>.txt`): a, the
    moveto every 5 s (Houston stays in attackmove); c, g, h, j, k, the player takes Houston's helm at 2000-4000
    (torpedoed by Nachi 160-232 s); d, from 540 (torpedoed by Minegumo 62 s); i, Houston selected plus moveto: no
    loss in 800 s but Houston never reaches EscapePoint. Whether a player moveto should pre-empt the override is with
    the ships lane; no plan yet.
  - **LOMP06 `lomp06_l67_l6_b`**: the reference seed's Narwhal fires 8 torpedoes at Sado Maru from 455 s and misses,
    then Kitakami sinks it. b attacks Kinjosan Maru instead of Toba Maru at 3800-5800 and completes at 269.86 s on both
    seeds (guard); c (Sado Maru there) fails 628.12 s.
  - **BSM09 reference**: Zuikaku_sqn03 and Shokaku_sqn04 pick Donald (118.00 / 134.60 s, free attack override) and kill
    it at 181.71 s (primary 3, Game Over). The helm `moveto` does not steer a plane squadron (Donald keeps its scene
    moveto), and F4F launches at the fighters are refused (deck slots full, or the target hidden, 0043F080). 20,3 wins.
  - **BSM01**: as on fe9811ba8 (no end); the lead's SetDeadMeat note applies.
  - **ESMP08** now completes 2500 s earlier on both seeds.
- **Re-plans on aa5f342e6** (cc9-lua67, 2026-10-18; the re-time base plus this lane's fix13/14/17/18/20 and skill
  fan-out flips). Export `cc9-lua52\local\l67_cur` (CF275C1FB94F), logs `local\l67r_<variant>_cur_<env>.log`. The
  committed alternative plans per lost row, reference seed first:
  - **JM06: the row is now `config/win_orders/jm06_s45_j6_k.txt`** (cc9-ships45's k): completed 676.96 s (reference),
    790.93 s (20,3), 733.94 s (20,5), guard on all three. m and n1 also complete (676.96 / 790.93 s); a4 and s do
    not end; c1 still does not end on this exe. c1's cause: phase 3 (primary 2, `CVHitByPlayer`) starts at 705 s,
    I-400 #2.1 torpedoes Lexington-class 01 from 463 s and sinks it at 2294 s, and PlayerSub 01 is at (2144, -489)
    rather than c1's station when the 34001 torpedo lines fire, so no player hit ever lands and the phase never ends.
  - **USN15**: g46_u15_tl fails 1522.41 s, g50_u15_nn 1727.61 s (Game Over). On l2 Fuso alone sinks California,
    Pennsylvania, West Virginia and two Fletchers between 1122 and 1252 s (killer_cat 4 at 230-844 m): a gunnery
    question before any re-plan.
  - **IJN08 reference**: z does not capture the HQ (primary 4 `AK`: Mission.HQTable[1] Japanese) in 100000 frames;
    p9_i08_w fails 1124.97 s, s49_i08_i11 1480.32 s, s48_i08_h5 does not end. 20,3 still completes with z.
  - **USN12 20,3**: e7, g44_u12_d and s38_u12_p2 do not end at 36000 frames (d fails the reference at 869.91 s).
  - **IJN18: the row is now `config/win_orders/ijn18_l67_i18_d.txt`**: EndMission 1911.74 s (reference),
    2011.84 s (20,3), both after `Crush` (primary 3, every USNShips unit dead), the same end form as s46_i18_a's
    old win (EndScene not reached, so no guard line is expected). Why a moved: on this exe Zao, the selected unit,
    drifts from 3.2 to 9.5 km of PhaseOneGoTo between 475 and 1355 s, so phase 1 ends at 1844 s instead of 353 s
    and every JapShips unit is dead by 2035.86 s. d takes Zao's helm to PhaseOneGoTo at full throttle (phase 1 at
    333 s) and from frame 7000 sends Zao, Kita2, Asa1 and Asa4 `moveto PhaseTwoGoTo repeat 20` (luaPh2MovieEnd
    selects a random survivor; the same lines for Kita1, Asa2 and Asa3 are refused because 00645060 rejects
    them). Phase 2 ends on its 540 s timer at 873 s. Without the phase-2 moves (b: takehelm only; c: moveto repeat 20) every JapShips unit dies in phase
    2: failed 710.95 s and 884.91 s (their orders are in `cc9-lua52\local`).
  - **JM08** still fails on HQ capture with every committed plan on this exe: e 1202.10 s, w1 1312.20 s,
    r12 1130.03 s (plus p2 1259.15 s on fcb6c6fa5, before this lane's flips). LandingShip units reach
    `Headquarter 01` from 984 s. Not re-planned here; it needs the landing force sunk before 980 s.
- **Re-time on 03d3df7f8** (cc9-lua68, 2026-10-18; main after the lua67 re-plans, ships57's SHIP_AI flips and cc13
  seg67A). Export `cc9-lua52\local\l68_m1` (9F2B57BCAA24), reference env, `--stop-after-end 40`; logs
  `cc9-lua52\local\l68r_<row>_m1_<ref|j3>.log`, summary `local\l68_sum.py`, queue `local\l68_queue.ps1` /
  `l68_runs.ps1` (at most two runs, 65 s apart). "Was" is the fcb6c6fa5 / aa5f342e6 rows above.

  | Row (plan) | frames | reference | 20,3 | was (ref / 20,3) |
  |---|---|---|---|---|
  | USN01 `l65_u1_g` | 26000 | completed 405.02 s | completed 415.02 s | 405.02 / 415.02 |
  | LOMP06 `l67_l6_b` | 16000 | completed 269.86 s | completed 269.86 s | 269.86 / 269.86 |
  | PRCP01 `p9_p01_k` | 30000 | completed 647.37 s | completed 920.45 s | 921.85 / 854.86 |
  | LOMP10 `p3_l10_b` | 36000 | completed 987.43 s | completed 991.43 s | 987.43 / 991.43 |
  | JM06 `s45_j6_k` | 120000 | completed 676.96 s | completed 790.93 s | 676.96 / 790.93 |
  | ESMP08 `p12_e8_rb` | 140000 | completed 1367.41 s | completed 1372.41 s | 1367.41 / 1372.41 |
  | IJN18 `l67_i18_d` | 64000 | EndMission 1676.51 s (Crush) | EndMission 1841.67 s (Crush) | 1911.74 / 2011.84 |
  | USN12 `g45_u12_e7` | 36000 | completed 1395.24 s | **no end** | 1395.24 / no end |
  | USN12 **`l68_u12_e8`** (new) | 36000 | completed 1395.24 s | completed 1500.34 s | - |
  | USN02 `s36_u2_p7` | 16000 | **failed 195.06 s** | - | 195.06 / 634.97 |
  | USN02 **`l68_u2_a`** (new) | 16000 | completed 475.01 s | completed 460.01 s | - |
  | IJN08 `p10_i08_z` | 100000 | **no end** | completed 3076.88 s | no end / 3076.88 |
  | JM08 `l65_j8_p2` | 110000 | **failed 1259.15 s** (HQ) | - | 1259.15 / 1165.06 |
  | BSM09 `l65_b9_c` | 36000 | **failed 183.06 s** (Donald) | - | 183.06 / EndMission 327.04 |
  | USN15 `s45_u15_l2` | 90000 | **failed 1257.15 s** | - | 1257.15 / 1297.19 |
  | BSM01 `p6_b1_a` | 40000 | no end | - | no end / no end |

  Every completion logged the guard ("refused a mission script's process launch"); the IJN18 EndMission wins log it
  too on this exe. Notes:
  - **USN02: the row is now `config/win_orders/usn02_l68_u2_a.txt`.** The player stays on Houston and re-issues
    `moveto Houston EscapePoint` every 200 frames from 540 to 5140, sends Exeter to (1000, -5450) at 4400, and at 5300 takes Houston's helm (`takehelm Houston
    1.0 EscapePoint stop 200`, a role-1 take that exempts Houston from SHIP_AI 274). b and c (the take at 6000 /
    7000) complete at 495.00 / 505.00 s (reference); the orders are in `cc9-lua52\local\l68_u2_<v>.txt`.
  - **USN12: the row is now `config/win_orders/usn12_l68_u12_e8.txt`**: e7 with every MontGrp moveto to (4500, 2500)
    sent to (7600, 7000) instead. e7 has no end under 20,3; e8 completes on both seeds, and its reference time is
    e7's.
  - **IJN08 reference**: on z, Maru1 dies at 1384 s to US_Airfield_03's squadrons, Maru2's landing craft launch
    only at 3715 s, and `Shipyard Entity CB3` never dies, so the HQ is not taken in 100000 frames. Two re-plans
    (`cc9-lua52\local\l68_i08_gen.py`, through physics9's planner): a sends Aoba, Kinugasa and Furutaka onto CB3 /
    US_Shipyard_03 from 36500, b also releases the Marus at 46000. Neither ends: Kinugasa (830 s) and Furutaka
    (1355 s) are dead before 36500, Aoba alone takes US_Shipyard_03 at 2390 s instead of 2900 s and CB3 survives;
    on b all 14 LandingShip units die together at 3343.64 s. IJN08's reference stays open.
  - **JM08**: USTroopTransport 01 (unit 353) launches 8 landing craft at about 790 s near (600, -700); they land at
    1037-1075 s and take the HQ (Party 2, the failure). CheckInvasion starts the invasion at once. Five re-plans fail
    at 1222-1285 s: q1 / q2 / q3 put AP01 first in the launch order from 12000 / 8000 / 10000, r1 / r2 send torpedoes
    at AP01 (`cc9-lua52\local\l68_j8_<v>.txt`). AP01 dies too late or not at all; the craft have to die before they
    land. JM08 stays open.
  - **BSM09** on be691fb5b's base (SquadronSetTravelSpeed ON, SQUADRON_LAND_TASK 5in) fails at 222.06 s instead
    of 183.06 s with the same plan: main moved it, not the travel speed (the OFF side of that pair fails at 222.06 s
    too).
  - USN15, BSM09's reference, BSM01 and USNOS wait on the ships and physics lanes as the lead's queue says.
- **Re-time on 4b74dd6b4 / 011d651dc** (cc9-lua69, 2026-10-18; main after the lua69 flips, physics20's
  kFollowReleaseArmBound / kHelmPlaneMovetoBound and the d6c0c5ccd console fix). Tree build of `agent/cc9-lua52`;
  reference env, `--stop-after-end 40`; logs `cc9-lua52\local\l69r_<row>_main_<ref|j3>.log`, summary
  `local\l69_sum.py <glob>`, runner `local\l69_runs.ps1` (rows in `local\l69_rows.txt`). Base per row in the last
  column; "was" is the 03d3df7f8 re-time above.

  | Row (plan) | frames | reference | 20,3 | base | was (ref / 20,3) |
  |---|---|---|---|---|---|
  | USN01 `l65_u1_g` | 26000 | **no end** | - | 011d651dc | 405.02 / 415.02 |
  | USN01 **`l69_u1_a`** (new) | 26000 | completed 420.02 s | **failed 1160.01 s** (Trans) | 4b74dd6b4 | - |
  | LOMP06 `l67_l6_b` | 16000 | completed 411.82 s | **failed 443.17 s** | 4b74dd6b4 | 269.86 / 269.86 |
  | PRCP01 `p9_p01_k` | 30000 | completed 649.86 s | completed 797.38 s | 4b74dd6b4 | 647.37 / 920.45 |
  | LOMP10 `p3_l10_b` | 36000 | completed 981.43 s | completed 1343.74 s | 4b74dd6b4 | 987.43 / 991.43 |
  | JM06 `s45_j6_k` | 120000 | completed 730.95 s | completed 691.95 s | 4b74dd6b4 | 676.96 / 790.93 |
  | ESMP08 `p12_e8_rb` | 140000 | completed 1379.37 s | completed 1367.41 s | 4b74dd6b4 | 1367.41 / 1372.41 |
  | IJN18 `l67_i18_d` | 64000 | EndMission 1691.52 s | EndMission 1651.49 s | 4b74dd6b4 | 1676.51 / 1841.67 |
  | USN12 `l68_u12_e8` | 36000 | completed 1435.27 s | completed 1525.36 s | 4b74dd6b4 | 1395.24 / 1500.34 |
  | USN02 `l68_u2_a` | 16000 | completed 475.01 s | completed 475.01 s | 4b74dd6b4 | 475.01 / 460.01 |
  | BSM09 **`l68_b9_d1`** (new) | 36000 | EndMission 327.04 s, all objectives | EndMission 327.04 s | 011d651dc | failed / - |
  | IJN08 `p10_i08_z` | 100000 | **failed 1129.98 s** (Game Over) | **no end** | 011d651dc / 4b74dd6b4 | no end / 3076.88 |
  | JM08 `l65_j8_p2` | 110000 | **failed 1140.04 s** (HQ) | - | 011d651dc | 1259.15 / - |
  | JM08 `l69_j8_h` (local) | 110000 | **no end**, HQ holds | **no end**, HQ holds | 4b74dd6b4 | - |
  | BSM01 `p6_b1_a` | 40000 | no end | - | 011d651dc | no end / - |

  Every completion logged the guard; BSM09 and IJN18 EndMission runs log it too. Notes:
  - **USN01: `l65_u1_g` is lost to the faithful kFollowReleaseArmBound flip** (physics20, 9f2d1cea3). Bisect over the
    first-parent merges 03d3df7f8..011d651dc, exports with d6c0c5ccd applied (`local\l69_bs<i>`): 03d3df7f8 completes
    405.02 s, merge 2 (50c83d892, the p20 flip) and every later point have no end, and current main with only
    `kFollowReleaseArmBound=false` (`local\l69_fr0`) completes 370.03 s. With the switch ON, ConTBD2's wingmen are shot
    down instead of stowing and Convoy2 (killed by ConTBD2 at 198.71 s before) survives, so the 231 s phase switch, the
    ConTBD Kill and the Nell waves never come. **The row is now `config/win_orders/usn01_l69_u1_a.txt`**: g plus every
    Con squadron re-targeted on Convoy2 at frames 4250-4261. Reference only: under 20,3 Convoy3 survives instead, and
    the Convoy2|Convoy3 lists (b, d, e in `local\l69_u1_<v>.txt`) kill Convoy2 too late for the "Trans" objective.
  - **BSM09: the row is `config/win_orders/bsm09_l68_b9_d1.txt`** (lua67's d1: Donald held NW, `10 moveto Donald
    -2400 3200 repeat 30`), winning with kHelmPlaneMovetoBound ON on both seeds.
  - **JM08**: the HQ loss on p2 / r2 is the cc13-audited naval gunfire (Missouri never dies; the escorts' AA destroys
    the Hosho Kates before release, tries d / e / f). Try 2, `local\l69_j8_h.txt` (`l69_j8_add.py` puts Missouri at the
    top of every airfield launch group, the Hosho classes take Missouri first from 6000, every destroyer torpedo is aimed
    at Missouri), sinks Missouri at 397.83 s (ref) / 569 s (20,3) and Nevada at 603 / 640 s. No craft lands
    (`bleed_total=0`) and the HQ is never neutralized, so **the loss is not forced**. With Missouri dead before
    StartInvasion the landing force idles; LSM 02, LST 01 and LST 02 (ref) or LSM 02 alone (20,3) survive 5500 s, so
    primary 1 never completes and there is no end. h2 (those three first from 16000) is identical to h. JM08 stays open
    on the last landships.
  - **IJN08** z reference now loses both Marus (Maru2 at 689.86 s to Jap_Airfield_sqn10, Maru1 at 1126.17 s to
    US_Airfield_03_sqn22), Game Over. The one re-plan, h (`local\l69_i08_h.txt`: the Marus held at z = -14000 from 7100,
    the first frame they exist, to 56100), loses them sooner (Game Over 724.95 / 684.96 s). The IJN08 reference stays open.
  - **BSM01**: lead/cc13 plan a (`local\l69_b1_a.txt`, `6950 takehelm HenryPT 0.6 1500 -2990`) keeps HenryPT clear of
    the shoal and reaches MissionPhase 3, but HenryPT dies at 1841.97 s and `MiniSub` fails. Both a and b launch 4-6
    depth charges with `depth_detonations=0 contact_detonations=0`. Waiting on gunnery53.
  - LOMP06's 20,3 and USN01's 20,3 are new losses; USN15 and USNOS wait on the ships lane.
- **BSM01 won: `config/win_orders/bsm01_l69_b1_d.txt`** (cc9-lua69, main ec0df436c, reference env): EndMission 809.93 s
  reference / 929.90 s under 20,3, MissionStatus=true, MissionPhase 4, guard logged; MiniSub dies at 679.86 / 645.22 s
  (`luaMiniSubHit` fires: hit listener 00988510 on HenryPT's charges), HenryPT survives (no commandhelpers.lua:5991
  error). The plan follows gunnery53's lane (SHIP_AI_OPEN_ITEMS 296.3): p6_b1_a's legs to (1915,-3084), then
  `6900 takehelm HenryPT 1.0 1400 -2980` (the z -2980 lane past the x 1300-1620 shoal), `7300 takehelm HenryPT 1.0
  -430 -2990` (the sub's west point near 432 s), `8500 attack HenryPT MiniSub repeat 3` (pursuit), and a depth-charge
  press every 40 frames from 7600 to 26000. Variant c (the attack from (900,-2980) at 7400) has no end. The lead/cc13
  6950 leg plans a / b (`local\l69_b1_a|b.txt`) stay superseded. Logs `cc9-lua52\local\l69r_b1d_main_{ref,j3}.log`.
- **USN15** on ec0df436c: `s45_u15_l2` fails at 1127.02 s; the cc13 timing fix a (Fuso block moved to frame 20000,
  `local\l69_u15_a.txt`) fails at 1122.02 s and b (a with the battleships held until 20000) at 1382.27 s. On this
  main **Yamashiro no longer dies** (999.28 s before) and kills the battleships at 1.4-2.7 km, after Fubuki-class
  torpedoes (cat 7, 2.5-4 km) take Maryland / California first. USN15 needs a Yamashiro-first re-plan with torpedo
  avoidance; it stays open.
- JM06 completes at 1457.30 s with the squadron step-tail flips (physics23, main ac7bc90b6); BSM09 d1 unchanged.
- **JM08 h3** (`cc9-lua52\local\l69_j8_h3.txt`: h plus `l69_j8_add.py` lines putting LST 02, LST 01 and LSM 02 at
  the top of every launch group from frame 20000; main ec0df436c): on the reference seed **all 11 landships die by
  1290 s, with Missouri at 592 s and Nevada at 603 s**. primary 1 and 2 complete, "Run" plays, and all three bomber
  waves spawn. The HQ then falls to the phase-2 waves at 1674.10 s (failed 1674.56 s; 15 phase-2 planes shot down).
  Under 20,3 there is no end: Missouri dies at 569 s, but LSM 01-02 and LST 01-03 survive. JM08 is now a phase-2
  air-defence problem. It is not a forced loss: the cc13 naval-gunfire audit no longer applies once Missouri dies
  before StartInvasion.
- **USN01 cause, refined by cc9-physics23** (`cc9-physics1\local\p23g_old_u1g.log`, 03d3df7f8): the old 198.71 s
  Convoy2 kill was a STRAY torpedo. ConTBD2|.-III was ordered at Convoy4, missed it by 180.7 m, and ran 58.8 m past
  Convoy2. Its release came from the aim lead gate, which was true for 2 ticks only. On current main the same plane gets
  lead=0, never releases, and dies at 181.3 s to Convoy3 AA. The bisect names the kFollowReleaseArmBound merge, but the
  mechanism is a knife-edge in the aim geometry, not a broken switch, so the row stays re-planned (`usn01_l69_u1_a`).
- Open rows at the lua69 handoff: JM08 (open no-end; h3 reaches phase 2 on ref; the lead routed the idle-LST question
  to cc13), IJN08 reference, USN15 (Yamashiro-first re-plan), USNOS (ships60), USN01 / LOMP06 under 20,3. The
  kVehicleClassPreloadCc15Bound re-flip is on HOLD (cc15: Ohka class 156; pairs listed in SQUADRON_LAND_TASK 5it).

## 2026-10-19, main ab2062aec (cc9-lua70, keeper)

Tools in `J:\PROG\battlestations-pacific-decompile-cc9-lua52\local`: runner `l70_runs.ps1` (rows `l70_rows.txt`, logs
`l70r_<row>_<variant>_<env>.log`), `l70_j8_cover.py` (JM08 phase-2 wave launches), `l70_j8_keep.py` (drop a class's
airfield launches in a frame window), `l70_stockuse.py` (planes launched per base and class, first no-stock frame),
`l70_grepc.py` (line-shape counts in a line window).

- **LOMP06 won on both seeds: `config/win_orders/lomp06_l70_l6_c.txt`** (l67 b with the Kinjosan Maru lines at
  3800-5800 aimed at Sado Maru). Completed 364.83 s reference / 367.83 s under 20,3, guard logged on both. The crucial
  cargos (`luaPickRnd` at 06_crucial_cargo.lua:151-153) are Kongosan Maru and Sado Maru on both seeds; b spent
  190-290 s on Kinjosan, and under 20,3 a stray torpedo took Kuryu Maru instead, Sado survived, and Yugiri's depth
  charges sank Narwhal at 443.17 s. Variant d (Kongosan held to 3800) wins the reference (379.83 s) but fails 20,3 at
  650.11 s.
- **JM08 phase 2: the HQ now holds on the reference seed; no end.** The h3 loss was a fighter-stock problem, not
  targeting: the airfield starts with 152 (Gekko) x30, 154 (Ki-43) x30, 166 x9, 167 x9; Hosho (150 Zero x20, 162,
  158) is sunk at 465.86 s by Hawaii, so its Zeros never launch. On h3 / i the airfield had launched all 60 fighters
  at Missouri and the landships by 23500, so the B-17 wave (spawned 1310 s, first bombs on the HQ at 1514 s) met
  three planes. i (h3 plus `l70_j8_cover.py`: slot-0 launches at every wave squadron from 26215) moved the HQ loss only
  to 1692.95 s. **j** (`local\l70_j8_j.txt`: i with every class-154 airfield launch in 16000-26215 dropped) keeps 15
  Ki-43s for phase 2: all 11 landships still die (LST 03 last at 1336.03 s, to a Ki-43 squadron), the HQ survives all
  three waves, and 15 of the 20 C47 planes die by 2190.71 s. The C47 #4.2, #4.3 and #4.4 leaders (and two wingmen)
  are never shot down, so CheckPrim3 (jm08.lua, `Mission.Dakota` empty) never fires: no end in 110000 frames. Both
  fighter classes are out of stock from frame 31894. Under 20,3 i and j still lose no landship after LSM 02 (no end).
- **USN01 20,3** on ab2062aec: `l69_u1_a` fails 1160.01 s (Game Over); Convoy2/3/5 survive, ConTBD2 dies to Convoy2's
  AA at 190.96 s before its release. **IJN08** reference `p10_i08_z` still fails 1129.98 s.
- **USN15 re-plan** (plans `local\l70_u15_<v>.txt`, built by `l70_u15_gen.py` / `l70_u15_gen2.py` from l69 a). The win
  needs Fuso, Yamashiro, Nachi and Ashigara dead (usn_15 lua:388-392, 862). Main-gun kills land only at 1-2.5 km
  (killer_cat 4); HE blasts do nothing against Yamashiro's armour 130. On this seed Yamashiro heads for JapanGoTo
  (0, 12000), Fuso for JapanEdgeMove1 and the CA groups for the edges.
  - c (a plus the advance and right groups pulled back to the battleship hold at 210): Nachi, Ashigara and Yamashiro
    (1409 s) die; Fuso lives; Game Over 1462.35 s. d (c plus an escort-first block at 14000): 1237.13 s.
  - **e** (c with the hold moved north to (0, 9000) and a's Fuso block moved to 30000): **Nachi 596 s, Yamashiro
    1349 s and Fuso 1855 s die on the reference seed**, Tennessee and Mississippi survive, no end: Ashigara passes the
    northern hold and runs aground at (-11021, 7213) at 1815 s. g (every ship on Ashigara at 38000) stalls at 3.5-5 km
    (attackmove at half throttle); h (moveto (-9300, 7900) first) grounds Tennessee at (-6209, 7904) behind the shoal.
  - i / j (part of the force left at the old hold on the Ashigara lane: two BBs, or Columbia, Denver and four
    Fletchers) kill Ashigara on both seeds (743-869 s), but the Fubuki screens' torpedoes (cat 7, 2.2-7.7 km) then
    sink most battleships, and Yamashiro or Fuso survives (i: 1602.49 s ref / 1427.32 s 20,3; j: 1422.31 s ref / no
    end on 20,3). Under 20,3 g fails 1912.79 s. USN15 stays open; the next lever is the torpedo screens
    (Fubuki-class09..12 with Ashigara, 06..08 with Yamashiro).
- **JM08 is deferred (lead).** i / j above are the current state. Three pending changes rewrite phase 2 and the
  post-Missouri landing, so any plan tuned now would be invalidated: cc13's formation group
  (kDeadLeaderDirectorStateCopyBound plus kWakeDecomposeSegmentProjectionBound; in the image Helena keeps the cruise
  after Missouri dies, so the host's idle force is a host defect), the C-47 paratrooper drop (class 136, today the
  host lets them go home without dropping), and ships60's astern-skip flip. Plans must shoot the C-47s before
  they reach the HQ; a win that relies on them going home breaks when the drop lands. StartInvasion is a
  knife-edge: Stephen Potter passes 0.1-0.7 m outside the 300 m LandPoint circle (x 301.1 ref / 300.3 under 20,3).
  Re-time after the flips.
- Re-times reported to the keeper (not re-run here):
  - gunnery53, f886af750 kRoleOwnerDeviceWalkBound ON: USN01 (16000 frames, input-tool orders `g53_ga_u1b.txt`).
    Northampton's turret shots go 113/113/109 -> 95/95/90 while the player holds the seat. Deaths and death rows are
    unchanged; no end on either side.
  - gunnery53, d69a8829c kShipAiNavGateClosedArmFinalBound ON: JM08 idle still fails (HQ), 977.93 -> 958.94 s ref.
    The astern-skip flip (via ships60's batch) gives JM08 idle no end in 1500 s. e96489cd3
    kLineOfFireLandFromMuzzleBound ON: USN13 idle Game Over 834.92 -> 659.96 s; JM08 and USN07 identical.
  - physics23, 917c0b57c kFollowReleaseSeamsBound ON: BSM09 d1 EndMission 327.04 -> 351.04 s (still a win); USN21
    s46_u21_f unchanged at 2916.72 s. b473e5fcf kPlaneDeviceWalkSharedDrawBound ON: BSM09 d1 ref unchanged at
    351.04 s, default env 330.04 -> 516.00 s; USN21 2911.72 -> 2921.72 s ref, completes in both envs.
  - physics24, fb31ab760 (kDogfightFireEnvelopeBound + HoldArm, kTaskGunBlockReconstruct + BurstTimerReset): BSM09
    d1 still wins, ref 351.04 -> 348.04 s, default env 516.00 -> 462.01 s; USN21 s46_u21_f unchanged (2906.71 s ref
    / 2891.70 s default). BSM01 l50_b1_a has no end within 1200 s at 19dcd5aba OFF, independent of these flips.
  - gunnery53, 843efc336 kNavigatorForceTorpedoBound ON: JM06 `s45_j6_k` ref completes 772.93 -> 790.93 s
    (luaJM6SubShot's torpedoes fire; Lexington-class 01 sinks 706.85 s instead of 759.04 s; the Narwhal survives).
    Default env: no end on either side, identical.
  - VehicleClass re-flip pairs (cc9-lua70, base 853ae51b5, after cc15's vcp7 4360598e7, ref env, BSP_DEATH_TABLE=1,
    `local\l70r_<row>_{main,vcon}_ref.log`): pair_diff "gameplay identical" on all four. USN21 s46_u21_f completes
    2906.71 s on both sides (590 classes purged, failures 0). BSM09 d1 ends 351.04 s on both (617 purged). USNOS
    `usnos_s45_os_k` Game Over 969.89 s on both (no Ohka reached), and ships60's `s59_os_c17` (60000 frames) reaches
    the Ohka drop: 16 `drop kamikaze ... -> release` transitions from 2643.50 s on both sides, no end, exit 0, Lua
    errors 0.
  - IJN08 (cc13 audit): the Marus are lost to the enemy airfields' bombers, not to the cruisers. The re-plan needs
    fighter or AA cover over the Marus, after a re-run on the post-batch main.

## 2026-10-19, re-time on the seg67 ship batch (main 241db740b; tree build c95d107da, cc9-lua70)

Logs `cc9-lua52\local\l70r_n_<row>_main_{ref,j3}.log`. The base includes kVehicleClassPreloadCc15Bound and
kPowerupGrantBound ON. Exports with VehicleClass OFF (`l70r_n_<row>_vcoff_ref.log`) give the same result for
LOMP06, LOMP10, IJN18 and JM06, so the moves are not the VehicleClass flip.

| Row (plan) | frames | reference | 20,3 | was (ref / 20,3) |
|---|---|---|---|---|
| USN01 `l69_u1_a` | 26000 | completed 400.03 s | **completed 440.02 s** | 420.02 / failed 1160.01 |
| USN02 `l68_u2_a` | 16000 | completed 495.00 s | **failed 425.02 s** (Game Over) | 475.01 / 475.01 |
| USN12 `l68_u12_e8` | 36000 | completed 1465.30 s | completed 1335.18 s | 1435.27 / 1525.36 |
| USN21 `s46_u21_f` | 80000 | completed 2906.71 s | completed 2911.72 s | 2906.71 / - |
| PRCP01 `p9_p01_k` | 30000 | completed 787.38 s | completed 1061.21 s | 649.86 / 797.38 |
| LOMP06 `l70_l6_c` | 16000 | **failed 613.12 s** (Narwhal sunk by Asagiri; Sado lives) | completed 330.84 s | 364.83 / 367.83 |
| LOMP10 `p3_l10_b` | 36000 | **no end** | **no end** | 981.43 / 1343.74 |
| JM06 `s45_j6_k` | 120000 | **no end** | completed 682.96 s | 790.93 / 691.95 |
| ESMP08 `p12_e8_rb` | 140000 | **failed 2525.49 s** (Chiyoda) | **failed 3231.18 s** | 1379.37 / 1367.41 |
| IJN18 `l67_i18_d` | 64000 | **no end** | **no end** | 1691.52 / 1651.49 |
| BSM09 `l68_b9_d1` | 36000 | EndMission 351.04 s | EndMission 285.05 s | 348.04 / - |
| BSM01 `l69_b1_d` | 40000 | **no end** | **no end** | 809.93 / 929.90 |
| USN15 `l70_u15_e` | 90000 | failed 1602.49 s | failed 1662.55 s | no end / - |
| USN15 `s45_u15_l2` | 90000 | failed 1081.98 s | - | 1127.02 / - |
| USN04 idle | 9000 | no end | no end | - |
| IJN08 `p10_i08_z` | 100000 | Game Over 2060.89 s | no end | failed 1129.98 / no end |

Every completion logged the guard. New losses on this base: LOMP06 ref, LOMP10, JM06 ref, ESMP08, IJN18, BSM01 and
USN02 20,3. USN01 20,3 is won again.

Re-plans on the same tree build (c95d107da):

- JM06 `l70_j6_n` (s45 k with the first PlayerSub 01 moveto at frame 11000 instead of 13000): completed 817.92 s ref /
  682.96 s 20,3. Variant m (torpedoes at the Narwhal) completed 2175.00 s ref / 1538.38 s 20,3. JM06 is won again.
- LOMP10 and IJN18: every attackmove unit in mode navigate_astern keeps d32c = 0 for the whole run on this base
  (LOMP10 Ashigara 3113 of 3113 steps, IJN18 Gear1-4 and Oma4-7), and its throttle swings between 0.5 and -0.625.
  On the l69 base the same units logged a closing d32c (Ashigara 12207 m at frame 10, 1448 m at 18000), and CB4 sank
  Asashimo, Sugi and Ashigara. The mode-3 skip (kShipAiNavigateAsternSkipBound, game_hosts_ship_ai.cpp ~10990) never
  reaches the 009EE765 output, so these leaders never close. An astern-skip OFF control is running.

## 2026-10-19, later bases (cc9-lua70, keeper)

The astern-skip stall in the section above was an interim: kShipAiNavigateAsternSkipBound went OFF on main 1258eafc3.
cc13 rev4 found the skip byte-faithful; the defect is the host's mode-3 approach steering.

cc15's run on main 88b7eb3d1, ref env, astern skip OFF:

| Row | 88b7eb3d1 ref |
| --- | --- |
| USN01 | completed 1200.05 s |
| USN12 | completed 1395.24 s |
| PRCP01 | completed 915.10 s |
| USN21 | completed 2906.71 s |
| ESMP08 | completed 1314.31 s (won again) |
| LOMP10 | completed 991.43 s (won again) |
| JM06 | completed 1658.49 s |
| BSM09 d1 | EndMission 285.05 s (log J:\PROG\battlestations-pacific-decompile-cc15-flip\local\fl3_b9_inbuild.log) |
| USN02 | failed 170.06 s |
| LOMP06 | failed 653.11 s |
| IJN18 | failed 525.00 s |
| BSM01 | no end |

- BSM09 d1 ref depends on the base: 351.04 s at c95d107da, 408.02 s at f10dce645, 285.05 s at 2e21536b2 and 88b7eb3d1.
  The ref and 20,3 columns of the re-time above are not swapped.
- BSM09 d1, default env: lost at 141.05 s after the land chain went ON (physics24 282567f82). Donald collides mid-air
  with a D3A Val; this is an RNG-coupled shift. The ref env stays at 285.05 s.
- USN01 u1i, default env: loses its 490.00 s completion under the release-timer reseed (3df710613, RNG-coupled).
- USN21 with NO orders completes ("Center force is retreating!") at 3006.81 s (gunnery54).
- USN02 l68_u2_a fails at 330.04 s, Game Over on Witte, on dc6d0d1a3 ref (gunnery55).
- BSM01 is REGRESSED-pending.
  - Cause (cc13): luaMiniSubHit fires only on a HenryPT hit on MiniSub. MiniSub grounds at the surface in the shallows
    (faithful), out of reach of HenryPT's d park point, and depth charges never reach the depth detonation there.
  - Ref-env re-plans that take HenryPT onto the sub with depth charges pressed:
    - local\l70_b1_j.txt (onto (-212,-2388)) completes at 752.94 s. A Depthcharge hit fires the listener; the sub dies
      at 596.68 s.
    - local\l70_b1_k.txt and l70_b1_l.txt complete at 926.90 s.
    - None of the three ends at 20,3, where the sub sits at (-222,-2402).
  - gunnery55 reports that b1d itself ends at 812.93 s on dc6d0d1a3.
Re-plans on merged main d175c1aac (cc9-lua70):

- USN02 `l70_u2_b` (config/win_orders/usn02_l70_u2_b.txt), a CANDIDATE only: the USN02 hold stands until cc9-bisect1 reports,
  and this is not a re-time. The plan is l68 a, plus Houston's artillery fired through the HUD path
  (input 0x9D, aim unit Haguro then Nachi, input 0x99 held 3000 frames twice).
  - The script-selected Houston's player-owned guns are otherwise silent: 0 shots and about 5,900 refusals on dc6d0d1a3.
  - With the plan: ref completes at 500.00 s, guard logged; Houston fires 214 main-battery shots (52 hits).
  - 20,3: Game Over at 145.05 s; Yukikaze torpedoes Houston at 142.70 s, before the first aim line has much effect.
- IJN18 row moved to `l70_i18_r` (config/win_orders/ijn18_l70_i18_r.txt) with the three switches flipped ON on
  agent/cc9-lua52; the lead lands it after a reach check. Trial (local\l70_i18_r.txt; export with kHarnessReleaseGunMapBound, kCruiseLatchedPlayerArmCc13Bound
  and kHelmPrepareMapUpGateCc13Bound ON; releasegun Zao at frame 1500):
  - ref: EndMission 1685.52 s with MissionStatus=true, guard logged. Zao fires 163 main-battery shots (111 hits),
    against 0 in the baseline, and lives to 507.85 s instead of 333.89 s.
  - 20,3: Game Over at 839.92 s. The early Zao kills are the same; Asa1 dies at 772.38 s and Asa4 at 838.82 s.
  - With the switches OFF the releasegun line is refused, and IJN18 d has no end.
  - Runs: rg0 (d plan, main build) no end, Zao dies 333.89 s; rg1 (trial export) above; rg2 (trial plan, switches OFF)
    identical to rg0. Logs local\l70r_rg{0,1,2}_*_ref.log and l70r_rg1_rg_j3.log in the cc9-lua52 tree.

Drift list, observed with cause pending (cc15, ref env on 46bdfac0a, OFF = ON). These are not re-times:

| Row | Observed on 46bdfac0a |
| --- | --- |
| USN01 | completed 1079.93 s |
| USN02 | failed 330.04 s (held; cc9-bisect1, good 241db740b 495.00 s) |
| USN12 | FAILED 859.91 s, Game Over (was completed 1465.30 s) |
| PRCP01 | completed 814.92 s |
| USN21 | completed 2941.74 s |
| ESMP08 | completed 1274.27 s |
| LOMP10 | completed 1303.70 s |
| JM06 | completed 898.90 s |
| BSM09 | EndMission 282.05 s |
| BSM01 | EndMission 812.93 s |
| LOMP06 | failed 668.11 s |
| IJN18 | no end |

Known cause so far: the astern-skip OFF interim step (070630984) explains USN02 and the JM08 HQ drift
(cc13, pending cc9-bisect1's confirmation).

Pending re-times from lane flips (each needs a re-time after it lands):
- physics25 5481a7ca5: USN01 u1i 1195.04 -> 1069.92 s; IJN08 Game Over 2611.42 -> 2511.32 s. USN21, BSM01 and JM08
  play the same with it. JM08 HQ dies 1031.93 s on l65_j8_p2 and 981.93 s on l70_j8_j.
- ships61 05cb6f1bc (navigation set): LOMP10 1303.70 -> 1050.45 s on the seeds+sync pair build. Every row moves.
- gunnery55 7b35dcea3 (heading wrap): JM06 j6_n 808.93 -> 811.93 s; j6_k 898.90 -> 889.91 s; IJN18 d no end either way.