#include "bsp/d3d9_resources.hpp"
#include <cstddef>

namespace bsp {
static_assert(offsetof(D3DSURFACE_DESC, MultiSampleType) == 0x10);
static_assert(offsetof(D3DSURFACE_DESC, Width) == 0x18);
static_assert(offsetof(D3DSURFACE_DESC, Height) == 0x1c);

HRESULT surface_initialize_00b3cc80(D3D9SurfaceBinding& binding, IDirect3DSurface9* surface) {
    // Native overwrites without releasing: only call on an empty binding.
    if (binding.surface) return D3DERR_INVALIDCALL;
    binding.surface = surface;
    if (!surface) {
        binding.format = D3DFMT_UNKNOWN;
        binding.width = binding.height = binding.wrapper_flags = 0;
        binding.multisample = D3DMULTISAMPLE_NONE;
        return S_OK;
    }
    surface->AddRef();
    D3DSURFACE_DESC description{};
    const HRESULT result = surface->GetDesc(&description);
    if (FAILED(result)) {
        // New interface failure cleanup; native ignores HRESULT.
        surface->Release();
        binding.surface = nullptr;
        return result;
    }
    binding.format = description.Format;
    binding.width = description.Width;
    binding.height = description.Height;
    binding.multisample = description.MultiSampleType;
    return result;
}

void surface_release(D3D9SurfaceBinding& binding) {
    // New owner cleanup, not the full native surface wrapper destructor.
    if (binding.surface) binding.surface->Release();
    binding = {};
}

HRESULT create_dynamic_buffers_00b2aeb0(IDirect3DDevice9& device, D3D9DynamicBuffers& buffers) {
    if (buffers.vertices || buffers.indices) return D3DERR_INVALIDCALL;
    constexpr DWORD usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
    static_assert(usage == 0x208);
    HRESULT result = device.CreateVertexBuffer(0x1000000, usage, 0, D3DPOOL_DEFAULT,
        &buffers.vertices, nullptr);
    if (FAILED(result)) return result;
    result = device.CreateIndexBuffer(0x100000, usage, D3DFMT_INDEX16, D3DPOOL_DEFAULT,
        &buffers.indices, nullptr);
    if (FAILED(result)) release_dynamic_buffers(buffers);
    return result;
}

void release_dynamic_buffers(D3D9DynamicBuffers& buffers) {
    if (buffers.indices) buffers.indices->Release();
    if (buffers.vertices) buffers.vertices->Release();
    buffers = {};
}
}
