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
#include "bsp/memory_stream.hpp"
#include <vector>
#include <limits>
#include "bsp/d3d9_texture.hpp"
#include "bsp/d3d9_reset_texture.hpp"
#include "bsp/d3d9_query.hpp"
#include "bsp/texture_load_policy.hpp"
#include <d3dx9.h>
#include "bsp/font_data.hpp"
#include "bsp/font_registry.hpp"
#include "bsp/font_geometry.hpp"
#include "bsp/stream_scalars.hpp"
#include <filesystem>
#include <algorithm>


bool probe_shader_bindings(IDirect3DDevice9&, const char*);
bool probe_material_states_and_constants(IDirect3DDevice9&);
bool probe_texture_atlas(IDirect3DDevice9&, IDirect3DTexture9&, const char*);

static bool probe_installed_font(IDirect3DDevice9& device, const char* atlas_path) {
    const auto game_root = std::filesystem::path(atlas_path).parent_path().parent_path().parent_path();
    const bsp::FontScriptResolver resolve = [&](const std::string& name,
        std::string& bytes, std::string& message) {
        std::string relative = name;
        std::replace(relative.begin(), relative.end(), '\\', '/');
        if (relative != "Scripts/fundamentals.lua" && relative != "Fonts/Fonts.lua") {
            message = "Font probe resolver does not expose this script: " + name;
            return false;
        }
        bsp::PhysicalFile script;
        bsp::MemoryStream memory;
        DWORD status{};
        if (!script.open_read_only_00bf52a0_fragment((game_root / relative).string().c_str(), status)
            || !bsp::memory_stream_from_physical_00bef750_fragment(script, memory, status)
            || !memory.fully_initialized()) {
            message = "Font script read failed: " + name;
            return false;
        }
        bytes.assign(reinterpret_cast<const char*>(memory.data_00bef610()),
            static_cast<std::size_t>(memory.size_00bef600()));
        return true;
    };
    bsp::FontRegistry registry;
    std::string decode_error;
    if (!bsp::load_font_registry_lua(resolve, "Fonts/Fonts.lua", false, std::nullopt, registry, decode_error)) {
        std::printf("Font registry: %s\n", decode_error.c_str());
        return false;
    }
    const auto* descriptor = bsp::find_font_00ac3570(registry, "aRiAl16");
    const auto* viper = bsp::find_font_00ac3570(registry, "Viper19");
    const bool registry_checked = registry.fonts.size() == 6 && registry.executed_paths.size() == 2
        && descriptor && descriptor->data_file == "arial18.dat" && descriptor->gfx_file == "arial18.tga"
        && descriptor->alpha_texture == "white.tga" && descriptor->alpha_texture_scale == 1.0f
        && !descriptor->uppercase_only && viper && viper->uppercase_only
        && !bsp::find_font_00ac3570(registry, "MissingFont");
    std::printf("Installed Lua font registry: entries=%zu name_lookup_defaults_and_flags=%d\n",
        registry.fonts.size(), registry_checked);
    if (!registry_checked) return false;
    const auto path = game_root / "Fonts" / descriptor->data_file;
    bsp::PhysicalFile file;
    DWORD error{};
    bsp::MemoryStream stream;
    if (!file.open_read_only_00bf52a0_fragment(path.string().c_str(), error)
        || !bsp::memory_stream_from_physical_00bef750_fragment(file, stream, error)
        || !file.close_00bf5090_fragment(error)) return false;
    bsp::FontData font;
    if (!bsp::decode_font_data_00ad4c30_fragment(stream, descriptor->scale_ratio, font, decode_error)) {
        std::printf("Font DAT: %s\n", decode_error.c_str());
        return false;
    }
    const bool decoded = font.source_record_count == 207 && font.glyphs.size() == 207
        && font.scaled_height == 23 && stream.position_00bef580() == stream.size_00bef600()
        && bsp::font_has_glyph_00ad4500(font, 0x0091)
        && !bsp::font_has_glyph_00ad4500(font, 0xff91)
        && bsp::font_accepts_text_byte_00ab6d00(font, 'A')
        && bsp::font_accepts_text_byte_00ab6d00(font, ' ')
        && !bsp::font_accepts_text_byte_00ab6d00(font, 0x91);
    const auto letter = font.glyphs.find('A');
    const bool fields = letter != font.glyphs.end()
        && letter->second.fields_00_0c == std::array<float, 4>{0.1015625f, 0.240234375f, 0.203125f, 0.263671875f}
        && letter->second.field_10 == 0 && letter->second.scaled_field_12 == 10
        && letter->second.scaled_field_14 == 10;
    const auto& space = bsp::select_font_glyph_00ad4480(font, 0x20);
    const auto& missing = bsp::select_font_glyph_00ad4480(font, 0xffff);
    const auto& fallback_source = font.glyphs.at(0x91);
    const bool special_glyphs = letter != font.glyphs.end() && &space == &font.space_lf_glyph
        && &bsp::select_font_glyph_00ad4480(font, 0x0a) == &space
        && space.scaled_field_12 == 5 && space.scaled_field_14 == 0
        && &bsp::select_font_glyph_00ad4480(font, 0x0d) == &font.carriage_return_glyph
        && font.carriage_return_glyph.scaled_field_12 == 0
        && &bsp::select_font_glyph_00ad4480(font, 'A') == &letter->second
        && &missing == &font.missing_glyph && &missing != &fallback_source
        && missing.fields_00_0c == fallback_source.fields_00_0c
        && missing.field_10 == fallback_source.field_10
        && missing.scaled_field_12 == fallback_source.scaled_field_12
        && missing.scaled_field_14 == fallback_source.scaled_field_14;
    // Font scalar call path seeds zero: a one-byte final read zero-fills the
    // upper byte and the following EOF read returns zero without moving.
    const auto last = stream.data_00bef610()[stream.size_00bef600() - 1];
    const bool short_scalar = stream.seek_00bef540(-1, 2)
        && bsp::stream_read_word_00be4320(stream) == last
        && bsp::stream_read_u32_00be4300(stream) == 0
        && stream.position_00bef580() == stream.size_00bef600();
    std::printf("Installed font DAT: glyphs=%zu scaled_height=%u signed_byte_acceptance=%d short_scalar=%d special_glyphs=%d\n",
        font.glyphs.size(), static_cast<unsigned>(font.scaled_height), decoded && fields, short_scalar, special_glyphs);
    if (!(decoded && fields && short_scalar && special_glyphs)) return false;
    struct FontVertex { float x, y, z, u, v; DWORD color; };
    std::array<FontVertex, 4> vertices{};
    std::array<std::uint16_t, 6> indices{};
    const bsp::FontGeometryParameters geometry{120, 90, 1, 1, font.scaled_height, 0};
    const bsp::FontGeometryLayout layout{sizeof(FontVertex), 0, 12, 20, {}};
    const bool quad = bsp::write_font_quad_00ab98f0_fragment(letter->second, geometry,
        layout, reinterpret_cast<std::uint8_t*>(vertices.data()), sizeof(vertices), 0, indices)
        && vertices[0].x == 0.125f && vertices[0].y == 0.125f
        && vertices[2].x > vertices[0].x && vertices[2].y > vertices[0].y
        && vertices[0].u == 0.240234375f && vertices[0].v == 0.1015625f
        && vertices[2].u == 0.263671875f && vertices[2].v == 0.203125f
        && std::all_of(vertices.begin(), vertices.end(), [](const FontVertex& v) {
            return v.z == 0 && v.color == 0xffffffffu;
        }) && indices == std::array<std::uint16_t, 6>{0, 1, 2, 0, 2, 3};
    std::printf("Installed font quad: normalized_positions_UVs_colors_and_indices=%d\n", quad);
    if (!quad) return false;
    const std::string gfx_name = "Fonts/" + descriptor->gfx_file;
    auto texture_stream = std::make_shared<bsp::MemoryStream>();
    if (!file.open_read_only_00bf52a0_fragment((game_root / gfx_name).string().c_str(), error)
        || !bsp::memory_stream_from_physical_00bef750_fragment(file, *texture_stream, error)
        || !file.close_00bf5090_fragment(error)) return false;
    HMODULE module = LoadLibraryExW(L"d3dx9_40.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module) return false;
    FARPROC address = GetProcAddress(module, "D3DXCreateTextureFromFileInMemoryEx");
    bsp::CreateTextureFromMemory create{};
    static_assert(sizeof(create) == sizeof(address));
    std::memcpy(&create, &address, sizeof(create));
    address = GetProcAddress(module, "D3DXGetImageInfoFromFileInMemory");
    bsp::ReadImageInfoFromMemory read_info{};
    static_assert(sizeof(read_info) == sizeof(address));
    std::memcpy(&read_info, &address, sizeof(read_info));
    std::unique_ptr<bsp::D3D9RetainedTexture2D> owner;
    HRESULT result = bsp::load_retained_texture_2d_00b2c2d0_fragment(device, read_info, create,
        texture_stream, {static_cast<std::uint32_t>(gfx_name.size()), gfx_name.c_str()}, 0, owner);
    std::weak_ptr<bsp::MemoryStream> retained = texture_stream;
    texture_stream.reset();
    // The installed font image is 512x256, 32-bit TGA. Metadata is independently
    // checked from its file header; this is not a native font material draw.
    D3DSURFACE_DESC initial{}, recreated{};
    if (SUCCEEDED(result) && !owner) result = E_FAIL;
    if (SUCCEEDED(result)) result = owner->texture()->GetLevelDesc(0, &initial);
    bool texture_checked = SUCCEEDED(result) && !retained.expired();
    if (texture_checked) {
        const auto* tga = owner->source()->data_00bef610();
        texture_checked = owner->source()->size_00bef600() >= 18
            && tga[12] == 0 && tga[13] == 2 && tga[14] == 0 && tga[15] == 1 && tga[16] == 32
            && initial.Width == 512 && initial.Height == 256 && initial.Format == D3DFMT_A8R8G8B8
            && initial.Pool == D3DPOOL_MANAGED && initial.Usage == 0 && owner->texture()->GetLevelCount() == 1
            && owner->options().width == initial.Width && owner->options().height == initial.Height
            && owner->options().format == initial.Format && owner->options().mip_levels == 1;
        owner->release_com();
        result = owner->recreate_00b3e190(device, create);
        if (SUCCEEDED(result)) result = owner->texture()->GetLevelDesc(0, &recreated);
        texture_checked = texture_checked && SUCCEEDED(result)
            && recreated.Width == initial.Width && recreated.Height == initial.Height
            && recreated.Format == initial.Format && owner->source()->position_00bef580() == 0;
    }
    owner.reset();
    texture_checked = texture_checked && retained.expired();
    FreeLibrary(module);
    std::printf("Installed font texture: hr=0x%08lx size=%ux%u format=%u retained_recreation=%d\n",
        static_cast<unsigned long>(result), recreated.Width, recreated.Height,
        static_cast<unsigned>(recreated.Format), texture_checked);
    return texture_checked;
}

// Recovered physical-to-memory route with explicit completeness checks and DLL
// import adapter. VFS mount selection and full texture registration remain absent.
// The optional input is the single-level DXT1 atlas identified in ASSET_ENTRY.md.
static bool probe_memory_texture(IDirect3DDevice9& device, const char* path) {
    bsp::PhysicalFile file;
    DWORD error{};
    if (!file.open_read_only_00bf52a0_fragment(path, error) || !file.valid_00bf5020()) return false;
    const auto end = file.size_00bf4f90();
    if (end < 128 || end > (std::numeric_limits<UINT>::max)()) return false;
    auto stream = std::make_shared<bsp::MemoryStream>();
    if (!bsp::memory_stream_from_physical_00bef750_fragment(file, *stream, error)
        || !stream->fully_initialized() || file.position() != end
        || !file.close_00bf5090_fragment(error) || file.valid_00bf5020()) return false;
    std::printf("Physical asset read: bytes=%u cached_position_matches_size=1 closed=1\n",
        stream->initialized_size());
    const auto* bytes = stream->data_00bef610();
    const auto byte_count = static_cast<std::size_t>(stream->size_00bef600());
    DWORD magic{};
    std::uint32_t actual{};
    if (!stream->read_00bef590(&magic, sizeof(magic), &actual)
        || actual != 4 || magic != 0x20534444 || stream->position_00bef580() != 4) return false;
    auto clone = stream->clone_reset_00bef6d0();
    std::uint16_t header_part{};
    if (clone.position_00bef580() != 0 || clone.data_00bef610() != bytes
        || !clone.seek_00bef540(0x100000004LL, 0)
        || !clone.read_00bef590(&header_part, 2, &actual) || actual != 2 || header_part != 124
        || stream->position_00bef580() != 4) return false;
    auto word = [&](std::size_t offset) {
        DWORD value{};
        std::memcpy(&value, bytes + offset, sizeof(value));
        return value;
    };
    const UINT width = word(16), height = word(12);
    if (word(0) != 0x20534444 || word(4) != 124 || word(84) != D3DFMT_DXT1
        || !width || !height || width % 4 || height % 4) return false;
    const std::size_t row_bytes = static_cast<std::size_t>(width / 4) * 8;
    if (static_cast<std::uint64_t>(byte_count - 128)
        != static_cast<std::uint64_t>(row_bytes) * (height / 4)) return false;
    HMODULE module = LoadLibraryExW(L"d3dx9_40.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module) return false;
    FARPROC address = GetProcAddress(module, "D3DXCreateTextureFromFileInMemoryEx");
    bsp::CreateTextureFromMemory create{};
    static_assert(sizeof(create) == sizeof(address));
    std::memcpy(&create, &address, sizeof(create));
    bsp::ReadImageInfoFromMemory read_info{};
    address = GetProcAddress(module, "D3DXGetImageInfoFromFileInMemory");
    static_assert(sizeof(read_info) == sizeof(address));
    std::memcpy(&read_info, &address, sizeof(read_info));
    std::unique_ptr<bsp::D3D9RetainedTexture2D> loaded;
    std::weak_ptr<bsp::MemoryStream> retained = stream;
    HRESULT result = bsp::load_retained_texture_2d_00b2c2d0_fragment(device, read_info, create,
        stream, {static_cast<std::uint32_t>(std::strlen(path)), path}, 0, loaded);
    if (!loaded) { FreeLibrary(module); return false; }
    auto& owner = *loaded;
    std::printf("Initial texture load: D3DX_image_info_actual_metadata_and_source_retention=%d\n",
        SUCCEEDED(result) && owner.source() == stream && owner.options().width == width
        && owner.options().height == height && owner.options().format == D3DFMT_DXT1);
    stream.reset();
    clone = bsp::MemoryStream{};
    // The texture is now the only wrapper/backing owner. Recreate using that
    // retained source after dropping the initial COM texture and local streams.
    if (SUCCEEDED(result)) {
        owner.release_com();
        result = owner.recreate_00b3e190(device, create);
    }
    auto* texture = owner.texture();
    const bool retained_checked = !retained.expired() && owner.source()->position_00bef580() == 4;
    std::printf("Memory texture source: shared_backing_independent_cursor_retained_recreation=%d\n",
        retained_checked && SUCCEEDED(result));
    D3DSURFACE_DESC description{};
    if (SUCCEEDED(result) && !texture) result = E_FAIL;
    if (SUCCEEDED(result)) result = texture->GetLevelDesc(0, &description);
    bool matched = retained_checked && SUCCEEDED(result) && description.Width == width
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
                    bytes + 128 + row * row_bytes, row_bytes) == 0;
            const HRESULT unlock = texture->UnlockRect(0);
            matched = matched && SUCCEEDED(unlock);
        }
    }
    std::printf("D3D9 installed DDS: hr=0x%08lx size=%ux%u managed_DXT1_bytes_match=%d\n",
        static_cast<unsigned long>(result), description.Width, description.Height, matched);
    if (matched) matched = probe_texture_atlas(device, *texture, path);
    owner.release_com();
    owner.assign_source_00b23640_fragment({});
    matched = matched && retained.expired();
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
    bsp::D3D9OcclusionQuery visibility;
    IDirect3DSurface9* target = nullptr;
    IDirect3DSurface9* readback = nullptr;
    IDirect3DSurface9* original = nullptr;
    auto physical_vertices = std::make_shared<bsp::VertexBufferBinding>();
    auto& vertices = *physical_vertices;
    vertices.flags = 0x1000;
    vertices.capacity = 80;
    HRESULT result = bsp::create_occlusion_query_00b5fe90(visibility, device);
    if (SUCCEEDED(result)) result = device.GetRenderTarget(0, &original);
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
        if (!bsp::begin_occlusion_query_00b5fc30(visibility)) result = E_FAIL;
        if (SUCCEEDED(result)) result = states.draw_primitive_00b21b40({}, D3DPT_TRIANGLELIST, 1, 1);
        const bool query_ended = bsp::end_occlusion_query_00b5fc60(visibility);
        if (SUCCEEDED(result) && (!query_ended || visibility.field_08 != 0)) result = E_FAIL;
        const HRESULT ended = device.EndScene();
        if (SUCCEEDED(result)) result = ended;
    }
    if (SUCCEEDED(result)) result = device.GetRenderTargetData(target, readback);
    // Diagnostic wait only: readback completion did not make GetData immediately
    // ready on this device. Native poll remains one call and returns false for
    // both pending and failure. Bound host retries to avoid hanging the probe.
    UINT query_polls{};
    if (SUCCEEDED(result)) {
        const auto deadline = GetTickCount64() + 2000;
        bool ready{};
        do {
            ++query_polls;
            ready = bsp::poll_occlusion_query_00b5fca0(visibility);
            if (ready) break;
            Sleep(1);
        } while (GetTickCount64() < deadline);
        if (!ready) result = E_FAIL;
    }
    D3DLOCKED_RECT pixels{};
    if (SUCCEEDED(result)) result = readback->LockRect(&pixels, nullptr, D3DLOCK_READONLY);
    DWORD inside{}, outside{}, covered_pixels{};
    if (SUCCEEDED(result)) {
        const auto* bytes = static_cast<const unsigned char*>(pixels.pBits);
        std::memcpy(&inside, bytes + 16 * pixels.Pitch + 16 * 4, 4);
        std::memcpy(&outside, bytes + 60 * pixels.Pitch + 60 * 4, 4);
        for (UINT y = 0; y < 64; ++y) {
            for (UINT x = 0; x < 64; ++x) {
                DWORD value{};
                std::memcpy(&value, bytes + y * pixels.Pitch + x * 4, 4);
                if ((value & 0xffffff) == 0x00ff00) ++covered_pixels;
            }
        }
        result = readback->UnlockRect();
    }
    bool matched = SUCCEEDED(result) && (inside & 0xffffff) == 0x00ff00
        && (outside & 0xffffff) == 0 && visibility.field_08 == 2
        && covered_pixels != 0 && bsp::occlusion_query_samples_00b5fce0(visibility) == covered_pixels;
    std::printf("Occlusion query draw: samples=%u readback_covered_pixels=%lu polls=%u state=%u completed=%d\n",
        bsp::occlusion_query_samples_00b5fce0(visibility), covered_pixels, query_polls, visibility.field_08, matched);
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
bool probe_font_geometry_reference();
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
    if (!probe_font_geometry_reference()) return 1;
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
        bsp::D3D9SurfaceBinding texture_level0{}, texture_level1{};
        bsp::D3D9ResetTexture2D reset_texture;
        reset_texture.width = reset_texture.height = 64;
        reset_texture.mip_count = 2;
        reset_texture.format = D3DFMT_A8R8G8B8;
        reset_texture.flags = D3DPOOL_DEFAULT;
        reset_texture.levels.push_back({0, &texture_level0});
        reset_texture.levels.push_back({1, &texture_level1});
        bsp::D3D9DefaultSurfaces defaults;
        bsp::RendererSynchronization synchronization{};
        bsp::set_renderer_synchronization_00b33aa0(synchronization, true);
        auto* lock = bsp::critical_section_create_00bd1860();
        const auto release_lock = [](bsp::TrackedCriticalSection* value) {
            bsp::critical_section_destroy_owned_0041cc80(value);
        };
        std::unique_ptr<bsp::TrackedCriticalSection, decltype(release_lock)> lock_owner(lock, release_lock);
        bsp::D3D9StateCache cache(*device, synchronization, lock);
        bsp::VertexBufferBinding reset_vertices;
        bsp::IndexBufferBinding reset_indices;
        bsp::D3D9OcclusionQuery query_a, query_b;
        bsp::D3D9QueryRegistry queries;
        reset_vertices.flags = reset_indices.flags = 0x1000;
        reset_vertices.capacity = 0x1000000;
        reset_indices.capacity = 0x100000;
        bool buffers_ready = false;
        const bool lost_skip = bsp::restore_dynamic_buffers_00b1fd90(buffers_ready, true,
            reset_vertices, reset_indices, *device) == S_FALSE
            && !buffers_ready && !reset_vertices.buffer && !reset_indices.buffer;
        result = cache.capture_default_surfaces_00b238d0_fragment(defaults);
        if (SUCCEEDED(result)) result = cache.create_registered_query_00b27c20_fragment(queries, query_a);
        if (SUCCEEDED(result)) result = cache.create_registered_query_00b27c20_fragment(queries, query_b);
        const bool created_queries = SUCCEEDED(result) && queries.size() == 2
            && query_a.query && query_b.query && query_a.field_08 == 1 && query_a.field_0c == 0
            && query_b.field_08 == 1 && query_b.field_0c == 0;
        query_a.field_08 = 7;
        query_a.field_0c = 123;
        if (SUCCEEDED(result)) result = bsp::restore_dynamic_buffers_00b1fd90(buffers_ready, false,
            reset_vertices, reset_indices, *device);
        auto* const first_vertices = reset_vertices.buffer;
        auto* const first_indices = reset_indices.buffer;
        const bool ready_skip = SUCCEEDED(result)
            && bsp::restore_dynamic_buffers_00b1fd90(buffers_ready, false,
                reset_vertices, reset_indices, *device) == S_FALSE
            && reset_vertices.buffer == first_vertices && reset_indices.buffer == first_indices;
        // Seed valid upload metadata to distinguish COM release from stream rewind.
        reset_vertices.cursor = 48;
        reset_indices.cursor = 6;
        auto* const original_color_owner = &defaults.color;
        auto* const original_depth_owner = &defaults.depth;
        color.format = D3DFMT_A8R8G8B8;
        color.width = depth.width = 128;
        color.height = depth.height = 128;
        depth.format = D3DFMT_D24S8;
        depth.depth_stencil = true;
        bsp::D3D9SurfaceRegistry registry;
        registry.append_00b2a7c0_fragment(color);
        registry.append_00b2a7c0_fragment(depth);
        if (SUCCEEDED(result)) result = registry.recreate_00b23b10_fragment(*device);
        if (SUCCEEDED(result)) result = bsp::restore_texture_levels_00b3dd90(reset_texture, *device);
        cache.release_dynamic_buffers_00b237d0(buffers_ready, reset_vertices, reset_indices);
        cache.release_dynamic_buffers_00b237d0(buffers_ready, reset_vertices, reset_indices);
        const bool released_buffers = !buffers_ready && !reset_vertices.buffer && !reset_indices.buffer
            && reset_vertices.cursor == 48 && reset_indices.cursor == 6
            && lock->depth == 0 && synchronization.nesting == 0;
        defaults.release_for_reset_00b262c0_fragment();
        const bool released_defaults = !defaults.color.surface && !defaults.depth.surface
            && defaults.color.width == 640 && defaults.depth.width == 640;
        queries.release_00b262c0_fragment();
        const bool released_queries = !query_a.query && !query_b.query && queries.size() == 2;
        if (SUCCEEDED(result)) result = bsp::release_texture_levels_00b3dd30(reset_texture);
        const bool released_texture = !reset_texture.texture && !texture_level0.surface
            && !texture_level1.surface && texture_level0.width == 64 && texture_level1.width == 32;
        registry.release_for_reset_00b262c0_fragment();
        if (SUCCEEDED(result)) result = device->Reset(&stored);
        if (SUCCEEDED(result)) result = defaults.restore_00b23b10_fragment(*device);
        if (SUCCEEDED(result)) result = bsp::restore_texture_levels_00b3dd90(reset_texture, *device);
        if (SUCCEEDED(result)) result = registry.recreate_00b23b10_fragment(*device);
        if (SUCCEEDED(result)) result = queries.restore_00b23b10_fragment(*device);
        const bool restored_queries = SUCCEEDED(result) && created_queries && released_queries
            && query_a.query && query_b.query
            && query_a.query->GetType() == D3DQUERYTYPE_OCCLUSION
            && query_b.query->GetType() == D3DQUERYTYPE_OCCLUSION
            && query_a.query->GetDataSize() == sizeof(DWORD)
            && query_b.query->GetDataSize() == sizeof(DWORD)
            && query_a.field_08 == 7 && query_a.field_0c == 123;
        const bool removed_queries = cache.unregister_query_00b27cf0(queries, &query_a)
            && queries.size() == 1 && queries.at(0) == &query_b
            && !cache.unregister_query_00b27cf0(queries, &query_a)
            && query_a.query && query_b.query
            && cache.unregister_query_00b27cf0(queries, &query_b) && queries.size() == 0
            && lock->depth == 0 && synchronization.nesting == 0;
        std::printf("Occlusion query reset: real_queries_fields_borrowed_removal_and_guard=%d\n",
            restored_queries && removed_queries);
        bool restored_texture = SUCCEEDED(result) && released_texture;
        for (const auto& level : reset_texture.levels) {
            IDirect3DSurface9* current_level{};
            if (SUCCEEDED(result)) result = reset_texture.texture->GetSurfaceLevel(level.level, &current_level);
            restored_texture = restored_texture && SUCCEEDED(result)
                && current_level == level.binding->surface
                && level.binding->width == (64u >> level.level)
                && level.binding->height == (64u >> level.level)
                && level.binding->format == D3DFMT_A8R8G8B8;
            if (current_level) current_level->Release();
        }
        std::printf("2D texture reset: existing_cached_mip_wrappers_reinitialized=%d\n", restored_texture);
        if (SUCCEEDED(result)) result = bsp::restore_dynamic_buffers_00b1fd90(buffers_ready, false,
            reset_vertices, reset_indices, *device);
        D3DVERTEXBUFFER_DESC reset_vertex_desc{};
        D3DINDEXBUFFER_DESC reset_index_desc{};
        if (SUCCEEDED(result)) result = reset_vertices.buffer->GetDesc(&reset_vertex_desc);
        if (SUCCEEDED(result)) result = reset_indices.buffer->GetDesc(&reset_index_desc);
        const bool restored_buffers = SUCCEEDED(result) && lost_skip && ready_skip && released_buffers
            && buffers_ready && reset_vertex_desc.Size == 0x1000000 && reset_index_desc.Size == 0x100000
            && reset_vertex_desc.Pool == D3DPOOL_DEFAULT && reset_index_desc.Pool == D3DPOOL_DEFAULT
            && reset_vertex_desc.Usage == 0x208 && reset_index_desc.Usage == 0x208
            && reset_index_desc.Format == D3DFMT_INDEX16
            && reset_vertices.cursor == 48 && reset_indices.cursor == 6;
        std::printf("Dynamic buffer reset: readiness_lost_gates_metadata_and_guard=%d\n", restored_buffers);
        IDirect3DSurface9* restored_color{}, *restored_depth{};
        if (SUCCEEDED(result)) result = device->GetRenderTarget(0, &restored_color);
        if (SUCCEEDED(result)) result = device->GetDepthStencilSurface(&restored_depth);
        const bool restored_defaults = SUCCEEDED(result) && released_defaults
            && &defaults.color == original_color_owner && &defaults.depth == original_depth_owner
            && restored_color == defaults.color.surface && restored_depth == defaults.depth.surface
            && defaults.color.width == 640 && defaults.color.height == 480
            && defaults.depth.width == 640 && defaults.depth.height == 480
            && defaults.color.format == D3DFMT_A8R8G8B8 && defaults.depth.format == D3DFMT_D24S8
            && defaults.color.wrapper_flags == 0 && defaults.depth.wrapper_flags == 0
            && !defaults.color.depth_stencil && !defaults.depth.depth_stencil
            && cache.depth_binding_calls() == 1;
        if (restored_color) restored_color->Release();
        if (restored_depth) restored_depth->Release();
        std::printf("Default surface reset: existing_owners_reinitialized_without_depth_bind=%d\n",
            restored_defaults);
        D3DSURFACE_DESC color_desc{}, depth_desc{};
        if (SUCCEEDED(result)) result = color.surface->GetDesc(&color_desc);
        if (SUCCEEDED(result)) result = depth.surface->GetDesc(&depth_desc);
        matched = restored_queries && removed_queries && restored_texture
            && restored_buffers && restored_defaults && SUCCEEDED(result)
            && color_desc.Width == 128 && color_desc.Height == 128
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
        matched = SUCCEEDED(bsp::release_texture_levels_00b3dd30(reset_texture)) && matched;
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
    if (matched && argc > 1) matched = probe_installed_font(*device, argv[1]);
    if (device) device->Release();
    if (api) api->Release();
    DestroyWindow(window);
    UnregisterClassA(name, instance);
    return matched ? 0 : 1;
}
