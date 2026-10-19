# oct18b mission ship milestone A-C

`BSP_NATIVE_MISSION_SHIP=1` opts into a retained diagnostic composition. The
default is OFF. Preparation starts on the first mission frame, before native
BeginFrame; the ordinary retained command executes before the existing overlay
and screenshot callback. The installed model is
`models/ships/us/fletcher.mmod` (SHA-256
`a6d4d1bac3d7e13f01f3ab474d5c2f445f42dfb04cb95facc8d8f7257470d211`).
When ON and `--screenshot <path>` is supplied, the existing capture callback
also retains mission frames 1, 60 and 180 with `_N001`, `_N060` and `_N180`
inserted before the filename extension. All three use the same run and normal
back-buffer capture after the ordinary command and HUD overlay.

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

## Verification

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

ON attempt 07 (`local/oct18b_render_on_attempt07.log` and manifest) completes
the real 73BF80 preload with 179 entries, B107F0 resources and the 26 native
Fletcher models/445 sections. B1F4D0 submits one queue command with batch
entry counts 119 and 12. B1EBE0/B1D950 executes 131 B24010 calls, all returned,
on every rendered mission view. It reaches 600 application frames and 599
successful presents. Mission landmarks show simulation times 0.05, 2.999998
and 9.000016 seconds at mission frames 1, 60 and 180 respectively.

The same run retains `local/oct18b_render_on_probe07_N001.png`,
`local/oct18b_render_on_probe07_N060.png` and
`local/oct18b_render_on_probe07_N180.png`. Visual inspection shows a Fletcher
silhouette, too dark to establish recognizable texture. Native scene lighting
is being composed to supply the installed ship shader's required ambient cube
and sun. The **textured ship bar is not yet reached**. Singleton drain also
fast-fails after camera/foliage retirement; an ON-only terminate dump diagnoses
the exact boundary without unwinding or repairing native owners.

Earlier failures and fixes are retained in the local attempt04-06 logs/dumps:
the native cache is forward-only, so complete 73BF80 preloading must precede
B107F0; B24B00 index binding also requires the original D61DE0 table pointer.
No cursor reset, fabricated shader row or substitute source compiler is used.
See `OCT18B_RENDER_PRELOAD.md` and `OCT18B_RENDER_MODEL.md` for exact evidence.
D onward still requires mission object associations and the main scene/multipass
submission, followed by terrain/ocean/sky/particles.
