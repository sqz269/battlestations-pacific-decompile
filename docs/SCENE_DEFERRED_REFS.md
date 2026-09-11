# Scene deferred references (`0046AAB0`): the queued entity commands

Addresses: 0046aab0 0046a9f0 00925a90 0077d600 00469610 004690d0 00468350 00467170
(read but owned elsewhere: 004e6b30, 0046d930, 0046b730, 007b3730, 00414db0, 00438e10, 00469010)

The `.scn` grammar lets a unit name another scene object in a property value (`R`, and the
`RPath`/`RFort` variants of the same reference type code). The name cannot be resolved while the
file is being read, because the object it names may not exist yet. `0046AAB0` is the pass that
resolves them after every entity of the instantiate pass has been constructed.

**The deferred reference is not a generic pointer patch.** Every record on the list is one
**entity command**: the `"Command"` sub-block's `Command` enum plus its `CommandTarget = R "<name>"`.
Nothing is written back into the owner's property bag; the resolved pair is handed to the order
dispatcher `0077D600`, which builds a session message. Naming this "deferred references" is the
`docs/SCENE_FILE_READER.md` hypothesis; the recovered mechanism is a command queue.

## The chain

| Stage | Address | Name (hypothesis) | ABI |
| --- | --- | --- | --- |
| produce | `004E6B30` | `BSP_SceneUnit_ApplyCommandProperty` (already named) | `__fastcall(instance, bag)`, RET 0 |
| queue | `00469610` | `BSP_SceneDatabase_QueueEntityCommand` (already named) | `__thiscall(entity, const char* command, const char* target)`, RET 0Ch |
| record ctor | `004690D0` | `BSP_SceneCommandRecord_Construct` | `__thiscall(record, void* entity, const char* command, const char* target)` |
| list push | `00468350` | `BSP_SceneCommandList_PushBack` | `__thiscall(listHeader, record*)` |
| resolve | `0046AAB0` | `BSP_SceneDatabase_ResolveDeferredReferences` | `__thiscall(void)`, effective RET 0 (tail `JMP`) |
| clear | `0046A9F0` | `BSP_SceneDatabase_ClearPendingReferences` | `__thiscall(void)`, RET 0 |

`0046AAB0`'s five callers are `0046DF00` (instantiate pass only), `0046B730`, `00944680`
`BSP_LuaBinding_Spawn`, `00944FD0` `BSP_LuaBinding_GenerateObject` and `00945450`: a script-spawned
entity drains the same queue the file reader filled.

## The record and the list

`00469610` is `LEA ECX,[ESI+0x14c]` at `0046965E` before `CALL 00468350`, with `ESI = ECX` at entry,
so the list header is **scene database +14Ch** and `ESI` is the scene database itself.

List header (3 dwords at `+14Ch`), from `00468350` (push) and `0046A9F0` (clear):

| Offset | Field | Evidence |
| --- | --- | --- |
| `+14Ch` | count | `*param_1 = *param_1 + 1` on push, `ADD dword ptr [EDI],-1` at `0046AA49` on erase |
| `+150h` | head | `param_1[1] = node` for the first push; `0046AAB3: MOV EAX,[ECX+0x150]` starts the walk |
| `+154h` | tail | `param_1[2] = node` on every push; `0046AA46` restores it when the last node is erased |

Node (12 bytes, `operator new(0xC)` at `00468350`):

| Offset | Field | Evidence |
| --- | --- | --- |
| `+0h` | prev | `*piVar1 = param_1[2]` (the old tail); `0046AA21..0046AA2D` unlink |
| `+4h` | next | `piVar1[1] = 0`; `0046AC14: MOV EAX,[EDX+0x4]` advances the resolve walk |
| `+8h` | record | `piVar1[2] = param_2`; `0046AAD4: MOV EAX,[EAX+0x8]` |

Record (`operator new(0x14)` at `00469627`, filled by `004690D0`, destroyed by `00469010`):

