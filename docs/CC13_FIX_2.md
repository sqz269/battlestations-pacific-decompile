# CC13 FIX 2: AI behaviour divergences behind OFF switches

Packet cc13-fix2 (cc13 lineage, Claude), under cc9's HOST FIX TERMS of 2026-10-13. It continues
docs/CC13_FIX_1.md with the AI items of docs/CC13_GPR_69.md section 7.12 (items 3, 4, 5, 7 and 8)
and the two section 7.7 models. Each fix:

- re-reads the image bytes before the edit (`bsp.py disasm-raw`, with the addresses cited in the
  switch comment and the commit message);
- binds the existing cc13 model (src/gameplay_seg69_70_cc13.cpp, src/band_009f_cc13.cpp) instead of
  writing a second copy;
- sits behind its own `inline constexpr bool k...Bound = false;` and changes nothing while OFF (no
  new log line either);
- has one commit carrying the prediction;
- passed `./scripts/build.ps1`, including ctest (2/2).

Status: **build-tested** only. Nothing here is game-validated, and no game runs were made. cc9 pairs
each switch and decides whether to flip it.

## Fixes

| sha | switch | file:line | prediction ON | RNG impact |
| --- | --- | --- | --- | --- |
| e3dc014c4 | `kAiFormationShapeBound` | include/bsp/ai_command_tick.hpp:206; flags in src/ai_command_tick.cpp (MOVETO, MOVETOATTACK closing arm, PATROLTO, IDLE), the SELLING ground arm in src/game_hosts_ai.cpp, and the run at src/game_hosts_ai.cpp:2138 | 00A11070 runs at the image's call sites (00A12538, 00A12BE0, 00A15665, 00A12442, 00A12350; 00A12A48 through the REGROUPINGMOVE switch). Each ship-led group with a formation (+284h) under those commands gets column 0 rewritten every tick: shape 0, then the 24 box slots `-(x, z) * Formation_UnitDist` for later ship members with a record. This affects most campaign missions with AI ship groups, because followers keep station on the box slots instead of the join offsets. | none |
| 3601d8e3a | `kDefendThinkImageBound` | include/bsp/game_hosts_ai.hpp:136; src/game_hosts_ai.cpp:4963, :5019 | Defend planner only. Members of the team's objective set ([00E188A8]+team*4+21A4h) become defend records (00A2DEF0's second arm). With two or more records, the 00A28300 order (0 < T and higher weight first) drives the anchor tie-break and the spawn-tail record. Nothing changes while the objective sets are empty. | none |
| 4e53dcffd | `kCaptureNoTargetImageBound` | include/bsp/game_hosts_ai.hpp:146; src/game_hosts_ai.cpp:4012 | Capture planner with no enemy CommandBuilding. The clock (+38h/+3Ch/+40h) advances on every think, so the spawn ramp is further along when a target first appears. Each group is handed off by its own nearest own list-28 entity outside the group. This differs from the host only when a group holds the only own entity, or when that entity is inactive. | none |
| 0a6b42ca9 | `kAiDuelEscortThinkBound` | include/bsp/game_hosts_ai.hpp:161; src/game_hosts_ai.cpp:3744 | Effective game modes 4 and 5 only; no campaign mission is affected. Duel moves toward a blend of the two centroids until an enemy group is within 4000 or the own team has one group, then attacks the nearest. Escort follows the team's objective set. | 00A2CCF0 draws stream 1 once per call that passes the ship and 00A2C9F0 gates. The draw goes to the host's **private** LCG (`random_00bd2f10`), as order_attack's does. With ratio 1.0 the result is always MOVETO. The replaced 00A1CB80 path's order_attack draws disappear when a think moves or patrols instead. No stream-0 change. |
| 8489f5844 | `kMergeNewAttackCommandBound` | include/bsp/game_hosts_ai.hpp:154; src/game_hosts_ai.cpp:1925 | Merges (00A2DB80) install a new command of the same class (7/8/9) on the absorber instead of rebinding. Only CAUTIOUSATTACK's +24h/+28h reset (0/4). No order change is expected while the host's 00A14DD0 has no route slot. `commands_replaced` rises. | none |
| 1bf6e3260 | `kAiRegroupingMoveTickBound` | include/bsp/ai_command_tick.hpp:215; src/ai_command_tick.cpp:387 | REGROUPINGMOVE (00A126C0) steers the leader to `(dest + 3 * centroid) / 4` while a member trails beyond AutoMerge_LeaveDist. Then come 00A02020, 00A10DC0 and 00A11070. Only Lua installs REGROUPINGMOVE, and none of this installation's 621 .lua files names it, so no mission is expected to change. | none |

