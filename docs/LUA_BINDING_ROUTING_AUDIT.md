# Which unrouted mission natives can silently close a script branch

Packet `cc8_lua_binding_routing_audit`. It generalises the `GetDifficulty` defect: an unimplemented
or unrouted native in this host falls through to a bare `return 0` and pushes nothing, a Lua script
reads that as `nil`, and a `nil` in a comparison closes the branch without a message. `GetDifficulty`
turned USN04's entire Japanese carrier strike off at every difficulty that way.

**The headline is a reframing.** Ranking the gap by how often a native is called is the wrong
ranking, and it points at the wrong work. The right one is whether the script *uses the result*, and
by that measure the largest remaining hole in the campaign is `GenerateObject`.

## 1. Bodies against rows

45 `bsp::lua_binding_*` bodies exist across `src/lua_binding_{ai,core,mission,navigator,spawn}.cpp`.
The host has exactly one routing table, `kScriptOrderBindings` in `src/game_hosts_script_orders.cpp`
(`GameScriptOrdersHost::handles` is `find_binding(name) != nullptr`), with 22 rows after this
stream added `GetDifficulty`. There are no sibling tables; `src/mission_lua_host.cpp`'s table is the
full name-to-address list of the binding surface, not a routing table.

Bodies with no row:

| body | binding | address | why it is not simply routed |
| --- | --- | --- | --- |
| `lua_binding_setlog` | `SETLOG` | 0088C620 | routed by this packet; pushes nothing |
| `lua_binding_enable_messages` | `EnableMessages` | 008CFE40 | routed by this packet; pushes nothing |
| `lua_binding_load_message_map` | `LoadMessageMap` | 008C61C0 | routed by this packet; pushes nothing |
| `lua_binding_music_control_set_level` | `Music_Control_SetLevel` | 008C4D10 | routed by this packet; pushes nothing |
| `lua_binding_scoring_set_final_scoring_function_name` | `Scoring_SetFinalScoringFunctionName` | 008B8640 | routed by this packet; pushes nothing |
| `lua_binding_scoring_real_play_time_running` | `Scoring_RealPlayTimeRunning` | 008B87F0 | pushes a boolean, but no campaign script reads it |
| `lua_binding_prepare_class` | `PrepareClass` | 008C8F70 | **not routed**: its host reader `resolve_global_integer` is a stub returning false, so routing it would report work it does not do |
| `lua_binding_spawn` and three siblings | `GenerateObject` | 00944FD0 | **not routed**: needs a real `LuaBindingSpawnHost`. See section 4 |
| `lua_binding_ai_*`, 14 bodies | the `AI_*` family | 00A37310-00A38430 | **not routed**: no campaign mission in the three logs calls one |

## 2. Called while unimplemented, ranked by calls

Summed over four recent runs — this branch's `usn04_tick_long.log`, the ai-squadron tree's
`acc_ijn01_after.log` and `tw_usn01.log`, and the dive-bomb tree's `usn04_d4h.log`, all read-only.
42 natives were called while unimplemented. The head of the list:

| native | address | calls |
| --- | --- | --- |
| `FindEntity` | 00898E30 | 521 |
| `NavigatorMoveOnPath` | 008A3600 | 360 |
| `GetSelectedUnit` | 008AB070 | 180 |
| `SETLOG` | 0088C620 | 71 |
| `NavigatorSetAvoidLandCollision` | 008A3B10 | 50 |
| `NavigatorSetTorpedoEvasion` | 008A3CD0 | 50 |
| `MovCamNew_AddPosition` | 008B79F0 | 37 |
| `SetGuiName` | 008A8F90 | 36 |
| `SetNumbering` | 0088FE30 | 36 |
| `GetActDialogIDs` | 008CB730 | 27 |
| `PrepareClass` | 008C8F70 | 26 |

`GetDifficulty` is in the list at 4 calls, from logs taken before it was routed — which is the point
of section 3: it sat near the bottom of this ranking and was the most damaging entry in it.

## 3. What the scripts do with the result, which is the ranking that matters

Scanned across the 212 campaign script files (the modded `COTP-*` and `multi` trees excluded, as the
torpedo and dive-bomb surveys exclude them), counting call sites, assignments (`X = Native(`),
comparisons and immediate indexing:

| native | calls | assigned | compared | indexed |
| --- | --- | --- | --- | --- |
| **`GenerateObject`** | 1771 | **1639** | 0 | 0 |
| **`FindEntity`** | 4716 | **4172** | 2 | 3 |
| **`GetSelectedUnit`** | 135 | **130** | 0 | 4 |
| `GetDifficulty` | 79 | 76 | 3 | 0 |
| `IsListenerActive` | 54 | 0 | **49** | 0 |
| `GetClosestBorderZone` | 9 | 9 | 0 | 0 |
| `Kill` | 119 | 0 | 6 | 0 |
| `IsClassChanged` | 3 | 0 | 3 | 0 |
| `GetActDialogIDs` | 6 | 2 | 0 | 0 |
| `NavigatorSetTorpedoEvasion` | 365 | 0 | 0 | 0 |
| `NavigatorSetAvoidLandCollision` | 343 | 0 | 0 | 0 |
| `MovCamNew_AddPosition` | 142 | 0 | 0 | 0 |
| `NavigatorMoveOnPath` | 122 | 0 | 0 | 0 |
| `EnableMessages` | 125 | 0 | 0 | 0 |
| `SetAirBaseSlotCount` | 107 | 0 | 0 | 0 |
| `SetGuiName` | 73 | 0 | 0 | 0 |
| `Scoring_RealPlayTimeRunning` | 64 | 0 | 0 | 0 |
| `SETLOG` | 35 | 0 | 0 | 0 |

