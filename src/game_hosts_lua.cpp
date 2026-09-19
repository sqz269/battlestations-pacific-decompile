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

#include "bsp/air_operations.hpp"
#include "bsp/game_hosts_ai.hpp"
#include "bsp/objective_units.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/global_script_folders.hpp"
#include "bsp/lua_spawn_new.hpp"
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

// ---------------------------------------------------------------------------
// The three objective bindings, docs/MISSION_OBJECTIVES.md
// ---------------------------------------------------------------------------
// 008CD440 Objectives_Add, 008CDD60 Objectives_AddUnit and 008CE510
// Objectives_RemoveUnit are the only producers of the eight per-player-slot
// objective sets at game+21A4h..+21C0h that 00A2C450 walks. Their argument
// decoding is read from the listing: 00B677E0 BSP_LuaObject_ArgumentAt is
// __thiscall(frame, out, index) and the index is the FIRST of its three pushes,
// so scanning the body for it gives the order directly (local/argscan.py).
//
//   008CD440  arg0 optional int (008CD544 IsInteger, 008CD57F/008CD59A GetInteger)
//             arg1 optional int (008CD5E1 IsInteger, 008CD617/008CD626 GetInteger)
//             arg2 string       (008CD65A/008CD669 GetString)   <- the objective name
//             arg3 string       (008CD6D3/008CD6E2 GetString)
//             arg4 string       (008CD744/008CD758 GetString)
//             arg5 optional bool(008CD7D6 IsBoolean, 008CD832/008CD846 GetBoolean)
//             arg6.. the targets, walked without an immediate index
//   008CDD60  arg0 party, arg1 slot, arg2 name (kObjectiveNameArgument),
//             arg3.. the targets (kObjectiveFirstTargetArgument)
//
// Argument 0 builds a mask of every slot whose player carries that party and
// argument 1, when present, replaces it with that one slot; the add loop then
// runs once per set bit (008CDAF0 SHL EAX,CL against the mask at [ESP+1Ch],
// 008CDB09 MOV ECX,[EDX+EBP] over game+21A4h). A target is an entity table and
// its `ID` is the identity milestone 2l established, so `unit = ID - 1` is the
// same resolve 00888AA0 stands in for at 00888AA0's host binding.
int objective_argument_int(lua_State* state, int index, bool& present) {
    const int slot = index + 1;
    present = false;
    if (slot > lua_gettop(state)) return 0;
    const int type = lua_type(state, slot);
    if (type != LUA_TNUMBER && type != LUA_TSTRING) return 0;
    present = true;
    return static_cast<int>(lua_tonumber(state, slot));
}

std::string objective_argument_string(lua_State* state, int index) {
    const int slot = index + 1;
    if (slot > lua_gettop(state)) return std::string();
    if (lua_type(state, slot) != LUA_TSTRING) return std::string();
    const char* text = lua_tolstring(state, slot, nullptr);
    return text != nullptr ? std::string(text) : std::string();
}

// 00888AA0's stand-in: the `ID` field 00928A00 seeds, minus one.
bool objective_argument_unit(lua_State* state, int index, std::size_t& unit) {
    const int slot = index + 1;
    const int top = lua_gettop(state);
    if (slot > top || lua_type(state, slot) != LUA_TTABLE) return false;
    lua_getfield(state, slot, "ID");
    const int type = lua_type(state, -1);
    const bool number = type == LUA_TNUMBER || type == LUA_TSTRING;
    const int id = number ? static_cast<int>(lua_tonumber(state, -1)) : 0;
    lua_settop(state, top);
    if (!number || id <= 0) return false;
    unit = static_cast<std::size_t>(id - 1);
    return true;
}

// 008CDEF2's loop and 008CDFE0's explicit slot. This process has one player
// record, slot 0, and no party field on it, so the party arm cannot select a
// second slot; the explicit arm is exact. Labelled substitution for
// objective_party_slot_mask's player+28h read.
unsigned int objective_slot_mask(bool have_party, int party, bool have_slot, int slot) {
    if (have_slot && slot >= 0 &&
        static_cast<std::size_t>(slot) < bsp::game::GameObjectiveSets::kSlotCount) {
        return bsp::objective_explicit_slot_mask(slot);
    }
    (void)party;
    // Without player records only slot 0 is active here (008CDF58's two bytes).
    return have_party ? 1u : 1u;
}

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
    const bool objective_row = dispatch_row.address == 0x008cd440u
        || dispatch_row.address == 0x008cdd60u || dispatch_row.address == 0x008ce510u;
    const bool get_property_row = dispatch_row.address == 0x0088bf80u;
    const bool ready_row = dispatch_row.address == 0x00895d20u;
    const bool launch_row = dispatch_row.address == 0x0089e3c0u;
    // Packet cc8_lua_generate_object. It is handled here rather than routed to
    // the orders host because it needs both the units host, through that host,
    // and this host's own `thisTable`.
    const bool generate_row = dispatch_row.address == 0x00944fd0u;
    // Packet cc8_spawn_new_route. Same reason as the row above: the request it
    // queues is drained into the units host through this host's own frame step,
    // and its callback needs this host's `thisTable`.
    const bool spawn_new_row = dispatch_row.address == 0x0094c480u;
    const bool handled = avoidance_setting || objective_row || get_property_row || ready_row
        || launch_row || generate_row || spawn_new_row
        || (orders != nullptr && GameScriptOrdersHost::handles(dispatch_row.name));
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
    if (objective_row && !host->error_replay()) {
        bsp::game::GameObjectiveSets& sets = bsp::game::game_objective_sets();
        bool have_party = false;
        bool have_slot = false;
        const int party = objective_argument_int(state, 0, have_party);
        const int slot_argument = objective_argument_int(state, 1, have_slot);
        const unsigned int mask =
            objective_slot_mask(have_party, party, have_slot, slot_argument);
        const bool is_add = dispatch_row.address == 0x008cd440u;
        const bool is_remove = dispatch_row.address == 0x008ce510u;
        // 008CD65A for Add and 008CDFFA (kObjectiveNameArgument) for AddUnit
        // both read argument 2 as the objective name.
        const std::string name = objective_argument_string(state, 2);
        // 008CD440's targets begin after its three strings and its boolean;
        // 008CDD60's at kObjectiveFirstTargetArgument.
        const int first_target = is_add ? 6 : bsp::kObjectiveFirstTargetArgument;
        int units_touched = 0;
        for (int k = 0; k < static_cast<int>(bsp::game::GameObjectiveSets::kSlotCount); ++k) {
            if ((mask & (1u << k)) == 0u) continue;
            if (is_add) sets.add_objective(k, name);
            for (int arg = first_target; arg < argc; ++arg) {
                std::size_t unit = 0;
                if (!objective_argument_unit(state, arg, unit)) continue;
                const bool moved = is_remove ? sets.remove_unit(k, name, unit)
                                             : sets.add_unit(k, name, unit);
                if (moved) ++units_touched;
            }
        }
        host->note_objective_binding(dispatch_row.name, name, mask, units_touched);
        return 0;
    }
    if (get_property_row && !host->error_replay()) {
        return host->run_get_property_0088bf80(state, argc);
    }
    if (ready_row && !host->error_replay()) {
        return host->run_is_ready_to_send_planes_00895d20(state, argc);
    }
    if (launch_row && !host->error_replay()) {
        return host->run_launch_squadron_0089e3c0(state, argc);
    }
    if (generate_row && !host->error_replay()) {
        return host->run_generate_object_00944fd0(state, argc);
    }
    if (spawn_new_row && !host->error_replay()) {
        return host->run_spawn_new_00949750(state, argc);
    }
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
    if (vfs_.ready()) {
        resources_ = std::make_unique<bsp::VfsLocaleRuntime>(vfs_.context(),
            vfs_.search_registrations(), content_suffixes_, []() -> std::uint32_t { return 0; });
    }
}

