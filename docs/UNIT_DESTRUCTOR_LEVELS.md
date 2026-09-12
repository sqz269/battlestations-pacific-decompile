# The unit's own destructor levels (packet `cc2_unit_destructor_levels`)

Addresses: 0081f3a0, 00822700, 006fe570, 00959940, 0095a860, 0087a410, 0087ac60, 0077e380,
0077e810, 00925f00, 009289e0, 00875490, 00874f00, 00440a30, 0081c940, 0074e7f0, 00779780,
004b5670, 004b5680, 00804300

Ghidra was **read-only** for this packet. Every descriptive name below is a hypothesis, not a
recovered symbol. This closes the five open questions `docs/ENTITY_LIFECYCLE_TAILS.md` left:
the levels between the concrete unit class and `GameEntity`, the Lua self table, the producer of
`player+8h`/`+9h`, the spatial index, and the reader of `entity+70h`.

## The headline

**The release is seven levels deep and the most derived class contributes nothing.**
`00CFC3D0[0h] = 006FE570` is `MDestroyer`'s scalar deleting destructor and it calls `0081F3A0`,
which is the **level-5** class body: `0081F3A0` rewrites the eight vptrs `0081ED40` installs
(`00D09678`/`00D0965C`/`00D09654`/`00D09650`/`00D09648`/`00D09630`/`00D0962C`/`00D09628`), not the
`00CFC3xx` set. `00822700`, slot 0 of the level-5 vtable `00D09678`, is byte-identical to
`006FE570` and calls the same body. So level 6 has no destructor of its own, which is the
destructor half of `docs/UNIT_INSTANCE_LAYOUT.md`'s result that level 5 owns all `0x1188` bytes.

**Three unit-owned pointers are never released by any level**: the weapon director at `+738h`
(`docs/WEAPON_DIRECTOR.md`), the gunnery pass at `+6DCh` (`docs/UNIT_GUNNERY_PASS.md`,
`operator_new(558h)`) and the `1ACh` damage-state instance at `+360h` (`docs/UNIT_PARTS.md`).
The strings `0x738`, `0x6dc`, `0x744` and `0x360` do not occur in any of the seven bodies.
Whether something outside the release owns them is `contract: unread`; through these fields the
release frees nothing.

**`0081F3A0` is not a vector deleting destructor.** It is `__thiscall(this)` with a plain `RET`
and no count argument. The same holds for `00959940` and `0077E380`; all three carried a
`CG_vector_deleting_dtor_*` heuristic tag, which this packet replaces.

## The chain

The constructors are `00925CE0` -> `00928630` -> `0077EED0` -> `0087B670` -> `0095CC90` ->
`0081ED40` -> `006FE460` (`docs/UNIT_INSTANCE_LAYOUT.md`). The destructors mirror it exactly,
each level found from slot 0 of its own `+0h` vtable and confirmed by the vptr set it rewrites:

| level | class body | `RET` | body | slot-0 deleting dtor | vtable | vptrs rewritten |
| --- | --- | --- | --- | --- | --- | --- |
| 6 | *none* | | | `006FE570` | `00CFC3D0` | — |
| 5 | `0081F3A0` | plain | `0081F3A0`..`0081F8AD` | `00822700` | `00D09678` | 8, `00D096xx` |
| 4 | `00959940` | plain | `00959940`..`00959BE4` | `0095A860` | `00D1A698` | 7, `00D1A6xx` |
| 3 | `0087A410` | plain | `0087A410`..`0087A4DA` | `0087AC60` | `00D0DF70` | 6, `00D0DFxx` |
| 2 | `0077E380` | plain | `0077E380`..`0077E490` | `0077E810` | `00D03E80` | 5, `00D03Exx` |
| 1 | `009287B0` | plain | `009287B0`..`0092885B` | `009289E0` | `00D192E0` | 4, `00D192xx` |
| 0 | `00925780` | plain | `00925780`..`009258C2` | `00925F00` | `00D19120` | 3, `00D19xxx` |

