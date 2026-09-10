#pragma once
#include "bsp/material_samplers.hpp"
#include "bsp/shader_reflection.hpp"
#include "bsp/material_shadow_samplers.hpp"
#include "bsp/instance_sort.hpp"
#include <memory>
#include <utility>

namespace bsp {
struct MaterialCloneState;
// One native effect identity spans all fourteen mode programs. Its owned
// fallback at+98 is acquired before the construction serial is consumed.
// Shared ownership keeps metadata/fallback identity common across mode passes
// and material clones; full native effect cache/constructor remains separate.
struct CompiledMaterialEffect {
    CompiledMaterialEffect(MaterialEffectSortMetadata metadata,
        std::shared_ptr<LogicalTexture> fallback)
        : sort_metadata(metadata), fallback_texture_98(std::move(fallback)) {}
    MaterialEffectSortMetadata sort_metadata;
    std::shared_ptr<LogicalTexture> fallback_texture_98;
};
// Typed ownership for one compiled descriptor/combiner pass. This composes the
// recovered source, reflection and state builders; it is not the complete
// native effect loader/cache, fallback policy, or original intrusive ABI.
struct CompiledMaterialPass {
    IDirect3DVertexShader9* vertex{};
    IDirect3DPixelShader9* pixel{};
    std::vector<ReflectedShaderConstant> vr, pr;
    ShaderConstantBindings vb, pb;
    MaterialSamplerPass pass;
    MaterialSamplerCounters sampler_counts;
    MaterialShadowSamplerPass shadow_samplers;
    std::shared_ptr<RenderStateBlock> states = std::make_shared<RenderStateBlock>();
    ShaderLuaCode base, effect;
    std::string engine_program_name;
    std::string vertex_source, pixel_source;
    std::shared_ptr<CompiledMaterialEffect> effect_owner;
    CompiledMaterialPass() = default;
    ~CompiledMaterialPass();
    CompiledMaterialPass(const CompiledMaterialPass&) = delete;
    CompiledMaterialPass& operator=(const CompiledMaterialPass&) = delete;
};
struct MaterialPassCompileSettings {
    std::int32_t mode{};
    std::uint32_t generation{3};
    bool projected_shadow{};
    bool linear_shadow_map{}; // Explicit builder+AA input; producer is unported.
    // Optional actual preconstructed effect identity. All mode passes use the
    // same owner. Construction/fallback acquisition consumes serial BEFORE
    // this call; descriptor fields update even if later compilation fails.
    std::shared_ptr<CompiledMaterialEffect> effect_owner;
};
// Explicit normal/reflection/etc selector and renderer generation, as consumed
// by00b3c3a0/00b3b3c0. Uses the installed D3DX9_40 compiler; retains
// installed Lua/HLSL unchanged. Output unchanged if any stage fails.
bool compile_material_pass(IDirect3DDevice9&, const ShaderScriptResolver&,
    const std::string& descriptor, const MaterialPassCompileSettings&,
    std::shared_ptr<CompiledMaterialPass>&, std::string& error);
// Reads the retained material/effect/first-texture values consumed by00B51DF0.
// The supplied pass must be the material's actual EffectOwner. Missing producer
// metadata is an explicit failure; no pointer/name hash or zero ID is invented.
bool read_compiled_material_sort_fields(const MaterialCloneState&,
    const CompiledMaterialPass&, RenderBatchMaterialKeyFields&, std::string& error);
}
