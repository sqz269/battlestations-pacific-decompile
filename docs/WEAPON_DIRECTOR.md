# The weapon director: the unit command controller at unit+738h

Addresses: `00810F60`, `008366D0`, `008363E0`, `008362A0`, `0071BE80`, `0071BED0`, `0071BF20`,
`0071BF70`, `0071D560`, `0071D580`, `0071DA50`, `0071DAD0`, `0071DD30`, `0071DFD0`, `0071E050`,
`0071E0D0`, `0071E150`, `0071C1E0`, `00835640`, `00835940`, `008359C0`, `00835A40`, `00836210`,
`0071D5E0`, `008364E0`, `00835860`, `00835740`, `00720CD0`, `0089A8B0`, `0089C360`.
Vtables `00D09F58` (derived), `00D09EC0` (base), `00D09F40`/`00D09EA8` (secondary),
`00CFD9C4` (kind 5Ah message), `00D02E98` (kind 5Eh message), `00CF938C` (embedded target ref).

Every descriptive name here is a hypothesis, not a recovered symbol, except the six field names
in "Field layout", which come from literal strings the native property dump passes.

## What the object is

`docs/UNIT_WEAPON_DEVICES.md` establishes `unit->vtable[114h]` = `0080E150` =
`mov eax,[ecx+738h]; ret`. The producer of that field is `00810F60`, part of the unit's subobject
construction:

| Site | Step |
| --- | --- |
| `00810F80`-`00810F85` | `operator_new(250h)` |
| `00810F9D`-`00810FA0` | `ECX` = the block, `PUSH ESI` (the unit), `CALL 008366D0` |
| `00810FA9` | `MOV dword ptr [ESI + 0x738],EAX` |

So the object is **250h bytes**, its constructor is `008366D0`, `__thiscall(this)(unit)`, `RET 4`
at `00836789`, and the unit is its constructor argument.

It is not only a weapon director. `008362A0` is a property dump, `__thiscall(this)(sink)`, that
pushes each field to `sink->vtable[0Ch](0, name, typeId, value)` with a **literal name string**:

| Offset | Name string at `008362A0` | Type arg | Constructor default |
| --- | --- | --- | --- |
| `+240h` | `torpedoAvoidance` | 3 (bool) | 1 (`00836724`) |
| `+241h` | `shipCollisionAvoidance` | 3 (bool) | 1 (`0083672A`) |
| `+242h` | `landCollisionAvoidance` | 3 (bool) | 1 (`00836730`) |
| `+243h` | `cruiseIsHeading` | 3 (bool) | 0 (`00836736`) |
| `+244h` | `cruiseSteerOrHeading` | 2 (float) | 0.0f (`0083673D`, `MOVSS` of a zeroed XMM0) |
| `+248h` | `cruiseThrust` | 2 (float) | 0.0f (`00836745`) |

The object is therefore the unit's **command controller**: navigation policy, cruise autopilot,
a ten-slot command array, the fire/move permissions and the fire target all live in it. "Weapon
director" is the role the weapon bindings use it for, and the name the ledger keeps.

## Construction

`008366D0` (derived), `__thiscall(this)(unit)`, `RET 4`:

| Order | Site | Step |
| --- | --- | --- |
| 1 | `008366F9`-`00836700` | `arg = unit->vtable[60h](1)` |
| 2 | `00836705` | `CALL 008363E0` (base constructor), `ECX` = this, `RET 8` |
| 3 | `00836717` | `[this] = 00D09F58` (derived primary vtable) |
| 4 | `0083671D` | `[this+1Ch] = 00D09F40` (derived secondary vtable) |
| 5 | `00836724`-`00836745` | the six named fields above |
| 6 | `0083674D` | `[this+24Ch] = unit` |
| 7 | `00836753`-`0083676A` | `[this+38h] = new(40h)` built by `009F6A20(this)` |

Step 2 takes two stack arguments (`RET 8` at `00836491`) but only one `PUSH` follows the virtual
call: the `1` pushed at `008366F9` for `unit->vtable[60h]` is still on the stack and is reused as
the base constructor's second argument. Read from the assembly, not the pseudocode, which drops it.

