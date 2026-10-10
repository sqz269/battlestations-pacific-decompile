# Mesh resource classifier consumer readiness

**The predicates and descriptor owner are available, but the retained application
consumer is not attached.** At baseline `5ec099c2a`, complete Source adapters expose
the genuine classification route. The current `GameNativeResourceApplication`
retains manager/parser registration and exports borrowed contexts; it does not
construct the root-dispatch, classifier, mesh-loading or resource-deletion graph.
A production attachment is held by specific missing bindings, including the
fallback type cells at `0109021C`, not by another mesh descriptor or startup call.

Only this document and its report change. This packet uses Source, build
registration and five bounded indexed classifier lookups. No Native body, data,
handler, table or PE-byte window was reopened; no live Ghidra analysis or mutation,
C++/CMake/ledger change, build, test or probe occurred. New reconstruction,
build, runtime and gameplay credit is zero.

## Available bindings and their identities

The accepted [VFS owner review](CC12_VFS_MESH_RESOURCE_TYPE_OWNER_READINESS.md)
establishes current `GameNativeTypeStorage`, `owner_services.types()`, common
SceneResource/root dependencies and their shared lifetime. That owner is already
retained by the actual VFS application and exposed by `GameVfsHost::native_types()`.
The current constructors and startup calls need no duplication.

| Consumer input | Genuine current Source binding | Constraint |
| --- | --- | --- |
| Mesh predicate storage | `types.mesh_resource_types()` | Copying this aggregate copies borrowed references/pointers, not the volatile ID values. Keep the same owner alive. |
| `B936B0` render-mesh predicate | `mesh_01090444[0..2]` | Own, scene, root; excludes name. |
| `B939C0` skined-mesh-resource predicate | `skined_01090454[0..3]` | Own, mesh, scene, root; excludes name. |
| `B93A40` matrix-resource predicate | `matrix_01090468[0..3]` | Own, mesh, scene, root; excludes name. |
| Classifier `type64_E19B64` | `types.resource_selectors().aux_00e19b64[0]` | Current MAux own-ID cell, not a chosen skined-resource ID. |
| Classifier `type54_E19A98` | `types.resource_selectors().convex_00e19a98[0]` | Current MConvexObject own-ID cell. |
| Classifier `type44_E19BE4` | `types.resource_selectors().geom_mesh_00e19be4[0]` | Current MGeomMesh own-ID cell, not a chosen render-mesh ID. |
| Classifier fallback tokens | Actual current `0109021C/01090220/01090224` cells | Consumer contract exists; retained backing/writer binding was not found. |

`NativeMeshResourceTypeCalls::matches_type` implements exactly the three named
predicate targets and forwards everything else to its supplied `other_`. Each
predicate compares current IDs in order, without initialization, nonzero filtering,
snapshotting or name comparison. An unknown target requires its genuine provider;
neither `false`, success, a guessed target nor a circular forwarding chain supplies it.

The production selector families already receive their own IDs through the same
SceneResource/counter service. The historical CP fixture explicitly chose selector
values to exercise lists `44` and `64`. Those fixture assignments are not production
bindings or expected production outcomes for these mesh items. The consumer must
borrow the real selector cells even when no derived-list predicate matches.

## Exact Source attachment contract

The following edges exist in complete Source bodies; their retained application
construction is the missing composition:

1. `dispatch_native_resource_items_00B7E970` captures the selected parser's current
   slot `8`, invokes `calls.parse_item`, then reloads manager `+24`, captures that
   resource's current slot `C`, and invokes `calls.append_item`.
2. `NativeGameResourceDispatchCalls` handles captured target `71BB40` by calling
   `append_and_classify_native_game_item_0071BB40`. Renderer hooks, parser calls and
   other append targets forward to its supplied actual dispatch bindings.
3. `71BB40` first runs base append `B87AA0`, including null, and returns after that
   append if the item is null. Otherwise it tests current selector cells in order
   `64`, `54`, `44`, adding the raw pointer only to the first matching derived list.
   The item table is reacquired between tests. For `64`, the token precedes the
   table capture; for `54/44`, the table precedes the getter and slot-C read.
4. The classifier handles captured predicate `B86950` using its three current
   fallback cells. Other targets go to `context.other_types`, which must reach the
   existing `NativeMeshResourceTypeCalls` for `B936B0/B939C0/B93A40` and preserve
   genuine forwarding for other item types.
5. `NativeMeshResourceCalls` handles parser targets `B947A0/B94850/B94900`, retaining
   separate acquisition frames. It forwards other parser targets, renderer hooks
   and every append call. `NativeDefaultResourceDispatchCalls` supplies only the
   proven root hooks and base append; it forwards all parser calls and other appends.

The mesh factories stamp Source profile identities `D63738/D6375C/D63780`.
The game-resource constructor `71B810` stamps `CFD8CC` and initializes its three
derived-list triplets. Classification reads the actual current table at runtime;
numeric original entries are dispatch identities, not callable original code.
Readable real profile data and its lifetime are prerequisites. No Native table
was opened or new profile-to-slot mapping inferred by this audit.

`NativeDefaultResourceLoadCacheCalls` already implements its known metric and
default/game resource factories, and forwards stream opens to the existing VFS
binding. It still requires genuine remaining metric/factory calls. The retained
`NativeResourceLoadCacheContext` also needs actual VFS/allocation publications,
name resolution and the root-dispatch context. Merely storing a mesh predicate
adapter would not create this loading route.

## Actual production call sites and remaining gaps

