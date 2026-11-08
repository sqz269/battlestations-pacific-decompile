// One headless source regression: fresh mission bounds must precede InitAll's
// real squadron pass-C hook. Link rebuilt bsp_game objects except game_main.obj.
// The original PE is mapped as read-only data; no game or original code runs.
#include "bsp/game_hosts_units_internal.hpp"
#include "bsp/game_hosts_mission_frame.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include <float.h>
#include <cmath>
#include <fstream>
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
bsp::SceneProperty number(const char* key, float value) {
    bsp::SceneProperty p;
    p.key = key; p.type_letter = "F"; p.has_float = true; p.float_value = value;
    return p;
}
bsp::SceneProperty point(std::string key, float x, float z) {
    bsp::SceneProperty p;
    p.key = std::move(key); p.type_letter = "V3";
    p.has_vector3 = true; p.vector3 = {x, 0.0f, z};
    return p;
}
}

int main(int argc, char** argv) {
    try {
        _control87(_PC_24, _MCW_PC);
        using namespace bsp::game;
        constexpr GameNativeDataSpan spans[] = {
            {0x00ce2000u, 1}, {0x00cf0000u, 1}, {0x00d00000u, 1},
            {0x00d10000u, 1}, {0x00d50000u, 1}, {0x00d60000u, 1}, {0x00d70000u, 1}
        };
        if (argc >= 2 && std::string(argv[1]) != "--child") {
            wchar_t executable[32768];
            require(GetModuleFileNameW(nullptr, executable, std::size(executable)) != 0,
                "missing probe executable path");
            const bool late = argc > 2 && std::string(argv[2]) == "--late";
            GameNativeDataBootstrapChild child(executable,
                L"--child \"" + std::filesystem::path(argv[1]).wstring() + L"\""
                    + (late ? L" --late" : L""), spans, std::size(spans));
            child.reserve_and_resume();
            child.wait_for_mapping(30000);
            require(WaitForSingleObject(child.process_handle(), 30000) == WAIT_OBJECT_0,
                "probe child timed out");
            DWORD result;
            require(GetExitCodeProcess(child.process_handle(), &result) != 0,
                "missing probe child result");
            return static_cast<int>(result);
        }
        require(argc >= 3, "expected original PE path or bootstrap child");
        const bool late = argc > 3 && std::string(argv[3]) == "--late";
        auto reservation = accept_native_data_handoff(spans, std::size(spans));
        GameNativeReadOnlyData data(argv[2], spans, std::size(spans), std::move(reservation));
        GameHostLog log;
        require(log.open("local/output/c787-fresh-load-probe.log"), "cannot open probe log");
        GameSingletonHost singletons(log);
        GameVfsHost vfs(log, singletons, data, argv[2]);
        GameMissionLuaHost mission(log, vfs);
        mission.create_state();
        lua_State* state = mission.mission_lua_state();
        require(state != nullptr, "missing Lua state");
        lua_pushcfunction(state, luaopen_base);
        lua_pushstring(state, "");
        lua_call(state, 1, 0);
        require(luaL_dostring(state, "VehicleClass = {}; thisTable = {}") == 0,
            "fixture Lua globals failed");
        require(mission.world_border_bounds() == nullptr, "fixture is not a fresh load");

        // Explicit source-fixture Map and unit data, not installed mission evidence.
        // Multiplayer corners differ from the campaign size-derived selection.
        bsp::ScenePropertyBlock root, map, multi;
        map.values = {number("BorderSizeX", 200.0f), number("BorderSizeY", 400.0f)};
        for (const char* name : {"IslandCapture1v1", "IslandCapture2v2", "IslandCapture3v3",
                "IslandCapture4v4", "Duel", "Escort", "Siege", "Competitive"}) {
            multi.values.push_back(point(std::string(name) + "_nw", -10.0f, 20.0f));
            multi.values.push_back(point(std::string(name) + "_se", 10.0f, -20.0f));
        }
        map.blocks.emplace_back("MultiPlayMapSizes", std::move(multi));
        root.blocks.emplace_back("Map", std::move(map));

        GameUnitsHost units(log, mission);
        GameSceneEntityRecord entity;
        entity.created = true; entity.name = "water"; entity.type_id = 1;
        entity.world[0] = entity.world[5] = entity.world[10] = entity.world[15] = 1.0f;
        entity.world[12] = 500.0f; entity.world[13] = 7.0f; entity.world[14] = -500.0f;
        units.create_units({entity});
        auto* slot = static_cast<GameUnitSlot*>(const_cast<void*>(units.unit_identity(0)));
        require(slot != nullptr, "fixture unit was not constructed");
        // Seed only the state that selects pass C's water arm. The command
        // owner, queue, descriptor and delivery path remain production code.
        slot->class_id = 0x0f;
        slot->state->class_id = 0x0f;
        slot->plane_control_mode_900 = 6;
        slot->motion_class.hull_height = 8.0f; // 8 - 0.5 > y7 selects Water in 007C6340.
        slot->message_ready_00bd = 1;
        bsp::plane_squadron_registry().clear();
        auto& squadron = bsp::plane_squadron_registry().add("water");
        squadron.squadron_unit = 0; squadron.member_units = {0};
        squadron.member_names = {"water"}; squadron.member_spawn_index = {0};
        mission.attach_units_hooks(&units);

        if (!late)
            require(publish_scene_load_map_bounds(mission, root, 0, false, 0),
                "current mission Map bounds were not published");
        units.commands().begin_loopback_drain_0076c600();
        mission.run_scene_load_init_all_0046eb4b({{"water", 1, 1, true}});
        units.commands().finish_loopback_drain_0076c600();
        require(slot->plane_control_mode_900 == 6, "fixture placement did not select Water");
        if (late)
            require(publish_scene_load_map_bounds(mission, root, 0, false, 0),
                "late publication failed");

        const auto& rows = units.commands().rows();
        const auto found = std::find_if(rows.begin(), rows.end(), [](const auto& row) {
            return row.source == "squadron_pass_c" && row.command == "stop" && row.issued;
        });
        require(found != rows.end(), "fresh-load water squadron lost its initial stop command");
        require(found->slot_pushed, "initial stop was not delivered into a director slot");
        // 007F2920 places Water at y0. The production command-message codec
        // quantizes x/z (99 -> 99.000702, -199 -> -199.000610 in this build).
        require(std::fabs(found->descriptor_position[0] - 99.0f) < 0.002f
                && found->descriptor_position[1] == 0.0f
                && std::fabs(found->descriptor_position[2] + 199.0f) < 0.002f,
            "initial stop target did not use the current campaign bounds and one-unit inset");
        require(std::count_if(rows.begin(), rows.end(), [](const auto& row) {
            return row.source == "squadron_pass_c" && row.command == "stop" && row.issued;
        }) == 1, "initial stop command was issued more than once");
        mission.attach_units_hooks(nullptr);
        bsp::plane_squadron_registry().clear();
        std::ofstream("local/output/c787-fresh-load-probe-result.txt")
            << "PASS: fresh-load InitAll/pass C delivered one quantized stop near (99, 0, -199)\n";
        std::cout << "PASS: fresh-load InitAll/pass C delivered one quantized stop near (99, 0, -199)\n";
    } catch (const std::exception& error) {
        std::ofstream("local/output/c787-fresh-load-probe-result.txt")
            << "FAIL: " << error.what() << '\n';
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
