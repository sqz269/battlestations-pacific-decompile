# Directional-light pool and allocator list

The reconstruction owns actual raw slabs through one caller-supplied
`DirectionalLightPoolStorage`. It uses the caller's `AllocatorListElement*&`
head representing native `00E188B4`. The host companion adds no second head,
slab table, free stack, slot ID, or automatic native destructor.

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Exact extents, native ABIs, live/disk hashes,
continuations, source hashes and fixture evidence are recorded in
[`directional_light_pool_audit.json`](../reports/directional_light_pool_audit.json).
Names below describe reconstructed behavior, not recovered source symbols.

## Public storage and integration

`AllocatorListElement` is the native three-word element: vtable, previous and
next. `AllocatorListDomain` takes a reference to the actual shared head and keeps
only external virtual-zero bindings. Each binding identifies the native vtable,
native virtual-zero function and actual host owner/invoker. Traversal checks the
element's current vtable, invokes the bound concrete method, then reloads that
same element's next pointer. An unknown or mismatched profile fails explicitly.
No foreign allocator receives a no-op callback.

`DirectionalLightPoolStorage` has the verified Win32 layout:

| Offset | Canonical field |
| --- | --- |
| `+00` | 12-byte allocator-list element |
| `+0c` | Actual 24-byte Win32 `CRITICAL_SECTION` |
| `+24` | Explicit signed recursion counter |
| `+28` | Raw slab-pointer table |
| `+2c` | Unsigned slab count |
| `+30` | Unsigned table capacity |
| `+34` | First slab with a free slot, or `ffffffff` |

The storage is `0x38` bytes on Win32. The companion binds the concrete pool
profile `00D62F00` / virtual-zero `00B7BA20` before native initialization exposes
the element. This permits a real allocation new-handler to traverse a pool
during construction after its native fields have been published.

The primary startup path must provide storage, a shared list/domain and a stable
companion; call `bind_static_directional_light_pool_01090154(pool)` and then
`initialize_static_directional_light_pool_00cd8080()`. The initializer constructs
the bound owner and registers `destroy_static_directional_light_pool_00ce0eb0`
through actual `std::atexit` by default. An explicit CRT registration function
can be supplied. Its return value is preserved, and registration failure does
not destroy the constructed pool. Unbound static adapters fail before proceeding.
Startup runs once; the storage, domain and companion must outlive every registered
callback. A C++ companion destructor does not invoke native shutdown or unbind it.

`allocate_directional_light_slot_00b7bd40()` and
`return_directional_light_slot_00b7b610(slot)` resolve that same static binding.
The direct pool methods support the same storage through explicit initialize,
allocate, return, trim and destroy calls. `live_slab_index(slot)` reads the
authoritative current `+1ec` word. The directional-owner constructor must preserve
it and keep any enlarged C++ companions outside the raw slot.

The existing string `SizedStoragePool` remains for names. Its inspected native
constructors do not prove membership in `00E188B4`; this change does not register
it in that list. The singleton lifetime-manager registry is also a separate domain.
Primary integration must add `src/allocator_list.cpp` and
`src/directional_light_pool.cpp` to the build and connect the actual startup path.

## Slabs, reuse and compaction

The initial table allocation is 32 pointers / 128 bytes; unused entries retain
their allocation bytes. Both the native table allocator (`00BF55BE`) and free
thunk (`00BF6989`) jump to the existing `00BF681B` / `00BF65AC` boundaries. The
reconstruction uses `singleton_lifetime_allocate/free`, including its real CRT
new-handler behavior.

Each `0x3e44` slab contains 32 raw `0x1f0` slots, a 32-entry 16-bit free stack at
`+3e00`, its 16-bit count at `+3e40`, and two untouched tail bytes. `00B7AC90`
writes count 32, free indices `31..0`, and each slot's `+1ec` slab-table ID; it
leaves the other slot bytes intact. Allocation pops slots `0..31` initially.
Return uses the slot's live ID and signed exact division by `0x1f0` to recover
its within-slab index, pushes it, and updates the minimum free-slab index.
There is no invented double-return or pointer-repair policy.

`00B7BA20` does not lock. It frees each fully empty slab, copies the last table
entry into the hole, decrements count, and rewrites all 32 IDs in a moved slab.
It retries the same index, since the replacement can also be empty, then
recomputes the first available slab. Actual slot addresses and other bytes stay
in the moved slab; only the pointer-table index changes. The table allocation and
capacity remain after all slabs are trimmed.

## Publication, failure and destruction

`00B7BAC0` enters the real critical section and increments the explicit counter.
It publishes a new first-free index before allocating a slab. If the table must
grow, it publishes `2 * capacity + 2` before allocating the replacement table.
The old table is freed before the replacement is published. Normal return
decrements recursion before leaving the critical section.

There is no allocation-path catch, RAII unlock, rollback, or compensating slab
free. If slab allocation throws, the new index and held recursion remain. If
table allocation throws, the already-created slab has not been appended and is
not freed by a local cleanup. These are deliberate native failure semantics.
The constructor has separate real unwind actions: table free `00B7ADB0`, critical
section cleanup `00402F70`, then allocator unlink `00403970`. Its MSVC `finally`
region applies only to that constructor and preserves those unwind states.

Destruction installs the pool vtable, frees every slab without invoking light
destructors, frees the table, drains positive explicit recursion with actual
`LeaveCriticalSection`, deletes the critical section, installs base vtable
`00D7A0C0`, and unlinks the same allocator element. Native stale table/count/
capacity and old link words are not replaced with zeroed host state.

Assembly was required after misleading no-return analysis at `00B7B24D`,
`00B7B9F9`, `00B7BA46`, `00B7BB56` and `00B7ADBC`. The audit includes these raw
continuations. `00CD8080..00CD8095` is a verified 22-byte initializer but was not
defined as a function in the current Ghidra program; the primary owns definition,
annotation, ledger and export integration.

## Validation boundary

Both new source modules compiled under MSVC Win32 with `/W4 /WX`. The repository
build script passed its existing `reconstructed_math` test (1/1); new-source build
registration is a primary integration step.

One isolated native-versus-host fixture passed 136 matching states and eight
matching allocation/free observations, comparing 3,714,020 raw slab bytes. Its
65 allocations create three slabs. Returning the first slab and the last slot
causes trim to remove an empty replacement at the same index and move the live
middle slab. All 32 moved IDs, including those in an empty replacement about to
be freed, are checked. Return through the new ID, exact-slot reuse and retained
payload pass. Finally, a correctly entered recursive critical section with
explicit/actual recursion two is drained during destruction and the shared head
is unlinked. Real malloc/free seed preimages before construction; four actual
Win32 critical-section APIs service native calls. No unresolved call receives
a placeholder implementation; unprovided native bytes trap.

This is reconstructed and build/fixture-tested behavior through a new C++
interface. Table growth beyond 32 slabs, injected allocation failure, native
exception unwinding, foreign allocator profiles and CRT startup/exit sequencing
were not runtime-tested by this one fixture. Byte/layout agreement does not
establish a drop-in ABI replacement or game validation.
