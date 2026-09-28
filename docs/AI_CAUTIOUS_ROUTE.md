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

## 11. Open items 2-5

**2. CAUTIOUSMOVE's call, 00A152B0 (bound OFF as `kCautiousMoveRouteBound`).**
- 00A152B0 is `00A14DD0(this+14h)(this+4h group, this+8h destination)`, then `00A10DC0`, then
  `JMP 00A11690`.
- Its constructor 00A102D0 (vtable 00D22AEC, instance 20h) sets `+14h` to vtable 00D22A08 and then
  00D22AE4, the flag `+18h` = 0, and the counter `+1Ch` = 4. That is the same base CAUTIOUSATTACK
  keeps at `+20h`.
- The switch runs `ai_cautious_approach_pass_00a14dd0` toward `+8h` before the follower pass.
- **Reach: none in this host.** CAUTIOUSMOVE is built only by 00A13340 (`BSP_AiCommand_CreateFromLua`)
  and by 00A2CCF0. 00A2CCF0 is reached from `BSP_AiPlanner_DuelThink` 00A25F70 for a non-ship leader
  when 00A2C9F0 passes and a stream-1 fraction exceeds the argument. No script in this installation
  names CAUTIOUSMOVE, and this host binds neither creator.
- **Prediction, written before its ON run:** identity on USN12 and USN04 (exit 0 or 1), with the
  cautious-route census unchanged.

**3. The out-of-map crossing of 0071FDE0.** 004BBDD0 (`__thiscall(world)(from, to, out)`, read whole):
- It clips `to` into the world box (`+711Ch`/`+7128h` in x, `+7130h`/`+7124h` in z), placing it 1 m
  inside along the segment from `from`.
- It keeps the original point when the clipped move is under 1.0 on either axis (00D7A24C).
- `out` = clipped + `normalize(...) * (00CE7530 / max(min(|n.x|, |n.z|), 00D7A2F8 -> 00CE746C))`.
- 0071FDE0 pushes the clipped point and then `out`, instead of the point.
- **Not bound: no reach.** `outside_map` stayed 0 on every run of sections 5-9. The route's
  candidates lie between two leaders, 0.3 of their distance off the line.

**4. The slot's `+18h`: confirmed as the follower's index, delivered by message.**
- The listener sub-object at `+10h` (vtable 00CFDB10) has slot 2 at 0071CDD0. That slot builds session
  message 60h with its argument at `+20h` and routes it with flags 7.
- 00721A40's 60h arm is 0071C0B0, `ECX = [director+1A4h] + 10h`, `JMP 007AE060`. It stores the
  argument at listener `+8h` = slot `+18h` when it is at most the source's point count - 1.
- So `+18h` is the index the follower reports, one session delivery late.
- The host reads the cursor's own index (`path_cursor.index_08`). That is the same value without the
  delivery delay. It stays LABELLED.

**5. 5Fh delivery under `kSetCommandQueueDelayBound`.**
- `post_user_path_point_0071d340` posts a loopback message when the switch is set, and the drain
  delivers it. It delivers at the call otherwise.
- The switch is ON on main since cc9-gunnery6's landing, so the section 9 pairs were measured with the
  5Fh points delivered at the row-9 drain. Nothing is left to do.

**Item 2 pair.** Commit `1a65d172b`. OFF is `local\s5cm_off_<m>.log` and ON (`local\s5cm_on`) is `local\s5cm_on_<m>.log`:
- USN12 3200/3000: exit 0, and the route census is unchanged (builds=2, points=7, clears=1).
- USN04 4700/4500: exit 0.

The prediction held. **`kCautiousMoveRouteBound` is ON.** It takes effect once a creator of CAUTIOUSMOVE (00A2CCF0 or 00A13340) is bound.

## 12. Handoff: the wedge binding (`00A11690` / `00A113D0`) and the state of open items 2-5

Worker cc9-ships5, 2026-09-28. The branch is merged with main at `118cf99b2`. The unit-host entry point
this binding needs is queued with cc9-lua6 and has not landed yet.

**Status of the section 7 items:**

| item | state | where |
| --- | --- | --- |
| 1. `00A11690` wedge | read whole, not bound: waits on the units-host entry point | section 10, this section |
| 2. CAUTIOUSMOVE's `00A14DD0` (`00A152B0`) | bound, `kCautiousMoveRouteBound` ON (identity pair USN12 / USN04, exit 0; no creator bound) | section 11 |
| 3. the out-of-map crossing `004BBDD0` in `0071FDE0` | read, not bound: no reach (`outside_map` = 0 on every run) | section 11 |
| 4. slot `+18h` | confirmed: the follower's index delivered by message 60h (`0071CDD0` -> `0071C0B0` -> `007AE060`); the host reads the cursor index directly (LABELLED, one delivery early) | section 11 |
| 5. 5Fh delivery | done: posted to the loopback queue under `kSetCommandQueueDelayBound` (ON on main) | section 11 |

**The wedge plan.**
1. **Sync first.** Run `python tools/bsp.py sync` once lua6's sha lands. That landing adds
   `bool GameUnitsHost::set_formation_member_offset_0070d080(std::size_t leader, std::size_t member,
   int column, float lateral, float axial)`, which is false without a record.
2. **The store sites it must reproduce.**
   - `00A11A57` writes `MOVSS [EAX+10h]` (lateral, column 0).
   - `00A11A5C` writes `MOVSS [EAX+20h]` (axial, column 0).
   - `EAX` is 0070D080's record for the member.
   - The values are `-0.0 - off.x` and `-0.0 - off.z` (`00D7A208`, `SUBSS`).
   - In the units host these are the same two fields `record.lateral[0]` / `record.axial[0]` that the
     join decomposition writes at `src/game_hosts_units.cpp` 18790/18791.
