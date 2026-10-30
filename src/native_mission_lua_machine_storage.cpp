#include "bsp/native_mission_lua_machine_storage.hpp"

#include "bsp/settings_seg29_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <cstdint>

namespace bsp {
namespace {

using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);

volatile Word& profile(void* machine) noexcept {
    return *reinterpret_cast<volatile Word*>(machine);
}

lua_State* volatile& state(void* machine) noexcept {
    return *reinterpret_cast<lua_State* volatile*>(
        reinterpret_cast<Word>(machine) + 4u);
}

volatile std::uint8_t& owns(void* machine) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(
        reinterpret_cast<Word>(machine) + 8u);
}

struct LibraryRow {
    const char* name;
    lua_CFunction open;
};
static_assert(sizeof(LibraryRow) == 8);

// Exact values/order of Native CF8350..CF838F, mapped to genuine linked
// Source functions and strings. Volatile members retain the initial/next
// opener and current-name read epochs. No Native address is dereferenced.
const volatile LibraryRow libraries[] = {
    {"", luaopen_base},
    {"table", luaopen_table},
    {"io", luaopen_io},
    {"os", luaopen_os},
    {"string", luaopen_string},
    {"math", luaopen_math},
    {"debug", luaopen_debug},
    {nullptr, nullptr},
};

} // namespace

void* construct_native_mission_lua_machine_006b8740(void* actual_machine) {
    void* const captured = actual_machine;
    profile(captured) = 0x00cf8394u;
    lua_State* const created = luaL_newstate();
    state(captured) = created;
    lua_atpanic(created, &cc13_seg29::lua_machine_panic_006b8720);
    lua_gc(state(captured), LUA_GCSETPAUSE, 100);

    const volatile LibraryRow* row = libraries;
    lua_CFunction opener = row->open;
    while (opener != nullptr) {
        lua_pushcclosure(state(captured), opener, 0);
        const char* const name = row->name;
        lua_State* const string_state = state(captured);
        lua_pushstring(string_state, name);
        lua_call(state(captured), 1, 0);
        opener = (row + 1)->open;
        ++row;
    }

    owns(captured) = 1;
    return captured;
}

void* delete_native_mission_lua_machine_006b87c0(
    void* actual_machine, std::uint32_t flags) {
    void* const captured = actual_machine;
    const std::uint8_t captured_owns = owns(captured);
    profile(captured) = 0x00cf8394u;
    if (captured_owns != 0) {
        lua_close(state(captured));
    }

    // Force the same post-callback low-byte read epoch without claiming the
    // Native receiver/argument placement or arbitrary caller-stack aliases.
    const auto* const current_flag_byte =
        reinterpret_cast<const volatile std::uint8_t*>(&flags);
    if ((*current_flag_byte & 1u) != 0) {
        singleton_lifetime_free(captured);
    }
    return captured;
}

} // namespace bsp
