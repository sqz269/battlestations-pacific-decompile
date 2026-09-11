# GUI native geometry and material binding integration

Addresses: 00AB2540, 00ACF8F0, 00AB3CB0, 00AD0D80, 00AA9F10,
00B18A40, 00AA6870, 00B74650, 00B72B40.

This batch integrates native geometry ownership (5ba2b90), material binding
(b57f439), and the combined Screen ownership fixture (46c2b31). The primary
review added the model-class precondition in e1f50c3. Tested source and artifact
hashes are pinned in `reports/gui_geometry_material_integration.json`.

`GuiNativeGeometryOwners` constructs actual mesh/section slots, registers their
canonical companions, and uses their actual +04 counts. Its constructor hook
fragment associates the same raw mesh with model+180 while preserving native
sentinels. Its section publication suffix requires actual stream/material/layout
services; it does not connect the separate typed geometry rebuild to raw mesh+54.

The material binder implements clip-parameter registration, release-before-retain
source-owner replacement and the known color publication sequence. ClipBox
fields, a genuinely owning token for the same widget/layout, and the actual
material projection remain required. No new registry or synthetic native count
supplies those missing contracts.

## Model/group boundary correction

AA5840's plain-page path creates a cGroup; child creation AA6640 and the
AAA710 child path create models. In native group storage, +180 is the child-array
capacity integer. AA6870/B74650 read that offset as a model geometry pointer.
An inherited color vtable slot therefore does not establish safe use on an
arbitrary Screen root. The public adapter now checks the same canonical live
model companion before any color write; its raw accessor accepts NativeModelOwner.
This is an explicit host precondition, not an invented check in the original
game. External model-backed resource roots still need their canonical association.

One focused case added to an existing ignored scene fixture checks a populated
group root is rejected before color/geometry access and its model child accepts
the color update. The child's mesh is absent in that case, so this does not
validate raw material diffuse publication. The correction and original ABI are
also recorded in `docs/GUI_MATERIAL_BINDING.md` and selected Ghidra comments.

## Verification

The integrated MSVC Win32 build and both existing CTests passed. Three ignored
fixtures were compiled against that same library:

- Native geometry: actual +04 ownership, model+180 sentinel association,
  retained mesh replacement, section reuse, terminal retirement and pool return.
- Model/group color boundary, plus the existing group/model scene cleanup checks.
- Combined Screen acquisition/teardown using actual weak, camera, directional,
  scene, group, model and camera-store owners. Two Screens share exactly one
  store/scene, with actual scene counts1 to2 to1 to0. Before each derived release,
  the manager logical pass has already unbound the dying GUI tree. Final release
  removes the store and companions, nulls the retained weak handle, and leaves
  all five tested native pools with zero live slabs. No protective node retains.

The Screen fixture creates a D3D9 NULLREF device and supplies controlled viewport
dimensions, callback boundaries and CRT globals. It performs no draw. The current
dispatch dependency requires an explicit CRT-mode reference; the preserved
historical fixture was adapted only for that binding and its local header path.
See `docs/GUI_SCREEN_OWNERSHIP_FIXTURE.md` for its other limits. No permanent test
suite was added, and original Screen machine instructions were not executed.

All nine functions already existed in the verified bsp project/program and had
zero call gaps. Nine names/comments were applied under the write lock, six prior
comment fields were preserved, and all nine exports were refreshed. The two
model-class comments were extended after the primary correction and read back.
No function definition or flow mutation was needed for this batch.

Forty-one worker-local artifacts were preserved with hashes. The earlier four
scene/bounds worker worktrees were retired only after publication a941472 and
verification of their fifty preserved artifacts. Their evidence remains in
`local/gui-scene-workers/` and `local/gui-scene-retirement.json`.

## Follow-up packets

- ClipBox constructor/copy/reader/current24 behavior and type16 integration are
  active in `orch2_gui_clip_box_r`.
- Actual material110h storage/pool/reference ownership is active in
  `orch2_native_material_owner_s`; the semantic material projection remains an
  explicit separate dependency.
- `orch2_gui_widget_retention_s` is checking actual widget/material lifetime
  against manager direct deletion and derived/base destruction. A standalone
  shared_ptr token over today's unique widget/page owners would not prove that
  contract. Initial tracing already shows the older A9E130 base-wrapper label
  needs revalidation against the actual vtable; no terminal wrapper is inferred.
- Frontend progress correction37db316 is complete in its worker tree and queued
  for separate review/integration. It is not part of this tested source.

Raw renderer stream/layout/material publication, full GUI rendering, native SEH
equivalence and gameplay remain unvalidated. Descriptive names remain hypotheses;
these are reconstructed C++ interfaces, not drop-in binary replacements.