3. **The shape-0 rewrite.**
   - `0070EFD0(0)` at `00A11866` first rewrites column 0 of every member, including non-ships. It
     reads `00E08F18 + j*8` as floats (command-object pointers, so values near 0) times
     FormationShipDist, and times 1.5 on z.
   - The leader's record gets (0, 0) (`0070F03A`).
   - Ask lua6 for a second entry point, or reproduce it through the first one: 0 for the leader, and
     for non-ship members the pointer-as-float products (denormal, effectively 0; LABELLED).
4. **The pure rule** goes in `src/ai_command_tick.cpp` as `ai_formation_wedge_00a11690` over the
   `AiCommandTickHost`. It needs:
   - the leader forward row `+0ECh/+0F4h`: the heading is `atan2(x, z)` (_CIatan2 with ST0 = z);
   - the leader position;
   - `tick_danger_cost_00a010f0` (bound);
   - `Formation_UnitDist` (tuning `+210h`, `kAiTuningFormationUnitDist`, 500 in this installation).
   **Keep the image's quirk.** The frame rotates `(d[0], d[1])` and `d[1]` is the zero y, so the
   threat's z never enters. The x87 order is in section 10. `004F2F40` is
   `BSP_Geometry_NormalizeVector2DWithCutoff`; read its cutoff before binding.
5. **Callers.** It runs after the follower pass in CAUTIOUSATTACK (`00A15350`), CAUTIOUSMOVE
   (`00A152CD` `JMP`) and DEFENDPOSITION (`00A15500`). One switch, `kCautiousWedgeBound`, starts OFF.
6. **The tail.** `0077A080` / `0077C880` on the formation `+284h` is the formation-update send.
   Record it unless lua6's entry point already raises the equivalent refresh.

**Prediction to write before the ON run (USN12 3200/3000, route switches ON).**
- The Montpelier group (12 members, a ship leader with a formation) gets a wedge on every
  CAUTIOUSATTACK tick from 6.10 s on.
- Ship members take rows `r = 1, 2, 3` of 3, 5 and 7 places at 500 m spacing. The 11 followers fill
  rows 1-2 (8 places) and 3 of row 3.
- Their station errors move and the fight moves: exit 3.
- USN10's Atlanta-class 01 group moves the same way.
- USN01, USN04 and USN02 hold no CAUTIOUSATTACK. Also check that no DEFENDPOSITION group there has a
  ship leader with a formation, because those groups would run the pass too. List them from the OFF
  census before predicting identity.

## 13. The wedge bound OFF (`kCautiousWedgeBound`), corrections, and the predictions

Worker cc9-ships6, 2026-09-28, packet `cc9_cautious_wedge`. Code: `ai_formation_wedge_00a11690` in
`src/ai_command_tick.cpp`, called after the follower pass of CAUTIOUSATTACK, CAUTIOUSMOVE and
DEFENDPOSITION; the host methods and the census are in `src/game_hosts_ai.cpp`. The column-0 writes go
through the units-host entry point `GameUnitsHost::set_formation_member_offset_0070d080` (cc9-lua6).

**The routines.**

| routine | ABI | body (exclusive end) | coverage |
| --- | --- | --- | --- |
| `00A11690` wedge | `__fastcall(base)`, `[base+4h]` the owner group, `RET` | `00A11690`-`00A11AED` | complete except the tail `0077A080`/`0077C880` (replication, below) |
| `00A113D0` threat direction | `__thiscall(base)(float out[3])`, `RET 4` | `00A113D0`-`00A1152A` | complete |
| `004F2F40` normalize | `__thiscall(float v[2])`, `RET` | `004F2F40`-`004F2FAE` | complete (already named `BSP_Geometry_NormalizeVector2DWithCutoff`) |

**Corrections to section 10.**
- **13 samples, not 12.** The angle is stored as binary32 after each `FADD` of the double pi/6 at
  `00CEC730`, and the loop continues while the double at `00CE3828` (6.2831854820251465, 2pi rounded
  to binary32) is above it. The accumulated angle reaches 6.2831845 before the test fails, so a 13th
  sample runs at essentially 2pi. It only wins on a strictly greater cost than sample 0.
- **A fourth caller.** RETREAT's tick `00A156E0` (vtable `00D22A60`, slot `+0Ch` at `00D22A6C`) runs
  `00A10DC0` and then `00A11690` at `00A156EE`. RETREAT's tick is not bound in this host, so that call
  has no reach here.
- **The normalizer's cutoff.** `004F2F40` compares the squared length with the double 1e-10 at
  `00CE3820` and divides by the double 1e-5 at `00CE3C70` at or below it. A zero `d[0]` therefore
  gives `u = v = (0, 0)`, not a unit vector.

**What a zero danger cost does.** The seed is -1e10 and the test is strict, so when every sample
costs the same the first sample wins. Sample 0 is `a = 0`: `d = (750 sin 0, 0, 750 cos 0) = (0, 0,
750)`. Its `d[0]` is exactly zero, so `u` and `v` normalize to `(0, 0)`, `F = P = 0`, and every placed
member gets `(-0.0, -0.0)`. The wedge then puts every ship member's column-0 station on the leader's
own point. This is the image's arithmetic; the host's danger cost is LABELLED (its entity list is
the active units, and `PartyPresence_DistanceMin/Max` come from this installation's
highlvlaiglobals.lua, 2000 and 4000), so a zero here depends on that substitution being faithful.

**Does the host need `0070EFD0(0)`?**
- For ship members of the AI group with a record, no: the wedge overwrites their column 0 in the same
  call.
- It matters for records the wedge does not place: non-ship members, and formation members that are
  not in the AI group's list. `0070EFD0(0)` sets their column 0 to about 1e-38 m, because the shape-0
  "table" at `00E08F18` is the registered command objects (vtable, ordinal) of
  `bsp/entity_orders.hpp`. That puts those stations on the leader's point too.
- So the host reproduces it through the same entry point: every live record of the leader's
  formation gets `(0.0, 0.0)` first. LABELLED: 0.0 instead of the ~1e-38 products.
