#include "bsp/d3d9_resources.hpp"
#include <cstddef>

namespace bsp {
D3D9DefaultSurfaces::~D3D9DefaultSurfaces() {
    surface_release(depth);
    surface_release(color);
}

void D3D9DefaultSurfaces::release_for_reset_00b262c0_fragment() {
    surface_release_for_reset_00b3d510(depth);
    surface_release_for_reset_00b3d510(color);
}

HRESULT D3D9DefaultSurfaces::restore_00b23b10_fragment(IDirect3DDevice9& device) {
    // Typed ownership preflight; native assumes release has emptied the owners.
    if (color.surface || depth.surface) return D3DERR_INVALIDCALL;
    IDirect3DSurface9* acquired{};
    HRESULT result = device.GetRenderTarget(0, &acquired);
    if (SUCCEEDED(result)) result = acquired
        ? surface_initialize_00b3cc80(color, acquired) : E_POINTER;
    if (acquired) acquired->Release();
    if (FAILED(result)) return result;
    acquired = nullptr;
    result = device.GetDepthStencilSurface(&acquired);
    if (SUCCEEDED(result)) result = acquired
        ? surface_initialize_00b3cc80(depth, acquired) : E_POINTER;
    if (acquired) acquired->Release();
    // Sequential ownership: a later depth failure leaves restored color alive.
    // Native ignores failures. No rollback or final depth bind occurs here.
    return result;
}

void D3D9SurfaceRegistry::append_00b2a7c0_fragment(D3D9SurfaceBinding& surface) {
    // Native pointer array grows separately and does not retain the wrapper.
    // std::vector replaces its allocator/capacity bookkeeping in this projection.
    surfaces_.push_back(&surface);
}

bool D3D9SurfaceRegistry::remove_00b25630(const D3D9SurfaceBinding* surface) noexcept {
    for (std::size_t index = 0; index < surfaces_.size(); ++index) {
        if (surfaces_[index] != surface) continue;
        if (index != surfaces_.size() - 1) surfaces_[index] = surfaces_.back();
        surfaces_.pop_back();
        return true;
    }
    return false;
}

void D3D9SurfaceRegistry::release_for_reset_00b262c0_fragment() {
    // Native +3Ch callbacks use a raw cursor; the typed supported domain requires
    // a stable list. This is not a mutation-safe callback traversal replacement.
    for (auto* surface : surfaces_) surface_release_for_reset_00b3d510(*surface);
}

HRESULT D3D9SurfaceRegistry::recreate_00b23b10_fragment(IDirect3DDevice9& device) {
    HRESULT first_failure = S_OK;
    for (auto* surface : surfaces_) {
        const auto result = surface_recreate_00b3d550(*surface, device);
        // Native ignores HRESULT and proceeds through the entire list. Report
        // the first failure without changing successful-path iteration order.
        if (FAILED(result) && SUCCEEDED(first_failure)) first_failure = result;
    }
    return first_failure;
}
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

void surface_release_for_reset_00b3d510(D3D9SurfaceBinding& binding) {
    if (binding.surface) {
        // Preserve the original balanced pair before dropping the owned reference.
        binding.surface->AddRef();
        binding.surface->Release();
    }
    if (binding.surface) {
        binding.surface->Release();
        binding.surface = nullptr;
    }
}

HRESULT surface_recreate_00b3d550(D3D9SurfaceBinding& binding, IDirect3DDevice9& device) {
    if (binding.surface) return D3DERR_INVALIDCALL; // New empty-owner precondition.
    if (binding.depth_stencil) {
        return device.CreateDepthStencilSurface(binding.width, binding.height,
            binding.format, binding.multisample, 0, TRUE, &binding.surface, nullptr);
    }
    return device.CreateRenderTarget(binding.width, binding.height, binding.format,
        binding.multisample, 0, FALSE, &binding.surface, nullptr);
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
