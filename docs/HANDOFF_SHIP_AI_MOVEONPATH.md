# Handoff: the `moveonpath` AI state step `009E59C0`, the last hop before a ship follows a path

Addresses: `009E59C0`, `009E5770` (the projected sibling), `007ADC30`, `007ADC60`, `009DE050`,
`009E00A0`, `0071E430`, `00521EA0`, `00414C60`, `0071BFF0`, `0071F600`

Written by `agent/cc8-navigator-path` at the close of packet `cc8_navigator_path`. The chain above
this step is now bound and measured; this step is what remains. Nothing here is a reading of
`009E59C0`'s body — it is the scoping, so the next owner does not have to redo it.

## The gate this handoff does NOT trigger

Packet `cc8_navigator_path` took the before/after and **the ships did not move less**. Fleet
`total_path` went **34734.24 -> 35103.17 (+368.93 m)** and the controlled Lexington's `moved=100.51`
was unchanged to the centimetre. So the native binding lands on its own and this document is a
scoping handoff, not a hold.

## What is already bound, and exactly where it stops

`docs/LUA_BINDING_MOVE_ON_PATH.md` has the chain whole. In short, and all of it measured on
`local/nav_after_usn04.log`:

`NavigatorMoveOnPath(unit, path, PATH_FM_CIRCLE)` -> `5Bh` session message -> `00721A40`'s `5Bh`
arm -> `director->vtable[60h](00E08F80 moveonpath, descriptor, 0.0f)` -> the ship AI selects the
`moveonpath` state.

```
MissionLuaNative::NavigatorMoveOnPath   008a3600  concrete       calls=98
ShipAiState::moveonpath_step            009e59c0  UNIMPLEMENTED  calls=179
Navigator::path_object_set_follow_mode  0071c1b0  UNIMPLEMENTED  calls=98
```

**179 calls a run reach `009E59C0` and find no body.** That is the single reason no ship follows a
path. The orders land on `Lexington-class01` and `Yorktown-class01`.

The one observable consequence today is not path following at all: `008A3600`'s commanded-speed
store defaults argument 4 to the class MaxSpeed, and the after run shows
`cruise state with an active commanded speed on "Yorktown-class01": 009e12bd divided +24h
16.719 m/s ... throttle 1.000000`. That is where the +368.93 m came from.

## Size: this is a normal-sized packet, not a sweep

| routine | body | bytes | state |
| --- | --- | --- | --- |
| `009E5770 BSP_ShipAi_MoveToPosStateStep` | `009E5770-009E59B7` | 583 | **already projected** |
| `009E59C0 BSP_ShipAi_MoveOnPathStateStep` | `009E59C0-009E5C90` | **720** | the subject |

720 bytes is 23% larger than a sibling this repository has already projected complete, so it
decompiles and does not need the huge-function listing sweep.

## The callees, by concept — almost all already named

This is the useful part. `009E59C0`'s callee list answers most of the questions the packet brief
would ask, before the body is read:

| concept the brief asks about | callee | what its name already gives |
| --- | --- | --- |
| **path-point advance** | `007ADC60 BSP_EntityCommand_IsOnFinalLeg` | the path is modelled as **legs**, and there is a final-leg test — this is the waypoint cursor |
| **empty path** | `007ADC30 BSP_EntityCommand_SlotHasNoLegs` | the slot-level "no legs" guard |
| **arrival radius** | `00414C60 BSP_Vector2f_LengthWithCutoff` | a 2D distance with a cutoff, i.e. the arrival test, and horizontal-only like `command_arrival_reached` |
| **the steering output** | `009DE050 BSP_ShipAi_SetNavigationGoal` | the goal hand-off; `movetopos` uses the same family |
| **the stop arm** | `009E00A0 BSP_ShipAi_HoldHeadingAndStop` | what it does when there is nowhere to go |
| **completion** | `0071E430 BSP_WeaponDirector_EndCommand` | pops the slot when the path finishes |
| target resolve | `00521EA0 BSP_CommandTarget_ResolveObject` | the descriptor's object |
| target pose | `00427EB0`, `00414DB0` | refreshed world position |
| mission events | `00984300 BSP_MissionEvents_ReportCommand` | the script-visible report |
| **unnamed** | `0071BFF0` | the only unnamed callee; read it first |

`0041E870` / `00419CC0` / `00BD1510` are the dead error-prefix residue of
`docs/MISSION_LUA_MACHINE.md` and model nothing.

**So "legs" is the vocabulary, not "points".** The follow modes (`PATH_FM_CIRCLE` and its siblings)
and the argument-3 integer are still unread; `0071C1B0` stores them at `object+8h` and `object+0Ch`
on the slot's path object, so whichever of those two fields `009E59C0` reads is what
`PATH_FM_CIRCLE` selects. That is the one question the callee list does not answer and the body
should.

## The two things the state step needs that do not exist yet

1. **The path object itself.** `0071C1B0` writes onto `[director+1A0h + i*4]`, and
   `src/game_hosts_commands.cpp`'s `command_count` already records that this process "does not
   build" one. `0071F600 BSP_WeaponDirector_BeginCommandBase` is where the executable builds it,
   and `src/command_execution.cpp`'s header comment has labelled it a contract since packet
   `cc2_director_commands`.
2. **The authored points.** `GameSceneEntityRecord::path_points_local`
   (`include/bsp/game_hosts_scene_contents.hpp`) already holds them from `007B34F0`'s kind-1
   property-bag branch, in LOCAL space, and explicitly notes that no Path entity is constructed.
   `command_execution.cpp`'s `path_point_counts` is the array waiting for the count.

So the order is: `0071F600`'s path build (which turns `path_points_local` into legs on a path
object), then `009E59C0` over it. A packet that does `009E59C0` alone will have nothing to iterate.

## The measurement to take

Same shape as this packet's, and the numbers to beat are on the same three logs in
`J:\PROG\battlestations-pacific-decompile-cc8-navigator-path\local\`:

- baseline `nav_base2_usn04.log`: `moved=100.51 total_path=34734.24`
- with the native bound `nav_after_usn04.log`: `moved=100.51 total_path=35103.17`

A working state step should move `Lexington-class01` and `Yorktown-class01` along their authored
`CarrierPath1` / `CarrierPath4` points in order, so the tests are: successive path points reached
in order, heading change, and `ShipAiState::moveonpath_step 009e59c0` leaving the UNIMPLEMENTED
census entirely. Diff the whole native table in BOTH directions, and take the ship's **pose**
heading, not `ship ai step heading`, which is a control-block field 0.7 to 0.9 rad off.
