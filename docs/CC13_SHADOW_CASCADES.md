# Directional shadow view fitting (cc13-shadow1)

Addresses: 00A8AEB0, 00A8B930, 00A8BD20, 00A8ECB0, 00AD7A30, 00AD7360, 00AD72A0, 00AE4CF0, 00AE47E0, 00AE4040, 00AE37F0, 00AE1670, 00AE15E0, 00AE0A50 (reconstructed); 00B1F4D0, 00B1F5E0 (named)

Continues the shadow section of docs/CC13_SOUND_STREAMING.md (cc13-audio1), which reconstructed
the cascade submit 00A8EA20, the two single-view paths 00A8E860/00A8E180, the point projection
00A8AC90 and the line intersection 00A8B860. This packet reconstructs the four x87 fitting bodies
those paths call or that sit beside them. Code: `src/shadow_cascades_cc13.cpp`,
`include/bsp/shadow_cascades_cc13.hpp` (namespace `bsp::cc13_shadow`). Names are hypotheses, not
recovered symbols.

Overlap check: cc12's docs mention 00A8BD20 only as the cascade-setup boundary
(docs/NATIVE_CAMERA_OWNER_NEXT.md, docs/NATIVE_CAMERA_COMMAND_NEXT.md) and treat 00AD7A30 as the
incomplete parent of their shadow-entry leaves (docs/NATIVE_SHADOW_ENTRY_ITERATION.md,
docs/NATIVE_SHADOW_UPDATE_LEAVES.md). No cc12 lease covered these addresses. The second batch reconstructs 00AD7A30 and its children
00AD7360 and 00AD72A0. It calls cc12's complete 00AD7440, 00AE1C20 and setter leaves through
host methods, so their native bodies stay theirs.

## Status

| Address | Name (ledger) | C++ | Status |
|---|---|---|---|
| 00A8BD20 | BSP_DirectionalShadow_FitCascade | `fit_shadow_cascade_00a8bd20` | reconstructed, build-tested |
| 00A8AEB0 | BSP_DirectionalShadow_FitSingleView_Provisional | `fit_shadow_box_view_00a8aeb0` | reconstructed, build-tested |
| 00A8ECB0 | BSP_DirectionalShadow_FitAndSubmitView_Unreferenced | `fit_and_submit_shadow_slice_00a8ecb0` | reconstructed, build-tested |
| 00A8B930 | BSP_ShadowMath_BuildTrapezoidHomography_Unreferenced | `build_trapezoid_homography_00a8b930` | reconstructed, build-tested |
| 00AD7A30 | BSP_FoliageSystem_RenderForCamera_Provisional | `render_foliage_for_camera_00ad7a30` | reconstructed, build-tested |
| 00AD7360 | BSP_FoliageSystem_RenderGroups_Provisional | `render_foliage_groups_00ad7360` | reconstructed, build-tested |
| 00AD72A0 | BSP_FoliageSystem_DebugDraw_Provisional | `debug_draw_foliage_00ad72a0` | reconstructed, build-tested |
| 00AE1670 | BSP_FoliageEntry_DebugDrawQuadtree_Provisional | `debug_draw_entry_group_00ae1670` | reconstructed, build-tested |
| 00AE15E0 | BSP_FoliageEntryNode_DebugDraw | `debug_draw_entry_node_00ae15e0` | reconstructed, build-tested |
| 00AE0A50 | BSP_FoliageEntryNode_DrawBox | `draw_node_box_00ae0a50` | reconstructed, build-tested |
| 00AE4CF0 | BSP_BushGroup_Render_Provisional | `render_bush_group_00ae4cf0` | reconstructed, build-tested |
| 00AE47E0 | BSP_FoliageCell_CullAndEmit | `cull_and_emit_foliage_cell_00ae47e0` | reconstructed, build-tested |
| 00AE4040 | BSP_FoliageGroup_DebugDrawQuadtree | `debug_draw_bush_group_00ae4040` | reconstructed, build-tested |
| 00AE37F0 | BSP_FoliageQuadNode_DebugDraw | `debug_draw_quad_node_00ae37f0` | reconstructed, build-tested |
| 00B1F4D0 | BSP_RenderCommandQueue_SubmitNamedScene | none | named |
| 00B1F5E0 | BSP_RenderCommandQueue_SubmitDrawable_Provisional | none | named |

