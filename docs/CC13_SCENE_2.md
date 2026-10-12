# Segments 88/91: second scene-graph pass (cc13-scene2)

Segment 88 is `00B6D3C0..00B80ED0` (c3dNode, cCamera, the mesh object and model base,
c3dNodeAnimator, cOptimized3dNodeAnimator, the light family, resource containers); segment
91 is `00B8A2B0..00B999A0` (resource items, the flare, cGroup / cGroupParamsResource, skin
and bone models, the quadtree node, cRenderMeshResource, the CPU ocean spectrum helpers and
the grid index builder). Names are descriptive hypotheses, not recovered symbols.

Status words: *reconstructed* (C++ in `src/scene_graph2_cc13.cpp`, declarations in
`include/bsp/scene_graph2_cc13.hpp`, namespace `bsp::cc13_scene2`), *build-tested*
(`scripts/build.ps1` passes). Nothing here is fixture-tested, ABI-compatible or
game-validated. These are new C++ interfaces over host data: every image routine a body
calls is reached through an abstract service interface whose virtuals are named by address,
so nothing is stubbed. x87 arithmetic is evaluated in double and rounded to float wherever
the native code stores a float; non-commutative x87 forms were read from the bytes or from
capstone's Intel-named listing (DC-form FMUL, FSUBRP, FDIVRP), never from Ghidra's listing text.

## Triage

At `origin/main` (index of 2026-10-12) the two ranges held 340 functions with a reviewed
ledger name, no reconstruction record and no tag. A normalised-body grouping (callee names,
addresses and immediates masked) reduced them to 201 distinct bodies; 41 groups are
identical instantiations (pointer arrays, trees, iterator wrappers, type leaves). A rel32 and
absolute-dword scan of the installed executable found 147 of the 340 unreferenced (mostly
container instantiations, the type-info thunks, the ring writer and the 32-point FFTs).

