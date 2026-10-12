# Scene renderer object, segment 82 render half (packet cc13_scene_renderer)

The address range 00B0CF80..00B1BD20 belongs to segment 82's partition, but it is **not particle code**.
It is the scene-renderer object, the "render service" at [00F8D39C].

The code is in `src/scene_renderer_cc13.cpp` and `include/bsp/scene_renderer_cc13.hpp` (namespace
`bsp::scene_renderer`). Names are hypotheses.

- **Status.** Everything below is reconstructed and build-tested. None of it is fixture-tested,
  game-validated or wired into the host.
- **Calls.** `reports/cc13_scene_renderer_calls.json` holds 47 rows, all of which pass
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

## 4. Scene properties and small setters (packet cc13_scene_spine)

`004E8510` (caller `BSP_SceneRecord_ApplyHeaderProperties`) reads three property bags from the scene
header and hands each block to the service at `[00F8D39C]`. The bag keys name the fields:

| Setter | Fields | Keys (in block order) |
| --- | --- | --- |
| 00B0E9B0 | +84h..+ACh, +A8h a byte | HDR: BrightPassTreshold, BrightPassOffset, MiddleGray, `1 - AdaptationPercent/100`, BloomScale, MinLuminance, MaxLuminance, NewHDRParam0, NewHDRParam1, AntiBandingON, ParticleLuminanceMul |
| 00B0EA20 | +230h..+24Ch | MotionBlur: MaxVelocity, VelocityScale, MinSpeed, Divider, MaxBlur, then three zero words |
| 00B0EA70 | +28Ch..+2A8h, and a copy at +628h..+644h in the order 2,1,0,3,6,5,4,7 | UnderWaterWave: HorizSpeed, HorizScale, HorizPhase, HorizAmplitude/1000, VertSpeed, VertScale, VertPhase, VertAmplitude/1000 |

- **Motion blur.** `BSP_LuaBinding_SetMotionBlurParams` (0088E560) builds the same block as
  `{1.0, 0.5, a0, (a1 - a0)/a2, a2, 0, 0, 0}`. +238h (MinSpeed) is the threshold that 00B101C0
  tests. The last three words are a velocity impulse: 00B101C0 adds them once and then zeroes them.
- **Correction.** The section 2 and header text for 00B0D6A0/00B0CFC0 ("start, rate, duration") is
  a reading of the setter alone. The bag names make +238h/+23Ch/+240h MinSpeed/Divider/MaxBlur, and
  +244h..+24Ch the impulse. The C++ names are kept as they are.
- **Small setters.**
  - **00B172E0** stores effect+B0h, the signed sort priority. The GUI model setup passes 0.
  - **00B17310** stores material+104h, the alpha reference (see `BSP_Material_GetWord104`).
  - **00B19A60** calls vtable +28h on every entry of the 004DE4B0 registry (2Ch entries, object at
    +28h). The registry holds texture sources, not particle clocks: 00B1B4D0 looks samplers up in
    it, and the Caustics/ShoreWave sources (vtables D64478/D644B4) have
    `BSP_TextureSource_ClearChildren` (C303B0, releases the frame textures) at +28h and
    `BSP_TextureSource_SetTime` (C302A0) at +18h. The base vtable D5E554 has a bare RET at +28h.
    The ledger name of 004DE4B0 (`BSP_ParticleClock_GetSingleton`) is therefore misleading; it
    belongs to another lane and is left as is.
  - **00B33A10** is renderer vtable 00D5F0A8 slot +E8h, called by `BSP_Game_TeardownSessionState`
    (004DAD4A). It runs 00B19A60, then clears byte +B4h, the "in use" flag that material
    construction 00B18900 sets, on every effect in the renderer's effect cache (+1A9Ch/+1AA0h).

## 5. Scene submission and the motion-blur node (00B101C0, 00B10500)

