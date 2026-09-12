# The command controller's execution engine (`unit+738h`, `+54h`..`+1CBh`)

Addresses: `00720CD0` `00720CA0` `00720850` `008358D0` `0071E6C0` `00835C70` `0071F600`
`0071E7F0` `0071E610` `0071BE40` `0071DEE0` `0071D780` `0071D6D0` `00835F60` `00836920`
`00D09EC0` `00D09F58` `00D09FE8` `00D7A218` `00F87574`

Packet `cc2_director_commands`. Every descriptive name here is a hypothesis, not a recovered
symbol, with one exception: `InternalClearPrimaryCommand` is the literal trace string
`00720850` passes to `TRIV_body_004254B0`, so `00720850`'s own name is recovered.

Follows `docs/WEAPON_DIRECTOR.md` (the controller object at `unit+738h`, `250h` bytes, constructor
`008366D0` reached from `00810F60`). Contracts this doc cites without re-reading:
`docs/CRUISE_COMMAND.md` (`00816E30`, `0071ECF0`, `0071E550`, `00835E90`, `0071C830`, `0071D880`,
`00721A40`, `00721030`, `007216D0`, `00835AE0`/`00835B40`/`00835BF0`), `docs/UNIT_COMMANDED_SPEED.md`
(`009E1170` stages, `00836920` stage handling, `0071D810`, `0071D9E0`, `0071BE60`, `007788D0`,
`00465080`), `docs/SCENE_COMMAND_TYPES.md` (the 26 command classes),
`docs/ENTITY_ORDER_MESSAGE.md` (`0077D600`), `docs/SESSION_MESSAGE_DISPATCH.md` (kind `58h`).

## Headline

The controller is a **ten-slot command queue plus a one-slot override**. A command is a pointer to
one of the 26 eight-byte singletons in `00E08EF8..00E08FC7`; the per-command data is a separate
`18h`-byte parameter record stored beside the pointer. The command classes carry **no behaviour**:
their vtables are four constant getters (`docs/COMMAND_CLASSES.md`). All execution is in the
controller, which dispatches by **comparing the command pointer against the singleton addresses**.

## Command-slot layout

A slot is `1Ch` bytes: the command pointer, then the `18h`-byte parameter record.

| Offset in slot | Field | Evidence |
| --- | --- | --- |
| `+0h` | command singleton pointer, or 0 for an empty slot | `00720850`: `param_1[(index+3)*7]`, i.e. `+54h + 1Ch*index`; `0071E6C0` stores at `0071E764` |
| `+4h` | `u8 hasTargetEntity` | `00720CD0` `00720D42: MOV byte ptr [ESP+0xc],0x1` under `entity != 0` |
| `+5h` | `u8 hasPosition` | `00720850` `007209xx`: set to 1 when the slot is snapped to the target's world position |
| `+6h` | `u16 targetEntityId` | `00720CD0` `00720D47: MOV AX,word ptr [EAX+0x174]` (`kEntityOrderObjectIdOffset`) |
| `+8h` | target entity pointer | `00720CD0` `00720D31: MOV dword ptr [ESP+0x10],EAX` |
| `+0Ch`,`+10h`,`+14h` | target position x, y, z | `00720CD0` `00720CFA`..`00720D24`, three `MOVSS` from `00F87574`/`78`/`7C`; `00720850` copies `target+0FCh/+100h/+104h` |
| `+18h` | trailing dword, always written 0 by the producers read | `00720CD0` `00720D68: MOVSS dword ptr [ESP+0x28],XMM0` with `XORPS XMM0,XMM0` |

The shift loop in `00720850` (`piVar5[-4] = piVar5[3]` .. `piVar5[2] = piVar5[9]`) moves exactly
these eight fields one slot down, which is what fixes the `1Ch` stride and the field boundaries.

## Controller command state

