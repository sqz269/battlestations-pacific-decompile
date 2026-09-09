// Diagnostic host window only; this is not the reconstructed game window handler.
#include "bsp/d3d9_startup.hpp"
#include "bsp/d3d9_states.hpp"
#include "bsp/d3d9_resources.hpp"
#include "bsp/d3d9_buffers.hpp"
#include "bsp/d3d9_vertex_layout.hpp"
#include "bsp/win32_window.hpp"
#include <cstring>
#include <cstdio>
#include "bsp/physical_file.hpp"
#include <vector>
#include <limits>
#include "bsp/d3d9_texture.hpp"


bool probe_shader_bindings(IDirect3DDevice9&, const char*);
bool probe_material_states_and_constants(IDirect3DDevice9&);
bool probe_texture_atlas(IDirect3DDevice9&, IDirect3DTexture9&, const char*);

// Recovered physical read route with diagnostic size/short-read checks and DLL
// import adapter. This is not the native VFS/memory-wrapper implementation.
// The optional input is the single-level DXT1 atlas identified in ASSET_ENTRY.md.
static bool probe_memory_texture(IDirect3DDevice9& device, const char* path) {
    bsp::PhysicalFile file;
    DWORD error{};
    if (!file.open_read_only_00bf52a0_fragment(path, error) || !file.valid_00bf5020()) return false;
    const auto end = file.size_00bf4f90();
    if (end < 128 || end > (std::numeric_limits<UINT>::max)()) return false;
    std::vector<char> bytes(static_cast<std::size_t>(end));
    std::uint32_t actual{};
    if (!file.seek_00bf4f20(0, FILE_BEGIN, error)
        || !file.read_00bf5030(bytes.data(), static_cast<std::uint32_t>(bytes.size()), actual, error)
        || actual != bytes.size() || file.position() != end
        || !file.close_00bf5090_fragment(error) || file.valid_00bf5020()) return false;
    std::printf("Physical asset read: bytes=%u cached_position_matches_size=1 closed=1\n", actual);
    auto word = [&](std::size_t offset) {
        DWORD value{};
        std::memcpy(&value, bytes.data() + offset, sizeof(value));
        return value;
    };
    const UINT width = word(16), height = word(12);
    if (word(0) != 0x20534444 || word(4) != 124 || word(84) != D3DFMT_DXT1
        || !width || !height || width % 4 || height % 4) return false;
    const std::size_t row_bytes = static_cast<std::size_t>(width / 4) * 8;
    if (static_cast<std::uint64_t>(bytes.size() - 128)
        != static_cast<std::uint64_t>(row_bytes) * (height / 4)) return false;
    HMODULE module = LoadLibraryExW(L"d3dx9_40.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module) return false;
    FARPROC address = GetProcAddress(module, "D3DXCreateTextureFromFileInMemoryEx");
    bsp::CreateTextureFromMemory create{};
    static_assert(sizeof(create) == sizeof(address));
    std::memcpy(&create, &address, sizeof(create));
    IDirect3DTexture9* texture = nullptr;
    const bsp::MemoryTextureOptions options{width, height, 1, D3DFMT_DXT1};
    HRESULT result = bsp::texture_create_from_retained_memory_00b3e190(device, create,
        bytes.data(), static_cast<UINT>(bytes.size()), options, texture);
    D3DSURFACE_DESC description{};
    if (SUCCEEDED(result) && !texture) result = E_FAIL;
    if (SUCCEEDED(result)) result = texture->GetLevelDesc(0, &description);
    bool matched = SUCCEEDED(result) && description.Width == width
        && description.Height == height && description.Format == D3DFMT_DXT1
        && description.Pool == D3DPOOL_MANAGED && description.Usage == 0
        && texture->GetLevelCount() == 1;
    D3DLOCKED_RECT locked{};
    if (matched) {
        result = texture->LockRect(0, &locked, nullptr, D3DLOCK_READONLY);
        matched = SUCCEEDED(result);
        if (matched) {
            matched = locked.Pitch >= 0 && static_cast<std::size_t>(locked.Pitch) >= row_bytes;
            for (UINT row = 0; matched && row < height / 4; ++row)
                matched = std::memcmp(static_cast<const char*>(locked.pBits)
                    + static_cast<std::size_t>(row) * locked.Pitch,
                    bytes.data() + 128 + row * row_bytes, row_bytes) == 0;
            const HRESULT unlock = texture->UnlockRect(0);
            matched = matched && SUCCEEDED(unlock);
        }
    }
    std::printf("D3D9 installed DDS: hr=0x%08lx size=%ux%u managed_DXT1_bytes_match=%d\n",
        static_cast<unsigned long>(result), description.Width, description.Height, matched);
    if (matched) matched = probe_texture_atlas(device, *texture, path);
    if (texture) texture->Release();
    FreeLibrary(module);
    return matched;
}

static bool probe_shader_constants(IDirect3DDevice9& device) {
    auto* lock = bsp::critical_section_create_00bd1860();
    bsp::RendererSynchronization sync{};
    bsp::set_renderer_synchronization_00b33aa0(sync, true);
    bool matched = false;
    {
        bsp::D3D9StateCache state(device, sync, lock);
        const float values[8]{1, -2, 0.25f, 4, 5, 6, -7, 8};
        float vertex[8]{}, pixel[8]{};
        HRESULT result = state.set_vertex_shader_constants_f_00b21820(3, values, 2);
        if (SUCCEEDED(result)) result = state.set_pixel_shader_constants_f_00b218c0(5, values, 2);
        if (SUCCEEDED(result)) result = device.GetVertexShaderConstantF(3, vertex, 2);
        if (SUCCEEDED(result)) result = device.GetPixelShaderConstantF(5, pixel, 2);
        const bool skipped = state.set_vertex_shader_constants_f_00b21820(0, nullptr, 0) == S_FALSE
            && state.set_pixel_shader_constants_f_00b218c0(0, nullptr, 0) == S_FALSE;
        matched = SUCCEEDED(result) && skipped && std::memcmp(values, vertex, sizeof(values)) == 0
            && std::memcmp(values, pixel, sizeof(values)) == 0
            && state.vertex_constant_calls() == 1 && state.pixel_constant_calls() == 1
            && state.vertex_constant_bytes() == 32 && state.pixel_constant_bytes() == 32
            && sync.nesting == 0 && lock->depth == 0;
        std::printf("D3D9 shader constants: hr=0x%08lx VS_PS_readback_and_zero_skip=%d\n",
            static_cast<unsigned long>(result), matched);
    }
    bsp::critical_section_destroy_owned_0041cc80(lock);
    return matched;
}

// Host setup for a bounded pixel check, not the game's material/stream pipeline.
static bool probe_draw(IDirect3DDevice9& device) {
    IDirect3DSurface9* target = nullptr;
    IDirect3DSurface9* readback = nullptr;
    IDirect3DSurface9* original = nullptr;
    auto physical_vertices = std::make_shared<bsp::VertexBufferBinding>();
    auto& vertices = *physical_vertices;
    vertices.flags = 0x1000;
    vertices.capacity = 80;
    HRESULT result = device.GetRenderTarget(0, &original);
    if (SUCCEEDED(result)) result = device.CreateRenderTarget(64, 64, D3DFMT_A8R8G8B8,
        D3DMULTISAMPLE_NONE, 0, FALSE, &target, nullptr);
    if (SUCCEEDED(result)) result = device.CreateOffscreenPlainSurface(64, 64, D3DFMT_A8R8G8B8,
        D3DPOOL_SYSTEMMEM, &readback, nullptr);
    if (SUCCEEDED(result)) result = bsp::vertex_buffer_recreate_00b492b0(vertices, device);
    if (SUCCEEDED(result)) result = device.SetRenderTarget(0, target);
    bsp::RendererSynchronization sync{};
    bsp::D3D9StateCache states(device, sync, nullptr);
    if (SUCCEEDED(result)) result = device.SetDepthStencilSurface(nullptr);
    auto stream = std::make_shared<bsp::LogicalVertexStream>();
    stream->physical = physical_vertices;
    stream->declaration = std::make_shared<bsp::VertexDeclaration>();
    stream->declaration->append_00b48330(D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITIONT);
    stream->declaration->append_00b48330(D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR);
    if (stream->declaration->stride != 20
        || !stream->declaration->contains_00b47c90(D3DDECLUSAGE_COLOR, 0)
        || stream->declaration->find_00b47ce0(D3DDECLUSAGE_COLOR, 0) != 1
        || stream->declaration->offset_00b47c40(D3DDECLUSAGE_COLOR, 0) != 16
        || stream->declaration->size_00b47c60(D3DDECLUSAGE_COLOR, 0) != 4) result = E_FAIL;
    stream->flags = 0x1000;
    stream->tag = 0x40000001;
    void* mapped = nullptr;
    if (SUCCEEDED(result)) result = states.lock_vertex_stream_00b49980(*stream, 4, 0, true, mapped);
    if (SUCCEEDED(result)) {
        struct Vertex { float x, y, z, rhw; DWORD diffuse; };
        static_assert(sizeof(Vertex) == 20);
        const Vertex triangle[] = {{-100, -100, 0, 1, 0xffff0000}, {4, 4, 0, 1, 0xff00ff00},
            {60, 4, 0, 1, 0xff00ff00}, {4, 60, 0, 1, 0xff00ff00}};
        std::memcpy(mapped, triangle, sizeof(triangle));
        states.unlock_vertex_stream_00b49a80(*stream);
        if (stream->mapped || stream->vertex_count != 4 || stream->offset != 0
            || vertices.cursor != 80 || vertices.lock_depth != 0) result = E_FAIL;
    }
    if (SUCCEEDED(result)) {
        states.bind_vertex_stream_00b24840(0, stream);
        auto equivalent = std::make_shared<bsp::LogicalVertexStream>(*stream);
        states.bind_vertex_stream_00b24840(0, equivalent);
        states.bind_vertex_stream_00b24840(0, stream);
    }
    bsp::D3D9VertexLayout layout;
    layout.append_stream_00b48a00(stream->declaration);
    if (SUCCEEDED(result)) result = layout.create_if_missing_00b60a10(device);
    D3DVERTEXELEMENT9 declaration_elements[3]{};
    UINT declaration_count = 3;
    if (SUCCEEDED(result)) result = layout.native()->GetDeclaration(declaration_elements, &declaration_count);
    if (SUCCEEDED(result) && (declaration_count != 3 || layout.stride() != 20
        || declaration_elements[0].Type != D3DDECLTYPE_FLOAT4
        || declaration_elements[0].Usage != D3DDECLUSAGE_POSITIONT
        || declaration_elements[1].Offset != 16 || declaration_elements[1].Usage != D3DDECLUSAGE_COLOR
        || declaration_elements[2].Stream != 0xff)) result = E_FAIL;
    if (SUCCEEDED(result)) result = device.SetVertexDeclaration(layout.native());
    std::printf("D3D9 vertex layout: hr=0x%08lx elements=%u stride=%u\n",
        static_cast<unsigned long>(result), declaration_count, layout.stride());
    if (SUCCEEDED(result)) result = device.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    if (SUCCEEDED(result)) result = device.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    if (SUCCEEDED(result)) {
        states.initialize_defaults_00b26170();
        states.set_stream_frequency_00b24a40(0, 1);
        states.set_render_state_00b24460(D3DRS_ZENABLE, FALSE);
        states.set_render_state_00b24460(D3DRS_CULLMODE, D3DCULL_NONE);
        states.set_render_state_00b24460(D3DRS_LIGHTING, FALSE);
        result = device.Clear(0, nullptr, D3DCLEAR_TARGET, 0xff000000, 1, 0);
    }
    if (SUCCEEDED(result)) result = device.BeginScene();
    if (SUCCEEDED(result)) {
        result = states.draw_primitive_00b21b40({}, D3DPT_TRIANGLELIST, 1, 1);
        const HRESULT ended = device.EndScene();
        if (SUCCEEDED(result)) result = ended;
    }
    if (SUCCEEDED(result)) result = device.GetRenderTargetData(target, readback);
    D3DLOCKED_RECT pixels{};
    if (SUCCEEDED(result)) result = readback->LockRect(&pixels, nullptr, D3DLOCK_READONLY);
    DWORD inside{}, outside{};
    if (SUCCEEDED(result)) {
        const auto* bytes = static_cast<const unsigned char*>(pixels.pBits);
        std::memcpy(&inside, bytes + 16 * pixels.Pitch + 16 * 4, 4);
        std::memcpy(&outside, bytes + 60 * pixels.Pitch + 60 * 4, 4);
        result = readback->UnlockRect();
    }
    bool matched = SUCCEEDED(result) && (inside & 0xffffff) == 0x00ff00
        && (outside & 0xffffff) == 0;
    std::printf("D3D9 draw readback: hr=0x%08lx inside=0x%08lx outside=0x%08lx checked=%d\n",
        static_cast<unsigned long>(result), inside, outside, matched);
    auto index_stream = std::make_shared<bsp::LogicalIndexStream>();
    index_stream->physical = std::make_shared<bsp::IndexBufferBinding>();
    auto& indices = *index_stream->physical;
    indices.flags = 0x1000;
    indices.capacity = 6;
    index_stream->index_count = 3;
    if (matched) result = bsp::index_buffer_recreate_00b49180(indices, device);
    if (matched && SUCCEEDED(result)) result = states.lock_index_stream_00b49b60(*index_stream, 0, 0, false, mapped);
    if (matched && SUCCEEDED(result)) {
        const unsigned short elements[] = {0, 1, 2};
        std::memcpy(mapped, elements, sizeof(elements));
        states.unlock_index_stream_00b49c70(*index_stream);
        states.bind_index_stream_00b24b00(index_stream, 0);
        states.bind_index_stream_00b24b00(index_stream, 1); // Same object, new base vertex.
        result = device.Clear(0, nullptr, D3DCLEAR_TARGET, 0xff000000, 1, 0);
    }
    if (matched && SUCCEEDED(result)) result = device.BeginScene();
    if (matched && SUCCEEDED(result)) {
        const HRESULT rejected = states.draw_indexed_00b24010({}, D3DPT_TRIANGLELIST, 0, 5, 0, 1);
        result = rejected == S_FALSE
            ? states.draw_indexed_00b24010({}, D3DPT_TRIANGLELIST, 0, 3, 0, 1) : E_FAIL;
        const HRESULT ended = device.EndScene();
        if (SUCCEEDED(result)) result = ended;
    }
    if (matched && SUCCEEDED(result)) result = device.GetRenderTargetData(target, readback);
    if (matched && SUCCEEDED(result)) result = readback->LockRect(&pixels, nullptr, D3DLOCK_READONLY);
    if (matched && SUCCEEDED(result)) {
        const auto* bytes = static_cast<const unsigned char*>(pixels.pBits);
        std::memcpy(&inside, bytes + 16 * pixels.Pitch + 16 * 4, 4);
        std::memcpy(&outside, bytes + 60 * pixels.Pitch + 60 * 4, 4);
        result = readback->UnlockRect();
    }
    matched = matched && SUCCEEDED(result) && (inside & 0xffffff) == 0xff00
        && (outside & 0xffffff) == 0 && states.vertex_binding_calls() == 1
        && states.index_binding_calls() == 1;
    std::printf("D3D9 indexed readback: hr=0x%08lx inside=0x%08lx vertex_binds=%u index_binds=%u checked=%d\n",
        static_cast<unsigned long>(result), inside, states.vertex_binding_calls(), states.index_binding_calls(), matched);
    states.bind_vertex_stream_00b24840(0, nullptr);
    states.bind_index_stream_00b24b00(nullptr, 0);
    // Explicit registry lifecycle until native stream constructors/destructors are ported.
    states.register_logical_stream_00b4b1e0(vertices, *stream);
    states.register_logical_stream_00b4b1e0(vertices, *stream); // Duplicate suppressed.
    states.register_logical_stream_00b4b1e0(indices, *index_stream);
    states.rewind_vertex_buffer_00b232b0(vertices);
    states.rewind_index_buffer_00b231c0(indices);
    matched = matched && vertices.logical_streams.size() == 1
        && stream->offset == 0xffffffff && index_stream->offset == 0xffffffff
        && vertices.cursor == 0 && indices.cursor == 0
        && vertices.dynamic_locks == 0 && indices.dynamic_locks == 0
        && vertices.lock_depth == 0 && indices.lock_depth == 0;
    if (matched) result = states.lock_vertex_stream_00b49980(*stream, 4, 0, false, mapped);
    if (matched && SUCCEEDED(result)) {
        struct Vertex { float x, y, z, rhw; DWORD diffuse; };
        const Vertex triangle[] = {{-100, -100, 0, 1, 0xffff0000}, {4, 4, 0, 1, 0xff0000ff},
            {60, 4, 0, 1, 0xff0000ff}, {4, 60, 0, 1, 0xff0000ff}};
        std::memcpy(mapped, triangle, sizeof(triangle));
        states.unlock_vertex_stream_00b49a80(*stream);
        result = states.lock_index_stream_00b49b60(*index_stream, 0, 0, false, mapped);
        if (SUCCEEDED(result)) {
            const unsigned short elements[] = {0, 1, 2};
            std::memcpy(mapped, elements, sizeof(elements));
            states.unlock_index_stream_00b49c70(*index_stream);
        }
    }
    if (matched && SUCCEEDED(result)) {
        states.bind_vertex_stream_00b24840(0, stream);
        states.bind_index_stream_00b24b00(index_stream, 1);
        result = device.Clear(0, nullptr, D3DCLEAR_TARGET, 0xff000000, 1, 0);
    }
    if (matched && SUCCEEDED(result)) result = device.BeginScene();
    if (matched && SUCCEEDED(result)) {
        result = states.draw_indexed_00b24010({}, D3DPT_TRIANGLELIST, 0, 3, 0, 1);
        const HRESULT ended = device.EndScene();
        if (SUCCEEDED(result)) result = ended;
    }
    if (matched && SUCCEEDED(result)) result = device.GetRenderTargetData(target, readback);
    if (matched && SUCCEEDED(result)) result = readback->LockRect(&pixels, nullptr, D3DLOCK_READONLY);
    if (matched && SUCCEEDED(result)) {
        const auto* bytes = static_cast<const unsigned char*>(pixels.pBits);
        std::memcpy(&inside, bytes + 16 * pixels.Pitch + 16 * 4, 4);
        std::memcpy(&outside, bytes + 60 * pixels.Pitch + 60 * 4, 4);
        result = readback->UnlockRect();
    }
    matched = matched && SUCCEEDED(result) && (inside & 0xffffff) == 0xff
        && (outside & 0xffffff) == 0 && stream->offset == 0 && index_stream->offset == 0
        && vertices.cursor == 80 && indices.cursor == 6
        && vertices.dynamic_locks == 1 && indices.dynamic_locks == 1;
    states.bind_vertex_stream_00b24840(0, nullptr);
    states.bind_index_stream_00b24b00(nullptr, 0);
    states.unregister_vertex_stream_00b4b3f0(vertices, *stream);
    states.unregister_index_stream_00b4b390(indices, *index_stream);
    matched = matched && vertices.logical_streams.empty() && indices.logical_streams.empty();
    std::printf("D3D9 buffer rewind: hr=0x%08lx reused_pixel=0x%08lx checked=%d\n",
        static_cast<unsigned long>(result), inside, matched);
    device.SetVertexDeclaration(nullptr);
    if (original) { device.SetRenderTarget(0, original); original->Release(); }
    bsp::buffer_release(vertices);
    if (readback) readback->Release();
    if (target) target->Release();
    return matched;
}

#ifdef BSP_HAS_GUI_REFERENCE
bool probe_gui_geometry_reference();
#endif
#ifdef BSP_HAS_CAMERA_REFERENCE
bool probe_camera_reference();
#endif
int main(int argc, char** argv) {
#ifdef BSP_HAS_CAMERA_REFERENCE
    if (!probe_camera_reference()) return 1;
#endif
#ifdef BSP_HAS_GUI_REFERENCE
    if (!probe_gui_geometry_reference()) return 1;
#endif
    const HINSTANCE instance = GetModuleHandleA(nullptr);
    const char* name = "BSP D3D9 reconstruction probe";
    HWND window{};
    ATOM window_class{};
    DWORD window_error{};
    const bsp::PlatformWindowOptions window_options{
        instance, name, DefWindowProcA, nullptr, false, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480};
    if (!bsp::create_platform_window_00becee0_fragment(window_options, window, window_class, window_error)) {
        if (window) DestroyWindow(window);
        if (window_class) UnregisterClassA(name, instance);
        return 1;
    }
    RECT outer{}, expected{0, 0, 640, 480};
    const bool window_checked = GetWindowRect(window, &outer)
        && AdjustWindowRect(&expected, WS_CAPTION, FALSE)
        && outer.left == 0 && outer.top == 0
        && outer.right - outer.left == expected.right - expected.left
        && outer.bottom - outer.top == expected.bottom - expected.top
        && GetClassLongA(window, GCL_CBWNDEXTRA) == 24
        && (GetClassLongA(window, GCL_STYLE) & CS_GLOBALCLASS) != 0
        && (GetWindowLongA(window, GWL_STYLE) & WS_CAPTION) == WS_CAPTION
        && (GetWindowLongA(window, GWL_EXSTYLE) & 0x300) == 0x300
        && !IsWindowVisible(window);
    std::printf("Recovered window creation: class_extra_style_geometry_hidden=%d\n", window_checked);
    if (!window_checked) {
        DestroyWindow(window); UnregisterClassA(name, instance); return 1;
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
            const bool initially_idle = !bsp::renderer_device_lifecycle_busy_00b20220(*lock);
            EnterCriticalSection(&lock->native);
            ++lock->depth;
            const bool observed_busy = bsp::renderer_device_lifecycle_busy_00b20220(*lock);
            --lock->depth;
            LeaveCriticalSection(&lock->native);
            const bool lifecycle_gate = initially_idle && observed_busy
                && !bsp::renderer_device_lifecycle_busy_00b20220(*lock);
            bsp::RendererSynchronization synchronization{};
            bsp::set_renderer_synchronization_00b33aa0(synchronization, true);
            bsp::D3D9StateCache cache(*device, synchronization, lock);
            cache.initialize_defaults_00b26170();
            cache.initialize_defaults_00b26170();
            DWORD zfunc{}, cull{}, min_filter{}, vertex_filter{};
            matched = lifecycle_gate && SUCCEEDED(device->GetRenderState(D3DRS_ZFUNC, &zfunc))
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
    if (matched) {
        bsp::D3D9DefaultSurfaces surfaces;
        auto& binding = surfaces.color;
        bsp::RendererSynchronization synchronization{};
        bsp::set_renderer_synchronization_00b33aa0(synchronization, true);
        auto* lock = bsp::critical_section_create_00bd1860();
        const auto release_lock = [](bsp::TrackedCriticalSection* value) {
            bsp::critical_section_destroy_owned_0041cc80(value);
        };
        std::unique_ptr<bsp::TrackedCriticalSection, decltype(release_lock)> lock_owner(lock, release_lock);
        bsp::D3D9StateCache cache(*device, synchronization, lock);
        bsp::D3D9DynamicBuffers buffers{};
        result = cache.capture_default_surfaces_00b238d0_fragment(surfaces);
        // Reacquisition exercises replacement ownership of the same COM surfaces.
        if (SUCCEEDED(result)) result = cache.capture_default_surfaces_00b238d0_fragment(surfaces);
        if (SUCCEEDED(result)) result = cache.bind_depth_surface_00b21690(nullptr);
        const bsp::D3D9SurfaceBinding empty_wrapper{};
        if (SUCCEEDED(result)) result = cache.bind_depth_surface_00b21690(&empty_wrapper);
        if (SUCCEEDED(result)) result = cache.bind_depth_surface_00b21690(&surfaces.depth);
        IDirect3DSurface9* bound_depth{};
        if (SUCCEEDED(result)) result = device->GetDepthStencilSurface(&bound_depth);
        const bool capture_checked = SUCCEEDED(result) && bound_depth == surfaces.depth.surface
            && surfaces.depth.format == D3DFMT_D24S8 && !surfaces.depth.depth_stencil
            && !surfaces.color.depth_stencil && surfaces.color.wrapper_flags == 0
            && surfaces.depth.wrapper_flags == 0 && cache.depth_binding_calls() == 4
            && lock->depth == 0 && synchronization.nesting == 0;
        if (bound_depth) bound_depth->Release();
        std::printf("Default surfaces: retained_color_depth_kind_zero_and_depth_bind_count=%d\n", capture_checked);
        D3DSURFACE_DESC retained_surface{};
        if (SUCCEEDED(result)) result = binding.surface->GetDesc(&retained_surface);
        if (SUCCEEDED(result)) result = bsp::create_dynamic_buffers_00b2aeb0(*device, buffers);
        D3DVERTEXBUFFER_DESC vertices{};
        D3DINDEXBUFFER_DESC indices{};
        if (SUCCEEDED(result)) result = buffers.vertices->GetDesc(&vertices);
        if (SUCCEEDED(result)) result = buffers.indices->GetDesc(&indices);
        matched = capture_checked && SUCCEEDED(result) && retained_surface.Width == binding.width
            && retained_surface.Height == binding.height && binding.width == 640 && binding.height == 480
            && binding.format == D3DFMT_A8R8G8B8 && vertices.Size == 0x1000000
            && indices.Size == 0x100000 && indices.Format == D3DFMT_INDEX16
            && vertices.Usage == 0x208 && indices.Usage == 0x208
            && vertices.Pool == D3DPOOL_DEFAULT && indices.Pool == D3DPOOL_DEFAULT;
        std::printf("D3D9 resources: hr=0x%08lx surface=%ux%u vertex_bytes=%u "
            "index_bytes=%u checked=%d\n", static_cast<unsigned long>(result),
            binding.width, binding.height, vertices.Size, indices.Size, matched);
        bsp::release_dynamic_buffers(buffers);
    }
    if (swap_chain) { swap_chain->Release(); swap_chain = nullptr; }
    if (matched) {
        // Exercise the recovered surface methods with registered-style offscreen
        // resources. The host orchestrates Reset, not the incomplete game reset loop.
        bsp::D3D9SurfaceBinding color{}, depth{};
        color.format = D3DFMT_A8R8G8B8;
        color.width = depth.width = 128;
        color.height = depth.height = 128;
        depth.format = D3DFMT_D24S8;
        depth.depth_stencil = true;
        bsp::D3D9SurfaceRegistry registry;
        registry.append_00b2a7c0_fragment(color);
        registry.append_00b2a7c0_fragment(depth);
        result = registry.recreate_00b23b10_fragment(*device);
        registry.release_for_reset_00b262c0_fragment();
        if (SUCCEEDED(result)) result = device->Reset(&stored);
        if (SUCCEEDED(result)) result = registry.recreate_00b23b10_fragment(*device);
        D3DSURFACE_DESC color_desc{}, depth_desc{};
        if (SUCCEEDED(result)) result = color.surface->GetDesc(&color_desc);
        if (SUCCEEDED(result)) result = depth.surface->GetDesc(&depth_desc);
        matched = SUCCEEDED(result) && color_desc.Width == 128 && color_desc.Height == 128
            && depth_desc.Width == 128 && depth_desc.Height == 128
            && color_desc.Format == D3DFMT_A8R8G8B8 && depth_desc.Format == D3DFMT_D24S8
            && color_desc.Usage == D3DUSAGE_RENDERTARGET
            && depth_desc.Usage == D3DUSAGE_DEPTHSTENCIL;
        std::printf("D3D9 surface reset: hr=0x%08lx color_usage=%lu depth_usage=%lu checked=%d\n",
            static_cast<unsigned long>(result), color_desc.Usage, depth_desc.Usage, matched);
        const bool removal = registry.remove_00b25630(&color) && registry.size() == 1
            && registry.at(0) == &depth && !registry.remove_00b25630(&color)
            && color.surface && depth.surface && registry.remove_00b25630(&depth)
            && registry.size() == 0;
        matched = matched && removal;
        std::printf("Surface reset registry: swap_last_remove_without_surface_release=%d\n", removal);
        bsp::surface_release_for_reset_00b3d510(color);
        bsp::surface_release_for_reset_00b3d510(depth);
    }
    if (swap_chain) swap_chain->Release();
    if (matched) {
        bsp::VertexBufferBinding vertices{};
        bsp::IndexBufferBinding indices{};
        vertices.flags = indices.flags = 0x1000;
        vertices.capacity = 0x1000000;
        indices.capacity = 0x100000;
        result = bsp::vertex_buffer_recreate_00b492b0(vertices, *device);
        if (SUCCEEDED(result)) result = bsp::index_buffer_recreate_00b49180(indices, *device);
        for (UINT pass = 0; pass < 2 && SUCCEEDED(result); ++pass) {
            bsp::BufferLockResult vertex_lock{}, index_lock{};
            result = bsp::vertex_buffer_lock_00b4ba00(vertices, 48, 0, false, vertex_lock);
            if (SUCCEEDED(result)) {
                std::memset(vertex_lock.data, 0, 48);
                bsp::vertex_buffer_unlock_00b4b9d0(vertices);
                if (vertex_lock.base_offset != pass * 48 || vertex_lock.flags
                    != static_cast<DWORD>(pass == 0 ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE)) result = E_FAIL;
            }
            if (SUCCEEDED(result)) result = bsp::index_buffer_lock_00b4b850(indices, 6, 0, false, index_lock);
            if (SUCCEEDED(result)) {
                const unsigned short triangle[] = {0, 1, 2};
                std::memcpy(index_lock.data, triangle, sizeof(triangle));
                bsp::index_buffer_unlock_00b4b820(indices);
                if (index_lock.base_offset != pass * 6 || index_lock.flags
                    != static_cast<DWORD>(pass == 0 ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE)) result = E_FAIL;
            }
        }
        matched = SUCCEEDED(result) && vertices.cursor == 96 && indices.cursor == 12
            && vertices.lock_depth == 0 && indices.lock_depth == 0
            && vertices.dynamic_locks == 2 && indices.dynamic_locks == 2;
        std::printf("D3D9 buffer uploads: hr=0x%08lx vertex_cursor=%u index_cursor=%u checked=%d\n",
            static_cast<unsigned long>(result), vertices.cursor, indices.cursor, matched);
        bsp::buffer_release(indices);
        bsp::buffer_release(vertices);
    }
    if (matched) matched = probe_draw(*device);
    if (matched) matched = probe_shader_constants(*device);
    if (matched && argc > 1) matched = probe_shader_bindings(*device, argv[1]);
    else if (matched) std::puts("Shader asset probe skipped: supply installed atlas DDS path to locate game scripts.");
    if (matched) matched = probe_material_states_and_constants(*device);
    if (matched && argc > 1) matched = probe_memory_texture(*device, argv[1]);
    if (device) device->Release();
    if (api) api->Release();
    DestroyWindow(window);
    UnregisterClassA(name, instance);
    return matched ? 0 : 1;
}
