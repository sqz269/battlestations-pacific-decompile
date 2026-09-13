// bsp_game.exe milestone 2f: the mission Lua machine over the mounted virtual
// file system. See include/bsp/game_hosts_lua.hpp for the address list.
//
// The order is 00884be0's order and the reading rules are the reconstruction's:
// bsp::initialise_mission_lua_host drives the bring-up, bsp::run_lua_chunk the
// two embedded chunks, bsp::run_script_with_variants the mission chunk, and
// bsp::call_entry_point_if_defined the four named entry points. What this file
// supplies is the interpreter (the repository's stock Lua 5.1.1, which
// docs/MISSION_LUA_MACHINE.md establishes as the matched library once
// LUA_COMPAT_LSTR is 2), the file reads through the phase-2 mounts, and the
// binding bodies, which are host records rather than game behaviour.

#include "bsp/game_hosts_lua.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/global_script_folders.hpp"
#include "bsp/mission_lobby_settings.hpp"
#include "bsp/mission_lua_bindings.hpp"
#include "bsp/mission_lua_machine.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/ship_ai_path_turn_ramp.hpp"
#include "bsp/ship_ai_settings_block.hpp"
#include "bsp/game_ship_avoidance_tuning_lua.hpp"
#include "bsp/gameplay_settings.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/lua_numeric.hpp"
#include "bsp/vehicle_class_lua_load.hpp"
#include "bsp/vehicle_class.hpp"
#include "bsp/native_string.hpp"
#include "bsp/recon_values.hpp"
#include "bsp/vfs_locale_runtime.hpp"
#include "bsp/vfs_provider_manager.hpp"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

// lua.h defines lua_getglobal as a two-argument macro, which would swallow the
// one-argument host method of the same name. The method body uses lua_getfield
// with LUA_GLOBALSINDEX, which is exactly what the macro expands to and what
// 006b8460 performs.
#undef lua_getglobal

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <functional>
#include <type_traits>

namespace bsp::game {
namespace {

// 00885110 opens the script through [0109ceec] virtual +4h with mode 2.
constexpr std::uint32_t kScriptReadMode = 2;

// 006b8740 step 4, the pair table at 00cf8350 in its own order. The addresses
// are in bsp::kMissionLuaStandardLibraries; these are the matched library's
// openers for the same seven rows, and luaopen_package is absent from both.
const lua_CFunction kLibraryOpeners[bsp::kMissionLuaStandardLibraryCount] = {
    luaopen_base, luaopen_table, luaopen_io, luaopen_os,
    luaopen_string, luaopen_math, luaopen_debug,
};

GameMissionLuaHost* host_from_upvalue(lua_State* state) {
    return static_cast<GameMissionLuaHost*>(lua_touserdata(state, lua_upvalueindex(1)));
}

// One body for all 560 rows of 00e0b7b8. 006b8610 uses nup = 0 because every
// native row is a distinct function; the executable needs identity at call
// time, so it carries the host pointer and the row index as upvalues. The
// global is still a plain C closure under a plain name, which is the only part
// of the registration contract that is recovered.
int binding_trampoline(lua_State* state) {
    GameMissionLuaHost* host = host_from_upvalue(state);
    const int row = static_cast<int>(lua_tointeger(state, lua_upvalueindex(2)));
    const int argc = lua_gettop(state);
    if (host == nullptr) return 0;
    // Milestone 2m: the eight rows src/lua_binding_navigator.cpp reconstructs
    // run their own bodies over this process's created instances. Every other
    // row keeps milestone 2l's record.
    const bsp::MissionLuaBinding& dispatch_row =
        bsp::mission_lua_bindings()[static_cast<std::size_t>(row)];
    GameScriptOrdersHost* orders = host->script_orders();
    const bool avoidance_setting = dispatch_row.address == 0x008d0740u;
    const bool handled = avoidance_setting || (orders != nullptr
        && GameScriptOrdersHost::handles(dispatch_row.name));
    // The replay of a failed named call, which the executable makes only to
    // recover the error message, must not count a second time.
    if (!host->error_replay()) {
        host->note_native_call(static_cast<std::size_t>(row), argc, handled);
        // Milestone 2l. Which created instance a binding was called on, read
        // off argument 1 when it is an entity table. The `ID` field is the one
        // 00928a00 seeds, so this is the same identity the native carries.
        if (argc >= 1 && lua_type(state, 1) == LUA_TTABLE) {
            lua_getfield(state, 1, "ID");
            // `ID` is the key text (00928BA5 through 00B67630's lua_pushlstring).
            // lua_tonumber converts a numeric string, so both spellings resolve
            // and nothing else in this process depends on which one arrives.
            const int type = lua_type(state, -1);
            if (type == LUA_TNUMBER || type == LUA_TSTRING) {
                host->note_binding_subject(static_cast<std::size_t>(row),
                    static_cast<int>(lua_tonumber(state, -1)));
            }
            lua_settop(state, argc);
        }
        // Milestone 2l. `CreateScript` (row 00898750) is the one binding whose
        // argument the executable keeps: the mission's own stage init hands it
        // the name of the function that issues the mission's orders, and the
        // binding body is a record, so without the name there is nothing to run
        // later. The value is read, not invented, and no other row is read.
        if (std::strcmp(dispatch_row.name, "CreateScript") == 0 && argc >= 1
            && lua_type(state, 1) == LUA_TSTRING) {
            const char* name = lua_tolstring(state, 1, nullptr);
            if (name != nullptr) host->note_created_script(std::string(name));
        }
    }
    // The nineteen entity-returning rows of the table end in one recovered tail
    // (docs/LUA_BINDING_ENTITY.md): they push thisTable[key] for the entity they
    // resolved, or, at 0089903C, nil when the lookup produced nothing. This
    // process resolves no entity, so every one of them takes the nil arm, which
    // is a recovered result rather than a substitute: a binding that returns one
    // value is different from one that returns none, and the shipped scripts
    // assign from these.
    const bsp::MissionLuaBinding& binding = dispatch_row;
    if (avoidance_setting) {
        // 008D0849 uses bare 00B66250, which is lua_toboolean with no type
        // gate. Native argument zero is this C callback's stack slot one;
        // absent/nil/false are false, numeric zero and strings are true.
        // Only the proven value store is projected; no private Lua owner,
        // diagnostic string or native SEH construction is claimed here.
        if (!host->error_replay())
            host->set_avoid_all_ship_collision_008d0852(lua_toboolean(state, 1) != 0);
        return 0;
    }
    if (handled && !host->error_replay()) {
        return orders->dispatch(state, binding.name, argc);
    }
    if (bsp::mission_binding_returns_entity(binding.name)) {
        // 0089903C is the arm the native takes when the lookup produced
        // nothing. When it produced an entity the same tail pushes that
        // entity's thisTable slot instead, and milestone 2l fills those slots
        // for the created scene instances, so `FindEntity` can answer for real.
        if (host->push_resolved_entity(state, binding.name, argc)) return 1;
        if (!host->error_replay()) host->note_entity_return();
        lua_pushnil(state);
        return 1;
    }
    return 0;
}

// 00b69e00, the callback 00b6a303 installs as the DoFile global. It runs the
// named script on the same state; the argument is a virtual-file-system path
// with no directory of its own.
int dofile_trampoline(lua_State* state) {
    GameMissionLuaHost* host = host_from_upvalue(state);
    const char* path = lua_tolstring(state, 1, nullptr);
    if (host == nullptr) return 0;
    return host->run_dofile(path != nullptr ? std::string(path) : std::string());
}

// 006b8720 was not read by the packet that recovered the machine, so this is
// the executable's own panic handler rather than a reconstruction. Reaching it
// means an unprotected error escaped, which every path here protects against.
int panic_trampoline(lua_State* state) {
    static_cast<void>(state);
    return 0;
}

}  // namespace

GameMissionLuaHost::GameMissionLuaHost(GameHostLog& log, GameVfsHost& vfs)
    : log_(log), vfs_(vfs) {
    // The content-suffix list at manager +48h/+4Ch is empty in this process, as
    // milestone 2b recorded, so every override query answers with the base file
    // alone. The resource adapter is the same one the locale tables read
    // through; it is what supplies 00886280's folder enumeration.
    if (vfs_.manager() != nullptr) {
        resources_ = std::make_unique<bsp::VfsLocaleRuntime>(vfs_.manager()->context(),
            vfs_.search_registrations(), content_suffixes_, []() -> std::uint32_t { return 0; });
    }
}

GameMissionLuaHost::~GameMissionLuaHost() {
    if (state_ != nullptr) {
        lua_close(state_);
        state_ = nullptr;
    }
}

GameVehicleClassRow GameMissionLuaHost::read_vehicle_class_row(int index) {
    // Milestone 2i. The table is already in this state: 00886900's autoload
    // folder ran Scripts/datatables/autoload/vehicleclasses.lua as one of its
    // scripts. Nothing here parses a file; the read is a plain table lookup
    // against the interpreter the recovered bring-up built.
    GameVehicleClassRow row;
    row.index = index;
    if (state_ == nullptr || index < 0) return row;
    const int top = ::lua_gettop(state_);
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_pushinteger(state_, index);
        ::lua_gettable(state_, -2);
        if (::lua_type(state_, -1) == LUA_TTABLE) {
            row.found = true;
            const auto number = [&](const char* key) -> float {
                ::lua_getfield(state_, -1, key);
                const float value = ::lua_type(state_, -1) == LUA_TNUMBER
                    ? static_cast<float>(::lua_tonumber(state_, -1)) : 0.0f;
                ::lua_settop(state_, ::lua_gettop(state_) - 1);
                return value;
            };
            const auto text = [&](const char* key) -> std::string {
                ::lua_getfield(state_, -1, key);
                const char* value = ::lua_type(state_, -1) == LUA_TSTRING
                    ? ::lua_tolstring(state_, -1, nullptr) : nullptr;
                std::string out = value != nullptr ? value : "";
                ::lua_settop(state_, ::lua_gettop(state_) - 1);
                return out;
            };
            row.name = text("Name");
            row.type = text("Type");
            row.max_speed = number("MaxSpeed");
            row.max_accel = number("MaxAccel");
            row.retardation = number("Retardation");
            row.max_rot_angle = number("MaxRotAngle");
            row.max_rot_angle_change_ratio = number("MaxRotAngleChangeRatio");
            row.length = number("Length");
            // 00960363 uses bare GetNumber, including numeric strings and
            // the native float32 spill. Other row readers keep their scope.
            ::lua_getfield(state_, -1, "Width");
            row.width = lua_number_float32_00b66270(::lua_tonumber(state_, -1));
            ::lua_settop(state_, ::lua_gettop(state_) - 1);
            row.height = number("Height");
            row.mass = number("Mass");
        }
    }
    ::lua_settop(state_, top);
    return row;
}

