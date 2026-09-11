# Entity think dispatch (`00929460`, the two lists at `00F89AB0` and `00F89ABC`)

Addresses: 00929460, 00929150, 00928380, 00928330, 00928300, 00928030, 0088a240,
00874d00, 00898150

Read-only analysis: no rename, comment, prototype or save was made in Ghidra from this
packet. Every descriptive name below is a hypothesis, not a recovered symbol, with two
exceptions the image itself spells: the scope label `"SGameEntity::Call"` assigned once
inside `009290A0` and the chunk text `collectgarbage()` at `00D19480`.

`docs/FIXED_STEP_FANOUT.md` item 8 established that `00929460` is row 8 of the fixed-step
fan-out and left the follow-up `entity_think_dispatch` open: what `00929150`, `00928330`
and `00928380` do, how the think function is actually invoked, and why the garbage-collection
countdown gates untimed entities. This packet answers those and corrects two readings that
the earlier doc took from truncated pseudocode.

## When it runs

| caller | site | `CL` / step | effect |
| --- | --- | --- | --- |
| `00875BB0` fixed-step driver | `00875E64` | `0.05f` (`00D0DE84`) | row 8 of the fan-out, once per fixed step |
| `00874D00` | `00874D22` | `0.05f`, only when `CL == 0` | an extra whole step forced outside the driver |

`00874D00` is `__fastcall void(char)`, body `00874D00-00874DD7`. It re-runs seven of the
fan-out's sixteen rows: `00888230` first, then the think list, then the world-gated block
(`00778450`, `0077EC20`, `00874C90`, `00925F20`, `0076FFC0`) and the ungated tail
(`00926700`, `009273A0`, `00903610`). The `char` in `CL` selects between two modes:

- `CL != 0`: the think list is skipped entirely (`00874D14 TEST BL,BL` / `JNZ 0x00874D27`)
  and every step argument is replaced by `0.0f` (`00874D47 XORPS XMM0,XMM0`), so the pass
  settles newly created entities without advancing simulated time. The three Lua bindings
  that can run inside a script all pass this mode: `00944D8F` (`BSP_LuaBinding_Spawn`),
  `0094530F` (`BSP_LuaBinding_GenerateObject`) and `00896969`
  (`BSP_LuaBinding_LaunchAirBaseSlot`), each `MOV CL,0x1` immediately before the call.
- `CL == 0`: the full pass including the think list with the fixed step. Only `FUN_0046B730`
  uses this mode, twice (`0046B914` and `0046B92C`, each `XOR CL,CL`), after the loop that
  ends at `0046B912`. What that routine is was not read.

That split is the re-entrancy argument for the whole design: the bindings a think function
could reach never re-enter `00929460`, so the walk below only has to defend against
registrations, not against nested walks.

## The two lists and the node

Both globals are the same 0Ch-byte list object. `00928380`'s prologue reads its argument's
`+4h` as the head (`00928387 MOV EDI,[EAX+4]`) and `00928330`'s reads `[ESI]` as the count
(`00928333 CMP dword ptr [ESI],0x0`), so the layout is settled by the producers, not by the
walk.

| offset | field | evidence |
| --- | --- | --- |
| `+0h` | `count` | `00929524 SUB dword ptr [00F89AB0],0x1`; `009283CD ADD dword ptr [ESI],0x1` |
| `+4h` | `head` | `00929466 MOV EAX,[00F89AB4]`; `0092834C MOV [ESI+4],ECX` on the pop |
| `+8h` | `tail` | `0092951E MOV [00F89AB8],ECX` when the erased node had no successor |

| global | list | populated by | drained by |
| --- | --- | --- | --- |
| `00F89AB0` | live think list | `00928380` only, from the pending list | the walk below, `00928300` on entity death |
| `00F89ABC` | pending registrations | `0088A240` from `BSP_Entity_SetThinkScriptName` at `0088A34B` | `00928380` then `00928330`, once per call |

The node is a 0Ch-byte heap block (`00928396 PUSH 0xC` into the allocator at `00BF681B`;
`0088A240` allocates the same size):

