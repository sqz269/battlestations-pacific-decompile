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

#include "bsp/mission_load_hosts.hpp"
#include "bsp/mission_lua_host.hpp"
#include "bsp/ship_ai_obstacle_tables.hpp"
#include "bsp/unit_rudder_curve.hpp"

struct lua_State;

namespace bsp {
class VfsLocaleRuntime;
}

namespace bsp::game {

class GameHostLog;
class GameVfsHost;
class GameScriptOrdersHost;

// One binding the running scripts actually reached, with the row address the
// table at 00e0b7b8 carries for it.
struct GameMissionNativeCall {
    std::string name;
    std::uint32_t address{0};
    unsigned long long calls{0};
    int last_argument_count{0};
    // Milestone 2l: the created instances this binding was called on, by the
    // `ID` field of the entity table in its first argument. It is how the run
    // answers how many of the mission's ships its own script addresses, and it
    // is read off the call rather than chosen: every binding whose argument 1
    // is an entity table contributes, and no list of "order" bindings is
    // hand-picked.
    std::vector<int> entity_subjects;
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
    // class+A8h, the `Height` key 00960230 writes beside `Length`. Milestone 2r
    // reads it because 00826866 places the keel sample point at half the hull
    // height below the pose and 00937C90's hull body needs neither, so the two
    // keys are the same reader's pair. A missing key stores 0.
    float height{0.0f};                      // class+A8h
    // class+B0h, the `Mass` key 00960230 writes at 0096043A with the default
    // 1.0f. 00937C90 puts it in the body descriptor's +04h at 009399F7 and
    // 00937CF1 compares it against 100.0 to choose the physics material.
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
    // Milestone 2l: the names the mission script handed `CreateScript`, and
    // what running each one did. usn_2_java.lua's `luaStageInit` creates one,
    // `luaInit`, and that function is where the mission issues its own orders.
    std::vector<std::string> created_scripts;
    std::vector<GameMissionEntryPointRun> created_script_runs;
    std::size_t self_table_entities{0};   // thisTable slots 00928a00 would build
    unsigned long long entity_resolves{0};  // 0089903c's resolved arm
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

    // Milestone 2m. One integer field of `VehicleClass[index]`, optionally one
    // level down, for the two reads 0095c640 makes that the row above does not
    // carry: `LandingShip` (default 0) and `Catapult.LaunchedClass` (default
    // -1). The same plain table lookup against the live interpreter.
    int read_vehicle_class_integer(int index, const char* key, const char* nested_key,
        int fallback);

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

    // Milestone 2p. The seven AutoThrust keys of the same loader that
    // 009ec7c0 BSP_UnitBot_ComputeThrottleCeiling consumes, read off
    // `ShipGlobals["Navigator"]["AutoThrust"]` (docs/GAMEPLAY_SETTINGS.md rows
    // +6CCh, +6D0h, +6D4h, +6E0h, +6E4h, +6E8h and +6ECh, written by
    // 0083cc2c..0083ce3c). 0083cc2c itself is not projected; only its reads
    // run. False leaves the output untouched.
    bool read_auto_thrust_0083cc2c(ShipAiAutoThrustSettings& out);

    // Milestone 2k. The two reads 0087d7b0 makes into the global config object
    // 00432650 hands out: `Globals["Minimap"]["MinimapRange"]` into +6Ch and
    // `["VisibilityRange"]` into +70h (docs/HUD_MINIMAP.md). 0087d7b0 itself is
    // not reconstructed and its other seventy reads are not performed; this runs
    // `scripts/datatables/globals.lua` on the mission machine through the
    // recovered file runner 00885110 and takes those two numbers out of the
    // installed data. False leaves both outputs untouched.
    bool read_minimap_globals_0087d7b0(float& minimap_range, float& visibility_range);

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