Result: **334 of 340** have a reconstruction record (`reconstructed_build_tested`; ten of
them are backfilled records for cc9's ocean reconstructions, see below). Six bodies remain:
`00B708E0` (hi-res screenshot tiles), `00B8DD60` (flare geometry builder), `00B98BE0`
(grid patch builder) and its three strip writers `00B978B0`, `00B97C80`, `00B980A0`.

Shared-shape records (scope `shared_shape_model` or `table_model`) point at one generic
function or table entry: the native bodies are byte-identical after relocation and differ
only in the element type, the global or the callee named in the table.

## Corrections to earlier names

| Address | Old | New | Evidence |
| --- | --- | --- | --- |
| vtable +14h leaves (10) | `*_GetTypeDescriptor` | `*_GetTypeName` | descriptors are {own id, ancestor ids, name word}: light 0109018C..0109018C+0Ch "cLight" (00B7BFD0 writes `param_1[3]`), directional +10h, node 0108FF98 "c3dNode" |
| 00B72080 | `BSP_Node_UpdateWorldIfRoot_Provisional` | `BSP_Node_RefreshLeafWorlds` | the test 00B6D980 is `MOV EAX,[ECX+38h]` (child count) |
| 00B8B940 | `BSP_RoundFloatToField20` | `BSP_Object_TruncateFloatToField20` | `OR EAX,0C00h; FLDCW; FISTP qword` |
| 00B91650 | `BSP_SkinModel_ConstructWithBindings` | `BSP_SkinModel_Destruct` | vtable store, resize 0, free, 00B750C0; called by scalar delete 00B916C0 |
| 00B91280 | `BSP_BoneModel_DestroyBones` | `BSP_BoneModel_Destruct` | same shape; called by 00B915A0 |
| 00B70200 | `..._SetPositionAndTarget_Provisional` | `BSP_Camera_SetEyeAndTarget` | arg 1 goes to slot +30h, arg 2 to +1A0h; +1ACh = normalize(arg2 - arg1) |
| 00B70760 | `..._ComputeViewMatrix_Provisional` | `BSP_Camera_ApplyObliqueNearClip` | see Camera |
| 00B94E90 / 00B950B0 | `BSP_OceanFft_BitReverseRows/Columns` | `BSP_OceanFft32_Rows/Columns` | full 32-point FFTs, not only the bit reversal |

Ledger and Ghidra were updated together (`tools/ghidra_annotate.py --apply`; prior values in
`local/ghidra-annotations-20261012T233548Z.json` of the worker tree).

## Class layouts used

- **c3dNode**: +08h..+14h vector, +30h parent, +34h first child, +38h child count, +3Ch next
  sibling, +40h previous sibling, +48h render mask (bit 30 = quadtree-tagged), +50h word,
  +54h NativeString name {length, chars}, +5Ch flags (2 world valid, 8 inverse valid),
  +60h inverse world, +A0h back link, +ACh visibility factor, +F0h world rows (+120h
  translation), +130h animator, +174h retained reference.
- **cCamera** (vtable 00D62CF0): +17Ch byte, +180h..+198h words (+190h clear colour, +18Ch
  depth, +194h), +1A0h target, +1ACh direction, +1C4h..+1D8h projection parameters (fov
  first, +1C8h aspect), +1DCh, +1E0h projection, +2A0h projection copy, +2E0h oblique clip
  plane, +2F0h flags (bit 4 frustum valid), +2F4h plane set, +438h target reference,
  +440h..+454h.
- **Scene type descriptors**: own id, the parent's id list, class-name word. Ids come from the
  counter singleton [0109DB7C]+04h. Table `kSceneTypeSlots` lists every leaf (descriptor,
  id count, kind). The four IsKindOf lists in segment 91 belong to vtable 00D63590 (bone
  model, provisional), skin model (00B8F920 returns [01090344]), quadtree node (00B916F0
  returns [010903E0]) and the compact track item ([0109042C]).
- **Flare** (vtable 00D63420, 1E0h payload in pool 010902A8): +ACh scale, +184h colour-2
  scale, +18Ch element pointer array, +198h anchor, +1A4h NDC, +1ACh clip position, +1BCh
  camera, +1C0h duration, +1C4h dt accumulator, +1C8h limit flag, +1CCh reference,
  +1D0h/+1D4h occlusion queries, +1DCh facing. Element and vertex layouts: header.
- **Quadtree node** (vtable 00D63650): +174h leaf byte, +178h level, +17Ch/+188h box.
- **Mit pools**: payload 1E0h (flare, vtable 00D63410), 194h (pools 010903F0 quadtree node
  and 01090358 skin model, vtable 00D6355C), 190h (pool 01090390 bone model, vtable
  00D63560); base vtable 00D7A0C0. The slot-order behaviour of allocate/free/trim was
  already modelled by `MitBlockPool` (`src/post_effects_cc13.cpp`); this pass verified it
  against 00B90800/00B90760 and records those addresses there.

No `{2,&field}` / `{0,"name"}` property tables reference these classes in the two ranges, so
no original field names were recovered; offsets above are named by use.

## Findings worth knowing

- **Oblique near plane** (00B70760): the decompile reads the clip plane untransformed; the
  listing copies it to the stack and transforms it by the view matrix (00B65C30 with +60h)
  first. q = (sgn(c.x)/P0, sgn(c.y)/P5, 1, (1 - P10)/P14), k = 1/f(((qx*cx + qy*cy) + cz) +
  qw*cw) and the third projection column becomes k*c.
- **Sun flare billboard** (00B8BDD0): the flare node's world translation is a direction; the
  disc is built at camera + dir*4500 with radius 300. The up vector switches to (0,0,-1)
  when |dir.y| < 0.01, i.e. for a nearly HORIZONTAL direction; a vertical direction keeps
  (0,1,0) and the cross product degenerates. Reproduced as written.
- **Flare update** (00B8D150): visibility = f((visible/total * fade) * 0.7) is written to
  +ACh for the quad writers and then overwritten with 1.0 at the end; element +20h receives
  trunc(max(ratio - 0.5, 0)) through a reused stack slot.
- **Skin model Clone** (00B8F960) builds a plain c3dNode with the same name and copies no
  state; the bone model Clone (00B912E0) looks its bones up by name under the new parent.
- **cGroup clone** (00B8F6F0) moves children of the 00D63590 class to the end of the sibling
  list in reverse order; with no child left the native code reads [0 + 3Ch].
- **Mesh element reader** (00B934D0) computes the min start / max end of its ranges and
  discards both.
- **Quadtree build** (00B91D70): split along the larger x/z extent at the mean world
  position (sums rounded to float per child), depth limit 6, fewer than 5 children make a
  leaf, the root needs 10 tagged children.

## Records not created

| Address | Reason |
| --- | --- |
| 00B708E0 | Decompile is stack-corrupted; needs a full listing read (tile loop, projection offsets, surface copies). |
| 00B8DD60 | About 600 instructions (vertex/index buffers, atlas lookups for both element names, two materials, per-type draw sections). |
| 00B98BE0, 00B978B0, 00B97C80, 00B980A0 | Integer index generation, roughly 1,000 decompile lines; `grid_builder_build_all_00b99790` calls BuildPatch as an image routine. |

The ten cc9 ocean bodies (00B94CA0, 00B95750, 00B96B90, 00B96C10, 00B96C50, 00B97100,
00B952D0, 00B95AA0, 00B95D30 in `src/ocean_spectrum.cpp`, 00B960C0 in
`src/ocean_wave_field.cpp`) were reconstructed by cc9 without records; this pass added
`reconstructed_ledger_backfilled_from_source` records and did not edit those files.

## Open questions

- The class names behind lists 0108FFEC (mesh object), 010900C8 (channel group) and
  010903C8 (vtable 00D63590) were not recovered from strings.
- Whether the flare up-vector test is a shipped bug or intended.
- Node +50h, camera +194h and the +17Ch byte still have no observed reader in these ranges.

## Handoff (2026-10-12)

Next steps, in order of value: the grid patch builder and strip writers (pure integer code;
transcribe from the decompile and check the reload points marked by Ghidra's `uVarN =
local_48` assignments in the listing), the flare geometry builder 00B8DD60 (uses the element
layout already in the header), then 00B708E0 from the listing. The x87 tracer used for
this pass was `local/sc2_x87.py` in the cc13-scene2 worker tree, not committed (capstone over
disk bytes, symbolic stack, per-store expressions); it is linear and does not follow branches, so trace basic
blocks separately.
