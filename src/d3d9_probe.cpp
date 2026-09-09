// Diagnostic host window only; this is not the reconstructed game window handler.
#include "bsp/d3d9_startup.hpp"
#include "bsp/d3d9_states.hpp"
#include <cstdio>

int main() {
    const HINSTANCE instance = GetModuleHandleA(nullptr);
    const char* name = "BSP D3D9 reconstruction probe";
    WNDCLASSA window_class{};
    window_class.lpfnWndProc = DefWindowProcA;
    window_class.hInstance = instance;
    window_class.lpszClassName = name;
    if (!RegisterClassA(&window_class)) return 1;
    const HWND window = CreateWindowExA(0, name, name, WS_CAPTION,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, instance, nullptr);
    if (!window) {
        UnregisterClassA(name, instance);
        return 1;
    }
    // SDK 32 matches Direct3DCreate9(20h) in renderer constructor 00b32410.
    IDirect3D9* api = Direct3DCreate9(D3D_SDK_VERSION);
    IDirect3DDevice9* device = nullptr;
    IDirect3DSwapChain9* swap_chain = nullptr;
    D3DPRESENT_PARAMETERS stored{}, actual{};
    DWORD flags{};
    HRESULT result = E_FAIL;
    if (api) {
        bsp::D3D9StartupOptions options{};
        options.window = window;
        options.width = 640;
        options.height = 480;
        result = bsp::d3d9_create_device_prefix_00b2aeb0(
            *api, options, stored, flags, device);
        if (SUCCEEDED(result)) result = device->GetSwapChain(0, &swap_chain);
        if (SUCCEEDED(result)) result = swap_chain->GetPresentParameters(&actual);
    }
    std::printf("D3D9 device probe: hr=0x%08lx flags=0x%lx size=%ux%u "
        "windowed=%d format=%u depth=%u interval=0x%x\n",
        static_cast<unsigned long>(result), flags, actual.BackBufferWidth,
        actual.BackBufferHeight, actual.Windowed,
        static_cast<unsigned>(actual.BackBufferFormat),
        static_cast<unsigned>(actual.AutoDepthStencilFormat), actual.PresentationInterval);
    bool matched = SUCCEEDED(result) && actual.BackBufferWidth == 640
        && actual.BackBufferHeight == 480 && actual.Windowed
        && actual.BackBufferFormat == D3DFMT_A8R8G8B8
        && actual.EnableAutoDepthStencil && actual.AutoDepthStencilFormat == D3DFMT_D24S8
        && actual.PresentationInterval == D3DPRESENT_INTERVAL_DEFAULT;
    if (matched) {
        auto* lock = bsp::critical_section_create_00bd1860();
        if (!lock) matched = false;
        else {
            bsp::RendererSynchronization synchronization{true, false, 0};
            bsp::D3D9StateCache cache(*device, synchronization, lock);
            cache.initialize_defaults_00b26170();
            cache.initialize_defaults_00b26170();
            DWORD zfunc{}, cull{}, min_filter{}, vertex_filter{};
            matched = SUCCEEDED(device->GetRenderState(D3DRS_ZFUNC, &zfunc))
                && SUCCEEDED(device->GetRenderState(D3DRS_CULLMODE, &cull))
                && SUCCEEDED(device->GetSamplerState(0, D3DSAMP_MINFILTER, &min_filter))
                && SUCCEEDED(device->GetSamplerState(D3DVERTEXTEXTURESAMPLER0,
                    D3DSAMP_MINFILTER, &vertex_filter))
                && zfunc == D3DCMP_LESS && cull == D3DCULL_CCW
                && min_filter == D3DTEXF_LINEAR && vertex_filter == D3DTEXF_LINEAR
                && cache.render_calls() == 19 && cache.sampler_calls() == 140
                && lock->depth == 0 && synchronization.nesting == 0
                && synchronization.observed_enabled;
            std::printf("D3D9 defaults: render_calls=%u sampler_calls=%u zfunc=%lu "
                "cull=%lu min_filter=%lu vertex_filter=%lu balanced=%d checked=%d\n",
                cache.render_calls(), cache.sampler_calls(), zfunc, cull, min_filter,
                vertex_filter, lock->depth == 0 && synchronization.nesting == 0, matched);
        }
        bsp::critical_section_destroy_owned_0041cc80(lock);
    }
    if (swap_chain) swap_chain->Release();
    if (device) device->Release();
    if (api) api->Release();
    DestroyWindow(window);
    UnregisterClassA(name, instance);
    return matched ? 0 : 1;
}
