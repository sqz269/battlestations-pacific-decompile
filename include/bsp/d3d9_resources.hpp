#pragma once
#include "bsp/d3d9_startup.hpp"
#include <vector>

namespace bsp {
// Projection of surface wrapper fields +18h..2ch. Caller owns the retained COM
// reference. This is the initialization helper, not a replacement/assignment API.
struct D3D9SurfaceBinding {
    D3DFORMAT format{};
    UINT width{};
    UINT height{};
    D3DMULTISAMPLE_TYPE multisample{};
    DWORD wrapper_flags{}; // Native +28h: preserved on non-null initialization.
    IDirect3DSurface9* surface{};
    bool depth_stencil{}; // Native +30h selects recreation API; not inferred from format.
};
HRESULT surface_initialize_00b3cc80(D3D9SurfaceBinding& binding, IDirect3DSurface9* surface);
void surface_release(D3D9SurfaceBinding& binding);
void surface_release_for_reset_00b3d510(D3D9SurfaceBinding& binding);
HRESULT surface_recreate_00b3d550(D3D9SurfaceBinding& binding, IDirect3DDevice9& device);

// Unique typed COM owners for renderer+197Ch/+198Ch. Full native wrapper
// construction, intrusive references and registry participation are not modeled.
// Reacquire defaults after reset; do not recreate them using generic surfaces.
struct D3D9DefaultSurfaces {
    D3D9SurfaceBinding color;
    D3D9SurfaceBinding depth;
    D3D9DefaultSurfaces() = default;
    ~D3D9DefaultSurfaces();
    D3D9DefaultSurfaces(const D3D9DefaultSurfaces&) = delete;
    D3D9DefaultSurfaces& operator=(const D3D9DefaultSurfaces&) = delete;
    // Default-owner fragments only: no readiness/lost gates, other three color
    // slots, guard, resource/listener callbacks or binding-cache operations.
    void release_for_reset_00b262c0_fragment();
    // Reinitializes these existing bindings, color before depth, without binding
    // depth to the device. Both COM owners must be empty after release.
    HRESULT restore_00b23b10_fragment(IDirect3DDevice9& device);
};

// Borrowed reset-list projection of renderer+1B0Ch/+1B10h/+1B14h.
// Owners must unregister before destruction; registry does not AddRef/Release.
// No list mutation/reallocation or owner destruction during reset traversal.
class D3D9SurfaceRegistry {
public:
    // Explicit factory append fragment, not the full native surface factory.
    void append_00b2a7c0_fragment(D3D9SurfaceBinding& surface);
    // Native ECX=array, stack pointer-to-target-pointer, AL found, RET4.
    // Removes first match, replacing it with the final entry; no stable ordering.
    bool remove_00b25630(const D3D9SurfaceBinding* surface) noexcept;
    std::size_t size() const noexcept { return surfaces_.size(); }
    const D3D9SurfaceBinding* at(std::size_t index) const { return surfaces_.at(index); }
    // Concrete wrapper callback projections only. Full reset gates, default
    // owners, other resource lists and listener notifications are not included.
    void release_for_reset_00b262c0_fragment();
    HRESULT recreate_00b23b10_fragment(IDirect3DDevice9& device);
private:
    std::vector<D3D9SurfaceBinding*> surfaces_;
};

// Owned COM resources created within 00b2aeb0. Engine wrapper attachment,
// resource registry and stream allocation bookkeeping are still separate work.
struct D3D9DynamicBuffers {
    IDirect3DVertexBuffer9* vertices{};
    IDirect3DIndexBuffer9* indices{};
};
HRESULT create_dynamic_buffers_00b2aeb0(IDirect3DDevice9& device, D3D9DynamicBuffers& buffers);
void release_dynamic_buffers(D3D9DynamicBuffers& buffers);
}
