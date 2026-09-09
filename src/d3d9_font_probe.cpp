// Installed-source diagnostic: recovered search lists, supplied loose mount.
#include "bsp/font_geometry_owner.hpp"
#include "bsp/d3d9_texture.hpp"
#include "bsp/material_textures.hpp"
#include "bsp/material_samplers.hpp"
#include "bsp/material_constants.hpp"
#include "bsp/material_parameter_bindings.hpp"
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
#include <limits>
#include <utility>

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
struct FontShaderOwner {
    IDirect3DVertexShader9* vertex;
    IDirect3DPixelShader9* pixel;
    FontShaderOwner(IDirect3DVertexShader9& vs, IDirect3DPixelShader9& ps) : vertex(&vs), pixel(&ps) {
        vertex->AddRef(); pixel->AddRef();
    }
    ~FontShaderOwner() { pixel->Release(); vertex->Release(); }
    FontShaderOwner(const FontShaderOwner&) = delete;
    FontShaderOwner& operator=(const FontShaderOwner&) = delete;
};

bool font_constants(const std::vector<bsp::ReflectedShaderConstant>& reflection,
    const bsp::ShaderConstantBindings& bindings, std::vector<float>& words, bool vertex) {
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
        // Non-system values are supplied through the recovered named parameter
        // table below. Matrices, visibility and time remain renderer inputs.
        if (std::any_of(bindings.material_constants.begin(), bindings.material_constants.end(),
                [&](const auto& material) { return material.name == c.name; })) continue;
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
            else if (c.name == "cVisibility" && c.columns == 1) out[0] = 1;
            else if (c.name == "cElapsedTime" && c.columns == 1) out[0] = 0;
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

bool font_material_constants(IDirect3DVertexShader9& vertex, IDirect3DPixelShader9& pixel,
    const bsp::ShaderConstantBindings& vb, const bsp::ShaderConstantBindings& pb,
    std::vector<float>& vwords, std::vector<float>& pwords, std::string& error) {
    // Explicit inputs for this installed-font draw: resolved alpha scale 1,
    // unclipped viewport and base context colors. This is not a reconstructed
    // context constructor or the unresolved font/effect cache selection path.
    struct Input { const char* name; std::array<float, 4> words; std::uint32_t count; };
    std::array<Input, 8> inputs{{
        {"cOverbrightAlphatex", {0,1,0,0}, 2}, {"cLowColor", {0,0,0,1}, 4},
        {"cHighColor", {1,1,1,1}, 4}, {"cBlendFactor", {0,0,0,0}, 1},
        {"cClip", {0,0,0,0}, 1}, {"cClipCenter", {0,0,0,0}, 2},
        {"cClipBorder", {1,1,1,1}, 4}, {"cAspectRatio", {1,0,0,0}, 1}
    }};
    for (const auto* stage : {&vb, &pb}) for (const auto& constant : stage->material_constants) {
        const auto input = std::find_if(inputs.begin(), inputs.end(),
            [&](const auto& value) { return constant.name == value.name; });
        if (input == inputs.end() || constant.register_set != 2 || constant.parameter_type != 3
            || constant.register_count != 1 || constant.rows != 1 || constant.columns != input->count) {
            error = "Unresolved font material input: " + constant.name;
            return false;
        }
    }
    auto shaders = std::make_shared<FontShaderOwner>(vertex, pixel);
    bsp::MaterialParameterSelectors selectors;
    selectors[0] = bsp::MaterialParameterSelector{vb, pb};
    bsp::MaterialParameterBindings bindings;
    const auto register_inputs = [&]() {
        for (const auto& input : inputs)
            if (bindings.register_words_00b17e10_00b44d60(input.name,
                    {input.words.data(), input.words.size(), input.count, false}, error).status
                == bsp::MaterialParameterRegistrationStatus::unsupported) return false;
        return true;
    };
    if (!bindings.set_shader_00b19210_fragment(shaders, selectors, error) || !register_inputs()) return false;
    // One lifecycle exercise against actual compiled metadata: assignment of the
    // same shader clears registrations, then borrowed alpha changes reach its
    // real register without re-registration. Pack final values before drawing.
    const auto registered = bindings.size();
    if (!registered || !bindings.set_shader_00b19210_fragment(shaders, selectors, error)
        || bindings.size() != 0 || bindings.shader_identity() != shaders.get() || !register_inputs()
        || bindings.size() != registered) return false;
    const bsp::MaterialParameterRecord* alpha = nullptr;
    for (std::size_t i = 0; i < bindings.size(); ++i)
        if (bindings.record(i)->name == "cOverbrightAlphatex") alpha = bindings.record(i);
    if (!alpha) return false;
    inputs[0].words[1] = 0.5f;
    if (bindings.pack_00b423c5(0, vwords, pwords) != bsp::MaterialConstantPackStatus::complete) return false;
    for (const auto& stage : {std::pair<std::int32_t, const std::vector<float>*>{alpha->vertex_registers[0], &vwords},
             {alpha->pixel_registers[0], &pwords}})
        if (stage.first >= 0 && (*stage.second)[static_cast<std::size_t>(stage.first) * 4 + 1] != 0.5f) return false;
    inputs[0].words[1] = 1;
    if (bindings.pack_00b423c5(0, vwords, pwords) != bsp::MaterialConstantPackStatus::complete) return false;
    std::printf("Installed font material bindings: VS=%zu PS=%zu registered=%zu same_shader_clear=1 borrowed_alpha_update=1 retained_shader_owner=1\n",
        vb.material_constants.size(), pb.material_constants.size(), bindings.size());
    return true;
}
}