// ---------------------------------------------------------------------------
// Milestone 2j: the rudder curve's own producer
// ---------------------------------------------------------------------------

namespace {

// 00d0b67c, the literal the loader formats into its path buffer at 0083b6c3.
// The VFS takes forward slashes, which is the same spelling every other script
// path in this process uses.
constexpr const char* kShipGlobalsScriptPath = "Scripts/datatables/ShipGlobals.lua";
constexpr const char* kShipGlobalsGlobal = "ShipGlobals";   // 00d0b670
// The table 0083ce56 runs against. docs/UNIT_RUDDER_CURVE.md: the reads that
// bracket the fragment (0083cdd7, 0083ce19) are the AutoThrust fields, so the
// wrapper the fragment holds at [ESP+0BCh] is ShipGlobals["Navigator"].
constexpr const char* kNavigatorKey = "Navigator";
// Milestone 2p: the sub-table 0083cba8..0083ce3c reads the eleven AutoThrust
// keys from, of which 009ec7c0 consumes seven.
constexpr const char* kAutoThrustKey = "AutoThrust";
// Milestone 2k, the two keys 0087d7b0 reads into global config +6Ch and +70h.
constexpr const char* kGlobalConfigScriptPath = "scripts/datatables/globals.lua";
constexpr const char* kGlobalsGlobal = "Globals";
constexpr const char* kMinimapKey = "Minimap";
constexpr const char* kMinimapRangeKey = "MinimapRange";
constexpr const char* kMinimapVisibilityKey = "VisibilityRange";

// bsp::UnitRudderCurveLoaderHost over the live interpreter. A handle is a Lua
// stack index; the four native helpers become the four stack operations they
// are. 00b67700's release is the wrapper's destructor, which the executable
// answers by leaving the value on the stack until the whole block is popped:
// the fragment never reads a released temporary.
class LuaRudderCurveLoader final : public bsp::UnitRudderCurveLoaderHost {
public:
    LuaRudderCurveLoader(lua_State* state, int navigator_index, GameHostLog& log)
        : state_(state), current_(navigator_index), log_(log) {}

    void release_temporary_00b67700(int handle) override {
        static_cast<void>(handle);
        ++releases_;
        log_.implemented("GameSettings::release_lua_temporary", "00b67700");
    }
    int get_by_name_00b67800(int table, const char* key) override {
        log_.implemented("GameSettings::lua_get_by_name", "00b67800");
        if (state_ == nullptr) return 0;
        if (lua_type(state_, table) != LUA_TTABLE) {
            lua_pushnil(state_);
            return ::lua_gettop(state_);
        }
        ::lua_getfield(state_, table, key);
        return ::lua_gettop(state_);
    }
    void assign_current_table_00b67690(int handle) override {
        // 0083ce84: the returned wrapper is assigned into the fragment's
        // current-table slot, which is what makes the three key lookups run
        // against TurnMultipliers instead of its parent.
        current_ = handle;
        log_.implemented("GameSettings::lua_assign_current_table", "00b67690");
    }
    int get_by_index_00b67720(int table, int index) override {
        log_.implemented("GameSettings::lua_get_by_index", "00b67720");
        if (state_ == nullptr) return 0;
        if (lua_type(state_, table) != LUA_TTABLE) {
            lua_pushnil(state_);
            return ::lua_gettop(state_);
        }
        lua_pushinteger(state_, index);
        ::lua_gettable(state_, table);
        return ::lua_gettop(state_);
    }
    float get_number_00b66270(int handle) override {
        log_.implemented("GameSettings::lua_get_number", "00b66270");
        if (state_ == nullptr) return 0.0f;
        if (lua_type(state_, handle) != LUA_TNUMBER) {
            ++misses_;
            return 0.0f;
        }
        return static_cast<float>(::lua_tonumber(state_, handle));
    }
    int current_table() override { return current_; }

    std::size_t misses() const noexcept { return misses_; }
    std::size_t releases() const noexcept { return releases_; }

private:
    lua_State* state_;
    int current_;
    GameHostLog& log_;
    std::size_t misses_{0};
    std::size_t releases_{0};
};

}  // namespace

bool GameMissionLuaHost::load_ship_globals_0083b6e6() {
    if (state_ == nullptr) {
        log_.unimplemented("GameSettings::run_ship_globals_script", "00b69d40");
        return false;
    }
    // The native runner is the Lua state owner's, with its own override list;
    // this process has one state and one recovered file runner, so the call
    // itself stays a record and the file is run through 00885110.
    log_.unimplemented("GameSettings::run_ship_globals_script", "00b69d40");
    set_phase("ship globals");
    const bsp::LuaChunkResult result = bsp::run_script_file(*this, kShipGlobalsScriptPath);
    const bool ok = result.status == bsp::LuaChunkStatus::Ok;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kShipGlobalsGlobal);
    const bool table = lua_type(state_, -1) == LUA_TTABLE;
    ::lua_settop(state_, top);
    log_.implemented("GameSettings::get_ship_globals_table", "00b67800");
    if (table) {
        std::array<float, 5> tuning;
        std::string error;
        if (!read_ship_avoidance_tuning_lua(*state_, tuning, error)) {
            log_.notef("ship avoidance settings load failed: %s", error.c_str());
            return false;
        }
        avoidance_tuning_ = tuning;
        avoidance_tuning_loaded_ = true;
        log_.implemented("GameSettings::load_avoidance_tuning_projection", "0083b5e0");
        log_.notef("stored ship avoidance tuning 194=%.9g 1d4=%.9g 1d8=%.9g 214=%.9g 218=%.9g",
            tuning[0], tuning[1], tuning[2], tuning[3], tuning[4]);
        // Literal store in the native settings load, not a Lua key/default
        // inferred from the mission. The explicit process reload replays it.
        avoid_all_ship_collision_ = kAvoidAllShipCollisionLoaderDefault;
        avoid_all_ship_collision_loaded_ = true;
        log_.implemented("GameSettings::load_avoid_all_ship_collision", "0083bcd5");
    }
    log_.notef("gameplay settings: %s run through 00885110 (the owner's own runner 00b69d40 "
        "at 0083b6e6 is a record), chunk ok=%d, `%s` is %s", kShipGlobalsScriptPath,
        ok ? 1 : 0, kShipGlobalsGlobal, table ? "a table" : "absent");
    return table;
}

bool GameMissionLuaHost::read_avoid_all_ship_collision(bool& value) const noexcept {
    if (!avoid_all_ship_collision_loaded_) return false;
    value = avoid_all_ship_collision_;
    return true;
}

void GameMissionLuaHost::set_avoid_all_ship_collision_008d0852(bool value) {
    avoid_all_ship_collision_ = value;
    avoid_all_ship_collision_loaded_ = true;
    log_.implemented("GameSettings::set_avoid_all_ship_collision", "008d0852");
}

bool GameMissionLuaHost::read_avoidance_tuning(std::array<float, 5>& values) const noexcept {
    if (!avoidance_tuning_loaded_) return false;
    values = avoidance_tuning_;
    return true;
}

bool GameMissionLuaHost::read_minimap_globals_0087d7b0(float& minimap_range,
    float& visibility_range) {
    if (state_ == nullptr) {
        log_.unimplemented("HudMinimap::global_config_load", "0087d7b0");
        return false;
    }
    // 0087d7b0 is the loader 004ddb90 runs over the global config object; it is
    // not reconstructed, so the routine stays a record and only its two Minimap
    // reads are performed, on this process's one Lua state.
    log_.unimplemented("HudMinimap::global_config_load", "0087d7b0");
    set_phase("global config");
    const bsp::LuaChunkResult result = bsp::run_script_file(*this, kGlobalConfigScriptPath);
    const bool ok = result.status == bsp::LuaChunkStatus::Ok;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kGlobalsGlobal);
    bool read = false;
    float range = 0.0f;
    float visibility = 0.0f;
    if (lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_getfield(state_, -1, kMinimapKey);
        if (lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, kMinimapRangeKey);
            ::lua_getfield(state_, -2, kMinimapVisibilityKey);
            if (lua_type(state_, -2) == LUA_TNUMBER && lua_type(state_, -1) == LUA_TNUMBER) {
                range = static_cast<float>(::lua_tonumber(state_, -2));
                visibility = static_cast<float>(::lua_tonumber(state_, -1));
                read = true;
            }
        }
    }
    ::lua_settop(state_, top);
    log_.notef("minimap range: %s run through 00885110, chunk ok=%d, "
        "Globals[\"Minimap\"] %s (MinimapRange=%.1f VisibilityRange=%.1f); 0087d7b0, the "
        "loader that writes them into the global config object at +6Ch and +70h, is a record",
        kGlobalConfigScriptPath, ok ? 1 : 0, read ? "read" : "absent",
        static_cast<double>(range), static_cast<double>(visibility));
    if (!read) return false;
    minimap_range = range;
    visibility_range = visibility;
    return true;
}

