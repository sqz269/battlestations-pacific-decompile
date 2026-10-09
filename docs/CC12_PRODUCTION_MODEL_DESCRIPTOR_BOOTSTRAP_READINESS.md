# Production model descriptor bootstrap readiness

Root reviewed this bounded packet and replayed its retained Source/capture pins. It adds no implementation or new runtime credit. The current Root build authority is the separate 509-input primary review; historical Source121 and reported Source507 context remain scoped to their captures.

Decision: the persistent model descriptor/guard and its bootstrap can use the
application's existing genuine type domains. Startup initialization remains held
on one specific evidence gate: the original placement of 00CD7E60 in the CRT
initialization schedule. The checked Source has no real lazy-call alternative.
This is a missing production member and scheduling evidence, not a missing Native
model type constructor or counter provider.

The inspected worker Source is 705b9d10beb94891822178c1bd9c3676c0ac34df. This
packet changes only this document and its report. It adds no C++, CMake, tests,
probe, Native body/address work, ledger or GPR query/mutation. The preceding
renderer candidate is present in this worktree; this packet claims none of its
pending integration/build/runtime evidence.

## Existing complete contract

ModelTypeBootstrapStorage binds a byte guard 01090030 and the four-word descriptor
01090034. ModelTypeDescriptor has own ID, node ID, root ID and native name address
at offsets 0/4/8/C. Its POD declaration does not initialize storage; constructing
ModelTypeBootstrap only binds references. It must use the same TypeIdCounterLifetime
instance as its LightTypeBootstrap, not merely another counter with equal values.

The static initializer's existing Source sequence is:

1. Return without writes when the shared guard is any nonzero byte.
2. Publish guard 1, then the recovered name-address word 00D62DD4.
3. Initialize the same shared node descriptor, which uses its existing root/node
   guards and the same counter.
4. Capture both parent IDs before either model parent store, then store them.
5. Obtain the shared counter, capture next_id_04, increment it with unsigned
   wrapping, and finally publish the captured model own ID.

The lazy initializer shares that same guard but can target another descriptor.
It interleaves each parent load/store instead of the static entry's two-value
capture. Initializing an alternate target can leave the canonical model descriptor
untouched while preventing later canonical initialization. It is not a substitute
for verified startup scheduling.

The type/name/predicate leaves read current canonical words directly. They neither
initialize nor check the guard; a zero token can match initial zero storage. The
name is a recovered numeric address word, not a host string pointer. No callback,
cached token array or hard-coded model ID can replace these contracts.

These behaviors are present in src/model_type_bootstrap.cpp:17-69 and its header.
The historical MODEL_TYPE_BOOTSTRAP.md explicitly excludes process startup ordering
and game execution from its fixture claims; those claims are not rerun here.

## Genuine production ownership is already available

| Required domain | Existing production owner | Readiness |
| --- | --- | --- |
| Model guard/descriptor | GameNativeTypeStorage is the stable owner of the other process type cells. | Add the missing model guard, four-word descriptor and reference aggregate here; no current model member exists. |
| Shared counter object and publication | GameNativeVfsApplication::Impl.type_counter_0109db7c; NativeVfsOwnerServices::types(). | Already the same publication and lifetime service passed to common_types and existing family initializers. Do not create or prepublish another counter. |
| Root/node guards and descriptors | Impl.type_storage.light_types(), borrowed by Impl.common_types. | Already persistent. ModelTypeBootstrap obtains the actual node through common_types.storage(); do not copy its IDs into a substitute parent descriptor. |
| Shared singleton lifetime/deletion | NativeVfsOwnerServices binds its actual TypeIdCounterLifetime before initialize_core. | Existing native counter registration and current-profile deletion providers are present. |
| Failure retention and one-attempt startup | initialize_core's attempted flag; GameVfsHost::invoke_native and requires_process_retention; GameStartupHost's VFS exit policy. | Existing path can retain a failed model initializer without resetting guards or replaying the attempt. |

The actual wiring is at src/game_native_vfs_application.cpp:53-59,79-100:
owner_services owns the counter context over the existing publication, and
common_types is constructed from owner_services.types() and type_storage.light_types().
initialize_core passes that very same service and bootstrap to the current type
families. NativeVfsOwnerServices::bind_deletion binds the same context for singleton
destruction; no new deletion callback is required.

