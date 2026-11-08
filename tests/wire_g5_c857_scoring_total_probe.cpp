// One production Lua-dispatch check over the retained mission score records.
// Link rebuilt game objects except game_main.obj, /MANIFEST:EMBED,
// /BASE:0x20000000, /DYNAMICBASE:NO. The PE supplies read-only constants;
// neither original game code nor the reconstructed game loop executes.
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_mission_result.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/locale_tables.hpp"
#include <float.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

static void require(bool value, const char* reason) {
    if (!value) throw std::runtime_error(reason);
}
static void run(lua_State* state, const char* script) {
    if (luaL_dostring(state, script) != 0)
        throw std::runtime_error(lua_tostring(state, -1));
    require(lua_gettop(state) == 0, "score-total dispatch leaked Lua stack entries");
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
            return static_cast<int>(result);
        }
        require(argc >= 3 && std::string(argv[1]) == "--child", "expected original PE path");
        auto reservation = accept_native_data_handoff(spans, std::size(spans));
        GameNativeReadOnlyData data(argv[2], spans, std::size(spans), std::move(reservation));
        GameHostLog log;
        require(log.open("local/output/c857-scoring-total-probe.log"), "cannot open probe log");
        GameSingletonHost singletons(log);
        GameVfsHost vfs(log, singletons, data, argv[2]);
        bsp::GameFrameControlState control;
        bsp::LocaleTables locale;
        GameMissionResultHost result(log, control, vfs, locale);
        GameMissionLuaHost mission(log, vfs);
        mission.attach_mission_result(&result);
        require(bsp::initialise_mission_lua_host(mission) != 0,
            "mission binding registration failed");
        lua_State* const state = mission.mission_lua_state();
        require(state != nullptr, "missing mission Lua VM");

        // Explicit fixture values in the production owner's actual records.
        // The keyed map deliberately differs from the stored native total.
        bsp::MissionResultHost& records = result;
        result.set_session_mode(0);
        records.scoring_slot(0).totals_1c8[6] = 137;
        records.scoring_slot(0).score_maps[0]["different keyed score"] = 9000;
        records.scoring_slot(2).totals_1c8[6] = 259;
        run(state, "assert(select('#',Scoring_GetTotalMissionScore())==1); "
            "assert(Scoring_GetTotalMissionScore()==137); "
            "assert(Scoring_GetTotalMissionScore(2)==137)");
        records.scoring_slot(0).totals_1c8[6] = -17;
        run(state, "assert(Scoring_GetTotalMissionScore()==-17)");
        // Exercise the original argument-selected-slot branch without any
        // online service or session transport.
        result.set_session_mode(1);
        run(state, "assert(Scoring_GetTotalMissionScore(2)==259)");
        std::cout << "PASS: production scoring-total dispatch, owned record reload, "
            "single result and slot selection\n";
        std::ofstream("local/output/c857-probe-result.txt")
            << "PASS: production scoring-total dispatch, owned record reload, "
               "single result and slot selection\n";
    } catch (const std::exception& error) {
        std::ofstream("local/output/c857-probe-result.txt") << "FAIL: " << error.what() << '\n';
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
