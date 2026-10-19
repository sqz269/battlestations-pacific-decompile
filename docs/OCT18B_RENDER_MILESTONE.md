# oct18b mission ship milestone A-C

`BSP_NATIVE_MISSION_SHIP=1` opts into a retained diagnostic composition. The
default is OFF. Preparation starts on the first mission frame, before native
BeginFrame; the ordinary retained command executes before the existing overlay
and screenshot callback. The installed model is
`models/ships/us/fletcher.mmod` (SHA-256
`a6d4d1bac3d7e13f01f3ab474d5c2f445f42dfb04cb95facc8d8f7257470d211`).

The composition reuses the existing renderer, original VFS/material/shader and
geometry providers, frame-clock publication and native queue. It adds the
actual B14A10/B107F0 resource owner, D5E5E0/B1D950 ordinary domain, B71A80
Operator camera with viewport/fog, a D5D220 foliage base required by gathering,
and the installed MMOD scene. See the bounded evidence in
`OCT18B_RENDER_A1.md`, `OCT18B_RENDER_SCENE.md`, `OCT18B_RENDER_A2.md` and
`OCT18B_RENDER_MODEL.md`.

Camera position, light/fog values and foliage scalar/normal are explicit
diagnostic choices. They are not claimed as recovered mission settings.
Reconstructed entrypoints retain original-address evidence and source-domain
ownership contracts; these C++ interfaces are not drop-in original ABI hooks.
Failure after entering native ownership retains the graph until process exit.

## Verification before shader-preload integration

`scripts/build.ps1` passed MSVC Win32 compilation and both existing CTests.
The frozen reference is main `e3d55e84c980e5a788b55d77fd0dc8aa8a9bd6f1`,
retained under `local/oct18b_render_reference/`. Both USN04 runs use 181
mission frames at 0.05 seconds and the same 1280x720/vsync options.

`tools/pair_diff.py local/oct18b_render_pair_baseline.log
local/oct18b_render_final_off_usn04.log --json
local/oct18b_render_final_off_pair_diff.json --limit 3` exited **0**:
21 identical units, 1349 identical native rows, 665 identical summary rows,
and no other unmasked differences. Earlier matched captures also had identical
PNG bytes (SHA-256
`0405bf03536f2598626219705ba3de8b2b232b6922bd8cd56bef84fc4c688f1d`).

ON attempt 04 reaches the original resource-initializer material compiler but
faults at executable RVA `00354d54`, reading `row+8` from a null compiled-shader
cache result. Its evidence is retained in
`local/oct18b_render_on_attempt04.log`, its manifest and
`local/oct18b_render_on_attempt04.dmp`. The original cache lookup advances a
forward-only cursor; the original 73BF80 material preload is not yet composed
at this checkpoint. There is no successful ordinary draw or ON screenshot at
this checkpoint, so the recognizable textured ship bar is **not reached**.

The next integration is the existing real 73BF80 preload before B107F0, with
the same material and cache providers. This is not permission to reset cache
cursors, fabricate shader rows or substitute a source compiler. D onward
still requires mission object associations and the main scene/multipass
submission, followed by terrain/ocean/sky/particles.
