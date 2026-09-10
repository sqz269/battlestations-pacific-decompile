#pragma once
#include "bsp/material_samplers.hpp"
#include "bsp/shader_reflection.hpp"
#include "bsp/material_shadow_samplers.hpp"
#include <memory>

namespace bsp {
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
};
// Explicit normal/reflection/etc selector and renderer generation, as consumed
// by00b3c3a0/00b3b3c0. Uses the installed D3DX9_40 compiler; retains
// installed Lua/HLSL unchanged. Output unchanged if any stage fails.
bool compile_material_pass(IDirect3DDevice9&, const ShaderScriptResolver&,
    const std::string& descriptor, const MaterialPassCompileSettings&,
    std::shared_ptr<CompiledMaterialPass>&, std::string& error);
}
