# Actual Lua bootstrap and shader combiner dependencies

Addresses: 00b6a020, 00b669c0, 00b437f0, 00b439c0, 00b660a0, 00b66330

These six routines now have implementations using the established actual Lua owner,
Lua object and native string storage. Descriptive names remain hypotheses, not recovered
symbols. The interfaces require explicit services and stack inputs; they are not binary
replacements for the original ABI. See `reports/native_shader_descriptor_dependencies.json`.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| B6A020 open | ECX actual 4C8h owner; stack library mask; RET4 | Complete, 823 bytes |
| B669C0 panic | ECX Lua state; EAX0; RET | Complete, 23 bytes |
| B437F0 combiner entry | ECX unused; stack string slots and Lua entry; RET8 | Complete, 449 bytes |
| B439C0 combiner table | ECX descriptor passed through; stack slots, Shader, field string; RET0C | Complete, 320 bytes |
| B660A0 string predicate | ECX actual 14h object; AL boolean; RET | Complete, 68 bytes |
| B66330 float/default | ECX actual 14h object; stack float; ST0 result; RET4 | Complete, 66 bytes |

## Bootstrap behavior

`open_native_lua_state_00b6a020` writes owns00, replaces state04 with a new Lua state,
installs the panic callback and opens libraries in the native table order. Base always
opens, including mask zero; bits 2,4,8,10h,20h,40h,80h select package, table, io, os,
string, math and debug respectively. Each opener gets its name and discards results.
The initial stack top is stored at owner08, retained under the existing `opaque_08`
member name. Tracking slots, stack offset and high-water fields are not reset. An old
state is not closed before replacement.

PC, X360COMP and case-insensitive EU/USA/JAP REGION assignments use native temporary
strings and protected calls only after successful loading; their statuses are ignored.
X360COMP reads the live byte after the PC temporary is released. Region data is read
again before each comparison. Null, empty and unknown region names produce no assignment.
Normal PC/X360/EU/USA cleanup uses captured data/length; JAP uses captured data and the
current temporary length. The reconstructed constant-string construction uses the
existing 41E870 helper rather than the native inline constant-length copy.

DoFile is an externally supplied real callback with **zero upvalues**. The fundamentals
getter is also required. B68340's inspected producer establishes its readable 0Ch layout:
vtable00, allocation pointer04, low length DWORD08. The bootstrap calls the getter twice,
capturing length from the first result and bytes from the second. It ignores the
`luaL_loadbuffer` status and calls `lua_call(0, LUA_MULTRET)` without protection. This
implementation does not invent a cache owner, VFS fallback, rollback or state-null check.
Missing required host services are rejected at the C++ interface boundary.

The panic callback calls `lua_tolstring(state, lua_gettop(state), nullptr)` and returns
zero. It discards the returned pointer; **it does not remove the Lua stack value**.
A numeric top value is converted to a string in place. Fatal process termination was
not exercised in this packet.

## Combiner behavior

The inner reader iterates actual Lua key/value objects. Integral-number keys participate,
but the ordinal advances on **every** entry, including rejected keys. Ordinal zero selects
mode using exact NUMBER/default13; ordinal one selects exact STRING/default empty. Later
entries are ignored. A missing ordinal-zero assignment leaves the native mode local
uninitialized. `NativeShaderCombinerStackInputs` supplies this native stack preimage
explicitly; its callback is a host input boundary, not a recovered game callback.

The destination is `slots + mode * 8` with 32-bit arithmetic and no bounds check. Callers
must provide an index yielding a valid native string header. Existing slots remain;
duplicate modes overwrite their previous names. Embedded NUL names follow the existing
C-string constructor. The local name length is cached after its copy. Destination resize
uses that length; final copy reads the current local pointer after resize and uses the
current destination length. Normal local cleanup uses cached length/current pointer;
exception cleanup uses the current header.

The outer reader performs field lookup, TABLE gate, releases that object, then looks up
the field again. It terminates on the value object and accepts integral-number outer
keys only. The field's NativeString length does not replace C-string lookup semantics.
Missing, Boolean and empty tables preserve all destination slots. Normal teardown releases
value, key, table. The four inner unwind states clean key, value, local string and temporary
string; four outer states cover gate, table, key and value. Their metadata/thunks were
checked, but original exception execution was not tested. The B43A7A..B43A7F gap follows a
jump and is unreachable alignment, not a missing call-return tail.

B660A0 accepts only kind2 with exact LUA_TSTRING. B66330 accepts only kind2 with exact
LUA_TNUMBER and spills the conversion to float32 before returning; all other cases load
the supplied fallback. Numeric strings do not pass either numeric gate.

## Verification and follow-up

One ignored native differential fixture relocates all six complete original bodies and
uses stock Lua 5.1.1 through register/stack adapters. It retains 31 prior dependency bodies
for existing adapter reuse; only reached dependencies execute. Library table entries are
mapped to the corresponding stock openers. The panic callback has a C-to-native ABI
trampoline; no Lua VM behavior was reimplemented.

- 36 paired bootstrap cases (72 executions): three masks, six region inputs, two platform
  bytes, full 1224-byte owner preservation, old-state survival, two distinct fundamentals
  views, multiple return values and a zero-upvalue DoFile call.
- Eleven Lua values across five object kinds for string/float helpers, including numeric
  strings, signed zero, a value rounded by float32, NaN and infinity. Panic stack/coercion
  behavior is compared directly.
- Nine combiner shapes under both integer conversion modes, explicit native stack seed,
  duplicate modes, rejected keys, fresh metatable lookup, absent tables, retained slots,
  embedded NUL strings and matching native allocation/release traces.
- Installed debugshader modes 0,1,3,4 and alphablend mode7 all yield `dummy.shfx`, followed
  by actual descriptor string cleanup. DoFile loads the installed loose dx9_lua.inc. The
  supplied fundamentals fixture script is not the game's cached fundamentals asset.
- 31 code/data spans match the installed PE initial image; eight unwind states and the
  native library table/literals are checked. Win32 `/W4 /WX /fp:strict` build and both
  existing CTests pass. No permanent tests were added.

Full descriptor B43B00 still needs the actual Lua file/chunk path B69D40/B66CA0 and its
VFS/stream contracts before integration. The existing projected Lua owner is not the
matching actual owner. Cache ownership 00884770/B68340, original exception ABI, arbitrary
callback-driven header mutations, final shader publication/compilation and rendering or
gameplay validation remain open. This packet does not connect these routines to the
running executable or claim a runnable game rebuild.
