# Actual Lua chunk, file and DoFile loading

Addresses: 00b66ca0, 00b69d40, 00b69e00, 00b679b0, 00b67720, 00b677e0, 00426520, 00427110, 004d0fa0

Nine routines now operate on actual native Lua owner/object, string and vector storage.
Their C++ interfaces still require application services and are not binary ABI/SEH
replacements. Descriptive names are hypotheses, not recovered symbols. Evidence is in
`reports/native_lua_file_loading.json`; prior bootstrap work is in
`docs/NATIVE_SHADER_DESCRIPTOR_DEPENDENCIES.md`.

| Routine | Original ABI | Complete PE bytes | Coverage |
| --- | --- | ---: | --- |
| B66CA0 chunk | ECX owner; stack path/flag; RET8 | 316 | Complete |
| B69D40 file + overrides | ECX owner; stack path/flag; RET8 | 165 | Complete; stored Ghidra tail incomplete |
| B69E00 DoFile | ECX Lua state; EAX0; RET | 280 | Complete |
| B679B0 call frame | ECX owner; stack output; EAX output; RET4 | 52 | Complete |
| B67720 index lookup | ECX object; stack output/index; EAX output; RET8 | 188 | Complete |
| B677E0 argument wrapper | Same as index lookup | 32 | Complete |
| 426520 vector reserve | ECX 0Ch header; stack signed capacity; RET4 | 290 | Complete |
| 427110 vector resize | ECX 0Ch header; stack signed count; RET4 | 120 | Complete |
| 4D0FA0 vector destroy | ECX header; RET | 23 | Complete; stored Ghidra tail incomplete |

## Chunk execution and external ownership

`NativeLuaFileServices` supplies the live VFS manager publication, an actual-header override
append service and the application's zero-upvalue DoFile callback. Manager and stream
tables must contain callable original-ABI functions. This packet implements the consumer;
it does not resolve numeric vtables or supply a substitute VFS/stream owner.

B66CA0 reads the current manager and calls table+04 with the NativeString path and mode2.
A null stream, false stream+18 result, or zero 64-bit stream+30 length returns immediately.
In particular, the latter two branches do **not** release the returned reference.

The length is read again for allocation (low DWORD), then again for the requested read
count (low DWORD). The stream table pointer is captured before the third length read;
read target+24 is fetched from that captured table afterward. The read receives a count
output pointer, but the output count is ignored. No zero initialization of the allocation
or short-read recovery is added.

Only the flag's low byte controls decoding. A prefix is replaced with spaces through the
first byte01 inclusive; subsequent bytes rotate four bits. A missing marker turns the
whole inspected range into spaces. The decoder rereads length before the loop and after
each byte. The incremented 32-bit index is sign-extended and then compared as unsigned64
against the live length. The valid buffer extent remains a native caller/provider contract.

Before loading, the path data pointer is captured (null means empty chunk name), then
length is read once more. The current owner's Lua state is read after that callback.
`luaL_loadbuffer` status is ignored. The stream reference DWORD at+04 is atomically
decremented; zero dispatches current table+00 with no stack arguments. The PE import at
CE2220 is verified as `KERNEL32.dll!InterlockedDecrement`. Only then does the code reread
owner.state04 and perform unprotected `lua_call(0, LUA_MULTRET)`. The buffer is freed only
after successful return. No rollback or protected execution is inserted.

## File and callback composition

B69D40 runs the initial chunk before constructing an empty actual string vector. It then
reads the **current** VFS publication and calls BDEF90 through the required service.
After append returns, it captures the vector begin/end and runs each 8-byte path in order,
including duplicates. It neither snapshots path values nor recomputes the end between
calls. Normal and C++ exception cleanup resize to zero and free the current backing.
The BDEF90 producer itself and actual VFS open/stream owners remain external.

DoFile creates the actual borrowed 4C8h owner and reinstalls the application's callback
with zero upvalues. B679B0 writes kind3/index1/top-at0C/untracked. B677E0 delegates to
B67720 with index0, producing the argument reference. Every non-kind2 source produces an
untracked kind2 reference at base+index; kind2 performs a real numeric-key Lua table lookup,
then registers the actual output pointer in the owner's tracking slots. Padding is retained.

