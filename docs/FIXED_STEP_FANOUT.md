# Fixed-step subsystem fan-out (`00875E0C..00875EDF` and the tail)

Addresses: 00875e0c, 00c5c540, 004462d0, 0042e630, 0098bdb0, 00874de0, 00926700,
00888230, 00929460, 00778450, 0077ec20, 00874c90, 00925f20, 0076ffc0, 009273a0,
00903610, 00a317f0

Every name here is a hypothesis, not a recovered symbol, except the two the image itself
spells: the profiler scope strings `Simulate` (inside `00C5C540`) and `SEntity::InitAll`
(inside `00925F20`).

`docs/IN_MISSION_SUBSYSTEM_TICK.md` reconstructed the fixed-step driver `00875BB0` as a
clock rule plus four host methods and left `run_step_subsystems` as `contract: unread` for
all sixteen calls. This packet reads each callee's body and turns that host method into a
sequence. `include/bsp/fixed_step_fanout.hpp` and `src/fixed_step_fanout.cpp` hold the
reconstruction; the job waves of the same step are `docs/FIXED_STEP_JOB_WAVES.md`.

The fan-out runs **once per fixed step**, i.e. once per 0.05 s of simulated time, zero or
more times per rendered frame. The float every call receives is `00D0DE84 = 0.05f`, never
the frame delta.

## The sixteen calls in listing order

`this` is the ECX the site sets; `game` is `[00E188A8]`. `ret` is the callee's own return
form, which is where the argument count comes from (checklist rule 7): a `RET 4` callee
consumes the float the site pushed, a `RET` callee consumes nothing.

| # | site | callee | name | `this` | args | ret | gate | owner |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `00875E0C` | `00C5C540` | `DYN_PhysicsWorld_Simulate` | `[game+18h]` | `0.05f` | `RET 4` | always | physics (library) |
| 2 | `00875E24` | `004462D0` | `BSP_GameDynamicsList_ApplyBuoyancyStep` | `[game+30h]` | `0.05f` | `RET 4` | always | unit (reconstructed) |
| 3 | `00875E33` | `0042E630` | `BSP_SpatialIndex_GetSingleton` | none | none | `RET` | always | pure/registry |
| 4 | `00875E3A` | `0098BDB0` | `BSP_SpatialIndex_RefreshMovedNodes` | call 3's `EAX` | `0.05f` | `RET 4` | always | scene/registry |
| 5 | `00875E3F` | `00874DE0` | `BSP_FixedStepCallbackList_Run` | none | none | `RET` | always | engine |
| 6 | `00875E44` | `00926700` | `BSP_DeferredEntityEventQueue_Drain` | none | none | `RET` | always | entity |
| 7 | `00875E55` | `00888230` | `BSP_MissionLuaHost_DrainQueuedCalls` | `[game+1A08h]` | none | `RET` | always | script |
| 8 | `00875E64` | `00929460` | `BSP_EntityThinkList_RunDue` | none | `0.05f` | `RET 4` | always | script |
| 9 | `00875E91` | `00778450` | `BSP_Session_PumpStep` | `game+1EF0h` | `0.05f` | `RET 4` | **`+4ACh`** | session |
| 10 | `00875E96` | `0077EC20` | `BSP_Replication_ApplyPendingEntityCreates` | none | none | `RET` | **`+4ACh`** | session |
| 11 | `00875E9B` | `00874C90` | `BSP_TickRegistry_FlushPendingGroups` | none | none | `RET` | **`+4ACh`** | engine |
| 12 | `00875EA2` | `00925F20` | `BSP_SEntity_InitAll` | `CL = 0` | none | `RET` | **`+4ACh`** | entity |
| 13 | `00875EBF` | `0076FFC0` | `BSP_Session_FlushOutboundStep` | `game+1EF0h` | `0.05f, 1` | `RET 8` | **`+4ACh`** | session |
| 14 | `00875EC4` | `00926700` | `BSP_DeferredEntityEventQueue_Drain` | none | none | `RET` | always | entity |
| 15 | `00875EC9` | `009273A0` | `BSP_EntityEventQueues_FlushPending` | none | none | `RET` | always | entity |
| 16 | `00875EDA` | `00903610` | `BSP_World_ReleaseExpiredObjects` | `[game+19CCh]` | none | `RET` | always | world |

