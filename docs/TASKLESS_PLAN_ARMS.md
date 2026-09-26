# The planner for a plane with no commanded target (packet cc9_taskless_plan_arms)

2026-09-25. Ghidra read-only. Names are hypotheses. The open item comes from
docs/FIGHTER_ROLL_TRACE.md section 5:
- a task-less Zero has its controls repaired by vehicle avoidance at about 30 s;
- the host's planner then never runs again, because it returns before any arm when
  `command_target_plus_one == 0`;
- so the controls stay frozen until the plane falls into the sea.

## 1. The image's entry

- **The entry tests.** `0099D300` `BSP_PilotBot_PlanControls` begins with one special case,
  `0099D309`-`0099D3C2`. When the byte `plan+26Ch == 2` and two flight-state bytes (`+9C2h` on the
  unit and on `[plan+270h]`) are set, it writes the neutral plan and returns:
  - every slot's desired is 0, with its active byte set;
  - throttle 1.0;
  - the modes `+2CCh`/`+2D0h`/`+2D4h`/`+2D8h` = 0.
- **No command-target test.** Otherwise it continues at `0099D3C5` with the free-flight test and runs
  its arms: the low-airspeed gate, the stick override, the yaw base, the roll and pitch arms, the
  yaw arm and the throttle arms. There is no test of a command target anywhere on that path.
- **The heading.** The yaw base reads `plan+2C0h` at `0099DEB8`. A task-less plane keeps whatever
  was last written there. With the mode reseeded to 1 each think (`0099B450`), the base term is 0
  under the landed yaw-base gate, and the plane holds its heading.
- **The host.** Its early return is its own guard, dating from the stand-in that steered every mode
  toward the commanded target's bearing.

**What the arms do for a repaired, task-less plane.**
- The reseed sets every slot's desired to the live value, `+2B4h` to TravelSpeed x
  NewTravelSpeedMul and `+2D8h` to 1.
- The throttle arm then drives the throttle toward that speed.
- The pitch arm drives the pitch slot toward `plan+2BCh`, which the reseed zeroes, above the
  attitude floor.
- The roll arm levels the bank toward the bank target the reseed leaves at 0.

So the repaired values are overwritten each think, instead of freezing.

## 2. The binding

`kPlannerTasklessArmsBound` (`src/game_hosts_units.cpp`, `plan_yaw_0099d300`) removes the early
return. Without a target:
- the heading the yaw and roll arms use is `plan+2C0h` when written, else the current heading (a
  zero error);
- the attack-range statistic is skipped.

## 3. Predictions, written before the pair

The pair is `kPlannerTasklessArmsBound` OFF against ON, one tree (main `da4ba1baf` plus this commit),
E2 9000, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`. `kZeroTraceDiag` is ON on both sides; it
only logs.

On main (`local\SV_ON_9000.log`) the Zeros already survive, through coupling, so "2 -> 0" may not be
observable on this base. The direct rows are the planner call count and the Zeros' controls.

| row | OFF | ON prediction |
| --- | --- | --- |
| planner calls (`yaw_plans`) against thinks | about 86k of 153k | equal to thinks, within 1% |
| Zero controls frozen after a repair (a Zero holding one control triple for more than 20 traced seconds while alive) | 0-2 Zeros | 0 |
| Zero depth kills (A6M) | 0-2 | 0 |
| US fighter sea losses | 0-1 | 0-1 |
| Kate / Val deaths | 16 / 16 | 12-16 each |
| hit records | 450-750 | 450-750 |
| torpedo / dive-bomb releases | 2-8 / 0-4 | 2-8 / 0-4 |
| Lexington moved | 5.5-7.5 km | 5.5-7.5 km |
| mission end | none | none |

**The risk.** Every task-less plane now plans every think. The spawned squadrons before their first
order, and the escorts, will fly the reseeded plan instead of their live controls. Their paths
change, and the rows are judged by band.

## 4. The pairs, measured

Two pairs from one tree (main `da4ba1baf` plus this commit), each differing only by
`kPlannerTasklessArmsBound`. `kZeroTraceDiag` is ON on every side, window 1600x900 in every log.
**Freeze rows:** from `local/zfreeze.py`, the longest run of one control triple per Zero while alive.

**Pair 1, current main** (`TP_OFF` / `TP_ON`: the turn chain and target validity ON):

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| planner calls / thinks | 85,706 / 152,986 | 152,986 / 152,986 | equal to thinks | held |
| Zeros frozen more than 20 s | none | none | 0 | held |
| Zero depth kills | 0 | 0 | 0 | held |
| every gameplay row | - | identical to OFF, distance moved included | bands | held |

On current main no Zero is repaired and stranded, so the task-less arms change no gameplay row.

**Pair 2, the base where the freeze reproduces** (`TZ_OFF` / `TZ_ON`: the turn chain and target
validity OFF, as in `local\ZT_9000.log`):

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| planner calls / thinks | 90,158 / 150,204 | 156,633 / 156,633 | equal to thinks | held |
| **Zeros frozen more than 20 s** | **#4.2 (53 s), #8.2 (54 s)** | **none** | 0 | **held** |
| **Zero depth kills** | **2** | **0** | 0 | **held** |
| US fighter sea losses | 2 (Lexington sqn01 pair) | 2 (the same) | 0-1 on ON | not this switch's row: they belong to that base's missing target validity |
| Kate / Val deaths | 16 / 16 | 16 / 16 | 12-16 | held |
| hit records | 553 | 554 | 450-750 | held |
| torpedo / dive-bomb releases | 6 / 0 | 6 / 0 | 2-8 / 0-4 | held |
| fighter kills | 9 | 9 | - | - |
| Lexington moved | 6648 m | 6648 m | 5.5-7.5 km | held |
| plane distance moved | 1,085,244 m | 1,142,300 m | - | +5.3% (the two Zeros fly on) |

**Verdict.** `kPlannerTasklessArmsBound` lands **ON**.
- It is the image's entry, since `0099D300` has no command-target test.
- On the base that strands the Zeros it recovers both: 2 depth kills become 0.
- On current main it is neutral on every gameplay row.