- **00B101C0** `(service, scene, camera)`, RET 8. Its callers are `BSP_FrontEndPreview_Draw` and
  0068A0D0. It tracks node +1C0h for motion blur:
  - +22Ch = node present and node+44h == 0.
  - When +22Ch is set: velocity +19Ch = (position - previous +1B4h) / dt + impulse.
    - dt = elapsed / frequency from `[01090AB0]` vtable +1Ch.
    - +1B0h = |velocity|. The new position goes to +1B4h.
  - +22Dh (blur active) is set when speed > MinSpeed and motion blur (+219h) is on. It stays set
    while speed > MinSpeed × 0.95 (0.05 is at [00D5E1C8]).
  - While blur is active the node's visibility factor is 0.0; otherwise it is 1.0.
  - It then constructs the 44h command (vtable D5E13C, 00B17240) for (scene, camera, +1D4h).
  - Clip plane: when camera y (+124h) < -5.0 it sets the extra clip plane (0, -1, 0, 2); otherwise
    it clears it.
  - It appends the command with flags 1 and sets the last-command metadata {0, 24h, 5Ch}.
  - When byte +B0h is set it retains the camera in +10h.
- **00B10500** `(service, command, targets)`. When the node is tracked, it draws the node alone
  through the queue:
  - 00B1F5E0(scene = camera+A4h, camera, targets, node), then 00B1E6D0 runs and pops that command.
  - Around the draw: clear flags 0, visibility 1.0, then visibility 0.0 again.
  - The blurred passes therefore leave the node out, and it is drawn unblurred.

## 6. The scene render 00B156C0

`(service, command)`, RET 4. The body is about 1B00h bytes. It was reconstructed from the listing,
not from the pseudocode, which loses the stack after the first batch block.

Camera = command+28h -> +8h (00B1BF40). The steps run in this order.

1. **Device check.** Return unless the renderer's vtable +2Ch reports that it can render.
2. **Frame constants.**
   - Save the near plane, then set it to 0.5.
   - Save the clear colour with its top byte forced to FFh.
   - Motion-blur direction: n = normalize(+19Ch). +1A8h = -0.5·(n·X) and +1ACh = -0.5·(n·Y), where
     X and Y are the camera's projected axes.
   - 00B0DB00(+314h, clock float `[01090AB0]+4`, camera).
   - +5A8h = the delayed view-projection (+55Ch). +5E8h = inverse(camera view-projection).
   - From the +4Ch target: +4h/+8h = 0.5/size and +184h/+188h = 1/size.
   - A 3×3 kernel of float4 offsets at +F4h..+180h.
   - +B4h = 00B0EB10 far-plane corners.
   - When the lens effect (+21Ah) is on: the bloom half texel goes to +18Ch/+190h, and the five
     +258h words become +26Ch..+27Ch (a/20000, 20000/b, clamp(1-d), 20000/e, clamp(1-f)).
   - +1D8h..+214h = the clip-to-texture matrix (0.5, -0.5, plus the half-texel offset).
3. **DRSH.** The geometry pass into three colour targets.
   - Targets +1C8h, with colour surfaces +38h/+3Ch/+40h.
   - Clear flags 7, colour FFFFFFFFh, render mode 6.
   - Both batch lists run (helper below). Then 00B10500 runs if +22Dh is set.
   - Restore the render mode, clear flags and near plane. Then +60h vtable +10h, and restore the
     clear colour.
4. **SCNE.** Colour pass into targets +1CCh.
   - The colour surface is +48h when blur is active, otherwise +44h.
   - Both batch lists run, the target is resolved (00B4CAF0), and texture stages 0..7 are unbound.
