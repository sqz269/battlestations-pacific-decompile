# The unit state message `MT_SHIP_SYNC` and its apply side (packet `cc_unit_orders_apply`, part A)

Addresses: 00816C80, 00812FA0, 00813020, 00812D40, 0080D9B0, 0080DA00, 00813950, 00813DC0,
00813EF0, 0075A660, 0077C710, 00780670, 0076C600, 0076E520, 00768530, 00521E30, 0092F2E0,
00927F30, 0092BD70, 0077C2A0, and read-only 00813CA0, 00816B00, 0080DAD0, 0042AC60, 00825F20,
00822C20, 0081F980, 00762500, 004499C0.

Worker `agent/cc-unit-orders-apply`, 2026-09-11 UTC. Ghidra was read-only for this packet: no
renames, comments, prototypes, function creation or saves. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`; every live query verified both. Descriptive names are
hypotheses, not recovered symbols.

## Answer to the packet question

**The apply side of message `8Ch` is `00816C80`, the unit virtual at primary-vtable slot `18Ch`.
It is not the writer of `unit+980h`/`+984h`.** It routes the message's `+40h`/`+44h` pair into
the order ring at `unit+838h` through `00812FA0`, which writes ring slots only.

`unit+980h` and `unit+984h` are ring fields, not free-standing unit fields: the ring object
starts at `unit+838h`, so they are `ring+148h` and `ring+14Ch`. That is why every scan at
displacement `980h`/`984h` in `docs/UNIT_COMMAND_PRODUCERS.md` came back empty. Their writers are

| what writes it | store instruction | base |
| --- | --- | --- |
| per frame, rate limited | `0042AC9A`/`0042ACB0`/`0042ACB6` `MOVSS [ECX],XMM0` inside `BSP_Math_StepTowards` | ECX from `008130D1 LEA ESI,[EBX+148h]` and `00813104 LEA EDI,[EBX+14Ch]` in `00813020`, EBX = `unit+838h` |
| immediate throttle set | `0080D9E8 MOVSS [ECX+148h],XMM0` | ECX = `unit+838h` (`008235C4`, `0081FCDF`, `00820158`) |
| immediate rudder set | `0080DA3A MOVSS [ECX+14Ch],XMM0` | ECX = `unit+838h` (`0081FCDF` path, call at `0081FD6E`) |
| the same two routines inlined into the ship motion update | `00826708 MOVSS [EBX+148h],XMM0`, `0082674C MOVSS [EBX+14Ch],XMM0` | EBX = `unit+838h` (`00826116 LEA EBX,[EDI+528h]`, EDI = `unit+310h`) |
| construction | `00812F6E`, `00812F66` | EAX = `unit+838h` (`0081EDEF`) |

The per-frame path is the one that matters: `00813020` clamps the ring slot under the read
cursor against that slot's own bounds and calls `BSP_Math_StepTowards` on `ring+148h` and
`ring+14Ch` with `ring+154h * dt` and `ring+158h * dt`. The store is through a pointer the
callee holds, so no displacement scan could ever have found it.

## The receive pipeline

```
transport -> 0076E520  decode one packet, push the message object onto session+24Ch
             00768530  factory: read the 8-bit type, operator new, construct, vtable[2](stream)
          -> 0076C600  drain session+24Ch once per frame, per message:
             00780670  gate on game state 0Dh, tick window and message category
             00521E30  resolve the 16-bit id at msg+18h to the entity
          -> 0077C710  sequence filter, then entity->vtable[18Ch](msg)
          -> 00816C80  the apply
