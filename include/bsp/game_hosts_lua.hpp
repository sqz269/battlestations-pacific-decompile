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

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bsp/gameplay_settings_tail.hpp"
#include "bsp/air_operations.hpp"
#include "bsp/lua_spawn_new.hpp"
#include "bsp/mission_load_hosts.hpp"
#include "bsp/mission_lua_host.hpp"
#include "bsp/ship_ai_obstacle_tables.hpp"
#include "bsp/game_tuning_singleton.hpp"
#include "bsp/unit_rudder_curve.hpp"
#include "bsp/vehicle_class_lua_load.hpp"

struct lua_State;

namespace bsp {
class VfsLocaleRuntime;
struct ShipAiPathSearchTurnRamp;
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
    float width{0.0f};                       // class+A4h, Width at00960368
    // class+A8h, the `Height` key 00960230 writes beside `Length`. Milestone 2r
    // reads it because 00826866 places the keel sample point at half the hull
    // height below the pose and 00937C90's hull body needs neither, so the two
    // keys are the same reader's pair. A missing key stores 0.
    float height{0.0f};                      // class+A8h
    // class+B0h, the `Mass` key 00960230 writes at 0096043A with the default
    // 1.0f. 00937C90 puts it in the body descriptor's +04h at 009399F7 and
    // 00937CF1 compares it against 100.0 to choose the physics material.
    float mass{0.0f};
    // The plane rate and acceleration keys 007D1F70 reads into the plane class
    // descriptor. src/plane_class_fields.cpp carries the store address for each
    // one; the key spellings here are that reader's, not guesses. They are zero
    // on a ship row, which is correct - only a plane row carries them.
    //
    // CAVEAT that has to travel with the numbers: vehicleclasses.lua is the one
    // file in this installation's scripts/datatables that carries a local
    // modification date, so these are this installation's plane rates and not
    // provably retail. The tuning in planeglobals.lua is a separate question
    // (docs/PLANE_CONTROL_RATE_LAW.md).
    float roll_spd{0.0f};             // desc+1A8h, 007D2530
    float pitch_spd{0.0f};            // desc+1ACh, 007D2569
    float yaw_spd{0.0f};              // desc+1B0h, 007D25A2
    float yaw_roll_ratio{0.0f};       // desc+1B4h
    float slide_ratio{0.0f};          // desc+1B8h
    float roll_accel{0.0f};           // desc+1BCh
    float pitch_accel{0.0f};          // desc+1C0h, 007D2731
    float yaw_accel{0.0f};            // desc+1C4h
    float negative_pitch_ratio{0.0f}; // desc+1D8h
    float plane_stall_spd{0.0f};      // desc+184h, 007D2351
    float turn_roll_spd{0.0f};        // desc+1C8h, 007D25DB
    float turn_roll{0.0f};            // desc+25Ch, 007D289B - the bank normaliser
    // The three aerodynamic keys 007D1F70 also reads, in the spellings
    // src/plane_class_fields.cpp:279-292 records against their writers.
    // 007DBD3A pairs XDrag with the body lateral velocity ctl+3Ch and
    // 007DBD50 pairs YDrag with ctl+40h, so these two ARE the coupling that
    // turns a plane's velocity onto its nose; 007C6340 seeds the spawn
    // airspeed from TravelSpeed and 009F9D30 divides MaxSpd by
    // Pilot/Torpedo/ReferenceSpeed for the run profile's distance scale.
    float x_drag{0.0f};               // desc+174h XDrag, 007D2189
    float y_drag{0.0f};               // desc+170h YDrag, 007D2150
    float max_spd{0.0f};              // desc+188h MaxSpd, 007D238A
    float travel_speed{0.0f};         // desc+18Ch TravelSpeed, 007D23C3
    // The four the thrust and drag accelerations are built from. 007C4990 makes
    // the drag coefficient desc+50Ch out of two of them, Accel / MaxSpd^2, which
    // is what puts a plane's equilibrium airspeed exactly on MaxSpd.
    float accel{0.0f};                // desc+164h Accel, 007D20C6
    float glide_rate{0.0f};           // desc+208h GlideRate, 007D2B10
    float drag_pitch_ratio{0.0f};     // desc+1D4h DragPitchRatio, 007D2829
    float air_brake_drag{0.0f};       // desc+1DCh AirBrakeDrag, 007D226D
    // desc+1F0h DropAngle, the gain AND the cap of 009FB800's dive arm. Its
    // climb twin desc+1ECh has no key in any shipped row, which is why an AI
    // plane dives toward a lower commanded altitude but never climbs toward a
    // higher one through that routine (docs/PLANE_FLIGHT.md, "009FB800").
    float drop_angle{0.0f};
    // desc+194h SwimHeight, 007D2413. One of the two terms of the free-flight
    // arm's water line; the other, desc+508h, is derived at 007C4D03.
    float swim_height{0.0f};
    // desc+198h MinWaterSpd. 007CB7F0 (007CB81A) leaves a live AI aircraft of a
    // class with a non-zero value in free flight when it touches the water.
    // Packet cc9_water_surface_law.
    float min_water_spd{0.0f};
    // desc+268h TurnCircleRadius. The dive-bomb approach constructor 009C3EA0
    // multiplies it twice, at 009C3F86 into approach+B4h and at 009C3FB5 into
    // approach+B8h/+BCh. Packet cc8_dive_race.
    float turn_circle_radius{0.0f};
};

