# The mission Lua named call: arguments, result depth and the deferred queue

Addresses: 00887750 00887e50 00887b30 00887220 008874c0 00887500 00886e00 008875d0 00885a30 006edf00 006b88e0 006b85f0 00887c30 00887560 008876b0 00885350 004c1570 00888230 008c8390 0045f440 005b8c30 00452360 009290a0

Packet `cc2_named_call_args`, worktree `agent/cc2-named-call-args`. Ghidra was read-only: no
rename, comment, prototype or save was made from this packet. Every name here is a hypothesis,
not a recovered symbol.

`docs/MISSION_LUA_HOST.md` recovered the shape of the named call and `docs/MISSION_LUA_SELF_TABLE.md`
recovered argument 1. Its open item was "`00887750`'s arguments 2 through 7 beyond the self key were
not attributed individually". This packet attributes all seven from the stack slots, settles what
the `mode` of the result collection is, reads the deferred queue that makes `00887E50` thread safe,
and identifies `008874C0`.

## The seven arguments of `00887750`

`__thiscall(MissionLuaHost* this, ...)`, `RET 1Ch`, so seven stack arguments. The prologue pushes
three dwords of SEH state (`00887750`-`0088775E`), subtracts `18h` and pushes `EBX`/`ESI`, so the
return address sits at `[ESP+2Ch]` and argument *N* at `[ESP+2Ch+4N]`. `PUSH EBP` at `008877BF` and
`PUSH EDI` at `008877C6` move the same slots to `[ESP+34h+4N]` for the rest of the body.

| # | slot | slot after EBP/EDI | type | first read | optional | meaning |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | `[ESP+30h]` | `[ESP+38h]` | `const NativeString*` | `008878C7` | yes, `0` is passed by four of the five call sites | self key; the block runs only when the pointer is non-null **and** the size word `[EDI]` is non-zero |
| 2 | `[ESP+34h]` | `[ESP+3Ch]` | `const NativeString*` | `0088785A` | no | the dotted entry-point name, `ECX` into the `'.'` splitter `00BD20A0` |
| 3 | `[ESP+38h]` | `[ESP+40h]` | `const ArgVector*` | `0088790F` | yes, `0` skips the whole block | `{+0h first record, +4h count}`, records of `14h` bytes |
| 4 | `[ESP+3Ch]` | `[ESP+44h]` | `int` stack_first | `008877E8` | `0` disables stack forwarding | absolute or negative Lua index |
| 5 | `[ESP+40h]` | `[ESP+48h]` | `int` stack_last | `00887812` | read only when argument 4 is non-zero | absolute or negative Lua index |
| 6 | `[ESP+44h]` | `[ESP+4Ch]` | `ResultSink*` | `008877C0`, `008879A5` | yes, `0` discards results | 14h-byte sink, see below |
| 7 | `[ESP+48h]` | `[ESP+50h]` | `char` force | `00887782` | — | skips the `game+1FE4h == 2` gate; the slot is then reused at `0088784A` to hold the error-handler index |

Argument 7 is read exactly once. `0088784A MOV dword ptr [ESP+50h],EAX` overwrites the same slot
with `lua_gettop` after the `debugtrap` lookup, and `00887971` reads it back as the `errfunc` index
for the pcall. Nothing else reads the force byte, so a caller cannot observe it after entry.

### Index normalisation (`008877E8`-`0088782A`)

Both indices are normalised as `top + index + 1` when negative, which maps `-1` to the current top.
The normalisation is **asymmetric**: the whole block is gated on `stack_first != 0`
(`008877EC CMP EBP,EBX` / `008877F2 JZ`), so when `stack_first` is zero `stack_last` is never
normalised and never used. `008877F4` issues a `lua_gettop` whose result is discarded before the
sign test; it is dead in the listing, not a reading error.

Step 10 is gated the same way (`00887943 TEST EBP,EBP`) and additionally by
`stack_first <= stack_last` (`0088794B`), so an empty or inverted range pushes nothing.

### The pushed-argument count

`EBX` is both the zero constant of the function and the `nargs` accumulator. It is cleared at
`00887780` and is set to `1` **only inside the self-key block** (`008878DC MOV EBX,1`). When
argument 1 is null or empty, `nargs` therefore starts at **0**, not at 1. `docs/MISSION_LUA_HOST.md`
step 8 says "`nargs` starts at 1"; that holds only for the self-key path. Recorded before this
correction: "`nargs` starts at 1."

