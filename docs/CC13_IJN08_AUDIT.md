# IJN08: the cruisers on the Maru lane (cc13-ijn08 audit, 2026-10-19)

This is a read-only audit for cc9 lead order 4: on the reference seed, the IJN08 order file `p10_i08_z` fails at
1129.98 s because both Marus are lost. The question is whether Kinugasa and Furutaka behave as the binary would on the
Maru lane, or whether a host gap drives them. No game runs and no host edits were made for this audit.

## Evidence

- Log: `J:\PROG\battlestations-pacific-decompile-cc9-lua52\local\l70r_w_i08_main_ref.log`, mtime 2026-10-19 00:24:36
  local time, SHA-256 `DB6C3403CE1F1F854AE33F9145D09E50694E37EDC7EC2E5996D2483BFE753513`. This is the newest IJN08 log
  in any tree. The run started at 23:49 on 2026-10-18 (`.log.inputs`).
- Exe: the cc9-lua52 tree's `build\win32\Release\bsp_game.exe`, built at 22:44:59. That tree fast-forwarded to
  **ab2062aec** at 22:43:33 and had no other commit before 00:25, so the exe is ab2062aec.
- Orders: `config/win_orders/ijn08_p10_i08_z.txt`, 2700 lines.
- Mission script in this installation: `scripts/missions/ijn/ijn_8_north_sol.lua` (34120 bytes, mtime 2024-07-13). The
  log loads it as `Scripts/missions/Ijn\ijn_8_north_sol.lua`.
- Earlier cc13 advice: `docs/SQUADRON_LAND_TASK.md` 5iq/5it item 4 says "keep Kinugasa / Furutaka off the Maru lane
  (CruzaLeft/Right) or behind Kongo / Hiei". `docs/WINS_TABLE.md` lines 901-906 and 959-961 cover the z loss.

### Switches that changed after the run's exe

Running `local\ijn08_switchdiff.py` on ab2062aec and origin/main e690aecc0 shows these flips that affect this row:

| switch | at ab2062aec | on main | commit |
| --- | --- | --- | --- |
| kShipAiNavigateAsternSkipBound | absent, so the OFF path ran | **ON** | e73b7450f |
| kShipAiNavGateClosedArmFinalBound | absent, so the OFF path ran | **ON** | d69a8829c |
| kLineOfFireLandFromMuzzleBound | absent, so the OFF path ran | **ON** | e96489cd3 |
| kPlaneDeviceWalkPerRackBound, kPlaneDeviceWalkSharedDrawBound, kRoleOwnerDeviceWalkBound | OFF or absent | ON | 917c0b57c, b473e5fcf |
| kFollowReleaseSeamsBound, kDiveBombGoawayFloatStoresBound | absent | ON | 917c0b57c |

**The newest IJN08 log therefore does not show current main.** No IJN08 run exists yet on e73b7450f or later.

## What the mission does (ijn_8_north_sol.lua, this installation)

- `luaSpawnPh2Units` (lines 1095-1276) builds two Japanese columns. On the left, Kongo leads Haruna, Aoba, Furutaka,
  Kage1/2, Mine1/2 and **Maru1**. On the right, Kirishima leads Hiei, **Kinugasa**, Kako, Kage3/4, Mine3/4 and
  **Maru2**. Each unit runs `JoinFormation(unit, leader)`, `NavigatorSetTorpedoEvasion(true)` and
  `NavigatorSetAvoidLandCollision(true)`. The Marus get no order of their own; they only follow their leader.
- The US cruiser groups CruzaLeft (LeftNO1/2 and LeftDD1-5) and CruzaRight (RightNO1/2 and RightDD1-5) get
  `NavigatorAttackMove(leader, JapTrans[1 or 2])` (lines 1239 and 1272). Every 210 s, `luaSpawnElcos` (lines 928-977)
  spawns an Elco PT boat at each Allied shipyard and `NavigatorAttackMove`s it at a random Maru.
- In phase 2 (lines 541-567), every Allied-held airfield runs `luaAirfieldManager` with fighters {135}, bombers
  {118, 325} and the live `Mission.JapTrans` as its targets. "Jap_Airfield" is one of these airfields; it is
  Allied-held, so `Jap_Airfield_sqnNN` squadrons are enemy aircraft.
- Fail rule (lines 573-579): `luaMissionFailed` runs when `luaRemoveDeadsFromTable(Mission.JapTrans)` is empty, which
  ends in `luaMissionFailedNew(HQ, "Game Over")`.

## Timeline (reference seed, 20 frames per second)

