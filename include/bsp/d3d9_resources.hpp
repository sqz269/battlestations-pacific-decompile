#pragma once
#include "bsp/d3d9_startup.hpp"

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

// Owned COM resources created within 00b2aeb0. Engine wrapper attachment,
// resource registry and stream allocation bookkeeping are still separate work.
struct D3D9DynamicBuffers {
    IDirect3DVertexBuffer9* vertices{};
    IDirect3DIndexBuffer9* indices{};
};
HRESULT create_dynamic_buffers_00b2aeb0(IDirect3DDevice9& device, D3D9DynamicBuffers& buffers);
void release_dynamic_buffers(D3D9DynamicBuffers& buffers);
}
