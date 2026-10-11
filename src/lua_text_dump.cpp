#include "bsp/lua_text_dump.hpp"

#include <cstdio>
#include <limits>
#include <stdexcept>
extern "C" {
#include <lua.h>
}

namespace bsp {
namespace {

int abs_index(lua_State* L, int index) {
    return index > 0 || index <= LUA_REGISTRYINDEX ? index : lua_gettop(L) + index + 1;
}

// CVTTSS2SI truncation, 80000000h out of range (00B66A60 / 00B66290).
std::int32_t truncate_float32(float value) {
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(value);
}

std::string format_int(long long value) {
    char buffer[32];
    std::snprintf(buffer, sizeof buffer, "%lld", value);
    return buffer;
}

} // namespace

const char* lua_type_name_00b68460(int lua_type) noexcept {
    static const char* const names[] = {"none", "nil", "bool", "lightuserdata", "number",
                                        "string", "table", "function", "userdata", "thread"};
    const unsigned slot = static_cast<unsigned>(lua_type + 1); // 00B68461, bound 9
    return slot <= 9 ? names[slot] : "unknown";
}

std::string lua_value_text_0043c1c0(lua_State* L, int index, const LuaTextEntityHandle& handle_of) {
    index = abs_index(L, index);
    switch (lua_type(L, index)) {
    case LUA_TNIL: return "nil";                                   // 00CE4380
    case LUA_TBOOLEAN: return lua_toboolean(L, index) ? "true" : "false";
    case LUA_TNUMBER: {
        const float value = static_cast<float>(lua_tonumber(L, index));
        const std::int32_t whole = truncate_float32(value);
        if (static_cast<double>(value) == static_cast<double>(whole))
            return format_int(whole);                              // 004260B0 "%d"
        char buffer[400];
        std::snprintf(buffer, sizeof buffer, "%f", static_cast<double>(value)); // 0043BDF0
        return buffer;
    }
    case LUA_TSTRING: {
        lua_pushvalue(L, index);
        const char* text = lua_tostring(L, -1);
        std::string result = std::string("\"") + (text ? text : "") + "\""; // 00CE436C
        lua_pop(L, 1);
        return result;
    }
    case LUA_TLIGHTUSERDATA: {
        const void* entity = lua_touserdata(L, index);
        if (!entity) return "\"_ptr_x_\"";                       // 00CE4354
        if (!handle_of) throw std::invalid_argument("entity handle hook is required");
        return "\"_ptr_" + format_int(handle_of(entity)) + "_\""; // 00CE4360, 00CE4368
    }
    case LUA_TFUNCTION: {
        // 004263B0 formats the pointer as a signed 32-bit %d.
        const auto pointer = static_cast<std::int32_t>(reinterpret_cast<std::intptr_t>(lua_topointer(L, index)));
        return "\"_function_" + format_int(pointer) + "_\"";       // 00CE4348
    }
    default:
        return std::string("\"_other_type:_") + lua_type_name_00b68460(lua_type(L, index)) + "_\""; // 00CE4338
    }
}

void dump_lua_table_text_0043c530(lua_State* L, int index, int indent, int depth,
                                  const LuaTextEntityHandle& handle_of, const LuaTextAppend& append) {
    index = abs_index(L, index);
    if (!lua_checkstack(L, 6)) throw std::length_error("Lua stack exhausted");
    // 0043C571..0043C5A2: the prefix is `indent` tab characters.
    const std::string prefix(indent > 0 ? static_cast<std::size_t>(indent) : 0u, '\t');
    if (depth < 0) {
        append(prefix + "\tnil, -- too deep recursion");          // 00CE43A0
        return;
    }
    append(prefix + "{\n");                                       // 00CE439C
    lua_pushnil(L);
    while (lua_next(L, index)) {
        const int value = lua_gettop(L);
        append(prefix + "\t[" + lua_value_text_0043c1c0(L, value - 1, handle_of) + "] ="); // 00CE4394, 00CE4398
        if (lua_istable(L, value)) {
            append("\n");                                         // 00CE4390
            dump_lua_table_text_0043c530(L, value, indent + 1, depth - 1, handle_of, append);
        } else {
            append(" " + lua_value_text_0043c1c0(L, value, handle_of) + ",\n"); // 00CE3A90, 00CE438C
        }
        lua_pop(L, 1);
    }
    append(prefix + (indent != 0 ? "},\n" : "}\n"));             // 00CE4388 / 00CE4384
}

void emit_lua_global_text_006ad8d0(lua_State* L, const char* name,
                                   const LuaTextEntityHandle& handle_of, const LuaTextAppend& append) {
    lua_getfield(L, LUA_GLOBALSINDEX, name);
    if (lua_istable(L, -1)) {
        append(std::string(name) + " =\n");                       // 00CF8288
        dump_lua_table_text_0043c530(L, -1, 0, 0x10, handle_of, append);
    } else {
        append(std::string(name) + " = " + lua_value_text_0043c1c0(L, -1, handle_of) + "\n"); // 00CF8284, 00CE4390
    }
    lua_pop(L, 1);
}

} // namespace bsp