GameMissionLuaHost::~GameMissionLuaHost() {
    // The world walk holds a bare pointer to this host for the spawn drain.
    bsp::set_spawn_queue_drain(nullptr);
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
            // The plane rate/accel keys, in the spellings 007D1F70's reader
            // uses (src/plane_class_fields.cpp:301-311). A ship row has none of
            // them and reads zero, which is what a caller should see.
            row.roll_spd = number("RollSpd");
            row.pitch_spd = number("PitchSpd");
            row.yaw_spd = number("YawSpd");
            row.yaw_roll_ratio = number("YawRollRatio");
            row.slide_ratio = number("SlideRatio");
            row.roll_accel = number("RollAccel");
            row.pitch_accel = number("PitchAccel");
            row.yaw_accel = number("YawAccel");
            row.negative_pitch_ratio = number("NegativePitchRatio");
            row.plane_stall_spd = number("StallSpd");
            row.turn_roll_spd = number("TurnRollSpd");
            row.turn_roll = number("TurnRoll");
            // The aerodynamic trio. XDrag and YDrag are the body-frame damping
            // 007DBD37-007DBE0D applies, TravelSpeed is the airspeed 007C6340
            // seeds a plane with, and MaxSpd is the numerator of the run
            // profile's speed ratio at 009F9D30. A ship row carries none of
            // them and reads zero, which is the right answer for a ship.
            row.x_drag = number("XDrag");
            row.y_drag = number("YDrag");
            row.max_spd = number("MaxSpd");
            row.travel_speed = number("TravelSpeed");
            // 007D20C6 scales Accel in place by tuning+31Ch * tuning+320h when
            // the second is above 1.0 and leaves it raw otherwise
            // (src/plane_class_fields.cpp:207-219). The raw value is read here:
            // thrust and the derived drag coefficient desc+50Ch both take
            // desc+164h, so the scaling cancels out of the equilibrium airspeed
            // and only changes how quickly a plane reaches it. Labelled partial.
            row.accel = number("Accel");
            row.glide_rate = number("GlideRate");
            row.drag_pitch_ratio = number("DragPitchRatio");
            row.air_brake_drag = number("AirBrakeDrag");
            row.drop_angle = number("DropAngle");
            row.swim_height = number("SwimHeight");
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

namespace {

// The two script paths 007E2A20 runs, in its order, and the global the second
// defines; include/bsp/game_tuning_singleton.hpp carries the literals. These are
// the forward-slash spellings this process's VFS takes, matching
// kShipGlobalsScriptPath above.
constexpr const char* kPlaneGlobalsConstantsPath = "Scripts/global/luaMW_init.lua";
constexpr const char* kPlaneGlobalsScriptPath = "Scripts/datatables/PlaneGlobals.lua";

// bsp::GameTuningLuaHost over a **private** Lua state.
//
// The private state is not a convenience, it is required, and the requirement
// was found the hard way. 007E2A20 creates its own Lua state owner and runs the
// two scripts in it; the first run of this adapter used the mission state
// instead, on the reasoning that this process has one interpreter. That run
// died at the first unit with "unit observer creator projection is
// unavailable", because `Scripts/global/luaMW_init.lua` line 432 is
// `VehicleClass = {}`. Running it on the shared state wipes the table the
// autoload step filled from vehicleclasses.lua, and every unit afterwards has
// no class. The native's private state is what keeps that from mattering, so
// this reproduces it.
//
// The file bytes still come through the mission host's VFS - that part is
// shared and correct - but the chunks run on the private state.
//
// A handle is a registry reference, not a stack index, and that is also forced:
// the recovered driver takes the globals table, then PlaneGlobals out of it,
// then releases the globals table while still holding the root. Stack indices
// cannot express that. luaL_ref answers LUA_REFNIL for a nil value and
// lua_rawgeti pushes nil back for it, so a key the data file omits travels
// through as nil and lands on the kind's default.
class LuaGameTuningHost final : public bsp::GameTuningLuaHost {
public:
    LuaGameTuningHost(GameMissionLuaHost& owner, GameHostLog& log)
        : owner_(owner), log_(log) {
        state_ = luaL_newstate();
        if (state_ != nullptr) {
            // The same seven openers 006B8740 step 4 installs on the mission
            // state, in the order of the pair table at 00CF8350. The private
            // state gets the libraries the scripts expect and no more;
            // luaopen_package is absent from the native's table too.
            for (std::size_t i = 0; i < bsp::kMissionLuaStandardLibraryCount; ++i) {
                lua_pushcfunction(state_, kLibraryOpeners[i]);
                lua_pushstring(state_, "");
                if (lua_pcall(state_, 1, 0, 0) != 0) {
                    lua_settop(state_, lua_gettop(state_) - 1);
                }
            }
            // 00B6A303 with callback 00B69E00: DoFile is a global the binding
            // table does not carry, installed separately on the state. The
            // private state needs it because luaMW_init.lua line 455 calls
            // DoFile("unlocks.lua") - without it the constants script aborts
            // there, which leaves the DEG/KMH helpers defined (they are above
            // it) but everything below it undefined.
            lua_pushlightuserdata(state_, this);
            lua_pushcclosure(state_, &LuaGameTuningHost::dofile_trampoline, 1);
            lua_setfield(state_, LUA_GLOBALSINDEX, "DoFile");
        }
    }
    ~LuaGameTuningHost() override {
        if (state_ != nullptr) {
            lua_close(state_);
        }
    }
    LuaGameTuningHost(const LuaGameTuningHost&) = delete;
    LuaGameTuningHost& operator=(const LuaGameTuningHost&) = delete;

    void run_script(const char* path) override {
        // 00B69D40, the private state owner's own runner. The bytes come from
        // the same VFS-backed reader 00885110 uses; only the state differs.
        log_.implemented("GameTuning::run_script", "00b69d40");
        if (state_ == nullptr || path == nullptr) {
            ++script_failures_;
            return;
        }
        const std::string script_path(path);
        if (!owner_.open_script(script_path)) {
            ++script_failures_;
            log_.notef("plane globals: %s did not open", path);
            return;
        }
        const int size = owner_.script_size();
        std::vector<char> buffer(static_cast<std::size_t>(size < 0 ? 0 : size));
        owner_.read_script(buffer.empty() ? nullptr : buffer.data(), size);
        owner_.close_script();
        if (buffer.empty()) {
            ++script_failures_;
            log_.notef("plane globals: %s is empty", path);
            return;
        }
        if (luaL_loadbuffer(state_, buffer.data(), buffer.size(), path) != 0 ||
            lua_pcall(state_, 0, 0, 0) != 0) {
            ++script_failures_;
            const char* message = lua_tolstring(state_, -1, nullptr);
            log_.notef("plane globals: %s failed: %s", path,
                message != nullptr ? message : "(no message)");
            lua_settop(state_, lua_gettop(state_) - 1);
        }
    }

    Handle globals() override {
        log_.implemented("GameTuning::globals", "00b67980");
        if (state_ == nullptr) return encode(LUA_REFNIL);
        lua_pushvalue(state_, LUA_GLOBALSINDEX);
        return encode(luaL_ref(state_, LUA_REGISTRYINDEX));
    }

    Handle table_field(Handle parent, const char* key) override {
        log_.implemented("GameTuning::table_field", "00b67800");
        if (state_ == nullptr) return encode(LUA_REFNIL);
        push(parent);
        if (lua_type(state_, -1) != LUA_TTABLE) {
            lua_settop(state_, lua_gettop(state_) - 1);
            ++missing_;
            return encode(LUA_REFNIL);
        }
        lua_getfield(state_, -1, key);
        const int ref = luaL_ref(state_, LUA_REGISTRYINDEX);
        lua_settop(state_, lua_gettop(state_) - 1);
        if (ref == LUA_REFNIL) {
            ++missing_;
            if (missing_names_.size() < 8 && key != nullptr) {
                missing_names_.push_back(key);
            }
        }
        return encode(ref);
    }

    Handle table_element(Handle parent, int one_based_index) override {
        log_.implemented("GameTuning::table_element", "00b67720");
        if (state_ == nullptr) return encode(LUA_REFNIL);
        push(parent);
        if (lua_type(state_, -1) != LUA_TTABLE) {
            lua_settop(state_, lua_gettop(state_) - 1);
            ++missing_;
            return encode(LUA_REFNIL);
        }
        lua_pushinteger(state_, one_based_index);
        lua_gettable(state_, -2);
        const int ref = luaL_ref(state_, LUA_REGISTRYINDEX);
        lua_settop(state_, lua_gettop(state_) - 1);
        if (ref == LUA_REFNIL) ++missing_;
        return encode(ref);
    }

    bsp::GameTuningLuaValue value(Handle handle) override {
        log_.implemented("GameTuning::value", "00b66270");
        bsp::GameTuningLuaValue out;
        out.nil = true;
        if (state_ == nullptr) return out;
        push(handle);
        const int type = lua_type(state_, -1);
        if (type == LUA_TNUMBER) {
            out.nil = false;
            out.number = lua_tonumber(state_, -1);
            out.boolean = out.number != 0.0;
        } else if (type == LUA_TBOOLEAN) {
            out.nil = false;
            out.boolean = lua_toboolean(state_, -1) != 0;
            out.number = out.boolean ? 1.0 : 0.0;
        } else if (type == LUA_TSTRING) {
            // 00B66270 is a bare GetNumber, so it converts a numeric string.
            out.nil = false;
            out.number = lua_tonumber(state_, -1);
            out.boolean = out.number != 0.0;
        }
        lua_settop(state_, lua_gettop(state_) - 1);
        return out;
    }

    void number_triple(Handle handle, float out[3]) override {
        log_.implemented("GameTuning::number_triple", "00b67a80");
        out[0] = out[1] = out[2] = 0.0f;
        if (state_ == nullptr) return;
        push(handle);
        if (lua_type(state_, -1) == LUA_TTABLE) {
            for (int i = 0; i < 3; ++i) {
                lua_pushinteger(state_, i + 1);
                lua_gettable(state_, -2);
                if (lua_type(state_, -1) == LUA_TNUMBER) {
                    out[i] = static_cast<float>(lua_tonumber(state_, -1));
                }
                lua_settop(state_, lua_gettop(state_) - 1);
            }
        }
        lua_settop(state_, lua_gettop(state_) - 1);
    }

    void release(Handle handle) override {
        log_.implemented("GameTuning::release", "00b67700");
        ++releases_;
        if (state_ == nullptr) return;
        luaL_unref(state_, LUA_REGISTRYINDEX, decode(handle));
    }

    // Whether the private state ended up with the root table 007E2A20 reads.
    bool root_is_table(const char* name) {
        if (state_ == nullptr) return false;
        const int top = lua_gettop(state_);
        lua_getfield(state_, LUA_GLOBALSINDEX, name);
        const bool table = lua_type(state_, -1) == LUA_TTABLE;
        lua_settop(state_, top);
        return table;
    }

    std::size_t missing() const noexcept { return missing_; }
    std::string missing_names() const {
        std::string out;
        for (const std::string& name : missing_names_) {
            if (!out.empty()) out += ",";
            out += name;
        }
        return out;
    }
    std::size_t script_failures() const noexcept { return script_failures_; }

private:
    // LUA_REFNIL is -1 and LUA_NOREF is -2, so the bias keeps every encoded
    // handle a small positive number and round-trips both.
    static int dofile_trampoline(lua_State* state) {
        LuaGameTuningHost* self = static_cast<LuaGameTuningHost*>(
            lua_touserdata(state, lua_upvalueindex(1)));
        const char* path = lua_tolstring(state, 1, nullptr);
        if (self != nullptr && path != nullptr) {
            self->run_script(path);
        }
        return 0;
    }

    static Handle encode(int ref) { return static_cast<Handle>(ref + 8); }
    static int decode(Handle handle) { return static_cast<int>(handle) - 8; }
    void push(Handle handle) { lua_rawgeti(state_, LUA_REGISTRYINDEX, decode(handle)); }

    lua_State* state_{nullptr};
    GameMissionLuaHost& owner_;
    GameHostLog& log_;
    std::size_t missing_{0};
    std::vector<std::string> missing_names_;
    std::size_t releases_{0};
    std::size_t script_failures_{0};
};

}  // namespace

bool GameMissionLuaHost::load_plane_globals_007e2a20() {
    set_phase("plane globals");
    LuaGameTuningHost host(*this, log_);
    bsp::GameTuningBlock block{};
    // The recovered driver runs both scripts itself, in the native's order, and
    // walks all 423 key paths. Nothing here chooses keys.
    bsp::game_tuning_load_007e2a20(host, block);
    const bool table = host.root_is_table(bsp::kGameTuningRootTable);

    if (table) {
        plane_globals_ = block;
        plane_globals_loaded_ = true;
        log_.implemented("GameTuning::load_from_plane_globals", "007e2a20");
    }
    // The four values the plane control path actually consumes, logged so a run
    // says whether the authored data arrived rather than leaving it assumed.
    // The rotation factors are what 007DA710's rate polynomial reads through the
    // 00F872F0 mirror; the control range is what 007D9A70's speed ramp reads.
    // docs/PLANE_CONTROL_RATE_LAW.md, docs/PLANE_CONTROL_AUTHORITY.md.
    log_.notef("plane globals: `%s` is %s, keys missing=%zu script failures=%zu; "
        "RotationFactors A=%.9g B=%.9g C=%.9g, ControlRange %.9g..%.9g; missing keys: %s",
        bsp::kGameTuningRootTable, table ? "a table" : "absent",
        host.missing(), host.script_failures(),
        static_cast<double>(block.dynamics_rotation_factors_a),
        static_cast<double>(block.dynamics_rotation_factors_b),
        static_cast<double>(block.dynamics_rotation_factors_c),
        static_cast<double>(block.dynamics_spd_multipliers_control_range_min),
        static_cast<double>(block.dynamics_spd_multipliers_control_range_max),
        host.missing_names().c_str());
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
    // Packet cc8_airops_launch_tick. 006C5050's creator and the squadron's live
    // plane count at entity+3CCh are the two things the air-operations code
    // cannot reach on its own; both are bound here, where the orders host and the
    // mission table are both in hand, and both are cleared on a detach.
    // Packet cc8_spawn_new_route, and the same rule: the world walk that runs
    // the spawn drain cannot reach this host, so the host publishes itself for
    // the duration of the mission and takes itself back on a detach. The queue
    // is cleared with it, because the manager is constructed per world
    // (004DFAC3 writes 00F89B3C in BSP_Game_ConstructWorld and 004D2D7E clears
    // it in BSP_Game_DestroyWorld), so a request cannot outlive its mission.
    bsp::set_spawn_queue_drain(orders == nullptr ? nullptr : this);
    if (orders == nullptr) {
        bsp::spawn_request_queue().clear();
        bsp::set_air_ops_squadron_factory(nullptr);
        bsp::set_air_ops_squadron_plane_count(nullptr, nullptr);
        return;
    }
    bsp::set_air_ops_squadron_factory(this);
    bsp::set_air_ops_squadron_plane_count(
        [](std::uint32_t squadron, void* context) -> std::int32_t {
            GameScriptOrdersHost* host = static_cast<GameScriptOrdersHost*>(context);
            return host != nullptr ? host->air_ops_squadron_plane_count(squadron) : 0;
        },
        orders);
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

namespace {
// Argument 0 of all three air-operations bindings is the entity table 00888AA0
// resolves. Its `ID` is the unit index plus one, which is what the deck registry
// is bound to.
int air_ops_entity_id(lua_State* state) {
    if (state == nullptr || ::lua_type(state, 1) != LUA_TTABLE) return 0;
    const int top = ::lua_gettop(state);
    ::lua_getfield(state, 1, "ID");
    const int type = ::lua_type(state, -1);
    const int id = (type == LUA_TNUMBER || type == LUA_TSTRING)
        ? static_cast<int>(::lua_tonumber(state, -1)) : 0;
    ::lua_settop(state, top);
    return id;
}

// 0089E3C0 reads its integers through the call frame; a non-number argument
// reaches it as the reader's own zero.
std::int32_t air_ops_integer_argument(lua_State* state, int index, bool& present) {
    const int slot = index + 1;
    present = false;
    if (slot > ::lua_gettop(state)) return 0;
    const int type = ::lua_type(state, slot);
    if (type != LUA_TNUMBER && type != LUA_TSTRING) return 0;
    present = true;
    return static_cast<std::int32_t>(::lua_tonumber(state, slot));
}
} // namespace

bool GameMissionLuaHost::stationary_class_exists(const std::string& name) {
    if (state_ == nullptr || name.empty()) return false;
    const int top = ::lua_gettop(state_);
    bool found = false;
    // 00851CB0: BSP_LuaStateOwner_GetGlobals, BSP_LuaObject_GetByName with the
    // literal, then BSP_LuaObject_GetByNativeString with the type's own text.
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "StationaryClass");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_getfield(state_, -1, name.c_str());
        found = ::lua_type(state_, -1) == LUA_TTABLE;
    }
    ::lua_settop(state_, top);
    return found;
}

