#include "bsp/compiled_material.hpp"
#include "bsp/material_clone.hpp"
#include <d3dx9shader.h>
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
    HMODULE value = LoadLibraryExW(L"d3dx9_40.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    ~CompilerModule() { if (value) FreeLibrary(value); }
};
}
namespace bsp {
bool read_compiled_material_sort_fields(const MaterialCloneState& material,
    const CompiledMaterialPass& pass, RenderBatchMaterialKeyFields& output,
    std::string& error) {
    error.clear();
    if (material.effect.get() != &pass || !pass.effect_owner
        || !pass.effect_owner->sort_metadata.descriptor_assigned) {
        error = "Material key requires its retained effect and loaded descriptor metadata";
        return false;
    }
    RenderBatchMaterialKeyFields fields;
    const auto& effect = pass.effect_owner->sort_metadata;
    fields.effect_b0 = effect.priority_b0;
    fields.effect_c0 = static_cast<std::uint8_t>(effect.construction_serial_c0);
    fields.material_count34 = material.textures.count();
    if (fields.material_count34 > 0 && material.textures.textures()[0]) {
        const auto& texture = *material.textures.textures()[0];
        if (!texture.sort_metadata) {
            error = "Material key first texture has no construction metadata";
            return false;
        }
        fields.texture0_present = true;
        fields.texture20 = texture.sort_metadata->construction_serial20;
    }
    output = fields; return true;
}
CompiledMaterialPass::~CompiledMaterialPass() {
    if (pixel) pixel->Release();
    if (vertex) vertex->Release();
}
bool compile_material_pass(IDirect3DDevice9& device, const ShaderScriptResolver& resolver,
    const std::string& descriptor, const MaterialPassCompileSettings& settings,
    std::shared_ptr<CompiledMaterialPass>& output, std::string& error) {
    const auto mode = settings.mode;
    const auto generation = settings.generation;
    const auto projected_shadow = settings.projected_shadow;
    error = "Material compiler could not complete the selected pass.";
    ShaderLuaCode base, effect;
    if (mode < 0 || mode >= 14 || descriptor.find('\0') != std::string::npos
        || !load_shader_lua_code(resolver, descriptor, false, {}, base, error)) return false;
    if (settings.effect_owner)
        apply_material_effect_sort_descriptor_00b45ee0(settings.effect_owner->sort_metadata, base.options);
    if (base.combiners[static_cast<std::size_t>(mode)].empty()
        || !load_shader_lua_code(resolver, base.combiners[static_cast<std::size_t>(mode)], false, {}, effect, error)) return false;
    bsp::ShaderVertexProgram vp;
    bsp::ShaderPixelProgram pp;
    if (bsp::assemble_shader_programs(base, effect, mode, generation, projected_shadow, vp, pp) != bsp::ShaderSourceStatus::complete) return false;
    std::string vs_source, ps_source;
    if (bsp::generate_pixel_source_00b39880(pp, ps_source) != bsp::ShaderSourceStatus::complete) return false;
    CompilerModule module;
    if (!module.value) return false;
    decltype(&D3DXCompileShader) compile{};
    decltype(&D3DXDisassembleShader) disassemble{};
    const FARPROC compile_address = GetProcAddress(module.value, "D3DXCompileShader");
    const FARPROC disassemble_address = GetProcAddress(module.value, "D3DXDisassembleShader");
    std::memcpy(&compile, &compile_address, sizeof(compile));
    std::memcpy(&disassemble, &disassemble_address, sizeof(disassemble));
    if (!compile || !disassemble) return false;
    const auto profiles = bsp::select_shader_profiles_00b43b00(generation, base.vertex_profile, base.pixel_profile);
    const auto compile_source = [&](const std::string& source, const std::string& profile, DWORD flags, ID3DXBuffer** bytecode) {
        if (source.size() > std::numeric_limits<UINT>::max() || source.find('\0') != std::string::npos) {
            error = "Shader source exceeds native strlen/UINT success domain.";
            return E_INVALIDARG;
        }
        OwnedCom<ID3DXBuffer> errors;
        const HRESULT hr = compile(source.data(), static_cast<UINT>(source.size()),
            nullptr, nullptr, "main", profile.c_str(), flags, bytecode, &errors.p, nullptr);
        if (FAILED(hr) && errors.p) error.assign(static_cast<const char*>(errors.p->GetBufferPointer()), errors.p->GetBufferSize());
        return hr;
    };
    OwnedCom<ID3DXBuffer> ps, vs, assembly;
    //00b45ee0/00b3c3a0 retain the full base filename stem and append mode,T/F,3.
    // The compiler's case-sensitive shore test uses this name, not the combiner.
    const auto dot = descriptor.find_last_of('.');
    const auto engine_name = descriptor.substr(0,dot) + std::to_string(mode)
        + (projected_shadow ? "T" : "F") + "3";
    const DWORD pixel_flags = engine_name.find("shore") != std::string::npos ? 0 : 0x1400;
    if (FAILED(compile_source(ps_source, profiles.pixel, pixel_flags, &ps.p))
        || FAILED(disassemble(static_cast<const DWORD*>(ps.p->GetBufferPointer()), FALSE, nullptr, &assembly.p))) return false;
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
        || FAILED(compile_source(vs_source, profiles.vertex, 0x1200, &vs.p))) return false;
    auto owner = std::make_shared<CompiledMaterialPass>();
    owner->effect_owner = settings.effect_owner;
    auto& vr = owner->vr; auto& pr = owner->pr;
    auto& vb = owner->vb; auto& pb = owner->pb;
    const auto registry = bsp::make_system_constant_registry_00b5bf70();
    if (!bsp::reflect_shader_constants_00b3aea0(static_cast<const std::uint32_t*>(vs.p->GetBufferPointer()), vr, error)
        || !bsp::reflect_shader_constants_00b3aea0(static_cast<const std::uint32_t*>(ps.p->GetBufferPointer()), pr, error)
        || !bsp::map_shader_constants_00b3aea0(vr, registry, vb)
        || !bsp::map_shader_constants_00b3aea0(pr, registry, pb)) return false;
    auto& pass = owner->pass;
    auto& counters = owner->sampler_counts;
    if (!bsp::append_material_samplers_00b3b280(base.samplers, pass, counters)
        || !bsp::append_material_samplers_00b3b280(effect.samplers, pass, counters)) return false;
    bsp::prune_material_sampler_states(pass, pb.sampler_mask);
    if (!configure_material_shadow_samplers_00b3c0b2_fragment(base, effect,
        settings.linear_shadow_map, pass, owner->shadow_samplers)) return false;
    for (const auto* script : {&base, &effect}) for (const auto& setting : script->render_states)
        owner->states->states.push_back({static_cast<D3DRENDERSTATETYPE>(setting.state), setting.value});
    if (FAILED(device.CreateVertexShader(static_cast<const DWORD*>(vs.p->GetBufferPointer()), &owner->vertex))
        || FAILED(device.CreatePixelShader(static_cast<const DWORD*>(ps.p->GetBufferPointer()), &owner->pixel))) return false;
    owner->base = std::move(base); owner->effect = std::move(effect);
    owner->engine_program_name = engine_name;
    owner->vertex_source = std::move(vs_source); owner->pixel_source = std::move(ps_source);
    if (owner->effect_owner)
        (*owner->effect_owner->parameter_selectors)[static_cast<std::size_t>(mode)] =
            MaterialParameterSelector{owner->vb, owner->pb};
    output = std::move(owner); error.clear();
    return true;
}
bool assign_compiled_material_effect_00b19210_fragment(MaterialCloneState& material,
    std::shared_ptr<const CompiledMaterialPass> pass, std::string& error) {
    if (!pass) {
        if (!material.parameters.set_shader_00b19210_fragment({}, {}, error)) return false;
        material.effect.reset();
        return true;
    }
    if (!pass->effect_owner || !pass->effect_owner->parameter_selectors) {
        error = "Material assignment requires the actual compiled effect and mode metadata";
        return false;
    }
    if (!material.parameters.set_shared_shader_00b19210_fragment(pass->effect_owner,
        pass->effect_owner->parameter_selectors, error)) return false;
    pass->effect_owner->parameter_dirty_b4 = 1;
    material.effect = std::move(pass);
    return true;
}
}
