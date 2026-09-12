# Entity event queues: the deferred hit queue, the two pending lists and world expiry

Addresses: 00926700, 00926E80, 009239A0, 00924990, 00926670, 00926890, 00925050, 00470470,
004704B0, 009273A0, 00926FA0, 009269B0, 00924A50, 00924B10, 009267F0, 00926C80, 00926D90,
00925A00, 009248D0, 00781260, 009263C0, 00925C40, 00925C90, 00926390, 00951FB0, 00928C80,
004BCA80, 00903610, 009035E0, 00922FD0, 00903670

Packet `cc2_entity_event_queues`, read-only in Ghidra. Every descriptive name below is a
hypothesis, not a recovered symbol. This packet closes three rows of the follow-up table in
`docs/FIXED_STEP_FANOUT.md` (`deferred_entity_event_queue`, `entity_pending_queues`,
`world_expired_objects`), which are fan-out rows 6 and 14 (`00926700`), 15 (`009273A0`) and
16 (`00903610`).

Everything below was read from the raw listing (`python tools/bsp.py ghidra disasm`), because
the non-returning `_free` annotation truncates the pseudocode of `00926700` and `009273A0`
(`docs/FIXED_STEP_FANOUT.md`, the five gaps).

## Summary

| queue | container | producer | consumer | order |
| --- | --- | --- | --- | --- |
| deferred hit events | `std::list` at `00F899C0` | `00926E80`, under a lock | `00926700`, no lock | **LIFO** |
| pending destroy | `std::list<entity*>` at `00F899A8` | `00926C80` Destroy, under a lock | `009273A0` pass D, no lock | FIFO |
| pending kill | `std::list<entity*>` at `00F899B4` | `00926D90` Kill, under a lock | `009273A0` pass K, no lock | FIFO |
| expiry | intrusive chain from `[[world+4h]]` through `+38h` | `00922FD0` sets `+6Ch = 1` | `00903610`, two-step grace | in chain order |

## 1. The deferred hit-event queue

### The record: one kind, 60h bytes

The follow-up table asked whether the producer's record is the 60h-byte one or the 54h-byte hit
record of `docs/HIT_NARROWPHASE.md`. **It is both**: the queued value is the 54h-byte hit record
followed by a 12-byte impact direction, so the payload is 60h bytes and the node is 68h.

| offset (node) | offset (value) | writer | meaning |
| --- | --- | --- | --- |
| `+0h` | — | `009266AE`, `00926780` | forward link (the drain walks away from it) |
| `+4h` | — | `009266BA`, `00926789` | the link the producer appends to and the drain pops from |
| `+8h`..`+5Bh` | `+0h`..`+53h` | `00926F41` `00925050` | the hit record; `docs/PROJECTILE_IMPACT.md` has the field table, `bsp/hit_narrowphase.hpp` the constants |
| `+10h` | `+8h` | the narrowphase | the hit position; the drain passes `&value` so the consumer reads it at `+8h` |
| `+5Ch`..`+67h` | `+54h`..`+5Fh` | `00926F48`, `00926F4E`, `00926F54` | the impact direction, three floats |

The node size is the literal in `00926691`: `PUSH 0x68 / CALL operator_new`. `0x68 = 8 + 0x60`.
`kHitRecordOffDirection = 0x54` and `kHitRecordSize = 0x54` in `bsp/hit_narrowphase.hpp` already
record the value-relative split; this packet adds only the total.

There is **one event kind**. The drain has no discriminator: every node it pops goes to the same
callee `009239A0`, and the only producer of a node is `00926E80`. The "`vtable[+24h]` dispatch"
the follow-up asked about is not a kind selector; it is step 4 inside `009239A0`
(`00923A2C`, the scoring hook), already documented in `docs/PROJECTILE_IMPACT.md`.

### `00926E80` `BSP_DeferredEntityEventQueue_Push`, the producer