`008363E0` (base) installs `00D09EC0`/`00D09EA8`, zeroes `+228h`, `+22Ch`, `+230h`, `+238h`
(`0083642F`), sets `+234h` = 1, installs the embedded target-reference vtable `00CF938C` at
`+224h`, stores its second argument in `+23Dh`, then asks the object at `+34h` two class
questions (`vtable[5Ch](0Bh)`, then `vtable[5Ch](9)`); when both are false it sets `+3Ch` = 1 and
`+3Dh` = 1. So a unit of neither of those two classes starts with fire and move both allowed.

## Field layout

| Offset | Meaning | Evidence |
| --- | --- | --- |
| `+00h` | primary vtable: `00D09EC0` base, `00D09F58` derived | `008363E0`, `00836717` |
| `+1Ch` | secondary vtable: `00D09EA8` base, `00D09F40` derived | `0083671D` |
| `+34h` | session endpoint; `ECX` for every `BSP_Session_RouteMessage` | `0071DAA2`, `0071E026` |
| `+38h` | 40h-byte subobject built by `009F6A20` | `00836777` |
| `+3Ch` | `allowFire` byte | written at `00836216`, defaulted at `008363E0` |
| `+3Dh` | `allowMove` byte | written at `0071D5E4`, defaulted at `008363E0` |
| `+54h`..`+16Ch` | ten command slots, stride `1Ch` | `00720CD0`'s clear loop |
| `+1CCh` | dword written by enable sub-kind 2 | `0071C1FB`-block of `0071C1E0` |
| `+220h` | artillery flag (sub-kind 3) | `0071C231` |
| `+221h` | sub-kind 4 flag | `0071C246` |
| `+222h` | torpedo flag (sub-kind 5) | `0071C25B` |
| `+223h` | sub-kind 6 flag | `0071C270` |
| `+224h` | embedded entity reference, vtable `00CF938C` | `008363E0` |
| `+238h` | **fire target** entity pointer, the reference's `+14h` | `008364E0` reads, `00836230` clears |
| `+23Ch` | byte that gates an unforced fire-target change | `00835860` |
| `+23Dh` | byte from the base constructor's second argument | `008363E0`, read at `008355F0` |
| `+240h`..`+248h` | the six named avoidance and cruise fields | `008362A0` |
| `+24Ch` | owning unit | `0083674D` |
| size | `250h` | `00810F80` |

## Vtable `00D09F58` (derived), read from `00D09F58`-`00D09FD7`

Thirty-two slots; the table ends where the string `finished` starts at `00D09FD8`. The slots this
packet needed:

| Slot | Target | Role |
| --- | --- | --- |
| `+00h` | `008367D0` | scalar deleting destructor |
| `+18h` | `00835840` | reads `+238h` (not read here) |
| `+1Ch` | `008355F0` | `+23Dh` gate, else tail-call `vtable[18h]` |
| `+24h` | `0071D560` | stance allows fire |
| `+28h` | `0071D580` | stance allows move |
| `+2Ch` | `008364E0` | get fire target |
| `+38h` | `00835640` | apply a kind 5Ah message (derived override) |
| `+3Ch` | `00835690` | double-dispatch thunk to `msg->vtable[8]` |
| `+40h` | `0071DA50` | send "allow fire" (sub-kind 0) |
| `+44h` | `0071DAD0` | send "allow move" (sub-kind 1) |
| `+58h` | `00720CD0` | clear the ten command slots and issue one command |
| `+5Ch` | `0071C150` | class test |
| `+64h` | `00836210` | store `allowFire` |
| `+68h` | `0071D5E0` | store `allowMove` |

## The stance rules, recovered from the bodies

`0071D560` and `0071D580` have no Ghidra function; both were decoded from raw bytes.

`0071D560`-`0071D57A` (inclusive end), `__thiscall(this unused)(int stance)`, `RET 4`:

```
0071d560 8b 44 24 04    MOV EAX,[ESP+4]
0071d564 83 f8 01       CMP EAX,1
0071d567 74 0a          JZ  0071d573
0071d569 83 f8 02       CMP EAX,2
0071d56c 74 05          JZ  0071d573
0071d56e 33 c0          XOR EAX,EAX
0071d570 c2 04 00       RET 4
0071d573 b8 01 00 00 00 MOV EAX,1
0071d578 c2 04 00       RET 4
```

