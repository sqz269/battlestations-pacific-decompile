# CC13-FIX10: Lua proximity triggers (`AddProximityTrigger`, `RemoveTrigger`)

Switch: `kLuaProximityTriggersBound` in `include/bsp/game_hosts_lua.hpp`, committed **OFF**.
Grant: cc9 gameplay lead, 2026-10-17. Status: reconstructed, build-tested; not game-validated
(no runs in this packet).

With the switch OFF nothing changes: the trampoline rows stay unhandled, so both natives
are recorded UNIMPLEMENTED and push nothing, and the fixed-step walk is not called.

With it ON:

- `AddProximityTrigger(entity, name, callback, target, range)` (`008C9730`) stores a
  trigger on the watched unit's list.
- Each fixed step, after this host's motion step, every active unit's triggers are tested.
  A hit calls the Lua global `callback(entity, name)` and retires the trigger, so it fires
  once.
- `RemoveTrigger(entity, name)` (`008C5FD0`) retires every trigger on that unit with that
  name and returns whether any matched.

## What the image does

| address | what it does | evidence |
| --- | --- | --- |
| `008C9730` | Argument 3 goes through `0088A810` (the command-target descriptor: `ID` present makes an entity target, kind 1, position_valid 0; `ID` nil makes a Vector3 point, position_valid 1; a null entity leaves the zero point). Argument 4 is read with `00B66270`. Then `007505C0`, argument 1 through `008C9350` (nil leaves the name disengaged), argument 2 through `00B662B0`, then `00750790`. The list is `entity(0)->vtable[60h]()`, joined through `0074FAB0`. Returns 0. | decompile; `0088A810` listing `0088A84F..0088A8C7` |
| `007505C0` / `0074FEE0` | `new 60h`. The 18h-byte descriptor is copied to `+44h`, and `+5Ch = range * range` as a float. Also `+11h = 1` (enabled), `+18h = 2` (priority) and vtable `00D00580`. For an entity target, an observer (`00694A60`) is registered whose `0074FFE0` retires the trigger when that entity goes. | decompile |
| `00750790` | `+1Ch` = name, `+24h` = name engaged, `+28h` = callback. | decompile |
| `0074FAB0` -> `0071C470` -> `00876020` | The trigger becomes a sub-node of the entity's tick element (`+4h` = element). It is inserted after every sub-node whose priority `+18h` is at most 2. | decompile of `00876020` |
| `008759B0` (wave 2 of the fixed step, `00875B90`) | Walks the element's sub-list. It takes `next` (`+0Ch`) **before** running the node. A node with `+11h` set gets `vtable[0Ch](step)`. A node with `+10h` set is then unlinked and destroyed through `vtable[0](1)`. Wave 2 admits an element only when `payload+5Eh == 0 && payload+BDh != 0`. | decompile; docs/FIXED_STEP_JOB_WAVES.md |
| `00750000` | The watched entity is `(+4)->+28h`. When `+C8h` is clear, its pose is refreshed (`00414DB0`). Then `dx = wx - tx` and `dz = wz - tz` (FSTP float each), and `d2 = (dx*dx + 0*0) + dz*dz` (FSTP float). It fires `0074FDD0` when `+5Ch > d2` (`FCOMIP`/`JBE`, so a NaN never fires). The target is the descriptor `+44h` through `006F7DD0`: for an entity, its `+FCh/+100h/+104h`; for a point, the point; otherwise the zero point `00F87574`. Planar (x, z), squared, from the world translations. `RET 4`, and dt is not read. | listing `00750000..00750089` |
| `0074FDD0` | args = {`00884A90` entity variant of `(+4)->+28h` (the watched entity), the name (`00886A10`) or tag 6 (nil)}; `00887E50(host, 0, +28h, args, 0, -1)`; then `vtable[10h]`. | decompile; docs/MISSION_LUA_HOST.md (tag 6 pushes nil) |
| `0071C4A0` (vtable `+10h`) | `+10h = 1`, `+11h = 0`: the retire request. The same wave-2 walk releases the node. **One-shot.** | listing |
| `008C5FD0` -> `0074FBB0` | `RemoveTrigger`: walks the list. Every node whose `vtable[8h](1)` answers true (the Lua trigger base `0074FCD0`), whose name is engaged, and whose name is equal (`00435C40`: same length, then `_stricmp`) gets `vtable[10h]`. Every match is retired, not only the first. It pushes the boolean "any matched" and returns 1. | decompile |

The callback's arguments are `(watched entity, name or nil)`. The trigger fires inside the
fixed step, during wave 2 and after wave 1's element steps. If `00887E50` queues the call, it
is drained at the fan-out's row 7 (`00875E55`) of the same step. Either way the call comes
after the step's motion and before the next step.

## The host code

| piece | where |
| --- | --- |
| switch, trigger record, list, counters | `include/bsp/game_hosts_lua.hpp` |
| trampoline rows (`0x008c9730`, `0x008c5fd0`) | `src/game_hosts_lua.cpp`, `binding_trampoline` |
| `run_add_proximity_trigger_008c9730`, `run_remove_trigger_008c5fd0`, `run_proximity_triggers_008759b0`, `check_proximity_trigger_00750000` | `src/game_hosts_lua.cpp`, after the listener helpers |
| the per-step call, after `motion_step_00825f20` | `src/game_hosts_mission_frame.cpp`, `FixedStepBinding::run_step_subsystems` |
| summary line | `src/game_hosts_lua.cpp`, with the listener summaries |