| offset | field | evidence |
| --- | --- | --- |
| `+0h` | `prev` | `009294FC MOV ECX,[EAX]` then `00929502 MOV [ECX+4],ESI`, i.e. `prev->next = next` |
| `+4h` | `next` | `0092948B MOV ESI,[EAX+4]`, the cursor advance at `00929539` |
| `+8h` | `entity` | `00929484 MOV ECX,[EAX+8]`, the `this` of every think call |

**The node carries nothing else.** The think function name, the delay flag and the remaining
delay all live on the entity, so a node is only a membership token. That is why `00928380`
can rebuild a node from `*(src_node+8h)` alone (`00928393 MOV EBX,[EDI+8]`).

## The entity fields the walk reads

| offset | meaning | evidence |
| --- | --- | --- |
| `+5Ch` | must be **set**: initialised | `00929487 CMP byte ptr [ECX+5Ch],0x0` / `JZ` to the erase path; written `1` at `00922F4B` inside `00922F30`, one of the routines `BSP_SEntity_InitAll` (`00925F20`) drives |
| `+5Dh` | must be **clear** | `00929490`; written `1` at `009272EF` (in `FUN_00927050`) and `009274CE` (in `BSP_EntityEventQueues_FlushPending`). Meaning unread. |
| `+60h` | must be **clear** | `00929496`. No writer read; meaning unread. |
| `+5Eh` | must be **clear** | `0092949C`. No writer read; meaning unread. |
| `+1D8h` | think function name, must be non-null | `009294A2`; `strdup`ped at `0088A373`, freed and nulled at `0088A35B`/`0088A363` |
| `+1DCh` | delay armed | `009294AB`; set `1` with the delay at `008982D1`, cleared at `0088A37F` |
| `+1E0h` | remaining delay, seconds | `009294B4 MOVSS XMM0,[ECX+1E0h]`; written at `008982CB` |
| `+178h` | Lua self key passed to the named call | `009290A0`, `docs/MISSION_LUA_SELF_TABLE.md` |

`+1D8h` and `+1DCh` are already declared as `kEntityThinkScriptNameOffset` and
`kEntityThinkStateByteOffset` in `include/bsp/lua_binding_core.hpp`; that header's
"always cleared" note for `+1DCh` was true of its own producer and is completed here:
the flag is set by the delay binding at `00898150`.

## Globals and constants

| address | type | value | role |
| --- | --- | --- | --- |
| `00F89A04` | `float` | runtime | script countdown, shared by the untimed thinks and the GC |
| `00D7A2B0` | `double` | `3.0` | countdown refill; `00929557 FADD double ptr [00D7A2B0]` |
| `00CE3800` | `float` | `0.5` | minimum accepted delay in the binding at `00898150` |
| `00D0DE84` | `float` | `0.05` | the fixed step both callers pass |
| `00D19480` | text | `collectgarbage()` | the chunk run when the countdown expires |
| `0109CEFC` | object | singleton | `vtable[+0Ch]` gates the GC; also read by `BSP_ResourceManager_LoadAndCache` at `00B807BC`/`00B809C7`. The predicate's body was not read, so this document names it by slot, never by a verb. |

## The rule, per call (`00929460`, `__cdecl void(float)`, `RET 4`)

1. `00F89A04 -= step` (`00929460 FLD` / `0092946B FSUB float ptr [ESP+4]` / `00929477 FSTP`).
   This happens **before** the walk, so the countdown a node tests already has this step
   subtracted.
2. Walk the live list from `00F89AB4`. The successor is captured into `ESI` at `0092948B`
   before anything else happens to the node, and the loop advances with it at `00929539`.
   For the entity `e = node->entity`:
   - **ineligible** (any field test above fails): unlink the node, `00F89AB0 -= 1`, free it
     (`0092952C CALL 0x00BF65AC`), continue with the captured successor.
   - **timed** (`e+1DCh != 0` and `e+1E0h > 0`): `e+1E0h -= step` (x87, `009294C7..009294D7`);
     if the result is `<= 0` call the think at `009294E5`. The routine never re-arms the
     delay, so from the next call on the entity takes the untimed branch until a script
     re-arms it through `00898150`.
   - **untimed** (`e+1DCh == 0`, or the delay has already run out): call the think at
     `009294F5` **only if `00F89A04 < 0`** (`009294EC COMISS XMM1,[00F89A04]` with `XMM1`
     zeroed at `0092946F`/`00929536`, `JBE` skipping the call).
