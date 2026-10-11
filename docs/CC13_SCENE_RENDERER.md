# Scene renderer object, segment 82 render half (packet cc13_scene_renderer)

The address range 00B0CF80..00B1BD20 belongs to segment 82's partition, but it is **not particle code**.
It is the scene-renderer object, the "render service" at [00F8D39C].

The code is in `src/scene_renderer_cc13.cpp` and `include/bsp/scene_renderer_cc13.hpp` (namespace
`bsp::scene_renderer`). Names are hypotheses.

- **Status.** Everything below is reconstructed and build-tested. None of it is fixture-tested,
  game-validated or wired into the host.
- **Calls.** `reports/cc13_scene_renderer_calls.json` holds 10 rows, all of which pass
  `tools/verify_report_calls.py`.

## 1. Overlap check

These parts are already covered by other work, and this packet skips them:
- construction, parameters and texture vectors (docs/NATIVE_RENDER_SERVICE_*.md);
- publish and destroy base (00B0F020, 00B0F0C0);
- render resources: release, depth target, remap textures (00B0F6E0..00B0FEB0);
- the batch-preparation job (00B0D930, 00B0F1C0..00B0FFB0);
- the post-effect pass scalar deletes (00B10120..00B101A0);
- material-record allocation (00B0D3B0);
- `src/native_render_service_base.cpp`.

No cc12 lease touches this range at the time of writing.

Of the 80 remaining `FUN_` entries, 41 have no caller in the index. A rel32 and absolute-dword scan of 31 of them found references only for 00B17240 (the D5E13C pointer) and 00B19910 (rel32 from 00B19B06).
A spot check shows template instantiations (RefPtr release bodies at 00B0F310..00B0F520, spaced 30h), so
the image appears to keep unreferenced COMDAT bodies. They are not reconstructed.

## 2. Service fields set here

| Field | Writer | Meaning (evidence) |
| --- | --- | --- |
| +219h | 00B0D580 | motion blur (Settings_ApplyAll, settings +8Dh) |
| +21Ah, +258h..+268h | 00B0D020 | lens effect on/off and five words (binocular view 00452B80) |
| +21Bh, +250h, +254h | 00B0CF80 | sight blur: word, amount, enabled = amount ≥ 0.0001 |
| +21Ch | 00B0D160 | byte, caller 00BA0090 (open) |
| +220h | 00B0D080 | old-film mode (options) |
| +224h | 00B0D0B0 | value; the movie-screen exits store 0.0 (open) |
| +238h..+24Ch | 00B0D6A0 / 00B0CFC0 | start, rate = (end − start)/duration (or 1.0), duration, three words |
| +30h | (read) | the render listener; 00B0D170/180/190 tail-call 00B4EF90 / 00B4EC90 / 00B4F030 on it |

## 3. Routines

- **00B0D580 BSP_RenderService_SetMotionBlurAndSamples** — `(uint8 motion_blur, int samples)`, RET 8.
  Ghidra shows one argument; RET 8 and the EDI forward show two. It pushes the antialias sample count
  into the render textures +44h/+48h (00B4D270), the shadow map +64h's texture holder (00B4D170, fetched
  twice) and the targets +50h/+58h (00B3CE20 sets target+24h). It calls device [00F8D394] vtable +80h
  with an 8-byte local whose result it never reads.
- **00B0D170 BSP_RenderService_AttachToListener.** 00AF63A0 hands it a particle model's second mesh;
  BSP_Waterdrops_Spawn also calls it.
- **00B0D190 BSP_RenderService_FlushListener** is the `flush_render_resources_00b0d190` host call in
  `src/game_render_frame.cpp`.
- **00B17240 BSP_SceneRenderCommand_Execute** — the function pointer at D5E13C.
  - When 00B20240 (BSP_D3D9Renderer_CanExecuteCommand) passes, it calls 00B46A70(ECX = command+4,
    EDX = 00B1BF40(command)), then device +114h(command+1Ch), then **00B156C0(ECX = service, command)**:
    the scene render.
  - Ghidra drops the register arguments of 00B46A70.
- **00B0DAA0 BSP_RenderRecord_NameLess.** Name ordering on +124h/+128h with _stricmp. Its only caller is
  00B13790.
- **00B18AE0 / 00B18B40** — BSP_Material_RegisterFloat3Parameter / ...MatrixParameter. These are the
  missing siblings of the recovered 00B18AA0/00B18B00/00B18B20: 00B17E10(..., 3, 0) and (..., 16, 1).

## 4. Handoff

- **The scene render spine is not reconstructed.** None of these has a doc or source:
  - 00B156C0 is the scene render, about 1B00h bytes, reached from 00B17240.
  - Below it are 00B151E0 (4E0h), the recursive 00B148E0 (900h), and its callees 00B13C90, 00B13D00,
    00B13F80, 00B14400, 00B139C0, 00B137E0, 00B133D0, 00B13BF0 and 00B13790.
  - 00B0DB00 (E70h) and 00B10500 are called from 00B156C0. 00B0EB10 (800h) is called from 00B156C0 and
    00BA0090. 00B105B0 is called from 00B151E0 and 00B171C0.
  - Most of these are too large to decompile reliably; check the listing (x87 and register arguments).
- **The front-end preview group** is 00B0E9B0, 00B0EA20, 00B0EA70 and 00B101C0. docs/GAME_RENDER_FRAME.md
  describes it as the front-end 3D preview draw. Check `src/game_render_frame.cpp` before starting.
- **Small leaves with unidentified owners:** 00B172E0 (+B0h setter, caller 00ACC8A0), 00B17310 (+104h
  setter, callers 005A8F50/005A9770/0093DE10) and 00B19A60 (calls vtable +28h on each 2Ch record of a list,
  caller 00B33A10). They are not on the render service; their objects are open.
- **Open meanings:** +21Ch, +224h, the three trailing words of 00B0D6A0, and the vtable +80h query in
  00B0D580.