bool probe_font_material(IDirect3DDevice9& device,
    const std::shared_ptr<const bsp::FontResources>& resources,
    const char* game_root, const std::string& descriptor_name, bool wrapped) {
    if (!game_root || !resources || !resources->gfx || !resources->alpha
        || !resources->gfx->texture() || !resources->alpha->texture()
        || !bsp::font_has_glyph_00ad4500(resources->data, 0x41)) return false;
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
    if (!font_constants(vr, vb, vwords, true) || !font_constants(pr, pb, pwords, false)) return false;
    bsp::MaterialSamplerPass pass;
    bsp::MaterialSamplerCounters counters;
    if (!bsp::append_material_samplers_00b3b280(base.samplers, pass, counters)
        || !bsp::append_material_samplers_00b3b280(effect.samplers, pass, counters)
        || counters.pixel != 2 || counters.vertex != 0 || pb.sampler_mask != 1) return false;
    bsp::prune_material_sampler_states(pass, pb.sampler_mask);
    const std::u16string_view fixture = wrapped ? u"A A\nA" : u"A";
    const std::u16string_view case_only = wrapped ? u"a a\na" : u"a";
    bsp::FontGeometryUpdateParameters parameters;
    parameters.multiline = wrapped ? 1 : 0;
    parameters.single_line = {0.125f, 1, 0, 1};
    parameters.wrapped = {0.03125f, 0.25f, 2, 2, 0, 1, 1, 5, 3};
    parameters.origin_x = 64;
    parameters.origin_y = wrapped ? 0.0f : 64.0f;
    struct InkBounds { int left, top, right, bottom; unsigned pixels{}; };
    std::vector<InkBounds> ink_bounds;
    int left = 256, top = 256, right = -1, bottom = -1;
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
    unsigned visible = 0, outside = 0;
    int min_x = 256, min_y = 256, max_x = -1, max_y = -1;
    bool lifecycle_checked = false;
    try {
        hr = device.CreateRenderTarget(256, 256, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &target.p, nullptr);
        if (SUCCEEDED(hr)) hr = device.CreateOffscreenPlainSurface(256, 256, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &readback.p, nullptr);
        if (SUCCEEDED(hr)) hr = device.CreateVertexShader(static_cast<const DWORD*>(vs.p->GetBufferPointer()), &vertex_shader.p);
        if (SUCCEEDED(hr)) hr = device.CreatePixelShader(static_cast<const DWORD*>(ps.p->GetBufferPointer()), &pixel_shader.p);
        if (SUCCEEDED(hr) && !font_material_constants(*vertex_shader.p, *pixel_shader.p, vb, pb, vwords, pwords, error)) {
            std::fprintf(stderr, "Font material binding failed: %s\n", error.c_str());
            hr = E_FAIL;
        }
        if (SUCCEEDED(hr)) hr = device.SetDepthStencilSurface(nullptr);
        if (SUCCEEDED(hr)) hr = device.SetRenderTarget(0, target.p);
        const D3DVIEWPORT9 viewport{0,0,256,256,0,1};
        if (SUCCEEDED(hr)) hr = device.SetViewport(&viewport);
        for (const auto& setting : {std::pair<D3DRENDERSTATETYPE, DWORD>{D3DRS_CULLMODE,D3DCULL_NONE},
            {D3DRS_SCISSORTESTENABLE,FALSE}, {D3DRS_SRGBWRITEENABLE,FALSE}, {D3DRS_COLORWRITEENABLE,15},
            {D3DRS_SEPARATEALPHABLENDENABLE,FALSE}, {D3DRS_BLENDOP,D3DBLENDOP_ADD}})
            if (SUCCEEDED(hr)) hr = device.SetRenderState(setting.first, setting.second);
        if (SUCCEEDED(hr)) hr = device.SetSamplerState(0, D3DSAMP_SRGBTEXTURE, FALSE);
        {
            bsp::RendererSynchronization sync{};
            bsp::D3D9StateCache state(device, sync, nullptr);
            std::unique_ptr<bsp::FontGeometryOwner> geometry_owner;
            if (SUCCEEDED(hr)) {
                bsp::FontGeometryOwner staged(device, state,
                    {resources, vertex_shader.p, pixel_shader.p, pass, pb.sampler_mask});
                const auto initial = staged.update_00aba8d0_fragment(fixture, parameters, error);
                if (initial != bsp::FontGeometryUpdate::rebuilt) hr = E_FAIL;
                if (SUCCEEDED(hr)) hr = staged.bind();
                const auto* bound_vertex_shader = state.vertex_shader();
                const auto* bound_pixel_shader = state.pixel_shader();
                const auto* bound_stream = staged.geometry().main.vertices.get();
                geometry_owner = std::make_unique<bsp::FontGeometryOwner>(std::move(staged));
                auto& owner = *geometry_owner;
                const bool moved = SUCCEEDED(hr) && bound_vertex_shader && bound_pixel_shader
                    && state.vertex_shader() == bound_vertex_shader && state.pixel_shader() == bound_pixel_shader
                    && owner.geometry().main.vertices.get() == bound_stream
                    && staged.text().empty() && staged.bind() == D3DERR_INVALIDCALL;
                auto invalid = parameters;
                invalid.origin_x = std::numeric_limits<float>::quiet_NaN();
                const bool same = moved
                    && owner.update_00aba8d0_fragment(case_only, invalid, error) == bsp::FontGeometryUpdate::unchanged
                    && owner.text() == fixture && owner.geometry().main.vertices.get() == bound_stream
                    && state.vertex_shader() == bound_vertex_shader && state.pixel_shader() == bound_pixel_shader;
                const std::weak_ptr<bsp::LogicalVertexStream> first_vertex = owner.geometry().main.vertices;
                const std::weak_ptr<bsp::LogicalIndexStream> first_index = owner.geometry().main.indices;
                const bool rebuilt = same
                    && owner.rebuild_00abb1d0_fragment(parameters, error) == bsp::FontGeometryUpdate::rebuilt
                    && first_vertex.expired() && first_index.expired()
                    && !state.vertex_shader() && !state.pixel_shader();
                const auto stale = owner.metrics();
                const auto* retained_vertex = owner.geometry().main.vertices.get();
                const auto* retained_index = owner.geometry().main.indices.get();
                const auto* retained_layout = owner.geometry().main.layout.get();
                const auto* retained_bytes = owner.geometry().vertices.data();
                const auto retained_count = owner.geometry().placements.size();
                const bool cleared = rebuilt
                    && owner.update_00aba8d0_fragment({}, invalid, error) == bsp::FontGeometryUpdate::cleared
                    && owner.text().empty() && owner.metrics().measured_width == 0
                    && owner.metrics().line_count == stale.line_count
                    && owner.metrics().wrapped_height == stale.wrapped_height
                    && owner.metrics().initial_x == stale.initial_x
                    && owner.metrics().normalized_vertical_offset == stale.normalized_vertical_offset
                    && owner.geometry().main.vertices.get() == retained_vertex
                    && owner.geometry().main.indices.get() == retained_index
                    && owner.geometry().main.layout.get() == retained_layout
                    && owner.geometry().vertices.data() == retained_bytes
                    && owner.geometry().placements.size() == retained_count
                    && owner.geometry().main.vertex_count == 0 && owner.geometry().main.primitive_count == 0
                    && owner.geometry().shadow.vertex_count == 0 && owner.geometry().shadow.primitive_count == 0;
                const bool empty_rebuild = cleared
                    && owner.rebuild_00abb1d0_fragment(invalid, error) == bsp::FontGeometryUpdate::unchanged;
                const std::weak_ptr<bsp::LogicalVertexStream> cleared_vertex = owner.geometry().main.vertices;
                const std::weak_ptr<bsp::LogicalIndexStream> cleared_index = owner.geometry().main.indices;
                const bool refilled = empty_rebuild
                    && owner.update_00aba8d0_fragment(fixture, parameters, error) == bsp::FontGeometryUpdate::rebuilt
                    && cleared_vertex.expired() && cleared_index.expired();
                const auto& snapshot = owner.geometry();
                const bool shared = refilled && snapshot.main.vertices == snapshot.shadow.vertices
                    && snapshot.main.indices == snapshot.shadow.indices && snapshot.main.layout == snapshot.shadow.layout
                    && snapshot.main.vertex_count == snapshot.shadow.vertex_count
                    && snapshot.main.primitive_count == snapshot.shadow.primitive_count
                    && snapshot.main.vertices->physical->logical_streams.size() == 1
                    && snapshot.main.indices->physical->logical_streams.size() == 1;
                lifecycle_checked = moved && same && rebuilt && cleared && empty_rebuild && refilled && shared;
                if (!lifecycle_checked) hr = E_FAIL;
                if (SUCCEEDED(hr)) hr = owner.bind(true);
                if (SUCCEEDED(hr)) hr = owner.bind(false);
                std::printf("Installed font owner: wrapped=%d bound_move=%d same_text=%d explicit_rebuild_fresh=%d empty_retains=%d empty_rebuild_skips=%d changed_fresh=%d main_shadow_shared=%d checked=%d error=%s\n",
                    wrapped, moved, same, rebuilt, cleared, empty_rebuild, refilled, shared, lifecycle_checked, error.c_str());
                if (SUCCEEDED(hr)) {
                    const auto& metric = owner.metrics();
                    const auto& placements = snapshot.placements;
                    bool scalar_checked = placements.size() == 1;
                    if (wrapped) {
                        scalar_checked = snapshot.lines.size() == 3 && placements.size() == 5
                            && metric.line_count == 3 && metric.container_width == 30 && metric.measured_width == 15
                            && metric.height == 23 && std::fabs(metric.wrapped_height - 62.1f) < 0.00001f
                            && std::fabs(metric.normalized_wrapped_height - 0.1725f) < 0.0000001f
                            && std::fabs(metric.normalized_vertical_offset - 0.16375f) < 0.0000001f;
                        constexpr std::array<float, 5> expected_x{5, 25, 5, 25, 5};
                        constexpr std::array<float, 5> expected_y{0, 0, 19.55f, 19.55f, 39.1f};
                        constexpr std::array<std::uint16_t, 5> expected_code{0x41, 0x20, 0x41, 0x0a, 0x41};
                        for (std::size_t i = 0; scalar_checked && i < 5; ++i)
                            scalar_checked = placements[i].code_unit == expected_code[i]
                                && placements[i].x == expected_x[i]
                                && std::fabs(placements[i].y - expected_y[i]) < 0.00001f;
                        // D3D9 selected 24-bit x87 precision here. Keep the existing
                        // decimal tolerances for the native-style y/height spills.
                        unsigned short control;
                        __asm fnstcw control
                        std::printf("Installed wrapped font layout: lines=%zu glyphs=%zu width=%g height=%.9g vertical_offset=%.9g x87=0x%x soft_wrap_LF_spacing_and_alignment=%d error=%s\n",
                            snapshot.lines.size(), placements.size(), metric.measured_width,
                            metric.wrapped_height, metric.normalized_vertical_offset, control, scalar_checked, error.c_str());
                        if (!scalar_checked) for (const auto& point : placements)
                            std::printf("Wrapped placement: code=%04x x=%.9g y=%.9g\n", point.code_unit, point.x, point.y);
                    }
                    if (!scalar_checked) hr = E_FAIL;
                    // The owner uploaded this exact CPU snapshot. Bounds consume it
                    // without running another scalar pass or writing another quad.
                    for (std::size_t i = 0; SUCCEEDED(hr) && i < placements.size(); ++i) {
                        if (placements[i].code_unit != 0x41) continue; // Spaces/LF have no ink in this fixture.
                        const auto& a = snapshot.vertices[i * 4];
                        const auto& b = snapshot.vertices[i * 4 + 2];
                        const InkBounds bounds{static_cast<int>(std::floor(a.x * 960)) - 1,
                            static_cast<int>(std::floor(a.y * 720)) - 1,
                            static_cast<int>(std::ceil(b.x * 960)) + 1,
                            static_cast<int>(std::ceil(b.y * 720)) + 1};
                        ink_bounds.push_back(bounds);
                        left = (std::min)(left, bounds.left); top = (std::min)(top, bounds.top);
                        right = (std::max)(right, bounds.right); bottom = (std::max)(bottom, bounds.bottom);
                    }
                    if (left < 2 || top < 2 || right >= 254 || bottom >= 254 || left >= right || top >= bottom) hr = E_FAIL;
                }
            }
            auto states = std::make_shared<bsp::RenderStateBlock>();
            for (const auto* script : {&base, &effect}) for (const auto& setting : script->render_states)
                states->states.push_back({static_cast<D3DRENDERSTATETYPE>(setting.state), setting.value});
            if (SUCCEEDED(hr)) {
                state.bind_render_state_block_00b27a80(states);
                state.bind_sampler_state_block_00b27b90(std::make_shared<bsp::SamplerStateBlock>(pass.sampler_states));
            }
            for (const auto& c : vr) if (c.register_set == 2 && SUCCEEDED(hr))
                hr = state.set_vertex_shader_constants_f_00b21820(c.register_index, vwords.data() + c.register_index * 4, c.register_count);
            for (const auto& c : pr) if (c.register_set == 2 && SUCCEEDED(hr))
                hr = state.set_pixel_shader_constants_f_00b218c0(c.register_index, pwords.data() + c.register_index * 4, c.register_count);
            if (SUCCEEDED(hr)) hr = device.Clear(0, nullptr, D3DCLEAR_TARGET, 0xff000000, 1, 0);
            if (SUCCEEDED(hr)) hr = device.BeginScene();
            if (SUCCEEDED(hr)) {
                const auto& range = geometry_owner->geometry().main;
                hr = state.draw_indexed_00b24010({}, range.primitive_type, range.minimum_vertex,
                    range.vertex_count, range.start_index, range.primitive_count);
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
                        bool inside = false;
                        for (auto& bounds : ink_bounds)
                            if (x >= bounds.left && x <= bounds.right && y >= bounds.top && y <= bounds.bottom) {
                                ++bounds.pixels; inside = true;
                            }
                        if (!inside) ++outside;
                    }
                }
                hr = readback.p->UnlockRect();
            }
            if (geometry_owner) {
                const HRESULT unbound = geometry_owner->unbind();
                const bool detached = SUCCEEDED(unbound) && !state.vertex_shader() && !state.pixel_shader();
                if (SUCCEEDED(hr)) hr = detached ? S_OK : E_FAIL;
                std::printf("Installed font owner cleanup: wrapped=%d detached=%d\n", wrapped, detached);
                geometry_owner.reset(); // Owner and registries go before the borrowed cache.
            }
            state.invalidate();
        }
    } catch (const std::exception& exception) {
        std::fprintf(stderr, "Font owner probe exception: %s\n", exception.what());
        hr = E_FAIL;
    }
    bool restored = SUCCEEDED(device.SetRenderTarget(0, old_target.p));
    restored = SUCCEEDED(device.SetDepthStencilSurface(old_depth.p)) && restored;
    restored = SUCCEEDED(saved.p->Apply()) && restored;
    const auto lit_lines = std::count_if(ink_bounds.begin(), ink_bounds.end(),
        [](const InkBounds& bounds) { return bounds.pixels != 0; });
    const bool matched = SUCCEEDED(hr) && lifecycle_checked && visible > 0 && outside == 0 && restored
        && lit_lines == (wrapped ? 3 : 1);
    std::printf("Installed bilinear font draw: descriptor=%s glyph=%s hr=0x%08lx visible=%u outside=%u bounds=%d,%d..%d,%d expected=%d,%d..%d,%d sampler_mask=%u restored=%d lit_lines=%td checked=%d\n",
        descriptor_name.c_str(), wrapped ? "wrapped_A_space_A_LF_A" : "A", static_cast<unsigned long>(hr), visible, outside,
        min_x, min_y, max_x, max_y, left, top, right, bottom, pb.sampler_mask, restored, lit_lines, matched);
    return matched;
}
