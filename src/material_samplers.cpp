#include "bsp/material_samplers.hpp"

namespace bsp {
void append_material_texture_00b5f100(MaterialSamplerPass& pass, std::int32_t index, bool vertex_stage) {
    pass.textures.push_back({index, vertex_stage});
}
void set_material_sampler_state_00b5ed60(MaterialSamplerPass& pass, const SamplerStateValue& value) {
    for (auto& entry : pass.sampler_states.states) {
        if (entry.sampler == value.sampler && entry.state == value.state) {
            if (entry.value != value.value) entry.value = value.value;
            return;
        }
    }
    pass.sampler_states.states.push_back(value);
}
void remove_material_sampler_slot_00b5eff0(MaterialSamplerPass& pass, std::uint32_t slot) {
    auto& entries = pass.sampler_states.states;
    for (;;) {
        std::size_t i = 0;
        while (i < entries.size() && entries[i].sampler != slot) ++i;
        if (i == entries.size()) return;
        if (i + 1 != entries.size()) entries[i] = entries.back();
        entries.pop_back();
    }
}
bool append_material_samplers_00b3b280(const std::vector<ShaderLuaSampler>& samplers,
    MaterialSamplerPass& pass, MaterialSamplerCounters& counters) {
    auto next = counters;
    for (const auto& sampler : samplers) {
        if (sampler.texture_source != 0 || sampler.index < 0) return false;
        auto& stage = sampler.declaration.vertex_stage ? next.vertex : next.pixel;
        if (stage >= (sampler.declaration.vertex_stage ? 4u : 16u) || next.references == UINT32_MAX) return false;
        ++stage; ++next.references;
    }
    for (const auto& sampler : samplers) {
        append_material_texture_00b5f100(pass, sampler.index, sampler.declaration.vertex_stage);
        ++counters.references;
        auto& stage = sampler.declaration.vertex_stage ? counters.vertex : counters.pixel;
        const auto slot = stage + (sampler.declaration.vertex_stage ? 16 : 0);
        for (const auto& state : sampler.sampler_states)
            set_material_sampler_state_00b5ed60(pass, {slot, static_cast<D3DSAMPLERSTATETYPE>(state.state), state.value});
        ++stage;
    }
    return true;
}
void prune_material_sampler_states(MaterialSamplerPass& pass, std::uint32_t usage) {
    for (std::uint32_t slot = 0; slot < 16; ++slot)
        if (!(usage & (1u << slot))) remove_material_sampler_slot_00b5eff0(pass, slot);
}
HRESULT bind_material_textures_00b43470(D3D9StateCache& state, const MaterialSamplerPass& pass,
    const std::vector<std::shared_ptr<LogicalTexture>>& textures, std::uint32_t usage) {
    // Native count is signed16; reject impossible typed owners before calls.
    if (textures.size() > 32767) return D3DERR_INVALIDCALL;
    std::uint32_t vertex = 0, pixel = 0;
    for (const auto& entry : pass.textures) {
        if (entry.index < 0) return D3DERR_INVALIDCALL; // Owner-file path is separate.
        auto& count = entry.vertex_stage ? vertex : pixel;
        if (++count > (entry.vertex_stage ? 4u : 16u)) return D3DERR_INVALIDCALL;
    }
    vertex = 16; pixel = 0;
    for (std::size_t i = 0; i < pass.textures.size(); ++i) {
        const auto& entry = pass.textures[i];
        const auto slot = entry.vertex_stage ? vertex++ : pixel++;
        if (!entry.vertex_stage && !(usage & (1u << (i & 31)))) continue;
        std::shared_ptr<LogicalTexture> texture;
        if (static_cast<std::size_t>(entry.index) < textures.size()) texture = textures[entry.index];
        const HRESULT result = state.bind_texture_00b24710(slot, std::move(texture));
        if (FAILED(result)) return result;
    }
    return D3D_OK;
}
}