| Offset | Field | Evidence |
| --- | --- | --- |
| `+30h` | `int mode`: 0 idle, 1 queue head is current, 2 override is current | `0071BE40` reads it; `0071E6C0` `0071E795` sets 1; `0071E7F0` sets 2; `00720850` sets 1 or 2 |
| `+44h` | `u8` "queue head accepted its target" | `0071F600` writes `BSP_WeaponDirector_CommandAcceptsTarget`'s answer; `00835C70` overwrites it |
| `+48h` | `int` stage of the queue-head command | `00836920` `00836A81: CMP dword ptr [ESI+0x48],0x2`; raised by `0071D810` (contract) |
| `+4Ch` | `u8` "override accepted its target" | `0071F600` (`param_2 == 0` arm); `00835C70`; cleared by `0071E7F0` |
| `+50h` | `int` stage of the override command | `00836920` `param_1[0x14] == 1` before `FUN_0071D9E0(2)`; cleared by `0071E7F0` |
| `+54h`..`+16Bh` | the ten queued slots, stride `1Ch` | `00720CA0`'s loop from `+150h` down by `1Ch`, ten iterations |
| `+16Ch`..`+187h` | the **previous** command record, same slot shape | `00720850` copies slot 0 into it before removing slot 0 |
| `+188h`..`+1A3h` | the **override** command record, same slot shape | `0071E7F0` writes all eight fields; `0071BE40` returns `+188h` in mode 2 |
| `+1A4h`..`+1CBh` | ten path objects, one per slot, `50h` bytes each | `00720850` `operator_new(0x50)` then `FUN_0071FB90`; indexed `+1A4h + 4*index` in `0071DEE0` |

`+1A4h` and `+1CCh` reconcile with `docs/WEAPON_DIRECTOR.md`: the ten pointers end at `+1C8h`, and
the sub-kind-2 dword at `+1CCh` is the next field.

`00F87574`/`78`/`7C` lie past `.data`'s raw size, so the "default position" the empty-slot
reinitialiser copies is `(0,0,0)` at load unless something writes it. That is stated as a fact about
the image, not about the running game.

## The current command

`0071BE40 BSP_WeaponDirector_CurrentCommand`, `__fastcall(controller)`:

```
mode == 1  ->  *(controller+54h)    // queue head
mode == 2  ->  *(controller+188h)   // override
otherwise  ->  0
```

So "the current command pointer" is not one field: `+30h` selects between the head of the queue and
the override record. `00836920` reads both in the same pass, `[ESI+54h]` into `EAX` for the queue
arms and `[ESI+188h]` into `EBP` at `00836A93` for the override arms from `00836D67` on.

## Issuing a command

### `008358D0 BSP_WeaponDirector_SetCommand`, vtable `+60h`, `__thiscall(controller)(command, params)`

1. `0071E6C0 BSP_WeaponDirector_PushCommandSlot`; a false answer ends the routine with 0.
2. When `command->vtable[0Ch]()` (category) is 1 or 2 and the session mode at `[00E188A8]+1FE4h` is
   0 or 1, force the fire target through `00835860 BSP_WeaponDirector_SetFireTarget`.

### `0071E6C0 BSP_WeaponDirector_PushCommandSlot`

Refuses when `0071D780 BSP_WeaponDirector_CommandCount` is above 9. Scans slots 0..9 for the first
null command pointer. Refuses a command identical to the one in the preceding slot when that slot's
target matches. Refuses unless `0071D6D0 BSP_WeaponDirector_CommandAcceptsTarget` passes. Otherwise
writes `+54h + 1Ch*i` and assigns the parameter record through `0071DB50`, registers an observer on
the resolved target, and lifts `mode` from 0 to 1.

`0071D780 CommandCount` is not a plain occupancy count: a `moveonpath` slot whose path object holds
a non-empty point vector counts as `(end - begin) / 0Ch` (the waypoint count), so the "queue is
full" and "more than one command queued" tests are in waypoints, not slots.

