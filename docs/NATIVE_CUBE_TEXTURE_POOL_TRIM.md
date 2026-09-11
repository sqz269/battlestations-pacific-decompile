# Native cube-texture pool trimming

This packet reconstructs complete `B3E690..B3E72F` (160 bytes), the actual
`D61944` slot0 trim operation for canonical cube pool `0108DB70`. Native ABI is
ECX actual pool, RET, with no semantic result. The new
`trim_native_cube_texture_pool_00b3e690(void*) noexcept` C++ interface borrows
actual Win32 pool, table and slab storage; it is not a binary replacement.
Descriptive names remain hypotheses.

The raw pool is 38h bytes, with allocator node +00/+04/+08, critical section
+0C, depth +24, table +28, current count +2C, capacity +30 and earliest +34.
A cube slab is 6C4h bytes: 32 slots of 34h bytes with a DWORD pool index at
slot+30, 32 free-index WORDs at +680, a WORD free count at +6C0 and two untouched
padding bytes at +6C2. These offsets come from fresh full native bytes and the
cube route documented in `NATIVE_TEXTURE_POOL_GLOBAL_STARTUP_NEXT.md`.

## Free, compact and rescan

The unsigned index is compared with current count on every scan. Each reached
iteration reloads current table and the current slab pointer. Only an exact
WORD free count of 32 causes actual `singleton_lifetime_free`, the existing
host CRT boundary corresponding to native `BF65AC`. There is no logical-owner
destructor call.

The 55-byte returning-free continuation `B3E6B6..B3E6EC` is absent from the
current Ghidra listing but present in both live bytes and the installed PE.
It reloads current table and count, captures the current last pointer and
stores it at the current index **before** decrementing count. If a slab moved,
it reloads that current table/slab and rewrites all 32 DWORD pool indices at
+30 with stride34h. Decrementing the outer index causes the following increment
to rescan the replacement, including a replacement that is also entirely free.
Ignored trailing table words remain stale; capacity does not shrink.

After removal, native captures the current nonempty-count test before writing
earliest=`FFFFFFFF`. If that captured test was true, it captures the table
cursor once, rereads current count at each later loop test, and publishes the
first slab with nonzero WORD free count. It does not clear payload, free-stack
or padding bytes, relink allocator nodes, touch the critical section, or change
explicit recursion depth. Reached pool/table/slab extents must be valid.

## Actual allocator-list binding

`bind_native_cube_texture_pool_trim_00d61944(void*, AllocatorListDomain&)`
binds the actual first 12-byte allocator element to concrete profile `D61944`,
native entry `B3E690`, actual pool context and this complete trim function.
It is host dispatch setup, not another native function. It creates no private
head, pool, slot state or substitute pool companion. A repeated identical
binding is idempotent and does not change raw bytes or publish the node.

Callers establish this binding in the same actual shared-list domain before
pool initialization publishes the node. Metadata allocation therefore happens
before publication; a real CRT new-handler can subsequently dispatch the
published pool through the existing `AllocatorListDomain::trim_all_004b46b0`.
Pool lifetime and canonical static initialization remain separate packets.

## Original-body comparison

The focused ignored fixture captures four fresh live-Ghidra/installed-PE spans,
202 bytes: the entire 160-byte trim, complete 33-byte `4B46B0` list traversal,
five-byte free entry and actual four-byte `D61944` profile. Each query verifies
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. The installed
executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The fixture rechecks the PE bytes at execution and protects code/profile pages.
It executes the untouched full trim body, and also executes original
`4B46B0 -> D61944 -> B3E690` dispatch from an actual borrowed head/node.

Only the profile's function pointer and global-traversal head operand relocate;
the external free entry bridges to an observer that performs actual host CRT
free. Source execution links the actual primary-library trim, binding, shared
list and free providers, verified by linker-map entries. It does not replace
the owned native operation with a source callback. Explicit pointer identities
are normalized while actual raw pool fields, all live slab bytes and inactive
table entries remain in the comparison.

One phase deliberately redirects current table/count **after actual free**
inside the observer. This controlled service-boundary perturbation verifies
reload order; it is not claimed as normal CRT free behavior. Other phases
exercise moved-empty rescan, last-slab deletion, all-free removal, no available
slab, and zero count with an invalid unused table value. Surviving payload,
free-index stack and padding bytes, all 32 moved tokens, unchanged capacity
and recursion, and stale inactive table words are checked. No freed-memory
read is used.

The comparison passed with 45,651 identical DWORD trace entries (182,604
bytes) and six actual slab frees on each side. All four runtime span
postimages match only the two listed absolute relocations and external free
bridge; the owned 160-byte trim body remains unpatched. Five linker-map
provider checks confirm the real primary-library trim, binding, shared list
and free implementations. The actual CRT free DLL is verified as Win32 PE014c.

`./scripts/build.ps1` passed with both existing CTests, and all eight native
seed spans matched disk. An ignored CMake source-registration include places
the owned production translation unit in the primary `bsp_core` library;
the focused strict Win32 fixture links that exact library. No tracked tests
were added. The audit report records source, executable, library, trace,
linker-map and supporting-script hashes.

Permanent shared CMake/ledger/packet changes and Ghidra annotations remain
integrator-owned. Repair the returning-free flow at `B3E6B1` and refresh its
export under the write lock; unrelated jump-alignment gaps stay untouched.
No game validation or complete process startup is claimed by this packet.