| time | event | log |
| --- | --- | --- |
| 351.5 s (frame 7030) | Phase 2 units exist. Maru1 starts at (-1000, -11700) behind Kongo and Maru2 at (3000, -11700) behind Kirishima; Kinugasa is at (2500, -11250) in the right column, Furutaka at (-1500, -11250) in the left | L64828-L64997 |
| 355 s | The Allied airfields start their free attack. From 403 s, Jap_Airfield_sqn07 has level-bomb tasks (009B9030) on Maru1 and Maru2 | L66409, L74175, L75906 |
| 450 s (frame 9000) | The order file sends Kongo and Kirishima, the column leaders, to attack airfields. The Marus keep `state=follow` at throttle 1.25 behind them (Maru2 at heading -0.49) | L88695-L135395 |
| 460-462 s (frames 9200-9241) | **The order file**, not AI, sends Furutaka and Kinugasa to `attack` Jap_Airfield and then US_Airfield_01. Each order runs 005FAAE0 -> 0077C470(unit, 2, 0), 0077D600(attackmove) | L87709 |
| 464.7 s | Maru2 takes its first damage | death row |
| 462-730 s | Kinugasa attack-moves on US_Airfield_01 in `mode=navigate_astern` (mode 3) and shells buildings near it. Its hull holds `target=0.4776` while the approach heading is `desired_1d8=-0.3142` | L88231-L145050 |
| 618.3 s | A level-bomb stick from Jap_Airfield_sqn10 hits Maru2 at about (2420, -9770): 11 blasts of bullet 72, 36 each, health 2933 -> 1244. Flooding starts (first_water_t 618.27). Strafing adds 161 category-0 hits | L123596-L123897 |
| **689.86 s** | **Maru2 sinks**, killed by Jap_Airfield_sqn10\|.-II (category 10, blast). Water 33287 against a capacity of 9000 | L137935, L235222 |
| 730.6 s | Kinugasa's fire target changes from US_Airfield_01 to RightDD5 (CruzaRight). It takes its first damage | L145758 |
| 745-747 s | An Elco fires torpedoes at Kinugasa. From 747.2 s, Kinugasa makes 319 torpedo-evasion overrides | L148456, L229079 |
| 748-833 s | Kongo, Haruna and Kirishima destroy CruzaLeft | death rows |
| **816.17 s** | **Kinugasa sinks** near the coast. The killer is "Coastal Gun, Japanese 02" at 1101 m; RightNO shells (bullet 9) did most of the damage before that. Flooding started at 730.6 | L162230, L235221 |
| 1040.9 s | Maru1, which followed Kongo about 9 km north to about (-2470, -2790) near US_Airfield_03, is hit by bomb sticks from US_Airfield_03_sqn22 | L200967 |
| **1126.17 s** | **Maru1 sinks** (US_Airfield_03_sqn22\|.-II, category 10) | L215004 |
| **1129.98 s** | `mission end: ... failed "Game Over" entity="HQ"` | L216961 |

Furutaka is still alive at the end: at 1080-1155 s it is attack-moving on US_Shipyard_02, about 7 km from it.
Kinugasa and Furutaka never kill or damage a Maru. Both Marus are sunk by the airfield squadrons that the Lua script
sends at `Mission.JapTrans`.

## Decisions, host sites and verdicts

