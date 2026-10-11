# Terrain and foliage render kernels (cc13-render1, segment 81)

Packet `cc13_terrain_render`, partition segment 81 (`00ADA420..00AF3750`). Code:
`include/bsp/terrain_render.hpp`, `src/terrain_render.cpp`. Names are hypotheses, not
recovered symbols. Status: reconstructed and build-tested only; no fixture, not
ABI-compatible, not game-validated.

## Scope and what was left alone

The segment already holds named terrain height-field code (`00ADA890..00AE0390`), the atlas
parser (`docs/ATLAS_PARSER.md`), the foliage group manager and the particle/traceline
pools. The terrain segment line test (`00AE9BD0..00AED100`, `00ADEB40`, `00ADEB80`,
`00ADF1B0`) belongs to the gameplay lane (`docs/SCENE_CONTENTS_HOSTS.md` section 10,
`src/game_hosts_scene_contents.cpp`) and was not touched. This packet took the foliage
(BushGroup / TreeGroup) render side, which had no doc.

## Box to sphere: `00ADA600`

`__fastcall(ECX = out, EDX = box)`, `RET`. Half extent `(max - min) * 0.5` is written to a
stack temporary and measured with `00419440`; the centre is `(min + max) * 0.5`
(`00ADA603..00ADA6D3`, 0.5 is the double `00D7A280`). Callers `00ADB980`, `00ADD3F0` and
`00ADE0E0` (the last two build the "Terrain Visibility Group" nodes). The next bytes,
`00ADA6E0..00ADA71A`, are a separate `RET 10h` routine with no Ghidra function (two index
clamps to `+38h`/`+3Ch` minus one with a `00D5D658` store); not analysed further.

## Foliage cell culling: `00AE4CF0` -> `00AE47E0` -> `00AE2FE0`

- `00AE4CF0` (`RET 4`, from `00AD7360`): unless the camera mode at `+198h` is 2 or 7, the far
  and near planes are set to 700.0 (`00D1F980`) and 0.01 (`00D7A238`) for the walk and restored
  afterwards. The walk starts at the root with the group's model, stream and material
  pointers; its return (instance count) is stored at `+30h`.
- `00AE47E0` (recursive): for each frustum plane (`BSP_Camera_GetFrustum`,
  `BSP_PlaneSet_GetCount`, iterated from the last) it calls `00AE2FE0` (`00AE485E`) and
  returns at once on 0 (`00AE4865`); the results are summed. `CMP [ESP+10h],6` / `JNZ`
  (`00AE488C`): a sum other than 6 recurses into the children at `+2Ch..+38h` when `+24h` is
  set, or emits an instanced draw section for the instance range `+48h..+4Ch`; a sum of
  exactly 6 (every plane straddling) takes a path through
  `BSP_DrawSectionPool_AllocateStaticSlot`. The meaning of that split is not established.
- `00AE2FE0` (`RET 14h`), reconstructed as `foliage_cell_classify_plane_00ae2fe0`: the
  cell box is `x0 +00h, z0 +04h, x1 +08h, z1 +0Ch, y0 +40h, y1 +44h`, offset by the node's world
  translation at `+120h/+124h/+128h` after a conditional `BSP_Transform_RefreshWorldMatrix`.
  For X and Z the code selects the far and near ends with explicit compares; for Y it assumes
  `y0 <= y1`. The far vertex behind the plane gives 0, the near vertex on or behind gives 1,
  otherwise 10.

## Bush cell LOD walk: `00AE1A10` -> `00AE05E0`

- `00AE05E0` (`__fastcall(ECX = count, EDX = planes)`, stack (centre*, half extent*),
  `RET 8`), reconstructed as `terrain_box_classify_planes_00ae05e0`: per plane
  `r = |c|*ez + |a|*ex + |b|*ey`, `s = c*pz + a*px + b*py - d`; `s < -r` returns 2 at once,
  `s < r` makes the result 0, otherwise it stays 1.
