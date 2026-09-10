#include "bsp/d3d9_startup.hpp"
#include "bsp/native_renderer_parameters.hpp"
#include <cstddef>

namespace bsp {
static_assert(sizeof(D3DPRESENT_PARAMETERS) == 0x38);
static_assert(offsetof(D3DCAPS9, DevCaps) == 0x1c);
static_assert(offsetof(D3DCAPS9, VertexShaderVersion) == 0xc4);

HRESULT d3d9_create_device_prefix_00b2aeb0(
    IDirect3D9& api, const D3D9StartupOptions& options,
    NativeRendererParametersOwner& renderer_parameters,
    D3DPRESENT_PARAMETERS& stored_parameters, DWORD& behavior_flags,
    IDirect3DDevice9*& device) {
    if (device != nullptr) return D3DERR_INVALIDCALL;
    auto& parameters = stored_parameters;
    parameters = {};
    parameters.BackBufferWidth = options.width;
    parameters.BackBufferHeight = options.height;
    parameters.BackBufferFormat = options.backbuffer_format;
    parameters.BackBufferCount = options.backbuffer_count;
    parameters.MultiSampleType = options.multisample;
    parameters.MultiSampleQuality = 0;
    parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    parameters.hDeviceWindow = options.window;
    parameters.Windowed = TRUE; // 00b2af8e: even when fullscreen was requested.
    parameters.EnableAutoDepthStencil = options.depth_format != D3DFMT_UNKNOWN;
    parameters.AutoDepthStencilFormat = options.depth_format;
    parameters.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    parameters.FullScreen_RefreshRateInHz = options.fullscreen_refresh_rate;
    parameters.PresentationInterval = ~(options.presentation_sync << 31) & 0x80000000u;

    // 00B2AF9E/00B2AFA4: original arguments go to renderer+1A20/+1A24,
    // before GetDeviceCaps and CreateDevice can observe or mutate the owner.
    renderer_parameters.width_0c = options.width;
    renderer_parameters.height_10 = options.height;

    D3DCAPS9 caps{};
    behavior_flags = 0;
    HRESULT result = api.GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
    if (FAILED(result)) return result;
    // MOVZX word at caps+c4h: compare shader major/minor only.
    const bool hardware = (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0
        && (caps.VertexShaderVersion & 0xffffu) >= 0x101u;
    behavior_flags = D3DCREATE_MULTITHREADED | (hardware
        ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING);
    result = api.CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, options.window,
        behavior_flags, &parameters, &device);
    // 00b2b014 changes the stored structure after CreateDevice. It does not reset
    // the device here. Preserve API mutations of the other presentation fields.
    parameters.Windowed = !options.fullscreen;
    return result;
}
}