### `00720CD0 BSP_WeaponDirector_IssueTargetCommand`, vtable `+58h`, `__thiscall(controller)(entity)`

`RET 4` at `00720D76`. Two steps:

1. The `00720CA0` loop inlined: index 9 down to 0, `00720850(index)` for every slot whose command
   pointer is non-null.
2. Build an `18h`-byte parameter record on the stack in the slot shape above
   (`hasTargetEntity = entity != 0`, `targetEntityId = entity->+174h`, entity pointer, the three
   floats from `00F87574`, trailing 0) and call `this->vtable[60h](00E08F60, &record)`.

`00E08F60` is the **`follow`** singleton (`docs/SCENE_COMMAND_TYPES.md` row 14, category 3,
requires a target), not an attack command; see the correction below.

### `0071E7F0` — set the override command, `__thiscall(controller)(command, params)`

Refuses when mode is already 2 with the same command and a matching target, and refuses unless
`CommandAcceptsTarget` passes. Refuses when the queue head is `attackmove` and the new target is a
different object, and when the queue head has category 1 or 2. Otherwise unregisters the observer on
the old override target, copies all eight parameter fields to `+18Ch`..`+1A3h`, stores the command
at `+188h`, registers the new observer, sets `mode = 2`, clears `+4Ch` and `+50h`, and calls
`this->vtable[6Ch](0)`.

## Beginning a command

### `0071F600` — the base begin, base vtable `+78h`, `__thiscall(controller)(useQueueHead)`

Picks `(+188h, +18Ch)` when the argument is 0 and `(+54h, +58h)` when it is non-zero, stores
`CommandAcceptsTarget`'s answer at `+4Ch` or `+44h`, and when the answer is true:

* Queue-head arm only, before the acceptance test: if the resolved target passes
  `target->vtable[5Ch](1Ch)`, the command's category is 1 or 2, and `target+54h` equals the session
  endpoint's `+54h`, the queue head is **rewritten to `moveto`** (`00E08F68`). Attacking something
  on your own side becomes a move order.
* `moveonpath` arm (`00E08F80`): builds the path through the slot-0 path object at `+1A4h`, with
  two branches on the parameter record's `hasTargetEntity` byte, then `FUN_0071F3B0`.
* Sends a `"start"` message: `BSP_NativeString_Assign("start")` then
  `FUN_00984300(sessionEndpoint, params, command, &string)`, followed by the pooled-block release
  pair `BSP_SizedStoragePool_GetSingleton` / `BSP_SizedStoragePool_ReturnBlock` and, when the target
  resolves, `FUN_00984800(sessionEndpoint, target)`. The queue-head arm gates the send on
  `DAT_00F8A0C4` being non-null.

### `00835C70 BSP_WeaponDirector_BeginCurrentCommand`, derived vtable `+78h`

Overrides `0071F600` and calls it. Its own work, in order:

1. Invalidate the path objects at `+1A4h` and `+1A8h` (`TRIV_body_0071BDB0`, byte `+18h` set to 1).
2. Return 1 early when the owning unit's `+184h` byte is set and the command is `cruise` or `stop`.
3. `FUN_0071F600(arg)`; a false answer returns 0.
4. With session mode at `[00E188A8]+1FE4h` not 2, one of four arms by command identity:
   * `follow` (`00E08F60`): when the target does not resolve, or the unit has no group
     (`unit+284h == 0`), or the target's group differs, the result becomes
     `BSP_Entity_ControllerBelongsToAnother()`.
   * `cruise` (`00E08F70`) or `stop` (`00E08F88`): `BSP_WeaponDirector_RaiseCommandStage(1)`; for
     `cruise` also `00835AC0` with `unit+984h`, `unit->vtable[50h]()` and `unit+980h`
     (`docs/CRUISE_COMMAND.md`).
   * `attackmove` (`00E08F78`): when `FUN_00521E70(0)` passes or the current fire target is null,
     and the current fire target differs from the resolved command target, set the fire target
     through `00835860`.
