// Focused source regression for 0084EB60 and the live host's lazy class readers.
// Link bsp_game objects except game_main.obj, plus its libraries. The original
// PE supplies read-only data in a controlled child; no game/original code runs.
// Use /MANIFEST:EMBED /BASE:0x20000000 /DYNAMICBASE:NO; argv[1] is the installed PE.
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include "bsp/oct18_tail_b.hpp"
#include <cstring>
#include <iostream>
#include <iterator>
#include <stdexcept>
extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
class ResetRunner final : public bsp::MissionLuaRunString {
public:
    explicit ResetRunner(bsp::game::GameMissionLuaHost& lua) : lua_(lua) {}
    void run_string_006b8ad0(const char* text, int mode) override {
        require(mode == 2, "wrong native reset mode");
        const auto result = bsp::run_lua_chunk(lua_, text,
            static_cast<int>(std::strlen(text)), text, false, false, mode);
        require(result.status == bsp::LuaChunkStatus::Ok, "reset chunk failed");
    }
private:
    bsp::game::GameMissionLuaHost& lua_;
};
}

int main(int argc, char** argv) {
    try {
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
            if (result == 0)
                std::cout << "PASS: reset clears global; lazy readers survive GC/repeat reset; "
                    "replacement releases old table; stack preserved\n";
            return static_cast<int>(result);
        }
        require(argc >= 3 && std::string(argv[1]) == "--child", "expected original PE path");
        auto reservation = accept_native_data_handoff(spans, std::size(spans));
        GameNativeReadOnlyData data(argv[2], spans, std::size(spans), std::move(reservation));
        GameHostLog log;
        require(log.open("local/output/c816-stationary-probe.log"), "cannot open probe log");
        GameSingletonHost singletons(log);
        GameVfsHost vfs(log, singletons, data, argv[2]);
        GameMissionLuaHost mission(log, vfs);
        mission.create_state();
        lua_State* state = mission.mission_lua_state();
        require(state != nullptr, "missing Lua state");
        lua_pushcfunction(state, luaopen_base);
        lua_pushstring(state, "");
        lua_call(state, 1, 0);
        ResetRunner reset(mission);
        auto chunk = [&](const char* text) {
            require(luaL_dostring(state, text) == 0, "fixture chunk failed");
        };
        // An explicit fixture table, not installed class/gameplay evidence.
        chunk("StationaryClass={Dock={Model='first',Count=7}}; "
            "weak=setmetatable({StationaryClass},{__mode='v'})");
        lua_pushinteger(state, 123); // callers' existing stack must survive.
        auto check = [&](const char* model) {
            require(mission.stationary_class_exists("Dock"), "class missing after reset");
            require(!mission.stationary_class_exists("Absent"), "absent class invented");
            require(mission.read_stationary_class_string("Dock", "Model") == model,
                "wrong retained model");
            require(mission.read_stationary_class_string("Dock", "Count").empty(),
                "non-string field accepted");
            require(lua_gettop(state) == 1 && lua_tointeger(state, 1) == 123, "stack changed");
        };
        check("first");
        mission.retain_stationary_class_table();
        bsp::oct18_tail_b::reset_stationary_class_table_0084eb60(reset);
        chunk("assert(StationaryClass==nil); collectgarbage('collect'); assert(weak[1]~=nil)");
        check("first");
        mission.retain_stationary_class_table(); // repeated nil reset retains the class lifetime.
        bsp::oct18_tail_b::reset_stationary_class_table_0084eb60(reset);
        check("first");
        chunk("StationaryClass={Dock={Model='second'}}");
        check("second"); // a fresh global takes precedence before its reset.
        mission.retain_stationary_class_table();
        bsp::oct18_tail_b::reset_stationary_class_table_0084eb60(reset);
        chunk("collectgarbage('collect'); assert(weak[1]==nil); assert(StationaryClass==nil)");
        check("second");
        std::cout << "PASS: native reset clears global; class lookup survives GC/repeated reset; "
            "replacement releases old table; stack preserved\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
