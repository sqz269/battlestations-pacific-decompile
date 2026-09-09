# Memory-backed D3D9 texture creation

`00b3e190` receives a texture wrapper in ECX and returns with plain RET.
It obtains the device through `00b1fef0` (renderer+1A10h), then uses the retained
file at wrapper+4Ch: virtual+30h supplies byte count and `00bef610` follows
file+8h -> +8h to the data. Output is wrapper+10h.

The native import is `D3DXCreateTextureFromFileInMemoryEx` from `d3dx9_40.dll`:
IAT `00ce2400`, thunk `00c2dfe6`. Assembly establishes these arguments:

| Argument | Native value |
|---|---|
| Width / height | Wrapper+28h / +2Ch |
| Mip levels | Wrapper+3Ch |
| Usage / pool | 0 / D3DPOOL_MANAGED |
| Format | A8R8G8B8 when wrapper+18h is R8G8B8; otherwise UNKNOWN |
| Filter / mip filter | 00070004h / FFFFFFFFh |
| Color key, image info, palette | Zero/null |

`src/d3d9_texture.cpp` reproduces this call with explicit device, byte span and
projected options. The caller supplies a live imported function and owns the
resulting COM reference. The new API rejects a nonempty output, exposes HRESULT,
and does not reconstruct the native file registry or singleton traversal.
The native body does not release an existing texture or inspect HRESULT.
Its surrounding ownership/reset policy remains unresolved.

The existing diagnostic probe now accepts an optional DDS path:

```powershell
./build/win32/Release/bsp_d3d9_probe.exe 'I:/SteamLibrary/steamapps/common/Battlestations Pacific/interface/textures/menu_dxt1_2.dds'
```

Its host adapter reads the file and resolves the installed 32-bit system
`d3dx9_40.dll`. No SDK redistributable or original asset is copied into the repo.
The diagnostic explicitly requests the one mip level identified in the inventory;
this does not establish native header-to-mip selection. It accepts the selected
single-level DXT1 shape, validates the real resource's descriptor and compares
every compressed row after LockRect against the original DDS payload.

Validation passed: Win32 Release build, both existing CTest checks, existing
D3D9 probe, and a 1024x1024 managed DXT1 texture whose payload matches the
installed file. The same probe now checks recovered vertex/pixel shader constant
uploads at registers 3/5, two float4 values each, by device readback; zero-count
calls skip uploads, counters remain one call/32 bytes each, and locking balances.
No new test suite was added. These are resource/register checks; the atlas image is now drawn by a diagnostic host (see ATLAS_RENDERING.md),
but has not been compared with the original game.

`reports/texture_memory_evidence.json` records original/Ghidra byte parity.
Next: implement the established atlas item parser and connect its rectangles to
texture binding, while the material path still needs shader/state blocks and
constant builder `00b42350`. See [ATLAS_PARSER.md](ATLAS_PARSER.md) and
[MATERIAL_EXECUTION.md](MATERIAL_EXECUTION.md).
