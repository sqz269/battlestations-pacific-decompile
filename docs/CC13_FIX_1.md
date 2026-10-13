# CC13 FIX 1: verified host divergences behind OFF switches

Packet cc13-fix1 (cc13 lineage, Claude), under cc9's HOST FIX TERMS of 2026-10-13. Each fix:

- re-reads the image bytes before the edit (disk bytes through `bsp.py disasm-raw` / `ghidra bytes`);
- sits behind its own `inline constexpr bool k...Bound = false;`, and changes nothing while OFF;
- has one commit carrying the prediction;
- passed `./scripts/build.ps1`, including ctest (2/2).

Status: **build-tested** only. Nothing here is game-validated, and no game runs were made. cc9 pairs
each switch and decides whether to flip it.

## Fixes

| switch | file:line | sha | prediction ON | RNG impact |
| --- | --- | --- | --- | --- |
| `kAiArrivalSpeedGuardBound` | include/bsp/ai_planner_tails.hpp:98, src/ai_planner_tails.cpp (`ai_tail_unit_arrival_value`) | 830078055 | Outside a CommandBuilding's capture range, a unit faster than 0.1 contributes 0 (`00A03840 JA`). The Capture a/b terms then count only units inside the range. Capture target choice changes wherever ships were approaching. Missions without CommandBuildings are unaffected. | none |
| `kCloseAttackBestSeedBound` | include/bsp/ai_close_attack_tick.hpp:155 | 54c5659e0 | The best-score seed is -1e10 (`[00CE4ADC]`). An admitted candidate scoring 0 (beyond FarDist, or a zero-weight member of the target group) now gets a SetTarget/AttackMove order instead of the 0.4/0.6 fallback MOVETO. Applies to CLOSEATTACK/DEFENDPOSITION, the two host callers at game_hosts_ai.cpp:2130 and :2157. The commit message also names RETREAT's prelude, which the host does not route through this tick. | none in the pass |
| `kCloseAttackSquadronHomeBound` | include/bsp/game_hosts_ai.hpp:111, src/game_hosts_ai.cpp `close_member_squadron_excluded` | 5fcfb4ac2 | A squadron member that passes 007EDA90 also goes through `009FFEB0` (`00A1441B`). With `kAiSquadronRtbExclusionBound` (already true), empty level, torpedo and dive bombers in close-attack groups are sent home rather than ordered. Expect more RTB orders in carrier missions. | none in the gate |
| `kCloseAttackCurrentTargetRadiusBound` | include/bsp/ai_close_attack_tick.hpp:169, src/ai_close_attack_tick.cpp | 65e0c172a | An enemy current target between r and sqrt(2) r of the centre is appended to the pass's candidate list (`00A14604`-`00A1464C`), and it stays there for later members. Expect fewer target switches and fewer fallback movetos. The `vt[5Ch](2)` test is taken as true (labelled). | none |
| `kOrderAttackCautiousGatesBound` | include/bsp/game_hosts_ai.hpp:120, src/game_hosts_ai.cpp `order_attack` | 3adb71b5d | Ship-led groups never take CAUTIOUSATTACK. Other groups take it only when the target is beyond CautionMove_Dist (8000) and the midpoint's net danger is > 0, and then with probability 1 - aggressive (`00A2CC2E`-`00A2CC65`). The ON path wires the existing `ai_attack_arm_00a2cbd0` and `ai_should_route_cautiously_00a2c9f0`. | Uses the host's **private** `Impl::rng` LCG, not stream 0. It draws one fewer time per order that fails a gate, which shifts that LCG's later users. The image draws stream 1. |
| `kAiBulletClassGateBound` | include/bsp/ai_tuning_globals.hpp:214, src/ai_tuning_globals.cpp `ai_bullet_type_class_gate_009fe270`, src/game_hosts_ai.cpp `class_gate` | a251a861e | DepthCharge (0Bh) gets accuracy only against class 8 and Paratroopers (0Fh) only against class 1Ch (`009FE44A`, `009FE465`). Depth-charge carriers score surface ships and planes lower. Applied only at the two scoring users. The census and diag sites are left alone. | none |
| `kProjectileGravityImageBound` | include/bsp/projectile_impact.hpp:32 | 114e04b47 | g = 9.8100004196 (`00CF9058`, (double)9.81f) instead of 9.81038. That is about 0.12 m and 1 ms over a 3000 m fall, and shells land a few cm long. It affects every projectile_flight_step user, including game_hosts_gunnery.cpp:9265 and :8196 through the shared header. Every same-binary pair drifts by the last bit. | none directly; moved hits can shift later stream-0 draws |
| `kCaptureCurrentTargetEpsilonBound` | include/bsp/game_hosts_ai.hpp:126, src/game_hosts_ai.cpp `capture_plan_00a29fd0` | 649c97735 | The epsilon is 1e-10 (`00A2A5C0`), not the literal 1e-11. Only d^2 in (1e-11, 1e-10] changes, and there the weight is flat, so nothing observable is expected. Parity only. | none |

## Skipped or not applied

- **00A13B60 items 1-3** (the five world-registry lists, the per-list team rule, list-order ties).
  The host's candidate set is `units.count()`. Squadrons are indices past that, and no host producer
  exists for list membership or the `+5Ch/+5Dh/+5Eh/+60h` flags. Doing these needs producer work
  rather than a switch. Item 7 (sticky only for kind 2 on the enemy team) was not part of this packet.
- **GPR_48 R1 / R2** (rate-law crossing floor, BC4h yaw gain). The floor function sits in
  plane_control_rate.cpp (unleased), but `sp`, the per-axis gain and the yaw gain all come from the
  caller at **src/game_hosts_units.cpp:33486/33511**, which cc9 leases (agent/cc9-lua52). A
  library-only switch would do nothing, or would be wrong, without the caller.
- **GPR_47 section 3** (007D0B80: the controller gate on the 63h arm, the engine-fire and model-less
  spin arms skipping the tail, the C10h overwrite). The model plane_death_modes.cpp is unleased, but
  its only caller is **src/game_hosts_units.cpp:17337**, which is leased. ON cannot change behaviour
  without the caller passing the gate and consuming a "reaches tail" output.
- **GPR_53 section 8.2 items 1, 1b, 3, 5** are all in **src/game_hosts_gunnery.cpp**, which
  agent/cc9-gunnery36 leases. Not edited.
- **GPR_69 item 5 (00CE3820)**. include/bsp/ai_planners.hpp:218 and ai_planner_tails.hpp:68 already
  hold 1.0e-10 (5e6b6a083, unswitched). Only the remaining literal was switched (649c97735).
  `near_radius_squared()` returning 9e6 (game_hosts_ai.cpp:5449) is a separate, labelled divergence
  and was not touched.
- **GPR_69 section 7.12 items 3, 4, 5, 7, 8 and 9** were not in this packet's list and were not attempted.
