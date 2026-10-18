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
2. **Squadron targets.** The drain polls each proximity trigger's entity target. When the host
   registry shows a squadron with at least one member and none alive (gunnery `unit_dead`), the
   drain queues a notice. This stands for `007F3970`'s Kill of the squadron when its last plane
   goes. A plane that landed counts as alive here, but in the image it has left `+3D0h`.
3. **Timing.** The drain runs after the host's rows, which is the image's row-15 position
   relative to the walks. Kills from the motion step or the walk are drained after that step's
   walk. Kills from the rows are drained before the next walk. The drain runs after row 16
   (`00903610`), not before it; that row deletes and does not Kill.
4. **Once per unit.** The `+5Eh` gate is a per-unit flag in the Lua host. The image
   re-enables a unit's kill-list pass after `00925A00`'s cancel path; the host does not.
5. **Individual planes as targets.** Not fed, beyond Lua `Kill`. No call in the four missions
   below targets a single plane.
6. **Kind.** Only proximity triggers registered the observer, so only they are retired.
   Squad-ordnance triggers have no target.

### Missions reached (this installation, `kLuaProximityTriggersBound` ON)

| script | line | watched -> target | effect of the observer |
| --- | --- | --- | --- |
| `COTP-USN/usn_03_battle_of_santa_cruz.lua` (and `PRCPUS/`) | 961 | Hornet -> `Mission.HornetKillers[1]` (a spawned squadron), 1500 m | If that squadron is wiped out before it comes within 1500 m, the trigger retires and `luaUS03HornetStrikeIncoming` never runs from it. Before this packet the host kept testing the dead squadron's slot pose. |
| `COTP-USN/PRCPUS/prcp_06_2ndbattle.lua` | 2546 | Hiei -> Enterprise (`GenerateObject`), 3900 m | Retired only when Enterprise is Lua-Killed or its wreck reaches KillDepth. While it sinks, the trigger still tests the wreck, as in the image. |
| `COTP-IJN/PRCPIJN/jm02.lua`, `prcpjm02.lua` | 1123 | F2A06 -> `luaPickRnd(Mission.Bombers)` (a G3M squadron), 1500 m | As usn_03. `luaPickRnd` is script-side `math.random`, not image RNG. |
| `COTP-USN/usn_09_leyte.lua` | 738 (807 in `PRCPUS/`) | Airfield -> Atlanta (`FindEntity`), 2100 m | As prcp_06. |

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
   count dead, the same proxy cc9's spent-ordnance latch uses for `007ED790`. A plane back in
   its hangar counts as a member.
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
   `unit_active()` stands for wave 2's admission, and the callback runs inline.
   `proximity_trigger_callback_missing` now counts both kinds.

### Missions reached (this installation, the switch ON)

| script | line | squadron -> callback | prediction |
| --- | --- | --- | --- |
| `bsm/bsm_04_vengance_at_luzon.lua` | 1150, 1174 | `Mission.CurrentAttacker` -> `luaAttackerRetreat`, `luaLastAttackerDropped` | When the player's attacker has released its last round, the dialogue and retreat logic runs. In phase 4 `Mission.LastSquadDropped = true`. OFF, neither runs. |
| `bsm/bsm_04_vengance_at_luzon.lua` | 1440 | `Mission.FortressRed` -> `luaFortressDroppedBombs` | When the B-17s' racks are empty, `luaObj_Completed("primary", 2)` and the PAYLOAD dialogue. OFF, that primary objective never completes from here. |
| `usn/USNRM/usn_1_pearl.lua` (modded, mtime 2024-10-29) | 1507 | `Mission.OklahomaKillers` -> `luaOklahomaKillersRetreat` | When the torpedo squadron has dropped, `UnitSetFireStance(2)` and `PilotMoveToRange` to JapRetreat. OFF, it stays on its attack. |
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
