#include "bsp/d3d9_texture.hpp"

namespace bsp {
HRESULT texture_create_from_retained_memory_00b3e190(IDirect3DDevice9& device,
    CreateTextureFromMemory create, const void* bytes, UINT size,
    const MemoryTextureOptions& options, IDirect3DTexture9*& output) {
    if (!create || output) return D3DERR_INVALIDCALL;
    // CMP +18h,14h; request A8R8G8B8 for R8G8B8, otherwise UNKNOWN.
    const auto format = options.source_format == D3DFMT_R8G8B8
        ? D3DFMT_A8R8G8B8 : D3DFMT_UNKNOWN;
    // Exact native stack at 00b3e1b4..00b3e1e5: managed pool, usage zero,
    // filter 70004h, mip filter FFFFFFFFh, zero color key and optional outputs.
    return create(&device, bytes, size, options.width, options.height,
        options.mip_levels, 0, format, D3DPOOL_MANAGED, 0x70004u, 0xffffffffu,
        0, nullptr, nullptr, &output);
}
}
