# Native mesh resource type publication and classification

Addresses: 00b93000, 00b93010, 00b930d0, 00b930e0, 00b931d0, 00b931e0,
00b936b0, 00b939c0, 00b93a40, 00b869c0, 00b93bd0, 00cd8690, 00cd86f0,
00cd87b0.

Mesh resource items now have concrete type publication and classification
bindings. Fourteen complete native bodies total 690 bytes; all instruction
owners and 12 direct calls match saved Ghidra and the installed executable.
Twelve previously missing function entries are defined, annotated and saved.
The existing `native_mesh_subset_loading` source/header contain the extension;
no CMake registration changed. Names are reconstruction hypotheses except for
the literal class strings verified in the executable.

This corrects the CO follow-up: item slot8 targets B93000/B930D0/B931D0 are
own-ID getters. The classification predicates are at slotC:
B936B0/B939C0/B93A40. Slot14 targets B93010/B930E0/B931E0 return current
name-address words, not literal source names.

## Actual descriptors and publication

| Family | Guard | Descriptor | Words before name | Name word |
| --- | --- | --- | --- | --- |
| cSceneResource | 0109020C | 01090210 | own, root | 01090218 |
| cRenderMeshResource | 01090440 | 01090444 | own, scene, root | 01090450 |
| cSkinedMeshResource | 01090441 | 01090454 | own, mesh, scene, root | 01090464 |
| cMatrixIndexedMeshResource | 01090442 | 01090468 | own, mesh, scene, root | 01090478 |

The spelling `cSkinedMeshResource` is the literal string at D637B8. Other name
addresses are D631E4, D637A4 and D637CC. Name fields contain numeric original
image addresses. Binding the borrowed descriptors does not reset or initialize
their contents.

B869C0[62] and B93BD0[71] take a caller descriptor in ECX, use process-wide
guards, have no stack arguments and return with plain RET. Both publish their
guard and name before calling a parent initializer, copy inherited IDs, then
consume the SAME 6FAC20 counter's current DWORD+4, increment with unsigned
wrap, and publish the captured old value as own ID. The shared counter and
root are the existing actual `TypeIdCounterLifetime`/`LightTypeBootstrap`.
Scene calls root BEA780; mesh calls scene B869C0. Guards remain set and earlier
writes stand if a dependency throws.

CD8690[79] initializes the global mesh descriptor. CD86F0[161] and
CD87B0[161] initialize the two derived descriptors. CRT array cells
CE35F0/CE35F8/CE3600 point to those entries. No startup router is silently
replaced or presumed to call these new source methods.

Ordering differs between bodies. B93BD0 alternates each parent load and
destination store. The CRT base form captures both parent IDs before either
store. Derived entries compare the mesh guard before publishing their own
guard/name; they use that captured condition for the inlined base initializer.
They then capture all three base IDs before storing any of them. Source keeps
these distinct schedules and does not recheck the mesh guard after its native
comparison, add synchronization, saturate the counter or roll back on error.

## Getters, predicates and composition

Six six-byte getters are `MOV EAX,[global]; RET`, with no ordinary inputs.
They return current cells, including a changed name or zero ID. The three
40-byte predicates ignore ECX, take one stacked token, compare current
3/4/4 ID cells in order and stop at the first match; they return AL and RET4.
They exclude the name word, perform no lazy initialization and do not filter
zero. Upper EAX in the native predicate is unspecified.

`NativeMeshResourceTypeCalls` handles exactly the three numeric predicate
targets and forwards other targets. The existing 71BB40 classifier can thus
consume items from all three reconstructed mesh parsers using the actual
current item profile and type cells. Primary append and classification keep
raw pointer ownership: neither increments item or mesh reference counts.

## Validation

The tracked MSVC Win32 build and both existing CTests pass. The existing local
D3D9 mesh probe now additionally starts the actual cold shared type counter
and its raw lifetime manager, initializes root/scene and a caller-target mesh
descriptor, and demonstrates that the process guard can suppress later global
initialization. Explicit independent fixture preimages then exercise cold CRT
base publication, derived-first initialization, idempotence and unsigned
counter wrap across FFFFFFFE, FFFFFFFF and zero.

The fixture reserves its own 01090000..0109FFFF region in a suspended child,
before CRT allocations, and commits actual descriptor cells at their original
addresses. Six original getter bodies and three original predicate bodies are
copied unchanged from the byte-verified PE into executable fixture storage;
their absolute operands are unchanged. Source agrees with these native leaves
before publication, for the initialized IDs and excluded names, and after a
current-ID mutation. Startup initializer bodies are validated by source
execution and complete listing evidence, not an original-code oracle.

All three full mesh parsers run the existing nine fields, two complete subsets,
actual GPU buffers and generic layout readback. Their items enter an actual
71B810 game-resource object, then 71BB40 calls the recovered predicates. The
fixture explicitly supplies selector cells derived from recovered IDs: skined
items reach list64; mesh and matrix items reach list44. These are fixture
selectors, not proof of production E19B64/E19A98/E19BE4 publication or meaning.
Current-profile item deletion retires all 57 canonical geometry owners. The
new counter unregisters/deletes and its actual manager drains/frees; all pools,
caches, tree/list storage and the owned descriptor/code mappings are released.
D3D9 device/API references reach zero.

The initial oracle attempt found the desired global address unavailable after
child CRT startup. Reserving the fixture-owned region through the existing
suspended-child handle fixed placement without removing another allocation.
A later final cleanup check exposed the newly created raw lifetime manager;
the fixture now drains and frees it explicitly. Final execution exits zero.

## Evidence limits and next work

Partial-publication exceptions, aliases and concurrent mutation schedules are
listing-checked rather than newly fault-injected. Application CRT routing,
production selector-cell initialization, full root traversal, original
initializer execution and private stack/ABI/FH3/SEH behavior remain unproven.
The hot effect fixture still uses the documented explicit zero-pass preimage
and actual descriptor named `generic`; it does not supply successful cold
shader/compiler/cache replacements. There is no gameplay or image-parity claim.

Next, trace production E19B64/E19A98/E19BE4 producers and the application's
CRT initializer dispatch for CD8690/CD86F0/CD87B0. Remaining mesh item methods
include B94020 and per-profile accessor/copy paths; their earlier classification
as slotC methods was incorrect. Publication is on the agent branch only;
moving main remains unreviewed.

Reports: [source and byte evidence](../reports/native_mesh_resource_classification_cp.json),
[function definitions](../reports/native_mesh_resource_classification_flow_cp.json),
[annotations](../reports/native_mesh_resource_classification_annotations_cp.json),
[integration receipts](../reports/native_mesh_resource_classification_integration_cp.json).
Frozen local proof: `local/native_mesh_resource_classification_cp/registered/`.
