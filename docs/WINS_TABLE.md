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
