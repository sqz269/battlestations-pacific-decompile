# World post-base entity-header contract (CC12)

**IMPLEMENTED_UNBUILT:** a bounded C++ helper now implements `009037F0`'s storage
and publication schedule over supplied actual World storage. Its entire Native
89-byte/34-instruction body has one allocation dependency, already represented
by the qualified host CRT boundary. The helper neither creates the World owner
nor makes the full World lifetime ready. It is a new C++ Source interface, not
a binary replacement. Original-function credit remains 0: this address already
has a projected reconstruction. Actual-storage fragment acceptance, CMake
registration and build validation remain with the integrator.

The earlier [base-constructor audit](CC12_RAW_WORLD_BASE_CONSTRUCTOR_READINESS.md)
retains authority for `004CB030`, its 97 category headers and matrix sentinel.
The earlier [hierarchy audit](CC12_NATIVE_WORLD_HIERARCHY_OWNER_SOURCE_READINESS.md)
retains authority for complete attachment/unlink schedules. This contract closes
only post-base allocation, its exact publication/failure state and the local
caller unwind boundary; it does not repeat or admit either larger owner.

## Exact entry and normal schedule

The hypothetical name is `BSP_World_AllocateEntityChains`. The original ABI is
Win32 member-call style: ECX is the borrowed actual World, the low byte of the
first 32-bit stack argument is the active byte, and the second 32-bit stack
argument is ignored. Both normal exits consume eight bytes with RET8. EAX is
the second allocation result. ESI/EDI are explicitly saved/restored; the usual
EBX/EBP nonvolatile contract is retained. EDX is not an input. This is not a
two-argument CDECL routine, despite the incomplete Ghidra prototype.

| Site | Ordered effect |
|---|---|
| `009037F0` | Read the first stack argument's low byte before saving registers. |
| `009037FC` | Store DWORD 0 at World+4A8. |
| `00903802` | Store the active BYTE at World+4A4; adjacent bytes stay untouched. |
| `00903808` | Call CDECL `00BF681B(0x0C)`; caller removes four argument bytes. |
| `00903814`, `17`, `19` | For a nonnull first allocation, zero DWORDs header+4, +0, +8 in this order. |
| `00903822` | Publish the first result at World+4, including a returned null. |
| `00903825` | Call the same CDECL allocator again with 0x0C. |
| `00903831`, `34`, `36` | For a nonnull second allocation, zero DWORDs header+4, +0, +8 in this order. |
| `0090383A` / `00903841` | Publish the second pointer / null at World+8; return that value in EAX. |

The first pointer is published **before** the second allocator is entered.
There is no table install, callback registration, entity traversal, reference
decrement, sentinel operation or ready-byte write in this body. In particular,
World+4AC is unchanged. Reinitializing an already owning World would overwrite
its old pointers and World+4A8 without releasing them; the intended admission
precondition is fresh post-base storage, with no existing ownership in those
fields. The actual caller supplies the same zero-initialized 0x4BC World that
completed `004CB030`.

## Physical header layout and deletion profile

Both allocations are exactly 12 bytes, with native four-byte pointers and
DWORD count. Neither has a vptr, embedded deleter, allocation prefix, sentinel
or node allocation. The three field meanings are confirmed by the actual
consumer/producer `009258F0`, rather than inferred from the zero stores.

| World slot | Header+0 | Header+4 | Header+8 | Entity linkage |
|---|---|---|---|---|
| +4 | first entity | last entity | count | previous+34, next+38 |
| +8 | first root entity | last root entity | count | previous+40, next+44 |

`009258F0` appends every attached entity through World+4. With a null hierarchy
parent it also appends through World+8; a nonnull parent instead owns the inline
header at parent+48/+4C/+50. These headers are distinct from the base constructor's
category headers at World+18, whose order is **count/head/tail** and whose
allocated node lifetime has different providers. Calling the category-header
constructor or destructor is not part of the recovered `009037F0` schedule.

The concrete host allocation profile is
`SingletonAllocationRequest{SingletonAllocationKind::object, 0x0C, 0x0C}` with
two direct `singleton_lifetime_allocate` calls in the new helper. This
keeps native and host storage sizes equal and returns actual storage, not an
opaque token or C++ vector index. The matching raw deallocator is
`singleton_lifetime_free`, representing Native `00BF65AC`. No per-header
destructor callback is needed or evidenced. Those services preserve the host
allocation/free domain; they do not claim original CRT exception-object or
new-handler identity.

Native normal World destruction `00904C40` first runs entity teardown and fixed
step/matrix passes. Root entities use `00926D90(7)` through +44; all entities use
`00922FD0` and dynamic entity vslot+DC through +38. `00874D00` and `00904600`
remain real dependencies. They are named boundaries, not newly reconstructed
or replaced by empty callbacks here. Only afterwards does `00904D82..00904DA7`
free World+8 then World+4 through `00BF65AC`, nulling each slot **after** its free
returns. The headers are not passed through a virtual deleting destructor.
The later World+4A8 refcount/vslot0 release is a separate owned object.