5. **POST.** Starts with +680h = 0.
   - When blur is active: the +664h pass draws texture +48h into surface +44h, clears the depth (flags 2), and
     runs 00B10500 again.
   - List 1 is drawn into the raw surface +58h with render mode 8/9 (camera mode 0/3), and +58h is
     copied to +5Ch (00B3CDA0, `BSP_D3D9Surface_StretchFrom`: StretchRect, linear filter).
   - The +64h shadow-map holder target with the device depth surface, then passes +68h (+44h) and
     +6Ch (+54h, +60h holder). List 1 runs again with mode 10/11.
     - When the cockpit helper +0Ch (BSP_CockpitHelper_Construct) has a visible node at +8h,
       `BSP_CockpitHelper_BeginFrame` (00B3C960: cockpit camera synced to the main camera, cockpit
       shadow) and `BSP_CockpitHelper_DrawCockpit` (00B3C6E0: the node drawn with the cockpit camera
       in render mode 12) bracket that pass, it uploads constants first, and it unbinds afterwards.
   - Post objects:
     - +18h vtable +10h; device +114h({0, 10h, 70h}).
     - +1Ch and +2Ch only when the lens effect (+21Ah) or sight blur (+21Bh) is on.
     - Luminance +20h with +90h. **Correction:** the ledger calls that argument a clear colour, but
       +90h is the HDR `1 - AdaptationPercent/100`.
     - Tone map +24h with (luminance+23Ch, +84h, +88h, +8Ch, +98h, +9Ch); then +28h.
   - Composite:
     - +74h (lens: luminance, bloom and +38h), +80h (sight blur: luminance and bloom), or +70h.
     - sRGB write is on for the composite.
   - Distortion +30h renders, and its extent goes to +194h/+198h.
   - Final copy into the back-buffer targets +1D4h:
     - **Old film.** Used when film mode +220h ≠ 0 and (+224h > 0 or mode ≥ 2). Passes +658h/+65Ch
       (by +21Ch); +228h, +648h and +64Ch are set; then 00B529A0(+34h, +1D4h, +2ACh, flag, extent y,
       +224h).
     - **Plain.** Otherwise passes +650h/+654h.
     - Slot 1 is the distortion output when +30h exists and the cockpit node is not visible.
       Otherwise it is +668h.
     - The +654h variant skips the cockpit test. The other three call 00B3C550 on +0Ch without a
       null check.
   - 00B151E0 runs, then the label reset.
- **Batch helper.** It is used twice.
  - When guard byte 0108D6DC is 0 and both lists hold more than 50 batches: list+8 = index, both
    lists go to the frame job pool, and it waits (+8(1)).
  - Otherwise each list prepares itself (vtable +0Ch).
  - Either way the lists then execute (vtable +8h(index, camera)).
- **Labels.** "DRSH", "SCNE" and "POST" are assigned into `[00F8D39C]+684h` (the global, not ECX).
  00B13510 resets the label to "X" at each scope end.
- **RNG.** Nothing on this spine draws from 00BD2F10.

## 7. Helpers

- **00B0EB10** `(ECX unused, camera, out[16])`, RET 8. Builds the perspective from the camera fov,
  aspect, near and far, with near clamped up to 0.5.
  - It inverts it and transforms NDC (±1, ±1, 0, 1), dividing by w.
  - Scale far/near, rotate by the world 3×3 (+F0h rows), w = 1.
  - It also rotates the four near points; that result is dead.
  - The other caller is 00BA0090.
- **00B0DB00** `(history = service+314h, time, camera)`, RET 8. Delayed camera history.
  - 8 slots of {world, fov, time}, 48h each from +8h. Count at +4h, delay at +0h.
  - It restarts when the camera moved more than 20.0 since +288h, and drops the oldest slot when
    full.
  - Interpolation:
    - Lerp position and the Z and Y columns between the bracketing slots, with the fraction clamped
      to [0, 1].
    - Re-orthonormalize: x = n(y×z), y = n(z×x). The squared sums add y² first.
    - Lerp the fov.
  - +248h = invert_affine(world) × perspective (00B63B30, 00413920).
  - The bracketing walk only moves back while time[c] < t ≤ time[d], which increasing times never
    satisfy, so in practice the newest pair is used.
  - With one slot the fraction is ±inf/NaN. -inf clamps to 0. NaN would pass the clamp
    unchanged; the native does the same.
- **00B0E970** `(command, index)`, RET 4: list vtable +0Ch(index) and +8h(index, camera) for
  command+0Ch+4·index, the unthreaded batch step. Nothing references it (rel32, absolute and Ghidra
  scans), so it is named but not reconstructed.
