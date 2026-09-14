# Actual resource-cache node BN

The packet reconstructs the actual `1Ch` cache node constructor and allocator
in [native_resource_cache_node.cpp](../src/native_resource_cache_node.cpp).
It follows the frozen frontier contract from commit
`d93fe02665629fbcdc1276359d21d9a8d12edf85`; worker base is
`c1579357bc6c61e3036f870c5047d4b402d49a28`.
The [report](../reports/native_resource_cache_node_bn.json) carries native
spans, supported call rows, separate decoded-only transfers, membership, hashes and checks.

| Entry / full span | Original ABI | Coverage |
|---|---|---|
| `B7F220..B7F28F`, 112 bytes | ECX node; stack left,parent,right,source pair,color; EAX node; RET14h | Complete source |
| `B7F6A0..B7F711`, 114 bytes | Incoming ECX unused; same five stack arguments; EAX captured allocation; RET14h | Complete source plus catch below |
| `B7F712..B7F726`, 21 bytes | FH3 catch frame; captured allocation at EBP-14h | Complete decoded catch represented by source free/rethrow; listing ends at B7F71A |
| `CC20D0..CC20E0`, 17 bytes | FH3 unwind frame; EBP-14h/18h captures | Complete evidence: placement-delete call is a no-op |
| `CC20E1..CC20EA`, 10 bytes | Load EAX FuncInfo, tail-jump FH3 runtime | Complete decoded metadata evidence; no current function membership |

## Actual storage and construction

The existing `RESOURCE_CACHE_OWNERSHIP.md` layout agrees with the producer:
links at `+0/+4/+8`, owning key length/data at `+C/+10`, raw resource pointer
at `+14`, color byte at `+18` and nil byte at `+19`. The allocation is
28 bytes; the constructor leaves bytes `+1A/+1B` untouched. The source pair
is an actual `0Ch` key-header/raw-pointer record. There is no projected tree,
allocator member, resource retain, or source-pair copy in these interfaces.

`B7F238` compares `node+C` with source before any link/key stores. Then the
three links are stored, followed by zero length and zero key pointer. Existing
output key storage is abandoned by that initial clear. An identical source
skips resize/copy, leaving an empty key while copying its current raw value.
Other source aliases observe the preceding node stores.

Nonidentity reads current source length and calls actual raw-pool `41DD40`
with preserve1. After that call, the source length guard, destination length,
source data, and destination data are all reread in native order. The copy
uses the current destination length. `BF7680`'s observed backward-overlap
branch is represented by `memmove` for valid supported buffers; zero-count
copies retain the existing raw string layer's omission of the host CRT call.

Only after copying the key does `B7F277` read current source+8. It stores that
raw resource, then the low color byte, then nil0. A pool operation or aliasing
copy can therefore change the resource subsequently copied. No reference
count, virtual call, key rollback, or resource cleanup is introduced.

`NativeStringRawPoolContext` borrows the real pool publication, small-return
gate and raw lifetime-manager publication. Resize reacquires the native pool
at its own allocation/return boundaries, including getter exceptions. Outer
node allocation/free reuse the canonical singleton CRT service with both
native and host sizes exactly `1Ch`.

## Allocation and failure ownership

`B7F6C3` allocates `1Ch` before entering state0. The result is captured both
at EBP-14h and EBP-18h; construction executes only when it is nonnull. The
function returns captured ESI, including the null branch, rather than relying
on constructor EAX. The existing allocator normally returns nonnull or throws.

Handler `CC20E1` selects FuncInfo `DFB2E8`, with magic `19930522`, maxState3,
unwind map `DFB2D0`, one try block at `DFB2BC`, and flags1:

| State | Next | Action |
|---|---|---|
| 0 | -1 | None |
| 1 | 0 | CC20D0 |
| 2 | -1 | None |

The try covers states0..1, catchHigh2, one catch-all descriptor at `DFB2AC`
with adjectives40h, type0, handler `B7F712`. `CC20D0` reads both captured
allocation words, calls `401130` and drops 8 stack bytes. The complete
`401130` body is one `RET`; it does not free or destroy a partial key.