The two halves of that table are the whole finding. `NavigatorSetTorpedoEvasion` is called 365 times
and its result is never read once: unimplemented costs nothing. `IsListenerActive` is called 54
times and 49 of them are the condition of an `if`: unimplemented makes every one of those branches
dead, exactly as `GetDifficulty` did. The comparison counts are from a loose pattern and should be
read as "worth checking", but the assignment counts are exact.

## 4. `GenerateObject` is the `GetDifficulty` of the rest of the campaign

1639 of its 1771 call sites assign the result. A nil there is not a dead branch but a dead **unit**,
and everything the script later does with that variable is a call on nil.

It matters to this stream specifically. `docs/TORPEDO_MISSION_SURVEY.md` names the torpedo aircraft
of the two candidate missions other than USN04:

* USN22: `Mission.Avenger`, a `TBM_1`, from `GenerateObject` at `usn_ormoc.lua:1287`.
* USN01: `Mission.ScoutBomba = GenerateObject("ScoutDauntless")` at `usn_1_marshall.lua:711`.

So both are blocked by the same shape of defect USN04 was, one table down. The body exists —
`bsp::lua_binding_spawn` and its three siblings, with the arguments already decoded — and what is
missing is a `LuaBindingSpawnHost`: `create_scene_object`, `find_spawn_position`,
`place_spawn_directly`, `object_vcall_118`, `object_vcall_11c`, `scene_resolve_deferred_references`,
`object_entity_id`.

**That host is now most of the way built on this branch.** `create_air_ops_squadron_006c5050` already
creates a unit mid-mission from a class id and a position and hands back an entity id, and
`attach_created_entity_00928a00` already gives it the `thisTable` slot a script variable needs. A
`GenerateObject` host is the same two steps with the scene object's own arguments.

## 5. What this packet routed, and why it changes no behaviour

Five rows, each for a body that already exists and whose host methods are honestly answered:
`SETLOG`, `EnableMessages`, `LoadMessageMap`, `Music_Control_SetLevel` and
`Scoring_SetFinalScoringFunctionName`. **All five return 0 — they push nothing.** The native pushes
nothing either, so to a script `nil` and "no value" are the same thing and no branch changes. Their
value is that the host's own records stop saying UNIMPLEMENTED for work that is in fact
reconstructed, which is what made the `GetDifficulty` gap invisible for so long.

`PrepareClass` is deliberately **not** routed although its body exists: `resolve_global_integer` is a
stub returning false, so the body would run and do nothing while the record claimed otherwise. An
honest `UNIMPLEMENTED` is better than a green record over a stub. It is listed as a packet instead.

**So `GetDifficulty` was the last cheap win of its kind.** Every remaining entry that can close a
branch needs a host, not a row.

## 6. Packets, with addresses

| packet | binding | address | what it needs | why |
| --- | --- | --- | --- | --- |
| `lua_binding_generate_object` | `GenerateObject` | 00944FD0 | a `LuaBindingSpawnHost`, mostly present on this branch | 1639 assignments; blocks USN01 and USN22's torpedo aircraft |
| `lua_binding_get_selected_unit` | `GetSelectedUnit` | 008AB070 | the player's selected unit | 130 assignments |
| `lua_binding_listener_family` | `IsListenerActive`, `AddListener` | 008C6BB0, 008C6760 | the listener registry | 49 comparisons, all branch conditions |
| `lua_binding_prepare_class` | `PrepareClass` | 008C8F70 | `resolve_global_integer` over the class tables | body exists, host is a stub |
| `lua_binding_get_closest_border_zone` | `GetClosestBorderZone` | 008AECD0 | the zone table | 9 assignments |
| `lua_binding_get_act_dialog_ids` | `GetActDialogIDs` | 008CB730 | the dialog table | 2 assignments |

`FindEntity` 00898E30 is left off: it reports UNIMPLEMENTED but is **not** a pure nil — the
entity-returning tail `0089903C` is reconstructed and `push_resolved_entity` answers it for every
entity that has a `thisTable` slot. Its record is misleading rather than its behaviour; worth a
separate look at why the record says otherwise.

## Uncertainty

* The comparison column comes from a loose textual pattern and over-counts a call that merely sits
  inside an `if` block. The assignment column is exact.
* Only four logs, three missions. A native no campaign mission in those three calls may still matter
  elsewhere; the `AI_*` family is the obvious case.
* Whether any of the 42 pushes a value in the native that this audit assumed it does not was checked
  only for the six bodies that exist.

## Host methods

| method | address | coverage |
| --- | --- | --- |
| `bsp::lua_binding_setlog` | 0088C620 | routed; pushes nothing |
| `bsp::lua_binding_enable_messages` | 008CFE40 | routed; pushes nothing |
| `bsp::lua_binding_load_message_map` | 008C61C0 | routed; pushes nothing |
| `bsp::lua_binding_music_control_set_level` | 008C4D10 | routed; pushes nothing |
| `bsp::lua_binding_scoring_set_final_scoring_function_name` | 008B8640 | routed; pushes nothing |

## no_ghidra_function

None.

## Validation

Build and both ctest suites. No run of its own: the five rows cannot change a script branch, which
is the point of section 5.
