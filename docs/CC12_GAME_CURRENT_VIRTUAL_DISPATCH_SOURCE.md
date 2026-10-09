# Game current virtual dispatch Source (CC12)

`NativeGameCurrentVirtualCalls final : NativeGameLifetimeCalls` supplies the
three concrete virtual-call overrides from the
[reviewed readiness contract](CC12_GAME_CURRENT_VIRTUAL_DISPATCH_READINESS.md).
This is new Source glue with **zero Original-function credit**. It changes
neither the existing lifetime implementations nor their abstract interfaces.
Registration, compilation and complete emitted-code review remain with the
integrator; no build or runtime result is claimed here.

```cpp
void virtual_scalar(void* captured, std::uint32_t byte_slot,
                    std::uint32_t flags) override;
void virtual_terminal(void* captured) override;
void virtual_04(void* receiver, void* record) override;
```

## Actual call behavior

Scalar calls explicitly qualify the inherited concrete
`NativeGamePhysicsLifetimeCalls::physics_virtual_scalar`. The captured receiver,
exact byte offset and full flags word pass through unchanged. That existing
body reads the current primary table and chosen entry, invokes the actual
Win32 scalar method and ignores its return. No service override substitutes for
this qualified body.

Terminal calls freshly copy the receiver's current primary-table pointer and
byte-slot0 method representation, then invoke `void (__thiscall*)(void*)` with
the captured receiver. The caller has already decremented the reference count.
There is no second decrement, count check, scalar-flags argument, fixed BD30E0
substitution or caller-cell clear.

Peer-record calls freshly copy current table byte-slot4 and invoke
`void (__thiscall*)(void*, void*)`. ECX receives the unchanged captured receiver;
the unchanged captured record is one callee-popped pointer argument. No peer
lookup, list change, record free or lock action occurs in this service.

The two new raw bridges copy pointer representations with `memcpy` through byte
storage. They introduce no fabricated object reference, cached table, profile-ID
translation or synthetic method table. All three methods make no receiver
access after their target call. None is `noexcept`, and no catch, rollback,
owning cleanup or successful empty callback is added. Target exceptions
propagate to the existing caller's failure handling.

The header rejects non-MSVC/non-Win32 targets. Compile-time assertions require
four-byte object, DWORD and method pointers and a concrete, non-abstract
`NativeGameCurrentVirtualCalls`. These assertions await the integrator's build;
their presence alone is not compilation evidence.

## Preconditions and composition limits

Each receiver must be actual live storage with a readable current primary table
containing genuinely callable methods of the documented Win32 shapes. The
table and method code must remain live across the call. Real target behavior
must be qualified for the receiver's owner, construction and allocator domain.
Peer-record access/lifetime must satisfy the actual method's contract. A target
may retire its receiver; the bridge does not access it afterward.

Numeric Original-PE profile identities and copied Original entries do not become
rebuilt callable tables. The service does not produce a table or accept a
recording override as proof of real destruction. Byte-copy access provides no
synchronization or native invalid-storage fault equivalence; concurrent table
mutation is outside the contract.

The service owns no receiver, allocation or context. It must outlive every
borrowing context and retained operation. The full Game still requires the
constructed graph's matching lifetime contexts, same service identity, actual
publications, canonical owners and compatible allocation/free domains. Existing
Game scalar ownership, child families, player-profile teardown and application
composition are not supplied by these three methods.

## Evidence and verification

The prior audit matched eight native call/epilogue windows totaling 109 bytes
against the installed PE. Its pinned report supplies the consumer ABI evidence;
this Source packet adds no native entry or new Ghidra analysis. The dedicated
[report](../reports/cc12_game_current_virtual_dispatch_source.json) pins the new
files and reused declarations/dispatcher, and distinguishes inherited native
evidence from the pending Source build and emitted-code review.

Only the new header, implementation and this document/report change. No CMake,
ledger, configuration or Ghidra mutation, build, test, probe or native execution
was performed. No Original ABI replacement, full Game composition, application
startup or gameplay readiness is claimed.

## Primary build and emitted review

Core registration and the concrete-class/Win32 assertions now compile. The complete scalar bridge is 12 bytes / 5 instructions: its18h service-base adjustment and direct physics tail call retain all explicit argument words. Terminal is 16 bytes / 8 instructions and peer is 20 bytes / 9 instructions; current table/slot loads, payload ECX, record forwarding and argument cleanup match their Source contracts, with no after-call receiver access. The concrete physics dispatcher is separately retained/reviewed at 23 bytes / 10 instructions. These three roots remain absent from the game map.

The normal MSVC Win32 build exited zero and all three existing checks passed. Seventeen bounded physical Source/recipe fingerprints were captured before the build, rechecked afterward and retained with whole objects, unique exact Core members, library, map and logs in `local/cc12_current_dispatch_primary_review`. No new tests/probes or current application runtime checks were performed. See `reports/cc12_current_dispatch_primary_review.json`.