3. After the walk, if `00F89A04 < 0`: refill it with `+= 3.0` and, when
   `[0109CEFC]->vtable[+0Ch]()` returns a non-zero byte (`00929568 CALL EAX` /
   `0092956A TEST AL,AL`), run `collectgarbage()` through
   `BSP_LuaMachine_RunString(machine, "collectgarbage()", 0, 0, 2)` on
   `*(*(00E188A8)+1A08h)+4h`, the mission Lua host's machine.
4. Splice: `00928380(this = 00F89AB0, src = 00F89ABC)` appends a **new** live node for every
   pending entity, then `00928330(this = 00F89ABC)` frees every pending node. A registration
   made during step *n* therefore first thinks in step *n+1*, and a registration made during
   the walk cannot disturb the cursor.

`ADD ESP,0x8` at `009295A6` balances the `SUB ESP,0x8` scratch at `00929472`; the pushed
`00F89ABC` is consumed by `00928380`'s `RET 4`.

## How the think function is invoked

`00929150` is `__fastcall void(entity in ECX)`, `RET`, body `00929150-009291CF`, and
`00929460` is its only caller (two sites).

1. `00929168 MOV ESI,ECX`, `0092916B MOV EAX,[ESI+1D8h]`; a null name returns immediately,
   so the eligibility test is re-checked here.
2. A stack `NativeString` is built from the `char*`:
   `0041E870 BSP_NativeString_Assign(this = ESP+8, src = e+1D8h)` at `0092917A`.
3. `009290A0(this = entity, name = &that string, args = 0, stack_first = 0, stack_last = -1)`
   at `00929194`, `RET 10h`. Per `docs/MISSION_NAMED_CALL_ARGS.md` this forwards
   `BSP_MissionLuaHost_CallNamedThreadSafe(self_key = entity+178h, name, args, stack_first,
   stack_last)`; `args = 0` skips the argument-vector block and `stack_first = 0` disables
   stack forwarding.
4. The string's buffer is returned to the sized storage pool: `00419CC0(buffer, size+1, 1)`
   at `009291B4`, then `00BD1510` at `009291BB` with that result in `ECX`. The three pushes
   are never popped by the caller - the single `ADD ESP,0x14` at `009291CC` accounts only for
   the 12-byte SEH frame and the 8-byte scratch - so the pair consumes them. This is an
   allocator contract and is not reconstructed here.

**The think function takes no arguments at all** - no self table pushed as a parameter, no
step, no delta. It is reached by *name* through the mission host's thread-safe named call,
scoped by the entity's self key at `+178h`; it is not a stored Lua reference and not a
`lua_pcall` from this module. The name is the string the `SetThink` binding (`00897FB0` ->
`0088A330`) duplicated, so a think function is re-resolved by name on every invocation.

## Why the garbage-collection countdown gates untimed entities

`00F89A04` is one countdown with two consumers, and step 2 above reads it before step 3
refills it. In the call where it goes negative, every untimed think runs **and** the Lua
collector runs, in that order. Between such calls no untimed think runs at all. With
`00D7A2B0 = 3.0` and a `0.05f` step, that is one untimed pass every sixty fixed steps.

So the countdown is not "a GC timer that incidentally also gates thinks": it is the
period of the whole slow script pass. A `SetThink` with no delay is a three-second
heartbeat, not a per-step callback, and the batch of Lua work it generates is immediately
followed by the collection that reclaims it. The clamp in the delay binding
(`00CE3800 = 0.5`) keeps a timed entity at least ten fixed steps apart, so no think
function can run per-step by either route.

