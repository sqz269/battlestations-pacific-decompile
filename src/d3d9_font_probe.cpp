// Installed-source diagnostic: recovered search lists, supplied loose mount.
#include "bsp/font_geometry.hpp"
#include "bsp/font_layout.hpp"
#include "bsp/d3d9_texture.hpp"
#include "bsp/material_textures.hpp"
#include "bsp/material_samplers.hpp"
#include "bsp/material_constants.hpp"
#include "bsp/shader_reflection.hpp"
#include "bsp/resource_path.hpp"
#include "asset_stream_probe.hpp"
#include <d3dcompiler.h>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace {
template<class T> struct OwnedCom {
    T* p{};
    ~OwnedCom() { if (p) p->Release(); }
    OwnedCom() = default;
    OwnedCom(const OwnedCom&) = delete;
    OwnedCom& operator=(const OwnedCom&) = delete;
};
struct CompilerModule {
    HMODULE value = LoadLibraryExW(L"d3dcompiler_47.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    ~CompilerModule() { if (value) FreeLibrary(value); }
};

bool font_constants(const std::vector<bsp::ReflectedShaderConstant>& reflection,
    std::vector<float>& words, bool vertex) {
    std::vector<bool> occupied(words.size() / 4);
    for (const auto& c : reflection) {
        if (c.register_set == 3) {
            if (vertex || c.name != "MyTexture0" || c.register_index != 0 || c.register_count != 1) return false;
            continue;
        }
        if (c.register_set != 2 || c.parameter_type != 3 || !c.register_count
            || c.register_index >= occupied.size() || c.register_count > occupied.size() - c.register_index) return false;
        for (unsigned i = 0; i < c.register_count; ++i) {
            if (occupied[c.register_index + i]) return false;
            occupied[c.register_index + i] = true;
        }
        auto* out = words.data() + 4 * c.register_index;
        if (c.name == "cWorldMat" || c.name == "cViewProjMat") {
            if (!vertex || c.register_count != 4 || c.rows != 4 || c.columns != 4) return false;
            const float identity[]{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
            // Explicit host matrix maps native x/960,y/720 coordinates to a
            // 256x256 target while retaining glyph size in logical pixels.
            const float projection[]{7.5f,0,0,0, 0,-5.625f,0,0, 0,0,1,0, -1,1,0,1};
            bsp::write_system_matrix_00b404a0(out, c.name == "cWorldMat" ? identity : projection);
        } else if (c.name == "cVtxElemScale" || c.name == "cVtxElemOffset") {
            if (!vertex || c.register_count < 3 || c.register_count > 8 || c.rows != 1 || c.columns != 4) return false;
            std::fill(out, out + c.register_count * 4, c.name == "cVtxElemScale" ? 1.0f : 0.0f);
        } else {
            if (c.register_count != 1 || c.rows != 1) return false;
            if (c.name == "cMatDiffColor" && c.columns == 4) std::fill(out, out + 4, 1.0f);
            else if ((c.name == "cVisibility" || c.name == "cAspectRatio") && c.columns == 1) out[0] = 1;
            else if (c.name == "cOverbrightAlphatex" && c.columns == 2) { out[0] = 0; out[1] = 1; }
            else if (c.name == "cClipBorder" && c.columns == 4) std::fill(out, out + 4, 1.0f);
            else if ((c.name == "cClip" || c.name == "cElapsedTime") && c.columns == 1) out[0] = 0;
            else if (c.name == "cClipCenter" && c.columns == 2) { out[0] = 0; out[1] = 0; }
            else {
                std::fprintf(stderr, "Font draw unresolved reflected constant: %s\n", c.name.c_str());
                return false;
            }
        }
        std::printf("Font %s constant %s: c%u count=%u\n", vertex ? "VS" : "PS",
            c.name.c_str(), c.register_index, c.register_count);
    }
    return true;
}
}

bool probe_font_material(IDirect3DDevice9& device, const bsp::FontData& font,
    const std::shared_ptr<bsp::D3D9RetainedTexture2D>& gfx,
    const std::shared_ptr<bsp::D3D9RetainedTexture2D>& alpha,
    const char* game_root, const std::string& descriptor_name) {
    if (!game_root || !gfx || !alpha || !gfx->texture() || !alpha->texture()
        || !bsp::font_has_glyph_00ad4500(font, 0x41)) return false;
    auto selected_name = descriptor_name;
    if (!bsp::normalize_resource_path_00bee690(selected_name) || selected_name != "guifontbilinear.shfx") return false;
    AssetStreamProbe assets(std::string(game_root) + "\\");
    const bsp::ShaderScriptResolver resolver = [&](const std::string& requested, std::string& bytes, std::string& error) {
        std::shared_ptr<bsp::MemoryStream> stream;
        std::string logical;
        if (!assets.read(requested, stream, error, &logical)) return false;
        std::printf("Mounted font shader lookup: %s -> %s\n", requested.c_str(), logical.c_str());
        bytes.assign(reinterpret_cast<const char*>(stream->data_00bef610()),
            static_cast<std::size_t>(stream->size_00bef600()));
        return true;
    };
    bsp::ShaderLuaCode base, effect;
    std::string error;
    // Installed dx9_lua.inc defines RM_NORMAL=0.
    if (!bsp::load_shader_lua_code(resolver, selected_name, false, {}, base, error)
        || base.combiners[0] != "dummy.shfx"
        || !bsp::load_shader_lua_code(resolver, base.combiners[0], false, {}, effect, error)) {
        std::fprintf(stderr, "Font Lua: %s\n", error.c_str()); return false;
    }
    bsp::ShaderVertexProgram vp;
    bsp::ShaderPixelProgram pp;
    if (bsp::assemble_shader_programs(base, effect, 0, 3, false, vp, pp) != bsp::ShaderSourceStatus::complete) return false;
    std::string vs_source, ps_source;
    if (bsp::generate_pixel_source_00b39880(pp, ps_source) != bsp::ShaderSourceStatus::complete) return false;
    CompilerModule module;
    if (!module.value) return false;
    pD3DCompile compile{};
    decltype(&D3DDisassemble) disassemble{};
    const FARPROC compile_address = GetProcAddress(module.value, "D3DCompile");
    const FARPROC disassemble_address = GetProcAddress(module.value, "D3DDisassemble");
    std::memcpy(&compile, &compile_address, sizeof(compile));
    std::memcpy(&disassemble, &disassemble_address, sizeof(disassemble));
    if (!compile || !disassemble) return false;
    const auto profiles = bsp::select_shader_profiles_00b43b00(3, base.vertex_profile, base.pixel_profile);
    const auto compile_source = [&](const std::string& source, const std::string& profile, ID3DBlob** output) {
        OwnedCom<ID3DBlob> errors;
        const HRESULT hr = compile(source.data(), source.size(), "installed guifontbilinear",
            nullptr, nullptr, "main", profile.c_str(), 0, 0, output, &errors.p);
        if (FAILED(hr) && errors.p) std::fwrite(errors.p->GetBufferPointer(), 1, errors.p->GetBufferSize(), stderr);
        return hr;
    };
    OwnedCom<ID3DBlob> ps, vs, assembly;
    if (FAILED(compile_source(ps_source, profiles.pixel, &ps.p))
        || FAILED(disassemble(ps.p->GetBufferPointer(), ps.p->GetBufferSize(), 0, nullptr, &assembly.p))) return false;
    std::vector<std::uint32_t> used_uv(10), used_color(2);
    std::vector<bsp::ShaderField> selected;
    if (bsp::parse_pixel_usage_00b61280(static_cast<const char*>(assembly.p->GetBufferPointer()), used_uv, used_color)
            != bsp::ShaderSourceStatus::complete
        || bsp::append_selected_interpolators_00b36800(base.interpolators, effect.interpolators,
            &used_uv, &used_color, selected) != bsp::ShaderSourceStatus::complete) return false;
    // ShaderCode still writes the complete descriptor OUT (including UV1).
    // Pixel liveness filters only PackInterpolators and its mapping, not the
    // sVertexOut declaration/zero initialization supplied by the assembler.
    vp.packing_fields = selected; vp.interpolators = {};
    bsp::append_interpolator_mapping_00b34aa0(selected, vp.interpolators);
    if (bsp::generate_vertex_source_00b39110(vp, vs_source) != bsp::ShaderSourceStatus::complete
        || FAILED(compile_source(vs_source, profiles.vertex, &vs.p))) return false;
    std::vector<bsp::ReflectedShaderConstant> vr, pr;
    bsp::ShaderConstantBindings vb, pb;
    const auto registry = bsp::make_system_constant_registry_00b5bf70();
    if (!bsp::reflect_shader_constants_00b3aea0(static_cast<const std::uint32_t*>(vs.p->GetBufferPointer()), vr, error)
        || !bsp::reflect_shader_constants_00b3aea0(static_cast<const std::uint32_t*>(ps.p->GetBufferPointer()), pr, error)
        || !bsp::map_shader_constants_00b3aea0(vr, registry, vb)
        || !bsp::map_shader_constants_00b3aea0(pr, registry, pb)) return false;
    std::vector<float> vwords(256 * 4), pwords(224 * 4);
    if (!font_constants(vr, vwords, true) || !font_constants(pr, pwords, false)) return false;
    bsp::MaterialSamplerPass pass;
    bsp::MaterialSamplerCounters counters;
    if (!bsp::append_material_samplers_00b3b280(base.samplers, pass, counters)
        || !bsp::append_material_samplers_00b3b280(effect.samplers, pass, counters)
        || counters.pixel != 2 || counters.vertex != 0 || pb.sampler_mask != 1) return false;
    bsp::prune_material_sampler_states(pass, pb.sampler_mask);
    bsp::MaterialTextureSlots slots;
    auto gfx_logical = std::make_shared<bsp::LogicalTexture>(); gfx_logical->texture = gfx->texture();
    auto alpha_logical = std::make_shared<bsp::LogicalTexture>(); alpha_logical->texture = alpha->texture();
    if (!bsp::set_material_texture_00b189f0(slots, 0, gfx_logical)
        || !bsp::set_material_texture_00b189f0(slots, 1, alpha_logical)) return false;

    struct Vertex { float x, y, z, u, v; std::uint32_t color; };
    std::array<Vertex, 4> vertices{};
    std::array<std::uint16_t, 6> indices{};
    bsp::FontSingleLineLayout line;
    std::string layout_error;
    if (!bsp::build_font_single_line_00ab9fd0_fragment(font, u"A",
        {0.125f, 1, 0, 1}, line, layout_error) || line.placements.size() != 1) return false;
    bsp::FontGeometryParameters parameters;
    // Explicit host placement of the recovered single-line pen in the target.
    parameters.x = 64 + line.placements[0].x;
    parameters.y = 64 + line.placements[0].y;
    parameters.height = line.height;
    bsp::FontGeometryLayout geometry;
    geometry.stride = sizeof(Vertex); geometry.uv_offset = 12; geometry.packed_color_offset = 20;
    const auto& glyph = bsp::select_font_glyph_00ad4480(font, line.placements[0].code_unit);
    if (!bsp::write_font_quad_00ab98f0_fragment(glyph, parameters, geometry,
        reinterpret_cast<std::uint8_t*>(vertices.data()), sizeof(vertices), 0, indices)) return false;
    // Readback bounds derive from the generated quad and explicit host matrix.
    const int left = static_cast<int>(std::floor(vertices[0].x * 960)) - 1;
    const int top = static_cast<int>(std::floor(vertices[0].y * 720)) - 1;
    const int right = static_cast<int>(std::ceil(vertices[2].x * 960)) + 1;
    const int bottom = static_cast<int>(std::ceil(vertices[2].y * 720)) + 1;
    if (left < 2 || top < 2 || right >= 254 || bottom >= 254 || left >= right || top >= bottom) return false;
    OwnedCom<IDirect3DStateBlock9> saved;
    OwnedCom<IDirect3DSurface9> old_target, old_depth, target, readback;
    OwnedCom<IDirect3DVertexShader9> vertex_shader;
    OwnedCom<IDirect3DPixelShader9> pixel_shader;
    HRESULT hr = device.CreateStateBlock(D3DSBT_ALL, &saved.p);
    if (SUCCEEDED(hr)) hr = device.GetRenderTarget(0, &old_target.p);
    if (SUCCEEDED(hr)) {
        const HRESULT depth = device.GetDepthStencilSurface(&old_depth.p);
        if (FAILED(depth) && depth != D3DERR_NOTFOUND) hr = depth;
    }
    // No device mutation until all required restoration handles are captured.
    if (FAILED(hr)) return false;
    hr = device.CreateRenderTarget(256, 256, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &target.p, nullptr);
    if (SUCCEEDED(hr)) hr = device.CreateOffscreenPlainSurface(256, 256, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &readback.p, nullptr);
    if (SUCCEEDED(hr)) hr = device.CreateVertexShader(static_cast<const DWORD*>(vs.p->GetBufferPointer()), &vertex_shader.p);
    if (SUCCEEDED(hr)) hr = device.CreatePixelShader(static_cast<const DWORD*>(ps.p->GetBufferPointer()), &pixel_shader.p);
    if (SUCCEEDED(hr)) hr = device.SetDepthStencilSurface(nullptr);
    if (SUCCEEDED(hr)) hr = device.SetRenderTarget(0, target.p);
    const D3DVIEWPORT9 viewport{0,0,256,256,0,1};
    if (SUCCEEDED(hr)) hr = device.SetViewport(&viewport);
    for (const auto& setting : {std::pair<D3DRENDERSTATETYPE, DWORD>{D3DRS_CULLMODE,D3DCULL_NONE},
        {D3DRS_SCISSORTESTENABLE,FALSE}, {D3DRS_SRGBWRITEENABLE,FALSE}, {D3DRS_COLORWRITEENABLE,15},
        {D3DRS_SEPARATEALPHABLENDENABLE,FALSE}, {D3DRS_BLENDOP,D3DBLENDOP_ADD}})
        if (SUCCEEDED(hr)) hr = device.SetRenderState(setting.first, setting.second);
    if (SUCCEEDED(hr)) hr = device.SetSamplerState(0, D3DSAMP_SRGBTEXTURE, FALSE);
    unsigned visible = 0, outside = 0;
    int min_x = 256, min_y = 256, max_x = -1, max_y = -1;
    {
        bsp::RendererSynchronization sync{};
        bsp::D3D9StateCache state(device, sync, nullptr);
        const bsp::LogicalVertexShader vshader{vertex_shader.p};
        const bsp::LogicalPixelShader pshader{pixel_shader.p};
        if (SUCCEEDED(hr)) hr = state.bind_vertex_shader_00b21d10(&vshader);
        if (SUCCEEDED(hr)) hr = state.bind_pixel_shader_00b21c20(&pshader);
        auto states = std::make_shared<bsp::RenderStateBlock>();
        for (const auto* script : {&base, &effect}) for (const auto& setting : script->render_states)
            states->states.push_back({static_cast<D3DRENDERSTATETYPE>(setting.state), setting.value});
        if (SUCCEEDED(hr)) {
            state.bind_render_state_block_00b27a80(states);
            state.bind_sampler_state_block_00b27b90(std::make_shared<bsp::SamplerStateBlock>(pass.sampler_states));
            hr = bsp::bind_material_textures_00b43470(state, pass, slots.textures(), pb.sampler_mask);
        }
        for (const auto& c : vr) if (c.register_set == 2 && SUCCEEDED(hr))
            hr = state.set_vertex_shader_constants_f_00b21820(c.register_index, vwords.data() + c.register_index * 4, c.register_count);
        for (const auto& c : pr) if (c.register_set == 2 && SUCCEEDED(hr))
            hr = state.set_pixel_shader_constants_f_00b218c0(c.register_index, pwords.data() + c.register_index * 4, c.register_count);
        auto stream = std::make_shared<bsp::LogicalVertexStream>();
        stream->physical = std::make_shared<bsp::VertexBufferBinding>();
        stream->physical->flags = 0x1000; stream->physical->capacity = sizeof(vertices);
        stream->flags = 0x1000; stream->tag = 0x40000001;
        stream->declaration = std::make_shared<bsp::VertexDeclaration>();
        stream->declaration->append_00b48330(D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION);
        stream->declaration->append_00b48330(D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD);
        stream->declaration->append_00b48330(D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR);
        auto layout = std::make_shared<bsp::D3D9VertexLayout>(); layout->append_stream_00b48a00(stream->declaration);
        auto index_stream = std::make_shared<bsp::LogicalIndexStream>();
        index_stream->physical = std::make_shared<bsp::IndexBufferBinding>();
        index_stream->physical->flags = 0x1000; index_stream->physical->capacity = sizeof(indices);
        index_stream->index_count = 6;
        if (SUCCEEDED(hr)) hr = bsp::vertex_buffer_recreate_00b492b0(*stream->physical, device);
        if (SUCCEEDED(hr)) hr = bsp::index_buffer_recreate_00b49180(*index_stream->physical, device);
        if (SUCCEEDED(hr)) hr = layout->create_if_missing_00b60a10(device);
        void* mapped = nullptr;
        if (SUCCEEDED(hr)) hr = state.lock_vertex_stream_00b49980(*stream, 4, 0, false, mapped);
        if (SUCCEEDED(hr)) {
            std::memcpy(mapped, vertices.data(), sizeof(vertices));
            state.unlock_vertex_stream_00b49a80(*stream);
            state.bind_vertex_stream_00b24840(0, stream);
            hr = state.bind_vertex_layout_00b23f20(layout);
        }
        if (SUCCEEDED(hr)) hr = state.lock_index_stream_00b49b60(*index_stream, 0, 0, false, mapped);
        if (SUCCEEDED(hr)) {
            std::memcpy(mapped, indices.data(), sizeof(indices));
            state.unlock_index_stream_00b49c70(*index_stream);
            state.bind_index_stream_00b24b00(index_stream, 0);
        }
        if (SUCCEEDED(hr)) hr = device.Clear(0, nullptr, D3DCLEAR_TARGET, 0xff000000, 1, 0);
        if (SUCCEEDED(hr)) hr = device.BeginScene();
        if (SUCCEEDED(hr)) {
            hr = state.draw_indexed_00b24010({}, D3DPT_TRIANGLELIST, 0, 4, 0, 2);
            const HRESULT ended = device.EndScene();
            if (SUCCEEDED(hr)) hr = ended;
        }
        if (SUCCEEDED(hr)) hr = device.GetRenderTargetData(target.p, readback.p);
        D3DLOCKED_RECT pixels{};
        if (SUCCEEDED(hr)) hr = readback.p->LockRect(&pixels, nullptr, D3DLOCK_READONLY);
        if (SUCCEEDED(hr)) {
            const auto* data = static_cast<const unsigned char*>(pixels.pBits);
            for (int y = 0; y < 256; ++y) for (int x = 0; x < 256; ++x) {
                DWORD color{}; std::memcpy(&color, data + y * pixels.Pitch + x * 4, 4);
                if (color & 0xffffff) {
                    ++visible;
                    min_x = (std::min)(min_x, x); max_x = (std::max)(max_x, x);
                    min_y = (std::min)(min_y, y); max_y = (std::max)(max_y, y);
                    if (x < left || x > right || y < top || y > bottom) ++outside;
                }
            }
            hr = readback.p->UnlockRect();
        }
        state.invalidate();
    }
    bool restored = SUCCEEDED(device.SetRenderTarget(0, old_target.p));
    restored = SUCCEEDED(device.SetDepthStencilSurface(old_depth.p)) && restored;
    restored = SUCCEEDED(saved.p->Apply()) && restored;
    const bool matched = SUCCEEDED(hr) && visible > 0 && outside == 0 && restored;
    std::printf("Installed bilinear font draw: descriptor=%s glyph=A hr=0x%08lx visible=%u outside=%u bounds=%d,%d..%d,%d expected=%d,%d..%d,%d sampler_mask=%u restored=%d checked=%d\n",
        descriptor_name.c_str(), static_cast<unsigned long>(hr), visible, outside,
        min_x, min_y, max_x, max_y, left, top, right, bottom, pb.sampler_mask, restored, matched);
    return matched;
}
