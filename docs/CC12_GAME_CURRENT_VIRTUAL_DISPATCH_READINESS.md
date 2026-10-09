# Game current virtual dispatch readiness (CC12)

The three remaining pure call signatures have viable bounded current-table
bridges. Scalar dispatch can reuse an existing concrete implementation. Terminal
and peer-record dispatch need small direct calls with their distinct native
argument shapes. The consumer ABI for peer-record slot4 is now established by
the listing; its concrete receiver family, target implementation and lifetime
remain unqualified. This read-only packet adds no Source implementation or full
Game composition credit.

## Exact abstract interface and minimal provider

`NativeGameLifetimeCalls` declares pure `virtual_scalar` and `virtual_terminal`;
it inherits pure `virtual_04` from `NativeGameEmbeddedLifetimeCalls`. The matching
scalar/terminal declarations in the array, container, singleton and class-cleanup
bases share those signatures. The additional profile-collection and SAP bases
have concrete methods. A minimal new provider can preserve the existing abstract
interfaces and override exactly these three methods:

```cpp
struct NativeGameCurrentVirtualCalls final : NativeGameLifetimeCalls {
    void virtual_scalar(void* captured, std::uint32_t byte_slot,
                        std::uint32_t flags) override;
    void virtual_terminal(void* captured) override;
    void virtual_04(void* receiver, void* record) override;
};
```

This is a proposed ordinary C++ service, with no owner, fabricated table, callback
registry or successful empty fallback. None of the methods should be `noexcept`;
an actual target exception propagates to the existing caller's retained failure
state. These signatures neither allocate an object nor establish a production
table or destructor. No concrete derived `NativeGameLifetimeCalls` provider or
application construction of `GameNativeGameRuntime` was found in `src`/`include`.

## Scalar: preserve the byte slot and full flags word

The existing concrete
`NativeGamePhysicsLifetimeCalls::physics_virtual_scalar(void*, uint32_t, uint32_t)`
reads the receiver's current primary table, reads the requested byte-offset
entry, then invokes `void* (__thiscall*)(void*, uint32_t)` and ignores the result.
The proposed override can call this body explicitly qualified through its
inherited physics service. It must forward the exact supplied byte slot and
all flag bits; a slot is not a pointer-array index and flags must not be reduced
to a boolean or forced to 1.

The Game parent passes slot0 for E18678/E1867C/E198BC and the three grids, and
slot+0C for E18DB0/E19698/E1930C, Game+19C8 and the movie publication. These calls
push 1, use ECX as the captured object, and clear the relevant publication only
after return. Scene-list payloads, embedded entries/owners, class-cleanup
payloads and singleton cleanup also use this abstract scalar boundary. The
native parent windows `004DCFFF..004DD006` and `004DD055..004DD05D` confirm slot0
and byte slot+0C; `004BF94A..004BF951` confirms scene payload slot0.

This mechanical helper is already concrete and performs no allocation, context
substitution, catch or direct free. The actual selected scalar method owns its
destruction/free policy. A qualified call also prevents an unrelated recording
override of `physics_virtual_scalar` from becoming the default implementation.

## Terminal: current slot0, no argument and no extra decrement

`virtual_terminal(captured)` is reached **after** an interlocked decrement at
captured+4 has returned zero. Native windows in `004CC760`, `007F8180` and
`004C86A0` all then reload `[captured]`, read table slot0, set ECX to the captured
object and call with no stacked argument. The suitable bridge type is
`void (__thiscall*)(void*)`; the caller does not consume the return value.

The bridge must read the current table when invoked and call current slot0.
It must not decrement again, add a new count check, cache a pre-decrement table,
force slot+4/flags1, clear the owner's publication, or read the object after the
terminal returns.
Those choices belong to the caller or actual terminal. In particular:

- Game+21F4 is left unchanged by its parent terminal path.
- The five Game+19D4..19E4 cells are cleared after their terminal returns.
- `004CC760` clears each pointer-range cell after its terminal returns.
- `007F8180` decrements directly through Win32 before calling the service, then
  clears participant+104/+108/+10C; a service decrement hook is not involved.
- Container reserve/resize paths use their own decrement and clear schedules.

`invoke_native_ref_counted_delete_00bd30e0` supplies an explicit BD30E0 body,
which dispatches slot+4/flags1 through a separate abstract binding. It is not a
generic current-slot0 implementation: substituting it would assume every
current terminal equals BD30E0.