bool GameMissionLuaHost::read_auto_thrust_0083cc2c(bsp::ShipAiAutoThrustSettings& out) {
    // Milestone 2p. 0083CBA8..0083CE3C of 0083B5E0, the eleven AutoThrust keys
    // it writes into 00424C40()+6C4h..+6ECh. This reads the seven
    // 009EC7C0 BSP_UnitBot_ComputeThrottleCeiling consumes
    // (docs/GAMEPLAY_SETTINGS.md rows +6CCh, +6D0h, +6D4h, +6E0h, +6E4h, +6E8h
    // and +6ECh) off the already-loaded `ShipGlobals` table. The loader
    // fragment itself is not projected, so 0083CC2C stays a record and only
    // its reads are performed; the values come from the installation's own
    // Scripts/datatables/ShipGlobals.lua, not from this file.
    if (state_ == nullptr) return false;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kShipGlobalsGlobal);
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    ::lua_getfield(state_, -1, kNavigatorKey);
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    ::lua_getfield(state_, -1, kAutoThrustKey);
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    const int table = ::lua_gettop(state_);
    struct KeyField {
        const char* key;
        float bsp::ShipAiAutoThrustSettings::* field;
    };
    static const KeyField kKeys[] = {
        {"HdgDiffValueMin_Slow", &bsp::ShipAiAutoThrustSettings::hdg_diff_value_min_slow},
        {"HdgDiffValueMax_Slow", &bsp::ShipAiAutoThrustSettings::hdg_diff_value_max_slow},
        {"ThrustMin_Slow",       &bsp::ShipAiAutoThrustSettings::thrust_min_slow},
        {"HdgDiffValueMin_Fast", &bsp::ShipAiAutoThrustSettings::hdg_diff_value_min_fast},
        {"HdgDiffValueMax_Fast", &bsp::ShipAiAutoThrustSettings::hdg_diff_value_max_fast},
        {"ThrustMin_Fast",       &bsp::ShipAiAutoThrustSettings::thrust_min_fast},
        {"HdgDiffDangerMul",     &bsp::ShipAiAutoThrustSettings::hdg_diff_danger_mul},
    };
    bsp::ShipAiAutoThrustSettings settings{};
    bool complete = true;
    for (const KeyField& entry : kKeys) {
        ::lua_getfield(state_, table, entry.key);
        if (lua_type(state_, -1) != LUA_TNUMBER) {
            complete = false;
        } else {
            settings.*(entry.field) = static_cast<float>(::lua_tonumber(state_, -1));
        }
        ::lua_pop(state_, 1);
    }
    ::lua_settop(state_, top);
    if (!complete) return false;
    out = settings;
    return true;
}

bool GameMissionLuaHost::read_path_turn_ramp(ShipAiPathSearchTurnRamp& out,
    std::string& error) {
    if (state_ == nullptr) {
        error = "path turn ramp requires the live mission Lua state";
        return false;
    }
    return read_ship_ai_path_turn_ramp_lua(*state_, out, error);
}

namespace {
struct ShipDepthReadContext {
    int type_id;
    std::int32_t session_mode;
    const bool* crt_sse2_conversion;
    bool read_array;
    GameShipNavigationInput result;
};
static_assert(std::is_trivially_destructible_v<ShipDepthReadContext>);

// Key-to-record relation established by 00841B7B..008425A1. Offsets reuse the
// existing producer enum; which scalar a leaf consumes comes from the existing
// kShipLeafTuningSources table, not a second mapping of class depth values.
const char* depth_key(std::uint32_t offset) noexcept {
    switch (static_cast<AvoidZoneDepthSlot>(offset)) {
    case AvoidZoneDepthSlot::kMotherShip: return "MotherShip";
    case AvoidZoneDepthSlot::kDestroyer: return "Destroyer";
    case AvoidZoneDepthSlot::kTBoat: return "TBoat";
    case AvoidZoneDepthSlot::kSmallLandingShip: return "SmallLandingShip";
    case AvoidZoneDepthSlot::kLargeLandingShip: return "LargeLandingShip";
    case AvoidZoneDepthSlot::kBattleShip: return "BattleShip";
    case AvoidZoneDepthSlot::kCargoShip: return "CargoShip";
    case AvoidZoneDepthSlot::kLightCruiser: return "LightCruiser";
    case AvoidZoneDepthSlot::kHeavyCruiser: return "HeavyCruiser";
    case AvoidZoneDepthSlot::kMiniSub: return "MiniSub";
    case AvoidZoneDepthSlot::kSubmarine: return "Submarine";
    }
    return nullptr;
}

int read_ship_depth_protected(lua_State* state) {
    auto& context = *static_cast<ShipDepthReadContext*>(
        lua_touserdata(state, lua_upvalueindex(1)));
    // This C frame has no arguments. Every automatic object is trivial, so
    // a Lua error crosses no C++ destructor. At most six tracked slots live;
    // each contains one reference, inside the actual owner's 50/5 capacities.
    NativeLuaStateStorage owner;
    construct_native_lua_state_00b66bd0(&owner);
    owner.state_04 = state;
    NativeLuaObjectStorage globals, classes, row, type;
    native_lua_globals_00b67980(owner, &globals);
    native_lua_get_by_name_00b67800(globals, &classes, "VehicleClass");
    native_lua_get_by_index_00b67720(classes, &row, context.type_id);
    native_lua_get_by_name_00b67800(row, &type, "Type");
    const char* type_name = native_lua_string_00b662b0(type);
    const auto* native_kind = vehicle_class_kind_row(type_name);
    const ShipLeafClassInfo* leaf = nullptr;
    if (native_kind != nullptr) {
        for (std::size_t i = 0; i < ship_leaf_class_count(); ++i) {
            if (std::strcmp(native_kind->lua_type, kShipLeafClasses[i].lua_type) == 0) {
                leaf = &kShipLeafClasses[i];
                break;
            }
        }
    }
    if (leaf == nullptr)
        return luaL_error(state, "VehicleClass[%d].Type '%s' has no supported ship depth producer",
            context.type_id, type_name != nullptr ? type_name : "<unavailable>");
    destroy_native_lua_object_00b67700(type);

    const char* variant = nullptr;
    if (leaf->leaf == ShipLeafClass::Cruiser || leaf->leaf == ShipLeafClass::LandingShip) {
        NativeLuaObjectStorage flag;
        const bool cruiser = leaf->leaf == ShipLeafClass::Cruiser;
        native_lua_get_by_name_00b67800(row, &flag,
            cruiser ? "HeavyCruiser" : "BigLandingShip");
        const bool alternate = native_lua_boolean_or_00b662f0(flag, 0) != 0;
        destroy_native_lua_object_00b67700(flag);
        variant = cruiser
            ? (alternate ? "HeavyCruiser true" : "HeavyCruiser false")
            : (alternate ? "BigLandingShip true" : "BigLandingShip false");
    }
    const ShipLeafTuningSource* source = nullptr;
    for (std::size_t i = 0; i < ship_leaf_tuning_source_count(); ++i) {
        const auto& candidate = kShipLeafTuningSources[i];
        if (candidate.leaf == leaf->leaf
            && (variant == nullptr || std::strcmp(candidate.variant, variant) == 0)) {
            source = &candidate;
            break;
        }
    }
    if (source == nullptr)
        return luaL_error(state, "ship leaf has no established class+570 source");
    const char* key = depth_key(source->scalar_source);
    if (key == nullptr)
        return luaL_error(state, "ship depth source has no established Lua key");
    destroy_native_lua_object_00b67700(row);
    destroy_native_lua_object_00b67700(classes);

    const auto settings_offset = ship_tuning_block_offset(context.session_mode);
    NativeLuaObjectStorage ship_globals, depths, values, first;
    native_lua_get_by_name_00b67800(globals, &ship_globals, "ShipGlobals");
    native_lua_get_by_name_00b67800(ship_globals, &depths,
        settings_offset == kAvoidZoneDepthsSingleOffset
            ? "AvoidZoneDepthsSingle" : "AvoidZoneDepthsMulti");
    native_lua_get_by_name_00b67800(depths, &values, key);
    native_lua_get_by_index_00b67720(values, &first, 1);
    // The producer uses bare GetInteger, not GetIntegerOrDefault or IsNumber.
    // Preserve tonumber -> float32 spill -> selected CRT conversion, including
    // final nil/nonnumeric values becoming the native zero conversion result.
    const auto scalar = native_lua_integer_00b66290(first, *context.crt_sse2_conversion);
    destroy_native_lua_object_00b67700(first);
    context.result.tuning.scalar = static_cast<std::uint32_t>(scalar);
    context.result.tuning.array_source = source->array_source;
    context.result.tuning.scalar_source = source->scalar_source;
    context.result.settings_block_offset = settings_offset;
    context.result.class_key = key;
    if (context.read_array) {
        const bool submarine = leaf->leaf == ShipLeafClass::Submarine;
        const std::size_t reads = submarine ? ShipLeafTuningOffsets::kTuningArrayCount : 1;
        for (std::size_t i = 0; i < reads; ++i) {
            const std::uint32_t offset = submarine
                ? kSubmarineTuningArraySources[i] : source->array_source;
            const int index = 1 + static_cast<int>((offset - source->scalar_source) / 4);
            NativeLuaObjectStorage element;
            native_lua_get_by_index_00b67720(values, &element, index);
            const auto value = native_lua_integer_00b66290(element, *context.crt_sse2_conversion);
            destroy_native_lua_object_00b67700(element);
            if (submarine) {
                context.result.tuning.array[i] = static_cast<std::uint32_t>(value);
            } else {
                // The native surface leaf copies one settings dword four times.
                // Read element2 once so an authored __index runs once as well.
                for (auto& slot : context.result.tuning.array)
                    slot = static_cast<std::uint32_t>(value);
            }
        }
    }
    destroy_native_lua_object_00b67700(values);
    destroy_native_lua_object_00b67700(depths);
    destroy_native_lua_object_00b67700(ship_globals);
    destroy_native_lua_object_00b67700(globals);
    return 0;
}
bool read_ship_tuning_lua(lua_State& state, int type_id, std::int32_t session_mode,
    const bool& crt_sse2_conversion, bool read_array,
    GameShipNavigationInput& output, std::string& error) {
    if (type_id < 0) {
        error = "ship depth requires an actual VehicleClass index";
        return false;
    }
    ShipDepthReadContext context{type_id, session_mode, &crt_sse2_conversion, read_array, {}};
    const int top = lua_gettop(&state);
    lua_pushlightuserdata(&state, &context);
    lua_pushcclosure(&state, &read_ship_depth_protected, 1);
    const int status = lua_pcall(&state, 0, 0, 0);
    if (status != 0) {
        const char* message = lua_tostring(&state, -1);
        error = message != nullptr ? message : "ship depth lookup raised a non-string Lua error";
        lua_settop(&state, top);
        return false;
    }
    lua_settop(&state, top);
    output = context.result;
    error.clear();
    return true;
}

struct ShipLayerTimingReadContext { std::array<float, 6> result; };
static_assert(std::is_trivially_destructible_v<ShipLayerTimingReadContext>);

int read_ship_layer_timing_protected(lua_State* state) {
    auto& context = *static_cast<ShipLayerTimingReadContext*>(
        lua_touserdata(state, lua_upvalueindex(1)));
    // Only trivial automatic objects cross a possible Lua longjmp. At most
    // five tracked slots are live, each containing one reference.
    NativeLuaStateStorage owner;
    construct_native_lua_state_00b66bd0(&owner);
    owner.state_04 = state;
    NativeLuaObjectStorage globals, ship_globals, land;
    native_lua_globals_00b67980(owner, &globals);
    native_lua_get_by_name_00b67800(globals, &ship_globals, "ShipGlobals");
    native_lua_get_by_name_00b67800(ship_globals, &land, "LandAvoidance");
    std::size_t count = 0;
    const auto* records = ship_ai_settings_keys(count);
    for (std::size_t i = 0; i < context.result.size(); ++i) {
        const std::uint32_t offset = 0x1f4u + 4u * static_cast<std::uint32_t>(i);
        const ShipAiSettingsKeyRecord* record = nullptr;
        for (std::size_t j = 0; j < count; ++j) {
            if (records[j].settings_offset == offset) {
                record = &records[j];
                break;
            }
        }
        constexpr char prefix[] = "LandAvoidance.";
        if (record == nullptr || record->getter != ShipAiSettingsGetter::kNumber
            || std::strncmp(record->lua_path, prefix, sizeof(prefix) - 1) != 0)
            return luaL_error(state, "ship layer timing has no established source at %d", offset);
        NativeLuaObjectStorage pair, value;
        native_lua_get_by_name_00b67800(land, &pair, record->lua_path + sizeof(prefix) - 1);
        native_lua_get_by_index_00b67720(pair, &value, record->array_index);
        context.result[i] = native_lua_number_00b66270(value);
        destroy_native_lua_object_00b67700(value);
        destroy_native_lua_object_00b67700(pair);
    }
    destroy_native_lua_object_00b67700(land);
    destroy_native_lua_object_00b67700(ship_globals);
    destroy_native_lua_object_00b67700(globals);
    return 0;
}
} // namespace

