# The fourteen AI mission Lua bindings

Addresses: 00a37310 00a37400 00a37650 00a37790 00a378c0 00a37a00 00a37b30 00a37d50 00a37eb0
00a38010 00a38200 00a38430 00a38960 00a38a50 00a37250 00a32350 00a371c0 00888d20 009ffc80
00a07f60 00a07f80 00a2bd00 00b677e0 00b67930

Packet `cc2_lua_binding_ai`, worktree `agent/cc2-lua-binding-ai`. Ghidra was read-only. Every
descriptive name here is a hypothesis, not a recovered symbol; the fourteen Lua global names are
genuine, from the 560-entry table at `00E0B7B8` (`docs/LUA_BINDING_TABLE.md`).

The reconstruction is `include/bsp/lua_binding_ai.hpp` and `src/lua_binding_ai.cpp`. The argument
readers and the result writer of `include/bsp/lua_binding_core.hpp` are reused rather than
redeclared; no top-level type or constant name in this packet duplicates one there.

## The shape they share

All fourteen are `__fastcall(lua_State* in ECX)` returning the result count in EAX, with the
prologue and epilogue `docs/LUA_BINDING_CORE.md` records: `00B66C00` borrowed owner, `00B679B0`
call frame, `00B66400` result count, `00B669A0` close, and the one-time `luakod` log-category
static-local guarded by bit 0 of a per-handler flag byte (`00F8AB70`, `00F8AB78`, `00F8AB80`,
`00F8AB88`, `00F8AB90`, `00F8AB98`, `00F8ABA0`, `00F8ABA8`, ...). That static has no per-call
effect and is not modelled.

Arguments come from `00B677E0`. Its body is twelve instructions and settles the call shape:
`__thiscall(ECX = frame)(LuaObject* out, int index)`, `RET 8`, forwarding to `00B67720` with ECX
untouched and answering `out` in EAX. Five handlers build that object **directly in an outgoing
by-value argument slot** — `SUB ESP,0x14`, `MOV ECX,ESP`, push the index and the pointer, call
`00B677E0`, then immediately call a routine that pops the object with `RET 0x14`. That is why the
pseudocode shows a bare `FUN_00a37250()` with no visible argument.

Thirteen handlers push nothing, so their result count is zero; `AIGetTargetWeight` pushes one
number through `00B66480`.

## The routines

| Address | Global | Arguments | Results | Coverage |
| --- | --- | --- | --- | --- |
| `00A37310` | `AICreate` | none | none | complete |
| `00A37400` | `AIEnable` | int, bool, optional table | none | complete |
| `00A37650` | `AIEnableGrouping` | entity, bool | none | complete |
| `00A37790` | `AIMergeGroups` | entity, entity | none | complete |
| `00A378C0` | `AIGetGroupInfo` | entity | a table, see below | complete |
| `00A37A00` | `AISetCommand` | entity, table | none | complete |
| `00A37B30` | `AISetHintWeight` | entity, [int,] number | none | complete |
| `00A37D50` | `AISetDefendResourcePercent` | number | none | complete |
| `00A37EB0` | `AISetSpawnSceneUnitsWeightMul` | number | none | complete |
| `00A38010` | `AISetQuickSpawnTargetPos` | int, bool, optional vector | none | complete |
| `00A38200` | `AIGetTargetWeight` | int, int, optional int or bool | one number | complete |
| `00A38430` | `AISetTargetWeight` | int-or-string x2, bool, number, or the six-argument form | none | complete |
| `00A38960` | `AIReloadGlobals` | none | none | complete |
| `00A38A50` | `AICreateGroup` | a table of entities | none | complete |

All fourteen bodies were read end to end. Four callees are labelled `contract: unread` or
`partial` in the host table below; nothing in the handlers themselves is unread.

### AICreate `00A37310`