GameNativeTypeStorage is noncopyable and nonmovable. Its reference aggregates point
to its own stable cells. The recovered addresses identify their roles; these are
host-owned Source members, not fixed-address mappings. require_common_bootstrap
checks every shared light-family guard/descriptor identity before initialization.
That check cannot discover a
different counter hidden inside LightTypeBootstrap; the actual VFS constructor's
same-object wiring establishes the counter precondition.

A bounded future implementation can add a retained model guard/descriptor and
model_types() reference view to GameNativeTypeStorage. Binding that view must not
consume an ID or reset live cells. Fresh Source process storage may follow the
owner's existing initial-storage construction; rebinding/reusing storage may not
zero it. If a retained bootstrap service is exposed, it can be a VFS Impl member
after common_types, borrowing owner_services.types(), common_types and that model
view. Its lifetime and every borrower must stay inside those existing owners.
The existing storage class's temporary initializer-adapter pattern is also valid
for a call-only entry; the descriptor/guard, not the adapter, must be retained.

No model-pool slot, model-node constructor, atlas, renderer, fixture descriptor or
synthetic Native owner is needed merely to own the descriptor and execute its
already reconstructed initializer. Actual model production remains separate work.

## Initialization placement is the remaining gate

Current initialize_resource_types preserves the represented sequence: resource
selectors, camera, extra resource types and mesh families. VFS then initializes
the memory-stream type, the separate physical-provider pool, and the physical
stream type. The model family is absent from that sequence.

Neither those Source call sites nor the inspected model-bootstrap documentation
establish where omitted 00CD7E60 belongs in the original CRT order. Function
addresses are not an initializer-order receipt. Inserting it after the camera,
before the extra families, at the end, or on first renderer access would consume
the shared monotonic counter at an unverified point and change later IDs.

A scoped search of all .cpp/.hpp/.inc files under src and include/bsp finds
initialize_static_00cd7e60 and initialize_00b74f90 only in their own declarations
and definitions. There is no real Source lazy invocation whose current schedule
could supply a separately evidenced path. No new lazy call is proposed.

The next evidence gate is therefore a verified original CRT placement for the
existing static entry (or separately established genuine lazy caller schedule).
This packet does not open Native data to obtain it. A storage-only member could
be implemented now, but must remain uninitialized and receive no startup/type-ID
admission until that scheduling evidence exists. No missing Native initializer
body, counter allocator or model constructor blocks the storage composition.

## Failure and retirement schedule

The model guard is published before dependency work. A throw after that point
leaves the guard and any partial descriptor/counter/parent changes intact. Do not
clear a guard, reconstruct the owner, retry initialize_core or compensate a
counter increment. initialize_core marks its attempt before type calls;
GameVfsHost::invoke_native records a thrown operation, and the existing startup
retention policy exits without CRT cleanup while keeping the graph alive.

All descriptor and bootstrap borrowers must retire before their owner disappears.
On normal shutdown the shared singleton manager drains while VFS/type services
are still retained. Its existing CFB6C4 route calls the same counter destructor,
which clears the counter publication but does not reset type guards/descriptors.
The renderer is released before GameVfsHost at src/game_hosts.cpp:1598-1602; VFS
type storage survives the shared drain and its renderer borrowers. A new bootstrap
member after common_types would be destroyed before common_types, type_storage
and owner_services. It must not recreate a counter or rerun initialization after
drain. Descriptor member destruction requires no new Native retirement callback.

## Evidence boundary

The report pins bounded current Source/documentation and the exact search results.
No build, tests, probe, runtime or artifact replay was performed. Source121 is
frozen historical evidence, with 121 selected inputs, four artifacts, 37 selected
whole objects, 41 selected Core roots and three checks; those are not a complete
application dependency manifest or total Core membership.

At task dispatch Root reported a newer 507-input build with three checks, pending
emitted review/smoke and admission. This packet does not claim that build or replay
old Source121 artifact hashes against the now-current build paths. No original
ABI/FH3, full startup, model numbering, visual or gameplay result is admitted.