```

* `0076E520`, `__thiscall(session, peer)`. Calls `00768530`, sets `msg+18h` from `peer+174h`
  and `msg+14h` from `peer+14h`, then appends to the vector at `session+24Ch` (count `+250h`,
  capacity `+254h`, insertion pointer `+258h`).
* `00768530`, `__fastcall(stream)`. Reads an 8-bit type into a switch, `operator new` of the
  class size, calls the class constructor, then `piVar2->vtable[2](stream)`, the deserializer.
  For type `8Ch` the size is `4Ch` (`00769380 PUSH 4Ch`) and the constructor is `0075A630`
  (`00769394`).
* `0076C600`, `__fastcall(session)`. Pops messages in order, calls `00780670` with `EDX` = the
  message and a status out-parameter, and acts on the status: `0` keep and stop, `1` remove
  without destroying, `2` remove and call `vtable[0](1)`, the scalar deleting destructor. The
  drop path prints ` multi msg (%s) from tick (%d) is dropped in gamestate (%d) just because`
  using the message-name table at `00E0AB68` indexed by `msg+10h`.
* `00780670`, `__fastcall(unused ECX, EDX = message, [esp+4] = int* status)`, `RET 4`. Requires
  `game+5D4h == 0Dh`. Category `47h` transfers ownership to the list at `00F871A0`; category
  `61h` goes to `00805C60`; otherwise it compares the message tick `msg+0Ch` against
  `DAT_00F876B0` with a window of `3Ch` ticks, or `1770h` for category `49h`. For a message
  at or before the current tick it resolves the sender and takes `0077C710` when the message
  answers category `48h`, or `00780120` otherwise.
* `00521E30`, `__fastcall(uint16 id)`. Two-range handle table: below `DAT_00F89A10` it reads
  `DAT_00F89A54 + (id - DAT_00F89A0C)*10h + 0Ch`, otherwise `DAT_00F89AA8 + (id - DAT_00F89A60)*10h + 0Ch`.
* `0077C710`, `__thiscall(entity, message)`, `RET 4`. Walks the list at `entity+2A4h` for the
  node whose `+8` equals `msg+14h`, rejects the message when `msg+0Ch` is behind the node's
  stored 16-bit tick by less than `7531h`, stores the new tick and calls
  `entity->vtable[18Ch](msg)` at `0077C79B`. Gated on `entity+5Eh == 0`.

## The message class, type `8Ch` = `MT_SHIP_SYNC`

The name comes from the table at `00E0AB68`: index `8Ch` -> `00D02158` `"MT_SHIP_SYNC"`.
Neighbours are `8Ah` `MT_SHIP_CREATE`, `8Bh` `MT_MOTHERSHIP_CREATE`, `8Dh` `MT_SHIP_GUNS_SYNC`.

`4Ch` bytes, vtable `00D02D94` with five slots:

| slot | target | role |
| --- | --- | --- |
| `+00h` | `00762500` | scalar deleting destructor, sets vptr `00CE4974` |
| `+04h` | `00813DC0` | serialize into a bit stream |
| `+08h` | `00813EF0` | deserialize from a bit stream |
| `+0Ch` | `0075A660` | `bool __stdcall(int category)`, true for `8Ch`, `48h`, `46h` |
| `+10h` | `004499C0` | `MOV AL,1; RET`, shared by every message class |

The base class constructor `0075B430` sets vptr `00D02C68`, `+04h = 3`, `+08h`/`+0Ch = 0`,
`+10h` = the type byte, `+14h` = a player slot. `0075A630` then overwrites the vptr with
`00D02D94` and clears `+04h`, `+08h`, `+0Ch`, `+10h`, `+14h`, `+18h`, `+1Ah`.

Payload, from the builder `00813CA0` and the two stream routines:

| offset | meaning | built from | wire |
| --- | --- | --- | --- |
| `+0Ch` | send tick | `0076E520` / `00782790` | base |
| `+10h` | type byte `8Ch` | `0075B430` | base, 8 bits |
| `+14h` | entity/session id | `0076E520` from `peer+14h` | base |
| `+18h` | sender id | `0077C2A0` from `sender+174h` | base, 12 bits |
| `+20h` | heading | `unit->vtable[50h]()` | quantized, 16 bits, range `±3.14159265f` (`00D7A264`) |
| `+24h` | yaw rate | `0092D700(scratch)+4` | quantized, 10 bits, range `±1.5f` (`00CE380C`) |
| `+28h`/`+2Ch`/`+30h` | position X/Y/Z | `unit+6A4h..+6ACh` | X and Z only, 22 bits, range `±24000.0f` (`00D02F64`); Y is not sent |
| `+34h`/`+38h`/`+3Ch` | velocity X/Y/Z | `0092D6A0(scratch)` | X and Z only, 12 bits, range `±50.0f` (`00D09290`); Y is not sent |
| `+40h` | ordered throttle | `unit+980h` (`00813D4B`) | quantized, 10 bits, range `±2.0f` (`00CE3958`) |
| `+44h` | ordered rudder | `unit+984h` (`00813D54`) | quantized, 10 bits, range `±2.0f` |
| `+48h` | flooded flag | `unit+110Ch < 0.5f` (`00CE3800`) | 1 byte |
| `+49h` | collision group | `[unit+1018h]+64h` | 1 byte |
| `+4Ah` | class byte | `unit+118Ch` when the `unit+170h` class query returns 2, else 0 | 1 byte |

The `±2.0f` range on `+40h`/`+44h` is the same constant `00CE3958` that `00815440` uses as the
order record's upper bound, which is independent confirmation that the pair is the clamped
`[-2,+2]` order and not something else.

`00813DC0` and `00813EF0` are mirror images: both call the base half (`0075B480` / `0075B4C0`),
then walk `+28h`, `+30h`, `+20h`, `+24h`, `+34h`, `+3Ch`, `+40h`, `+44h` through the quantized
float helper (`004295C0` write, `004293F0` read, arguments `value/ref, 0, 1, bits, range`), then
`+4Ah`, `+48h`, `+49h` through the byte helper (`004290B0` write, `00428D70` read). Both are
`__thiscall(message, stream)` ending in `RET 4`. `00813EF0` adds `4` to the stream pointer before
the first field; `00813DC0` does not.

## The apply, `00816C80`

`void __thiscall(unit, message*)`, `RET 4`, body `00816C80..00816E23`. Reached only through
vtable slot `18Ch`; nine vtables carry it, always paired with `00816B00` in slot `188h`
(`00CF9238/23C`, `00CFA900/904`, `00CFB8C0/8C4`, `00CFC558/55C`, `00CFFBB8/BBC`, `00D017B8/7BC`,
`00D09800/804`, `00D0C108/10C`, `00D0C7D0/7D4`). Slot `188h` is the send side (`00816B00`
builds message `8Ch` through `00813CA0` and hands it to `0077C2A0`), slot `18Ch` the receive
side. Both fall inside the unresolved `00CFC3D0+188h..23Ch` span that
`docs/UNIT_INSTANCE_UPDATE.md` left open.

Steps, in native order:

1. `msg->vtable[0Ch](8Dh)`: for `MT_SHIP_GUNS_SYNC` the work goes to `00813950` and the routine
   returns. For `MT_SHIP_SYNC` the predicate `0075A660` answers false and execution continues.
2. `EBX = msg+0Ch - DAT_00F876B0`, the signed tick difference; `00780670` has already
   guaranteed it is `<= 0`. `EBX` is negated at `00816CE9`, so the age in ticks is
   `currentTick - msgTick >= 0`.
3. If `msg+49h` differs from `[unit+1018h]+64h`, call `0092BD70` with ECX = the controller at
   `unit+1018h`. That routine stores the byte at `controller+64h` and then rebuilds the physics
   collision filter through `00C31DC0`/`00C47F60`/`00C47F90`, selecting a mask from the unit
   class query `[[controller+1Ch]+538h]->vtable[1Ch]()`.
4. Gate `00816CCC`: `COMISS [unit+BCCh], 1.0f (00D7A24C); JA out`. Everything below happens only
   when `unit+BCCh <= 1.0f`. An unordered comparison falls through, so a NaN applies.
5. Dead reckoning. With `T = ageTicks * 0.05f` (`00D0DE84`, twenty ticks per second) it fills a
   `24h`-byte stack record and passes it to `0092F2E0` with ECX = `unit+1018h`:

   | record | value | evidence |
   | --- | --- | --- |
   | `+00h` | `msg+34h` velocity X | `00816D11`/`00816D2A` |
   | `+04h` | `msg+3Ch` velocity Z | `00816CE4`/`00816D5C` |
   | `+08h` | `msg+28h + msg+34h * T` | `00816D6A..00816D72` |
   | `+0Ch` | `msg+30h + msg+3Ch * T` | `00816D76..00816D7E` |
   | `+10h` | `BSP_Math_AddWrappedAngle(msg+20h, msg+24h * T)` | `00816D97`, stored `00816DA1` |
   | `+14h` | `msg+24h` yaw rate | `00816DB0` |
   | `+18h`, `+1Ch` | `0.0f` | `00816CF3`, `00816CF9` |
   | `+20h` | byte `1` | `00816DB6` |

   The stack offsets in the listing shift by 8 at `00816D50` and back at the `00816D97` call,
   which returns with `RET 8`; the table above is already in the caller's frame.
   `0092F2E0`, `__thiscall(controller, const record*)`, copies the eight floats to
   `controller+40h..+5Ch`, sets `controller+60h = 1` and tail-jumps to `0092E5B0`.
6. `00927F30(unit, 1)`, `bool __thiscall(unit, int role)`, `RET 4`: true when
   `unit[1ACh + role*4]` equals the active local-player slot `game+18ECh`, and only when that
   slot is `<= 7`. **When it is false** - that is, for a unit the local player is not flying -
   the order pair is pushed into the ring by `00812FA0` with ECX = `unit+838h`, arguments
   `(msg+40h, msg+44h, msg+4Ah, ageTicks)`.
7. If `msg+48h` is set, `unit+10D0h = 0.0f`.
8. `[unit+170h]->vtable[0]()`; when it returns `2`, `unit+118Ch = msg+4Ah`. This is the exact
   inverse of the builder's `00813CA0` tail.

`00813950`, the `MT_SHIP_GUNS_SYNC` half, `void __thiscall(unit, message)`: walks the turret
list at `unit+48h` (next pointer at `+44h`), and for each turret whose `vtable[5Ch](22h)` is
true it patches the next `44h`-byte sub-record of the message (first at `msg+24h`) with
`msg+20h`, `msg+21h` and `turret+4BCh`, sets the tick through `00782790(msg+0Ch)` and calls
`turret->vtable[18Ch](subRecord)`. It stops after `msg+22h` records.

## The order ring at `unit+838h`

One object, `168h` bytes, constructed by `00812D40` from the unit constructor `0081ED40`
(`0081EDEF LEA ECX,[ESI+838h]`). All offsets below are relative to `unit+838h`; the unit-relative
address is `838h` plus the offset.

| ring | unit | field | evidence |
| --- | --- | --- | --- |
| `+000h..+13Fh` | `+838h..+977h` | ten `20h`-byte slots | `CMP EAX,0Ah` at `00812FDE`, `0080D9CC`, `008266EC`, `0081315E` |
| `+140h` | `+978h` | read cursor | `00813020` reads it, `0080D9B0`/`0080DA00` start their fill there |
| `+144h` | `+97Ch` | write cursor | `0080DAD0` indexes with it; `00812D40` seeds it with `DAT_00E0B51C` |
| `+148h` | `+980h` | current throttle | `0080D9E8`, `00826708`, `00812F6E`, stepped in `00813020` |
| `+14Ch` | `+984h` | current rudder | `0080DA3A`, `0082674C`, `00812F66`, stepped in `00813020` |
| `+150h` | `+988h` | current kind byte | `00813134` |
| `+154h` | `+98Ch` | throttle slew per second, `8.0f` initially (`00CE3918`) | `00812F84`, used `008130C2` |
| `+158h` | `+990h` | rudder slew per second, `2.0f` initially (`00CE3958`) | `00812F8C`, used `008130F7` |
| `+15Ch` | `+994h` | confirmed throttle | `00813144`, and `0080DB3C` writes it from the slot |
| `+160h` | `+998h` | confirmed rudder | `00813152`, and `0080DB34` |
| `+164h` | `+99Ch` | confirmed kind | `0081314A`, and `0080DB45` |

One slot, `20h` bytes, exactly the record `00815440` builds:

| slot offset | field | evidence |
| --- | --- | --- |
| `+00h` | throttle | `00812FE1`, `0080D9CF` |
| `+04h` | rudder | `00812FE5`, `0080DA1F` |
| `+08h` | predicted flag: `1` = extrapolated copy, `0` = authoritative | `00812FED` clears it, `00813186` sets it on the copy, `00812D62` seeds `1` |
| `+0Ch` | throttle upper bound, `+2.0f` | `00812D6A`, read `0081304x` |
| `+10h` | throttle lower bound, `-2.0f` (`00CE7D7C`) | `00812D74` |
| `+14h` | rudder upper bound, `+2.0f` | `00812D65`, read `008130A1` |
| `+18h` | rudder lower bound, `-2.0f` | `00812D6F`, read `00813084` |
| `+1Ch` | kind byte | `00812FEA`, `0081312D` |

### `00812FA0`, the back-fill the apply uses

`void __thiscall(ring, float throttle, float rudder, uint8 kind, int ageTicks)`, `RET 10h`,
body `00812FA0..00813013`.

```
age   = min(ageTicks, DAT_00E0B51C)          ; 00812FAA/00812FB4, the global is 4
index = writeCursor - age; if (index < 0) index += 10
while (index != writeCursor) { write(index); index = (index + 1) % 10 }
write(writeCursor)
write(i): slot[i].throttle = throttle; slot[i].rudder = rudder;
          slot[i].kind = kind; slot[i].predicted = 0