All four reconstructions are complete bodies with no partial ranges. They are build-tested only:
the Win32 build and the two existing CTests pass. Nothing is fixture-tested, ABI-compatible or
game-validated, and nothing is wired into a host yet. audio1's `ShadowCascadeSubmitHost::fit_cascade`
and `ShadowSingleViewHost::fit_single_view` are the intended binding points.

## Method

Every arithmetic chain was taken from `tools/x87_sym.py`, which decodes the FPU operations from the
instruction bytes, and checked against the `disasm-raw` listing, with frame offsets tracked by hand
across pushes and callee pops. Unspilled x87 intermediates are reproduced in `double`, on the
assumption that the image runs at 53-bit precision. Each FSTP to a float slot becomes a float cast.
That assumption is wrong for the main thread, which runs at 24-bit precision control
(docs/CC13_FPU_PRECISION.md); the cascade fit is reached from `00A8EA20` <- `00A8F3B0` <-
`004CA440 BSP_Game_Render` / `004CCBD0` / `00503510`, all on the main thread. cc13-fix14 converted
`00A8BD20` to per-operation float rounding in the image's order (`pc24`: the depth remap, cos
gamma, `1 - cos^2`, `f - n` and the shift, the shifted perspective `[10]`/`[14]`, the camera-space
light direction dot products, the shared square-length sum, and the far/near depth scale); about
1 ulp in stored matrix entries. cc13-fix15 gave the other three bodies the same pass:
`00A8AEB0` is reached from `00B3C960` <- `00B156C0` <- scene render command `00B17240` (its executing thread was not traced; assumed
the device-owning main thread) and from `0059D7B0` <- `004CA440 BSP_Game_Render`,
so main thread; only its depth remap had unspilled chains (`00A8B55E`, `00A8B568`). `00A8ECB0`
and `00A8B930` have no caller (rel32 scan of the PE, absolute-dword scan and Ghidra xrefs all
empty), so no thread can be proved; they are modelled at 24 bits like the owner's live fits.
`00A8ECB0` had one unspilled chain (`q = f/(f-n)`, `00A8EF08`); `00A8B930` is now fully
per-operation (`add24`/`sub24`/`mul24`/`div24`, order from `tools/x87_sym.py 00A8B930`).
Constants were read
from the PE through `ghidra bytes`. Two tool limits needed hand work:

- x87_sym drops `+ 0.0` terms. The inlined translate loop in BD20 at 00A8C636 adds a stack zero
  (`FADD ST(7)` / `FADD ST(5)`), which changes -0 to +0. The reconstruction keeps those terms.
- Loops run only one iteration in the tool. Loop indexing, bound updates and frame overlaps came from
  the listing.

The fitters call these reconstructions directly: `multiply_camera_matrices_00413920`,
`transform_point_004142e0`, `transform_native_vector4_00b62d10`,
`native_projection_tangent_00412e20` and audio1's `intersect_lines_2d_00a8b860` /
`project_shadow_point_00a8ac90`. Everything that needs a camera object, the CRT or the render
queue goes through `ShadowFitHost`. 00B6FC30 (FLD aspect +1C8h, FMUL fov +1C4h, FSTP32) is
written out inline. `reports/cc13_shadow1_calls.json` lists all 129 modelled call sites (all batches), and
`tools/verify_report_calls.py` passes on all of them.

## Owner layout used

Owner = the directional shadow object (ECX of 00A8EA20). +08h float (copied to cascade camera
+178h), +0Ch light (vslot 0Ch query, direction +1E0h), +20h + 4c cascade cameras, +30h render
camera, +34h scene for commands, +38h..+40h light direction, +44h + 40h*c cascade matrices,
+144h + 40h*c shader matrices, +244h + 40h*c bias matrices (read only), +390h..+39Ch split
table, +3A0h + c byte, +3ACh + 40h*c depth-remapped light views, +4ACh + 10h*c depth columns,
+4ECh n_opt factor (1.5, set by 00A8EA20), +4F0h + 4c targets. Camera: +5Ch flags (bit 1 = world
valid), +F0h world matrix, +110h..+118h world row 2 (forward), +120h..+128h position,
+1C4h fov, +1C8h aspect.

## 00A8B930: trapezoid homography (unreferenced)

ABI: ECX out float[9] (row-major 3x3), EDX p (float2, an input register), stack a, b, c (float2
pointers), RET 0Ch. There is no rel32 or absolute reference. The body matches the
trapezoidal-shadow-map N_T construction:

1. u = (b - c) * 0.5 gives rows (u.x, u.y | ...) and (u.y, -u.x | ...).
2. Translate to i = line(p, c) x line(a, b) through 00A8B860. p stays in EDX across the call;
   00A8B860 does not write EDX.
3. Shear so that b and c are centred.
4. Scale rows 1 and 2 so that b maps to 1.
5. Projective step: row 3 = row 2, then row 2 += (0, 0, 1).
6. Translate so that b and p are centred in y/w (factor 00CEC9E0 = -0.5).
7. Scale row 2 so that p lands on y/w = -1.

The reconstruction keeps the dead stores to out[2] (u.x^2 + u.y^2) and out[5] (`p - p` from
`FSUBR ST0,ST0`, which gives NaN for an infinite product). It also keeps every reread of p and b
after earlier stores, which matters if `out` aliases an input. -u.x comes from SUBSS with
00D7A208 = -0.0f, an exact negation. Every multiply, add and divide rounds to float in the
image's association order (24-bit PC assumed; the function is unreferenced).

## 00A8AEB0: box-bounded single view

ABI: __thiscall(owner, unused, const float3* box_min, const float3* box_max), RET 0Ch. The two
callers pass their camera first, and this body never reads that slot (`[esp+16Ch]`). The render
camera comes from +30h, which the callers have just retained.

1. The split table becomes 100/200/300/400 (00CE3D08, 00CE386C, 00CE3AE8, 00CFD710). +44h and
   +144h are reset to identity through 004134F0.
2. Light direction: when light vslot 0Ch(00B7AAE0 word) passes, normalize light +1E0h. Its y is
   then pushed away from the horizon: y >= 0 gives max(y, t), otherwise min(y, -t), with
   t = tan(0.436332f) (00D5B564, 3EDF66F4h) and the native NaN choices. The result is
   renormalized. When the query fails, the direction is (0, 1, 0).
3. View = look-at(eye = camera position + 10000 * dir (double 00CE4BD8), target = camera position,
   up (0, 1, 0)). It is set on shadow camera +20h.
4. The eight corners of [box_min, box_max] go into light space through 004142E0. Their bounds give
   projection = ortho(bounds) * diag(-1, 1, 1, 1) (00D7A260).
5. depth = identity with [10] = 1/(zmax - zmin) and [14] = zmin/(zmin - zmax), each difference
   rounded to float before the divide (main thread, 24-bit PC). light view =
   view * depth goes to +3ACh. Its third column ([2], [6], [10], [14]) goes to +4ACh..+4B8h and
   then over +14Ch/+15Ch/+16Ch/+17Ch. The depth-scale pointer (00B6FEC0) is +4ACh.
6. +44h = camera view-projection (00B70490), and +144h = +44h * +244h.

## 00A8ECB0: older fit-and-submit (unreferenced)

ABI: __thiscall(owner, float near, float far, int cascade), SEH frame (only for the name string),
RET 0Ch. There is no reference. It builds the slice corners from tx = tan(fov*aspect/2) and
ty = tan(fov/2) and a unit-scale perspective P ([10] = f/(f-n) with f-n rounded to float first
(24-bit PC assumed), [11] = 1, [14] = -n*[10],
[15] = 0). It projects the corners through view*P with 00A8AC90, and then overwrites those results
(the first product's destination is the corner array). H is a look-at from the ground point
(x, 0, z) under the camera along the normalized horizontal forward (world +110h, 0, +118h). The
fit uses TD = look-at((0,0,0) -> (0,-1,0), up (0,0,1)) and O = ortho(-1, 1, 0, 1, -100, 100).
Outputs:

- cascade matrix = H*P*TD*O, shader = that * bias;
- cascade camera view H and projection P*TD*O;
- byte +3A0h + c = 0 (EBX is 0 after the corner loop);
- an SHBU command through 00B1F4D0 for target +4F0h + 4c, with metadata {0, 62h, 1Eh}.

The world matrix is refreshed up to three times (two consecutive flag tests, then a third test
after the length call). The reconstruction keeps all three.

## 00A8BD20: cascade fit (LiSPSM-style)

ABI: __thiscall(owner, float near, float far, int cascade), RET 0Ch, no SEH. Caller 00A8EA20, four
times. Its blocks, with their listing ranges:

1. 00A8BD2B..00A8BF4C: slice corners C (as 00A8ECB0), W = C through the camera world +F0h (REP
   MOVSD copy).
2. 00A8BF4E..00A8C1D9: far and near quad centres (float sums ((0+1)+2)+3, then *0.25 from double
   00D7A348). Middle = (near + far) * 0.5. A light look-at from middle + 10000*dir to the middle,
   with up (0,0,1) when |dir.y| > |dir.x| and (0,1,0) otherwise.
3. 00A8C1DE..00A8C46D: the [zmin, zmax] of W in that view (start values +/-1e10, 00CE4970 and
   00CE4ADC). light view * depth remap goes to +3ACh + 40h*c and its third column to
   +4ACh + 10h*c, which becomes cascade camera +20h+4c's depth-scale pointer.
4. 00A8C472..00A8C58B: cosGamma = dot(-dir, forward) (float), sinGamma = sqrt(1 - cos^2) (00BF7030),
   n_opt = (+4ECh)/sinGamma. When n_opt >= far/(far-near) (FCOMI/JB: NaN keeps the slice),
   shift = ((far-near)*n_opt - far) + 1e-4f (double 00D7A268). The shifted bounds are
   near' = shift + near and far' = far + shift.
5. 00A8C58D..00A8C7AD: T = z translation by shift. The shifted corners come from an inlined affine
   loop with zero terms. P' has [0] = 2n'/width and [5] = 2n'/height (width and height from the
   shifted near quad), [10] = f'/(f'-n'), [11] = 1 and [14] = n'f'/(n'-f'). TP = T*P'.