int GameMissionLuaHost::run_is_ready_to_send_planes_00895d20(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    if (state == nullptr) return 0;
    // 00895E3B takes the block, 00895E4B the class test, 00895E5F the answer.
    // The native pushes a boolean whatever happens, so a unit with no deck in
    // this process answers false rather than pushing nothing.
    const bsp::AirOpsDeck* deck = bsp::air_ops_decks().find_by_entity_id(
        air_ops_entity_id(state));
    const bool ready = deck != nullptr && bsp::air_ops_is_ready_to_send_planes_00895d20(*deck);
    ++summary_.air_ops_ready_calls;
    if (ready) ++summary_.air_ops_ready_true;
    if (summary_.air_ops_ready_calls <= 12) {
        log_.notef("  IsReadyToSendPlanes 00895d20: deck=%d slots=%zu launch_in_progress=%u "
            "-> %s", deck != nullptr ? 1 : 0, deck != nullptr ? deck->slots.size() : 0u,
            deck != nullptr ? deck->launch_in_progress : 0u, ready ? "true" : "false");
    }
    ::lua_pushboolean(state, ready ? 1 : 0);
    log_.implemented("MissionLuaNative::IsReadyToSendPlanes", "00895d20");
    return 1;
}

int GameMissionLuaHost::run_launch_squadron_0089e3c0(lua_State* state, int argument_count) {
    if (state == nullptr) return 0;
    bsp::AirOpsDeck* deck = bsp::air_ops_decks().find_mutable_by_entity_id(
        air_ops_entity_id(state));
    ++summary_.air_ops_launch_calls;
    if (deck == nullptr) {
        // With no block the native would still run 006CC690 against whatever the
        // getter returned; this process has nothing to run it on, so the record
        // is the honest answer and the caller gets no index.
        log_.notef("  LaunchSquadron 0089e3c0: no deck for this entity, no slot");
        log_.unimplemented("MissionLuaNative::LaunchSquadron", "0089e3c0");
        return 0;
    }
    bsp::AirOpsLaunchRequest request;
    bool present = false;
    // 0089E4xx reads argument 1 as the class token and argument 2 as the count.
    request.vehicle_class = static_cast<std::uint32_t>(
        air_ops_integer_argument(state, 1, present));
    request.count = air_ops_integer_argument(state, 2, present);
    // The 00B663F0 argument-count test against 4: only a fourth argument
    // replaces the arm, which otherwise defaults to class+134h.
    bool arm_present = false;
    const std::int32_t arm = air_ops_integer_argument(state, 3, arm_present);
    // class+134h is not authored under any key in this installation's
    // vehicleclasses.lua, so the default is zero here. contract.
    request.class_default_arm = 0;
    request.arm_given = arm_present && argument_count >= 4;
    request.arm = request.arm_given ? arm : request.class_default_arm;

    const bsp::AirOpsLaunchResult result
        = bsp::air_ops_launch_squadron_006cc690(*deck, request);
    if (result.started) ++summary_.air_ops_launch_started;
    if (result.queued) ++summary_.air_ops_launch_queued;
    if (summary_.air_ops_launch_calls <= 12) {
        log_.notef("  LaunchSquadron 0089e3c0: class=%u count=%d arm=%d%s -> slot %d (%s), "
            "returns %d", request.vehicle_class, request.count, request.arm,
            request.arm_given ? " (given)" : " (class default)", result.slot_index,
            result.started ? "started" : result.queued ? "queued" : "none",
            result.slot_index + 1);
    }
    // 0089E4xx pushes the 006CC690 result plus one, which is the 1-based index
    // into the same `slots` array GetProperty publishes.
    ::lua_pushinteger(state, static_cast<lua_Integer>(result.slot_index) + 1);
    log_.implemented("MissionLuaNative::LaunchSquadron", "0089e3c0");
    return 1;
}