The gate for rows 9-13 is `00875E69..00875E7F`: `game != 0` **and**
`[[game+19CCh]+4ACh] != 0`, the world object's active byte. When the world is inactive the
block is skipped entirely and control resumes at row 14 (`JE 0x875EC4` twice).

`00926700` is called twice per step (rows 6 and 14), with the whole gated block between
them, so events the gated calls queue are drained in the same step.

## What each callee does

**1. `00C5C540`, the Dynamics world step.** `__thiscall void(world, float)` on `[game+18h]`,
body `00C5C540..00C5C706`. Part of the third-party `mitengine` Dynamics block
(`[00C30930,00C5DF60)`), so the library name is kept. It opens an rdtsc profiler scope
labelled `Simulate` (`00C5C588`), copies each registered body's transform into its proxy,
then runs **its own** fixed sub-stepping: `accumulated += dt`, then while
`accumulated > this[0]` it calls `00C5BB30(this, this[0])` and subtracts, bounded by the
substep budget in `this[0xD]`; a remainder larger than `00D7A398` gets one more partial
substep, and the accumulator is cleared at `00C5C6C6`. The engine therefore takes several
physics substeps inside one 0.05 s game step.

**2. `004462D0`.** Already reconstructed; see `docs/GAME_DYNAMICS_LIST.md` and
`include/bsp/game_dynamics_list.hpp`. Not re-read here.

**3+4. `0042E630` then `0098BDB0`.** These are one C++ statement, not two calls with one
argument each: the float is pushed at `00875E2F`, the getter `0042E630` (`__cdecl`, `RET`,
no arguments) supplies the `this` in `EAX`, and `0098BDB0` (`__thiscall`, `RET 4`) consumes
the pushed float. `0042E630` is a double-checked lazy singleton: `operator new(0x16018)`,
constructor `0042D450` (a `memset` and nothing else), registered with
`BSP_SingletonLifetime_Register`; 26 call sites across the image. `0098BDB0` walks the
list at `[this+16014h]` through `+48h` and, for every node whose `+8h` byte is clear, calls
`0098BC70`, which refreshes the node's entity pose (`+4Ch`, through
`BSP_EntityPose_RefreshWorld 00414DB0`), copies the world matrix `+CCh` and the cached
inverse `+110h` into the node, computes a four-byte key from a transformed point
(`0098A750` -> `BSP_Vector3f_TransformAffinePoint`, `0098AD60` -> the CRT float helpers)
and, when the key differs from `[node+3Ch]`, re-registers the node through `0098A3D0` and
`0098A310`, then recurses over the children at `[node+FCh]`. The "spatial index" reading of
the key is **provisional**; what is proven is the singleton, the walk, the matrix copy and
the re-registration on a changed key. The float argument is not read by `0098BDB0` itself.

**5. `00874DE0`, the per-step callback list.** `__cdecl void(void)`, `RET`. Walks the
intrusive list whose head slot is `00E0B748` and whose end sentinel is `00E0B76C`
(node: `+0Ch` prev, `+10h` next, `+18h` linked, `+19h` repeating), calling
`node->vtable[+10h](0.05f)` on each and unlinking any node whose `+19h` byte is clear, so a
registration made with `repeating = false` fires exactly once. The producer is `00875A80`
(`__thiscall void(node, bool repeating)`), which appends under the critical section from
`00875340`; `00874DE0` walks **without** taking that lock. Callers of the registrar:
`006F5610`, `007AC000`.