After that, `0088793C ADD EBX,[ECX+4h]` adds the record count verbatim and
`00887953 LEA EBX,[EBX+EAX+1]` adds `stack_last - stack_first + 1`. The record count is the
container's count, not the number of values actually pushed: a tag-4 record pushes nothing
(`docs/MISSION_LUA_HOST.md`, `00885DA0`), so a light-userdata record makes `nargs` exceed the real
stack. The reconstruction models the two counts separately.

### What the five call sites pass

| Site | Containing function | Callee | this | argument 1..5 | gate |
| --- | --- | --- | --- | --- | --- |
| `0045F4DF` | `0045F440` `BSP_Game_CallLuaEntryPointThreadSafe` | `00887E50` | `[game+1A08h]` | `0`, local `NativeString(name)` at `[ESP+18h]`, the caller's `args` from `[ESP+50h]`, `0`, `-1` | `00B66200` "global is defined and not nil" |
| `005B8D1C` | `005B8C30` `BSP_VoiceManager_UpdateSoundFade` | `00887E50` | `[game+1A08h]` | `0`, local `NativeString` built at `005B8CEA`, `0`, `0`, `-1` | `BL` at `005B8CD3`, the fade-complete flag |
| `004525D1` | `00452360` `BSP_PanelSequence_ConsumeCommands` | `00887E50` | `[game+1A08h]` | `0`, `command+8h`, `0`, `0`, `-1` | the command's virtual `+4h` must return `4` (`004525B0 CMP EAX,4`); the site also sets `00E17BFA` to 1 |
| `00929139` | `009290A0` `BSP_MissionEntity_RegisterLuaScript` | `00887E50` | `[game+1A08h]` | `entity+178h`, and its own four arguments forwarded unchanged | none on this path |
| `00888264` | `00888230` `BSP_MissionLuaHost_DrainQueuedCalls` | `00887E50` | the host itself | `node+8h`, `node+10h`, `node+18h`, `node+24h`, `node+28h` | `[host+10h] != 0` |
| `00887B4D` | `00887B30` `BSP_MissionLuaHost_CallNamedForced` | `00887750` | forwarded | `a1..a5`, then `results = 0`, `force = 1` | none |
| `008C843E` | `008C8390` `BSP_LuaBinding_DebugTrap` | `00887750` | `[game+1A08h]` | `0`, `"debug.traceback"`, `0`, `0`, `-1`, `&sink`, `0` | none |

`00929139` is the only site that passes a self key and the only site that forwards a stack range;
`009290A0` is `__thiscall(entity, const NativeString* name, const ArgVector* args, int stack_first,
int stack_last)`, `RET 10h`, and pushes `[ESP+28h]`, `[ESP+24h]`, `[ESP+20h]`, `[ESP+1Ch]` and
`entity+178h` at `0092911E`-`00929138`. The ledger evidence for `009290A0` said the third argument
was the literal `0`; it is the caller's `args` pointer. Recorded before this correction:
"Forwards (entity+178h as the self key, the name NativeString, 0, the first stack index, -1)".

`EBP` is zero at the panel site: the function zeroes it at `00452538` and the whole listing filtered
for `EBP` shows no write between there and `004525D1` (next write is the `POP EBP` at `0045261D`).

`ghidra xrefs 00887E50` reports 42 call sites and printed 40 of them. This packet read five:
`0045F4DF`, `005B8D1C`, `004525D1`, `00929139` and `00888264`. The 35 printed but not read are
`005B9969`, `005BC394`, `00734176`, `0073521F`, `0074FE89`, `007912E0`, `00906453`, `00920B9B`,
`00947B81`, `0096D5A9`, `00981171`, `009819AB`, `00981D6C`, `0098239B`, `009828BC`, `00982B7C`,
`00982F3C`, `00983560`, `009839F0`, `00983E7A`, `0098423A`, `00984710`, `00984ACA`, `00984DEA`,
`0098517A`, `009854BE`, `0098582A`, `00985B8A`, `00985E59`, `0098639E`, `0098672A`, `00986A2C`,
`00986EAA`, `00888F5D` and `0098952A`; two more were not printed by the listing. Coverage of the
call-site contract is therefore **partial: 5 of 42**. Every site read passes the same shape, and
the one that differs (`00929139`) is the entity path.

## The result sink and what "mode" really is