Levels 1 and 0 are the eleven-step rule table of `docs/ENTITY_LIFECYCLE_TAILS.md` section 1 and
are not restated here. `0077E4A0` and `0077E4B0` are level 2's adjustor thunks
(`SUB ECX,24h` / `SUB ECX,10h` then `JMP 0077E810`), the `+24h` and `+10h` vptr entries.

## The rule table, in execution order

"site" is the call or store instruction. Every level rewrites its own vptr set first.

### Level 5, `BSP_UnitVehicleBase_Destruct 0081F3A0`

| # | site | step | gate |
| --- | --- | --- | --- |
| 1 | `0081F43A` | for each element of the **point-effect array** at `+758h` (count `+75Ch`): `00867B10 BSP_PointEffect_StopChildren(effect)`, `effect[+9h] = 1` (`0081F43F`), release-ref (`0081F454`) with `effect->vtable[0]()` at zero (`0081F465`), slot nulled twice (`0081F467`, `0081F46E`) | `count != 0`, element non-null |
| 2 | `0081F488` | `0092CFD0(motion = [unit+1018h])`: releases the physics proxy at `motion+2Ch` through `00C34F70(ECX = [[00E188A8]+18h], proxy)` and nulls it. The controller is the other orchestrator's object (`docs/UNIT_PARTS.md`'s `+1018h` correction) | always, even on null |
| 3 | `0081F499` / `0081F49F` | `00932840(motion)` then `free(motion)` | `[unit+1018h] != 0` |
| 4 | `0081F4B2` | `free([unit+73Ch])`, then `+73Ch = 0` | non-null |
| 5 | `0081F4CF` | `[unit+740h]->vtable[10h]()`, then `+740h = 0` | non-null |
| 6 | `0081F4E2` | `free([unit+117Ch])`, then `+117Ch`/`+1180h`/`+1184h = 0` | non-null |
| 7 | `0081F517` / `0081F520` | `BSP_RefPtrRange_Release 0081C940(ECX = [unit+1118h], EDX = [unit+111Ch])` then `free([unit+1118h])`, then `+1118h`/`+111Ch`/`+1120h = 0` | `[unit+1118h] != 0` |
| 8 | `0081F53C` | `BSP_UnitSubObject10D4_Destruct 0074E7F0(unit+10D4h)`: base vptr `00D00514` and `free` of `+1Ch`, `+18h`, `+20h` | always |
| 9 | `0081F54C` | `BSP_UnitSubObject10A0_Destruct 0081EC00(unit+10A0h)` | always |
| 10 | `0081F55F` / `0081F567` | `BSP_LiveEffectReferences_Resize 004C9550(unit+FFCh, 0)` then `free([unit+FFCh])` | always |
| 11 | `0081F56C` | `[unit+BD0h] = 00D09480`, the embedded subobject's base vptr | always |
| 12 | `0081F595` / `0081F59D` / `0081F5A4` | the **recursive critical section** at `+BD4h`: while `[cs+18h] > 0`, decrement and `LeaveCriticalSection` (`[00CE2210]`); then `DeleteCriticalSection` (`[00CE2214]`); then `free(cs)`; `+BD4h = 0` | `[cs+18h] > 0` for the loop, non-null for the rest |
| 13 | `0081F5CB`, `0081F5F2`, `0081F619`, `0081F640`, `0081F667`, `0081F68E`, `0081F6B5`, `0081F6DC` | eight ref slots `+BC0h`, `+BBCh`, `+BB8h`, `+BB4h`, `+BB0h`, `+BACh`, `+BA8h`, `+BA4h`: release-ref, `vtable[0]()` at zero, null | each non-null |
| 14 | `0081F705` | eh vector destructor over **five** 4-byte slots at `+B54h`, element `BSP_RefPtrSlot_Release 00440A30` | always |
| 15 | `0081F71F` | the same over **four** slots at `+B44h` | always |
| 16 | `0081F72F` | `0093B920(unit+A20h)`, the **repair task** | always |
| 17 | `0081F742` / `0081F74A` | `004C9550(unit+A14h, 0)` then `free([unit+A14h])` | always |
| 18 | `0081F767` | eh vector destructor over **four** slots at `+A00h`, element `00440A30` | always |
| 19 | `0081F77F`, `0081F7A6`, `0081F7CD`, `0081F7F4` | four ref slots `+9F4h`, `+9F0h`, `+9ECh`, `+9E8h` | each non-null |
| 20 | `0081F834` / `0081F85F` | second pass over `+758h`: release-ref and null each element, then `free([unit+758h])` | `[unit+758h] != 0` |
| 21 | `0081F874` | `free([unit+74Ch])` | non-null |
| 22 | `0081F887` | `BSP_UnitOwnedRefSlot_Reset 00809650(unit+72Ch)`: base vptr `00D09004` and `[unit+734h]->vtable[0](1)` | `[unit+734h] != 0` |
| 23 | `0081F896` | `CALL 00959940`, level 4 | always |