    // Milestone 2l: the per-entity Lua tables 00928a00 builds. One slot of
    // `thisTable` per created scene instance, keyed by the decimal of the u16
    // at entity+174h, carrying the `ID`, `Dead` and `Ptr` fields that routine
    // seeds (docs/MISSION_LUA_SELF_TABLE.md). 00928a00 itself and its caller
    // 0077e830 are records: what the executable supplies is the slot, so the
    // entity-returning bindings can take the arm at 0089903c that pushes
    // thisTable[key] instead of the nil arm. Returns how many slots it made.
    // `class_index` is the `Type = E ShipClasses : <symbol>` id the enum
    // library resolved, which is the row index of the installed `VehicleClass`
    // global. docs/MISSION_LUA_SELF_TABLE.md records that `Class` is added to
    // the slot later by a per-kind setter through 00b675d0 and not by 00928a00;
    // that the value is the `VehicleClass` row is established by the shipped
    // scripts, which read `.Class.Type` against the literal set the rows' own
    // `Type` keys carry ("Cruiser", "Destroyer", "Fighter", ...) and also read
    // `.Class.Length`, `.Class.Name`, `.Class.Height` and `.Class.Width`, all
    // top-level keys of the same rows. The setter itself stays a record.
    struct SceneEntity {
        std::string name;
        int id{0};
        int class_index{-1};
    };
    std::size_t attach_scene_entities_00928a00(const std::vector<SceneEntity>& entities);

    // Milestone 2l: the script objects the mission's own stage init created.
    // The `CreateScript` binding body 00898750 is a record, so the trampoline
    // keeps the name it was handed and nothing else. Running each one is the
    // executable's stand-in for the script manager: 00898750 registers the
    // object and the fixed step's script rows 00888230 (00875e55) and 00929460
    // (00875e64) are what would call it, and none of the three is
    // reconstructed. Each named global is called once with one fresh table,
    // which is the `this` a script function takes. Returns how many ran.
    std::size_t run_created_scripts();

    // Milestone 2m: the host that runs the eight reconstructed binding bodies.
    // Attached once the created instances exist, because every one of the eight
    // addresses an entity. A row the host does not handle keeps milestone 2l's
    // record.
    void attach_script_orders(GameScriptOrdersHost* orders) noexcept;
    GameScriptOrdersHost* script_orders() const noexcept;

    // Called by the binding trampolines; public so the C callbacks can reach it.
    // `handled` says the row ran its reconstructed body rather than standing in
    // for a native one, which is what separates a concrete record from the
    // unimplemented policy.
    void note_native_call(std::size_t row, int argument_count, bool handled = false);
    void note_created_script(std::string name);
    void note_binding_subject(std::size_t row, int entity_id);
    // A failed named call is replayed once with errfunc 0 purely to recover the
    // message for the log. That replay is the executable's, not the game's, so
    // its binding calls are not counted twice.
    void set_error_replay(bool active) noexcept;
    bool error_replay() const noexcept;
    // One line per binding the scripts called on an entity table, and the count
    // of distinct created instances the mission's own script addressed.
    void report_entity_subjects();
    // The resolved arm of the entity tail, for `FindEntity` only: 00925a90's
    // own lookup is a record, so the executable walks the instances the
    // instantiate pass created and, on a hit, pushes that entity's thisTable
    // slot. Every other entity-returning row resolves its subject from game
    // state this process does not own and keeps the recovered nil arm.
    bool push_resolved_entity(lua_State* state, const char* binding_name, int argument_count);
    void note_entity_return();
    // Milestone 2m. The globals walk 004d3167 performs: 00b67980 opens the
    // table, 00b67080 / 00b67190 iterate it and 00b66200 is
    // `lua_type(value) == LUA_TFUNCTION`. One entry per key, in the order the
    // interpreter yields them.
    std::vector<bsp::LuaGlobalEntry> lua_global_entries();
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
    // Milestone 2l: the created instances by name, with the id their thisTable
    // slot is keyed by. This is the executable's stand-in for 00925a90.
    std::map<std::string, int> scene_entity_ids_;
    GameScriptOrdersHost* script_orders_{nullptr};
    bool error_replay_{false};
    GameMissionLuaSummary summary_;
};

}  // namespace bsp::game
