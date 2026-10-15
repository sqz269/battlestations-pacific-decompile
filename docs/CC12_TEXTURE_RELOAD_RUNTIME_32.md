# Independent texture reload runtime verification 32

The previously deferred Root comparison now passes, both with the frozen f083
worker family and with the combined Source build
`cdaf687819b79632f2d78713a696cb0b36f2aaa1`. Each fresh one-shot run executed
the existing complete relocated Original `B3FA90..B3FD7A` body and the existing
reconstructed Source reload against genuine application VFS, texture, renderer,
COM and ownership services. Both exited zero. The machine-readable receipt is
[`cc12_texture_reload_runtime_32.json`](../reports/cc12_texture_reload_runtime_32.json).

This closes the additional Root successful-path comparison deferred in
[`CC12_DEFERRED_RUNTIME_RECEIPTS_24.md`](CC12_DEFERRED_RUNTIME_RECEIPTS_24.md).
Earlier failed attempts remain unchanged historical evidence. The current
Source run also includes the normal startup shadow target and shared renderer
drain; it does not enable the separate font, GUI-effect or POST probes.

## Source and execution identities

| Case | Production inputs | Probe executable SHA-256 | Result |
| --- | --- | --- | --- |
| Root 05 | Frozen f083 family; 137 application objects and three libraries | `1a4d42c118f7e64c44f9e645d45e2c8c5f68b7014b2f510c8edca6395adc2010` | Exit 0; artifact comparison passed |
| Root 06 | Combined Source `cdaf687819b79632f2d78713a696cb0b36f2aaa1`; 138 application objects and three libraries | `65310c59bee54cf30789b6c71d7e723fb62e17414c5aa5a0eab4623c0951475f` | Exit 0; artifact comparison passed |

Root 06 replaces only that build's `game_main.obj` with a forwarding entry
compiled from the exact same commit's main and project headers. Its two edits
include the unchanged comparison fixture and forward startup through the
existing initialize-then-compare host. The 138 other application objects and
three complete libraries are unchanged, hash-checked combined-build inputs.
The existing build receipt records the Win32 build and three passing suites;
those suites are separate from the new native execution evidence.

Both runs use `tools/run_game.ps1`, fresh private copies of the accepted
windowed options, an 800x600 presentation override, and a two-iteration loop.
Each presents once and skips once. The launcher manifest's checkout HEAD is
launch metadata; the frozen input manifests establish compiled Source identity.
No production C++ body was edited or recompiled for these runs.

## Comparison and cleanup actually exercised

The current installed PE, live configured Ghidra program and frozen Original
body agree on all 747 bytes, SHA-256
`2040b2711596de753d0e87c2119184dc72ea843f173cc9fc7037efab5bee0b09`.
The installed executable remains SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Only 16 named direct-call operands and eight publication/import cells change
in the relocated execution: 96 operand bytes. All other bytes are checked.
Those bridges execute concrete Source children and real Win32/D3DX/COM
providers. Original child bodies and exception handling are not proved by them.

Each side cold-loads installed `black.tga` through the canonical cache; the
normal `error.tga` fallback is retained. The real image-info, creation and
level-zero descriptor providers each succeed once per side. The images are
32x32 DXT1 with six mip levels. Disjoint output frames begin with the existing
explicit A5/5A preimages. Actual image-info and descriptor output bytes agree.

All 80 owner bytes agree before and after reload after normalizing only four
independent allocation/serial words: name `+0C`, texture COM `+10`, serial `+20`
and retained source `+4C`. Each invocation separately preserves its canonical
companion, retained source identity/count and untouched native fields. Actual
cache removal, renderer unregister, final texture retirement, Source acquired
frame completion and replay rejection pass. Eight temporarily rebound numeric
virtual-table words and the real COM descriptor/provider hooks are restored.
The Original-side stream counters do not instrument the Source side, so their
different logged totals are not presented as a call-count comparison.

The normal singleton drain clears the renderer, Lua, definitions, system,
entry cache and queue publications and joins the worker. Device and API final
COM release counts are both zero. The current Source case constructs one
normal 2048 shadow target and closes its metadata after the same shared drain.

## Remaining activation work

This proves the successful real-provider 2D reload path on the two pinned
families. The renderer's `1B00` pointer array is empty, so nonempty removal,
failed HRESULTs, invalid inputs, provider exceptions, Original FH3/SEH and
drop-in binary ABI remain unproved. It does not establish full `B24FB0`
activation, cube/volume/effect reload, drawing, visual parity or gameplay.

Native Text activation still has concrete composition requirements. The
default `AB8CE0` shader selection reads signed height from platform `+28`.
Normal startup currently publishes its 44-byte HWND/focus receiver, whose
contract covers `+30/+41`; its opaque bytes are not valid configuration fields.
The existing complete 184-byte platform construction/deletion Source path has
not been activated by that host. Publication identity alone cannot admit the
focus receiver to Text's height read.

The persistent Text application also owns its buffer services privately and
does not yet bind them to the existing Text effect consumer. Actual host
activation must compose that binding, persistent native fonts and widget
owners, canonical shader-cache ordering, real buffers/pass contexts and
retirement. The runtime still reports 43 unimplemented host methods, including
Text buffer/section production. Startup fidelity and gameplay remain open.
