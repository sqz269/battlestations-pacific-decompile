#include "bsp/locale_lua_context.hpp"
#include "bsp/gui_lua_reader.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cwctype>
#include <limits>
#include <stdexcept>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace bsp {
namespace {

float narrow_lua_number(double number) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    float result;
    __asm {
        fld qword ptr [number]
        fstp dword ptr [result]
    }
    return result;
#else
    return static_cast<float>(number);
#endif
}

struct StackRestore {
    lua_State* state;
    int top;
    ~StackRestore() { lua_settop(state, top); }
};

struct PathRequest {
    const char* path;
    std::size_t length;
    bool valid{true};
    bool has_outer_environment{false};
};

// Called inside lua_pcall. Only trivial objects live across Lua operations,
// so a Lua __index error cannot jump past C++ objects requiring destruction.
int lookup_path_00b68d70(lua_State* state) {
    auto* request = static_cast<PathRequest*>(lua_touserdata(state, 1));
    if (!lua_checkstack(state, 4)) {
        return luaL_error(state, "locale Lua path stack allocation failed");
    }
    lua_pushvalue(state, LUA_GLOBALSINDEX); // index 3: current object
    bool root_pseudo_object = true;
    std::size_t begin = 0;
    while (begin < request->length) {
        std::size_t end = begin;
        while (end < request->length && request->path[end] != '.') {
            ++end;
        }
        if (end == begin || lua_type(state, 3) != LUA_TTABLE) {
            request->valid = false;
            return 1;
        }
        lua_pushlstring(state, request->path + begin, end - begin);
        lua_gettable(state, 3); // native string lookup honors __index
        if (lua_isnil(state, -1)) {
            // Native _atol is _strtol(...,10), not a whole-token validator.
            // A dot ends a base-10 conversion just as the temporary NUL does.
            long parsed = std::strtol(request->path + begin, nullptr, 10);
            if (parsed > (std::numeric_limits<std::int32_t>::max)()) {
                parsed = (std::numeric_limits<std::int32_t>::max)();
            } else if (parsed < (std::numeric_limits<std::int32_t>::min)()) {
                parsed = (std::numeric_limits<std::int32_t>::min)();
            }
            const auto index = static_cast<std::int32_t>(parsed);
            if (index != 0 || (end - begin == 1 && request->path[begin] == '0')) {
                lua_pop(state, 1);
                if (root_pseudo_object) {
                    // Native00b67720 on kind1 ADDS to LUA_GLOBALSINDEX,
                    // unlike table-kind2 indexing. +1 uses the outer active
                    // function's environment, never this trampoline's one.
                    if (index == 0) {
                        lua_pushvalue(state, LUA_GLOBALSINDEX);
                    } else if (index == 1 && request->has_outer_environment) {
                        lua_pushvalue(state, 2);
                    } else if (index == 2) {
                        lua_pushvalue(state, LUA_REGISTRYINDEX);
                    } else {
                        return luaL_error(state,
                            "locale root numeric pseudoindex has no bound native call frame");
                    }
                } else {
                    lua_pushnumber(state, static_cast<lua_Number>(index));
                    lua_gettable(state, 3);
                }
            }
        }
        lua_replace(state, 3);
        root_pseudo_object = false;
        begin = end + 1;
    }
    return 1; // Includes empty path => globals, and trailing-dot acceptance.
}

} // namespace

const char* locale_lua_type_name_00b68460(int type) noexcept {
    switch (type) {
    case LUA_TNONE: return "none";
    case LUA_TNIL: return "nil";
    case LUA_TBOOLEAN: return "boolean";
    case LUA_TLIGHTUSERDATA: return "lightuserdata";
    case LUA_TNUMBER: return "number";
    case LUA_TSTRING: return "string";
    case LUA_TTABLE: return "table";
    case LUA_TFUNCTION: return "function";
    case LUA_TUSERDATA: return "userdata";
    case LUA_TTHREAD: return "thread";
    default: return "unknown";
    }
}

std::string locale_lua_value_string_00b69130(lua_State& state, int index) {
    const int type = lua_type(&state, index);
    if (type == LUA_TBOOLEAN) {
        return lua_toboolean(&state, index) ? "true" : "false";
    }
    if (type == LUA_TNUMBER) {
        const double number = lua_tonumber(&state, index);
        const bool integral = gui_lua_is_integer_number_00b66a60(number);
        const float narrowed = narrow_lua_number(number);
        std::array<char, 52> buffer{};
        const int count = integral
            ? std::snprintf(buffer.data(), buffer.size(), "%d",
                static_cast<std::int32_t>(narrowed))
            : std::snprintf(buffer.data(), buffer.size(), "%f",
                static_cast<double>(narrowed));
        if (count < 0 || static_cast<std::size_t>(count) >= buffer.size()) {
            throw std::runtime_error("locale Lua numeric formatting failed");
        }
        return {buffer.data(), static_cast<std::size_t>(count)};
    }
    if (type == LUA_TSTRING) {
        // Native0041e870 measures strlen, so an embedded NUL truncates here.
        return lua_tostring(&state, index);
    }
    if (type == LUA_TLIGHTUSERDATA) {
        return "ptr";
    }
    return locale_lua_type_name_00b68460(type);
}

std::string LocaleLuaContext::context_string_00b692c0(const std::string& path) {
    if (path.find('\0') != std::string::npos || path.size() >
            static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)())) {
        throw std::invalid_argument("invalid native locale Lua path string");
    }
    StackRestore restore{&state_, lua_gettop(&state_)};
    PathRequest request{path.c_str(), path.size()};
    lua_pushcfunction(&state_, lookup_path_00b68d70);
    lua_pushlightuserdata(&state_, &request);
    lua_Debug frame{};
    if (lua_getstack(&state_, 0, &frame) && lua_getinfo(&state_, "f", &frame)) {
        lua_getfenv(&state_, -1);
        lua_remove(&state_, -2);
        request.has_outer_environment = true;
    } else {
        lua_pushnil(&state_);
    }
    if (lua_pcall(&state_, 2, 1, 0) != 0) {
        const char* error = lua_tostring(&state_, -1);
        throw std::runtime_error(error ? error : "locale Lua path lookup failed");
    }
    return request.valid ? locale_lua_value_string_00b69130(state_, -1) : "invalid";
}

char16_t LocaleLuaContext::crt_uppercase_00c0391c(char16_t code_unit) {
    return static_cast<char16_t>(std::towupper(static_cast<wint_t>(code_unit)));
}

} // namespace bsp
