#pragma once
// bsp_game.exe milestone 2f: the mission Lua machine as a process binding.
//
// Addresses: 00884be0 (BSP_MissionLuaHost_Initialize, called at 004dd627 from
// BSP_Game_OnInitOnce), 006b8740 / 006b8610 / 006b89f0 / 006b8ad0 (the machine
// construction, the binding registration and the two chunk runners), 00b6a303
// with callback 00b69e00 (the DoFile global the binding table does not carry),
// 00886900 with 00886370 (the global script folders, called at 004dc72f from
// BSP_Game_ConstructGlobalSubsystems), 005e2f00 (the LobbySettings table,
// called at 004e02d0 from BSP_Game_LoadMissionScene), 00885110 / 00885fb0 /
// 008860b0 (the script file, its content variants and the mission chunk),
// 00887750 / 00887b30 / 00887e50 with 0045f440 and 0045f520 (the named-call
// rule and the two entry-point wrappers) and 00b66200 (the defined check).
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of bsp::MissionLuaHostServices, satisfied either by the repository's
// stock Lua 5.1.1 over the mounted virtual file system or by the explicit
// unimplemented policy in GameHostLog. The 560 bindings of the table at
// 00e0b7b8 are installed as real Lua globals whose bodies are host records: a
// script that calls one gets a logged call and a nil result, never invented
// game behaviour.
//
// Evidence: docs/MISSION_LUA_MACHINE.md, docs/MISSION_LUA_HOST.md,
// docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bsp/mission_lua_host.hpp"
#include "bsp/unit_rudder_curve.hpp"

struct lua_State;

namespace bsp {
class VfsLocaleRuntime;
}

namespace bsp::game {

class GameHostLog;
class GameVfsHost;

// One binding the running scripts actually reached, with the row address the
// table at 00e0b7b8 carries for it.
struct GameMissionNativeCall {
    std::string name;
    std::uint32_t address{0};
    unsigned long long calls{0};
    int last_argument_count{0};
};

// Milestone 2i. One row of the installed `VehicleClass` global, read out of the
// live Lua state the recovered global-script step 00886900 loaded
// `Scripts/datatables/autoload/vehicleclasses.lua` into. Only the keys the
// motion path of 00825f20 reads are taken; the full descriptor reader
// 00831840 / 00960230 is bsp/ship_class_fields.hpp's and needs a descriptor
// object this process does not build.
struct GameVehicleClassRow {
    bool found{false};
    int index{-1};
    std::string name;   // "Name"
    std::string type;   // "Type", the literal 00964790's string chain compares
    float max_speed{0.0f};                   // class+500h
    float max_accel{0.0f};                   // class+504h
    float retardation{0.0f};                 // class+508h
    float max_rot_angle{0.0f};               // class+4F8h
    float max_rot_angle_change_ratio{0.0f};  // class+4FCh
    float length{0.0f};                      // class+A0h, written by 00960230
    float mass{0.0f};
};

// One of the four names 004dfb70 invokes.
struct GameMissionEntryPointRun {
    std::string name;
    bool defined{false};   // 00b66200 through 0045f440 / 0045f520
    bool dispatched{false};
    int pcall_status{0};
    std::string error;     // recovered with errfunc 0; the native discards it
};

struct GameMissionLuaSummary {
    bool machine_started{false};
    bool self_table_created{false};    // thisTable, 004e0305
    std::size_t entity_returns{0};     // entity-returning bindings that pushed nil
    std::size_t libraries_opened{0};
    std::size_t bindings_registered{0};
    bool platform_chunk_ran{false};
    bool fundamentals_ran{false};
    bool dofile_installed{false};
    std::size_t global_folder_scripts{0};
    std::size_t global_folder_errors{0};
    bool lobby_settings_published{false};
    std::string mission_script_path;   // what 008860b0 built
    std::size_t mission_chunks_run{0}; // base plus content variants
    bool mission_chunk_ok{false};
    std::string mission_chunk_error;
    std::size_t dofile_calls{0};
    std::vector<std::string> dofile_paths;
    std::vector<GameMissionEntryPointRun> entry_points;
    unsigned long long native_calls{0};
    std::vector<GameMissionNativeCall> natives; // distinct, in first-call order
    std::string first_error;
    std::string first_error_phase;
};

// The mission Lua machine for one run. It is built the way 00884be0 builds it
// and it outlives the load, because 004dd627 constructs it once per process.
class GameMissionLuaHost final : public bsp::MissionLuaHostServices {
public:
    GameMissionLuaHost(GameHostLog& log, GameVfsHost& vfs);
    ~GameMissionLuaHost() override;
    GameMissionLuaHost(const GameMissionLuaHost&) = delete;
    GameMissionLuaHost& operator=(const GameMissionLuaHost&) = delete;