No argument is read and nothing is pushed: between the frame at `00A373A2` and the result count at
`00A373B8` there is one instruction pair, `CALL 0x00A32350`. That body was read in full. It
allocates 1A8h bytes (`00BF55BE`), zeroes them (`00BF79F0`), constructs with `00A31730`, then
builds a 4x4 identity matrix on the stack — XMM1 holds `00D7A24C` (1.0f) and is stored at
`ESP+0Ch`, `+20h`, `+34h` and `+48h` while XMM0 (zero) fills the other twelve slots,
`00A323A2..00A3241A` — and calls the vtable slot at `+98h` with the scene root from
`[00E188A8]+19CCh` and that matrix, then the slot at `+A0h`, then `00A03A60`.

### AIEnable `00A37400`

| Slot | Type | Use |
| --- | --- | --- |
| 0 | integer | record index into the globals array |
| 1 | boolean | the record's enabled byte |
| 2 | table | `attackRatio` and `aggressiveRatio`, only on the gate below |

`00A374C4` computes the record address as `LEA ESI,[EAX*8+0]; SUB ESI,EAX; LEA ESI,[ESI*4+0xF8A8C8]`
— that is `0xF8A8C8 + 28 * index`. The gate for the table is **two** conditions in order: `00A37515`
re-reads the byte it just stored (`CMP byte ptr [ESI],0`), and `00A37527` requires `count >= 3`.
Both field reads fall back to `00CE3800`, whose bytes are `00 00 00 3F`, so the default is 0.5f.

### AIEnableGrouping `00A37650`, AIMergeGroups `00A37790`, AISetCommand `00A37A00`

These three resolve their entity arguments through `00A37250`, whose body is twenty instructions
and is the packet's key finding. It takes one LuaObject **by value** (`RET 0x14`), calls
`00888AA0` — `BSP_ObjectHandle_FromLuaTable`, per `docs/LUA_BINDING_ENTITY.md` — and answers
`entity+16Ch`. There is no null test on `00888AA0`'s result, so a table that is not an entity
faults in the load at `00A37277`.

`AIEnableGrouping` opens argument 1 first (`00A376E9`, `PUSH 0x1`) and argument 0 second
(`00A37703`), resolves the group from argument 0 and stores argument 1's boolean into
`group+5648h` at `00A37731` with no null test.

`AIMergeGroups` resolves argument 0 into ESI and argument 1 into EAX, calls
`00A2DB80(ECX = first)(second)`, then clears `first+5648h` at `00A37875`.

`AISetCommand` resolves argument 0 into ESI, builds a command from argument 1 with
`00A13340(ECX = group)(LuaObject by value)` and installs it with `00A2BD00`.

### AIGetGroupInfo `00A378C0`

Resolves argument 0 into a group, creates a Lua table with `00B67930` and fills it with
`00A2EEE0(ECX = group)(table)`. `00B67930`'s body was read: `lua_createtable` through `00A67D10`,
a reference through `00A673D0`, and the out object is stamped kind 2, the tracked-registry kind of
`docs/LUA_OBJECT_API.md`. Because a registry reference pops the value it takes, whether this
handler leaves anything on the Lua stack depends on `00A2EEE0`, whose 551 instructions were not
read past its gate. The result count is still `00B66400`'s difference, never a literal. **This is
the one handler whose result count is not established.**

### AISetHintWeight `00A37B30`

Two exact-count branches, `00A37BDB` (`== 2`) and `00A37C4A` (`== 3`); any other count falls
through to the epilogue at `00A37CFC` having read nothing.

| Count | Slot 0 | Slot 1 | Slot 2 | Native |
| --- | --- | --- | --- | --- |
| 2 | entity table | number weight | — | `00A07F60` |
| 3 | entity table | integer party | number weight | `00A07F80` |

The handle is **not** the entity: `00A37C20` calls `00888D20(ECX = argument 0's LuaObject)`, whose
body reads the field named at `00CFAD08` — the four bytes `50 74 72 00`, `"Ptr"` — through
`00B67800` and converts it with `00B662D0`. `00A07F60` then stores the float into the container at
`00F8A740` keyed by that handle; `00A07F80` uses `0xF8A750 + party * 12`, an array of per-party
maps. Both are `RET 4` with the float as the single stack argument, pushed by `FSTP` at
`00A37C1D`.

