// Diagnostic atlas host: parser, texture binding and image readback. This is
// not the game's UI material or window implementation.
#include "bsp/texture_atlas.hpp"
#include "bsp/gui_texture.hpp"
#include "bsp/gui_geometry.hpp"
#include <cmath>
#include "bsp/d3d9_states.hpp"
#include <filesystem>
#include <fstream>
#include <iterator>
#include <cstdio>
#include <cstring>

bool probe_texture_atlas(IDirect3DDevice9& device, IDirect3DTexture9& texture,
    const char* dds_path) {
    std::filesystem::path descriptor(dds_path);
    descriptor.replace_extension(".ats");
    std::ifstream input(descriptor, std::ios::binary);
    if (!input) return false;
    const std::string text{std::istreambuf_iterator<char>(input), {}};
    UINT lookups = 0;
    const auto atlas = bsp::parse_texture_atlas_00aeeaf0(text, descriptor.generic_string(),
        [&](std::string_view path, std::uint32_t flags) -> void* {
            ++lookups;
            const std::string name(path);
            return flags == 0 && _stricmp(name.c_str(), dds_path) == 0 ? &texture : nullptr;
        });
    if (atlas.status != bsp::TextureAtlasParseStatus::success || lookups != 1
        || atlas.texture != &texture || atlas.items.size() != 7) {
        std::printf("Atlas parse failed: %s items=%zu lookups=%u\n",
            atlas.detail.c_str(), atlas.items.size(), lookups);
        return false;
    }
    std::filesystem::create_directories("local");
    std::ofstream records("local/atlas_items.tsv");
    records.precision(9);
    for (const auto& record : atlas.items) {
        records << record.name;
        for (float value : record.uv) records << '\t' << value;
        for (auto value : record.packed_uv) records << '\t' << value;
        records << '\n';
    }
    records.flush();
    if (!records.good()) return false;
    const auto& item = atlas.items.front();
    const std::array<std::uint16_t, 6> packed{0, 0, 16383, 16383, 16383, 16383};
    if (item.name != "interface/textures/fe/achievement/ca_of" || item.packed_uv != packed)
        return false;
    unsigned retained = 0, dimensions = 0, fallbacks = 0;
    bsp::GuiTextureCallbacks callbacks;
    callbacks.find_atlas_item = [&](std::string_view name) {
        const std::string terminated(name);
        return bsp::find_texture_atlas_item_00aefb20(atlas.items, terminated.c_str());
    };
    callbacks.load_texture = [&](std::string_view, std::uint32_t flags) -> void* {
        if (flags == 0) ++fallbacks;
        return nullptr; // Diagnostic miss; native loader is an external dependency.
    };
    callbacks.width = [&](void* value) {
        ++dimensions;
        D3DSURFACE_DESC desc{};
        static_cast<IDirect3DTexture9*>(value)->GetLevelDesc(0, &desc);
        return desc.Width;
    };
    callbacks.height = [&](void* value) {
        ++dimensions;
        D3DSURFACE_DESC desc{};
        static_cast<IDirect3DTexture9*>(value)->GetLevelDesc(0, &desc);
        return desc.Height;
    };
    callbacks.retain = [&](void* value) {
        ++retained;
        static_cast<IDirect3DTexture9*>(value)->AddRef();
    };
    std::array<float, 4> resolved_uv{0,0,1,1};
    std::array<float, 2> logical_size{};
    auto* selected = bsp::resolve_gui_texture_00aa2660(
        "///INTERFACE\\TEXTURES\\FE\\ACHIEVEMENT\\CA_OF.TGA", resolved_uv, logical_size, 1, callbacks);
    if (selected) static_cast<IDirect3DTexture9*>(selected)->Release();
    if (selected != &texture || resolved_uv != item.uv || retained != 1 || dimensions != 2
        || logical_size[0] != static_cast<float>(256.0 / 960.0)
        || logical_size[1] != static_cast<float>(256.0 / 720.0)) return false;
    std::array<float, 4> mirrored{1,1,0,0};
    std::array<float, 2> fixed_size{1,0};
    selected = bsp::resolve_gui_texture_00aa2660("unrelated/path/ca_of.tga", mirrored, fixed_size, 1, callbacks);
    if (selected) static_cast<IDirect3DTexture9*>(selected)->Release();
    const std::array<float, 4> expected_mirror{item.uv[2],item.uv[3],item.uv[0],item.uv[1]};
    if (selected != &texture || mirrored != expected_mirror || dimensions != 2 || retained != 2) return false;
    const auto saved_uv = mirrored;
    selected = bsp::resolve_gui_texture_00aa2660("missing_fixture.tga", mirrored, fixed_size, 1, callbacks);
    if (selected || mirrored != saved_uv || fixed_size != std::array<float,2>{1,0}
        || retained != 2 || fallbacks != 1) return false;
    std::printf("GUI atlas resolver: size=%.9g,%.9g flip_fallback_reference_checked=1\n",
        logical_size[0], logical_size[1]);
    IDirect3DSurface9 *target = nullptr, *readback = nullptr, *previous = nullptr;
    IDirect3DStateBlock9* previous_state = nullptr;
    HRESULT result = device.CreateStateBlock(D3DSBT_ALL, &previous_state);
    if (SUCCEEDED(result)) result = device.GetRenderTarget(0, &previous);
    if (SUCCEEDED(result)) result = device.CreateRenderTarget(256, 256, D3DFMT_A8R8G8B8,
        D3DMULTISAMPLE_NONE, 0, FALSE, &target, nullptr);
    if (SUCCEEDED(result)) result = device.CreateOffscreenPlainSurface(256, 256,
        D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &readback, nullptr);
    if (SUCCEEDED(result)) result = device.SetRenderTarget(0, target);
    bsp::RendererSynchronization sync{};
    bsp::D3D9StateCache state(device, sync, nullptr);
    auto logical = std::make_shared<bsp::LogicalTexture>();
    logical->texture = &texture;
    if (SUCCEEDED(result)) result = state.bind_texture_00b24710(0, logical);
    if (SUCCEEDED(result) && state.bind_texture_00b24710(0, logical) != S_FALSE) result = E_FAIL;
    IDirect3DBaseTexture9* bound = nullptr;
    if (SUCCEEDED(result)) result = device.GetTexture(0, &bound);
    if (SUCCEEDED(result) && bound != &texture) result = E_FAIL;
    if (bound) bound->Release();
    if (SUCCEEDED(result)) result = device.SetVertexShader(nullptr);
    if (SUCCEEDED(result)) result = device.SetPixelShader(nullptr);

    if (SUCCEEDED(result)) result = device.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    if (SUCCEEDED(result)) result = device.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    if (SUCCEEDED(result)) result = device.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    if (SUCCEEDED(result)) result = device.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    if (SUCCEEDED(result)) result = device.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    state.set_render_state_00b24460(D3DRS_ZENABLE, FALSE);
    state.set_render_state_00b24460(D3DRS_ALPHABLENDENABLE, FALSE);
    state.set_render_state_00b24460(D3DRS_ALPHATESTENABLE, FALSE);
    state.set_render_state_00b24460(D3DRS_CULLMODE, D3DCULL_NONE);
    state.set_sampler_state_00b24610(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    state.set_sampler_state_00b24610(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    struct Vertex { float x, y, z, rhw, u, v; };
    bsp::GuiQuadParameters geometry;
    geometry.width = logical_size[0]; geometry.height = logical_size[1];
    geometry.y_scale = 0.75f; // Observed native global, supplied explicitly.
    geometry.uv = {resolved_uv[0], resolved_uv[1], resolved_uv[2], resolved_uv[3]};
    std::array<bsp::GuiQuadVertex, 4> generated{};
    if (!bsp::gui_write_cropped_quad_00ab1860(geometry, generated)) result = E_FAIL;
    Vertex quad[4]{};
    // Diagnostic logical-to-pixel projection; native geometry supplies corners.
    for (std::size_t i = 0; i < generated.size(); ++i) {
        const auto& v = generated[i];
        quad[i] = {v.x * 960.0f - 0.5f, v.y * 960.0f - 0.5f, v.z, 1, v.u, v.v};
    }
    auto physical = std::make_shared<bsp::VertexBufferBinding>();
    physical->flags = 0x1000; physical->capacity = sizeof(quad);
    auto stream = std::make_shared<bsp::LogicalVertexStream>();
    stream->physical = physical; stream->flags = 0x1000; stream->tag = 0x40000001;
    stream->declaration = std::make_shared<bsp::VertexDeclaration>();
    stream->declaration->append_00b48330(D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITIONT);
    stream->declaration->append_00b48330(D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD);
    auto layout = std::make_shared<bsp::D3D9VertexLayout>();
    layout->append_stream_00b48a00(stream->declaration);
    if (SUCCEEDED(result)) result = bsp::vertex_buffer_recreate_00b492b0(*physical, device);
    if (SUCCEEDED(result)) result = layout->create_if_missing_00b60a10(device);
    void* mapped = nullptr;
    if (SUCCEEDED(result)) result = state.lock_vertex_stream_00b49980(*stream, 4, 0, true, mapped);
    if (SUCCEEDED(result)) {
        std::memcpy(mapped, quad, sizeof(quad));
        state.unlock_vertex_stream_00b49a80(*stream);
        state.bind_vertex_stream_00b24840(0, stream);
        result = state.bind_vertex_layout_00b23f20(layout);
        if (SUCCEEDED(result) && state.bind_vertex_layout_00b23f20(layout) != S_FALSE) result = E_FAIL;
    }
    if (SUCCEEDED(result)) result = device.Clear(0, nullptr, D3DCLEAR_TARGET, 0xffff00ff, 1, 0);
    if (SUCCEEDED(result)) result = device.BeginScene();
    if (SUCCEEDED(result)) {
        result = state.draw_primitive_00b21b40({}, D3DPT_TRIANGLESTRIP, 0, 2);
        const HRESULT ended = device.EndScene();
        if (SUCCEEDED(result)) result = ended;
    }
    if (SUCCEEDED(result)) result = device.GetRenderTargetData(target, readback);
    D3DLOCKED_RECT pixels{};
    if (SUCCEEDED(result)) result = readback->LockRect(&pixels, nullptr, D3DLOCK_READONLY);
    bool written = false;
    if (SUCCEEDED(result)) {
        BITMAPFILEHEADER file_header{};
        BITMAPINFOHEADER info{};
        file_header.bfType = 0x4d42;
        file_header.bfOffBits = sizeof(file_header) + sizeof(info);
        file_header.bfSize = file_header.bfOffBits + 256 * 256 * 4;
        info.biSize = sizeof(info); info.biWidth = 256; info.biHeight = -256;
        info.biPlanes = 1; info.biBitCount = 32; info.biCompression = BI_RGB;
        std::filesystem::create_directories("local");
        std::ofstream image("local/atlas_item.bmp", std::ios::binary);
        image.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
        image.write(reinterpret_cast<const char*>(&info), sizeof(info));
        for (UINT y = 0; y < 256; ++y)
            image.write(static_cast<const char*>(pixels.pBits) + y * pixels.Pitch, 256 * 4);
        image.flush(); written = image.good();
        result = readback->UnlockRect();
    }
    const HRESULT null_layout = state.bind_vertex_layout_00b23f20(nullptr);
    IDirect3DVertexDeclaration9* retained_layout = nullptr;
    const HRESULT got_layout = device.GetVertexDeclaration(&retained_layout);
    const bool layout_checked = SUCCEEDED(null_layout) && SUCCEEDED(got_layout)
        && retained_layout == layout->native() && state.vertex_layout_calls() == 2;
    if (retained_layout) retained_layout->Release();
    const HRESULT unbound = state.bind_texture_00b24710(0, nullptr);
    bool matched = SUCCEEDED(result) && SUCCEEDED(unbound) && written && layout_checked
        && state.texture_binding_calls() == 2;
    bound = nullptr;
    if (FAILED(device.GetTexture(0, &bound)) || bound) matched = false;
    if (bound) bound->Release();
    if (previous && FAILED(device.SetRenderTarget(0, previous))) matched = false;
    if (previous_state && FAILED(previous_state->Apply())) matched = false;
    if (previous_state) previous_state->Release();
    if (previous) previous->Release();
    if (readback) readback->Release();
    if (target) target->Release();
    std::printf("D3D9 atlas: hr=0x%08lx items=%zu packed_quarter=%u image_and_binding=%d\n",
        static_cast<unsigned long>(result), atlas.items.size(), item.packed_uv[2], matched);
    return matched;
}