6. 00A8C7B2..00A8C9ED: the light direction goes through the render camera view (00B6FCB0). It is
   normalized (each square spilled, 00BF7030), then taken through P' with w = 0 (00B62D10). The
   byte +3A0h + c records w < 0. The light point is xyz * (1/w).
7. When |w| <= 0.001f (00D7A23C), 00A8C9F3..00A8CF40: LA = look-at(point -> (0, 0, 0.5)), with up
   chosen as in block 2. The corners go through TP*LA with a w divide. X = ortho(bounds, z padded by
   30.0 (00CE7630)).
8. Otherwise (NaN included), 00A8CFEE..00A8DA00: the corners go through TP with a w divide, and
   their box centre is computed. Up = the normalized shifted slice axis (far centre - near centre).
   LA = look-at(point -> centre, up). The corners then go through LA with a w divide. Bounds are
   collected along with max |x/z| and max |y/z|. X = perspective with [0] = 1/max|x/z|,
   [5] = 1/max|y/z|, [10] = A/(A-B), [11] = 1, [14] = -B*[10]. When w < 0: B = -max(zmin, 0.001)
   and A = zmin. Otherwise: B = max(0.6f*zmin, 0.001) (double 00CEFF98) and A = zmax.
9. 00A8DA09..00A8DD15: mirror = diag(-1,1,1,1) unless w < 0. Cascade matrix =
   V*T*P'*LA*(X*mirror), shader = that * bias, and the depth column is copied over +14Ch... The
   cascade camera view is the render camera view (00B6FCB0 again), its projection is
   T*P'*LA*(X*mirror), and cascade camera +178h = owner +08h.

Native details kept:

- All bound updates use `if (!(v > lo)) lo = v` / `if (!(hi > v)) hi = v`, so a NaN replaces the
  bound.
- The magnitude tests negate on zero and NaN (`-0.0f - v`).
- Path 7 divides by a w that can be tiny, exactly as the native does.
- The FLD1 that stays on the x87 stack through the third loop (00A8D665, popped at 00A8D600)
  has no arithmetic effect.

## Callee notes

- 00AD7A30 is __thiscall(ECX = foliage system [00F8C210], camera, float), RET 8. The float slot is
  reused as a spill and is never read. When the camera is non-zero, it either loads "bush.mshd"
  through [00F8D394] vslot 48h into +A4h, or (when it is already loaded) sets effect float 0 and 6
  to foliage +08h, float 2 to +08h*0.5 (00D7A280) and byte 13Ch = 1. It then dispatches
  00AD7440 (byte +6 clear) or 00AD7360(camera), and 00AD72A0 when byte +5 is set. Callers:
  005F42D0, 00A8E860, 00A8F3B0. The name says "render for camera" because the calls feed the
  bush group renderer through 00AD7360 / BSP_BushGroup_Render_Provisional.
- 00B1F4D0 is __thiscall(queue, scene, camera, target, BSP_NativeString by value), SEH, RET 14h. It
  constructs a 44h command (00B1F170), assigns the name to command +14h, appends to queue +14h,
  then traverses visible nodes into command +28h and uploads instance groups.
