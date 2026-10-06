#pragma once

// Safety guard, NOT an image behaviour: the rebuild never lets a mission script start
// a process. The image opens the full os and io libraries (006B8740 step 4), so a
// script can call os.execute or io.popen. This installation's modded
// scripts\global\commandhelpers.lua (mtime 2024-10-29) calls os.execute("sus_prog.exe")
// at the top of luaMissionCompletedNew, and bsp_game runs with the current directory
// set to the game root, so the original behaviour would launch that executable.
// These openers wrap the stock ones and replace os.execute and io.popen with stubs
// that refuse and report through stderr. Every other os/io function is unchanged.

#include <cstddef>
#include <cstdio>

// Declared, not included: lua.h defines lua_getglobal and friends as macros, which
// break mission_lua_host.hpp's same-named virtual members in any file that includes
// this header first. These match the Lua 5.1.1 prototypes.
struct lua_State;
extern "C" {
int luaopen_os(lua_State* state);
int luaopen_io(lua_State* state);
void lua_pushcclosure(lua_State* state, int (*fn)(lua_State*), int n);
void lua_setfield(lua_State* state, int index, const char* key);
void lua_pushnil(lua_State* state);
void lua_pushstring(lua_State* state, const char* text);
const char* lua_tolstring(lua_State* state, int index, std::size_t* length);
}

namespace bsp {

inline int lua_exec_refused(lua_State* state) {
    const char* command = lua_tolstring(state, 1, nullptr);
    std::fprintf(stderr, "bsp: refused a mission script's process launch: %s\n",
        command != nullptr ? command : "(non-string)");
    lua_pushnil(state);
    lua_pushstring(state, "process launch disabled by the rebuild host");
    return 2;
}

// luaopen_os leaves the os table on the stack top (Lua 5.1 luaL_register).
inline int luaopen_os_guarded(lua_State* state) {
    const int results = luaopen_os(state);
    lua_pushcclosure(state, lua_exec_refused, 0);
    lua_setfield(state, -2, "execute");
    return results;
}

// luaopen_io leaves the io table on the stack top.
inline int luaopen_io_guarded(lua_State* state) {
    const int results = luaopen_io(state);
    lua_pushcclosure(state, lua_exec_refused, 0);
    lua_setfield(state, -2, "popen");
    return results;
}

}  // namespace bsp
