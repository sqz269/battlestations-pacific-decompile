# VFS mesh resource type owner readiness

**The established family is already composed in current Source.** At baseline
`e46ca137f`, the retained `GameNativeVfsApplication` owns the real Source backing
for SceneResource, render-mesh resource, skined-mesh resource and matrix-indexed
mesh resource. Its startup calls `CD8690`, `CD86F0` and `CD87B0` through the existing
initializer service, using the same application counter and common root bootstrap.
No additional backing, initializer, owner binding or CMake registration is needed
for this family. The minimum new implementation packet therefore has **zero files**.

This is a Source readiness audit, not a new reconstruction or execution result.
Only this document and its report change. No new Native body/data/handler window,
live Ghidra analysis query, Ghidra mutation, C++/CMake/ledger/config edit, build,
test or probe was performed. The exact lease anchor is `00CD86F0`.

## Canonical backing and established initializers

`GameNativeTypeStorage` contains the following guards and fixed-size arrays;
its constructor binds `NativeMeshResourceTypeStorage` directly to these members.
The owner cannot be copied or moved. Its public `mesh_resource_types()` accessor
returns the same stable aggregate view without copying IDs or initializing cells.
The numeric addresses in member names identify recovered roles; these Source
members are not a claim that the owner maps storage at original absolute addresses.

| Family | Guard role | Descriptor role and words | Existing initializer |
| --- | --- | --- | --- |
| SceneResource | `0109020C` | `01090210`: own, root, name | `B869C0` |
| Render-mesh resource | `01090440` | `01090444`: own, scene, root, name | `B93BD0` caller target; `CD8690` global |
| Skined-mesh resource | `01090441` | `01090454`: own, mesh, scene, root, name | `CD86F0` |
| Matrix-indexed mesh resource | `01090442` | `01090468`: own, mesh, scene, root, name | `CD87B0` |

`NativeMeshResourceTypeIds` borrows the counter, root bootstrap and these same
guards/descriptors. Binding itself does not reset or initialize them. Existing
Source preserves the important schedules:

- `B869C0` publishes its process guard and numeric name `D631E4`, initializes
  root through `BEA780`, copies the root ID and consumes the shared counter.
- The caller-target `B93BD0` publishes guard/name `D637A4`, invokes SceneResource,
  and alternates parent loads and destination stores. Global `CD8690` instead
  captures both parent IDs before either store.
- `CD86F0` and `CD87B0` test their own guards, capture the mesh guard before
  publishing their own guard/name, and use that captured decision for the base
  initializer. They capture all three inherited IDs before any inherited-ID
  store. Their names remain numeric `D637B8` and `D637CC`.
- Consumption uses `get_006FAC20()->next_id_04`, stores its unsigned increment
  before returning the captured old value, then publishes that value as own ID.
  Zero is valid after wrap. Guard publication and partial writes survive failure;
  there is no rollback, nonzero filter, counter saturation or added synchronization.

The process guards also govern caller-target initializers. A call on another
descriptor can suppress a later global initialization; a readiness integration
must not reset guards or manufacture a replacement global descriptor to mask that
behavior. The current retained startup already calls the intended global entries.

Current implementation anchors are `game_native_type_storage.hpp:79`,
`game_native_type_storage.cpp:16,55`, `native_mesh_subset_loading.hpp:163` and
`native_mesh_subset_loading.cpp:435`.

## Actual retained counter, root and lifetime route

The route is concrete in `game_native_vfs_application.cpp`:

1. `Impl` owns one `type_counter_0109DB7C` publication cell, `owner_services`,
   `type_storage` and `common_types`. Its `NativeVfsPublicationCells` borrow the
   existing host manager cell `01090AA0` and that same counter cell.
2. `NativeVfsOwnerServices` constructs its `SoundLifetimeAccess` from the manager
   cell and its `TypeIdCounterLifetime` from that access plus the counter cell.
   `types()` returns that retained member by reference.
3. `common_types(owner_services.types(), type_storage.light_types())` binds the
   common root bootstrap to that exact counter and the owner's root/light cells.
   The application constructor binds shared deletion services before type getters.
4. `initialize_core()` calls
   `type_storage.initialize_resource_types(owner_services.types(), common_types)`.
   That method constructs `NativeMeshResourceTypeIds` from the same arguments and
   `mesh_resource_types_`, then calls all three global mesh initializers. The same
   service supplies SceneResource dependencies for the earlier selector and extra
   resource initializers. Memory/physical stream types and VFS registration follow.
