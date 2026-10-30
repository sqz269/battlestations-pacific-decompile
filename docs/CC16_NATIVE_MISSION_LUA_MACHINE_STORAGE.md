# Raw mission Lua machine storage: 006B8740 and 006B87C0

This packet supplies the complete bounded constructor and scalar operations over
actual 0Ch backing, using the real linked Lua 5.1.1 functions and the existing panic
callback. Strict MSVC Win32 object compilation and native/PE/COFF checks pass.
The new Source interfaces are not yet registered in CMake, linked into Core or
adopted by the host. Full host lifetime and numeric-profile dispatch remain open.

The published base is `9e360e71494881f48b7944862b7777f69ef5ac9d`.
The explicit private readiness input is
`414237d0820620782aeb28760b63fff825e74b4f`, whose report has SHA-256
`120479c2d6c5a94a5ee64767ff17fe1775c3bb500af4ef88cbb3f5bf4ad5e904`.
Its tree is preserved. This branch changes exactly the new header, source,
this document and `reports/cc16_native_mission_lua_machine_storage.json`.

The native envelopes are already represented by older semantic reconstruction
labels. This necessary actual-storage/library-contract composition earns zero
new unique Original-body, vendor-body, duplicate-body, ABI or activation credit.
No panic body, callback class, owner projection, fake state or forwarding-only
scalar was added.

## Native extent, allocation and ABI

The fresh current PE hash and independently decoded complete bodies match the
frozen live/PE blocks in the private input. The installed PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Body | Complete normal bytes | Native shape |
| --- | --- | --- |
| `006B8740..006B87B5` | 118 bytes / 37 instructions | ECX=machine; no public stack arguments; EAX=captured machine; RET |
| `006B87C0..006B87EC` | 45 bytes / 15 instructions | ECX=machine, flags DWORD on stack; EAX=captured machine; RET4 |

The private caller capture establishes the allocation size: `00884BF8`
pushes 0Ch and `00884BFC` calls BF681B; `00884C16` calls the constructor
and `00884C34` publishes the result at captured host+4. The constructor itself
does not allocate machine backing. The observed fields are DWORD profile+0,
lua_State pointer+4 and owns BYTE+8; bytes+9..B remain padding/preimage.

The numeric profile CF8394 has native slot0=006B87C0. Following CF8398 is
the `nSlu` text, not another method. CF8394 is stored as identity data; it is
never dereferenced as Source code, installed as a callable Source table or used
to authenticate a receiver. `006B8790` lies inside the CALL at
`006B878F..006B8793`; it is not a separate entry.

The reused panic body `006B8720..006B8736` is 23 bytes. Its Native ECX callback
ABI is different from the existing Source callback used with the linked library.
The private Ghidra input has no separate function there. No Ghidra query, repair,
mutation, import or save was performed by this implementation packet; current
disk checks are explicitly separate from the earlier target-verified live capture.

The public APIs are ordinary C++ `void*(void*)` and
`void*(void*,uint32_t)`. They have Source caller cleanup and Source C API
calls, rather than Native receiver/register/stack transport. No binary ABI,
FH3/SEH, arithmetic flags, private stack aliases or fault-timing equivalence is
claimed.

## Complete constructor schedule

`construct_native_mission_lua_machine_006b8740`:

1. Captures the actual machine pointer and stores numeric CF8394 at+0.
2. Calls genuine luaL_newstate, writes its result to+4 and gives that captured
   newly-created pointer to lua_atpanic with the actual existing panic callback.
   It does not reload+4 for this panic setter.
3. Reloads current state+4 and calls lua_gc with operation6/data100.
4. Reads the initial Source table opener. For each nonnull opener it reloads
   current state+4 and pushes the genuine function with zero upvalues; reads
   the current row name, reloads state+4 and pushes it; then reloads state+4
   and makes unprotected lua_call(L,1,0).
5. Reads the next opener before advancing/testing the row. After the null
   terminator, and only after all calls return, stores owns BYTE+8=1.
   It returns the captured machine. It never initializes padding or clears the
   earlier owns preimage before this final store.

The internal const-volatile table preserves initial/next opener and name read
epochs. All function pointers are real Source relocations, and the strings are
the exact Native values mapped into Source:

| Native row/name pointer | Name | Native function | Real linked Source function |
| --- | --- | --- | --- |
| CF8350 / CE3A0C | empty string | A67210 | luaopen_base |
| CF8358 / CF8344 | table | A66010 | luaopen_table |
| CF8360 / CF8340 | io | A652B0 | luaopen_io |
| CF8368 / CF833C | os | A64190 | luaopen_os |
| CF8370 / CF8334 | string | A63910 | luaopen_string |
| CF8378 / CF832C | math | A61C50 | luaopen_math |
| CF8380 / CF8324 | debug | A61620 | luaopen_debug |
| CF8388 | null/null | null | null/null |

No luaL_openlibs, package, B6A020 bootstrap, invented mask, script cache or process
cell is substituted. These are the complete fixed table values established by
the private native table/pointed-string captures and rechecked against the PE.

## Complete scalar schedule

`delete_native_mission_lua_machine_006b87c0` captures the full owns byte once,
then restores profile CF8394 before branching on that captured byte. Any nonzero
byte selects close; owns0 does not read state+4. The close branch reloads current
state+4 and invokes genuine lua_close with no state-null guard.