The spawn chain (00A0D1D0 / 00A163D0 stream-1 draws, `[capture]` / `[defend]` / `[duel]` /
`[escort]` quick-spawns) stays a record in this host. None of these switches adds a draw from
00A0D1D0 or 00A163D0. Only `ijn_11` and `ijn_02_force_z` author stock, and the spawn arms they
would reach remain records.

## Corrections to docs/CC13_GPR_69.md

- 7.10 says the merge's new commands reset "the member-pass schedule". They do not. 00A2BD00
  (00A2BD00-00A2BD2E) only deletes the old object and stores the new pointer. The schedule is the
  group's +5650h, which neither 00A2BD00 nor the constructors touch.

## Skipped or partly applied

- **00A2CCF0 on its own (item 6b).** Its only caller is the Duel think. It is bound inside
  `kAiDuelEscortThinkBound` and has no separate switch.
- **00A29FD0 item 2 (no activity filter on targets).** Not in this packet's list. The host's
  `have_target` test keeps `unit_active`. With `kCaptureNoTargetImageBound` ON, an inactive enemy
  list-28 entity still becomes a target inside `capture_plan_00a29fd0` (labelled in the commit).
- **Above 32 Defend records.** The 00A26CD0/00A256B0 partition path was not read, so the build order
  stays.
- **Squadron members in 00A2DEF0's set test** are not tested, because the host's record scorer reads
  units only.
- **00A11070 past 24 slots.** The image reads past its table. The model stops, and the overrun is
  counted (`formation_shape_overruns`).
- **MT_FORMATION_SET** (0077A080 / 0077C880) is not sent, as for 00A11690. Single player has no peer.

## Handoff

All six switches are OFF. To pair one, flip it alone and run the usual same-binary pair. These are
the expected signatures:

- `kAiFormationShapeBound`: the summary line `ai formation shape` appears, and follower ship
  positions move.
- `kDefendThinkImageBound`: the `defend_records_seen` counts change only on missions with objective
  units.
- `kCaptureNoTargetImageBound`: `capture_path_thinks` rises on missions without enemy CommandBuildings.
- `kAiDuelEscortThinkBound`, `kAiRegroupingMoveTickBound`: no expected change in campaign missions.
- `kMergeNewAttackCommandBound`: only `commands_replaced` moves.

## cc13-fix2b: why `ai formation shape calls=0` on every campaign row

