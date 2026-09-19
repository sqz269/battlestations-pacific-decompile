# Handoff: the natives USN04 still drops, ranked by script impact

Addresses: 00898e30 008a3600 008ab070 008a3b10 008a3cd0 008c6760 008c6bb0 008ac5c0 008a0a10

Written by `agent/cc8-spawn-new` on 2026-09-19 at the close of packet `cc8_spawn_new_route`, from
that packet's own USN04 runs. Nothing here is a conclusion about a binding's body; it is the
selection evidence, so the next owner does not have to re-measure it.

Base: `main` at `4bdba10d4` plus this branch. The counts come from
`local/spawn_final_usn04.log` (3200 frames, 3000 mission frames, `--menu-select USN04`).

## 1. The census line is not trustworthy as it stands, and FindEntity is the proof

`MissionLuaNative::FindEntity 00898e30 UNIMPLEMENTED calls=132` and `entity_resolves=132` are in
the SAME report. Every one of those 132 calls is answered with a real entity table.

`binding_trampoline` in `src/game_hosts_lua.cpp` decides `handled` before the entity-returning arm
at the bottom runs, and `note_native_call` records the status from that flag. So a row that answers
through `push_resolved_entity` was counted as unimplemented. `FindEntity` is the only one of the
nineteen `kEntityReturningBindings` rows that resolves - `push_resolved_entity` refuses every other
name by construction (`src/game_hosts_lua.cpp`, `if (std::strcmp(binding_name, "FindEntity") != 0)
return false;`) - which is why `entity_resolves` matches its call count exactly.

Fixed on this branch by `note_entity_resolved`, recording the status from the outcome. **Confirm it
before trusting the new census**: the check is that `FindEntity` reads `concrete calls=132` and that
no other row changed status.

**Consequence for the ranking: `FindEntity` is NOT a gap and must not be taken as one.** It also
means the 399-call figure over 24 natives is really 267 over 23.

## 2. The ranking, corrected from the script

Rank is by what the mission's control flow loses, not by call count.

### 1. `NavigatorMoveOnPath` `008A3600` (98 calls) with `NavigatorSetAvoidLandCollision` `008A3B10` (18) and `NavigatorSetTorpedoEvasion` `008A3CD0` (18)

The eight script sites are all of this shape, at `usn_19_coralus.lua:515`, `520`, `1368` and five
more:

```lua
NavigatorMoveOnPath(Mission.Lex, FindEntity("CarrierPath1"), PATH_FM_CIRCLE)
NavigatorMoveOnPath(Mission.Town, FindEntity("CarrierPath4"), PATH_FM_CIRCLE)
```

So the mission's capital ships - the Lexington the player defends, and the Town - are ordered to
circle authored patrol paths, and the order is a no-op today. The path argument is **already a real
entity**, because `FindEntity` resolves (section 1), so this is not blocked on anything.

Measured, and offered as a symptom rather than a proof: `summary mission world ...
controlled=Lexington-class01 moved=100.51` over `simulated=150.00 s`, i.e. 0.67 m/s, in both the
before and after columns of `cc8_spawn_new_route`. A carrier told to circle a path is not doing it.
Whether the 100.51 m is the patrol not running, or the ship simply starting from rest, is the first
thing to settle.

**Grep by concept before writing anything — and this one already found three pieces, so do not
build a fourth.** Searching for what the packet would DO rather than for `008A3600`:

* **the command type exists.** `src/attack_commands.cpp` carries `kCommandMoveOnPath` with a row in
  the command table (line 55) and arms in the tick at 409, 426, 455 and 506, and
  `src/command_completion.cpp:320` has its completion arm. Its own comment says the scope: "`moveonpath`:
  0099A1DC's factory only; the path machinery is a peer's."
* **the path build is a labelled contract, not missing by accident.**
  `src/command_execution.cpp:7`: "moveonpath path build in 0071F600 and the whole of 00836920 are
  contracts". Read that comment before deciding what the packet owes.
* **the scene already reads the authored path points.** `GameSceneEntityRecord::path_points_local`
  in `include/bsp/game_hosts_scene_contents.hpp:79-81` is a partial projection of `007B34F0`'s
  kind-1 property-bag branch, "Numeric `Point%002i` lookup order ... Pos triples remain LOCAL", and
  it explicitly notes no Path entity is constructed.

So both ends exist and the middle does not: the script names a Path entity, `FindEntity` resolves
it to a real entity table, the scene pass holds that entity's points, and the command layer knows
the command - but nothing binds `008A3600` and nothing turns the held points into the path
`0071F600` would build. That is the shape of the packet, and it is smaller than "reconstruct path
following".