bool read_ship_depth_input_lua(lua_State& state, int type_id, std::int32_t session_mode,
    const bool& crt_sse2_conversion, GameShipDepthInput& output, std::string& error) {
    GameShipNavigationInput result{};
    if (!read_ship_tuning_lua(state, type_id, session_mode, crt_sse2_conversion,
        false, result, error)) return false;
    output = {result.tuning.scalar, result.settings_block_offset,
        result.tuning.scalar_source, result.class_key};
    return true;
}

bool read_ship_navigation_input_lua(lua_State& state, int type_id, std::int32_t session_mode,
    const bool& crt_sse2_conversion, GameShipNavigationInput& output, std::string& error) {
    return read_ship_tuning_lua(state, type_id, session_mode, crt_sse2_conversion,
        true, output, error);
}

bool read_ship_layer_timing_input_lua(lua_State& state, std::array<float, 6>& output,
    std::string& error) {
    ShipLayerTimingReadContext context{};
    const int top = lua_gettop(&state);
    lua_pushlightuserdata(&state, &context);
    lua_pushcclosure(&state, &read_ship_layer_timing_protected, 1);
    const int status = lua_pcall(&state, 0, 0, 0);
    if (status != 0) {
        const char* message = lua_tostring(&state, -1);
        error = message != nullptr ? message : "ship layer timing lookup raised a non-string Lua error";
        lua_settop(&state, top);
        return false;
    }
    lua_settop(&state, top);
    output = context.result;
    error.clear();
    return true;
}

bool GameMissionLuaHost::read_ship_depth_input(int type_id, std::int32_t session_mode,
    GameShipDepthInput& output, std::string& error) {
    if (state_ == nullptr) {
        error = "ship depth requires the live mission Lua state";
        return false;
    }
    // Same represented CRT capability selection used by the existing decal
    // loader. The pure reader accepts the borrowed conversion mode explicitly.
    const bool sse2_conversion =
        IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE;
    return read_ship_depth_input_lua(*state_, type_id, session_mode,
        sse2_conversion, output, error);
}

bool GameMissionLuaHost::read_ship_navigation_input(int type_id, std::int32_t session_mode,
    GameShipNavigationInput& output, std::string& error) {
    if (state_ == nullptr) {
        error = "ship navigation tuning requires the live mission Lua state";
        return false;
    }
    const bool sse2_conversion =
        IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE;
    return read_ship_navigation_input_lua(*state_, type_id, session_mode,
        sse2_conversion, output, error);
}

bool GameMissionLuaHost::read_ship_layer_timing_input(std::array<float, 6>& output,
    std::string& error) {
    if (state_ == nullptr) {
        error = "ship layer timing requires the live mission Lua state";
        return false;
    }
    return read_ship_layer_timing_input_lua(*state_, output, error);
}

bool GameMissionLuaHost::read_turn_multipliers_0083ce56(bsp::UnitRudderCurveSettings& out) {
    if (state_ == nullptr) return false;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kShipGlobalsGlobal);
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    ::lua_getfield(state_, -1, kNavigatorKey);
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    const int navigator = ::lua_gettop(state_);
    LuaRudderCurveLoader loader(state_, navigator, log_);
    const bsp::UnitRudderCurveSettings settings
        = bsp::unit_rudder_curve_load_0083ce56(loader);
    const bool complete = loader.misses() == 0;
    ::lua_settop(state_, top);
    if (!complete) return false;
    out = settings;
    return true;
}

bool GameMissionLuaHost::started() const noexcept { return state_ != nullptr; }

const GameMissionLuaSummary& GameMissionLuaHost::summary() const noexcept { return summary_; }

void GameMissionLuaHost::set_phase(std::string phase) { phase_ = std::move(phase); }

void GameMissionLuaHost::note_entity_return() { ++summary_.entity_returns; }

void GameMissionLuaHost::create_self_table_004e0305() {
    if (state_ == nullptr) return;
    // 004e0305 runs 00b67350 on "recon" (set-nil) on the same pass that creates
    // the self table. The table itself is empty here: 00928a00 fills one slot
    // per entity, and this process creates no entity.
    lua_createtable(state_, 0, 0);
    lua_setfield(state_, LUA_GLOBALSINDEX, bsp::kMissionSelfTableGlobal);
    lua_pushnil(state_);
    lua_setfield(state_, LUA_GLOBALSINDEX, bsp::kMissionReconGlobal);
    summary_.self_table_created = true;
    log_.implemented("MissionLua::create_self_table", "004e0305");
    log_.notef("self table \"%s\" created empty and \"%s\" cleared; 00928a00 adds one slot "
        "per entity and this process creates none", bsp::kMissionSelfTableGlobal,
        bsp::kMissionReconGlobal);
}

namespace {

// bsp::ReconLuaInstanceView is a reference to the instance's own mutable state
// slot (native game+1A08 -> host+04 -> instance+04). This process has one
// machine, so the slot is this adapter's own member and the reconstruction sees
// the same identity rule it was written against.
class ReconShellHost final : public bsp::ReconValuesHost {
public:
    explicit ReconShellHost(lua_State* state) : state_(state) {}

    bsp::ReconLuaInstanceView current_mission_lua_instance_1a08_04() override {
        return bsp::ReconLuaInstanceView{state_};
    }

private:
    lua_State* state_;
};

}  // namespace

void GameMissionLuaHost::install_recon_tables_00803a40() {
    if (state_ == nullptr) return;
    ReconShellHost host(state_);
    bsp::ReconValuesContext context{host, bsp::recon_category_names_00e0b590.data()};
    bsp::install_recon_values_00803a40(context);
    log_.unimplemented("Recon::publish_slot_table", "00806b10");
    log_.notef("recon shell built by 00803a40: three party indices, each with enemy, "
        "neutral, unknown and own, each of those with the nineteen category maps of "
        "00E0B590. Every map is empty: 00806b10 and 00805d90 fill them from the recon "
        "slot lists and this process builds none, so a script that asks for detected "
        "units gets an empty list rather than an error");
}

void GameMissionLuaHost::note_error(const std::string& message) {
    if (summary_.first_error.empty() && !message.empty()) {
        summary_.first_error = message;
        summary_.first_error_phase = phase_;
    }
}

void GameMissionLuaHost::attach_script_orders(GameScriptOrdersHost* orders) noexcept {
    script_orders_ = orders;
}

GameScriptOrdersHost* GameMissionLuaHost::script_orders() const noexcept {
    return script_orders_;
}

