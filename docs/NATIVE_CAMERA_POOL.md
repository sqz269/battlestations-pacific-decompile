# Native camera pool

`NativeCameraPool` allocates actual raw camera slots through one caller-supplied
`NativeCameraPoolStorage`. It reuses the canonical `AllocatorListDomain` bound to
the actual shared `00E188B4` head, the shared CRT allocation/free implementation,
and real Win32 critical-section calls. Names describe reconstructed behavior;
they are not recovered C++ symbols.

Evidence comes from `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, checked against the installed PE. Exact function
ranges, original ABIs, hashes, continuations and validation are recorded in
[`native_camera_pool_audit.json`](../reports/native_camera_pool_audit.json).

## Canonical storage and lifetime

The caller supplies stable storage, a shared allocator-list domain, and the host
companion. Binding the companion does not initialize the raw owner. There is no
second slab table or list head. The Win32 owner is exactly `0x38` bytes:

| Offset | Field |
| --- | --- |
| `+00` | Native allocator element: vtable, previous, next |
| `+0c` | Actual 24-byte `CRITICAL_SECTION` |
| `+24` | Signed explicit recursion counter |
| `+28` | Actual slab-pointer table |
| `+2c` | Unsigned slab count |
| `+30` | Unsigned table capacity |
| `+34` | First nonfull slab, or `ffffffff` |

The companion binds concrete profile `00D62CE0` / virtual-zero `00B716D0` before
initialization publishes the allocator element. Initialization prepends that
element, initializes the section and pool words, and reserves 32 pointers
(`0x80` bytes). Allocation uses `singleton_lifetime_allocate`; its real CRT
new-handler behavior is shared with the existing owners. Free uses
`singleton_lifetime_free`. Unknown allocator profiles retain the canonical
domain's explicit failure behavior.

For native static identity `0108FFB0`, bind the supplied companion with
`bind_static_native_camera_pool_0108ffb0`, then call
`initialize_static_native_camera_pool_00cd7dd0`. Startup constructs that owner
and registers `destroy_static_native_camera_pool_00ce0e30`, using `std::atexit`
by default. A supplied CRT registration function must be real; its result is
returned and failure does not undo construction. Unbound static adapters fail.
Storage, domain and companion must outlive callbacks; startup runs once. The
C++ companion destructor does not destroy, unlock or unregister the native owner.

`allocate_native_camera_slot_00b71930` and
`return_native_camera_slot_00b71350` resolve the same static binding. The return
wrapper's actual native range is **`00B71350..00B7135B` (12 bytes)**: it takes the
slot in ECX, pushes it, binds the pool in ECX, calls the direct return method
(which pops that stack argument), then returns without popping arguments. The
earlier discovery's ten-byte extent truncated the CALL and is superseded by
this verified range. These public functions expose new C++ interfaces.

## Raw slots and compaction

Each `0x8BC4` slab contains 32 slots of `0x45C` bytes. Every slot's `+458` word is
its current slab-table index. `00B6FED0` sets these 32 IDs, writes the free-index
stack `31..0` at `+8B80`, and writes count 32 at `+8BC0`. The slot prefixes and
two tail bytes at `+8BC2` retain their allocation preimages. No camera constructor
runs when allocating a raw slot, and camera construction must preserve `+458`.

Allocation initially returns slots `0..31`. Return reads the slot's live ID,
recovers its within-slab index by native signed division by `0x45C`, pushes the
index, and updates the minimum free-slab index. Valid slots belong to this pool;
there is no added pointer repair or double-return policy.

Trim is the concrete allocator virtual-zero method and does not lock internally.
It frees each fully empty slab, moves the last table pointer into the hole,
decrements the count, and rewrites **all 32** moved IDs. It retries the same index
because the replacement may also be empty, then finds the first nonfull slab.
The moved slab's actual addresses and payload bytes remain stable. Trimming all
slabs preserves the allocated table and its capacity.

## Ordering and failure

Allocation enters the actual section, increments explicit recursion, and
publishes the first-free index before allocating a slab. On table growth it
publishes `2 * capacity + 2` before allocation, copies current pointers, frees the
old table, then publishes the replacement. It decrements recursion before leaving
the section on normal return. There is no allocation-path catch, rollback,
automatic unlock or compensating slab free on failure.

The constructor has separate unwind states: `00B6FFC0` frees the pointer table,
`00402F70` drains positive recursion and deletes the actual section, then
`00403970` unlinks the actual allocator element. Its MSVC `finally` region is
limited to construction. Destruction frees every slab without invoking camera
destructors, frees the table, drains positive recursion, deletes the section,
installs base vtable `00D7A0C0`, and unlinks the same element. It leaves the native
stale table/count/capacity and old link words intact.

Assembly establishes the return continuations after free at `00B7113D`,
`00B71155`, `00B716A9`, `00B716F6`, `00B71806`, and `00B6FFCC`; misleading
no-return analysis cannot terminate these reconstructed functions early.

## Validation and integration

Strict MSVC Win32 compilation passed with `/std:c++20 /W4 /WX /fp:strict /MD`.
The existing repository build script and `reconstructed_math` check passed (1/1).
The new module was compiled and linked explicitly for the fixture; primary
integration adds `src/native_camera_pool.cpp` to CMake and connects startup.

One isolated native-versus-host fixture passed 136 matching states, eight matching
allocation/free observations, and 8,336,740 matching raw slab bytes. Its 65 raw
allocations create three slabs. Empty-replacement retry, all 32 moved IDs,
return/reuse after movement, retained payload, full trimming, shared allocator
traversal, and destruction with actual/explicit recursion two all match. The
fixture executes verified original bodies in its own process, uses real
malloc/free with deterministic preimages and actual Win32 section APIs, and
records every explicit boundary patch. It does not inject into the game.

Table growth past 32 slabs, injected allocation failure, exception unwinding,
foreign allocator profiles, concurrent access and actual CRT startup/exit
sequencing were not runtime-tested. These paths rely on assembly/source evidence
and explicit bindings. This is reconstructed, build-tested and fixture-tested
behavior, not a drop-in ABI replacement or game validation.
