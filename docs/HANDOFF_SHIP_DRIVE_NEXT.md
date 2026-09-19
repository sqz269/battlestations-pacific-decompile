# Handoff: what is left after the ship drive gate

Written at the end of packet `cc8_ship_drive` (branch `agent/cc8-ship-drive`, merged to main as
`f8763e950`). `docs/SHIP_AI_DRIVE_GATE.md` is the packet; this is only what it did not do.

**Read first, because it changes what old numbers mean.** Two host bugs that packet fixed were
both upstream of a lot of measurement:

1. `0071F600`'s only caller is `00835D33` inside `00835C70 BeginCurrentCommand`, and it builds
   the path object for the QUEUE HEAD. This host built one per 5Bh message, and the build zeroes
   `path_travelled`, `path_advances` and the cursor's join index, so a script that re-orders
   every 3.06 s reset the cursor 49 times a mission.
2. `GameCommandsHost::register_units` ran `directors.assign`, and `create_units` calls it again
   for every batch of units a mission spawns. Every unit's director - ten command slots, queue
   mode, stage, cruise latch, path object - and every unit's commanded speed were destroyed
   twelve times on USN04, once per three-plane squadron batch.

## (a) The Lexington's displacement: the AI coordinator, not the path chain

The script's `moveonpath` never becomes the queue head for `Lexington-class01`, so it never
begins and the run has no cursor row for it. Its command rows are 24 consecutive
`moveto ai_command_tick` with `curr=1`: `00A02020 AiCommand::issue_moveto`, 104 calls a run,
identical across all four runs of the packet and unchanged by the director fix.

Open: **where the image excludes the player-controlled unit from a group order.** Not in
`00A02020` itself - no `+184h` test in its first 120 instructions - and not in the ship drive:
`docs/SHIP_AI_DRIVE_GATE.md` section 4 has the whole census of `unit+184h` tests and
`009F3F80 BSP_ShipAi_DriveOrderRing` is not in it. The precedent to compare is
`0099C230 BSP_BotTask_BreakOffAllowedForUnit`, which the five bomber break-off routines share and
which answers false only for the unit the in-mission interface manager `00E198C4` is attached to.
The likely place is the group membership or the coordinator tick, not the issue.

File: `src/game_hosts_ai.cpp`, unleased since `cc8-formation` retired. Note before changing
anything there: the image DOES terminate the player unit's running command every director step
(`00836962..00836985`, `PUSH 2 / 0071D810(2)`), and this host already models that arm in
`weapon_director_step_prepass_00836941` (`src/unit_commanded_speed.cpp`). So the question is not
"is there a gate" but "why does the coordinator keep re-issuing into it".

## (b) `007ADD70`'s multi-leg advance

This host takes ONE leg per director step. `007ADD70`'s own loop can take several: the
remaining-leg arithmetic is `007ADD88-007ADDD3` and the `atan2` lookahead that decides how far to
skip is `007ADE5D-007ADF80`, neither transcribed. `docs/SHIP_AI_PATH_CURSOR.md` records the same
hole, and `advance_path_cursor_00836bf0` in `src/game_hosts_commands.cpp` carries the note at the
call to `ship_ai_path_next_index_007adcc0`.

Whether it is a visible defect depends on the spacing of the authored points against the arrival
radius `min(unit+9C8h * 2.5, 0082E850 * 1.2)`. **The 450 s run below does not answer it, because
no advance happened at all**, so nobody has yet seen two legs in a row and the multi-leg question
cannot be settled by measurement until one does.

## (c) The difficulty-1 run, exactly as scoped

`GetDifficulty 008AE030` answers `game_effective_difficulty()`, a hardcoded zero at
`src/game_hosts_script_orders.cpp:145`. A second zero is `chosen_difficulty()` at
`src/game_hosts_mission.cpp:933`. **Check whether the first alone drives the script's two-member
`SpawnNew` requests**: change it LOCALLY, build, run USN04, read, revert, and do not commit.

