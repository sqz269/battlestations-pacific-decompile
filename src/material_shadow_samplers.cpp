#include "bsp/material_shadow_samplers.hpp"
#include <cstring>
#include <limits>
#include <new>
#include <utility>

namespace bsp {
std::int32_t find_shadow_sampler_00b347e0(const ShaderLuaCode& descriptor,
    const char* name) {
    if (!name || descriptor.samplers.size() > INT32_MAX) return -1;
    const std::size_t length = std::strlen(name);
    for (std::size_t i = 0; i < descriptor.samplers.size(); ++i) {
        const auto& candidate = descriptor.samplers[i].declaration.name;
        if (candidate.size() == length && _stricmp(candidate.c_str(), name) == 0)
            return static_cast<std::int32_t>(i);
    }
    return -1;
}

bool configure_material_shadow_samplers_00b3c0b2_fragment(
    const ShaderLuaCode& base, const ShaderLuaCode& effect, bool linear_shadow_map,
    MaterialSamplerPass& sampler_pass, MaterialShadowSamplerPass& output) {
    if (base.samplers.size() > INT32_MAX || effect.samplers.size() > INT32_MAX)
        return false;
    std::uint64_t base_pixels = 0;
    for (const auto& sampler : base.samplers)
        if (!sampler.declaration.vertex_stage) ++base_pixels;
    const auto map_index = find_shadow_sampler_00b347e0(effect, "ShadowMap");
    const auto texture_index = find_shadow_sampler_00b347e0(effect, "ShadowTexture");
    const auto slot = [base_pixels](std::int32_t index) -> std::int32_t {
        if (index < 0) return -1;
        const std::uint64_t value = base_pixels + static_cast<std::uint32_t>(index);
        return value < 20 ? static_cast<std::int32_t>(value) : -2;
    };
    const auto map_slot = slot(map_index);
    const auto texture_slot = slot(texture_index);
    if (map_slot == -2 || texture_slot == -2) return false;
    try {
        auto next = sampler_pass;
        if (map_slot >= 0) {
            const auto map = static_cast<std::uint32_t>(map_slot);
            const DWORD filter = linear_shadow_map ? D3DTEXF_LINEAR : D3DTEXF_POINT;
            // Exact native replacement order after PS-unused state pruning.
            set_material_sampler_state_00b5ed60(next, {map, D3DSAMP_MAGFILTER, filter});
            set_material_sampler_state_00b5ed60(next, {map, D3DSAMP_MINFILTER, filter});
            set_material_sampler_state_00b5ed60(next, {map, D3DSAMP_MIPFILTER, D3DTEXF_NONE});
            set_material_sampler_state_00b5ed60(next, {map, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP});
            set_material_sampler_state_00b5ed60(next, {map, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP});
        }
        sampler_pass = std::move(next);
        output.shadow_map_slot = map_slot;
        output.shadow_texture_slot = texture_slot;
        return true;
    } catch (const std::bad_alloc&) {
        return false;
    }
}

HRESULT bind_material_shadow_samplers_00b430cf_fragment(D3D9StateCache& states,
    const MaterialShadowSamplerPass& pass, const MaterialShadowSamplerInputs& inputs) {
    const auto valid = [](std::int32_t slot) { return slot >= -1 && slot < 20; };
    if (!valid(pass.shadow_texture_slot) || !valid(pass.shadow_map_slot))
        return D3DERR_INVALIDCALL;
    HRESULT result = D3D_OK;
    if (pass.shadow_texture_slot != -1) {
        result = states.bind_texture_00b24710(static_cast<UINT>(pass.shadow_texture_slot),
            inputs.shadow_texture);
        if (FAILED(result)) return result;
    }
    if (pass.shadow_map_slot != -1) {
        if (!inputs.has_first_light) return D3DERR_INVALIDCALL;
        const auto& texture = inputs.first_light_has_shadow_owner
            ? inputs.first_light_shadow_map : pass.fallback_shadow_map;
        result = states.bind_texture_00b24710(static_cast<UINT>(pass.shadow_map_slot), texture);
    }
    return result;
}
}
