#pragma once
#include "bsp/material_samplers.hpp"
#include <memory>

namespace bsp {
struct MaterialShadowSamplerPass {
    std::int32_t shadow_map_slot{-1};     // Native pass +78h.
    std::int32_t shadow_texture_slot{-1}; // Native pass +7Ch.
    // Native pass ctor requests white.tga. Caller supplies its retained logical
    // equivalent; underlying COM lifetime follows the LogicalTexture contract.
    std::shared_ptr<LogicalTexture> fallback_shadow_map; // Native +84h.
};

// Native effect-descriptor name lookup: first equal-length, case-insensitive
// match; return the overall descriptor ordinal, with no stage/source filter.
// This host API accepts ordinary descriptor sizes fitting signed32.
std::int32_t find_shadow_sampler_00b347e0(const ShaderLuaCode& descriptor,
    const char* name);

// Success-path slice of builder00b3b3c0. Run AFTER ordinary state pruning.
// Slot = base pixel-sampler count + effect descriptor's overall name ordinal.
// linear_shadow_map projects builder byte+AAh; its producer is not inferred.
// Preserve both outputs on unsupported selected slots or allocation failure;
// retain the caller-supplied fallback. Native builder ABI is documented in
// docs/MATERIAL_SHADOW_SAMPLERS.md; this typed interface is not native ABI.
bool configure_material_shadow_samplers_00b3c0b2_fragment(
    const ShaderLuaCode& base, const ShaderLuaCode& effect, bool linear_shadow_map,
    MaterialSamplerPass& sampler_pass, MaterialShadowSamplerPass& output);

struct MaterialShadowSamplerInputs {
    // Native global00f8d39c -> owner+3Ch -> texture+8h.
    std::shared_ptr<LogicalTexture> shadow_texture;
    bool has_first_light{};
    // Distinguish no shadow owner (use fallback) from an existing owner whose
    // virtual+8h returns null (bind null). Later lights are not searched.
    bool first_light_has_shadow_owner{};
    std::shared_ptr<LogicalTexture> first_light_shadow_map;
};

// Native00b430cf..00b4315f binds ShadowTexture first, then ShadowMap.
// Caller supplies the observed first-light state and borrowed native texture
// getters as stable logical owners. No shadow-map generation is performed.
// An empty required light list becomes INVALIDCALL instead of native failure.
HRESULT bind_material_shadow_samplers_00b430cf_fragment(D3D9StateCache&,
    const MaterialShadowSamplerPass&, const MaterialShadowSamplerInputs&);
}
