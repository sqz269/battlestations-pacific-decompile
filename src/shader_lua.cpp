#include "bsp/shader_lua.hpp"
#include <cmath>
#include <cstring>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace bsp {
namespace {
struct ScriptContext {
    lua_State* state;
    const ShaderScriptResolver& resolve;
    std::vector<std::string> paths;
    std::string error;

    bool execute(const std::string& name) {
        std::string bytes;
        if (!resolve(name, bytes, error)) return false;
        const int top = lua_gettop(state);
        int status = luaL_loadbuffer(state, bytes.data(), bytes.size(), name.c_str());
        if (!status) status = lua_pcall(state, 0, LUA_MULTRET, 0);
        if (status) {
            const char* message = lua_tostring(state, -1);
            error = name + ": " + (message ? message : "Lua execution failed");
        } else paths.push_back(name);
        lua_settop(state, top);
        return status == 0;
    }
};
int do_file(lua_State* state) {
    auto* context = static_cast<ScriptContext*>(lua_touserdata(state, lua_upvalueindex(1)));
    const char* name = lua_tostring(state, 1);
    // execute returns before lua_error: no live C++ temporaries cross longjmp.
    if (name && context->execute(name)) return 0;
    if (!name) context->error = "DoFile requires a filename";
    lua_pushstring(state, context->error.c_str());
    return lua_error(state);
}
std::optional<std::string> string_field(lua_State* state, const char* key) {
    lua_getfield(state, -1, key);
    std::optional<std::string> value;
    if (lua_type(state, -1) == LUA_TSTRING) {
        std::size_t length{};
        const char* text = lua_tolstring(state, -1, &length);
        value = std::string(text, length);
    }
    lua_pop(state, 1);
    return value;
}
bool integer_value(lua_State* state, int index, bool strict, std::int32_t& result) {
    const double number = strict && lua_type(state, index) != LUA_TNUMBER ? 0 : lua_tonumber(state, index);
    const float rounded = static_cast<float>(number);
    if (!std::isfinite(rounded) || rounded < -2147483648.0 || rounded >= 2147483648.0) return false;
    result = static_cast<std::int32_t>(rounded);
    return true;
}
bool integer_key(lua_State* state, int index) {
    std::int32_t key{};
    return lua_type(state, index) == LUA_TNUMBER && integer_value(state, index, true, key)
        && static_cast<float>(lua_tonumber(state, index)) == static_cast<float>(key);
}
bool read_options(lua_State* state, ShaderLuaOptions& output, std::string& error) {
    const auto boolean = [&](const char* key, bool& value) {
        lua_getfield(state, -1, key);
        if (lua_type(state, -1) == LUA_TBOOLEAN) value = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
    };
    const auto integer = [&](const char* key, std::int32_t& value) {
        lua_getfield(state, -1, key);
        const bool ok = lua_type(state, -1) != LUA_TNUMBER || integer_value(state, -1, true, value);
        lua_pop(state, 1);
        if (!ok) error = std::string(key) + ": unsupported numeric conversion";
        return ok;
    };
    const auto string = [&](const char* key, std::string& value) {
        lua_getfield(state, -1, key);
        if (lua_type(state, -1) == LUA_TSTRING) value = lua_tostring(state, -1);
        lua_pop(state, 1);
    };
    // Preserve native lookup order, including its exact type gates and defaults.
    if (!integer("PipeID", output.pipe_id) || !integer("Priority", output.priority)) return false;
    string("VertexFormat", output.vertex_format);
    boolean("ReceiveShadows", output.receive_shadows);
    string("ShadowShader", output.shadow_shader);
    output.has_shadow_shader = !output.shadow_shader.empty();
    boolean("FinalLODFadeOut", output.final_lod_fade_out);
    lua_getfield(state, -1, "FinalLODFadeOutRange");
    if (lua_type(state, -1) == LUA_TNUMBER)
        output.final_lod_fade_out_range = static_cast<float>(lua_tonumber(state, -1));
    lua_pop(state, 1);
    if (!integer("RTCount", output.render_target_count)) return false;
    boolean("VisilityFade", output.visibility_fade);
    boolean("AlphaToCoverage", output.alpha_to_coverage);
    boolean("NoBandingFix", output.no_banding_fix);
    boolean("DisableAlphaToCoverage", output.disable_alpha_to_coverage);
    boolean("CompressedVertices", output.compressed_vertices);
    if (!integer("CompressedElemCount", output.compressed_element_count)) return false;
    string("InstanceGenerator", output.instance_generator);
    boolean("PixelPositionRegister", output.pixel_position_register);
    boolean("OutputAlpha", output.output_alpha);
    boolean("LoResBlend", output.lo_res_blend);
    boolean("WriteDepth", output.write_depth);
    return true;
}
struct StateDefinition { const char* key; std::uint32_t id, tag; };
const StateDefinition render_definitions[] = {
#include "shader_render_state_registry.inc"
};
const StateDefinition sampler_definitions[] = {
#include "shader_sampler_state_registry.inc"
};
const StateDefinition texture_stage_definitions[] = {
#include "shader_texture_stage_state_registry.inc"
};
template<std::size_t N>
bool read_states(lua_State* state, const char* key, const StateDefinition (&definitions)[N],
    std::vector<ShaderLuaRenderState>& output, std::string& error) {
    const int saved = lua_gettop(state);
    lua_getfield(state, -1, key);
    if (!lua_istable(state, -1)) { lua_settop(state, saved); return true; }
    for (const auto& definition : definitions) {
        lua_getfield(state, -1, definition.key);
        const bool numeric = lua_type(state, -1) == LUA_TNUMBER;
        lua_pop(state, 1);
        if (!numeric) continue;
        lua_getfield(state, -1, definition.key); // Native performs a fresh lookup after type checking.
        std::uint32_t bits{};
        if (definition.tag == 0) {
            std::int32_t value{};
            if (!integer_value(state, -1, false, value)) { error = "Unsupported render-state integer"; return false; }
            bits = static_cast<std::uint32_t>(value);
        } else {
            const float value = static_cast<float>(lua_tonumber(state, -1));
            std::memcpy(&bits, &value, sizeof(bits));
        }
        lua_pop(state, 1);
        bool existing = false;
        for (const auto& entry : output) if (entry.state == definition.id) existing = true;
        if (!existing) output.push_back({definition.id, bits});
    }
    lua_settop(state, saved);
    return true;
}
bool read_combiners(lua_State* state, std::array<std::string, 14>& output, std::string& error) {
    const int saved = lua_gettop(state);
    lua_getfield(state, -1, "Combiners");
    if (!lua_istable(state, -1)) { lua_settop(state, saved); return true; }
    const int list = lua_gettop(state);
    lua_pushnil(state);
    while (lua_next(state, list)) {
        if (integer_key(state, -2)) {
            if (!lua_istable(state, -1)) { error = "Combiner value is not a table"; return false; }
            const int record = lua_gettop(state);
            unsigned ordinal = 0;
            std::int32_t mode{};
            bool have_mode = false;
            std::string name;
            lua_pushnil(state);
            while (lua_next(state, record)) {
                if (integer_key(state, -2)) {
                    if (ordinal == 0) {
                        mode = 13;
                        if (lua_type(state, -1) == LUA_TNUMBER && !integer_value(state, -1, true, mode)) {
                            error = "Unsupported combiner mode conversion"; return false;
                        }
                        have_mode = true;
                    } else if (ordinal == 1 && lua_type(state, -1) == LUA_TSTRING) {
                        name = lua_tostring(state, -1);
                    }
                }
                ++ordinal; // Unlike field records, every entry advances native ordinal.
                lua_pop(state, 1);
            }
            if (!have_mode || mode < 0 || static_cast<std::size_t>(mode) >= output.size()) {
                error = "Combiner has no valid destination slot"; return false;
            }
            output[static_cast<std::size_t>(mode)] = std::move(name);
        }
        lua_pop(state, 1);
    }
    lua_settop(state, saved);
    return true;
}
bool read_samplers(lua_State* state, std::vector<ShaderLuaSampler>& output, std::string& error) {
    const int saved = lua_gettop(state);
    lua_getfield(state, -1, "Samplers");
    const bool present = lua_istable(state, -1) != 0;
    lua_pop(state, 1);
    if (!present) return true;
    lua_getfield(state, -1, "Samplers"); // Native fetches again after its gate.
    if (!lua_istable(state, -1)) { error = "Samplers changed to a non-table"; return false; }
    const int list = lua_gettop(state);
    lua_pushnil(state);
    while (lua_next(state, list)) {
        if (integer_key(state, -2)) {
            if (!lua_istable(state, -1)) { error = "Sampler value is not a table"; return false; }
            ShaderLuaSampler sampler;
            const auto name = [&](const char* key, std::string& value) {
                lua_getfield(state, -1, key);
                const char* text = lua_tostring(state, -1); // Coerces numbers; native copies through NUL.
                if (text) value = text;
                lua_pop(state, 1);
                if (!text) error = std::string("Sampler ") + key + " is not string-convertible";
                return text != nullptr;
            };
            const auto optional_integer = [&](const char* key, std::int32_t& value) {
                lua_getfield(state, -1, key);
                const bool accepted = integer_key(state, -1);
                lua_pop(state, 1);
                if (!accepted) return true;
                lua_getfield(state, -1, key);
                const bool ok = integer_value(state, -1, false, value);
                lua_pop(state, 1);
                if (!ok) error = std::string("Unsupported sampler ") + key + " conversion";
                return ok;
            };
            if (!name("Name", sampler.declaration.name)) return false;
            lua_getfield(state, -1, "Type");
            std::int32_t dimension{};
            const bool dimension_ok = integer_value(state, -1, false, dimension);
            lua_pop(state, 1);
            if (!dimension_ok) { error = "Unsupported sampler Type conversion"; return false; }
            sampler.declaration.dimension = static_cast<std::uint32_t>(dimension);
            if (!optional_integer("TextureSource", sampler.texture_source)) return false;
            if ((sampler.texture_source == 1 || sampler.texture_source == 3)
                && !name("TextureSourceName", sampler.texture_source_name)) return false;
            if (!optional_integer("Index", sampler.index)) return false;
            lua_getfield(state, -1, "VertexSampler");
            const bool boolean = lua_type(state, -1) == LUA_TBOOLEAN;
            lua_pop(state, 1);
            if (boolean) {
                lua_getfield(state, -1, "VertexSampler");
                sampler.declaration.vertex_stage = lua_type(state, -1) == LUA_TBOOLEAN
                    && lua_toboolean(state, -1) != 0;
                lua_pop(state, 1);
            }
            // Each native state-list owner tests TABLE then obtains a fresh reference.
            const auto states = [&](const char* key, const auto& definitions, auto& values) {
                lua_getfield(state, -1, key);
                const bool table = lua_istable(state, -1) != 0;
                lua_pop(state, 1);
                return !table || read_states(state, key, definitions, values, error);
            };
            if (!states("SamplerStates", sampler_definitions, sampler.sampler_states)
                || !states("TextureStageStates", texture_stage_definitions, sampler.texture_stage_states)) return false;
            output.push_back(std::move(sampler));
        }
        lua_pop(state, 1);
    }
    lua_settop(state, saved);
    return true;
}
bool read_fields(lua_State* state, const char* key, std::vector<ShaderField>& output, std::string& error) {
    const int saved = lua_gettop(state);
    lua_getfield(state, -1, key);
    if (!lua_istable(state, -1)) { lua_settop(state, saved); return true; }
    const int list = lua_gettop(state);
    lua_pushnil(state);
    while (lua_next(state, list)) {
        if (!lua_istable(state, -1)) { error = std::string(key) + ": field value is not a table"; return false; }
        const int record = lua_gettop(state);
        ShaderField field;
        field.name.clear(); field.semantic_index = 0; field.component_mask = 0;
        unsigned ordinal = 0;
        lua_pushnil(state);
        while (lua_next(state, record)) {
            const bool accepted = integer_key(state, -2);
            if (accepted) {
                if (ordinal == 0) field.name = lua_type(state, -1) == LUA_TSTRING ? lua_tostring(state, -1) : "undef";
                else if (ordinal < 5) {
                    std::int32_t value{};
                    if (!integer_value(state, -1, ordinal == 2 || ordinal == 4, value)) {
                        error = std::string(key) + ": unsupported numeric conversion"; return false;
                    }
                    switch (ordinal) {
                    case 1: field.scalar_type = static_cast<ShaderScalarType>(value); break;
                    case 2: field.component_count = static_cast<std::uint32_t>(value); break;
                    case 3: field.semantic = static_cast<ShaderSemantic>(value); break;
                    case 4: field.semantic_index = static_cast<std::uint32_t>(value); break;
                    }
                }
                ++ordinal;
            }
            lua_pop(state, 1);
        }
        if (ordinal < 4) { error = std::string(key) + ": incomplete field record"; return false; }
        output.push_back(std::move(field));
        lua_pop(state, 1);
    }
    lua_settop(state, saved);
    return true;
}
}
bool load_shader_lua_code(const ShaderScriptResolver& resolver, const std::string& descriptor,
    bool x360comp, const std::optional<std::string>& region,
    ShaderLuaCode& output, std::string& error) {
    lua_State* state = luaL_newstate();
    if (!state) { error = "Lua state allocation failed"; return false; }
    ScriptContext context{state, resolver, {}, {}};
    lua_pushcfunction(state, luaopen_base);
    lua_pushliteral(state, "");
    const int opened = lua_pcall(state, 1, 0, 0);
    bool ok = opened == 0;
    if (!ok) context.error = "Lua base initialization failed";
    if (ok) {
        lua_pushboolean(state, 1); lua_setglobal(state, "PC");
        lua_pushboolean(state, x360comp); lua_setglobal(state, "X360COMP");
        if (region) { lua_pushlstring(state, region->data(), region->size()); lua_setglobal(state, "REGION"); }
        lua_pushlightuserdata(state, &context);
        lua_pushcclosure(state, do_file, 1); lua_setglobal(state, "DoFile");
        ok = context.execute("Scripts\\fundamentals.lua") && context.execute(descriptor);
    }
    ShaderLuaCode loaded;
    if (ok) {
        lua_getglobal(state, "Shader");
        ok = lua_istable(state, -1) != 0;
        if (!ok) context.error = "Descriptor did not produce a Shader table";
        else {
            loaded.constants = string_field(state, "Constants").value_or("");
            loaded.vertex = string_field(state, "VS").value_or("");
            loaded.pixel = string_field(state, "PS").value_or("");
            loaded.vertex_profile = string_field(state, "VSVersion");
            loaded.pixel_profile = string_field(state, "PSVersion");
            loaded.executed_paths = context.paths;
            ok = read_options(state, loaded.options, context.error)
                && read_fields(state, "VertexInput", loaded.vertex_inputs, context.error)
                && read_fields(state, "Interpolators", loaded.interpolators, context.error)
                && read_combiners(state, loaded.combiners, context.error)
                && read_states(state, "RenderStates", render_definitions, loaded.render_states, context.error)
                && read_samplers(state, loaded.samplers, context.error);
        }
    }
    lua_close(state);
    if (!ok) { error = context.error; return false; }
    output = std::move(loaded); error.clear(); return true;
}
ShaderSourceStatus assemble_shader_programs(const ShaderLuaCode& base, const ShaderLuaCode& effect,
    std::int32_t mode, std::uint32_t generation, bool projected_shadow,
    ShaderVertexProgram& vertex_output, ShaderPixelProgram& pixel_output) {
    ShaderVertexProgram vertex;
    append_vertex_inputs_00b35930(base.vertex_inputs, effect.vertex_inputs, vertex.inputs);
    append_vertex_system_fields_00b35be0(vertex.system_values);
    const auto selected = append_selected_interpolators_00b36800(
        base.interpolators, effect.interpolators, nullptr, nullptr, vertex.outputs);
    if (selected != ShaderSourceStatus::complete) return selected;
    vertex.packing_fields = vertex.outputs;
    append_interpolator_mapping_00b34aa0(vertex.outputs, vertex.interpolators);
    vertex.constants = make_system_constant_registry_00b5bf70();
    vertex.register_limit = system_constant_annotation_limit;
    vertex.base.header = base.constants; vertex.effect.header = effect.constants;
    vertex.base.vertex_code = base.vertex; vertex.effect.vertex_code = effect.vertex;
    vertex.effect.render_mode = mode;
    vertex.effect.shadow_helper = effect.options.receive_shadows;
    vertex.base.decode_inputs = base.options.compressed_vertices;
    vertex.base.decode_field_limit = static_cast<std::uint32_t>(base.options.compressed_element_count);
    for (const auto& sampler : base.samplers) vertex.base.samplers.push_back(sampler.declaration);
    for (const auto& sampler : effect.samplers) vertex.effect.samplers.push_back(sampler.declaration);
    ShaderPixelProgram pixel;
    pixel.inputs = vertex.outputs; pixel.unpack_fields = vertex.outputs;
    append_pixel_system_fields_00b372d0(pixel.system_values);
    pixel.interpolators = vertex.interpolators;
    pixel.constants = vertex.constants; pixel.register_limit = vertex.register_limit;
    pixel.base.header = base.constants; pixel.effect.header = effect.constants;
    pixel.base.pixel_code = base.pixel; pixel.effect.pixel_code = effect.pixel;
    pixel.base.samplers = vertex.base.samplers; pixel.effect.samplers = vertex.effect.samplers;
    pixel.effect.render_mode = mode;
    pixel.effect.shadow_helpers = effect.options.receive_shadows;
    pixel.effect.alpha_override = effect.options.output_alpha;
    pixel.base.vpos = base.options.pixel_position_register; pixel.effect.vpos = effect.options.pixel_position_register;
    pixel.base.suppress_time_transform = base.options.no_banding_fix;
    pixel.base.premultiply_alpha = base.options.lo_res_blend;
    pixel.color_outputs = static_cast<std::uint32_t>(effect.options.render_target_count);
    pixel.depth_output = effect.options.write_depth;
    pixel.visibility_alpha = base.options.visibility_fade;
    pixel.zero_fog = generation < 3; pixel.projected_shadow = projected_shadow;
    vertex_output = std::move(vertex); pixel_output = std::move(pixel);
    return ShaderSourceStatus::complete;
}
}
