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

## 7. The second pair and the verdict

Commit `1c69af06e`. OFF is `local\s5r_off3_<m>.log`. ON is `pair_export --flip kCautiousRouteBound=true
--flip kClearOrdersSendBound=true` into `local\s5r_on2`, with logs `local\s5r_on2_<m>.log`.

| mission | pair_diff | builds | points | clears | waits | `tick_orders` OFF -> ON |
| --- | --- | --- | --- | --- | --- | --- |
| USN12 3200/3000 | exit 3 | 2 | 7 | 1 | 46 | 49 -> 0 |
| USN10 3200/3000 | exit 3 | 1 | 4 | 0 | 48 | 95 -> 46 |
| USN01 3200/3000 | exit 1 | 0 | 0 | 0 | 0 | 74 -> 74 |
| USN04 4700/4500 | exit 1 | 0 | 0 | 0 | 0 | 78 -> 78 |
| USN02 9200/9000 | exit 1 | 0 | 0 | 0 | 0 | 1 -> 1 |

**USN12:**
- It builds at 6.10 s with 3 waypoints.
- It issues `clearorders` at 90.75 s. The queue empties this time.
- It rebuilds at 93.80 s from (-1060, -4573) with 2 waypoints. Two user paths are queued and none is
  dropped.
- Montpelier moves 2247.71 m (OFF 2428.02).

**USN10:**
- Gameplay is identical to the first ON run: pair_diff of `s5r_on_usn10` against `s5r_on2_usn10`
  exits 1.
- Hit records go from 18 to 23 and damage from 10158.5 to 19077.2 against OFF.

**Verdict: both switches ON.** Every prediction of section 6 held.

**Left open:**
- 00A11690.
- CAUTIOUSMOVE's own call of 00A14DD0 (00A152B0).
- The out-of-map crossing of 0071FDE0 (`outside_map` stayed 0).
- What +18h really tracks (the listener at `+10h`).
- The session delivery of 5Fh with `kSetCommandQueueDelayBound`.

## 8. Does the image attach the path behind `cruise`? The merged head, and the re-pairs

**The image does attach the path, so the section 4 premise was wrong about the image, not the host.**
- The queued `moveonpath` ends `cruise` through the director step's pre-pass 00836941
  (`weapon_director_step_prepass_00836941`, 00836962..00836985). When the head's stage is running
  (1) and 0071BE60 counts more than one filled slot, or the unit is player-controlled, it raises the
  head's stage to 2 with 0071D810.
- That rule tests neither the head's category nor the queued command's category. So `cruise`
  (category 3) ends as soon as the `moveonpath` (category 3) is queued behind it.
- The first ON log shows exactly that at 6.10 s: `raise_primary_stage`, `build_clear_command` and
  `apply_clear_command`, then "Montpelier cleared `cruise` from slot 0 ... the queue now holds
  `moveonpath`", then `begin_user_path_0071f6a5`.
- My section 4 test ("no director arm ends `cruise` for a queued category-3 command") looked only at the
  per-command arms (stop, follow, attackmove, the generic arrival) and missed the pre-pass.
- **The route arm builds waypoints the image also follows.**

**The `clearorders` arm on main.** Main (`8d9b938c7`, merged here as `2d37190cf`) posts 0071D880 as a 5Dh
every-slot message for 00816E30's clear-all at 0081733E and for 0071E5AA's drop
(`kSetCommandClearAllMessageBound`, cc9-gunnery6). The `clearorders` arm's own 0071D880 call at 008171BD
is still `record("EntityCommandArm::send_clear_commands")` there. So `kClearOrdersSendBound` is not
redundant. After the merge it posts the same message through `route_clear_command`, delivered at the
row-9 drain like the others. Section 31's rule holds: 00A14DD0 reads the director only on a later
command tick, seconds after any delivery. Both route switches are OFF on `2d37190cf`.

**Predictions for the re-pairs on `2d37190cf`, written before their ON runs.** OFF is `local\s5m_off_<m>.log`.
- **`kCautiousRouteBound` alone (`local\s5m_route`):**
  - USN12: one build at about 6.1 s (3 waypoints and the target). Montpelier follows the user path.
    Once it passes the first waypoint (about 90 s), a `clearorders` on every tick leaves the path
    attached: builds=1, clears in the twenties, no moveto.
  - USN10: builds=1, clears=0, waits in the forties.
  - USN01, USN04 and USN02: builds=0, identity (exit 0 or 1).
  - **Verdict rule:** as in section 4. The switch cannot flip alone, because its clears are empty
    without the second switch.
- **Both switches (`local\s5m_both`):**
  - USN12: as in section 7. The first `clearorders` empties the queue (on the drain), and the next tick
    rebuilds with 2 waypoints: builds 2 or 3, clears 1 or 2, no moveto.
  - USN10: as the route-alone run.
  - The references: identity.
  - **Verdict rule:** as in section 6.
- Timings may differ by a step from sections 5-7. `kSetCommandQueueDelayBound` now delivers the 5Fh and
  5Dh messages at the drain.

**Addendum to section 8: which prediction the pre-pass changed.**
- Only the section 4 premise changed: "the user `moveonpath` stays queued behind `cruise` and never
  attaches".
- With 00836941 read, the path attaches one director step after its 5Fh delivery. The predicted
  sequence of four builds (10 points, 2 dropped) followed by a late moveto is therefore not the
  image's.
