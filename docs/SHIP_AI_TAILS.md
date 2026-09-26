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

`kShipAiSnapshotBound` binds 009DDBC0 + 009DA0D0. It was committed OFF with the predictions in
section 4 (57eb1160b), measured on USN02 9000 and E2 9000, and set ON by the verdict in section 5.
The warning timer 009DA8D0 (section 2) is still a record.

## 4. The binding and its predictions (the switch committed OFF)

`kShipAiSnapshotBound` (`src/game_hosts_ship_ai.cpp`):
- `replan_finish_009ddbc0` copies the aliasing host members into `ctl_.snapshot`, the saved blocks.
- `hold_009da0d0` copies them back.
- A hold before the first snapshot would restore the constructor's zeroes. It is recorded as
  `ShipAi::hold_before_snapshot` and not applied. The first controller step replans (interval 0),
  so it should not occur.

**Predictions, written before the pairs.** One tree (main 9a27b915b plus this), `local\bin\sn_off`
against `local\bin\sn_on`, `BSP_GUNNERY_RNG_STREAMS=1`, 1600x900. USN02 9200/9000 (the surface
reference; since d0caf31e2 the mission fails at 44.6 s with an idle player, on both sides) and E2
USN04 9200/9000.
- **Rows.**
  - `ShipAi::hold` and `ShipAi::replan_finish` turn concrete at their OFF counts. Together they are
    one call per AI step: 162,000 in E2, split about 129,000 and 33,000.
  - `ShipAi::hold_before_snapshot` does not appear.
- **Unimplemented total.** It falls by exactly those two counts.
- **Where state can move.** Two host writers change these fields between replans:
  - 009ED6B0's 007788B0 gate clears the station-keeping enable +3A5h on formation followers;
  - 009EE6B5 raises +1F0h to the planner's path length.

  With the binding, both are undone at the next step. The raise repeats every step, so +1F0h is
  unchanged while the path is. The +3A5h clear stops persisting, so a follower's station-keeping
  arm (009EDA28) runs again on the steps after it.
- **Summary-line prediction.** Movement is confined to the formation followers' station keeping:
  the `ship ai` station and follow lines, and follower paths. Deaths and the Lexington's path may
  shift only through that.
  - USN02 still fails at about 44.6 s on both sides.
  - If no line moves, then no host code wrote these fields between replans in these runs.

## 5. The pairs and the verdict