- `00AE1A10` (recursive, from `00AE1FB0`, the `BushOjjekt` path): 2 skips the node; an
  internal node (`+34h`) that is not wholly inside recurses into `+1Ch..+28h`; otherwise a
  draw section covers vertices `+2Ch..+30h` (triangle count `n/3`) with material
  `group+110h` when the centre is within the `00AD5720` distance of the point at
  `group+118h..+120h`, else `group+10Ch`. The section's `+34h` keeps that distance.

## Foliage instance bake: `00AE41C0` (unreferenced)

`__thiscall(pos*, vertex_out, index_out, short base, uv_scale*, uv_offset*, scale*)`,
`RET 1Ch`. No rel32 or absolute-dword reference was found in the PE. For every element
`+20h..+24h` (inclusive) of the mesh at `+8h`:

- Two draws from the shared generator `00BD2F10`: yaw over 0..2*pi (`00CE3D9C`) and a value
  over 0.3..1 (`00CE69C8`) that is popped unused (`00AE420F`). The scale is the seventh
  argument (`00AE41C9`), not the second draw.
- Each vertex (24h output stride): position `scale * v`, rotated about Y with
  `x' = c*x - s*z`, `z' = s*x + c*z` (`00AE42EA..00AE432F`), plus the offset; the normal rotated
  the same way and not scaled; UV `uv * uv_scale + uv_offset`; colour from the stream when the
  declaration has semantic 10 (`BSP_VertexDeclaration_HasSemantic`) else `0xFFFFFFFF`.
- Indices of the element plus `base` in 16-bit arithmetic (`00AE4437..00AE448A`).

Reconstructed as `foliage_bake_vertex_00ae41c0` and `foliage_bake_indices_00ae41c0`.

## Named without reconstruction

- `00AE2450` `BSP_BushGroup_Initialise` (from `00AD9050`): `_normal.tga` texture names,
  a `count*6` vertex stream in `pf44nss2uf42uf44cc.mvfm`, the `BushGroup` model (vtable
  `00D5D4A0`), `bush.mshd` / `bush_lores.mshd`.
- `00AE5870` `BSP_TreeGroup_Initialise` (from `00AD91C0`): the TreeGroup counterpart with
  `seaweed.mshd` / `coral.mshd`.
- `00AE5070` `BSP_TreeGroupInstance_Construct`, `00AE1E10` `BSP_BushGroupType_Construct`.

## Foliage quadtree loaders: `00AE0F10` and `00AE5550` (cc13-render2)

Both loaders are recursive and read their tree from the stream that the group initialiser
receives as argument 1 (`00AE2450`, `00AE5870`). They call two slots on that stream. Each
slot is `__thiscall` with a single stack argument that is always 0 (`PUSH EBP` or
`PUSH EBX` with the register already zeroed):
- `+38h` returns a dword in `EAX`;
- `+44h` returns a float on `ST0`.

These meanings come from the return registers and from how the values are used. The stream
class itself is not identified. The pseudocode for both loaders is unreliable, so the
reading below comes from the listing. The x87 tails were checked against raw bytes
(`DC CA`, `DE EA` at `00AE0FFA`; `DC C9`, `DE C2` at `00AE5628`). Capstone's text for these
forms is easy to misread.

`00AE0F10` builds the bush node, 38h bytes. It is called by `00AE2450` at `00AE2ACF` and
walked by `00AE1A10`. ABI: `__thiscall(ECX = new node, reader, centre*, half*, y_offset)`,
`RET 10h`, returns `ECX`. The caller passes the group's world Y (`[node+124h]` after a
conditional `BSP_Transform_RefreshWorldMatrix`) as `y_offset`, and passes half extents
computed with the 0.5 at `00D7A280`.
- The record starts with `dword leaf` and then `dword +2Ch` (first vertex).
- An internal node (`leaf == 0`) then has four `{dword present, child record}` entries.
  Each present child adds 1 to `+34h`. The child's half extent is half the parent's. The
  child's centre is offset from the parent's in x and z in the order (-,-), (+,-), (+,+),
  (-,+). Its y is `y - h`, `0.0` (`XORPS`), `y + h`, `0.0`.