`0071D580`-`0071D59A` is the same shape with `CMP EAX,3` first and `CMP EAX,2` second. So:

| Predicate | Body | True for |
| --- | --- | --- |
| `vtable[24h]` allows fire | `0071D560` | stance 1, stance 2 |
| `vtable[28h]` allows move | `0071D580` | stance 2, stance 3 |

That is exactly the table `scripts/global/commandhelpers.lua` annotates, so the script decoding
carried in `include/bsp/unit_weapons.hpp` as `contract: unread` is now confirmed by the bodies:
`STANCE_HOLD_FIRE` 0 holds both, `STANCE_FREE_FIRE`/`STANCE_GUARD` 1 frees fire only,
`STANCE_FREE_ATTACK` 2 frees both, `STANCE_MOVE_ONLY` 3 frees move only. The two neighbours
`0071D5A0` (`stance == 0`) and `0071D5B0` (constant 1) are further predicates of the same family,
unused by this path.

## Applying a stance is a networked command

`0071DA50` (`vtable[40h]`) and `0071DAD0` (`vtable[44h]`) write no field. Each is
`__thiscall(this)(bool)`, `RET 4`, and builds a session message on its own stack:

| Message offset | Value | Site in `0071DA50` |
| --- | --- | --- |
| `+00h` | `00CFD9C4` | `0071DA92` |
| `+04h` | 1 | `0071DA86` |
| `+18h`, `+1Ah`, `+1Ch` | 0 | `0071DA7D`, `0071DA82`, `0071DA8E` |
| `+20h` | sub-kind | `0071DA9A` |
| `+24h` | the boolean argument | `0071DA9E` |

built by `BSP_SessionMessage_ConstructBase(5Ah)` (`0075B430`) and routed by
`BSP_Session_RouteMessage(msg, 7, 0)` (`0077C2A0`) with `ECX = this->[34h]` (`0071DAA2`).
The permission therefore does not take effect locally at the call site; it comes back through the
session. `0071DFD0` and `0071E0D0` are the same routine with a different sub-kind, and their
`ECX = this->[34h]` at `0071E026` shows they are methods of this same object, not free functions.

### The sub-kind table

Every sender writes the sub-kind at message `+20h` and the boolean at `+24h`:

| Sub-kind | Sender | Receiver writes |
| --- | --- | --- |
| 0 | `0071DA50` (`vtable[40h]`) | `vtable[64h]` -> `+3Ch` allowFire |
| 1 | `0071DAD0` (`vtable[44h]`) | `vtable[68h]` -> `+3Dh` allowMove |
| 2 | `0071DD30` | `+1CCh` (dword, not a boolean) |
| 3 | `0071DFD0` artillery enable | `+220h` |
| 4 | `0071E050` | `+221h` |
| 5 | `0071E0D0` torpedo enable | `+222h` |
| 6 | `0071E150` | `+223h` |
| 7 | `00835940` | `+240h` `torpedoAvoidance` |
| 8 | `008359C0` | `+241h` `shipCollisionAvoidance` |
| 9 | `00835A40` | `+242h` `landCollisionAvoidance` |

### The receiver

`00835640` is the derived apply, `__thiscall(this)(msg)`, `RET 4`, decoded from raw bytes
(`00835640`-`0083568E`, no Ghidra function): sub-kinds 7, 8 and 9 store `msg[+24h] != 0` into
`+240h`, `+241h` and `+242h`; anything else tail-jumps to the base body at `0083568A`.

`0071C1E0` is the base apply, `__thiscall(this)(msg)`, and is the handler the packet asked for:

```
kind = msg[+20h]
  0 -> this->vtable[64h](msg[+24h])
  1 -> this->vtable[68h](msg[+24h])
  2 -> this[+1CCh]  = msg[+24h]
  3 -> this[+220h]  = msg[+24h] != 0
  4 -> this[+221h]  = msg[+24h] != 0
  5 -> this[+222h]  = msg[+24h] != 0
  6 -> this[+223h]  = msg[+24h] != 0
```

