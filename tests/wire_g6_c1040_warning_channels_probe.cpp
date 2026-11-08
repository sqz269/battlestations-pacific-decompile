// One production listener-owner regression. Link rebuilt game objects except
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
#include "bsp/warning_owner.hpp"
#include "bsp/engine_seg49_64_78_cc13.hpp"
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
    require(lua_gettop(state) == 0, "warning channels leaked Lua stack entries");
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
            if (result == 0) std::cout << "PASS: production warning-channel owner, ordering and teardown\n";
            return static_cast<int>(result);
        }
        // The bootstrap appends its inherited handoff argument after the PE path.
        require(argc >= 3 && std::string(argv[1]) == "--child", "expected original PE path");
        auto reservation = accept_native_data_handoff(spans, std::size(spans));
        GameNativeReadOnlyData data(argv[2], spans, std::size(spans), std::move(reservation));
        GameHostLog log;
        require(log.open("local/output/c1040_warning_channels_probe.log"), "cannot open probe log");
        GameSingletonHost singletons(log);
        GameVfsHost vfs(log, singletons, data, argv[2]);
        GameMissionLuaHost mission(log, vfs);
        // Use the normal complete binding table and binding_trampoline route,
        // including its diagnostic subject lookup, rather than private closures.
        require(bsp::initialise_mission_lua_host(mission) != 0,
            "mission binding registration failed");
        lua_State* const state = mission.mission_lua_state();
        require(state != nullptr, "missing mission Lua VM");
        require(current_warning_owner() == nullptr, "unexpected manager before construction");
        GameGlobalSubsystemsHost globals(log, vfs);
        globals.build_warnings();
        bsp::WarningOwner* owner = globals.warnings();
        require(owner != nullptr && current_warning_owner() == owner,
            "warning singleton does not borrow game+21E0");
        require(owner->channels_f8 && owner->channels_f8->empty(),
            "constructor did not create the canonical empty channel map");
        run(state, R"lua(
            AddListener("shipLanded", "Zulu", {callback="late", entity={}})
            AddListener("SHIPLANDED", "alpha", {callback="early", entity={}})
            assert(IsListenerActive("shiplanded", "ALPHA"))
        )lua");
        auto& entries = *owner->channels_f8;
        require(entries.size() == 2 && entries[0].id == "alpha" && entries[1].id == "Zulu",
            "channel slots do not follow the native case-insensitive id order");
        require(entries[0].native_other != nullptr && entries[1].native_other != nullptr,
            "real AddListener did not produce the native loaded subscriptions");
        require(entries[0].native_other->callback == "early", "lost actual Lua callback");
        std::weak_ptr<const bsp::cc13_seg49_64_78::EventSubscription> replaced =
            entries[0].native_other;
        run(state, "AddListener('shiplanded','ALPHA',{callback='replacement', entity={}})");
        require(entries.size() == 2 && replaced.expired()
                && entries[0].native_other->callback == "replacement",
            "re-add did not replace the manager's existing slot");
        run(state, R"lua(
            RemoveListener("shipLanded", "zULU")
            assert(not IsListenerActive("shipLanded", "Zulu"))
        )lua");
        require(entries.size() == 1, "RemoveListener mutated a separate registry");
        std::weak_ptr<const bsp::cc13_seg49_64_78::EventSubscription> removed_at_teardown =
            entries[0].native_other;
        require(owner->critical_section_24->depth == 0, "listener call leaked manager lock depth");
        globals.teardown_004d2bb0();
        require(current_warning_owner() == nullptr && removed_at_teardown.expired(),
            "manager teardown left its publication or subscription payload alive");
        run(state, "assert(not IsListenerActive('shipLanded','alpha'))");
        globals.build_warnings();
        require(current_warning_owner() == globals.warnings()
                && globals.warnings()->channels_f8->empty(),
            "new mission inherited previous manager's listeners");
        run(state, "AddListener('failure','fresh',{callback='fresh_callback', entity={}, failureType={}, duration={}})");
        require(globals.warnings()->channels_f8->size() == 1
                && globals.warnings()->channels_f8->front().native_other != nullptr,
            "AddListener did not follow the new manager publication");
        globals.teardown_004d2bb0();
        std::cout << "PASS: production Add/Remove/IsListenerActive share the manager's "
            "ordered channel storage; replacement and teardown release loaded payloads; "
            "a new manager starts empty\n";
        return 0;
    } catch (const std::exception& error) {
        std::ofstream("local/output/c1040_warning_channels_probe_error.txt") << error.what() << '\n';
        std::cerr << error.what() << '\n';
        return 1;
    }
}
