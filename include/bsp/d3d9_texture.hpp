#pragma once
#include "bsp/d3d9_startup.hpp"

namespace bsp {
// Opaque because this call always passes null for optional image information.
struct D3DXImageInfo;
using CreateTextureFromMemory = HRESULT (WINAPI *)(IDirect3DDevice9*, const void*, UINT,
    UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, DWORD, DWORD, D3DCOLOR,
    D3DXImageInfo*, PALETTEENTRY*, IDirect3DTexture9**);

struct MemoryTextureOptions {
    UINT width{};              // native object+28h
    UINT height{};             // +2Ch
    UINT mip_levels{};         // +3Ch
    D3DFORMAT source_format{}; // +18h
};

// Semantic port of 00b3e190: ECX=native wrapper, no stack arguments, RET.
// The caller supplies the device and retained file's bytes instead of native
// singleton/file-service traversal. The imported function/module must be live.
// Output must be empty; caller owns the returned COM reference. Native code
// ignores HRESULT; this interface returns it. It does not release an old texture.
HRESULT texture_create_from_retained_memory_00b3e190(IDirect3DDevice9&,
    CreateTextureFromMemory, const void* bytes, UINT size,
    const MemoryTextureOptions&, IDirect3DTexture9*& output);
}