- 00B1F5E0 is __thiscall(queue, scene, camera, target, object), SEH, RET 10h. It builds and appends
  the command in the same way, with no name and no traversal. When object +48h & camera +19Ch is
  non-zero, it calls object vslot 20h(command +28h, 0, 1.0f, 0) and uploads.

## Foliage draw (00AD7A30, 00AD7360, 00AD72A0)

The foliage system is the object at [00F8C210]. Fields used: +05h debug-draw byte, +06h byte
selecting the entry pass (clear) or the group render (set), +08h scalar, +54h and +64h native
pointer vectors (begin +4, end +8 of each descriptor), and +A4h the bush effect. Host:
`FoliageRenderHost`.

- 00AD7A30 (ABI and steps in the named-callee notes below; listing 00AD7A30..00AD7B4D). It calls
  00AD7360 and 00AD72A0 directly. The global is passed in separately, because the native
  rereads [00F8C210] for each float rather than using ECX.
- 00AD7360, RET 4. Camera +198h equal to 2 or 7 skips the bush groups (00AE4CF0 makes the same
  test). The +54h entries are cleared (00AE1C20) in every case.
- 00AD72A0, RET. Quadtree debug draw of the groups (00AE4040) and the entries (00AE1670 ->
  00AE15E0, root +18h).

Both loops reread the end pointer on every step and keep the checked-iterator 00BF6713 calls
(begin past end, then dereference and increment at or past end). The native handler may return,
and the walk then continues as the native does. The constant `test reg, reg` /
`cmp reg, reg` container tests are omitted, because their reg is owner+54h or owner+64h and they
can never fail.

## Foliage quadtree cull and debug draw (segment 81)

Host: `FoliageQuadHost`. The draw-section, stream and camera-plane calls are host methods named
after their ledger callees. 00AE2FE0 is a host method too, so it can bind to
`foliage_cell_classify_plane_00ae2fe0` in docs/CC13_TERRAIN_RENDER.md.

- 00AE4CF0 (group render) clears the mesh sections (00B73C10) at entry, and again at exit
  unless byte +38h is set. Around the walk it sets far 700 and near 0.01, then restores the saved
  +1D8h/+1D4h. Each step happens only when camera +198h is neither 2 nor 7, and the mode is reread
  for the restore. Walk arguments: root +10h, model +0, source +1Ch, stream +20h, layout +24h,
  geometry +28h, material +34h, count 0, and header (+4) +0Ch/+10h for section +10h/+18h.
- 00AE47E0 (cull and emit) sums the plane classes from the last plane down. Any 0 returns the
  running count unchanged. The model transform is refreshed after the loop. A sum of exactly 6
  (all six planes straddling) emits through the inline static-slot allocation (00B85EE0 +
  00B857F0, EH state 1). A leaf (+24h clear) emits through 00533FA0 (EH state 2). Other nodes
  recurse into +2Ch..+38h, threading the count. One emission:
  - instances = +4Ch - +48h + 1;
  - layout, then section +08h = 4, then material;
  - the source's vslots 20h/24h, then a stream slot (00B4B370) built into a view (00B4A9B0)
    over physical source+58h at byte offset +48h << 5, kept in cell +54h with view +54h =
    80000000h. Native has no null check before that store;
  - stream +20h and the view are appended, section +18h/+0Ch/+14h/+10h are written, then the
    instance count;
  - the section is appended to the geometry, the creator references of section and view are
    released, and +54h is cleared.
- 00AE37F0 / 00AE4040 (bush quadtree debug draw) draw from the transform x +120h / z +128h with
  y = 0.0. The `+ 0.0` on the box y is kept. Both branches end with the instance-marker loop,
  whose index checks call 00BF6713 and continue.
- 00AE1670 / 00AE15E0 / 00AE0A50 (entry quadtree debug draw): 00AE15E0 always draws its node box
  in white, raised by 20, and recurses when +34h is set. Box ends are float spills. The y ends
  are float(float(c +/- h) + offset).

## Terrain open items from docs/CC13_TERRAIN_RENDER.md (answered here)