`__fastcall(ECX = record, EDX = direction)`, `RET`, body `00926E80`-`00926F99`. Callers
`00819A20`, `0084BAD0`, `0084BC60` (`docs/FIXED_STEP_FANOUT.md`).

| order | site | step |
| --- | --- | --- |
| 1 | `00926EA2` | `00924990()` returns the lock owner; `EBP = owner[+4h]` is the `CRITICAL_SECTION` |
| 2 | `00926EBB` | when it exists: `EnterCriticalSection` through `[00CE2218]`, then `[cs+18h] += 1` |
| 3 | `00926ED4` | `00E18684 = 1` |
| 4 | `00926EDB` | `00470470` default-constructs a 60h record in the frame at `ESP+1Ch` |
| 5 | `00926EFF` | `00926670(head, head[+4h], &temp)`: `operator new(0x68)`, links, copy-constructs the value with `009253A0` — a `push_back` of the empty temporary |
| 6 | `00926F0D` | `STL_xlen_throw_00926890(1)`, the length-checked size increment, bumps `00F899C8` |
| 7 | `00926F12` | `head[+4h] = node; node[+4h][+0h] = node` — the new node is adjacent to the sentinel on the `+4h` side |
| 8 | `00926F41` | `back()` (two empty checks through `00BF6713`), then `00925050(node+8h, record)` copies the real record over it, heap part array included |
| 9 | `00926F46`-`00926F54` | the three direction floats to `value+54h`..`+5Ch` |
| 10 | `00926F65` | when `record[+4h]` (the shot) is non-null: `shot->vtable[5Ch](29h)`. **The result is discarded** — no test of `EAX` follows |
| 11 | `00926F73` | `004704B0` destroys the stack temporary, `[cs+18h] -= 1`, `LeaveCriticalSection` through `[00CE2210]` |

### `00926700` `BSP_DeferredEntityEventQueue_Drain`, rows 6 and 14

`__cdecl void(void)`, `RET`, body `00926700`-`009267C2`. **Takes no lock**: the only calls in the
body are `00BF6713`, `009239A0`, `004704B0` and `free`. The producer's critical section therefore
protects producers against each other, not against the drain.

| order | site | step |
| --- | --- | --- |
| 1 | `00926700` | `[00F899C8] == 0` jumps straight to step 7 |
| 2 | `00926714` | `node = list.back()` = `head[+4h]`, with two inlined empty checks (`00BF6713`) |
| 3 | `00926735` | `subject = *(void**)(node+8h)`, the record's `+0h` field |
| 4 | `0092673F` | dispatch only when `subject != 0` and `subject[5Eh] == 0` and `subject[5Fh] == 0` |
| 5 | `00926750` | `009239A0(ECX = subject, node+8h, node+5Ch)` |
| 6 | `0092675B`-`009267A6` | `list.pop_back()`: **re-read** `head[+4h]`, unlink, `004704B0(node+8h)`, `free(node)`, size `-= 1` |
| 7 | `009267BB` | `00E18684 = 0`, then `RET` |

**Order is LIFO.** The producer links the new node at `head[+4h]` (step 7 above) and the drain
pops `head[+4h]`; both use the same end of the sentinel ring, so the most recently queued hit is
dispatched first. This holds without deciding which of `+0h`/`+4h` is `_Next`: starting from an
empty ring, inserting n1 then n2 gives `head[+4h] = n2`, `n2[+4h] = n1`, `head[+0h] = n1`,
`n1[+0h] = n2`, and the drain reads `head[+4h]`.

**Re-entrancy is unsafe.** Step 6 re-reads `head[+4h]` after the dispatch instead of reusing the
node from step 2 (this is `back()` followed by `pop_back()`, two separate inlined expressions). If
a handler under `009239A0` pushes a new event, that new node becomes `head[+4h]`, so the drain
pops and destroys it **without dispatching it**, while the node it just dispatched stays in the
list and is dispatched a second time on the next iteration. The loop still terminates, because the
count and the node population stay in step. Whether any handler on the `vtable[ECh]` path reaches
`00926E80` is `contract: unread` here; `0084BAD0` (the blast, a peer packet) and `00819A20` are
producers, so the hazard is reachable in principle.

