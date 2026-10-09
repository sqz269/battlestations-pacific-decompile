# Model descriptor consumer binding readiness

At `4abacea00`, the production VFS owner has the admitted model and model-base
descriptor cells, but **no production caller connects them to an actual model
or model-base object**. The earliest existing Source consumer pair is the
resource graph builder's live model-ID query and its built-in model-base node
factory/type predicate. Their interfaces already accept the correct borrowed
cells; the missing work is the actual provider, owner and invocation composition.
Adding another view or copying a token into an unrelated context would not close
that gap.

This packet owns only this document and its JSON report. It performs no new
Native body/data read, Ghidra query, Source edit, build, test or runtime execution.
The selected model admission remains the immutable
[Source534 primary review](CC12_MODEL_CLASS_UNLOCK_SOURCE_PRIMARY_REVIEW.md),
report SHA-256
`9fc3a11764a53ed6c100471213d97509b47f510617d6bea9c32f39c5b4c7f218`.
Its three checks and two-Present startup are historical build/runtime evidence,
not execution proof for a newly bound model consumer. Source509/511 receipts and
all older readiness documents are unchanged.

## Current ownership and dispatch

| Current Source owner | What is actually retained or called |
| --- | --- |
| `GameNativeVfsApplication::Impl` | `NativeVfsOwnerServices`, `GameNativeTypeStorage` and `LightTypeBootstrap`; the latter and all represented type initializers use the same `owner_services.types()` counter. The resource type sequence runs before VFS allocation/registration. |
| `GameVfsHost` | Owns the VFS application and `GameNativeResourceApplication`. `native_types()` returns the actual retained storage; `native_owners()` returns its real services. No extra accessor is needed for this packet's cells. |
| `GameNativeVfsRuntime` | Receives `NativeStreamTypeIdStorage&`, the actual VFS allocation, string/retained-memory services and shared singleton cells. Its raw-services view does not contain model IDs or build a model graph. |
| `GameNativeResourceApplication::Impl` | Owns the real resource-manager/parser publication cells, raw string context, parser names, factory deletion provider and deletion bindings. It exposes the manager/parser contexts; it has no load/cache, mesh-loading or resource-graph production composition. |
| `GameSingletonHost` | Owns the live `00E19B90` game-factory publication and its manager-bound context. `publish_game_resource_factory_008f840b()` separately publishes the `00F8D31C` alias. Neither cell is a loaded model or descriptor. |
| Renderer `DestructionGraph` | Already owns `ActualSceneRuntime`, `GeneratedModelLifetimeRuntime` and `NativeNodeDestructionRuntime` using the VFS raw name pool. Its `LightTypeBootstrap` borrows the same common cells and counter. These real domains should be reused. |

The resource factory and manager are useful prerequisites. Existing
`load_native_game_resource_007188a0()` captures the current factory before the
manager getter and forwards both into `load_and_cache_native_resource_00b80720()`.
That caller publishes manager+20 before cache lookup. A cache hit or a provider
callback does not prove a game-resource profile or a later virtual+8 target.
The default load/cache adapter supplies its known factories and VFS open calls,
but still requires a complete remaining provider and raw parser dispatch.

The current production mesh type bootstrap covers resource-item descriptors
`01090444`, `01090454` and `01090468`. `NativeMeshResourceTypeCalls` dispatches
only predicates `00B936B0`, `00B939C0` and `00B93A40`, using those actual arrays
before forwarding other targets. These are mesh-resource item types, separate
from model `01090034` and model-base `01090044`. Repointing those arrays to model
cells would change the meaning of the dispatch.

## Earliest real consumers and exact future bindings

The existing `build_native_resource_graph_00b891a0()` creates the selected
instance, invokes real node factories, parents the nodes and publishes
instance+0C before its item/postprocess/bone stages. Its `build_node()` captures
the node's current table, then reads the current model-own cell through
`native_graph_model_type_00b74310()`, then calls the captured table's slot+0C.
That order must survive composition. The `007137F0` wrapper already exists.

`NativeResourceGraphBuiltinDispatch::create_node()` really calls
`create_native_model_base_00b86720()` for that selected target. It adopts the
returned actual 174h node from its 178h pool slot, installs one canonical
`NativeModelBaseReference`, and keeps the existing scene/lifetime bindings.
The reference and the dispatcher's direct `00B743E0` route read the three live
`NativeModelBaseTypeTokens` cells. They do not require a new descriptor type.

