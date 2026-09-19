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

### The trap in fixing it, which cost one wasted run

`GameHostLog`'s record is **sticky on first insert**. `GameHostLog::record` in `src/game_hosts.cpp`
walks `records_` for the method name and, on a hit, returns the existing entry with only `++calls`;
it sets `implemented` solely in the `records_.push_back` that creates one. So whichever of
`implemented()` / `unimplemented()` runs first decides the row for the whole run, and a later call
of the other kind changes nothing but the count.

The first version of this fix called `log_.implemented(...)` from the entity arm and left
`note_native_call`'s `log_.unimplemented(...)` in place. `note_native_call` runs first, so the row
stayed `UNIMPLEMENTED` and merely counted every call twice. That is not a deduction: a run on that
build (`local/census_usn04.log`) printed
`MissionLuaNative::FindEntity 00898e30 UNIMPLEMENTED calls=264`, the doubled count, beside an
unchanged `entity_resolves=132`. **`src/game_hosts.cpp` is orch4's**, so
`GameHostLog` cannot grow an "upgrade this record" method; the fix has to be to not record a status
before the answer is known.

What is on the branch now: `note_native_call` skips the `unimplemented` line for a row where
`bsp::mission_binding_returns_entity(binding.name)` is true, and the arm at the bottom of
`binding_trampoline` calls `note_entity_status(binding, resolved)`, which records exactly one of the
two. The `native <name> argc=` line and the summary row are unaffected.

### NOT CONFIRMED BY A RUN

The corrected version on this branch (`b5a31c82f`) **has not been confirmed by a run**. It compiles
and both ctest targets pass, and the reasoning is the measured `calls=264` above, but no run has
read back the corrected census line. The packet ended at its context limit rather than spend it on
another 40-minute run.

**These are the three checks, and they are the first measure the successor should take** — the next
USN04 run makes them for free:

1. `MissionLuaNative::FindEntity 00898e30` reads **`concrete calls=132`**. `264` means the fix did
   not take and the sticky record is still deciding the row; `UNIMPLEMENTED calls=132` means the
   status is being recorded somewhere this packet did not find.
2. The other eighteen `kEntityReturningBindings` rows still read `UNIMPLEMENTED` with the same
   counts as `local/spawn_final_usn04.log` — in particular `GetSelectedUnit 008ab070 calls=49`.
   Any of them flipping to `concrete` means `push_resolved_entity` is answering a row it should
   refuse.
3. `entity_resolves` is still `132`, i.e. the change touched the reporting and not the resolution.

If check 1 fails, the change is reporting-only and can be reverted on its own; nothing else in the
packet depends on it.

**Consequence for the ranking: `FindEntity` is NOT a gap and must not be taken as one.** It also
means the 399-call figure over 24 natives is really 267 over 23.

### Correction from packet `cc8_navigator_path`: every earlier log's entity rows read `2n-1`

The fix above took only half. `b5a31c82f` guarded the first-insert branch of `note_native_call`
and left the repeat branch calling `log_.unimplemented` unconditionally, on top of the
`note_entity_status` the bottom of `binding_trampoline` already makes. So an entity-returning row
was recorded twice for every call after the first: `1 + 2(n-1) = 2n-1`.

**In every log written before `d47d7bead`, every one of the nineteen `kEntityReturningBindings`
rows reads `2n-1`, not `n`. The real count is `(printed + 1) / 2`.** Measured on two independent
runs of the unfixed build (`local/census2_usn04.log` and `local/nav_before_usn04.log`): `FindEntity`
printed `concrete calls=263` for 132 calls and `GetSelectedUnit` printed `UNIMPLEMENTED calls=97`
for 49. It is not only the row that resolves — the status was right on both, only the count was
wrong, and it was wrong for all nineteen.

`d47d7bead` puts the same guard on the repeat branch. Confirmed by
`local/nav_base2_usn04.log`: `FindEntity 00898e30 concrete calls=132`,
`GetSelectedUnit 008ab070 UNIMPLEMENTED calls=49`, `entity_resolves=132`, and a world summary
identical to the unfixed run's (`moved=100.51 total_path=34734.24`), which is the control that
shows the change is reporting-only.

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

## 5. Other self-satisfying branches in the same script