void GameMissionLuaHost::note_native_call(std::size_t row, int argument_count,
    bool handled) {
    const bsp::MissionLuaBinding* rows = bsp::mission_lua_bindings();
    if (row >= bsp::mission_lua_binding_count()) return;
    const bsp::MissionLuaBinding& binding = rows[row];
    ++summary_.native_calls;
    auto found = native_index_.find(binding.name);
    if (handled) {
        // The row runs its own reconstructed body; GameScriptOrdersHost records
        // the method as concrete at the row's address. Only the counters and the
        // per-binding line belong here.
        if (found == native_index_.end()) {
            native_index_.emplace(binding.name, summary_.natives.size());
            GameMissionNativeCall record;
            record.name.assign(binding.name);
            record.address = binding.address;
            record.calls = 1;
            record.last_argument_count = argument_count;
            summary_.natives.push_back(record);
            log_.notef("  binding %-28s argc=%d phase=%s (reconstructed body)",
                binding.name, argument_count,
                phase_.empty() ? "(none)" : phase_.c_str());
            return;
        }
        GameMissionNativeCall& reached = summary_.natives[found->second];
        ++reached.calls;
        reached.last_argument_count = argument_count;
        return;
    }
    if (found == native_index_.end()) {
        native_index_.emplace(binding.name, summary_.natives.size());
        GameMissionNativeCall record;
        record.name.assign(binding.name);
        record.address = binding.address;
        record.calls = 1;
        record.last_argument_count = argument_count;
        summary_.natives.push_back(record);
        // A binding the scripts reached is one native body this process does
        // not have. The record carries the row's own address, so the report
        // names the routine rather than the table.
        char address[16];
        std::snprintf(address, sizeof(address), "%08lx",
            static_cast<unsigned long>(binding.address));
        char method[96];
        std::snprintf(method, sizeof(method), "MissionLuaNative::%s", binding.name);
        log_.unimplemented(method, address);
        log_.notef("  native %-28s argc=%d phase=%s", binding.name, argument_count,
            phase_.empty() ? "(none)" : phase_.c_str());
        return;
    }
    GameMissionNativeCall& record = summary_.natives[found->second];
    ++record.calls;
    record.last_argument_count = argument_count;
    char address[16];
    std::snprintf(address, sizeof(address), "%08lx", static_cast<unsigned long>(binding.address));
    char method[96];
    std::snprintf(method, sizeof(method), "MissionLuaNative::%s", binding.name);
    log_.unimplemented(method, address);
}

int GameMissionLuaHost::read_vehicle_class_integer(int index, const char* key,
    const char* nested_key, int fallback) {
    if (state_ == nullptr || index < 0 || key == nullptr) return fallback;
    const int top = ::lua_gettop(state_);
    int value = fallback;
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_pushinteger(state_, index);
        ::lua_gettable(state_, -2);
        if (::lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, key);
            if (nested_key != nullptr) {
                if (::lua_type(state_, -1) == LUA_TTABLE) {
                    ::lua_getfield(state_, -1, nested_key);
                } else {
                    ::lua_pushnil(state_);
                }
            }
            if (::lua_type(state_, -1) == LUA_TNUMBER) {
                value = static_cast<int>(::lua_tonumber(state_, -1));
            }
        }
    }
    ::lua_settop(state_, top);
    return value;
}

std::vector<bsp::LuaGlobalEntry> GameMissionLuaHost::lua_global_entries() {
    std::vector<bsp::LuaGlobalEntry> entries;
    if (state_ == nullptr) return entries;
    const int top = ::lua_gettop(state_);
    lua_pushnil(state_);
    while (::lua_next(state_, LUA_GLOBALSINDEX) != 0) {
        // 00b662b0 GetString on the key; a non-string key has no name to insert.
        if (lua_type(state_, -2) == LUA_TSTRING) {
            bsp::LuaGlobalEntry entry;
            const char* name = lua_tolstring(state_, -2, nullptr);
            if (name != nullptr) entry.name.assign(name);
            entry.is_function = lua_type(state_, -1) == LUA_TFUNCTION;
            entries.push_back(entry);
        }
        ::lua_settop(state_, ::lua_gettop(state_) - 1);
    }
    ::lua_settop(state_, top);
    return entries;
}

int GameMissionLuaHost::run_dofile(const std::string& path) {
    ++summary_.dofile_calls;
    if (std::find(summary_.dofile_paths.begin(), summary_.dofile_paths.end(), path)
        == summary_.dofile_paths.end()) {
        summary_.dofile_paths.push_back(path);
    }
    log_.implemented("MissionLua::dofile_callback", "00b69e00");
    if (path.empty()) return 0;
    const std::string saved = phase_;
    phase_ = "DoFile " + path;
    const bsp::LuaChunkResult result = bsp::run_script_file(*this, path);
    if (result.status != bsp::LuaChunkStatus::Ok) {
        log_.notef("  DoFile %s did not run cleanly", path.c_str());
    }
    phase_ = saved;
    return 0;
}

// ---------------------------------------------------------------------------
// Bring-up
// ---------------------------------------------------------------------------

bool GameMissionLuaHost::start_machine_00884be0() {
    if (state_ != nullptr) return true;
    set_phase("machine");
    // 004dd627 is the one call site: BSP_Game_OnInitOnce builds the machine
    // once per process, not once per mission.
    log_.implemented("MissionLuaHost::initialize", "00884be0");
    summary_.bindings_registered = bsp::initialise_mission_lua_host(*this);
    if (state_ == nullptr) return false;
    summary_.machine_started = true;

    // 00b6a303, the LuaStateOwner layer over the same state. DoFile is not one
    // of the 560 rows, and the first executable line of the stock mission
    // script calls it (docs/MISSION_LUA_MACHINE.md, gap 1).
    lua_pushlightuserdata(state_, this);
    lua_pushcclosure(state_, dofile_trampoline, 1);
    lua_setfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaStateOwnerGlobal);
    summary_.dofile_installed = true;
    log_.implemented("MissionLua::install_state_owner_global", "00b6a303");

    // 00884c94: the fundamentals bytes come from the singleton at 00884770, so
    // the chunk name is a label. This process has no such singleton, so the
    // bytes are read from the mounted Scripts/fundamentals.lua, which is what
    // 00884770 caches; the chunk name is kept as the native's label.
    std::vector<std::uint8_t> bytes;
    if (resources_ != nullptr && resources_->read_file("Scripts/fundamentals.lua",
            kScriptReadMode, bytes) && !bytes.empty()) {
        set_phase("fundamentals");
        const bsp::LuaChunkResult result = bsp::run_lua_chunk(*this,
            reinterpret_cast<const char*>(bytes.data()), static_cast<int>(bytes.size()),
            bsp::kMissionLuaFundamentalsChunk, true, false, 2);
        summary_.fundamentals_ran = result.status == bsp::LuaChunkStatus::Ok;
        log_.implemented("MissionLua::fundamentals_chunk", "00884c94");
    } else {
        log_.unimplemented("MissionLua::fundamentals_chunk", "00884770");
    }
    log_.notef("mission lua machine: %zu libraries, %zu bindings, platform chunk=%d "
        "fundamentals=%d DoFile=%d", summary_.libraries_opened, summary_.bindings_registered,
        summary_.platform_chunk_ran ? 1 : 0, summary_.fundamentals_ran ? 1 : 0,
        summary_.dofile_installed ? 1 : 0);
    return true;
}

std::size_t GameMissionLuaHost::run_global_script_folders_00886900() {
    if (state_ == nullptr || resources_ == nullptr) {
        log_.unimplemented("MissionLua::global_script_folders", "00886900");
        return 0;
    }
    set_phase("global folders");
    bsp::NativeStringStorage& strings = bsp::crt_string_storage();
    bsp::LanguageCatalogSource* source = resources_.get();
    bsp::GlobalScriptFolderContext context{strings, source};
    const std::size_t ran_before = summary_.global_folder_scripts;
    bsp::load_global_script_folders_00886900(*this, context);
    log_.implemented("MissionLua::global_script_folders", "00886900");
    log_.notef("global script folders: %zu scripts ran, %zu did not load cleanly",
        summary_.global_folder_scripts - ran_before, summary_.global_folder_errors);
    return summary_.global_folder_scripts - ran_before;
}

namespace {

// bsp::LobbySettingsSyncHost and bsp::LobbySettingsOptionSource over the live
// interpreter, for packet cc2_lobby_settings' reconstruction of 005E2F00.
// Everything the routine writes into Lua is done here; everything it reads from
// a registry or a record this process does not own is a host record.
class LobbySettingsBinding final : public bsp::LobbySettingsSyncHost {
public:
    LobbySettingsBinding(lua_State* state, GameHostLog& log) : state_(state), log_(log) {}

    void set_global_nil(const char* name) override {
        if (state_ == nullptr) return;
        lua_pushnil(state_);
        lua_setfield(state_, LUA_GLOBALSINDEX, name);
        nil_global_ = true;
        log_.implemented("MissionLua::lobby_settings_set_global_nil", "00b67350");
    }
    void set_global_new_table(const char* name) override {
        if (state_ == nullptr) return;
        lua_createtable(state_, 0, static_cast<int>(bsp::kLobbySettingsFieldCount));
        lua_setfield(state_, LUA_GLOBALSINDEX, name);
        table_created_ = true;
        log_.implemented("MissionLua::lobby_settings_set_global_table", "00b67580");
    }
    void open_global_table(const char* name) override {
        if (state_ == nullptr) return;
        lua_getfield(state_, LUA_GLOBALSINDEX, name);
        open_ = lua_type(state_, -1) == LUA_TTABLE;
        if (!open_) ::lua_settop(state_, ::lua_gettop(state_) - 1);
        log_.implemented("MissionLua::lobby_settings_open_table", "00b67800");
    }
    void table_set_nil(const char* field) override {
        if (!open_) return;
        lua_pushnil(state_);
        lua_setfield(state_, -2, field);
        ++nil_fields_;
    }
    void table_set_number(const char* field, int value) override {
        if (!open_) return;
        lua_pushinteger(state_, value);
        lua_setfield(state_, -2, field);
        ++number_fields_;
    }
    void table_set_string(const char* field, const char* value) override {
        if (!open_) return;
        lua_pushstring(state_, value != nullptr ? value : "");
        lua_setfield(state_, -2, field);
        ++string_fields_;
    }
    void close_table() override {
        if (!open_) return;
        ::lua_settop(state_, ::lua_gettop(state_) - 1);
        open_ = false;
    }
    const char* option_label(int slot) override {
        static_cast<void>(slot);
        // 005E2320 needs the mission record at 00E19594, the player-count bytes
        // at game+2017h/+2018h and the label map, none of which this process
        // owns. It is never reached on the single-player path.
        log_.unimplemented("MissionLua::lobby_settings_option_label", "005e2320");
        return "";
    }
    void publish_mode_flags(const bsp::LobbySettingsModeFlags& flags) override {
        flags_ = flags;
        // The three bytes 00E0C978, 00E17BF2 and 00E08880. The executable holds
        // them as its own state; 0080FC30's gate reads the first of them.
        log_.implemented("MissionLua::lobby_settings_mode_flags", "005e3017");
    }
    void publish_command_points(float value) override {
        command_points_ = value;
        log_.implemented("MissionLua::lobby_settings_command_points", "00e0cfb4");
    }

