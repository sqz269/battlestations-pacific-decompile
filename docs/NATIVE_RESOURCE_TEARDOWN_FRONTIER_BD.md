# Native resource teardown frontier BD

The next ready packet is six raw pointer-array helpers, **396 original bytes**,
which close the two array-cleanup dependencies reached by resource destruction's
FH3 map. They can be implemented independently using existing allocation/free
services. Complete resource teardown still requires actual cache, hierarchy-pool
and finite item-deletion contracts.

The [report](../reports/native_resource_teardown_frontier_bd.json) records six
live/PE-matched complete spans, ten direct call rows, two unwind edges and the
missing returning-free tails. `B88430` and `718810` were byte-rechecked against
the prior construction frontier: their 636-byte and 96-byte bodies are unchanged.
Every live batch verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The installed binary SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Closed backing-array packet

| Function | Inclusive complete span | Bytes | Contract |
| --- | --- | ---: | --- |
| Primary reserve | `00b872f0-00b8734e` | 95 | ECX raw header; signed capacity stack; RET4 |
| Hierarchy reserve | `00b87350-00b873ae` | 95 | Same schedule, separate native identity |
| Primary resize | `00b873c0-00b8740f` | 80 | ECX raw header; signed count stack; RET4 |
| Hierarchy resize | `00b87410-00b8745f` | 80 | Same schedule, separate native identity |
| Primary array destroy | `00b87b20-00b87b36` | 23 | Resize0; free current backing pointer; RET |
| Hierarchy array destroy | `00b87b40-00b87b56` | 23 | Resize0; free current backing pointer; RET |

Each header is exactly **0Ch bytes**: data at+0, integer count at+4 and integer
capacity at+8. Elements are four-byte pointer words. This is not an STL vector
layout, and none of these bodies invokes an element method, AddRef or release.

Reserve clamps signed requested capacity below16 to16, then compares current
capacity. If growth is needed it allocates `request*4` moduloDWORD through
`BF55BE`, reads current count after allocation and shallow-copies forward.
The loop rereads source data and count; a copy is skipped only when the computed
destination slot is null. After the loop it frees the current old data through
`BF6989`, then publishes the captured new data and capacity. No unused slots or
count are initialized. Do not introduce overflow guards, rollback, a bulk-copy
replacement or a whole-loop null-allocation recovery path.

Resize captures the requested count and calls reserve only when it exceeds
current signed capacity. It then reads current count and zeroes added slots,
rereading data each iteration. It compares current count again before shrinking,
decrements the actual count word repeatedly, then stores the requested count.
Removed slots and pointees are untouched. Negative counts retain the original
signed-loop and DWORD-address arithmetic; they are not clamped to zero.

Destroy always calls resize0 and then frees the **current** data pointer. Count
ends at zero; data and capacity remain stale after free. With negative capacity,
this cleanup can allocate64 bytes through reserve before final free. An empty
shortcut or unconditional `noexcept` would lose observed behavior.

Ghidra currently stops both destroy bodies at the returning `BF6989` call.
The verified tails are `83 C4 04 5E C3` (`ADD ESP,4; POP ESI; RET`) at
`B87B32..B87B36` and `B87B52..B87B56`, after calls at `B87B2D` and `B87B4D`.
The primary integrator owns these repairs and subsequent annotation/export.

## What this removes from resource teardown

`B88430`'s map at `DFBA44` unwinds hierarchy backing at state3 through
`CC25EE -> B87B40(resource+1C)`, then primary backing at state2 through
`CC25E3 -> B87B20(resource+10)`. State1 cleans the name through `41DD20`; state0
stamps the ref-counted base through `BD30F0`. The normal destructor inlines the
same array cleanup schedules after its corresponding state transitions.

The concrete raw string and ref-base bodies already exist, as do the source
allocation/free services. Resource construction `34c512e4` is complete within
its documented scope. The parent's separately verified `B7F290` cache-pair
implementation is also complete; this audit does not relabel it as missing.

`718810` frees and clears three non-owning classification triplets, then calls
`B88430`. The latter still decrements each primary item's reference count, calls
slot0 only at zero, performs mandatory `4C1400/B801C0` cache-name removal,
destroys hierarchy records and returns their native88h pool slots. None of those
ownership steps is supplied by the backing-array helpers.

The cache route requires the actual manager, `B7E7B0` find and `B7FA60` checked
iterator erase. Its library-tree contract is not permission to port STL erase or
cast the semantic `GameResourceManager` as native storage. The hierarchy route
requires `B88180/B7D7D0/B7D640` plus its distinct pool state at
`1090238/1090250/1090254/1090260`; the existing string pool is not interchangeable.

Item deletion must use finite actual profiles. Matched `D631C0` data establishes
fallback slot0=`BD30E0`, slot4=`B86990`; the fallback scalar destructor is named
but has no implementation in this worker tree. Existing input/shader/text deletion
providers do not prove arbitrary resource-item support. Typed Mesh/Note/GroupParams
payloads and the existing reader/model code remain implemented at their own
interfaces, without proving native parser-result ownership.

## Implementation ownership and evidence

Claim all six addresses above and new `native_resource_pointer_array.hpp/.cpp`,
its document and report. Implement both reserve bodies first, then resize and
destroy. Recheck leases before claiming; the observed lease list had no conflict
for these addresses or files. This packet is disjoint from resource construction,
`B7F290`, the manager/cache, `B88430/718810` and shared process-host files.
Coordinate name/reconstruction/CMake integration and Ghidra repairs with the root.

After implementation, use the strict Win32 build and existing checks; add at most
one local capsule if needed for current-header aliasing, negative count/capacity
and returning-tail behavior. This audit made no source/Ghidra changes and ran no
build, replay, test or game. All new evidence is under ignored
`local/resource-teardown-frontier-bd/`. All 1,062 files in the prior construction
delivery were rehashed and remain unchanged, including its SQLite index. The
root has also retained that complete delivery separately.