After normal close return, it performs a volatile read of only the low byte of
the Source by-value flags DWORD. Only bit0 controls the real
singleton_lifetime_free call. That call receives the captured original machine
base, with no subobject adjustment. The function returns that captured pointer
even if retired. No receiver field is read after free.

There is no state/owns/profile clear after the callback, and no padding write.
A callback replacement of any machine field is not followed or repaired.
Flags0 still closes an owned VM and leaves stale fields. Independent caller
metadata must prevent replay/reclose.

The explicit volatile argument-byte read keeps the flag epoch after close.
It does not recreate a Native caller's physical argument slot or arbitrary
aliases into that stack.

## Genuine provider and failure contracts

Caller supplies a genuine live, aligned 0Ch allocation from
`singleton_lifetime_allocate({object,12,12})`, retained for the matching
`singleton_lifetime_free` domain. The existing BF681B default in
NativeGameArrayCalls delegates to this real malloc/current-new-handler allocator;
the matching free calls std::free. The constructor creates no backing and the
scalar admits no guessed foreign free domain. The VM state is allocated and
retired through linked luaL_newstate/lua_close; the machine free never frees
that state pointer directly.

All body-specific dependencies are real and present:

- The existing `cc13_seg29::lua_machine_panic_006b8720` declaration and definition
  are reused. They call actual lua_gettop/lua_tolstring(top,null), return 0 and
  add no pop or log. There is no new panic body or Native code-address cast.
- The actual fetched header declares Lua 5.1.1 and GCSETPAUSE=6. Its real sources
  define the required APIs and seven openers. The existing Lua archive positively
  defines all 16 required API/opener symbols.
- The existing Core archive positively defines the actual panic and
  allocation/free providers. Eight relevant repository dependency pins match
  the frozen base; none were edited. Archive/source pins support the named
  providers, not a fresh whole-tree build attestation.

Caller custody starts before entry and remains independent of the machine,
state and callbacks through last use:

| Epoch | Retained state and limit |
| --- | --- |
| Before newstate returns | Only profile+0 is written. State+4 and owns+8 may still be preimages. |
| Null newstate result | Null is published and passed to the real panic setter without a fix, successful default or recovery. The linked library owns the resulting policy; no Original terminal equivalence is asserted. |
| Later constructor interruption | A state may exist while owns retains its incoming byte. Do not infer a safe scalar cleanup from that byte or invent rollback. Retain actual allocation/current-state/provider metadata until its disposition is established. |
| Scalar close | Nonzero owns requires a genuine current linked-Lua state and independently live callback/context/services custody. Close can run finalizers. |
| Close failure/nonlocal transfer | No subsequent flag read or free is reached. There is no pretend completion, retry or implicit cleanup. |
| Normal flags0 | VM is closed; machine fields remain stale. Caller prevents reuse. |
| Normal flags1 | Captured original backing is freed once after close and the late flag test. No later receiver access occurs. |

No catch, setjmp, noexcept adapter, automatic destructor or terminal policy was
introduced. Unprotected Lua error/panic/longjmp/OOM behavior remains the actual
linked library's boundary. A foreign C++ escape through Lua C frames does not
establish safe interpreter reuse or diagnostic lua_close merely because the
allocation metadata is still available. Machine backing must remain valid for
every reached access/free; surviving process cells do not extend service lifetimes.

Native 6B8740/87C0 have no local machine rollback frame. The allocation action in
caller884BE0 and the later host/queue/payload/FH3 graph remain outside this packet.

## Strict object and emitted checks

Final compilation used MSVC19.51.36244.0 Win32:
`/c /std:c++17 /O2 /Ob2 /DNDEBUG /MD /EHsc /W4 /WX /fp:strict /FAs`,
the actual repository headers and existing Lua Source include directory.
It passed without warnings. This was object compilation only.

Independent COFF decoding consumes the complete two public code sections:

| Source function | Bytes | Direct call relocations |
| --- | ---: | --- |
| Constructor |123| luaL_newstate, lua_atpanic, lua_gc, lua_pushcclosure, lua_pushstring, lua_call |
| Scalar |52| lua_close, singleton_lifetime_free |

The constructor has a DIR32 relocation to the existing panic. Its 64-byte table
has fourteen DIR32 relocations for the seven exact function/name pairs and an
eight-byte zero terminator. No old Native pointer value is executed.

The final constructor's current-state loads are at Source offsets1F,43,4E,58;
panic uses captured EAX at16. Initial opener read2C and name read4C precede their
respective uses; next opener read65 precedes advance68. Owns is stored only
at75 after loop exit. Scalar owns read05 precedes profile store08;
current state is read12 only on the close branch; close call16 precedes
flag-byte load1E/test22; captured-base free call28 precedes saved-pointer
return30 with no receiver reread.

Three compiler-emitted private field-address accessors are retained in the COFF
census; the public functions inline them. They earn no Native or duplicate credit.
No local x87 or FP-control instruction appears in either public body. Descendant
library/runtime FP behavior is not proven by this check.

The report retains object/function hashes, raw code, instruction and relocation
records, exact table reconstruction, source/provider pins and ignored local
compile artifacts under `local/native_mission_lua_machine_storage/`.

No tests were added or run. No full build, executable link, probe, target body,
game or DLL was executed. CMake, ledgers, guards, existing semantic APIs/callers,
the original installation and frozen peer execution trees were untouched.

The root integrator still owns independent review, common build/link and source
admission. This packet does not adopt 884BE0, authenticate CF8394/D0E7A8 dispatch,
implement the 560 bindings, or close full host/queue/payload retirement.