- In its place, section 6 and section 8 predict one build, a follow, and a clear-and-rebuild once the
  first waypoint is passed. Section 9 measured exactly that.
- The counts of points per build, the 8-ahead limit and the no-slot rule were not affected.

## 9. The re-pairs on the merged head, and the verdict

Commit `2d37190cf` (main `8d9b938c7` merged). OFF is `local\s5m_off_<m>.log`. ON is `local\s5m_route` (the
route switch alone) and `local\s5m_both` (both switches).

| mission | route alone: exit, builds / clears / waits | both: exit, builds / clears / waits | `tick_orders` OFF -> both |
| --- | --- | --- | --- |
| USN12 3200/3000 | 3, 1 / 21 / 27 | 3, 2 / 1 / 46 | 49 -> 0 |
| USN10 3200/3000 | 3, 1 / 0 / 47 | 3, 1 / 0 / 47 | 94 -> 46 |
| USN01 3200/3000 | 1, 0 / 0 / 0 | 1, 0 / 0 / 0 | 74 -> 74 |
| USN04 4700/4500 | 1, 0 / 0 / 0 | 1, 0 / 0 / 0 | 78 -> 78 |
| USN02 9200/9000 | 1, 0 / 0 / 0 | 1, 0 / 0 / 0 | 1 -> 1 |

**USN12 with both switches:**
- It builds at 6.10 s, issues `clearorders` at 90.75 s (the queue empties on the drain), and rebuilds at
  93.80 s with 2 waypoints.
- Two user paths are queued and none is dropped.
- Montpelier moves 2247.37 m (OFF 2428.02).

**USN10:** the both-switches run is gameplay-identical to the route-alone run (exit 1). Against OFF, hit
records go from 19 to 26 and damage from 9944.7 to 20582.0.

**Verdict: both switches ON.** Every prediction of section 8 held. Sections 5-7 were the pre-merge
record, and this section supersedes their verdict.

## 10. Open item 1: `00A11690`, the threat-facing wedge (read, not bound)

`__fastcall(base)`, body 00A11690-00A11AEC, listing read whole. It runs after the follower pass in
CAUTIOUSATTACK (00A15350), DEFENDPOSITION and CAUTIOUSMOVE. It is the sibling of the move family's
00A11070, and the two are docs/AI_COMMAND_TICK.md's "ai_group_formation_shape" family. Neither is bound
in this host.

1. **The gate.** The group's first member must answer `vtable[5Ch](6)` (a ship). Its formation
   `+284h` must be non-null.
2. **00A113D0, the threat direction.** For `a = 0; a < 2pi (00CE3828); a += pi/6` (the double at
   00CEC730, twelve samples):
   - `d = (sin a * 750, 0 * 750, cos a * 750)` (the double 750.0 at 00D22C70);
   - `cost = 00A010F0(leader+FCh + d, group+5638h == 0)`;
   - the strict greatest cost, seeded with -1.0e10 at 00CE4ADC, keeps its `d`.
   An empty group samples around the zero vector at 00F87574.
3. **The frame** (00A1172B..00A11837):
   - `h = atan2([leader+0ECh], [leader+0F4h])` (_CIatan2 with ST0 = z, ST1 = x), then `a = -h`.
   - `u = normalize2(cos a * d[0] + sin a * d[1], cos a * d[1] - sin a * d[0])`, through 004F2F40.
     **The listing reads `d[0]` and `d[1]` (`[ESP+2Ch]`, `[ESP+30h]`), and `d[1]` is the zero y.**
     So `u` is `+/-(cos h, sin h)` by the sign of the threat's x alone. The threat's z never enters.
     This is the image's own arithmetic.
   - `v` is the same with `a + pi/2` (the double pi/2 at 00CE3830). It is negated (the double -1.0
     at 00D7A250) when its second component is negative.
4. **The shape.** 0070EFD0(0) on the group (`+14h` of the formation object). Shape 0 indexes the table
   base 00E08F18, which holds command-object pointers, so column 0 of every member is rewritten from
   those words read as floats times FormationShipDist. The ship branch below overwrites them;
   non-ship members keep them.
5. **The wedge.** `s = tuning+210h` (Formation_UnitDist, 500 in this installation's
   highlvlaiglobals.lua line 125). `F = s*u` and `P = s*v`.
   - The members are walked from the second entry of `+5640h`; ship members with a record (0070D080)
     are placed in rows `r = 1, 2, ...` of `2r + 1` places `j = 0..2r`:
     `off = r*F`, plus `j*(P - F)` for `0 < j <= r`, plus `(j - r)*(-P - F)` for `j > r`.
   - `record+10h = -0.0 - off.x` and `record+20h = -0.0 - off.z`, which are column 0's lateral and
     axial.
   - A row ends when `j` exceeds `2r` (the limit starts at 2 and grows by 2).
6. **The tail.** 0077A080 and 0077C880 on the formation (`+284h`) send the formation update.

**Why it is not bound here.** The column-0 records live in the units host's formation model
(`src/game_hosts_units.cpp`, `record.lateral[0]` at the 0070D7B0 decomposition). That file is leased
to cc9-lua6. The binding needs one units-host entry point:
`bool set_formation_member_offset_0070d080(std::size_t leader, std::size_t member, int column,
float lateral, float axial)`, false without a record. The shape-0 rewrite of 0070EFD0 would be a
second entry point. With them, the rule above is pure arithmetic over the AI host's danger cost, so
it can sit in `src/ai_command_tick.cpp` beside 00A14DD0.