```

It back-dates the remote order over the last `min(age, 4)` ticks of history and marks every
touched slot authoritative. It never touches `+148h`/`+14Ch`, the cursors or the bounds.
`DAT_00E0B51C` is written with `4` by the ring constructor itself (`00812F50`).

### `00813020`, the per-frame tick

`void __thiscall(ring, float dt)`, called only from `00825F20`. In order:

1. Read `slot[readCursor]`. Clamp its throttle into `[+10h, +0Ch]` and its rudder into
   `[+18h, +14h]`, with the native shape `if (low <= v) { if (high < v) v = high; } else v = low;`
   so an unordered comparison against the low bound yields the low bound.
2. `unit_step_towards(&ring+148h, clampedThrottle, ring+154h * dt)` and
   `unit_step_towards(&ring+14Ch, clampedRudder, ring+158h * dt)`. **This is the write to
   `unit+980h` and `unit+984h`.**
3. `ring+150h = slot[readCursor].kind`. If that slot is authoritative (`+08h == 0`), publish
   `ring+15Ch = ring+148h`, `ring+160h = ring+14Ch`, `ring+164h` = the kind, that is
   `unit+994h/+998h/+99Ch`.
4. Copy `slot[writeCursor]` forward into `slot[(writeCursor+1) % 10]` with `REP MOVSD` of eight
   dwords and set the copy's predicted flag to `1`.
5. Advance. When `game+1FE4h == 2` (a networked client) the read cursor advances by one modulo
   ten, so the ring keeps four ticks of lag between write and read; otherwise the read cursor
   jumps to the old write cursor, consuming immediately. The write cursor becomes
   `(writeCursor + 1) % 10` either way.

That explains the whole scheme. Locally, an order published by `0080DAD0` at the write cursor is
consumed on the next tick. Over the network, the write cursor runs four ticks ahead of the read
cursor, the ring extrapolates the last known order forward by copying it with the predicted flag
set, and a `MT_SHIP_SYNC` arriving late overwrites the predicted copies with the authoritative
value through `00812FA0`. The confirmed triple at `+994h..+99Ch` only ever advances on
authoritative slots.

### The immediate setters

`0080D9B0` and `0080DA00`, both `void __thiscall(ring, float)`, `RET 4`, bodies
`0080D9B0..0080D9F2` and `0080DA00..0080DA44`. Each fills the pending span from the read cursor
to the write cursor inclusive with one value, and then sets `+148h` (throttle) or `+14Ch`
(rudder) directly, bypassing the slew limit. Callers:

* `00822C20` at `008235D5`, the ship spawn path (`StartSpeed`, `ShipYardLaunch`): throttle set
  to a ratio whose denominator is `0080FC30(unit)`, the reference speed.
* `0081F980` at `0081FCEA`, `0081FD6E` and `00820161`, the scripted state load
  (`thrust`, `maxSpeed`, `gameUnit`, `navigatorParams`).
* `00825F20` inlines both at `008266CE..0082674C`, gated on the byte at `unit+61h`, sourcing
  `unit+FC4h` for throttle and `unit+FDCh` for rudder. In that function ECX is `unit+310h`
  (`00825F32 LEA ESI,[EDI-310h]` makes ESI the unit), so `EBX = EDI+528h` is `unit+838h`.

## Corrections

* `docs/UNIT_COMMAND_PRODUCERS.md` line 21-22 and the section "`+980h` and `+984h` are never
  written: the scan that proves it" record a negative result that is wrong in its conclusion.
  The scans themselves were correct: no instruction anywhere in `.text` names displacement
  `980h` or `984h` in a store. The fields are written, at displacement `148h`/`14Ch` from the
  ring at `unit+838h`, and per frame through a pointer held by `BSP_Math_StepTowards`. The
  doc's own remaining explanation - "a member function of an object embedded in the unit at an
  offset below `980h` writing at a small displacement" - is the correct one; the missing base
  is `+838h`, which the doc's list of secondary bases (`+010h`, `+024h`, `+170h`, `+1E4h`,
  `+310h`, `+38Ch`, `+72Ch`) did not include because `+838h` is an embedded member, not a base
  subobject.
* `docs/UNIT_COMMAND_PRODUCERS.md` line 224 says "The apply side of message `8Ch` was not
  found". It is `00816C80`.
* `docs/UNIT_COMMAND_PRODUCERS.md` line 82 says `unit+97Ch` "is reloaded from memory before each
  write and is **not** advanced". True of `0080DAD0` alone; `00813020` advances it every frame.
* `include/bsp/unit_orders.hpp` declares `UnitOrderQueue::kMaxSlots = 8` with the comment
  "capacity of this projection only, not a recovered native bound". The native bound is **ten**,
  proven by `CMP EAX,0Ah` at `00812FDE`, `0080D9CC`, `0080DA1C`, `008266EC` and `0081315E`.
  That header is owned by another packet and was not edited; `include/bsp/unit_state_message.hpp`
  carries the complete ring under the distinct name `UnitOrderRing`.
* `include/bsp/unit_orders.hpp` treats `unit+994h/+998h/+99Ch` as "the current order". They are
  the last *confirmed* order; the live pair the motion code reads is `+980h`/`+984h`. The same
  header maps `+998h` to `kUnitOffScriptYawControl` for aircraft. Both readings cannot hold for
  one class; the aircraft evidence (`0089DB05`) and the ship evidence (`00813152`) are each
  sound, so the offset is reused by two unit families and the name should be class-qualified.
* `docs/UNIT_INSTANCE_UPDATE.md` lists `00CFC3D0+188h..23Ch` as an unresolved span. Slots
  `188h` and `18Ch` are the ship sync send and apply pair.
* Ghidra's name `CG_vector_deleting_dtor_00816b00` is an automated classification and is wrong:
  `00816B00` is the ship sync send virtual. `FUN_00762500` genuinely is a scalar deleting
  destructor.

## Uncertainty

* `msg+2Ch` and `msg+38h` (the Y components) are filled by the builder and never transmitted.
  The apply never reads them, so their post-deserialize contents are whatever the constructor
  left, that is zero. Modelled as zero.
* `unit+BCCh`, the gate at step 4, was not traced to a producer. Its comparison against `1.0f`
  looks like a death or sink fraction but that is a guess.
* `0092E5B0`, the tail of `0092F2E0`, was not opened; the eight floats and the two flags are
  proven, what the controller does with them is not.
* `0075A660` answers true for `8Ch`, `48h` and `46h`. Only `48h` is exercised on this path.
  The other two categories were not chased.
* The record `00813950` hands to `turret->vtable[18Ch]` is `44h` bytes with three patched
  fields; its remaining layout was not recovered.
* `00780120`, the non-`48h` branch of the dispatcher, was not opened.

## Follow-up packets

* `unit_sync_vtable_tail` - addresses `00CFC3D0+188h..23Ch` as data, `008138A0`, `004F17E0`,
  and the turret apply reached from `00813950`; resolve the rest of the unresolved slot span
  now that `188h`/`18Ch` are known, and recover the `44h`-byte gun sub-record.
* `session_message_dispatch` - addresses `00780670`, `00780120`, `0077C2A0`, `00768530`,
  `0076C600`; recover the message categories `46h`, `47h`, `48h`, `49h`, `61h`, the full
  factory table against the `00E0AB68` name table, and the `0077C2A0` routing flags. Note that
  `0077C2A0` decompiles with `unaff_retaddr` and `unaff_EDI`: its real ABI is
  `__thiscall(session = ECX, message = [esp+4], ...)`, taken from `0077C2AB MOV EDI,ECX` and
  `0077C2D0 MOV EBP,[ESP+18h]`.
* `unit_motion_command_source` - addresses `00825F20`, `unit+FC4h`, `unit+FDCh`, `unit+61h`,
  `unit+BCCh`; find what feeds the pair the ship motion update pushes into the ring, which is
  the remaining half of "how a player order becomes motion".

## `no_ghidra_function`

Read from the disk listing; no functions were created.

| start | end (inclusive) | proposed name | evidence |
| --- | --- | --- | --- |
| 00813DC0 | 00813EE6 | `BSP_UnitStateMessage_Serialize` | `00813EE4 RET 4`; vtable `00D02D98` |
| 00813EF0 | 00813FFE | `BSP_UnitStateMessage_Deserialize` | `00813FFC RET 4`; vtable `00D02D9C` |
| 0075A660 | 0075A681 | `BSP_UnitStateMessage_IsCategory` | `0075A677`/`0075A67F RET 4`; vtable `00D02DA0` |
| 00762500 | 0076251E | `CG_scalar_deleting_dtor_00762500` | `0076251C RET 4`; vtable `00D02D94` |
| 004499C0 | 004499C2 | `BSP_SessionMessage_AlwaysTrue` | `MOV AL,1; RET`; slot `+10h` of every message vtable |

## State reached

| address | state |
| --- | --- |
| `00816C80` | analysed, reconstructed, build-tested |
| `00812FA0`, `00813020`, `00812D40`, `0080D9B0`, `0080DA00` | analysed, reconstructed, build-tested |
| `0077C710`, `00780670`, `0076C600`, `0076E520`, `00768530`, `00521E30` | analysed |
| `00813DC0`, `00813EF0`, `0075A660`, `00813950`, `0092F2E0`, `00927F30`, `0092BD70` | analysed |
| `00813CA0`, `00816B00`, `0080DAD0`, `00825F20`, `00822C20`, `0081F980`, `0077C2A0` | read only |
