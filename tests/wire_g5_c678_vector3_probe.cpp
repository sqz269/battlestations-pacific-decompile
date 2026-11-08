// Focused review regression probe. Link rebuilt bsp_game objects except
// game_main.obj, plus bsp_core/lua/zlib and the game's Windows libraries.
// Use /MANIFEST:EMBED /BASE:0x20000000 /DYNAMICBASE:NO; argv[1] is the original
// PE, mapped as read-only data only in a controlled bootstrap child. No game
// startup, game loop or original code runs.
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include "bsp/mission_lua_host.hpp"
#include <float.h>
#include <iostream>
#include <iterator>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

int main(int argc, char** argv) {
    try {
        _control87(_PC_24, _MCW_PC);
        using namespace bsp::game;
        constexpr GameNativeDataSpan spans[] = {
            {0x00ce2000u, 1}, {0x00cf0000u, 1}, {0x00d00000u, 1},
            {0x00d10000u, 1}, {0x00d50000u, 1}, {0x00d60000u, 1}, {0x00d70000u, 1}
        };
        if (argc == 2) {
            wchar_t executable[32768];
            if (!GetModuleFileNameW(nullptr, executable, std::size(executable)))
                throw std::runtime_error("missing probe executable path");
            const auto original = std::filesystem::path(argv[1]);
            GameNativeDataBootstrapChild child(executable,
                L"--child \"" + original.wstring() + L"\"", spans, std::size(spans));
            child.reserve_and_resume();
            child.wait_for_mapping(30000);
            if (WaitForSingleObject(child.process_handle(), 30000) != WAIT_OBJECT_0)
                throw std::runtime_error("probe child timed out");
            DWORD result;
            if (!GetExitCodeProcess(child.process_handle(), &result))
                throw std::runtime_error("missing probe child result");
            if (result == 0)
                std::cout << "PASS: registered DoVector3, numeric strings, float32 narrowing, throwing __index untouched\n";
            return static_cast<int>(result);
        }
        if (argc < 3 || std::string(argv[1]) != "--child")
            throw std::runtime_error("expected original PE path or controlled child");
        auto reservation = accept_native_data_handoff(spans, std::size(spans));
        GameNativeReadOnlyData data(argv[2], spans, std::size(spans), std::move(reservation));
        GameHostLog log;
        GameSingletonHost singletons(log);
        GameVfsHost vfs(log, singletons, data, argv[2]);
        GameMissionLuaHost mission(log, vfs);
        GameUnitsHost units(log, mission);
        GameScriptOrdersHost orders(log, units);
        mission.attach_script_orders(&orders);
        mission.create_state();
        lua_State* state = mission.mission_lua_state();
        if (!state) throw std::runtime_error("missing live Lua state");
        lua_pushcfunction(state, luaopen_base);
        lua_pushstring(state, "");
        lua_call(state, 1, 0);
        mission.register_global_function(*bsp::find_mission_lua_binding("DoVector3"));
        const char* checks = R"lua(
            local v = DoVector3("1.5", 2, 3)
            assert(v.x == 1.5 and v.y == 2 and v.z == 3, "numeric-string conversion")
            assert(DoVector3("16777217", 2, 3).x == 16777216, "float32 narrowing")
            local touched = false
            local t = setmetatable({}, {__index = function()
                touched = true
                error("DoVector3 must not read ID")
            end})
            local result = DoVector3(t, 2, 3)
            assert(not touched, "table __index was invoked")
            assert(result.x == 0 and result.y == 2 and result.z == 3, "table-to-zero conversion")
        )lua";
        if (luaL_dostring(state, checks) != 0)
            throw std::runtime_error(lua_tostring(state, -1));
        mission.attach_script_orders(nullptr);
        std::cout << "PASS: registered DoVector3 trampoline/dispatch, numeric strings, float32 narrowing, throwing __index untouched\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