`00887220 BSP_MissionLua_CollectStackResults` is `__thiscall(ResultSink* this, int count, int mode)`,
`RET 8`, body `00887220`-`0088736A`. `ECX` is saved into `EDI` at `0088723D`; `[EDI+4h]` is the Lua
machine it calls `lua_gettop` on, `EDI+8h` is the vector it appends to.

```
top   = lua_gettop([this+4h])                 ; 00887262
first = count < 1 ? 1 : top - count + 1       ; 0088726B..0088727C
for i in first..top:
    LuaVariant tmp;                           ; vtable 00D0E6F4, tag -1
    00886E00(this, &tmp, i, 0, mode)          ; 008872BF
    006EDF00(this+8h, &tmp)                   ; 008872CB, push_back
    00886920(&tmp)                            ; 008872E1, destroy
```

It never pops: the caller restores the top (`008879BF lua_settop(saved_top)`).

`00886E00` is `__thiscall(ResultSink* this, LuaVariant* out, int stack_index, int depth,
int max_depth)`. Its entire body is guarded by `depth < max_depth`, and the table case recurses at
`00886F97` with `depth+1` (`00886F02 LEA EBP,[EDI+1]`, `EDI` = the incoming depth) and the same
`max_depth` reloaded from `[ESP+F8h]` at `00886F77`. **The third literal `00887220` receives is not
a mode selector; it is the maximum table-expansion depth**, and `00887220` passes it straight
through with depth 0.

| Caller | count | depth limit | meaning |
| --- | --- | --- | --- |
| `006B89F0` chunk runner | native | 2 | a table result expands its members, and a member table expands one level |
| `008879B3` named call | `lua_gettop - saved_top` | 4 | three levels of nested tables |
| `008874C7` pending-results scope | `-1 - base` | 4 | same limit as the named call |

At `depth == max_depth` the record is left exactly as `00887220` constructed it: vtable `00D0E6F4`,
tag `-1`, three zero dwords. That "unset" tag is distinct from tag 6 (nil), so a caller can tell a
truncated table member from a real nil.

### What a record holds per Lua type