5. Stores the result at `+44h` (queue head) or `+4Ch` (override).

**Stage a begun command enters.** `cruise` and `stop` enter stage 1 explicitly at `00835DA0`/
`00835DA8` through `0071D810` (the `009E1170` stage ladder, `docs/UNIT_COMMANDED_SPEED.md`). The
other 24 classes are begun without a stage call, so they stay at the stage the previous clear left,
which `00720850` and `0071E7F0` leave at 0.

## The per-step update

`00836920 BSP_WeaponDirector_Step` is `docs/UNIT_COMMANDED_SPEED.md`'s contract; body
`00836920-00836EA7`. There is no command-object step method to call, because the command classes
have none, so the arms below are the whole per-command step. Read for this packet, not re-annotated:

| Site | Command | Rule |
| --- | --- | --- |
| `00836920` entry | (all) | `00835F60` first, then the stage bookkeeping |
| `00836A6C` | (all, generic arrival) | with more than one command queued, stage below 1, the head not category 1/2 and the last queued command category 1/2: raise stage 2 when the squared horizontal distance between the two world positions is below the **double** `4000000.0` at `00D09FE8` (`FLD double ptr`, `FCOMIP`, `JBE`), i.e. within 2000 units |
| `00836A8E` | `stop` | raise stage 2 when more than one command is queued, or `unit+184h` is set, or `COMISS` says the motion controller's speed `*(unit+73Ch)+28h` is **not** below the `0.0f` at `00D7A218` (`JC` skips the raise only for a negative or unordered speed) |
| `00836ADC` | `follow` | raise stage 2 unless the unit is in a group, the group's leader (`007788D0`) exists and is not the unit itself, the command's target is that leader, and fewer than two commands are queued |
| `00836B46` | `attackmove` | target gone: stage 2. Target passes `vtable[5Ch](41h)`: nothing. Target passes `vtable[5Ch](1Ch)` and is flagged at `+5Eh` or belongs to the session's own side: reissue `moveto` with `00465080(target,0)` through `0071ECF0`, then stage 2. Otherwise, a live visible unit failing `FUN_005457C0(target+54h)`: stage 2 |
| `00836BF1` | `moveonpath` | the path-follow arm; at `00836D0A` a true answer from `007ADD70` on the path cursor means arrival, and the arm calls `0071D810(2)` then sends `"finished"` (`00D09FD8`) with `FUN_00984300` and the `moveonpath` singleton |
| `00836D67`.. | override arms | dispatch on `EBP = [ESI+188h]`; `00836D69` compares `attackmove` |

`00835F60`, base vtable `+7Ch`, runs first: if the fire target at `+238h` fails
`vtable[5Ch](41h)`, and a chain of byte tests on the target holds, it routes a kind-`5Eh` message
(`00D02E98`) through `BSP_Session_RouteMessage(msg, 7, 0)` and drops the target.

## Completing or replacing a command

`00720850 InternalClearPrimaryCommand`, `__thiscall(controller)(index)`, body `00720850-00720C95`.
It is the only routine that removes a command. Callers: `00720CA0` (clear all), `00720CD0`
(vtable `+58h`) and `00721A40` (the `MT_GAMEUNIT_SETCMD` receive side, `docs/CRUISE_COMMAND.md`).

1. Trace `"---InternalClearPrimaryCommand: %s (%d), mode:%d, (%s, %d)"` with the command's
   `vtable[4]()` name, or the literal `"EmptyCommand"` when the slot is empty.
2. When `mode == 0`: 2 if the override at `+188h` is set, 1 if the queue head is set, otherwise
   return without touching anything.
