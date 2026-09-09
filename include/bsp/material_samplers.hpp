#pragma once
#include "bsp/d3d9_states.hpp"
#include "bsp/shader_lua.hpp"

namespace bsp {
struct MaterialTextureReference { std::int32_t index{}; bool vertex_stage{}; };
struct MaterialSamplerPass {
    std::vector<MaterialTextureReference> textures;
    SamplerStateBlock sampler_states;
};
struct MaterialSamplerCounters { std::uint32_t references{}, vertex{}, pixel{}; };
void append_material_texture_00b5f100(MaterialSamplerPass&, std::int32_t index, bool vertex_stage);
void set_material_sampler_state_00b5ed60(MaterialSamplerPass&, const SamplerStateValue&);
void remove_material_sampler_slot_00b5eff0(MaterialSamplerPass&, std::uint32_t slot);
// Source0 projection of00b3b280; counters are shared across base/effect calls.
// Unsupported source/index/slot fails before mutation; no texture loader stubs.
bool append_material_samplers_00b3b280(const std::vector<ShaderLuaSampler>&,
    MaterialSamplerPass&, MaterialSamplerCounters&);
//00b3b3c0 cleanup projection; removes unused pixel states, leaves vertex states.
void prune_material_sampler_states(MaterialSamplerPass&, std::uint32_t pixel_usage_mask);
//00b43470..00b4352f static source0 projection. Explicit typed material owner,
// borrowed texture COM lifetimes as in LogicalTexture. Does not bind state blocks.
HRESULT bind_material_textures_00b43470(D3D9StateCache&, const MaterialSamplerPass&,
    const std::vector<std::shared_ptr<LogicalTexture>>& material_textures, std::uint32_t pixel_usage_mask);
}