One tree (main 9a27b915b plus section 4's binding), `local\bin\sn_off` against `local\bin\sn_on`,
`BSP_GUNNERY_RNG_STREAMS=1`. Every log shows `window resolution override fit: 2560x1440 -> 1600x900`.

### USN02 9200/9000 (`local\sn_off_usn02.log`, `local\sn_on_usn02.log`)

| Line | OFF | ON |
|---|---|---|
| `ShipAi::hold` / `ShipAi::replan_finish` | records | concrete, same counts |
| `ShipAi::hold_before_snapshot` | absent | absent |
| unimplemented total | 4,905,576 | 4,695,309 |
| mission end (`EndMission`, failed, entity `Jupiter`) | 44.60 s | 39.65 s |
| gunnery deaths | 22 | 19 |
| station_keeping | 44,727 | 29,756 |
| path_picks / plan requests | 124,495 | 134,131 |
| arm tail stops / arrival_latches | 989 / 606 | 166 / 28 |
| trace lines with `dir=astern` | 520 | 0 |
| `ShipAi::drive_astern_heading` | 4,264 | absent |

The rows and the missing `hold_before_snapshot` came out as predicted. The unimplemented total
fell by less than the two rows (252,000). The rest is the run itself moving, and records rose
elsewhere.

**The prediction of where state moves was wrong.** It listed two host writers between replans
(+3A5h and +1F0h). It missed the per-frame obstacle block of the chain after the hold,
009F43C6..009F44B2 (`src/ship_ai_obstacle_tables.cpp`):
- With blk+1C8h clear and the direction latched, 009F4439 does not write +1D0h.
- The profile snap at 009F448E then starts from the previous step's +1D0h, with a window of
  -1.5..1.5.

Without the snapshot that previous value is the last step's own output, so the snap's result
carries from step to step and can reach the window edge (how 009D6B40 moves it was not re-read
for this section). Samidare at step 70 traces `throttle=1.500` OFF against
`1.000` ON, the first divergent trace line. From there the astern latch (+35Ch = 2, which is
outside the restored ranges) engages, and 009F409E steers about the reciprocal heading. With the
snapshot, 009DA0D0 puts the replan step's +1D0h back before the chain runs on every step. The
snap cannot build up, the latch is never taken, and no ship traces astern.

That is the image's order: 009F50E0 calls 009DA0D0 before the per-frame chain (section 1). The
restored span matches section 1 byte for byte:
- the twelve dwords +1C4h..+1F0h;
- the 27 bytes +38Ch..+3A6h, the whole `ShipAiStationRequest`;
- +39Ch and +3A5h/+3A6h, carried inside that request.

The drop in station keeping and the earlier failure follow from the changed paths. They are not a
separate writer. Both runs simulate all 9,000 frames; the failure does not end the run.

**Per-entity table, both ways.** The clock offset is zero: one tree, the same frames, and both
first hits land within 0.1 s (30.40 s against 30.50 s).
- Allied ships sunk: 14 without the binding, 11 with it. John1, John2 and Encounter survive with
  it. Exeter goes down earlier (43.45 s against 35.45 s).
- Japanese ships sunk: 8 on both sides, the same eight. Haguro, Yamakaze and Asagumo survive both.
  The kill times and credits move, because the Allied ships now fight longer.

### E2, USN04 9200/9000 (`local\sn_off_e2.log`, `local\sn_on_e2.log`)

| Line | OFF | ON |
|---|---|---|
| `ShipAi::hold` / `ShipAi::replan_finish` | records | concrete, same counts |
| `ShipAi::hold_before_snapshot` | absent | absent |
| unimplemented total | 4,440,677 | 4,229,915 |
| station_keeping | 17,938 | 46,798 |
| path_picks (`ShipAi::pick_path_point`) | 144,062 | 115,202 |
| brain_targets | 0 | 8 |
| gunnery deaths | 37 | 39 |
| trace lines with `dir=astern` | 0 | 0 |

Here the station-keeping arm runs more, and the path-pick arm, its alternative, runs less. That is
section 4's +3A5h mechanism: the per-step clear is undone at the next hold. The astern carry-over
does not arise in this mission.

**Per-entity table, both ways.** No ship's fate changes, and the Lexington's row is the same on both
sides. Only aircraft rows move:
- two aircraft of `Lexington-class01_sqn01` die at 381.3 s and 381.9 s with the binding, and live
  without it;
- several aircraft kills change their credited ship (a D3A, B5Ns, one Val kill moves from a
  Fletcher to the Lexington);
- the kill times that stay with the same aircraft move by at most 3.4 s.

Rows new with the binding, all records: `BotStateDiveBombDone::enter` (1),
`BotStateDiveBombDone::station_keeping` (158), and three `HudWeaponGroupScreen` gunner rows (152
each). No row disappears. USN02 loses `ShipAi::drive_astern_heading` and gains
`ShipAiFollow::min_float` (1,043).

### Verdict: ON

The binding is the image's order and span (009F51AE and 009F51B7 in 009F50E0, section 1). Both
missions move for reasons traced to the restored fields: the +1D0h carry-over in USN02, and the
+3A5h clear in E2. Neither is a divergence of the binding. The OFF behaviour, where per-step
writes persist between replans, was the host's. `kShipAiSnapshotBound` is set true.

## 6. The turn clearance and the path-plan head (packet `cc9_ship_ai_turn_clearance`)

cc9-gunnery, 2026-09-26. Addresses: 009EF910 009F4D10 009F4D87 009ED3E0 00778890 0070D400
0070D5D0 009ED41F 009ED460 009D9DE0 0070D290.

### What was actually missing

- **009EF910 `BSP_ShipAi_RefreshTurnClearance`**, body `009EF910..009F00F3`. It is **already
  reconstructed and run every step.**
  - The rule is `ship_ai_refresh_turn_clearance_009ef910` (`src/ship_ai_clearance_profile.cpp`,
    `docs/SHIP_AI_CLEARANCE_PROFILE.md`, packet `cc_ai_clearance_profile`).
  - `run_clearance_refresh` calls it straight after `ship_ai_publish_order_009f4d10` returns. Its
    rows `ShipAi::refresh_turn_clearance_009ef910` and `ShipAiObstacle::clearance_37c_producer`
    are concrete at 162000 calls in E2.
  - The UNIMPLEMENTED row `ShipAiOrder::tail_009ef910`, rank 6 of
    `docs/UNIMPLEMENTED_RANKING_2.md`, is the publish reconstruction's own hook for the same call
    (`009F4D87`, 009F4D10's only call into it). It recorded instead of saying where the body runs.
  - Nothing between the hook and `run_clearance_refresh` reads what 009EF910 writes. The slot
    copy-back touches `order.slots`; the clearance reads `blk+324h/+330h/+35Ch` and the geometry.
    So the row is a bookkeeping gap, not a behaviour gap.
- **009ED3E0's head**, `009ED3E0..009ED498`. It is **reconstructed but not bound**
  (`src/ship_ai_path_corridor.cpp`, `docs/SHIP_AI_PATH_CORRIDOR.md`, packet `cc_ai_corridor`). The
  host passed the `20.0f` of `00CE3930` for both corridor widths.
  - The head asks `00778890` whether the ship leads its formation group (`entity+284h`, leader
    `[group+14h]`).
  - When it does, `0070D400` and `0070D5D0` reduce the member records' across-axis column to the
    largest offset on each side: each slot is clamped into `[0, 1200]`, the result floored at 5 and
    capped at 400.
  - Each extent + 20.0 (capped at 600) becomes a width.
  - `009D9DE0` stores the widths into the plan blocks (`plan+4h`, `plan+8h`) and throws a running
    search away when a width moves by more than 25. `009EE61E` publishes `max(30, plan+8h)` for
    the point the follower steers to.

### The binding (`kShipAiTurnClearanceBound`)

- **The corridor head.** `CorridorBinding` answers the three calls from the units host's
  formation groups:
  - `unit_formation_group_0284` and `formation_leader_0014` for `00778890`;
  - for the extents, the member records' across-axis value through `formation_station_0070d290`
    at unit scales. That is `record+10h + column*4`, 0070D290's own read.

  `ship_ai_path_corridor_widths_009ed3e0` turns them into the two widths, which go to the refresh
  arm in place of the literal.
- **The hook.** `ShipAiOrder::tail_009ef910` becomes `done`.
- Labelled: the leader's own record, which 0070D290's leader branch skips, contributes 0 to the
  reduction. A leader's formation offset is its own station, normally 0.

The summary line `ship ai corridor` counts the refreshes whose widths came from a group and the
largest width.

### Predictions (written before the runs)

Same-tree pairs, switch only, `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`: USN02 9200/9000 and
E2 (USN04 9200/9000).

1. **Rows.** `ShipAiOrder::tail_009ef910` goes from UNIMPLEMENTED to concrete: 162000 calls on
   E2, and on USN02 the count of its `refresh_turn_clearance` row. `ShipAiPath::refresh_plan_head`
   goes from UNIMPLEMENTED to concrete at the `plan requests` count (136950 on E2, 149843 on
   USN02). Three new concrete rows appear: `00778890`, `0070D400` and `0070D5D0`.
2. **Widths.** On USN02, the formation leaders (the Houston and Exeter groups the script joins,
   `usn_2_java.lua` `JoinFormation`) take group widths: `group_widths > 0` and `max_width`
   between 25 and 420. E2 takes group widths only if USN04 forms a group: 0 or a few leaders.
3. **Paths and tracks.** Plan requests stay equal (the refresh cadence does not depend on the
   widths). Seeds and accepts move on USN02 when a wider corridor changes the searches or a
   width change throws a search away. The leaders' tracks and their followers' stations can move
   by tens of metres. `station_keeping` (16424 on USN02) moves by less than 10%.
4. **Outcomes.**
   * USN02: deaths 20 +/- 3, hit records 487 +/- 15%, failure at 39.65 s +/- 5 s. A cascade
     through who is afloat can move single deaths by tens of seconds.
   * E2: identical if no ship leads a group. Otherwise deaths 35 +/- 2 and hit records
     594 +/- 5%.
5. **The native table.** Both ways, only the rows in (1) change status. Rows with
   ship-motion-dependent counts can move on USN02.

### Results

Builds `tc_off` / `tc_on`, one tree (`85622ab37`), only the switch differs. Window line and
module directory checked, logs deleted first.

| run | deaths | hit records | damage | end | plan requests / seeds / accepts | station keeping | group widths (max) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| USN02 OFF | 22 | 440 | 59209.2 | 39.65 s | 145972 / 951 / 143147 | 20778 | 0 |
| USN02 ON | 22 | 440 | 59209.2 | 39.65 s | 145972 / 951 / 143147 | 20778 | 6605 (420.0) |
| E2 OFF | 35 | 595 | 7845.7 | - | 136955 / 1657 / 132002 | - | 0 |
| E2 ON | 35 | 595 | 7845.7 | - | 136955 / 1657 / 132002 | - | 7895 (420.0) |

**The native table, both ways.** The only changes are the predicted ones, plus
`PlatformLoopCallbacks::pretranslate` (the Windows message pump, 28 against 20):
* `ShipAiOrder::tail_009ef910` becomes concrete (252000 on USN02, 162000 on E2);
* `ShipAiPath::refresh_plan_head` becomes concrete at the plan-request count;
* the new concrete rows `00778890`, `0070D400` and `0070D5D0`.

**Per-entity tables.** The death tables are identical line for line on both missions, and so is
every `ship ai` per-step line (16611 on USN02, 15954 on E2, same digest both ways). So no ship's
track, station or fate moved, and there is no clock offset to subtract.

1. **Held:** every row change was predicted.
2. **Held:** groups give widths up to 420 on both missions. E2 does form groups: 7895 group
   refreshes, where I predicted few.
3. **Held for the counts:** plan requests, seeds, accepts and station keeping are unchanged. The
   "tracks can move" half did not occur. The widths reach `009D9DE0`, but in these runs no
   running search is thrown away and no steering line changes. Why the published width
   (`009EE61E`) changes nothing here was not traced further.
4. **Held:** deaths and hit records are identical, and so is USN02's failure time.

Note for the integrator: this tree's USN02 row is 22 deaths / 440 hit records / 39.65 s, against
the 2026-09-26 b reference's 20 / 487. The move comes from landings since `7c421ba25`, not from
this switch; which landing was not isolated here.

**Decision: ON.** `kShipAiTurnClearanceBound` is true.

## 7. The whole arm-final step 009DE5B0 (packet `cc9_ship_ai_tails_2`, part a)

cc9-gunnery, 2026-09-26. Body `009DE5B0..009DF117`, `void __thiscall(blk)(float unused)`, `RET 4`.
Reconstruction `ship_ai_arm_final_step_009de5b0` (`src/ship_ai_arm_final_step.cpp`); section list
in `docs/HEADING_TARGET_SECTIONS.md` section 1.

### What was bound before

`torpedo_override_009de8f1` ran sections 1 and 3 to 6 at both 009DE5B0 call sites: the arm tail
at `009EF213` and the station arm's jump at `009EE57B`. The row `ShipAiArmTail::after_arm` stayed
a record. Section 7, the free-bearing query `009DEE0B..009DF10F`, never ran.

### The binding (`kShipAiArmFinalWholeBound`)

At both call sites the whole routine runs with its twelve host calls:

| call | host answer |
| --- | --- |
| `0092D730` | the unit's signed body-axis speed |
| the heading slot | the unit's heading |
| `009DE853` | the unit+102Ch latch |
| `009DA1D0` | the torpedo gate, behind `kShipTorpedoResponseBound` as before |
| `class+500h` | the class max speed |
| the `+608h` walk | the neighbour view, as section 6 had it |
| `00778890`, `0070D400`, `0070D5D0` | the formation group, as in section 6 of this doc |
| `007788B0` | `unit_is_formation_follower_007788b0` |
| `0070E450` | **record**: the leader's own travel layer, so the "moved" searcher is never chosen |
| `009DC2E0` | **record**: false, the same stand-in the sector scan uses |

`blk+354h` is projected twice in this host: the throttle profile's hold and the block's clamp.
The routine is handed the hold, which section 5 read on the partial path. The clamp is raised with
it when section 4 raises it, and restored otherwise.

### Predictions (written before the runs)

Same-tree pairs, switch only, `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`: USN02 9200/9000 and
E2 (USN04 9200/9000).

1. **Rows.**
   * `ShipAiArmTail::after_arm` goes from UNIMPLEMENTED to concrete at its current count (136509
     class on E2).
   * New rows appear: `ShipAiArmFinal::step_009de5b0`; the section-7 calls `00778890`, `0070D400`,
     `0070D5D0` and `007788B0` (concrete); `0070E450` and `009DC2E0` (records).
2. **Heading and astern outputs on USN02.** Sections 1 to 6 are the same rules as the partial
   path, over the same inputs, including the torpedo override inside the whole pass. Section 7's
   query always answers false. So `blk+324h` and the astern latch come out bit-identical:
   `torpedo_overrides`, `zone_escape_turns` and `separation_turns` are equal both ways.
3. **Counts.** Station keeping, path picks (plan requests, seeds, accepts) and every per-ship
   AI line are identical.
4. **Outcomes.** No ship's track or fate moves. Deaths and hit records are identical: USN02
   22 / 440 with the failure at 39.65 s, E2 35 / 595.
5. **The native table.** It differs only in the rows of (1).

### Results

Builds `af_off` / `af_on`, one tree (`43be33806`), only the switch differs. Window line and
module directory checked, logs deleted first.

| run | deaths | hit records | end | `ship ai` lines digest | death table digest |
| --- | --- | --- | --- | --- | --- |
| USN02 OFF / ON | 22 / 22 | 440 / 440 | 39.65 s both | `19be554862` both | `c6b1476e9c` both |
| E2 OFF / ON | 43 / 43 | 796 / 796 | - | `1a7c03f25b` both | `c9c78a38b6` both |

**Native table.** It changed only as predicted, plus the message-pump counter:
* `ShipAiArmTail::after_arm` becomes concrete (145972 on USN02, 109814 on E2);
* new rows: `step_009de5b0` (166750 / 160908), `00778890`, `007788B0`, `0070D400` and `0070D5D0`
  concrete; `0070E450` and `009DC2E0` records.

All five predictions held. The heading target, the astern latch and every per-ship line are
bit-identical: the torpedo override now runs inside the whole pass with the same answers. This
tree's E2 row (43 / 796) differs from the 2026-09-26 b reference's 35 / 594. Both builds of this
pair show it, so it comes from the landings since `7c421ba25`, not from this switch.

**Decision: ON.** `kShipAiArmFinalWholeBound` is true. Section 7's own effect waits on `009DC2E0`,
which still answers a recorded false.

## 8. The navigation arm tail 009EEAAB (packet `cc9_ship_ai_tails_2`, part b)

`ShipAi::navigation_arm_tail` (009EEAAB, rank 10) is UNIMPLEMENTED only because the host recorded it
on the path where the image itself skips it. The disk listing shows the jump:

```
009ee59b: je  0x9ee5a5
009ee59d: test bl, bl
009ee59f: je  0x9ef206          ; the path gate is closed: skip 009EEAAB..009EF205
...
009ef206: fld dword ptr [esp + 0xd0]
009ef213: call 0x9de5b0         ; the arm-final step, which both paths reach
```

- **Gate open:** the host already runs the tail whole (`ship_ai_navigation_arm_tail_009eeaab`, body
  009EEAAB..009EF226, the row `done` at the same name).
- **Gate closed:** the host returned through a record of the tail's own address. With
  `kShipAiNavTailGateBookkeeping`, that path counts under its own row,
  `ShipAi::navigation_gate_closed_009ee59f`.

No behaviour changes.

**Prediction (before the run):** E2 9000, OFF against ON, same tree.
- The tail's row stops being UNIMPLEMENTED. It becomes concrete at the count of open-gate passes,
  and the new gate row takes the rest.
- Deaths, hit records, every ship-ai line and the death table are identical.

**Result:** held (`nb_off` / `nb_on`, E2 9200/9000, tree `bf6eddc7a`).
- The tail's row is concrete at 130833 calls, and `navigation_gate_closed_009ee59f` is concrete at
  1313. Together they make the OFF record's 132146.
- Deaths and hit records (51 / 841), the per-ship `ship ai` digest (`62143c922f`) and the death
  table digest are identical both ways.

`kShipAiNavTailGateBookkeeping` is ON. Tail (b) was bookkeeping, not a gap.


## 9. The warning timer 009DA8D0 (packet `cc9_ship_ai_tails_2`, part c)

`ShipAi::tail_009da8d0` (body 009DA8D0..009DA93F) is a countdown on brain+B40h, re-armed from
brain+B3Ch. On expiry it reports to the warning manager [00F8A0C4] for the brain's own unit
(brain+AA8h):

```
009da8e6: fcomi st(1)            ; dt against B40h
009da8e8: jb    0x9da935         ; dt < B40h: B40h -= dt (009DA935..009DA937)
009da8ea: fsubr [esi + 0xb3c]    ; otherwise B40h = B40h + (B3Ch - dt)
009da8f8: cmp byte ptr [esi + 0x3f2], 0   ; blk+3EAh set: 00977690 (torpedo report)
009da913: cmp byte ptr [esi + 0x3f1], 0   ; blk+3E9h set: tail jump to 00977820
```

- **Seeds.** The brain constructor draws B3Ch = uniform(1, 2) at 009F12CD and B40h =
  -uniform(0, B3Ch) at 009F12F1 (009F12F6 FCHS, stored at 009F1300). The host already made both
  draws for the stream order. It now keeps the values in the controller.
- **Reports.** They go through the HUD worker's entries `game_warning_report_torpedo_00977690` and
  `game_warning_torpedo_effect_00977820` (include/bsp/game_hosts_mission_frame.hpp). Each has its
  own guards, and their remaining steps are records on the HUD side. The HUD switch
  `kWarningManagerTickBound` is still OFF on this tree.
- **Flag producers.** The host clears both flags in the hull pre-step (009E04A0 and 009E04A9, image
  stores of 0) and never sets them. A byte-immediate scan found one store of 1 to +3E9h, at
  00A3F13D in 00A3F100. Its object is not established, and register-source stores were not
  swept.

`kShipAiWarningTimerBound` gates the binding.

**Prediction (before the runs):** USN02 9200/9000 and E2 9200/9000, OFF against ON, same tree.
- `ShipAi::tail_009da8d0` goes from UNIMPLEMENTED to concrete at the same call count.
- The summary line `ship ai warning timer` shows expiries > 0 with the switch ON.
- Torpedo and effect reports are 0, because no host path sets either flag. The two new report
  rows do not appear, and neither do the HUD entries' rows.
- The timer draws nothing and writes only B40h. Deaths, hit records, every ship-ai line and the
  death tables are identical.

**Result:** held (`wt_off` / `wt_on`, tree `07ae5066d`, streams on).

| Pair | tail_009da8d0 calls | Expiries ON | Reports | Deaths / hit records |
|---|---|---|---|---|
| USN02 9200/9000 | 252000 | 9417 | 0 / 0 | 22 / 440 both ways |
| E2 9200/9000 | 162000 | 6363 | 0 / 0 | 51 / 836 both ways |

- The tail's row turned concrete at the same count, and no other gameplay row moved.
  `PlatformLoopCallbacks::pretranslate` differs by a few calls. It is the window message pump and
  varies between runs.
- Every ship-ai line and the death tables are identical by digest.

`kShipAiWarningTimerBound` is ON. The reports stay silent until something sets blk+3E9h or
blk+3EAh (see section 10).

## 10. Handoff (end of the cc9-gunnery segment, 2026-09-26)

What is left for a successor, in the order I would take it.

1. **00957DA0 `PlayerGunSeat::segment_query`** (src/game_hosts_gunnery.cpp, the record just after
   the 00954A10 angle clamp).
   - The image casts the 1000-unit segment from the camera through the spatial index
     (00957D79..00957DA0) and aims at what it hits. The host records it, and every gun takes the
     range-sphere point.
   - Bind it through `GameGunneryHost::query_segment_units` (the HUD's const query over
     `SegmentBinding`, docs/HUD_PICK_SEGMENT_QUERY.md section 4). That query's exclude is
     one-based, and it restores the binding's two trace counters.
   - Check first that the kind filter and the ignore entity used at 00957DA0 match what 009043A0
     passes (kind 0, ignore vtable+B0h). If they differ, extend `SegmentQueryArgs` rather than
     reusing the HUD form.
   - Only the player's gun seat takes this path, so the pairs should move only when a player seat
     is manned. Predict and pair it on USN02 and E2 as usual.
2. **The warning flags' producer.**
   - blk+3E9h / +3EAh are only cleared by the host (hull pre-step 009E04A0 / 009E04A9).
   - A byte-immediate scan found one store of 1 to +3E9h, at 00A3F13D in 00A3F100. Its object is
     not established. Register-source stores (`88 /r` with other registers, and word or dword
     stores covering the pair) were not swept.
   - Until a producer is bound, the section 9 reports never fire. The HUD switch
     `kWarningManagerTickBound` must also be ON for them to do more than their guard records.
3. **Remaining `ShipAi` records** in src/game_hosts_ship_ai.cpp, none of them started:
   - `throttle_ceiling` 009F4DA0 (brain+34Ch / +350h);
   - `step_009eca20`;
   - `hold` / `hold_before_snapshot` 009DA0D0;
   - `replan_finish` 009DDBC0;
   - `replan_prepare_threat_scan` at 009F158A;
   - `station_keeping_arm` at 009EDA28;
   - `neighbour_list_add_009f0d20`.

   Read each record's call counts from the latest E2 log before choosing, and check each against
   the listing first. Tail (b) showed that a record can sit on a path the image itself skips.