    bool nil_global() const noexcept { return nil_global_; }
    bool table_created() const noexcept { return table_created_; }
    std::size_t nil_fields() const noexcept { return nil_fields_; }
    std::size_t number_fields() const noexcept { return number_fields_; }
    std::size_t string_fields() const noexcept { return string_fields_; }
    const bsp::LobbySettingsModeFlags& flags() const noexcept { return flags_; }
    float command_points() const noexcept { return command_points_; }

private:
    lua_State* state_;
    GameHostLog& log_;
    bool open_{false};
    bool nil_global_{false};
    bool table_created_{false};
    std::size_t nil_fields_{0};
    std::size_t number_fields_{0};
    std::size_t string_fields_{0};
    bsp::LobbySettingsModeFlags flags_{};
    float command_points_{0.0f};
};

// The MultiLobbySettings registry 008D2F50 loads from
// Scripts/datatables/MultiGlobals.lua. This process does not load it, and the
// native map is a std::map, so a missing key reads as zero rather than failing;
// the record answers with that zero.
class LobbySettingsOptionsBinding final : public bsp::LobbySettingsOptionSource {
public:
    explicit LobbySettingsOptionsBinding(GameHostLog& log) : log_(log) {}
    int option_number(int slot, int option) override {
        static_cast<void>(slot);
        static_cast<void>(option);
        log_.unimplemented("MissionLua::lobby_settings_option_number", "008d2f50");
        return 0;
    }

private:
    GameHostLog& log_;
};

}  // namespace

void GameMissionLuaHost::publish_lobby_settings_005e2f00() {
    if (state_ == nullptr) return;
    // Packet cc2_lobby_settings reconstructed 005E2F00 in full, so milestone 2i's
    // substitute (a zeroed thirteen-field table, on the guess that a script
    // would otherwise index a nil global) is gone. The routine's own answer for
    // this run is the opposite of that guess: 005E2F93..005E2FCC is a
    // single-player early-out that sets the global **nil** at 005E2F59 and
    // publishes no table at all. game+1FE4h is zero here, so that is the path
    // taken, and the executable now takes it rather than inventing a table.
    bsp::LobbySettingsGameState state{};
    state.network_session = false;      // game+1FE4h
    state.game_mode_forced = false;     // game+61Ch
    state.effective_game_mode = 8;      // 004bca50, the single-player campaign
    LobbySettingsBinding sync(state_, log_);
    LobbySettingsOptionsBinding options(log_);
    bsp::lobby_settings_sync_005e2f00(sync, options, state);
    summary_.lobby_settings_published = sync.table_created();
    log_.implemented("MissionLua::sync_lobby_settings", "005e2f00");
    log_.notef("LobbySettings: %s (single player, game+1FE4h = 0), fields nil=%zu number=%zu "
        "string=%zu; mode flags powerups=%d reload_payload=%d map=%d, command points %.1f",
        sync.nil_global() ? "the global is set nil by the 005e2f93 early-out"
                          : "a table was published",
        sync.nil_fields(), sync.number_fields(), sync.string_fields(),
        sync.flags().powerups_enabled ? 1 : 0, sync.flags().reload_payload_on ? 1 : 0,
        sync.flags().map_enabled ? 1 : 0, static_cast<double>(sync.command_points()));
}

bool GameMissionLuaHost::run_mission_script_008860b0(const std::string& script_name) {
    if (state_ == nullptr) return false;
    summary_.mission_script_path = bsp::mission_script_path(script_name);
    set_phase("mission chunk");
    log_.implemented("MissionLua::run_mission_script", "008860b0");
    const std::vector<bsp::LuaChunkResult> results
        = bsp::run_mission_script(*this, script_name);
    summary_.mission_chunks_run = results.size();
    summary_.mission_chunk_ok = !results.empty()
        && results.front().status == bsp::LuaChunkStatus::Ok;
    log_.notef("mission script %s: %zu chunk(s) run, base ok=%d",
        summary_.mission_script_path.c_str(), results.size(),
        summary_.mission_chunk_ok ? 1 : 0);
    return summary_.mission_chunk_ok;
}

bool GameMissionLuaHost::call_entry_point(const std::string& name, bool threadsafe) {
    GameMissionEntryPointRun run;
    run.name = name;
    if (state_ == nullptr) {
        summary_.entry_points.push_back(run);
        return false;
    }
    set_phase(name);
    log_.implemented(threadsafe ? "MissionLua::call_entry_point_threadsafe"
                                : "MissionLua::call_entry_point_forced",
        threadsafe ? "0045f440" : "0045f520");
    run.defined = global_is_defined(name.c_str());
    const bsp::NamedCallOutcome outcome
        = bsp::call_entry_point_if_defined(*this, name, {}, threadsafe);
    run.dispatched = outcome.dispatched;
    run.pcall_status = outcome.pcall_status;
    if (outcome.dispatched && outcome.pcall_status != 0) {
        // The native hands debugtrap to lua_pcall as its error handler, and
        // 008c8390 returns no results, so Lua 5.1 replaces the error object
        // with nil and the caller learns only that the call failed. The
        // executable reruns the same call with errfunc 0 purely to recover the
        // message for the log; that rerun is the executable's, not the game's.
        const int top = ::lua_gettop(state_);
        set_error_replay(true);
        lua_getfield(state_, LUA_GLOBALSINDEX, name.c_str());
        if (!lua_isnil(state_, -1)) {
            if (::lua_pcall(state_, 0, 0, 0) != 0) {
                const char* message = lua_tolstring(state_, -1, nullptr);
                run.error = message != nullptr ? message : "(no message)";
            }
        }
        set_error_replay(false);
        ::lua_settop(state_, top);
        note_error(run.error);
    }
    log_.notef("  entry point %-20s defined=%d dispatched=%d status=%d %s", name.c_str(),
        run.defined ? 1 : 0, run.dispatched ? 1 : 0, run.pcall_status,
        run.error.empty() ? "" : run.error.c_str());
    summary_.entry_points.push_back(run);
    return run.dispatched && run.pcall_status == 0;
}

std::size_t GameMissionLuaHost::attach_scene_entities_00928a00(
    const std::vector<SceneEntity>& entities) {
    if (state_ == nullptr || entities.empty()) return 0;
    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (lua_isnil(state_, -1)) {
        ::lua_settop(state_, ::lua_gettop(state_) - 1);
        return 0;
    }
    std::size_t made = 0;
    std::size_t classes = 0;
    for (const SceneEntity& entity : entities) {
        char key[16];
        std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, entity.id);
        // 00928b53 assigns a fresh table, then 00928bxx seeds `ID`, `Dead` and
        // `Ptr`. The native `Ptr` is lightuserdata(entity), the entity object
        // itself; this process has no such object, so the slot carries the
        // entity's own id as a light pointer and nothing dereferences it.
        lua_createtable(state_, 0, 3);
        // Corrected by packet cc_lua_find_entity: `ID` is the key **text**, not a
        // number. 00928BA5 hands 00B67630 the same NativeString the key was
        // formatted into, and 00B67630's second push is 00A67A10
        // `lua_pushlstring` (docs/MISSION_LUA_SELF_TABLE.md line 93 already said
        // so). It matters because every shipped helper that takes an entity
        // table indexes `thisTable[Obj.ID]`, and a number key and a string key
        // are different keys: with a number here, luaGetDistance3D read
        // `thisTable[<number>]` as nil and answered a distance of zero.
        ::lua_pushstring(state_, key);
        lua_setfield(state_, -2, "ID");
        lua_pushboolean(state_, 0);
        lua_setfield(state_, -2, "Dead");
        lua_pushlightuserdata(state_,
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(entity.id)));
        lua_setfield(state_, -2, "Ptr");
        // `Class`, the field a per-kind setter adds through 00b675d0. It is the
        // installed `VehicleClass` row the autoload folder already loaded, not
        // a table this file builds: the slot is assigned the global's own row.
        if (entity.class_index >= 0) {
            lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
            if (lua_istable(state_, -1)) {
                lua_rawgeti(state_, -1, entity.class_index);
                if (lua_istable(state_, -1)) {
                    lua_setfield(state_, -3, "Class");
                    ++classes;
                } else {
                    ::lua_settop(state_, ::lua_gettop(state_) - 1);
                }
            }
            ::lua_settop(state_, ::lua_gettop(state_) - 1);
        }
        lua_setfield(state_, -2, key);
        // Packet cc_lua_find_entity: the slot is built for every entity that
        // reaches virtual slot 39; only the entities whose world bucket
        // 0088B1B0 walks enter the name index the FindEntity tail uses.
        if (entity.findable) scene_entity_ids_[entity.name] = entity.id;
        ++made;
    }
    ::lua_settop(state_, ::lua_gettop(state_) - 1);
    summary_.self_table_entities = made;
    log_.unimplemented("MissionLua::entity_lua_attach", "00928a00");
    log_.notef("thisTable: %zu per-entity slot(s) built for the created scene instances, "
        "%zu of them with the installed `VehicleClass` row as their `Class` field. 00928a00 "
        "and its caller 0077e830 are records, and so is the per-kind `Class` setter that "
        "goes through 00b675d0; what the executable supplies is the slot with its recovered "
        "`ID`, `Dead` and `Ptr` fields, so the entity tail at 0089903c can take its "
        "resolved arm instead of the nil one", made, classes);
    return made;
}