1. Which builder each terrain type uses: none of the three in this image.
   - 00ADD3F0, 00ADE0E0 and 00ADC930 have no E8/E9 rel32 call or jump and no absolute dword in
     any section.
   - The scan is `local/sh1_rel32.py` in the shadow1 tree. As a control, it does find
     00883058 CALL 00ADE820.
   - Ghidra xrefs are empty as well.
   - Live terrain goes BSP_Landscape_LoadTerrain (00882AC0) -> BSP_TerrainHeightField_LoadTdt
     (00ADDA60) / BSP_TerrainHeightField_AttachModel (00ADE820) -> 00ADE500 -> 00ADDA60. These
     load .tdt tiles (00ADFEB0 BSP_TerrainTile_ReadChunks) instead of building from a heightmap.
2. 00ADC930 patch kind and edge test, from the listing:
   - There is one patch type, not two. The 44h object is built by the base constructor
     00AED9A0(21h, 21h) (vtable 00D5D2DC). The derived part then zeroes +38h..+40h, builds a
     second 33x33 grid with 00ADAF50(21h, 21h) on ECX = object+38h (EH state 3), installs
     vtable 00D5D314 and sets +0Ch = 2. That is ordinary constructor chaining.
   - The edge test (00ADCA90..00ADCAB7) replaces the sample with 0.0 instead of reading the
     source. It does so when `[esp+1Ch] - 20h == [esp+24h]` and the row index is 32, or when
     `[esp+84h] - 20h == EBP` and the column index is 32. That reads as: the last patch along an
     axis has no 33rd source sample. The slot meanings (size and origin) are provisional.
   - A patch is created (00ADCE77, after `[00F8C218]` vslot 4) only once some sample exceeds
     0.01 (00D7A238; COMISS/JB at 00ADCB04, then > 0 at 00ADCE3F), or the value at
     `[esp+80h]` (computed in 00ADCB0A..00ADCE36, not traced) is above 0 (00ADCE44).
     Otherwise, after 33x33 samples, the patch is skipped (00ADCE72).
3. Whether the heightmap patch box top reaches a culling decision: not in this image, because
   00ADD3F0 is unreferenced (item 1). The under-covering box top is latent.

## Uncertainty

- x87 exponent range: the double model diverges only when an intermediate overflows or underflows
  double but not the x87 80-bit range. That is not reachable for camera-scale inputs.
- FLD/FSTP copies quiet a signalling NaN, and the plain float copies here do not. No such input is
  expected.
- The ECB0 corner projections and the B930 dead stores are reproduced but have no observable
  effect except through aliasing.
- The meaning of owner +08h (copied to camera +178h) and of the bias matrices +244h is not
  established.
- The LiSPSM and TSM labels are interpretations of the arithmetic. No string or symbol confirms
  them.

## Ghidra

All seven first-batch ledger names and evidence comments were applied with `tools/ghidra_annotate.py --apply` on 2026-10-12 and the project was saved. The prior names and comments are recorded in the worker tree's `local/ghidra-annotations-20261012T005723Z.json`. Exports were not refreshed, because the snapshot is shared and cc13-flowfix1's body repair is still in progress. The second batch (00AD7A30, 00AD7360, 00AD72A0, 00AE1670) was applied the same way; its prior values are in `local/ghidra-annotations-20261012T013625Z.json`. The third batch (00AE0A50, 00AE15E0, 00AE1670, 00AE47E0, 00AE4CF0, 00AE37F0, 00AE4040) is in `local/ghidra-annotations-20261012T015610Z.json`. Lead note: scene2's ledger names for 00B1F4D0/00B1F5E0 supersede the ones given here.

## Handoff (cc13-shadow1, 2026-10-12)

Status at handoff: 14 routines reconstructed and 2 named across the four batches. All are build-tested only, and `reports/cc13_shadow1_calls.json` (129 rows) passes. Next candidates:

- Bind `fit_shadow_cascade_00a8bd20` / `fit_shadow_box_view_00a8aeb0` behind audio1's
  `ShadowCascadeSubmitHost::fit_cascade` / `ShadowSingleViewHost::fit_single_view` once a host
  implements `ShadowFitHost` (camera getters/setters, 00B63F10 look-at, 00B62A10 ortho, 00419510 /
  00419440 with the CRT, 00BF7030).
- Foliage: the draw-section and stream callees (00B4A9B0 is unnamed) and the meaning of the sum-6
  split in 00AE47E0 remain open.
- 00B1F4D0 / 00B1F5E0 could join the render-queue reconstructions (00B1F170, 00B1EB80, 00B1CB50
  are already complete in cc12's native slices).
- A native differential test against the image bodies would move these from build-tested to
  fixture-tested. The four bodies are pure functions of the owner, the camera fields and the host
  results, so a recorded-host harness is enough.