Confirmed by the installed scripts: `competitive05.lua:696` calls `AISetHintWeight(unit, 20)`, the
two-argument form, and `competitive05.lua:1120` calls it on a table element.

### AISetDefendResourcePercent `00A37D50` and the global overlap

`009FFC80` runs **before** the argument is read, at `00A37DF6`. Its body was read in full: it
takes the game object at `00E188A8`, dispatches on `004BCA50`'s answer through the jump table at
`009FFCFC`, and returns 0..6, with 0 on both failure paths. The clamp is `[0, 1]` — `FLDZ` /
`FCOMIP` at `00A37E24` sends anything not at or above zero to zero, then `COMISS` against
`00D7A24C` (1.0f) caps the top — and the store is `MOVSS [ESI*4+0xF8A8BC],XMM0` at `00A37E4B`.

**An unresolved overlap.** `00F8A8BC` and `00F8A8C8` are twelve bytes apart, so a slot of 3 or more
writes into the first record of the 1Ch-stride array that `AIEnable` and `AISetQuickSpawnTargetPos`
use. Both readings come from the assembly, not the pseudocode, and both are certain. The cross
references confirm the two globals have independent writers (`00A335D0` writes `00F8A8BC`;
`00A32DF0` and `00A163D0` write `00F8A8C8`). This document records the overlap rather than
resolving it: either `009FFC80`'s slots 3..6 are unreachable on this path, or the shipped build
has a latent out-of-bounds write here. Deciding between those needs `00A335D0` read, which this
packet did not do.

`islandcapture01.lua:1744` calls `AISetDefendResourcePercent(0.2)`, the single-number form.

### AISetSpawnSceneUnitsWeightMul `00A37EB0`

Not the same clamp shape. The handler stores the minimum `00D7A23C` (bytes `6F 12 83 3A`, 0.001f)
into `00E0E35C` first and overwrites it only when the argument reaches that minimum, then caps at
1.0f. So an argument **below** the minimum yields 0.001f, not the argument and not zero. Fifteen
installed scripts call `AISetSpawnSceneUnitsWeightMul(0)` (for example `competitive03.lua:520`),
and every one of them therefore sets the global to 0.001f, not to zero.

### AISetQuickSpawnTargetPos `00A38010`

| Slot | Type | Use |
| --- | --- | --- |
| 0 | integer | record index |
| 1 | boolean | the record's valid byte at `+0Ch` |
| 2 | table | a vector 3, read only when slot 1 is true (`00A38117`) |

`00A38137` calls `00888760(ECX = an out buffer, EDX = the LuaObject)` and copies three floats from
the returned pointer. The flag is stored unconditionally and the three floats only when it is set,
so a false call leaves the previous position bytes in place.

### AIGetTargetWeight `00A38200`

Both class arguments go through `00964790(ECX = class id, DL = 1)`, the vehicle-class descriptor
factory of `docs/VEHICLE_CLASS_DESCRIPTORS.md`. A null from either skips the query and the pushed
weight stays at the zero written by `XORPS` at `00A3831C`.

The two optional arguments are read by **two independent equality tests**, `00A38340` (`CMP EAX,3`)
and `00A38380` (`CMP EAX,4`), not by a range test. With exactly four arguments the first test fails
and **argument 2 is never read**; the integer keeps the zero stored at `00A38337`. EDI is the zero
register throughout (it supplies the index 0 at `00A3829B` and both null comparands at `00A3831A`
and `00A3832B`) and is only overwritten at `00A383B5`, after those uses.

The query is `00A08460(ECX = subject, EDX = mode)(target, flag)` returning the weight in ST0, and
the push is `00B66480` at `00A383D9`, a float push distinct from the `00B664B0` integer push.

### AISetTargetWeight `00A38430`

Each class argument is an integer or a name; `00B660A0` decides per argument. The string path
copies the borrowed string, then walks the 97 `char*` entries at `00E0CD80` with a strcmp loop
bounded by `0x61`, leaving the index or `-1`. Entry 0 is `00D0E694`, `"NULL"`.