3. **`index != 0`** (a queued command, not the current one): delete the slot's path object through
   its `vtable[0]( 1 )`, shift slots `index+1..9` down one place, reinitialise slot 9 (command 0,
   `hasTargetEntity`/`hasPosition`/id/entity 0, position = the `00F87574` default, trailing 0),
   shift the path pointers down, allocate a fresh `50h` path object into `+1C8h`, return.
4. **`index == 0`** (the current command completing or being replaced):
   * `FUN_0071E610` when `mode == 2` — clears the override.
   * `mode = 1`; count the occupied slots up to the first empty one.
   * Copy slot 0 (command and all eight parameter fields) into the previous-command record at
     `+16Ch`; unregister the observer on the resolved target.
   * **Fewer than two commands**: when a target resolves, refresh its pose if `target+0C8h` is 0 and
     snap slot 0's parameters to the target's world position (`target+0FCh/+100h/+104h`) with
     `hasPosition = 1`, `hasTargetEntity = 0`, id 0, entity 0, trailing 0. Then `slot0.command = 0`
     and `mode = 0`. The queue is now empty and the controller is idle.
   * **Two or more**: shift slots 1..count-1 down, clear slot `count-1`'s command and reinitialise
     its parameters to the defaults. The next queued command becomes the head.
   * When anything was occupied: delete path object 0, shift the path array down, allocate a fresh
     `50h` object into `+1C8h`.
   * `this->vtable[6Ch](1)` — `00835BF0` on the derived vtable (`docs/CRUISE_COMMAND.md`).

`0071E610` (clear the override) mirrors step 4's snap on the override record: `mode = 1`, snap
`+194h/+198h/+19Ch` to the target's world position with `hasPosition = 1` at `+18Dh`, clear the
entity fields, unregister the observer, `+188h = 0`, `this->vtable[6Ch](0)`.

`0071DEE0`, vtable `+20h`, `__thiscall(controller)(index)`, answers whether slot `index` still holds
an order worth executing: false for an empty slot and for `stop`, `cruise`, `Leave` and `disband`;
for `moveonpath` only when the slot's path object reports a non-empty point list
(`pathObject->vtable[8]()` then `FUN_0059CAC0 > 0`); true for every other class.

## Host table

One row per native call site projected in `src/command_execution.cpp`. `this`/args are the register
and stack shape at the site.

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `00720CEB` | `00720850` | `clear_command_slot` | `ECX` controller, `PUSH index` / ret void | slot's command pointer non-null |
| `00720D6E` | `vtable[60h]` = `008358D0` | `set_command` | `ECX` controller, `PUSH &record`, `PUSH 00E08F60` / ret bool | none |
| `008358D9` | `0071E6C0` | `push_command_slot` | `ECX` controller, command, params / ret bool | none |
| `008358F0` | `command->vtable[0Ch]` | `command_category` | `ECX` command / ret int | controller non-null |
| `0083591E` | `00835860` | `set_fire_target` | `ECX` controller, target, forced=1 / ret void | category 1 or 2, session mode 0 or 1, target resolves |
| `00835C8D`, `00835CA5` | `TRIV_body_0071BDB0` | `invalidate_path_object` | `ECX` controller, index / ret object | path pointer non-null |
| `00835CE8` | `0071F600` | `begin_command_base` | `ECX` controller, `PUSH useQueueHead` / ret bool | unit `+184h` gate passed |
| `00835DA0`, `00835DA8` | `0071D810` | `raise_command_stage` | `ECX` controller, `PUSH stage` / ret void | command is `cruise` or `stop` |
| `00835E19` | `00835AC0` | `latch_cruise_fields` | contract, `docs/CRUISE_COMMAND.md` | command is `cruise` |
| `00835D8x` | `BSP_Entity_ControllerBelongsToAnother` | `controller_belongs_to_another` | ret bool | command is `follow`, group test failed |
| `0071E6D2` | `0071D780` | `command_count` | `ECX` controller / ret int | none |
| `0071E75x` | `0071D6D0` | `command_accepts_target` | command, params / ret bool | a free slot exists |
| `0071E73C` | `vtable[14h]` = `00836040` | `normalize_self_target` | `ECX` controller, command, params / ret bool | acceptance passed |
| `0071F62x` | `0071D6D0` | `command_accepts_target` | command, params / ret bool | the chosen record's command is non-null |
| `0071F6Cx` | `FUN_00984300` | `send_command_message` | session endpoint, params, command, name / ret void | acceptance byte set |
| `00720880` | `command->vtable[4]` | `command_name` | `ECX` command / ret const char* | slot non-empty |
| `007208xx` | `pathObject->vtable[0]` | `destroy_path_object` | `ECX` path object, `PUSH 1` / ret void | pointer non-null |
| `00720Axx` | `FUN_0071FB90` | `create_path_object` | `ECX` new block, controller / ret object | `operator_new(50h)` succeeded |
| `00720C8x` | `vtable[6Ch]` = `00835BF0` | `on_command_changed` | `ECX` controller, `PUSH 1` / ret void | end of the index-0 arm |

