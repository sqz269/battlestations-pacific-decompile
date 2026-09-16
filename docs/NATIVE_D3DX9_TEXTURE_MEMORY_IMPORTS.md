# Native D3DX9 texture-memory imports

`NativeD3dx9TextureMemoryImports` supplies the four concrete function-pointer
types already consumed by `NativeTextureLoadingContext`. It borrows one
caller-owned, live `d3dx9_40.dll` module and resolves these exact exports:

| Thunk | IAT | Export | Context field |
|---|---|---|---|
| `00C2DFEC` | `00CE23FC` | `D3DXGetImageInfoFromFileInMemory` | `image_info_00c2dfec` |
| `00C2DFE6` | `00CE2400` | `D3DXCreateTextureFromFileInMemoryEx` | `create_texture_00c2dfe6` |
| `00C2DFE0` | `00CE2404` | `D3DXCreateCubeTextureFromFileInMemory` | `create_cube_texture_00c2dfe0` |
| `00C2DFDA` | `00CE2408` | `D3DXCreateVolumeTextureFromFileInMemory` | `create_volume_texture_00c2dfda` |

The cube and volume entries come from the existing
`NativeD3dx9CubeVolumeMemoryImports` object. Its typed entry accessors expose the
same resolved functions directly, without a trampoline or global slot. The new
binder resolves the image-info and 2D-memory-Ex entries with the existing exact
typedefs in `d3d9_texture.hpp`; cube and volume use the existing context typedefs
in `native_texture_loading_cache.hpp`.

A null module or any missing named export throws during binder construction.
There is no `LoadLibrary` call, DLL search/version policy, callback injection,
device construction or renderer lookup. The caller must keep the supplied module
loaded while any returned entry is used. Calls go directly to D3DX, preserving
its HRESULT and output-pointer behavior.

The original executable import table and live Ghidra program agree on the four
names, thunks and IAT cells above. MSVC Win32 built the complete project and all
three existing CTests passed. A temporary manifest-embedded Win32 smoke borrowed
the installed `C:\Windows\SysWOW64\d3dx9_40.dll`, resolved all four entries and
called only image-info on retained bytes from the installed
`menu_dxt1_2.dds`. D3DX returned `S_OK`, 1024x1024, depth 1, one mip and
`D3DRTYPE_TEXTURE`.

Texture creation still requires a real D3D9 device and was not invoked by that
smoke. The production texture cache, renderer/device recreation graph, gameplay
behavior and visual parity remain outside this packet. Exact validation inputs,
hashes and command boundaries are in
`reports/native_d3dx9_texture_memory_imports.json`; the local smoke receipt is
ignored under `local/output/`.
