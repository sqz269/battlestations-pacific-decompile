# Image inspection and retained 2D loading

`load_retained_texture_2d_00b2c2d0_fragment` composes the recovered image-info,
initial dimension/mip policy, D3DX creation, actual texture metadata and source
retention into a reusable function. Both the existing DDS check and installed
font GFX loading now use it. Physical-file conversion supplies a complete shared
memory stream beforehand; the function uses its base and length independently
of its cursor. It does not reopen the file or clone/reset that stream wrapper.

The containing native loader takes stack name/callback arguments and ends with
RET 8. The relevant 2D constructor `00b3f930` uses ECX wrapper, five stack
arguments and RET 14h. The new HRESULT/unique_ptr interface projects only the
successful 2D route. Native VFS selection, optional renderer guard, device retry,
callback, manager cache and allocator/intrusive object ABI remain outside it.

## Actual metadata correction

Fresh constructor assembly establishes that recreation metadata comes from
`IDirect3DTexture9::GetLevelDesc(0)`, not directly from source image information:

| Native field | Value |
|---|---|
| `+18` | Actual level-zero format (`00b3f9ff`) |
| `+28/+2C` | Actual level-zero width/height (`00b3f9c8/00b3f9cb`) |
| `+34/+38` | Separate saved policy dimensions from constructor arguments |
| `+3C` | Requested mip count, assigned by the loader at `00b2c62f` |

The previous typed initializer seeded recreation width/height/format from the
source and policy. This worked for the checked DDS but could differ after D3DX
conversion. Initialization now reads actual level metadata, while retaining
the requested mip count. The initial request still converts source R8G8B8 to
A8R8G8B8 and otherwise requests UNKNOWN. Recreation applies the same native
comparison to the stored actual format. `MemoryTextureOptions::format` therefore
means the current call's input format, not an immutable source-file format.

The complete 337-byte constructor `00b3f930..00b3fa81` (exclusive end) matches
the saved Ghidra image and installed PE: SHA-256
`c879d9f611054c5f3f805b7fece0f6cee524d84818e1edd87e773d5f10a72117`.
The image-info call/gate and source/mip assignment were separately checked;
see `reports/font_geometry_texture_audit.json`.

## Explicit host boundaries

The implementation uses the official SDK `_D3DXIMAGE_INFO` tag and function
pointer ABI. D3DX9_40 remains a supplied live import; the library does not own
the module. Output must be empty and the stream fully initialized. Non-2D
resource types are rejected. Unlike the native code, failed image-info and
level-description HRESULTs stop this projection. A failed level description
releases the new COM texture instead of retaining unusable metadata. A nonnull
COM output with a failed creation HRESULT still proceeds through metadata and
source retention; the caller receives the creation HRESULT and owned output.
Allocation exceptions propagate and temporary ownership cleans up on unwind.

`GetType`, diagnostic balanced AddRef/Release pairs, separate saved-size fields
and the rest of native constructor bookkeeping are not reconstructed by the
metadata read. This is neither the full constructor nor a drop-in replacement.
See [TEXTURE_LOAD_POLICY.md](TEXTURE_LOAD_POLICY.md) for exact name gates and
quality arithmetic, and [TEXTURE_STREAM_LIFETIME.md](TEXTURE_STREAM_LIFETIME.md)
for native ownership and reload distinctions.

## Validation

The Win32 build, both existing CTests and full D3D9 probe pass. The installed
1024x1024 DXT1 atlas still matches its complete compressed payload after source
retention and recreation. The actual Fonts.lua selects Arial16 and its
`Fonts/arial18.tga`: independent file-header inspection confirms 512x256, 32-bit
TGA. Real D3DX creates managed A8R8G8B8 with one mip; actual metadata, retained
source lifetime and recreation dimensions/format are checked after closing the
file and dropping the local stream owner. Texture ownership then releases the
last retained stream.

The TGA SHA-256 is
`1f88a76ca802aedd1015d5ba7fe0af6d8a700b2501c3c7320f85564cc9a7fba4`.
This check does not exercise the R8G8B8 conversion branch, resource resizing,
nonzero mip reduction, font shader/material binding, glyph drawing or gameplay.
No new test target was introduced. Current output is recorded in
`reports/font_geometry_texture_probe.txt`; the earlier default-policy report
remains historical evidence for the direct initializer.