The exported destructor pseudocode incorrectly stops at `_free` calls. The
disk bytes and current live bytes establish the omitted continuations:
`00904D8F ADD ESP,4; 00904D92 MOV [ESI+8],EBX` and
`00904DA2 ADD ESP,4; 00904DA5 MOV [ESI+4],EBX`. Ghidra was not mutated.

## Allocation failure and caller unwind ownership

`00BF681B` is a throwing allocator: its full 105-byte/33-instruction body retries
`malloc` through `__callnewh` and throws `bad_alloc` when the handler declines.
The Source service currently implements that same malloc/handler/throw policy
using the host CRT. Native null-result branches are nevertheless real bytes in
`009037F0` and must remain in a body-preserving implementation; returning null
is not the observed exhausted-allocation behavior of this binding.

| Exit | Resulting World state |
|---|---|
| Both allocations return nonnull | +4 and +8 each own a distinct zeroed header; +4A8=0; active byte set. |
| First allocator throws | +4/+8 retain their entry values; the earlier +4A8 and active-byte writes remain. |
| Second allocator throws | +4 owns the published first header; +8 retains its entry value; no local free occurs. |
| An allocator hypothetically returns null | Its explicit null arm publishes null and normal execution continues; this does not establish destructor safety. |

There is **no EH frame or rollback in `009037F0`**. In the actual caller
`004DE610`, state0 covers base construction only. At `004DE688` EBP becomes -1;
`004DE68F` writes that -1 to the EH state after the two argument pushes;
`004DE696` publishes the World at Game+19CC; `004DE69C` invokes `009037F0` with
both stack words 1. Thus the World is already externally reachable and the
caller's constructor cleanup state is inactive when either header allocation
can throw.

The ten-byte handler `00C671A0` loads FuncInfo `00D8FD04` and jumps to
`00BF6B43`. Its 36-byte descriptor has magic19930522, maxState45, unwind map
`00D8FD28`, zero try blocks and EHFlags1. State0's physical eight-byte entry is
`{-1,00C66FC0}`. That eight-byte action loads the saved raw allocation and tails
to `0042B100`; the complete eight-byte `0042B100` pushes ECX, calls `00BF6989`,
pops ECX and returns. The five-byte `00BF6989` jumps to `00BF65AC`. This frees
the raw World after a base-construction exception; it is not an active cleanup
for the later header allocation call. No caller-local post-base rollback is
present. Outer-frame or process cleanup is outside this packet and is not
asserted absent.

Normal `00904C40` is not an acceptable recovery callback for this partial
state: `00904C73` loads World+8 and `00904C76` dereferences it before a null
test; World+4 is likewise dereferenced at `00904C9E`. A second-allocation throw
on the actual fresh caller leaves +8 zero. Adding RAII cleanup of the first
header, clearing Game+19CC, catching the exception, or invoking the full World
destructor would change the native schedule. Any future recovery policy needs
its own explicitly qualified owner contract.

## Bounded Source implementation

[`native_world_chain_headers.hpp`](../include/bsp/native_world_chain_headers.hpp)
declares `NativeWorldChainHeader` and
`allocate_native_world_chain_headers_009037f0(void*,uint32_t,uint32_t)`.
[`native_world_chain_headers.cpp`](../src/native_world_chain_headers.cpp)
implements that new normal C++ interface. It uses MSVC Win32 guards and static
assertions for 12-byte standard-layout/trivially-destructible headers, native
pointer widths, and offsets0/4/8. The third argument is intentionally unused.
Only the second argument's low byte is stored at World+4A4.

The helper writes owner fields with `memcpy`, preserving the byte widths
without inventing a complete World C++ class or aliasing it through a different
layout. Placement default initialization begins each trivial header object's
C++17 lifetime without writing fields or allocating storage again; the explicit
stores then zero last, first and count in the Native order. It retains both
allocation calls, null arms and publication between calls. There are no owning
locals, exception catches, cleanup callbacks or calls to a World destructor.

The Source preserves this storage/exception schedule; native register behavior,
RET8, exact instruction emission and original CRT object identity are not
claimed. No assembly was added. The public helper requires actual supplied
post-base World storage and does not substitute a vector, opaque token,
test-owned World producer or generic callback facade. Compilation and build
acceptance remain pending; full World admission still depends on actual
owner/base/table/EH contracts and normal teardown providers. This fragment
does not advance startup or game readiness.

Current `GameWorldHost::build_entity_chains_009037f0` populates vectors of unit
indices and records a summary. It supplies neither these raw allocations nor
their native publication/failure behavior. It is not the physical implementation
of this contract and was not changed.

## Evidence and scope

Analysis used the existing `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Each live CLI call verifies project `bsp`, program
path, x86 language and image base via `Client.verify`; no imports, annotations,
flow repairs or other project writes occurred. The machine-readable
[report](../reports/cc12_world_post_base_header_contract.json) retains exact
Native spans, SHA-256 hashes, the live/disk comparisons and current Source pins.
This packet changes exactly the new dedicated header/source and this
document/report. No CMake/config/ledger edits, builds, tests, fixtures, probes
or Native entry execution occurred. The integrator will register the helper
and perform the normal build. Source review and JSON/diff checks do not
constitute build or runtime evidence.
