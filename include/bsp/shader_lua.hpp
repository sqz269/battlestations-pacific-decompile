#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include "bsp/shader_source.hpp"

namespace bsp {
// Host resolver supplies native-requested names explicitly; no guessed overlays.
using ShaderScriptResolver = std::function<bool(const std::string&, std::string&, std::string&)>;
struct ShaderLuaCode {
    std::string constants, vertex, pixel;
    std::optional<std::string> vertex_profile, pixel_profile;
    std::vector<std::string> executed_paths;
    std::vector<ShaderField> vertex_inputs, interpolators;
};
// Adapter for00b6a020/00b69d40 and code-string portion of00b43b00.
// Opens base only, PC=true, supplied X360COMP/REGION, fundamentals then descriptor.
// Uses protected host calls and explicit errors instead of native panic/SEH.
// Successful strings require exact Lua string type. Output unchanged on failure.
bool load_shader_lua_code(const ShaderScriptResolver&, const std::string& descriptor,
    bool x360comp, const std::optional<std::string>& region,
    ShaderLuaCode& output, std::string& error);
}
