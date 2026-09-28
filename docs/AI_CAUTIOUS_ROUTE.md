# The CAUTIOUSATTACK route of `00A14DD0` (packet `cc9_director_moveonpath_route`)

Worker cc9-ships5, 2026-09-28. This continues docs/PLANNER_TASK_CHOICE.md section 15 (cc9-ships4's read).
Code: `ai_cautious_approach_pass_00a14dd0` in `src/ai_command_tick.cpp`, its host in
`src/game_hosts_ai.cpp`, and the director's user path in `src/game_hosts_commands.cpp`. The switch is
`kCautiousRouteBound` in `include/bsp/ai_command_tick.hpp`.

## 1. `00A14DD0`, read whole (`disasm-raw`, 00A14DD0-00A152A7)

`__thiscall(base = command+20h)(group, point)`, `RET 8`. In `base`, `+4h` is the flag and `+8h` is
the counter (`+24h` / `+28h` of the command, 0 and 4 from 00A109B0).

1. **The gate.** The first member of `group+5640h` must pass 009FE080 (00A14E0F). An empty list is
   the CRT abort 00BF6713.
2. **The slot.** 00778860 on that leader returns its director's path object 0. Its `vt[+4h]`
   (0071FC30) is `MOV AL,1`.
3. **No route.** If `vt[+4h]` is false or `counter < 1` (00A14E50 `JL`), 00A02020(leader, point)
   issues the moveto.
4. `vt[+0Ch]` (0071FC40, "attached": the follower's source at `+14h` is this object's point vector
   at `+40h`). If it is false, the flag is cleared (00A14E63).
5. With the flag set, `vt[+10h]` (0071D2A0: `[+18h]` when attached, else -1) at or below 0 returns
   (00A14E77). This waits while the leader is still on the first leg.
6. **Clear.** If attached (00A14E86), `clearorders` goes to the leader: 004F1830(1) builds the
   descriptor, then `0077D600(00E08F08, desc)` at 00A14EA7. The flag is cleared and the pass returns.
7. **Build** (00A14EB5..00A1525C):
   - `team` = `(group+5638h == 0)`.
   - `lead` = 00A10C20, the leader point, and `d` = `point - lead` (floats).
   - `off` = `d` turned by 0042B490(00CE3C64 = pi/2) and scaled by 0.3 (double 00CE3DC8). 0042B490 is
     `BSP_Matrix_BuildRotationY(-0.0 - pi/2)` and 00439820 applies it.
   - `n` = counter. The counter is decremented. `step` = `d / n`.
   - For `k` = `n-1` down to 1, `base` = `lead + step*k`. The candidates are `base`, `base+off` and
     `base-off`. The one with the least 00A010F0 cost wins; ties go to the earlier candidate (a strict
     `FCOMPI`, seeded with 1.0e10 at 00CE4970). The winner is pushed into a local vector.
   - The vector is sent from its last element down, which is the nearest point first, through the
     slot's `vt[+18h]` (0071D340). The target follows (00A15254), and the flag is set.

**00A010F0**, the danger cost, `__fastcall(point, int team)`:
- It walks `[[00E188A8]+19CCh]+58h` and keeps the entities whose `+54h` equals `team`.
- For each it takes the x/z distance (0 at or below 1e-10) and
  `w = 00419010(PartyPresence_DistanceMin -> 1, PartyPresence_DistanceMax -> 0, dist)`.
- While `w > 0` it adds `009FDF30([entity+0C4h]) * w`.
- This installation's `highlvlaiglobals.lua` (mtime 2024-07-13) sets Min = 2000 and Max = 4000 in all
  seven mode tables.
- The Hungarian comment on those lines reads: whoever is within Min weighs 1, beyond Max 0. So the cost
  measures enemy presence.

## 2. The receiver, the director's user path

- **0071D340** (`vt[+18h]`) builds MT_GAMEUNIT_ADDUSERPATHPOINT (5Fh, vtable 00CFDA14, the point at
  `+20h`, presence 1) and sends it with `0077C2A0(unit, msg, 7, 0)`.
- **00721A40's 5Fh arm, 007207C0** (listing read):
  - 0071DC80 finds the last queued command. It answers 1 (new path) for an empty queue, for a last
    command that is not `moveonpath`, or for a `moveonpath` whose descriptor object is of type 12h.
    It answers 0 (append) for a `moveonpath` with descriptor kind 0.
  - **New:** if director `vtable[34h]` 00835E90 accepts `moveonpath`, then 0071FDE0(point, 1, n) runs
    on slot object `n`. It erases the object's points, pushes this one, and queues `moveonpath` with a
    zeroed descriptor through director `vtable[60h]` 008358D0.
  - **Append:** 0071FDE0(point, 0, n-1) runs on the last slot's object. It returns when more than 7
    points lie ahead of `[+18h]`.
- **0071F600's descriptor-kind-0 arm** begins that `moveonpath`. Slot 0's `vtable[8]` resets it, the
  path source is built over its own points (`BSP_EntityPathSource_CreateForSlot`), and the cursor starts
  with the pair {1, 5}. It is then "attached".
- **Not reproduced:**
  - 0071FDE0's out-of-map crossing push (004BBDD0). It is counted as `outside`.
  - The session echo (unit `vtable[13Ch]` and message 57h).

## 3. Substitutions (LABELLED in the code)

- The commands host keeps one user path per director, the one the last 5Fh named. `+18h` is taken as
  the follower's current index.
- A squadron leader has no director in this host, so it answers "no slot" and keeps the moveto.
- The danger list is the host's active units in index order. The two distances are the installation's
  values, because the host's tuning block carries only its reconstructed keys.
- 5Fh is delivered on the session drain when `kSetCommandQueueDelayBound` is set, otherwise at the call.
- 00A11690, the pass after the followers, stays unread. It opens on the leader's formation `+284h`.
- CAUTIOUSMOVE (00A152B0) calls the same 00A14DD0 on its `+14h` base. It is not bound here.

## 4. Predictions, written before any ON run

OFF logs: `local\s5r_off_usn10.log` and `local\s5r_off_usn12.log` (cc9-ships5).
- In both, the cautious leader (Atlanta-class 01, Montpelier) is in `cruise` when CAUTIOUSATTACK starts.
  USN12's first command is at 6.10 s, USN10's at 10.65 s.
- The command tick runs about every 3 s.
- `tick_orders` is 95 on USN10 and 49 on USN12.

`cruise` is category 3 and is not a weapon, so 00835E90 accepts a queued `moveonpath` behind it. No
director arm ends `cruise` for a queued category-3 command. The generic arrival needs a last command of
category 1 or 2. So:

- **The user `moveonpath` stays queued behind `cruise` and never attaches.** No `clearorders` and no
  wait are expected.
- **Each cautious leader builds on four consecutive ticks**, with 3, 2, 1 and 0 waypoints:
  - that is 4 + 3 + 2 + 1 = 10 points sent;
  - build 1 queues a new user path with 4 points, and build 2 appends 3;
  - build 3 appends one and drops the target (8 ahead);
  - build 4's target is dropped.
- **The fifth tick issues the moveto.** It clears the queue as in OFF.
- **Effect:** the leader's first moveto comes four ticks late (about 12 s). Nothing else changes in the
  arm.
  - USN12: `tick_orders` 49 -> 45; builds=4, points=10, clears=0, movetos below the OFF leader orders
    by 4. Montpelier leaves `cruise` about 12 s later, so its track and the fight move (exit 3).
  - USN10: `tick_orders` 95 -> 91; builds=4 and points=10 for Atlanta-class 01 only. The Cleveland
    group holds MOVETOATTACK. Exit 3.
- **USN04, USN02 and USN01** hold no CAUTIOUSATTACK (section 11.4): identity (exit 0 or 1), builds=0.
- **Verdict rule:** a build count or point count other than above, or a moved reference row, keeps the
  switch OFF.

## 5. The first pair, and why the switch stayed OFF

Same tree, commit `3743a0548`. OFF is `local\s5r_off2_<m>.log` and ON (`kCautiousRouteBound` alone,
`local\s5r_on`) is `local\s5r_on_<m>.log`.

| mission | pair_diff | builds | points | clears | waits | `tick_orders` OFF -> ON |
| --- | --- | --- | --- | --- | --- | --- |
| USN10 3200/3000 | exit 3 | 1 | 4 | 0 | 48 | 95 -> 46 |
| USN12 3200/3000 | exit 3 | 1 | 4 | 21 | 27 | 49 -> 0 |
| USN01 3200/3000 | exit 1 | 0 | 0 | 0 | 0 | 74 -> 74 |
| USN04 4700/4500 | exit 1 | 0 | 0 | 0 | 0 | 78 -> 78 |
| USN02 9200/9000 | exit 1 | 0 | 0 | 0 | 0 | 1 -> 1 |

**The mechanism prediction failed.**
- **`cruise` did not hold the user path back.** In this host the queued `moveonpath` became the head.
  Montpelier runs `moveonpath` from step 270 and closes on the first waypoint at about 16.5 m/s.
  - Montpelier reached the first waypoint at 90.75 s.
  - Atlanta-class 01 never reached its first waypoint (the leg is a quarter of 7.9 km), so it waited
    on every tick (48 waits).
- **`clearorders` did nothing.** From 90.75 s, Montpelier issued it on every tick, 21 times. The
  host's `clearorders` arm (008171BD in 00816E30) reaches `0071D880` SendClearCommands, and that call
  was only a record in this host. So the path stayed attached, and neither a rebuild nor the moveto
  ever came.

**Verdict:** `kCautiousRouteBound` stays OFF under the protocol. The reference rows are identical.

**The fix.** `kClearOrdersSendBound` (`include/bsp/game_hosts_commands.hpp`) binds 0071D880 as the
every-slot clear. That is the same body `clear_all_commands` already runs for a flagged order. No OFF
run on USN01, USN02, USN04, USN10 or USN12 sends `clearorders` (`send_clear_commands` is absent
from all five OFF logs), so the arm is identity there by itself.

## 6. Predictions for the second pair (both switches), written before its ON runs

- **USN12 3200/3000:**
  - It is identical to the first ON run up to 90.75 s: the build at 6.10 s with 3 waypoints, then
    the follow.
  - At 90.75 s `clearorders` empties Montpelier's queue. The next tick (about 3 s later) builds 2
    waypoints plus the target from the current position, and Montpelier follows the new path.
  - By 150 s: builds 2 or 3, clears 1 or 2, no moveto (`tick_orders` 0).
  - Against OFF it is exit 3. Against the first ON run it moves only after 90.75 s.
- **USN10 3200/3000:** it is identical to the first ON run in gameplay, because Atlanta-class 01 never
  passes its first waypoint. So builds=1, clears=0 and waits=48, and it is exit 3 against OFF.
- **USN01, USN02, USN04:** identity against OFF, with builds=0 and clearorders=0.
- **Verdict rule:** as in section 4. Also, a USN12 clear that does not end the attachment keeps both
  switches OFF.
