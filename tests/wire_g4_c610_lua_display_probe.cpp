#include "bsp/gui_lua_runtime.hpp"
#include "bsp/game_native_lua_display_string.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_string_float_format.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
static void require(bool value) { if (!value) throw std::runtime_error("chain610 probe failed"); }
static void number(void* context, bsp::NativeString& out, float value) {
    bsp::construct_native_string_float_header_0043bdf0(&out,
        *static_cast<bsp::NativeStringRawPoolContext*>(context), value);
}
static void throwing_number(void*, bsp::NativeString&, float) { throw std::runtime_error("injected formatter"); }
int main() {
    lua_State* state = luaL_newstate();
    require(state != nullptr);
    {
        bsp::GuiLua51Host lua(*state);
        std::string error;
        require(lua.execute_archive("Keys = { [1.25] = 17, Text = 18, [false] = 19 }", error));
        auto& process = bsp::game::game_native_string_process();
        bsp::NativeLuaDisplayStringCalls calls{process.strings(), number, &process.raw_context()};
        const auto globals = lua.globals();
        const auto table = lua.get_by_name(globals, "Keys");
        bsp::GuiLuaRef key, value;
        int count = 0, numeric = 0, words = 0, boolean = 0;
        bool restart = true;
        while (lua.next(table, key, value, restart)) {
            restart = false;
            const auto kind = lua.type_of(key);
            const int top = lua_gettop(state);
            alignas(bsp::NativeString) std::byte fresh[sizeof(bsp::NativeString)];
            bsp::NativeString* result = lua.to_display_string(key, fresh, calls);
            require(lua_gettop(state) == top && lua.type_of(key) == kind);
            if (kind == bsp::GuiLuaType::Number) { require(std::strcmp(result->data(), "1.250000") == 0); ++numeric; }
            else if (kind == bsp::GuiLuaType::String) { require(std::strcmp(result->data(), "Text") == 0); ++words; }
            else { require(std::strcmp(result->data(), "false") == 0); ++boolean; }
            bsp::destroy_native_string_header_0041dd20(result, process.raw_context());
            bsp::game::discard_sound_configuration_key_display(lua, key, process.strings());
            require(lua_gettop(state) == top && lua.type_of(key) == kind);
            if (kind == bsp::GuiLuaType::Number) {
                bsp::NativeLuaDisplayStringCalls throws{process.strings(), throwing_number, nullptr};
                bool caught = false;
                try { lua.to_display_string(key, fresh, throws); }
                catch (const std::runtime_error&) { caught = true; }
                require(caught && lua_gettop(state) == top && lua.to_number(key) == 1.25);
            }
            ++count;
            lua.release(value); lua.release(key);
        }
        require(count == 3 && numeric == 1 && words == 1 && boolean == 1);
        lua.release(table); lua.release(globals);
        require(lua_gettop(state) == 0);
    }
    lua_close(state);
    std::cout << "PASS: actual-pool display/discard, numeric key preservation, iteration, stack and throw restoration\n";
}
