# Shared observer edge registration and deleting dispatch

This packet closes concrete edge creation, repeated registration, and canonical
deletion for the actual storage from `observer_lifetime.hpp`. The implementation
does not add another graph or reference-count representation. The existing
`NativeObserverLifetime` now invokes the actual canonical deleting wrapper after
removing an edge from both endpoints; noncanonical tables retain their required
virtual service.

The target was verified as `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, on the configured loopback8089 bridge. Assembly and
bytes are authoritative. `BSP_` names below are hypotheses, not recovered
symbols. `CG_scalar_deleting_dtor_00693ca0` records the observed compiler-wrapper
identity; it is not labeled as an original game symbol.

## Storage and public API

`006948A9..006948B5` initializes an actual 16-byte allocation as:

| Offset | Value |
| --- | --- |
| `+0` | Native vtable `00CF7E64` |
| `+4` | Actual first endpoint address from ECX |
| `+8` | Actual callback-owner address from EDX |
| `+C` | DWORD reference count1 |

The dword at `00CF7E64` is `00693CA0`. The following words refer to unrelated
singleton wrapper tables; this packet does not infer further edge virtual slots
from adjacent data.

The new header `include/bsp/observer_edges.hpp` exposes:

```cpp
NativeObserverEdgeStorage* create_observer_edge_00694850(
    NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& owner,
    NativeObserverLifetime& lifetime);
void register_observer_pair_00694a60(
    NativeObserverOwnerStorage& first, NativeObserverOwnerStorage& owner,
    NativeObserverLifetime& lifetime);
NativeObserverEdgeStorage* delete_observer_edge_00693ca0(
    NativeObserverEdgeStorage* edge, std::uint32_t flags) noexcept;
```

Creation and registration use the existing shared lock singleton and pair
lookup. Callers pass actual endpoint bases, including a voice entry's actual
embedded `callback_owner_18`; they must not substitute entity IDs or semantic
metadata addresses. All object and pointer-array allocations use the canonical
`singleton_lifetime_allocate/free` CRT boundary.

## Recovered sequences and ABI

`00694850` has ECX=first endpoint, EDX=callback owner, EAX=new edge, plain RET.
It captures the current shared section, enters it, allocates/initializes the
edge, appends that pointer to the first endpoint, then appends the same pointer
to the callback owner, unlocks, and returns the pointer. It does not add a
pending-dispatch entry.

Each append compares count against a captured capacity. Equality triggers
`capacity = old_capacity * 2 + 2`, with the capacity store before allocation.
The DWORD byte product is `capacity * 4`. After allocation, the loop reloads
current count and source data, copies pointer cells with native null-destination
guards, frees the old allocation, and publishes the replacement. It then reads
the current data/count, writes the new cell when its computed address is
nonnull, and increments count. The same endpoint may be supplied twice;
native consequently appends the same pointer twice to that one array.

`00694A60` has ECX=first, EDX=owner, plain RET, with no promised EAX result.
It captures an outer lock, calls the separately locked `006949D0` lookup, and
either calls separately locked creation or increments the found edge's DWORD
reference count modulo32. Repeated registration does not append duplicate
entries or allocate another edge.

The EH funclets at `00C7E970` and `00C7E990` only destroy the captured lock
guard through `00411EE0`. They do not delete a newly allocated edge or roll
back an earlier append if a later allocation fails. This partial-progress
behavior is retained. No exception-driven graph repair was invented.

`00693CA0` has ECX=edge, flags DWORD on stack, EAX=original edge address even
after free, RET4. It writes `00CF7E64` and calls `_free` only when bit0 of flags
is set. Endpoint pointers and reference count are otherwise untouched. It does
not unregister or decrement: native callers already did those operations.

The shared helper in `src/observer_lifetime.cpp` reads the current edge vtable
after both endpoint erases. At native call sites `00695399` and `00695654`, table
`00CF7E64` now binds directly to `delete_observer_edge_00693ca0(edge,1)`. Other
tables still call `ObserverLifetimeServices::delete_edge_virtual_00`; no unknown
edge subtype is assigned the canonical destructor.

## Ghidra changes and boundaries

The worker's explicitly authorized writes used the existing write lock and
lease checks. `00694850` had falsely terminating `_free` calls hiding three-byte
`ADD ESP,4` instructions at `00694914` and `00694984`. Both were repaired and
saved with prior flow state in `reports/observer_edges_flow_repair.json`. The
`0069495D..0069495F` gap after an unconditional jump is alignment padding and
was left alone.

`00693CA0` initially had no function start. Verified bytes established the
31-byte range `00693CA0..00693CBE`, final instruction `00693CBC RET4` of length3.
`tools/ghidra_define_function.py` disassembled that exact range and created the
function, then saved. Creation reported a 28-byte stored body even though all
11 instructions were listed. The false `CALL_RETURN` at `00693CB1` survived
because its continuation was already decoded and the usual gap detector found
no missing instruction.

A separately authorized, local orchestration call cleared that one supported
flow-override endpoint under the lock. The result records `CALL_RETURN -> NONE`,
and pseudocode now returns the original pointer after free. Before/after
prototype, listing, flow and pseudocode are retained in
`reports/observer_edge_definitions.json`, followed by successful save/export.
No Ghidra script, deletion/recreation, callee no-return change, or body-extension
operation was used. Stored inclusive bounds remain `00693CA0..00693CBE`; the
prototype metrics still report10 instructions versus11 in the complete listing.
The initial 28-byte membership discrepancy is retained as a metadata limit;
correct returned-pointer pseudocode is not proof that every stored body byte
was added. Root owns final names, comments, prototypes and that follow-up.

No missing start remains in this packet's three-function closure. Two unrelated
adjacent singleton wrappers were only identified while delimiting the vtable:
`00694250..00694278` (final `00694276 RET4`, length3) and
`00693E10..00693E38` (final `00693E36 RET4`, length3). They had no function starts
at inspection, are outside this packet, and were neither leased nor changed.

## Validation

`./scripts/build.ps1` passed for Release MSVC Win32 and existing CTest
`reconstructed_math` passed1/1. No permanent tests were added.

One ignored fixture passed via `cmd /c local\run_observer_edges_check.cmd`.
It compiles `local/observer_edges_check.cpp` with `/std:c++17 /EHsc /MD /W4 /WX`
against the built `bsp_core.lib`. Real storage, CRT allocation, Win32 recursive
sections, singleton registration, and canonical edge creation/deletion are used.
The noncanonical edge-deletion service throws if called, so it cannot fake the
canonical cleanup. The fixture covers repeated registration, capacity2-to6
growth with retained edge identity, pending-slot holes, modulo count wrap,
reference-ignoring owner destruction, same-endpoint duplicate append/removal,
deleting flags2/3, return-address preservation, and singleton shutdown.

Allocation-failure/new-handler mutation and malformed storage were not injected;
their sequence is assembly-backed. Native differential, original ABI/SEH,
concurrent teardown and game-runtime behavior remain unvalidated. Actual endpoint
construction, callback invocation and dispatch-vector population are outside
this packet. Canonical edge lifetime itself is concrete and connected.
