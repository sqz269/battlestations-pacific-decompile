# Input Lua, read-only native data and MPAK storage services

`NativeLuaServiceBindings` now supplies the existing fundamentals, `DoFile` and
override callbacks from one retained service bundle. Both Lua entry structures
use the same zero-upvalue trampoline. Scoped thread-local activation restores
the preceding bundle across nested execution and C++ unwinding; the override
callback forwards its passed manager. The source routing policy is explicit and
does not establish original cross-thread, longjmp or FH3 behavior.

`GameNativeReadOnlyData` verifies the complete installed executable before
committing its original `.rdata` bytes at their recorded addresses. It retains
read-only, non-executable table/literal pages for raw source dispatchers and
releases its own reservation after all borrowers finish. The full section and
every page are checked in the populated VFS/Lua fixture. See
`GAME_NATIVE_READONLY_DATA_AZ.md` for image identity and address constraints.

Two MPAK interfaces now have concrete storage implementations:

| Native routine | Supplied behavior | Native ABI |
| --- | --- | --- |
| 00BB4140 | Actual file-vector lookup, returning validation, current backing reload | ECX vector; stacked index; EAX record; RET4 |
| 00BB4F40 | Scan 14h directory rows for a member, retaining captured end | ECX first; EDX end; stacked key; EAX row/end; RET4 |
| 00BB6180 | Copy actual DWORD-vector storage, allocator preimage, returning CRT diagnostics | ECX destination; stacked source; EAX destination; RET4 |

Directory lookup reuses `find_native_string_vector_005efba0`. That existing
function captures count before data, compares counted lengths then empty or CRT
case-insensitive equality, and computes the signed index against current data.
Its source and saved name are retained. Four full live/disk envelopes, every
instruction owner, eight internal direct calls and five table spans are checked.
The three supplied storage roots have saved names, original register/stack ABI
annotations and refreshed exports. No body repair was required in this batch.

The focused comparisons cover eight lookup cases and five offset-copy cases,
including returning validation, preserved allocator words, self-alias and a
returning `memmove_s` error. The offset fixture compiles the unchanged source
body with allocation redirected to the same instrumented service as native
copied code; it does not prove original allocator or exception identity. The
Lua fixtures additionally execute physical HANDLE reads, FileStore chunks,
nested `DoFile`, duplicate suffix order `MCPP`, stream reference/cursor checks,
Lua close and zero-counter shutdown. All captures and probe binaries stay under
ignored `local/`; reports pin their hashes.

Production ownership remains incomplete. The VFS fixture seeds valid records and
uses the retained semantic lifetime domain. Physical-stream pools, render-batch
pool/lock services and type-counter ownership still need one raw 01090AA0 domain.
The physical and render-batch adaptations are dispatched independently. Their
shared deletion routes must be installed before registration. The container
packet is still reviewing native growth/unwind behavior; this batch does not
enable MPAK archive loading. Saved ownership holes BF4398 and container helper
tails belong to those follow-ups and are not counted as repaired here. The game
host does not yet reach the completed raw graph; original ABI and gameplay
remain unvalidated.