// Actual selected class+570 bits and the existing producer's provenance.
// A successful depth read initializes every field; false leaves output intact.
// Native 0083B5E0/00837DE0 and the ship-leaf tails are projected only for this
// scalar. This is not the full settings singleton or a descriptor replacement.
struct GameShipDepthInput {
    std::uint32_t class_reference_0570;
    std::uint32_t settings_block_offset; // 80h for session zero, F0h otherwise
    std::uint32_t scalar_source;         // selected kShipLeafTuningSources slot
    const char* class_key;               // static producer key, e.g. Destroyer
};

// Same reader on an explicitly borrowed live interpreter. The conversion mode
// is the existing 0109EEA4 projection required by native_lua_integer_00b66290.
// Used by the host below and focused installed-data verification. Restores the
// Lua stack and leaves output unchanged on failure; does not execute scripts.
bool read_ship_depth_input_lua(lua_State&, int type_id, std::int32_t session_mode,
    const bool& crt_sse2_conversion, GameShipDepthInput&, std::string& error);

// Selected class+560h..56Ch and +570h, using the existing native leaf source
// table. Partial settings projection: no whole singleton or descriptor is made.
// Surface leaves replicate Lua element2; Submarine retains elements2..5.
struct GameShipNavigationInput {
    ShipLeafTuning tuning;
    std::uint32_t settings_block_offset; // 80h for session zero, F0h otherwise
    const char* class_key;
};

bool read_ship_navigation_input_lua(lua_State&, int type_id, std::int32_t session_mode,
    const bool& crt_sse2_conversion, GameShipNavigationInput&, std::string& error);