**6+14. `00926700`, the deferred entity-event queue.** `__cdecl void(void)`, `RET`. While
the count `00F899C8` is non-zero it takes the front node of the `std::list` at `00F899C4`,
and when the record's subject `[node+8h]` is non-null with `+5Eh` and `+5Fh` both clear it
calls `009239A0(subject, &record, [node+5Ch])`; then it unlinks the node, destroys the
record (`004704B0`) and frees it. It clears the "queue non-empty" byte `00E18684` on exit.
The producer is `00926E80` (`__thiscall`, under the lock from `00924990`), which sets
`00E18684 = 1` and queues a 60h-byte record; its callers are `00819A20` (explosion path),
`0084BAD0` and `0084BC60`. `009239A0` resolves a source object through
`source->vtable[+5Ch](29h/2Ah/44h)` type queries and finally calls
`subject->vtable[+24h](sourceId, &position, extra)`, the position being the three words at
`record+8h`. Reading the queue as *damage/impact* events is **provisional**; the queue,
the drain and the dispatch are not. The drain does **not** take the producer's lock.

**7. `00888230`, the queued Lua calls.** `__thiscall void(luaHost)`, `RET`, on
`[game+1A08h]` (`docs/MISSION_LUA_HOST.md`). While `[this+10h]` is non-zero it pops the
front of the list at `[this+0Ch]` and calls
`BSP_MissionLuaHost_CallNamedThreadSafe(node+8h, node+10h, node+18h, node[9], node[10])`,
then destroys (`008876B0`) and frees the node. This is where calls queued from other
threads reach the Lua machine, one drain per fixed step.

**8. `00929460`, the entity think list.** `__cdecl void(float)`, `RET 4`. Subtracts the step
from the global countdown `00F89A04`; when it goes negative it is refilled from
`00D7A2B0` and, if `[00109CEFC]->vtable[+0Ch]()` agrees, `collectgarbage()` is run through
`BSP_LuaMachine_RunString`. It then walks the list at `00F89AB4` (`+0h` prev, `+4h` next,
`+8h` entity, tail `00F89AB8`, count `00F89AB0`): an entity with `+5Ch` set, `+5Dh`, `+60h`
and `+5Eh` clear and a script object at `+1D8h` has its think timer `+1E0h` decremented by
the step and its think function (`00929150`) run when the timer expires, or immediately
when `+1DCh` is clear and the global countdown expired. The **first** node that fails the
eligibility test is unlinked, freed and the routine returns, so at most one dead entry is
reaped per step and the rest of the list waits for the next one.

**9. `00778450`, the session pump.** `__thiscall void(session, float)`, `RET 4`, on
`game+1EF0h`. Sets the `00F876A1` in-step flag (saving and restoring the previous value),
runs the countdown at `[this+278h]`: when it expires the two objects at `+188h` and `+18Ch`
are deleted through their `vtable[0](1)` and the timer is reset from `00D7A260`. If the
global `00F8A2FC` exists it gets `vtable[+5Ch](step)`. When `[this+F4h]` is non-zero it
runs `0076AA00(step)`, `00785CC0` (only with `+18Ch`), `007700E0`, `00777850` and
`BSP_PeerManager_PumpMessageQueues 00776230`. `docs/GAME_SESSION_POLLS.md` describes the
same routine as the first half of `BSP_Multiplayer_Tick 00778560`, which passes the **raw
frame delta**; here it gets the fixed step.

**10. `0077EC20`, pending replicated creates.** `__cdecl void(void)`, `RET`. With the count
`00F871A8` non-zero it walks the list at `00F871A4`, resolves each record's 16-bit id
against one of two id tables (`00F89A54`/`00F89A0C` below `00F89A10`, `00F89AA8`/`00F89A60`
at or above it, stride 10h, slot field `+0Ch`) and, when the slot is free, instantiates the
entity through the record's `vtable[+1Ch]`, replays nine values through the new object's
`vtable[+154h]` when `[record+88h]` is set, and calls `0077EB50`. An occupied slot is the
`Duplicated Create %d received` branch.