| Count | Slot 0 | Slot 1 | Slot 2 | Slots 3,4,5 | Weight slot |
| --- | --- | --- | --- | --- | --- |
| under 5 | class | class | boolean | not read | 3 |
| 5 or more | class | class | boolean | int, int, bool | 6 |

The ten values are assembled into a stack record and handed to
`00A32500(ECX = record, DL = 1)(-1)` at `00A3890E`. `00A32500`'s head shows `DL` selecting the
insert branch, which walks a 20h-stride table addressed by the globals `00F8AB5C` and `00F8AB60`
(`SHL ECX,5` at `00A32521`). **The record's exact byte layout is not established**: that would need
`00A32500`'s 236 instructions read, which this packet did not do, so `AiTargetWeightRule` in the
header carries the fields in write order and says so.

`competitive05.lua:809` calls `AISetTargetWeight(102, 67, false, 5)` — two integers, a boolean and
a number, the four-argument form, weight in slot 3. That is the only form the installed scripts
use.

### AIReloadGlobals `00A38960`

One call, to `00A371C0`, whose whole body is `CALL 0x004C1C50; MOV ECX,EAX; JMP 0x00A335D0`.
`00A335D0` is the AI globals loader and one of the writers of `00F8A8BC`; its body was not read.

### AICreateGroup `00A38A50`

Argument 0 is a table. The handler iterates it with `00B67190` / the first-pair helper, pushes each
element's handle (through `00888D20`, the same `"Ptr"` read) into a `std::vector`, then runs a
second loop over that vector:

1. `MOV ECX,[ESI+16Ch]` — the entity's current group. When non-null, `00A2D9D0(ECX = that group)`
   `(entity, 1)` removes it.
2. If no group has been built yet, `operator new(0x5660)` at `00A38CAE`, then
   `00A2DFA0(ECX = the block)(entity)` answering the group in EAX. **A null allocation leaves the
   group null and the loop continues.**
3. Otherwise `00A2D8E0(ECX = group)(entity)` adds the entity.

After the loop, `group+5648h` is cleared with no null test. An empty argument table, or a failed
allocation, reaches that store with a null pointer. The reconstruction declines to reproduce the
fault and guards the store, which is noted in the source.

The 5660h from the allocation is the group object's size, and it contains every group offset this
packet found.

## The AI group object

| Offset | Established by | Meaning |
| --- | --- | --- |
| `+10h` | `00A2D8E0` links the entity here | member list head |
| `+563Ch` | `00A2D9D0` passes it to `0077BEA0` | member list, the removal side |
| `+5644h` | gate in `00A2DB80` and `00A2EEE0` | population; zero makes both a no-op |
| `+5648h` | `00A37731` store, `00A37875` and `00A38D04` clears | grouping-enabled byte |
| `+564Ch` | `00A2BD00` reads, deletes and rewrites | the owned current command |
| size `5660h` | `00A38CAE` `PUSH 0x5660` | |

An entity's back-pointer to its group is `entity+16Ch`, read at `00A37277` and written at
`00A2D8F5`. `00A2BD00` deletes the previous command through its vtable slot 0 with the flag 1 and
then stores the new pointer, so handing it the pointer it already holds would free and dangle.

## The per-party AI globals

| Address | Field | Written by |
| --- | --- | --- |
| `00F8A8BC + slot*4` | defend resource percent, float | `00A37E4B` |
| `00F8A8C8 + idx*1Ch` | enabled, byte | `00A37507` |
| `+04h` | attack ratio, float | `AIEnable`'s table branch |
| `+08h` | aggressive ratio, float | `AIEnable`'s table branch |
| `+0Ch` | quick-spawn valid, byte | `00A3817C` |
| `+10h`,`+14h`,`+18h` | quick-spawn position, three floats | `00A3817C` onward |
| `00E0E35C` | spawn scene-units weight multiplier, float | `AISetSpawnSceneUnitsWeightMul` |
| `00F8A740` | global hint-weight container | `00A07F60` through `00975C40` |
| `00F8A750 + party*12` | per-party hint-weight containers | `00A07F80` through `00975C40` |
| `00F8AB5C`, `00F8AB60` | base and count of the 20h-stride target-weight table | `00A32500` |