namespace {

// 00CE3828 holds the double 2*pi, and 0046DD4C compares the yaw against it: the
// rotation is applied only when the argument is SMALLER. 00CE38B8's default
// 10.0f is therefore not a special case but a value chosen to fail that test,
// which is what "keep the authored orientation" means.
constexpr double kSceneGenerateObjectYawSentinelCeiling = 6.283185307179586;

// 0088B840's predicate and 00888760's reader: a position argument is a table of
// exactly three numbers, written to [out], [out+4], [out+8].
bool read_vector3_00888760(lua_State* state, int index, float out[3]) {
    if (::lua_type(state, index) != LUA_TTABLE) return false;
    int found = 0;
    for (int i = 1; i <= 3; ++i) {
        ::lua_rawgeti(state, index, i);
        if (::lua_type(state, -1) != LUA_TNUMBER) {
            ::lua_settop(state, ::lua_gettop(state) - 1);
            return false;
        }
        out[i - 1] = static_cast<float>(::lua_tonumber(state, -1));
        ::lua_settop(state, ::lua_gettop(state) - 1);
        ++found;
    }
    // 0088B974 CMP EDI,3 requires exactly three entries; a fourth makes it not a
    // position table. The rawgeti walk above cannot see a fourth, so this checks
    // it directly rather than claiming the native's count.
    ::lua_rawgeti(state, index, 4);
    const bool extra = ::lua_type(state, -1) != LUA_TNIL;
    ::lua_settop(state, ::lua_gettop(state) - 1);
    return found == 3 && !extra;
}

// 00467050(frame, 0.0f, value, 0.0f), the yaw-only rotation 0046DD9F applies to
// the local frame. Only the two axes a yaw touches are rewritten; the
// translation row the caller may just have written is left alone.
void apply_scene_yaw_00467050(float world[16], float yaw) {
    const float c = std::cos(yaw);
    const float s = std::sin(yaw);
    world[0] = c;  world[1] = 0.0f; world[2] = -s;
    world[4] = 0.0f; world[5] = 1.0f; world[6] = 0.0f;
    world[8] = s;  world[9] = 0.0f; world[10] = c;
}

// The `thisTable` slot for an entity id, which is what the entity-returning tail
// 0089903C pushes. Leaves it on the stack on success.
bool push_resolved_entity_by_id(lua_State* state, int entity_id) {
    if (state == nullptr || entity_id <= 0) return false;
    lua_getfield(state, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (!lua_istable(state, -1)) {
        ::lua_settop(state, ::lua_gettop(state) - 1);
        return false;
    }
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, entity_id);
    lua_getfield(state, -1, key);
    if (!lua_istable(state, -1)) {
        ::lua_settop(state, ::lua_gettop(state) - 2);
        return false;
    }
    ::lua_remove(state, -2);
    return true;
}

}  // namespace

int GameMissionLuaHost::run_generate_object_00944fd0(lua_State* state, int argument_count) {
    ++summary_.generate_object_calls;
    if (state == nullptr) return 0;
    // 009450A0/00B662B0: argument 0 is the authored object's name and is the key
    // 0046D96F looks up. Anything else returns nothing, as 0046D9AF does for a
    // name the map does not hold.
    if (argument_count < 1 || ::lua_type(state, 1) != LUA_TSTRING) return 0;
    const char* raw_name = ::lua_tolstring(state, 1, nullptr);
    const std::string name = raw_name != nullptr ? raw_name : std::string();
    if (name.empty()) return 0;

    bsp::game::SceneSpawnPoolEntry* entry = bsp::game::scene_spawn_pool().find(name);
    if (entry == nullptr) {
        ++summary_.generate_object_unknown;
        if (summary_.generate_object_unknown <= 8) {
            log_.notef("  GenerateObject 00944fd0: \"%s\" is in no held-back record, so "
                "nothing is created, which is 0046D9AF's answer for a name the "
                "named-object map does not hold", name.c_str());
        }
        return 0;
    }
    // 0046D930 has no already-created arm: it instantiates on every call. This
    // process keeps the entry so a second call cannot make a second carrier, and
    // answers it with the same entity, which is a DEVIATION and is here because
    // a duplicated capital ship is worse than a repeated handle.
    if (entry->spawned) {
        ++summary_.generate_object_repeat;
        if (push_resolved_entity_by_id(state, entry->entity_id)) return 1;
        return 0;
    }

    // The shape decision of 0094518A..00945229. Argument 1 is a second name when
    // it is a string; when it is a three-number table it is the world position,
    // and otherwise the position is argument 2. The yaw follows it.
    bsp::game::GameSceneEntityRecord record = entry->record;
    int position_index = -1;
    if (argument_count > 1 && ::lua_type(state, 2) == LUA_TTABLE) {
        position_index = 2;
    } else if (argument_count > 2 && ::lua_type(state, 3) == LUA_TTABLE) {
        position_index = 3;
    }
    bool placed = false;
    float position[3] = {0.0f, 0.0f, 0.0f};
    if (position_index > 0 && read_vector3_00888760(state, position_index, position)) {
        // 0046DD48 copies the three floats into the local frame's +30h/+34h/+38h
        // translation slots, leaving the authored rotation alone.
        record.world[12] = position[0];
        record.world[13] = position[1];
        record.world[14] = position[2];
        placed = true;
    }
    // 0046DD4C..0046DD9F: the yaw applies only when it is SMALLER than the double
    // 2*pi at 00CE3828, and the default 10.0f at 00CE38B8 is the sentinel that
    // means "keep the authored orientation". 10.0 > 2*pi, so the sentinel fails
    // the same test rather than needing a special case.
    const int yaw_index = position_index > 0 ? position_index + 1 : 0;
    bool yawed = false;
    if (yaw_index > 0 && argument_count >= yaw_index
        && ::lua_type(state, yaw_index) == LUA_TNUMBER) {
        const double yaw = ::lua_tonumber(state, yaw_index);
        if (yaw < kSceneGenerateObjectYawSentinelCeiling) {
            apply_scene_yaw_00467050(record.world, static_cast<float>(yaw));
            yawed = true;
        }
    }

    if (script_orders_ == nullptr) return 0;
    const std::uint32_t entity_id
        = script_orders_->create_unit_from_scene_record_0046db4b(record);
    if (entity_id == 0u) {
        log_.notef("  GenerateObject 00944fd0: \"%s\" (%s) reached no creator, so nothing "
            "was made", name.c_str(), record.class_name.c_str());
        return 0;
    }
    // 0046DBE8 finishes with 00925F20 BSP_SEntity_InitAll, whose part this host
    // can do is the `thisTable` slot every entity that reaches virtual slot 39
    // carries; without it the script's own variable resolves to nothing.
    if (!attach_created_entity_00928a00(static_cast<int>(entity_id), name,
                                        record.type_id)) {
        log_.notef("  GenerateObject 00944fd0: \"%s\" was created as unit %u but took no "
            "`thisTable` slot, so the script's variable would be an id no binding "
            "resolves", name.c_str(), entity_id);
        return 0;
    }
    entry->spawned = true;
    entry->entity_id = static_cast<int>(entity_id);
    // The deck the scene pass built for a held-back carrier, handed over now that
    // the unit exists. 006CADD0 mode 1 runs on the load-time created path, which
    // this entity skipped, so without this a script-spawned carrier would answer
    // `GetProperty(carrier, "slots")` with nothing and IsReadyToSendPlanes would
    // refuse it for ever.
    if (entry->has_deck) {
        bsp::air_ops_decks().set(name, entry->deck);
        bsp::air_ops_decks().bind_entity_id(static_cast<int>(entity_id), name);
        log_.notef("  GenerateObject 00944fd0: \"%s\" carries an air-ops deck "
            "(slots=%zu stock=%zu), registered now that the unit exists",
            name.c_str(), entry->deck.slots.size(), entry->deck.stock.size());
    }
    ++summary_.generate_object_created;
    if (summary_.generate_object_created <= 16) {
        log_.notef("  GenerateObject 00944fd0: \"%s\" (%s type=%d party=%d) -> id %u at "
            "(%.1f %.1f %.1f)%s%s", name.c_str(), record.class_name.c_str(),
            record.type_id, record.party, entity_id,
            record.world[12], record.world[13], record.world[14],
            placed ? " placed" : " authored frame", yawed ? " yawed" : "");
    }
    log_.implemented("MissionLuaNative::GenerateObject", "00944fd0");
    if (push_resolved_entity_by_id(state, entry->entity_id)) return 1;
    return 0;
}