| Offset | Field | Producer evidence | Consumer evidence |
| --- | --- | --- | --- |
| `+0h` | owner entity | `*param_1 = param_2` | `0046AAD7: MOV EDI,[EAX]`; `EDI` is `this` for `00414DB0` and `0077D600` |
| `+4h` | command-name length | `BSP_NativeString_Resize(strlen(param_3),0)` | not read |
| `+8h` | command-name chars | `_memcpy(param_1[2], param_3, param_1[1])` | `0046AAD9`: `EBP`, the compare argument |
| `+0Ch` | target-name length | second `Resize`/`memcpy` pair | `0046AB2A: CMP [EAX+0xc],EBX` selects the branch |
| `+10h` | target-name chars | second pair | `0046AB2F: MOV EAX,[EAX+0x10]`, the `00925A90` argument |

`00469010` frees exactly two pooled string blocks, `(+4h,+8h)` and `(+0Ch,+10h)`, which is the
producer-side statement of the layout. Both name pointers fall back to the empty string `00E18560`
when null (`0046AAE2` and `0046AB36`).

## The global command registry (`00E19A70`)

`00E19A70` is the head of a second list, `{?, next@+4h, object@+8h}`, populated by the static
initialisers at `00CDC6A0..00CDC7E0`. Its elements are command types, not scene objects: the base
class's `vtable[4]` at `006F7F30` returns the literal **`"undefined command name"`**
(`00CFB38C`), and `vtable[8]` at `006F7F40` is `XOR AL,AL ; RET`. `vtable[0Ch]` is `__purecall`.

| vtable slot | Base body | Use here |
| --- | --- | --- |
| `+4h` | `006F7F30`, returns `"undefined command name"` | the name the record's command string is compared against |
| `+8h` | `006F7F40`, returns false | when true the command is skipped if no target was named (provisional: "requires a target") |
| `+0Ch` | `__purecall` | an id, compared to an int at `006F9110`; not used by this pass |

`00467170` is the same scan factored out (`BSP_CommandRegistry_FindByName`), so the walk in
`0046AAB0` is an inlined find-by-name over that registry.

## The resolution rule (`0046AAB0`)

`SUB ESP,0x20` then `PUSH EBX/EBP/ESI/EDI`; `this` is saved at `[ESP+14h]` and the walk cursor at
`[ESP+10h]`. Ghidra drops `ECX` at the `0077D600`, `00414DB0` and `00438E10` sites; the listing has
it. For each record:

1. `EDI = record->owner`, `EBP = record->command_name` (or `00E18560`).
2. Scan `00E19A70`. For each node, `ECX = node->object`, call `vtable[4]` for its name, then
   `00438E10(ECX = EBP, EDX = that name)`; zero means equal. **The compare is case-insensitive,
   and the shipped data needs it**: the corpus authors both `Follow` and `follow`, `Moveto`,
   `MoveTo` and `cruise`.
3. On a match `ESI = node->object` (`0046AB18`). A null object ends the record.
4. `CMP [record+0Ch],0` (the **target-name length**) selects the branch.
   - **Zero, position target.** `ESI->vtable[8]()` must return false, otherwise the record is
     dropped (`0046ABA6`). If the owner's pose byte `+C8h` is clear, `00414DB0` refreshes it
     (`ECX = EDI`); the descriptor then takes the owner's own world position `+FCh/+100h/+104h`,
     with `kind = 0`, `+1h = 1`, id `0` and object `0`.
   - **Non-zero, object target.** `00925A90(ECX = *(*(00E188A8)+19CCh), record->target_name)`
     returns the entity or null; null ends the record (`0046AB4F`). The descriptor takes
     `kind = 1`, `id = target+174h`, `object = target`, and the position from the read-only zero
     vector `00F87574..00F8757C` (all-zero bytes, no writer in the image).
