# The entity order message (`0077D600` -> `007798D0` -> `0077C2A0`)

Addresses: 0077D600 007798D0 00A2BD90 0075B430 0077C2A0 00521EA0 0071ECF0 00525250 008A2BC0
00465080 004F1830 0076E520 00770B50 00779FF0 006FE530 00E08EF8 00E08F7C 00E19A68

Packet `cc2_entity_orders`, part 1. Follow-up of `docs/SCENE_DEFERRED_REFS.md` ("`0077D600`, what
the resolved pair is used for"). Names below are hypotheses, not recovered symbols, except
`MT_COMMAND`, which is a literal in the image.

## Headline

`0077D600` does not write the order into the entity. It normalises the target descriptor, notifies
the owning AI group, and then builds one **`MT_COMMAND`** session message (type byte `58h`, name
string `00D0267C` through the message-type name table at `00E0AB68`) and hands it to the entity's
router `0077C2A0`. The router decides, from the session mode and a routing-flag word, whether the
message is delivered locally (`0076E520`), sent to each peer (`00770B50`), or both. The command
itself travels as **one byte**: the registration ordinal of the command-type object.

## Corrections to `docs/SCENE_DEFERRED_REFS.md`

| Was | Is | Evidence |
| --- | --- | --- |
| "asks the `00521EA0` singleton `vtable[5Ch](0Fh)`" | `00521EA0` is not a singleton getter. It is `__fastcall(SceneCommandTarget*)`, the descriptor's own id-to-pointer resolver, and it returns the **target object**. `vtable[5Ch]` is then called on that target. | `0077D6B9: LEA ECX,[ESP+0x18]` (the local descriptor) immediately before `CALL 0x00521EA0`; the body reads `+0h`, `+2h`, `+4h` and caches into `+4h`. It is the same resolution `0077D676..0077D6B3` inlines. |
| "replaces the target with `singleton+9D4h`" | with **`target+9D4h`**, read from the object `00521EA0` returned. | `0077D6ED: LEA ECX,[ESP+0x18] ; CALL 0x00521EA0 ; MOV EAX,[EAX+0x9d4]`. |
| "constructs a session message ... with the tag `\" Mv\"`" | There is no `" Mv"` tag. `00D03630` is the message vtable and its first slot is the pointer `00764D20`, whose little-endian bytes `20 4d 76 00` print as `" Mv"` in a hex dump. The message's identity is the type byte `58h` = `MT_COMMAND`. | `007798E1: MOV [ESI],0xd03630` sets the vptr; `007798D1: PUSH 0x58` is the type byte `0075B430` stores at `+10h`; `00E0AB68[0x58] -> 00D0267C = "MT_COMMAND"`. |
| `vtable[5Ch]` unnamed | `bool IsKindOf(int classId)`, `006FE530`, `RET 4`. | `docs/UNIT_INSTANCE_UPDATE.md` line 131 and its section `006FE530`. |

## `0077D600 BSP_Entity_IssueCommand`

`__thiscall(entity /*ECX*/, void* command, const SceneCommandTarget* target, int flags)`,
`RET 0Ch` at `0077D7EA`. Coverage: complete, `0077D600-0077D7EA`. 47 call sites.

Frame: MSVC EH record (`PUSH -1`, `PUSH 00C89EF8`, `FS:[0]` chain), `SUB ESP,0x5C`, then
`PUSH EBX/EBP/ESI/EDI`. In the post-`EDI` frame `EBP` = command (`[ESP+74h]`), `ESI` = the caller's
descriptor (`[ESP+7Ch]`), `EDI` = entity, `EBX` = 0; the **local descriptor copy** is at `[ESP+18h]`,
a temporary pooled string at `[ESP+10h]`, and the **message object** at `[ESP+30h]`. Ghidra drops
`ECX` at the `00521EA0`, `0041E870`, `00A2BD90`, `007798D0` and `0077C2A0` sites; the listing has it.

1. `0077D623..0077D66A` copy all seven descriptor fields into `[ESP+18h]` in descriptor order
   (`word +0h`, `word +2h`, `dword +4h`, four floats). The copy is what fixes the 0x18-byte size.
2. `0077D652/0077D670` `CMP byte [ESP+18h],BL ; JZ 0077D773`. A position target (`kind == 0`) skips
   the whole retarget block; only an object target can be normalised.
3. `0077D676..0077D6B3`, object pointer null: resolve it from the uint16 id through the two handle
   tables, then store it into the local copy. Still null ends the block.
4. `0077D6B9..0077D6CF` `target = 00521EA0(&local)` and `target->IsKindOf(0Fh)`. False ends the block.
5. `0077D6D5..0077D6E9` `0041E870(&localString, command->vtable[4]())` builds a pooled string of the
   command's name, which nothing then reads (see the open question).
6. `0077D6ED..0077D74B` **retarget**: `newTarget = *(void**)(00521EA0(&local) + 9D4h)`; the local
   descriptor becomes `object = newTarget`, `kind = (newTarget != 0)`, `id = newTarget ?
   *(uint16*)(newTarget + 174h) : 0`, `position = 00F87574/78/7C` (the read-only zero vector),
   `reserved = 0.0f` (`XORPS` at `0077D751`).
7. `0077D74B..0077D76E` free the temporary string when its char pointer is non-null.
8. `0077D773..0077D782` `if (*(int*)(command + 4h) == [00E08F7C]) 00521EA0(&local);` — resolve and
   cache the object pointer once more. `00E08F7C` is `00E08F78 + 4h`, the **ordinal field of the
   `attackmove` command object**, so the gate reads "this command is `attackmove`".
9. `0077D787..0077D7AC` `if (entity+16Ch != 0 && (entity+284h == 0 || *(void**)(entity+284h+14h) ==
   entity)) 00A2BD90(entity+16Ch, command, target)`. The descriptor passed here is `ESI`, the
   **caller's** descriptor, not the retargeted local copy.
10. `0077D7AC..0077D7BE` `007798D0(&message, command, &local, flags)`.
11. `0077D7C3..0077D7D3` `0077C2A0(entity, &message, 0, 0)`. The EH state word is set to 0 right
    before the call and no destructor runs after it.

### Handle resolution (`00521EA0`, inlined at `0077D676`)

`__fastcall(SceneCommandTarget* d)`, returns the object or 0.

```
if (d->kind == 0) return 0;
if (d->object == 0) {
    d->object = (d->id < [00F89A10])
        ? *(void**)([00F89A54] + (d->id - [00F89A0C]) * 0x10 + 0Ch)
        : *(void**)([00F89AA8] + (d->id - [00F89A60]) * 0x10 + 0Ch);
}
return d->object;
```

Two 16-byte-entry tables with the object pointer at `+0Ch`, split at the threshold `00F89A10`, each
biased by its own base index. The comparison is signed (`JGE` at `0077D683`).

## `007798D0`, the `MT_COMMAND` builder

`__thiscall(message /*ECX*/, void* command, const SceneCommandTarget* target, std::uint8_t flags)`,
`RET 0Ch` at `00779939`, returns `this` in `EAX`. Coverage: complete, `007798D0-00779939`.
It first calls `0075B430 BSP_SessionMessage_ConstructBase(this, 58h)`.

| Offset | Size | Value | Producer |
| --- | --- | --- | --- |
| `+0h` | 4 | vptr `00D02C68` then `00D03630` | `0075B44B`, overwritten at `007798E1` |
| `+4h` | 4 | `3` then `1` | `0075B436`, overwritten at `007798DA` |
| `+8h` | 4 | `0` | `0075B43D` |
| `+0Ch` | 4 | `0` (send tick, filled by the session) | `0075B444`; `docs/UNIT_STATE_MESSAGE.md` |
| `+10h` | 1 | `58h` = `MT_COMMAND` | `0075B451` from the `PUSH 0x58` at `007798D1` |
| `+14h` | 4 | local player slot: `[[00E188A8] + slot*4 + 18CCh]` when `0 <= slot <= 7`, else 0, `slot = [[00E188A8] + 18ECh]` | `0075B454..0075B47D` |
| `+18h` | 2 | `0`; the router writes `sender+174h` | `007798E9`; `0077C40E` |
| `+1Ah` | 1 | `0`; the router sets 1 on the client relay branch | `007798ED`; `0077C36B` |
| `+1Ch` | 1 | `0`; the router sets 1 when `[00E18DB7]` and `message->vtable[0Ch](49h)` | `007798F0`; `0077C398` |
| `+20h` | 1 | low byte of `command + 4h`, the command's **registration ordinal** | `0077992F/00779933` |
| `+21h` | 1 | the `flags` argument | `007798F3/007798F7` |
| `+24h` | 2 | descriptor `+0h`/`+1h` (`kind`, `position_valid`) as one word | `007798FE/00779901` |
| `+26h` | 2 | descriptor `+2h`, the target entity id | `00779905/00779909` |
| `+28h` | 4 | descriptor `+4h`, the resolved target pointer | `0077990D/00779914` |
| `+2Ch`/`+30h`/`+34h` | 4 each | descriptor `+8h`/`+0Ch`/`+10h`, the position | `00779917..00779926`, x87 `FLD/FSTP` |
| `+38h` | 4 | descriptor `+14h` | `00779929/0077992C` |

`+1Dh..+1Fh` and `+22h..+23h` are never written by the builder. The record ends at `+3Ch`; the
caller's stack block runs from `[ESP+30h]` to `[ESP+5Fh]`, so 0x30 bytes are reserved. The target
**pointer** at `+28h` is a local address and cannot survive the wire; which of these fields the
stream routines actually serialise was not read (`contract: unread`).

## `00A2BD90`, the AI-group notification

`__thiscall(aiGroup /*ECX*/, void* command, const SceneCommandTarget* target)`, seven instructions,
`RET 8` on the null path and a tail `JMP EAX` otherwise:

```
00A2BD90: MOV ECX,[ECX+0x564c] ; TEST ECX,ECX ; JZ 00A2BDA1
00A2BD9A: MOV EAX,[ECX] ; MOV EAX,[EAX+0x24] ; JMP EAX
00A2BDA1: RET 0x8
```

`aiGroup+564Ch` is `kAiGroupCommandOffset` of `docs/LUA_BINDING_AI.md`, the group's current command
object; when it is set the call forwards both stack arguments to its `vtable[24h]`. Ghidra's
"could not recover jumptable" warning at `00A2BD9F` is a tail call, not a switch. Two callers:
`0077D600` and `0071ECF0`, which reaches it only when its own subject
(`param_1[0Dh]->vtable[140h]()`) exists, answers `IsKindOf(2)`, has `+16Ch` set at `[+0x16c]`
(`piVar2[0x5b]`) and `007788B0` returns false. `0071ECF0` then calls its own `vtable[30h]`, builds a
message through `0071C830` and routes it through `0077C2A0` with the routing-flag override `7`,
which is the only site seen to override `[00E0AF1C]`.

## Where the message goes

`0077C2A0 BSP_Session_RouteMessage`, `__thiscall(entity /*ECX*/, message, routeFlagsOverride, out)`,
`RET 0Ch`. Read for the routing decision only; the rest is `contract: unread`.

- `0077C2A3` returns immediately when `[00E188A8]` is null or `[world+5D4h] < 0Ah`, i.e. outside the
  in-mission game states.
- `0077C2FD..0077C30A` `routeFlags = routeFlagsOverride ? routeFlagsOverride : [00E0AF1C]`.
  `0077D600` passes 0, so the order message always takes the global default.
- `[world+1FE4h]` is the session mode; 1 and 2 are the two branches the routine distinguishes.
- `routeFlags & 1` -> `0076E520([world+1EF0h], message, entity)`, the **local delivery**
  (`docs/UNIT_STATE_MESSAGE.md` documents `0076E520` as the session's decode-and-push entry).
- `routeFlags & 4` -> for each peer in the entity's list at `+2A4h/+2A8h`: `message+18h =
  entity+174h`, then `00770B50([world+1EF0h], peer+50h, message)`, the **network send**.
- `00779FF0` is a message-type predicate (`msg+10h` against `82h`, `81h`, `A7h`, `A8h`, `A9h`, ...),
  used to pick the branch, not to apply the order.

Whether a given build applies the order in the same frame therefore depends on `[00E0AF1C]`, which
this pass did not read at run time (`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 6: no run log was
taken, so no claim is made about the shipped value).

## Host table

One row per native call site inside `0077D600`, plus the two sites the builder owns.

| Step | Site | Callee | this | args | ret | Gate |
| --- | --- | --- | --- | --- | --- | --- |
| resolve target object | `0077D6BD` | `00521EA0` | - (`__fastcall`) | `ECX` = the local descriptor | target object or 0 | `kind != 0` and the inline resolve left a pointer |
| class query | `0077D6CB` | indirect `vtable[5Ch]` = `006FE530` | the target object | `0Fh` | bool | after the resolve |
| command name | `0077D6DD` | indirect `vtable[4]` on the command | the command | none | `const char*` | `IsKindOf(0Fh)` true |
| string construct | `0077D6E4` | `0041E870` | `[ESP+10h]`, a local | the name | - | same |
| resolve again | `0077D6ED` | `00521EA0` | - | `ECX` = the local descriptor | target object | same |
| string free | `0077D767` | `00419CC0` | - | chars, len+1, 1 | block | the char pointer is non-null |
| string free | `0077D76E` | `00BD1510` | the `00419CC0` result | none | - | same |
| attackmove re-resolve | `0077D782` | `00521EA0` | - | `ECX` = the local descriptor | ignored | `command+4h == [00E08F7C]` |
| AI group notify | `0077D7A7` | `00A2BD90` | `entity+16Ch` | command, the caller's descriptor | - | `entity+16Ch != 0` and (`entity+284h == 0` or `[entity+284h+14h] == entity`) |
| build message | `0077D7BE` | `007798D0` | `[ESP+30h]`, the message | command, the local descriptor, flags | the message | always |
| base construct | `007798D5` | `0075B430` | the message | `58h` | - | always |
| route | `0077D7D3` | `0077C2A0` | the entity | the message, 0, 0 | - | always |

## The 47 call sites

The `flags` argument is the **first** of the three pushes. Every site that passes a constant passes
`1`. Two helpers build the descriptor in place and are not part of the argument list:
`00465080(hidden return, entity)` has `RET 8`, `004F1830` is `__fastcall` with `RET 0`, so the
`PUSH 0x1` that precedes either of them is the flags argument, not theirs.

Read in full (argument shape and the command object established from the listing):

| Site | Containing function | Command | Target | Flags |
| --- | --- | --- | --- | --- |
| `0046AC0B` | `0046AAB0` scene deferred-reference queue | `ESI`, the matched registry object | `[ESP+1Ch]`, the built descriptor | `1` |
| `005252ED` | `00525250` player order screen | `EDX` from `00523130` | `[ESP+0Ch]`, built inline with `kind=1` | `playerUnit->IsKindOf(18h)` |
| `008A2D17` | `008A2BC0` Lua binding | `00E08F68` `moveto` | `00465080` result | `1` |
| `0077F8D7`, `0077F92F` | `0077F7B0` | `00E08F08` `clearorders`, `00E08F88` `stop` | register | `1` |

Read for the argument shape only (command object and flags from the listing, body not read):
`00465172`, `00535B7D`, `005F9AFA`, `005F9B96`, `005F9C1E`, `005F9CDA`, `005FABA1`, `0065D14C`,
`0065D1C7`, `0067A7CD`, `0067AAA2`, `006CCF91`, `006CD014`, `006CD04A`, `006CD0CD`, `006CD49D`,
`007F340F`, `007C3EF8`, `008454C9`, `008465F2`, `00894597`, `0089A850`, `008A2EC7`, `008A3077`,
`008A3273`, `008A42A7`, `008A4532`, `008A472A`, `008A4907`, `008A4AB7`, `008A4EAC`, `008A507F`,
`008A52B6`, `008A5467`, `008A7569`, `008A7729`, `008A78E9`, `008A7AA9`, `009FFF09`, `00A0216A`,
`00A120EB`, `00A14A6E`, `00A14EA7`.

`00A2F6F0` is listed as a caller by the index but no `CALL 0x0077d600` appears in its listing;
not resolved (`contract: unread`).

The `this` pointer comes from the global player unit `[00E188D8]` at six sites
(`docs/UNIT_INSTANCE_UPDATE.md`: the player-controlled unit instance), from `ESI`/`EDI` locals at
the rest, and from `[ESI+9D4h]` at `007C3EF8`, the same `+9D4h` field the retarget in step 6 reads.

## Open questions

- The pooled string built at `0077D6E4` from the command's name is freed at `0077D767` without
  being read. Either `0041E870` has a side effect this pass did not look for, or it is a leftover.
- Which `MT_COMMAND` fields the stream routines put on the wire, and in what quantisation, was not
  read; `docs/UNIT_STATE_MESSAGE.md` shows the pattern for `MT_UNITSTATE` (`8Ch`).
- `[00E0AF1C]`, the default routing flags, was not read at run time, so "the local session applies
  it immediately" is neither confirmed nor denied here.
- `entity+284h` is a back-pointer whose `+14h` is the owning entity. Its class was not established.