`00886E00` switches on `006B7EC0` (the machine's `lua_type`) and writes the same 20-byte variant the
argument records use (`MissionLuaArgumentType` in `include/bsp/mission_lua_host.hpp`), tag at `+4h`:

| `lua_type` | Lua type | reader | setter | resulting tag |
| --- | --- | --- | --- | --- |
| 0 | nil | — | `00886920` reset then `[out+4h] = 6` | 6 Nil |
| 7 | full userdata | — | same as nil | 6 Nil |
| 1 | boolean | `006B8000` | `008869F0`, `CVTSI2SS` | **0 Number**, 0.0 or 1.0 |
| 2 | light userdata | `006B80A0` | `00886B30` | 4, the tag `00885DA0` cannot push back |
| 3 | number | `006B8020` | `008869D0` | 0 Number |
| 4 | string | `006B8060` | `00886A10` | 1 String |
| 5 | table | `lua_next` walk `006B85B0` | per entry | 5 Table |
| 6 | function | the literal `"<Function>"` at `00D0E78C` | `00886A10` | 1 String |
| other | `LUA_TNONE`, thread | `006B86B0(1, buf)` assertion | — | — |

A boolean result is therefore indistinguishable from the numbers 0 and 1 once collected, and a
function is indistinguishable from a script that returned the string `"<Function>"`. The table walk
pushes nil, calls `006B85B0` repeatedly, reads each key as a number (`006B8040`) or a string
(`006B8060`) according to `006B7F60`, appends it through `007814E0`/`00781430`, builds the value
recursively and pops one slot with `006B85F0`. Those four appenders and `006EE0D0`/`006EE4E0` were
not read; contract: unread.

### `ResultSink`, 14h bytes

| Offset | Field | Evidence |
| --- | --- | --- |
| `+0h` | vtable `00D0E798` | constructor `008875D0` |
| `+4h` | `LuaMachine*` | `008877DD MOV [EBP+4h],EDI` with `EDI = [host+4h]`; `0088725B` reads it for `lua_gettop` |
| `+8h` | `LuaVariant* begin` | `006EDF00` `param_1[0]` |
| `+Ch` | `int size` | `006EDF00` `param_1[1]`, `00885A33` reads it |
| `+10h` | `int capacity` | `006EDF00` `param_1[2]`, doubling growth through `006B87F0` |

`00887750` does not clear the sink: when `[sink+4h]` already holds a machine it resizes the item
vector to zero (`008877D5` `006B88E0(sink+8h, 0)`), then stores the host's machine. A sink reused
across two calls without that pre-existing machine keeps the first call's items. `006B88E0` destroys
surplus elements through vtable slot 0 with argument 0 (non-deleting) and `006EDF00` appends with a
`14h` stride.

Exactly two sinks are constructed in the image (`ghidra xrefs 008875D0`): `008C83DA` in the debug
trap, which is the only sink a named call ever fills, and `004276AD` in `00427190`, the console
command path, which fills it from the chunk runner. `00885A30` is the only reader:
`__thiscall(ResultSink*, NativeString* out)`, it walks the `14h`-byte elements and appends each
through `008855B0` (contract: unread). So every named-call result in the shipped game ends up as
text in the debug trap's log line.

## `008874C0`

`__thiscall(PendingResults* this)`, body `008874C0`-`008874FE`, the non-deleting destructor of a
16-byte RAII object:

| Offset | Field |
| --- | --- |
| `+0h` | vtable `00D0E6F0` |
| `+4h` | `ResultSink*` |
| `+8h` | `int base`, negative |
| `+Ch` | `char armed` |

When armed it re-fetches the host through `[[00E188A8]+1A08h]`, calls
`00887220(sink, -1 - base, 4)` and pops `-base` values with `006B85F0`, so it collects *N* results
and drops *N+1* stack slots, the extra slot being the called function itself.

It is unreached. The vtable constant `00D0E6F0` is written at exactly two addresses
(`ghidra xrefs 00d0e6f0`): `008874C7` and `00887507`, both inside the two destructors themselves.
No constructor, inline or otherwise, installs that vtable, and `008874C0` has no xrefs at all;
`00887500` survives only because it is slot 0 of the vtable. Treat it as the emitted-but-never-
instantiated "deferred results" holder that documents the intended asynchronous form of the named
call, not as live behaviour.

## `00887B30`: what it forces

`00887B30`-`00887B54`, `RET 14h`, thirteen instructions. It reloads its five stack arguments and
calls `00887750(a1, a2, a3, a4, a5, 0, 1)`; `ECX` is untouched, so `this` passes through. What the
`1` buys is the `00887782` gate only: the call runs even while `game+1FE4h == 2`. Three further
differences follow from the constants rather than the flag: it never queues (it does not consult the
frame job pool), it never collects results (argument 6 is `0`), and it runs on the calling thread.
Its only caller is `0045F520`.

## `00887E50`: the marshalling and the queue

`00887E50`-`00888227`, `__thiscall(this, self_key, name, args, stack_first, stack_last)`, `RET 14h`.
Three paths:

1. **Refuse.** `game+1FE4h == 2` at `00887E73` returns with nothing done.
2. **Inline.** `004C1130` then the frame job pool's virtual `+10h` (an indirect call, target not
   resolvable statically; contract: unread) returns zero. The body then saves `[0109CEEC]+14h` into
   a two-dword scope object `{vtable 00D0E6EC, saved}`, re-tests `game+1FE4h == 2` at `00887EC8`,
   and runs the `00887750` body verbatim with no sink and no force. On the way out `00885350`
   compares `[0109CEEC]+14h` with the saved value and, when it grew, constructs the empty string
   at `00CE3A0C` and calls `00BDC9B0 BSP_VFS_LeaveFileBlock` on `[0109CEEC]`. That object is a
   **file-block nesting unwinder, not a mutex**: it repairs a virtual-file-system block the script
   opened and did not close. `00887750` builds the same object at `00887774` but only releases it on
   the refusal path (`008877A7`).
3. **Queue.** The pool says worker thread. `004C1570` lazily creates an 8-byte singleton over
   `00F878FC` whose `+4h` is a `CRITICAL_SECTION`; `00888120` enters it through `[00CE2218]` and
   bumps the recursion count at `cs+18h`, and `0088820D` leaves through `[00CE2210]`. Everything
   between is one `std::list` push_back and a deep copy.

### The deferred call record

`this+8h` is the list object, `this+0Ch` its head node and `this+10h` its size (the drain
`00888230` reads the same three). `00887C30(list, [head+4h], &empty)` allocates the node, the caller
splices it so it becomes the last element (`00888171 MOV [EDI+4h],EBP` with `EDI` = the head), and
`00885F10(1)` increments the size. The payload is then filled in place, `EBP = node+8h`:

| Offset | Field | Written at | How |
| --- | --- | --- | --- |
| `node+8h` | `NativeString self_key` | `008881A0` | `00425F40(node+8h, arg1)`, skipped when arg1 is null |
| `node+10h` | `NativeString name` | `008881B7`, `008881CB` | `0041DD40` resize to `[arg2]`, then `00BF7680` memcpy of `[arg2+4h]` |
| `node+18h` | `LuaVariant vector` | `008881ED` | `00887560(node+18h, arg3)`, skipped when arg3 is null |
| `node+24h` | `int stack_first` | `008881E4` | copied verbatim from `[ESP+6Ch]` |
| `node+28h` | `int stack_last` | `008881DB` | copied verbatim from `[ESP+70h]` |

The field order is confirmed independently by the drain, which calls
`00887E50(node+8h, node+10h, node+18h, node[9], node[10])` at `00888264`. `NativeString` is
`{+0h size, +4h data}`, which is why the self-key test at `008878CD` reads `[EDI]`.

`00887C30` is `__thiscall(list, next, prev, const Record* value)`: `operator new(2Ch)`, store the two
links, then copy-construct the payload at `node+8h` from the value through `00887A10`. The `2Ch`
confirms the layout above: 8 bytes of links and a `24h`-byte payload.

`00887560` is `__thiscall(vector* dst, const ArgVector* src)`: `resize(0)`, reserve `src->count`
through `006B87F0`, then `006EDF00` for each `14h`-byte record. The arguments are therefore **deep
copied at queue time**, so a caller may free its records immediately.

`008876B0` is the record destructor and its whole `39h`-byte body is `006B88E0(record+10h, 0)` and
`00BF6989` on that vector's buffer. It never releases the two `NativeString` buffers at `record+0h`
and `record+8h`: there is no `0041DD20` and no `00419CC0`/`00BD1510` pair in it. On the queue path
that is harmless, because the record it destroys there is the empty temporary, but `00888230` calls
the same destructor on the filled node before freeing it, so **every deferred call leaks its name
and its self key**. That is a static reading of a short body, not a measured leak; no run was made.

**The queue is FIFO and the two stack indices are a hazard.** The drain pops the front, so order is
preserved, but `stack_first`/`stack_last` are stored as plain integers and replayed on the main
thread's Lua stack one fixed step later. The only caller that passes a non-zero `stack_first`
(`009290A0`, forwarded from `00898750 BSP_LuaBinding_CreateScript`) runs inside a Lua binding, so on
a worker thread its forwarded range would name slots of a stack that no longer exists. No run-time
evidence was collected for that path; it is a static reading of the copy.

So "thread safe" means **the queue and its critical section**, not any lock on the Lua state: the
interpreter is only ever entered from the draining thread.

## Coverage

| Address | Name | Coverage |
| --- | --- | --- |
| `00887750` | `BSP_MissionLuaHost_CallNamed` | complete, all seven arguments attributed |
| `00887E50` | `BSP_MissionLuaHost_CallNamedThreadSafe` | complete for the queue path and the gates; the inline body is the `00887750` reading |
| `00887B30` | `BSP_MissionLuaHost_CallNamedForced` | complete |
| `00887220` | `BSP_MissionLua_CollectStackResults` | complete |
| `00886E00` | variant builder | partial: the table case's four appenders `007814E0`, `00781430`, `006EE0D0`, `006EE4E0` were not read |
| `008874C0` | pending-results destructor | complete, and unreached |
| `00887E50` call sites | — | partial: 5 of 42 read |

## no_ghidra_function

none. Every address above is inside a Ghidra function; `008874C0` has one
(`FUN_008874c0`, body `008874C0`-`008874FE`), as do `00887500`, `00886E00`, `008875D0` and
`00885A30`, checked with `python tools/bsp.py ghidra proto <addr> --brief`.

## Open questions

- The frame job pool's virtual `+10h` (`00887E86`) was not resolved; the "am I a worker thread"
  reading is inherited from `docs/MISSION_LUA_HOST.md`.
- `008855B0`, the per-element text appender `00885A30` uses, was not read, so how a table result
  prints is unknown.
- `MissionLuaHostServices::collect_results(int count, int mode)` in
  `include/bsp/mission_lua_host.hpp` calls the third argument a mode. It is a depth limit. That
  header is outside this packet's lease and was left alone; the comment should be corrected by
  whoever owns it.
- Whether any of the 37 unread `00887E50` sites passes a self key or a stack range was not
  established.
- No run-time evidence was taken. `bsp_game.exe` reaches `00887E50` on the mission path, so the
  queue-versus-inline branch and the stack-forwarding hazard are candidates for a logged run.
