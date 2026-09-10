// Installed-source diagnostic: recovered search lists, supplied loose mount.
#include "bsp/font_geometry_owner.hpp"
#include "bsp/d3d9_texture.hpp"
#include "bsp/material_textures.hpp"
#include "bsp/material_samplers.hpp"
#include "bsp/material_constants.hpp"
#include "bsp/material_parameter_bindings.hpp"
#include "bsp/effect_cache.hpp"
#include "bsp/shader_reflection.hpp"
#include "bsp/resource_path.hpp"
#include "asset_stream_probe.hpp"
#include "bsp/compiled_material.hpp"
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
using FontShaderOwner = bsp::CompiledMaterialPass;

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

bool font_material_constants(const bsp::EffectOwner& shaders,
    const bsp::ShaderConstantBindings& vb, const bsp::ShaderConstantBindings& pb,
    std::vector<float>& vwords, std::vector<float>& pwords, std::string& error) {
    // Explicit inputs for this installed-font draw: resolved alpha scale 1,
    // unclipped viewport and base context colors. This is not a reconstructed
    // context constructor or the full native renderer effect loader.
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

// Installed-font constraints around the shared material pass compiler.
std::shared_ptr<FontShaderOwner> load_font_effect(AssetStreamProbe& assets, IDirect3DDevice9& device,
    const std::string& canonical_name, std::string& error) {
    const bsp::ShaderScriptResolver resolver = [&](const std::string& requested, std::string& bytes, std::string& message) {
        std::shared_ptr<bsp::MemoryStream> stream;
        std::string logical;
        if (!assets.read(requested, stream, message, &logical)) return false;
        std::printf("Mounted font shader lookup: %s -> %s\n", requested.c_str(), logical.c_str());
        bytes.assign(reinterpret_cast<const char*>(stream->data_00bef610()),
            static_cast<std::size_t>(stream->size_00bef600()));
        return true;
    };
    std::shared_ptr<FontShaderOwner> owner;
    if (!bsp::compile_material_pass(device, resolver, canonical_name, {0,3,false,false}, owner, error)
        || owner->base.combiners[0] != "dummy.shfx" || owner->sampler_counts.pixel != 2
        || owner->sampler_counts.vertex != 0 || owner->pb.sampler_mask != 1) return {};
    return owner;
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
    std::string error;
    unsigned loads = 0;
    bsp::EffectCache cache({
        [&](std::string& name) { std::string ignored; return assets.resolve(name, ignored); },
        [&](const std::string& name, std::string& load_error) -> bsp::EffectOwner {
            ++loads;
            return load_font_effect(assets, device, name, load_error);
        },
        // Material effect vtable00d61a00+Ch ->00a82250: XOR EAX,EAX; RET.
        [](const void*) -> std::uint32_t { return 0; }
    });
    auto shaders = cache.acquire_00b318b0_fragment(selected_name, error);
    if (!shaders || cache.size() != 1) {
        std::fprintf(stderr, "Font effect cache: %s\n", error.c_str()); return false;
    }
    auto alias = cache.acquire_00b318b0_fragment("GUIFONTBILINEAR.MSHD", error);
    const auto canonical_name = cache.entry(0)->canonical_name;
    auto canonical = cache.acquire_00b318b0_fragment(canonical_name, error);
    if (alias != shaders || canonical != shaders || loads != 1 || cache.size() != 1
        || cache.accounted_size() != 0) return false;
    const auto alias_count = cache.entry(0)->aliases.size();
    alias.reset(); canonical.reset();
    const std::weak_ptr<const void> retained = shaders;
    cache.clear_00b31750_fragment();
    if (cache.size() || cache.accounted_size() || retained.expired() || shaders.use_count() != 1) return false;
    std::printf("Installed font effect cache: loads=%u aliases=%zu canonical=%s accounting=0 retained_after_clear=1\n",
        loads, alias_count, canonical_name.c_str());
    const auto& font_effect = *static_cast<const FontShaderOwner*>(shaders.get());
    const auto& vr = font_effect.vr; const auto& pr = font_effect.pr;
    const auto& vb = font_effect.vb; const auto& pb = font_effect.pb;
    const auto& pass = font_effect.pass;
    std::vector<float> vwords(256 * 4), pwords(224 * 4);
    if (!font_constants(vr, vb, vwords, true) || !font_constants(pr, pb, pwords, false)) return false;
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
        if (SUCCEEDED(hr) && !font_material_constants(shaders, vb, pb, vwords, pwords, error)) {
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
                    {resources, font_effect.vertex, font_effect.pixel, pass, pb.sampler_mask});
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
            if (SUCCEEDED(hr)) {
                state.bind_render_state_block_00b27a80(font_effect.states);
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