bool GameMissionLuaHost::push_resolved_entity(lua_State* state, const char* binding_name,
    int argument_count) {
    if (state == nullptr || binding_name == nullptr) return false;
    if (scene_entity_ids_.empty()) return false;
    // Only FindEntity. Its argument is a name and 00925a90 answers the scene
    // database's entity of that name; every other entity-returning row takes
    // its subject from game state this process does not own.
    if (std::strcmp(binding_name, "FindEntity") != 0) return false;
    if (argument_count < 1 || lua_type(state, 1) != LUA_TSTRING) return false;
    const char* name = lua_tolstring(state, 1, nullptr);
    if (name == nullptr) return false;
    const std::map<std::string, int>::const_iterator found = scene_entity_ids_.find(name);
    if (found == scene_entity_ids_.end()) return false;
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, found->second);
    lua_getfield(state, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (lua_isnil(state, -1)) {
        ::lua_settop(state, ::lua_gettop(state) - 1);
        return false;
    }
    lua_getfield(state, -1, key);
    ::lua_remove(state, -2);
    if (lua_isnil(state, -1)) {
        ::lua_settop(state, ::lua_gettop(state) - 1);
        return false;
    }
    if (!error_replay_) ++summary_.entity_resolves;
    return true;
}

void GameMissionLuaHost::set_error_replay(bool active) noexcept { error_replay_ = active; }

bool GameMissionLuaHost::error_replay() const noexcept { return error_replay_; }

void GameMissionLuaHost::note_binding_subject(std::size_t row, int entity_id) {
    if (row >= bsp::mission_lua_binding_count()) return;
    const char* name = bsp::mission_lua_bindings()[row].name;
    for (GameMissionNativeCall& record : summary_.natives) {
        if (record.name != name) continue;
        for (int existing : record.entity_subjects) {
            if (existing == entity_id) return;
        }
        record.entity_subjects.push_back(entity_id);
        return;
    }
}

void GameMissionLuaHost::report_entity_subjects() {
    std::vector<int> distinct;
    std::size_t bindings = 0;
    for (const GameMissionNativeCall& record : summary_.natives) {
        if (record.entity_subjects.empty()) continue;
        ++bindings;
        log_.notef("  script binding %-28s %08lx addressed %zu created instance(s) in "
            "%llu call(s)", record.name.c_str(),
            static_cast<unsigned long>(record.address), record.entity_subjects.size(),
            record.calls);
        for (int id : record.entity_subjects) {
            bool seen = false;
            for (int existing : distinct) {
                if (existing == id) {
                    seen = true;
                    break;
                }
            }
            if (!seen) distinct.push_back(id);
        }
    }
    std::size_t reconstructed = 0;
    for (const GameMissionNativeCall& record : summary_.natives) {
        if (record.entity_subjects.empty()) continue;
        if (GameScriptOrdersHost::handles(record.name.c_str())) ++reconstructed;
    }
    log_.notef("summary mission script orders instances=%zu/%zu bindings=%zu "
        "reconstructed=%zu entity_resolves=%llu: milestone 2m runs the reconstructed "
        "bodies of the rows counted as reconstructed; the rest keep the record with "
        "their own row address",
        distinct.size(), summary_.self_table_entities, bindings, reconstructed,
        summary_.entity_resolves);
}

void GameMissionLuaHost::note_created_script(std::string name) {
    for (const std::string& existing : summary_.created_scripts) {
        if (existing == name) return;
    }
    summary_.created_scripts.push_back(std::move(name));
}

std::size_t GameMissionLuaHost::run_created_scripts() {
    if (state_ == nullptr || summary_.created_scripts.empty()) return 0;
    // The script manager. 00898750 is the `CreateScript` binding body, which
    // registers a script object; the fixed step's row 7 at 00875e55 drains the
    // queued Lua calls through 00888230 and row 8 at 00875e64 dispatches the
    // due entity think through 00929460. None of the three is reconstructed, so
    // the executable calls the named global once with one fresh table, which is
    // the `this` a script function takes, and records all three.
    log_.notef("script objects: the mission's stage init handed CreateScript %zu name(s). "
        "The binding body 00898750, the queued-call drain 00888230 (fixed-step row 7 at "
        "00875e55) and the due-think dispatch 00929460 (row 8 at 00875e64) are all "
        "records, so the executable calls each named global once with one fresh table and "
        "says so; the call itself is the executable's, the name is the mission's",
        summary_.created_scripts.size());
    log_.unimplemented("MissionScript::create_script", "00898750");
    log_.unimplemented("MissionScript::drain_queued_calls", "00888230");
    log_.unimplemented("MissionScript::run_due_entity_think", "00929460");

    const std::size_t natives_before = summary_.natives.size();
    const unsigned long long calls_before = summary_.native_calls;
    std::size_t ran = 0;
    // A script may create further script objects while it runs, which is what
    // the native manager then picks up, so the walk reads the list by index and
    // takes each name by value: the vector grows underneath it. The cap is the
    // executable's own guard against a script that creates itself.
    constexpr std::size_t kCreatedScriptRunLimit = 32;
    for (std::size_t i = 0;
         i < summary_.created_scripts.size() && i < kCreatedScriptRunLimit; ++i) {
        const std::string name = summary_.created_scripts[i];
        GameMissionEntryPointRun run;
        run.name = name;
        set_phase(name);
        run.defined = global_is_defined(name.c_str());
        std::vector<bsp::MissionLuaArgument> arguments;
        bsp::MissionLuaArgument self;
        self.type = bsp::MissionLuaArgumentType::Table;
        arguments.push_back(self);
        const bsp::NamedCallOutcome outcome
            = bsp::call_entry_point_if_defined(*this, name, arguments, true);
        run.dispatched = outcome.dispatched;
        run.pcall_status = outcome.pcall_status;
        if (outcome.dispatched && outcome.pcall_status != 0) {
            // The same rerun with errfunc 0 the entry points use, purely to
            // recover the message for the log.
            const int top = ::lua_gettop(state_);
            set_error_replay(true);
            lua_getfield(state_, LUA_GLOBALSINDEX, name.c_str());
            if (!lua_isnil(state_, -1)) {
                lua_createtable(state_, 0, 0);
                if (::lua_pcall(state_, 1, 0, 0) != 0) {
                    const char* message = lua_tolstring(state_, -1, nullptr);
                    run.error = message != nullptr ? message : "(no message)";
                }
            }
            set_error_replay(false);
            ::lua_settop(state_, top);
            note_error(run.error);
        }
        if (outcome.dispatched) ++ran;
        log_.notef("  script object %-20s defined=%d dispatched=%d status=%d %s",
            name.c_str(), run.defined ? 1 : 0, run.dispatched ? 1 : 0, run.pcall_status,
            run.error.empty() ? "" : run.error.c_str());
        summary_.created_script_runs.push_back(run);
    }
    log_.notef("script objects reached %zu further binding(s) in %llu call(s); every one of "
        "them is a host record with its own row address, so no order leaves the script",
        summary_.natives.size() - natives_before, summary_.native_calls - calls_before);
    report_entity_subjects();
    return ran;
}

void GameMissionLuaHost::report_mission_script_state() {
    if (state_ == nullptr) return;
    const int base = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, "Mission");
    if (!lua_istable(state_, -1)) {
        ::lua_settop(state_, base);
        log_.notef("summary mission script state: the shipped script's `Mission` table "
            "is not a table, so the run did not reach its stage init");
        return;
    }
    const int mission = ::lua_gettop(state_);
    struct Field {
        const char* key;
        const char* label;
    };
    static const Field kFields[] = {
        {"MissionPhase", "MissionPhase"}, {"EndMission", "EndMission"},
        {"Party", "Party"}, {"Distance", "Distance"}, {"Measure", "Measure"},
        {"MissionComplete", "MissionCompleteRan"},
        {"MissionFailed", "MissionFailedRan"},
    };
    std::string line;
    for (const Field& field : kFields) {
        lua_getfield(state_, mission, field.key);
        const int type = lua_type(state_, -1);
        std::string value;
        if (type == LUA_TNIL) {
            value = "nil";
        } else if (type == LUA_TBOOLEAN) {
            value = lua_toboolean(state_, -1) != 0 ? "true" : "false";
        } else if (type == LUA_TNUMBER || type == LUA_TSTRING) {
            const char* text = lua_tolstring(state_, -1, nullptr);
            value = (text != nullptr) ? text : "?";
        } else {
            value = "(other)";
        }
        ::lua_settop(state_, mission);
        if (!line.empty()) line += ' ';
        line += field.label;
        line += '=';
        line += value;
    }
    // The shape the shipped `luaGetOwnUnits` walks: recon[Mission.Party].own,
    // whose members are the nineteen category maps. Reported as a count so the
    // log says whether the mission-end path would find a table or raise.
    lua_getfield(state_, mission, "Party");
    const bool party_number = lua_type(state_, -1) == LUA_TNUMBER;
    const lua_Number party = party_number ? lua_tonumber(state_, -1) : 0;
    ::lua_settop(state_, mission);
    int own_categories = -1;
    if (party_number) {
        lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionReconGlobal);
        if (lua_istable(state_, -1)) {
            lua_pushnumber(state_, party);
            ::lua_gettable(state_, -2);
            if (lua_istable(state_, -1)) {
                lua_getfield(state_, -1, "own");
                if (lua_istable(state_, -1)) {
                    own_categories = 0;
                    lua_pushnil(state_);
                    while (lua_next(state_, -2) != 0) {
                        ++own_categories;
                        ::lua_settop(state_, ::lua_gettop(state_) - 1);
                    }
                }
            }
        }
    }
    ::lua_settop(state_, base);
    log_.notef("summary mission script state: %s entity_resolves=%llu "
        "self_table_entities=%zu recon_own_categories=%d", line.c_str(),
        summary_.entity_resolves, summary_.self_table_entities, own_categories);
}

