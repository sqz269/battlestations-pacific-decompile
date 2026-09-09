#pragma once
#include "bsp/d3d9_resources.hpp"

namespace bsp {
struct D3D9CachedTextureLevel {
    UINT level{};                       // Native eight-byte record +0h.
    D3D9SurfaceBinding* binding{};       // Borrowed projection of record +4h.
};

// Concrete 2D texture reset projection, not a complete native texture wrapper.
// Adopt one COM reference in texture; level wrappers are borrowed, never deleted.
// Their lifetimes/list membership must remain stable throughout both phases.
struct D3D9ResetTexture2D {
    IDirect3DTexture9* texture{};        // Native +10h; owned reference.
    UINT mip_count{};                   // +14h.
    D3DFORMAT format{D3DFMT_UNKNOWN};    // +18h.
    DWORD flags{};                      // +1Ch.
    UINT width{}, height{};             // +28h/+2Ch.
    std::vector<D3D9CachedTextureLevel> levels; // +40h/+44h records/count.
    D3D9ResetTexture2D() = default;
    ~D3D9ResetTexture2D();
    D3D9ResetTexture2D(const D3D9ResetTexture2D&) = delete;
    D3D9ResetTexture2D& operator=(const D3D9ResetTexture2D&) = delete;
};

// Native ECX wrapper, RET, no guard. Preserve metadata/level owners; release
// cached level surfaces in order before dropping the texture COM reference.
// Typed null-entry/oversized-list failure happens before any release.
HRESULT release_texture_levels_00b3dd30(D3D9ResetTexture2D&);

// Native ECX wrapper, stack device, RET4, no guard. CreateTexture first, then
// GetSurfaceLevel and initialize each existing cached binding in order.
// Typed interface rejects invalid pool or nonempty level owners and reports
// COM errors, retaining any earlier successful phase writes without rollback.
// Native ignores HRESULT and has an uninitialized creation-output failure path.
HRESULT restore_texture_levels_00b3dd90(D3D9ResetTexture2D&, IDirect3DDevice9&);
}