- On USN12 the Montpelier formation is exactly the AI group's 12 ships, so the rewrite changes
  nothing observable there.

**The tail.** `0077A080` builds MT_FORMATION_SET (78h, `00D02348`) from the group's `+500h` column and
every record's four columns, and `0077C880` sends it to the other peers (`0077C7B0`), or to the
secondary's first peer when `[00E188A8+1FE4h]` is 2. A single-player session delivers nothing, so the
host does not model it.

**Host methods** (`AiCommandTickHost`, all in `src/game_hosts_ai.cpp`):

| method | native | contract |
| --- | --- | --- |
| `tick_member_forward_row` | entity `+ECh..+F4h` (`00A1172B`, `00A11732`) | `GameUnitsHost::unit_pose` forward row |
| `tick_member_has_formation_0284` | entity `+284h` (`00A116F3`) | `unit_formation_group_0284 >= 0` |
| `tick_formation_shape0_0070efd0` | `0070EFD0(0)` at `00A11866` | every live record of the leader's formation to (0, 0), through the entry point |
| `tick_formation_set_column0_0070d080` | `0070D080` at `00A1193E`, stores `00A11A57`/`00A11A5C` | the entry point with column 0; false without a record |
| (existing) `tick_danger_cost_00a010f0` | `00A010F0` at `00A114B9` | `EDX = (group+5638h == 0)` |
| (existing) `tick_tuning_field(0x210)` | `00A371A0` at `00A11879` | Formation_UnitDist, 500 |

The census is one line per wedge run (`ai cautious wedge:`, the first 60) and the summary
`summary mission ai cautious wedge bound= calls= runs= placed= no_record= zero_frame=`. `calls` counts
every call by a bound tick. `runs` counts calls past both gates. `zero_frame` counts runs where `u` is
`(0, 0)`.

