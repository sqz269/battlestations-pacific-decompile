# Handoff: why a ship with a correct navigation goal does not drive

Written at the end of packet `cc8_ship_moveonpath` (branch `agent/cc8-ship-moveonpath`, merged
to main as `16ffe45d3`). The path side is finished and measured; this is the gate that is left,
and it is **not** in the path chain. Nothing below is a reading of a body I opened: the two
numbers are measured, the hypothesis is the cc8 integrator's and is labelled as such, and the
work is to confirm or kill it.

## What is already established, so you do not re-derive it

`docs/SHIP_AI_PATH_CURSOR.md` has the whole `moveonpath` chain read from the listing. The state
step `009E59C0` is projected and exercised, and on USN04 (`--mission-frames 3000`, 0.05 s):

```
unit                 path             pts mode start from  at dir final advances travelled
Lexington-class01    CarrierPath1       6    3     5    2   2 fwd    no        0      0.00
Yorktown-class01     CarrierPath4       8    3     5    2   2 fwd    no        0     48.30
```

Both cursors are right, both correctly report no final leg (mode 3 `PATH_FM_CIRCLE` never
does), and `009E59C0` hands `009DE050 BSP_ShipAi_SetNavigationGoal` the current leg's waypoint
on **all 179** of its steps (`ShipAiState::set_navigation_goal` went 8400 -> 8579, exactly +179).
The hull does not move. `moved=100.51` for the controlled Lexington is unchanged to the
centimetre across three same-binary runs.

**The two numbers this handoff exists for:**

1. `WeaponDirector::path_follow_00836bf0` ran **900** times against **6000** possible director
   steps for the two carriers (3000 fixed steps x 2 units). Its only gate is the listing's own
   at `00836BF0` - `CMP EAX,0xe08f80`, the director's current command. So the 98 issued
   `moveonpath` commands are current about **15%** of the time and are otherwise displaced.
2. Both carriers' command rows read `latch -`, `steer 0.000`, `thrust 0.000`, before this packet
   and after it. `009E59C0` is not what stops them.

Logs: `J:\PROG\battlestations-pacific-decompile-cc8-ship-moveonpath\local\mop_before_usn04.log`,
`mop_after3_usn04.log`, and the 216-entry call diff `local\calldiff.txt`.

## The hypothesis to confirm or kill (the cc8 integrator's, not a reading)

> `Lexington-class01` is the CONTROLLED unit in every USN04 run
> (`controlled=Lexington-class01`). In the image the player drives the controlled unit, so the
> ship AI's drive output may be deliberately suppressed for it - compare `0099C230`, the gate
> the bomber break-off shares, which returns false only for the unit the in-mission interface is
> attached to. If the host gates the AI drive on "is the controlled unit", then the Lexington
> standing still with a correct goal is FAITHFUL for a process with no player input, and the
> unit to judge path following by is the **Yorktown**.

It is cheap to kill: the Lexington's `travelled` is 0.00 and the Yorktown's is 48.30 under the
same command with the same kind of cursor, which is already a difference in the right direction,
but 48.30 m in 150 s is not path following either. So confirming the gate would move the
question to the Yorktown rather than answer it.

## The two questions, in order

1. **Which routine consumes `009DE050`'s goal into throttle and rudder for a ship, and where
   does it stop for the Yorktown?** `009DE050 BSP_ShipAi_SetNavigationGoal` is
   `__thiscall(blk)(const float* goal2d, char keep_mode, unsigned char final_leg)`, RET 0Ch,
   body `009DE050-009DE1A6`, already read and projected in `src/ship_ai_state_steps.cpp`; `blk`
   is `brain+8h`. Follow the fields it writes to their readers. This process publishes 54000
   times and promotes 54000 times (`summary mission ship ai ... publishes=54000
   promotions=54000`) and the hull still does not move, so the break is between the goal store
   and the order slot, or between the order slot and `00825F20`'s motion head.
2. **Does the controlled-unit gate exist in the image at all?** `0099C230` is the comparison the
   integrator offers; `009F3DF3` and `009F5E06` already read `unit+184h`, the player-controlled
   byte, and `GameUnitsHost::unit_player_controlled_0184` models it. If a ship-AI drive site
   tests that byte, the hypothesis is confirmed by a reading rather than by the two distances.

Do not assume either answer from the numbers above. A stand-in is a proof or a hole, and this
one is a hole.

## What would also help, and is one edit

`CommandQueueState::path_point_counts` (`include/bsp/command_execution.hpp`,
`src/command_execution.cpp:67`) is still empty, and it is the queue step a `moveonpath` command
takes. The count it wants now exists: `GameDirector::path_points.size()`, filled by this
packet's `begin_path_command_0071f600`. Wiring it is one edit inside `Impl::command_count`
(`src/game_hosts_commands.cpp:206`), but it changes queue arithmetic at three call sites (456,
507, 570) - from 1 to 6 and 8 for the two carriers - so it needs its own USN04 run. It is a
candidate for what displaces the 98 commands in number 1 above; that is a guess, not a reading.
