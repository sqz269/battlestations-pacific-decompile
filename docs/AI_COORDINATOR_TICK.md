# Binding the AI coordinator's tick

Addresses: `00A32D50` (the coordinator's fixed-step advance, slot `+8h` of vtable `00D23168`,
which Ghidra has no function for), `00A32350` BSP_AiController_Create, `00A31730` (its
constructor), `00A2E720` (the composition pass), `00A182C0` -> `00A181A0` (the party think),
`009FFE50` BSP_Ai_IsPartyAiEnabled, `004BCA50` BSP_Game_GetEffectiveGameMode, `00A335D0` /
`00A371A0` (the tuning loader and its block), `00A2CBD0` (the attack order), `00A2C790` (the
per-member pass), `00A2C8D0` (the auto-merge predicate), `00A25B90` (the planner spawn),
`00A179E0` (the brain's engagement pass) and `0077D600` (the order path).

Packet `cc8_ai_coordinator_tick`. The group think and the planners were reconstructed by
`cc2_ai_group_think` and the planners packet; neither had a caller. This packet is the host
that calls them and the measurement of what they do.

## 1. What the coordinator needs at mission start

**Construction.** `00A32350 BSP_AiController_Create` has exactly two callers: `004E1838`
inside `004DFB70 BSP_Game_LoadMissionScene` and `00A373AF` inside `00A37310
BSP_LuaBinding_AICreate`. Both are mission load. Its constructor `00A31730` installs vtable
`00D23168` on the tick-element node at `coordinator+170h` (`00A3177F`) and registers that node
into fixed-step group 0 through `00875890` at `00A31766`. `GameAiCoordinatorHost::create_00a32350`
runs once after `create_units`, which is the equivalent point: phase 3 seeds from the world
entity collections, and there is nothing to seed before the units exist.

**The group list needs no separate filler.** `00F8AA60`/`00F8AA64` is element 2 of
`g_aiGroupsByTeam` at `00F8AA48`, stride `0Ch`, and `BSP_AiGroup_Construct` appends every group
to `g_aiGroupsByTeam[group+5638h]` unconditionally at `00A2E086`-`00A2E0BD`. The host makes the
same append inside `create_group`, so the proximity list the compose pass walks fills itself.
`docs/AI_GROUP_THINK.md`'s open question "what puts a group into the list at `00F8AA60`" was
already answered in `docs/AI_PLANNERS.md`; it is stale, not open.

**The party gate is the binding constraint.** `009FFE50` was read in full for this packet.
`009FFE59` tests `[[00E188A8]+61Ch]`. When it is set, `009FFE62` calls `004BCA50` and
`009FFE67` `CMP EAX,3` / `JA` returns true for every slot above game mode 3, while modes 0 to 3
fall through `009FFE6C`..`009FFE73` and return true **only for party slot 0 and slot 4**. The
other arm at `009FFE84` tests `[ecx+1FE4h]` and answers `slot != 0`.

All three measured missions carry sides 0, 1 and 2. Only side 0 gets a brain. The enemy sides
are never planned for. That is the shipped rule and not a gap in this host, and it is why the
census prints `ai_enabled` per party beside `record`.

**The tuning block has no loader here.** `00A371A0`'s block is filled by `00A335D0`, which has
two callers: `00A3717C` in `CG_array_ctor_helper_00a37130` and a tail jump at `00A371C7` in
`BSP_AiGlobals_Reload`. No AI globals script runs in this process, so every tuning field is
zero, `AutoMerge_MergeDist` at `+208h` included.

## 2. The host

`include/bsp/game_hosts_ai.hpp` and `src/game_hosts_ai.cpp`. `GameAiCoordinatorHost::Impl`
implements both `bsp::AiGroupThinkHost` and `bsp::AiPlannerHost`.

`fixed_step` calls `bsp::ai_coordinator_fixed_step_00a32d50`, which is `00A32D50`'s gate on
`00E0E34C` followed by the composition pass and the party pass. The float the wave passes is
discarded, exactly as the `RET 4` with no pushes says, and only advances this host's own copy
of the fixed-step clock `00F876A4`.

**Handles.** Every `void*` either sequence receives is a `Group*`, never a separate cursor
object, because the sequences mix list walks with handles they were given directly (a planner's
owned group, the emptied-list head) and a callee cannot tell the two apart. The walk position
lives on the group. This was found the hard way: the first build crashed with `0xC0000005` on
`IJN01` because `order_attack` received a raw group from the planner and a cursor from the
enemy-team walk in the same call.

### Substitutions, each labelled at its call site

| native | what this host does |
| --- | --- |
| `00A2E835`, the five world collections | one collection holding every created unit; 1 to 4 empty, so one group is seeded per team per pass rather than up to five |
| `00A2C8D0`, the auto-merge predicate | false. It delegates to `from+564Ch` `vtable[+14h]` and this process builds no AI command object to ask, so no auto-merge is ever taken |
| `00A371A0`, the tuning block | 0. No loader runs, so the proximity merge compares against a zero distance and takes nothing |
| `004BCA50`, the game mode | 0, the single-player campaign value. It decides both the party gate and `ai_party_think_mode`'s GroupWalk answer |
| the party record | a slot is enabled when a created unit carries that side. There is no party record block |
| `009FFD70`, the leader order key | the leader's unit index. The key only has to order a pair consistently |
| `00A2C450`, the world sets | the group's team against the brain's set index |
| `00A25B90`, the planner spawn | recorded. This process creates no unit at run time |
| `00A2C790`, the per-member dispatch | the members are counted and the call recorded. The `member->vtable[+114h]` dispatch is unread |

### The output path

`00A2CBD0` chooses `MOVETOATTACK` (`00A10890`) or `CAUTIOUSATTACK` (`00A109B0`) and installs
it on the group. This host takes that choice and records it, then issues the consequence to
each member as a scene command through `GameUnitsHost::issue_player_command`, which is the
`0046AAB0` registry resolve and the `0077D600` MT_COMMAND hop a scripted order and a player
order both take. That is what makes the plane task machinery and the ship order ring treat the
AI's decision as a native order. It is a labelled substitution for `00A2C790`'s unread
dispatch, not a reconstruction of it.

The token is the member's own kind, because the 26-row registry has no single "attack": a plane
takes `dogfight` against an air group and `attackmove` against a surface one, and a ship takes
`artillery`. Observed on `IJN01`: 75 `artillery` tokens resolved to `attackmove` through the
registry's first-match walk and 4 `dogfight` resolved to themselves. That resolve is reported
as observed and not explained here.

## Contract: the fixed-step hook is not in this commit

`src/game_hosts_units.cpp` is held by the torpedo worker. The five lines below were applied
locally to take the measurements and then left out of the commit. They go in on their own once
the file is free.

```
#include "bsp/game_hosts_ai.hpp"                       // beside game_hosts_gunnery.hpp
    std::unique_ptr<GameAiCoordinatorHost> ai;         // beside `gunnery`
    host.ai = std::make_unique<GameAiCoordinatorHost>(host.log, *this);
    host.ai->create_00a32350();                        // after host.gunnery->attach_00864bd0()
    if (host.ai != nullptr) host.ai->fixed_step(step_seconds);   // before the gunnery step
    if (host.ai != nullptr) host.ai->report();                   // after the gunnery report
```

`cmake/startup.cmake` gains one appended line for `src/game_hosts_ai.cpp`, and that line **is**
in this commit, so the file compiles into the build with no caller until the hook lands.

## no_ghidra_function

`00A32D50`. Twenty bytes between two functions, read from the listing:
`CMP byte [00E0E34C],0 / JE / CALL 00A2E720 / CALL 00A182C0 / RET 4`. Every other address this
packet cites has a Ghidra function.

## Validation

`./scripts/build.ps1` Win32 `/W4 /WX`: clean. `ctest --test-dir build/win32 -C Release`: 2/2
pass. No test added. Every before run was taken on this tree with the wiring stashed.

The coordinator behaves identically on all three missions: `compose=3000`, `thought=39`,
`planner_ticks=156`, `claims=1`, `spawn_arms=78`, `attack_orders=1`, `refused=0`, first command
at 4.85 s, two groups created, parties 1 and 2 with `record=1 ai_enabled=0 brain=0`.

### IJN01, the uncommanded-aircraft mission

| | before | after |
| --- | --- | --- |
| plane yaw_plans | 3000 | 5904 |
| plane pose_rotations | 3625 | 8504 |
| plane heading_change | 3.862 rad | 8.881 rad |
| plane distance moved | 1213845.72 m | 1270266.90 m |
| pilot attack ordered | 2 | 4 |
| gunnery shots / hull / deaths / damage | 34 / 4 / 2 / 500.0 | 35 / 4 / 2 / 500.0 |
| commands issued | 0 | 79 |

The aircraft no longer fly straight: the yaw planner nearly doubled its work and the fleet's
total heading change more than doubled. `free_flight` stays at 99000 because that counter is
`007CE040`'s flight-model arm selection, free flight against ground roll against surface, and
not a task test; it cannot move for a plane that is airborne. Gunnery is unchanged, which is
what one attack order in 150 s should do.

### USN01

| | before | after |
| --- | --- | --- |
| pilot attack ordered | 5 | 14 |
| gunnery hits / hull / deaths | 23 / 23 / 1 | 62 / 62 / 2 |
| gunnery damage | 220.0 | 3518.9 |
| gunnery shots | 8711 | 3476 |
| commands issued | 0 | 24 |

The largest outcome change of the three, from 24 member commands at 4.85 s. Shots fell by 60
per cent while hits rose by a factor of 2.7, so the aircraft are being aimed rather than firing
at nothing. `closed_mean` went from +3906.4 m to -123587.2 m: the newly ordered units open
range on their targets instead of closing. The order reaches them and the approach is not
solved, which belongs to the plane task machinery and not to the coordinator.

### USN02

| | before | after |
| --- | --- | --- |
| gunnery hits / hull | 126 / 125 | 168 / 168 |
| gunnery damage | 13673.7 | 19723.2 |
| gunnery shots | 872 | 562 |
| first hit | 41.85 s | 31.90 s |
| ship AI standoff choices | 8400 | 15953 |
| ship AI curve refreshes | 2450 | 4660 |
| commands issued | 0 | 14 |

Fourteen ship commands. Hits and damage rose on a third fewer shots and the first hit landed
ten seconds earlier, because the ordered ships close on their target instead of idling. The
standoff census nearly doubled: the ring arm now runs for ships that have somewhere to be.
Deaths are unchanged at 3.

## Corrections

Appended to `docs/AI_GROUP_THINK.md` and `docs/AI_PLANNERS.md`, never rewriting their text.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ai_coordinator_hook` | `00A32D50` | the five-line contract above, once `src/game_hosts_units.cpp` is free |
| `ai_command_object` | `00A10890`, `00A109B0`, `00A2C790`, `00A2C8D0` | the group's AI command and its `vtable[+114h]` per-member dispatch. Without it no merge predicate can be answered and no command is ever cleared, which is why one attack order lands per mission and never a second |
| `ai_globals_loader` | `00A335D0`, `00A371A0` | the tuning block's store sites and the Lua key names, so `AutoMerge_MergeDist` and the planner's range fields stop being zero |
| `ai_world_collections` | `00A2E835` | the five entity collections at `world+19CCh`, so phase 3 seeds more than one group per team |
| `plane_task_from_order` | `007CE040` | why an `attackmove` order reaches a plane and leaves `closed_mean` negative |

## Correction appended by cc8_ai_command_lifetime

**This doc's follow-up row `ai_command_object` states a wrong cause.** It says one attack order
lands per mission "because the already-on-target test returns early and nothing clears the
command". The second clause is wrong: `00A2CBD0` deletes the old command through its vtable slot
0 with flag 1 and installs the new one whenever the target differs, so a different target lands
a different order and nothing has to clear anything. `00A13340`, the command factory, was never
the blocker. `docs/AI_COMMAND_LIFETIME.md` has the two real caps.

**Two host methods in `src/game_hosts_ai.cpp` were wrong and are fixed there.**
`split_detached_members` was a no-op on the reasoning that nothing in this process detaches a
member; `00A2E260` does not split on detachment, it splits on `009FE080`, and it is the group
multiplier. `group_has_groupable_combatant` admitted the plane base `0Fh`, which `009FE080` does
not admit at all.

Re-measured on IJN01: groups created 2 to 4, splits taken 0 to 2, planner claims 1 to 2,
commands issued 79 to 34. Plane motion, the pilot attack table and the gunnery outcome are
identical to the digit, so the 45 orders that went away were inert. The census gains
`splits_taken`.

The validation tables above are from the run before this correction and are left as they were
written. USN01 and USN02 were not re-measured; the hook packet re-runs all three on the merged
tree anyway.