**11. `00874C90`, the tick-registry flush.** `__cdecl void(void)`, `RET`. Splices every
node of the pending list (head slot `00E0B6D8`, end sentinel `00E0B704`) into the group its
`+14h` index names, then resets the pending list to empty. This is the producer side of the
job waves: see `docs/FIXED_STEP_JOB_WAVES.md`. It runs **inside the world gate**, so
registrations made while the world is inactive stay pending.

**12. `00925F20`, `SEntity::InitAll`.** `__fastcall void(char)`, `RET`. The site sets only
the low byte (`00875EA0 XOR CL,CL`), so the argument is `false` and the upper 24 bits of
ECX are the previous value, untouched: the callee reads `CL` alone. Guarded by the count
`00F899D4`. It opens the profiler scope whose string is `SEntity::InitAll`, then makes
three passes over the pending-entity list at `00F899D0`, calling each entity's
`vtable[+9Ch]`, then `vtable[+A0h]`, then `vtable[+A4h]`, reporting progress to `0057C1A0`
between entities and logging `INIT,ENUM:%d`. It sets `00F899A5 = 1`. Entities spawned
during a step are therefore initialised in three phases before the step ends.

**13. `0076FFC0`, the outbound session step.** `__thiscall void(session, float, int)`,
`RET 8`, on `game+1EF0h`. The literal `1` is pushed first and the float second, so the
prototype is `(step, mode)` with `mode = 1` here and `mode = 0` from `BSP_Multiplayer_Tick`
(`docs/GAME_SESSION_POLLS.md`); the mode reaches `0076F210(step, mode)`. With
`[this+F4h] == 1` (host) it runs `00785F90` or `00785FC0` depending on `[this+E9h]` and the
game state, then `007866B0(step)`. Always: `0076F210`,
`BSP_SessionTransport_FlushPeerBuffers 007848F0`, `00782870`, `0076C4A0(step)`. Finally,
with the game in state `0Dh`, `game+5FCh` set, `[this+290h]` set, `[this+188h]` present and
`00784CD0` agreeing, it calls `BSP_Game_RequestState(10h)` and clears `[this+290h]` - the
end-of-mission transition. It also sets and restores `00F876A1`.

**15. `009273A0`, the two pending entity queues.** `__cdecl void(void)`, `RET`. Returns
immediately when both counts `00F899B0` and `00F899BC` are zero. Otherwise it runs
`00926FA0` (which takes the registry lock from `00924A50` and applies the range algorithm
`009269B0`) over the list at `00F899A8` and then over the one at `00F899B4`, then resets
both lists to self-linked empty, zeroes both counts and frees every node. Coverage is
**partial**: `00926FA0`'s payload was not followed past `009269B0`, so what the two queues
carry is not established here.

**16. `00903610`, the world's expired objects.** `__thiscall void(world)`, `RET`, on
`[game+19CCh]`. Walks the list at `[[this+4h]]` through `+38h`. A node whose counter
`+6Ch` is at least 1 is incremented; once it reaches 3 the node's own sub-list at `+50h` is
drained through `009035E0` and the node is destroyed through `vtable[0](1)`, with the walk
resuming from the predecessor. Objects are therefore released two fixed steps after they
are marked, which is the grace period the render thread's double buffer needs.

## The tail, `00875FC1..00875FFD`

After the interpolation wave the driver stores the leftover accumulator into `00F876B4`
(`00875FC1`) and then tests four things in order (`00875FD1..00875FF5`): `00F8AB04` is
non-zero, the in-mission interface manager `00E198C4` is non-null, its sub-object `+54h` is
non-null, and that sub-object's byte `+5h` is set. Only then does `00875FF7` call
`00A317F0`. In this build `00A317F0` is a single `RET`: the hook exists and does nothing,
so the tail has no observable effect and a host may implement it as a no-op. The
surrounding code (`00A31730` registers a tick node at `owner+170h` in the same segment)
places the stub in the strategic-coordinator area.