**Predictions, written before any ON run.** OFF is the tree's own build. ON is `pair_export --flip
kCautiousWedgeBound=true` into `local\ships6_<row>`.

- **USN12 3200/3000.**
  - Groups (reference-era OFF log `s5m_both_usn12`): CAUTIOUSATTACK led by Montpelier (12 ships, 11
    followers joined to its formation at start), DEFENDPOSITION led by Fortress-07 (4 members), and the
    player's NONCONTROL group of three destroyers led by Shigure at (3500, -5859).
  - Fortress-07 is not a ship base, so its calls stop at the first gate: `calls` exceeds `runs`.
  - The first run is at 6.10 s, in the tick of the first route build. Montpelier is near (-2440,
    -4202), about 6.1 km from Shigure, so every ring sample is beyond 4000 m of every player ship
    and costs 0.
  - First wedge line: `samples=13`, `threat=(0.0 0.0 750.0)`, `cost=0.0`, `u=(0.0000 0.0000)`,
    `v=(0.0000 0.0000)`, `dist=500`, `shape0=12` (11 if the entry point does not answer the leader's
    own record, a host detail rather than a mechanism failure), `placed=11`, `not_ship=0`, `no_record=0`, and both
    offsets `(-0.0, -0.0)`.
  - `zero_frame` equals `runs` while Montpelier stays beyond 4750 m of all three player ships. In the
    OFF run it was still about 4.7 km away at 93.8 s. Any later run with a non-zero cost has
    `u = +/-(cos h, sin h)`, the sign being that of the threat's x.
  - **Positions.** From 6.10 s every follower's station is Montpelier's own point (across 0, along 0)
    instead of the join offsets (across about +/-500 m, along 200 to 1700 m). The eleven followers
    close on Montpelier and bunch around it. Montpelier's own route is the same unless ship avoidance
    turns it.
  - `pair_diff` exits 3. The fight measures may move; no sign is predicted for them.
- **USN04 4700/4500.** Reference i (`local\rb9_usn04.log`, main `d466d4250`) has `cautious=0` in the
  parties census and `defendposition=0` on both defend paths. The 00A28A60 records path has
  `records=0`. So no group runs a wedge-calling tick: `calls=0`, and `pair_diff` exits 0 or 1. It is 1
  if the summary's `bound=` value counts as a difference.

**Verdict rule.**
- A mechanism failure keeps the switch OFF and is recorded. That is a wrong census on USN12's first
  line, a non-zero `u` while every cost is 0, or a USN04 call.
- A spread miss with the mechanism matching may flip, and is recorded.

## 14. The wedge pair, and the verdict

The runs use commit `d12dcbd70` (main, with the units-host entry point). OFF is the tree's own build,
`local\ships6_off_<m>.log` (bsp_game.exe SHA-256 prefix `07B3A099D1F5`). ON is `pair_export --flip
kCautiousWedgeBound=true` into `local\ships6_on` (prefix `A1AC42A26F49`), with logs
`local\ships6_on_<m>.log`. Both use `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`, lockstep
0.05, an idle player, present interval immediate, and `tools/run_game.ps1`. A 300-frame OFF smoke
of USN12 (`local\ships6_smoke.log`) exited 0 with `bound=0 calls=0`.

| mission | pair_diff | calls / runs / placed / no_record / zero_frame | unit table |
| --- | --- | --- | --- |
| USN12 3200/3000 | exit 1 (gameplay identical) | 99 / 49 / 539 / 0 / 26 | identical, Montpelier 2247.37 m both |
| USN04 4700/4500 | exit 1 (gameplay identical) | 0 / 0 / 0 / 0 / 0 | identical (81 rows) |

**The mechanism matched the prediction line for line.**
- The first USN12 line is at 6.10 s: `samples=13 threat=(0.0 0.0 750.0) cost=0.0 u=(-0.0000 0.0000)
  v=(0.0000 0.0000) dist=500 shape0=12 placed=11 not_ship=0 no_record=0`.
- Its offsets are `first=(0.0 -0.0)` and `last=(-0.0 -0.0)`. The first place's lateral is +0.0
  because `u[0]` is -0.0, so `F[0]` and `off[0]` are -0.0, and -0.0 - (-0.0) = +0.0.
- **Corrected (the first version of this section misread the census).** The summary is
  `calls=99 runs=49 placed=539 no_record=0 zero_frame=26`. The first 26 runs (6.10 s to 82.25 s) have a
  zero frame and write (-0, -0), with one +0 lateral. From 84.65 s the 23 remaining runs keep
  `threat=(750.0 0.0 -0.0)`, the eastward sample, with a cost of 0.1 rising to 0.7 by 90.75 s.
  Montpelier is then within 4750 m of the player ships. Those runs have `u=(-0.2699 0.9629)` and
  `v=(0.9629 0.2699)`, and the wedge writes real rows. The end-of-run formation dump shows them, for
  example Cleveland (431.46, -252.66), Charles (862.93, -505.33) and Foote (-73.86, -1115.59). The OFF
  dump keeps the join offsets (-13.10, 450.00) and so on.
- So the all-zero-cost case did stack every ship member's station on Montpelier's own point
  (offsets -0, -0) until 82.25 s. This is the image's own arithmetic (sections 10 and 13).
  After that the stations form the threat-facing wedge.
- **Fortress-07 did call.** `calls` exceeds `runs` by 50, which is the DEFENDPOSITION group's 50
  ticks. Every one stopped at the first gate, because Fortress-07 is not a ship base. That is the
  prediction's `calls > runs`. The earlier sentence here, that the group never called, was wrong.
- USN04 made no call, as predicted.

**Do the followers pile onto Montpelier? No, and the follower pass does not separate them either.
Nothing in this host reads the new stations on USN12.**
- The formation group is `column=0` with all 12 ships (log: `formation group 0: leader=Montpelier
  type=6 column=0`), so the writes land in the live column.
- The one motion consumer of column 0 is the `follow` state's station point, 0070D290 through
  009DF2D0 (`FollowFormationPointBinding`). It is never called in either run, because no ship
  enters `follow`. The state census has 12 `cruise`, 282 `moveonpath`, 9 `stop` and no `follow`, and
  the director reports `follow=0`. `summary mission ai follow requests=575 available=0 refused=575`
  (00779D50) is identical OFF and ON.
- The only other readers are the path planner's group extents 0070D400 / 0070D5D0 (2816 calls in
  both runs). They read each member's station `across`, which the wedge changes from about +/-500
  to 0. The picked paths are unchanged, and `path_publishes=0` in both runs. Inference: the extents
  feed a path that is not published on this row. That was not traced further.

**Verdict: the switch is ON.** The mechanism matched. The spread miss (exit 1 against a predicted
exit 3) comes from a consumer this host does not reach on USN12 (followers never enter `follow`),
not from the wedge. When a follower does enter `follow` under a CAUTIOUSATTACK, CAUTIOUSMOVE or
DEFENDPOSITION leader with no danger within 4750 m, its station will be the leader's own point, as
in the image.

**Open.**
- Why USN12's eleven followers never take `follow`: every 00779D50 follow request is refused, and
  `formation_requests=0`. That belongs to the follow-request packet, not this one.
- Whether the group extents' consumer is ever published.
- RETREAT's call at `00A156EE`, which waits on a RETREAT tick binding.

## 15. Why no USN12 follower takes `follow` (packet `cc9_follow_request`): no host defect

Worker cc9-ships6, 2026-09-28. The packet asked which test of `00779D50` the host fails on USN12's
575 refused follow requests. **Every refusal is the image's own answer.** What keeps the followers out
of `follow` is the authored `Cruise` they are placed with, and the image's director never ends it for
a formation member. Nothing was bound, so there is no `kFollowRequestBound` and no pair.

**`00779D50` BSP_Entity_MayFollowTarget, read whole this packet.**
- ABI: `__thiscall(entity)(const char* token, entity* target)`, `RET 8`.
- Body: `00779D50`-`00779E0F`, followed by `INT3` padding.
- It matches `entity_may_follow_target_00779d50` in `src/ship_ai_states.cpp` test for test.

| site | test | field or state read | refuses when |
| --- | --- | --- | --- |
| `00779D53` | follower byte | follower `+5Dh` | set |
| `00779D68` | `00438E10(token, 00CFB52C "follow")` | the command token | not `follow` |
| `00779D76` | target present | the argument | null |
| `00779D83` | target `vt+5Ch(2)` | target kind | false |
| `00779D8D`, `00779D93` | the follower's and the target's byte | `+5Dh` of each | set |
| `00779D9F` | `00803510(follower+54h, target+54h)` | party relation | not 0 (not the same party) |
| `00779DAB` | `00779820(target)` | the same entity, or the same non-null `+284h` group | true |
| `00779DB4..00779DC5` | owner player | `+188h` of each | differ and target's is not 9 (skipped in the host, no producer) |
| `00779DCC`..`00779DF9` | `vt+5Ch(6)`, `vt+5Ch(8)` of each | ship base, not kind 8 | follower or target fails |

`00779820`, `__thiscall(entity)(other)`, `RET 4`, body `00779820`-`00779847`, was read whole. It
answers 1 when `other` is the entity itself, or when `[entity+284h]` is non-null and equal to
`[other+284h]`.

**The 575 refusals on USN12, OFF and ON alike:**
- **The eleven followers fail `00779DAB`.** Their diag lines read `ship 1/1, kind2=1, party 0/0,
  alive 1/1`, so every earlier test passes. They are already in Montpelier's group, joined at load
  (`formation group 0: leader=Montpelier ... members=[Montpelier,Cleveland,...]`, `joins=11`). The
  image refuses a follow request between two members of the same group; the join already happened.
  11 followers × 49 CAUTIOUSATTACK ticks is 539.
- **Shigure, Samidare and Shiratsuyu fail `00779D9F`** (`party 1/0`). They are the player's ships,
  listed in Montpelier's AI group on the first ticks (`first_group_members=15`). 575 - 539 = 36 = 3
  ships × 12 ticks. That split is inferred from the totals, because the diag lines stop after 12.
  Whether the image would put the player's ships in that AI group at all is a grouping question
  outside this packet.

**The producer of `follow` is not the request.** As `docs/SHIP_UNIT_GROUP_FOLLOW.md` section 4
records, `follow` is issued by the follower's own director. The idle re-issue at `00836DC9` tests
`007788B0` at `00836E13`, then takes `007788D0` at `00836E28` and pushes `00E08F60` at `00836E38`.
The host models that arm (`weapon_director_idle_reissue_00836dc9`) and answers both predicates from
the group. It is reached only when the director is idle: the stage is 2 at `00836A81`, or the
queue is empty.

**USN12's followers are never idle.**
- Each of the twelve DestroyerGen ships is placed with the authored token `Cruise` (`authored token
  "Cruise" x12 resolves to command class "cruise"`).
- Their command rows read `cruise scene`. Cleveland, for example, holds `cruise` at throttle 0.598,
  heading 1.597, 10 m/s, for the whole run and moves 1490.14 m.
- In the image a `cruise` never ends for such a unit:
  - `00835C70` BSP_WeaponDirector_BeginCurrentCommand raises a `cruise` or `stop` to stage 1 and
    latches the cruise fields. It tests `007788B0` only for a `follow` command.
  - `00836920`'s per-kind arms cover `stop` (`00836A8E`), `follow` (`00836ADC`), `attackmove`
    (`00836B45`) and `moveonpath` (`00836BF0`). None of them is `cruise` (`00E08F70`).
  - The started-stage terminator at `00836962..00836985` ends a stage-1 command only when a second
    slot is filled (`0071BE60 > 1`) or the unit is player-controlled (`+184h`).
  - The follower pass `00A10DC0` pushes no command to a ship member, only the join request.
- So an AI formation follower placed with `Cruise` keeps cruising in the image as well. USN12's
  followers sail straight at their cruise speed while Montpelier runs its cautious route.

**Other missions do produce `follow`.** The reference-i logs (`local\rb9_<m>.log` in cc9-gunnery7)
show the director's follow count as USN04 17, JM06 9, USN13 8, USN02 4 and USN01 2. JM08 has 0, like
USN12. The request and producer machinery works where a follower's director goes idle.

**Consequence for the wedge (section 14).** Its writes on USN12 have no motion consumer, and the
image would have none either. The station 0070D290 is read only by the `follow` step.

**Not settled.**
- A rel32 scan of `.text` (`local\ships6_rel32.py`) finds 18 `CALL 0071D810` sites and no caller
  outside eight functions:
  - `00836920`: seven sites, all placed above.
  - `00835C70`: `00835E12`.
  - `0071DDB0`, `0071E430` EndCommand, `0071F290` CommandControllerBase_Update and `00835B40`
    RetargetCommandSlot.
  - `0084E010`: five sites. It is a vtable slot at `00D0BE14`, which is not the ship director's
    table `00D09FC4`.
  - `009B3900`.
- `00836D12` belongs to the `moveonpath` arm.
- **Closed in section 16.** The four generic director routines and `009B3900` were read there, and
  none ends a `cruise` for a formation follower.
- JM08's `follow=0` was not traced.

## 16. RETREAT (`00A156E0`) is never created, and the section 15 residual read (packet `cc9_retreat_tick`)

Worker cc9-ships6, 2026-09-28. **Step 1 (census) finds no row that creates RETREAT, so the tick is not
bound (no `kRetreatTickBound`).** Step 2 reads the five residual-risk functions of section 15 instead.

**Who can create a RETREAT (vtable `00D22A60`).** A scan of the image for the bytes `60 2A D2 00`
finds four sites:

| site | function | what it is |
| --- | --- | --- |
| `00A0FEDB` | `00A0FE90` | a type answer (`GetType` family), not a creator |
| `00A100D7` | `00A0FF60` BSP_AiCommand_DescribeType | a type answer, not a creator |
| `00A13770` | `00A13340` BSP_AiCommand_CreateFromLua | the inline construction at `00A1376E` |
| `00A2BE9A` | `00A2BE70` (the RETREAT installer in docs/AI_PLANNERS.md's table) | **no caller**: no rel32 `CALL`/`JMP` and no absolute reference to `00A2BE70` anywhere in `.text` |

- `00A13340`'s only caller is `00A37AD6`, inside `00A37A00` BSP_LuaBinding_AISetCommand (Lua
  `AISetCommand`).
- No mission script in this installation calls `AISetCommand`. That is docs/GAME_EXECUTABLE.md's
  scan of the 299 `.lua` files, re-checked here with a case-insensitive grep of `scripts/`.
- The `retreat` strings in the scripts are unit commands and unit properties
  (`GetProperty(unit, "unitcommand") == "retreat"`, `unit.Retreat`), not AI group commands.
- So in this installation the image never creates a RETREAT group command, and its tick `00A156E0`
  (with its wedge call at `00A156EE`) is unreachable.

**The census.** Every row's final group dump lists the commands its groups end on; none is RETREAT.

| row (log) | groups | final group commands | RETREAT |
| --- | --- | --- | --- |
| USN04 (`rb9_usn04`) | 3 | CLOSEATTACK, MOVETOATTACK, NONCONTROL | 0 |
| USN04 E2 (`rb9_e2`) | 2 | IDLE | 0 |
| USN01 (`rb9_usn01`) | 5 | DEFENDPOSITION, MOVETOATTACK, NONCONTROL | 0 |
| USN02 (`rb9_usn02`) | 2 | CLOSEATTACK, NONCONTROL | 0 |
| JM06 (`rb9_jm06`) | 3 | CLOSEATTACK, NONCONTROL | 0 |
| JM08 (`rb9_jm08`) | 5 | CAUTIOUSATTACK, DEFENDPOSITION, MOVETOATTACK, NONCONTROL | 0 |
| USN13 (`rb9_usn13`) | 5 | DEFENDPOSITION, MOVETOATTACK, NONCONTROL | 0 |
| BSM01 (`rb9_bsm01`) | 2 | DEFENDPOSITION, IDLE | 0 |
| LOMP06 (`rb9_lomp06`) | 4 | CLOSEATTACK, DEFENDPOSITION, NONCONTROL | 0 |
| USN12 (`ships6_on_usn12`) | 3 | CAUTIOUSATTACK, DEFENDPOSITION, NONCONTROL | 0 |

The final dump is not a creation census. The creator analysis above is what makes zero exact: the one
reachable creator is a Lua binding no script calls. RETREAT would run 0 ticks on every row.

Beside the packet: JM08 ends with a CAUTIOUSATTACK group and USN01 with DEFENDPOSITION groups, so
both rows reach the wedge `00A11690`. Neither was paired for it.

**Step 2: the section 15 residual risk, closed.** The question was whether any stage raiser ends a
`cruise` for a formation follower. None does.

| function | ABI / read | what raises the stage | follower-specific `cruise` end? |
| --- | --- | --- | --- |
| `0071E430` BSP_WeaponDirector_EndCommand | `__thiscall(controller)(command, terminal)`, `RET 8`, body `0071E430-0071E4B7`, read whole | its caller says the command is done | no. Its ten rel32 callers are the ship states `moveto` (`009E5997`), `moveonpath` (`009E5C70`), `attackmove` (`009E88C1`) and its tangent sub-state (`009F3718`); the plane tasks (`009BCB0E`, `009C312E`, `009CFB47`); and the pilot-bot routines `009F7C90` (twice) and `009F83A8` in `009F8160`, which revalidate attack and ordnance commands. The ship `cruise` step `009E1170` is not a caller, and there is no absolute reference (bytes `30 E4 71 00`) |
| `0071DDB0` | `__thiscall`, one entity argument, listing read `0071DDB0-0071DE96` | a command slot whose target resolves to the argument (an entity being removed) and whose director `vt+70h` agrees, from `[00E188A8]+5D4h >= 0Ch` | no. A `cruise` names the unit itself, so only its own removal ends it |
| `0071F290` BSP_CommandControllerBase_Update | listing read `0071F290-0071F378` | `0071F36E`: a present, not-yet-accepted command whose `vt+78h(1)` (`00835C70`) answers false | no. `00835C70` answers 1 for `cruise` unless the base begin `0071F600` fails; it tests `007788B0` only for `follow` |
| `00835B40` BSP_WeaponDirector_RetargetCommandSlot | listing read `00835B40-00835BC7` | a retarget onto the same object, only for `moveto` `00E08F68` or `attackmove` `00E08F78` (`00835BB9`, `00835BC0`) | no |
| `009B3900` | listing read `009B3900-009B398B`; sole caller `009B3F09` in BSP_SquadronLandTask_Tick | the squadron's command target differs from the task's | no: a squadron path |

With section 15's reading of `00836920` and `00835C70`, this covers every direct `CALL 0071D810` site
(section 15's rel32 scan). A scan for the absolute bytes `10 D8 71 00` finds no match, so there is
no vtable or pointer reference to it either. The same four-byte scan does find the RETREAT vtable
above, so the negative is not vacuous.

**Conclusion.** An AI formation follower placed with `Cruise` keeps cruising in the image. USN12's
followers never taking `follow` is the image's behaviour. The wedge's stations there have no motion
consumer in the image either.

## 17. The wedge on JM08 and USN01 (packet `cc9_wedge_reference_rows`)

Worker cc9-ships6, 2026-09-28. The pairs are same-tree pairs on main `4215e40de`, where
`kCautiousWedgeBound` is ON.
- **ON** is the tree's own build.
- **OFF** is `pair_export --commit 4215e40de --flip kCautiousWedgeBound=false` into
  `local\ships6_off`.
- Launch lines are reference i's: `--frames 3200 --press-start-frame 30 --menu-select <m>
  --mission-frames 3000 --mission-frame-seconds 0.05`, with `BSP_GUNNERY_RNG_STREAMS=1` and
  `BSP_DEATH_TABLE=1`, an idle player and present interval immediate, through `tools/run_game.ps1`.

**Predictions, written before any run.** They are drawn from reference i's end-of-run group dumps
(`rb9_jm08`, `rb9_usn01`) and its director census.

- **JM08.**
  - The groups that call the wedge are DEFENDPOSITION, led by "Medium Bunker, Concrete 01" (17
    members), and CAUTIOUSATTACK, led by "Wildcat #1.1" (3 members).
  - Neither leader is a ship base: one is a structure, the other a plane. Every call stops at the
    first gate, so `calls > 0`, `runs = 0` and `placed = 0`.
  - The Helena group ends on MOVETOATTACK, which does not call the wedge. If it held a
    wedge-calling command earlier in the run, `runs` would be above 0. It still could not move a
    ship, because JM08's director census has `follow=0`: no station is ever read for motion.
  - `pair_diff` exits 0 or 1. The summary line's `bound=` differs.
- **USN01.**
  - The wedge caller is DEFENDPOSITION, led by "Storage, 05 01" (8 members), a structure. So
    `calls > 0` and `runs = 0`.
  - USN01 does have `follow=2`: its director issues `follow` twice. Those followers belong to
    Enterprise's MOVETOATTACK formation, which never calls the wedge. So a wedge write could not
    reach a station that is read.
  - `pair_diff` exits 0 or 1.

**Verdict rule.** Identity on both rows confirms the switch's attribution. A row that moves keeps the
switch as it is. It is then recorded as a reference-j flag with the moved lines.

**The pairs.**
- OFF is `local\ships6_woff_<m>.log`, from the export with bsp_game.exe SHA-256 prefix
  `8F4037FD5CA0`.
- ON is `local\ships6_won_<m>.log`, from the tree's build with prefix `C5E53ED6C6CB`.
- Every log ends in the final COM release, and each one's module directory is its own binary's.

| mission | pair_diff | ON census: calls / runs / placed | death rows | unit table | director `follow` |
| --- | --- | --- | --- | --- | --- |
| JM08 3200/3000 | exit 1, gameplay identical | 82 / 0 / 0 | identical (9) | identical (52) | 0 |
| USN01 3200/3000 | exit 1, gameplay identical | 48 / 0 / 0 | identical (5) | identical (28) | 2 |

**Verdict.**
- Every prediction held. On both rows each wedge call stops at the ship gate, because no group that
  calls it has a ship leader.
- Neither row moves, so there is no reference-j flag. `kCautiousWedgeBound` stays ON.
- The exit 1 rather than 0 is the wedge summary line (`bound=`, `calls=`). The host-method totals are
  equal (JM08 973 / 486, USN01 1010 / 516), because the wedge's `done` entries fire only past the
  gates. No gameplay line differs.
- **Attribution.** Across USN04, USN12, JM08 and USN01, the wedge has not moved a measured row. Only
  USN12 runs it past the gates, and there its stations have no motion consumer (sections 14-16).

## 18. Handoff (cc9-ships6, at the end of four packets)

**State on main** (`4215e40de` plus this branch's docs):
- **`kCautiousWedgeBound` ON.** `ai_formation_wedge_00a11690` in `src/ai_command_tick.cpp` runs after
  the follower pass of CAUTIOUSATTACK, CAUTIOUSMOVE and DEFENDPOSITION. Its host methods are in
  `src/game_hosts_ai.cpp`, and it writes through
  `GameUnitsHost::set_formation_member_offset_0070d080`.
- **Pairs.** USN12, USN04, JM08 and USN01 are all gameplay-identical (sections 14 and 17).
- **Ghidra names applied (provisional):** `00A11690` BSP_AiCommand_FormationWedge, `00A113D0`
  BSP_AiCommand_WedgeThreatDirection, `00779820` BSP_Entity_SharesUnitGroup.

**Settled facts, not to re-derive:**
- The threat ring has 13 samples.
- A zero danger cost gives a zero frame, and every station lands on the leader's point.
- The shape-0 table holds the registered command objects, so its offsets are about 1e-38 m.
- The replication tail (78h) has nothing to deliver in single-player.
- USN12's refused follow requests are the image's own answers (section 15).
- A follower placed with `Cruise` never takes `follow`, in the image as in the host (sections 15-16).
- RETREAT is unreachable in this installation (section 16).

**What would make the wedge observable.** A mission where a CAUTIOUSATTACK, CAUTIOUSMOVE or
DEFENDPOSITION group has a ship leader, and whose followers' directors go idle into `follow`. That
means followers not placed with `Cruise`, or a `Cruise` that ends. None of the measured rows has one.
A scan of the authored scenes for ship groups placed without `Cruise` under such a command would
find a row. That scan was not done.

**Open, in order of value:**
1. **The group extents.** `0070D400` / `0070D5D0` read each member's station `across`, which the
   wedge changes, and the path planner uses them (2816 calls on USN12). Their consumer publishes
   nothing on USN12 (`path_publishes=0`). Whether they ever feed a published path is untraced.
2. **JM08's `follow=0`.** Presumably the same authored-`Cruise` cause; not traced.
3. **The player's ships in Montpelier's AI group: answered in section 19.** The image does not do
   this. Its phase 3 seeds one group per entity (`00A2DFA0` at five sites, never `00A2D8E0`). The
   host's lump-per-collection reading was wrong.
   - The fix is bound OFF as `kAiGroupSeedPerEntityBound`, at `04df2a5c5` and `a1ad6a72b`.
   - Its pairs move USN12, USN04 and JM08 (exit 3, the mechanism matching).
   - The flip awaits a reference rebaseline. It changes every mission's AI grouping, so the wedge
     and cautious-route rows of sections 9, 14 and 17 would need re-measuring after it.
4. **The DEFENDPOSITION tick itself.** `00A15500` runs the wedge and then `00A13B60` with its own
   leader point. The host's DEFENDPOSITION arm calls the follower pass and the wedge, and the
   caller-side `00A13B60`, as before.

## 19. The player's ships in Montpelier's AI group (packet `cc9_player_ships_in_ai_group`)

Worker cc9-ships6, 2026-09-28. **The host diverges from the image.** The player's ships are put
there by the composition pass `00A2E720`'s phase 3, which the reconstruction misread.

**The host's join site.**
- `ai_groups_compose_00a2e720` in `src/ai_group_think.cpp`, phase 3, walks the host's one seed
  collection. That collection is every created unit (`first_seed_candidate`, collection 0, in
  `src/game_hosts_ai.cpp`).
- It creates one group from the first admitted candidate. It then adds **every** later admitted
  candidate to that group through `add_group_member` (`00A2D8E0`).
- The admission rule is `ai_group_seed_candidate`: the four flag bytes, ungrouped, and team < 2.
  It has no party or team equality test.
- So on USN12's first compose, Montpelier's twelve ships, the fortresses and the player's three
  destroyers (team 1) all land in one group. The next compose's eviction `00A2DDE0`
  (`ai_group_member_still_belongs`: party slot and team must equal the group's) removes the
  mismatched ones: USN12 `evicted=3`.

**The image's rule, read from the listing.** Phase 3 is `00A2E81A..00A2EA5A`: five loops over
`[[00E188A8]+19CCh]` lists at `+64h`, `+13Ch`, `+160h` and so on.
- Each loop admits a candidate on these tests, with EBP = 2:
  - `+5Ch` set;
  - `+5Dh`, `+60h` and `+5Eh` clear;
  - `+16Ch` (its group) null;
  - `+54h` below 2 (`00A2E838..00A2E85C`).
- For each admitted candidate it calls `new(5660h)` (`00A2E863`), then the constructor `00A2DFA0`
  with that entity (`00A2E881`).
- The five constructor calls are `00A2E881`, `00A2E8FC`, `00A2E967`, `00A2E9D8` and `00A2EA49`.
- A rel32 scan of `.text` for `CALL 00A2D8E0` finds no site inside `00A2E720`'s phase 3. Its sites
  are `00A16FD6`, `00A22ACE`, `00A22B80`, `00A2DDB2`, `00A2E181`, `00A2E447` (the split),
  `00A2E6A7` and `00A38CF6`.
- **So the image seeds one singleton group per entity.** Groups grow only through phase 4's
  auto-merge, which walks one party's groups at a time (`first_group_of_party`) and merges by
  leader distance within AutoMerge_MergeDist (650). A group never holds a different-party member in
  the image, and USN12's 36 party refusals at `00779D9F` cannot occur there.

**Bound OFF: `kAiGroupSeedPerEntityBound`** in `include/bsp/ai_group_think.hpp`. True makes phase 3
call `create_group` for each admitted candidate. The name differs from the brief's
`kAiGroupMemberFilterBound`, because the image's rule is not a filter on the join: it has no join
there at all.

**Blast radius.** This changes how every AI group forms. Groups become distance clusters inside one
party, instead of per-team lumps later split by groupability, so identity is not expected anywhere
AI groups exist.

**Predictions, written before any ON run.** The pairs use `pair_export --flip
kAiGroupSeedPerEntityBound=true` against the tree's own build (OFF), with reference i's launch lines.

| row | OFF census (earlier logs) | ON prediction |
| --- | --- | --- |
| USN12 3200/3000 | groups_created=3, auto_merges=0, evicted=3, follow requests 575 refused | `groups_created` rises to about the number of admitted units at the first compose (tens); `auto_merges` > 0; `evicted` 0 for party reasons. No request ever pairs a player ship with Montpelier, so the 36 party refusals vanish. Montpelier's group holds only ships whose group leaders came within 650 m while merging, so fewer than 12 members is likely. The CAUTIOUSATTACK assignment and the route may then go to a different or smaller group. pair_diff exit 3 |
| USN04 4700/4500 | groups_created=12, auto_merges=0, prox_merges=9, evicted=14 | `groups_created` rises and `auto_merges` > 0; the commands and targets move; exit 3 |
| JM08 3200/3000 | groups_created=5, auto_merges=0, evicted=41 | the same kind of move; exit 3 |

**Verdict rule.**
- The mechanism is checked by:
  - `auto_merges > 0` on every row;
  - no party eviction;
  - on USN12, no `ai diag follow` line with `party 1/0`.
- A mechanism failure keeps the switch OFF.
- If the mechanism matches, the gameplay moves are expected and do not by themselves block a flip.
  Because the change reaches every mission's grouping, the lead decides the flip after a reference
  rebaseline; this packet records the pairs.

**The pairs.**
- OFF is the tree's build of `04df2a5c5`, with logs `local\ships6_goff_<m>.log`.
- ON is `pair_export --commit a1ad6a72b --flip kAiGroupSeedPerEntityBound=true` into
  `local\ships6_gon` (bsp_game.exe SHA-256 prefix `F1E82347D226`), with logs
  `local\ships6_gon_<m>.log`.
- `a1ad6a72b` only restructures the switch so that the ON value compiles under `/WX`. It had left
  unreachable code. The OFF path is unchanged.

| row | pair_diff | grouping OFF -> ON | follow requests refused | moved gameplay lines |
| --- | --- | --- | --- | --- |
| USN12 3200/3000 | exit 3 | created 3 -> 19, merges (`prox_merges`) 0 -> 12, evicted 3 -> 0 | 575 -> 423, no `party 1/0` line | 4 unit rows; Montpelier moves 2247.37 -> 2240.06 m; deaths, hits and damage unchanged |
| USN04 4700/4500 | exit 3 | created 12 -> 39, merges 9 -> 28, evicted 14 -> 12 | 1274 -> 681 | deaths 43 -> 42, damage 11740.0 -> 13529.3, hit records 798 -> 760 (hull 115 -> 147), shots 9611 -> 10817, dive-bomb releases 4 -> 9 of 19, torpedo releases 7 -> 8 of 16; 66 unit rows |
| JM08 3200/3000 | exit 3 | created 5 -> 182, merges 0 -> 161, evicted 41 -> 7 | 917 -> 699 | deaths 9 -> 11, damage 3681.8 -> 4149.8, hit records 342 -> 361, shots 2091 -> 2324; 32 unit rows |

**USN12's groups ON.**
- The player's ships are in their own party-1 groups: Shigure with 2 members, Shiratsuyu with 1.
- Montpelier's CAUTIOUSATTACK group has 4 ships. The wedge then places 3 per run (`placed=147` over
  49 runs).
- The other team-0 ships form MOVETOATTACK groups: Columbia with 6, and Claxton and Foote alone.
- Fortress-07's DEFENDPOSITION group keeps 4.

**Verdict: the mechanism matched, and the switch stays OFF pending the lead's rebaseline decision.**
- No different-party member ever joins a group. USN12's party evictions and its 36 `00779D9F`
  refusals are gone.
- Groups now form only through phase 4 merges. The host counts those as `prox_merges`; its
  `auto_merges` counter stays 0 on both sides. The prediction's "`auto_merges` > 0" named the wrong
  counter; the merges themselves happened as predicted.
- Every row moves, as predicted. USN04 and JM08 move in their fights: more damage, more hull hits,
  and more ordnance releases.
- The switch is the image's arithmetic, but it reaches every mission's AI grouping. Per the rule
  above it is recorded rather than flipped here.
