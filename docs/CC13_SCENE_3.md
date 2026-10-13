# CC13 scene pass 3: the hi-res tile renderer and the flare geometry builder (cc13-scene3)

Worker cc13-scene3, packet cc13-scene3, 2026-10-13, branch `agent/cc13-scene3`. This pass finishes the two bodies that
docs/CC13_SCENE_2.md left: `00B708E0` and `00B8DD60`. Both were read from a capstone listing over the PE
(`local/s3tools/s3_dis.py`). For `00B708E0` that was necessary because Ghidra's decompile of it is stack-corrupted.

Status: **reconstructed, build-tested** only. That means `./scripts/build.ps1` passed. Neither function is
ABI-compatible, fixture-tested or game-validated, and neither is wired into a host. No test was added. The code is in
`include/bsp/scene_graph_cc13.hpp` and `src/scene_graph_cc13.cpp`, which now include `bsp/scene_graph2_cc13.hpp` for
the flare types. Renderer, surface, atlas and draw-section calls go through `HiresTileServices` and
`FlareGeometryServices`; each virtual names its image address or vtable slot.

## 00B708E0 BSP_Camera_RenderHiresTiles

ABI: `__thiscall(ECX camera, const char* file, int width, int height, callback, context)`, RET 14h, returning AL.
The arguments were mapped from the frame after the four register pushes (ESP = entry - D0h): `[esp+D4h]` is file,
`+D8h` width, `+DCh` height, `+E0h` the callback and `+E4h` its context. The callback is called with ECX = context
and no stack arguments.

1. The renderer (`00F8D394`) is asked whether it is in a scene (`+2Ch`). If it is, the scene is ended (`+14h`) and
   begun again at exit (`+0Ch`).
2. The viewport size comes from `00B1F740` on camera `+180h`, read twice. Then `tx = width / vw` and
   `ty = height / vh` by signed IDIV, and the target surface is `+84h(tx*vw, ty*vh, 16h, 2)`.
3. The projection at `+1E0h` (16 floats) is saved with REP MOVSD, and then:
   - `P[0] = f(FILD tx * P[0])`;
   - `P[5] = f(FILD ty * P[5])`.
4. The loop runs columns `j < tx` on the outside and rows `i < ty` on the inside. `x` advances by vw per column and
   `y` by vh per row. For each tile:
   - `P[8] = f(f(tx-1) - 2j)` and `P[9] = f(-(f(ty-1) - 2i))`, where `f(n-1)` comes from CVTSI2SS;
   - all 16 entries are reloaded and stored in place with FLD/FSTP, which only quiets a signalling NaN;
   - the matrix is copied to `+2A0h`, and the flags become `(flags & FFFFFF4Bh) | 8`;
   - BeginScene runs, then the callback, whose AL is kept, then EndScene;
   - a 3Ch-byte stack holder (`00B3F450` / `00B3F4E0`) receives the back buffer (`+7Ch`). When that succeeds, it is
     copied into `{x, y, x+vw, y+vh}` of the target (`+38h(holder, 0, &rect)`).
5. Afterwards the saved projection is restored to `+1E0h` and `+2A0h` by MOVSS, which is bit-exact, and the flags
   are updated again.
6. The target is saved to `file` (`+34h(file, 0)`) and deleted (`+4h(1)`).
7. The result is the last tile's callback AL, or 0 when there are no tiles.

## 00B8DD60 BSP_Flare_BuildGeometry

ABI: `__thiscall(ECX flare, const element vector*)`, RET 4.

1. The element pointer vector is assigned (`00B8D850` on `+18Ch`), and the mesh object is built (`00B73B60` with
   ECX = 0BCh, then `00B73D70`).
2. The vertex declaration is `"simplecolor2.mvfm"` (renderer `+38h`). The vertex buffer holds `4n` vertices
   (`+5Ch`, usage 1); the declaration is released straight after.
3. Flare `+1CCh` gets the layout from `+40h`. Its stack record holds the declaration, three frame words and 1.
4. Vertex stream 0 is set (`00B73BB0`).
5. The index buffer holds `6n` indices (`+60h`, usage 1, format 65h) and is locked through its `+0Ch`. Element `i`
   gets the indices `4i, 4i+1, 4i+2, 4i+2, 4i+3, 4i`. In the same loop, for each element:
   - both atlas names (`+48h`/`+50h`) are compared with a fresh empty string (`0041DD40` + `00CE3A0C`); an empty name
     gives a null rect, otherwise the rect comes from `00AEFB20` on atlas `00F8C26C`;
   - size (`+34h/+38h`) = origin (`+8h/+0Ch`) by FLD/FSTP.
6. The buffer is unlocked (`+10h`), set as the index stream (`00B73B70`) and released.
7. Two materials are created with `00535320`: M0 is `"flare.mshd"` and M1 is `"flare2.mshd"`.
8. One pass over the elements:
   - type 0 and type 1 count `n0` / `n1`. The first element of either type, while both counts are still 0, sets M0
     slot 0 to the texture (`+8h`) of its `rects[0]`, with no null test;
   - type 2 counts `n2`. The first one sets M1 slots 0 and 1 from `rects[0]` / `rects[1]`; a null rect gives a null
     texture;
   - other types are skipped.
9. Up to three draw sections are built (`00B85EE0` with ECX = 60h, then `00B857F0`). Each has primitive type 4,
   vertex count `4n` and primitive count `2n`:
   - `n0` quads with M0, starting at vertex and index 0;
   - `n1` quads with M0;
   - `n2` quads with M1.

   Each section starts after the previous section's vertices and indices. Each gets vertex stream 0 (`00B73260`),
   the flare layout and its material, is appended to the mesh (`00B73C60`) and released. The ranges are written
   before any null test on the section. The layout assumes the elements are already grouped by type: the vertex
   order follows the element order, while the sections follow the type counts.
10. M0, M1 and the mesh are released around `00B75170(flare, 0, mesh, -1.0f, -1.0f)`, and then `00B8DAC0` runs.

The old ledger evidence for `00B8DD60` named `"flareocclusion.mshd"` and a call to `00B8B980` at `00B8DCD5`. Both
are wrong for this body: `00B8DCD5` lies before the function. The evidence was rewritten; the name is unchanged.

## Precision

- The only x87 arithmetic is in `00B708E0`: FILD×float products and the tile offsets. The host evaluates them in
  double and rounds each stored float once. That matches the image under 53- or 64-bit precision control.
- The listings contain no DC-form or reversed x87 forms.
- FLD/FSTP copies are modelled with signalling-NaN quieting.

## Handoff

Done. Both bodies are recorded with `ledger add-function`. There is no open question beyond host binding, which no
caller needs today.
