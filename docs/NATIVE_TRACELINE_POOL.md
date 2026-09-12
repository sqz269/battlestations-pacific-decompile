# Native Traceline pool

Addresses: `00AF19D0`, `00AF32F0`, `00AF1D30`, `00AF1EA0`, `00AF1AC0`,
`00AF3170`, `00AF1C70`, `00AF3250`, `00CD77F0`, `00CE0B70`.

The supplied `38h` owner corresponds to static `00F8C288`. It uses the application's
existing `AllocatorListDomain` for `00E188B4`, the same allocator element at owner+0,
real Win32 `CRITICAL_SECTION` storage, and real CRT allocations. No secondary list,
slot map, or semantic pool owns its allocation state. The separate `BAD6F0` tracer
uses `7B0h` slots; Model uses `188h` slots. Neither pool can supply these Tracelines.
Names below are descriptive hypotheses, not recovered symbols. The C++ entrypoints
have new interfaces and are not original ABI replacements.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| `AF19D0` initialize slab | ECX slab, stack slab ID; EAX slab; `AF1A10 RET4` | complete |
| `AF32F0` allocate raw slot | ECX initialized pool; EAX slot; `AF3417`/`AF342B RET` | complete |
| `AF1D30` return raw slot | ECX pool, stack slot; `AF1D99 RET4` | complete |
| `AF1EA0` static raw return | ECX slot; select F8C288, call AF1D30; `AF1EAB RET` | complete |
| `AF1AC0` free table backing | ECX table header; `AF1ACD RET` | complete |
| `AF3170` construct pool | ECX fresh pool; EAX same; `AF3242 RET` | complete |
| `AF1C70` destroy pool | ECX pool; `AF1CF9 RET` | complete |
| `AF3250` trim empty slabs | ECX pool, D5D924 virtual0; `AF32E9`/`AF32EF RET` | complete |
| `CD77F0` static initializer | no arguments; EAX atexit status; `CD7805 RET` | complete, no saved Ghidra function |
| `CE0B70` static exit callback | no arguments; select F8C288, tail AF1C70 | complete |

The constructor is the pool-layout producer: base profile `D7A0C0`, previous+4,
next+8, then derived profile `D5D924`. It initializes section+0C and depth+24,
table+28/count+2C/capacity+30/earliest+34, reserves 32 pointer cells (`80h`), and
sets earliest to unsigned `FFFFFFFF`. The profile's first DWORD is `AF3250`.
Bind `bind_static_native_traceline_pool_00f8c288` before construction publishes the
element; this only binds its real trim method and borrows the same owner/domain.

The slab producer writes WORD free count `32` at `37C0`, reverse WORD indices
31..0 at `3780`, and 32 retained slab IDs at slot+`1B8`, stride `1BC`.
The slab is `37C4h`; payloads `[slot,slot+1B8)` and final padding WORD `37C2`
are preserved. These offsets come from all producer writes at `AF19D9..AF1A0B`.

Allocation enters the real section and increments explicit depth. On no available
slab it publishes earliest=count before allocating `37C4h`, then grows a full table
with wrapped `capacity*2+2` and wrapped byte size `capacity*4`. Capacity publishes
before allocation; current entries copy, old backing frees, replacement publishes,
then the new slab pointer appends and count increments. A decremented/reloaded WORD
free count selects a free index. Exhausting a slab scans only later indices. There
is no EH frame: allocation failure preserves the reached lock, depth and earlier
publications. Payload construction is a separate operation.

Return uses the slot's retained ID to select its current slab, computes wrapped
32-bit slot-minus-slab, divides the signed result by 444 with truncation toward zero,
and stores its low WORD at the free-stack top. Assembly `AF1D58..AF1D69` proves
the signed reciprocal division. It reloads free count after that store, increments,
lowers earliest with an unsigned comparison, then decrements depth and leaves.
There is no native double-return or range validation. The host helper
`owns_native_traceline_slot` checks a current slab range, exact `1BCh` boundary and
retained ID; it does not establish allocation or object lifetime. Keep the pool
and slabs stable while using it. `AF1EA0` is also used from constructor unwind.

Trim holds no internal lock. It frees wholly empty slabs, moves the last table
pointer into the hole before decrementing count, rewrites all 32 moved IDs, and
retries that hole. The final scan recomputes earliest from a captured table pointer
and current count. Capacity and stale unused table cells remain. Destruction frees
slabs in ascending index order, frees nonnull table backing, drains positive signed
depth, deletes the section and unlinks the same base element. It does not destroy
Traceline payloads, clear stale metadata, or physically free the owner.