Step 1 nulls every slot it releases, so step 20's release pass can never find a live element;
only its `free` of the buffer does work. Step 1's element test is written
`NEG ECX; SBB ECX,ECX; TEST ECX,0xE186EC` (`0081F429`..`0081F42D`) - the immediate is arbitrary
because `ECX` is already `0` or `-1`, so the test means "element non-null".

### Level 4, `BSP_UnitGameObject_Destruct 00959940`

| # | site | step | gate |
| --- | --- | --- | --- |
| 1 | `009599B3` | release-ref `+538h`, null | non-null |
| 2 | `009599DF` | `004BCA80(ECX = 0)`: `[00E188DC] = 0` then `00B0D7B0(ECX = [00F8D39C], 0)` | `[unit+4A4h] == [00E188DC]` |
| 3 | `009599F2` | release-ref `+4A4h`, null | non-null |
| 4 | `00959A1B` | `[unit+6F4h]->vtable[4h](1)`, null | non-null |
| 5 | `00959A33` | `[unit+724h]->vtable[0h](1)`, null | non-null |
| 6 | `00959A7E` | drain the list record at `+424h` (`{count +424h, head +428h, tail +42Ch}`): unlink each node, decrement the count, `free` it. Loop back-edge `00959A8C` | `head != 0` |
| 7 | `00959AD1` | the same drain **twelve times**, once per 12-byte record of the **gun category list array** at `+394h` (`EDI` starts at `+398h`, step `0Ch`, `EBP = 0Ch` at `00959A95`). Loop back-edge `00959ADB` | per record |
| 8 | `00959B09` | the pooled string at `+718h`/`+714h` returned through `00419CC0` then `00BD1510` | `[unit+718h] != 0` |
| 9 | `00959B31` | the same for `+6D0h`/`+6CCh` | non-null |
| 10 | `00959B49` | release-ref `+670h`, null | non-null |
| 11 | `00959B6C` | `free([unit+664h])`, then `+664h`/`+668h`/`+66Ch = 0` | non-null |
| 12 | `00959B9B` | eh vector destructor over **ten** `0x18`-byte records at `+53Ch`, element `004BD350` (a `JMP` thunk to `004B7EF0`) | always |
| 13 | `00959BA6` | `BSP_GunCategoryList_Clear 00955EB0(unit+424h)` | always |
| 14 | `00959BBF` | eh vector destructor over **twelve** `0Ch`-byte records at `+394h`, element `00957080` (a `JMP` thunk to `00955EB0`) | always |
| 15 | `00959BCE` | `CALL 0087A410`, level 3 | always |

Steps 6/13 and 7/14 are the same belt-and-braces shape as level 5's 1/20: the drain is inlined
first and the out-of-line clear runs afterwards on an already-empty list. `00957080`'s target is
what identifies the `+394h` array as the gun category lists.

### Level 3, `BSP_UnitTickableEntity_Destruct 0087A410`

