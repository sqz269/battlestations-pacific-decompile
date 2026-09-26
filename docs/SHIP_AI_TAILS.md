# The ship AI's per-step tails

cc9-platform2, 2026-09-25, packet `cc9_ship_ai_tails`. Names are descriptive hypotheses, not
recovered symbols. `docs/UNIMPLEMENTED_RANKING_2.md` ranks these tails at 162,000 calls each in E2:
18 per mission frame.

## 1. The snapshot pair: 009DDBC0 and 009DA0D0

`ship_ai_controller_step_009f50e0` (`src/ship_ai_states.cpp`) calls one of the two on every step:
- 009DDBC0 (`replan_finish`) on a replan step, after the state step (009F51AE);
- 009DA0D0 (`hold`) on every other step (009F51B7).

Both are `__fastcall(blk)` with a plain RET, and each is a straight copy:

| 009DDBC0 (replan): saved <- current | 009DA0D0 (hold): current <- saved |
| --- | --- |
| +3A8h..+3BCh <- +38Ch..+3A0h (six dwords) | +38Ch..+3A0h <- +3A8h..+3BCh |
| +3C0h..+3C2h <- +3A4h..+3A6h (three bytes) | +3A4h..+3A6h <- +3C0h..+3C2h |
| +1F4h..+210h <- +1C4h..+1E0h (eight dwords) | +1C4h..+1E0h <- +1F4h..+210h |
| +214h <- +1E4h (a byte) | +1E4h <- +214h |
| +218h..+220h <- +1E8h..+1F0h (three dwords) | +1E8h..+1F0h <- +218h..+220h |

**What it means.** Each replan snapshots two blocks:
- the steering goal +1C4h..+1F0h: the mode, throttle hold, requested direction, desired throttle,
  rudder and heading, and 009DE050's +1DCh..+1F0h;
- the station request +38Ch..+3A6h: 009DA3B0's direction, heading, leader speed, +39Ch, radius and
  the three gate bytes +3A4h/+3A5h/+3A6h.

Every non-replan step then restores both blocks before the per-frame chain runs. So between replans,
any per-step write to these fields lasts one step only.
- The station-keeping enable +3A5h is cleared by the 007788B0 gate inside 009ED6B0's
  direct-control arm (009ED73F).
- +39Ch is pinned to 1.25f by 009EE5A5 before the path refresh.

In the image both are restored at the next step. The host runs neither copy (records
`ShipAi::replan_finish`, `ShipAi::hold`), so those writes persist in the host until the next
replan rewrites the fields.

**Where the host keeps the fields** (`src/game_hosts_ship_ai.cpp`, the controller `ctl_`):

| offsets | host member |
| --- | --- |
| +1C4h..+1D8h | `ctl_.blk.{mode, throttle_hold_1c8, requested_direction, desired_throttle, desired_rudder, desired_heading}` |
| +1DCh..+1F0h | the matching fields of `ctl_.goal` (`ShipAiGoalPlan`, 009DE050's) |
| +38Ch..+3A6h | `ctl_.station_request` (009DA3B0's layout) |
| +39Ch | also `ctl_.speed_scale_39c` |
| +3A5h | also `ctl_.blk.flag_3a5` |
| +3A6h | also `ctl_.flag_3a6` |

The binding snapshots and restores the host members that alias each offset, keeping the duplicates
in step.

## 2. The warning timer 009DA8D0

`__thiscall(brain, float dt)`, RET 4, body 009DA8D0..009DA941, the tail at 009F5239:
- it counts +B40h down by dt;
- on expiry it re-arms +B40h with the period +B3Ch, carrying the overshoot (`+B3Ch - dt + old`);
- with +3F2h set it calls 00977690 on `[00F8A0C4]` with the ship `[brain+AA8h]`, which the
  pseudocode names `BSP_WarningManager_ReportTorpedo`;
- with +3F1h set it tail-jumps to 00977820 on the same manager.

These are reports to the HUD's warning manager, not gameplay. Binding them moves only records and
HUD warning state, so they come after the snapshot pair.

## 3. Status

`src/game_hosts_ship_ai.cpp` is leased by `agent/cc9-aa-targeting` (`cc9_usn02_sameside_torpedoes`,
until 10:31). The binding waits for the lease. The plan:
- a switch `kShipAiSnapshotBound` for 009DDBC0 + 009DA0D0, committed OFF;
- predictions on USN02 9000 (the surface reference) and E2 9000;
- flip on the verdict.
