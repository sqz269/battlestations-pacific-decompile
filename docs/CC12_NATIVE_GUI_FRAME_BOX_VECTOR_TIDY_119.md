# Raw FrameBox vector tidy 119

Whole AD21B0 `[AD21B0,AD21F6)`70B/26 starts and the existing undefined5B
AD2320 tail are composed in the existing `native_gui_frame_box_records.hpp/.cpp`.
The explicit Source APIs take actual header, SAME actual render owners/raw
strings and one required prepared operation. Original AD21B0 uses ECX actual16B
header/no stacked arguments/RET; AD2320 preserves receiver/caller stack while
tailing to it. These new C++ interfaces are not Original binary/private-EH ABI.
No production FrameBox owner, allocator/copy/rollback or GUI activation is added.
Baseline is unchanged116 commit `56e94a25a3a88692f213eb20094235f19546b9d5`.

```cpp
void tidy_native_gui_frame_box_vector_00ad21b0(void*, NativeRenderActualOwners&,
    NativeStringRawPoolContext&, NativeGuiFrameBoxVectorTidyOperation&);
void cleanup_native_gui_frame_box_vector_00ad2320(void*, NativeRenderActualOwners&,
    NativeStringRawPoolContext&, NativeGuiFrameBoxVectorTidyOperation&);
```

## Required Source preparation

`NativeGuiFrameBoxVectorTidyOperation(header, expected_first, expected_end,
fresh_records_pointer, record_count)` has no default/copy/move. This concrete
C++17 pointer/count array is the required stable prepared metadata span; there
is no generic span abstraction or default callback. Metadata owns no vector,
allocation, resource, registry, pool or graph. Caller prepares every distinct
fresh111 operation and retains the SAME complete independent graph, backing,
header and metadata outside the failing scope BEFORE entry/callbacks.

Constructor validation runs in the FIRST member initializer, using only this
address/sizeof and supplied inputs before any member store. It checks finite
DWORD-aligned2Ch constructed range/count correspondence, checked known-region
arithmetic/disjointness and all child phases fresh. It performs no raw header
snapshot. Null/null expected range plus an empty array can therefore be prepared
without reading arbitrary raw end/capacity bytes. This byte-store ordering does
not restore object lifetimes or legalize invalid C++ placement.

Entry rechecks exact fresh operation/header identity, known regions/fresh child
metadata and known service object/cell regions before raw effects. Full backing
extents, valid live object lifetimes, canonical2D provenance, whole provider/
scratch disjointness and SDK allocator membership remain caller proofs. The
owner-base region check does not prove the full derived registry allocation.
Providers/external callbacks cannot mutate this operation/prepared111 array;
only this engine and its direct111 calls advance them. Access and lifetimes are
externally serialized. Failed103 records are NOT constructed owners and are
excluded: their copied resource pointer can lack an acquired AddRef. AD1910
committed-prefix rollback remains a separate unprovided caller.

## Exact engine schedule

After metadata preflight the Source phase becomes entered. Capture initial begin
once; null skips the actual end READ and free, then still reaches all three
zero stores. A nonnull begin captures end once. Captured values must match the
prepared range before record callbacks; mismatch is a sticky entered Source
failure, an additional orchestration rule rather than Native validation.

Iterate FORWARD with actual cursor as111 receiver. Each call uses its distinct
prepared fresh111 operation and SAME real domains. Advance cursor2Ch and the
completed prefix ONLY AFTER return, comparing against the cached end. No raw
header frontier is advanced. After all returns, RELOAD CURRENT header+4 and
invoke concrete `singleton_lifetime_free` even when that CURRENT value is null.
Only after the free returns zero actual+4, +8 and+C in order. Never touch+0.

```text
00ad21c5 CALL 00ad0860
00ad21d5 CALL 00bf65ac
```

The Source forwarder passes the SAME operation/services directly. Its optimized
body is a single5B JMP to the Source engine; this does not make it a drop-in
Native ECX wrapper. AD2320 remains undefined in Ghidra: raw bytes
`e9 8b fe ff ff` independently establish its Native tail target. The verifier
checks that real target entry but cannot claim an enclosing live tail listing.
Its proposed/implemented Source name suffix produces the expected undefined-
entry warning; no function/listing/name is created there. Only a fragment ledger
record is used for AD2320.

