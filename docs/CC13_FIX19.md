# CC13-FIX19: trigger target-gone observer and `AddSquadOrdnanceDroppedTrigger`

Packet cc13-fix19 adds two things to the Lua trigger machinery from packet cc13-fix10
(`docs/CC13_FIX10.md`):

- the target-gone observer `0074FFE0`, under the existing `kLuaProximityTriggersBound`;
- `AddSquadOrdnanceDroppedTrigger` (`008C9A10`), under a new switch,
  `kLuaSquadOrdnanceDroppedTriggerBound`.

`AddLineOfSightTrigger` (`008C9460`) is skipped. Both switches are committed **OFF**.
Status: reconstructed, build-tested; not game-validated. No game runs, no flips, no Ghidra
writes.

With both switches OFF, nothing changes. The `RemoveTrigger` and `AddSquadOrdnanceDroppedTrigger`
rows stay UNIMPLEMENTED, nothing is walked, and nothing is drained.

## 1. The target-gone observer `0074FFE0`

### What the image does

| step | address | what it does | evidence |
| --- | --- | --- | --- |
| register | `0074FEE0` | When descriptor `+44h` has kind != 0 (an entity target), `00521EA0` resolves the target and `00694A60` registers the pair (target, trigger`+30h`). The `+30h` sub-object's vtable is the adjustor table `00D00568`. | decompile |
| table | `00D00568` | `+0h` `0074FFD0` (dtor thunk), `+4h` `0074FFE0`, `+8h` `0042B120`, `+0Ch` `0042B130`, `+10h` `0042B140`, `+14h` `006935C0` | bytes `00D00568..00D0057F` |
| slot `+8h` | `0042B120` | `RET 4`, nothing else | disk bytes |
| slot `+4h` | `0074FFE0` | `if (notifier == 00521EA0(this+14h)) (this-30h)->vtable[10h]()`: the trigger's retire `0071C4A0` (`+10h = 1`, `+11h = 0`). No callback. | decompile |
| who calls `+4h` | `00696330` | Three callers: `00925C40` (called only by the unreferenced `009263C0`), `009273A0` (the same body inlined at `009274DE..00927510`, pass K, the fan-out's row 15) and `00A2D440` (AI group destruct, whose notifier is a group, not a unit). | `bsp.py callers 00696330` and `00925C40`; `docs/ENTITY_EVENT_QUEUES.md` |
| who calls `+8h` | `00696340` | From `00925C90`, the destroy hook `00926390` (pass D). For a trigger that is the no-op `0042B120`. | decompile of `00925C90` |
| the gate | `009274C8` | Pass K skips an entity whose `+5Eh` is already set. Otherwise it sets `+5Dh`/`+5Eh`/`+5Fh` (`+5Ch = 0`), notifies (`+4h`), then runs `vtable[80h]`. | `docs/ENTITY_EVENT_QUEUES.md` |
| endpoint delete | `00695760` -> `006953C0` | Deleting the target detaches its edges and frees them through each edge's `vtable[0](1)`. No owner callback runs. | `show 006953C0` (one virtual call, the edge delete) |

So a trigger is retired only when its target goes through the **kill list**, that is, a
`00926D90` Kill. Damage death reaches `00926C80` Destroy alone (`docs/ENTITY_EVENT_QUEUES.md`,
"Does a unit killed by damage run the on-killed hook?"), and its slot-`+8h` notice is a no-op.

That gives these cases:
- **Ship sunk by damage.** The trigger keeps testing the wreck until the wreck reaches
  KillDepth (`00825F20` -> `00826628` -> `00926D90(1)`).
- **Ship Killed from Lua.** `008AC5C0` retires the trigger at row 15 of the same step.
- **Squadron target.** Retired when `007F3970` removes the last plane and Kills the squadron.

**Timing.** Row 15 runs after waves 1 and 2. So in the step whose wave 1 queued the Kill, the
wave-2 walk still tests (and can fire) the trigger; the retire lands before the next walk.

### Switch decision

Under `kLuaProximityTriggersBound`, with no new switch. `0074FEE0` registers the observer for
every entity-target trigger, so the image has it whenever such a trigger exists. The switch is
still OFF on `origin/main` (checked after `git fetch`, before the commit), so its ON behaviour
could be extended.

### The host code

| piece | where |
| --- | --- |
| `target_gone_00925c40(unit)`: queues the notice, once per unit (stands for the `+5Eh` gate) | `src/game_hosts_lua.cpp` |
| `target_gone_drain_009273a0()`: the squadron poll, then `0074FFE0` on every proximity trigger with that entity target | `src/game_hosts_lua.cpp` |
| feed 1: `on_entity_killed_00928c80` (the host's inline pass K at KillDepth and at the kamikaze removal, `game_hosts_units.cpp`) queues step e's notice ahead of its own step f | `src/game_hosts_lua.cpp` |
| feed 2: `run_kill_008ac5c0`, each victim passed to `kill_unit_00926d90` | `src/game_hosts_lua.cpp` |
| the drain call, right after `run_subsystems_00875e0c` (the host's rows 1-16), before the motion step and the walk | `src/game_hosts_mission_frame.cpp` |
| summary line `trigger target gone notices=... retired=...` | `src/game_hosts_lua.cpp` |

`game_hosts_units.cpp` is not edited: both of its pass-K sites already call
`on_entity_killed_00928c80`.

### Substitutions and divergences (labelled in the code)

1. **Feeds.** The gunnery funnel `kill_unit_00926d90` serves both Kill and Destroy and cannot
   tell them apart, so the host has no general kill-list hook. These Kill callers are fed:
   - the host's inline pass-K sites (ship KillDepth, kamikaze removal);
   - Lua `Kill`;
   - the squadron poll.

   These Kill callers are **not** fed:
   - the plane depth kill (`007CE3A7`);
   - `SquadronLandAndKill`'s per-plane kills;
   - the ship AI's landed-remainder kill (`00749B7B`);
   - the other `00926D90` callers.

   A target killed only through one of these keeps its trigger.
   Lua Kill of a unit gunnery already counts dead feeds nothing, which matches the image's Kill:
   `00926DE1 CMP byte [EDI+5Fh],0` / `00926DED JNE 00926E5A` returns before anything else
   (cc13-fix19b check). In the image that unit's earlier Kill already notified its observers;
   here a gunnery-funnel death does not, which is the unfed-caller gap above.
2. **Squadron targets.** The drain polls every registry squadron (packet cc13-fix19c; before, only
   the squadrons a proximity trigger targeted). When the host
   registry shows a squadron that has held a member and now has none alive, the drain queues a
   notice. This stands for `007F3970`'s Kill of the squadron when its last plane goes
   (`007F3A16 CMP [ESI+3CCh],0` / `JNE`, `007F3A2A CALL 00926D90`). A member is alive when its
   registry entry is not `NoUnit` and gunnery does not count it dead. A plane that dies or lands
   is removed and Killed: `kSquadronRemovesDeadBound` and the stock return's `007F3970`
   (`game_hosts_units.cpp`) clear its entry to `NoUnit`, as the image's compaction of `+3D0h`
   does. Packet cc13-fix19b: "has held a member" is a sticky per-target flag
   (`squadron_target_resolved_`). Before it, the test needed a non-`NoUnit` member, so a
   compacted squadron never read as gone and its trigger never retired.
   **Timing substitution (cc13-fix19c).** Counting a gunnery-dead member as gone looks ahead of
   the compaction. For a plane that dies, the image lowers `+3CCh` at the plane's destroy flush
   (`007BCAA0 -> 007F3970`), so the host can notice the squadron's Kill up to one units step early.
   The other callers of `007F3970` Kill and remove in one go, so they leave no such window. They
   are a landed plane (`007F3CD9` calls `00926D90` with cause 5, then `007F3CE7` calls `007F3970`)
   and Lua `SquadronLandAndKill` (`008A223F` calls `00926D90`, then `008A2249` calls `007F3970`).
   **Every squadron, watched or not (cc13-fix19c).** The image Kills the squadron whether or not a
   trigger observes it, and pass K handles an entity once (`009274C8` tests `+5Eh`). So the notice
   is taken at the death and sets `target_gone_notified_` (the `+5Eh` stand-in). A trigger made
   afterwards, aimed at the dead squadron, is never retired, as in the image. Two edges this closes:
   (a) a trigger is removed, the squadron empties while nothing watches it, and a new trigger
   follows; the host used to retire the new one. (b) The squadron empties in the step its trigger
   is created; the host used to keep the trigger forever. In (b) the trigger now retires at the next
   drain when it was created before that drain. In the image it would retire only when it was
   registered before pass K ran on the squadron.
   **Fused slot (cc13-fix19c).** Lua Kill of a squadron Kills only its members (`008AC72F JE`,
   `008AC732 CALL 007ED380`, then the jump at `008AC737` that skips the `00926D90` at `008AC756`). So the
   squadron's notice comes only from this poll. `target_gone_00925c40` now drops a plane-level
   notice for the fused wing-0 slot. Before, a kamikaze or KillDepth removal of wing 0 (through
   `on_entity_killed_00928c80`) retired triggers aimed at a squadron whose other planes still flew.
3. **Timing.** The drain runs after the host's rows, which is the image's row-15 position
   relative to the walks. Kills from the motion step or the walk are drained after that step's
   walk. Kills from the rows are drained before the next walk. The drain runs after row 16
   (`00903610`), not before it; that row deletes and does not Kill.
4. **Once per unit.** The `+5Eh` gate is a per-unit flag in the Lua host. The image
   re-enables a unit's kill-list pass after `00925A00`'s cancel path; the host does not.
5. **Individual planes as targets.** Not fed, beyond Lua `Kill`. Since cc13-fix19c, the wing-0
   plane of a squadron is not fed even by Lua `Kill`, kamikaze or KillDepth. It shares the fused
   slot's id with its squadron, so `target_gone_00925c40` drops its notice. A trigger aimed at
   that plane retires only when the whole squadron goes (labelled RESIDUAL in the code).
   Re-checked in cc13-fix19d against the files the reachable rows actually run:
   - USN11 `COTP-USN/PRCPUS/usn_06_guadalcanal_2nd_battle.lua`
   - USN10 `COTP-USN/PRCPUS/usn_05_guadalcanal_1st_battle.lua`
   - PRCP05 `COTP-USN/PRCPUS/usn_09_Leyte.lua`
   - PRCP08 `COTP-USN/usn_12_Indochina.lua`
   - PRCP01 `COTP-USN/PRCPUS/usn_03_battle_of_santa_cruz.lua`
   - JM02 `COTP-IJN/PRCPIJN/prcpjm02.lua`

   No uncommented `AddProximityTrigger` there targets a single plane. The entity targets are:
   - ships: usn_06:2546 Enterprise, usn_09_Leyte:807 Atlanta
   - script markers: usn_03:1863 `FindEntity("NavEnterpriseStart")`, usn_12:1550 a B-17
     release point
   - squadrons: usn_03:961 `Mission.HornetKillers[1]`, prcpjm02:1123
     `luaPickRnd(Mission.Bombers)`

   usn_06:1964 watches a recon plane, but its target is a point.
6. **Kind.** Only proximity triggers registered the observer, so only they are retired.
   Squad-ordnance triggers have no target.

### Missions reached (this installation, `kLuaProximityTriggersBound` ON)

Mission IDs (packet cc13-fix19c re-map). A menu mission runs the script its scene's `StageScript`
(or `GameStageScript`) names, not the file its name resembles (docs/MISSION_LUA_MACHINE.md). Each
ID below is the `["id"]` of the `scripts/datatables/missiontree.lua` entry whose `sceneFile` names
that scene. A script that no missiontree scene names is marked **unreached**.

| script | line | watched -> target | effect of the observer |
| --- | --- | --- | --- |
| `COTP-USN/PRCPUS/usn_03_battle_of_santa_cruz.lua`: **PRCP01** (missiontree `COTP-USN/PRCPUS/usn_03_Battle_of_Santa_Cruz.scn`, StageScript `COTP-USN\\PRCPUS\\usn_03_battle_of_santa_cruz`). `COTP-USN/usn_03_battle_of_santa_cruz.lua`, the same line: **unreached** (no scene's StageScript names it) | 961 | Hornet -> `Mission.HornetKillers[1]` (a spawned squadron), 1500 m | If that squadron is wiped out before it comes within 1500 m, the trigger retires and `luaUS03HornetStrikeIncoming` never runs from it. Before this packet the host kept testing the dead squadron's slot pose. |
| `COTP-USN/PRCPUS/usn_06_guadalcanal_2nd_battle.lua`: **USN11** (the script USN11's StageScript runs). The same call is at the same line in `COTP-USN/PRCPUS/prcp_06_2ndbattle.lua`, which is **unreached** (no scene's StageScript names it) | 2546 | Hiei -> Enterprise (`GenerateObject`), 3900 m | Retired only when Enterprise is Lua-Killed or its wreck reaches KillDepth. While it sinks, the trigger still tests the wreck, as in the image. |
| `COTP-IJN/PRCPIJN/prcpjm02.lua`: **JM02** (missiontree `COTP-IJN/PRCPIJN/prcpijn_02_force_z.scn`, StageScript `COTP-IJN\\PRCPIJN\\prcpjm02`). `COTP-IJN/PRCPIJN/jm02.lua`, the same line: **unreached** (the `ijn_02_force_z` scenes name `challenge\\jm02`, `Ijn\\JM02` and `Ijn\\JM\\JM02`, and none of those scenes is in missiontree) | 1123 | F2A06 -> `luaPickRnd(Mission.Bombers)` (a G3M squadron), 1500 m | As usn_03. `luaPickRnd` is script-side `math.random`, not image RNG. |
| `COTP-USN/PRCPUS/usn_09_leyte.lua` (line 807): **PRCP05** (StageScript `COTP-USN\\PRCPUS\\usn_09_Leyte`). `COTP-USN/usn_09_leyte.lua` (line 738): **unreached** (its scene `COTP-USN/usn_09_leyte.scn` names `USN\\usn_09_Leyte` and is not in missiontree) | 738 / 807 | Airfield -> Atlanta (`FindEntity`), 2100 m | As prcp_06. |

There is no image RNG on this path. The only image calls are `00521EA0` and the retire.

## 2. Doc fix: `docs/CC13_FIX10.md`, JM13 `:3251`

For a nil target, if `0088A810` yields nil, the point arm runs `00888760`. That function
initialises nothing: its key loop finds no keys, and it copies its own uninitialised locals
to the output. The image's target is then uninitialised stack with position_valid 1. The
host's zero point (position_valid clear) is a stand-in for that indeterminate value. The
FIX10 doc now says so, and its items 4 and 5 point here.

## 3. `AddSquadOrdnanceDroppedTrigger` (`008C9A10`), `kLuaSquadOrdnanceDroppedTriggerBound`

### What the image does

| address | what it does | evidence |
| --- | --- | --- |
| `008C9A10` | `00750660` first. Then argument 1 through `008C9350` (nil leaves the name disengaged) and argument 2 through `00B662B0`, stored by `00750790`. Then argument 0 through `00888AA0`, `vtable[60h]()`, `0074FAB0`. Returns `00B66400` (0 results). No number argument. | decompile |
| `00750660` | `new 34h`. `+4h..+0Ch = 0`, `+10h = 0`, `+11h = 1`, `+14h = 1`, `+18h = 2`, name/callback fields cleared, vtable `00D005C0`, `+30h = 0`. | `src/cc13_seg41_land.cpp` (existing) |
| `00D005C0` | `+0h` `00750520`, `+4h` `007504C0`, `+8h` `0074FCD0` (the Lua-trigger test that `RemoveTrigger` asks), `+0Ch` `007504F0`, `+10h` `0071C4A0` | bytes `00D005C0..00D005D3` |
| `007504C0` (`vtable[4h]`) | `00876020(ECX = element, trigger)` (priority 2: appended), then `+30h = element+28h`, the entity | disasm `007504C0..007504D8` |
| `007504F0` (`vtable[0Ch]`, `RET 4`) | `+30h == 0`: `vtable[10h]`, a silent retire. Else `007ED790(ECX = +30h, 0)`. When that is false, `0074FDD0`: the callback `(entity, name or nil)`, then `vtable[10h]`. **One-shot.** The step argument is unread. | disasm `007504F0..0075051A` |
| `007ED790` | For `i < +3CCh`: `007B9140(ECX = [+3D0h+4i], 0)`. The first true answers true. | disasm `007ED790..007ED7CF` |
| `007B9140` | `vtable[5Ch](17h)` (MPlaneKamikaze) answers 1. Otherwise it walks the devices (`+974h`, count `+994h`) with `vtable[210h](2Ah, arg)`. | `show 007B9140` |
| `006E3FE0` (`vtable[210h]`) | Any attached round answering `vtable[5Ch](2Ah)`. The flag-1 arm is skipped for arg 0. | `show 006E3FE0` |
| `008C5FD0` / `0074FBB0` | `RemoveTrigger` walks the same list, asks `vtable[8h](1)` (`0074FCD0`, true), and compares the name. So it retires these triggers too. | decompile (no `+11h` test) |

There is no float arithmetic and no RNG (`00BD2F10` is not reached).

### The host code

| piece | where |
| --- | --- |
| switch | `include/bsp/game_hosts_lua.hpp` |
| `ProximityTrigger::Kind` (the record's vtable) | `include/bsp/game_hosts_lua.hpp` |
| row `0x008c9a10`; `RemoveTrigger`'s row is bound when either switch is on | `src/game_hosts_lua.cpp`, `binding_trampoline` |
| `run_add_squad_ordnance_dropped_trigger_008c9a10`, `check_squad_ordnance_trigger_007504f0` | `src/game_hosts_lua.cpp` |
| `fire_lua_trigger_0074fdd0` (split out of `check_proximity_trigger_00750000`, unchanged) | `src/game_hosts_lua.cpp` |
| the walk dispatches `vtable[0Ch]` by kind; the mission frame runs it when either switch is on | `src/game_hosts_lua.cpp`, `src/game_hosts_mission_frame.cpp` |
| summary line `squad ordnance triggers bound=1 ...` | `src/game_hosts_lua.cpp` |

### Substitutions (labelled in the code)

1. **Entity.** It must be a squadron of the host's registry, whose slot keys the list. Anything
   else is counted `unresolved` and not stored. The image would attach the trigger and then read
   `+3CCh` of a non-squadron. Every use in this installation passes a squadron.
2. **Members.** `+3D0h`/`+3CCh` are replaced by the registry members that gunnery does not
   count dead, the same proxy cc9's spent-ordnance latch uses for `007ED790`. A plane that lands
   is removed and Killed (the stock return's `007F3970`, `game_hosts_units.cpp`), so it is no
   longer a member, as in the image.
3. **No live member.** The trigger is not tested. This stands for the squadron's `+5Eh`: when
   `007F3970` removes the last plane it Kills the squadron, and that drains at row 15 before the
   next walk. So a destroyed squadron never fires the trigger. In the image it could fire in a
   walk that falls between the last plane's removal and row 15, if that order ever occurs.
4. **Per-plane test.** It is `unit_is_kind_of(m, 17h)` plus `plane_holds_rack_round_007b9140(m)`.
   The host answers that query for flag 1. The flag-1 arm needs `[00E17BF2]`, which is 0 in
   single player, so it gives the flag-0 answer.
5. **`+30h` null.** That arm (a silent retire) is not reachable here, because the squadron slot
   keys the list.
6. **Timing and admission.** As FIX10: the walk runs after the host's motion step,
   `unit_active()` stands for wave 2's admission, and the callback runs inline. Packet
   cc13-fix19b: wave 2 admits the squadron entity (`+5Eh` clear, `+BDh` set). Pass K sets `+5Eh`
   for any Killed entity (`009274D2 MOV byte [ESI+5Eh],1`). The squadron is Killed by `007F3970`
   when `+3CCh` reaches 0. Lua Kill of a squadron Kills only its members (item 2 above). Other
   direct Kills of a squadron object were not enumerated. The `+5Eh` store at `005FBC1B` is in
   `005FB080`, on an object whose class is not established. The list is keyed by the fused
   wing-0 slot, so the walk also admits that slot while `plane_squadron_live_fused_slot` finds the
   squadron holding a plane. Before this, a dead wing-0 leader stopped the squadron's triggers.
   Packet cc13-fix19c: a fused slot is refused once the drain has noticed the squadron's Kill
   (`target_gone_notified_`). That also covers the window of up to one step in which gunnery
   counts every member dead before compaction. The test needs `kLuaProximityTriggersBound`, which
   fills that vector. With only the ordnance switch on, `007504F0`'s own no-live-member return
   covers the case.
   A proximity trigger watching such a squadron is posed at its current leader
   (`flight_leader()`, `+3D0h[0]` after compaction), standing for the squadron's `+FCh`
   (docs/CONSTRUCT_WORLD.md, "the squadron's position +FCh | the leader's pose").
   `proximity_trigger_callback_missing` now counts both kinds.

### Missions reached (this installation, the switch ON)

Mission IDs are mapped through missiontree and the scene `StageScript`, as for the
target-gone table above (packet cc13-fix19c).

| script | line | squadron -> callback | prediction |
| --- | --- | --- | --- |
| `bsm/bsm_04_vengance_at_luzon.lua`: **BSM04** (StageScript `BSM\\bsm_04_vengance_at_luzon`) | 1150, 1174 | `Mission.CurrentAttacker` -> `luaAttackerRetreat`, `luaLastAttackerDropped` | When the player's attacker has released its last round, the dialogue and retreat logic runs. In phase 4 `Mission.LastSquadDropped = true`. OFF, neither runs. |
| `bsm/bsm_04_vengance_at_luzon.lua`: **BSM04** | 1440 | `Mission.FortressRed` -> `luaFortressDroppedBombs` | When the B-17s' racks are empty, `luaObj_Completed("primary", 2)` and the PAYLOAD dialogue. OFF, that primary objective never completes from here. |
| `usn/USNRM/usn_1_pearl.lua` (modded, mtime 2024-10-29): **USNRM01** (StageScript `USN\\USNRM\\usn_1_pearl`) | 1507 | `Mission.OklahomaKillers` -> `luaOklahomaKillersRetreat` | When the torpedo squadron has dropped, `UnitSetFireStance(2)` and `PilotMoveToRange` to JapRetreat. OFF, it stays on its attack. |
| `global/commandhelpers.lua` | 3577, 14797 | `unit` -> `luaAutomaticReloader` (the debug reloader cheat) | Reached only through that cheat. |

Each fire depends on the host's rack state. The cc9 release code empties the racks, and this
packet does not touch it: there is no notify hook, because the image polls in the walk rather
than reacting to a release event.

## 4. `AddLineOfSightTrigger` (`008C9460`): skipped

`00750230` works in four stages:
1. It takes the 3-D squared distance of the watched entity from the target entity and tests
   it against `+48h`.
2. Inside that range it counts `+4Ch` down by the step. When the count goes below 0, it adds
   `[00D7A280]` back.
3. It casts `0084B6B0`, which is `0098B370`'s sweep over the spatial index (every collidable
   scene object, islands and terrain included). For a squadron, the cast starts from its first
   member `+3D0h`.
4. It fires only when the first hit is the target.

The host has no model of `0098B370`. `GameGunneryHost::query_segment_units` is `0098ADD0`
over the host's units, and `landscape_segment_hit_00904400` answers over units too ("this
host has no islands or terrain"). Neither reproduces which object a sweep hits first. That
decides the result in `bsm_03` (the Palawan passage) and `bsm_05`, which are the missions that
use it. So no host query is faithful, and the trigger is not approximated.

`RemoveTrigger` still answers false for line-of-sight names, and `00750190`, its target-gone
observer, is not modelled. Uses in this installation:
- `bsm/bsm_03_running_the_palawan_passage.lua` `:868`, `:993`, `:1189-1191`;
- `bsm/bsm_05_raid_on_balikpapan.lua` `:951`;
- `global/commandhelpers.lua` `:17369`, `:17381`.

## Uncertainty

- Not game-validated, and no pair was run.
- The thread that runs wave-2 jobs is not established. Nothing here does float math, so that
  does not matter.
- Two orderings come from `docs/ENTITY_EVENT_QUEUES.md` and are not re-derived here: that
  `007F3970`'s squadron Kill comes from the plane's destroy hook, and the row-15 re-loop.