| # | site | step | gate |
| --- | --- | --- | --- |
| 1 | `0087A474` | `free([unit+380h])`, then `+380h`/`+384h`/`+388h = 0` | non-null |
| 2 | `0087A499` | `free([unit+348h])`, then `+348h`/`+34Ch`/`+350h = 0`: the **parts-descriptor vector** whose begin is `+348h` and end `+34Ch` (`docs/UNIT_PARTS.md`). Only the pointer array is freed; the descriptors point into the vehicle class and are not owned | non-null |
| 3 | `0087A4B5` | `BSP_UnitTickElement_Destruct 00875490(unit+310h)`: base vptr `00D0DEC8`, unlink the element over its `+4h`/`+8h` pair, then `ADD ECX,1Ch` and tail-jump to `BSP_UnitTickElement_ClearSubList 00874F00` on `unit+32Ch` | always |
| 4 | `0087A4C4` | `CALL 0077E380`, level 2 | always |

Step 3 is the **only** tick-element unregistration in the whole chain; no other level reads
`+310h` except to rewrite its vptr (`docs/TICK_ELEMENT_OVERRIDES.md`,
`docs/FIXED_STEP_JOB_WAVES.md`).

### Level 2, `BSP_UnitOwnerEntity_Destruct 0077E380`

| # | site | step | gate |
| --- | --- | --- | --- |
| 1 | `0077E3F3` / `0077E404` | build the **kind-`4Fh` session message** carrying `entity+70h` (`BSP_SessionMessage_ConstructEntityRelease 00779780`, payload at `msg+20h`) and route it: `BSP_Session_RouteMessage 0077C2A0(this, msg, 4, 0)` | `[00E0AF20] != 0` **and** `[00E188A8] != 0` **and** `[[00E188A8]+1FE4h] == 1` (`0077E3C8`..`0077E3E9`) |
| 2 | `0077E417` | `0077BEA0(ECX = 00F87194, &local)` with the unit stored in the local at `0077E413`: erase the unit from the global container at `00F87194` | always |
| 3 | `0077E427` | `006E0860(unit+2B0h)`, base vptr `00CF9348` | always |
| 4 | `0077E434` / `0077E43D` | `0077D930(unit+2A4h)` clears the circular list sentinel at `+2A8h` and zeroes the count at `+2ACh`; the caller then `free([unit+2A8h])` and sets `+2A8h = 0` (`0077E44B`) | always |
| 5 | `0077E452` | `0077B980(unit+298h)`, the same list clear over the sentinel at `+29Ch` | always |
| 6 | `0077E46C` | eh vector destructor over the **three** `0x34`-byte **recon detection records** at `+1E8h`, element `BSP_ReconDetectionRecord_Destruct 00804300` (base vptr `00D08E74`, then `ADD ECX,14h` and `JMP 004B7EF0`) | always |
| 7 | `0077E47B` | `CALL 009287B0`, level 1 | always |

## The five questions

### 1. The ordered chain

The five tables above plus the eleven steps of `docs/ENTITY_LIFECYCLE_TAILS.md` section 1.

### 2a. No step removes a spatial-index node