## Failure, free and lifetime limits

The only established111 zero family is a genuine completed canonical registered
2D creator with D61948/BD30E0/B3F590 and SAME texture context/pool, canonical
render registry, VFS/raw string publication/gate/raw-manager cells. No arbitrary
virtual adapter, fake terminal, replacement pool or per-record callback is used.

An escaping C++ exception marks tidy failed, retains active cursor/completed
prefix/free phase and does no cleanup, free, header zero or retry. Completed
records can already have freed resources/returned names; failed111 can already
have decremented to zero or returned a name without clearing its raw header.
Retained raw storage/pointers are diagnostic identities, not healthy replay
capabilities. No reset/rollback API exists. A different fresh operation must
not replay that lifetime.

Entered/failed tidy metadata destruction terminates using metadata only;
fresh/returned destruction reads metadata only. This is explicit Source
orchestration, not Original ownership/destructor behavior. Neither metadata
object retains or repairs the actual graph. Existing111 atomic/noexcept/import,
cleanup-throws and compiler-EH-hoist limits remain unchanged. Its terminating
Entry failures do not unwind into tidy. Tidy's new C++ catch/phase is not Native
local EH (the Original tidy has none), surrounding FH3/double-exception or
arbitrary SEH proof. Flags mark Source scheduling, not Native exception origin.

Caller independently keeps the captured old buffer AND any current-free
allocation live through their applicable actions. Header changes cannot revoke
captured-range lifetime. CURRENT free value must be null or a valid compatible
Source malloc-family base, disjoint from live header/metadata. If it differs
from initial begin, this body frees ONLY the current value; no extra old-buffer
free is invented. Header-cleared/returned does not prove whole graph retirement.
`singleton_lifetime_free` is the existing noexcept std::free boundary, with no
manager registration. Native/private/foreign heaps, pool slots or new/delete
compatibility are not inferred. Raw ACEF10 allocation and private CRT semantics
remain separate. No typed vector/FrameBox projection is cast into a raw owner.

## Verification and evidence

Fresh MSVC Win32 `/MD /std:c++17 /O2 /Gy /W4 /WX /fp:strict /EHsc` compilation
passed. Initial strict compilation used C++20; the canonical build caught that
its std::span API was unavailable in the actual C++17 target. The API was
corrected to the required pointer/count pair without CMake or Native scheduling
changes. The first full failed build.log was overwritten by the final rerun
before the preservation instruction arrived; only its truthful captured excerpt
and initial draft are retained, NOT a complete recovered log or rerun failure.
The Windows max macro was also handled locally with parenthesized numeric_limits.
Final required `./scripts/build.ps1` passed, existing3/3 checks7.47s. Those tests
do not execute this new tidy path. No new test, fixture, probe or runtime ran.

Root reviewed the complete revised Source/constructor and optimized slices.
Ctor129B: validation+29 precedes first member store+39. Engine571B includes its
Source preflight/catch; begin+121/null+135 skips end+137/free;111+168 precedes
cursor+170/completed+179; current begin+181, SDK free+18C evenNULL, then ordered
zeros+19B/+1A2/+1A9. Failed phase+22C precedes rethrow without cleanup. Tidy
metadata destructor18B and forwarder5B are inspected. Their final bodies, plus
the engine, are byte-identical across the C++17 correction; only ctor changed.
Both COMPLETE103/111 file prefixes and optimized copy163B/release323B/record-
metadata-destructor17B remain byte-identical.

Native70B/26 starts plus5B tail match live Ghidra, installed Original and prior98.
Every live/annotation/export batch verifies `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. Installed executable SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Only existing AD21B0 is annotated under the lock with prior-value history,
preserved name/comments, save and forced exports. AD2320 stays undefined.
The unique function/fragment ledger rows preserve older records; no Main/CMake
or other Source changes. Final Source, object/assembly/logs, bytes/history,
exports and three libraries are sealed in
`local/cc12_native_gui_frame_box_vector_tidy_119/manifest.json` before reuse.