void GameMissionLuaHost::report_natives(std::size_t limit) {
    std::vector<const GameMissionNativeCall*> ordered;
    ordered.reserve(summary_.natives.size());
    for (const GameMissionNativeCall& record : summary_.natives) ordered.push_back(&record);
    std::stable_sort(ordered.begin(), ordered.end(),
        [](const GameMissionNativeCall* a, const GameMissionNativeCall* b) {
            return a->calls > b->calls;
        });
    log_.notef("mission script natives: %llu calls over %zu distinct bindings",
        summary_.native_calls, summary_.natives.size());
    std::size_t shown = 0;
    for (const GameMissionNativeCall* record : ordered) {
        if (shown++ >= limit) break;
        log_.notef("  binding %-28s %08lx calls=%llu", record->name.c_str(),
            static_cast<unsigned long>(record->address), record->calls);
    }
}

// ---------------------------------------------------------------------------
// bsp::MissionLuaHostServices
// ---------------------------------------------------------------------------

void GameMissionLuaHost::create_state() {
    state_ = luaL_newstate();
    log_.implemented("MissionLua::create_state", "006b8740");
}

void GameMissionLuaHost::set_panic_function(std::uint32_t function) {
    static_cast<void>(function);  // 006b8720, whose body was not read
    if (state_ == nullptr) return;
    lua_atpanic(state_, panic_trampoline);
    log_.unimplemented("MissionLua::set_panic_function", "006b8720");
}

void GameMissionLuaHost::set_gc_pause(int what, int pause) {
    if (state_ == nullptr) return;
    lua_gc(state_, what, pause);
    log_.implemented("MissionLua::set_gc_pause", "006b8768");
}

void GameMissionLuaHost::open_standard_library(const bsp::LuaStandardLibrary& library) {
    if (state_ == nullptr) return;
    const std::size_t index = summary_.libraries_opened;
    if (index >= bsp::kMissionLuaStandardLibraryCount) return;
    lua_pushcclosure(state_, kLibraryOpeners[index], 0);
    ::lua_pushstring(state_, library.name);
    lua_call(state_, 1, 0);
    ++summary_.libraries_opened;
    log_.implemented("MissionLua::open_standard_library", "006b8790");
}

void GameMissionLuaHost::register_global_function(const bsp::MissionLuaBinding& binding) {
    if (state_ == nullptr) return;
    const bsp::MissionLuaBinding* rows = bsp::mission_lua_bindings();
    const std::size_t row = static_cast<std::size_t>(&binding - rows);
    lua_pushlightuserdata(state_, this);
    lua_pushinteger(state_, static_cast<lua_Integer>(row));
    lua_pushcclosure(state_, binding_trampoline, 2);
    lua_setfield(state_, LUA_GLOBALSINDEX, binding.name);
    log_.implemented("MissionLua::register_global_function", "006b8610");
}

bool GameMissionLuaHost::open_script(const std::string& path) {
    script_ = OpenScript{};
    script_.path = path;
    if (resources_ == nullptr) {
        log_.unimplemented("MissionLua::open_script", "00885110");
        return false;
    }
    script_.open = resources_->read_file(path, kScriptReadMode, script_.bytes);
    log_.implemented("MissionLua::open_script", "00885110");
    if (script_.open && phase_ == "global folders") ++summary_.global_folder_scripts;
    return script_.open;
}

int GameMissionLuaHost::script_size() {
    return static_cast<int>(script_.bytes.size());
}

void GameMissionLuaHost::read_script(char* buffer, int size) {
    if (buffer == nullptr || size <= 0) return;
    const std::size_t count
        = std::min(static_cast<std::size_t>(size), script_.bytes.size());
    if (count != 0) std::memcpy(buffer, script_.bytes.data(), count);
}

void GameMissionLuaHost::close_script() {
    script_ = OpenScript{};
}

std::vector<std::string> GameMissionLuaHost::script_variant_names(const std::string& path) {
    log_.implemented("MissionLua::script_variant_names", "00bdef90");
    if (resources_ == nullptr) return {};
    return resources_->override_paths_00bdef90(path);
}

int GameMissionLuaHost::lua_gettop() {
    return state_ != nullptr ? ::lua_gettop(state_) : 0;
}

void GameMissionLuaHost::lua_settop(int index) {
    if (state_ != nullptr) ::lua_settop(state_, index);
}

int GameMissionLuaHost::luaL_loadbuffer(const char* buffer, int size, const char* chunk_name) {
    if (state_ == nullptr) return 1;
    const int status = ::luaL_loadbuffer(state_, buffer != nullptr ? buffer : "",
        static_cast<std::size_t>(size < 0 ? 0 : size), chunk_name);
    if (status == 0 && chunk_name != nullptr
        && std::strcmp(chunk_name, bsp::kMissionLuaPlatformChunk) == 0) {
        summary_.platform_chunk_ran = true;
    }
    return status;
}

int GameMissionLuaHost::lua_pcall(int nargs, int nresults, int errfunc_index) {
    if (state_ == nullptr) return 1;
    return ::lua_pcall(state_, nargs, nresults, errfunc_index);
}

std::string GameMissionLuaHost::lua_tolstring_at_top() {
    if (state_ == nullptr) return {};
    const char* text = lua_tolstring(state_, -1, nullptr);
    const std::string message = text != nullptr ? text : std::string();
    if (!message.empty()) {
        note_error(message);
        if (phase_ == "global folders") ++summary_.global_folder_errors;
        if (phase_ == "mission chunk") summary_.mission_chunk_error = message;
        log_.notef("  chunk error in %s: %s", phase_.empty() ? "(none)" : phase_.c_str(),
            message.c_str());
    }
    return message;
}

int GameMissionLuaHost::collect_results(int count, int mode) {
    static_cast<void>(mode);  // 00887220's literal, 2 for chunks and 4 for calls
    if (state_ != nullptr && count > 0) ::lua_settop(state_, ::lua_gettop(state_) - count);
    return count;
}

void GameMissionLuaHost::lua_getglobal(const char* name) {
    if (state_ != nullptr) lua_getfield(state_, LUA_GLOBALSINDEX, name);
}

void GameMissionLuaHost::lua_pushstring(const char* text) {
    if (state_ != nullptr) ::lua_pushstring(state_, text != nullptr ? text : "");
}

void GameMissionLuaHost::lua_gettable(int index) {
    if (state_ != nullptr) ::lua_gettable(state_, index);
}

void GameMissionLuaHost::lua_remove(int index) {
    if (state_ != nullptr) ::lua_remove(state_, index);
}

void GameMissionLuaHost::lua_pushvalue(int index) {
    if (state_ != nullptr) ::lua_pushvalue(state_, index);
}

void GameMissionLuaHost::push_argument(const bsp::MissionLuaArgument& argument) {
    if (state_ == nullptr) return;
    switch (argument.type) {
    case bsp::MissionLuaArgumentType::Number:
        lua_pushnumber(state_, static_cast<lua_Number>(argument.number));
        return;
    case bsp::MissionLuaArgumentType::String:
        ::lua_pushstring(state_, argument.text.c_str());
        return;
    case bsp::MissionLuaArgumentType::Boolean:
        lua_pushboolean(state_, argument.boolean ? 1 : 0);
        return;
    case bsp::MissionLuaArgumentType::EntityId: {
        // thisTable[format("%d", id)], or nil for a zero id. The self table is
        // packet cc_mission_natives'; without it the lookup answers nil, which
        // is the same value a missing entity produces.
        if (argument.entity_id == 0) {
            lua_pushnil(state_);
            return;
        }
        char key[16];
        std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat,
            static_cast<int>(argument.entity_id));
        lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
        if (lua_isnil(state_, -1)) return;  // the nil stays as the argument
        lua_getfield(state_, -1, key);
        ::lua_remove(state_, -2);
        return;
    }
    case bsp::MissionLuaArgumentType::Skipped:
        return;
    case bsp::MissionLuaArgumentType::Table:
        lua_createtable(state_, static_cast<int>(argument.elements.size()), 0);
        for (std::size_t i = 0; i < argument.elements.size(); ++i) {
            push_argument(argument.elements[i]);
            lua_rawseti(state_, -2, static_cast<int>(i) + 1);
        }
        return;
    case bsp::MissionLuaArgumentType::Nil:
    default:
        lua_pushnil(state_);
        return;
    }
}

bool GameMissionLuaHost::global_is_defined(const char* name) {
    if (state_ == nullptr) return false;
    lua_getfield(state_, LUA_GLOBALSINDEX, name);
    const bool defined = !lua_isnil(state_, -1);
    ::lua_settop(state_, -2);
    log_.implemented("MissionLua::global_is_defined", "00b66200");
    return defined;
}

int GameMissionLuaHost::game_lifecycle_state() {
    // game+1FE4h, the local view mode. Single player is 0; value 2 is the state
    // 00887750 and 00887e50 refuse to run in.
    return 0;
}

void GameMissionLuaHost::adjust_call_stack_marker(int delta) { call_stack_marker_ += delta; }

void GameMissionLuaHost::adjust_reentrancy_depth(int delta) { reentrancy_depth_ += delta; }

bool GameMissionLuaHost::on_frame_job_thread() {
    // 004c1130 then the pool's virtual +10h. This process has no frame job
    // pool, so nothing is ever queued for the main thread.
    log_.unimplemented("MissionLua::on_frame_job_thread", "004c1130");
    return false;
}

void GameMissionLuaHost::queue_named_call_for_main_thread(const std::string& name) {
    static_cast<void>(name);
    log_.unimplemented("MissionLua::queue_named_call", "00887c30");
}

}  // namespace bsp::game
