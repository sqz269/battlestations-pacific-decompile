// Review regression: link rebuilt bsp_game objects except game_main.obj, plus
// bsp_core/lua/zlib and Windows libraries. Use /MANIFEST:EMBED, /BASE:0x20000000
// and /DYNAMICBASE:NO. Maps the original PE only as read-only data; no game loop
// or original executable code runs. argv[1] is the original PE path.
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include "bsp/game_native_readonly_data.hpp"
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
            GameNativeDataBootstrapChild child(executable,
                L"--child \"" + std::filesystem::path(argv[1]).wstring() + L"\"",
                spans, std::size(spans));
            child.reserve_and_resume();
            child.wait_for_mapping(30000);
            if (WaitForSingleObject(child.process_handle(), 30000) != WAIT_OBJECT_0)
                throw std::runtime_error("probe child timed out");
            DWORD result;
            if (!GetExitCodeProcess(child.process_handle(), &result))
                throw std::runtime_error("missing probe child result");
            if (result == 0) std::cout << "PASS: cached class 8 Length=100, front distance=110\n";
            return static_cast<int>(result);
        }
        if (argc < 3 || std::string(argv[1]) != "--child")
            throw std::runtime_error("expected original PE path or bootstrap child");
        auto reservation = accept_native_data_handoff(spans, std::size(spans));
        GameNativeReadOnlyData data(argv[2], spans, std::size(spans), std::move(reservation));
        GameHostLog log;
        if (!log.open("local/output/wire_g5_c706_ship_camera_probe.log"))
            throw std::runtime_error("cannot open probe log");
        GameSingletonHost singletons(log);
        GameVfsHost vfs(log, singletons, data, argv[2]);
        GameMissionLuaHost mission(log, vfs);
        mission.create_state();
        lua_State* state = mission.mission_lua_state();
        if (!state) throw std::runtime_error("missing Lua state");
        lua_pushcfunction(state, luaopen_base);
        lua_pushstring(state, "");
        lua_call(state, 1, 0);
        // These distinct dimensions are from installed VehicleClass[8]:
        // Type=Submarine, Length=100, CameraDistanceFront/Side/Vertical=110.
        // The remaining row/settings are a bounded fixture, not game evidence.
        const char* fixture = R"lua(
            -- Unrelated settings sections remain indexable in this fixture;
            -- their scalar conversions are unused by the camera assertion.
            local settings
            settings = setmetatable({}, {__index = function() return settings end})
            ShipGlobals = settings
            VehicleClass = { [8] = { Type = "Submarine", Name = "tat", Length = 100,
                CameraDistanceFront = 110, CameraDistanceSide = 110,
                CameraDistanceVertical = 110 } }
        )lua";
        if (luaL_dostring(state, fixture) != 0)
            throw std::runtime_error(lua_tostring(state, -1));
        const auto* base = mission.loaded_vehicle_class_base_fields(8, "c706 camera probe");
        if (base == nullptr || base->length != 100.0f)
            throw std::runtime_error("00960230 did not retain class 8 Length=100");
        // A reread would now return 7/9. Verify the production cached path,
        // including its distinction between hull length and front distance.
        if (luaL_dostring(state,
                "VehicleClass[8].Length = 7; VehicleClass[8].CameraDistanceFront = 9") != 0)
            throw std::runtime_error(lua_tostring(state, -1));
        bsp::ShipClassCameraInputs camera{};
        camera.base_length = -1.0f;
        const bool read = mission.read_ship_class_camera_00831e0d(8, camera);
        if (!read || camera.base_length != 100.0f || camera.distance_front != 110.0f)
            throw std::runtime_error("cached class 8 Length=" + std::to_string(camera.base_length)
                + ", CameraDistanceFront=" + std::to_string(camera.distance_front));
        std::ofstream("local/output/wire_g5_c706_ship_camera_probe_result.txt")
            << "PASS: cached class 8 Length=100, CameraDistanceFront=110 after Lua changes\n";
        std::cout << "PASS: cached class 8 returns Length=100 and CameraDistanceFront=110 after Lua changes\n";
    } catch (const std::exception& e) {
        std::ofstream("local/output/wire_g5_c706_ship_camera_probe_result.txt")
            << "FAIL: " << e.what() << '\n';
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