- Every node, leaf or internal, ends with the tail from `00AE0FAE`: `dword +30h`
  (vertex end), then `float y0` and `float y1`. Each float has `y_offset` added and is
  rounded to float. The node gets `centre.y = (y0 + y1) * 0.5` and `half.y = (y1 - y0) * 0.5`.
  So the y value the parent passed in, including the odd `0.0`, is never observable.
- A leaf stores `FFFFFFFFh` at `+18h` and nulls `+1Ch..+28h`. An internal node leaves `+18h`
  unwritten.

`00AE5550` builds the foliage cell node, 58h bytes. It is called by `00AE5870`. `00AE47E0`
culls it through `00AE2FE0`, and `00AE37F0` draws it. ABI: `__thiscall(ECX = new node,
reader, rect*)`, `RET 8`. The rect is `x0, z0, x1, z1` at `+00h..+0Ch`.
- On entry the loader zeroes `+14h..+1Ch` (a vector), `+24h` and `+54h`.
- The record starts with `dword leaf` and then `dword +48h`.
- An internal node splits at `mx = x0 + (x1 - x0) * 0.5` and `mz = z0 + (z1 - z0) * 0.5`;
  the difference stays on the x87 stack. The four children are `(x0,z0,mx,mz)`,
  `(mx,z0,x1,mz)`, `(mx,mz,x1,z1)` and `(x0,mz,mx,z1)`. Each present child adds 1 to `+24h`.
- Every node ends with `dword +4Ch`, then `float +40h` and `float +44h`. `+48h..+4Ch` is the
  instance range `00AE47E0` draws, and `+40h/+44h` is the y range that `00AE2FE0` tests.
- A leaf never writes `+2Ch..+38h`. The cull reads those slots only when `+24h` is set.

Reconstructed as `bush_quad_node_read_00ae0f10` and `foliage_quad_node_read_00ae5550`,
with the stream behind the `FoliageQuadReader` interface. A failed `operator_new` leaves the
native slot null without consuming the child's record; the C++ does not reproduce that path.

## Foliage quadtree debug draw: `00AE4040` -> `00AE37F0` (named only)

`00AE4040` is `MOV EAX,[ECX]; MOV ECX,[ECX+10h]; PUSH EAX; CALL 00AE37F0`. It passes the
group's root node `+10h` and the group's model. Its only caller is `00AD72A0`.

`00AE37F0` (`__thiscall(node, model)`, `RET 4`) reads the model's transform through
vtable `+40h` and takes the world x and z from `+120h` and `+128h`. It draws with the
renderer slot `+B4h` (`[00F8D394]`). That slot is `00B290D0` (vtable `00D5F0A8 + B4h`), which
reserves a 16-byte record through `BSP_RendererRecords16_Reserve`. Each call passes two
points and two colours.
- A leaf with an empty vector at `+14h` draws the rect at y = 0 in white.
- Any other node draws the box from `+40h` to `+44h`. The colour is `FFFF0000h` (A, R) when
  byte `+28h` is set and `FF00FFFFh` otherwise.
- The draw then recurses into `+2Ch..+38h` when `+24h` is set, and makes one call per element
  of the `+14h` vector. That call was not traced.

This is debug visualisation, so it is named only.

## TreeGroup type: `00AE5160` (named only)

The handoff called `00AE5160` the "TreeGroup render" function. It is not a render routine.
It is the TreeGroup type constructor, the counterpart of `00AE1E10`. Its only caller is
`00AD7030`, and it takes a model name. What it sets:
- `+38h` is the name, as a NativeString.
- `+00h` is the resource loaded through `BSP_NativeResourceManager_LoadDefaultFactory`.
- `+04h` is that resource's vtable `+8` call with `(0, 1.0f)`.
- `+20h..+24h` is the element range, from `00B732B0` when one exists and otherwise
  `0..count-1`.
