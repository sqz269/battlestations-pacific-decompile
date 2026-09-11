# Native MeshObject pool

`NativeMeshPool` implements the storage owner at native `0108FFF8`, with
`NativeMeshPoolStorage` holding the actual allocator-list element, initialized
Win32 critical section, slab-pointer table, and native counters. The companion
receives the same `AllocatorListDomain` used by other pools; it creates no
replacement list, pool, payload object, or abstract allocate/return service.
The caller explicitly binds this owner for the static entry points.

This adds concrete pool lifecycle to the earlier
[mesh ownership audit](MESH_OBJECT_LIFETIME.md). Mesh construction and destruction
remain in the separate native mesh owner implementation. Returning a raw slot
does not destroy its payload, and destroying the pool does not call any mesh
destructor. These are new C++ interfaces with Win32 storage offsets, not callable
replacements for the original executable's ABI.

## Native evidence

Every function below was read through `bsp.py ghidra` after its built-in target
verification of `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Full spans
matched the installed PE, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
[The report](../reports/native_mesh_pool.json) preserves span bytes, hashes,
original names/comments, final instruction addresses and lengths, original ABI,
and remaining Ghidra repairs. Descriptive names are reconstruction hypotheses.
Neither the saved analysis nor the installed executable was modified here.

| Address | Exclusive end | Implemented behavior and original ABI |
| --- | --- | --- |
| `00B727F0` | `00B72833` | Initialize slab metadata; ECX slab, stack index, EAX slab, RET4 |
| `00B728D0` | `00B728DE` | Free nonnull table; ECX pool+28, RET |
| `00B72CE0` | `00B72D6A` | Destroy slab/table storage, section, and list element; ECX pool, RET |
| `00B72DA0` | `00B72E08` | Return raw slot; ECX pool, stack slot, RET4 |
| `00B72F70` | `00B72F7C` | Push incoming ECX slot, select canonical pool, call return, RET |
| `00B73890` | `00B73963` | Construct/register storage; ECX pool, EAX pool, RET |
| `00B73970` | `00B73A10` | Trim empty slabs; ECX pool, RET |
| `00B73A10` | `00B73B4C` | Allocate raw slot; ECX pool, EAX slot, RET |
| `00B73B60` | `00B73B6A` | Ignore incoming ECX, select canonical pool, tail allocation |
| `00CD7E40` | `00CD7E56` | Construct canonical pool and register CRT atexit; EAX registration result |
| `00CE0E40` | `00CE0E4A` | Select canonical pool, tail destruction |

## Storage and slot layout

The owner occupies 38h bytes on Win32. `+00/+04/+08` is the shared
`00E188B4` list's vtable/previous/next element, with concrete vtable `00D62D50`
and virtual-zero target `00B73970`. `+0C` contains the 24-byte Win32 critical
section; `+24` is its signed explicit recursion counter. `+28/+2C/+30` is the
slab-pointer table/count/capacity, and `+34` is the earliest available slab or
`FFFFFFFFh`. Compile-time assertions check these offsets and the owner size.

Each slab is exactly 1844h bytes. It contains 32 C0h-byte slots, each with a
BCh-byte mesh payload and a pool-owned DWORD at `slot+BC`. This trailing index
is outside the mesh object and must survive its constructor and destructor.
At slab `+1800` there are 32 uint16 free indices, initialized `31..0`; the
uint16 available count at `+1840` starts at 32. The initializer changes no
payload byte and does not touch the final two slab bytes at `+1842`.

Allocation enters the actual critical section and increments `+24`. When
`+34` is the sentinel, it publishes the new slab index before allocating
1844h bytes; the slab initializer uses the live index after that allocation
returns. Pointer-table growth publishes `2*capacity+2` before allocating,
copies the live pointer count, frees the old table, installs the replacement,
and appends the slab. It decrements the free count and pops that uint16 index,
so a fresh slab issues slots in ascending order. Exhaustion searches only later
slabs. Both normal returns decrement `+24` and leave the critical section.

Return reads the current slab index from `slot+BC`, performs the native signed
low-32-bit pointer subtraction divided by C0h, pushes the uint16 slot index,
then increments the live memory count and lowers `+34` with an unsigned
comparison. It checks no membership, alignment, bounds, or duplicate-return
condition. Valid pool membership is a caller invariant.

Trim has no internal critical-section entry. Each completely free slab is
freed, replaced by the last slab pointer, and removed from the count. For a
moved slab, all 32 trailing DWORDs are rewritten to the new table index,
including slots that are currently free. The loop retries that same index
because the replacement can also be empty, then recalculates `+34`.

## Lifecycle and exceptions

Construction prepends the actual element to the shared list, publishes its
concrete vtable, initializes the section and fields, and publishes capacity 32
before allocating the initial 128-byte table. Its three native unwind leaves
are `00CC1BA0` (base unlink), `00CC1BA8` (section destruction), and `00CC1BB3`
(table cleanup). State map `00DFAB58` chains state 2 to 1 to 0 to -1; the
descriptor is `00DFAB70`. The implementation preserves that order with MSVC
`__try/__finally`, including Win32 abnormal termination.

Allocation has no local exception-unwind region: a throwing slab or table
allocation leaves its published fields and entered critical section in their
native state. There is no scope guard, rollback, or safe null-return promise.
The existing `singleton_lifetime_allocate/free` boundary supplies the concrete
CRT allocation/new-handler/free behavior already used by the native model pool.
It allocates actual table/slab bytes; it does not substitute heap allocations
for individual mesh slots.

Destruction frees every slab, then the pointer table, before draining positive
explicit recursion with matching `LeaveCriticalSection` calls, deleting the
section, restoring the base vtable, and unlinking the same allocator element.
Table/count/capacity and old list-link fields are not cleared. The host
companion's C++ destructor performs no implicit native cleanup.

The static initializer requires a prior binding, constructs that exact owner,
and returns the CRT registration result; registration failure does not roll back
construction. The callback destroys the same owner. The owner, companion, and
shared list domain must outlive callbacks; startup/shutdown must each be called
once. A different canonical companion binding fails explicitly.

## Ghidra handoff and validation

Read-only flow inspection found missing continuation instructions after calls
`00B73944`, `00B73991`, `00B72CF8`, `00B72D10`, and `00B728D7`. The full matched
native spans establish that every free call returns and that trimming continues
through `00B73A0F`. Allocation's old `00B73AA1` issue is already repaired in the
current analysis. `00CD7E40` needs a function definition ending at the one-byte
RET at `00CD7E55`, exclusive end `00CD7E56`. Repairs, Ghidra renames/comments,
and export refresh are left to the primary integrator; this worker only updates
repository evidence/name/reconstruction ledgers.

MSVC Win32 Release compilation and both existing CTests pass after freshly
verified seed ranges. One ignored focused fixture exercises the real shared
model/mesh allocator list and Win32 section: 1,025 allocations (table 32 to 66),
ascending slot selection, payload-preserving reuse, moved-slab metadata for all
32 slots, repeated empty-slab trimming, canonical atexit selection/result, and
draining two actual recursion entries during shutdown. Source and log hashes
are retained in the report. This is a host fixture and original-byte audit;
the existing native math test does not establish mesh-pool ABI compatibility,
original mesh-instruction execution, rendering, or game validation.