Current `GameVfsHost` constructs `GameNativeResourceApplication` from the shared
singleton host, strings and mapped data. The resource application's complete body
creates manager/parser publication contexts, verifies the shared lifetime/string
domain, installs deletion bindings and exposes registration/getter operations.
It has no type-owner, classifier, mesh-loading or root-dispatch member.

`GameVfsHost::raw_resource_manager_context()` and
`raw_game_resource_parsers_context()` correctly forward to those retained contexts.
Their current bounded Source search finds the two accessor declarations but no
production call site. `native_types()` separately exposes the real stable type owner.
These available accessors do not themselves attach a loading/classification route.

The complete game-file attachment search returns no occurrences of the classifier,
mesh type adapter, root-dispatch context, mesh parser adapter or default load-cache
adapter constructions/accessor calls. Wider Source searches find the standalone
implementations and interfaces. The particle runtime, for example, borrows a load
cache and resource-reference provider from its caller and checks domain identities;
it does not supply those missing owners. These are bounded identifier findings,
not proof against every possible unnamed, external or runtime route.

| Held contract | Current evidence | Requirement before attachment |
| --- | --- | --- |
| Fallback `0109021C[3]` backing/publication | Classifier pointer and `B86950` read predicate only; no dedicated Source owner/writer found. The fallback item constructor only stamps profile/refcount. | Establish actual retained cells and producer/initialization contract; do not reuse SceneResource IDs, adjacent data or a zeroed stand-in. |
| Real remaining type/root-dispatch calls | Existing adapters retain explicit forwarding references; no game construction found. | Identify each reached concrete target/provider and its retained owner. A rejecting or successful terminal invented for this packet would not close the graph. |
| Mesh loading and renderer services | `NativeMeshLoadingContext` requires the same reader/metadata, geometry, material/effect/texture and generator domains, with persistent acquired frames. | Bind the actual retained services and failure retention. Do not substitute the CP hot zero-pass fixture for cold shader/effect providers. |
| Container/item deletion | Complete container and mesh reference adapters exist, but no retained application composition was found. | Bind the same manager/strings, hierarchy pool, geometry owners and genuine remaining reference dispatch through resource retirement and shared drain. |

The generic mutable CRT data owner exposes its recovered CRT cells; it is not an
established fallback type descriptor/provider. No fallback guard, full descriptor
layout, Native initializer address or CRT position is inferred from `0109021C` or
its neighbors.

## Ownership and failure semantics to preserve

Base append and classification do not increment item or mesh references. Derived
lists borrow pointers; exceptions preserve any preceding primary/derived append.
The game-resource destructor frees derived-list storage without releasing their
pointees, then its base destructor decrements the current primary items exactly
through the real reference route. Its native null-item behavior is not repaired.

`NativeResourceContainerReferences` routes the real game container and fallback
profiles, and its item context calls back through that same adapter. A composed
`NativeMeshResourceReferences` handles current mesh-item scalar deletion using the
same `NativeRenderActualOwners` geometry domain. Remaining references still require
their actual forwarding provider. The hierarchy pool must be the same initialized
pool used by parsing. Descriptors, contexts, profile data, readers, raw strings,
manager publications, geometry and acquisition frames must survive all consumers,
loaded-resource destruction and the applicable shared drain.

## Minimum next packet

No consumer C++ implementation packet is ready while these bindings are held.
The smallest independent prerequisite is
`cc12_resource_classifier_fallback_type_writer_discovery`:

- Own `docs/CC12_RESOURCE_CLASSIFIER_FALLBACK_TYPE_WRITER_DISCOVERY.md` and
  `reports/cc12_resource_classifier_fallback_type_writer_discovery.json`, plus its
  ignored evidence directory. Start from cell `0109021C` and the already known
  consumer `B86950`; writer entrypoint is **unknown**.
- Establish the true writer and actual owner/publication contract before proposing
  backing or a startup call. Any subsequent Native instruction/body/data boundary
  needs its own exact lease; listing/control-flow/register-ABI recovery belongs to
  Astra/Root. That future work was not performed or authorized by this Source audit.
- Retain the existing mesh/selector owner. Do not add copied IDs, a made-up guard,
  fake predicate sink, guessed table, or duplicate mesh initializer.

Once that provider and the actual forwarding/loading/deletion inputs are established,
a separate retained-consumer packet can own `game_native_resource_application.hpp/.cpp`
and `game_hosts_vfs.hpp/.cpp` for concrete construction and call-site wiring. Its
existing semantic anchors are `B7E970/B7F430`, `71BB40`,
`B936B0/B939C0/B93A40`, with `B80720` and resource retirement as explicit dependencies.
This is a conditional ownership boundary, not a claim that those four files alone
can solve unestablished provider contracts. Any missing owner API must be resolved
in a separately bounded packet before implementation admission.

## Retained evidence and limits

The report freezes 35 current Source/configuration/tool files and five accepted or
historical context files, all matching baseline Git blobs after CRLF normalization.
It retains 22 complete command receipts with exact argv, exit status, stdout and
stderr, including three legitimate empty searches, five bounded indexed lookups and
the 4,180-row tracked Source inventory. Every stderr file is empty. Existing build
registration includes the relevant adapters and owners; it is not a fresh build.

The prior CP 14 routines, native leaf/classification fixtures and preceding owner
review remain historical context. No original ABI, full CRT execution, original
numeric-ID ordering, current application runtime, full resource traversal, gameplay
or image parity is established. Render-mesh `01090444`, skined-mesh-resource
`01090454` and matrix-resource `01090468` remain distinct from camera-resource
`01090288`, skin-model `01090344`, compact `0109042C` and excluded skined-mesh
`01090370`; none of those other families was used to fill a missing contract.