See the overlap note under `AISetDefendResourcePercent`: the first two rows are not disjoint for
slots 3 and above.

## The host table

Every native step, as the call site that reaches it. `native` is the callee and `address` the call
site; the containing function is the packet handler named in the row.

| Host method | address | native | Containing | Contract |
| --- | --- | --- | --- | --- |
| `ai_controller_create` | `00A373AF` | `00A32350` | `AICreate` | body read in full; allocates 1A8h, constructs, attaches with an identity matrix, activates |
| `ai_group_of_entity_argument` | `00A3771F` | `00A37250` | `AIEnableGrouping` | body read in full; `RET 14h`, one LuaObject by value, answers `entity+16Ch` |
| `ai_group_of_entity_argument` | `00A37849` | `00A37250` | `AIMergeGroups` | same callee, argument 0 |
| `ai_group_of_entity_argument` | `00A37864` | `00A37250` | `AIMergeGroups` | same callee, argument 1 |
| `ai_group_of_entity_argument` | `00A3797B` | `00A37250` | `AIGetGroupInfo` | same callee |
| `ai_group_of_entity_argument` | `00A37AB9` | `00A37250` | `AISetCommand` | same callee |
| `ai_group_merge` | `00A3786C` | `00A2DB80` | `AIMergeGroups` | `__thiscall(ECX = into)(from)`; head read, returns at once when `from+5644h` is zero. contract: partial |
| `new_lua_table` | `00A3798B` | `00B67930` | `AIGetGroupInfo` | body read in full; `lua_createtable` + registry ref, out object kind 2 |
| `ai_group_write_info` | `00A3799F` | `00A2EEE0` | `AIGetGroupInfo` | `__thiscall(ECX = group)(table)`; head read, gated on `+5644h`, first field `"leader"` (`00CFD760`). contract: partial |
| `ai_command_from_table` | `00A37AD6` | `00A13340` | `AISetCommand` | `__thiscall(ECX = group)(LuaObject by value)`; head read, dispatches on `"commandType"` (`00D22A88`). contract: partial |
| `ai_group_set_command` | `00A37ADE` | `00A2BD00` | `AISetCommand` | body read in full; deletes `+564Ch` through vtable slot 0 with flag 1, then stores |
| `lua_table_ptr_field` | `00A37C20` | `00888D20` | `AISetHintWeight` | body read in full; reads the `"Ptr"` field (`00CFAD08`) and converts with `00B662D0` |
| `ai_hint_weight_set_global` | `00A37C27` | `00A07F60` | `AISetHintWeight` | body read in full; `00975C40` slot in the container at `00F8A740`, keyed by the handle |
| `ai_hint_weight_set_for_party` | `00A37CC4` | `00A07F80` | `AISetHintWeight` | body read in full; same lookup at `0xF8A750 + party*12` |
| `ai_current_party_slot` | `00A37DF6` | `009FFC80` | `AISetDefendResourcePercent` | body read in full; jump table at `009FFCFC`, answers 0..6, 0 on failure |
| `read_vector3` | `00A38137` | `00888760` | `AISetQuickSpawnTargetPos` | `__fastcall(ECX = out, EDX = LuaObject*)`, three floats from the returned pointer. contract: unread |
| `vehicle_class_descriptor` | `00A382C9` | `00964790` | `AIGetTargetWeight` | `__fastcall(ECX = class id, DL = 1)`; already reconstructed in `docs/VEHICLE_CLASS_DESCRIPTORS.md` |
| `vehicle_class_descriptor` | `00A38303` | `00964790` | `AIGetTargetWeight` | same callee, argument 1 |
| `ai_target_weight_query` | `00A383C4` | `00A08460` | `AIGetTargetWeight` | `__fastcall(ECX = subject, EDX = mode)(target, flag)`, weight in ST0; 1354 instructions. contract: unread |
| `push_number_float` | `00A383D9` | `00B66480` | `AIGetTargetWeight` | float result push, distinct from the `00B664B0` integer push |
| `ai_target_weight_rule_apply` | `00A3890E` | `00A32500` | `AISetTargetWeight` | `__fastcall(ECX = record, DL = 1)(-1)`; head read, 20h-stride table at `00F8AB5C`/`00F8AB60`. contract: partial |
| `ai_reload_globals` | `00A389FF` | `00A371C0` | `AIReloadGlobals` | body read in full, three instructions; tail-jumps to the globals loader `00A335D0`. `00A335D0` contract: unread |
| `ai_group_remove_entity` | `00A38CA5` | `00A2D9D0` | `AICreateGroup` | `__thiscall(ECX = current group)(entity, 1)`; body read, removes from `+563Ch` and clears `entity+16Ch` |
| `ai_group_construct` | `00A38CCE` | `00A2DFA0` | `AICreateGroup` | `__thiscall(ECX = the 5660h block)(first entity)`, answers the group. contract: partial |
| `ai_group_add_entity` | `00A38CF6` | `00A2D8E0` | `AICreateGroup` | body read; calls `009FE0B0`, sets `entity+16Ch`, links into `group+10h` |

