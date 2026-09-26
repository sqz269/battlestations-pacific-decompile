# The side-AI scheduler's gate and periods: their writers (packet `cc9_bot_scheduler_writers`)

Addresses: 00914EF0 (the reader), 0091B2E0, 0091C560, 00905340, 008B87F0 (the writers).

00914EF0 (`BSP_BotScheduler_Update`, `bsp::update_bot_scheduler_00914ef0`) runs on the object at
game+21A0h. Its countdowns never ran in this host because four fields had no writer:
- `enabled` (+1498h);
- `slots_active` (+14A4h);
- the update period (+14A8h);
- the recalc period (+14ACh).

## 1. The writers, from a scan of the four displacements over .text

The scans were `98 14 00 00`, `A4 14 00 00`, `A8 14 00 00` and `AC 14 00 00`, with `--limit 4000`.
The hits in 007E2A20 (`BSP_GameTuning_LoadFromPlaneGlobals`) are `[ESP+1498h]`-style stack slots
of a large frame, not the object. The other hits (00920A20, 009E9190, 009F30F0, the pipe and Lua
routines) are other objects' fields at the same offsets.

| field | writer | site | value | when |
| --- | --- | --- | --- | --- |
| +14A8h update period | 0091B2E0 | 0091BC23 `FSTP [ESI+14A8h]` | `Scoring["InGameScoreUpdateTimeInterval"]`, default 1.0 (0091BC0E `FLD1`) | 0091B2E0 is called by `BSP_Game_OnInit` 004E3AA0, once per process |
| +14ACh recalc period | 0091B2E0 | 0091BC65 `FSTP [ESI+14ACh]` | `Scoring["ReCalcTimeInterval"]`, default 5.0 ([00CE3850]) | the same |
| +1498h enabled | 0091C560 `BSP_MissionScoring_ResetForNewMission` | 0091C628 `MOV BYTE [ESI+1498h],1` | 1 | called by `BSP_Game_ConstructWorld` 004DE610, once per mission load |
| +14A4h slots_active | 0091C560 | 0091C610 `MOV [ESI+14A4h],BL` | 0 | the same |
| +14A4h slots_active | 00905340 `BSP_MissionScoring_SetRealPlayTimeRunning` | 00905344 `MOV [ECX+14A4h],AL` | the argument | the Lua native `Scoring_RealPlayTimeRunning` (008B87F0, argument 0 as a boolean at 008B88EF, ECX = [game+21A0h] at 008B88FA, CALL 00905340 at 008B8901) |

0091C560 also reloads both countdowns from the periods (0091C603 `+1494h = +14ACh`, 0091C61C
`+149Ch = +14A8h`), zeroes the play time +14A0h (0091C62F) and +14A5h, stores the local slot at
+1424h, and tail-jumps to 0090FA40, which was not read.

**This installation's values.** `Scripts\datatables\Scoring.lua` (17,807 bytes, 2024-07-13, the
untouched bulk) lines 861 and 862 set `InGameScoreUpdateTimeInterval = 1` and
`ReCalcTimeInterval = 5`, which are the defaults. Both missions' scripts call
`Scoring_RealPlayTimeRunning(true)` in their init:
- `Scripts\missions\usn\usn_19_coralus.lua` line 42 (2024-08-26);
- `usn_2_java.lua` line 35 (2024-07-13).

`luaInitMissionEnd` in `Scripts\global\commandhelpers.lua` (2024-10-29, modded) calls it with
`false` at mission end.

**What the object is.** It is the mission scoring object: it holds the real play time and the
scoring intervals. Its two passes are the side AI's:
- **Every update period:** a `SessionMessage` 15h (0075B430). Then for each of the eight player
  slots, `BSP_BotSideAi_RecomputeAssetScores` 00914390 while +14A4h is set, and in mode 1 a
  dispatch through 0076A9F0.
- **Every recalc period:** `BSP_BotSideAi_RebuildUnitDemands` 00911E80 and
  `BSP_BotSideAi_RebuildGoalRequests` 00912A60 for each slot.

"Retarget" in the host's names is the earlier reading. In this host all five callees are records
(`BotScheduler::*`). 00914390 has a reconstruction (`src/bot_scheduler_output.cpp`) that the
scheduler host does not call yet. So once the gate opens, the passes run as records and change
no unit.

## 2. The binding

