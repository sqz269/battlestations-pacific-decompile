# World entity-chain drain contract (CC12)

A bounded actual-storage implementation is feasible using the **existing
concrete** `NativeGamePhysicsLifetimeCalls::physics_virtual_scalar` body for
`(entity,0,1)`. No new stub deletion callback is needed. This establishes a
mechanical call boundary, not a production entity family or complete World
table: supplied entities must already have genuine callable current tables
and qualified scalar-deletion lifetimes. This packet is read-only, Source0.

## Actual receiver and payload

The complete `00904390` adapter is 8 bytes /2 instructions: load
ECX=`[World+4]`, then tail-jump to `009041A0`. The complete drain is 100 bytes
/43 instructions, with ECX=actual header, no stack arguments and plain RET.
It saves ESI/EDI; ESI retains the same header throughout. World+4 is read once,
not reloaded after entity destruction.

`009037F0` supplies two separate actual12-byte headers. World+4 receives its
first allocation, before the second allocation for World+8. The drain consumes
only World+4. Current `allocate_native_world_chain_headers_009037f0` supplies
this storage using the real singleton allocation domain and the exact
`{first+0,last+4,DWORD count+8}` layout. Its fresh empty state is `{0,0,0}`.

The header's pointers identify **the entity itself**, with previous/next links
at entity+34/+38 and the primary method-table pointer at entity+0. There is no
separate12-byte list node or payload pointer at node+8. Category-header clear70,
which frees separate nodes and leaves their payloads borrowed, is a different
lifetime. Header allocation does not establish the entity allocation domain.
The drain frees neither its header nor the World and does not clear World+4.
The header must survive every entity callback and the entire drain.

## Exact loop, unlink and reload order

| Native sites | Effect |
|---|---|
| `009041A6..A9` | If header count is zero, return without reading its head or normalizing any fields. |
| `009041B0..B2` | Reload the current head into ECX; read that entity's previous pointer. |
| `009041B5..C2` | Unlink if previous is nonnull, next is nonnull, or **signed** count<=1. Otherwise skip all unlink writes. |
| `009041C8..D3` | Write previous->next from entity+38, or replace header.first when previous is null. |
| `009041D5..E7` | Reload entity.next; write next->previous from entity+34, or replace header.last when next is null. |
| `009041EA..F0` | Zero entity.next, then entity.previous; decrement header count as a DWORD. |
| `009041F4..FA` | Read this entity's current primary table and its slot0; ECX stays the actual entity; push flags1 and invoke the scalar method. |
| `009041FC..FF` | Reload count from the retained header; if nonzero, reload its current head at B0 and repeat. |

The signed comparison is the actual JG at C2, not an unsigned test or simply
count==1. A future Source body should preserve the DWORD decrement and the
native read/write ordering. It must not carry a cached next entity across the
destructor: the callee may free the current entity and modify head/count.
There is no read of the just-destroyed entity after the virtual call. The
method's return value is ignored.

There is no iteration bound, stalled-loop exit, null-head repair or diagnostic
failure state. Count-nonzero/head-null faults natively. If unlink is skipped,
the destructor must still be called; the next iteration depends on what that
real destructor does to the retained header. This contract does not invent
progress for an incoherent or non-progressing owner.

No local EH frame, catch or rollback exists. A throwing scalar method leaves
all already performed unlink/count writes intact and propagates out; no next
iteration or header free follows. A future ordinary C++ drain must therefore
not be marked noexcept or add an owning cleanup object. General Native
FH3/SEH and invalid-storage fault equivalence remain outside that interface.

## Existing Source APIs and the usable service

`drain_entity_chain_009041a0` currently operates on `DeferredDestroyChain` and
an abstract `WorldDeferredDestroyHost`. It collects statistics and has an extra
`stalled` return when an unlinked destructor leaves count/head unchanged. That
exit is explicitly non-native. No production subclass of that drain host was
found in `src`. This interface cannot be relabeled as the actual-storage body.

`WorldExpiryHost::destroy_entity_vtable0` is also an abstract boundary. Its
current `GameHost` binding logs `unimplemented`, increments a counter and marks
an index token's expiry counter -1. It neither invokes nor frees an actual
polymorphic entity. `NativeGameLifetimeCalls::virtual_scalar` and the related
container/embedded interfaces remain pure virtual; their call sites are not
concrete implementations.

There is, however, an existing real implementation in
`src/native_game_physics_lifetime.cpp`:

```cpp
using Method = void* (__thiscall*)(void*, std::uint32_t);
auto method = reinterpret_cast<Method>(word(ptr(p), slot));
(void)method(p, flags);
```

`ptr(p)` reads the actual32-bit primary table pointer; `word(...,slot)` reads
the selected byte-offset entry. `(p,0,1)` therefore performs the required
Win32 receiver/one-flags-word call and ignores the return. The body has no
profile substitution, catch, counter, stall guard or allocation. It is not
noexcept, so exceptions propagate. It is already registered in bsp_core by
`cmake/startup.cmake:164`; no build or new emitted-code qualification is claimed
by this inspection.

Its public contract explicitly requires **actual callable tables**, and rejects
numeric original-image vtables as Source tables. Reusing this concrete body
does not grant the physics attachment types' lifetime to a World entity. A
future bounded drain should bind this existing default body explicitly (for
example, a qualified call through the borrowed service), or independently
qualify an actual override. It must not accept a recording/no-op override as
proof of scalar deletion. No new `destroy_entity` facade is needed.

The typed `NativeEntityIdTableProfile::scalar_delete` is a C++ function pointer
whose header expressly says it is not an original thiscall pointer; it is not
an interchangeable table entry here. `GameNativeGameRuntime` has a real scalar
entry, but validates its own exact Game owner identity; it is not a World
entity deletion service. Those objects are not substitutes for genuine entity
receivers with +34/+38 links and a compatible current slot0.

## Bounded next implementation and remaining ownership

The next Source packet can take an actual `NativeWorldChainHeader*` plus the
existing lifetime service, reproduce the loop above, and provide the World+4
receiver adapter. It can reuse the producer's header type, real pointers and
the concrete slot0 call, with no generic callback class, stalled exit, fake
entity or fabricated table. Its caller must supply live storage and genuine
current entity scalar methods whose flags1 destruction/free domain is valid.

The production entity producer/table/lifetime connection remains unqualified
here. No entire polymorphic family was inspected. The normal World destructor
and full World method table remain separate; the drain's implementable consumer
contract does not establish their readiness. The [report](../reports/cc12_world_entity_chain_drain_contract.json)
retains exact Native bytes and current Source/service pins. Only this document
and report change: no Source/build/tests/probes/Ghidra/shared metadata writes.
