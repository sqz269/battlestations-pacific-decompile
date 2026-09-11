# Entity identity: the u16 id registries, the qualified-name lookup and the creator field writes

Addresses: 009517C0 009516D0 00951720 00951560 00951660 00927940 00F89A08 00F89A5C 00521E30
009251F0 00467F30 00438E10 004E9E40 004E9920 004E9F80 0048D6C0 00928630 009287B0 004E9520

Packet `cc2_entity_identity` (worker `agent/cc2-entity-identity`), Ghidra read-only. Every
descriptive name below is a hypothesis, not a recovered symbol. Three follow-ups are answered
here: `unit_entity_id_registry` (docs/UNIT_INSTANCE_LAYOUT.md), `entity_registry_find`
(docs/SCENE_DEFERRED_REFS.md) and `scene_entity_creator_fields` (docs/SCENE_ENTITY_CREATE.md).

Headline: the u16 at `entity+174h` is a **slot index into one of two fixed-size handle tables**,
not a counter. Each table owns a contiguous id range and a flat array of 16-byte slots; the
"allocator" only moves a slot between two intrusive lists. `009517C0`'s second argument is not a
flag: it is a **requested id**, and zero means "take the next free one". Release happens in the
entity destructor `009287B0`. The per-party lookup `009251F0` matches a **backslash-separated
path**, not a `party:name` pair.

## Part 1: the two registries at `00F89A08` and `00F89A5C`

### The pair

