# Session message routing and local delivery (packet `cc2_session_dispatch`)

Addresses: 0077C2A0 0076E520 00778450 0076C600 00780670 0077FE80 00780120 00779FF0 00779F90
0076FAD0 00816E30 00721A40 00778820 00836240 00720FE0 00764D00 0075B090 00835740 00821E80
0095ABE0, and read-only 00777850 00780090 0077C710 00768530 00782790 00770B50 00770AF0
0080E440 00814560 00805C60 0071C1E0.

Worker `agent/cc2-session-dispatch`, 2026-09-11 UTC. Ghidra was read-only for this packet: no
renames, comments, prototypes, function creation or saves. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Every descriptive name is a hypothesis, not a recovered
symbol. No run-time evidence: the `bsp_game.exe` harness does not load a mission, so nothing here
was observed on a frame (`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 6).

## Headline

`0077C2A0` is not a send. It is a **three-destination fan-out** whose inputs are the session mode
at `world+1FE4h`, a route-flag word, and the message's own class. In a local session the mode is
0, the flag word is **forced to 1** whatever the call site passed, and the only destination is
`0076E520`, which **serialises the message into a 400h-byte stack buffer and rebuilds a second
message from those bytes** before appending it to the array at `session+24Ch`. That array is
drained once per fixed step by `BSP_Session_PumpStep`, and the stamp `0076E5C6` gives the copy the
current tick, so a message routed earlier in a step is due when the drain reaches it **in that same
step**. There is no kind table: the receiver is chosen by an ordered ladder of `msg->vtable[0Ch]`
is-a tests followed by two byte-indexed switches on `msg+10h`.

`world+1FE4h` and `session+F4h` are the same field. `game+1EF0h` is the session
(`docs/FIXED_STEP_FANOUT.md` rows 9 and 13) and `1FE4h - 1EF0h = F4h`; `007784F8` reads it at
`session+F4h` and `0077C31D` at `world+1FE4h`.

## Session mode and the default flag word

`0076FAD0 BSP_Session_SetMode`, `__thiscall(session, mode)`, `RET 4`. `0076FB1A` returns when the
mode is unchanged, `0076FB30` saves the old one at `+F8h`, and `0076FE85` stores the new one at
`+F4h`. It is the **only writer** of `[00E0AF1C]` (three `WRITE` xrefs, all inside it).

| Mode | Written at | `[00E0AF1C]` | Meaning from the router |
| --- | --- | --- | --- |
| 0 | `0076FE26` (the `0076FD71` teardown arm) | 1 | no session; local only |
| 1 | `0076FBAF` (the `0076FB60 CMP EDI,1` arm) | 1 | host; `0077C3BE` owns the peer list |
| 2 | `0076FD2C` (the `0076FCD7 CMP EDI,EBP(2)` arm) | 2 | client; `0077C3A2` sends to the host |

## `0077C2A0 BSP_Session_RouteMessage`

`__thiscall(entity /*ECX*/, message, routeFlagsOverride, out)`, `RET 0Ch`, body
`0077C2A0..0077C469`. Coverage: complete.

Frame: `SUB ESP,0Ch` then `PUSH EDI/EBP/EBX/ESI`. `0077C2AD` spills `this` to `[ESP+4]` because
`0077C3D3 ADD EDI,0x2A4` clobbers `EDI` for the peer walk; `0077C436` restores it. With the
push depth accounted for, `EBP = [ESP+18h]` is the message, `[ESP+1Ch]` the flag override and
`[ESP+28h]` the out pointer. Ghidra renders the message as `unaff_retaddr`; the listing has it.

1. `0077C2A3..0077C2BE` return when `[00E188A8]` is null or `[world+5D4h] < 0Ah`.
2. `0077C2C4..0077C2FB` `if (this->vtable[5Ch](5) && (int)this[528h] >= 0 && this->vtable[5Ch](1Ch))
   message->vtable[0Ch](0D3h);` The result is discarded and `EAX` is overwritten at `0077C2FD`, so
   this is an is-a test whose value is not used. Left as an open question below.
3. `0077C302..0077C30A` `flags = override ? override : [00E0AF1C]` (`CMOVNZ`).
4. `0077C30D..0077C323` `if (flags == 4 && mode != 1) return;` The test is on the whole word, so
   5, 6 and 7 pass it.
5. `0077C32F` `if (mode == 0) flags = 1;` **This is the local-session rule.** A site that passes 7
   still gets local-only delivery; a site that passes exactly 4 has already returned at step 4.
6. `0077C337..0077C35C` `if ((flags & 2) && !(flags & 1))`: `00779FF0(message)` true, or mode 1,
   sets `flags |= 1`.
7. `0077C35E..0077C37C` else `if ((flags & 4) && mode == 2)`: `message[1Ah] = 1`, then
   `flags = 00779FF0(message) ? 3 : 2`. `NEG BL; SBB EBX,EBX; NEG EBX; ADD EBX,2` is the
   `predicate ? 3 : 2` idiom. A client cannot address the peers, so the peer bit becomes the host
   bit and the message is marked for relay.
8. `0077C37F..0077C398` `if ([00E18DB7] && message->vtable[0Ch](49h)) message[1Ch] = 1;`
9. `0077C39C..0077C3B3` `if (mode == 2 && (flags & 2)) 00779F90(this, message);`
10. `0077C3B8..0077C436` `if (mode == 1 && (flags & 4))` walk the `std::list` at `this+2A4h`
    (head `[this+2A8h]`, node `+0h` next, `+8h` peer): `message[18h] = this[174h]`, then
    `00770B50(session, [peer+50h], message)`. The `CMP EDI,EDI; JZ` at `0077C3E0` makes the
    `00BF6713` iterator checks dead code.
11. `0077C43A..0077C45A` `if (flags & 1) { 0076E520(game+1EF0h, message, this); if (out) *out = 0; }`

The addressee is the **sender itself**: `msg+18h` is always `this->+174h`, the entity id of the
routing entity, on both the peer path (`0077C40E`) and the local path (`0076E5B2`). A routed
message is "apply this to my counterpart", never "apply this to someone else".

### Routing decision table

`P` is `00779FF0`: the type byte is `81h`, `82h`, `A7h`, `A8h` or `A9h`, or
`msg->vtable[0Ch](4Ah)` answers true (`00779FF0..0077A01D`).

| mode | incoming flags | effective flags | local `0076E520` | host `00779F90` | peers `00770B50` |
| --- | --- | --- | --- | --- | --- |
| any | game state `< 0Ah` | - | no | no | no |
| `!= 1` | `== 4` | - | no | no | no |
| 0 | anything else | 1 | **yes** | no | no |
| 1 | 1 | 1 | yes | no | no |
| 1 | 2 | 3 | yes | no | no |
| 1 | 4, 5, 6, 7 | unchanged | bit 0 | no | **yes** |
| 2 | 1 | 1 | yes | no | no |
| 2 | 2 | `P ? 3 : 2` | only if `P` | **yes** | no |
| 2 | 4 (via 5, 6, 7) | `P ? 3 : 2`, relay marked | only if `P` | **yes** | no |

With the shipped defaults this means: **single player applies everything locally; a client applies
nothing locally and sends to the host; a host applies locally and broadcasts only when the call
site overrides the flags** (`7` from the weapon director, `4` from the health message of
`docs/UNIT_DAMAGE_AND_DEATH.md`). No run log was taken, so `[00E0AF1C]`'s value at any real frame is
a static reading of `0076FAD0`, not an observation.

## `0076E520 BSP_Session_EnqueueIncomingMessage`

`__thiscall(session, message, sender)`, MSVC EH frame, `SUB ESP,0x428`. Coverage: complete for the
copy and the append; the stream object's vtable `00D0385C` is `contract: unread`.

The local path is a **round trip through the wire format**, not a pointer hand-off:

1. `0076E53F` `EDI = message` (`[ESP+448h]`), `0076E5A9` `EDX = sender` (`[ESP+44Ch]`).
2. `0076E561` `EBP = 400h`, the capacity of the stack buffer at `[ESP+38h]`; two stream headers are
   built at `[ESP+28h]` and `[ESP+10h]`, the second carrying the vtable `00D0385C`.
3. `0076E575` `message->vtable[4](stream)`, the serialiser.
4. `0076E5A4` `00768530(stream)`, the factory: it reads the type byte back, `operator new`s the
   class and runs `vtable[2](stream)` (`docs/UNIT_STATE_MESSAGE.md`). `EBP` is the **new** message.
5. `0076E5B2` `copy[18h] = sender[174h]`; `0076E5C6` `00782790(copy, [00F876B0])`, the send tick;
   `0076E5CB` `copy[14h] = message[14h]`.
6. `0076E5D1` if `[session+258h] == 0`, `[00CE221C](&session+250h)` (an interlocked increment)
   returns the new count and the copy is stored at `[session+24Ch] + 4*count - 4`. Otherwise the
   reallocating path at `0076E5F5` grows `+254h` through `00BF55BE`.

Consequences: fields the serialiser does not write do not survive the loopback, and the caller's
message object is still the caller's to destroy. The queue holds `message*`; `+24Ch` is the base,
`+250h` the count, `+254h` the capacity and `+258h` a cursor that is non-null only while a drain is
in progress, which is what steers a re-entrant enqueue to the slow path.

### Correction to `docs/UNIT_STATE_MESSAGE.md`

| Was | Is | Evidence |
| --- | --- | --- |
| `0076E520`, `__thiscall(session, peer)`; sets `msg+14h` from `peer+14h` | `__thiscall(session, message, sender)`; `copy+14h` is copied from the **source message's** `+14h` | `0077C445 PUSH EDI; 0077C446 PUSH EBP` gives `(message, sender)`; `0076E53F MOV EDI,[ESP+448h]` is the message and `EDI` is not written again before `0076E5CB MOV EDX,[EDI+0x14]` |
| `0076C600` status `0` is "keep and stop" | keep and **continue**: `0076C721 MOV EBX,EBP` advances to the next entry without unlinking | `0076C678 JGE 0x0076C721` on the in-mission branch, then `0076C723 JMP 0x0076C6FA`, the loop tail |

## The drain, and whether a local order lands in the same step

`00778450 BSP_Session_PumpStep`, `__thiscall(session, step)`, fixed-step row 9. `007784F8` branches
on the mode:

* mode 0 -> `00778540 0076C600` and nothing else. `0076C600`'s **only caller** is this site.
* mode 1 or 2 -> `0076AA00(step)`, `007700E0`, `00777850`, `00776230`. `00777850` walks the same
  `+24Ch`/`+250Ch`/`+258h` array (`0077788D..007778BA`) and calls `00780670` itself, so the
  loopback queue is drained in every mode. `00777850` is `contract: partially read`: only its queue
  walk was followed, not its 1071-line category ladder.

`0076C600 BSP_Session_DrainMessageQueue`, `__fastcall(session)`. Coverage: complete. For each entry
it sets `[session+258h]` to the next slot, defaults the status to 2, calls `00780670(message,
&status)`, and then: status `!= 0` shifts the tail down and decrements `+250h`, destroying the
message through `vtable[0](1)` when the status is 2; status `0` leaves the entry in place and moves
on, except below game state `0Ah`, where a message with a non-zero send tick is logged with
` multi msg (%s) from tick (%d) is dropped in gamestate (%d) just because` (`0076C697`, name table
`00E0AB68` indexed by `msg+10h`) and consumed.

**Same-step answer.** The copy's tick is stamped from `[00F876B0]` at enqueue time, so its delta in
`00780670` is `0` and it passes the `EBP <= 0` branch at `00780767` immediately. A local order
therefore reaches its receiver **in the same fixed step if it was routed before row 9** of
`docs/FIXED_STEP_FANOUT.md` (the think pass at row 8 and everything earlier), and **in the next
step** if it was routed after it (rows 10 to 16, the render pass, or the variable-rate frame).
This is a static reading: no run log exercised the path.

## `00780670 BSP_Session_DispatchQueuedMessage`

`__fastcall(unused ECX, message /*EDX*/, int* status)`, `RET 4`. Coverage: complete. `ECX` is never
read before it is written at `00780681`, so `0076C600`'s `MOV CL,1` is not an input.

| Test | Site | Destination | Status |
| --- | --- | --- | --- |
| `game+5D4h != 0Dh` | `00780675` | none | `0` keep |
| `vtable[0Ch](47h)` | `0078069F` | spliced onto the list at `00F871A0` through `0077BAC0`/`0077DDA0`, for `0077EC20` at fixed-step row 10 | `1` release |
| `vtable[0Ch](61h)`, sender resolved | `007806EE`, `007806FC` | `00805C60(entity + 1E8h + 34h*msg[24h], msg[20h], 0)` | `2` |
| `vtable[0Ch](61h)`, sender missing | `0078072B` | held while `msg[0Ch] - tick > -258h`, then dropped | `0` / `2` |
| tick ahead by more than the window | `0078076B` | dropped | `2` |
| tick ahead within the window | `0078076F` | retried next step | `0` |
| entity missing | `007807C4` | held ten ticks (`CMP EBP,-0Ah`), then dropped | `0` / `2` |
| `entity+BDh == 0` | `00780789` | retried next step | `0` |
| `vtable[0Ch](48h)` | `00780797` | `0077C710(entity, msg)`, the per-peer sequence filter, then `entity->vtable[18Ch](msg)` | `2` |
| otherwise | `007807B6` | `00780120(entity, msg, status)` | set by the callee |

The window is `3Ch` ticks, or `1770h` when `vtable[0Ch](49h)` answers true (`00780751..00780760`).

## Kind to receiver

There is no table keyed by `msg+10h` at the session level. The type byte selects the message
**class**, and the class's `vtable[0Ch]` answers an is-a set; the dispatcher asks for categories in
a fixed order. `00E0AB68` is only a name table for logging (`0076C68F`, `docs/UNIT_STATE_MESSAGE.md`).

The is-a sets read here:

| Class | `vtable[0Ch]` | Answers true for |
| --- | --- | --- |
| `MT_COMMAND` `58h`, vtable `00D03630` | `00764D00` | `58h`, `49h`, `46h` |
| fire target `5Eh`, vtable `00D02E98` (built by `00835740`) | `0075B090` | `5Eh`, `59h`, `49h`, `46h` |
| director `5Ah` | `0071C640` (`docs/WEAPON_DIRECTOR.md`) | `5Ah`, `59h`, `49h`, `46h` |

`0075B090` has no Ghidra function; the bytes `8B 44 24 04 83 F8 5E 74 14 83 F8 59 74 0F 83 F8 49 74
0A 83 F8 46 74 05 33 C0 C2 04 00 B8 01 00 00 00 C2 04 00` decode as the four comparisons above.

`00780120`, `__thiscall(entity, message, int* status)`, `RET 8`, is the ladder. Coverage: partial -
complete for the ladder and the `4Eh` and `58h` arms; the `4Bh` player block
(`00780170..007803D9`) and the `4Fh`, `50h`, `51h`, `52h` and `97h` arms are `contract: unread`.
Its first act is the host relay: `00780147` `if (msg[1Ah] && mode == 1) 0077C7B0(msg, 0)`, which is
what the client's `0077C36B` marker is for.

| Order | Category | Site | Receiver |
| --- | --- | --- | --- |
| 1 | `4Bh` | `00780162` | player-slot block, `entity->vtable[144h]`/`[148h]`, `0059BBD0`, `entity+528h` |
| 2 | `4Ch` | `007803DF` | `entity->vtable[148h](msg[20h], msg[24h])`, then `0077FE80` |
| 3 | `57h` | `00780422` | `entity->vtable[13Ch]()`, result `+184h = 0` |
| 4 | **`4Eh`** | `00780457` | `entity+70h = msg[24h]`, then `entity->vtable[70h](msg[20h])` - the receive side of the destroy broadcast of `docs/UNIT_DAMAGE_AND_DEATH.md` step 6 |
| 5 | `4Fh` | `0078048D` | `entity->vtable[10h]()`, `00926D90` |
| 6 | `50h` | `007804DF` | `entity->vtable[158h]` |
| 7 | `51h` | `0078051E` | `0077F7B0` |
| 8 | `52h` | `00780548` | `00922F30` / `00922F80` |
| 9 | `97h` | `00780591` | `00778890`, `0070EFD0`, `entity->vtable[118h]`/`[11Ch]` |
| 10 | **`58h`** | `00780607` | `entity->vtable[160h](msg)` = `00816E30 BSP_UnitInstance_ApplyEntityCommand` |
| 11 | **`59h`** | `00780636` | `00778820(&ref, entity)` then `00721A40(&ref, msg)` |
| 12 | none | `007803FD` | `0077FE80(entity, msg, status)` |

`0077FE80`, `__thiscall(entity, message, int* status)`, `RET 8`. Coverage: complete for the gate and
the table; the five arms are named by their call targets only. `0077FE88` `message->vtable[10h]()`
false sets the status to `0` and keeps the message. Otherwise `kind - 53h` indexes the byte table at
`0078005C` into six targets at `00780044`: `53h` -> `0077FFAF`, `54h` -> `00780015`
(`entity->vtable[0Ch](&msg[20h], msg[28h])`), `76h` -> `0077FEC8` (`00521E30` then `0077F940`),
`77h` -> `0077FEE4` (`0077BD70`), `78h` -> `0077FF03`, and **everything else** ->
`0078002F entity->vtable[164h](msg)`. Its two callers are `00780120` and `00780090`
(`contract: unread`).

Slot `164h` is the generic kind switch. `00821E80 BSP_UnitInstance_HandleMessage` is the unit body
and `0095ABE0 BSP_Unit_HandleMessage` the base it falls back to (`0082237C` and the `4Bh` arm at
`00821EEB`). The two slots are fixed by the vtable images: `00821E80` and `00816C80` sit `28h`
apart in all six shared vtables (`00CF9214`/`00CF923C`, `00CFA8DC`/`00CFA904`, `00CFB89C`/`00CFB8C4`,
`00CFC534`/`00CFC55C`, `00D097DC`/`00D09804`, `00D0C7AC`/`00D0C7D4`), and
`docs/UNIT_STATE_MESSAGE.md` fixes `00816C80` at `18Ch`, so `00821E80` is `164h` and the dword
before it, `00816E30`, is `160h`.

`00821E80`'s switch: `kind - 4Bh`, range `0..55h`, byte table at `00822400`, 27 targets at
`00822394`. Kinds with a real target are `4Bh`, `4Ch`, `6Ah`-`70h`, `7Ah`-`7Ch`, `8Eh`-`96h`,
`98h`-`9Bh`, `9Dh`-`A0h`; every other index selects target `1Ah` = `0082237B`, the base call.
`99h` -> `00821FF0` -> `0080E440 BSP_UnitInstance_DetachPart(msg[20h])`, matching
`docs/UNIT_PARTS.md`. `9Ah` -> `00821FD6` -> `00814560`. `0095ABE0`'s own switch covers `4Bh`,
`4Ch`, `79h`, `7Dh`-`80h` and returns false for the rest (`0095AE06`).

### The five kinds of the packet

| Kind | Path | Receiver | Coverage |
| --- | --- | --- | --- |
| `58h` `MT_COMMAND` | `49h` window, not `48h` -> `00780120` -> `IsA(58h)` | `00816E30 BSP_UnitInstance_ApplyEntityCommand` | receiver head read; its 380-line body is `contract: partial` |
| `5Ah` director | `49h` window -> `00780120` -> `IsA(59h)` -> `00721A40` -> `IsA(5Ah)` | `director->vtable[38h]` = `0071C1E0`, the `00CFDA78` data slot of `docs/WEAPON_DIRECTOR.md` | complete for the route |
| `5Eh` fire target | same, then `IsA(5Eh)` at `00721BD8` | `00720FE0(msg)` resolves `msg[20h]` to an entity, `00836240(director, target, force)` writes `+238h`/`+23Ch` | complete for the route |
| `99h` detach | ladder falls through -> `0077FE80` -> default -> `vtable[164h]` | `00821FF0` -> `0080E440` | complete for the route |
| `4Eh` destroy | `00780120` -> `IsA(4Eh)` | `entity+70h = msg[24h]`; `entity->vtable[70h](msg[20h])` | complete |
| `9Ah` death | ladder falls through -> `0077FE80` -> default -> `vtable[164h]` | `00821FD6` -> `00814560` | route complete, `00814560` `contract: unread` |

`00721A40 BSP_WeaponDirector_ApplyGameUnitMessage`, `__thiscall(ref, message)`, `RET 4`. Coverage:
complete for the ladder, `contract: unread` for the arm bodies. `0077 8820
BSP_EntityWeaponDirectorRef_Construct` builds the 18h-byte stack ref first. The ladder tests
`5Ah`, `5Bh`, `5Ch`, `5Dh`, `5Eh`, `5Fh` and `60h` in that order, after requiring `ref[0]`,
`director+34h`, `unit->vtable[13Ch]()` non-null and that object's `+5Eh` zero.

`00836240`'s second argument is `msg+23h`, pushed at `00721BE6` **before** the call to `00720FE0`,
which is `__fastcall(msg)` ending in a bare `RET` at `0072100A`/`0072101E`/`00721021` and so pops
nothing. The target pointer is pushed on top of it, giving `00836240(director, target, force)` with
`RET 8` (`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 7).