// --- Packet cc8_spawn_new_route ------------------------------------------
// 0094C480 SpawnNew / 00949750 the parse and enqueue / 0094C490 the drain.
// docs/LUA_SPAWN_NEW_HOST.md carries the evidence for every address here.

namespace {

// One `groupMembers` element. The six keys are the ones the shipped scripts
// write; the native reads them out of the Lua table into the 10h-byte element
// at record+4h (the class at +0h, the name at +8h). A key the script omits
// stays at the zero the record is constructed with, because every read in
// 00949750 goes through an or-default helper and never raises.
int read_member_int(lua_State* state, int table_index, const char* key) {
    ::lua_getfield(state, table_index, key);
    int value = 0;
    if (::lua_type(state, -1) == LUA_TNUMBER) {
        value = static_cast<int>(::lua_tonumber(state, -1));
    }
    ::lua_settop(state, ::lua_gettop(state) - 1);
    return value;
}

std::string read_member_string(lua_State* state, int table_index, const char* key) {
    ::lua_getfield(state, table_index, key);
    std::string value;
    if (::lua_type(state, -1) == LUA_TSTRING) {
        const char* text = ::lua_tolstring(state, -1, nullptr);
        if (text != nullptr) value = text;
    }
    ::lua_settop(state, ::lua_gettop(state) - 1);
    return value;
}

// Both ranges are two numbers at Lua indices 1 and 2 (00949CDA / 00949D19 for
// `angleRange`, 00949D9E / 00949DD4 for `distRange`), not at 0 and 1.
bool read_range_pair(lua_State* state, int table_index, const char* key,
                     float& low, float& high) {
    ::lua_getfield(state, table_index, key);
    bool read = false;
    if (::lua_type(state, -1) == LUA_TTABLE) {
        const int range = ::lua_gettop(state);
        ::lua_rawgeti(state, range, 1);
        ::lua_rawgeti(state, range, 2);
        if (::lua_type(state, -2) == LUA_TNUMBER && ::lua_type(state, -1) == LUA_TNUMBER) {
            low = static_cast<float>(::lua_tonumber(state, -2));
            high = static_cast<float>(::lua_tonumber(state, -1));
            read = true;
        }
        ::lua_settop(state, range);
    }
    ::lua_settop(state, ::lua_gettop(state) - 1);
    return read;
}

}  // namespace

int GameMissionLuaHost::run_spawn_new_00949750(lua_State* state, int argument_count) {
    if (state == nullptr) return 0;
    // argc=1 in every logged call, and 0094C480's `RET` with no immediate plus
    // 00949750's own `RET 4` say the thunk forwards exactly the one lua_State.
    // The Lua-visible argument is one table; anything else is the arm that
    // reads twelve absent fields and queues a record nothing can satisfy, so it
    // is refused here rather than queued forever.
    if (argument_count < 1 || ::lua_type(state, 1) != LUA_TTABLE) {
        ++summary_.spawn_new_rejected;
        return 0;
    }
    ++summary_.spawn_new_calls;

    bsp::SpawnNewRequest request;
    request.serial = bsp::next_spawn_request_serial_00949f2b();
    request.party = read_member_int(state, 1, "party");
    ::lua_getfield(state, 1, "player");
    request.player = ::lua_toboolean(state, -1) != 0;
    ::lua_settop(state, ::lua_gettop(state) - 1);
    request.callback = read_member_string(state, 1, "callback");
    request.id = read_member_string(state, 1, "id");

    // `groupMembers`, the vector at record+4h/+8h. 00948EE7 computes its size as
    // `(end - begin) >> 4`, so one 10h-byte element per member.
    ::lua_getfield(state, 1, "groupMembers");
    if (::lua_type(state, -1) == LUA_TTABLE) {
        const int members = ::lua_gettop(state);
        for (int i = 1;; ++i) {
            ::lua_rawgeti(state, members, i);
            if (::lua_type(state, -1) != LUA_TTABLE) {
                ::lua_settop(state, members);
                break;
            }
            const int member = ::lua_gettop(state);
            bsp::SpawnNewGroupMember row;
            row.type_class_id = read_member_int(state, member, "Type");
            row.name = read_member_string(state, member, "Name");
            row.crew = read_member_int(state, member, "Crew");
            row.race = read_member_int(state, member, "Race");
            row.wing_count = read_member_int(state, member, "WingCount");
            row.equipment = read_member_int(state, member, "Equipment");
            request.members.push_back(std::move(row));
            ::lua_settop(state, members);
        }
    }
    ::lua_settop(state, ::lua_gettop(state) - 1);

    // `area` is a sub-table and `refPos`, `angleRange`, `distRange` and `lookAt`
    // are read out of it: 00949B1A pushes the `area` key and the four reads that
    // follow (00949B30, 00949CC1, 00949D50, 00949E2B) are on what it produced,
    // which is also how every call site in this installation writes them.
    ::lua_getfield(state, 1, "area");
    if (::lua_type(state, -1) == LUA_TTABLE) {
        const int area = ::lua_gettop(state);
        ::lua_getfield(state, area, "refPos");
        if (read_vector3_00888760(state, ::lua_gettop(state), request.ref_pos)) {
            request.has_ref_pos = true;
        }
        ::lua_settop(state, area);
        request.has_angle_range = read_range_pair(state, area, "angleRange",
            request.angle_low, request.angle_high);
        // Only `distRange` has an absence arm (00949D95 CALL 00B65FB0 /
        // 00949D9C JNZ), and only its first element is clamped: 00949E0A loads
        // 10.0f, 00949E12 COMISS and 00949E21 JA select the larger.
        float dist_low = bsp::kSpawnNewDistRangeDefaultLow;
        float dist_high = bsp::kSpawnNewDistRangeDefaultHigh;
        read_range_pair(state, area, "distRange", dist_low, dist_high);
        request.dist_low = dist_low < bsp::kSpawnNewDistRangeLowMinimum
                               ? bsp::kSpawnNewDistRangeLowMinimum : dist_low;
        request.dist_high = dist_high;
        ::lua_getfield(state, area, "lookAt");
        if (read_vector3_00888760(state, ::lua_gettop(state), request.look_at)) {
            request.has_look_at = true;
        }
        ::lua_settop(state, area);
    }
    ::lua_settop(state, ::lua_gettop(state) - 1);

    ::lua_getfield(state, 1, "excludeRadiusOverride");
    if (::lua_type(state, -1) == LUA_TTABLE) {
        const int block = ::lua_gettop(state);
        request.exclude.present = true;
        request.exclude.own_horizontal
            = static_cast<float>(read_member_int(state, block, "ownHorizontal"));
        request.exclude.enemy_horizontal
            = static_cast<float>(read_member_int(state, block, "enemyHorizontal"));
        request.exclude.own_vertical
            = static_cast<float>(read_member_int(state, block, "ownVertical"));
        request.exclude.enemy_vertical
            = static_cast<float>(read_member_int(state, block, "enemyVertical"));
        request.exclude.formation_horizontal
            = static_cast<float>(read_member_int(state, block, "formationHorizontal"));
    }
    ::lua_settop(state, ::lua_gettop(state) - 1);

    if (summary_.spawn_new_queued < 16) {
        log_.notef("  SpawnNew 0094c480: serial %u party %d, %zu group member(s), "
            "callback \"%s\"%s, angleRange %s, refPos %s(%.1f %.1f %.1f)",
            request.serial, request.party, request.members.size(),
            request.callback.c_str(), request.id.empty() ? "" : " id set",
            request.has_angle_range ? "given" : "absent",
            request.has_ref_pos ? "" : "ABSENT ",
            static_cast<double>(request.ref_pos[0]),
            static_cast<double>(request.ref_pos[1]),
            static_cast<double>(request.ref_pos[2]));
    }
    bsp::spawn_request_queue().enqueue_00949530(std::move(request));
    ++summary_.spawn_new_queued;
    log_.implemented("MissionLuaNative::SpawnNew", "0094c480");
    // 00949750 returns with no value pushed; the binding pushes nothing either.
    return 0;
}

