#include "bsp/d3d9_reset_texture.hpp"
#include <climits>

namespace bsp {
namespace {
bool valid_levels(const D3D9ResetTexture2D& owner, bool require_empty) {
    if (owner.levels.size() > INT_MAX) return false;
    for (const auto& level : owner.levels)
        if (!level.binding || (require_empty && level.binding->surface)) return false;
    return true;
}

bool decode_texture_creation(DWORD flags, DWORD& usage, D3DPOOL& pool) {
    const DWORD pool_value = flags & 0xf;
    if (pool_value > 3) return false; // Native malformed path uses device as pool.
    pool = static_cast<D3DPOOL>(pool_value);
    usage = (flags & 0x10) ? D3DUSAGE_RENDERTARGET : 0;
    switch (flags & 0xf00) {
    case 0x100: usage |= 2; break;
    case 0x200: usage |= 0x4000; break;
    case 0x300: usage |= 0x40; break;
    case 0x400: usage |= 0x100; break;
    case 0x500: usage |= 0x80; break;
    }
    if ((flags & 0xf000) == 0x1000) usage |= 0x200;
    if ((flags & 0xff000000) == 0x1000000) usage |= 0x400;
    return true;
}
}

D3D9ResetTexture2D::~D3D9ResetTexture2D() {
    if (texture) texture->Release(); // New typed cleanup, not native full destructor.
}

HRESULT release_texture_levels_00b3dd30(D3D9ResetTexture2D& owner) {
    if (!valid_levels(owner, false)) return D3DERR_INVALIDCALL;
    for (const auto& level : owner.levels)
        surface_release_for_reset_00b3d510(*level.binding);
    IDirect3DTexture9* observed = owner.texture;
    if (observed) {
        observed->AddRef();
        observed->Release();
    }
    if (owner.texture) {
        owner.texture->Release();
        owner.texture = nullptr;
    }
    return S_OK;
}

HRESULT restore_texture_levels_00b3dd90(D3D9ResetTexture2D& owner, IDirect3DDevice9& device) {
    DWORD usage{};
    D3DPOOL pool{};
    if (!valid_levels(owner, true) || !decode_texture_creation(owner.flags, usage, pool))
        return D3DERR_INVALIDCALL;
    IDirect3DTexture9* created = nullptr;
    HRESULT result = device.CreateTexture(owner.width, owner.height, owner.mip_count,
        usage, owner.format, pool, &created, nullptr);
    if (FAILED(result) || !created) {
        if (created) created->Release();
        return FAILED(result) ? result : E_POINTER;
    }
    IDirect3DTexture9* previous = owner.texture;
    if (previous != created) {
        owner.texture = created;
        created->AddRef();
        if (previous) previous->Release();
    }
    created->Release(); // Drop temporary creation reference, including identity case.
    for (const auto& level : owner.levels) {
        IDirect3DSurface9* acquired = nullptr;
        result = owner.texture->GetSurfaceLevel(level.level, &acquired);
        if (SUCCEEDED(result)) result = acquired
            ? surface_initialize_00b3cc80(*level.binding, acquired) : E_POINTER;
        if (acquired) acquired->Release();
        if (FAILED(result)) return result;
    }
    return result;
}
}