`contract: unread` for this packet: `0071DB50` (parameter assignment), `00836040`, `0071FB90`,
`0059CAC0`, `007ADD70`, `00984300`, `00984800`, `0071F3B0`, `00521E70`, `005457C0`, `007788D0`.

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| `include/bsp/weapon_director.hpp:225` `kDirectorAttackCommandDescriptor = 0x00e08f60` | `00E08F60` is the **`follow`** singleton, category 3 (movement), `requires_target` true. `00720CD0` therefore issues a follow order on the entity, not an attack order. | `docs/SCENE_COMMAND_TYPES.md` row 14; the vtable at `00CFB518` has `[4] = 006F8830` returning `00CFB52C`, which is the inline literal `follow` in `.rdata`; `[0Ch] = 006F8840` is `MOV EAX,3; RET` |
| `docs/WEAPON_DIRECTOR.md`: `00720CD0` "builds a parameter block holding `(entity != 0)` as a u16" | the `u16` write at `00720D2A` is a zeroing of both `hasTargetEntity` and `hasPosition`; `00720D42` then writes only the byte at `+0h`, so `hasTargetEntity` is a byte and `+1h` stays 0 | `00720D2A: MOV word ptr [ESP+0xc],0x0`; `00720D42: MOV byte ptr [ESP+0xc],0x1`; the same two fields are copied separately by `00720850`'s shift loop |
| `docs/WEAPON_DIRECTOR.md`: `00720CD0` "calls `00720850(index)` for each non-empty slot" | correct, and `00720850` is `__thiscall` with `ECX` = the controller, not a free function of the index | `00720CE9: MOV ECX,EBP` before `00720CEB: CALL 0x00720850` |

## Open questions

* Who calls `vtable[78h]`. `00835C70` still has no direct caller in the image; `00D09FD0` and
  `00D09FD4` are its only references.
* The `50h` path object built by `0071FB90` and its `vtable[4]`/`[8]` point list.
* Whether `+18h` of the parameter record is ever non-zero; every producer read writes 0.

## Correction from docs/COMMAND_COMPLETION.md (packet cc2_command_completion)

- **Was:** 00984300, 00984800, 0071F3B0, 007788D0 and 005457C0 listed as unread callees of the command path
  **Is:** All five are read. 00984300 and 00984800 are the `command` and `target` mission-event dispatchers; 0071F3B0 is a refcounted-pointer reset that is on the path only because 0071F600 drops a path object with it; 007788D0 is the controlling-entity accessor; 005457C0 is the still-hostile test.
  **Evidence:** docs/COMMAND_COMPLETION.md sections 3 and 5; bodies 00984300-009847FD, 00984800-00984B93, 0071F3B0-0071F3D8, 007788D0-007788DE, 005457C0-005457D8.