## Registration and removal (contracts, read only as far as needed)

| routine | ABI | what it does here |
| --- | --- | --- |
| `0088A330` `BSP_Entity_SetThinkScriptName` | `__thiscall(entity, const char*)`, `RET 4` | on the null-to-name transition only, `0088A34B CALL 0x0088A240` with `ECX = 00F89ABC`; then frees the old name, duplicates the new one into `+1D8h` and clears `+1DCh`. Membership is therefore added once per entity and never removed by a later `SetThink`. |
| `0088A240` | `__thiscall(list*, entity)` | `operator new(0xC)`, `{prev = tail, next = 0, entity}`, appended at the tail, `count += 1` |
| `00898150` | Lua binding, `__fastcall(lua_state)` | reads an entity handle at argument 0 and a number at argument 1, stores `max(number, 0.5f)` at `+1E0h` and `1` at `+1DCh` |
| `00928300` | `__thiscall(list*, entity)`, `RET 4` | linear search for the node whose `+8h` equals the entity, then `00928030`; the return value both call sites ignore |
| `00928030` | `__thiscall node*(list*, node*)`, `RET 4` | unlink, `count -= 1`, free, return the successor (or `0` when the erased node was the tail) |
| `00929800` | entity death path | `00929AA7` and `00929AB2` call `00928300` with `ECX = 00F89AB0` and `ECX = 00F89ABC` and the dying entity, then free `+1D8h` at `00929B18`. Eager removal from both lists. |

## Host table

`this` is the `ECX` the site sets. Rows are in execution order.