`00927940` constructs both in place: `00951660(this, 0, a)` then `00951660(this+54h, a, b)`
(`0092795F` pushes the literal `0`, `0092796A`..`0092796F` push `a` again as the second
registry's first id, `00927970 LEA ECX,[ESI+0x54]`). So registry **A** is at `00F89A08` with ids
`[0, a)` and registry **B** at `00F89A5C = 00F89A08+54h` with ids `[a, a+b)`: the ranges are
contiguous by construction, which is what lets every reader pick the table with a single
`id < A.count` test. `a` and `b` are the pair constructor's arguments and no caller of `00927940`
exists in the image (`xrefs 00927940` is empty), so the two capacities were **not read**.

Both registries are reset, not reconstructed, at mission boundaries: `BSP_Game_OnInit` calls
`00951560` on each (`004E3F22`/`004E3F27`, `004E3F2C`/`004E3F31`) and
`BSP_Game_LoadMissionScene` does the same (`004E01E3`..`004E01F2`).

### Registry layout (0x54 bytes)

| Offset | Size | Field | Evidence |
| --- | --- | --- | --- |
| `+0h` | 4 | vtable `00D19B88` | `00951660` `*param_1 = &PTR_CG_scalar_deleting_dtor_00951790_00d19b88` |
| `+4h` | 4 | first id of the range | `00951660` `param_1[1] = param_2`; `009517E0 SUB EDX,[ECX+4]`; `00521E3B` |
| `+8h` | 4 | id count (slots) | `00951660` `param_1[2] = param_3`; loop bound at `00951721`; `00521E33 CMP EAX,[00F89A10]` |
| `+0Ch` | 0x20 | free list (layout below) | `00951560` `+0Ch`/`+10h`/`+1Ch`/`+20h` stores |
| `+2Ch` | 0x20 | live list | `00951560` `+2Ch`/`+30h`/`+3Ch`/`+40h` stores |
| `+4Ch` | 4 | slot array, `operator_new(count*10h)` | `00951660` `param_1[0x13] = uVar1` |
| `+50h` | 4 | free slot count | `00951660` `param_1[0x14] = count-1`; `00951815 ADD [ECX+0x50],-1`; `00951773 ADD [ECX+0x50],1` |

### List layout (0x20 bytes, twice)

Each list is an **embedded terminator node** followed by the head pointer. A node's `prev` holds
the address of the link that points at it (the previous node's `next` field, or the head pointer
itself), which is why unlink is two stores with no branch.

| Offset in list | Field | Evidence |
| --- | --- | --- |
| `+0h` | terminator `next`, left 0 | `00951560` `*(this+0Ch) = 0`, `*(this+2Ch) = 0` |
| `+4h` | terminator `prev` = tail node | `00951560` `*(this+10h) = slots+10h`; pop reads it at `009517D4` |
| `+8h` | terminator id, unused | slot layout, never read for the terminator |
| `+0Ch` | terminator payload, unused | — |
| `+10h` | head node | `00951560` `*(this+1Ch) = &slot[count-1]`, `*(this+3Ch) = this+2Ch` (empty live list) |
| `+14h` | zeroed at reset, meaning unread | `00951560` `*(this+20h) = 0`, `*(this+40h) = 0` |

So the free list head is `+1Ch` and its tail is `+10h`; the live list head is `+3Ch` and its tail
is `+30h`. An empty list points its head at its own terminator and the terminator's `prev` at the
head slot (`00951560` `*(this+30h) = this+3Ch`, `*(this+3Ch) = this+2Ch`).

### Slot layout (0x10 bytes, `count` of them)

| Offset | Size | Field | Evidence |
| --- | --- | --- | --- |
| `+0h` | 4 | `next` | `009517F6`, `00951560` first loop |
| `+4h` | 4 | `prev` (address of the link pointing here) | `009517EA`, `00951560` second loop |
| `+8h` | 2 | the id, `first + index` | `00951560` `*(short*)(slot+8) = *(short*)(this+4) + i` |
| `+0Ch` | 4 | the entity pointer, 0 while free | `009517F2 MOV [EDX+0xC],ESI`; `009516D0` writes 0 |

`00951560` gives **slot 0 the id 0 and leaves it out of the free list** (the build loop starts at
index 1), and `00951720` skips index 0 as well (`00951727 CMP ESI,0x1 / JL`). Id `first` is
therefore reserved: in registry A that is the id 0 that `docs/LUA_BINDING_ENTITY_LOOKUP.md` and
`docs/UNIT_STATE_MESSAGE.md` treat as "no entity".

### The allocation rule `009517C0`

`__thiscall(registry, uint16 requestedId, void* entity) -> uint16`, `RET 8` (`0095181B`). The
second argument is a requested id, not a flag: `009517CF TEST AX,AX` tests only its low 16 bits,
and the non-zero arm at `009517DD`..`009517E6` computes `slots + (requested - first)*10h`, i.e. it
addresses the slot the caller named.

1. If `free == 0`, call `00951720` (`009517C6`) to rebuild the free list; `this` comes back in ECX.
2. `requestedId == 0`: take the **tail** of the free list, `node = *(this+10h)`, and return the
   id already stored in that slot (`009517D7 MOVZX EAX,word ptr [EDX+8]`).
3. `requestedId != 0`: take `slots + (requestedId - first)*10h` and return the requested id
   unchanged. No range check, no occupancy check.
4. `node.payload = entity` (`009517F2`), unlink the node from whichever list holds it
   (`009517F8`, `00951800`), push it at the head of the live list `+3Ch` (`00951802`..`00951813`),
   and `free -= 1` (`00951815`).

Ordering, not wraparound: ids are fixed to slots forever, so there is no monotonic counter and
nothing to wrap. A fresh registry hands out `first+1`, `first+2`, ... because `00951560` builds
the chain with the tail at `slot[1]`; released ids go back at the **head** (`009516D0`), so
recycling is FIFO and a recycled id is only reissued after every never-used slot is gone.

### The release rule `009516D0`

`__thiscall(registry, uint16 id)`, `RET 4`. `node = slots + (id - first)*10h`; `node.payload = 0`;
unlink; push at the head of the free list `+1Ch`; `free += 1`.

The only caller is the entity destructor `009287B0` at `00928810`, and it picks the registry with
the same two-range test every reader uses: `009287EB MOVZX EAX,word ptr [ESI+0x174]`,
`009287F5 CMP ECX,[00F89A10]` (registry A's count), `00928804 MOV ECX,0xF89A08` when below,
`0092880B MOV ECX,0xF89A5C` otherwise. So construction chooses a registry from a caller flag but
destruction chooses it from the id, and the two agree only because the ranges are contiguous.

### The free-list sweep `00951720` and its index inconsistency

`__fastcall(registry)`. For `i = count-1` down to `1`: if `slots[i].payload == 0`, unlink that
node and push it on the free list, `free += 1`. It is the exhaustion path of `009517C0`.

The two indices in the loop body do not agree. The occupancy test at `00951737` uses
`EDI = i*10h` (`00951731 SHL EDI,0x4`), the raw index, while the relink at `00951741`..`00951747`
uses `(i - [ECX+4])*10h`, the index biased by `first`. For registry A (`first = 0`) they are the
same slot. For registry B (`first = a`) the sweep tests slot `i` and relinks slot `i - a`, and for
`i < a` it walks *below* the array. Recorded as an original-code defect, provisional: it only
runs when a registry is exhausted, and no run-time evidence was collected for that path.

### Readers that resolve an id back to an entity

`00521E30 BSP_EntityHandleTable_Resolve` (`__fastcall(uint16)`) is the shared out-of-line form:
`id < [00F89A10]` selects `[00F89A54] + (id - [00F89A0C])*10h + 0Ch`, otherwise
`[00F89AA8] + (id - [00F89A60])*10h + 0Ch` - the slot payload, exactly the field `009517C0`
wrote. The same three loads are inlined at 24 further sites (`xrefs 00F89A10`), among them
`BSP_Entity_IssueCommand 0077D67D`, `BSP_Replication_ApplyPendingEntityCreates 0077EC55`,
`BSP_BotSideAi_RebuildGoalRequests 0091329D`, `FUN_0071DC80 0071DCDA` and `FUN_008E4680 008E4723`;
those bodies were **not read** by this packet.

### What writes `+174h`

Only `00928630 BSP_GameEntity_Construct` at `009286ED` (`MOV word ptr [ESI+0x174],AX`), from the
`009517C0` return. Its first stack argument selects the registry - `009286B8 CMP byte ptr
[ESP+0x28],BL` with `BL = 0` from `00928660 XOR EBX,EBX` (the only write to EBX before the site;
the listing was filtered for EBX) - so **zero selects registry B** and non-zero registry A. Its
second stack argument is the requested id, passed straight through to `009517C0`
(`009286CE`/`009286DA` load the same `[ESP+0x30]` on both arms). Scene entities take the zero
arm: `FUN_004E9520` pushes the caller's id then `XOR EBX,EBX / PUSH EBX`
(`004E953D`..`004E9542`), so a scene creator can carry a mission-authored id into registry B,
while `BSP_UnitOwnerEntity_Construct` passes the literal `1` at `0077EEF9` and lands in A.

## Part 2: the qualified-name match `009251F0`

`__stdcall(node, const char* name) -> node or null`, `RET 8` (`00925209`). Ghidra types it
`__fastcall` because the prologue stores ECX into the reserved local at `009251FB`; ECX is never
read afterwards, so it is a vestigial `this` and carries nothing. The node arrives at `[ESP+0xC]`
and the name at `[ESP+0x14]` (after the `PUSH ESI`).

Callers: `00925ACA` inside `BSP_EntityRegistry_FindEntityByName 00925A90` (the per-party entry,
already documented in docs/SCENE_DEFERRED_REFS.md) and its own recursion at `0092529E`. The
registry it walks is therefore the scene database's entity registry
`*(*(00E188A8) + 19CCh)`; `009251F0` itself only sees one node and its children.

Node contract used: the virtual at vtable `+10h` returns the node's name as `const char*`
(`009251F6`..`00925201`, and again at `00925214`, `0092524F`, `0092526F`), `+50h` is the child
count (`0092522F CMP dword ptr [EDI+0x50],0`) and `00467F30(node+48h, i)` returns child `i` by
walking the list at `node+48h` from its head, chained by `+44h` and bounded by the count at
`+50h` - the same list shape docs/SCENE_DEFERRED_REFS.md records for the party array.

The grammar is a **case-insensitive backslash path**, `component\component\...\leaf`:

1. If the node's name is null, fail (`00925203`).
2. `00438E10 BSP_CString_CompareInsensitive(nodeName, name)` on the **whole** remaining string; on
   equality return this node (`00925228 MOV EAX,EDI`). An unqualified name therefore matches at
   whatever level the caller started from.
3. Otherwise, if the node has children and `_strchr(name, '\\')` finds a separator
   (`00925237 PUSH 0x5C`, `0092523A CALL 00BF86F0`), let `n = sep - name`. Require
   `strlen(nodeName) == n` (the inline strlen at `00925254`..`0092525B`, compared at `00925263`)
   and `__strnicmp(name, nodeName, n) == 0` (`00925273`, `ADD ESP,0xC` at `00925278`, so three
   arguments). Only then recurse into every child with the remainder after the separator
   (`00925286 ADD EBX,1`, `00925290`..`0092529E`), returning the first non-null hit.
4. Anything else returns null.

So the first component names *this* node and the rest is matched against its subtree; a party name
is just the first component of such a path. There is no `:` or `@` form anywhere in the routine,
and the match is exact per component - no prefix or suffix matching, because the length equality
at `00925263` is checked before the compare. `_strchr` and `__strnicmp` are CRT contracts.

## Part 3: the `Cloud` creators, `004E9E40` and `004E9920`

Neither address is a Ghidra function: `004E9910 BSP_SceneShipyard_RegisterStock` ends at
`004E991F` and `FUN_004E9D40` (the `LandingPoint` creator, 0x224 bytes) ends at `004E9E35`. Both
Cloud creators were read from the raw listing, decoded from `ghidra bytes` with capstone
(`local/disasm_raw.py`), and are recorded in the report under `no_ghidra_function` with inclusive
ends: `004E9E40`..`004E9F7C` and `004E9920`..`004E99A8`.

Creator ABI, confirmed from the body and matching the correction in docs/SCENE_ENTITY_FACTORY.md
("`ECX` = the class id, `EDX` = the entity name, stack `parent`, `&frame block`, `properties`,
`0`"): the incoming ECX is **dead** - `004E9E46 MOV ECX,[ESP+0xC]` overwrites it with the third
stack argument, the property bag, before any use - and `EDX` is saved into EBX at `004E9E63` and
spent only as the name. `RET 0x10` at `004E9F67` confirms the four stack arguments; the fourth is
never read.

### `004E9E40`, the instantiate pass

| Site | Store or call | What it does | Evidence |
| --- | --- | --- | --- |
| `004E9E5E`/`004E9E65` | `008F2260(bag, "CloudType")` | the only property this creator reads; the literal is `00CE5FF4` | `bytes 00CE5FF4` = `CloudType` |
| `004E9E74`..`004E9E7F` | gate | property type tag `[prop+4h]` must be 4 or 2, else return null (`004E9F6A`, `XOR EAX,EAX`) | `CMP EAX,4 / CMP EAX,2` |
| `004E9E87` | `008F0DF0(prop)` | the type name as `const char*` | same pair as `007B37E4` in docs/SCENE_DEFERRED_REFS.md |
| `004E9E91`, `004E9EDA` | `0041E870(&temp, value)` | a temporary string per resolve | two SEH try indices, `[ESP+0x20] = 0` then `1` |
| `004E9EA2`, `004E9EEB` | `00479970(&temp)` | resolve the cloud class by name; on a miss it reads the Lua global table `CloudClass` (`BSP_LuaObject_GetByName(.., "CloudClass")` inside `00479970`) | the first result is discarded, the second is pushed |
| `004E9EFC` | `0047BBA0(manager, class)` with `manager = [[00E188A8]+21D0h]` | creates the cloud; `0047BBA0` is a one-line forwarder to `this->vtable[8]` | entity returned in EAX, `004E9F01 MOV ESI,EAX` |
| `004E9F4B` | `entity->vtable[98h](parent, registry, frameBlock)` | attaches the new entity; `registry = [[00E188A8]+19CCh]`, the same registry `009251F0` searches | pushes at `004E9F3C`/`004E9F47`/`004E9F48`, reverse order |
| `004E9F50` | `0048D6C0(entity, name)` | writes the entity name | body below |

`0048D6C0(entity, const char* name)` is the name store: `BSP_NativeString_Resize(entity+154h,
strlen(name), 0)` then `_memcpy(*(entity+158h), name, *(entity+154h))` when the buffer is
non-null. That is the `{length +154h, buffer +158h}` pair already declared as
`kUnitInstanceNameLengthOffset`/`kUnitInstanceNameBufferOffset` in
include/bsp/scene_unit_creators.hpp, reached here through a helper instead of inline.

**No store to `+174h`, `+354h` or `+538h` appears in either Cloud creator body**, and neither
writes a class pointer into the entity: the class object travels as the *argument* of the
manager's `vtable[8]`. The id is written only by `00928630` at `009286ED`, which the manager's
`vtable[8]` reaches through the entity constructor; that callee was **not read** (contract). The
`+354h`/`+538h` class-pointer fields named in the packet brief are not written on this path.

### `004E9920`, the registration pass

`__thiscall(bag)`, `RET 0` (`004E99A8`). Same property read (`CloudType` at `004E9938`), same type
gate, same temporary string, one `00479970` resolve at `004E996F` whose result is discarded, then
the temporary is released (`00419CC0` / `00BD1510`). It creates nothing: it forces the class to
load before the instantiate pass runs.

### Where the party is written: `004E9F80`, the `WaterMine` creator

Cloud has no `Party` property (docs/SCENE_ENTITY_FACTORY.md lists only `CloudType`), so the party
store is documented from the neighbouring typed creator, read complete
(`004E9F80`..`004EA072`, also `no_ghidra_function`).

| Site | Store or call | Evidence |
| --- | --- | --- |
| `004E9F9C`/`004E9FA5` | `008F2260(bag, "Type")`, value taken from `[prop+0Ch]` as a dword, not a string | literal `00CE4780` = `Type` |
| `004E9FB1` | `006EAFE0(&local, typeValue)` then `MOV ESI,[EAX]` - the type object | `004E9FB6` |
| `004E9FB8`/`004E9FC7` | `008F2260(bag, "Party")`, value at `[prop+0Ch]` | literal `00CE5804` = `Party` |
| `004E9FEB` | `type->vtable[20h](0, party, frame+30h, frame+20h, 0, 0, 0)` - seven pushes, party is the second argument | pushes `004E9FD5`..`004E9FE7` |
| `004E9FED` | the created entity is `result+0CCh` | `MOV ESI,[EAX+0xCC]` |
| `004EA025`..`004EA057` | the name store inline: `LEA EDI,[ESI+0x154]`, `0041DD40(edi, strlen, 0)`, then `_memcpy(*(edi+4), name, *edi)` | the same field pair `0048D6C0` writes |

So the party never reaches a field of the new entity in the creator: it is an argument to the type
object's spawn method, and the entity's own party field (if any) is written inside that callee,
which was **not read**.

## Host table (one row per native call site in the reconstruction)

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `004E9E65` | `008F2260` | `find_property` | this = property bag; `const char* key`; returns the record | none |
| `004E9E74` | - | `property_type_tag` | reads `[record+4h]` | record non-null |
| `004E9E87` | `008F0DF0` | `property_string_value` | this = record; returns `const char*` | tag 4 or 2 |
| `004E9EA2` | `00479970` | `acquire_entity_class` (first call, result discarded) | this = temp string; returns the class | tag gate passed |
| `004E9EEB` | `00479970` | `acquire_entity_class` (second call, result used) | as above | as above |
| `004E9EFC` | `0047BBA0` | `create_entity_from_class` | this = `[[00E188A8]+21D0h]`; the class; returns the entity | as above |
| `004E9F4B` | `entity->vtable[98h]` | `attach_entity_to_scene` | this = entity; `parent`, `[[00E188A8]+19CCh]`, `frameBlock`; void | entity non-null (unchecked natively) |
| `004E9F50` | `0048D6C0` | `set_entity_name` | this = entity; `const char* name`; void | none |
| `004E996F` | `00479970` | `acquire_entity_class` | this = temp string; result discarded | tag 4 or 2 |
| `00928810` | `009516D0` | (release, modelled as a pure rule) | this = registry chosen by `id < A.count`; the u16 id | always, in the destructor |

`0041E870`, `00419CC0`, `00BD1510`, `0041DD40`, `006EAFE0`, `_strchr`, `__strnicmp` and
`BSP_NativeString_Resize` are contracts: the temporary-string and CRT plumbing is not projected.

## Reconstruction

include/bsp/entity_identity.hpp, src/entity_identity.cpp. The registries are index-based structs
(the native `prev` pointer-to-link is a modelling detail, not a layout claim); allocation, release,
the sweep and the two-range resolve are pure rules over them; the qualified-name match is a pure
rule over an injected node interface; the Cloud creator is a sequence over
`SceneCloudCreatorHost`, one virtual per native call site. The sweep's native index inconsistency
is reproduced as a guarded skip rather than an out-of-range access, and is commented at the two
instruction addresses. Offsets are reused from include/bsp/unit_instance_layout.hpp and
include/bsp/scene_unit_creators.hpp; nothing is redeclared.

## Coverage

| Routine | Coverage |
| --- | --- |
| `009517C0` | complete, `009517C0`-`0095181D` |
| `009516D0` | complete, `009516D0`-`00951710` |
| `00951720` | complete, `00951720`-`00951786` |
| `00951560` | complete, `00951560`-`00951614` |
| `00951660` | complete, `00951660`-`009516A5` |
| `00927940` | complete, `00927940`-`00927994`; the capacities are its caller's arguments and no caller exists |
| `009251F0` | complete, `009251F0`-`009252B8`; the `vtable[10h]` name getter is a contract |
| `004E9E40` | complete, `004E9E40`-`004E9F7C` |
| `004E9920` | complete, `004E9920`-`004E99A8` |
| `004E9F80` | complete, `004E9F80`-`004EA072`; the `vtable[20h]` spawn callee is a contract |
| `0048D6C0` | complete |
| `00928630` | not re-read; only the `+174h` store and the registry/id argument plumbing, from the listing |

## Open questions

- The two registry capacities (`00927940`'s arguments) are unknown: the pair constructor has no
  caller in the image, so the id ranges are contiguous but unbounded here.
- `004E9E40` resolves the class name twice and throws the first result away. The register pass
  `004E9920` does the same single resolve, so the first call is probably the lazy-load and the
  second the fetch, but nothing in the two bodies proves it.
- The sweep's index inconsistency has no run-time evidence; `bsp_game.exe` would have to exhaust a
  registry to reach it.
- The `vtable[10h]` node-name getter was read only as a slot (`00CFC3D0+10h` is `0042E950`, which
  Ghidra has no function for), so the name it returns is inferred from its use, not from its body.