float GameMissionLuaHost::spawn_attempt_delay_0087f800() {
    if (spawn_attempt_delay_read_) return spawn_attempt_delay_;
    spawn_attempt_delay_read_ = true;
    // The image default at 00CE74F8, overridden by Globals["SpawnAttemptDelay"]
    // exactly as 0087F7D1..0087F800 overrides globalConfig+2DCh. The globals
    // script is already run by read_minimap_globals_0087d7b0, so this reads the
    // table the run left behind rather than running it a second time.
    spawn_attempt_delay_ = bsp::kSpawnAttemptDelayDefault;
    if (state_ != nullptr) {
        const int top = ::lua_gettop(state_);
        lua_getfield(state_, LUA_GLOBALSINDEX, kGlobalsGlobal);
        if (lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, bsp::kSpawnAttemptDelayGlobalsKey);
            if (lua_type(state_, -1) == LUA_TNUMBER) {
                spawn_attempt_delay_ = static_cast<float>(::lua_tonumber(state_, -1));
            }
        }
        ::lua_settop(state_, top);
    }
    log_.notef("spawn queue: SpawnAttemptDelay = %.3f s (globalConfig+2DCh, "
        "0087F800; image default 0.8 at 00CE74F8, this installation's "
        "scripts/datatables/globals.lua sets 0.5)",
        static_cast<double>(spawn_attempt_delay_));
    return spawn_attempt_delay_;
}

void GameMissionLuaHost::run_spawn_queue_0094c490(float step_seconds) {
    // DAT_00F876A4. 0094C490 never reads the delta 0094C8F0 pushes for it, so
    // the step is used only to advance the clock the interval is measured on.
    spawn_world_clock_ += step_seconds;
    bsp::SpawnRequestQueue& queue = bsp::spawn_request_queue();
    // 0094C4AE `CMP dword ptr [EDI + 0x8],EBX` with EBX = 0: an empty queue
    // returns before the clock is even read.
    if (queue.empty()) return;
    if (script_orders_ == nullptr) return;
    if (!queue.attempt_due(spawn_world_clock_, spawn_attempt_delay_0087f800())) return;

    // DEVIATION, labelled. 0094C508's walk prefers a record whose party is
    // active in the table at `game+18CCh + party*4`. This process has no party
    // table, so every party counts as active and the walk always answers the
    // head - which is what the native itself does whenever the queue holds
    // fewer than two records (0094C4F7 CMP EAX,2 / JC 0094C56B).
    const std::vector<bool> party_active(8, true);
    const std::size_t index = queue.select_0094c508(party_active);
    bsp::SpawnNewRequest request = queue.erase_009439b0(index);
    queue.stamp_attempt(spawn_world_clock_);
    ++summary_.spawn_new_attempts;
    ++request.attempts;

    fulfil_spawn_request_009483d0(request);

    if (!request.fulfilled) {
        // 0094C5AD JZ 0094C802: the record goes back on the list and is tried
        // again next interval. The native never drops it, so neither does this.
        ++summary_.spawn_new_requeued;
        if (spawn_requeue_logged_ < 8) {
            ++spawn_requeue_logged_;
            log_.notef("  spawn queue 0094c490: request serial %u made nothing on "
                "attempt %u, so it goes back on the list (009478B0) and is retried "
                "next interval", request.serial, request.attempts);
        }
        queue.requeue_009478b0(std::move(request));
        return;
    }
    ++summary_.spawn_new_fulfilled;
    complete_spawn_request_0094c777(request);
}

void GameMissionLuaHost::fulfil_spawn_request_009483d0(bsp::SpawnNewRequest& request) {
    // 00949300 creates nothing unless EVERY member's placement passes; 009483D0
    // then makes them all and sets record+C0h. The all-or-nothing rule is kept;
    // the placement test itself is not, because this process runs no occupancy
    // or exclusion test - see docs/LUA_SPAWN_NEW_HOST.md, "What is not tested".
    if (request.members.empty()) return;
    if (!request.has_ref_pos) return;

    std::vector<std::uint32_t> made;
    made.reserve(request.members.size());
    for (std::size_t i = 0; i < request.members.size(); ++i) {
        const bsp::SpawnNewGroupMember& member = request.members[i];
        if (member.type_class_id <= 0) break;
        const bsp::SpawnNewFrame frame = bsp::spawn_member_frame_0094a140(request, i);

        bsp::game::GameSceneEntityRecord record;
        // The script's own `Name` twice over in one mission ("Lexkiller 1" at
        // both line 1059 and line 1115), and the squadron registry is keyed by
        // name, so the request's serial disambiguates them. The script never
        // sees this string: it addresses the unit through the entity table the
        // callback hands it.
        char suffix[24];
        std::snprintf(suffix, sizeof(suffix), " #%u.%zu", request.serial, i + 1);
        record.name = (member.name.empty() ? std::string("SpawnNew") : member.name) + suffix;
        // DEVIATION, labelled. 009483D0 branches on the class's own
        // `vtable+18h(6)` kind test: kind 6 takes the class's `vtable+28h(0)`
        // constructor and everything else takes operator_new(0x414) plus
        // BSP_PlaneSquadronTickableEntity_Construct. This process cannot ask a
        // class its kind, so it takes the plane-squadron arm for every member.
        // All 28 group members in this installation's usn_19_coralus.lua are
        // aircraft (types 150/158/159/162), so no call site in the mission this
        // packet measures takes the other arm.
        record.class_name = "PlaneSquadronGen";
        record.class_id = 0x18;
        record.type_id = member.type_class_id;
        record.party = request.party;
        record.created = true;
        record.world[0] = 1.0f;
        record.world[5] = 1.0f;
        record.world[10] = 1.0f;
        record.world[12] = frame.position[0];
        record.world[13] = frame.position[1];
        record.world[14] = frame.position[2];
        apply_scene_yaw_00467050(record.world, frame.yaw);

        // `WingCount` has to reach 007F4580, and the seam reads it off the
        // spawn-pool entry for the same reason a held-back row does: the
        // property bag the key lives in does not exist here either.
        // docs/PLANE_SQUADRON_HOST.md.
        bsp::game::scene_spawn_pool().add(record);
        if (bsp::game::SceneSpawnPoolEntry* entry
                = bsp::game::scene_spawn_pool().find(record.name)) {
            entry->wing_count_present = member.wing_count > 0;
            entry->wing_count_raw = member.wing_count;
        }
        const std::uint32_t entity
            = script_orders_->create_unit_from_scene_record_0046db4b(record);
        if (bsp::game::SceneSpawnPoolEntry* entry
                = bsp::game::scene_spawn_pool().find(record.name)) {
            entry->spawned = entity != 0u;
            entry->entity_id = static_cast<int>(entity);
        }
        if (entity == 0u) break;
        if (!attach_created_entity_00928a00(static_cast<int>(entity), record.name,
                                            member.type_class_id)) {
            break;
        }
        made.push_back(entity);
        if (summary_.spawn_new_units + made.size() <= 16) {
            log_.notef("  spawn queue 0094c490: serial %u member %zu \"%s\" type %d "
                "WingCount %d party %d -> entity %u at (%.1f %.1f %.1f)",
                request.serial, i + 1, member.name.c_str(), member.type_class_id,
                member.wing_count, request.party, entity,
                static_cast<double>(frame.position[0]),
                static_cast<double>(frame.position[1]),
                static_cast<double>(frame.position[2]));
        }
    }
    if (made.size() != request.members.size()) {
        // 00949300's `bVar4 &= ...` gate: a group that cannot be placed whole is
        // not placed at all. What is already made cannot be unmade here, so the
        // partial result is reported rather than hidden.
        if (!made.empty()) {
            log_.notef("  spawn queue 0094c490: serial %u made %zu of %zu members before "
                "stopping; 00949300 admits a group only when every member passes, so "
                "this partial group is a defect in this process, not the native's rule",
                request.serial, made.size(), request.members.size());
        }
        summary_.spawn_new_units += made.size();
        request.created = std::move(made);
        return;
    }
    summary_.spawn_new_units += made.size();
    request.created = std::move(made);
    request.fulfilled = true;  // 009487B9 MOV byte ptr [ESI + 0xC0],1
}

void GameMissionLuaHost::complete_spawn_request_0094c777(
    const bsp::SpawnNewRequest& request) {
    // 0094C777 tests the record's completion function pointer and, when it is
    // set, calls it once per created entity. The Lua side of that dispatch was
    // NOT recovered: 00949750 pushes no code-address immediate and nothing in
    // the 0094A140 -> 00949300 -> 009483D0 chain calls a Lua helper, so how the
    // `callback` NativeString at record+84h reaches the interpreter is open.
    //
    // What IS settled, from the shipped script rather than the listing, is the
    // arity: the named global takes one argument per group member, in order.
    // `luaLexKillersSpawned(unit1,unit2,unit3,unit4)` at usn_19_coralus.lua:1166
    // answers a four-member request and uses all four; `luaBombersSpawnedLex`
    // at :3081 answers a one-member request at difficulty 0 and uses only
    // `unit1`, taking `unit2` on the branch whose request has two members. That
    // is a CONTRACT read off the consumer, and it is why this call is made here
    // instead of being left unimplemented.
    if (state_ == nullptr) return;
    if (request.callback.empty()) return;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, request.callback.c_str());
    if (!lua_isfunction(state_, -1)) {
        ++summary_.spawn_new_callback_missing;
        ::lua_settop(state_, top);
        log_.notef("  spawn queue 0094c490: serial %u created %zu unit(s) but the script "
            "defines no global \"%s\", so nothing was called",
            request.serial, request.created.size(), request.callback.c_str());
        return;
    }
    int pushed = 0;
    for (std::uint32_t entity : request.created) {
        if (!push_resolved_entity_by_id(state_, static_cast<int>(entity))) {
            lua_pushnil(state_);
        }
        ++pushed;
    }
    if (::lua_pcall(state_, pushed, 0, 0) != 0) {
        const char* message = lua_tolstring(state_, -1, nullptr);
        log_.notef("  spawn queue 0094c490: \"%s\" raised: %s",
            request.callback.c_str(), message != nullptr ? message : "(no message)");
        note_error(message != nullptr ? message : std::string("(no message)"));
        ::lua_settop(state_, top);
        return;
    }
    ++summary_.spawn_new_callbacks;
    ::lua_settop(state_, top);
    log_.notef("  spawn queue 0094c490: serial %u fulfilled, \"%s\"(%d unit table(s)) ran",
        request.serial, request.callback.c_str(), pushed);
}