// Settings+1F4h..208h: moveMin, moveMax, shipMin, shipMax, travelMin, travelMax.
// Bare native Number conversion; no fallback or bounds adjustment. Both new
// borrowed readers restore the stack and preserve output on protected errors.
// They read the current interpreter without running scripts.
bool read_ship_layer_timing_input_lua(lua_State&, std::array<float, 6>&,
    std::string& error);

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
    std::size_t party_mirrors{0};         // slots 00928f50's mirror wrote `Party` on
    unsigned long long entity_resolves{0};  // 0089903c's resolved arm
    unsigned long long native_calls{0};
    // The three objective bindings. docs/MISSION_OBJECTIVES.md.
    unsigned long long objective_binding_calls{0};
    unsigned long long objective_units_touched{0};
    // 0088BF80 GetProperty. docs/MISSION_LUA_GETPROPERTY.md. `served` counts the
    // calls whose key one of the reconstructed readers answered; `unserved` the
    // keys that reach the arm the native leaves empty.
    unsigned long long get_property_calls{0};
    unsigned long long get_property_served{0};
    unsigned long long get_property_unserved{0};
    unsigned long long get_property_slots_rows{0};
    // 00895D20 and 0089E3C0. docs/AIROPS_LAUNCH_GATES.md.
    unsigned long long air_ops_ready_calls{0};
    unsigned long long air_ops_ready_true{0};
    unsigned long long air_ops_launch_calls{0};
    unsigned long long air_ops_launch_started{0};
    unsigned long long air_ops_launch_queued{0};
    // 006C5050 through the factory seam. docs/AIROPS_LAUNCH_TICK.md.
    unsigned long long air_ops_squadrons_created{0};
    unsigned long long air_ops_squadron_key_pushes{0};
    // 00944FD0. docs/LUA_GENERATE_OBJECT_HOST.md.
    unsigned long long generate_object_calls{0};
    unsigned long long generate_object_created{0};
    unsigned long long generate_object_repeat{0};
    unsigned long long generate_object_unknown{0};
    // 0094C480 / 00949750 / 0094C490. docs/LUA_SPAWN_NEW_HOST.md.
    unsigned long long spawn_new_calls{0};       // tables the binding accepted
    unsigned long long spawn_new_rejected{0};    // argument 1 was not a table
    unsigned long long spawn_new_queued{0};      // records linked at 00949530
    unsigned long long spawn_new_attempts{0};    // 0094C490 passes that took one
    unsigned long long spawn_new_fulfilled{0};   // records that reached +C0h = 1
    unsigned long long spawn_new_requeued{0};    // 009478B0 pushes
    unsigned long long spawn_new_units{0};       // entities appended at +CCh
    unsigned long long wing_member_tables{0};    // cc9_mission_end, 00928A00 per wing plane
    unsigned long long spawn_new_callbacks{0};   // named globals actually called
    unsigned long long spawn_new_callback_missing{0};
    std::vector<GameMissionNativeCall> natives; // distinct, in first-call order
    std::string first_error;
    std::string first_error_phase;
};

