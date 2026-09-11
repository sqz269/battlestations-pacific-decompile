#include "bsp/lua_state_owner.hpp"

#include <cstring>
#include <new>
#include <stdexcept>
#include <utility>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace bsp {
namespace {
struct Library {
    const char* name;
    lua_CFunction open;
};
// Native00d62bb8: base/package/table/io/os/string/math/debug, then null.
const Library libraries[] = {
    {"", luaopen_base}, {LUA_LOADLIBNAME, luaopen_package},
    {LUA_TABLIBNAME, luaopen_table}, {LUA_IOLIBNAME, luaopen_io},
    {LUA_OSLIBNAME, luaopen_os}, {LUA_STRLIBNAME, luaopen_string},
    {LUA_MATHLIBNAME, luaopen_math}, {LUA_DBLIBNAME, luaopen_debug}
};
int panic_00b669c0(lua_State* state) {
    (void)lua_tolstring(state, lua_gettop(state), nullptr);
    return 0;
}
void platform_chunk(lua_State* state, const char* text) {
    if (luaL_loadstring(state, text) == 0)
        (void)lua_pcall(state, 0, LUA_MULTRET, 0);
}
} // namespace

PcStorageLuaOwner::PcStorageLuaOwner(LuaStateOwnerEnvironment environment)
    : environment_(std::move(environment)) {
    if (!environment_.do_file)
        throw std::invalid_argument("Lua owner requires the native DoFile host contract");
}
PcStorageLuaOwner::~PcStorageLuaOwner() {
    close_storage_archive_00b65e80();
}
void PcStorageLuaOwner::open_storage_archive_00b6a020(std::uint32_t flags) {
    if (state_) throw std::logic_error("Lua owner is already open");
    library_flags_ = flags;
    state_ = luaL_newstate();
    if (!state_) throw std::bad_alloc();
    lua_atpanic(state_, panic_00b669c0);
    // Keep the reconstructed unprotected library/fundamentals calls inside a
    // protected host entry. No C++ automatic objects cross Lua's longjmp.
    lua_pushlightuserdata(state_, this);
    lua_pushcclosure(state_, bootstrap, 1);
    if (lua_pcall(state_, 0, LUA_MULTRET, 0) != 0) {
        const char* text = lua_tostring(state_, -1);
        try {
            throw std::runtime_error(text ? text : "Lua owner bootstrap failed");
        } catch (...) {
            close_storage_archive_00b65e80();
            throw;
        }
    }
}
int PcStorageLuaOwner::bootstrap(lua_State* state) {
    auto* owner = static_cast<PcStorageLuaOwner*>(lua_touserdata(state, lua_upvalueindex(1)));
    std::uint32_t bit = 1;
    for (const auto& library : libraries) {
        if (bit == 1 || (owner->library_flags_ & bit) != 0) {
            lua_pushcfunction(state, library.open);
            lua_pushstring(state, library.name);
            lua_call(state, 1, 0);
        }
        bit <<= 1;
    }
    owner->initial_stack_top_ = lua_gettop(state);
    platform_chunk(state, "PC=true");
    platform_chunk(state, owner->environment_.x360comp ? "X360COMP=true" : "X360COMP=false");
    // Native _stricmp compares C strings; preserve its NUL termination.
    const char* region = owner->environment_.region.c_str();
    if (_stricmp(region, "EU") == 0) platform_chunk(state, "REGION=\"EU\"");
    else if (_stricmp(region, "USA") == 0) platform_chunk(state, "REGION=\"USA\"");
    else if (_stricmp(region, "JAP") == 0) platform_chunk(state, "REGION=\"JAP\"");
    lua_pushlightuserdata(state, owner);
    lua_pushcclosure(state, do_file, 1);
    lua_setglobal(state, "DoFile");
    // Native deliberately ignores load status before its unprotected call.
    (void)luaL_loadbuffer(state, owner->environment_.fundamentals.data(),
        owner->environment_.fundamentals.size(), "Scripts\\fundamentals.lua");
    lua_call(state, 0, LUA_MULTRET);
    return lua_gettop(state);
}
int PcStorageLuaOwner::do_file(lua_State* state) {
    auto* owner = static_cast<PcStorageLuaOwner*>(lua_touserdata(state, lua_upvalueindex(1)));
    return owner->environment_.do_file(state, owner->environment_.do_file_context);
}
void PcStorageLuaOwner::close_storage_archive_00b65e80() noexcept {
    if (!state_) return;
    if (environment_.before_close)
        environment_.before_close(state_, environment_.close_context);
    lua_close(state_);
    state_ = nullptr;
}
} // namespace bsp
