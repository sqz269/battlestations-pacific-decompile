// Diagnostic atlas host: parser, texture binding and image readback. This is
// not the game's UI material or window implementation.
#include "bsp/texture_atlas.hpp"
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
    if (SUCCEEDED(result)) result = device.SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
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
    const auto& uv = item.uv;
    const Vertex quad[]{{-0.5f,-0.5f,0,1,uv[0],uv[1]}, {255.5f,-0.5f,0,1,uv[2],uv[1]},
        {-0.5f,255.5f,0,1,uv[0],uv[3]}, {255.5f,255.5f,0,1,uv[2],uv[3]}};
    if (SUCCEEDED(result)) result = device.Clear(0, nullptr, D3DCLEAR_TARGET, 0xffff00ff, 1, 0);
    if (SUCCEEDED(result)) result = device.BeginScene();
    if (SUCCEEDED(result)) {
        result = device.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(Vertex));
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
    const HRESULT unbound = state.bind_texture_00b24710(0, nullptr);
    bool matched = SUCCEEDED(result) && SUCCEEDED(unbound) && written
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