The separate catch loads captured EBP-14h and calls `BF65AC` at `B7F716`.
Its decoded continuation drops that argument, pushes two zeros and calls
`BF6885` at `B7F722` to rethrow. Source catches construction failures, frees
only the captured outer allocation, then rethrows the same C++ exception.
Allocation failure occurs before that try; no catch cleanup runs for it.

## Native call evidence and repair proposal

| Site | Native target | Native argument/cleanup evidence |
|---|---|---|
| B7F259 | 41DD40 resize | ECX key; length,preserve1; callee RET8 |
| B7F26F | BF7680 copy | destination,source,count; B7F274 ADD ESP,0Ch |
| B7F6C3 | BF681B allocate | size1Ch; B7F6CA ADD ESP,4 |
| B7F6F8 | B7F220 construct | ECX captured node; five arguments; callee RET14h |
| CC20D8 | 401130 placement delete | two captured pointer words; CC20DD ADD ESP,8 |
| B7F716 | BF65AC free | captured outer pointer; decoded B7F71B ADD ESP,4 |
| B7F722 | BF6885 rethrow | two zero arguments; no normal continuation |
| CC20E6 | BF6B43 tail jump | EAX points to DFB2E8; FH3 support only |

Live xrefs show B7F6F8 as the constructor's direct caller and B80004 as the
allocator's direct caller. Their argument setup was reviewed; all five
arguments remain explicit, including an unrestricted low color byte.

The live catch body still ends at `B7F71A`; `B7F71B..B7F726` is decoded and
PE-matched but unowned. Proposed integrator repair: attach that continuation
to `B7F712` under the write lock, preserve comments, refresh exports and save.
The decoded `CC20E1..CC20EA` handler also has no current function. Its proposed
separate function creation must preserve the known FH3 transfer and existing
metadata. No repair, no-return change, annotation or shared ledger edit was
made in this packet. The final call verifier checks all six supported owned
CALL rows and passes. The unowned rethrow and handler JMP remain separate
decoded-only evidence until repair. An initial eight-row verifier receipt
is retained showing exactly those two membership failures. No source branch
is omitted because of the analysis membership gap.

## Verification

Every live batch used `bsp.py ghidra`, which verifies project `bsp`, program
`/battlestationspacific.exe`, x86 language and base `00400000`; configured
project file is `C:/Users/sqz269/bsp.gpr`. Autostart was disabled, and no
scripts or Ghidra mutations were run. Ten complete body/support spans,
371 bytes total, match fresh live Ghidra and the installed PE.

The changed TU compiled with MSVC Win32 `/std:c++17 /EHsc /O2 /MD /W4 /WX
/fp:strict`. An ignored fixture linked the new object explicitly with the
already-frozen orchestrator library SHA256
`134578367593f74415bb6fd12b51d103dfc6575dcb94d2f692640dde69c78251`.
Its original source path, prior freeze receipt and current frozen-file hash
are in the report; no library was recopied from main or a rebuilt worktree.

The fixture uses an actual constructed native pool, raw nodes/pairs, and the
canonical allocation/free functions. It passed key/value copy, identity and
source/link alias cases, arbitrary color byte and preserved padding, source
key/resource mutation during allocation, unchanged resource counts, partial
constructor state on pool-getter failure, and outer-node free exactly once
before rethrow. Only the fixture process temporarily instruments its own CRT
malloc/free imports and new handler to make those allocation events observable;
production source has no test callbacks. The probe embeds its manifest and
restores its imports/handler before exit. No permanent test was added.

These are complete source storage schedules with explicit host CRT and raw
pool dependencies. Register/stack spill identity, original hardware SEH/FH3
transport, arbitrary invalid pointers and wrapping copy ranges are not proven
equivalent. No insertion, resource manager, production binding, integrated
build or game/frame behavior is claimed by this worker.

## Primary integration correction

Registered and built with both existing tests passing; see [BN integration](NATIVE_CACHE_BLOCK_INTEGRATION_BN.md) for exact source/build revisions and evidence. Primary saved reviewed names, original ABI analysis views and preserved comments, then refreshed exports. The node catch membership and two FH3 handler definitions are complete; earlier worker observations remain historical evidence. The package report now exposes all seven direct calls to the whole-report verifier. Fixture libraries remain pinned to BL; complete production loading, native FH3 identity and gameplay remain unproved.
