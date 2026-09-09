# First installed atlas image

The existing D3D9 probe now connects the reconstructed ATS line/item parsing,
memory-backed DDS creation and texture binding to a diagnostic quad renderer.
It reads the installed `menu_dxt1_2.ats` beside the optional DDS argument:

```powershell
./scripts/build.ps1
./build/win32/Release/bsp_d3d9_probe.exe 'I:/SteamLibrary/steamapps/common/Battlestations Pacific/interface/textures/menu_dxt1_2.dds'
```

All seven item names, float UV coordinates and packed words were compared with
the independently recorded descriptor evidence in `atlas_parser_evidence.json`.
The first item's quarter coordinate packs to 16383. Native constructor helper
`00467cf0` was checked: extension stripping uses the last dot and excludes index
zero. Missing native fields remain unsupported instead of acquiring invented
defaults. The outer parser uses explicit texture lookup and path/ownership
adapters; it is counted as a fragment while the native manager remains unported.

`00b24710` binds retained logical texture identities, maps slots 16..19 to
Direct3D vertex texture slots by adding F1h, skips identical logical pointers,
and explicitly unbinds null replacements. The C++ projection retains shared
logical records while borrowing COM pointers. The probe exercises pixel slot
zero, identity skipping and null unbinding with the installed DDS. Vertex
texture slot remapping is assembly-derived and has not been device-probed.

The 256x256 GPU readback is written to ignored `local/atlas_item.bmp`; inspection
shows the expected Japanese naval flag and Pacific map. `local/atlas_items.tsv`
holds the seven parsed records. The host uses point filtering, float UVs and a
fixed-function quad; the game's packed-UV consumer/material path is not ported.
No original asset or rendered asset image is committed to the repository.

An independent Pillow DDS decode of the same rectangle differs from GPU pixels
by up to RGB (1,6,1). This discrepancy is recorded, with cause unresolved; no
exact decoded-pixel parity or original-game render equivalence is claimed.
Compressed texture payload parity was already checked separately.

The shader bind wrappers `00b21d10` and `00b21c20` now preserve borrowed logical
identity. A distinct logical wrapper sharing the same COM shader skips binding
and leaves the original wrapper cached. They do not add native reference counts.
The existing probe compiles small diagnostic shaders using installed D3DCompiler,
checks real device bindings, identity skips, null unbinding, counters and balanced
locking, then restores the prior shaders. These are not recovered game shaders.

Validation: Win32 Release build and both existing CTest checks pass. The D3D9
probe passes its existing checks plus shader binding, DDS payload, ATS records
and atlas draw. `reports/atlas_render_validation.json` records the image hash
and the limits above. No new test suite was introduced.

Next dependencies are the native UI geometry/material consumer, shader/state
blocks and selective material constant generation. The constant investigation
identified a complete threshold-fade helper and bounded matrix packing; see
[MATERIAL_CONSTANTS.md](MATERIAL_CONSTANTS.md). The runnable game rebuild remains
incomplete.
