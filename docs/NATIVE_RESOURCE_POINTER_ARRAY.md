# Native resource pointer arrays

This packet reconstructs the six raw backing-array bodies used by the actual
resource and hierarchy-item arrays. It covers 396 original bytes and adds no
pointee operations. Successful resource destruction remains incomplete.

The reviewed discovery is `704ec61d`,
[NATIVE_RESOURCE_TEARDOWN_FRONTIER_BD.md](NATIVE_RESOURCE_TEARDOWN_FRONTIER_BD.md).
That document lives on the earlier construction worker branch until integration;
its evidence is copied read-only into this worker's ignored `local/` directory.
The construction worktree and its 1,062-file delivery were left untouched.

| Original range, inclusive | Bytes | Source entry | Original ABI |
| --- | ---: | --- | --- |
| `00B872F0..00B8734E` | 95 | `reserve_native_resource_item_pointers_00b872f0` | ECX header, signed capacity on stack, RET 4 |
| `00B87350..00B873AE` | 95 | `reserve_native_hierarchy_item_pointers_00b87350` | ECX header, signed capacity on stack, RET 4 |
| `00B873C0..00B8740F` | 80 | `resize_native_resource_item_pointers_00b873c0` | ECX header, signed count on stack, RET 4 |
| `00B87410..00B8745F` | 80 | `resize_native_hierarchy_item_pointers_00b87410` | ECX header, signed count on stack, RET 4 |
| `00B87B20..00B87B36` | 23 | `destroy_native_resource_item_pointers_00b87b20` | ECX header, RET |
| `00B87B40..00B87B56` | 23 | `destroy_native_hierarchy_item_pointers_00b87b40` | ECX header, RET |

None has a meaningful return value. The new C++ interfaces preserve the reviewed
memory and service schedule; they are not drop-in native ABI replacements.

## Storage and ordering

Each body receives the actual 0Ch header: DWORD data at +0, signed DWORD count at
+4, and signed DWORD capacity at +8. Elements are four-byte pointer words. This
is the physical layout already declared by `NativeRenderPointerArrayStorage`;
the new functions borrow its raw address without introducing another storage
type. Existing rendering array helpers have different minimum-capacity and
negative-count behavior and cannot substitute for these bodies.

Reserve clamps signed requests below 16 to 16 and compares the current signed
capacity. It passes `request * 4` modulo DWORD to the existing concrete allocator.
Only after allocation does it read the current count. The shallow forward copy
reloads the current count every iteration and current data for every performed
copy. It skips a copy, including its source load, when that computed destination
slot is null. It then frees the current data pointer before publishing the saved
allocation and clamped capacity. Count and unused slots remain untouched.

Resize compares signed request with current signed capacity and reserves first
when needed. Its growth index starts at the count read after reserve; every
computed slot uses current data and is zeroed only if the slot address is nonnull.
Negative counts are retained and can reach negative indices. Shrink decrements
the actual current count word one at a time while its signed value exceeds the
request, then writes the request unconditionally. Removed slots and pointees are
untouched. Address, index and byte arithmetic uses DWORD wrap.

Destroy always performs resize-to-zero before freeing current data. A negative
capacity therefore causes a 64-byte allocation, possible copy, old-buffer free,
and new-buffer free. It leaves data and capacity stale after free. There is no
empty shortcut, bounds repair, rollback or `noexcept` on these entries. Allocation
failure propagates; a reached allocation failure precedes copy and free.

The implementation uses volatile raw DWORD accesses to retain current-header
reloads, including overlapping data/header observations. All reached addresses
must be valid in the original 32-bit memory domain. No synchronization or
corrupt-header recovery is implied.

## Concrete dependencies and remaining lifetime work

`singleton_lifetime_allocate` and `singleton_lifetime_free` are the existing
concrete source boundaries for `BF55BE/BF681B` and `BF6989/BF65AC`. Production
passes identical native and host byte counts on Win32. There is no new injected
host, callback provider, STL container, CRT algorithm port, AddRef or pointee
release. The two entry families share reviewed internal source helpers while
retaining six distinct public identities.

This closes the six raw backing-array operations only. Base/game resource
destruction `B88430/718810` still needs actual cache removal
`4C1400/B801C0/B7E7B0/B7FA60`, hierarchy field/pool contracts
`B88180/B7D7D0/B7D640`, and finite actual item deletion profiles. Named functions
are not assumed implemented. The distinct hierarchy pool at `1090238/1090250/
1090254/1090260` is not the string pool. Existing ref-base cleanup, raw strings,
the separately reviewed resource construction packet and cache-pair `B7F290`
remain completed dependencies within their stated scopes. Existing semantic
Mesh/Note/GroupParams parsing remains implemented; its storage does not establish
native resource ownership.

## Evidence and validation

The six complete byte spans match both the original PE and freshly read live
Ghidra bytes. Every read-only CLI batch verifies `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The report carries all ten direct body call sites
and the two parent unwind-action edges. No Ghidra mutation was performed.

Ghidra currently omits the returning free tails after calls `B87B2D` and `B87B4D`.
PE and live bytes establish `83 C4 04 5E C3` through `B87B36` and `B87B56`.
Those returning-flow/body repairs remain root-owned. Complete byte coverage here
does not assert that the saved Ghidra body ranges are already repaired.

Development strict MSVC Win32 build and both existing CTests passed. One ignored
local capsule covers six normal observations across all six original bodies:
negative-capacity primary destruction, negative-count/capacity hierarchy
destruction, allocator/free header changes, copy aliasing the header, a negative
index yielding a null slot, and a null reserve destination slot. The original
trace was frozen before any source execution; all six source traces match it.
One separate source-only observation confirms allocation exceptions propagate
without freeing or modifying the header. No original exception unwind is claimed.

## Integrator correction: saved cleanup bodies

The missing-tail notes above describe the worker snapshot. The integrator has
now restored both complete 23-byte bodies through `B87B36` and `B87B56`.
Clearing the erroneous free-call overrides decoded the tails but left the stored
body ranges short; the supported locked definition tool then recreated the exact
verified ranges. Six reviewed hypothesis names and evidence comments are saved,
with prior names and comments retained in the
[repair record](../reports/native_resource_pointer_array_flow_repairs.json).
Fresh exports and follow-up scans show ten instructions and zero remaining call
gaps in each cleanup body. This adds no source body or exception-unwind claim.
Independent source review accepted `d67779eb`; fresh merged-candidate validation
remains required. The earlier construction and frontier deliveries are retained
in the integrator worktree, and the frontier document is now integrated.

The capsule links the actual production object, verifies its unique identical
member in the current `bsp_core.lib`, and retains the archive, object, compiler
input groups, headers, link map, manifest and traces. Allocation/free adapters are
shared test-only boundaries. Header overlap, callback changes and null allocation
are deliberately injected observations, not proof that the production allocator
returns overlapping/null storage. The capsule establishes neither actual
allocator runtime, original FH3 dispatch, pointee destruction nor gameplay.

The committed report records development evidence honestly as an uncommitted
source overlay. After commit, the same build and frozen capsule are repeated on
the clean exact commit, with immutable receipts under
`local/pointer-array-final/`. No permanent tests are added. The final receipt and
whole local delivery are external so their hashes can pin the exact commit
without a self-referential evidence commit.