    // 00884be0 at 004dd627, then the fundamentals chunk whose bytes the host
    // owns (00884770) and the DoFile global from the owner layer at 00b6a303.
    bool start_machine_00884be0();
    // 00886900 at 004dc72f: Scripts/global/ then Scripts/datatables/autoload/.
    std::size_t run_global_script_folders_00886900();
    // 005e2f00 at 004e02d0.
    void publish_lobby_settings_005e2f00();
    // 004e0305: the load creates the `thisTable` self table and clears `recon`
    // on the same pass. docs/MISSION_LUA_SELF_TABLE.md.
    void create_self_table_004e0305();
    // 008860b0: "Scripts/missions/" + name + ".lua" through 00885fb0 with the
    // content variants enabled.
    bool run_mission_script_008860b0(const std::string& script_name);
    // 0045f520 (forced) and 0045f440 (thread safe).
    bool call_entry_point(const std::string& name, bool threadsafe);

    // Milestone 2i: `VehicleClass[index]`, from the table the autoload folder
    // of 00886900 already ran. `index` is the id the scene's
    // `Type = E ShipClasses : <symbol>` resolved to, which is the same number
    // the installed table indexes its rows by.
    GameVehicleClassRow read_vehicle_class_row(int index);

    // Milestone 2j. The head of the gameplay settings loader 0083b5e0: it
    // formats `Scripts\datatables\ShipGlobals.lua` (the literal at 00d0b67c)
    // into a path at 0083b6c3, runs it through the Lua state owner's own runner
    // 00b69d40 at 0083b6e6, and takes the `ShipGlobals` global (00d0b670)
    // through 00b67980 / 00b67800 at 0083b721 / 0083b73d. The executable has one
    // Lua state, the mission machine's, so it runs the file through the
    // recovered file runner 00885110 on that state and records 00b69d40.
    // Returns true when the global is a table afterwards.
    bool load_ship_globals_0083b6e6();

    // 0083ce56..0083d10d of 0083b5e0, driven by the reconstruction in
    // bsp/unit_rudder_curve.hpp over the live `ShipGlobals["Navigator"]` table.
    // `found` is false when the machine or either table is missing, and the
    // curve settings are then left untouched.
    bool read_turn_multipliers_0083ce56(UnitRudderCurveSettings& out);

    bool started() const noexcept;
    const GameMissionLuaSummary& summary() const noexcept;
    // Logs the distinct bindings the scripts reached, highest count first.
    void report_natives(std::size_t limit);

    // --- bsp::MissionLuaHostServices -------------------------------------
    void create_state() override;
    void set_panic_function(std::uint32_t function) override;
    void set_gc_pause(int what, int pause) override;
    void open_standard_library(const bsp::LuaStandardLibrary& library) override;
    void register_global_function(const bsp::MissionLuaBinding& binding) override;

    bool open_script(const std::string& path) override;
    int script_size() override;
    void read_script(char* buffer, int size) override;
    void close_script() override;
    std::vector<std::string> script_variant_names(const std::string& path) override;

    int lua_gettop() override;
    void lua_settop(int index) override;
    int luaL_loadbuffer(const char* buffer, int size, const char* chunk_name) override;
    int lua_pcall(int nargs, int nresults, int errfunc_index) override;
    std::string lua_tolstring_at_top() override;
    int collect_results(int count, int mode) override;

    void lua_getglobal(const char* name) override;
    void lua_pushstring(const char* text) override;
    void lua_gettable(int index) override;
    void lua_remove(int index) override;
    void lua_pushvalue(int index) override;
    void push_argument(const bsp::MissionLuaArgument& argument) override;
    bool global_is_defined(const char* name) override;
    int game_lifecycle_state() override;
    void adjust_call_stack_marker(int delta) override;
    void adjust_reentrancy_depth(int delta) override;
    bool on_frame_job_thread() override;
    void queue_named_call_for_main_thread(const std::string& name) override;

    // Called by the binding trampolines; public so the C callbacks can reach it.
    void note_native_call(std::size_t row, int argument_count);
    void note_entity_return();
    int run_dofile(const std::string& path);
    void note_error(const std::string& message);
    void set_phase(std::string phase);

private:
    struct OpenScript {
        std::string path;
        std::vector<std::uint8_t> bytes;
        bool open{false};
    };

    GameHostLog& log_;
    GameVfsHost& vfs_;
    // Declared before the adapter that borrows it: the adapter keeps a
    // reference to this list for its whole life.
    std::vector<std::string> content_suffixes_; // manager +48h/+4Ch, empty here
    std::unique_ptr<bsp::VfsLocaleRuntime> resources_;
    lua_State* state_{nullptr};
    OpenScript script_;
    std::string phase_;
    int call_stack_marker_{0};  // game+1A18h
    int reentrancy_depth_{0};   // 00f87900
    std::map<std::string, std::size_t> native_index_;
    GameMissionLuaSummary summary_;
};

}  // namespace bsp::game