Report: requests, members per request, units created, whether BOTH callback arities are reached,
and whether `Objectives_Completed` stays 0 - at difficulty 0 `Mission.IJNFightersLex` is never
filled, because it is filled only from the SECOND group member, and the script gates at lines 576
and 582 would complete the primary objective by themselves.

The baseline to diff against is `summary SpawnNew 0094c480 calls=8 rejected=0 queued=8
attempts=8 fulfilled=8 requeued=0 units=8 callbacks=8 callback_missing=0 still_queued=0
interval=0.500 clock=150.0`, on `local/drive_party_usn04.log` in this worktree. Use
`python local/calltable.py <before.log> <after.log> <out.txt>` for the host-call census in both
directions; it reports NEW and LOST rows as well as changed counts.

## (d) `docs/AI_TARGET_WEIGHT_TERMS.md`'s IJN01 baseline is superseded

The AI baseline table at the top of that document was taken while `register_units` was destroying
every director on every `GenerateObject` batch, so every unit in it lost its command queue,
cruise latch and commanded speed several times during the measurement. It needs re-taking on a
build at `f8763e950` or later. The same caution applies to any before/after whose baseline log
predates that commit and whose mission spawns units.

## (e) The first leg needs more than 450 s of command, and that is now the open question

`local/drive_long_usn04.log`, USN04 at `--frames 9200 --mission-frames 9000` (449.96 s), on the
build at `737432c9f`. The Yorktown closed **5850.94 m -> 444.20 m** of its waypoint at about
16.7 m/s, then left `moveonpath` at step ~6980 and finished the run in `cruise`.

```
path cursor   Yorktown-class01 CarrierPath4  pts 8  mode 3  start 5  from 2  at 2  fwd
              final no   advances 0   travelled 3957.45
WeaponDirector::path_follow_00836bf0  concrete 6467   (of 9000 director steps, 72%)
WeaponDirector::path_build_0071f600   concrete 3
summary mission director completion   end_commands=5 queue_advances=10
summary mission world                 units=87 simulated=449.96 s moved=100.51 total_path=108653.79
```

Three things follow, and they replace "the ships do not drive" as the open question:

1. **The arm now holds its slot 72% of the time**, against 900/6000 = 15% before this packet.
   That number was the previous handoff's evidence of displacement and it is mostly gone.
2. **Distance is no longer the blocker; command lifetime is.** 444.20 m is the closest the ship
   got, and the arrival radius `min(unit+9C8h * 2.5, 0082E850 * 1.2)` must be under it, since the
   test failed on all 6467 tries. The leg needs roughly another minute of command and the command
   did not survive it: `end_commands=5` and the state is `cruise` by step 8390.
3. So **what ends the `moveonpath` command before it arrives** is the next measurement, not a
   longer run. The stage spine is `00836920`'s, the terminal raise is `0071D810(2)`, and the
   completion path is `0071E430` -> `00720850`. `travelled` restarts at each begin, so read it
   with `path_build`'s count beside it: 3957.45 m is the distance since the third begin, not the
   mission.

## What is measured and does not need redoing

`docs/SHIP_AI_DRIVE_GATE.md` sections 2, 3, 5 and 6, with four same-binary USN04 runs in
`J:\PROG\battlestations-pacific-decompile-cc8-ship-drive\local\`: `drive_before_usn04.log`,
`drive_after_usn04.log`, `drive_keep_usn04.log`, `drive_party_usn04.log`, and the call diffs
`drive_calldiff.txt`, `keep_calldiff.txt`, `party_calldiff.txt`. The 450 s run is
`drive_long_usn04.log`.

The **USN01 control** for the director fix is `local/drive_usn01_control.log`, 3000 frames on the
same build. USN01 creates its units in one batch, so nothing should move, and nothing did: five
torpedo tasks all reaching `done` (313 / 374 / 385 / 313 / 325 ticks), `goaway enters=1` on each
of the five, `swims_started=5`, `torpedo_drop drops=5 refusals=0 water_entry_breakups=0`,
`deaths=1`, `total_damage=1285.0`, `units=62 simulated=150.00 s`. That is the whole list the
integrator gave, reproduced.