`00836210` (`vtable[64h]`) is more than a store, decoded from raw bytes
(`00836210`-`0083623A`, no Ghidra function): it writes the byte to `+3Ch`, and when the value is
**false** it releases the embedded target reference at `+224h` through `006952A0 `BSP_Observer_UnregisterPair`` and zeroes
`+238h` (`00836230`). Forbidding fire drops the fire target. `0071D5E0` (`vtable[68h]`) is the
plain two-instruction store `MOV AL,[ESP+4]; MOV [ECX+3Dh],AL; RET 4`.

Who calls `0071C1E0` from the session side is a **contract: unread**. Ghidra records no caller;
the entry points are the data slot at `00CFDA78`, the thunk at `00835610` and the derived
override's tail jump. The session dispatcher itself was not opened.

The message class is shared: its `vtable[0Ch]` at `0071C640` answers true for kinds `46h`, `49h`,
`59h` and `5Ah`, and `0075AF60` is its default constructor, reached from
`BSP_SessionMessage_CreateFromStream` (`00768530`) on the receiving side.

## The fire target

**Read.** `0089C360` `BSP_LuaBinding_GetFireTarget` calls `unit->vtable[114h]` then
`director->vtable[2Ch]`, and turns the result's `+174h` u16 into the Lua entity key.
`008364E0` is two instructions, `MOV EAX,[ECX+238h]; RET` (no Ghidra function,
`008364E0`-`008364E6`). The fire target is stored as a **raw entity pointer at `+238h`**, the
`+14h` field of the embedded reference object at `+224h`; it is not an id and not a handle.

**Write.** `0089A8B0` `BSP_LuaBinding_SetFireTarget` resolves argument 1 in three branches and
then makes one call:

| Branch | Test | Target value |
| --- | --- | --- |
| nil | `BSP_LuaObject_IsNil` | none resolved; the setter is reached with the unresolved pointer |
| entity | `008889C0` | `BSP_ObjectHandle_FromLuaTable` |
| vector3 | else | `operator_new(1E4h)` + `004E5980`, placed with `vtable[98h](0, world+19CCh, identity)` at the read position, `+C8h` = 1, `+10Ch` = 0, `+54h` = 2 |

then `unit->vtable[114h]()` and `00835860(target, 1)`. So there is one setter, not three; the
three branches are the three argument shapes, and the third manufactures a throwaway world
entity to aim at a map position.

`00835860`, `__thiscall(this)(target, force)`:

```
if (force != 0 || this[+23Ch] == 0 || this[+238h] == 0)
    BSP_Session_RouteMessage(build_target_message(target, force), 7, 0)
```

An unforced call is ignored while `+23Ch` is set and a target is already held. Nothing is written
locally here either: `00835740` builds a **kind 5Eh** message, `__thiscall(msg)(target, force)`,
`RET 8`:

| Message offset | Value | Site |
| --- | --- | --- |
| `+00h` | `00D02E98` | `00835782` |
| `+04h` | 1 | `0083576E` |
| `+18h`, `+1Ah`, `+1Ch` | 0 | `00835767`, `0083576B`, `00835775` |
| `+20h` | `target ? target[+174h] (u16 id) : 0` | `0083578A`, `00835797` |
| `+22h` | `target != 0 && target->vtable[5Ch](2)` | `008357A4`, `008357BA` |
| `+23h` | the `force` byte | `008357B7` |

The target crosses the session as an **entity id**, so the writer of `+238h` is on the receiving
side and is **contract: unread**; the only local write found by a full `.text` scan for
`mov [reg+238h], reg` inside `00700000`-`00860000` is the base constructor's zero at `0083642F`
and the clear at `00836230`.

## The per-step work

Not found as a tick. The nearest thing the director does with a target is `vtable[58h]` =
`00720CD0`, `__thiscall(this)(entity)`: it walks the ten command slots at `+54h` downwards with
stride `1Ch`, calls `00720850(index)` for each non-empty slot, then builds a parameter block
holding `(entity != 0)` as a u16, the entity's `+174h` id, and the three floats at `00F87574`,
and issues `this->vtable[60h](00E08F60, &block)`. Whether a periodic aiming step exists, and
where guns receive their aim angles, is **contract: unread** for this packet.

## Host table