The exact binding expressions, once a genuine production graph owner/caller
exists, are:

```cpp
auto& model = files.native_types().model_types().model_01090034;
auto& base = files.native_types().model_base_types();
NativeModelBaseTypeTokens base_tokens{
    base.own_id_01090044, base.node_id_01090048, base.root_id_0109004c};
```

`NativeResourceGraphContext::model_type_01090034` must reference `model.own_id`;
`NativeResourceInstanceBoneContext::model_type_01090034` must reference `model`;
and `NativeResourceGraphBuiltinEnvironment::model_base_types` must contain
`base_tokens`. Those copies contain references, not copied IDs. The graph's
matrix-mesh token can borrow the existing
`files.native_types().mesh_resource_types().matrix_01090468[0]` cell. Its other
group/postprocess token families still need their own real owners.

The distinct 188h `NativeModelOwner` path uses a retained `ModelTypeBootstrap&`
inside `NativeModelEnvironment`. A future renderer-owned bootstrap can be
constructed from `files.native_owners().types()`, the renderer's existing
`graph.types`, and `files.native_types().model_types()`. Construction only binds
references; it must not re-run static or lazy type initialization. The existing
model callback reads `006EF860` only when the actual current profile/table
selects it, and uses the real node predicate during the node phase. Unknown
profiles or targets cannot be inferred from the C++ companion's class.

Model and model-base have different own IDs and share inherited node/root IDs.
The model-base predicate is not a substitute for the model predicate. Neither
predicate filters zero IDs, reads a guard to manufacture success, or initializes
the descriptor on demand.

The renderer's legacy `scenes.object_type_tokens = {0, node, root}` is not a
short path to this binding. The corresponding
`object_accepts_scene_type_006ef860()` is currently selected by the D3D mesh
probe, not a production model owner. Replacing zero with an ID snapshot would
neither create a current descriptor reference nor establish the missing caller.

## Current caller and prerequisite boundary

Targeted searches cover all Source extensions, including renderer `.inc` files,
headers and tests. The model accessors occur only in their declarations and
definitions. The graph context/built-in dispatcher occur only in their own
files. The built-in dispatcher is the sole `NativeResourceGraphCalls` subclass;
it requires an external provider for resource/item/foreign operations. No
production `NativeResourceInstanceBoneCalls` or `NativeResourcePostprocessCalls`
implementation, `NativeMeshLoadingContext` composition, `NativeModelEnvironment`
instance or `NativeUnitHealthPartsBindings` implementation was found.

The real future caller edge is already explicit in
`initialize_native_unit_health_parts_0087bcc0()`: capture the current class+50
resource/table, call unit+190, then read the captured table's +8 entry and call
`NativeUnitHealthPartsBindings::call_part_set_08(entry, resource, selector,
detail)`. A concrete binding may call the existing `007137F0` wrapper only when
that actual captured entry selects it. It must preserve the second word's float
bit pattern and the wrapper's existing x87 staging. It must not route every
resource to that wrapper because a factory once stamped `CFD8CC`.

That returned, fully built instance then enters the real `007135C0` constructor
and is published through model+160 and unit+360. The current semantic
`GameUnitSlot` or `VehicleClassDescriptor` cannot be cast into this chain.
The admitted `00749050` constructor now supplies the actual vehicle-class base
Source body; older documents saying it is absent are historical. It still does
not supply the actual loaded class receiver, authored Mesh header or production
class registry. `native_damageable_class_lua.hpp` still explicitly leaves
`0087CA80` Source absent at this baseline.

The consumer packet therefore remains held on these named dependencies:

| Required dependency | Current boundary |
| --- | --- |
| Physical pools | Complete model pool `01090054` and model-base pool `0109008C` Source providers exist, but neither is bound/started by the production resource-pool process. Plain-node `0108FF58` and group `010902F4` production ownership is also needed by the complete built-in graph. |
| Group/model owners | Reuse renderer scene, node and lifetime domains. Actual `NativeModelEnvironment`, `NativeGroupEnvironment`, canonical companions and pool retirement still need production composition. The 188h render model, 174h model-base node and 1ACh unit-part owner are different families. |
| Remaining type families | The graph needs the real group `0109032C`; postprocessing also needs compact `0109042C`, camera-resource `01090288` and skin-model `01090344` cells. Existing animation/bone and matrix-mesh cells are reusable. A camera object descriptor is not the camera-resource descriptor. |
| Resource creation/loading | Current factory/manager/string publications are real. A retained `NativeResourceLoadCacheContext`, root parser dispatch, mesh/material/texture acquisitions and complete captured-target providers remain uncomposed. |
| Graph operations | Complete external `NativeResourceGraphCalls`, parenting/group-sphere contexts, postprocess and bone services are required. Forwarding to an invented success/no-op or generic factory is not an implementation. |
| Actual class/unit caller | Loaded raw class/name/resource identity, current virtual+8 dispatch and actual unit/part storage/lifetime are still required. The constructor and class-model publisher bodies alone do not establish them. |
| Terminal/failure ownership | Completed instance resource/root references, selected-set terminal callbacks, failed acquisition frames and all canonical companions must survive their required cleanup. A constructed instance with an unwritten root is not a completed graph. |
| Later numbering | `NativeModelNumberingServices` has no production owner/caller. Its atlas/current node+0C/actual model hierarchy requirements remain separate even though its descriptor reference can use `model`. |

## Minimal next code packet

The smallest concrete executable prerequisite identified here is
`cc12_production_model_pool_subset_source`, separate from the held graph-consumer
packet. It has a real existing startup caller and reuses complete Source bodies.
Proposed ownership is exactly:

- `include/bsp/game_native_resource_pools.hpp`: retain one `NativeModelPoolStorage`
  and its `NativeModelPool`, plus distinct correctly aligned 38h model-base pool
  backing; add one-attempt states/statuses and checked accessors.
- `src/game_native_resource_pools.cpp`: bind the model companion with
  `bind_static_native_model_pool_01090054()` and the model-base raw owner with
  `bind_static_model_base_node_pool_0109008c()`, using the same existing
  `E188B4` allocator-list domain. Call the real `00CD7F00` and `00CD7F20`
  initializers once, preserving actual atexit status and sticky thrown state.
- `src/game_main.cpp`, its existing explicit resource-pool startup block: call
  those new one-attempt methods before any model consumer can run, alongside
  the existing pool startup. This is an explicitly qualified Source subset;
  this audit supplies no new Original CRT-slot ordering evidence.
- One new packet document and report; Root owns any shared admission metadata.

The model-base backing must establish the actual allocator-element/header
lifetime and 38h extent required by the existing raw initializer. It is not a
constructed node or a padded model. No slot allocator may receive cold zeroed
pool bytes before that initializer. Binding must precede initialization; pool
and allocator-list identities remain fixed through the registered CRT callbacks.
Do not call native pool destruction from the C++ process-owner destructor.
Every live payload and companion must retire before pool exit cleanup.
No new model type cells, extra counter, CMake provider substitute or implicit
initialization belongs in this packet. The relevant real providers are already
registered in Core; their current full emitted graphs still require review in
the future code packet's build.

After that prerequisite, the smallest honest descriptor-consumer integration is
a paired **retained graph owner plus actual invocation**. Ownership must cover
renderer `Impl`/its graph composition, the real resource application/load context
and the concrete unit health-parts `call_part_set_08` binding. Its exact type
references are given above; the existing low-level consumer headers need no new
ID-view API. It is not presently a ready one-file accessor or predicate patch.
Do not implement that composition until the table's incomplete providers and
actual class/unit receiver are supplied together or in reviewed prerequisite
packets. The eventual call must reach `build_native_game_resource_graph_007137f0`
for the genuinely selected target and retain its completed result through the
existing `007135C0`/unit+360 path.

## Lifetime and evidence limits

The eventual consumer is admitted only after successful VFS type initialization.
Guard/name writes and counter increments survive exceptions; adding a consumer
must not reset guards, retry initialization, choose IDs, snapshot the descriptor
or redirect a lazy initializer to other backing. All current-cell reads retain
their order and alias/reentry behavior. The same string publication cells,
counter and common descriptors must remain alive through every consumer and
the shared singleton drain. Finish logical/reference release while renderer
node services and VFS cells still exist; then destroy metadata owners. Failed
VFS/provider frames retain the existing process-retention policy, and built-in
graph dispatch destruction with live companions terminates rather than silently
destroying their native payloads.

Current Source process IDs are values produced by the represented startup
subset. They are not proven Original absolute numbers or fixed-address mappings.
The conditional virtual targets remain conditional. This Source-only audit adds
no build, ABI, startup, Native body or gameplay credit. The report pins current
Source and the immutable Source534 evidence separately; it does not label old
whole objects as current binaries.