5. `GameVfsHost` retains the application in `native_`; `native_types()` and
   `native_owners()` return its existing owners. Phase 2 calls `initialize_core()`.
   The application rejects a second attempt and does not unwind unknown partial
   native ownership into a fabricated successful startup.

`require_common_bootstrap` checks identity of all shared root/node/light storage
references. It does not introspect the bootstrap's private counter reference.
The actual application route above establishes the same-counter property by
construction; arbitrary external callers must still honor that API contract.

Counter registration uses the existing raw manager domain. Deletion bindings point
to the same retained `TypeIdCounterLifetime`; the `CFB6C4` destruction case dispatches
to its existing `6FAD40` implementation. That Source destructor clears the counter
publication and conditionally frees the owner; it does not reset descriptor guards
or unregister itself. The earlier CP fixture's explicit cleanup description must
not be substituted for this application destruction contract.

The application header requires its owners and borrowed services to survive the
shared drain. Current `GameStartupHost` shutdown drains singletons before deleting
the VFS owner; its renderer drain delegates to the same singleton host. An
interrupted VFS operation is retained through process exit. These are inspected
Source lifetime paths, not newly executed shutdown evidence.

## Order, consumer and family boundaries

Current `initialize_resource_types` orders selector initializers, camera object,
model/model-base, animation/bone resource, the explicitly inserted group initializer,
and then `CD8690/CD86F0/CD87B0`. The group insertion remains a Source composition
decision. Earlier fallible work can stop the sequence before mesh initialization.
The retained CP evidence identifies the three mesh CRT cells
`CE35F0/CE35F8/CE3600`; it does not establish the full application CRT order or
absolute numeric IDs produced after every original startup entry.

The available `NativeMeshResourceTypeCalls` adapter compares current descriptor
IDs at the real slot-C targets and forwards other targets. A complete bounded
identifier search over current `include/bsp` and `src` finds its declaration and
implementation, but no game-side construction or consumer of `mesh_resource_types()`
beyond the owner's accessor declaration. Thus this owner audit does not establish
attachment to every production classifier or resource graph. Any separate consumer
packet needs its concrete forwarding call chain, current item profiles/selectors,
and lifetime bindings established first; this report proposes no such code packet.

`01090454` skined-mesh **resource** remains distinct from excluded `01090370`
skined-mesh and from camera-resource `01090288`, skin-model `01090344` and compact
`0109042C`. None of those families supplies substitute cells or dependencies here.
Their writer ownership and initializer contracts were not reopened by this audit.

## Evidence and next action

The current bounded ledger lookup marks `CD86F0` complete and points to
`native_mesh_subset_loading.cpp`. The retained
[CP classification record](NATIVE_MESH_RESOURCE_CLASSIFICATION_CP.md) covers 14
already reconstructed bodies, 690 bytes, historical native getter/predicate oracle
and classification fixtures. Those 14 routines receive **no new reconstruction
credit** here. Its old branch-only status and then-unproved startup/selector
statements are historical; the current checked-in composition above is the basis
of this readiness finding. The CP startup initializers were Source-executed, not
original-machine-code differential oracles. This packet reruns none of that work.

The report freezes 21 selected current Source/configuration files and three
historical context files, all matching baseline Git blobs after CRLF normalization.
It retains complete bounded search outputs and a 4,180-row tracked Source inventory.
Existing build registration includes the initializer service, retained type owner,
VFS owner services and application route. Registration is not a fresh build result.

Close the proposed mesh type-owner implementation gap as **already Source-composed**.
Preserve these existing bindings. There is no remaining Source dependency to fill
within this owner packet and no reason to add another startup call or descriptor.
Original full CRT execution, native ABI compatibility, application/runtime behavior,
consumer attachment, gameplay and rendered parity remain separate unproved claims.

Primary review independently froze every reported artifact occurrence and matched
all24 selected baseline Git blobs against current Source/context. The actual
retained VFS route passes the same owner_services.types() into common-root
construction and mesh initialization; owned stable arrays back the borrowed
descriptors. Seven complete bounded query receipts and the4,180-row Source
inventory are retained. The owner gap is already Source-composed; add no
duplicate descriptor or startup call. Production classifier attachment, original
CRT execution/order, ABI and runtime/gameplay remain separately held. No new
build, fixture, Native body query or reconstruction credit is added.