The callback converts argument1 with `lua_tolstring`, passes the result to the existing
unchecked NativeString constructor, and calls B69D40 with flag0. It returns zero Lua
results even when scripts return values. Normal argument cleanup does not clear its local
kind; frame cleanup does. The normal borrowed-owner tail only conditionally closes and
does not clear the state field, whereas the EH owner cleanup calls B669A0. Required host
services are validated at the new C++ boundary; invalid Lua argument behavior is not replaced
with a friendly error.

## Actual string vector

`NativeStringVectorStorage` reuses the existing `NativeMeshWeightNamesStorage` declaration:
data/count/capacity, 12 bytes, with 8-byte length/data elements. Reserve clamps to at least
one, allocates shared-heap backing and deep-copies pooled strings while rereading the live
source count/data. It then releases old names forwards, frees current old backing, and
publishes the new pointer/capacity. Its sole unwind action calls 401130, a bare RET, so
there is no allocated-copy rollback on failure.

Resize grows through that reserve and zeros each new header. Shrink decrements the live
count before releasing each name in reverse order. Destruction resizes to zero and frees
backing, leaving the pointer/capacity fields stale. These full helpers supersede the earlier
no-grow-only fragment for consumers needing growth; existing fragment interfaces remain.

## Validation and remaining work

One ignored Win32 fixture executes all nine complete original bodies against the C++
implementations using stock Lua 5.1.1 through ABI adapters. It retains 31 prior dependency
bodies for adapter reuse; only reached dependencies execute. External fixture manager,
stream and override services are the same on both sides.

- Actual vector growth/deep copies, minimum capacity, shrink/regrowth and stale destruction
  headers agree, including native string allocation/release traces.
- Call frames, untracked argument references across four kinds/four offsets, metatable
  table lookup and actual 1224-byte tracking-owner snapshots agree.
- Missing/closed/empty streams retain the expected references. Shared references, multiple
  Lua returns, low-byte flags, marker/no-marker decoding, repeated 64-bit lengths with low
  DWORD consumers, ignored reported-zero read count, captured read table and post-release
  owner-state reload all agree.
- Ordered duplicate overrides, a changed current manager and nested zero-upvalue DoFile
  agree. Installed debugshader/alphablend and their nested dx9_lua.inc load through the
  actual Lua routines into Shader tables. Fixture streams supply their installed bytes;
  this does not validate native physical/archive VFS routing.
- Four child processes exercise syntax/runtime failure for original and rebuilt code.
  All reach panic after stream release and exit1. This validates fatal Lua behavior,
  not original MSVC exception unwinding. A rebuilt append exception verifies owned-name
  cleanup separately.
- The strict Win32 build and both existing CTests pass. No permanent tests were added.
  Seventeen code/data spans match the installed PE image; six unwind states are checked.

B69D40's stored body stops at B69DCC; complete disk code continues through B69DE4 (24 bytes).
The locked tail repair decoded the missing region but did not extend the stored body.
4D0FA0 similarly lacks FB2..FB6 (five bytes) in its stored body. Captured native fixtures
include both full tails; exports and reports retain this distinction. No Java or alternative
write path was used to bypass the disabled function-body extension route.

Next: reconstruct/bind the actual BDEF90 producer and VFS manager+04/stream ownership
contracts, then connect the native Lua bootstrap/file services to B43B00 descriptor loading.
Arbitrary invalid extents/indices, short-read uninitialized tails, original exception ABI,
general callback-driven storage mutation, game rendering and gameplay remain unvalidated.


## Correction from docs/NATIVE_LUA_SCRIPT_OVERRIDES.md

The actual BDEF90 override producer and BDEF80 registration thunk are now reconstructed, including native vector append and path helpers. Captured suffix order, duplicates, mutable candidate/current vtable behavior and nested original/rebuilt DoFile execution are verified. Actual callable VFS existence and stream ownership remain separate integration contracts. See that document and `reports/native_lua_script_overrides.json`.