`00816E30`'s head rebuilds the 18h-byte scene command descriptor that `0077D600` put on the wire:
`00816E8D` `+0h = msg[24h]` (word), `00816E7B` `+2h = msg[26h]` (word), `00816E92` `+4h = msg[28h]`,
and the four floats from `msg+2Ch..+38h`. The first float store at `00816E5E` writes
`[ESP+24h]` **before** `00816E69 PUSH EDI`, so it lands at `[ESP+28h]` in the post-push frame and
is not overwritten by the dword store; the descriptor is contiguous at `[ESP+20h]`. `0077A050`
turns the ordinal byte at `msg+20h` into the registered command object, compared against
`00E08F60`.

## Queue record layout

| Offset from `session` | Field | Evidence |
| --- | --- | --- |
| `+24Ch` | `message**`, the array base | `0076E5E6`, `0076C61A` |
| `+250h` | count, incremented through `[00CE221C]` | `0076E5D9`, `0076C604` |
| `+254h` | capacity, doubled plus 2 at `0076E61A` | `0076E601` |
| `+258h` | drain cursor; null outside a drain (`0076C714`) | `0076E5D1`, `0076C639` |

The queued object is a session message: `+0h` vtable, `+4h` class kind, `+0Ch` send tick, `+10h`
type byte, `+14h` player slot, `+18h` sender entity id, `+1Ah` relay marker, `+1Ch` audit marker
(`0075B430`, `docs/SESSION_TEARDOWN_LATCHES.md`, and this packet's `0077C36B`/`0077C398`).

## Host table

One row per native call site the reconstruction models.

| Step | Site | Callee | this | args | ret | Gate |
| --- | --- | --- | --- | --- | --- | --- |
| class test | `0077C2CC` | `vtable[5Ch]` | the entity | `5` | bool | always |
| class test | `0077C2E8` | `vtable[5Ch]` | the entity | `1Ch` | bool | kind 5 and `entity+528h >= 0` |
| discarded is-a | `0077C2FB` | `vtable[0Ch]` | the message | `0D3h` | ignored | both class tests true |
| privileged test | `0077C344` | `00779FF0` | - (`ECX` ignored) | the message | bool | `(flags & 2) && !(flags & 1)` |
| privileged test | `0077C36F` | `00779FF0` | - | the message | bool | `(flags & 4) && mode == 2` |
| relay mark | `0077C36B` | store | - | `msg+1Ah = 1` | - | same |
| audit test | `0077C392` | `vtable[0Ch]` | the message | `49h` | bool | `[00E18DB7] != 0` |
| send to host | `0077C3B3` | `00779F90` | the entity | the message | - | `mode == 2 && (flags & 2)` |
| send to peer | `0077C423` | `00770B50` | `game+1EF0h` | `[peer+50h]`, the message | - | `mode == 1 && (flags & 4)`, per node |
| local enqueue | `0077C44D` | `0076E520` | `game+1EF0h` | the message, the entity | - | `flags & 1` |
| clear out slot | `0077C45A` | store | - | `*out = 0` | - | `out != 0` |
| serialise | `0076E575` | `vtable[4]` | the message | the stream | - | always |
| rebuild | `0076E5A4` | `00768530` | - | the stream | the copy | always |
| stamp tick | `0076E5C6` | `00782790` | the copy | `[00F876B0]` | - | always |
| append | `0076E5E0` | `[00CE221C]` | - | `&session+250h` | the new count | `session+258h == 0` |
| grow | `0076E63D` | `00BF55BE` | - | bytes | the block | `session+258h != 0` and full |
| drain gate | `0076C65D` | `00780670` | - (`CL` ignored) | the message, `&status` | - | session and message non-null |
| drop log | `0076C69C` | `004254B0` | - | format `00D03868`, name, tick, state | - | state `< 0Ah` and `msg+0Ch > 0` |
| destroy | `0076C6F8` | `vtable[0]` | the message | `1` | - | status `2` |
| create defer | `007806CA` | `0077BAC0` | `00F871A0` | the list nodes, the message | node | `IsA(47h)` |
| sub-object | `00780718` | `00805C60` | `entity+1E8h+34h*msg[24h]` | `msg[20h]`, 0 | - | `IsA(61h)` and the entity exists |
| id resolve | `0078077E` | `00521E30` | - | `msg+18h` | entity | due message |
| sync deliver | `007807A4` | `0077C710` | the entity | the message | - | `IsA(48h)` |
| generic deliver | `007807B8` | `00780120` | the entity | the message, `&status` | - | otherwise |
| host relay | `00780158` | `0077C7B0` | the entity | the message, 0 | - | `msg+1Ah != 0 && mode == 1` |
| destroy apply | `00780473` | `vtable[70h]` | the entity | `msg[20h]` | - | `IsA(4Eh)` |
| command apply | `0078061C` | `vtable[160h]` | the entity | the message | - | `IsA(58h)` |
| director ref | `00780649` | `00778820` | the stack ref | the entity | - | `IsA(59h)` |
| director apply | `00780653` | `00721A40` | the stack ref | the message | - | `IsA(59h)` |
| session switch | `00780405` | `0077FE80` | the entity | the message, `&status` | bool | no ladder match |
| generic switch | `0078003B` | `vtable[164h]` | the entity | the message | bool | kind outside `53h..78h` |
| director slot | `00721A93` | `vtable[38h]` | the director | the message | - | `IsA(5Ah)` |
| fire target | `00721BF1` | `00836240` | the director | the target, `msg+23h` | - | `IsA(5Eh)` |

## Coverage

| Routine | Coverage |
| --- | --- |
| `0077C2A0` | complete, `0077C2A0..0077C469` |
| `0076E520` | complete for the copy and both append paths; the stream class `00D0385C` unread |
| `0076C600` | complete, `0076C600..0076C737` |
| `00780670` | complete, `00780670..007807D8` |
| `0077FE80` | complete for the gate, the table and the default; the five arms named by target only |
| `00780120` | partial: the ladder, the relay and the `4Eh`/`58h`/`59h` arms; `00780170..007803D9` and the `4Fh`-`97h` arms `contract: unread` |
| `00778450` | complete for the mode branch; the four networked callees unread |
| `00777850` | partial: the queue walk `0077788D..007778BA` only |
| `0076FAD0` | partial: the three `[00E0AF1C]` writes and the mode store; the per-mode teardown work unread |
| `00821E80`, `0095ABE0` | switch decoding complete; arm bodies unread except `99h` |
| `00721A40` | ladder complete; arm bodies unread except `5Eh` |
| `00816E30` | partial: the descriptor rebuild `00816E4B..00816EA6` only |
| `00836240`, `00720FE0` | argument shape only |

## Open questions

- `0077C2FB` calls `message->vtable[0Ch](0D3h)` and discards the result. No side effect is visible
  in the two is-a bodies read (`00764D00`, `0075B090`), so this is probably an inlined test whose
  body is empty in release. `0D3h` is outside every is-a set read here.
- `[00E0AF1C]` is never observed at run time. Everything about which destinations a shipped host
  uses rests on `0076FAD0`'s three stores.
- `00780090`, the second caller of `0077FE80`, was not opened, so the generic kind switch may have a
  second entry path.
- `00814560` (kind `9Ah`) and the `4Bh` player block of `00780120` were not read.

## Correction from docs/UNIT_MESSAGE_ARMS.md (packet cc2_unit_message_arms)

- **Was:** `0095ABE0`'s own switch covers `4Bh`, `4Ch`, `79h`, `7Dh`-`80h` and returns false for the rest (`0095AE06`).
  **Is:** the kind list is right, but 0095AE06 is not a false return: it calls 00878350(this, msg) with ECX = this and returns that callee's result. 00878350 answers 4Bh, 4Ch, 56h MT_DAMAGEDGFXLEVEL and D2h with 1, and only its own default returns 0. A caller of vtable[164h] therefore sees true for three more kinds than the two switches alone explain.
  **Evidence:** 0095AE06 PUSH EAX / 0095AE07 MOV ECX,ESI / 0095AE09 CALL 0x00878350, then the epilogue at 0095AE0E-0095AE1D with no MOV AL, so AL is the callee's. 00878350's body 00878350-0087839A is a four-case switch on byte [msg+10h] returning 1 for 4Bh/4Ch/56h/D2h and 0 by default. The pre-existing ledger record for 0095ABE0 already said "the default to the base handler 00878350", so the dispatch doc contradicted a record already in config/names/00950000.jsonl.
- **Was:** Kinds with a real target are `4Bh`, `4Ch`, ... ; every other index selects target `1Ah` = `0082237B`, the base call.
  **Is:** true as far as it goes, but three of the 27 targets do no unit work and the difference between them matters. Target 00h (4Bh, arm 00821EEA) calls the base and then forces AL to 1, discarding the base's answer. Target 01h (4Ch, arm 00821ED5) is the shared return-true epilogue and never reaches the base at all. Only target 1Ah (0082237B) returns the base's value. So 4Ch is swallowed by the unit rather than handled by it.
  **Evidence:** 00821EEA PUSH ESI / CALL 0095ABE0 / 00821EF1 MOV AL,1 versus 0082237B PUSH ESI / CALL 0095ABE0 / 00822381 MOV ECX,[ESP+20h] with no MOV AL. The byte table at 00822400 gives index 00h to offset 0 and index 01h to offset 1, and the target dwords at 00822394 are 00821EEA and 00821ED5.
- **Was:** `99h` -> `00821FF0` ... `9Ah` -> `00821FD6` -> `00814560`. ... the `4Bh` arm at `00821EEB`
  **Is:** the arm entry for 4Bh is 00821EEA; 00821EEB is the CALL instruction inside it. The 99h and 9Ah rows are correct.
  **Evidence:** target dword index 00h at 00822394 is 00821EEA, and the instruction at 00821EEA is PUSH ESI with the CALL at 00821EEB.
- **Was:** | `4Eh` destroy | `00780120` -> `IsA(4Eh)` | `entity+70h = msg[24h]`; `entity->vtable[70h](msg[20h])` | complete |
  **Is:** the route and the effect are right; the label is not. Kind 4Eh is MT_DEADMEAT and kind 4Fh is MT_DESTROY. Since the ladder's is-a ids and the message kind bytes coincide where both are known (the 58h test is MT_COMMAND), the arm read there is the MT_DEADMEAT one, not a destroy.
  **Evidence:** the name pointer table at 00E0AB68 gives index 4Eh the pointer 00D02710 -> "MT_DEADMEAT" and index 4Fh 00D02704 -> "MT_DESTROY". The same table's index 58h is 00D0267C -> "MT_COMMAND", which docs/ENTITY_ORDER_MESSAGE.md already fixed as the 58h message class, and index 8Ch is "MT_SHIP_SYNC", matching docs/UNIT_STATE_MESSAGE.md's kind 8Ch.