One row per native call site in `src/weapon_director.cpp`. `this`/args are the register and stack
shape at the site.

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `00810F85` | `00BF681B` `operator new` | `allocate_director` | (250h); ret block | - |
| `00810FA0` | `008366D0` | `construct_director` | ECX = block; (unit); RET 4 | block non-null |
| `00836700` | unit `vtable[60h]` | `unit_session_endpoint` | ECX = unit; (1); ret endpoint | - |
| `00836705` | `008363E0` | `construct_base` | ECX = this; (endpoint, 1); RET 8 | - |
| `00836403` | `00720180` | `construct_command_array` | ECX = this; (endpoint) | - |
| `0083676A` | `009F6A20` | `construct_subobject_38h` | ECX = block; (this); ret subobject | block non-null |
| `0071BE8F` | `vtable[24h]` = `0071D560` | `stance_allows_fire` | ECX = this; (stance); ret bool | - |
| `0071BE9D` | `vtable[28h]` = `0071D580` | `stance_allows_move` | ECX = this; (stance); ret bool | - |
| `0071BEAF` | `vtable[40h]` = `0071DA50` | `send_permission` | ECX = this; (bool) | - |
| `0071BEBD` | `vtable[44h]` = `0071DAD0` | `send_permission` | ECX = this; (bool) | - |
| `0071DA71` | `0075B430` | `session_message_construct_base` | ECX = msg; (5Ah); RET 4 | - |
| `0071DAB1` | `0077C2A0` | `session_route_message` | ECX = this->[34h]; (msg, 7, 0) | - |
| `0071DFF1` | `0075B430` | `session_message_construct_base` | ECX = msg; (5Ah) | - |
| `0071E035` | `0077C2A0` | `session_route_message` | ECX = this->[34h]; (msg, 7, 0) | - |
| `0071C1FB` | `vtable[64h]` = `00836210` | `apply_fire_permission` | ECX = this; (bool) | sub-kind 0 |
| `0071C212` | `vtable[68h]` = `0071D5E0` | `apply_move_permission` | ECX = this; (bool) | sub-kind 1 |
| `0083622B` | `006952A0 `BSP_Observer_UnregisterPair`` | `release_target_reference` | ECX = current target; EDX = this+224h | allowFire false and target non-null |
| `0083568A` | `0071C1E0` (tail `JMP`) | `apply_base_message` | ECX = this; (msg) | sub-kind not 7, 8, 9 |
| `00835760` | `0075B430` | `session_message_construct_base` | ECX = msg; (5Eh) | - |
| `008357A4` | target `vtable[5Ch]` | `class_test` | ECX = target; (2); ret bool | target non-null |
| `00835860`-site | `00835740` | `build_target_message` | ECX = msg; (target, force); RET 8 | the `+23Ch` gate |
| `00835860`-site | `0077C2A0` | `session_route_message` | (msg, 7, 0) | the `+23Ch` gate |
| `0089C360`-site | unit `vtable[114h]` = `0080E150` | `unit_weapon_director` | ECX = unit; ret director | - |
| `0089C360`-site | `vtable[2Ch]` = `008364E0` | `fire_target` | ECX = this; ret entity | - |
| `00720CD0`-site | `00720850` | `clear_command_slot` | (index) | slot non-empty |
| `00720CD0`-site | `vtable[60h]` | `issue_command` | ECX = this; (00E08F60, block) | - |

## Coverage

| Routine | Coverage |
| --- | --- |
| `008366D0`, `00810F60` (director part only), `0071BE80`, `0071BED0`, `0071BF20`, `0071BF70` | complete |
| `0071D560`, `0071D580`, `0071D5E0`, `008364E0`, `00836210`, `00835640` | complete, decoded from raw bytes |
| `0071DA50`, `0071DAD0`, `0071DFD0`, `0071E0D0`, `00835740`, `00835860` | complete as senders |
| `0071C1E0` | complete as a body; its caller is unread |
| `008363E0` | partial: the field writes and the two class tests are read; `00720180` is unread |
| `008362A0` | complete as a name source; the sink `vtable[0Ch]` is unread |
| `00720CD0` | partial: the slot loop and the command issue are read; `00720850` and the command descriptor `00E08F60` are unread |
| `0089A8B0`, `0089C360` | partial: the three argument branches and the two virtual hops; the Lua tail is unread |

