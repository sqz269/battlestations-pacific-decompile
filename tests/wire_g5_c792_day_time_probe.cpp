// One production-dispatch regression. Link rebuilt game objects except
// game_main.obj, with /MANIFEST:EMBED, /BASE:0x20000000, /DYNAMICBASE:NO.
// The original PE supplies read-only constants; no game loop/code runs.
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_global_subsystems.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/weather_config.hpp"
#include <float.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

static void require(bool value, const char* reason) {
    if (!value) throw std::runtime_error(reason);
}
static void run(lua_State* state, const char* script) {
    if (luaL_dostring(state, script) != 0)
        throw std::runtime_error(lua_tostring(state, -1));
    require(lua_gettop(state) == 0, "day-time dispatch leaked Lua stack entries");
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
            require(GetModuleFileNameW(nullptr, executable, std::size(executable)) != 0,
                "missing probe executable path");
            GameNativeDataBootstrapChild child(executable,
                L"--child \"" + std::filesystem::path(argv[1]).wstring() + L"\"",
                spans, std::size(spans));
            child.reserve_and_resume();
            child.wait_for_mapping(30000);
            require(WaitForSingleObject(child.process_handle(), 30000) == WAIT_OBJECT_0,
                "probe child timed out");
            DWORD result;
            require(GetExitCodeProcess(child.process_handle(), &result) != 0,
                "missing probe child result");
            if (result == 0) std::cout << "PASS: production day-time dispatch and weather owner\n";
            return static_cast<int>(result);
        }
        // The bootstrap appends its inherited handoff argument after the PE path.
        require(argc >= 3 && std::string(argv[1]) == "--child", "expected original PE path");
        auto reservation = accept_native_data_handoff(spans, std::size(spans));
        GameNativeReadOnlyData data(argv[2], spans, std::size(spans), std::move(reservation));
        GameHostLog log;
        require(log.open("local/output/c792_day_time_probe.log"), "cannot open probe log");
        GameSingletonHost singletons(log);
        GameVfsHost vfs(log, singletons, data, argv[2]);
        GameMissionLuaHost mission(log, vfs);
        // Use the normal complete binding table and binding_trampoline route,
        // including its diagnostic subject lookup, rather than private closures.
        require(bsp::initialise_mission_lua_host(mission) != 0,
            "mission binding registration failed");
        lua_State* const state = mission.mission_lua_state();
        require(state != nullptr, "missing mission Lua VM");
        GameGlobalSubsystemsHost globals(log, vfs);
        globals.build_weather();
        require(globals.weather() != nullptr, "missing actual weather owner");
        // This probe does not mount game archives. Seed one bounded fixture
        // value into the production owner; production uses 00444D20's Globals.lua.
        globals.weather()->start_day_time_04 = 18.75f;
        GameUnitsHost units(log, mission);
        GameScriptOrdersHost orders(log, units);
        orders.attach_global_subsystems(&globals);
        mission.attach_script_orders(&orders);
        run(state, R"lua(
            local t = GetDayTime()
            assert(t.hours == 18 and t.minutes == 45 and t.seconds == 0)
            SetDayTime(6, 30)
            t = GetDayTime()
            assert(t.hours == 6 and t.minutes == 30 and t.seconds == 0)
        )lua");
        require(globals.weather()->start_day_time_04 == 6.5f,
            "SetDayTime did not update the retained weather owner's +4h");
        run(state, R"lua(
            SetDayTime(1.99999999)
            local t = GetDayTime()
            assert(t.hours == 2 and t.minutes == 0 and t.seconds == 0)
            SetDayTime(6, 29.99999999)
            t = GetDayTime()
            assert(t.hours == 6 and t.minutes == 30 and t.seconds == 0)
            SetDayTime(6, 0, 1799.99999999)
            t = GetDayTime()
            assert(t.hours == 6 and t.minutes == 30 and t.seconds == 0)
            SetDayTime(-1)
            t = GetDayTime()
            assert(t.hours == 23 and t.minutes == 0 and t.seconds == 0)
            -- Numeric coercion must not perform the entity dispatch's ID lookup.
            local numeric_table = setmetatable({}, {
                __index = function() error('entity lookup') end
            })
            SetDayTime(numeric_table)
            t = GetDayTime(numeric_table)
            assert(t.hours == 0 and t.minutes == 0 and t.seconds == 0)
        )lua");
        globals.teardown_004d2bb0();
        globals.build_weather();
        globals.weather()->start_day_time_04 = 9.25f;
        run(state, "local t=GetDayTime(); assert(t.hours==9 and t.minutes==15 and t.seconds==0)");
        mission.attach_script_orders(nullptr);
        std::ofstream("local/output/c792_day_time_probe_result.txt")
            << "PASS: production day-time dispatch and weather owner\n";
        std::cout << "PASS: day-time dispatch uses the retained/rebuilt weather owner, "
            "optional integers, native result table and isolated Lua stack\n";
    } catch (const std::exception& error) {
        std::ofstream("local/output/c792_day_time_probe_result.txt") << "FAIL: " << error.what() << '\n';
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