The objective finding was not a one-off shape. Grepping `usn_19_coralus.lua` for the same pattern -
a gate that an EMPTY collection satisfies - gives five sites on four collections:

| line | gate | collection filled by |
| --- | --- | --- |
| 570 | difficulty 0 primary 1 | `Mission.IJNBombersLex`, from `luaBombersSpawnedLex` - **fixed by this packet** |
| 576, 582 | difficulty 1 and 2 primary 1 | `Mission.IJNBombersLex` **and `Mission.IJNFightersLex`** |
| 618 | `Mission.IJNShohoEscorts` | `table.insert` at 2096, 2116-2117 from `Mission.ShohoEscN` |
| 796 | `Mission.IJNTransports` | `table.insert` at 1808-1811, 1861-1862 from `Mission.TransN` |

Two things follow, and neither is settled here:

* **`Mission.IJNFightersLex` is still empty on difficulty 1 and 2.** It is filled only by
  `luaBombersSpawnedLex`'s difficulty-1/2 arm, from `unit2` - the SECOND group member. The runs
  behind this packet are difficulty 0, where every `SpawnNew` request carries one member, so that
  arm never ran and `Mission.IJNFightersLex` is still `{}`. On difficulty 1 or 2 the gates at 576
  and 582 would still complete the primary objective by themselves. Whether the two-member requests
  spawn correctly is UNTESTED: run USN04 at a higher difficulty to find out.
* **618 and 796 are filled late** - after the mission has advanced into the later phases - so
  whether their gate can be evaluated while the collection is still empty depends on the phase
  order. Check per phase; do not assume it is broken and do not assume it is safe.

## 6. Traps this packet hit, each of which cost something

1. **An immediate filter that could never match.** Ghidra prints an address immediate with **no
   leading zeros** (`PUSH 0xd0e1f8`), so `PUSH 0x00......` matches nothing anywhere in the image and
   proves nothing. Filter `PUSH 0x[0-9a-f]{5,8}` and classify the hits by segment; expect the MSVC
   prologue's SEH handler to be the one code address in an ordinary function. Also check the listing
   reached the body end: `ghidra disasm <fn> --lines N` stops silently at N.
2. **Decompiler-derived addresses.** Four addresses cited inside `009483D0` came from the
   pseudocode and were wrong by tens of bytes. Take every address from the listing. The decompiler
   also renders `CMP EAX,1 / JBE` as `< 2`, so a threshold read from pseudocode is off by one in
   its citation.
3. **A `done`-shaped column that is not what its name says.** `MissionLuaNative::FindEntity
   UNIMPLEMENTED calls=132` sat in the same report as `entity_resolves=132`. Section 1. Never pick
   work from one column without reconciling it against the rest of its own table.
4. **A sticky log record.** Section 1's trap: whichever of `implemented()` / `unimplemented()` runs
   first decides a row for the whole run.
5. **The launcher.** Never pipe `tools/run_game.ps1` through a short-circuiting filter
   (`Select-Object -First N`) - it closes the pipe, kills the run, exits 0 and writes no log. Use
   `-Last N` or a redirect. A mission-length run outlasts the foreground cap; note the expected end
   and do read-only work meanwhile.
6. **Your own in-flight run holds `bsp_game.exe`.** Linking during a run fails with
   `LNK1104: cannot open file ... bsp_game.exe`. The compile has already succeeded at that point;
   wait for the run, then link. Do not interpret it as a build break.
7. **Three id spaces in one log.** Entity ids, unit indices and vehicle-class ids all appear as
   small integers. `Mission.TypeD4Y = 159` is a vehicle class, `entity 22` is an entity id, and
   `units=57` is a count. An exact numerical coincidence across two of them means nothing.

Lease `cc8_spawn_new_route` covers `include/bsp/lua_spawn_new.hpp`, `src/lua_spawn_new.cpp`,
`include/bsp/game_hosts_lua.hpp`, `src/game_hosts_lua.cpp`, `src/game_hosts_world.cpp`,
`include/bsp/lua_binding_spawn.hpp`, `docs/LUA_SPAWN_NEW_HOST.md`, `docs/LUA_BINDING_SPAWN.md`,
`cmake/startup.cmake` and `reports/cc8_spawn_new_route.json` until 18:52Z. A navigator packet needs
`src/lua_binding_navigator.cpp` and its header, which are not on it.