- **00B151E0.** The per-effect render-statistics overlay, enabled by byte +681h.
  - It sorts the 12Ch records at +690h with std::sort (00B148E0 and its helpers 00B13790, 00B13BF0,
    00B13C90, 00B13D00, 00B13F80, 00B14400, iter_swap 00B13350). The order is 00B0DAA0 on the key
    at +124h.
  - The records come from 00B16F80 (docs/NATIVE_MATERIAL_DIAGNOSTICS_DRAW_CN.md): +11Ch is the pass
    label current at record time, +124h is label + effect name, and +4h/+3Ch/+74h/+ACh/+E4h count
    draws, primitives, vertices and shader registers per render mode.
  - It prints the records whose label (+11Ch) is in the +69Ch label list (00B105B0).
  - Each of the 14 render modes with draw calls gets a line `"%s : %s - RM(%i) - DP(%i) - PRM(%i) -
    VSF4(%i) - PSF4(%i)"`, then a total line. Lines are 18 px apart, in 500 px columns that wrap at
    600.
  - The print, 004B47E0, is a bare `RET` in this image, so nothing is drawn.
  - The C++ uses std::sort with a typed record, so equal keys may be ordered differently.

## 8. Status, uncertainty

| Routine | Coverage | Evidence |
| --- | --- | --- |
| 00B156C0 | complete (all paths) | listing B156C0..B16F7A |
| 00B101C0, 00B10500, 00B105B0 | complete | listing; x87 forms checked from bytes |
| 00B0EB10 | complete (dead near-point rotation omitted) | listing |
| 00B0DB00 | complete | listing plus `tools/x87_sym.py` walk |
| 00B151E0 | complete; std::sort substitutes the 00B148E0 instantiation | listing |
| 00B0E9B0, 00B0EA20, 00B0EA70, 00B172E0, 00B17310, 00B19A60, 00B33A10 | complete | listing |

- **Status.** 00B156C0, 00B101C0, 00B10500, 00B0EB10, 00B0DB00, 00B151E0 and 00B105B0 are
  reconstructed, along with the three bag setters and the three small setters. They are
  build-tested only: no fixture, no host wiring, no game run.
- **Rounding.** x87 extended intermediates are modelled with double, with float roundings where
  the listing stores. Sums of three products and divisions can differ in the last bit in rare
  cases. Float copies made with x87 (memcpy here) would quiet a signalling NaN in the native.
- **Later packets.** The queue helpers and the cockpit frame calls are in section 10.

## 10. Segment 83 queue helpers and the cockpit frame (packet cc13_seg83_queue)

Queue storage follows docs/RENDER_COMMAND_QUEUE.md: command pointers at +14h/+18h/+1Ch, plus a
world-label row vector at +24h/+28h/+2Ch with 14h rows {NativeString text, float pos[3]}.

| Routine | Name | Coverage |
| --- | --- | --- |
| 00B1F5E0 | BSP_RenderCommandQueue_SubmitDrawable | complete; confirms cc13-shadow1's Ghidra name |
| 00B1F4D0 | BSP_RenderCommandQueue_SubmitScene | complete |
| 00B1E6D0 | BSP_RenderCommandQueue_ExecuteAndPopLast | complete |
| 00B1EF60 | BSP_RenderCommandQueue_DrawWorldLabels | complete |
| 00B3C960 | BSP_CockpitHelper_BeginFrame | complete |
| 00B3C6E0 | BSP_CockpitHelper_DrawCockpit | complete |
| 00B1BF40, 00B1ECA0, 00B1E770, 00B1DAD0, 00B1D830, 00B1CCD0, 00B1CA90, 00B1C610, 00B1FF70, 00B214E0, 00B215D0 | named only | listing |

- **SubmitDrawable** `(queue, scene, camera, targets, node)`, RET 10h.
  - It appends a new command (00B1F170). Growth is max(2·capacity, 1).
  - When node+48h & camera+19Ch, it calls node vtable +20h(command+28h, 0.0, 1.0, 0), then
    00B1E990.
  - A failed allocation appends null and then faults on null+28h.
- **SubmitScene** `(queue, scene, camera, targets, NativeString label by value)`, RET 14h.
  - The label goes to command+14h. 00B72190 traverses the whole scene into command+28h.
  - The callee frees the label argument.
  - Callers: the GUI order, the game frame and the sound/shadow paths.
