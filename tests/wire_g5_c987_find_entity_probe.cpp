// One host regression fixture: FindEntity borrows real scene records and Lua
// slots, including a second sibling, a hidden name and a runtime-only entry.
// Link rebuilt game objects except game_main.obj, plus bsp_core/lua/zlib.
// /MANIFEST:EMBED /BASE:0x20000000 /DYNAMICBASE:NO. The original PE is mapped
// read-only by the bootstrap child; no original code or game loop runs.
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include <iostream>
#include <iterator>
#include <stdexcept>
extern "C" {
#include <lua.h>
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
            if (!GetModuleFileNameW(nullptr, executable, std::size(executable)))
                throw std::runtime_error("missing probe path");
            GameNativeDataBootstrapChild child(executable,
                L"--child \"" + std::filesystem::path(argv[1]).wstring() + L"\"",
                spans, std::size(spans));
            child.reserve_and_resume();
            child.wait_for_mapping(30000);
            if (WaitForSingleObject(child.process_handle(), 30000) != WAIT_OBJECT_0)
                throw std::runtime_error("probe child timed out");
            DWORD result;
            if (!GetExitCodeProcess(child.process_handle(), &result))
                throw std::runtime_error("missing child result");
            if (result == 0)
                std::cout << "PASS: c987 FindEntity scene hierarchy, sibling walk, lookup priority, visibility and runtime fallback\n";
            return static_cast<int>(result);
        }
        if (argc < 3 || std::string(argv[1]) != "--child")
            throw std::runtime_error("expected original PE path");
        auto reservation = accept_native_data_handoff(spans, std::size(spans));
        GameNativeReadOnlyData data(argv[2], spans, std::size(spans), std::move(reservation));
        GameHostLog log;
        GameSingletonHost singletons(log);
        GameVfsHost vfs(log, singletons, data, argv[2]);
        GameMissionLuaHost mission(log, vfs);
        GameSceneContentsHost scene(log, vfs);
        // Fixture injection into the real scene owner. Production fills these
        // fields in SceneReaderBinding::instantiate_entity, before Lua attach.
        auto& records = const_cast<std::vector<GameSceneEntityRecord>&>(scene.entities());
        const auto add_record = [&](const char* name, std::size_t id, std::size_t parent,
            bool generated = true, bool created = true) {
            GameSceneEntityRecord record;
            record.name = name;
            record.scene_id = id;
            record.parent_scene_id = parent;
            record.generated = generated;
            record.created = created;
            records.push_back(std::move(record));
        };
        add_record("RootCarrier", 10, 0);
        add_record("Decoy", 11, 10);
        add_record("Turret", 12, 10);
        add_record("Secret", 13, 10);
        add_record("airFIELD", 14, 0);
        add_record("AIRFIELD", 15, 0);
        add_record("SharedName", 16, 10);
        add_record("SharedName", 17, 0);
        // Hidden/gate-rejected records are retained as authored templates,
        // without an instantiated node. A live/findable leaf is insufficient
        // evidence for traversing its unavailable intermediate parent.
        add_record("HeldBack", 18, 10, false, false);
        add_record("LiveDescendant", 19, 18);
        mission.create_state();
        mission.create_self_table_004e0305();
        const std::vector<GameMissionLuaHost::SceneEntity> entities{
            {"RootCarrier", 101, -1, true}, {"Decoy", 102, -1, true},
            {"Turret", 103, -1, true}, {"Secret", 104, -1, false},
            {"airFIELD", 105, -1, true}, {"AIRFIELD", 106, -1, true},
            {"RuntimeOnly", 107, -1, true}, {"SharedName", 108, -1, true},
            {"LiveDescendant", 109, -1, true}
        };
        if (mission.attach_scene_entities_00928a00(entities) != entities.size())
            throw std::runtime_error("entity attach failed");
        auto* state = mission.mission_lua_state();
        const auto check = [&](const char* query, int expected) {
            lua_settop(state, 0);
            lua_pushstring(state, query);
            const bool found = mission.push_resolved_entity(state, "FindEntity", 1);
            if (found != (expected != 0)) throw std::runtime_error(query);
            if (found) {
                lua_getfield(state, -1, "Ptr");
                if (reinterpret_cast<std::uintptr_t>(lua_touserdata(state, -1))
                    != static_cast<std::uintptr_t>(expected)) throw std::runtime_error(query);
            }
        };
        check("airFIELD", 105);            // exact wins over another case-folded name
        check("AiRfIeLd", 106);            // existing map-order tie break
        check("rootcarrier\\turret", 103); // real sibling 1, through 00467F30
        check("RootCarrier\\Secret", 0);  // hierarchy hit still needs a findable id
        check("RootCarrier\\Missing", 0);
        check("Other\\Turret", 0);
        check("runtimeonly", 107);         // no authored scene-node source
        check("SharedName", 108);          // retain the existing flat-name identity
        check("RootCarrier\\SharedName", 0); // duplicate scene names lack a unique Lua id
        check("LiveDescendant", 109);       // the descendant really has a findable Lua id
        check("RootCarrier\\HeldBack\\LiveDescendant", 0);
        std::cout << "PASS: c987 FindEntity scene hierarchy, sibling walk, lookup priority, visibility and runtime fallback\n";
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