5. `0077D600(ECX = owner, ESI /* the command object */, &descriptor, 1)`.
6. `EAX = node->next`; when the walk ends, `ECX = this+14Ch` and `JMP 0046A9F0` empties the queue.

Only the **first** matching registry entry is used; every exit from the inner scan is a `break` to
the next record. There is no error report on any failure path: an unknown command name, a null
registry object, a `vtable[8]` refusal and an unresolved target name all drop the record silently,
and `0046A9F0` then frees it with the rest.

The descriptor is the 0x18-byte stack block at `[ESP+18h]`, `LEA ECX,[ESP+1Ch]` after `PUSH 1`:

| Offset | Field | Position branch | Object branch |
| --- | --- | --- | --- |
| `+0h` | byte, target kind | `0` | `1` |
| `+1h` | byte (provisional: position valid) | `1` | `0` |
| `+2h` | uint16 object id | `0` | `[target+174h]` |
| `+4h` | object pointer | `0` | the `00925A90` result |
| `+8h/+Ch/+10h` | float3 position | owner `+FCh/+100h/+104h` | `00F87574/78/7C` (zero) |
| `+14h` | float | `0.0` (`XORPS XMM0` at `0046ABF8`) | same |

`0077D600` copies all seven fields into its own frame in the same order, which confirms the size
and the field boundaries.

## `0077D600`, what the resolved pair is used for

`__thiscall(entity /*ECX*/, void* command, const CommandTarget* target, int flags)`, `RET 0Ch`
(`0077D7EA`), 47 callers across the order-issuing code. Its body:

1. copies the descriptor into locals;
2. when `kind != 0` and the object pointer is null, resolves the pointer from the id through the
   two handle tables at `00F89A54`/`00F89AA8` (16-byte entries, pointer at `+0Ch`), keyed by
   `00F89A0C`/`00F89A60` bases and split at `00F89A10`;
3. asks the `00521EA0` singleton `vtable[5Ch](0Fh)`, and when it answers true replaces the target
   with `singleton+9D4h` and the zero vector, calling `command->vtable[4]()` for its name and
   `BSP_NativeString_Assign` on it;
4. calls `00A2BD90(command, descriptor)` when `entity+16Ch` is set and `entity+284h` is null or
   points back at the entity;
5. calls `007798D0(command, &descriptor, flags)`, which constructs a session message through
   `0075B430 BSP_SessionMessage_ConstructBase` with the tag `" Mv"`, and finally
   `BSP_Session_RouteMessage`.

So the resolved command is issued as a network-routed order, not written into the entity. The
reconstruction models `0077D600` as one host call; steps 2..5 are a contract, not reconstructed.

## `00925A90`, the entity-by-name lookup

`__thiscall(registry, const char* name)`, `RET 4` (`00925AE3`), returns the entity or null. It
walks `registry+8h`: `[+8h]` is a party array with the count at `[+8h]+8h` and the head at
`[+8h]+0h`, parties chained by `+44h`, and defers the per-party search to `009251F0` (which itself
uses `00438E10`, `_strchr` and `__strnicmp`, so the name may be qualified). `registry` is always
`*(*(00E188A8) + 19CCh)`.

Call sites read for the contract (rule 3):

| Site | Containing function | `this` | argument |
| --- | --- | --- | --- |
| `0046AB48` | `0046AAB0` | `[[00E188A8]+19CCh]` | `record+10h`, or `00E18560` when null |
| `0046BB41` | `0046B730` | same | `[EBP+58h]`, or `00E18560` when null |
| `0046DA55` | `0046D930 BSP_SceneDatabase_CreateEntityByName` | same | `[EBX+58h]`, gated by the length at `[EBX+54h]` |
| `007B37E4` | `007B3730` (entity class) | same | `008F2260` then `008F0DF0`: a string property value |