- **ExecuteAndPopLast.** It runs the last command's vtable +0, rereads the slot, destroys
  (00B1DDD0) and frees the command, and decrements the count.
  - Unlike 00B1EBE0, it does not publish the queue's current context. 00B10500, 00B3C960 and
    00B3C6E0 use it right after a submit, to run that command immediately.
- **DrawWorldLabels** `(queue, camera)`, RET 4. It runs once per frame from the game render frame.
  - Each label is projected through the camera view-projection.
  - It prints only inside the clip volume. The compares are FCOMI/JB, so NaN coordinates skip.
  - Screen position is (x/w·0.5+0.5)·1024, (0.5-y/w·0.5)·768, truncated. The format is "%s\n".
  - The print is the bare-RET 004B47E0, so nothing appears.
  - The rows are then cleared.
  - The only producer, 00B1ECA0 AddWorldLabel (under the 00B1CCD0 lock), has no reference. The rows
    therefore stay empty in this image.
- **Renderer slots.** 00B214E0 (+7Ch) hands the back buffer to a target object. 00B215D0 (+84h)
  creates a system-memory offscreen surface wrapped in a BSP_D3D9Surface. 00B1FF70 is
  GetRenderTargetData (the luminance readback).
- **Unreferenced bodies.** The other 23 unnamed segment-83 entries have no rel32, absolute or Ghidra
  reference (local/sc2_rel32.py), except 00B20FD0, which only the unreferenced 00B23080 calls. They
  are left as COMDAT leftovers.
- **Cockpit.** service+0Ch is the helper (cockpit docs cover construction and lifetime only).
  Helper +8h is the node and +0Ch the cockpit camera. Either call drops a hidden node (release,
  clear +8h).
  - **BeginFrame** `(helper, main camera, targets)`, RET 8.
    - It copies the main camera's viewport size, fov, fog owner and aspect to the cockpit camera.
    - The node world translation becomes node − camera position (node vtable +34h).
    - The cockpit view is the main view with its translation zeroed.
    - It shows the node.
    - With both [00F8BBF0]+0Ch/+0Dh shadow switches on, the first scene light's shadow-map owner
      renders the node (00A8E180). The box is the cockpit camera position ± the node bound radius
      (vtable +44h result +0Ch). That command is then run.
  - **DrawCockpit** `(helper, main camera, targets)`, RET 8.
    - It shows the node and sets the cockpit camera to render mode 12.
    - It submits (main camera+A4h scene, cockpit camera, targets, node) and runs it.
    - It hides the node and resets its local matrix to identity (vtable +38h).
- **004DE4B0.** The ledger name says particle clock, but the evidence in section 4 shows the
  registry holds texture sources. The rename is left to its owning lane.
## 9. Handoff

- **Closed since the first handoff:**
  - Device vtable +114h is `BSP_D3D9Renderer_CommandHookNoOp` (00B20210, a bare `RET 4`). The
    words {0, 10h, 70h} that 00B156C0 passes, and command+1Ch that 00B17240 passes (the {0, 24h,
    5Ch} metadata 00B101C0 stores), reach no code in this image.
  - Service byte +B0h is zeroed by `BSP_RenderResources_Construct` (00B14CB8) and set to 1 by
    `BSP_RenderResources_InitializeMembers` (00B10912), next to +1C4h and +218h. A byte-store scan
    of the render range finds no other writer, so after initialization 00B101C0 always keeps the
    last submitted camera retained in +10h.
  - The +2ACh old-film parameter block is initialized by 00B0CD80
    (docs/NATIVE_RENDER_SERVICE_*.md) and consumed by 00B529A0 (docs/CC13_POST_EFFECTS_AND_RESOURCES.md).
  - Slot +28h of 00B19A60 and the +0Ch object: see sections 4 and 6.
- **Open:** 004DE4B0's ledger name says particle clock but the registry holds texture sources
  (owner lane to decide). The segment 83 queue helpers are covered in section 10.- **00BA0090** (BSP_OceanWaveField_Update, cc13-water1) also calls 00B0EB10 (its cCorner0..3 are the
  far-corner rays) and 00B0D160 (+21Ch). It belongs to the water lane.