## Open questions

- Who writes `+238h`. The fire target arrives as an entity id in the kind 5Eh message and the
  local write was not found; the resolver lives on the receive side of the session.
- The sub-kind 2 dword at `+1CCh`, and what sub-kinds 4 and 6 mean next to the artillery and
  torpedo flags at `+220h` and `+222h`. The pairing 3/4 and 5/6 suggests an enable plus a second
  per-weapon flag, untested.
- The class ids `0Bh` and `9` the base constructor tests on the object at `+34h`, which decide
  whether a unit starts with fire and move allowed.
- Whether a periodic director step aims guns, and where the AI target-weight model of
  `docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md` feeds in.
- The `1E4h`-byte throwaway entity `0089A8B0` builds for a map-position target, and its `+54h` = 2.

## Correction from docs/DIRECTOR_TARGET_GATE.md (packet cc2_director_target_gate)

- The session-side caller of `0071C1E0` is `00721A40` (`BSP_WeaponDirector_ApplyGameUnitMessage`)
  at `00721A93`, as `director->vtable[38h](message)`, reached from `00780120`'s `IsA(59h)` arm
  (also recorded in docs/SESSION_MESSAGE_DISPATCH.md). The "contract: unread" above is closed.
- `00720850` has since been read by packet cc2_director_commands and is documented in
  docs/COMMAND_EXECUTION.md; the `00720CD0` coverage row above is stale on that point.
- `director+40h` is a float countdown in seconds: `-1.0f` at construction (`00720225` inside the
  base constructor `00720180`, which `008363E0` calls with `ECX` still the director), `-= dt` per
  frame while non-negative (`0071F314` in `0071F290`, controller vtable `+0Ch`), and `3.0f` when a
  `cleartarget` order arrives (`00817031`). `0071DF70` rejects a new target only while the hold is
  strictly above `0.0f`; the constructed `-1.0f` passes.

## Correction from docs/DIRECTOR_UPDATE_ARMS.md (packet cc2_director_update_arms)

- **Was:** +221h 'sub-kind 4 flag' and +223h 'sub-kind 6 flag', with an open question asking what sub-kinds 4 and 6 mean
  **Is:** +221h is aaEnabled and +223h is depthChargeEnabled; the four bytes are one enable per weapon category, not two pairs
  **Evidence:** 008360C0 hands its visitor the string pointers 00D09E00 ('aaEnabled') at 00836114 and 00D09DDC ('depthChargeEnabled') at 008361A1, beside 00D09E0C ('artilleryEnabled') and 00D09DF0 ('torpedoEnabled'). The four strings are literals in .rdata
- **Was:** +238h 'fire target entity pointer' and +23Ch 'byte that gates an unforced fire-target change', both unnamed
  **Is:** their names are fireTargetID (00D09DCC) and fireTargetIsPrimary (00D09DB8)
  **Evidence:** 008361C9 loads 00D09DCC and 008361FA pushes 00D09DB8 in the same visitor
- **Was:** open question: who writes +238h; a full .text scan for mov [reg+238h], reg found only the constructor's zero and 00836230's clear
  **Is:** 00836240 BSP_WeaponDirector_StoreFireTarget writes it, reached from 00721A40's 5Eh arm at 00721BF1. The scan missed it because the store uses a different displacement
  **Evidence:** 0083626D LEA ESI,[ECX+224h]; 00836287 MOV [ESI+14h],EDI, and 224h + 14h = 238h
- **Was:** open question: whether a periodic director step aims guns
  **Is:** yes. 0071F290 arm 7 calls [controller+38h]->vtable[4](frameDelta), and that slot is 009F5DA0, the bot fire-target think, then vtable[7Ch] = 00836920 BSP_WeaponDirector_Step
  **Evidence:** 009F6A45 installs vtable 00D21B48; 00D21B4C holds 009F5DA0; 00D09FD4 holds 00836920
- **Was:** the field table jumps from +1CCh to +220h
  **Is:** +1D0h..+21Fh is twenty dwords the base constructor fills with D01502F9h
  **Evidence:** 0072032E MOV EAX,0D01502F9h then twenty stores from 00720333 to 007203A5