- `+30h` is the texture name and `+28h` is the effect name. Both come from the first element.
- `+0Ch` and `+10h` are the sums of the elements' `+10h` and `+18h`. These are likely vertex
  and index counts.
- `+14h` is the bounds' `+10h - +04h` (the y extent).
- `+18h = -1e10` (`00CE4ADC`, `D01502F9`) and `+1Ch = 1e10` (`00CE4970`).
- `+40h` is the material from `BSP_Material_CreateForEffectName`, with texture slot 0 set
  from the renderer's `+64h` load of the texture name.
- `+44h = 0`.

## Terrain Visibility Group builders: `00ADD3F0`, `00ADE0E0`, `00ADC930`

All three have no direct callers (`callers (0)`; they are presumably virtual). All three
build one node named `Terrain Visibility Group` (`BSP_Node_Construct`). Under it they build
patches of 33x33 vertices (`00AEDAC0(21h, 21h)`, a 38h object, attached by `00ADF8B0`).
Each patch's local box comes from `00ADA600` after dividing by the node's world matrix
diagonal (`+F0h`, `+104h`, `+118h`).

`00ADD3F0` is the heightmap form: `__thiscall`, seven arguments, `RET 1Ch`. Frame:
`0Ch + 8Ch + 10h`, so the arguments start at `[ESP+ACh]`. Argument 3 is the name copied to
`+54h`. Arguments 4..7 are `h_base`, `h_top`, `s_low`, `s_high`.
- It loads an 8-bit image (`00BEFCB0`) and flips its rows (`00ADD4D0..00ADD5A8`).
- It walks 32x32-cell patches. For each patch it calls `[00F8C218]` vtable `+4` with
  `(group, col >> 5, row >> 5)`.
- For each of the 33x33 samples it calls the patch's vtable `+1Ch` with `(h, x, z)`.

The sample kernel is `00ADD755..00ADD82A`, reconstructed as `terrain_heightmap_patch_00add3f0`.
Two findings in it:
1. The scale is `(h_top - h_base) / (s_high - s_low)`, but `s_low` is not subtracted from
   the sample. The mapping lands on `h_base..h_top` only when `s_low = 0`.
2. The second accumulator is stored as `max(h, running minimum)`. That is always `h`, so
   after the loop it holds the height of vertex (32, 32). `00ADD8E6` divides it by the
   world y scale and uses it as the patch box top. The box therefore under-covers any patch
   whose last vertex is not its highest. Whether this matters in game depends on the
   culling user of that box, which was not traced.

`00ADE0E0` is the flat form (`__thiscall(group, name)`). It builds 4x4 patches; this is read from the pseudocode (`00ADACF0(4)`, a loop bound of 4) and was not checked against the listing. Each patch
gets two 33x33 grids: the second one is attached by `00ADF920`. Every sample is written as
height 0, and the same min and "max" pair gives a box of y 0..0.

`00ADC930` (`__thiscall(group, source)`) takes its size from the source's vtable `+20h`,
in units of 32 cells. It builds 44h-byte patches of two kinds: vtables `00D5D2DC`
(`00AED9A0`) and `00D5D314` (`00ADAF50`). The pseudocode compares patch positions with `size - 20h`, which looks like an edge test; that reading was not checked against the listing. Named only.

## Handoff (cc13-render2, 2026-10-11)

Done here: `00AE0F10`, `00AE5550` and the `00ADD3F0` kernel are reconstructed and
build-tested. `00AE37F0`, `00AE4040`, `00AE5160`, `00ADD3F0`, `00ADE0E0` and `00ADC930` are
named. Open:
- Which of the three builders each terrain type uses, which needs their vtables.
- `00ADC930`'s patch-kind choice and edge test.
- Whether the patch box top (finding 2 above) reaches a culling decision.
- The `BSP_Shadow_*` helpers that cc13-render1 listed. `00AE0F10` in that range turned out to be the bush loader.
- The terrain segment line test stays cc9's (`SCENE_CONTENTS_HOSTS` section 10).
