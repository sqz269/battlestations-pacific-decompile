#include "bsp/font_registry.hpp"
#include <cstring>
#include <memory>
#include <utility>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace bsp {
namespace {
struct ScriptContext {
    lua_State* state;
    const FontScriptResolver& resolver;
    std::vector<std::string> paths;
    std::string error;
    bool execute(const std::string& path) {
        std::string bytes;
        if (!resolver(path, bytes, error)) return false;
        const int top = lua_gettop(state);
        int status = luaL_loadbuffer(state, bytes.data(), bytes.size(), path.c_str());
        if (status == 0) status = lua_pcall(state, 0, LUA_MULTRET, 0);
        if (status != 0) {
            const char* message = lua_tostring(state, -1);
            error = path + ": " + (message ? message : "Lua execution failed");
        } else paths.push_back(path);
        lua_settop(state, top);
        return status == 0;
    }
};
int do_file(lua_State* state) {
    auto* context = static_cast<ScriptContext*>(lua_touserdata(state, lua_upvalueindex(1)));
    const char* name = lua_tostring(state, 1);
    if (name && context->execute(name)) return 0;
    if (!name) context->error = "DoFile requires a filename";
    // execute has returned: its C++ temporaries do not cross lua_error's jump.
    lua_pushstring(state, context->error.c_str());
    return lua_error(state);
}
bool ordinary_table(lua_State* state, int index) {
    if (lua_type(state, index) != LUA_TTABLE) return false;
    if (!lua_getmetatable(state, index)) return true;
    lua_pop(state, 1);
    return false;
}
void field(lua_State* state, int table, const char* key) {
    lua_pushstring(state, key);
    lua_rawget(state, table);
}
float float_field(lua_State* state, int table, const char* key) {
    field(state, table, key);
    const float value = lua_type(state, -1) == LUA_TNUMBER
        ? static_cast<float>(lua_tonumber(state, -1)) : 1.0f;
    lua_pop(state, 1);
    return value;
}
bool required_string(lua_State* state, int index, std::string& output) {
    // Native 00b662b0 calls Lua's string conversion, including numeric coercion.
    const char* value = lua_tostring(state, index);
    if (!value) return false;
    output = value; // Native strlen/copy truncates at embedded NUL.
    return true;
}
bool read_descriptor(lua_State* state, FontDescriptor& output, std::string& error) {
    const int entry = lua_gettop(state);
    if (!ordinary_table(state, entry)) {
        error = "Each Fonts entry must be an ordinary table.";
        return false;
    }
    output.scale_ratio = float_field(state, entry, "scale_ratio");
    output.alpha_texture_scale = float_field(state, entry, "alphatexturescale");
    field(state, entry, "uppercase_only");
    output.uppercase_only = lua_type(state, -1) == LUA_TBOOLEAN
        && lua_toboolean(state, -1) != 0;
    lua_pop(state, 1);

    field(state, entry, "datafiles");
    const int files = lua_gettop(state);
    if (!ordinary_table(state, files)) {
        error = "Font datafiles must be an ordinary table.";
        return false;
    }
    // The original repeats integer index 1 for each field, ignoring later rows.
    const char* keys[] = {"Data", "GFX", "AlphaTexture"};
    std::string* outputs[] = {&output.data_file, &output.gfx_file, &output.alpha_texture};
    for (unsigned i = 0; i < 3; ++i) {
        lua_pushnumber(state, 1);
        lua_rawget(state, files);
        const int record = lua_gettop(state);
        if (!ordinary_table(state, record)) {
            error = "Font datafiles[1] must be an ordinary table.";
            return false;
        }
        field(state, record, keys[i]);
        bool ok = true;
        if (i == 2) {
            if (lua_type(state, -1) == LUA_TSTRING) *outputs[i] = lua_tostring(state, -1);
        } else ok = required_string(state, -1, *outputs[i]);
        lua_pop(state, 2);
        if (!ok) {
            error = std::string("Font datafiles[1].") + keys[i] + " requires a string or number.";
            return false;
        }
    }
    lua_pop(state, 1);
    return true;
}
}

bool load_font_registry_lua(const FontScriptResolver& resolver,
    const std::string& descriptor, bool x360comp,
    const std::optional<std::string>& region, FontRegistry& output,
    std::string& error) {
    error.clear();
    std::unique_ptr<lua_State, decltype(&lua_close)> owner(luaL_newstate(), &lua_close);
    if (!owner) { error = "Lua state allocation failed"; return false; }
    auto* state = owner.get();
    ScriptContext context{state, resolver, {}, {}};
    lua_pushcfunction(state, luaopen_base);
    lua_pushliteral(state, "");
    if (lua_pcall(state, 1, 0, 0) != 0) {
        error = "Lua base initialization failed";
        return false;
    }
    lua_pushboolean(state, 1); lua_setglobal(state, "PC");
    lua_pushboolean(state, x360comp); lua_setglobal(state, "X360COMP");
    if (region) {
        lua_pushlstring(state, region->data(), region->size());
        lua_setglobal(state, "REGION");
    }
    lua_pushlightuserdata(state, &context);
    lua_pushcclosure(state, do_file, 1); lua_setglobal(state, "DoFile");
    if (!context.execute("Scripts\\fundamentals.lua") || !context.execute(descriptor)) {
        error = context.error;
        return false;
    }
    if (!ordinary_table(state, LUA_GLOBALSINDEX)) {
        error = "Font script global environment must be an ordinary table.";
        return false;
    }
    field(state, LUA_GLOBALSINDEX, "Fonts");
    const int fonts = lua_gettop(state);
    if (!ordinary_table(state, fonts)) {
        error = "Descriptor did not produce an ordinary Fonts table.";
        return false;
    }
    FontRegistry loaded = output;
    lua_pushnil(state);
    while (lua_next(state, fonts)) {
        FontDescriptor font;
        // Convert a copy so number-to-string coercion does not corrupt lua_next's key.
        lua_pushvalue(state, -2);
        const bool named = required_string(state, -1, font.name);
        lua_pop(state, 1);
        if (!named) {
            error = "Font names require string or numeric table keys.";
            return false;
        }
        if (!read_descriptor(state, font, error)) return false;
        loaded.fonts.push_back(std::move(font));
        lua_pop(state, 1);
    }
    loaded.executed_paths.insert(loaded.executed_paths.end(),
        context.paths.begin(), context.paths.end());
    output = std::move(loaded);
    return true;
}

const FontDescriptor* find_font_00ac3570(const FontRegistry& registry,
    const std::string& name) noexcept {
    for (const auto& font : registry.fonts)
        if (font.name.size() == name.size()
            && (name.empty() || _stricmp(font.name.c_str(), name.c_str()) == 0))
            return &font;
    return nullptr;
}
}