**Why twice per step.** Rows 6 and 14 are the same call (`docs/FIXED_STEP_FANOUT.md`). Row 14
drains what rows 7 through 13 queued — the mission Lua calls, the think list and, when the world
gate is open, the session pump and `SEntity::InitAll` — in the same step rather than the next.
Row 6 drains whatever was queued after row 14 of the previous step, which is the frame work
outside the fixed step and rows 1 through 5. The claim that both are needed is structural; no run
log was taken for this packet.

`00E18684` is **write-only**: its two references in the whole image are `00926ED4` (set) and
`009267BB` (clear). Nothing reads it, so it cannot be the gate for anything; it is recorded as a
flag with no consumer.

## 2. The two pending entity lists

Two `std::list<entity*>` with 0Ch nodes (`00924B10`: `operator new(0xC)`, links at `+0h`/`+4h`,
the entity pointer at `+8h`). Static-initialised at `00CD3910`/`00CD3940`, registered for
destruction at `00CDF4A0`/`00CDF4B0`.

| list | object | sentinel | size | producer | consumer dispatch |
| --- | --- | --- | --- | --- | --- |
| destroy | `00F899A8` | `00F899AC` | `00F899B0` | `00926C80` at `00926D4E` | `entity->vtable[74h]()` at `0092747F` |
| kill | `00F899B4` | `00F899B8` | `00F899BC` | `00926D90` at `00926E40` | `entity->vtable[80h]()` at `0092751F` |

Both producers append at `head[+4h]` and the flush walks each list from `head[+0h]` forward, so
the pending lists are dispatched **FIFO**, in arrival order — the opposite of the deferred queue,
which pops the end the producer writes.

A third routine touches both: `00925A00` (`__thiscall(entity)`, body `00925A00`-`00925A89`) takes
the `009248D0` lock and, when the entity has no parent or its parent is still live
(`+5Ch != 0`, `+5Dh == 0`, `+60h == 0`, `+5Eh == 0`), removes the entity from the kill list with
`00781260` and clears `+5Fh`, clears `+5Eh`, removes it from the destroy list and clears `+60h`,
then clears `+5Dh`. It is the cancel path: an entity can leave either queue before the flush sees
it.

### `009273A0` `BSP_EntityEventQueues_FlushPending`, row 15

`__cdecl void(void)`, `RET`, body `009273A0`-`009275D1`. It takes **no lock at all**: neither the
snapshot copy nor the in-place clear is serialised against `00926C80`/`00926D90`, which do hold
the `009248D0` critical section.

| order | site | step |
| --- | --- | --- |
| 1 | `009273C2` | both sizes zero → `RET` |
| 2 | `009273DF`, `009273F1` | `00926FA0` copies each global list into a frame-local list |
| 3 | `009273F6`-`00927469` | reset both globals in place (sentinel links to itself, size `= 0`) and `free` every old node; no value destructor runs, the value being a raw pointer |
| 4 | `00927477`-`0092748E` | pass D over the destroy copy: `entity = node[+8h]`, `entity->vtable[74h]()` |
| 5 | `0092749E`-`00927537` | pass K over the kill copy, the block below |
| 6 | `0092753C`-`009275B3` | destroy both copies, sentinels included |
| 7 | `009275BA` | `JMP` back to step 1 — handlers that queued more are flushed in the same call |

The copy-then-clear-then-dispatch shape is what makes this drain re-entrant-safe, in contrast to
`00926700`: a handler that calls Kill or Destroy writes the now-empty globals and is picked up by
the next turn of the outer loop.