Reused, not rewritten: `bsp::gameplay_seg41_cc13::trigger_radius_squared` (`0074FF4E`,
`src/gameplay_seg41_cc13.cpp`). The distance test is written in the host at 24-bit precision
(item 6 below); that file's `proximity_check_00750000` is the 53-bit form. The callback goes through the same
call path the listener dispatchers use (`lua_getfield` global, `push_resolved_entity_by_id`,
`lua_pcall`, `note_error`).

The standalone bodies `lua_add_proximity_trigger` / `lua_remove_trigger`
(`src/cc13_left55_seg56c.cpp`) need the 72-method `Left55Seg56cHost` and object-level trigger
allocations. Neither exists in this host, so they are not wired. The host code follows the
same sequence.

## Substitutions and uncertainty (labelled in the code)

1. **Timing.** The walk runs once per fixed step, after this host's motion step and before the
   next step's fan-out. In the image it runs in wave 2. The units are walked in index order;
   the image walks the job groups' element order. Callbacks run inline; `00887E50`'s queued
   arm, which runs them at the fan-out's row 7 of the same step, is not modelled.
2. **Admission.** `unit_active()` stands for wave 2's `+5Eh`/`+BDh` test.
3. **Lists.** Only units of this host carry lists. A watched entity that is not a unit is
   counted `unresolved` and its trigger is not stored.
4. **Target gone.** The `0074FFE0` observer is not kept, so a trigger watching a target unit
   that is deleted keeps testing that unit's last pose. All of this installation's USN04/JM13
   calls target NavPoints or buildings.
5. **Other trigger kinds.** Line-of-sight (`008C9460`) and squad-ordnance (`008C9A10`) triggers
   stay unbound. `RemoveTrigger` answers false for their names, where the image answers true.
6. **x87 precision.** The host test assumes 24-bit precision control: each product and the sum
   round to float. The thread that runs wave-2 jobs is not established; under 53 bits the sum
   rounds once (`proximity_check_00750000` in `src/gameplay_seg41_cc13.cpp` models that). The
   two differ by at most one ulp at the boundary.
7. **`game+1FE4h == 2`.** `00887E50`'s early return is not modelled.

## Script sanity checks (this installation)

- USN04 `:1881-1885`: `luaUSN04_CarrierMemberReachedEvacZone(pEntity)` calls
  `RemoveTrigger(pEntity, "CarrierMemberReachedEvacZone")` from inside the callback. The
  trigger is still enabled there (`0074FDD0` retires it after the call), so `RemoveTrigger`
  answers true, and the second retire is harmless. The callback reads only its first argument.
- JM13 `:2812` watches `Mission.PlayerUnit` against `Mission.Prison` (`FindEntity("Radiotower 01")`,
  `:422`, a building: a scene marker in this host). `:3251` watches the generated
  `Mission.IJNParatrooper` against `Mission.CC`, which is nil: every assignment (`:398`) is
  commented out. `0088A810` then reads `ID` from a nil argument through `00B67910`; whether
  that raises a Lua error or yields nil (point arm, `00888760` on nil) is not established. The
  host takes neither arm: it leaves the zero point with position_valid clear.
- The callbacks take a global name. `00887E50` also splits dotted names into table lookups; the
  listener path this host reuses does not, and no USN04/JM13 call uses a dotted name.

## Prediction for a USN04 pair (switch ON vs OFF)

- `ConvoyReachedSplitNavpoint` (`:754`/`:757`, 100 m): each convoy ship that comes within
  100 m of NavpointConvoy01/02 runs `luaUSN04_ConvoyMove`, which calls `NavigatorMoveOnPath`
  toward its landing path. The convoy advances past the split navpoint, and
  `Mission.ConvoyReachedHarbour` counts up. OFF, nothing calls `luaUSN04_ConvoyMove`, so the ships keep the move order that took them to the navpoint.
- `JAP_CarrierGroupReachedNavPoint` (`:775`/`:1858`/`:3138`, North03): the first carrier-group
  member there runs `luaUSN04_CarrierGroupReachedNavPoint`. It removes the others' triggers and
  sets `CarrierEvacEnable`.
- `CarrierMemberReachedEvacZone` (`:1938`, North04, 50 m): each member there is removed and
  `Kill(pEntity, true)` runs, so the summary shows Lua kills for those hulls.
- `PTReachedEvacZone` (`:2434`, West04, 100 m): each PT that reaches West04 gets
  `NavigatorMoveToRange` to the nearest border zone and heads off the map.
- Log: `proximity trigger 00750000:` fire lines, the `AddProximityTrigger`/`RemoveTrigger`
  lines, and the summary's adds/fires/removes, with `unresolved=0`. The UNIMPLEMENTED census
  loses `AddProximityTrigger 008c9730` and `RemoveTrigger 008c5fd0`.