`kBotSchedulerWritersBound` (`include/bsp/game_hosts_mission_frame.hpp`):
- **At the host's construct-world point** (right after `build_entity_chains_009037f0`, the call
  004DE610 makes at 004DE69C):
  - 0091B2E0's two reads go through `GameMissionLuaHost::read_scoring_intervals_0091b2e0`, which
    runs Scoring.lua when `Scoring` is absent. The image reads them at OnInit; here they are read
    at the world build, before the reset that uses them.
  - Then 0091C560's resets. The tail 0090FA40 is the named record
    `MissionScoring::reset_tail_0090fa40`.
- **The Lua row 008B87F0** calls `game_scoring_set_real_play_time_running_00905340`. The host log
  shows the world build ahead of `luaStageInit`, so the script's `true` lands after the reset, as
  in the image.

Off, the fields keep their zero defaults and the native stays a record.

## 3. Predictions (written before the pairs; the switch committed OFF)

One tree (main 286b46269 plus this), `local\bin\bs_off` against `local\bin\bs_on`,
`BSP_GUNNERY_RNG_STREAMS=1`, 1600x900. USN04 4700/4500 and E2 = USN04 9200/9000. Ship bots are not
scheduled differently from planes: the passes are per player slot, not per unit. The same records
would run in USN02, so it is not added.
- **Rows.**
  - `MissionScoring::load_intervals` 1 (concrete), `reset_for_new_mission` 1,
    `reset_tail_0090fa40` 1 (record), and `set_real_play_time_running` 1.
    `MissionLuaNative::Scoring_RealPlayTimeRunning` (1 record) disappears.
  - `BotScheduler::retarget_begin` fires once per update period: 20 or 21 ticks at 0.05 s,
    depending on the float residue. That gives 214 to 225 in USN04 and 428 to 450 in E2.
  - `BotScheduler::slot_prepare` runs eight per update: 1,712 to 1,800 and 3,424 to 3,600.
  - `BotScheduler::think_a` and `think_b` run eight per recalc, every 100 or 101 ticks:
    44 to 45 recalcs, so 352 to 360 each, in USN04; 89 to 90, so 712 to 720 each, in E2.
  - `BotScheduler::slot_dispatch` does not run: the mode (+1FE4h) is 0 in this host, and the
    dispatch needs 1.
- **Gameplay.** None: every pass is a host record. Deaths, hit records and releases are identical,
  a band of zero. The per-entity tables and every summary line are identical. A move would be the
  finding.

## 4. The pairs and the verdict

One tree (286b46269 + 5f1d7e1bf), `local\bin\bs_off` against `local\bin\bs_on`,
`BSP_GUNNERY_RNG_STREAMS=1`. All four logs show the 1600x900 fit line and the final COM release.
The ON logs print `mission scoring: Scoring.lua read, InGameScoreUpdateTimeInterval=1.000
ReCalcTimeInterval=5.000`.

| row | USN04 ON | E2 ON |
| --- | ---: | ---: |
| `BotScheduler::retarget_begin` (0075B430) | 225 | 450 |
| `BotScheduler::slot_prepare` (00914390) | 1,800 | 3,600 |
| `BotScheduler::think_a` / `think_b` (00911E80 / 00912A60) | 360 / 360 | 720 / 720 |
| `BotScheduler::slot_dispatch` | 0 | 0 |
| `MissionScoring::load_intervals`, `reset_for_new_mission`, `set_real_play_time_running` | 1 each, concrete | 1 each |
| `MissionScoring::reset_tail_0090fa40` | 1 record | 1 record |
| unimplemented total, OFF -> ON | 2,194,415 -> 2,197,160 | 3,894,554 -> 3,900,044 |

**Every prediction held.** The counts are the top of each band: every 20 ticks and every 100
ticks, so the float residue lands at or below zero on the exact tick.
- `MissionLuaNative::Scoring_RealPlayTimeRunning` (1 record) is gone.
- One row moved that the predictions did not list: `MissionLua::open_script` 41 -> 42. The reader
  is `read_scoring_intervals_0091b2e0`, which runs Scoring.lua because `Scoring` is not yet a
  global at the world build.
- The per-entity tables, the gunnery damage lines (USN04 41 deaths and 727 hits; E2 51 and 836)
  and every summary line are identical, so the clock offset is zero. No gameplay line moved:
  every pass the open gate reaches is a host record.

**Verdict: ON.** The four fields now carry the image's values at the image's points, and the
scheduler's passes run on the Scoring.lua cadence. `kBotSchedulerWritersBound` is set true. The
side AI's passes become gameplay only when 00914390, 00911E80 and 00912A60 are bound. 00914390 is
already reconstructed (`src/bot_scheduler_output.cpp`).