void GameMissionLuaHost::report_spawn_queue() {
    if (summary_.spawn_new_calls == 0 && summary_.spawn_new_rejected == 0) return;
    log_.notef("summary SpawnNew 0094c480 calls=%llu rejected=%llu queued=%llu "
        "attempts=%llu fulfilled=%llu requeued=%llu units=%llu callbacks=%llu "
        "callback_missing=%llu still_queued=%zu interval=%.3f clock=%.1f",
        summary_.spawn_new_calls, summary_.spawn_new_rejected,
        summary_.spawn_new_queued, summary_.spawn_new_attempts,
        summary_.spawn_new_fulfilled, summary_.spawn_new_requeued,
        summary_.spawn_new_units, summary_.spawn_new_callbacks,
        summary_.spawn_new_callback_missing, bsp::spawn_request_queue().size(),
        static_cast<double>(spawn_attempt_delay_),
        static_cast<double>(spawn_world_clock_));
}

std::uint32_t GameMissionLuaHost::create_squadron(const bsp::AirOpsSquadronRequest& request) {
    // 006C74C6 calls 006C5050 and 006C74FF stores what comes back in slot+28h.
    // The unit itself is the script-orders host's to make, because that host owns
    // the units host; this adds the table slot the script indexes it by.
    if (script_orders_ == nullptr) return 0u;
    std::string name;
    std::int32_t wing = 0;
    const std::uint32_t entity = script_orders_->create_air_ops_squadron_006c5050(
        request.type, request.wing_count, request.equipment, request.home_base, name,
        wing);
    if (entity == 0u) return 0u;
    if (!attach_created_entity_00928a00(static_cast<int>(entity), name,
                                        static_cast<int>(request.type))) {
        log_.notef("  air ops squadron %s: unit %u exists but no `thisTable` slot was "
            "made, so `squadron` would name an id the bindings cannot resolve",
            name.c_str(), entity);
        return 0u;
    }
    ++summary_.air_ops_squadrons_created;
    return entity;
}

bool GameMissionLuaHost::attach_created_entity_00928a00(int entity_id,
    const std::string& name, int class_index) {
    if (state_ == nullptr || entity_id <= 0) return false;
    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (lua_isnil(state_, -1)) {
        ::lua_settop(state_, ::lua_gettop(state_) - 1);
        return false;
    }
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, entity_id);
    // The same four fields 00928A00 seeds, in the same order and with `ID` as the
    // key TEXT rather than a number; a number there is a different Lua key and
    // every shipped helper indexes `thisTable[Obj.ID]`.
    lua_createtable(state_, 0, 3);
    ::lua_pushstring(state_, key);
    lua_setfield(state_, -2, "ID");
    lua_pushboolean(state_, 0);
    lua_setfield(state_, -2, "Dead");
    lua_pushlightuserdata(state_,
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(entity_id)));
    lua_setfield(state_, -2, "Ptr");
    if (class_index >= 0) {
        lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
        if (lua_istable(state_, -1)) {
            lua_rawgeti(state_, -1, class_index);
            if (lua_istable(state_, -1)) {
                lua_setfield(state_, -3, "Class");
            } else {
                ::lua_settop(state_, ::lua_gettop(state_) - 1);
            }
        }
        ::lua_settop(state_, ::lua_gettop(state_) - 1);
    }
    lua_setfield(state_, -2, key);
    ::lua_settop(state_, ::lua_gettop(state_) - 1);
    if (!name.empty()) scene_entity_ids_[name] = entity_id;
    ++summary_.self_table_entities;
    return true;
}

void GameMissionLuaHost::note_objective_binding(const char* binding,
    const std::string& objective, unsigned int slot_mask, int units_touched) {
    ++summary_.objective_binding_calls;
    summary_.objective_units_touched += static_cast<unsigned long long>(units_touched);
    if (summary_.objective_binding_calls <= 24) {
        log_.notef("  objective binding %-22s name=\"%s\" slots=0x%02x units=%d",
            binding, objective.c_str(), slot_mask, units_touched);
    }
    log_.implemented("MissionLuaNative::Objectives", "008cd440");
}

namespace {
// 006C6681, 006C6695 and 006C690D compare the key with 00425850
// BSP_NativeString_EqualsCStringInsensitive, which delegates to the CRT
// stricmp, and the `planes` arm at 006C6667 calls 00BF7FBF __stricmp directly.
// Every key this reader matches is therefore matched without regard to case,
// which is why the shipped scripts' `NumSlots` and `Stock` reach the same arms
// as the binary's `numSlots` and `stock`.
bool get_property_key_is(const char* key, const char* name) noexcept {
    if (key == nullptr || name == nullptr) return false;
    for (;; ++key, ++name) {
        const unsigned char a = static_cast<unsigned char>(*key);
        const unsigned char b = static_cast<unsigned char>(*name);
        const unsigned char la = (a >= 'A' && a <= 'Z') ? static_cast<unsigned char>(a + 32) : a;
        const unsigned char lb = (b >= 'A' && b <= 'Z') ? static_cast<unsigned char>(b + 32) : b;
        if (la != lb) return false;
        if (la == 0) return true;
    }
}

// 006C6714 through 006C68A6 build one table per slot in this key order. The
// field offsets are the slot record's own, cross-checked against
// include/bsp/air_operations.hpp: classid slot+4h (006C6782), count slot+8h
// (006C67D2), equipment slot+10h (006C681F), squadron slot+28h (006C6895) and
// state slot+2Ch (006C672A). 006C68D9 advances the cursor by 58h.
void push_air_ops_slot_entry(lua_State* state, const bsp::AirOpsSlot& slot) {
    ::lua_createtable(state, 0, 5);
    ::lua_pushinteger(state, static_cast<lua_Integer>(slot.state));
    ::lua_setfield(state, -2, "state");
    ::lua_pushinteger(state, static_cast<lua_Integer>(slot.vehicle_class));
    ::lua_setfield(state, -2, "classid");
    ::lua_pushinteger(state, static_cast<lua_Integer>(slot.assigned_count));
    ::lua_setfield(state, -2, "count");
    ::lua_pushinteger(state, static_cast<lua_Integer>(slot.class_field_134));
    ::lua_setfield(state, -2, "equipment");
    // 006C6895 pushes the launched squadron, and the whole point of the key for
    // luaGetSlotsAndSquads is that it is nil until a launch fills slot+28h. An
    // unlaunched slot therefore carries no `squadron` field.
    //
    // RETRACTED, packet cc8_usn04_strike_class. Packet cc8_airops_launch_tick
    // changed this to push the entity's `thisTable` slot, reasoning that the
    // script hands `slot.squadron` straight to `PilotSetTarget`, which needs a
    // table. **The script does no such thing.** All four readers in
    // usn_19_coralus.lua go through `thisTable` themselves:
    //
    //   :545  local launchedWildcat = thisTable[tostring(GetProperty(Mission.Lex,
    //                                   "slots")[slotIndex].squadron)]
    //   :560, :1394, :1409 have the same shape, and :1411 is
    //         PilotSetTarget(launchedStriker, bombertrg) on the RESULT of :1409
    //
    // `kMissionLuaEntityKeyFormat` is "%d", so a `thisTable` key is the entity id
    // as text and `tostring` of the number is exactly that key. Pushing a table
    // here makes `tostring` yield "table: 0x...", so every one of those lookups
    // returns nil and the mission then orders nil. The original integer was
    // right and the reasoning that replaced it was wrong.
    // docs/USN04_STRIKE_CLASS.md.
    if (slot.launched_squadron != 0u) {
        ::lua_pushinteger(state, static_cast<lua_Integer>(slot.launched_squadron));
        ::lua_setfield(state, -2, "squadron");
    }
}
} // namespace