Pass K, `0092749E`-`00927521`, is `009263C0` inlined. `009263C0` (`BSP_SceneNode_Remove`,
`00926390`'s neighbour, body `009263C0`-`00926414`) has the same six steps in the same order and
**no references of any kind in the image** — no callers and no data reference, so it is not in a
vtable either. That makes `0092751F` the only reachable dispatch of slot `+80h` found.

| order | site | step |
| --- | --- | --- |
| a | `009274A8` | `h = entity->vtable[18h]()`, the render handle of `docs/CONTROLLED_UNIT.md` |
| b | `009274BB` | when `h != 0` and a second `vtable[18h]()` equals `[00E188DC]`: `004BCA80(ECX = 0)`, which stores `0` into `00E188DC` and calls `00B0D7B0` — the controlled unit lets go of the dying entity |
| c | `009274C8` | `entity[+5Eh] != 0` → skip the rest of this entity |
| d | `009274CE`-`009274DA` | `+5Dh = 1`, `+5Eh = 1`, `+5Fh = 1`, `+5Ch = 0` |
| e | `009274DE`-`00927510` | `00925C40` inlined: under the `00694280` observer lock read `entity[+8h] != 0`, release the lock, and when set call `00696330(entity)` |
| f | `0092751F` | `entity->vtable[80h]()` — the on-killed hook |

For `MDestroyer` (vtable `00CFC3D0`) the two dispatch slots resolve to `+74h = 00926390` and
`+80h = 00951FB0`, which clears `[this+4A4h]` and tail-jumps to `00779AF0`
`BSP_MissionEntity_OnKilledPlayerUnit`; the base implementation is `00928C80`. `00926390` is the
destroy hook proper: when `+5Dh` is clear it sets `+5Dh = 1` and `+60h = 1`, calls `00925C90`
(the observer notification twin of `00925C40`) and tail-calls `vtable[7Ch]`.

### Does a unit killed by damage run the on-killed hook?

**No.** This packet confirms `docs/UNIT_DAMAGE_AND_DEATH.md` and removes its stated caveat.

* Damage death reaches `0077D1A0` `BSP_UnitInstance_DestroyAndBroadcast`, the **only** caller of
  `00926C80` Destroy, which queues on the destroy list and so runs `vtable[74h]` alone.
* `00926D90` Kill is the only producer of the kill list. It also calls `vtable[70h](1)`, which for
  a unit is `00CFC3D0[70h] = 0077D1A0` — so an explicit Kill reaches Destroy as well and lands on
  **both** lists, while damage lands on one.
* The caveat was that a second dispatch of `vtable[80h]` elsewhere would change the reading. The
  second dispatch exists, at `00926408` inside `009263C0`, and `009263C0` is unreferenced.
* Scope: no program-wide byte scan for `CALL [reg+80h]` was run here either. The corroboration is
  the `+5Ch`/`+5Dh` byte scan already recorded in `docs/HIT_NARROWPHASE.md`, which found only
  `00922F30`, `00922F80`, `00922FD0`, `009263C0`, `00925F20`, `009274DA` and `009272EF` writing
  those bytes; of those only `009263C0` and `009274DA` sit in front of a `+80h` dispatch.

### Correction to `docs/FIXED_STEP_FANOUT.md`

| was | is | evidence |
| --- | --- | --- |
| "`00926FA0` (which takes the registry lock from `00924A50` and applies the range algorithm `009269B0`)" | `00926FA0` is the `std::list` copy constructor and takes no lock: `00924A50` allocates a 0Ch sentinel node and self-links it (`00924A50`-`00924A69`: `operator new(0xC)`, `[EAX] = EAX`, `[EAX+4] = EAX`), and `009269B0` is the range insert | `00926FC6` calls `00924A50` and stores the result into `dest[+4h]` at `00926FD2`; `00926FD5` sets `dest[+8h] = 0`; the six pushes at `00926FE3`-`00926FE8` are the range |
| "the front node of the `std::list` at `00F899C4`" (the drain) | the **back** node: `00926714` reads `head[+4h]`, the same end `00926F12` appends to | `00926F12` `MOV [EDI],ESI` with `EDI = head+4h`; `00926714` `MOV ESI,[EDX+4]` |
| `009273A0` "then resets ... **partial**: what the two queues carry is not established" | the queues carry one entity pointer per node at `+8h`; the payload of `00926FA0` is the list copy | `00924B10` `PUSH 0xC`; `00927477` `MOV ECX,[ESI+8]` feeding `vtable[74h]` |

## 3. World expiry

### `00903610` `BSP_World_ReleaseExpiredObjects`, row 16

`__thiscall(world)`, `RET`, body `00903610`-`0090366C`, `this = [game+19CCh]`. It walks the world
node's child chain: `head = *(void**)(*(void**)(world+4h))`, advancing through `entity+38h` — the
same chain `00904BF0` `BSP_World_UpdateEntities` walks (`docs/GAME_WORLD_ENTITIES.md`).

| order | site | step |
| --- | --- | --- |
| 1 | `00903620` | `c = entity[+6Ch]`; `c <= 0` → advance without becoming the resume anchor |
| 2 | `00903627` | `c += 1`, stored back |
| 3 | `00903630` | `c < 3` → the entity is kept, becomes the resume anchor (`00903660`), advance |
| 4 | `00903632` | `c >= 3`: while `entity[+50h] != 0` call `009035E0(entity[+48h])` — destroy every child subtree |
| 5 | `0090364A` | `entity->vtable[0](1)` — the scalar deleting destructor with the free flag; for `MDestroyer` that is `00CFC3D0[0h] = 006FE570` |
| 6 | `00903650` | resume at `anchor[+38h]`, or re-read the chain head when no anchor has been set yet, because the destroyed node has left the chain |

`009035E0` (`__thiscall(node)`, `RET`, body `009035E0`-`00903609`) is the recursive form of steps
4 and 5: `while (node[+50h] != 0) 009035E0(node[+48h]); node->vtable[0](1)`. It destroys the
deepest child first; the loop relies on the destructor unlinking the child and decrementing the
parent's `+50h` count, which is `contract: unread`.

The resume rule has a visible quirk: an entity whose counter is `<= 0` never becomes the anchor,
so after a release the walk restarts at the last **marked** entity and re-examines the unmarked
ones in between. They are no-ops, so the walk still terminates and still reaches the chain tail.

### The grace period

`+6Ch` is set to `1` by `00922FD0` `BSP_SceneNode_Kill`, which also sets `+5Eh = 1`, `+5Dh = 1`,
`+5Ch = 0` and recurses over children whose `+6Ch` is still clear, then tail-calls `vtable[84h]`
(`docs/LUA_BINDING_ENTITY_LOOKUP.md`, `docs/HIT_NARROWPHASE.md`). `00903670`
`BSP_EntityWorld_FlushActivations` marks children whose `+5Eh` is set and `+6Ch` clear.

So the counter is the number of expiry passes the entity has survived: marked (`1`), first pass
(`2`), second pass (`3`, released). An entity is therefore freed on the **second** expiry pass
after it is marked, and it has already failed the `+5Ch` active test since the frame it was
marked, so nothing updates it in between.

What the release does beyond `vtable[0](1)` — unlinking from the world chain, the id tables of
`docs/ENTITY_IDENTITY.md`, the Lua self table and the tick registry — is inside the destructor and
is `contract: unread` in this packet.

## Two locks, three owners

| lock owner | used by | protects |
| --- | --- | --- |
| `00924990` | `00926E80` only | the deferred queue against concurrent producers |
| `009248D0` | `00926C80`, `00926D90`, `00925A00` | the two pending lists against concurrent producers |
| `00694280` `BSP_ObserverLockOwner_Get` | `00925C40`, `00925C90`, pass K inlined | the observer list read at `entity+8h` |

All three take the `CRITICAL_SECTION` at `owner[+4h]` through the imports at `[00CE2218]` /
`[00CE2210]` and keep the recursion count at `cs+18h` themselves. Neither consumer
(`00926700`, `009273A0`, `00903610`) takes any of them.

## Host table

One row per native call site the reconstruction models.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `00926750` | `009239A0` | `dispatch_hit_009239a0` | subject; record, direction; void | subject non-null, `+5Eh` and `+5Fh` clear |
| `00926792` | `004704B0` | `destroy_hit_record_004704b0` | record; —; void | every popped node |
| `00926798` | `00BF65AC` | not modelled — CRT `free` | — | every popped node |
| `00926714`, `0092675B` | `00BF6713` | not modelled — CRT invalid-parameter path | — | size disagrees with the ring |
| `00926EA2` | `00924990` | `deferred_queue_lock` (enter/leave pair) | —; —; owner | always |
| `00926EDB`, `00926F73` | `00470470`, `004704B0` | not modelled — the frame temporary | — | always |
| `00926EFF` | `00926670` | `push_back_node_00926670` | list; head, last, temp; node | always |
| `00926F41` | `00925050` | `assign_hit_record_00925050` | node value; record; void | always |
| `00926F65` | `shot->vtable[5Ch]` | `query_shot_kind_29h` | shot; `29h`; ignored | `record[+4h]` non-null |
| `009273DF`, `009273F1` | `00926FA0` | `copy_pending_list_00926fa0` | dest; source; dest | either size non-zero |
| `0092747F` | `entity->vtable[74h]` | `on_entity_destroyed_74h` | entity; —; void | every destroy-list node |
| `009274A8`, `009274BB` | `entity->vtable[18h]` | `render_handle_18h` | entity; —; handle | every kill-list node |
| `009274C3` | `004BCA80` | `clear_controlled_unit_handle_004bca80` | `ECX = 0`; —; void | handle equals `[00E188DC]` |
| `009274DE`, `00927504` | `00694280` | `observer_lock` (enter/leave pair) | —; —; owner | `+5Eh` clear |
| `00927510` | `00696330` | `notify_observers_00696330` | entity; —; void | `entity[+8h]` non-zero |
| `0092751F` | `entity->vtable[80h]` | `on_entity_killed_80h` | entity; —; void | `+5Eh` clear |
| `0090363B` | `009035E0` | `destroy_child_subtree_009035e0` | child; —; void | `entity[+50h]` non-zero |
| `0090364E`, `00903606` | `entity->vtable[0]` | `destroy_entity_vtable0` | entity; `1`; void | counter reached 3 |

`009239A0`, `00696330`, `00B0D7B0` and everything under `vtable[74h]`/`[80h]`/`[0]` are contracts,
named and not read here beyond the bodies quoted above.

## Coverage

| routine | body | coverage |
| --- | --- | --- |
| `00926700` | `00926700`-`009267C2` | complete |
| `00926E80` | `00926E80`-`00926F99` | complete |
| `009273A0` | `009273A0`-`009275D1` | complete |
| `00926FA0` | `00926FA0`-`0092700B` | complete |
| `009269B0` | `009269B0`-`00926AC3` | partial: the insert loop `009269F0`-`00926A54` only; the tail from `00926A56` (`0077A7E0`) is unread |
| `00926C80`, `00926D90` | `00926C80`-`00926D8A`, `00926D90`-`00926E7B` | complete, as producers |
| `00925A00` | `00925A00`-`00925A89` | complete |
| `009263C0` | `009263C0`-`00926414` | complete |
| `00926390` | `00926390`-`009263B1` | complete |
| `00903610` | `00903610`-`0090366C` | complete |
| `009035E0` | `009035E0`-`00903609` | complete |
| `00925050` | `00925050`-`0092517F` | partial: the scalar copy `00925050`-`009250C7` and the part-array reallocation entry; the copy loop tail is `contract: unread` |
| `009239A0`, `00696330`, `00925C90`, `00928C80`, `00951FB0` | — | contract: unread (bodies quoted only where cited above) |

## Reconstruction

`include/bsp/entity_event_queues.hpp`, `src/entity_event_queues.cpp`:

* `DeferredEntityEventNode` / `PendingEntityNode` with the offset constants above.
* `deferred_queue_should_dispatch` and `pending_kill_should_tear_down`, the two flag gates.
* `expiry_step(counter)` -> `EntityExpiryAction`, the `+6Ch` rule.
* `DeferredEntityEventHost`, `PendingEntityQueueHost`, `WorldExpiryHost`: one pure-virtual per
  native call site in the host table.
* `drain_deferred_entity_events_00926700`, `flush_pending_entity_queues_009273a0` and
  `release_expired_world_objects_00903610`, the three drains as sequences over those hosts. They
  are shaped to replace the three unimplemented `FixedStepFanout::*` records of
  `docs/GAME_EXECUTABLE.md` (rows 6/14, 15 and 16).

The drains model the **order and the gates**, not the allocator: the C++ works over a caller-owned
`std::vector` of node views, because porting the MSVC `std::list` node churn would add nothing the
listing does not already state.

## Follow-up

| question | why it is open |
| --- | --- |
| does any `vtable[ECh]` handler reach `00926E80`? | it decides whether the drain's re-entrancy hazard fires in practice; `0084BAD0` is a peer packet |
| what `vtable[0](1)` does for an entity | the release's real effect on the id tables, the Lua self table and the tick registry |
| `009269B0`'s tail from `00926A56` | an unread branch through `0077A7E0` |
| `00927050`'s `+5Dh` write at `009272EF` | a third teardown site, in a property-bag routine outside this packet |

## Correction from docs/ENTITY_LIFECYCLE_TAILS.md (packet cc2_entity_lifecycle_tails)

- **Was:** 009269B0's tail from 00926A56 is an unread branch through 0077A7E0
  **Is:** the catch(...) rollback of the std::list range insert, reached only through the frame handler pushed at 009269B5 and ending in a rethrow; 0077A7E0 is the checked-iterator operator!=, library code
  **Evidence:** the only jumps in 009269B0 target 009269F0, 009269F9, 009269FE, 00926A10, 00926A1A, 00926A4C, 00926AA8, 00926A70 and 00926AB1; 00926AAC PUSH 0; PUSH 0; CALL 00BF6885
- **Was:** 00927050's +5Dh write at 009272EF, a third teardown site in a property-bag routine
  **Is:** the mission Lua deadMeat property applied at creation; 00927050 is the spawn-descriptor apply at virtual slot +9Ch, which also writes entity+70h there
  **Evidence:** 009272E3 CALL 00BD68D0 reads the field whose key string is at 00D190D8, 009272E8 tests the result, 009272EF and 009272F3 are the two stores; the routine's only direct caller is BSP_MissionEntity_CreateLuaSelfTable at 00928A1E and it sits at +9Ch of 31 vtables
- **Was:** whether any vtable[ECh] handler reaches 00926E80 is contract: unread, so the drain's re-entrancy hazard is reachable in principle
  **Is:** no handler reaches it; the hazard is not reachable through the hit path
  **Evidence:** 564 vtables anchored on the code's own data references, 163 distinct +ECh bodies, 81 of them without a Ghidra function and scanned raw for E8/E9 targets, forward search eight levels: no path to 00926E80. The three producers 00819A20, 0084BAD0 and 0084BC60 have 19 ancestors, all projectile and explosion ticks.
- **Was:** what the release does beyond vtable[0](1) is inside the destructor and is contract: unread
  **Is:** eleven steps: the entity id table, the name string, the +168h reference, the children, the world update chain, the sibling chain, the world+0Ch unit list, two pooled strings, the weak and callback owners and the observer edges
  **Evidence:** 009287B0, 00925780 and 00695760 read complete; the call sites are the host_steps rows