The particle-component fallback already demonstrates a genuine raw no-argument
current-slot0 call, but embeds it after its own decrement. Particle-resource
release also decrements and may choose a known-profile Source scalar. Neither
whole release routine can implement this already-decremented boundary.
`dispatch_native_render_resource_zero_terminal` accepts an already-zero owner,
but requires specific canonical rendering contexts, profile IDs and owner
registries; it is not an arbitrary current-table dispatcher. The online
notification helper demonstrates safe byte-copy loading of a no-argument
`__thiscall` entry, at the unrelated fixed byte slot+28, and is file-local.

## Peer-record slot4: call shape recovered, concrete family still external

In `flush_native_game_peer_records_007849c0`, a matching record reaches:

| Native sites | Actual effect |
|---|---|
| `00784AA4..AA8` | Recover the peer and load its current +4 receiver into ECX. |
| `00784AAB..AB0` | Load the receiver's current primary table, captured node+8 record, and table byte slot+4. |
| `00784AB3..AB4` | Push that record pointer and call the slot with ECX still the receiver. |
| `00784AB6..AC2` | Continue current-list traversal without caller argument cleanup. |
| `00784B04..B0B` | Fixed register pops, ADD ESP,24h and RET; there is no deferred per-record stack repair. |

The consumer therefore requires ECX=receiver, one 32-bit stacked record pointer
and callee cleanup of that pointer on normal return. No result is consumed.
A bounded bridge can load the current table/byte slot+4 and call
`void (__thiscall*)(void*, void*)`. It must pass the captured receiver and record
unchanged, with no peer reinterpretation, flags substitution, new decrement,
node free or lock action.

The previous source-only signature did not establish the machine call boundary;
the listing now does. No concrete slot4 target, recovered class name or payload
ownership was established in this packet. The bridge must require an actual
compatible method or ABI-compatible rebuilt thunk; a caller-supplied cdecl
function or an arbitrary C++ member pointer does not satisfy that requirement.
The native caller ignores results, so the semantic return type remains unknown.

## Actual tables, allocation domains and full Game composition

All three bridges require live actual receivers with a real readable current
primary table and callable entries of the required Win32 shapes. The table and
method code must remain live across the call. The selected method may retire its
receiver; the bridge must not access it afterward. Record/object lifetimes and
the scalar method's construction-compatible allocator are caller obligations.
Byte-copy reads of table and entry representations avoid inventing typed object
references. Such access is not synchronization or concurrent-mutation support.

Original-PE numbers such as CE7CB8 or D039CC are profile identities in several
reconstruction paths. Copying those numbers or copying an original table from a
read-only image does not make its entries rebuilt callable function addresses.
These bridges must not fabricate a table, treat a profile ID as an executable
Source pointer, or silently map an unknown ID to a known destructor. Actual
compatible original code would require a separately qualified loaded-image
execution environment; this packet does not establish one.

`GameNativeGameRuntime` supplies a genuine rebuilt Game-specific table whose
fastcall scalar thunk accepts the thiscall-shaped receiver/flags boundary and
validates its exact constructed owner. It accepts an externally supplied
`NativeGameLifetimeContext`; it does not create the missing lifetime service or
qualify arbitrary children. Its construction and lifetime contexts must share
the Game/grid publications and canonical Dyn construction/lifetime owners.

The lifetime context still requires the constructed graph's array, embedded,
profile, singleton, Lua-global, resource, class and physics contexts. Existing
defaults check the same call-service identity and, where applicable, the same
string-pool/manager/publication references. The retained player-profile context
provider explicitly supplies construction services only; its teardown must
share the eventual game's `NativeGameLifetimeCalls` instance.

Dispatch introduces no allocator. Inherited Game defaults still free through
`operator delete` or `std::free`, while container allocation forwards to the
singleton allocator; physics also borrows specific engine/scene memory services.
Each actual allocation/free pairing and retained context must remain qualified.
Making the three virtual methods concrete cannot repair a foreign allocation,
missing child table, original numeric stamp or incomplete owner lifetime.

## Verification boundary

Eight native instruction windows, 109 bytes total, were reread through the
target-verified read-only Ghidra CLI and matched against the installed PE. Source
signatures, call sites, raw helper implementations and runtime/context boundaries
were inspected and pinned in the [report](../reports/cc12_game_current_virtual_dispatch_readiness.json).
Only this document and report change. No Source, CMake, ledger, configuration,
Ghidra mutation, build, test, probe or native entry execution occurred. No full
Game caller/composition, application startup or gameplay admission is granted.
