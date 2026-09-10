#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d9.h>

namespace bsp {
// Ten stack arguments of 00b2aeb0 (thiscall, RET 28h), in native order.
struct D3D9StartupOptions {
    HWND window{};
    bool fullscreen{};
    UINT width{};
    UINT height{};
    D3DFORMAT backbuffer_format{D3DFMT_A8R8G8B8};
    UINT backbuffer_count{1};
    D3DMULTISAMPLE_TYPE multisample{D3DMULTISAMPLE_NONE};
    D3DFORMAT depth_format{D3DFMT_D24S8};
    DWORD presentation_sync{1}; // Native uses the low bit, not a nonzero test.
    UINT fullscreen_refresh_rate{};
};

struct NativeRendererParametersOwner;

// Partial reconstruction: stops before timeBeginPeriod and renderer resource setup.
// Caller owns the returned COM reference. It must supply an empty output pointer.
// Unlike native code, returns API failures rather than dereferencing a failed device.
// Writes the supplied actual parameter region before either device callback;
// presentation mutations never recopy dimensions into that region afterward.
HRESULT d3d9_create_device_prefix_00b2aeb0(
    IDirect3D9& api, const D3D9StartupOptions& options,
    NativeRendererParametersOwner& renderer_parameters,
    D3DPRESENT_PARAMETERS& stored_parameters, DWORD& behavior_flags,
    IDirect3DDevice9*& device);
}
