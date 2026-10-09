# Actual World entity-chain drain Source (CC12)

`drain_native_world_entity_chain_009041a0` implements the bounded actual-storage
drain using the existing concrete lifetime dispatcher. It borrows the genuine
12-byte `NativeWorldChainHeader` supplied by the existing World chain-header
producer and retains that same header throughout the call. This packet adds
one ordinary C++ body. Primary review and CMake registration are complete;
the normal MSVC Win32 build and all three existing checks passed.

```cpp
void drain_native_world_entity_chain_009041a0(
    NativeWorldChainHeader* actual_header,
    NativeGamePhysicsLifetimeCalls& lifetime_calls);
```

The native body is `009041A0..00904203`, 100 bytes including an unreachable
alignment instruction. Its original receiver is ECX, with no stacked arguments
and a plain RET. This new Source interface adds an explicit borrowed service;
it is not an original binary ABI replacement. The hypothetical name describes
observed behavior. The [contract](CC12_WORLD_ENTITY_CHAIN_DRAIN_CONTRACT.md)
records the adapter, producer, call-service audit and remaining family boundary.
The World+4 adapter `00904390` is outside this Source packet.

## Storage and call-service contract

The header is a live `NativeWorldChainHeader` object: first at +0, last at +4,
DWORD count at +8. Its object lifetime and storage must survive every entity
method and the entire drain. The function never frees the header or World and
does not reload World+4. Count zero returns without reading the head or
normalizing fields.

Each nonempty head is an actual entity with live pointer representations at
+34 (previous) and +38 (next). These are intrusive entity links, with no separate
list-node payload. The receiver's current primary table must contain a genuine
callable Win32 `__thiscall` scalar method in slot0. Flags1 must have a qualified
destruction/free lifetime compatible with that entity's allocation domain.
The caller supplies a real existing lifetime-service object that outlives the
call. The drain explicitly calls:

```cpp
lifetime_calls.NativeGamePhysicsLifetimeCalls::physics_virtual_scalar(
    entity, 0, 1);
```

That base body reads the actual current table and slot0, calls the entity with
flags1 and ignores its return. The qualified call bypasses service overrides;
no new deletion callback or fabricated table is introduced. The entity method
may free the current receiver and change the retained header's fields. Numeric
original-image tables, index tokens and recording overrides do not establish
this contract. Supplying this dispatcher does not supply the entity producer,
method table or scalar lifetime.

## Preserved schedule

The Source body follows the live listing's short-circuit order: read head and
previous; if previous is null, read next; only if both are null, compare count
as a signed DWORD against 1. High-bit counts therefore follow the signed native
JG branch, while decrement remains unsigned DWORD arithmetic.

When unlinking, it writes previous.next or header.first, reloads entity.next,
then reloads entity.previous for next.previous or header.last. It clears next
before previous and decrements count before calling slot0. A skipped unlink
still reaches the scalar method. There is no iteration limit, stalled-loop
return, null-head repair or fabricated progress.

Entity fields use `memcpy` of pointer object representations through byte
storage. The implementation creates no entity struct or typed reference into
opaque storage. Header accesses use its genuine type with volatile qualification
to preserve the explicit field accesses. Volatile supplies no synchronization
or proof of exact emitted instruction ordering. The contract is a valid-storage,
single-threaded schedule; concurrent mutation and native invalid-storage fault
equivalence remain unqualified.

After deletion, the body reads only the retained header's current count and,
if another iteration is required, current head. It never reads the just-deleted
entity or carries its next pointer across the call. A scalar exception
propagates with prior unlink/count writes retained. There is no catch, cleanup
owner, rollback or `noexcept` on the drain.

## Evidence and limits

The worker inspected the live `009041A0` listing through the target-verified
read-only BSP Ghidra CLI. The report preserves the inherited exact native-byte
pin, relevant Source pins, and the signature and schedule review. Only the two
new Source files and this dedicated document/report are changed.

No worker CMake/ledger/config/Ghidra mutation, build, test, probe or native-entry
execution was performed. Build and runtime results are not claimed. Production
entity ownership, the normal World destructor, the full World table, application
startup and gameplay remain outside this packet.

## Primary review and build

The primary independently replayed the complete 100-byte disk span against the
worker pin and reread the target-verified live instruction listing. The whole
emitted drain is 115 bytes /46 instructions. Its sole external relocation calls
the concrete `NativeGamePhysicsLifetimeCalls::physics_virtual_scalar` symbol;
there is no service-table dispatch. That existing method's complete emitted
body is 23 bytes /10 instructions, loads the receiver's current table and byte
slot, calls with the flags word, and returns with stack cleanup of its three
explicit arguments. Both whole objects occur exactly once in `bsp_core.lib`.

The emitted drain retains signed JG, unlink-before-call, next/previous clearing,
DWORD decrement and current-header reload after deletion. It never reads the
deleted entity after the call. The compiler reads previous before testing the
reloaded next during reciprocal-link selection; this is qualified valid-storage
behavior, not exact native instruction ordering or fault equivalence.

Eight bounded Source/recipe fingerprints were captured before the normal build,
rechecked afterward and retained with the whole objects, library, game map and
test log in `local/cc12_world_entity_chain_drain_primary`. The build exited zero;
`reconstructed_math`, `native_math_differential` and `tool_tests` all passed.
The existing duplicate-symbol LNK4006 warning remains. No new test or probe was
added. The drain root is absent from the game map, so this is Source/build
evidence, not application runtime evidence. The existing Original function is
already counted; this actual-storage fragment adds no new Original-function
credit. See `reports/cc12_world_entity_chain_drain_primary_review.json`.
