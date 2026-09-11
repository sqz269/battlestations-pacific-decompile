# Canonical native node hierarchy

This is a host adapter integration, not a reconstruction of additional native
bodies. `NativeNodeStorage` now stores actual 32-bit node addresses at parent+30,
first child+34, next sibling+3C and previous sibling+40. Count+38, flags and
matrices stay in the same physical prefix. `NativeCameraOwner` already obtains
its transform through `NativeNodeBinding`, so its prefix uses this representation
without another graph, pointer swizzling or casting a companion to raw storage.

`CameraTransformLink` addresses either one native DWORD or one diagnostic
companion-pointer field. Bound reads reload the DWORD and resolve its current
key through `SceneAttachmentRuntime::bindings_`; writes publish the target's
actual storage address. The immutable backing identity is checked against the
existing attachment `pointer_key` at bind time. No scene or light registry
membership is needed, and no second key map or hierarchy cache is introduced.
Standalone diagnostic transforms retain their existing pointer semantics.

Preconstruction creates link views and registers the stable companion without
reading or writing native hierarchy fields. A raw binding must have its actual
storage key and belong to one live runtime. Every traversed raw target must
have a stable binding in that runtime. Diagnostic/cross-runtime targets are
rejected by host link assignment before its single raw-word store. Existing
native algorithms keep their valid-input contract; no checks are added to the
raw original bodies and no whole-operation rollback is inferred.

Copy and move construction create independent diagnostic storage. Assignment
keeps the destination's backing and resolver ownership. The host
`CameraTransform` and `CameraState` copy/move APIs can now throw: all four source
links are resolved and all destination domains checked before assignment
stores. A later invalid link therefore cannot publish an earlier field.
Individual link value assignment also preserves its binding, and link proxy
copy construction is disabled. Native semantic `noexcept` unlink/release
operations continue to require valid live hierarchy inputs.

`forget_destroyed_binding` first finds the exact registered companion identity,
clears its resolver association and retires it from the same binding vector.
It does not read backing fields, so native storage may already be destroyed or
protected. The external companion must still be alive. A rejected duplicate or
an old companion at a reused address cannot retire the replacement binding.
`resolve_key` rejects retired keys; a reused address resolves only its current
live binding. Link `get()` itself still requires live readable backing and must
not be used to dereference a retired view. The runtime must outlive its bound
companions; this integration adds no ownership or automatic detachment policy.

The consumer audit covered native construction/parenting/destruction, camera
invalidation/world/view paths, scene/light child walks, generated-model release
and scene removal, render root unlink/release, GUI visibility traversal and
existing diagnostic tests/probes. Pointer expressions continue through the
proxy's current-value conversion. Only `auto*` initializers require explicit
`.get()`, preserving child/sibling reloads after callbacks. No bulk memory copy
or pointer-reference consumer of these four public links remains. Native
B6E010 publishes child+30 before callbacks, then reloads parent+34 before
prepending; B6D940 clears parent before updating current siblings. The existing
host algorithms retain those ordering points over the canonical words.

The focused verification uses actual node construction, preconstruction
bindings and full library scene/parenting/destruction providers. One sequence
checks reentrant prepend, sibling removal during traversal, nonnull reparent,
full raw frustum/inverse-VP getters over initialized camera-shaped storage,
diagnostic copying, invalid-link assignment without partial publication,
protected dead backing retirement and address reuse. Camera-tail values are
explicit fixture inputs; this is not a replay of the full native camera owner
constructor or destructor. Root-list heads and unrelated owner fields keep
their existing host representation. No complete object ABI, gameplay,
concurrent mutation or unmasked floating exception claim is made.

The final strict Win32 build, both existing CTests and all eight native seed
checks passed. The ignored sequence passed 50 assertions and retained six
literal 0x45C-byte storage snapshots. All 403 mapped COFF sections, including
280 library sections and their relocations, match 24 exact archive members;
both 41,731-byte runtime code postimages match the linked executable. This is
compiled/runtime host integration evidence, not original-body instruction
identity for the new adapters.

The final seal is
`J:/PROG/battlestations-pacific-decompile-native-node-raw-hierarchy/local/native_node_raw_hierarchy_final/sealed.json`
(SHA256 `f1ba7971e7103d4bbb157e4e5adfca9ff4f9aaf991bb128dd9301aa2510cbbe5`).
It pins 143 artifacts, 92 provider source/header files, and the actual full
worker library SHA256
`0b19e739485c49e4fc6596fb7c24bf072e52d702b6c7f460f864d8b987607a6c`.


## Primary main-library integration

The canonical storage and all audited consumers are integrated in main.
The primary verified 143 worker artifacts and 92 current source/header files:
84 literal matches and eight with CRLF/LF differences only. Strict Win32
compilation, both existing CTests and eight seeds passed.

The unchanged single sequence linked the actual main archive and passed all
50 assertions, with six literal raw snapshots totaling 6,696 bytes. All 24
exact archive members, 403 complete COFF sections (280 library) and two whole
41,731-byte runtime text postimages passed. Differences between worktrees are
compiler private namespace/lambda names, debug metadata and the private RTTI
scope names in two provider objects; the current linked sections were verified
exactly. This closes the four hierarchy-word migration, including nonnull
parents, reentry and exact-identity retirement. It introduces zero recovered
native bodies. Full camera owner constructor/destructor and other owner ABI
boundaries stated above remain open.

The primary library SHA256 is `848d569d9eaae6c435e84b62ab3712943a6432989924fe7153cd300122b4e306`. The read-only bundle is
`local/node_raw_hierarchy_primary/`, seal `c1181d47889f5c6656230ab392f467ea9b5db496dd6881c83fb1bb54e01c0c16`.
Evidence is recorded in `reports/native_node_raw_hierarchy_audit.json`.