## Reconstruction

`include/bsp/fixed_step_fanout.hpp` declares:

- `FixedStepFanoutOwner`, the owner areas above (a local enum; `MissionFrameOwner` in
  `bsp/mission_state_frame.hpp` has no physics or entity-registry value and is not edited
  here).
- `FixedStepFanoutStep`, one row per call site with `call_site`, `callee`, `owner`,
  `gated`, `takes_step` and the `this` expression, and the two accessors over the
  sixteen-row table.
- `FixedStepWorldGate` and `fixed_step_world_gate_open`, the `00875E69..00875E7F` rule.
- `FixedStepTailGate` and `fixed_step_tail_hook_runs`, the `00875FD1..00875FF5` rule.
- `FixedStepFanoutHost`, fifteen pure-virtual methods (sixteen call sites; `00926700` is
  one method called twice), each named for its callee's address.
- `run_fixed_step_subsystem_fanout_00875e0c(step, gate, host)` and
  `run_fixed_step_tail_00875fd1(tail, host)`.

Coverage: **complete** for the call sequence and both gates; the callee bodies are
summarised, not reconstructed. `src/fixed_step_fanout.cpp` contains no invented globals and
no stubs: the host is the only way out.

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/IN_MISSION_SUBSYSTEM_TICK.md`: `00875E33` -> `0042E630` takes `0.05f` | `0042E630` is `__cdecl int(void)`, `RET` at `0042E6DD`, and takes nothing | the float pushed at `00875E2F` is consumed by `0098BDB0`'s `RET 4` at `0098BDD5`; a `__cdecl` callee with an argument would need an `ADD ESP,4` the site does not have |
| same doc: `00875E3A` -> `0098BDB0` `this` is "`0042E630`'s result", contract unread | `__thiscall void(this, float)` on the singleton, refreshes moved nodes | `00875E38 MOV ECX,EAX`; body `0098BDB0..0098BDD7`, `RET 4` at `0098BDD5` |
| same doc: `00875EA2` -> `00925F20` "none" for `this` | `__fastcall void(char)` with the argument `false` in `CL` | `00875EA0 XOR CL,CL`; `0092638A` is `C3`, so nothing is popped and the only input is `CL` |
| same doc: `00875EBF` -> `0076FFC0` args "`0.05f, 1`" | the order is `(step, mode)` with `mode = 1`; `RET 8` | `00875EAD PUSH 1` then the float store at `00875EB6`, so the float is the lower argument; `007700DC RET 0x8` |
| same doc: the sixteen rows `contract: unread` | each row now carries a contract and an owner area | this document |

Nothing in the earlier doc was wrong about the sequence, the gate or the step constant.

## no_ghidra_function

none - all sixteen callees and the tail hook are Ghidra functions with bodies:
`00C5C540..00C5C706`, `0042E630..0042E6DD`, `0098BDB0..0098BDD7`, `00874DE0..00874E29`,
`00926700..009267C2`, `00888230..008882A8`, `00929460..009295AB`, `00778450..00778552`,
`0077EC20..0077EDE8`, `00874C90..00874CFE`, `00925F20..0092638A`, `0076FFC0..007700DE`,
`009273A0..009275D1`, `00903610..0090366C`, `00A317F0` (single `RET`).

Five of them decompile with a spurious `return` after `_free` because Ghidra marked the CRT
free helper non-returning (`python tools/bsp.py ghidra flow <fn>`): `0077EC20`
(`0077ED6A..0077ED77`), `00926700` (`0092679D..009267AD`), `00888230`
(`00888299..008882A0`), `00925F20` (`0092635A..00926367`), `00929460`
(`00929531..00929534`), and `009273A0` has five such gaps. Every contract above was taken
from the raw listing across those gaps, not from the truncated pseudocode; the integrator
can run `python tools/ghidra_flow_repair.py <fn> --apply` to remove them.

## Follow-up packets

| packet | addresses | why |
| --- | --- | --- |
| `spatial_index_rebucket` | 0042e630 0042d450 0098bdb0 0098bc70 0098a310 0098a3d0 0098a750 0098ad60 | the 16018h-byte singleton, the four-byte cell key and the re-registration pair; 26 call sites depend on the getter |
| `deferred_entity_event_queue` | 00926700 00926e80 009239a0 00924990 | the producer's 60h-byte record layout and the `vtable[+24h]` dispatch, which decides whether this is the damage path |
| `entity_pending_queues` | 009273a0 00926fa0 009269b0 00924a50 | the two lists at `00F899A8`/`00F899B4` are only a shape here |
| `entity_think_dispatch` | 00929460 00929150 00928330 00928380 | the think-function call itself, and why the GC countdown gates untimed entities |
| `sentity_init_all` | 00925f20 0057c1a0 00922f30 00922f80 | the three init phases and the progress callback, shared with the scene-file reader |
| `session_step_vs_frame` | 00778450 0076ffc0 0076f210 00778560 | the same two routines run once per step with `mode = 1` and once per frame with `mode = 0`; the difference is unread |
| `world_expired_objects` | 00903610 009035e0 | the two-step grace period and what the `+50h` sub-list holds |

## Correction from docs/ENTITY_THINK_DISPATCH.md

Packet `cc2-entity-think-dispatch` (main b50e9e8c) re-read item 8 from the listing after the
`_free` fall-through repair and corrects three claims that came from pseudocode truncated by
the non-returning free: (1) the walk does not stop after the first erase; (2) `00928330` is a
loop that clears the whole pending list at `00F89ABC`, not a single removal; (3) `00F89AB4` is
the head field of the 0Ch-byte list object at `00F89AB0` (`+0h` count, `+4h` head, `+8h` tail),
not the list itself, and the 3.0 refill constant is a double. The node is 0Ch bytes
`{prev, next, entity}`; the think name, delay flag and remaining delay live on the entity at
`+1D8h`/`+1DCh`/`+1E0h`; the think function is invoked by name with no arguments through
`BSP_MissionLuaHost_CallNamedThreadSafe(entity+178h, name, 0, 0, -1)` and re-resolved every
call; an untimed `SetThink` is a three-second heartbeat because it shares the collectgarbage
countdown `00F89A04`; `00898150` arms a delay clamped to 0.5f.

## Correction from docs/ENTITY_EVENT_QUEUES.md

Packet `cc2-entity-event-queues` (main 0bd8a3cc) read the three queue drains in full and corrects
this doc: (1) `00926FA0` is the `std::list` copy constructor and takes no lock; `00924A50` is
`operator new(0xC)` building a self-linked sentinel node stored into `dest+4h`, not a registry lock
(no `EnterCriticalSection` in `00926FA0` or `009273A0`); (2) the deferred drain takes the BACK node
(`00926714` reads `head[+4h]`, the end the producer links at `00926F12`), so the queue is LIFO and,
being lock-free with a re-read of the back pointer after dispatch, an event queued from inside a
handler is popped undispatched; (3) `009273A0`'s two lists hold 0Ch nodes with one entity pointer at
`+8h`; the flush copies both lists, clears the globals, dispatches `vtable[74h]` over the first copy and
the `009263C0` block ending in `vtable[80h]` over the second, and loops until both globals are empty.
The deferred queue holds one event kind (the 54h hit record plus the impact direction at `+54h`, in
68h nodes), expiry ages `+6Ch` and releases at 3; a damage death lands only on the destroy list, an
explicit Kill on both. `include/bsp/entity_event_queues.hpp` replaces the `drain_deferred_entity_events`,
`flush_pending_entity_queues` and `release_expired_world_objects` records of the fan-out host.