## What the installed scripts reach

Only four of the fourteen appear in the 299 installed mission scripts: `AISetHintWeight`,
`AISetTargetWeight`, `AISetSpawnSceneUnitsWeightMul` and `AISetDefendResourcePercent`. The other
ten — including all five group handlers, `AICreate` and `AIReloadGlobals` — have no call site in
the shipped mission scripts, so their argument contracts rest on the listing alone. `AICreateGroup`
is the one whose argument shape would most benefit from a live call.

No run-time evidence was gathered: `bsp_game.exe` does not reach these handlers, which run only
under a loaded mission script.

## Follow-up

- Read `00A335D0` and settle the `00F8A8BC` / `00F8A8C8` overlap. That is the open correctness
  question in this packet.
- Read `00A32500` in full to fix the target-weight record layout, and `00A2EEE0` to settle whether
  `AIGetGroupInfo` returns its table.
- Read `00A08460` for the target-weight model itself, the largest unread body the packet touched.

## Correction from docs/AI_GROUP_THINK.md (packet cc2_ai_group_think)

- **Was:** group+10h is the member list head, because 00A2D8E0 links the entity there
  **Is:** group+10h is the group's observer subobject with vtable 00D2306C; the member list is group+563Ch (std::list object), its sentinel node group+5640h and its size group+5644h
  **Evidence:** 00A2DFCD stores 00CE3CD4 at +10h and 00A2DFEB overwrites it with 00D2306C, a five-slot table (00A2D570, 00A2DA60, 00A2BD40, 00A2DB50, 0042B140). 00A2D8FD is LEA EDX,[ESI+0x10] feeding 00694A60 BSP_Observer_RegisterPair at 00A2D906, and 00A2E3B9/00A2E3BE pass the same +10h to 006956A0 to undo it. The list is built at 00A2DFF2-00A2E027: LEA EBX,[ESI+563Ch], CALL 004C1630, [EBX+4]=EAX, [EBX+8]=0.
- **Was:** the 00F8A8BC defend-percent array and the 1Ch records at 00F8A8C8 are not disjoint for slots 3 and above (flagged as unresolved)
  **Is:** the Defend_ResourcePercent array is exactly three dwords, indexed by difficulty 0..2, and the globals loader guards the index so it never overflows; only the unguarded Lua setter can reach the collision
  **Evidence:** 00A16B46 bounds the brain array with CMP EDI,0xF8A8BC and 00A182EE starts the party records at EBP = 0xF8A8C8 with stride 1Ch and bound 0xF8A9A8, so only three dwords lie between. 00A360F5 is CMP EDI,3 / JGE 00A3613E, skipping the store at 00A36126 for modes 3..6; EDI's only writes before that store are XOR EDI,EDI at 00A3374B and ADD EDI,1 at 00A370D4 (whole-listing filter). The key string at 00D23500 is 'Defend_ResourcePercent'. 00A37E4B stores with ESI = 009FFC80() (0..6) and no bound check.