`AF3170` installs handler `CBAA6E`, FH3 info `DF2868`, unwind map `DF2850`:
state 0 -> base unlink `CBAA50 -> 403970`; state 1 -> section cleanup
`CBAA58 -> 402F70`; state 2 -> table cleanup `CBAA63 -> AF1AC0`. Main body
advances directly 0 -> 2 after initialization. Source `__try/__finally` preserves
these cleanup actions; original FH3 dispatch compatibility remains unvalidated.

## Evidence and live-analysis gaps

All ten full spans were fetched through identity-verifying `bsp.py ghidra bytes`
and compared with the original disk PE. Binary SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, bridge8089.
`reports/native_traceline_pool.json` contains per-span hashes, instruction endpoints,
original ABI, old comments/prototypes, call rows and proposed names. Whole-function
listings establish EBX/EBP/ESI/EDI provenance. No Ghidra mutation was performed.

Returning-free gaps are `AF1ACC` (`POP ECX`), `AF1C8D..AF1C97`
(increment, cleanup4, loop), `AF1CA5..AF1CA7` (cleanup4),
`AF3229..AF322B` (cleanup4), `AF3276..AF32AF` (move, rewrite, retry), and
`AF3386..AF3388` (cleanup4). Saved pseudocode incorrectly returns before each.
Alignment gaps `AF325D..AF325F` and `AF33E9..AF33EF` are unreachable padding.
`CD77F0..CD7805` is a 22-byte complete raw routine ending in a one-byte RET;
the integrator must define `[CD77F0,CD7806)` before its two call rows can pass the
live verifier. `CD77FF`'s one stacked callback is cleaned by `CD7804 POP ECX`.
The neighboring `AF1C20`, `AF1DB0`, and `AF2650` are outside this reconstruction.

## Verification and limits

The standalone scratch fixture executes all ten original spans with operand fixes
derived from a complete Capstone decode. Original and source operate on separate
actual byte owners and slabs, both using real Win32 sections and the same real
malloc/new-handler/free service with allocation tracking and fresh-byte fill.
This instrumentation permits comparing all payload-preservation bytes; it does
not replace payload construction with a semantic model. Imported OS functions
call Windows directly. The original FH3 handler is not exercised in normal runs.

One bounded run passed 74,946 assertions over 2,204 repeated pool-state comparisons
and 807,821,736 repeated full-slab bytes. These are repeated observations, not
independent test cases. It covers slab preservation, 1,100 acquisitions crossing
table capacity32 ->66, selective returns, empty-slab moves/all32 IDs/retry, complete
return/trim, static raw return, real recursion drain, free order/shared-list relink,
and a backed negative slot at free count32 that aliases/reloads the count WORD.
The native static registration fixture returns a failure sentinel; source uses
real `std::atexit`. A later observation callback confirms table free and head unlink
at normal process exit. Host membership checks include unaligned rejection.

Scratch reproduction: `local/traceline_fixture.py` generates `original.hpp` and
evidence; `local/traceline-pool/build.ps1` compiles this current source plus canonical
allocator-list source; `replay.ps1` links current pool source with the integrator's
existing `build/win32/Release/bsp_core.lib`. Flags include MSVC x86 `/W4 /WX /MD
/O2 /fp:strict /MANIFEST:EMBED`; `/SAFESEH:NO` permits relocated original normal-path
SEH registration. Scratch files are ignored, preserved locally, and not permanent
tests. Portable scratch copies survive worktree retirement at
`C:/Users/sqz269/bsp-as-traceline-pool`: `make_fixture.py [repository]` regenerates
the evidence, and `replay.ps1 -SourceRoot <repository>` compiles that repository's
current source with the integrator libraries. This preserved replay also passed.
The standard `scripts/build.ps1` passed with CTest `reconstructed_math` (1/1).
That build excludes this new source until root registers it in CMake; the strict
standalone/replay compilations above include this source explicitly.
`verify_report_calls.py` checked 17 direct rows: 15 passed and the two `CD77F0`
call sites failed only because there is no saved function yet. Root owns that
definition and the final verifier rerun. No gameplay, installed-game invocation, original allocation
failure/FH3 dispatch, concurrent pool use, or drop-in ABI compatibility is claimed.