`src/lua_binding_navigator.cpp` and
`include/bsp/lua_binding_navigator.hpp` already carry the family from packet `cc_lua_navigator`
(`docs/LUA_BINDING_NAVIGATOR.md`): `navigator_move_to`, `navigator_attack_move`,
`navigator_command`, and a `NavigatorHost` with `NavigatorIssueFlags`. That doc's addresses are
`008A30D0` `NavigatorAttackMove`, `008A2F20` `NavigatorMoveToRange`, `008A2BC0`, `008A2D70`,
`00899D10` `JoinFormation` and three more - **`008A3600` is not among them**, so `MoveOnPath` is the
sibling that was left out, and it should be added to that host rather than given a new one. Both
existing routines resolve argument 0 through `00888AA0`, read argument 1 into a `SceneCommandTarget`
through `0088A810`, and issue through `0077D600 BSP_Entity_IssueCommand` with a fixed command
constant; check whether `008A3600` follows that shape with a different constant, which is what
`NavigatorAttackMove` and `NavigatorMoveToRange` are to each other.

The two `Set...` companions are 18 calls each and are per-ship modifiers on the same navigator
state, so they belong in the same packet and the same run.

### 2. `GetSelectedUnit` `008AB070` (49 calls)

Three script sites, all `Mission.SelUnit = GetSelectedUnit()` (`999`, `2060`, `2196`), and
`Mission.SelUnit` is read back by `SetSelectedUnit(Mission.SelUnit)` at `1031` inside
`luaEndZuikakuDeadMovie`. It is an entity-returning row, so its nil arm is the recovered one; it
needs the player's selection, which is game state this process does own in part (the run reports
`controlled=Lexington-class01`). Moderate impact, small surface.

### 3. `AddListener` `008C6760` (2) and `IsListenerActive` `008C6BB0` (2)

High value per call - the mission's hit and death reactions hang off listeners, and packet
`cc8_spawn_new_route` showed the `LexHitListener` is what the Lexington-strike branch depends on -
but **hard to validate in a 150-second run**. The Lex listener is registered by
`luaLexKillersSpawned`, which is reached only from `luaEndZuikakuDeadMovie`
(`usn_19_coralus.lua:1036`), the tail of the Zuikaku-sinking cinematic. `AddListener` is `calls=2`
in every column measured so far, and those two are other listeners. Either find which two, and use
them, or pick a mission that registers listeners at stage init. Do not take this one expecting the
Lex listener to appear in a 3000-frame USN04 run; it will not.

### 4. `Kill` `008AC5C0` (3), then everything else

`PrepareClass` `008C8F70` (10) is worth one look because a class the script prepares may be one it
later spawns. `EntityTurnToEntity` `008A0A10` (9) is now called by the spawn callbacks this packet
bound, so it has a real subject for the first time.

**Do not route** `SetGuiName` (18), `SetNumbering` (18), `MovCamNew_AddPosition` (6), `BlackBars`
(4), `SetAirBaseSlotCount` (2), `EnableInput` (2) or the eight singletons beyond what a control-flow
branch needs. None of them returns a value the script tests.

## 3. Method that worked, and is worth repeating

The measure is the script-visible consequence, and it is found by diffing the WHOLE native-call
table between the before and after runs, **in both directions**. In `cc8_spawn_new_route` the most
important rows went DOWN: `HideUnitHP` 41 to 0, `Blackout` 47 to 6, `Objectives_Completed` 1 to 0,
because an empty `Mission.IJNBombersLex` had been satisfying the mission's primary objective by
itself. Reading only the additions would have missed it and reading only the `UNIMPLEMENTED` row
would have called it "132 calls returned nil".

For each native taken: read the binding whole from the listing, grep the headers and sources for
what you are about to build BY CONCEPT, bind through the existing host, and group several natives
into one USN04 run.

## 4. State of `cc8_spawn_new_route` at handoff

`docs/LUA_SPAWN_NEW_HOST.md` is the packet's document and `reports/cc8_spawn_new_route.json` its
report. The route is bound and validated; the open items are listed in that document's section 9,
of which the one a future packet most wants is how the `callback` NativeString at `record+84h`
reaches the interpreter - the implemented call is a contract read off the script, not a recovered
dispatch.

Lease `cc8_spawn_new_route` covers `include/bsp/lua_spawn_new.hpp`, `src/lua_spawn_new.cpp`,
`include/bsp/game_hosts_lua.hpp`, `src/game_hosts_lua.cpp`, `src/game_hosts_world.cpp`,
`include/bsp/lua_binding_spawn.hpp`, `docs/LUA_SPAWN_NEW_HOST.md`, `docs/LUA_BINDING_SPAWN.md`,
`cmake/startup.cmake` and `reports/cc8_spawn_new_route.json` until 18:52Z. A navigator packet needs
`src/lua_binding_navigator.cpp` and its header, which are not on it.
