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

## Handoff

Open, in order of value: the quadtree builders `00AE0F10`, `00AE5550` and `00AE37F0`
(recursive, garbled stack frames, need the listing); the TreeGroup render path
(`00AE5160`); the "Terrain Visibility Group" builders `00ADD3F0`/`00ADE0E0` (mentioned in
`docs/NATIVE_NODE_RAW_CONSTRUCTION.md`); `00ADC930` (2048 bytes); the shadow helpers
`00AE0F10..00AE1C80` around the named `BSP_Shadow_*` vectors.