int GameMissionLuaHost::run_get_property_0088bf80(lua_State* state, int argument_count) {
    ++summary_.get_property_calls;
    if (state == nullptr) return 0;
    // 0088C09D reads argument 0 and hands it to 00888AA0
    // BSP_ObjectHandle_FromLuaTable; 0088C0A1 reads argument 1 and 0088C0B1
    // takes its string. 0088C0C0 then loads the entity's vtable and calls the
    // reader at +138h with (frame, key). This body stands in for that reader,
    // and like the native it pushes nothing of its own.
    const char* key = (argument_count >= 2 && ::lua_type(state, 2) == LUA_TSTRING)
        ? ::lua_tolstring(state, 2, nullptr) : nullptr;
    // 00D112FC "luaMW_GetProperty failed:" is built at 0088BFB3 and released at
    // 0088BFDA on every call, with no arm that reports it: the shipped build
    // constructs the message and throws it away. The marker is kept here
    // because it is the only name the native carries for the unanswered key.
    if (key == nullptr) {
        ++summary_.get_property_unserved;
        log_.notef("  GetProperty 0088bf80: no string key (argc=%d), "
            "luaMW_GetProperty failed:", argument_count);
        return 0;
    }

    // The air-operations reader 006C6630 sits at vtable+138h for both classes
    // that own a deck: 00D01768 for the mother ship (00758340 -> 00815870 then
    // 006C6630) and 00CF8D40 for the airfield (006D0E60). It answers exactly
    // four keys and pushes nothing for any other, which leaves the caller with
    // nil. docs/MISSION_LUA_GETPROPERTY.md.
    const bool wants_slots = get_property_key_is(key, "slots");
    const bool wants_num_slots = get_property_key_is(key, "numSlots");
    const bool wants_stock = get_property_key_is(key, "stock") || get_property_key_is(key, "planes");
    if (!wants_slots && !wants_num_slots && !wants_stock) {
        ++summary_.get_property_unserved;
        if (summary_.get_property_unserved <= 12) {
            log_.notef("  GetProperty 0088bf80: key \"%s\" reaches no reconstructed reader "
                "(luaMW_GetProperty failed:), so the call returns no value, which is what "
                "006C6B26 does for an unmatched key", key);
        }
        return 0;
    }

    // The deck now comes from the scene: 006CADD0 mode 1 builds it when a
    // MotherShipGen or AirField row loads, and it is keyed by the authored unit
    // name with the entity id bound in attach_scene_entities_00928a00.
    // docs/AIROPS_LOAD_FROM_SCENE.md. A unit with no deck, which is every class
    // that owns none, still reaches the four keys here because this host cannot
    // pick the reader by class; it answers with an empty deck rather than with
    // the nothing the native pushes. That deviation is unchanged.
    const bsp::AirOpsDeck* deck = bsp::air_ops_decks().find_by_entity_id(
        air_ops_entity_id(state));
    const bsp::AirOpsSlot* slots = deck != nullptr && !deck->slots.empty()
        ? deck->slots.data() : nullptr;
    int slot_count = deck != nullptr ? static_cast<int>(deck->slots.size()) : 0;

    if (wants_num_slots) {
        // 006C6929 loads the live slot count at block+50h and 006C693B pushes it
        // with 00B664B0 BSP_LuaObject_PushInteger.
        ++summary_.get_property_served;
        ::lua_pushinteger(state, static_cast<lua_Integer>(slot_count));
        log_.implemented("MissionLuaNative::GetProperty", "0088bf80");
        return 1;
    }
    if (wants_slots) {
        // 006C66ED seeds the Lua index at 1 and 006C68D3 advances it, so the
        // array the script indexes with LaunchSquadron's return is 1-based.
        ++summary_.get_property_served;
        ::lua_createtable(state, slot_count, 0);
        for (int index = 0; index < slot_count; ++index) {
            push_air_ops_slot_entry(state, slots[index]);
            ::lua_rawseti(state, -2, index + 1);
            ++summary_.get_property_slots_rows;
            if (slots[index].launched_squadron != 0u) {
                ++summary_.air_ops_squadron_key_pushes;
            }
        }
        log_.implemented("MissionLuaNative::GetProperty", "0088bf80");
        return 1;
    }
    // 006C6949 serves `stock` and `planes` from the same list, one entry per
    // stock record with `classid` and `count` (006C6A0F and 006C6A93). The
    // records come from the scene's `PlaneStock %d` blocks.
    ++summary_.get_property_served;
    const int stock_count = deck != nullptr ? static_cast<int>(deck->stock.size()) : 0;
    ::lua_createtable(state, stock_count, 0);
    for (int index = 0; index < stock_count; ++index) {
        const bsp::AirOpsStockEntry& entry = deck->stock[static_cast<std::size_t>(index)];
        ::lua_createtable(state, 0, 2);
        ::lua_pushinteger(state, static_cast<lua_Integer>(entry.vehicle_class));
        ::lua_setfield(state, -2, "classid");
        ::lua_pushinteger(state, static_cast<lua_Integer>(entry.count));
        ::lua_setfield(state, -2, "count");
        ::lua_rawseti(state, -2, index + 1);
    }
    log_.implemented("MissionLuaNative::GetProperty", "0088bf80");
    return 1;
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

float GameMissionLuaHost::read_vehicle_class_number(int index, const char* key,
    float fallback) {
    // The same plain `VehicleClass[index][key]` lookup as
    // read_vehicle_class_integer, kept as a number. 009623A9's caller squares
    // the result; this reader does not, because the Lua field is the modifier
    // and class+B8h is its square.
    if (state_ == nullptr || index < 0 || key == nullptr) return fallback;
    const int top = ::lua_gettop(state_);
    float value = fallback;
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_pushinteger(state_, index);
        ::lua_gettable(state_, -2);
        if (::lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, key);
            if (::lua_type(state_, -1) == LUA_TNUMBER) {
                value = static_cast<float>(::lua_tonumber(state_, -1));
            }
        }
    }
    ::lua_settop(state_, top);
    return value;
}

float GameMissionLuaHost::read_bullet_class_number(int index, const char* key,
    float fallback) {
    // Scripts/datatables/autoload/bulletclasses.lua publishes the selected
    // variant as the global `Bullets`, indexed by bullet class id: it DoFiles
    // classtables/<arcade|realistic>/bulletclasses.lua and assigns
    // `Bullets = ArcadeTable` or `= RealisticTable` on the GameMode switch.
    // Rows carry string keys, among them "Range", "FlyTime" and
    // "WaterTravelSpeed". docs/TORPEDO_CATEGORY_ADMISSION.md.
    if (state_ == nullptr || index < 0 || key == nullptr) return fallback;
    const int top = ::lua_gettop(state_);
    float value = fallback;
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "Bullets");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_pushinteger(state_, index);
        ::lua_gettable(state_, -2);
        if (::lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, key);
            if (::lua_type(state_, -1) == LUA_TNUMBER) {
                value = static_cast<float>(::lua_tonumber(state_, -1));
            }
        }
    }
    ::lua_settop(state_, top);
    return value;
}

std::string GameMissionLuaHost::read_bullet_class_string(int index, const char* key) {
    // The string companion of read_bullet_class_number. `Type` is what
    // 006EA910's case-insensitive name chain switches on to pick a weapon
    // class, and bsp::weapon_class_sub_type_for_lua_type maps it to the
    // constructor sub-type that 006E9890 derives the engagement range from.
    // docs/BULLET_ENGAGEMENT_RANGE.md.
    std::string value;
    if (state_ == nullptr || index < 0 || key == nullptr) return value;
    const int top = ::lua_gettop(state_);
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "Bullets");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_pushinteger(state_, index);
        ::lua_gettable(state_, -2);
        if (::lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, key);
            if (::lua_type(state_, -1) == LUA_TSTRING) {
                const char* text = lua_tolstring(state_, -1, nullptr);
                if (text != nullptr) value = text;
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
        // The decks the scene pass built are keyed by the authored unit name.
        // This is the one place that holds the name and the id the entity table
        // will carry as `ID`, so it is where the two are tied together.
        // docs/AIROPS_LOAD_FROM_SCENE.md.
        bsp::air_ops_decks().bind_entity_id(entity.id, entity.name);
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
    // Was `unimplemented`, which the header defines as "the caller receives a
    // neutral value". That is not what happens here: the loop above builds the
    // slot and all four of its fields, `00928A00` is `coverage: complete` in
    // docs/MISSION_ENTITY_LUA_ATTACH.md, and `self_table_entities` counts real
    // work. The wrong marker had a cost - it was read as "no Lua order can name
    // a unit", which is false and was reported as a blocker.
    log_.implemented("MissionLua::entity_lua_attach", "00928a00");
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
    // Packet cc8_spawn_new_route: the mission frame already calls this at the
    // end of a run, so the spawn-queue summary rides with it rather than asking
    // for a second call site in a file this packet does not own.
    report_spawn_queue();
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
    // 0088BF80's own line. `slots_rows` counts the slot tables the reader built,
    // which stays at zero for as long as this process holds no air-operations
    // block. docs/MISSION_LUA_GETPROPERTY.md.
    log_.notef("summary mission getproperty 0088bf80: calls=%llu served=%llu unserved=%llu "
        "slots_rows=%llu decks=%zu", summary_.get_property_calls, summary_.get_property_served,
        summary_.get_property_unserved, summary_.get_property_slots_rows,
        bsp::air_ops_decks().size());
    log_.notef("summary mission airops gates: ready_calls=%llu ready_true=%llu "
        "launch_calls=%llu started=%llu queued=%llu (00895d20, 0089e3c0)",
        summary_.air_ops_ready_calls, summary_.air_ops_ready_true,
        summary_.air_ops_launch_calls, summary_.air_ops_launch_started,
        summary_.air_ops_launch_queued);
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
