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