| site | callee | name | `this` | args | ret | gate |
| --- | --- | --- | --- | --- | --- | --- |
| `009294E5` | `00929150` | `run_entity_think_00929150` | entity | none | `RET` | timed branch, delay reached zero |
| `009294F5` | `00929150` | `run_entity_think_00929150` | entity | none | `RET` | untimed branch and `00F89A04 < 0` |
| `0092952C` | `00BF65AC` | `free_think_node` | none | node | `__cdecl`, `ADD ESP,4` at `00929531` | entity ineligible |
| `00929568` | `0109CEFC+vtable0Ch` | `gc_gate_predicate` | `[0109CEFC]` | none | byte in `AL` | `00F89A04 < 0` |
| `00929588` | `006B8AD0` | `lua_run_string` | `*(*(00E188A8)+1A08h)+4h` | `"collectgarbage()", 0, 0, 2` | `RET 10h` | gate returned non-zero |
| `00929597` | `00928380` | `splice_pending_into_live_00928380` | `00F89AB0` | `00F89ABC` | `RET 4` | always |
| `009295A1` | `00928330` | `clear_pending_00928330` | `00F89ABC` | none | `RET` | always |
| `0092917A` | `0041E870` | `native_string_assign` | `ESP+8` scratch | `entity+1D8h` | `RET 4` | inside `00929150`, name non-null |
| `00929194` | `009290A0` | `entity_call_named_009290a0` | entity | `&name, 0, 0, -1` | `RET 10h` | inside `00929150`, name non-null |
| `009291B4` | `00419CC0` | `pool_for_block` | none | `buffer, size+1, 1` | leaves the three | inside `00929150`, buffer allocated |
| `009291BB` | `00BD1510` | `pool_return_block` | `00419CC0`'s result | the same three | consumes the three | inside `00929150`, buffer allocated |
| `00874D22` | `00929460` | `run_due_entity_think_00929460` | none | `0.05f` | `RET 4` | inside `00874D00`, `CL == 0` |

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/FIXED_STEP_FANOUT.md` item 8: "The **first** node that fails the eligibility test is unlinked, freed and the routine returns, so at most one dead entry is reaped per step" | the walk continues after the free and reaps **every** ineligible node in one call | `00929531 ADD ESP,0x4` falls into `00929534 TEST ESI,ESI` / `00929539 MOV EAX,ESI` / `0092953B JNZ 0x00929484`. The earlier reading came from the pseudocode truncated by the non-returning `_free` annotation, which has since been repaired for this function. |
| same item: the countdown refill and the GC "when it goes negative" described before the walk | the subtraction is before the walk, the refill and the GC are **after** it, so the untimed branch always tests the pre-refill value | `00929477 FSTP` precedes the loop entry at `0092947D`; `00929542 COMISS` and `0092954B..00929588` follow `00929541 POP ESI` |
| same item: `00F89AB4` described as the list ("`+0h` prev, `+4h` next, `+8h` entity, tail `00F89AB8`, count `00F89AB0`") | `00F89AB0` is the list object and `00F89AB4` is only its head field; the offsets quoted are the node's | `00929592 MOV ECX,0xF89AB0` is the `this` of `00928380`, whose body reads `[ECX]`, `[ECX+4]` and `[ECX+8]` |
| `00929460`'s ledger evidence: `(float)_DAT_00D7A2B0` | the refill constant is a `double` | `00929557 FADD double ptr [00D7A2B0]`, bytes `00 00 00 00 00 00 08 40` = `3.0` |

## `no_ghidra_function`

None: every address in the Addresses line is the start of a Ghidra function. Three of the
bodies stop at a `CALL` to the CRT free helper that Ghidra marks non-returning, so the
listing has gaps the pseudocode turns into early returns. The bytes were read directly:

| function | gap (inclusive) | bytes | meaning |
| --- | --- | --- | --- |
| `00928330` | `0092836C-00928373` | `83 C4 04 83 3E 00 75 C4` | `ADD ESP,4` / `CMP [ESI],0` / `JNZ 0x00928338`: the pop is a **loop**, so `00928330` clears the whole list rather than removing one node as its pseudocode shows |
| `00928030` | `0092805F-00928068` | `83 C4 04 8B C6 5E C2 04 00` | `ADD ESP,4` / `MOV EAX,ESI` / `POP ESI` / `RET 4` |
| `00928030` | `00928076-0092807D` | `83 C4 04 8B C6 5E C2 04` | the same epilogue on the tail path |
| `0088A330` | `0088A360-0088A36C` | already recorded in `docs/LUA_BINDING_CORE.md` | `ADD ESP,4` / `MOV [ESI+1D8h],0` |

An integrator can clear these with `python tools/ghidra_flow_repair.py <fn> --apply`.

## Coverage

| routine | coverage |
| --- | --- |
| `00929460` | complete: `00929460-009295AB`, every branch and both callers |
| `00929150` | complete: `00929150-009291CF`, the allocator pair described as a contract |
| `00928380`, `00928330`, `00928300`, `00928030`, `0088A240` | complete, including the listing gaps above |
| `00874D00` | partial: the think-list decision and the mode argument are complete; the other nine rows it re-runs are `docs/FIXED_STEP_FANOUT.md`'s and were not re-read (`00874D27-00874DD7`) |
| `00898150` | partial: only the two entity writes and the clamp; the Lua argument marshalling and the result count (`00898150-008982C0`) are `docs/LUA_BINDING_CORE.md`'s |
| `00929800` | not covered; read only for its two `00928300` sites |

## Open questions

- `+5Dh`, `+5Eh` and `+60h` gate eligibility and no producer body was read. `+5Dh` is
  written inside `FUN_00927050` and `BSP_EntityEventQueues_FlushPending`, which suggests a
  pending-destruction marker, but that is a guess and the document does not use it.
- `[0109CEFC]->vtable[+0Ch]`'s body: the singleton is shared with the resource manager's
  load-and-cache path, so "do not collect while streaming" is plausible and unproven.
- A think function that destroys **another** entity reaches `00929800`, which frees that
  entity's live node through `00928300`. If the freed node is the successor the walk
  captured at `0092948B`, the cursor is dangling. No call site was found that does this, and
  the deferred registration path shows the author thought about mutation during the walk, but
  the erase path is not deferred.
- Run-time evidence was not gathered: `bsp_game.exe` does not reach the in-mission fixed step
  (`docs/GAME_EXECUTABLE.md`), so checklist rule 6 does not apply to any claim here.
