#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <array>
#include "bsp/shader_source.hpp"

namespace bsp {
// Host resolver supplies native-requested names explicitly; no guessed overlays.
using ShaderScriptResolver = std::function<bool(const std::string&, std::string&, std::string&)>;
struct ShaderLuaRenderState { std::uint32_t state, value; };
struct ShaderLuaSampler {
    ShaderSamplerDeclaration declaration;
    std::int32_t texture_source{}, index{};
    std::string texture_source_name;
    std::vector<ShaderLuaRenderState> sampler_states, texture_stage_states;
};
// Scalar/string portion of00b43b00. Typed host representation, not native layout.
struct ShaderLuaOptions {
    std::int32_t pipe_id{}, priority{};
    std::string vertex_format{"simple.mvfm"}, shadow_shader, instance_generator;
    bool has_shadow_shader{}, receive_shadows{}, final_lod_fade_out{};
    float final_lod_fade_out_range{0.01f};
    std::int32_t render_target_count{1};
    bool visibility_fade{true}; // Native key is misspelled "VisilityFade".
    bool alpha_to_coverage{}, no_banding_fix{}, disable_alpha_to_coverage{};
    bool compressed_vertices{true};
    std::int32_t compressed_element_count{999};
    bool pixel_position_register{}, output_alpha{true}, lo_res_blend{}, write_depth{};
};
struct ShaderLuaCode {
    ShaderLuaOptions options;
    std::string constants, vertex, pixel;
    std::optional<std::string> vertex_profile, pixel_profile;
    std::vector<std::string> executed_paths;
    std::vector<ShaderField> vertex_inputs, interpolators;
    std::array<std::string, 14> combiners; // native descriptor+48: eight-byte string slots
    std::vector<ShaderLuaRenderState> render_states;
    std::vector<ShaderLuaSampler> samplers;
};
// Adapter for00b6a020/00b69d40 and code-string portion of00b43b00.
// Opens base only, PC=true, supplied X360COMP/REGION, fundamentals then descriptor.
// Uses protected host calls and explicit errors instead of native panic/SEH.
// Code strings require exact STRING; sampler names use native coercion.
// Output unchanged on failure. Texture ownership/binding remains separate.
bool load_shader_lua_code(const ShaderScriptResolver&, const std::string& descriptor,
    bool x360comp, const std::optional<std::string>& region,
    ShaderLuaCode& output, std::string& error);
// Normal base/combiner source-assembly projection of00b3c3a0/00b3b3c0.
// Explicit external render mode/generation/projected selection; no cache,
// shadow-pass pairing, material ownership or compiler lifecycle.
ShaderSourceStatus assemble_shader_programs(const ShaderLuaCode& base, const ShaderLuaCode& effect,
    std::int32_t mode, std::uint32_t generation, bool projected_shadow,
    ShaderVertexProgram&, ShaderPixelProgram&);
}
