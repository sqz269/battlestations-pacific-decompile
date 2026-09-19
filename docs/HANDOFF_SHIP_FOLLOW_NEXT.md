# Handoff: the ship follow chain, one guard short of moving escorts

Written at the context threshold of packet `cc8_ship_follow` (branch `agent/cc8-ship-follow`).
`docs/SHIP_UNIT_GROUP_FOLLOW.md` is the packet; this is only what is left and what the next
session must not re-derive.

## Where it stands

Bound and measured, in order: the availability gate (`00779D50`), membership (`0077F940`'s runtime
arm), column 0 (`0070ED30`) and the station (`0070D290`), the follow state step (`009E1610` against
`009DF2D0` and `run_navigation_goal_009de050`), the director's producer inputs (`007788B0` /
`007788D0`), and the **script** path (`JoinFormation` -> `entity_command_is_available` ->
`session_route_formation_message`). The terminator `00836ADC` is transcribed but not wired.

The last USN01 run, `local/follow_script_usn01.log`:

```
summary unit formation groups=3 joins=13 creates=3 rejoins=0 clamped=3 columns_unmeasurable=10
  formation 0 leader=Convoy1      count=6 column=0
  formation 1 leader=Northampton  count=3 column=0
  formation 2 leader=Enterprise   count=7 column=0
summary mission director steps=186000 idle_reissues=52 stop=49 cruise=1 follow=2
Dunlap        follow ...
SaltLakeCity  follow ...
total_path=5600.63      (unchanged)
```

So the mission script authors **three** formations, the director issues `follow`, and the ship AI
**selects the `follow` state** for `Dunlap` and `SaltLakeCity`. Escorts still do not move.

## The one open defect, and how to find it in ten minutes

**No `ShipAiFollow::*` record appears anywhere in that run**, so `009E1610` never executed even
though its state was selected. The arm is in `src/game_hosts_ship_ai.cpp`,
`state_step_vtable0c`, at the `state->step == 0x009e1610u` test, and it is guarded by

```cpp
const std::int32_t group = owner_.units.unit_formation_group_0284(index_);
const std::size_t leader = owner_.units.formation_leader_0014(group);
if (group >= 0 && leader != static_cast<std::size_t>(-1) && leader != index_) { ... }
return;
```

One of those three is rejecting. The guard duplicates `009E1610`'s own first three gates, which the
step already applies through `leader_matches_kind_5c`, so **the cheapest fix is to delete the guard
and let the step decide** - that is also the more faithful shape. If it must stay, log the three
values once per unit and read which one fails. Note that the step needs a leader index for its
bindings, so `formation_leader_0014` still has to answer; if it is the one returning `-1`, the
group index is not what the units host recorded, and `unit_formation_group_0284` is the suspect.

Do NOT assume the guard is the only thing: confirm by looking for `ShipAiFollow::leader_kind` in
the next run's call table. It is the step's first host call and appears the moment the body runs.

## What is already known and must not be re-derived

* **Three conditions gate `follow`**, all read from the listing and recorded in section 9 of the
  packet doc: membership; a director stage that can return to 2; and a leader that has already laid
  wake. The second is why USN01 produced `follow=0` before the script path was bound - the idle
  re-issue's `proceed` gate passed 53 times in 186000 steps, once per ship at `t=0`.
* **`[unit+73Ch]+28h` is a timestamp, not an enable** (`-1.0` when never set, `00D7A260`), and
  `00836AC8`'s `JC` keeps a `stop` running while it is below zero. A ship that never had a commanded
  speed stored never returns to stage 2. USN04 proved the converse: `commanded_speeds=2` there and
  `follow=1`.
* **USN01 authors `StartSpeed` on exactly four units** - `Enterprise`, `Ralph`, `McCall`, `Blue` -
  and they are exactly the four that move. `Northampton`, `SaltLakeCity` and `Dunlap` have none.
* **The script authors the formations**: `JoinFormation` is called 13 times at `luaStageInit`.
  Before this packet every one was refused by a stub that returned a neutral `false`.
* **`columns_unmeasurable=10` of 13**: the script joins land before any leader has laid wake, so
  those records hold zero columns. Zero columns mean the station is the leader's own track. The
  integrator's ruling (step 4) is to bind what `00811180` returns for a ring with fewer than two
  samples rather than reporting it unmeasurable; **that is still open**, and it decides whether the
  three scripted formations form line-astern on the leader's wake or hold an offset.
* **The across sign is not read** (`00811726`-`00811760`, four x87 operands). It does not block
  column 0, which is self-consistent, but the canned LINE/COLUMN/DIAMOND tables in columns 1-3
  cannot be trusted until it is. No type-78h reshape has been seen in either mission; check the
  call table before spending anything on it.
* **`create_units` rebuilt the AI coordinator on every spawn batch.** Fixed in this packet
  (constructed once). NOT yet measured on USN04 - the run to take is a USN04 pair showing the AI
  counters becoming mission totals (`joins=17` visible in the AI summary rather than
  `available=0 refused=238`), plus the two-way call-table diff.

## Runs that exist, and what they are for

All under `local/` in this worktree. `wake_usn01_fixed.log` / `wake_usn04.log` are the packet-1
wake measurements; `wake_usn01_noappend.log` + `wake_null_calldiff.txt` are its same-binary null.
`follow_gate_usn01.log` is the gate step, `follow_join_usn01.log` membership,
`follow_columns_usn01.log` the columns, `follow_runs_usn01.log` / `follow_runs_usn04.log` the
director evidence, and `follow_script_usn01.log` the script path. The integrator asked that the
USN04 pair and the USN01 torpedo trace be held until escorts actually move.