| # | decision | host site | binary | verdict |
| --- | --- | --- | --- | --- |
| 1 | The cruisers and Marus join their columns | `follow issued` (game_hosts_commands.cpp:4474), formation column (game_hosts_units.cpp:40960); kFormationJoinFollowBound ON | 0077F940 -> 00720CD0; follow step 009E1610 | **FAITHFUL** |
| 2 | Kinugasa and Furutaka leave the Maru lane | the harness `attack` line through 005FAAE0 (`helm order applied`) | 005FAAE0 -> 0077C470(unit,2,0), 0077D600(00E08F78, target, 1) | **FAITHFUL**. This is player input from the order file, not AI |
| 3 | The Marus follow their leaders toward the airfields | ShipAi follow (`state=follow`), ship_ai_follow_step_009e1610; kShipFollowStationPointBound ON | 009E1610, 009DF2D0 | **FAITHFUL**. The order file moves the leaders and the Marus follow them in column |
| 4 | Attack-move approach steering (mode 3) for Kinugasa, Furutaka, CruzaLeft/Right and the Elcos | game_hosts_ship_ai.cpp:10985 (`navigate_astern_skip_009ee756`) | 009F3240 writes 1C4h = 3; 009ED6B0 at 009EE756 JNE 009EF206 skips the output block 009EE765..009EEAA2 | **GAP in this run**. The exe predates the switch, so the OFF output block steered every attack-moving ship. Kinugasa's hull held the output block's `target=0.4776` while the approach asked for -0.3142; Furutaka shows the same at 1080 s (`target=5.93`, `desired_1d8=-0.105`). The switch is **ON** on main (e73b7450f, audit in SHIP_AI_OPEN_ITEMS 300 §2) and **LIVE** for this row |
| 5 | Closed navigation gate | kShipAiNavGateClosedArmFinalBound | 009EE59F JZ 009EF206 -> 009DE5B0 | **GAP in this run**, **ON** on main (d69a8829c) |
| 6 | Torpedo evasion while Kinugasa is in the Elco torpedo field | the torpedo-track and commitment-band code of 009E04E0; ship_ai_seg67_switches_cc13.hpp (I) and cc13_ship_ai_commit_band.hpp (G) | 009E08EF (margin from the class Length times 0.6), 009E110F..009E112E (quarter band) | **GAP, still OFF on main and LIVE here**. kShipAiTorpedoMarginClassLengthBound and kShipAiCommitmentBandQuarterBound are OFF. Kinugasa made 319 evasion overrides between 747 s and its death at 816 s. The log cannot show whether these arms changed its course |
| 7 | Other ship-nav arms | (C) kShipAiStationLatchClearBound, (D) kShipAiParkedByteBeforeTailBound, (F) kShipAiClearanceRefreshSentinelBound, kShipAiApproachTimerSeedBound: all OFF | 009EE586, 009EE6F1, 009EF96F, 009E5625 | **UNSURE**. All four are known OFF mismatches and are LIVE for every ship. The approach seed only makes the first approach pass one pass late |
| 8 | Gun target: the ordered installation | command_execution.cpp:361 (00835DC2 attack-move gate -> set_fire_target) -> store_fire_target 00836240 | 00835DC2, 00835860, 00836240 | **FAITHFUL** (`fire=US_Airfield_01`) |
| 9 | Gun target changes to RightDD5 at 730.6 s | no log line names the writer. It is not an AutoTarget override (`override ...: Kinugasa` never appears) | 00836240 through an unlogged caller | **UNSURE** |
| 10 | Shelling over land, and the coastal guns' line of fire to Kinugasa | 0072CDD0 landscape query | 0072CDF1 / 0072CE8E (the muzzle is not raised) | **GAP in this run**, **ON** on main (e96489cd3) |
| 11 | The Marus are sunk by the airfield squadrons | Lua `luaAirfieldManager(..., JapTrans)`; level-bomb task 009B9030 (packet cc9_plane_level_bomb_task); rack walk 007C0D90 | 009B9030, prediction 009C7D71 | **FAITHFUL to the script**. The bombs land 12-41 m from the 009C7D71 prediction, and no level-bomb mismatch shows in this log. The plane device-walk and follow-release flips made after the run (917c0b57c, b473e5fcf) may change these strikes on main |

## Verdict

- **The cruisers do not cause the Maru losses.** The Lua script sends the Allied airfield squadrons at
  `Mission.JapTrans`, and level bombs plus the flooding they cause sink both Marus. Maru2 was hit at 618 s and sank at
  689.86 s; Maru1 was hit from 1041 s and sank at 1126.17 s. Kinugasa and Furutaka leave the lane at frames 9200-9241
  because the order file tells them to, and the Marus go north because they follow Kongo and Kirishima, which the order
  file also sends at the airfields. The previous cc13 advice to keep the cruisers off the Maru lane does not apply to
  this loss: the cruisers never meet the Marus' killers.
- **The run's cruiser behaviour was not faithful.** At ab2062aec, every attack-moving ship ran the OFF navigate-astern
  output block (decision 4) and the OFF gate and line-of-fire paths (decisions 5 and 10). That covers Kinugasa,
  Furutaka, both US cruiser groups and the Elcos. All three switches are **ON on main now**, so the gap is fixed and
  the fix is LIVE for this row. The current z result is unknown until IJN08 z is re-run on main at e73b7450f or later.
- **Still open on main, and LIVE here:** torpedo evasion (I) kShipAiTorpedoMarginClassLengthBound and (G)
  kShipAiCommitmentBandQuarterBound. Kinugasa uses these arms in the Elco torpedo field. They do not touch the Marus'
  air losses.
- **For the plan:** keeping the Marus alive needs AA cover or fighter cover against the Jap_Airfield and US_Airfield_03
  bombers, or leaders that do not take the Marus toward the airfields. Moving the cruisers will not do it.

Scratch tools are in `local\ijn08_*.py` in this tree.
