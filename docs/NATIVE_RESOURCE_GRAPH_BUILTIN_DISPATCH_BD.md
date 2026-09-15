# Resource graph builtin dispatch and ownership

Addresses: `00B891A0`, `00B6F8D0` (existing bodies, no new native reconstruction).

The existing full `B891A0` graph builder can now use `NativeResourceGraphBuiltinDispatch` to create plain nodes (`B866C0`), model-base nodes (`B86720`) and groups (`B86780`) through their complete canonical factories. The provider adopts each physical owner into the existing scene and generated-model lifetime registries. Logical root release reaches the original node hierarchy and terminal pool cleanup; the provider's companion list contains no hierarchy copy.

The environment borrows the original distinct plain/model pools, bound static group pool, initialized type descriptors, current factory/node tables, and the node runtime's raw name publication, gate and manager cells. Its name facade uses those same cells. Current builtin matrix/type entries dispatch to the existing complete implementations; resource, item and foreign-class entries require an external provider. These are explicit C++ interfaces, not native ABI replacements.

## Admission and destruction

All companion/list allocations and scene, lifetime and group-attachment capacity reservations precede the native factory. `GeneratedModelLifetimeRuntime` now supports separately counted attachment admissions; ordinary bindings preserve outstanding reservations during reentrant use on the serialized owning thread. Group adoption consumes scene/attachment credits without reconstructing native storage. Immediately before each existing reference constructor, the provider returns its node-lifetime credit; no callback or allocation intervenes before the constructor binds the reference.

Native factory exceptions retain the factories' existing cleanup, erase the pending companion record and cancel reservations. Metadata allocation failure occurs before native construction. Successful factories must return fresh owners with valid canonical profiles. An unexpected post-factory registration exception terminates as an adapter-contract violation; this does not invent native rollback. The provider and borrowed environment must outlive every created node, and the provider must have no live records when destroyed.

Plain-node reference destruction now supports the same raw name context used at construction. The existing complete `B6F8D0` sequence calls `B6F440`, returns the captured slot to the plain pool when flag bit zero is set, and retires canonical bindings after successful cleanup. Existing semantic-name callers remain supported. Native `B6F8D0` takes ECX owner and one flags stack word, returning the original pointer; the source overload supplies explicit runtime/name/pool access. Native `B891A0` takes ECX resource and two stack words; its source interface supplies the established graph context.

## Verification and limits

The strict MSVC Win32 build and both existing CTests pass. One focused source fixture runs two four-node graphs with group, default model-base, item-selected plain-node and fully delegated factory creation. Both explicit and automatic group-bound selection paths run. Item attachment invokes complete `B89E90` publication, preserves the creation word and uses the same raw synthesized-name domain. A nested model-base parent verifies canonical identity lookup.

Each graph is destroyed through complete `718D00` instance destruction and root logical release. All four companion identities disappear and the retained resource returns to reference count one. The same fixture checks metadata allocation failure, a native-factory name-allocation exception, and cross-pool reentrant creation while outer reservations remain active. No new permanent test cases or worker dispatches were added.

Two existing reference bodies total 1,312 bytes; disk/live Ghidra byte comparisons and 37 direct-call rows pass. Existing Ghidra names receive appended evidence with prior comments retained, project saved and exports refreshed. The report records exact source and fixture output hashes.

This is source composition evidence. This batch does not execute the original graph caller or deleting destructor. Earlier BA/BB native comparisons remain separate evidence. Publication-only fixture items leave animation and mesh postprocessor ranges empty; populated postprocessors were tested separately. Controlled-listener and resource-zero branches are not reached. Canonical scene/group companion materialization and attached-array projection limits remain, including the native parent publish-before-equality behavior documented in BC. Actual resource/class producers, executable graph admission, original FH3 behavior and gameplay validation remain open.

Evidence: `reports/native_resource_graph_builtin_dispatch_bd.json` and ignored `local/resource_graph_builtin_dispatch_bd/`.