Packet cc13-fix2b, static only (no game runs). Evidence: cc9's `lead_ai2_on_*` pair logs
(`J:\PROG\battlestations-pacific-decompile-cc9\local\`, 2026-10-13 06:22-06:31, all six switches ON)
and a rel32 / abs32 census of the PE on disk (`local\fix2b_rel32.py` in the cc13-fix2b tree).

**Cause: no AI command ever ticks, because no AI group exists.** The hook is not on the wrong
tick and no gate inside the tick is false. On all four rows the coordinator line reads
`game_mode=8 compose=0 seeds=0 groups_created=0 ... tick_orders=0`, the load-gate line reads
`created_at_load=0 groups_created=0`, and `ai parties ... commands=0`. The cautious-wedge pass
(00A11690, the sibling hook) also shows `calls=0` on all four rows. `formation_shape_due` is set
only inside `ai_command_tick_vt000c`, and that is never entered for a ship.

**The image agrees: it never runs 00A11070 on these rows.** Census of the PE:

- 00A11070 has exactly six references, all in AI command ticks: CALL 00A12350 (SELLING), JMP 00A12442
  (IDLE), JMP 00A12538 (MOVETO), JMP 00A12A48 (REGROUPINGMOVE), CALL 00A12BE0 (MOVETOATTACK) and
  CALL 00A15665 (PATROLTO). There is no abs32 reference.
- Each of those six ticks is reachable only through its vtable slot. The only references are the
  abs32 entries 00D229C4, 00D229EC, 00D22AC8, 00D22B20, 00D22B48 and 00D22BC8, so the ticks run only
  on a command object owned by an AiGroup.
- Those command vtables are referenced only by:
  - the command constructors and clones (00A0FCF0..00A10890, 00A12C60);
  - the group orders 00A2C2A0, 00A2C310, 00A2CCF0 and 00A185E0;
  - BSP_AiGroup_Construct 00A2DFA0 (installs IDLE at 00A2E13D);
  - BSP_AiPlanner_SellThink 00A22800;
  - the debug overlay 00A31000;
  - BSP_AiCommand_CreateFromLua 00A13340.
- The ways to create a group, with the callers of 00A2DFA0:
  - Coordinator-internal: 00A22920 split by class, 00A2E260 and the 00A2E6xx..00A2EAxx compose
    sites. These run only under the coordinator 00A31730.
  - Lua `AICreateGroup` 00A38A50 (00A38CCE). Its only reference is binding row 00E0C8BC.
  - The spawn route 00A16EF0 (00A16F7B), called only from 00947A30, which is called from
    BSP_SpawnRequest_CreateMembers (009487A1). It first tests the party brain `[00F8A89C + party*4]`
    and creates nothing when the brain is null.
- The brain slots are written only inside 00A182C0, the party think (00A183A7 / 00A183B5 /
  00A183D0; the other four references 00A15955, 00A1642A, 00A16B23 and 00A16F31 read the slots).
  The coordinator tick 00A32D50 is the only path into 00A182C0, and per docs/SHIP_AI_OPEN_ITEMS.md
  section 150, the coordinator exists only for a hosted session (game+1FE4h = 1) or after a
  developer command-line mode switch (game+61Ch).
- Lua: across the `.lua` files of this installation, every `AI*(` call is to AISetTargetWeight
  (430), AISetHintWeight (246), AISetSpawnSceneUnitsWeightMul (31) or AISetDefendResourcePercent
  (24). No file calls `AICreateGroup`, `AISetCommand` or `AICreate`.

So campaign ships are driven by script orders and the ship AI navigator, not by AI-planner
commands. In the image, the formation pass is dead on every mission-tree launch, just as it is in
the host. Answers to the packet's three hypotheses: (a) no, the ticks are right; (b) no
in-tick gate is false; (c) yes, but in both the image and the host. Campaign groups never get an
AI command object.

**No reach fix.** None is needed and none would be faithful: wiring 00A11070 into the
navigator or the Lua order path would add behaviour the image does not have. No source change.

**Which missions should show calls > 0:** none of the 23 reference rows, nor any other mission-tree
launch. Only a coordinator session would: a hosted multiplayer/skirmish game, or a launch with one
of the developer mode switches (`siege`, `duel`, `escort`, `competitive`, `ic1..ic4`). Even then,
the coordinator must survive the stage init's `SetGameMode(8, 0)`. The current harness has no such
row, so a same-binary pair cannot exercise `kAiFormationShapeBound`. On campaign rows it is
byte-identical whether ON or OFF. Leave it OFF, or flip it ON "by mechanism" with the other five.
Either way it cannot move a reference row. The same holds for `kAiRegroupingMoveTickBound`,
`kAiDuelEscortThinkBound` and the cautious-wedge census.
Note (cc13-fix22): the host's `game_mode()` returns only 8, so modes 4..7 are unreachable, and the identical `kAiDuelEscortThinkBound` pair (USN15/USN12/LOMP10/IJN18) proves nothing about those models (see `docs/CC13_PLANNER_VERIFY.md` section 0).
