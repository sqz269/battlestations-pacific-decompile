#include "bsp/lua_script_runtime.hpp"

#include <cstdio>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace bsp {
namespace {
class LiveScriptHost final : public GuiLuaScriptHost {
public:
    LiveScriptHost(lua_State* state, LuaScriptFiles& files) : state_(state), files_(files) {
        if (!state_) throw std::invalid_argument("Lua script runtime requires an open state");
    }
    bool read_file(const std::string& path, std::vector<char>& bytes) override {
        return files_.read_file_00b66ca0(path, 2, bytes);
    }
    std::vector<std::string> override_paths(const std::string& path) override {
        return files_.override_paths_00bdef90(path);
    }
    bool load_buffer(const char* bytes, std::size_t size, const char* name) override {
        return luaL_loadbuffer(state_, bytes, size, name) == 0;
    }
    void call_unprotected(int arguments, int results) override {
        const int top = lua_gettop(state_) - arguments - 1;
        // The native caller ignores compile status and calls even an error
        // string. Preserve that behavior inside a protected host boundary;
        // returning through pcall lets the outer C++ buffers unwind safely.
        if (lua_pcall(state_, arguments, results, 0) != 0) {
            const char* message = lua_tostring(state_, -1);
            const std::runtime_error failure(message ? message : "Lua script execution failed");
            lua_settop(state_, top);
            throw failure;
        }
    }
private:
    lua_State* state_;
    LuaScriptFiles& files_;
};
} // namespace

std::vector<GuiLuaChunkOutcome> LuaScriptRuntime::run_file(
    lua_State* state, const std::string& path, bool obfuscated) {
    LiveScriptHost host(state, files_);
    return run_gui_lua_file_00b69d40(host, path, obfuscated);
}
GuiLuaChunkOutcome LuaScriptRuntime::run_chunk(
    lua_State* state, const std::string& path, bool obfuscated) {
    LiveScriptHost host(state, files_);
    return run_gui_lua_chunk_00b66ca0(host, path, obfuscated);
}
bool LuaScriptRuntime::invoke_do_file(lua_State* state, const char* path,
                                     char* error, std::size_t error_size) noexcept {
    try {
        // The native string constructor uses strlen (embedded NUL truncates).
        (void)run_file(state, path, false);
        return true;
    } catch (const std::exception& failure) {
        std::snprintf(error, error_size, "%s", failure.what());
    } catch (...) {
        std::snprintf(error, error_size, "%s", "Lua script host failed");
    }
    return false;
}
int LuaScriptRuntime::do_file(lua_State* state, void* context) noexcept {
    auto* runtime = static_cast<LuaScriptRuntime*>(context);
    if (!runtime) {
        lua_pushliteral(state, "Lua DoFile requires its runtime context");
        return lua_error(state);
    }
    // 00b66c00 wraps the incoming state without ownership and reinstalls the
    // native DoFile global, even when invoked through a saved alias.
    lua_pushlightuserdata(state, runtime);
    lua_pushcclosure(state, do_file_closure, 1);
    lua_setglobal(state, "DoFile");
    const char* path = lua_tolstring(state, 1, nullptr);
    if (!path) {
        // 0041e870 would dereference this null source during strlen.
        lua_pushliteral(state, "DoFile requires a string or number filename");
        return lua_error(state);
    }
    char error[1024]{};
    if (runtime->invoke_do_file(state, path, error, sizeof(error))) return 0;
    // No C++ objects with destructors or active catches cross Lua's longjmp.
    lua_pushstring(state, error);
    return lua_error(state);
}
int LuaScriptRuntime::do_file_closure(lua_State* state) {
    return do_file(state, lua_touserdata(state, lua_upvalueindex(1)));
}
} // namespace bsp