Not read: `0046DD31`, `0054FB1F`, `0054FCB7`, `0054FEE0`, `00550077`, `006ACF23`, `006ACF8C`,
`006ACFC3`, `006ACFFC`, `006D21CC`, `006D5C76`, `006D5D8A`, `006D5DB0`, `006D5FCF`, `007420EC`,
`008093AD`, `0084A074`, `0084A09C`, `0084A1E4`, `0091..0095137E`, `009271B0`, `007F4C85`,
`007F4CD6`. Every site read uses the identical `MOV ECX,[[00E188A8]+19CCh] ; PUSH name ; CALL`
idiom with the same empty-string fallback, so the contract is one argument and one `this`.

## What the shipped scenes actually queue

Read-only scan of `I:/SteamLibrary/steamapps/common/Battlestations Pacific`, 259 `.scn` files:

| Measure | Value |
| --- | --- |
| `CommandTarget = R "..."` properties | 13194 |
| of those, empty (`R ""`) | 12060 |
| of those, a named entity | 1134 |
| `Command = E CommandType : <name>` properties (the key that actually queues) | 5018 |
| `CommandTarget` within two lines after a `Command` key, empty | 3734 |
| the same, named | 1005 |

Command names, as authored (case preserved):

| Name | Count | Name | Count |
| --- | --- | --- | --- |
| `Cruise` | 3892 | `Stop` | 12 |
| `Follow` | 888 | `SetTarget` | 4 |
| `Moveto` | 95 | `Kamikaze` | 4 |
| `AttackMove` | 56 | `LevelBomb` | 3 |
| `follow` | 32 | `MoveTo` | 2 |
| `MoveOnPath` | 27 | `DiveBomb` | 2 |
| | | `cruise` | 1 |

A representative block, `usn_5_midway.scn:28252`:

```
"Command" {
  Command = E CommandType : Follow ;
  CommandTarget = R "Enterprise" ;
}
```

So roughly three quarters of the queued records take the position branch and one quarter names
another entity, and the thirteen authored spellings collapse to eleven command types only because
the compare is case-insensitive.

## Corrections to `docs/SCENE_FILE_READER.md`

- Its scene-database table lists `+14Ch` and `+150h` as two lists. They are one: `+14Ch` is the
  `{count, head, tail}` header and `+150h` is that header's head pointer. `0046AAB0` walks the head
  and then tail-calls `0046A9F0` on the header, so the pass it "clears" is its own.
- Its deferred-reference paragraph says the match takes "the referenced object's position
  (`+FCh/+100h/+104h`)". The position on that branch is the **owner's**, read through `EDI`, which
  `0046AAD7` loads from `record+0h`; the matched registry object is the command, and its own
  position is never read.
- `00925A90` is a lookup, not the registrar that queues a deferred reference. The registrar is
  `00469610`, reached only from `004E6B30`.

## Coverage

| Routine | Coverage |
| --- | --- |
| `0046AAB0` | complete: every instruction of `0046AAB0..0046AC34` |
| `0046A9F0` | complete (both fall-through gaps after `00BF65AC` read from the listing) |
| `00468350`, `004690D0`, `00469610` | complete |
| `00925A90` | complete; its callee `009251F0` is a contract |
| `0077D600` | partial: the call contract and the five body steps above; the handle tables, `00521EA0`, `00A2BD90`, `007798D0` and the message layout are not reconstructed |
| `00467170` | complete |
| `00414DB0`, `00438E10`, `00469010` | contracts, owned elsewhere |

## Follow-up

| Packet | Addresses | Contract |
| --- | --- | --- |
| `scene_command_types` | 006f7f30 006f9110 00cdc6a0 | The command-type registry: which derived class each static initialiser registers, its `vtable[0Ch]` id, and which of the eleven authored names exist as objects |
| `entity_order_message` | 0077d600 007798d0 0075b430 00a2bd90 | The `" Mv"` order message: the handle tables, the `00521EA0` gate and the wire layout |
| `entity_registry_find` | 009251f0 | The per-party name match, including the `_strchr`/`__strnicmp` qualified-name path |