// The mission Lua machine for one run. It is built the way 00884be0 builds it
// and it outlives the load, because 004dd627 constructs it once per process.
// Packet cc8_airops_launch_tick: this host is also what 006C5050's creator seam
// resolves to, because the squadron has to reach the mission script's own table
// and that table is this host's.
class GameMissionLuaHost final : public bsp::MissionLuaHostServices,
                                 public bsp::AirOpsSquadronFactory,
                                 public bsp::SpawnQueueDrain {
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

    // The float twin of the reader above, for the `VehicleClass[index]` fields
    // 00960230 BSP_VehicleClass_ReadLuaFields stores as floats rather than
    // integers. The first caller is `ReconModifier`: 0096239A pushes the key at
    // 00D1AAF4, 009623A9 fetches the field, 009623AE FLD1 supplies the 1.0f
    // default, and 009623CF FMUL ST0,ST0 squares it before 009623D9 stores it
    // at class+B8h. Squaring is the CALLER's job here, because the field table
    // in src/vehicle_class_fields.cpp records the Lua value, not the square.
    // src/vehicle_class_fields.cpp lists the other float fields at 009623A9's
    // sibling offsets; this reader serves all of them.
    float read_vehicle_class_number(int index, const char* key, float fallback);

    // `Bullets[index][key]` from the live Lua state - the bullet class table that
    // Scripts/datatables/autoload/bulletclasses.lua publishes. Used for the
    // fields the flattened per-platform BSPGun table does not carry, notably
    // "FlyTime" and "WaterTravelSpeed". docs/TORPEDO_CATEGORY_ADMISSION.md.
    float read_bullet_class_number(int index, const char* key, float fallback);

    // `Bullets[index][key]` as a string; "" when absent. Used for `Type`.
    std::string read_bullet_class_string(int index, const char* key);

    // `DeviceClass[index][key]` as a string; "" when absent or not a string.
    // Used for the device row's `Mesh` model path. docs/GUN_BARREL_COUNT.md.
    std::string read_device_class_string(int index, const char* key);

    // The whole of a mounted resource through the VFS the scripts are read
    // from (mode 2, the script read mode); false when it does not open.
    bool read_resource_file(const std::string& path, std::vector<std::uint8_t>& bytes);

    // Milestone 2j. The head of the gameplay settings loader 0083b5e0: it
    // formats `Scripts\datatables\ShipGlobals.lua` (the literal at 00d0b67c)
    // into a path at 0083b6c3, runs it through the Lua state owner's own runner
    // 00b69d40 at 0083b6e6, and takes the `ShipGlobals` global (00d0b670)
    // through 00b67980 / 00b67800 at 0083b721 / 0083b73d. The executable has one
    // Lua state, the mission machine's, so it runs the file through the
    // recovered file runner 00885110 on that state and records 00b69d40.
    // Returns true when the global is a table afterwards.
    bool load_ship_globals_0083b6e6();

    // 007E2A20 BSP_GameTuning_LoadFromPlaneGlobals, driven by the reconstruction
    // in bsp/game_tuning_singleton.hpp over the live interpreter. The native
    // runs Scripts/datatables/PlaneGlobals.lua in a PRIVATE Lua state and copies
    // 423 key paths into a 6D0h-byte singleton, whose first 312 bytes it then
    // mirrors into the globals at 00F872F0 with one REP MOVSD at 007EAAE1 - the
    // rotation factors 007DA710 reads are three of them
    // (docs/PLANE_CONTROL_RATE_LAW.md). This process has one Lua state, so the
    // script runs on the mission state the same way ShipGlobals does.
    //
    // Returns true when the `PlaneGlobals` global is a table afterwards. The
    // block is left at its defaults when it is not; the native has no such
    // fallback, because operator new hands it raw storage and it would read
    // whatever was there.
    bool load_plane_globals_007e2a20();
    // The loaded block. All zeroes until load_plane_globals_007e2a20 succeeds.
    const GameTuningBlock& plane_globals() const noexcept { return plane_globals_; }
    bool plane_globals_loaded() const noexcept { return plane_globals_loaded_; }

    // Stored settings+4 projection: loader store 0083BCD5, mutable mission
    // binding store 008D0852. False means no producer has established it and
    // leaves output unchanged. This is not a per-frame ShipGlobals lookup.
    bool read_avoid_all_ship_collision(bool& value) const noexcept;
    void set_avoid_all_ship_collision_008d0852(bool value);
    // Stored +194,+1D4,+1D8,+214,+218 snapshot from the represented load.
    bool read_avoidance_tuning(std::array<float, 5>& values) const noexcept;
    // Packet cc9_ship_neighbour_list: the ShipAvoidance block settings+190h..+1D8h,
    // nineteen floats in offset order ((offset - 190h) / 4), read with the same
    // getters and loader defaults 0083B7AD..0083BCB8 use. False until loaded.
    bool read_ship_avoidance_block(std::array<float, 19>& values) const noexcept;
    // Packet cc9_plane_death_modes. A global two-number table such as
    // planepartclasses.lua's ExplosionExplosionDelay = {0.6, 1.8}, which
    // 004A9BD0 reads into [00E18710]/[00E1870C]. False when the global is
    // absent or not a table of two numbers.
    bool read_global_number_pair(const char* name, float& first, float& second);
    // Packet cc9_hit_accuracy: the four WeaponHitAccuracy sub-objects at
    // settings+240h/+298h/+2F0h/+348h as 0083C795..0083C919 fills them. False
    // until ShipGlobals ran; the caller then keeps the 00836EF0 defaults.
    bool read_weapon_hit_accuracy(bsp::WeaponHitAccuracyProfile (&out)[4]) const noexcept;
    void load_weapon_hit_accuracy_0083c795();

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

    // Packet cc9_unit_instance_step11. The EngineSoundSmoothRate of the four
    // engine-sound records 0083B5E0 fills at settings+5BCh + i*24h + 8h
    // (008405D5), off `ShipGlobals["Sounds"][name]` for name = Ship, TBoat,
    // Submarine, Plane (jump table 00842954). An absent key keeps the 0.2f
    // default 00B66330 is handed (00CE54A0). False when `Sounds` or a record
    // table is missing; `out` then keeps what it held.
    bool read_engine_sound_smooth_rates_0083b5e0(float (&out)[4]);

    // Read the recovered0083D492..0083D575 fragment on this actual Lua state.
    // Parent lookup errors return false with text; caller must reject the load.
    // Successful reads include the native per-field non-number fallbacks.
    bool read_path_turn_ramp(ShipAiPathSearchTurnRamp& out, std::string& error);

    // VehicleClass[type_id].Type selects one of the eight existing ship leaves;
    // HeavyCruiser/BigLandingShip use native exact-Boolean-or-false semantics.
    // Reads ShipGlobals.AvoidZoneDepthsSingle/Multi[class_key][1] through the
    // native Lua wrappers. The bare final GetInteger preserves numeric-string,
    // missing-value and nonnumeric conversion behavior; it adds no depth default.
    // Unsupported/non-ship types and failed parent lookups return false with an
    // error and preserve output. Call before AI construction with actual session
    // mode; an unbound class scalar must not be replaced with a fabricated zero.
    bool read_ship_depth_input(int type_id, std::int32_t session_mode,
        GameShipDepthInput& out, std::string& error);

    // Full selected ShipLeafTuning, sharing the scalar reader's Type/variant
    // mapping and protected parser. docs/GAME_SHIP_LAYER_INPUT.md.
    bool read_ship_navigation_input(int type_id, std::int32_t session_mode,
        GameShipNavigationInput& out, std::string& error);
    bool read_ship_layer_timing_input(std::array<float, 6>& out, std::string& error);

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
    // Packet cc_lua_find_entity: `findable` separates the two questions the
    // milestone 2l code ran together. Having a `thisTable` slot is decided by
    // entity virtual slot 39, which BSP_SEntity_InitAll 00925F20 calls on every
    // pending entity at 0092604E with no class filter; being answerable by
    // `FindEntity` is decided by 0088B1B0, which walks 14 of the 97 world
    // buckets (docs/LUA_BINDING_ENTITY_LOOKUP.md). `MovieCamPos` and
    // `MovieCamLookat` get a slot and are not findable, so the name index and
    // the slot table are built from different sets.
    struct SceneEntity {
        std::string name;
        int id{0};
        int class_index{-1};
        bool findable{true};
        // Packet cc8_ship_drive. 00928A00 does not seed these: 00928F50
        // BSP_MissionEntity_SetPartyRaceLuaMirror does, on the object 00927B40
        // answers with, through 00B67460 with the field names `Race` (00928FD9)
        // and `Party` (00929046), and it reads them off the entity rather than
        // off its own arguments (00928FC7 and 00929034 both load from ESI). The
        // shipped `commandhelpers.lua` indexes `recon[targetUnit.Party]` at 330,
        // 494 and 518, so a slot without `Party` makes every one of those raise.
        // A negative value means the caller does not know it and the mirror does
        // not run, which is the state of every marker.
        int party{-1};
        int race{-1};
    };
    std::size_t attach_scene_entities_00928a00(const std::vector<SceneEntity>& entities);

    // Packet cc_lua_find_entity: the `recon` shell, through the already
    // reconstructed bsp::install_recon_values_00803a40. 004E0305 sets the global
    // to nil on the mission-load pass, and on that path the native's route back
    // to a table is 00806B10, whose 006B8190 / 00803750 / 008037D0 descent
    // recreates whatever is nil before 00805D90 fills the nineteen category maps
    // (docs/RECON_SLOT_LISTS.md section 4). 00806B10 is driven by the recon slot
    // lists, which this process does not build, so it stays a record: what the
    // executable supplies is the empty shell 00803A40 builds, and every category
    // map is empty because no unit was ever detected. Without it the shipped
    // `luaGetOwnUnits` (commandhelpers.lua:12343) raises on the first index of
    // `recon`, which is what ended the run's mission-complete path.
    void install_recon_tables_00803a40();

    // Packet cc_lua_find_entity: where the mission's own script ended up, read
    // off its `Mission` table at the end of the run. Nothing in the executable
    // writes that table; it is the shipped script's own state, which is why it
    // is the honest measure of how far the run carried the mission.
    void report_mission_script_state();

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

    // Packet cc8_ship_drive. 00928F50 BSP_MissionEntity_SetPartyRaceLuaMirror,
    // the other writer of a thisTable slot: `Party` at 00929046 and `Race` at
    // 00928FD9, both through 00B67460. Run once the orders host is attached,
    // because that is where this host can reach a unit's party. Answers how many
    // slots it wrote.
    std::size_t mirror_party_race_00928f50();

    // Called by the binding trampolines; public so the C callbacks can reach it.
    // `handled` says the row ran its reconstructed body rather than standing in
    // for a native one, which is what separates a concrete record from the
    // unimplemented policy.
    void note_native_call(std::size_t row, int argument_count, bool handled = false);

    // The three objective bindings' own line. docs/MISSION_OBJECTIVES.md.
    void note_objective_binding(const char* binding, const std::string& objective,
        unsigned int slot_mask, int units_touched);

    // 0088BF80 GetProperty. The native resolves argument 0 to an entity, reads
    // argument 1 as the key, and calls the entity's own reader at vtable+138h;
    // it pushes nothing of its own and returns that reader's result count.
    // docs/MISSION_LUA_GETPROPERTY.md.
    int run_get_property_0088bf80(lua_State* state, int argument_count);

    // 00895D20 IsReadyToSendPlanes and 0089E3C0 LaunchSquadron, the two gates
    // between the carrier deck and the mission script's launch line.
    // docs/AIROPS_LAUNCH_GATES.md.
    // 00851CB0's lookup: the globals, then `StationaryClass`, then the row by
    // the type's own text. A name that answers there is a stationary prop and
    // has no `VehicleClass` row by construction.
    // docs/SCENE_STATIONARY_UNITS.md.
    bool stationary_class_exists(const std::string& name);

    int run_is_ready_to_send_planes_00895d20(lua_State* state, int argument_count);
    int run_launch_squadron_0089e3c0(lua_State* state, int argument_count);
    // 00944FD0 GenerateObject. Instantiates one of the entities the scene pass
    // held back at 0046D3C5, by name, and pushes its `thisTable` slot the way the
    // entity-returning tail 0089903C does. docs/LUA_GENERATE_OBJECT_HOST.md.
    int run_generate_object_00944fd0(lua_State* state, int argument_count);

    // Packet cc8_spawn_new_route. 0094C480 SpawnNew is a three-instruction thunk
    // into 00949750 on the manager at *(00F89B3C): it parses ONE Lua table,
    // queues a DCh-byte request and creates nothing. Returns no results, which
    // is what the native does and what every one of the fourteen call sites in
    // this installation's usn_19_coralus.lua expects - not one of them uses the
    // return value. docs/LUA_SPAWN_NEW_HOST.md.
    int run_spawn_new_00949750(lua_State* state, int argument_count);
    // The consumer, 0094C490, reached in GGame::OnMove step 20 through the
    // `CALL 0094C490; RET 4` thunk at 0094C8F0. One request per
    // `SpawnAttemptDelay`; a request that cannot be placed goes back on the
    // queue (009478B0) instead of being dropped. The step is the mission
    // frame's, because the native's own clock is the world time at DAT_00F876A4.
    void run_spawn_queue_0094c490(float step_seconds) override;
    void report_spawn_queue();

    // --- bsp::AirOpsSquadronFactory, packet cc8_airops_launch_tick ----------
    // 006C5050's seam. The unit is made by the script-orders host, which owns the
    // units host; what this adds is the `thisTable` slot, without which the
    // squadron the deck hands back is an id no binding can resolve.
    std::uint32_t create_squadron(const bsp::AirOpsSquadronRequest& request) override;
    // One more slot in the table 00928A00 filled at load, for a unit that did not
    // exist then. The native's own 00925F20 walk reaches every entity as it is
    // created, so a mid-mission unit gets its slot the same way.
    // Packet cc9_mission_end: the `thisTable` slot of every other unit the creator
    // just made (the squadron's wing members), keyed by unit id.
    void attach_wing_member_tables(std::size_t units_before, std::uint32_t leader_entity,
        int class_index);
    bool attach_created_entity_00928a00(int entity_id, const std::string& name,
        int class_index);
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
    // Packet cc8_spawn_new_route, second pass. An entity-returning row that
    // answers from push_resolved_entity is decided AFTER `handled`, so without
    // this it stayed UNIMPLEMENTED in the summary while resolving every call.
    // GameHostLog's record is sticky on first insert, so note_native_call must
    // not record a status for these rows and this must record exactly one.
    void note_entity_status(const bsp::MissionLuaBinding& binding, bool resolved);
    // Milestone 2m. The globals walk 004d3167 performs: 00b67980 opens the
    // table, 00b67080 / 00b67190 iterate it and 00b66200 is
    // `lua_type(value) == LUA_TFUNCTION`. One entry per key, in the order the
    // interpreter yields them.
    std::vector<bsp::LuaGlobalEntry> lua_global_entries();
    int run_dofile(const std::string& path);
    void note_error(const std::string& message);
    void set_phase(std::string phase);

private:
    // Packet cc8_spawn_new_route, the two halves of one drain pass.
    // 0094A140 -> 00949300 -> 009483D0: build the frame, create every member,
    // set record+C0h. 0094C777: the completion walk over record+CCh.
    void fulfil_spawn_request_009483d0(bsp::SpawnNewRequest& request);
    void complete_spawn_request_0094c777(const bsp::SpawnNewRequest& request);
    // globalConfig+2DCh, read once from Globals["SpawnAttemptDelay"].
    float spawn_attempt_delay_0087f800();

    struct OpenScript {
        std::string path;
        std::vector<std::uint8_t> bytes;
        bool open{false};
    };

    // 007E2A20's 6D0h-byte block, as far as kGameTuningKeys names it. Value
    // initialised rather than left raw, which is a deliberate divergence:
    // operator new at 0042E7A2 hands the native raw storage and it writes only
    // the keys, so a key the data file omits reads whatever was there. Zero is
    // the honest stand-in and plane_globals_loaded_ says whether any of it is
    // real.
    GameTuningBlock plane_globals_{};
    bool plane_globals_loaded_{false};

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
    bool avoid_all_ship_collision_{};
    bool avoid_all_ship_collision_loaded_{};
    std::array<float, 5> avoidance_tuning_{};
    bool avoidance_tuning_loaded_{};
    std::array<float, 19> ship_avoidance_block_{};
    bool ship_avoidance_block_loaded_{};
    bsp::WeaponHitAccuracyProfile weapon_hit_accuracy_[4]{};
    bool weapon_hit_accuracy_loaded_{};
    // Packet cc8_spawn_new_route. DAT_00F876A4, the world clock the drain
    // compares against manager+0Ch, accumulated from the mission frame's step
    // because this process has no world clock object of its own.
    float spawn_world_clock_{0.0f};
    float spawn_attempt_delay_{0.0f};
    bool spawn_attempt_delay_read_{false};
    unsigned spawn_requeue_logged_{0};
    GameMissionLuaSummary summary_;
};

}  // namespace bsp::game