None of the seven bodies contains a reference to `entity+1C4h` or a call to `0098A500`
`BSP_SpatialIndex_DetachNode`, `0098A3D0`, `0042E630`, `00929D70`, `00710B80` or `00951F40`
(grep of the concatenated listings of `0081F3A0`, `00959940`, `0087A410`, `0077E380`,
`009287B0` and `00925780`, the first and fourth read from the PE bytes past Ghidra's body end).

`docs/SPATIAL_INDEX.md`'s three attach sites explain why. The unit's grid presence is
**per part**: `00710B6D` attaches the part instance itself as the node, reached from
`BSP_UnitPartInstance_Construct`, and the matching detach is `00710B80`
(`if (part[+184h]) { 0098A500(0042E630(), part); part[+184h] = 0; }`), whose only caller is
`00951F40` - which no destructor level reaches. The moving-entity node at `entity+1C4h`
(`0092B30C`) is not the unit's: `00929E30` is the only vtable slot in the image that holds it
(`00D19584`, slot offset `84h` of the vtable based at `00D19500`, whose `[-4h]` RTTI slot is
`0`), it tests `[this+384h]` as a **byte**, and level 3 uses `+384h` as a vector's end pointer.
The unit's own vtable group is `00CFC384`..`00CFC3D0`.

So on release an attached part's node is left linked into the grid unless something before the
destructor detaches it. The trigger is `contract: unread`; the deferred-destroy pass
(`00904BF0` family, another orchestrator's) is the place to look.

### 2b. The recon detection records are dropped by level 2's last step

`0077E46C`, the eh vector destructor over three `0x34`-byte records at `unit+1E8h` with element
destructor `00804300`. `00804300` writes the base vptr `00D08E74` and tail-jumps to `004B7EF0`
for the sub-object at record `+14h`. `004BD350`, level 4's element destructor for the ten
`0x18`-byte records at `+53Ch`, is a `JMP` thunk to the same `004B7EF0`.

### 3. No unit-level destructor takes `thisTable[name]` back

`00CE7494` (`thisTable`), `00B67350` and `[00E188A8]+1A0Ch` do not occur in any of the seven
bodies. `[00E188A8]` is read twice in the chain and neither use touches the Lua registry:
level 2's session gate at `0077E3D9`/`0077E3E2` (`+1FE4h`) and, inside `0092CFD0`, the physics
world at `+18h`. `docs/ENTITY_LIFECYCLE_TAILS.md` reached this conclusion from levels 0 and 1
alone; it now holds for the whole unit chain, so a unit's `thisTable[name]`
(`docs/MISSION_LUA_SELF_TABLE.md`, given by slot 39) really does outlive the instance.

### 4. The producer of `player+8h`; `player+9h` has none

The gate `player[+8h] != 0 && player[+9h] == 0` reads a **`0x118`-byte participant record**.
`BSP_Game_ClaimParticipantRecord 004BB440` is the producer: `ESI = session+748h` at `004BB446`,
then a loop of eight (`CMP EAX,8` at `004BB45E`) testing `[ESI+8h] == 0` at `004BB450` and
stepping `0x118` at `004BB458`; on the first free record it writes **`004BB47D`
`MOV byte ptr [ESI+8],1`** with `+0Bh`, `+19h`, `+50h` and the name at `+58h`, and returns `0`
from `004BB464` with `RET 24h` when all eight are taken. `+8h` is the "record claimed" flag.
`004BB569` in `004BB550` writes the same byte on the same array.

The pointer array the gate readers index is eight dwords at `session+18CCh`, immediately after
the second eight-slot record array at `session+1008h`: `004BB660` sets
`[session + slot*4 + 18CCh] = session + slot*118h + 1008h` (and `old[+20h] = -1`), and
`004BB630` overwrites the entry with a live object (and writes `slot` to its `+20h`).
`BSP_SessionMessage_ConstructBase 0075B430` reads the same array at `0075B469` with the local
index from `[[00E188A8]+18ECh]` bounds-checked to `0..7`.

`+9h` has **no writer**. The only store to it in the image is the out-of-line setter
`BSP_ParticipantRecord_SetGateByte9 004B5680` (`MOV AL,[ESP+4]; MOV byte ptr [ECX+9],AL; RET 4`),
which has no `E8` call site; its twin `BSP_ParticipantRecord_SetClaimed 004B5670` is the `+8h`
setter and is likewise uncalled, which is what proves the two bytes are fields of one record
type. A scan of `.text` for every `C6 /r` and `88 /r` byte store to `[reg+9h]`, and for every
`disp32` byte store landing on field `9` of an eight-slot `0x118` array based at `session+748h`
or `session+1008h`, found nothing else. So the `+9h == 0` half of the gate is satisfied by the
record's zero-init in the shipped image. `coverage: partial` - a store reached through a base
other than those two array bases would not have been seen.

### 5. `entity+70h` is read by level 2

`0077E3EB MOV EAX,[ESI+0x70]`, the payload of the kind-`4Fh` message of level 2 step 1. It is
the only reader in the image. `include/bsp/entity_lifecycle_tails.hpp` already names the field
`kEntityOffDeadMeatMark` from its single writer at `009272F3`; the destructor forwards the mark
to the session, gated on `[[00E188A8]+1FE4h] == 1`.

## Coverage

| routine | status | coverage |
| --- | --- | --- |
| `0081F3A0` | analyzed, reconstructed, build-tested | complete, `0081F3A0`..`0081F8AD` |
| `00959940` | analyzed, reconstructed, build-tested | complete |
| `0087A410` | analyzed, reconstructed, build-tested | complete |
| `0077E380` | analyzed, reconstructed, build-tested | complete, `0077E380`..`0077E490` |
| `006FE570`, `00822700`, `0095A860`, `0087AC60`, `0077E810`, `009289E0`, `00925F00` | analyzed | complete |
| `00875490`, `00874F00`, `00440A30`, `0081C940`, `0074E7F0`, `00809650`, `00804300` | analyzed | complete |
| `0092CFD0`, `004BCA80`, `00779780`, `004B5670`, `004B5680` | analyzed | complete |
| `00932840`, `0093B920`, `0081EC00`, `004C9550`, `006E0860`, `0077D930`, `0077B980`, `004B7EF0`, `00955EB0` | analyzed | partial: entry contract only, bodies not read past the first block |
| `009287B0`, `00925780` | cited | `docs/ENTITY_LIFECYCLE_TAILS.md` |

## Corrections to earlier documents

| document | was | is | evidence |
| --- | --- | --- | --- |
| `ENTITY_LIFECYCLE_TAILS.md` | "the unit's own destructor levels above `009287B0`" open | four levels read: `0077E380`, `0087A410`, `00959940`, `0081F3A0`, and level 6 has no body | the level table above; each level's vptr set matches `docs/UNIT_INSTANCE_LAYOUT.md` |
| `ENTITY_LIFECYCLE_TAILS.md` | `0081F3A0`'s body "ends at `0081F56B` but the real body continues" | the real body ends at `0081F8AD`; Ghidra still stops at `0081F56B`, now at the `free` at `0081F567` | `disasm-raw 0081F56C`, `RET` at `0081F8AD` |
| `ENTITY_LIFECYCLE_TAILS.md` | the producer of `player+8h`/`+9h` not found | `+8h` is written `1` at `004BB47D` in `BSP_Game_ClaimParticipantRecord 004BB440`; `+9h` has no writer | `004BB446`..`004BB47D`; the uncalled setters `004B5670`/`004B5680` |
| `ENTITY_LIFECYCLE_TAILS.md` | "no step in the release chain was identified as a spatial-index removal" | none exists; the unit's grid presence is per part and the detach `00710B80` is not reached from any level | the grep above; `docs/SPATIAL_INDEX.md`'s `00710B6D` row |
| `ENTITY_LIFECYCLE_TAILS.md` | `entity+70h`: "no reader found" | read at `0077E3EB` as the payload of the kind-`4Fh` session message | `0077E3EB`..`0077E404`, `00779780`'s `+20h` store |
| ledger | `CG_vector_deleting_dtor_0081f3a0`, `_00959940`, `_0077e380` | class-body destructors, `__thiscall(this)` with a plain `RET` and no count argument | `0081F8AD`, `00959BE4`, `0077E490` |

## Open questions

| question | why it is open |
| --- | --- |
| who releases the weapon director `+738h`, the gunnery pass `+6DCh` and the damage-state instance `+360h` | no level reads or clears any of the three |
| who detaches a unit part's spatial-index node before the release | `00710B80`'s only caller `00951F40` is not reached from the chain; its seven callers were not read |
| `unit+740h`'s `vtable[10h]` and `unit+6F4h`'s `vtable[4h]` | the slots are called but the concrete targets were not resolved |
| `004BCA80`'s global `00E188DC` and the notify `00B0D7B0(ECX = [00F8D39C])` | the "current object" the level-4 step clears was not identified |
| whether `+8h` is ever set on the `session+1008h` record array | only the `session+748h` array has a writer, yet `004BB660` points the gate array at `+1008h` |
