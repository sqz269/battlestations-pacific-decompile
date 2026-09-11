# Actual render-buffer unregistration

This reconstructs nine complete functions against the actual native storage.
The four removal loops use swap-last replacement. They neither shift a suffix
nor release a pointee. The physical wrappers use the existing actual renderer
synchronization implementation, retain the entry-time renderer, and add no
exception cleanup.

## Address and ABI evidence

| Original range, exclusive end | New C++ entry | Original ABI |
| --- | --- | --- |
| `00B25300..00B25367` | `remove_native_renderer_vertex_pointer_00b25300` | ECX array; stack pointer-to-DWORD; RET4; full EAX 0/1 |
| `00B25370..00B253D7` | `remove_native_renderer_index_pointer_00b25370` | Same |
| `00B25580..00B255E7` | `remove_native_renderer_texture_pointer_00b25580` | Same |
| `00B4B2E0..00B4B347` | `remove_native_physical_logical_pointer_00b4b2e0` | Same |
| `00B268E0..00B268F3` | `unregister_native_renderer_vertex_stream_00b268e0` | ECX renderer; stack DWORD; RET4; full EAX 0/1 |
| `00B26900..00B26913` | `unregister_native_renderer_index_stream_00b26900` | Same |
| `00B27D40..00B27D53` | `unregister_native_renderer_texture_00b27d40` | Same |
| `00B4B390..00B4B3E1` | `unregister_native_physical_index_stream_00b4b390` | ECX physical; stack DWORD; RET4; no stable EAX contract |
| `00B4B3F0..00B4B441` | `unregister_native_physical_vertex_stream_00b4b3f0` | Same |

Every range was freshly checked through the project-guarded CLI against
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and the installed
PE. The four complete 103-byte removal bodies are byte-identical. The report
contains all preimages, hashes, current names/comments, and naming proposals.
Names remain descriptive hypotheses; the C++ interfaces are new entry points.

## Actual array and storage ordering

Only the array's data DWORD at `+0` and count DWORD at `+4` are touched. An
enclosing capacity DWORD at `+8` is preserved. Renderer wrappers address these
arrays at `+1AAC`, `+1AB8`, and `+1B00`; physical wrappers address `physical+8`.
The wrapper stack argument's address is passed to the corresponding raw loop.

At `B25300/B25303/B2530A` the loop captures data, count, and the wrapped
`data + count*4` end. `B25310` compares cursor and end as unsigned addresses.
If begin is not below end, it returns zero before loading either the value
word or any element. Otherwise `B25319` reads the sought DWORD once, then the
loop compares each actual DWORD and advances its numeric cursor by four.
This is raw 32-bit arithmetic, not C++ pointer ordering or a host container.

The match index uses `SUB` followed by arithmetic `SAR 2` at `B2533A..B2533C`;
the all-ones sentinel check is retained. The captured count determines the
last index and last data slot. A non-last match copies the captured last
DWORD into the matched slot. `B25354` then decrements the **current** count
word. This ordering matters even without callbacks: the element store may
alias that count word. No stale tail clearing, capacity change, deallocation,
reference decrement, pointee call, diagnostic, or null guard occurs.

The renderer wrappers return the raw loop's entire EAX, which its `XOR EAX`
and `SETNZ AL` establish as exactly zero or one. Physical wrappers discard
the loop's logical result; optional leave can change EAX, so their interface
returns void.

## Actual optional synchronization

Both physical wrappers read current mode at `0108D6DC`. When nonzero they
capture the pointer from actual global `00F8D394` into EDI, call the complete
`00B33AD0`, and save returned AL. They then call the complete `00B4B2E0` on
`physical+8`, regardless of the guard's returned value. Their final current
mode test decides whether to call `00B33B00` using the captured renderer.
They do not reload global `00F8D394` for leave.

The borrowed global pointer reference and
`NativeRendererSynchronizationGlobals` are actual supplied storage. No
whole-renderer or physical-owner overlay, global copy, owner initialization,
provider callback, or substitute synchronization policy is added. Existing
`native_renderer_synchronization_actual.cpp` supplies the full native guard
behavior, including current renderer-lock reads, wrapped global nesting,
real Win32 critical-section calls, and the mode reload after actual leave.

These wrappers contain no FS-chain registration, FuncInfo, unwind funclet,
or try/catch. The implementation similarly adds no RAII or catch cleanup.
An exception during entry propagates before removal and without a leave.
If entry changes mode to zero, removal runs and normal cleanup is skipped,
retaining the already-incremented nesting and acquired lock.

If entry mode is zero, native AL storage is uninitialized and the skipped
branch loads an uninitialized stack word into EDI. Neither value is consumed
by a call while exit mode stays zero. An intervening zero-to-nonzero change
would consume that indeterminate native state and is outside the supported
domain. The source records this precondition with `__assume(entry_enabled)`
at the final enabled branch, without adding an initialization or runtime
fallback. Its isolated DWORD MOV reads the saved AL word, including padding;
the complete leave helper ignores all 32 argument bits. This avoids a C++
indeterminate scalar read and preserves the native lack of initialization.
Unsynchronized racing mutations, inaccessible addresses on dereferenced
paths, and arbitrary SEH recovery are not claimed.

## Bounded native comparison and build

One private comparison runs all nine complete original bodies and both full
original guard helpers in a sparse relocated image. All fifteen owner,
helper, IAT, and global spans are checked before execution. The two globals
lie in the PE's zero-filled `.data` tail; their initial zeros are checked
against the PE section mapping and fresh Ghidra bytes, not described as
file-backed bytes. Sixteen absolute operand preimages are checked before
relocation. Original owner-to-owner and owner-to-guard calls remain intact;
no entry hooks replace any of these eleven complete functions.

Only the Enter/LeaveCriticalSection IAT cells point at observation shims.
Those shims call the real Win32 APIs and perform the same chosen mutations
for original and reconstructed runs. The unchanged actual synchronization
source is privately compiled with a forced-include header redirecting only
these two API names to the same shims. Production files contain no fixture
hooks. All owners, arrays, global records, and critical sections are real
supplied storage; residual fixture locks are released after observation.

The comparison passes **1,848 normalized words, 44 events of 42 words, and
16 bounded cases**. They cover first duplicate, last and missed removals;
an element/count alias producing current count 76; wrapped/empty unsigned
early-outs with unreadable data and value targets; all three renderer
offsets and full-EAX results; disabled mode; null-lock nesting wrap; global
renderer replacement while retaining the captured renderer; current mode
changes at actual enter and leave; and entry failure without removal or
automatic cleanup. Mode changes are deterministic API-boundary callbacks,
not a claim of thread-safety under data races.

The initial strict build exposed C4701 for typed reads of the deliberately
uninitialized guard. The final implementation uses isolated native MOV
reads and the explicit execution precondition described above. Fixture-only
compiler corrections added a signed-depth cast and a parenthesized page
bound; no behavioral expectation or production algorithm was changed to
make the comparison pass.

The fixture builds with MSVC Win32 `/std:c++17 /O2 /Oy- /EHsc /fp:strict /MD
/Gy /Gw /W4 /WX`. Private CMake registration adds only this new source before
running `scripts/build.ps1`; seed verification and both existing math checks
pass. The exact commands, output pins, and artifacts are in
`reports/native_render_buffer_unregistration_audit.json`.

No tracked tests, shared CMake, Ghidra annotations, exports, or ledger shards
are changed by this worker. The integrator owns registration and saved
annotation updates. This establishes reconstructed and fixture-tested
behavior through the stated actual-storage interfaces, not a drop-in ABI
replacement, complete logical/physical lifetime, or game validation.
