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
#include "bsp/game_hosts_mission_frame.hpp"

#include "bsp/air_operations.hpp"
#include "bsp/game_hosts_ai.hpp"
#include "bsp/bullet_engagement_range.hpp"  // packet cc9_ai_plane_loadout_arm
#include "bsp/objective_units.hpp"
#include "bsp/plane_squadron_host.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/game_hosts_ship_ai.hpp"  // packet cc9_unit_get_attack_target
#include "bsp/entity_orders.hpp"     // packet cc9_unit_get_attack_target
#include "bsp/game_hosts_gunnery.hpp"  // packet cc9_get_property_class_readers
#include "bsp/recon_sensor_pass.hpp"   // packet cc9_get_property_class_readers
#include "bsp/game_hosts_hud.hpp"
#include "bsp/hud_movie_camera.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/global_script_folders.hpp"
#include "bsp/lua_binding_mission_2.hpp"
#include "bsp/lua_spawn_new.hpp"
#include "bsp/mission_lobby_settings.hpp"
#include "bsp/mission_lua_bindings.hpp"
#include "bsp/mission_lua_machine.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/ship_ai_path_turn_ramp.hpp"
#include "bsp/ship_ai_settings_block.hpp"
#include "bsp/game_ship_avoidance_tuning_lua.hpp"
#include "bsp/gameplay_settings.hpp"
#include "bsp/mission_camera.hpp"
#include "bsp/ship_class_fields.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/lua_numeric.hpp"
#include "bsp/vehicle_class_lua_load.hpp"
#include "bsp/vehicle_class.hpp"
#include "bsp/lua_binding_navigator.hpp"
#include "bsp/native_string.hpp"
#include "bsp/recon_values.hpp"
#include "bsp/recon_slot_lists.hpp"  // packet cc9_recon_publication
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
#include <cstdlib>
#include <cstring>
#include <functional>
#include <optional>
#include <type_traits>

namespace bsp::game {
namespace {
// Packet cc9_spawn_new_id_queries: what the two id bindings answered.
struct SpawnIdQueryCensus {
    unsigned long long requested{0};
    unsigned long long answered_true{0};
    unsigned long long removes{0};
    unsigned long long removed{0};
};
SpawnIdQueryCensus g_spawn_id_census;

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
//             arg5.. the targets (arg6.. when arg5 is a boolean), walked
//             without an immediate index; see kObjectiveAddTargetWalkBound
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

// Packet cc9_objective_kind. 008DBF40: the case-insensitive index of the
// string in the six-entry table 00E0C948, or 6 when none matches (008DBFC5).
int objective_kind_008dbf40(const std::string& text) {
    static const char* const kKinds[] = {"primary", "secondary", "hidden",
        "marker1", "marker2", "marker3"};
    for (int i = 0; i < 6; ++i) {
        if (_stricmp(text.c_str(), kKinds[i]) == 0) return i;
    }
    return 6;
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

// Packet cc9_objective_add_targets, docs/SHIP_AI_OPEN_ITEMS.md section 118.3.
// True: 008CD440's target walk starts at argument 5 (008CD753 MOV ESI,5) and
// at 6 only when argument 5 is a boolean (008CD7C6 CMP EAX,5 / JLE, 008CD7ED
// IsBoolean, 008CD841 MOV ESI,6); a target that is neither an entity handle
// (008889C0) nor a vector3 table (0088B840) is a table whose elements are
// walked (008CD96F.., IterateFirst/IterateNext), as in 008CDD60 and 008CE510.
// False: Add's walk starts at 6 and a list table adds nothing. ON: section 120.4
// (USNOS, USNOS long, JM06, JM08 gameplay-identical; LOMP06 control).
inline constexpr bool kObjectiveAddTargetWalkBound = true;

// The units argument `index` names: an entity table itself, or, while
// kObjectiveAddTargetWalkBound, every entity-table element of a list table.
// Positions (vector3 tables) carry no ID and add no unit, as before.
void objective_argument_units(lua_State* state, int index,
                              std::vector<std::size_t>& out) {
    std::size_t unit = 0;
    if (objective_argument_unit(state, index, unit)) {
        out.push_back(unit);
        return;
    }
    if (!kObjectiveAddTargetWalkBound) return;
    const int slot = index + 1;
    const int top = lua_gettop(state);
    if (slot > top || lua_type(state, slot) != LUA_TTABLE) return;
    lua_pushnil(state);
    while (lua_next(state, slot) != 0) {
        if (lua_type(state, -1) == LUA_TTABLE) {
            lua_getfield(state, -1, "ID");
            const int type = lua_type(state, -1);
            if (type == LUA_TNUMBER || type == LUA_TSTRING) {
                const int id = static_cast<int>(lua_tonumber(state, -1));
                if (id > 0) out.push_back(static_cast<std::size_t>(id - 1));
            }
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
    }
    lua_settop(state, top);
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
    // Without player records only slot 0 is active here (008CDF58's two bytes:
    // in single player slot 0 has +8h set and +9h clear, slots 1..7 +9h set,
    // SHIP_AI 60.6). Section 125: 008CDEF2 keeps an active slot only when its
    // +28h equals the party argument, so another party's objective lands in no
    // slot. Before kLocalPartyFromSceneBound slot 0 takes every party.
    if (bsp::game::kLocalPartyFromSceneBound && have_party) {
        return party == bsp::game::scene_slot_party(0, 0) ? 1u : 0u;
    }
    (void)party;
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
// Packet cc9_wing_construction_lua: the Lua half acts only with the units half.
constexpr bool kWingConstructionLuaActive =
    kWingConstructionLuaBound && kWingConstructionInPassABound;

// Packet cc9_carrier_launch_skill (docs/SCENE_UNIT_SKILL.md section 6): an
// air-ops squadron starts at its owner's live skill, the bag `Skill` 006C5050
// reads through owner->vtable[12Ch](). ON: JM05 9200/9000 moves (14 US
// squadrons at 2), USN04 and USN13 are gameplay-identical.
constexpr bool kCarrierLaunchSkillBound = true;
// Packet cc9_lua_kill_script_entity, docs/SHIP_AI_OPEN_ITEMS.md section 57.
// True: Kill (008AC5C0) on a script entity (a CreateScript record, id from
// 100000) runs 00926D90 through GameScriptOrdersHost::entity_kill_00926d90.
// Every miss on the reference rows is luaCamOnTargetExt's
// Kill(Mission.CamScript), the luaDelay timetable calling it. False: the
// entity is not killed and the call counts as unresolved, as before.
// ON: pairs held, six rows gameplay identical (section 57.1).
constexpr bool kLuaKillScriptEntityBound = true;

// Packet cc9_find_entity_case (docs/CONTROLLED_UNIT.md, "FindEntity matches
// names case-insensitively"). 00925A90 hands each registry entry to 009251F0,
// whose first test (0092521E) is 00438E10, a null-guarded CRT _stricmp
// (00BF7FBF) over the whole name. True answers an exact miss with the first
// entry that compares equal ignoring ASCII case; false keeps the exact map.
// ON: IJN01 resolves "Airfield 02" -> "AirField 02" (entity_resolves 78 -> 79)
// and is gameplay-identical while PilotLand stays a record; JM05 identical.
constexpr bool kFindEntityCaseInsensitiveBound = true;

// Packet cc9_movie_camera_mover_bind: the MovCamNew_AddPosition table as
// 007A0EB0 reads it (docs/HUD_PICK_SEGMENT_QUERY.md 8.6). Keys the parser
// reads but the host does not model are listed in `unsupported_keys` when
// present and non-nil.
bool movie_number(lua_State* state, int table, const char* key, std::optional<float>& out) {
    lua_getfield(state, table, key);
    const bool is = lua_type(state, -1) == LUA_TNUMBER;
    if (is) out = static_cast<float>(lua_tonumber(state, -1));
    lua_pop(state, 1);
    return is;
}
bool movie_vector(lua_State* state, int table, std::array<float, 3>& out) {
    // 0079C040: three string keys among x/y/z read by name (0078FD70), else
    // the array part 1..3.
    lua_getfield(state, table, "x");
    const bool named = lua_type(state, -1) == LUA_TNUMBER;
    lua_pop(state, 1);
    const char* names[3] = {"x", "y", "z"};
    for (int i = 0; i < 3; ++i) {
        if (named) {
            lua_getfield(state, table, names[i]);
        } else {
            lua_rawgeti(state, table, i + 1);
        }
        out[static_cast<std::size_t>(i)] = static_cast<float>(lua_tonumber(state, -1));
        lua_pop(state, 1);
    }
    return true;
}
void parse_movie_keyframe(lua_State* state, int table, bsp::MovieKeyframeInput& in) {
    const int top = lua_gettop(state);
    lua_getfield(state, table, "postype");
    if (lua_type(state, -1) == LUA_TSTRING) in.postype = lua_tostring(state, -1);
    lua_pop(state, 1);
    lua_getfield(state, table, "transformtype");
    if (lua_type(state, -1) == LUA_TSTRING) in.transformtype = std::string(lua_tostring(state, -1));
    lua_pop(state, 1);
    lua_getfield(state, table, "wanderer");
    if (lua_type(state, -1) == LUA_TBOOLEAN) in.wanderer = lua_toboolean(state, -1) != 0;
    lua_pop(state, 1);
    lua_getfield(state, table, "position");
    if (lua_type(state, -1) == LUA_TTABLE) {
        in.has_position = true;
        const int position = lua_gettop(state);
        lua_getfield(state, position, "parent");
        if (lua_type(state, -1) == LUA_TTABLE) {
            in.has_parent = true;
            lua_getfield(state, -1, "ID");
            const int type = lua_type(state, -1);
            if (type == LUA_TNUMBER || type == LUA_TSTRING) {
                const int id = static_cast<int>(lua_tonumber(state, -1));
                if (id > 0) in.parent = static_cast<std::size_t>(id - 1);
            }
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
        lua_getfield(state, position, "parentID");
        if (!lua_isnil(state, -1)) in.unsupported_keys.push_back("parentID");
        lua_pop(state, 1);
        lua_getfield(state, position, "pos");
        if (lua_type(state, -1) == LUA_TTABLE) {
            std::array<float, 3> v{};
            movie_vector(state, lua_gettop(state), v);
            in.pos = v;
        }
        lua_pop(state, 1);
        lua_getfield(state, position, "polar");
        if (lua_type(state, -1) == LUA_TTABLE) {
            std::array<float, 3> v{};
            for (int i = 0; i < 3; ++i) {
                lua_rawgeti(state, -1, i + 1);
                v[static_cast<std::size_t>(i)] = static_cast<float>(lua_tonumber(state, -1));
                lua_pop(state, 1);
            }
            in.polar = v;
        }
        lua_pop(state, 1);
        for (const char* key : {"deckpos", "upvector", "modifier", "relativetotarget"}) {
            lua_getfield(state, position, key);
            if (!lua_isnil(state, -1)) in.unsupported_keys.push_back(key);
            lua_pop(state, 1);
        }
        // Packet cc9_movie_camera_keys, switch 6: 007A1470 IsBoolean, then
        // 007A1488 stores it at +D9h; a non-boolean value is ignored.
        lua_getfield(state, position, "terrainavoid");
        if constexpr (kMovieTerrainAvoidBound) {
            if (lua_type(state, -1) == LUA_TBOOLEAN) in.terrainavoid = lua_toboolean(state, -1) != 0;
        } else {
            if (!lua_isnil(state, -1)) in.unsupported_keys.push_back("terrainavoid");
        }
        lua_pop(state, 1);
    }
    lua_pop(state, 1);
    movie_number(state, table, "starttime", in.starttime);
    movie_number(state, table, "blendtime", in.blendtime);
    movie_number(state, table, "linearblend", in.linearblend);
    movie_number(state, table, "nonlinearblend", in.nonlinearblend);
    movie_number(state, table, "zoom", in.zoom);
    movie_number(state, table, "smoothtime", in.smoothtime);
    for (const char* key : {"event", "finishscript", "flyalt"}) {
        lua_getfield(state, table, key);
        if (!lua_isnil(state, -1)) in.unsupported_keys.push_back(key);
        lua_pop(state, 1);
    }
    lua_settop(state, top);
}

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
    const bool objective_status_row = kObjectiveStatusBound
        && (dispatch_row.address == 0x008bd340u || dispatch_row.address == 0x008bd900u);
    const bool objective_row = dispatch_row.address == 0x008cd440u
        || dispatch_row.address == 0x008cdd60u || dispatch_row.address == 0x008ce510u
        || objective_status_row;
    const bool get_property_row = dispatch_row.address == 0x0088bf80u;
    // Packet cc9_lua_kill.
    const bool kill_row = kLuaKillBound && dispatch_row.address == 0x008ac5c0u;
    // Packet cc9_lua_listeners.
    const bool add_listener_row = kLuaListenersBound && dispatch_row.address == 0x008c6760u;
    const bool remove_listener_row = kLuaListenersBound && dispatch_row.address == 0x008c6990u;
    const bool listener_active_row = kLuaListenersBound && dispatch_row.address == 0x008c6bb0u;
    // Packet cc9_set_invincible_native.
    const bool set_invincible_row = dispatch_row.address == 0x00897a50u;
    // Packet cc9_override_hp.
    const bool override_hp_row = dispatch_row.address == 0x008c1930u;
    const bool forced_recon_row = kForcedReconLevelBound && dispatch_row.address == 0x008aa8f0u;
    const bool add_damage_row = kLuaAddDamageBound && dispatch_row.address == 0x0088e000u;
    const bool aa_enable_row = kLuaAAEnableBound && dispatch_row.address == 0x0089c740u;
    const bool ship_speed_row = kLuaSetShipSpeedBound && dispatch_row.address == 0x00890d30u;
    // Packet cc9_unit_get_attack_target.
    const bool attack_target_row = kLuaUnitGetAttackTargetBound
        && dispatch_row.address == 0x008a6de0u;
    // Packet cc9_squadron_set_speed.
    const bool squadron_speed_row = kLuaSquadronSetSpeedBound
        && dispatch_row.address == 0x0089f780u;
    // Packet cc9_is_class_changed.
    const bool class_changed_row = kLuaIsClassChangedBound
        && dispatch_row.address == 0x008cc4b0u;
    // Packet cc9_set_submarine_depth_level.
    const bool sub_depth_row = kLuaSetSubmarineDepthLevelBound
        && dispatch_row.address == 0x00893f40u;
    // Packet cc9_set_air_base_slot_count.
    const bool slot_count_row = kLuaSetAirBaseSlotCountBound
        && dispatch_row.address == 0x008963e0u;
    // Packet cc9_device_reload_enabled.
    const bool device_reload_row = kLuaDeviceReloadEnabledBound
        && dispatch_row.address == 0x008c1350u;
    // Packet cc9_squadron_travel_alt.
    const bool travel_alt_row = kSquadronTravelAltBound
        && dispatch_row.address == 0x0089f550u;
    // Packet cc9_squadron_attack_alt.
    const bool attack_alt_row = kSquadronAttackAltBound
        && dispatch_row.address == 0x008a22b0u;
    // Packet cc9_get_closest_border_zone.
    const bool border_zone_row = kLuaClosestBorderZoneBound
        && dispatch_row.address == 0x008aecd0u;
    // Packet cc9_lua_formation_query.
    const bool in_formation_row = kLuaFormationQueryBound
        && dispatch_row.address == 0x008996a0u;
    const bool leave_formation_row = kLuaFormationQueryBound
        && dispatch_row.address == 0x00899eb0u;
    // Packet cc9_get_formation_leader.
    const bool formation_leader_row = kLuaFormationLeaderBound
        && dispatch_row.address == 0x00899af0u;
    // Packet cc9_get_last_catapulted.
    const bool last_catapulted_row = kLuaLastCatapultedBound
        && dispatch_row.address == 0x00892860u;
    // Packet cc9_add_untouchable_unit.
    const bool untouchable_row = kLuaAddUntouchableUnitBound
        && dispatch_row.address == 0x008ac140u;
    // Packet cc9_submarine_air.
    const bool unlimited_air_row = kSubmarineAirBound
        && dispatch_row.address == 0x00893c00u;
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
    // Packet cc9_spawn_new_id_queries.
    const bool spawn_id_requested_row = kLuaSpawnNewIdQueriesBound
        && dispatch_row.address == 0x00946380u;
    const bool spawn_id_remove_row = kLuaSpawnNewIdQueriesBound
        && dispatch_row.address == 0x00946390u;
    // Packet cc9_bot_scheduler_writers: Scoring_RealPlayTimeRunning (008B87F0),
    // argument 0 as a boolean (008B88EF) into 00905340 on [game+21A0h].
    const bool scoring_play_time_row
        = kBotSchedulerWritersBound && dispatch_row.address == 0x008b87f0u;
    // Packet cc9_set_selected_unit: SetSelectedUnit 008AB260 hands argument 0
    // to 00647300 on the HUD root (docs/CONTROLLED_UNIT.md). Switch in
    // bsp/game_hosts_hud.hpp, committed OFF.
    const bool select_unit_row
        = bsp::game::kSetSelectedUnitBound && dispatch_row.address == 0x008ab260u;
    const bool movie_add_row
        = bsp::game::kMovieMoverBound && dispatch_row.address == 0x008b79f0u;
    // Packet cc9_force_select_unit: ForceSelectUnit 008AAF30 takes no argument
    // and calls 006485A0 on [00E198C4]+40h (008AB014..008AB01F).
    const bool force_select_row
        = bsp::game::kForceSelectUnitBound && dispatch_row.address == 0x008aaf30u;
    const bool handled = avoidance_setting || objective_row || get_property_row || kill_row
        || add_listener_row || remove_listener_row || listener_active_row || set_invincible_row
        || forced_recon_row || add_damage_row || aa_enable_row || ship_speed_row
        || override_hp_row
        || attack_target_row || squadron_speed_row || class_changed_row || sub_depth_row
        || slot_count_row || device_reload_row || unlimited_air_row
        || in_formation_row || leave_formation_row || travel_alt_row || border_zone_row
        || untouchable_row
        || attack_alt_row
        || ready_row
        || launch_row || generate_row || spawn_new_row || scoring_play_time_row
        || spawn_id_requested_row || spawn_id_remove_row
        || select_unit_row || movie_add_row || force_select_row
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
        // Packet cc9_objectives_completed: 008BD340 / 008BD900.
        if (objective_status_row) {
            const std::string key = objective_argument_string(state, 2);
            const int status = dispatch_row.address == 0x008bd340u ? 1 : 2;
            int matched = 0;
            for (int k = 0; k < static_cast<int>(bsp::game::GameObjectiveSets::kSlotCount); ++k) {
                if ((mask & (1u << k)) == 0u) continue;
                if (sets.set_status(k, key, status)) ++matched;
            }
            static_cast<void>(matched);
            host->note_objective_binding(dispatch_row.name, key, mask, 0);
            return 0;
        }
        const bool is_remove = dispatch_row.address == 0x008ce510u;
        // 008CD65A for Add and 008CDFFA (kObjectiveNameArgument) for AddUnit
        // both read argument 2 as the objective name.
        const std::string name = objective_argument_string(state, 2);
        // 008CD440's targets begin after its three strings and its boolean;
        // 008CDD60's at kObjectiveFirstTargetArgument.
        int first_target = is_add ? 6 : bsp::kObjectiveFirstTargetArgument;
        if (kObjectiveAddTargetWalkBound && is_add) {
            // 008CD753 / 008CD841: 5, or 6 after a boolean argument 5.
            first_target = argc > 5 && lua_type(state, 6) == LUA_TBOOLEAN ? 6 : 5;
        }
        int units_touched = 0;
        for (int k = 0; k < static_cast<int>(bsp::game::GameObjectiveSets::kSlotCount); ++k) {
            if ((mask & (1u << k)) == 0u) continue;
            if (is_add) {
                if (bsp::game::kObjectiveKindBound) {
                    sets.add_objective(k, name, objective_kind_008dbf40(
                        objective_argument_string(state, 4)));
                } else {
                    sets.add_objective(k, name);
                }
            }
            for (int arg = first_target; arg < argc; ++arg) {
                std::vector<std::size_t> units;
                objective_argument_units(state, arg, units);
                for (const std::size_t unit : units) {
                    const bool moved = is_remove ? sets.remove_unit(k, name, unit)
                                                 : sets.add_unit(k, name, unit);
                    if (moved) ++units_touched;
                }
            }
        }
        host->note_objective_binding(dispatch_row.name, name, mask, units_touched);
        return 0;
    }
    if (get_property_row && !host->error_replay()) {
        return host->run_get_property_0088bf80(state, argc);
    }
    if (unlimited_air_row) {
        if (!host->error_replay()) host->run_set_unlimited_air_00893c00(state, argc);
        return 0;
    }
    if (slot_count_row) {
        if (!host->error_replay()) host->run_set_air_base_slot_count_008963e0(state, argc);
        return 0;
    }
    if (device_reload_row) {
        if (!host->error_replay()) host->run_set_device_reload_enabled_008c1350(state, argc);
        return 0;
    }
    if (attack_alt_row) {
        if (!host->error_replay()) host->run_squadron_set_attack_alt_008a22b0(state, argc);
        return 0;
    }
    if (travel_alt_row) {
        if (!host->error_replay()) host->run_squadron_set_travel_alt_0089f550(state, argc);
        return 0;
    }
    if (in_formation_row && !host->error_replay()) {
        return host->run_is_in_formation_008996a0(state, argc);
    }
    if (formation_leader_row && !host->error_replay()) {
        return host->run_get_formation_leader_00899af0(state, argc);
    }
    if (last_catapulted_row && !host->error_replay()) {
        return host->run_get_last_catapulted_00892860(state, argc);
    }
    if (untouchable_row) {
        if (!host->error_replay()) host->run_add_untouchable_unit_008ac140(state, argc);
        return 0;
    }
    if (border_zone_row && !host->error_replay()) {
        return host->run_get_closest_border_zone_008aecd0(state, argc);
    }
    if (leave_formation_row) {
        if (!host->error_replay()) host->run_leave_formation_00899eb0(state, argc);
        return 0;
    }
    if (sub_depth_row) {
        if (!host->error_replay()) host->run_set_submarine_depth_level_00893f40(state, argc);
        return 0;
    }
    if (class_changed_row && !host->error_replay()) {
        return host->run_is_class_changed_008cc4b0(state, argc);
    }
    if (squadron_speed_row) {
        if (!host->error_replay()) host->run_squadron_set_speed_0089f780(state, argc);
        return 0;
    }
    if (attack_target_row && !host->error_replay()) {
        return host->run_unit_get_attack_target_008a6de0(state, argc);
    }
    if (ship_speed_row) {
        if (!host->error_replay()) host->run_set_ship_speed_00890d30(state, argc);
        return 0;
    }
    if (aa_enable_row) {
        if (!host->error_replay()) host->run_aa_enable_0089c740(state, argc);
        return 0;
    }
    if (add_damage_row) {
        if (!host->error_replay()) host->run_add_damage_0088e000(state, argc);
        return 0;
    }
    if (forced_recon_row) {
        if (!host->error_replay()) host->run_set_forced_recon_level_008aa8f0(state, argc);
        return 0;
    }
    if (override_hp_row) {
        if (!host->error_replay()) host->run_override_hp_008c1930(state, argc);
        return 0;
    }
    if (set_invincible_row) {
        if (!host->error_replay()) host->run_set_invincible_00897a50(state, argc);
        return 0;
    }
    if (add_listener_row) {
        if (!host->error_replay()) host->run_add_listener_008c6760(state, argc);
        return 0;
    }
    if (remove_listener_row) {
        if (!host->error_replay()) host->run_remove_listener_008c6990(state, argc);
        return 0;
    }
    if (listener_active_row) {
        if (host->error_replay()) return 0;
        return host->run_is_listener_active_008c6bb0(state, argc);
    }
    if (kill_row) {
        if (!host->error_replay()) host->run_kill_008ac5c0(state, argc);
        return 0;
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
    if (spawn_id_requested_row || spawn_id_remove_row) {
        // 009458CF..00945923 (and 00945AD3..00945AEC): argument 0 of the
        // binding (stack slot 1) through 00B662B0 into a NativeString. A value
        // Lua cannot convert to a string gives the empty string, which matches
        // a record whose id is empty.
        std::string id;
        if (argc >= 1 && ::lua_isstring(state, 1)) {
            std::size_t length = 0;
            const char* text = lua_tolstring(state, 1, &length);
            if (text != nullptr) id.assign(text, length);
        }
        if (spawn_id_requested_row) {
            // 00945943..00945999 the scan, 009459A8 00B66450 pushes the boolean,
            // 009459B1 00B66400: one result.
            const bool requested = bsp::spawn_request_queue().id_is_requested_00945850(id);
            if (!host->error_replay()) {
                ++g_spawn_id_census.requested;
                if (requested) ++g_spawn_id_census.answered_true;
            }
            ::lua_pushboolean(state, requested ? 1 : 0);
            return 1;
        }
        // 00945B07..00945BAE: every match is unlinked and freed (009442A0,
        // 00BF65AC) and the count +8h decremented; 00945BB7: no result.
        if (!host->error_replay()) {
            ++g_spawn_id_census.removes;
            g_spawn_id_census.removed += bsp::spawn_request_queue().remove_id_00945a20(id);
        }
        return 0;
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
    // Packet cc9_movie_interface_and_reseed: every MovCamNew native starts with
    // 005CD240 on the movie screen (008B7941 AddPositions, 008B7AE1 AddPosition,
    // 008B7C92 SetFOV). Only that call is bound; the rows stay UNIMPLEMENTED for
    // the keyframe store and the FOV that follow it.
    if ((bsp::game::kMovieInterfacePushBound || bsp::game::kMovieMoverBound)
        && !host->error_replay()
        && (dispatch_row.address == 0x008b7850u || dispatch_row.address == 0x008b79f0u
            || dispatch_row.address == 0x008b7ba0u)) {
        bsp::game::hud_movie_screen_camera_005cd240();
    }
    // Packet cc9_movie_camera_mover_bind: 008B7AE8, the table to 007A44D0.
    if (movie_add_row) {
        if (!host->error_replay() && argc >= 1 && lua_type(state, 1) == LUA_TTABLE) {
            bsp::MovieKeyframeInput input;
            parse_movie_keyframe(state, 1, input);
            bsp::game::hud_movie_add_position_007a44d0(input);
        }
        return 0;
    }
    if (select_unit_row) {
        // 008AB260: BSP_ObjectHandle_FromLuaTable(argument 0), then 00647300.
        // The entity table's `ID` names the created unit, as the objective rows
        // resolve it. The native returns its result count, 0 here (no push).
        if (!host->error_replay()) {
            std::size_t unit = 0;
            bool reached = false;
            if (objective_argument_unit(state, 0, unit)) {
                bsp::game::hud_set_selected_unit_00647300(unit, reached);
            }
        }
        return 0;
    }
    if (force_select_row) {
        // 008AB01F CALL 006485A0; the native returns its empty call frame's
        // result count, 0.
        if (!host->error_replay()) bsp::game::hud_force_select_unit_006485a0();
        return 0;
    }
    if (scoring_play_time_row) {
        if (!host->error_replay()) {
            game_scoring_set_real_play_time_running_00905340(lua_toboolean(state, 1) != 0);
        }
        return 0;
    }
    if (handled && !host->error_replay()) {
        return orders->dispatch(state, binding.name, argc);
    }
    if (bsp::mission_binding_returns_entity(binding.name)) {
        // Packet cc8_spawn_new_route, second pass. `handled` was decided above,
        // before this arm ran, so a row that answers HERE was still counted as
        // unimplemented and the summary said so. On USN04 that made
        // `MissionLuaNative::FindEntity 00898e30 UNIMPLEMENTED calls=132` sit in
        // the same report as `entity_resolves=132`, which is every one of those
        // calls answered with a real entity table. The count was right and the
        // status was a lie, and the UNIMPLEMENTED census is what a reader picks
        // the next packet from. The status is now recorded from the outcome.
        // 0089903C is the arm the native takes when the lookup produced
        // nothing. When it produced an entity the same tail pushes that
        // entity's thisTable slot instead, and milestone 2l fills those slots
        // for the created scene instances, so `FindEntity` can answer for real.
        if (host->push_resolved_entity(state, binding.name, argc)) {
            if (!host->error_replay()) host->note_entity_status(binding, true);
            return 1;
        }
        if (!host->error_replay()) {
            host->note_entity_status(binding, false);
            host->note_entity_return();
        }
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

namespace {
// Packet cc9_ai_plane_loadout_arm: the mission interpreter the process-wide
// loadout reads go through, published by create_state, withdrawn on close.
lua_State* g_loadout_lua_state = nullptr;
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
        if (g_loadout_lua_state == state_) g_loadout_lua_state = nullptr;
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
            row.turn_roll_leader = number("TurnRollLeader");
            // The aerodynamic trio. XDrag and YDrag are the body-frame damping
            // 007DBD37-007DBE0D applies, TravelSpeed is the airspeed 007C6340
            // seeds a plane with, and MaxSpd is the numerator of the run
            // profile's speed ratio at 009F9D30. A ship row carries none of
            // them and reads zero, which is the right answer for a ship.
            row.x_drag = number("XDrag");
            row.y_drag = number("YDrag");
            row.max_spd = number("MaxSpd");
            row.travel_speed = number("TravelSpeed");
            // Packet cc9_unit_class_lands_troops (see the row declaration).
            {
                ::lua_getfield(state_, -1, "LandingShip");
                row.landing_ship_id = ::lua_type(state_, -1) == LUA_TNUMBER
                    ? static_cast<int>(::lua_tonumber(state_, -1)) : 0;
                ::lua_settop(state_, ::lua_gettop(state_) - 1);
                ::lua_getfield(state_, -1, "LandingShipAmount");
                row.landing_ship_amount = ::lua_type(state_, -1) == LUA_TNUMBER
                    ? static_cast<int>(::lua_tonumber(state_, -1)) : 0;
                ::lua_settop(state_, ::lua_gettop(state_) - 1);
                ::lua_getfield(state_, -1, "Rocketer");
                row.landing_ship_is_rocketer = ::lua_type(state_, -1) == LUA_TBOOLEAN
                    && ::lua_toboolean(state_, -1) != 0;
                ::lua_settop(state_, ::lua_gettop(state_) - 1);
                // 00833B98: a zero id skips the resolve and leaves +78Ch zero.
                if (row.landing_ship_id != 0) {
                    // Stack: VehicleClass, row. Index the class table (-2).
                    ::lua_pushinteger(state_, row.landing_ship_id);
                    ::lua_gettable(state_, -3);
                    if (::lua_type(state_, -1) == LUA_TTABLE) {
                        ::lua_getfield(state_, -1, "Type");
                        const char* target_type = ::lua_type(state_, -1) == LUA_TSTRING
                            ? ::lua_tolstring(state_, -1, nullptr) : nullptr;
                        const auto* kind = target_type != nullptr
                            ? bsp::vehicle_class_kind_row(target_type) : nullptr;
                        row.landing_ship_resolves = kind != nullptr
                            && kind->kind == bsp::VehicleClassKind::LandingShip;
                        ::lua_settop(state_, ::lua_gettop(state_) - 1);
                    }
                    ::lua_settop(state_, ::lua_gettop(state_) - 1);
                }
            }
            // Packet cc9_submarine_dive: 00854230's NumberOr keys, each keeping
            // its literal default when the row does not author it.
            const auto number_or = [&](const char* key, float fallback) -> float {
                ::lua_getfield(state_, -1, key);
                const float value = ::lua_type(state_, -1) == LUA_TNUMBER
                    ? static_cast<float>(::lua_tonumber(state_, -1)) : fallback;
                ::lua_settop(state_, ::lua_gettop(state_) - 1);
                return value;
            };
            row.sub_periscope_depth = number_or("PeriscopeDepth", -1.0f);
            if (row.sub_periscope_depth < 0.0f) {
                row.sub_periscope_depth = number_or("SwimDepth1", row.sub_periscope_depth);
            }
            row.sub_swim_depth2 = number_or("SwimDepth2", -1.0f);
            row.sub_swim_depth3 = number_or("SwimDepth3", -1.0f);
            row.sub_up_down_accel = number_or("UpDownAccel", 0.25f);
            row.sub_up_down_stop_time = number_or("UpDownStopTime", 5.0f);
            row.sub_up_speed = number_or("UpSpeed", 1.2f);
            row.sub_down_speed = number_or("DownSpeed", 1.2f);
            row.sub_air_run_out_time = number_or("AirRunOutTime", 120.0f);
            row.sub_air_reload_time = number_or("AirReloadTime", 5.0f);
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
            row.min_water_spd = number("MinWaterSpd");
            row.turn_circle_radius = number("TurnCircleRadius");
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

namespace {
// Packet cc9_hit_accuracy. 00836F80's reads, over ShipGlobals.WeaponHitAccuracy[cat]
// (00B67800 field, 00B67720 element, 00B66270 number): an absent key or element
// leaves the current value, as the native code does.
class WeaponHitAccuracyLuaTable final : public bsp::WeaponHitAccuracyTableHost {
public:
    WeaponHitAccuracyLuaTable(lua_State* state, int table) : state_(state), table_(table) {}
    float read_number(const char* key, int element, float current) override {
        const int top = ::lua_gettop(state_);
        lua_getfield(state_, table_, key);
        float value = current;
        if (lua_type(state_, -1) == LUA_TTABLE) {
            lua_rawgeti(state_, -1, element);
            if (lua_type(state_, -1) == LUA_TNUMBER) {
                value = static_cast<float>(lua_tonumber(state_, -1));
            }
        }
        ::lua_settop(state_, top);
        return value;
    }
private:
    lua_State* state_;
    int table_;
};
}  // namespace

// 0083C795..0083C919: WeaponHitAccuracy (00D0B344), then Artillery (00CE5454) into
// +240h, AA (00CFA420) into +298h, Torpedo (00CE544C) into +2F0h and DepthCharge
// (00CFA700) into +348h, each through 00836F80, over the 00836EF0 defaults.
void GameMissionLuaHost::load_weapon_hit_accuracy_0083c795() {
    static const char* const kCategories[4] = {"Artillery", "AA", "Torpedo", "DepthCharge"};
    for (bsp::WeaponHitAccuracyProfile& profile : weapon_hit_accuracy_) {
        bsp::apply_weapon_hit_accuracy_defaults_00836ef0(profile);
    }
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kShipGlobalsGlobal);
    if (lua_type(state_, -1) == LUA_TTABLE) {
        lua_getfield(state_, -1, "WeaponHitAccuracy");
        if (lua_type(state_, -1) == LUA_TTABLE) {
            const int parent = ::lua_gettop(state_);
            for (int c = 0; c < 4; ++c) {
                lua_getfield(state_, parent, kCategories[c]);
                if (lua_type(state_, -1) == LUA_TTABLE) {
                    WeaponHitAccuracyLuaTable table(state_, ::lua_gettop(state_));
                    bsp::load_weapon_hit_accuracy_profile_00836f80(weapon_hit_accuracy_[c], table);
                }
                ::lua_settop(state_, parent);
            }
            weapon_hit_accuracy_loaded_ = true;
        }
    }
    ::lua_settop(state_, top);
    for (int c = 0; c < 4; ++c) {
        const bsp::WeaponHitAccuracyProfile& p = weapon_hit_accuracy_[c];
        log_.notef("weapon hit accuracy %s sizes=%.0f/%.0f small=%.2f..%.2f large=%.2f..%.2f",
            kCategories[c], p.small_target_size, p.large_target_size,
            p.small_target_accuracy[0], p.small_target_accuracy[9],
            p.large_target_accuracy[0], p.large_target_accuracy[9]);
    }
    if (weapon_hit_accuracy_loaded_) {
        log_.implemented("GameSettings::load_weapon_hit_accuracy", "0083c795");
    } else {
        log_.unimplemented("GameSettings::load_weapon_hit_accuracy", "0083c795");
    }
}

bool GameMissionLuaHost::read_weapon_hit_accuracy(
    bsp::WeaponHitAccuracyProfile (&out)[4]) const noexcept {
    if (!weapon_hit_accuracy_loaded_) return false;
    for (int c = 0; c < 4; ++c) out[c] = weapon_hit_accuracy_[c];
    return true;
}

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
        {
            // Packet cc9_ship_neighbour_list: +190h..+1D8h for the neighbour list.
            std::uint32_t offsets[19];
            for (std::uint32_t i = 0; i < 19; ++i) offsets[i] = 0x190u + 4u * i;
            std::array<float, 19> block{};
            std::string block_error;
            if (bsp::game::read_ship_ai_settings_offsets_lua(*state_, offsets, block.data(),
                    block.size(), block_error)) {
                ship_avoidance_block_ = block;
                ship_avoidance_block_loaded_ = true;
            } else {
                log_.notef("ship avoidance block load failed: %s", block_error.c_str());
            }
        }
        log_.implemented("GameSettings::load_avoidance_tuning_projection", "0083b5e0");
        log_.notef("stored ship avoidance tuning 194=%.9g 1d4=%.9g 1d8=%.9g 214=%.9g 218=%.9g",
            tuning[0], tuning[1], tuning[2], tuning[3], tuning[4]);
        // Literal store in the native settings load, not a Lua key/default
        // inferred from the mission. The explicit process reload replays it.
        avoid_all_ship_collision_ = kAvoidAllShipCollisionLoaderDefault;
        avoid_all_ship_collision_loaded_ = true;
        log_.implemented("GameSettings::load_avoid_all_ship_collision", "0083bcd5");
        load_weapon_hit_accuracy_0083c795();
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

bool GameMissionLuaHost::read_ship_avoidance_block(
    std::array<float, 19>& values) const noexcept {
    if (!ship_avoidance_block_loaded_) return false;
    values = ship_avoidance_block_;
    return true;
}

bool GameMissionLuaHost::read_global_number_pair(const char* name, float& first,
    float& second) {
    if (state_ == nullptr) return false;
    const int top = ::lua_gettop(state_);
    ::lua_getfield(state_, LUA_GLOBALSINDEX, name);
    bool ok = false;
    if (lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_rawgeti(state_, -1, 1);
        ::lua_rawgeti(state_, -2, 2);
        if (::lua_isnumber(state_, -2) && ::lua_isnumber(state_, -1)) {
            first = static_cast<float>(::lua_tonumber(state_, -2));
            second = static_cast<float>(::lua_tonumber(state_, -1));
            ok = true;
        }
    }
    ::lua_settop(state_, top);
    return ok;
}

bool GameMissionLuaHost::read_global_nested_number(const char* table, const char* sub,
    const char* key, float& out) {
    if (state_ == nullptr) return false;
    const int top = ::lua_gettop(state_);
    ::lua_getfield(state_, LUA_GLOBALSINDEX, table);
    bool ok = false;
    if (lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_getfield(state_, -1, sub);
        if (lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, key);
            if (::lua_isnumber(state_, -1)) {
                out = static_cast<float>(::lua_tonumber(state_, -1));
                ok = true;
            }
        }
    }
    ::lua_settop(state_, top);
    return ok;
}

bool GameMissionLuaHost::read_scoring_intervals_0091b2e0(float& update_interval,
    float& recalc_interval) {
    update_interval = 1.0f;   // 0091BC0E FLD1
    recalc_interval = 5.0f;   // 0091BC4C FLD [00CE3850]
    if (state_ == nullptr) return false;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, "Scoring");
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        set_phase("scoring table");
        bsp::run_script_file(*this, "Scripts/datatables/Scoring.lua");
        lua_getfield(state_, LUA_GLOBALSINDEX, "Scoring");
    }
    bool ok = false;
    if (lua_type(state_, -1) == LUA_TTABLE) {
        ok = true;
        ::lua_getfield(state_, -1, "InGameScoreUpdateTimeInterval");  // 00D18974
        if (::lua_isnumber(state_, -1)) {
            update_interval = static_cast<float>(::lua_tonumber(state_, -1));
        }
        ::lua_pop(state_, 1);
        ::lua_getfield(state_, -1, "ReCalcTimeInterval");  // 00D18960
        if (::lua_isnumber(state_, -1)) {
            recalc_interval = static_cast<float>(::lua_tonumber(state_, -1));
        }
    }
    ::lua_settop(state_, top);
    return ok;
}

bool GameMissionLuaHost::read_lock_radius_multipliers_0087dc85(std::vector<float>& out) {
    out.clear();
    if (state_ == nullptr) return false;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kGlobalsGlobal);
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        set_phase("global config");
        bsp::run_script_file(*this, kGlobalConfigScriptPath);
        lua_getfield(state_, LUA_GLOBALSINDEX, kGlobalsGlobal);
    }
    if (lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_getfield(state_, -1, "Difficulty");
        if (lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, "HPMultipliers");
            ::lua_getfield(state_, -2, "LockRadiusMultipliers");
            // 0087DB08..0087DDB3: while HPMultipliers[index] is not nil, each
            // of the four vectors takes its entry at that index.
            if (lua_type(state_, -2) == LUA_TTABLE && lua_type(state_, -1) == LUA_TTABLE) {
                for (int index = 1;; ++index) {
                    ::lua_rawgeti(state_, -2, index);
                    const bool present = lua_type(state_, -1) != LUA_TNIL;
                    ::lua_pop(state_, 1);
                    if (!present) break;
                    ::lua_rawgeti(state_, -1, index);
                    out.push_back(static_cast<float>(::lua_tonumber(state_, -1)));
                    ::lua_pop(state_, 1);
                }
            }
        }
    }
    ::lua_settop(state_, top);
    log_.notef("lock radius: Globals[\"Difficulty\"][\"LockRadiusMultipliers\"] read %u "
        "value(s) from %s", static_cast<unsigned>(out.size()), kGlobalConfigScriptPath);
    return !out.empty();
}

bool GameMissionLuaHost::read_difficulty_multipliers_0087d7b0(std::vector<float>& hp_inverse,
    std::vector<float>& cheat_inverse) {
    hp_inverse.clear();
    cheat_inverse.clear();
    if (state_ == nullptr) return false;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kGlobalsGlobal);
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        set_phase("global config");
        bsp::run_script_file(*this, kGlobalConfigScriptPath);
        lua_getfield(state_, LUA_GLOBALSINDEX, kGlobalsGlobal);
    }
    bool ok = false;
    if (lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_getfield(state_, -1, "Difficulty");
        if (lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, "HPMultipliers");
            ::lua_getfield(state_, -2, "PlayerCheatMultipliers");
            // 0087DB08..0087DDB3: while HPMultipliers[index] is not nil, each
            // vector takes its entry at that index. 0087DB61 FLD1 / 0087DB63
            // FDIVRP store 1/HPMultipliers[i] into the vector at config+1Ch;
            // 0087DD33 FLD1 / 0087DD35 FDIVRP store 1/PlayerCheatMultipliers[i]
            // into the vector at config+4Ch.
            if (lua_type(state_, -2) == LUA_TTABLE && lua_type(state_, -1) == LUA_TTABLE) {
                ok = true;
                for (int index = 1;; ++index) {
                    ::lua_rawgeti(state_, -2, index);
                    const bool present = lua_type(state_, -1) != LUA_TNIL;
                    const float hp = static_cast<float>(::lua_tonumber(state_, -1));
                    ::lua_pop(state_, 1);
                    if (!present) break;
                    ::lua_rawgeti(state_, -1, index);
                    const float cheat = static_cast<float>(::lua_tonumber(state_, -1));
                    ::lua_pop(state_, 1);
                    hp_inverse.push_back(1.0f / hp);
                    cheat_inverse.push_back(1.0f / cheat);
                }
            }
        }
    }
    ::lua_settop(state_, top);
    log_.notef("difficulty: Globals[\"Difficulty\"] HPMultipliers/PlayerCheatMultipliers read %u "
        "value(s) from %s (0087DB61 / 0087DD33)", static_cast<unsigned>(hp_inverse.size()),
        kGlobalConfigScriptPath);
    return ok;
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

float GameMissionLuaHost::sub_attack_submarine_lost_time_04d4() const {
    // Packet cc9_ships7_entry_points. 0083B5E0's SubAttack block: 0083F692
    // pushes "SubAttack" (00D0A954), 0083F779 "SubmarineLostTime" (00D0A910),
    // 0083F79C BSP_LuaObject_GetNumber (lua_tonumber, spilled to float) and
    // 0083F7A1 FSTP [ESI+4D4h]. There is no default in the listing: a missing
    // or non-numeric key reads 0, which is what lua_tonumber answers here too.
    // This installation's Scripts/datatables/shipglobals.lua (mtime
    // 2024-07-13) authors ShipGlobals["SubAttack"]["SubmarineLostTime"] = 30
    // at line 473.
    if (state_ == nullptr) return 0.0f;
    const int top = ::lua_gettop(state_);
    float value = 0.0f;
    lua_getfield(state_, LUA_GLOBALSINDEX, kShipGlobalsGlobal);
    if (lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_getfield(state_, -1, "SubAttack");
        if (lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, "SubmarineLostTime");
            value = static_cast<float>(::lua_tonumber(state_, -1));
        }
    }
    ::lua_settop(state_, top);
    return value;
}

bool GameMissionLuaHost::read_ship_camera_settings_0083b5e0(bsp::ShipCameraSettings& out) {
    if (state_ == nullptr) return false;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kShipGlobalsGlobal);
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    ::lua_getfield(state_, -1, "ShipCamera");
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    const int table = ::lua_gettop(state_);
    float values[4] = {};
    static const char* const kKeys[4] = {
        "ZoomOffset", "LengthMult", "MinCameraAngle", "MaxCameraAngle"};
    bool complete = true;
    for (int i = 0; i < 4; ++i) {
        ::lua_getfield(state_, table, kKeys[i]);
        if (lua_type(state_, -1) == LUA_TNUMBER) {
            values[i] = static_cast<float>(::lua_tonumber(state_, -1));
        } else {
            complete = false;
        }
        ::lua_settop(state_, table);
    }
    ::lua_settop(state_, top);
    if (!complete) return false;
    out.zoom_offset = values[0];
    out.length_mult = values[1];
    out.min_angle_deg = values[2];
    out.max_angle_deg = values[3];
    return true;
}

bool GameMissionLuaHost::read_global_fov_ship_0087d7b0(double& degrees) {
    if (state_ == nullptr) return false;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kGlobalsGlobal);
    const bool loaded = lua_type(state_, -1) == LUA_TTABLE;
    ::lua_settop(state_, top);
    if (!loaded) {
        static_cast<void>(bsp::run_script_file(*this, kGlobalConfigScriptPath));
    }
    lua_getfield(state_, LUA_GLOBALSINDEX, kGlobalsGlobal);
    bool read = false;
    if (lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_getfield(state_, -1, "FOVs");
        if (lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, "Ship");
            if (lua_type(state_, -1) == LUA_TNUMBER) {
                degrees = ::lua_tonumber(state_, -1);
                read = true;
            }
        }
    }
    ::lua_settop(state_, top);
    return read;
}

bool GameMissionLuaHost::read_pipe_sight_params_0083b5e0(bool& enabled, float& zoom_rate) {
    if (state_ == nullptr) return false;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kShipGlobalsGlobal);
    bool read = false;
    if (lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_getfield(state_, -1, "PipeSightParams");
        if (lua_type(state_, -1) == LUA_TTABLE) {
            const int table = ::lua_gettop(state_);
            ::lua_getfield(state_, table, "pipesight_enabled");
            const bool has_enabled = lua_type(state_, -1) == LUA_TBOOLEAN;
            const bool value = has_enabled && ::lua_toboolean(state_, -1) != 0;
            ::lua_settop(state_, table);
            ::lua_getfield(state_, table, "zoom_rate");
            const bool has_rate = lua_type(state_, -1) == LUA_TNUMBER;
            const float rate = has_rate ? static_cast<float>(::lua_tonumber(state_, -1)) : 0.0f;
            if (has_enabled && has_rate) {
                enabled = value;
                zoom_rate = rate;
                read = true;
            }
        }
    }
    ::lua_settop(state_, top);
    return read;
}

bool GameMissionLuaHost::read_ship_class_camera_00831e0d(int type_id,
    bsp::ShipClassCameraInputs& out) {
    if (state_ == nullptr || type_id < 0) return false;
    const int top = ::lua_gettop(state_);
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (::lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    ::lua_pushinteger(state_, type_id);
    ::lua_gettable(state_, -2);
    if (::lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    const int row = ::lua_gettop(state_);
    const auto number = [&](const char* key, bool& present, float& value) {
        ::lua_getfield(state_, row, key);
        present = ::lua_type(state_, -1) == LUA_TNUMBER;
        if (present) value = static_cast<float>(::lua_tonumber(state_, -1));
        ::lua_settop(state_, row);
    };
    bsp::ShipClassCameraInputs in = out;
    number("CaptainCameraHeight", in.has_captain_camera_height, in.captain_camera_height);
    number("CameraDistanceFront", in.has_distance_front, in.distance_front);
    number("CameraDistanceSide", in.has_distance_side, in.distance_side);
    number("CameraDistanceVertical", in.has_distance_vertical, in.distance_vertical);
    number("CameraMinHeight", in.has_min_height, in.min_height);
    bool has_length = false;
    float length = 0.0f;
    number("Length", has_length, length);
    in.base_length = has_length ? length : 0.0f;
    ::lua_settop(state_, top);
    out = in;
    return true;
}

bool GameMissionLuaHost::read_engine_sound_smooth_rates_0083b5e0(float (&out)[4]) {
    // 0084041A..008405D5 of 0083B5E0: ShipGlobals["Sounds"], then one record
    // table per index (0084043C jump table), EngineSoundSmoothRate (00D0A5F8)
    // read through 00B66330 with the 0.2f default and stored at record+8h.
    if (state_ == nullptr) return false;
    static const char* const kRecordNames[4] = {"Ship", "TBoat", "Submarine", "Plane"};
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kShipGlobalsGlobal);
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    ::lua_getfield(state_, -1, "Sounds");
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    const int sounds = ::lua_gettop(state_);
    float rates[4] = {0.2f, 0.2f, 0.2f, 0.2f};
    for (int i = 0; i < 4; ++i) {
        ::lua_getfield(state_, sounds, kRecordNames[i]);
        if (lua_type(state_, -1) != LUA_TTABLE) {
            ::lua_settop(state_, top);
            return false;
        }
        ::lua_getfield(state_, -1, "EngineSoundSmoothRate");
        if (lua_type(state_, -1) == LUA_TNUMBER) {
            rates[i] = static_cast<float>(::lua_tonumber(state_, -1));
        }
        ::lua_settop(state_, sounds);
    }
    ::lua_settop(state_, top);
    for (int i = 0; i < 4; ++i) out[i] = rates[i];
    return true;
}

bool GameMissionLuaHost::read_physics_torpedo_force_0083b5e0(float& force, float& power) {
    // 0083FE53..0083FECD of 0083B5E0 (see the header).
    if (state_ == nullptr) return false;
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, kShipGlobalsGlobal);
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    ::lua_getfield(state_, -1, "Physics");
    if (lua_type(state_, -1) != LUA_TTABLE) {
        ::lua_settop(state_, top);
        return false;
    }
    const int physics = ::lua_gettop(state_);
    float f = 1.0f;   // 0083FE6C FLD1
    float p = 2.0f;   // [00CE3958]
    ::lua_getfield(state_, physics, "TorpedoForce");
    if (lua_type(state_, -1) == LUA_TNUMBER) f = static_cast<float>(::lua_tonumber(state_, -1));
    ::lua_settop(state_, physics);
    ::lua_getfield(state_, physics, "TorpedoForcePower");
    if (lua_type(state_, -1) == LUA_TNUMBER) p = static_cast<float>(::lua_tonumber(state_, -1));
    ::lua_settop(state_, top);
    force = f;
    power = p;
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

// Packet cc9_avoid_zone_draft_bodies.
struct DraftDepthReadContext {
    std::int32_t session_mode;
    const bool* crt_sse2_conversion;
    std::array<std::int32_t, 9> result;
};
static_assert(std::is_trivially_destructible_v<DraftDepthReadContext>);

int read_draft_depths_protected(lua_State* state) {
    auto& context = *static_cast<DraftDepthReadContext*>(
        lua_touserdata(state, lua_upvalueindex(1)));
    // Only trivial automatic objects cross a possible Lua longjmp.
    NativeLuaStateStorage owner;
    construct_native_lua_state_00b66bd0(&owner);
    owner.state_04 = state;
    NativeLuaObjectStorage globals, ship_globals, depths;
    native_lua_globals_00b67980(owner, &globals);
    native_lua_get_by_name_00b67800(globals, &ship_globals, "ShipGlobals");
    native_lua_get_by_name_00b67800(ship_globals, &depths,
        ship_tuning_block_offset(context.session_mode) == kAvoidZoneDepthsSingleOffset
            ? "AvoidZoneDepthsSingle" : "AvoidZoneDepthsMulti");
    // 00424DDF..004253FF's order (docs/AVOID_ZONE_DRAFT_LAYERS.md).
    constexpr AvoidZoneDepthSlot slots[9] = {AvoidZoneDepthSlot::kBattleShip,
        AvoidZoneDepthSlot::kMotherShip, AvoidZoneDepthSlot::kDestroyer,
        AvoidZoneDepthSlot::kTBoat, AvoidZoneDepthSlot::kLargeLandingShip,
        AvoidZoneDepthSlot::kCargoShip, AvoidZoneDepthSlot::kLightCruiser,
        AvoidZoneDepthSlot::kHeavyCruiser, AvoidZoneDepthSlot::kSubmarine};
    for (std::size_t i = 0; i < 9; ++i) {
        NativeLuaObjectStorage values, first;
        native_lua_get_by_name_00b67800(depths, &values,
            depth_key(static_cast<std::uint32_t>(slots[i])));
        native_lua_get_by_index_00b67720(values, &first, 1);
        context.result[i] = static_cast<std::int32_t>(
            native_lua_integer_00b66290(first, *context.crt_sse2_conversion));
        destroy_native_lua_object_00b67700(first);
        destroy_native_lua_object_00b67700(values);
    }
    destroy_native_lua_object_00b67700(depths);
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

bool GameMissionLuaHost::read_avoid_zone_draft_depths(std::int32_t session_mode,
    std::array<std::int32_t, 9>& output, std::string& error) {
    if (state_ == nullptr) {
        error = "avoid-zone draft depths require the live mission Lua state";
        return false;
    }
    const bool sse2_conversion =
        IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE;
    DraftDepthReadContext context{session_mode, &sse2_conversion, {}};
    const int top = ::lua_gettop(state_);
    ::lua_pushlightuserdata(state_, &context);
    ::lua_pushcclosure(state_, &read_draft_depths_protected, 1);
    const int status = ::lua_pcall(state_, 0, 0, 0);
    if (status != 0) {
        const char* message = ::lua_tolstring(state_, -1, nullptr);
        error = message != nullptr ? message : "draft depth lookup raised a non-string Lua error";
        ::lua_settop(state_, top);
        return false;
    }
    ::lua_settop(state_, top);
    output = context.result;
    error.clear();
    return true;
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
    if (kReconPublishBound) {
        log_.implemented("Recon::publish_slot_table", "00806b10");
    } else {
        log_.unimplemented("Recon::publish_slot_table", "00806b10");
    }
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
    mirror_party_race_00928f50();
}

// 00928F50 BSP_MissionEntity_SetPartyRaceLuaMirror, the root entity vtable's
// slot 11 (reached through the adjustor thunk 00951F30). It is a SEPARATE event
// from 00928A00's attach - the attach seeds ID, Dead and Ptr, and this runs when
// the entity's party is set - so it runs here, one hop after the attach, where
// the orders host has just made the unit table reachable. 00928FD9 sets `Race`
// and 00929046 sets `Party`, both through 00B67460, and both read off the entity
// rather than off the call's arguments (00928FC7 and 00929034 load from ESI);
// this host knows the party and not the race, so it writes the one it has.
//
// It matters because the shipped commandhelpers.lua indexes
// `recon[targetUnit.Party][allegiance]` at 330, 494 and 518. `recon`'s index
// keys are the three integers 00803A40 installs (src/recon_values.cpp, the
// `index != 3` loop), which are this installation's PARTY_ALLIED 0,
// PARTY_JAPANESE 1 and PARTY_NEUTRAL 2 (scripts/global/luamw_init.lua 73-75),
// and GameUnitRow::party is in the same space.
std::size_t GameMissionLuaHost::mirror_party_race_00928f50() {
    if (state_ == nullptr || script_orders_ == nullptr) return 0;
    const GameUnitsHost& units = script_orders_->units();
    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (!lua_istable(state_, -1)) {
        ::lua_settop(state_, ::lua_gettop(state_) - 1);
        return 0;
    }
    std::size_t mirrored = 0;
    for (std::size_t index = 0; index < units.count(); ++index) {
        const GameUnitRow* row = units.unit_row(index);
        if (row == nullptr || row->party < 0 || row->name.empty()) continue;
        const std::map<std::string, int>::const_iterator found
            = scene_entity_ids_.find(row->name);
        if (found == scene_entity_ids_.end()) continue;
        char key[16];
        std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, found->second);
        lua_getfield(state_, -1, key);
        if (!lua_istable(state_, -1)) {
            ::lua_settop(state_, ::lua_gettop(state_) - 1);
            continue;
        }
        lua_pushinteger(state_, row->party);
        lua_setfield(state_, -2, "Party");
        ::lua_settop(state_, ::lua_gettop(state_) - 1);
        ++mirrored;
    }
    ::lua_settop(state_, ::lua_gettop(state_) - 1);
    summary_.party_mirrors = mirrored;
    log_.implemented("MissionEntity::set_party_race_lua_mirror", "00928f50");
    log_.notef("thisTable: 00928f50's `Party` mirror written on %zu slot(s). Without it "
        "commandhelpers.lua:330 `recon[targetUnit.Party][allegiance]` indexes a nil, which is "
        "what reverted the GetSelectedUnit binding in packet cc8_ship_moveonpath; `Race` is "
        "not written because this host does not carry one", mirrored);
    return mirrored;
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
        // Packet cc8_spawn_new_route, second pass. GameHostLog's record is
        // sticky on FIRST insert - `record()` returns an existing entry with only
        // its count bumped, and sets `implemented` solely when it creates one -
        // so whichever of implemented()/unimplemented() runs first decides the
        // status for ever. For an entity-returning row the answer is not known
        // yet: `handled` was decided before the arm at the bottom of
        // binding_trampoline runs, and that arm resolves 132 of USN04's 132
        // `FindEntity` calls. Recording UNIMPLEMENTED here would lock in a status
        // the same report contradicts with `entity_resolves=132`, and a later
        // implemented() could not undo it. So the status is left to the arm,
        // which records exactly one of the two.
        if (!bsp::mission_binding_returns_entity(binding.name)) {
            log_.unimplemented(method, address);
        }
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
    // Packet cc8_navigator_path. The guard above was added to the first-insert
    // branch only, and this repeat branch kept calling unimplemented() for every
    // later call of an entity-returning row - on top of the note_entity_status()
    // the bottom of binding_trampoline already makes. That is two GameHostLog
    // bumps per call after the first, so n calls printed 1 + 2(n-1) = 2n-1.
    // `local/census2_usn04.log` measured it on both rows that have a known
    // count: FindEntity 132 calls printed `concrete calls=263` and
    // GetSelectedUnit 49 printed `UNIMPLEMENTED calls=97`. The status half of
    // b5a31c82f did take - the row reads `concrete` where it used to read
    // `UNIMPLEMENTED` - so only the count was still wrong. Same guard, same
    // reason: for these rows the arm at the bottom is the only recorder.
    if (!bsp::mission_binding_returns_entity(binding.name)) {
        log_.unimplemented(method, address);
    }
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
    if constexpr (kSEntityInitAllBound) {
        // 0089E611 XOR CL,CL / 0089E613 CALL 00925F20, right after 006CC690 and
        // before the result is pushed: a started launch is attached here.
        run_sentity_init_all_00925f20(false, 0x0089e613u);
    }
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

// CORRECTED by packet cc8_spawn_new_route. 00888760 BSP_LuaObject_ReadVector3
// does NOT index the table 1..3. Its whole body is one walk: 008887A9 opens the
// iteration with 00B67080, 0088882F steps it with 00B67190, and between them it
// compares each key against three string literals and stores the number it finds
// into one of three float slots:
//
//   008887C2 PUSH 0xceb488 ("x") ... 008887DD FSTP float ptr [ESP + 0x8]
//   008887E3 PUSH 0xd045f8 ("y") ... 008887FE FSTP float ptr [ESP + 0xc]
//   00888804 PUSH 0xcfd718 ("z") ... 0088881F FSTP float ptr [ESP + 0x10]
//
// There is no index path before the loop either (00888767..008887A9 is the
// prologue and two Lua temporaries). A table whose entries are at 1, 2 and 3
// matches no key and reads nothing.
//
// This is the shape the scripts actually pass. `GetPosition` builds its table
// with the same three named keys - 0088BA30, which
// GameScriptOrdersHost::push_vector3_table_0088ba30 already reproduces - and
// usn_19_coralus.lua then writes `spawnpos1.x`, `.y` and `.z` on it before
// handing it to `SpawnNew` as `area.refPos`. The previous reader here answered
// false for every one of those tables, which is why a 3000-frame USN04 run made
// eight requests and created nothing.
//
// The native does NOT require all three, and it reports nothing. Its three
// staging slots are written only by the three matches above - filtering the
// whole 85-instruction listing for `[ESP+0x8]`, `[ESP+0xc]` and `[ESP+0x10]`
// gives exactly those three `FSTP`s and the three reads below, and no
// initialisation anywhere - and the tail copies all three out unconditionally,
// whatever the walk found:
//
//   00888848 MOVSS XMM0,[ESP+0x8]  / 0088884e MOVSS [EDI],XMM0
//   00888852 MOVSS XMM0,[ESP+0xc]  / 00888858 MOVSS [EDI+0x4],XMM0
//   0088885d MOVSS XMM0,[ESP+0x10] / 00888867 MOVSS [EDI+0x8],XMM0
//
// then `0088888B MOV EAX,EDI` returns the OUT POINTER, not a success flag. So a
// table carrying only `x` and `z` leaves the y component at whatever that stack
// slot held, and the caller cannot tell. `00949750`'s own call site confirms the
// caller does not ask: `00949B9B LEA ECX,[ESP+0x80]` hands it a bare stack local
// with no adjacent pre-fill, `00949BA2 CALL 00888760`, and `00949BA7`/`00949BAB`/
// `00949BB0` read all three floats straight back with no test in between.
//
// DEVIATION, labelled. An absent component is left at zero here rather than at
// an uninitialised stack value, because uninitialised is not reproducible and
// zero is the only defensible substitute. The `bool` is this process's own
// signal and means "this table carried at least one of x, y, z", which is what
// keeps a caller's "position or not?" question answerable; the native asks a
// different question first (`008889C0`, is this an entity handle) and sends
// everything else here regardless. Requiring all three, which this reader did
// briefly, would refuse a `{x=..., z=...}` sea-level point the image accepts.
bool read_vector3_00888760(lua_State* state, int index, float out[3]) {
    if (::lua_type(state, index) != LUA_TTABLE) return false;
    static const char* const kVectorKeys[3] = {"x", "y", "z"};
    int found = 0;
    for (int i = 0; i < 3; ++i) {
        ::lua_getfield(state, index, kVectorKeys[i]);
        if (::lua_type(state, -1) == LUA_TNUMBER) {
            out[i] = static_cast<float>(::lua_tonumber(state, -1));
            ++found;
        } else {
            out[i] = 0.0f;
        }
        ::lua_settop(state, ::lua_gettop(state) - 1);
    }
    return found > 0;
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
    const std::size_t units_before = script_orders_->units().count();
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
    bool attached = false;
    if constexpr (kSEntityInitAllBound) {
        // Packet cc9_sentity_init_all. The creator's construction pushed the
        // entity (00928760); a plane squadron's pass A then pushes its wing.
        if (record.class_id == 0x18) {
            route_push_squadron(static_cast<int>(entity_id), name,
                record.type_id, units_before);
        } else {
            route_push_entity(static_cast<int>(entity_id), name, record.type_id);
        }
        // Packet cc9_generated_entity_party: the record's bag values, as
        // 00927050's kind-1 arm reads them at pass A (00928A1E).
        if (PendingEntity* node = find_pending(static_cast<int>(entity_id))) {
            ++summary_.generated_party_nodes;
            if constexpr (kGeneratedEntityPartyBound) {
                node->party = record.party;
                node->race = record.race;
                node->generated_party = true;
            }
        }
        run_sentity_init_all_00925f20(false, 0x0046dbe8u);  // 0046DBE6 XOR CL,CL
        // 00945311 MOV CL,1 / CALL 00874D00, BSP_Game_RunExtraFixedStep, whose
        // 00874D77 XOR CL,CL / 00874D79 CALL 00925F20 finds the list the
        // creator's own call just emptied. Only that row of 00874D00 is run,
        // unless packet cc9_run_extra_fixed_step's switch runs the whole body.
        if (kRunExtraFixedStepBound && extra_fixed_step_ != nullptr) {
            extra_fixed_step_->run_extra_fixed_step_00874d00(true, 0x00945311u);
        } else {
            run_sentity_init_all_00925f20(false, 0x00874d79u);
        }
        attached = init_all_attached(static_cast<int>(entity_id));
    } else {
        static_cast<void>(units_before);
        attached = attach_created_entity_00928a00(static_cast<int>(entity_id), name,
                                                  record.type_id);
    }
    if (!attached) {
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

float read_member_float(lua_State* state, int table_index, const char* key) {
    ::lua_getfield(state, table_index, key);
    float value = 0.0f;
    if (::lua_type(state, -1) == LUA_TNUMBER) {
        value = static_cast<float>(::lua_tonumber(state, -1));
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
        bool ref_is_entity = false;
        if constexpr (bsp::kSpawnNewEntityRefPosBound) {
            // Packet cc9_spawn_new_shipyard. 00949B60 CALL 008889C0 asks first
            // whether the value is an entity table (a non-nil `Ptr` whose object
            // answers vtable+5Ch(1)); yes takes 00888AA0 and 008F8530, which keep
            // the entity itself. This process's stand-in for that handle is the
            // `ID` field, as at every other 00888AA0 site here.
            const int ref = ::lua_gettop(state);
            if (::lua_type(state, ref) == LUA_TTABLE) {
                ::lua_getfield(state, ref, "ID");
                const int type = ::lua_type(state, -1);
                const int id = (type == LUA_TNUMBER || type == LUA_TSTRING)
                    ? static_cast<int>(::lua_tonumber(state, -1)) : 0;
                ::lua_settop(state, ref);
                float frame[16];
                if (id > 0 && spawn_ref_entity_frame_008f8680(id, frame)) {
                    ref_is_entity = true;
                    request.has_ref_pos = true;
                    request.ref_entity_id = id;
                    request.ref_frame_valid = true;
                    for (int i = 0; i < 16; ++i) request.ref_frame[i] = frame[i];
                    for (int i = 0; i < 3; ++i) request.ref_pos[i] = frame[12 + i];
                }
            }
        }
        if (!ref_is_entity
            && read_vector3_00888760(state, ::lua_gettop(state), request.ref_pos)) {
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
        request.exclude.own_horizontal = read_member_float(state, block, "ownHorizontal");
        request.exclude.enemy_horizontal = read_member_float(state, block, "enemyHorizontal");
        request.exclude.own_vertical = read_member_float(state, block, "ownVertical");
        request.exclude.enemy_vertical = read_member_float(state, block, "enemyVertical");
        request.exclude.formation_horizontal
            = read_member_float(state, block, "formationHorizontal");
    }
    ::lua_settop(state, ::lua_gettop(state) - 1);

    if (summary_.spawn_new_queued < 16) {
        log_.notef("  SpawnNew 0094c480: serial %u party %d, %zu group member(s), "
            "callback \"%s\"%s, angleRange %s, refPos %s%s(%.1f %.1f %.1f)",
            request.serial, request.party, request.members.size(),
            request.callback.c_str(), request.id.empty() ? "" : " id set",
            request.has_angle_range ? "given" : "absent",
            request.has_ref_pos ? "" : "ABSENT ",
            request.ref_entity_id > 0 ? "entity " : "",
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
    // Packet cc9_lua_listeners. SUBSTITUTION (labelled): the `kill` channel is
    // evaluated here, once per mission frame, for the deaths since the last
    // frame; the image evaluates it inside the kill flush (009813A0).
    if (kLuaListenersBound) dispatch_kill_listeners_009813a0();
    dispatch_hit_listeners_00988510();
    if (kLuaListenersBound && kLuaReconListenersBound) dispatch_recon_listeners_00980e50();
    if (kReconPublishBound) publish_recon_slot_tables_00806b10();
    // DAT_00F876A4. 0094C490 never reads the delta 0094C8F0 pushes for it, so
    // the step is used only to advance the clock the interval is measured on.
    spawn_world_clock_ += step_seconds;
    bsp::SpawnRequestQueue& queue = bsp::spawn_request_queue();
    // 0094C4AE `CMP dword ptr [EDI + 0x8],EBX` with EBX = 0 and the `JZ` at
    // 0094C4B1: an empty queue returns before the clock is even read.
    if (queue.empty()) return;
    if (script_orders_ == nullptr) return;
    if (!queue.attempt_due(spawn_world_clock_, spawn_attempt_delay_0087f800())) return;

    // DEVIATION, labelled. 0094C508's walk prefers a record whose party is
    // active in the table at `game+18CCh + party*4`. This process has no party
    // table, so every party counts as active and the walk always answers the
    // head - which is what the native itself does whenever the queue holds one
    // record or none (0094C4FA CMP EAX,0x1 / 0094C4FF JBE 0094C56B).
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

namespace {
// Packet cc9_spawn_new_placement: what 00941D30 walks, from this process's
// units. [[00E188A8]+19CCh]+58h is taken as every live unit; party is the row's,
// position unit+FCh..+104h (unit_position_00fc).
// SUBSTITUTION, labelled: 0071C4F0's world bounds are held by the zone runtime,
// which this host does not reach, so no member is refused as outside the map.
class SpawnPlacementUnits final : public bsp::SpawnPlacementWorld {
public:
    explicit SpawnPlacementUnits(const GameUnitsHost& units) : units_(units) {}
    bool point_outside_map_0071c4f0(const float*) const override { return false; }
    void placement_entities(std::vector<bsp::SpawnPlacementEntity>& out) const override {
        out.clear();
        for (std::size_t i = 0; i < units_.count(); ++i) {
            const GameUnitRow* row = units_.unit_row(i);
            if (row == nullptr || !units_.unit_alive_and_visible(i)) continue;
            bsp::SpawnPlacementEntity e;
            e.party = row->party;
            units_.unit_position_00fc(i, e.position[0], e.position[1], e.position[2]);
            out.push_back(e);
        }
    }
private:
    const GameUnitsHost& units_;
};
}  // namespace

bool GameMissionLuaHost::spawn_ref_entity_frame_008f8680(std::int32_t entity_id,
    float frame[16]) const {
    // 008F8680: [ref+14h] set -> (00414DB0 when [entity+C8h] == 0) and entity+CCh.
    // SUBSTITUTION, labelled: a unit's +CCh is read as the units host's pose rows
    // and position; a scene marker's as its authored world matrix, which nothing
    // in the mission moves (FixedInstance, docs/SCENE_ENTITY_FACTORY.md).
    if (script_orders_ == nullptr || entity_id <= 0) return false;
    const GameUnitsHost& units = script_orders_->units();
    if (units.scene_marker_frame(static_cast<std::uint32_t>(entity_id), frame)) return true;
    const std::size_t index = static_cast<std::size_t>(entity_id - 1);
    if (index >= units.count()) return false;
    float right[3], up[3], forward[3], translation[3];
    if (!units.unit_pose(index, right, up, forward, translation)) return false;
    for (int i = 0; i < 3; ++i) {
        frame[i] = right[i];
        frame[4 + i] = up[i];
        frame[8 + i] = forward[i];
        frame[12 + i] = translation[i];
    }
    frame[3] = frame[7] = frame[11] = 0.0f;
    frame[15] = 1.0f;
    return true;
}

void GameMissionLuaHost::fulfil_spawn_request_009483d0(bsp::SpawnNewRequest& request) {
    // 00949300 creates nothing unless EVERY member's placement passes; 009483D0
    // then makes them all and sets record+C0h. The all-or-nothing rule is kept;
    // with kSpawnNewPlacementBound the placement test 00941D30 and 0094A140's
    // retry run too (docs/SCENE_CONTENTS_HOSTS.md section 23).
    if (request.members.empty()) return;
    if constexpr (bsp::kSpawnNewEntityRefPosBound) {
        // 008F8680 reads the kept entity's +CCh each time 0094A140 asks, so a
        // unit that moved since the request moves the frame with it.
        if (request.ref_entity_id > 0) {
            float frame[16];
            if (spawn_ref_entity_frame_008f8680(request.ref_entity_id, frame)) {
                for (int i = 0; i < 16; ++i) request.ref_frame[i] = frame[i];
                for (int i = 0; i < 3; ++i) request.ref_pos[i] = frame[12 + i];
                request.ref_frame_valid = true;
            }
        }
    }
    if (!request.has_ref_pos) return;

    std::optional<SpawnPlacementUnits> placement_world;
    if constexpr (bsp::kSpawnNewPlacementBound) {
        // 00948CC0 reads each member's class +A0h `Length` / +A4h `Width`:
        // VehicleClass[Type] from the installed table.
        // VehicleClass[Type] through read_vehicle_class_row; an absent key
        // reads as 0, as the class field's zero default does.
        for (bsp::SpawnNewGroupMember& m : request.members) {
            const GameVehicleClassRow row = read_vehicle_class_row(m.type_class_id);
            if (!row.found) continue;
            m.class_length_a0 = row.length;
            m.class_width_a4 = row.width;
            m.has_class_extents = true;
        }
        placement_world.emplace(script_orders_->units());
        bsp::set_spawn_placement_world(&*placement_world);
    }
    struct ClearWorld {
        ~ClearWorld() { bsp::set_spawn_placement_world(nullptr); }
    } clear_world;

    std::vector<std::uint32_t> made;
    made.reserve(request.members.size());
    for (std::size_t i = 0; i < request.members.size(); ++i) {
        const bsp::SpawnNewGroupMember& member = request.members[i];
        if (member.type_class_id <= 0) break;
        const bsp::SpawnNewFrame frame = bsp::spawn_member_frame_0094a140(request, i);
        if constexpr (bsp::kSpawnNewPlacementBound) {
            if (i == 0 && request.exclude.present) {
                if (const bsp::SpawnPlacementResult* p = bsp::last_spawn_placement_0094a140()) {
                    log_.notef("  spawn placement 0094a140: serial %u attempt %u candidates %d "
                        "accepted %d angle %.4f distance %.1f entities %d nearest %.1f",
                        request.serial, request.attempts, p->candidates, p->accepted ? 1 : 0,
                        static_cast<double>(p->angle), static_cast<double>(p->distance),
                        p->entities, static_cast<double>(p->nearest));
                }
            }
        }
        // 00949300's `bVar4 &= ...`: no candidate passed, so nothing is made
        // and 0094C5AD requeues the record.
        if (frame.refused) break;

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
        // Packet cc9_spawn_new_shipyard. 00948519 asks the class vtable+18h(6);
        // the eight ship leaves answer yes (00963B70 Destroyer, 00963BF0
        // Cruiser, 00963C80 LandingShip, 00963D00 Cargo, 00963D80 BattleShip,
        // 00963E10 Submarine, 00963E90 TorpedoBoat, 00963F10 MotherShip each
        // compare 6), every plane and land leaf no. Kinds 7..0Eh are exactly
        // those eight (bsp::VehicleClassKind).
        // SUBSTITUTION, labelled: the image calls the class's vtable+28h(0)
        // with no scene creator in between; this process reaches the same
        // allocation through a DestroyerGen (07h) record, whose creator 004F0520
        // is the one that makes load-time ships here. The units host keys the
        // class on `type_id`, so a Cruiser or a Cargo is still that class.
        bool surface = false;
        if constexpr (bsp::kSpawnNewEntityRefPosBound) {
            const GameVehicleClassRow class_row = read_vehicle_class_row(member.type_class_id);
            const bsp::VehicleClassDescriptorRow* kind = class_row.found
                ? bsp::vehicle_class_kind_row(class_row.type.c_str()) : nullptr;
            surface = kind != nullptr
                && static_cast<int>(kind->kind) >= static_cast<int>(bsp::VehicleClassKind::Destroyer)
                && static_cast<int>(kind->kind) <= static_cast<int>(bsp::VehicleClassKind::TorpedoBoat);
            if (surface) {
                record.class_name = "DestroyerGen";
                record.class_id = 0x07;
            }
        }
        record.type_id = member.type_class_id;
        record.party = request.party;
        // Packet cc9_plane_scene_equipment: 00944210's 0043D8F0 merges the member
        // table over the seeded bag, so an authored `Equipment` is the bag's and
        // an absent one leaves the key unset (read as 0, 007CDFF0's arm).
        record.bag_equipment = member.equipment;
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
        const std::size_t units_before = script_orders_->units().count();
        const std::uint32_t entity
            = script_orders_->create_unit_from_scene_record_0046db4b(record);
        if (bsp::game::SceneSpawnPoolEntry* entry
                = bsp::game::scene_spawn_pool().find(record.name)) {
            entry->spawned = entity != 0u;
            entry->entity_id = static_cast<int>(entity);
        }
        if (entity == 0u) break;
        if constexpr (kSEntityInitAllBound) {
            // Packet cc9_sentity_init_all. Pushed at construction; the attach
            // is 0094879A's InitAll after the whole member loop, so every
            // member squadron is attached before any plane (each squadron's
            // pass A appends its wing to the tail).
            if (surface) {
                route_push_entity(static_cast<int>(entity), record.name,
                    member.type_class_id);
                // The member's property bag, which 009486B6 hands the entity at
                // +C0h and pass A reads as a generated entity's: 009420A0 seeds
                // Party = the request's party (009420AC) and Race = 2 for party 0,
                // else 1 (009420B9..009420C1, NEG/SBB/ADD 2); 00944210's
                // 0043D8F0 then merges the member table over it, so an authored
                // `Race` wins. The host reads an absent `Race` as 0, so 0 takes
                // the default (labelled: a Race of 0 authored on purpose would be
                // replaced; no reference call site authors one).
                if (PendingEntity* node = find_pending(static_cast<int>(entity))) {
                    node->party = request.party;
                    node->race = member.race > 0 ? member.race
                                                 : (request.party == 0 ? 2 : 1);
                    node->generated_party = true;
                }
            } else {
                route_push_squadron(static_cast<int>(entity), record.name,
                    member.type_class_id, units_before);
            }
        } else {
            if (!attach_created_entity_00928a00(static_cast<int>(entity), record.name,
                                                member.type_class_id)) {
                break;
            }
            attach_wing_member_tables(units_before, entity, member.type_class_id);
        }
        if (surface && !made.empty()) {
            // 009486C8..0094870E: a kind-6 member after the first (member index
            // [ESP+10h] > 0) calls 0077C8D0 with ECX = this member and the first
            // entity of record+CCh as the leader.
            const bsp::FormationJoinOutcome joined = bsp::entity_join_formation(
                *script_orders_, reinterpret_cast<void*>(static_cast<std::uintptr_t>(entity)),
                reinterpret_cast<void*>(static_cast<std::uintptr_t>(made.front())));
            log_.notef("  spawn queue 0094c490: serial %u member %zu joins member 1's "
                "formation (0094870E -> 0077C8D0), outcome %d", request.serial, i + 1,
                static_cast<int>(joined));
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
    if constexpr (kSEntityInitAllBound) {
        // 00948798 XOR CL,CL / 0094879A CALL 00925F20, after the member loop.
        run_sentity_init_all_00925f20(false, 0x0094879au);
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
    request.fulfilled = true;  // 009487AD MOV byte ptr [EBX + 0xc0],0x1
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

void GameMissionLuaHost::note_entity_status(const bsp::MissionLuaBinding& binding,
    bool resolved) {
    // The status of an entity-returning row, recorded from the OUTCOME because
    // note_native_call cannot know it yet. GameHostLog's record is sticky on
    // first insert, so the first call here decides the row for the run, and both
    // arms print at most one line (it prints only when calls == 1).
    char address[16];
    std::snprintf(address, sizeof(address), "%08lx",
        static_cast<unsigned long>(binding.address));
    char label[96];
    std::snprintf(label, sizeof(label), "MissionLuaNative::%s", binding.name);
    if (resolved) {
        log_.implemented(label, address);
    } else {
        log_.unimplemented(label, address);
    }
}

void GameMissionLuaHost::report_spawn_queue() {
    if (summary_.spawn_new_calls == 0 && summary_.spawn_new_rejected == 0) return;
    log_.notef("summary SpawnNew 0094c480 calls=%llu rejected=%llu queued=%llu "
        "attempts=%llu fulfilled=%llu requeued=%llu units=%llu callbacks=%llu "
        "callback_missing=%llu still_queued=%zu interval=%.3f clock=%.1f "
        "wing_member_tables=%llu",
        summary_.spawn_new_calls, summary_.spawn_new_rejected,
        summary_.spawn_new_queued, summary_.spawn_new_attempts,
        summary_.spawn_new_fulfilled, summary_.spawn_new_requeued,
        summary_.spawn_new_units, summary_.spawn_new_callbacks,
        summary_.spawn_new_callback_missing, bsp::spawn_request_queue().size(),
        static_cast<double>(spawn_attempt_delay_),
        static_cast<double>(spawn_world_clock_), summary_.wing_member_tables);
    if constexpr (kLuaSpawnNewIdQueriesBound) {
        log_.notef("summary SpawnNewIDIsRequested/SpawnNewIDRemove requested=%llu true=%llu "
            "removes=%llu removed=%llu (00945850/00945A20, packet cc9_spawn_new_id_queries)",
            g_spawn_id_census.requested, g_spawn_id_census.answered_true,
            g_spawn_id_census.removes, g_spawn_id_census.removed);
    }
}

std::uint32_t GameMissionLuaHost::create_squadron(const bsp::AirOpsSquadronRequest& request) {
    // 006C74C6 calls 006C5050 and 006C74FF stores what comes back in slot+28h.
    // The unit itself is the script-orders host's to make, because that host owns
    // the units host; this adds the table slot the script indexes it by.
    if (script_orders_ == nullptr) return 0u;
    std::string name;
    std::int32_t wing = 0;
    const std::size_t units_before = script_orders_->units().count();
    const std::uint32_t entity = script_orders_->create_air_ops_squadron_006c5050(
        request.type, request.wing_count, request.equipment, request.home_base, name,
        wing);
    if (entity == 0u) return 0u;
    if constexpr (kSEntityInitAllBound) {
        // Packet cc9_sentity_init_all. 006C5050 returns the squadron whatever
        // its attach will do; the attach is the next InitAll: 0089E613 when a
        // LaunchSquadron call started it, row 12 of the step otherwise.
        route_push_squadron(static_cast<int>(entity), name,
            static_cast<int>(request.type), units_before);
        // 006C5101..006C5113: `Skill` = owner->vtable[12Ch](), the owner's
        // unit+390h at this moment. The owner is the unit `HomeBase` names.
        int owner_skill = -1;
        const GameUnitsHost& units = script_orders_->units();
        for (std::size_t i = 0; i < units.count(); ++i) {
            const GameUnitRow* row = units.unit_row(i);
            if (row != nullptr && row->name == request.home_base) {
                owner_skill = units.skill_level(i);
                break;
            }
        }
        if (PendingEntity* node = find_pending(static_cast<int>(entity))) {
            node->launch_skill = owner_skill;
        }
        log_.notef("air ops launch skill: squadron=%s owner=%s skill=%d applied=%d "
            "(006C5101, packet cc9_carrier_launch_skill)", name.c_str(),
            request.home_base.c_str(), owner_skill, kCarrierLaunchSkillBound ? 1 : 0);
        ++summary_.air_ops_squadrons_created;
        return entity;
    }
    if (!attach_created_entity_00928a00(static_cast<int>(entity), name,
                                        static_cast<int>(request.type))) {
        log_.notef("  air ops squadron %s: unit %u exists but no `thisTable` slot was "
            "made, so `squadron` would name an id the bindings cannot resolve",
            name.c_str(), entity);
        return 0u;
    }
    attach_wing_member_tables(units_before, entity, static_cast<int>(request.type));
    ++summary_.air_ops_squadrons_created;
    return entity;
}

// Packet cc9_mission_end, docs/MISSION_END.md item 2. In the image each plane of a
// squadron is an entity of its own class (MPlaneDiveBomber, MPlaneTorpedoBomber,
// ...), whose vtable slot 39 (+9Ch) is the attach override 007CDF20 -> 0077E830 ->
// 00928A00. The squadron's own slot 39, 007F4580, spawns the planes, and the
// pending-entity pass 00925F20 dispatches +9Ch on each new node at 00926054. So
// every wing member carries a `thisTable` slot. The host attached only the
// squadron's leader unit. This gives each other unit the creator just made its
// slot, keyed by its own unit id (index + 1), as the leader's is.
void GameMissionLuaHost::attach_wing_member_tables(std::size_t units_before,
    std::uint32_t leader_entity, int class_index) {
    if (script_orders_ == nullptr) return;
    const GameUnitsHost& units = script_orders_->units();
    for (std::size_t index = units_before; index < units.count(); ++index) {
        const std::uint32_t id = static_cast<std::uint32_t>(index + 1);
        if (id == leader_entity) continue;
        const GameUnitRow* const row = units.unit_row(index);
        if (row == nullptr) continue;
        if (attach_created_entity_00928a00(static_cast<int>(id), row->name, class_index)) {
            ++summary_.wing_member_tables;
        }
    }
}

bool GameMissionLuaHost::attach_created_entity_00928a00(int entity_id,
    const std::string& name, int class_index, bool seed_class, bool findable) {
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
    // `Class` is 009292B0's third field, not 00928A00's: a stand-in written here
    // unless the InitAll walk's pass B writes it (kSEntityInitThisTableStepsBound).
    if (seed_class && class_index >= 0) {
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
    // `findable`: only the entities whose world bucket 0088B1B0 walks enter
    // the name index (packet cc_lua_find_entity); the load markers carry it.
    if (findable && !name.empty()) scene_entity_ids_[name] = entity_id;
    ++summary_.self_table_entities;
    return true;
}

// ---------------------------------------------------------------------------
// Packet cc9_sentity_init_all: 00925F20 over this process's pending list.
// ---------------------------------------------------------------------------
//
// One method per native call site of bsp::SEntityInitAllHost. What each pass
// does per class (docs/CONSTRUCT_WORLD.md section 17):
//   A  +9Ch  the `thisTable` attach, 00928A00 through each class's override;
//            the plane squadron's 007F4580 also constructs the wing.
//   B  +A0h  ships 00822C20, carriers 007593D0, squadrons 007F1FE0, planes
//            007D5D20, airfields 006D3C10: the scene-property readers. This
//            process does parts of them at creation (create_units's StartSpeed
//            arm and wake-ring fill, the scene-contents deck pass), so the pass
//            is a NAMED RECORD here, not a second run of those parts.
//   C  +A4h  ships 0081F980, carriers 00758210 (0081F980 then +11A8h = 1),
//            squadrons 007F4BA0, planes 007C9770 (thisTable.SquadronID, a
//            disable for +900h in {0,1}, 007C5AC0(-1.0)), airfields 006D5220:
//            a NAMED RECORD for the same reason; 0081F980's section binding is
//            the gunnery host's at creation.
//   C' the start branch: kind 2 at holder+4h only. This process's holders are
//            the kind-1 scene property bag (00922E35), and the kind-2 objects
//            (vtable 00D03754) come only from the session message paths
//            00768530 and 00774E30, the latter gated on session mode 1 or 2.
//            Exact: the branch is never taken in single player.
//   D  5Ch(2) then 0077F090, which returns at 0077F0A4 unless
//            [00E188A8]+1FE4h == 1; it is 0 in this process. Exact.
//   E  the holder at +C0h released through its slot 0 with 1. This process
//            keeps no holder object on the entity (the property bag stays on
//            the spawn-pool record), so the release is a NAMED RECORD.
namespace {
// Packet cc9_init_identity_gaps. 00925E1D MOV dword ptr [ESI+54h],2: the base
// entity's Party before any authored value (PARTY_NEUTRAL in this
// installation's luamw_init.lua).
constexpr int kEntityDefaultParty00925e1d = 2;

// The `Type` 00928100 writes, 00E0CD80[+C4h], for the classes whose slot +A4h is
// 009295B0 (section 1.3). +C4h is the scene class id each constructor stores:
// 004E59AD 41h, 004E5A0D 42h, 004E5A6D 43h, 0047B6C8 47h, 004E58B8 4Ah,
// 004E7F31 5Bh, 004E800E 5Ch. Strings read from the image at 00D18CCC..00D18E68.
// nullptr: the class has its own pass C (or no default one was found).
const char* default_pass_c_type_name(int class_id) noexcept {
    switch (class_id) {
    case 0x41: return "NAVPOINT";        // 00D18E68
    case 0x42: return "MOVIECAMPOS";     // 00D18E5C
    case 0x43: return "MOVIECAMLOOKAT";  // 00D18E4C
    case 0x47: return "GAMEPATH";        // 00D18E1C
    case 0x4A: return "CAMERAPATH";      // 00D18DFC
    case 0x5B: return "SIMPLEEFF";       // 00D18CD8
    case 0x5C: return "PERIODEFF";       // 00D18CCC
    default: return nullptr;
    }
}
}  // namespace

class GameMissionLuaInitAllBinding final : public bsp::SEntityInitAllHost {
public:
    explicit GameMissionLuaInitAllBinding(GameMissionLuaHost& host) : host_(host) {}

    std::int32_t pending_count_00f899d4() override {
        return static_cast<std::int32_t>(host_.pending_entities_.size());
    }
    std::size_t pending_size_00f899d0() override { return host_.pending_entities_.size(); }
    void* pending_at_00f899d0(std::size_t index) override {
        return &host_.pending_entities_[index];
    }
    // The name accessor's result is discarded at all four sites.
    void entity_name_vcall_10(void*) override {}
    void loading_progress_report_0057c1a0(int, float) override {
        // The loading bar's singleton [00E194B4]; this process draws no bar mid
        // mission.
        host_.log_.unimplemented("SEntity::InitAll loading_progress", "0057c1a0");
    }
    void entity_attach_lua_self_vcall_9c(void* entity) override {
        GameMissionLuaHost::PendingEntity& node = at(entity);
        ++host_.summary_.init_all_entities;
        if (node.load_scene) {
            // Packet cc9_load_time_init_all: what the load attach did per
            // instance besides the slot. The scene pass's decks are keyed by
            // the authored name (docs/AIROPS_LOAD_FROM_SCENE.md).
            bsp::air_ops_decks().bind_entity_id(node.entity_id, node.name);
            host_.load_attached_.insert(node.entity_id);
        }
        if (host_.attach_created_entity_00928a00(node.entity_id, node.name,
                node.class_index, !kSEntityInitThisTableStepsBound, node.findable)) {
            host_.init_all_attached_.insert(node.entity_id);
            if (node.wing_member) ++host_.summary_.wing_member_tables;
            if ((node.load_scene || node.generated_party) &&
                (node.party >= 0 || node.race >= 0)) {
                host_.write_party_race_fields(node.entity_id, node.party, node.race);
                if (node.generated_party) {
                    ++host_.summary_.generated_party_writes;
                    host_.log_.notef("  generated entity party: thisTable[%d] (\"%s\") Party=%d "
                        "Race=%d written at pass A (packet cc9_generated_entity_party)",
                        node.entity_id, node.name.c_str(), node.party, node.race);
                }
            }
        }
        host_.log_.implemented("SEntity::InitAll pass A attach_self_table", "0092604e");
        if (!node.squadron) return;
        // Packet cc9_load_time_squadron_hooks: the scene read's walk runs
        // before the script-orders host exists, and neither the hook nor the
        // wing marking below needs it; only the legacy wing append does.
        if (!kLoadTimeSquadronHooksBound && host_.script_orders_ == nullptr) return;
        // Packet cc9_wing_construction_lua: the list size before 007F4580.
        const std::size_t pending_before = host_.pending_entities_.size();
        if constexpr (kSquadronPassHooksCalled) {
            // Packet cc9_squadron_pass_hooks_calls: 007F4580, where the image
            // constructs the wing; the append below stands in for it until the
            // units host does (section 9.3).
            if (host_.units_hooks_ != nullptr && node.entity_id > 0) {
                host_.units_hooks_->on_squadron_pass_a_construct_wing(
                    static_cast<std::size_t>(node.entity_id - 1));
                host_.log_.implemented("SEntity::InitAll pass A squadron_construct_wing hook",
                    "007f4580");
            }
        }
        if constexpr (kWingConstructionLuaActive) {
            // Every node the hook's constructions pushed (00928760 CALL
            // 00926BE0 per plane) is this squadron's wing: 007F4B49 stores the
            // leader in the plane's +9D4h, and the append below set the same
            // three fields. The walk reaches them in this pass A because it
            // re-reads the list's size.
            for (std::size_t i = pending_before; i < host_.pending_entities_.size(); ++i) {
                GameMissionLuaHost::PendingEntity& plane = host_.pending_entities_[i];
                if (plane.squadron || plane.entity_id == node.entity_id) continue;
                plane.wing_member = true;
                plane.squadron_id = node.entity_id;
                plane.class_index = node.class_index;
                // Packet cc9_generated_wing_party: 007F491A, the squadron's
                // descriptor in the plane's +C0h.
                if (kGeneratedWingPartyBound && node.generated_party) {
                    plane.party = node.party;
                    plane.race = node.race;
                    plane.generated_party = true;
                }
                ++host_.summary_.init_all_wing_marked;
            }
            return;   // the wing append below is retired
        }
        // 007F4580 constructs each plane of the wing, and each construction
        // reaches 00928760 CALL 00926BE0: the planes join the tail now, after
        // every node already pending, and this same pass reaches them.
        const int leader = node.entity_id;
        const int class_index = node.class_index;
        const std::size_t first = node.units_before;
        const std::size_t end = node.units_end;
        if (host_.script_orders_ == nullptr) return;
        const GameUnitsHost& units = host_.script_orders_->units();
        for (std::size_t index = first; index < end && index < units.count(); ++index) {
            const int id = static_cast<int>(index) + 1;
            if (id == leader) continue;
            const GameUnitRow* const row = units.unit_row(index);
            if (row == nullptr) continue;
            if constexpr (kPendingListDedupBound) {
                // Packet cc9_pending_list_dedup: a plane some other pusher
                // already queued is not queued twice.
                if (host_.find_pending(id) != nullptr) {
                    ++host_.summary_.dedup_wing_append_skipped;
                    continue;
                }
            }
            GameMissionLuaHost::PendingEntity plane;
            plane.entity_id = id;
            plane.name = row->name;
            plane.class_index = class_index;
            plane.wing_member = true;
            plane.squadron_id = leader;  // 007F4B49 MOV [EBX+9D4h],ESI
            // Packet cc9_generated_wing_party: 007F491A, the squadron's
            // descriptor in the plane's +C0h.
            if (kGeneratedWingPartyBound && node.generated_party) {
                plane.party = node.party;
                plane.race = node.race;
                plane.generated_party = true;
            }
            host_.pending_entities_.push_back(std::move(plane));  // `node` stays valid
            ++host_.summary_.init_all_wing_appended;
        }
    }
    void entity_init_second_vcall_a0(void* entity) override {
        host_.log_.unimplemented("SEntity::InitAll pass B init_slot_a0", "00926110");
        if constexpr (kCarrierLaunchSkillBound) {
            // Packet cc9_carrier_launch_skill: 007F1FE0's kind-1 arm reads the
            // bag skill at 007F211B and ends with vtable[128h](skill) at
            // 007F226E; the squadron's 007ECF80 re-skills every wing member.
            const GameMissionLuaHost::PendingEntity& node = at(entity);
            if (node.squadron && node.launch_skill >= 0 && node.launch_skill <= 5
                && host_.units_hooks_ != nullptr && node.entity_id > 0) {
                host_.units_hooks_->set_skill_level_007b8ae0(
                    static_cast<std::size_t>(node.entity_id - 1), node.launch_skill);
                const bsp::PlaneSquadronHostRecord* sqn = bsp::plane_squadron_registry()
                    .find_by_member_unit(static_cast<std::size_t>(node.entity_id - 1));
                host_.log_.notef("air ops launch skill applied: squadron=%s skill=%d "
                    "members=%zu (007F226E)", node.name.c_str(), node.launch_skill,
                    sqn != nullptr ? sqn->member_units.size() : static_cast<std::size_t>(0));
            }
        }
        if constexpr (kSEntityInitThisTableStepsBound) {
            // Packet cc9_init_attach_order. Every class this process pushes
            // reaches 009292B0 in its pass B: the squadron 007F1FE0 at 007F218E
            // (the kind-1 arm 007F2101, which every single-player holder
            // takes), ships and carriers through 00822C20 -> 00955420, planes
            // through 007D5D20 -> 00955420 (both at 00955498). ClassID is the
            // squadron's first plane's descriptor +70h at 007F2181, the
            // entity's own at 00955420; both are the node's VehicleClass row.
            const GameMissionLuaHost::PendingEntity& node = at(entity);
            // A scene marker (no VehicleClass row) has no 009292B0 in its pass B.
            if (node.class_index < 0) return;
            if (host_.bind_lua_class_009292b0(node.entity_id, node.class_index, node.name)) {
                ++host_.summary_.init_all_class_bound;
            }
            host_.log_.implemented("SEntity::InitAll pass B bind_lua_class", "009292b0");
        }
    }
    void entity_init_third_vcall_a4(void* entity) override {
        host_.log_.unimplemented("SEntity::InitAll pass C init_slot_a4", "009261a1");
        if constexpr (kSquadronPassHooksCalled) {
            // Packet cc9_squadron_pass_hooks_calls: the squadron's pass C
            // 007F4BA0 issues its initial command at 007F4E9E.
            const GameMissionLuaHost::PendingEntity& squadron = at(entity);
            if (squadron.squadron && host_.units_hooks_ != nullptr && squadron.entity_id > 0) {
                host_.units_hooks_->on_squadron_pass_c_initial_command(
                    static_cast<std::size_t>(squadron.entity_id - 1));
                host_.log_.implemented("SEntity::InitAll pass C squadron_initial_command hook",
                    "007f4e9e");
            }
        }
        if constexpr (kSEntityInitThisTableStepsBound) {
            // Packet cc9_init_attach_order. 007C9770: when plane+9D4h is set,
            // thisTable[plane].SquadronID = that squadron's +174h id. Only
            // wing planes are separate plane nodes here; the squadron's node
            // stands for the squadron and its leader plane at once, and the
            // image's squadron table carries no SquadronID, so it gets none.
            const GameMissionLuaHost::PendingEntity& node = at(entity);
            if (node.class_index < 0 && node.marker_class_id >= 0) {
                // Packet cc9_load_time_init_all: a load marker of a class whose
                // slot +A4h is 009295B0 takes 009297E4 CALL 00928100 (section 8).
                const char* const type_name
                    = default_pass_c_type_name(node.marker_class_id);
                if (type_name != nullptr) {
                    const int party = node.marker_authored_party >= 0
                        ? node.marker_authored_party : kEntityDefaultParty00925e1d;
                    if (host_.mirror_identity_00928100(node.entity_id, party, node.name,
                            type_name)) {
                        ++host_.summary_.init_all_identity_mirrored;
                    }
                    host_.log_.implemented("SEntity::InitAll pass C mirror_identity", "00928100");
                }
                return;
            }
            if (node.wing_member && node.squadron_id > 0) {
                if (host_.set_plane_squadron_id_007c97e3(node.entity_id, node.squadron_id)) {
                    ++host_.summary_.init_all_squadron_ids;
                }
                host_.log_.implemented("SEntity::InitAll pass C plane squadron_id", "007c97e3");
            }
        }
    }
    bool entity_descriptor_kind_is_initial_state(void*) override {
        // 009261AD: the holder's +4h against 2. Kind 1 here, see above.
        host_.log_.implemented("SEntity::InitAll pass C start_state_branch", "009261ad");
        return false;
    }
    // Unreachable while the kind test above answers false.
    bool entity_descriptor_start_enabled_3c(void*) override { return false; }
    bool entity_flag_5e(void*) override { return false; }
    bool entity_flag_5c(void*) override { return false; }
    void entity_set_flag_5c(void*, bool) override {}
    void entity_enable_vcall_68(void*) override {}
    void entity_disable_vcall_6c(void*) override {}
    void* entity_first_child_48(void*) override { return nullptr; }
    void* entity_next_child_44(void*) override { return nullptr; }
    void scene_node_enable_00922f30(void*) override {}
    void scene_node_disable_00922f80(void*) override {}
    bool entity_session_gate_vcall_5c(void*, int argument) override {
        // Every class this process pushes (ships, carriers, squadrons, planes)
        // lists kind 2 in its +5Ch test (006DFE90, 007DDA80, 007EFB00 ...).
        return argument == bsp::kEntitySessionGateArgument;
    }
    void session_register_0077f090(void*) override {
        // 0077F09D CMP [ECX+1FE4h],1 / JNZ 0077F0D7: the peer walk needs a
        // session mode of 1; single player is 0, so the routine returns.
        host_.log_.implemented("SEntity::InitAll pass D session_register", "0077f090");
    }
    void entity_release_spawn_descriptor(void*) override {
        if constexpr (kSEntityInitPassEReleaseBound) {
            // Packet cc9_init_pass_e_property_bag. 00926301..00926319: the
            // holder's slot 0 with 1, then +C0h = 0. The holder owns the clone
            // 00922E2D made; this process's copy is the creator's temporary
            // record, which no reader holds past the creator call
            // (docs/SENTITY_INIT_ATTACH_ORDER.md section 7). So after this
            // pass no authored value is read from the entity's copy.
            ++host_.summary_.init_all_holders_released;
            host_.log_.implemented("SEntity::InitAll pass E release_spawn_holder", "00926317");
        } else {
            host_.log_.unimplemented("SEntity::InitAll pass E release_spawn_holder", "00926317");
        }
    }
    void set_init_active_flag_00f899a5(bool value) override {
        host_.init_active_00f899a5_ = value;
    }
    void log_enum_count_004b8490(std::int32_t pending_count) override {
        // 004B8490 formats into the routine's own 100h buffer; the text goes
        // nowhere the process shows, so the first few land in this log instead.
        if (host_.summary_.init_all_nonempty <= 8) {
            host_.log_.notef("  SEntity::InitAll 00925f20: " "INIT,ENUM:%d",
                static_cast<int>(pending_count));
        }
    }
    void clear_pending_list_00926335() override { host_.pending_entities_.clear(); }
    void recon_force_refresh_00807a50() override {
        host_.log_.unimplemented("SEntity::InitAll recon_force_refresh", "00807a50");
    }

private:
    static GameMissionLuaHost::PendingEntity& at(void* entity) {
        return *static_cast<GameMissionLuaHost::PendingEntity*>(entity);
    }
    GameMissionLuaHost& host_;
};

// Packet cc9_init_identity_gaps. 00928100 (docs/SENTITY_INIT_ATTACH_ORDER.md
// sections 1.3 and 8): thisTable[key] gets `Race` = +58h and `Party` = +54h
// (both numbers through 006B8260), `Name` = vt+10h when +154h is not 0, and
// `Type` = 00E0CD80[+C4h] (006B8360). `Race` is not written here: the record
// carries no authored Race (contract, section 8).
bool GameMissionLuaHost::mirror_identity_00928100(int entity_id, int party,
    const std::string& name, const char* type_name) {
    if (state_ == nullptr || entity_id <= 0) return false;
    const int base = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (!lua_istable(state_, -1)) {
        ::lua_settop(state_, base);
        return false;
    }
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, entity_id);
    lua_getfield(state_, -1, key);
    if (!lua_istable(state_, -1)) {
        ::lua_settop(state_, base);
        return false;
    }
    ::lua_pushnumber(state_, static_cast<lua_Number>(party));
    lua_setfield(state_, -2, "Party");
    if (!name.empty()) {
        ::lua_pushstring(state_, name.c_str());
        lua_setfield(state_, -2, "Name");
    }
    ::lua_pushstring(state_, type_name);
    lua_setfield(state_, -2, "Type");
    ::lua_settop(state_, base);
    return true;
}

// Packet cc9_init_attach_order. 009292B0 (docs/NATIVE_UNIT_CLASS_LUA.md): the
// entity's own `thisTable` slot through 00927B40, then `ClassID` through
// 00B67460 (an integer), `Name` through 00B66790, and `Class` through 00B675D0
// from VehicleClass[ClassID] (00B67980 globals, 00B67800, 00B67720). The row is
// assigned whatever it is, nil included, as 00B675D0 would.
bool GameMissionLuaHost::bind_lua_class_009292b0(int entity_id, int class_index,
    const std::string& name) {
    if (state_ == nullptr || entity_id <= 0 || class_index < 0) return false;
    const int base = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (!lua_istable(state_, -1)) {
        ::lua_settop(state_, base);
        return false;
    }
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, entity_id);
    lua_getfield(state_, -1, key);
    if (!lua_istable(state_, -1)) {
        ::lua_settop(state_, base);
        return false;
    }
    lua_pushinteger(state_, class_index);
    lua_setfield(state_, -2, "ClassID");
    ::lua_pushstring(state_, name.c_str());
    lua_setfield(state_, -2, "Name");
    lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (lua_istable(state_, -1)) {
        lua_rawgeti(state_, -1, class_index);
        lua_setfield(state_, -3, "Class");
    }
    ::lua_settop(state_, base);
    return true;
}

// Packet cc9_init_attach_order. 007C97C6..007C9805: 00927B40 gives the plane's
// slot, and 00B67460 stores the u16 at squadron+174h under "SquadronID"
// (00D05B80).
bool GameMissionLuaHost::set_plane_squadron_id_007c97e3(int plane_id, int squadron_id) {
    if (state_ == nullptr || plane_id <= 0) return false;
    const int base = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (!lua_istable(state_, -1)) {
        ::lua_settop(state_, base);
        return false;
    }
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, plane_id);
    lua_getfield(state_, -1, key);
    if (!lua_istable(state_, -1)) {
        ::lua_settop(state_, base);
        return false;
    }
    lua_pushinteger(state_, squadron_id);
    lua_setfield(state_, -2, "SquadronID");
    ::lua_settop(state_, base);
    return true;
}

GameMissionLuaHost::PendingEntity* GameMissionLuaHost::find_pending(int entity_id) {
    for (PendingEntity& node : pending_entities_) {
        if (node.entity_id == entity_id) return &node;
    }
    return nullptr;
}

// Packet cc9_load_time_init_all. The load attach's `Party`/`Race` fields for
// a SceneEntity that carries them (00928F50's mirror; every caller today
// passes -1, so this writes nothing yet).
void GameMissionLuaHost::write_party_race_fields(int entity_id, int party, int race) {
    if (state_ == nullptr) return;
    const int base = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, entity_id);
    if (lua_istable(state_, -1)) {
        lua_getfield(state_, -1, key);
        if (lua_istable(state_, -1)) {
            if (party >= 0) {
                lua_pushinteger(state_, party);
                lua_setfield(state_, -2, "Party");
            }
            if (race >= 0) {
                lua_pushinteger(state_, race);
                lua_setfield(state_, -2, "Race");
            }
        }
    }
    ::lua_settop(state_, base);
}

// Packet cc9_load_time_init_all. 0046CF40's creators push each instance from
// its base constructor (00928760); the scene read then runs InitAll once, at
// the first of its four sites (the [ESP+13h] latch, 0046EB50). Here the
// instances already exist (create_units, the markers), so the pushes are made
// in their order now: the units, then the markers, as the load attach took
// them. The dedup rules (section 9) keep one node per id if create_units has
// pushed already.
std::size_t GameMissionLuaHost::run_scene_load_init_all_0046eb4b(
    const std::vector<SceneEntity>& entities) {
    if (state_ == nullptr || entities.empty()) return 0;
    const unsigned long long before = summary_.init_all_entities;
    for (const SceneEntity& entity : entities) {
        if (entity.class_index >= 0) {
            // A scene unit: create_units pushed it (docs/CONSTRUCT_WORLD.md 26).
            route_push_entity(entity.id, entity.name, entity.class_index);
        } else {
            // A marker: its constructor is the scene-contents host's, which
            // does not push yet (section 10.2's contract).
            push_pending_entity_00926be0(entity.id, entity.name, entity.class_index);
        }
        if (PendingEntity* const node = find_pending(entity.id)) {
            node->load_scene = true;
            node->findable = entity.findable;
            node->marker_class_id = entity.marker_class_id;
            node->marker_authored_party = entity.marker_authored_party;
            node->party = entity.party;
            node->race = entity.race;
            ++summary_.load_init_all_pushes;
            if constexpr (kLoadTimeSquadronHooksBound) {
                // Packet cc9_load_time_squadron_hooks. A PlaneSquadronGen row
                // the scene read constructed: its registry record (not an
                // air-ops launch) names this unit as the fused leader, slot 0.
                // create_units resolved the members before this walk.
                // SUBSTITUTION, labelled: the host has no PlaneSquadron object
                // to test for vtable 00D087C0, so the registry stands in.
                if (entity.class_index >= 0 && entity.id > 0) {
                    const std::size_t index = static_cast<std::size_t>(entity.id - 1);
                    for (const bsp::PlaneSquadronHostRecord& record :
                         bsp::plane_squadron_registry().records()) {
                        if (record.from_air_ops_launch || record.squadron_unit != index) {
                            continue;
                        }
                        node->squadron = true;
                        ++summary_.load_squadron_nodes;
                        break;
                    }
                }
            }
        }
    }
    if constexpr (kLoadWingSquadronIdBound) {
        // Packet cc9_load_wing_squadron_id. Every plane 007F4580 constructs
        // carries its squadron at +9D4h (007F4B49), so its pass C (007C9770,
        // 007C97E8) writes SquadronID. SUBSTITUTION, labelled: the host fuses
        // slot 0 with the squadron node, which gets none (as for a mission-time
        // wing); members 1.. are marked here, after every node is pushed, since
        // a wing plane may precede its leader in the load list. class_index is
        // left as create_units pushed it.
        for (const bsp::PlaneSquadronHostRecord& record :
             bsp::plane_squadron_registry().records()) {
            if (record.from_air_ops_launch || record.squadron_unit == bsp::kPlaneSquadronNoUnit) {
                continue;
            }
            const PendingEntity* const leader =
                find_pending(static_cast<int>(record.squadron_unit) + 1);
            if (leader == nullptr || !leader->load_scene || !leader->squadron) continue;
            for (std::size_t m = 1; m < record.member_units.size(); ++m) {
                const std::size_t unit = record.member_units[m];
                if (unit == bsp::kPlaneSquadronNoUnit) continue;
                PendingEntity* const plane = find_pending(static_cast<int>(unit) + 1);
                if (plane == nullptr || !plane->load_scene || plane->squadron) continue;
                plane->wing_member = true;
                plane->squadron_id = leader->entity_id;
                ++summary_.load_wing_marked;
            }
        }
    }
    run_sentity_init_all_00925f20(false, 0x0046eb4bu);
    return static_cast<std::size_t>(summary_.init_all_entities - before);
}

void GameMissionLuaHost::route_push_entity(int entity_id, const std::string& name,
    int class_index) {
    if constexpr (kRoutePushesRetiredBound) {
        if (find_pending(entity_id) != nullptr) {
            ++summary_.route_pushes_retired;
            return;
        }
        ++summary_.route_fallback_pushes;
    }
    push_pending_entity_00926be0(entity_id, name, class_index);
}

void GameMissionLuaHost::route_push_squadron(int entity_id, const std::string& name,
    int class_index, std::size_t units_before) {
    if constexpr (kRoutePushesRetiredBound) {
        // The squadron's own push is create_units's; this call marks the node
        // a squadron and records its wing range (the dedup upgrade), which pass
        // A's wing append needs.
        if (find_pending(entity_id) != nullptr) {
            ++summary_.route_squadron_annotations;
        } else {
            ++summary_.route_fallback_pushes;
        }
    }
    push_pending_squadron_00926be0(entity_id, name, class_index, units_before);
}

void GameMissionLuaHost::push_pending_entity_00926be0(int entity_id,
    const std::string& name, int class_index) {
    if constexpr (kPendingListDedupBound) {
        // Packet cc9_pending_list_dedup. One node per constructed entity, as
        // 00928760's single push gives the image.
        if (find_pending(entity_id) != nullptr) {
            ++summary_.dedup_skipped_pending;
            return;
        }
        if (init_all_attached_.count(entity_id) != 0 || load_attached_.count(entity_id) != 0) {
            ++summary_.dedup_skipped_attached;
            return;
        }
    }
    PendingEntity node;
    node.entity_id = entity_id;
    node.name = name;
    node.class_index = class_index;
    pending_entities_.push_back(std::move(node));
    ++summary_.init_all_pushes;
}

void GameMissionLuaHost::push_pending_squadron_00926be0(int entity_id,
    const std::string& name, int class_index, std::size_t units_before) {
    if constexpr (kPendingListDedupBound) {
        // Packet cc9_pending_list_dedup. Never while a walk holds node
        // pointers: pushes come from the routes, outside 00925F20.
        // Packet cc9_wing_construction_lua: with the wing built in pass A the
        // squadron records no wing range and the deferral below has nothing to drop.
        const std::size_t units_end = kWingConstructionLuaActive ? units_before
            : (script_orders_ != nullptr ? script_orders_->units().count() : units_before);
        if (!kWingConstructionLuaActive && !init_active_00f899a5_) {
            // The wing's planes belong at the tail, after the squadron's pass A
            // (007F4580 constructs and pushes them there). A plain node another
            // pusher queued for one of them is dropped; pass A re-appends it.
            for (std::deque<PendingEntity>::iterator it = pending_entities_.begin();
                 it != pending_entities_.end();) {
                const int id = it->entity_id;
                const bool wing = id != entity_id && !it->squadron
                    && id >= static_cast<int>(units_before) + 1
                    && id <= static_cast<int>(units_end);
                if (wing) {
                    it = pending_entities_.erase(it);
                    ++summary_.dedup_wing_deferred;
                } else {
                    ++it;
                }
            }
        }
        if (PendingEntity* const existing = find_pending(entity_id)) {
            existing->squadron = true;
            existing->name = name;
            existing->class_index = class_index;
            existing->units_before = units_before;
            existing->units_end = units_end;
            ++summary_.dedup_squadron_upgrades;
            return;
        }
        if (init_all_attached_.count(entity_id) != 0 || load_attached_.count(entity_id) != 0) {
            ++summary_.dedup_skipped_attached;
            return;
        }
    }
    PendingEntity node;
    node.entity_id = entity_id;
    node.name = name;
    node.class_index = class_index;
    node.squadron = true;
    node.units_before = units_before;
    node.units_end = kWingConstructionLuaActive ? units_before
        : (script_orders_ != nullptr ? script_orders_->units().count() : units_before);
    pending_entities_.push_back(std::move(node));
    ++summary_.init_all_pushes;
}

void GameMissionLuaHost::run_sentity_init_all_00925f20(bool flag, std::uint32_t call_site) {
    ++summary_.init_all_calls;
    if (!pending_entities_.empty()) {
        ++summary_.init_all_nonempty;
        if (summary_.init_all_nonempty <= 8) {
            log_.notef("  SEntity::InitAll 00925f20 at %08x: %zu pending",
                static_cast<unsigned>(call_site), pending_entities_.size());
        }
    }
    GameMissionLuaInitAllBinding binding(*this);
    bsp::sentity_init_all_00925f20(flag, binding);
    log_.implemented("SEntity::InitAll", "00925f20");
}

bool GameMissionLuaHost::init_all_attached(int entity_id) const {
    return init_all_attached_.count(entity_id) != 0;
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

// The classes whose reader is 00927AD0 alone (docs/MISSION_LUA_GETPROPERTY.md
// 9.2): Path 47h, CameraPath 4Ah, NavPoint 41h, MovieCamPos 42h,
// MovieCamLookat 43h, LandingPoint 1Dh, Landscape 44h. They answer
// `unitcommand` but not `reconlevel`.
bool get_property_class_is_base_only(int class_id) noexcept {
    switch (class_id) {
    case 0x47: case 0x4A: case 0x41: case 0x42: case 0x43: case 0x1D: case 0x44:
        return true;
    default:
        return false;
    }
}
} // namespace

// Packet cc9_get_property_class_readers. Returns the pushed count (0 or 1)
// when `key` is one of the two class-chain keys the host binds, or -1 when it
// is not, so the caller falls through to the deck keys and the empty arm.
int GameMissionLuaHost::run_get_property_class_readers(lua_State* state, const char* key) {
    const bool wants_unitcommand = get_property_key_is(key, "unitcommand");
    const bool wants_reconlevel = get_property_key_is(key, "reconlevel");
    if (get_property_key_is(key, "ammoType")) {
        // 007EF1C0: 00779BB0 first (neither of its keys), then
        // 007EF1CF-007EF1EB _stricmp(key, "ammoType") == 0 -> 007EDAD0 ->
        // 00B66480 (push integer). Only a PlaneSquadronGen reaches 007EF1C0.
        ++summary_.get_property_ammotype_asked;
        if constexpr (kGetPropertySquadronAmmoTypeBound) {
            GameUnitsHost* units = units_hooks_;
            const int id = air_ops_entity_id(state);
            if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
                return 0;
            }
            const std::size_t index = static_cast<std::size_t>(id - 1);
            const bsp::PlaneSquadronHostRecord* sq =
                bsp::plane_squadron_registry().find_by_member_unit(index);
            if (sq == nullptr || sq->squadron_unit != index) return 0;
            const int ammo = units->squadron_ammo_type_007edad0(index);
            ++summary_.get_property_ammotype_served;
            if (ammo == 0) ++summary_.get_property_ammotype_zero;
            ::lua_pushinteger(state, static_cast<lua_Integer>(ammo));
            return 1;
        } else {
            return -1;
        }
    }
    if (!wants_unitcommand && !wants_reconlevel) return -1;
    if (wants_unitcommand) ++summary_.get_property_unitcommand_asked;
    if (wants_reconlevel) ++summary_.get_property_reconlevel_asked;
    if (!kGetPropertyClassReadersBound) return -1;
    // 00888AA0 resolves argument 0; the host's entity id is the units-host
    // index plus one (kMissionLuaEntityKeyFormat). SUBSTITUTION: an entity with
    // no units-host slot (a path, a nav point, a camera) is answered with
    // nothing, where the image would still run 00927AD0 on it.
    const GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) return 0;
    const std::size_t index = static_cast<std::size_t>(id - 1);

    if (wants_unitcommand) {
        // 00927AD0: director = vtable[114h](); none -> push nothing (00927B03).
        // SUBSTITUTION: every units-host slot has a director (the commands
        // host holds one per slot), so the no-director arm is never taken.
        // 0071BE40 (00927B07) -> the current command; a command pushes its
        // vtable[4]() name through 00B66710 (00927B1E), none pushes
        // "nocommand" (00D1926C, 00927B2B).
        const std::uint32_t command = units->director_current_command_0071be40(index);
        // Packet cc9_strafe_unitcommand: env-gated trace of what a script sees
        // (BSP_UNITCOMMAND_TRACE=1), the first 4000 answers.
        static const bool trace = [] {
            char* text = nullptr;
            std::size_t length = 0;
            const bool on = _dupenv_s(&text, &length, "BSP_UNITCOMMAND_TRACE") == 0 &&
                text != nullptr;
            std::free(text);
            return on;
        }();
        static int traced = 0;
        if (trace && traced < 4000) {
            ++traced;
            const char* n = command != 0u ? units->command_name_of(command) : "nocommand";
            log_.notef("unitcommand trace unit=%zu command=%08lx name=%s",
                index, static_cast<unsigned long>(command), n != nullptr ? n : "(null)");
        }
        if (command == 0u) {
            ++summary_.get_property_unitcommand_nocommand;
            ::lua_pushstring(state, "nocommand");
            return 1;
        }
        const char* name = units->command_name_of(command);
        if (name == nullptr || name[0] == '\0') {
            // A command object the host's class table does not name. The image
            // would push that object's own name; the host has none to push, so
            // the script sees nil, as it did before this binding.
            ++summary_.get_property_unitcommand_unnamed;
            return 0;
        }
        ++summary_.get_property_unitcommand_named;
        ::lua_pushstring(state, name);
        return 1;
    }

    // 00779BB0: only classes past 00927AD0 reach the `reconlevel` test.
    if (get_property_class_is_base_only(units->unit_class_id(index))) return 0;
    const GameGunneryHost* gunnery = units->gunnery();
    if (gunnery == nullptr) return 0;
    const bsp::ReconSensorPassState& pass = gunnery->recon_sensor_pass_state();
    // 00779C03 opens a new table as the frame's result, and 00779C12..00779C33
    // stores `table[party] = level` for party 0, 1 and 2 through 00B665D0 (a
    // number key and a number value) from the records at unit+1E8h, stride
    // 34h: +8h when the force byte +10h is set, else +4h. Every party is
    // written, 0 included.
    // SUBSTITUTION: the levels come from the host's recon pass the way
    // sync_recon_level_tables_0077b0c0 takes them: the unit's own side reads
    // 2 (0 once dead), a covered side its pass level, an uncovered side 0.
    // Forced levels (SetForcedReconLevel) are not modelled.
    const int own_side = units->unit_side_0054(index);
    const bool dead = gunnery->unit_dead(index);
    ::lua_createtable(state, 0, 3);
    for (int party = 0; party < 3; ++party) {
        int level = 0;
        if (party == own_side) {
            level = dead ? 0 : 2;
        } else if (pass.side_covered(party)) {
            const bsp::ReconDetectionLevel detected = pass.level(party, index);
            level = detected == bsp::ReconDetectionLevel::identified ? 2
                : detected == bsp::ReconDetectionLevel::blip ? 1 : 0;
        }
        ::lua_pushnumber(state, static_cast<lua_Number>(party));
        ::lua_pushnumber(state, static_cast<lua_Number>(level));
        ::lua_settable(state, -3);
    }
    ++summary_.get_property_reconlevel_tables;
    return 1;
}

namespace {
bool listener_key_equal(const std::string& a, const std::string& b) noexcept {
    // 00980150's map compares with BSP_NativeString_LessCaseInsensitive (00443D00).
    return a.size() == b.size() && _stricmp(a.c_str(), b.c_str()) == 0;
}

std::string listener_string_argument(lua_State* state, int slot) {
    if (slot > ::lua_gettop(state) || ::lua_type(state, slot) != LUA_TSTRING) return std::string();
    const char* text = ::lua_tolstring(state, slot, nullptr);
    return text != nullptr ? std::string(text) : std::string();
}

int listener_entity_id_at(lua_State* state, int index) {
    if (::lua_type(state, index) != LUA_TTABLE) return 0;
    ::lua_getfield(state, index, "ID");
    const int type = ::lua_type(state, -1);
    const int id = (type == LUA_TNUMBER || type == LUA_TSTRING)
        ? static_cast<int>(::lua_tonumber(state, -1)) : 0;
    ::lua_pop(state, 1);
    return id;
}

// 009721C0 reads a set from the block key: one entity table, or a table of them.
// Returns false when the key is absent or holds an empty table.
bool listener_read_entity_set(lua_State* state, int block, const char* key, std::vector<int>& out) {
    ::lua_getfield(state, block, key);
    const int value = ::lua_gettop(state);
    bool any = false;
    if (::lua_type(state, value) == LUA_TTABLE) {
        const int single = listener_entity_id_at(state, value);
        if (single > 0) {
            out.push_back(single);
            any = true;
        } else {
            ::lua_pushnil(state);
            while (::lua_next(state, value) != 0) {
                const int id = listener_entity_id_at(state, ::lua_gettop(state));
                if (id > 0) out.push_back(id);
                any = true;   // a non-entity member still makes the set non-empty
                ::lua_pop(state, 1);
            }
        }
    }
    ::lua_settop(state, value - 1);
    return any;
}
} // namespace

// 009722D0 reads an integer set: a number, or a table of numbers.
void listener_read_int_set_fn(lua_State* state, int block, const char* key, std::vector<int>& out);

// Packet cc9_lua_listeners. 008C6760 AddListener(channel, id, block) -> 00980C10:
// channel map (00980150), the id's slot (00978D60), the subscription 0097E360
// builds. A re-add of the same (channel, id) replaces the slot.
int GameMissionLuaHost::run_add_listener_008c6760(lua_State* state, int argument_count) {
    ++summary_.listener_adds;
    ListenerEntry entry;
    entry.channel = listener_string_argument(state, 1);
    entry.id = listener_string_argument(state, 2);
    if (argument_count >= 3 && ::lua_type(state, 3) == LUA_TTABLE) {
        ::lua_getfield(state, 3, "callback");   // 00972544, reader vtable[10h]
        if (::lua_type(state, -1) == LUA_TSTRING) {
            const char* text = ::lua_tolstring(state, -1, nullptr);
            if (text != nullptr) entry.callback = text;
        }
        ::lua_pop(state, 1);
        if (listener_key_equal(entry.channel, "kill")) {
            std::vector<int> ignored;
            listener_read_entity_set(state, 3, "entity", entry.entity_ids);   // +0Ch
            const bool attacker = listener_read_entity_set(state, 3, "lastAttacker", ignored);
            ::lua_getfield(state, 3, "lastAttackerPlayerIndex");              // +2Ch
            bool player_index = false;
            if (::lua_type(state, -1) == LUA_TTABLE) {
                ::lua_pushnil(state);
                if (::lua_next(state, -2) != 0) {
                    player_index = true;
                    ::lua_pop(state, 2);
                }
            }
            ::lua_pop(state, 1);
            entry.attacker_filters_set = attacker || player_index;
        }
        if (listener_key_equal(entry.channel, "hit")) {
            // 009725B0: target (+0Ch) and attacker (+2Ch) through 009721C0, attackType
            // (+3Ch) through 00970FF0, damageCaused (+5Ch) through 0096AAD0.
            listener_read_entity_set(state, 3, "target", entry.entity_ids);
            listener_read_entity_set(state, 3, "attacker", entry.attacker_ids);
            ::lua_getfield(state, 3, "attackType");
            if (::lua_type(state, -1) == LUA_TTABLE) {
                const int types = ::lua_gettop(state);
                ::lua_pushnil(state);
                while (::lua_next(state, types) != 0) {
                    if (::lua_type(state, -1) == LUA_TSTRING) {
                        const char* text = ::lua_tolstring(state, -1, nullptr);
                        if (text != nullptr) entry.attack_types.emplace_back(text);
                    }
                    ::lua_pop(state, 1);
                }
            }
            ::lua_pop(state, 1);
            std::vector<int> ignored;
            std::vector<int> damage;
            listener_read_int_set_fn(state, 3, "damageCaused", damage);
            for (int v : damage) entry.damage_range.push_back(static_cast<float>(v));
            const bool device = listener_read_entity_set(state, 3, "targetDevice", ignored);
            std::vector<int> player_index, fire, leak;
            listener_read_int_set_fn(state, 3, "attackerPlayerIndex", player_index);
            listener_read_int_set_fn(state, 3, "fireCaused", fire);
            listener_read_int_set_fn(state, 3, "leakCaused", leak);
            entry.hit_filters_unmodelled = device || !player_index.empty() || !fire.empty()
                || !leak.empty();
            if (kLuaHitFilterFieldsBound) {
                // Packet cc9_hit_listener_filters: only attackerPlayerIndex is left
                // unmodelled; the other three are matched at dispatch.
                entry.hit_filters_unmodelled = !player_index.empty();
                entry.hit_device_filter = device;
                for (int v : fire) entry.fire_range.push_back(static_cast<float>(v));
                for (int v : leak) entry.leak_range.push_back(static_cast<float>(v));
            }
            if (kLuaHitAttackerPlayerIndexBound) {
                // Packet cc9_hit_attacker_player_index: the set is kept (+4Ch, 009722D0)
                // and matched at dispatch, so it no longer marks the entry unmodelled.
                entry.attacker_player_indices = player_index;
                entry.hit_filters_unmodelled = kLuaHitFilterFieldsBound
                    ? false : (device || !fire.empty() || !leak.empty());
            }
        }
        if (listener_key_equal(entry.channel, "recon")) {
            // 00972450: callback, entity (+0Ch, 009721C0), oldLevel, newLevel and
            // party (+1Ch, +2Ch, +3Ch, 009722D0).
            listener_read_entity_set(state, 3, "entity", entry.entity_ids);
            listener_read_int_set_fn(state, 3, "oldLevel", entry.old_levels);
            listener_read_int_set_fn(state, 3, "newLevel", entry.new_levels);
            listener_read_int_set_fn(state, 3, "party", entry.parties);
        }
    }
    for (ListenerEntry& existing : listeners_) {
        if (listener_key_equal(existing.channel, entry.channel)
            && listener_key_equal(existing.id, entry.id)) {
            existing = entry;
            log_.implemented("MissionLuaNative::AddListener", "008c6760");
            return 0;
        }
    }
    log_.notef("  AddListener 008c6760: channel=\"%s\" id=\"%s\" callback=\"%s\" entities=%zu "
        "attacker_filters=%d (packet cc9_lua_listeners)", entry.channel.c_str(), entry.id.c_str(),
        entry.callback.c_str(), entry.entity_ids.size(), entry.attacker_filters_set ? 1 : 0);
    listeners_.push_back(std::move(entry));
    log_.implemented("MissionLuaNative::AddListener", "008c6760");
    return 0;
}

int GameMissionLuaHost::run_remove_listener_008c6990(lua_State* state, int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.listener_removes;
    const std::string channel = listener_string_argument(state, 1);
    const std::string id = listener_string_argument(state, 2);
    for (std::size_t i = 0; i < listeners_.size(); ++i) {
        if (listener_key_equal(listeners_[i].channel, channel)
            && listener_key_equal(listeners_[i].id, id)) {
            listeners_.erase(listeners_.begin() + static_cast<std::ptrdiff_t>(i));
            break;
        }
    }
    log_.implemented("MissionLuaNative::RemoveListener", "008c6990");
    return 0;
}

// 008C6BB0 -> 00980E00, the result pushed with 00B66450 BSP_LuaObject_PushBoolean.
int GameMissionLuaHost::run_is_listener_active_008c6bb0(lua_State* state, int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.listener_queries;
    const std::string channel = listener_string_argument(state, 1);
    const std::string id = listener_string_argument(state, 2);
    bool active = false;
    for (const ListenerEntry& entry : listeners_) {
        if (listener_key_equal(entry.channel, channel) && listener_key_equal(entry.id, id)) {
            active = true;
            break;
        }
    }
    ::lua_pushboolean(state, active ? 1 : 0);
    log_.implemented("MissionLuaNative::IsListenerActive", "008c6bb0");
    return 1;
}

// 009813A0's `kill` channel: 0097B8C0 collects the callback of every subscription
// whose condition (0096ACE0) holds, then each is called. The victim is the unit's
// entity (id = index + 1).
void GameMissionLuaHost::dispatch_kill_listeners_009813a0() {
    if (units_hooks_ == nullptr || state_ == nullptr) return;
    const std::vector<std::pair<std::size_t, float>> deaths = units_hooks_->destroyed_units();
    if (listener_death_seen_.size() < units_hooks_->count()) {
        listener_death_seen_.resize(units_hooks_->count(), false);
    }
    for (const auto& death : deaths) {
        const std::size_t unit = death.first;
        if (unit >= listener_death_seen_.size() || listener_death_seen_[unit]) continue;
        // A squadron entity dies only when 007F3970 removes its last plane (+3CCh
        // reaches 0, then 00926D90). SUBSTITUTION (labelled): the host fuses the
        // squadron with its wing-0 plane, so that slot's death is held until the
        // registry record's live count is 0, as kSquadronObserverLivenessBound does.
        bool squadron_alive = false;
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == unit
                && r.live_count() > 0) {
                squadron_alive = true;
                break;
            }
        }
        if (squadron_alive) continue;
        listener_death_seen_[unit] = true;
        ++summary_.listener_kill_deaths;
        const int victim = static_cast<int>(unit + 1);
        std::vector<std::string> callbacks;
        for (const ListenerEntry& entry : listeners_) {
            if (!listener_key_equal(entry.channel, "kill")) continue;
            if (!entry.entity_ids.empty()
                && std::find(entry.entity_ids.begin(), entry.entity_ids.end(), victim)
                    == entry.entity_ids.end()) {
                continue;   // 0096AD0D: the `entity` set rejects the victim
            }
            if (entry.attacker_filters_set) {
                // SUBSTITUTION (labelled): the host's death row carries no
                // attacker entity, so a non-empty lastAttacker or
                // lastAttackerPlayerIndex set is not matched.
                ++summary_.listener_attacker_filtered;
                continue;
            }
            if (!entry.callback.empty()) callbacks.push_back(entry.callback);
        }
        for (const std::string& name : callbacks) {
            const int top = ::lua_gettop(state_);
            lua_getfield(state_, LUA_GLOBALSINDEX, name.c_str());
            if (!lua_isfunction(state_, -1)) {
                ::lua_settop(state_, top);
                continue;
            }
            ++summary_.listener_kill_fires;
            const GameUnitRow* row = units_hooks_->unit_row(unit);
            log_.notef("  kill listener 009813a0: victim \"%s\" -> %s() (packet cc9_lua_listeners)",
                row != nullptr ? row->name.c_str() : "?", name.c_str());
            if (::lua_pcall(state_, 0, 0, 0) != 0) {
                const char* message = lua_tolstring(state_, -1, nullptr);
                note_error(message != nullptr ? message : std::string("(no message)"));
            }
            ::lua_settop(state_, top);
        }
    }
}

void listener_read_int_set_fn(lua_State* state, int block, const char* key, std::vector<int>& out) {
    ::lua_getfield(state, block, key);
    const int value = ::lua_gettop(state);
    if (::lua_type(state, value) == LUA_TNUMBER) {
        out.push_back(static_cast<int>(::lua_tonumber(state, value)));
    } else if (::lua_type(state, value) == LUA_TTABLE) {
        ::lua_pushnil(state);
        while (::lua_next(state, value) != 0) {
            if (::lua_type(state, -1) == LUA_TNUMBER) {
                out.push_back(static_cast<int>(::lua_tonumber(state, -1)));
            }
            ::lua_pop(state, 1);
        }
    }
    ::lua_settop(state, value - 1);
}

// Packet cc9_lua_recon_listeners. 00980E50 (from 0077B0C0 on each record's level
// change): only a live unit (+5Ch set, +5Dh/+5Eh/+60h clear) is dispatched; the
// four boxed values are (unit, old, new, party), matched by 00968470, and each
// passing callback is called with (unit, old, new, party).
// SUBSTITUTIONS (labelled): the host compares its recon pass's levels once per
// pass generation, at the mission frame; the unit's own party steps 0 -> 1 -> 2
// one level per pass (008065B0's +1 refresh); forced levels are not modelled.
void GameMissionLuaHost::dispatch_recon_listeners_00980e50() {
    if (units_hooks_ == nullptr || state_ == nullptr) return;
    const GameGunneryHost* gunnery = units_hooks_->gunnery();
    if (gunnery == nullptr) return;
    const bsp::ReconSensorPassState& pass = gunnery->recon_sensor_pass_state();
    if (pass.passes == recon_listener_generation_) return;
    recon_listener_generation_ = pass.passes;
    const std::size_t count = units_hooks_->count();
    if (recon_listener_levels_.size() < count * 3) recon_listener_levels_.resize(count * 3, 0);
    bool any_recon = false;
    for (const ListenerEntry& entry : listeners_) {
        if (listener_key_equal(entry.channel, "recon")) { any_recon = true; break; }
    }
    for (std::size_t u = 0; u < count; ++u) {
        const bool dead = gunnery->unit_dead(u);
        const int own = units_hooks_->unit_side_0054(u);
        for (int party = 0; party < 3; ++party) {
            int& last = recon_listener_levels_[u * 3 + static_cast<std::size_t>(party)];
            int published = 0;
            if (party == own) {
                // 008065B0 adds 1.0 to an own record each pass, which saturates the
                // value and publishes 2; the step substitution applies only without
                // the reset cycle.
                published = kReconListenerResetCycleBound ? 2 : (last < 2 ? last + 1 : 2);
            } else if (pass.side_covered(party)) {
                const bsp::ReconDetectionLevel detected = pass.level(party, u);
                published = detected == bsp::ReconDetectionLevel::identified ? 2
                    : detected == bsp::ReconDetectionLevel::blip ? 1 : 0;
            }
            std::vector<std::pair<int, int>> transitions;
            bsp::ReconDetectionLevel forced_level = bsp::ReconDetectionLevel::none;
            const bool forced = bsp::forced_recon_level(party, u, forced_level);
            if (kReconListenerResetCycleBound && !forced) {
                // 00807490 -> 00805BE0: old -> 0; then 00805AF0: 0 -> new.
                if (last != 0) transitions.emplace_back(last, 0);
                if (published != 0) transitions.emplace_back(0, published);
            } else if (published != last) {
                transitions.emplace_back(last, published);
            }
            last = published;
            if (dead || !any_recon) continue;   // 00980E50's live-unit gate
            for (const auto& transition : transitions) {
            const int old = transition.first;
            const int level = transition.second;
            ++summary_.listener_recon_changes;
            const int unit_id = static_cast<int>(u + 1);
            std::vector<std::string> callbacks;
            for (const ListenerEntry& entry : listeners_) {
                if (!listener_key_equal(entry.channel, "recon")) continue;
                auto holds = [](const std::vector<int>& set, int v) {
                    return set.empty() || std::find(set.begin(), set.end(), v) != set.end();
                };
                if (!holds(entry.entity_ids, unit_id) || !holds(entry.old_levels, old)
                    || !holds(entry.new_levels, level) || !holds(entry.parties, party)) {
                    continue;
                }
                if (!entry.callback.empty()) callbacks.push_back(entry.callback);
            }
            for (const std::string& name : callbacks) {
                const int top = ::lua_gettop(state_);
                lua_getfield(state_, LUA_GLOBALSINDEX, name.c_str());
                if (!lua_isfunction(state_, -1)) {
                    ::lua_settop(state_, top);
                    continue;
                }
                if (!push_resolved_entity_by_id(state_, unit_id)) lua_pushnil(state_);
                ::lua_pushnumber(state_, static_cast<lua_Number>(old));
                ::lua_pushnumber(state_, static_cast<lua_Number>(level));
                ::lua_pushnumber(state_, static_cast<lua_Number>(party));
                ++summary_.listener_recon_fires;
                const GameUnitRow* row = units_hooks_->unit_row(u);
                log_.notef("  recon listener 00980e50: \"%s\" party %d %d -> %d -> %s() (packet "
                    "cc9_lua_recon_listeners)", row != nullptr ? row->name.c_str() : "?", party,
                    old, level, name.c_str());
                if (::lua_pcall(state_, 4, 0, 0) != 0) {
                    const char* message = lua_tolstring(state_, -1, nullptr);
                    note_error(message != nullptr ? message : std::string("(no message)"));
                }
                ::lua_settop(state_, top);
            }
            }   // transitions
        }
    }
}

// Packet cc9_recon_publication (docs/RECON_PUBLICATION.md). 00806B10
// __thiscall(ReconSlot*, LuaInstance*), RET 4, and its fill 00805D90
// __thiscall(LuaInstance*, List* triple, int unused), RET 0Ch, both read whole:
//   pass 1 (00806B30..00806B9D): recon[slot+28h].enemy/own/neutral/unknown = nil
//     through 006B8390 (pushstring, pushnil, settable -3);
//   pass 2 (00806BA2..00806CB7): enemy from triple 1 (+DE4h), own from triple 0
//     (+DD8h), neutral from triple 2 (+DF0h), unknown from triple 3 (+DFCh), each
//     through 008037D0 (rawget, created by rawset when nil) and 00805D90.
// 00805D90 buckets the triple's records by category (skipping the ten classes of
// 00805DE6 and category 13h), then for each of the nineteen names of 00E0B590
// descends into (creates) that table and, per record in bucket order, sets
// table[itoa(+174h)] = thisTable[itoa(+174h)] (00927BF0 = getglobal thisTable,
// gettable, remove; 006B84D0 = settable -3).
// SUBSTITUTIONS (labelled):
//  - cadence: once per host recon pass generation, at the mission frame, after
//    the recon listeners; the image publishes inside 008079B0 right after the
//    slot's rebuild;
//  - the +25h dirty byte (set by 0077B0C0 on any level change of the slot and by
//    00803BA0 on a death it saw) is stood in for by "the published content
//    differs from the last publication of this party";
//  - the +174h id is the host's entity id (unit index + 1), the thisTable key;
//  - a squadron's +354h is its first resolved member's category, frozen then
//    (007F4BE8/007F5438 write it once, in the init slot 007F4BA0).
void GameMissionLuaHost::publish_recon_slot_tables_00806b10() {
    if (units_hooks_ == nullptr || state_ == nullptr) return;
    const GameGunneryHost* gunnery = units_hooks_->gunnery();
    if (gunnery == nullptr) return;
    const bsp::ReconSensorPassState& pass = gunnery->recon_sensor_pass_state();
    if (pass.passes == recon_publish_generation_) return;
    recon_publish_generation_ = pass.passes;
    ++summary_.recon_publish_passes;

    auto category_of = [&](std::size_t unit) -> int {
        const int class_id = units_hooks_->unit_class_id(unit);
        if (class_id != bsp::kReconSquadronClassId) {
            return bsp::recon_publish_category_for_class(class_id);
        }
        const auto cached = recon_squadron_category_.find(unit);
        if (cached != recon_squadron_category_.end()) return cached->second;
        for (const bsp::PlaneSquadronHostRecord& record :
             bsp::plane_squadron_registry().records()) {
            if (record.squadron_unit != unit) continue;
            std::size_t member = bsp::kPlaneSquadronNoUnit;
            if (!record.member_units.empty()) member = record.member_units.front();
            if (member == bsp::kPlaneSquadronNoUnit && !record.departed_units.empty()) {
                member = record.departed_units.front();
            }
            if (member == bsp::kPlaneSquadronNoUnit) break;
            const int category =
                bsp::recon_publish_category_for_class(units_hooks_->unit_class_id(member));
            recon_squadron_category_.emplace(unit, category);
            return category;
        }
        return bsp::kReconPublishNoCategory;   // +354h = 13h from 007F2DF6
    };

    // 00806BE4, 00806C1A, 00806C50, 00806C86: relation name and triple.
    static constexpr std::array<std::pair<const char*, int>, 4> kRelations{{
        {"enemy", 1}, {"own", 0}, {"neutral", 2}, {"unknown", 3}}};
    constexpr std::size_t kCategories = bsp::kReconPublishCategoryCount;
    std::vector<std::size_t> triple;
    for (int party = 0; party < 3; ++party) {
        // buckets[relation][category] = entity ids in triple order.
        std::array<std::array<std::vector<int>, kCategories>, 4> buckets;
        bool any_triple = false;
        for (std::size_t r = 0; r < kRelations.size(); ++r) {
            if (!gunnery->recon_triple_units(party, kRelations[r].second, triple)) continue;
            any_triple = true;
            for (const std::size_t unit : triple) {
                if (bsp::recon_publish_excludes_class_00805de6(
                        units_hooks_->unit_class_id(unit))) {
                    ++summary_.recon_publish_excluded;
                    continue;
                }
                const int category = category_of(unit);
                if (category < 0 || category >= static_cast<int>(kCategories)) {
                    ++summary_.recon_publish_no_category;
                    continue;
                }
                buckets[r][static_cast<std::size_t>(category)].push_back(
                    static_cast<int>(unit + 1));
            }
        }
        if (!any_triple) continue;   // no slot rebuilt for this party yet
        std::vector<std::uint32_t> content;
        for (std::size_t r = 0; r < buckets.size(); ++r) {
            for (std::size_t c = 0; c < kCategories; ++c) {
                for (const int id : buckets[r][c]) {
                    content.push_back(static_cast<std::uint32_t>((r << 28) | (c << 20)) |
                                      static_cast<std::uint32_t>(id));
                }
            }
        }
        std::vector<std::uint32_t>& last = recon_published_[static_cast<std::size_t>(party)];
        if (content == last) {
            ++summary_.recon_publish_unchanged;
            continue;
        }
        last = content;
        ++summary_.recon_publish_slots;

        const bsp::ReconLuaInstanceView instance{state_};
        const int top = ::lua_gettop(state_);
        // Pass 1, 00806B30..00806B9D.
        bsp::push_recon_global_table_006b8190(instance, bsp::kMissionReconGlobal);
        {
            bsp::ReconTableScope indexed = bsp::push_recon_index_table_00803750(instance, party);
            for (const char* key : {"enemy", "own", "neutral", "unknown"}) {
                ::lua_pushstring(state_, key);   // 006B8390
                ::lua_pushnil(state_);
                ::lua_settable(state_, -3);
            }
            bsp::pop_recon_table_scope(indexed);
        }
        bsp::pop_recon_global_table_006b8210(instance);
        // Pass 2, 00806BA2..00806CB7.
        bsp::push_recon_global_table_006b8190(instance, bsp::kMissionReconGlobal);
        bsp::ReconTableScope indexed = bsp::push_recon_index_table_00803750(instance, party);
        char key[16];
        for (std::size_t r = 0; r < kRelations.size(); ++r) {
            bsp::ReconTableScope related =
                bsp::push_recon_named_table_008037d0(instance, kRelations[r].first);
            for (std::size_t c = 0; c < kCategories; ++c) {   // 00805E5A..00805EEB
                bsp::ReconTableScope category = bsp::push_recon_named_table_008037d0(
                    instance, bsp::recon_category_names_00e0b590[c]);
                for (const int id : buckets[r][c]) {
                    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, id);
                    ::lua_pushstring(state_, key);                              // 006B8120
                    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);  // 00927BF0
                    ::lua_pushstring(state_, key);
                    ::lua_gettable(state_, -2);
                    ::lua_remove(state_, -2);
                    ::lua_settable(state_, -3);                                 // 006B84D0(-3)
                    ++summary_.recon_publish_entries;
                }
                bsp::pop_recon_table_scope(category);
            }
            bsp::pop_recon_table_scope(related);
        }
        bsp::pop_recon_table_scope(indexed);
        bsp::pop_recon_global_table_006b8210(instance);
        ::lua_settop(state_, top);
    }
}

// Packet cc9_lua_set_ship_speed. 00890D30 SetShipSpeed(entity, speed): argument 0
// through 00888AA0, argument 1 through 00B66270 (a number), then 00890E6F stores
// max(speed, 0) at [entity+73Ch]+24h and the clock [00F876A4] at +28h. No class
// test. SUBSTITUTION (labelled): an entity with no units-host slot is not reached.
int GameMissionLuaHost::run_set_ship_speed_00890d30(lua_State* state, int argument_count) {
    ++summary_.ship_speed_calls;
    const float speed = argument_count >= 2 ? static_cast<float>(::lua_tonumber(state, 2)) : 0.0f;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.ship_speed_unresolved;
        log_.notef("  SetShipSpeed 00890d30: entity id %d has no units-host slot (packet "
            "cc9_lua_set_ship_speed)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    units->store_commanded_speed_00890e6f(index, speed);
    ++summary_.ship_speed_units;
    const GameUnitRow* row = units->unit_row(index);
    log_.notef("  SetShipSpeed 00890d30: \"%s\" speed=%.2f (packet cc9_lua_set_ship_speed)",
        row != nullptr ? row->name.c_str() : "?", static_cast<double>(speed));
    log_.implemented("MissionLuaNative::SetShipSpeed", "00890d30");
    return 0;
}

// Packet cc9_unit_get_attack_target. 008A6DE0 UnitGetAttackTarget(entity): argument 0
// through 00888AA0, the director from the entity's vtable[114h] (008A6EF8), then:
//   director->vtable[48h](2) (008A6F0D) true  -> director->vtable[2Ch]() (008A6F1A), which
//     is 008364E0 `MOV EAX,[ECX+238h]`, the fire target, in the base (00D09EC0) and the
//     ship (00D09F58) director vtables;
//   false -> 0071BE40 current command (008A6F1E); its vtable[0Ch] category 1 or 2
//     (008A6F34..008A6F3C) -> 0071EB60 then 00521EA0 (008A6F44..008A6F4B); else nil.
// Slot 48h is a constant test on its argument: 008364B0 (base) is true for 0 and 2,
// 00836790 (ship; no Ghidra function) true for 0, 2 and 3, 0084D8F0 (the squadron block
// 00D0BD98 that 007F5009 stores at +348h) true for 0 and 1. So a ship's director always
// answers its fire target and only a squadron reaches the command arm.
// The result: non-null with +5Dh clear (008A6F58) -> thisTable[tostring(+174h)], else nil.
// SUBSTITUTIONS (labelled): "ship director" is a units-host slot with a ship AI row, and
// director+238h is that row's fire target, which the ship AI host holds by name; +5Dh
// clear is GameUnitsHost::unit_active; an entity with no units-host slot answers nil.
int GameMissionLuaHost::run_unit_get_attack_target_008a6de0(lua_State* state,
                                                           int argument_count) {
    (void)argument_count;
    ++summary_.attack_target_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.attack_target_unresolved;
        ++summary_.attack_target_nil;
        ::lua_pushnil(state);
        return 1;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    std::size_t target_plus_one = 0;
    const GameShipAiRow* ship_row = nullptr;
    if (GameShipAiHost* ai = units->ship_ai()) {
        for (const GameShipAiRow& r : ai->rows()) {
            if (r.unit_index == index) { ship_row = &r; break; }
        }
    }
    const char* arm = "command";
    if (ship_row != nullptr) {
        // director->vtable[48h](2) true: the fire target, director+238h.
        arm = "fire";
        ++summary_.attack_target_fire_arm;
        if (!ship_row->fire_target.empty()) {
            for (std::size_t i = 0; i < units->count(); ++i) {
                const GameUnitRow* candidate = units->unit_row(i);
                if (candidate != nullptr && candidate->name == ship_row->fire_target) {
                    target_plus_one = i + 1;
                    break;
                }
            }
        }
    } else {
        ++summary_.attack_target_command_arm;
        const std::uint32_t command = units->director_current_command_0071be40(index);
        const bsp::EntityOrderCommandClass* klass =
            command != 0u ? bsp::entity_order_command_class_by_address(command) : nullptr;
        if (klass != nullptr && (klass->category == 1 || klass->category == 2)) {
            bsp::SceneCommandTarget target;
            int mode = 0;
            if (units->active_command_descriptor_0071eb60(index, target, mode)) {
                target_plus_one = units->resolve_command_target_00521ea0(target);
            }
        }
    }
    const GameUnitRow* self = units->unit_row(index);
    const GameUnitRow* hit = target_plus_one != 0 ? units->unit_row(target_plus_one - 1) : nullptr;
    const bool live = target_plus_one != 0 && units->unit_active(target_plus_one - 1);
    if (live && push_resolved_entity_by_id(state, static_cast<int>(target_plus_one))) {
        ++summary_.attack_target_pushed;
    } else {
        ++summary_.attack_target_nil;
        ::lua_pushnil(state);
    }
    log_.notef("  UnitGetAttackTarget 008a6de0: \"%s\" arm=%s target=\"%s\" pushed=%d "
        "(packet cc9_unit_get_attack_target)", self != nullptr ? self->name.c_str() : "?", arm,
        hit != nullptr ? hit->name.c_str() : "", live ? 1 : 0);
    log_.implemented("MissionLuaNative::UnitGetAttackTarget", "008a6de0");
    return 1;
}

// Packet cc9_squadron_set_speed. 0089F780 SquadronSetSpeed(squadron, speed): argument 0
// through 00888AA0, argument 1 as a number (00B66270), then for i below [entity+3CCh]
// member[i] = i < 5 ? [entity+3D0h+4i] : null and member->vtable[3Ch](speed)
// (0089F8CA..0089F8FF). There is no class test. On every plane class vtable[3Ch] is
// 0074E1E0, which calls 007D9E80 on unit+AB0h. Returns no value.
// SUBSTITUTIONS (labelled): the members are the squadron registry's member_units that
// are still active, standing in for the compacted +3D0h array; an entity with no
// squadron record is counted unresolved and nothing is called.
int GameMissionLuaHost::run_squadron_set_speed_0089f780(lua_State* state, int argument_count) {
    ++summary_.squadron_speed_calls;
    const float speed = argument_count >= 2 ? static_cast<float>(::lua_tonumber(state, 2)) : 0.0f;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.squadron_speed_unresolved;
        log_.notef("  SquadronSetSpeed 0089f780: entity id %d has no units-host slot (packet "
            "cc9_squadron_set_speed)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    const bsp::PlaneSquadronHostRecord* record = nullptr;
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == index) {
            record = &r;
            break;
        }
    }
    const GameUnitRow* row = units->unit_row(index);
    if (record == nullptr) {
        ++summary_.squadron_speed_unresolved;
        log_.notef("  SquadronSetSpeed 0089f780: \"%s\" has no squadron record (packet "
            "cc9_squadron_set_speed)", row != nullptr ? row->name.c_str() : "?");
        return 0;
    }
    std::size_t planes = 0;
    std::size_t live = 0;
    for (std::size_t member : record->member_units) {
        if (member == bsp::kPlaneSquadronNoUnit || !units->unit_active(member)) continue;
        if (live++ >= 5) break;  // 0089F8D8: a sixth member would be a null object
        if (units->set_plane_forward_speed_007d9e80(member, speed)) ++planes;
    }
    summary_.squadron_speed_planes += planes;
    log_.notef("  SquadronSetSpeed 0089f780: \"%s\" speed=%.2f planes=%zu (packet "
        "cc9_squadron_set_speed)", row != nullptr ? row->name.c_str() : "?",
        static_cast<double>(speed), planes);
    log_.implemented("MissionLuaNative::SquadronSetSpeed", "0089f780");
    return 0;
}

// Packet cc9_is_class_changed. 008CC4B0 IsClassChanged(id): argument 0 as an integer
// (00B66290, 008CC5AF), then 00437F50 and CMP [EAX+ESI*4+2010h],ESI / SETNZ
// (008CC5CB..008CC5D6) into 00B66450 (lua_pushboolean). One result.
// The inverse map at registry+2010h is written only by 00506550 and 00592640. Both reset
// all 800h pairs to the identity and then store one pair, the player's chosen ship:
// 00592640 (the `continue` footer command) takes it from the profile's record for the
// selected mission (007FC490 over 005806A0).
// SUBSTITUTION (labelled): this process does not model that record, and the footer
// command is recorded unimplemented, so the map here is the identity and every answer
// is false. An id outside the 800h entries answers false, where the image would read
// past the map.
int GameMissionLuaHost::run_is_class_changed_008cc4b0(lua_State* state, int argument_count) {
    ++summary_.class_changed_calls;
    static const bsp::VehicleClassIndexMap identity_map = [] {
        bsp::VehicleClassIndexMap m;
        m.reset_identity_00592652();
        return m;
    }();
    const int id = argument_count >= 1 ? static_cast<int>(::lua_tonumber(state, 1)) : 0;
    const bool in_range = id >= 0 && id < bsp::kVehicleClassIndexMapSize;
    const bool changed = in_range && identity_map.to_type_id(id) != id;
    if (changed) ++summary_.class_changed_true;
    ::lua_pushboolean(state, changed ? 1 : 0);
    log_.implemented("MissionLuaNative::IsClassChanged", "008cc4b0");
    return 1;
}

// Packet cc9_set_submarine_depth_level. 00893F40 SetSubmarineDepthLevel(entity, level):
// argument 0 through 00888AA0, argument 1 as an integer; a request for 1 becomes 0 when
// +122Ch (periscopeState) is 2 or +1214h (the periscope node) is null; then 008528B0.
// Returns no value.
// SUBSTITUTIONS (labelled): periscopeState is never 2 here (no periscope damage is
// modelled) and +1214h is null only for a kamikaze class, which reads false, so a
// request for 1 passes; an entity with no units-host slot is counted unresolved.
int GameMissionLuaHost::run_set_submarine_depth_level_00893f40(lua_State* state,
                                                              int argument_count) {
    ++summary_.sub_depth_calls;
    const int level = argument_count >= 2 ? static_cast<int>(::lua_tonumber(state, 2)) : 0;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.sub_depth_unresolved;
        log_.notef("  SetSubmarineDepthLevel 00893f40: entity id %d has no units-host slot "
            "(packet cc9_set_submarine_depth_level)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    const bool stored = units->set_submarine_depth_level_008528b0(index, level);
    if (stored) ++summary_.sub_depth_stored;
    const GameUnitRow* row = units->unit_row(index);
    log_.notef("  SetSubmarineDepthLevel 00893f40: \"%s\" level=%d stored=%d now=%d (packet "
        "cc9_set_submarine_depth_level)", row != nullptr ? row->name.c_str() : "?", level,
        stored ? 1 : 0, row != nullptr ? static_cast<int>(row->submarine_depth_level) : -1);
    log_.implemented("MissionLuaNative::SetSubmarineDepthLevel", "00893f40");
    return 0;
}

// Packet cc9_set_air_base_slot_count. 008963E0 SetAirBaseSlotCount(entity, n): argument 0
// through 00888AA0, BSP_AirOps_GetBlock, argument 1 as an integer, then 006C7E20(n) on the
// block. 006C7E20 appends default records while [block+50h] < n (006C7E73 JAE, loop back
// at 006C8060) and destroys from the tail while it is above n (006C8069..006C8089). The
// default record (006C7EA3..006C7F1C): class 0, assigned 0, requested 3, class+134h copy
// 0, no squadron, state 1, timer 0.0, launch request clear. Returns no value.
// SUBSTITUTIONS (labelled): the deck is the air-ops deck registry's entry for the entity;
// an entity with no deck is counted unresolved (the image has no such test); a negative n
// is ignored, where the unsigned compare would grow the array without bound.
int GameMissionLuaHost::run_set_air_base_slot_count_008963e0(lua_State* state,
                                                            int argument_count) {
    ++summary_.slot_count_calls;
    const int count = argument_count >= 2 ? static_cast<int>(::lua_tonumber(state, 2)) : 0;
    const int id = air_ops_entity_id(state);
    bsp::AirOpsDeck* deck = id > 0 ? bsp::air_ops_decks().find_mutable_by_entity_id(id) : nullptr;
    if (deck == nullptr || count < 0) {
        ++summary_.slot_count_unresolved;
        log_.notef("  SetAirBaseSlotCount 008963e0: entity id %d n=%d has no deck or a "
            "negative count (packet cc9_set_air_base_slot_count)", id, count);
        return 0;
    }
    const std::size_t before = deck->slots.size();
    const std::size_t wanted = static_cast<std::size_t>(count);
    while (deck->slots.size() < wanted) {
        bsp::AirOpsSlot slot;
        slot.vehicle_class = 0;
        slot.assigned_count = 0;
        slot.requested_count = 3;                       // 006C7EB3
        slot.class_field_134 = 0;
        slot.launched_squadron = 0;
        slot.state = bsp::AirOpsSlotState::kCooldown;   // 006C7EDC, 1
        slot.timer = 0.0f;
        slot.launch_requested = false;
        deck->slots.push_back(slot);
    }
    while (deck->slots.size() > wanted) deck->slots.pop_back();
    if (deck->slots.size() != before) ++summary_.slot_count_resized;
    const GameUnitsHost* units = units_hooks_;
    const GameUnitRow* row = (units != nullptr && static_cast<std::size_t>(id) <= units->count())
        ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    log_.notef("  SetAirBaseSlotCount 008963e0: \"%s\" slots %zu -> %zu (packet "
        "cc9_set_air_base_slot_count)", row != nullptr ? row->name.c_str() : "?", before,
        deck->slots.size());
    log_.implemented("MissionLuaNative::SetAirBaseSlotCount", "008963e0");
    return 0;
}

namespace {
// 00E17BF2. One byte in the image, so one value per process here.
bool g_device_reload_enabled_00e17bf2 = false;
}  // namespace

bool lua_device_reload_enabled_00e17bf2() noexcept {
    return kLuaDeviceReloadEnabledBound && g_device_reload_enabled_00e17bf2;
}

namespace {
// Packet cc9_add_untouchable_unit: the +1D4h byte per units-host index. The image
// keeps it on each unit's AI object; nothing clears it except
// RemoveUntouchableUnit 008AC2B0, which no reference row calls.
std::vector<bool> g_unit_untouchable_1d4;
}  // namespace

bool lua_unit_untouchable_1d4(std::size_t index) noexcept {
    return kLuaAddUntouchableUnitBound && index < g_unit_untouchable_1d4.size()
        && g_unit_untouchable_1d4[index];
}

// Packet cc9_add_untouchable_unit. 008AC140 AddUntouchableUnit(unit): 00888AA0,
// vtable[140h](), byte +1D4h = 1. SUBSTITUTION (labelled): an entity with no
// units-host slot (a scene marker) is skipped, where the image would call its
// vtable[140h]; the reference rows pass only units.
int GameMissionLuaHost::run_add_untouchable_unit_008ac140(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.untouchable_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const bool valid = units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count();
    const GameUnitRow* row = valid ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    if (valid) {
        const std::size_t index = static_cast<std::size_t>(id - 1);
        if (g_unit_untouchable_1d4.size() <= index) g_unit_untouchable_1d4.resize(index + 1, false);
        g_unit_untouchable_1d4[index] = true;
        ++summary_.untouchable_marked;
    }
    log_.notef("  AddUntouchableUnit 008ac140: \"%s\" (unit %d) marked=%d t=%.2f (packet "
        "cc9_add_untouchable_unit)", row != nullptr ? row->name.c_str() : "?", id,
        valid ? 1 : 0, static_cast<double>(spawn_world_clock_));
    log_.implemented("MissionLuaNative::AddUntouchableUnit", "008ac140");
    return 0;
}

// Packet cc9_device_reload_enabled. 008C1350 SetDeviceReloadEnabled(flag): argument 0
// through 00B66250 (lua_toboolean) into the byte 00E17BF2 at 008C1458. No entity, no
// class test, no return value.
int GameMissionLuaHost::run_set_device_reload_enabled_008c1350(lua_State* state,
    int argument_count) {
    ++summary_.device_reload_calls;
    const bool flag = argument_count >= 1 && ::lua_toboolean(state, 1) != 0;
    g_device_reload_enabled_00e17bf2 = flag;
    if (flag) ++summary_.device_reload_true;
    log_.notef("  SetDeviceReloadEnabled 008c1350: flag=%d -> 00E17BF2 (packet "
        "cc9_device_reload_enabled)", flag ? 1 : 0);
    log_.implemented("MissionLuaNative::SetDeviceReloadEnabled", "008c1350");
    return 0;
}

// Packet cc9_squadron_travel_alt. 0089F550 SquadronSetTravelAlt(squadron, alt [, force]):
// argument 0 through BSP_ObjectHandle_FromLuaTable, argument 1 as a number, and argument 2
// as a boolean only when exactly three arguments are passed (TRIV_body_00b663f0 == 3),
// else false. No result.
int GameMissionLuaHost::run_squadron_set_travel_alt_0089f550(lua_State* state,
    int argument_count) {
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const float altitude = argument_count >= 2 ? static_cast<float>(::lua_tonumber(state, 2)) : 0.0f;
    const bool force = argument_count == 3 && ::lua_toboolean(state, 3) != 0;
    bool stored = false;
    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
        stored = units->set_squadron_travel_alt_0089f550(static_cast<std::size_t>(id - 1),
            altitude, force);
    }
    const GameUnitRow* row = (stored) ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    log_.notef("  SquadronSetTravelAlt 0089f550: \"%s\" alt=%.1f force=%d stored=%d (packet "
        "cc9_squadron_travel_alt)", row != nullptr ? row->name.c_str() : "?",
        static_cast<double>(altitude), force ? 1 : 0, stored ? 1 : 0);
    log_.implemented("MissionLuaNative::SquadronSetTravelAlt", "0089f550");
    return 0;
}

// Packet cc9_squadron_attack_alt. 008A22B0 SquadronSetAttackAlt(squadron, alt[, force]):
// argument 0 through 00888AA0 (008A23B6), argument 1 through 00B66270 (008A23E7),
// argument 2 through 00B66250 only when the count is exactly 3 (008A2409), else 0.
// No result (008A2490).
int GameMissionLuaHost::run_squadron_set_attack_alt_008a22b0(lua_State* state,
    int argument_count) {
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const float altitude = argument_count >= 2 ? static_cast<float>(::lua_tonumber(state, 2)) : 0.0f;
    const bool force = argument_count == 3 && ::lua_toboolean(state, 3) != 0;
    bool stored = false;
    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
        stored = units->set_squadron_attack_alt_008a22b0(static_cast<std::size_t>(id - 1),
            altitude, force);
    }
    const GameUnitRow* row = (stored) ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    log_.notef("  SquadronSetAttackAlt 008a22b0: \"%s\" alt=%.1f force=%d stored=%d (packet "
        "cc9_squadron_attack_alt)", row != nullptr ? row->name.c_str() : "?",
        static_cast<double>(altitude), force ? 1 : 0, stored ? 1 : 0);
    log_.implemented("MissionLuaNative::SquadronSetAttackAlt", "008a22b0");
    return 0;
}

// Packet cc9_lua_formation_query. 008996A0 IsInFormation(unit): argument 0 through
// BSP_ObjectHandle_FromLuaTable, then lua_pushboolean([unit+284h] != 0). One result.
// SUBSTITUTION (labelled): an entity with no units-host slot answers false (the image
// would read its +284h).
int GameMissionLuaHost::run_is_in_formation_008996a0(lua_State* state, int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.in_formation_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const bool valid = units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count();
    const std::int32_t group = valid
        ? units->unit_formation_group_0284(static_cast<std::size_t>(id - 1)) : -1;
    const bool in_formation = group >= 0;
    if (in_formation) ++summary_.in_formation_true;
    const GameUnitRow* row = valid ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    log_.notef("  IsInFormation 008996a0: \"%s\" group=%d -> %d (packet "
        "cc9_lua_formation_query)", row != nullptr ? row->name.c_str() : "?",
        static_cast<int>(group), in_formation ? 1 : 0);
    log_.implemented("MissionLuaNative::IsInFormation", "008996a0");
    ::lua_pushboolean(state, in_formation ? 1 : 0);
    return 1;
}

// Packet cc9_get_last_catapulted. 00892860 GetLastCatapulted(ship): 0089295E
// 00888AA0 on argument 0, 00892987 vtable[5Ch](6), then 00892993 00953A60
// (00953A60-00953A77: [unit+630h] < 0 answers 0, else [unit+550h+18h*i]); 0
// pushes nil (00892A4B 00B66430), a plane its thisTable slot keyed by the
// +174h id (008929B6..008929F8). A non-ship pushes nothing. LABELLED: no path in this host writes
// unit+630h (the launch 006EC8E0 and the message to 00957450 are not built),
// so a ship's index is its constructor value, -1.
int GameMissionLuaHost::run_get_last_catapulted_00892860(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.last_catapulted_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const bool valid = units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count();
    const bool ship = valid && units->unit_is_kind_of(static_cast<std::size_t>(id - 1), 6);
    if (summary_.last_catapulted_calls <= 8) {
        const GameUnitRow* row = valid ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
        log_.notef("  GetLastCatapulted 00892860: \"%s\" ship=%d -> %s (unit+630h = -1, "
            "packet cc9_get_last_catapulted)", row != nullptr ? row->name.c_str() : "?",
            ship ? 1 : 0, ship ? "nil" : "no result");
    }
    log_.implemented("MissionLuaNative::GetLastCatapulted", "00892860");
    if (!ship) {
        ++summary_.last_catapulted_not_ship;
        return 0;
    }
    ++summary_.last_catapulted_nil;
    ::lua_pushnil(state);
    return 1;
}

// Packet cc9_get_formation_leader. 00899AF0 GetFormationLeader(unit): 00888AA0 on
// argument 0 (00899BEF), 007788D0 (00899C08), then nil (00899CB4) or the leader's
// thisTable slot (00899C15..00899C65). One result.
// SUBSTITUTION (labelled): an entity with no units-host slot answers nil, as a unit
// whose +284h is null does; a leader whose thisTable slot is missing also answers nil.
int GameMissionLuaHost::run_get_formation_leader_00899af0(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.formation_leader_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const bool valid = units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count();
    const std::int32_t group = valid
        ? units->unit_formation_group_0284(static_cast<std::size_t>(id - 1)) : -1;
    const std::size_t leader = group >= 0 ? units->formation_leader_0014(group)
                                          : static_cast<std::size_t>(-1);
    bool pushed = false;
    if (valid && leader < units->count()) {
        pushed = push_resolved_entity_by_id(state, static_cast<int>(leader) + 1);
    }
    if (pushed) {
        ++summary_.formation_leader_found;
        if (static_cast<int>(leader) + 1 != id) ++summary_.formation_leader_other;
    }
    if (summary_.formation_leader_calls <= 24 || (pushed && static_cast<int>(leader) + 1 != id)) {
        const GameUnitRow* row = valid ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
        const GameUnitRow* lrow = pushed ? units->unit_row(leader) : nullptr;
        log_.notef("  GetFormationLeader 00899af0: \"%s\" group=%d -> %s%s (packet "
            "cc9_get_formation_leader)", row != nullptr ? row->name.c_str() : "?",
            static_cast<int>(group), lrow != nullptr ? lrow->name.c_str() : "nil",
            pushed && static_cast<int>(leader) + 1 != id ? " (not the argument)" : "");
    }
    log_.implemented("MissionLuaNative::GetFormationLeader", "00899af0");
    if (!pushed) ::lua_pushnil(state);
    return 1;
}

// Packet cc9_lua_formation_query. 00899EB0 LeaveFormation(unit): argument 0 through
// BSP_ObjectHandle_FromLuaTable, then 0077C980(unit, 0). With [unit+284h] set that sends
// message 77h (null target), delivered by 0077FE80 arm 3 to 0077BD70(unit, null).
// SUBSTITUTIONS (labelled): the leave runs at the call instead of through
// BSP_Session_RouteMessage; the slot counter bump for [unit+1ACh] < 8
// (BSP_SlotCounter_Increment) is not modelled; the host's leave counts the unit in its
// death-leave counter and prints its destroy-path note.
int GameMissionLuaHost::run_leave_formation_00899eb0(lua_State* state, int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.leave_formation_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const bool valid = units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count();
    const std::int32_t group = valid
        ? units->unit_formation_group_0284(static_cast<std::size_t>(id - 1)) : -1;
    if (group >= 0) {
        units->leave_group_on_destroy_0077bd70(static_cast<std::size_t>(id - 1));
        ++summary_.leave_formation_left;
    }
    const GameUnitRow* row = valid ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    log_.notef("  LeaveFormation 00899eb0: \"%s\" group=%d left=%d (0077C980 -> 77h -> "
        "0077BD70, packet cc9_lua_formation_query)", row != nullptr ? row->name.c_str() : "?",
        static_cast<int>(group), group >= 0 ? 1 : 0);
    log_.implemented("MissionLuaNative::LeaveFormation", "00899eb0");
    return 0;
}

// Packet cc9_submarine_air. 00893C00 SetUnlimitedAirSupply(entity, flag): argument 0
// through 00888AA0, argument 1 through 00B66250 (lua_toboolean), stored at unit+1280h
// with no class test. Returns no value.
// SUBSTITUTION (labelled): an entity with no units-host slot, or a slot that is not a
// seeded submarine, is counted but stores nothing (the image writes the byte anyway).
int GameMissionLuaHost::run_set_unlimited_air_00893c00(lua_State* state, int argument_count) {
    ++summary_.unlimited_air_calls;
    const bool flag = argument_count >= 2 && ::lua_toboolean(state, 2) != 0;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    bool stored = false;
    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
        stored = units->set_unlimited_air_00893c00(static_cast<std::size_t>(id - 1), flag);
    }
    if (stored) ++summary_.unlimited_air_stored;
    const GameUnitRow* row = (units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count())
        ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    log_.notef("  SetUnlimitedAirSupply 00893c00: \"%s\" flag=%d stored=%d (packet "
        "cc9_submarine_air)", row != nullptr ? row->name.c_str() : "?", flag ? 1 : 0,
        stored ? 1 : 0);
    log_.implemented("MissionLuaNative::SetUnlimitedAirSupply", "00893c00");
    return 0;
}

// Packet cc9_lua_aa_enable. 0089C740 AAEnable(entity, flag): argument 0 through
// 00888AA0, argument 1 through 00B66250 (lua_toboolean), then, when the entity's
// vtable[114h] director exists, 0071E050(flag) -> director+221h.
// SUBSTITUTIONS (labelled): the flag is written into the unit's scene director
// entry (by name) at the call, not routed through session message 5Ah; an entity
// with no units-host slot is counted unresolved; every units-host slot is taken to
// have a director.
int GameMissionLuaHost::run_aa_enable_0089c740(lua_State* state, int argument_count) {
    ++summary_.aa_enable_calls;
    const bool flag = argument_count >= 2 && ::lua_toboolean(state, 2) != 0;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const GameUnitRow* row = (units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count())
        ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    if (row == nullptr) {
        ++summary_.aa_enable_unresolved;
        log_.notef("  AAEnable 0089c740: entity id %d has no units-host slot (packet "
            "cc9_lua_aa_enable)", id);
        return 0;
    }
    // 007202FD: a director with no pass-B entry starts with all four enables set.
    bsp::game::SceneDirectorEnables enables;
    if (const bsp::game::SceneDirectorEnables* e = bsp::game::scene_director_enables_find(row->name)) {
        enables = *e;
    }
    enables.anti_air = flag;                                   // 0071C246, +221h
    bsp::game::scene_director_enables_set(row->name, enables);
    if (!flag) ++summary_.aa_enable_disables;
    log_.notef("  AAEnable 0089c740: \"%s\" aaEnabled=%d (packet cc9_lua_aa_enable)",
        row->name.c_str(), flag ? 1 : 0);
    log_.implemented("MissionLuaNative::AAEnable", "0089c740");
    return 0;
}

// Packet cc9_lua_add_damage. 0088E000 AddDamage(entity, amount); returns no value.
// SUBSTITUTION (labelled): an entity with no units-host slot is not reached.
int GameMissionLuaHost::run_add_damage_0088e000(lua_State* state, int argument_count) {
    ++summary_.add_damage_calls;
    const float amount = argument_count >= 2 ? static_cast<float>(::lua_tonumber(state, 2)) : 0.0f;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || units->gunnery() == nullptr || id <= 0
        || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.add_damage_unresolved;
        log_.notef("  AddDamage 0088e000: entity id %d has no units-host slot (packet "
            "cc9_lua_add_damage)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    units->gunnery()->apply_script_damage_0095da00(index, amount);   // 0088E15B vtable[1ACh]
    ++summary_.add_damage_units;
    const GameUnitRow* row = units->unit_row(index);
    log_.notef("  AddDamage 0088e000: \"%s\" amount=%.1f (packet cc9_lua_add_damage)",
        row != nullptr ? row->name.c_str() : "?", static_cast<double>(amount));
    log_.implemented("MissionLuaNative::AddDamage", "0088e000");
    return 0;
}

// Packet cc9_lua_hit_listeners. 00988510, the `hit` channel, fed by 0077CE60 on
// every applied hit. The host's hit rows come from the gunnery host's queue.
// SUBSTITUTIONS (labelled): the channel is evaluated at the host's frame, for the
// hits since the last frame; attackType compares the bullet class's `Type`
// case-insensitively; damageCaused is matched as [min, max] on the applied damage;
// an entry with a non-empty targetDevice, attackerPlayerIndex (until
// kLuaHitAttackerPlayerIndexBound), fireCaused or
// leakCaused set is not matched (counted); callbacks are called with no argument.
void GameMissionLuaHost::dispatch_hit_listeners_00988510() {
    if (units_hooks_ == nullptr || state_ == nullptr) return;
    GameGunneryHost* gunnery = units_hooks_->gunnery();
    if (gunnery == nullptr) return;
    const std::vector<GameGunneryHost::GameGunneryHitEvent> events = gunnery->take_hit_events();
    if (!kLuaListenersBound || !kLuaHitListenersBound) return;
    for (const GameGunneryHost::GameGunneryHitEvent& hit : events) {
        ++summary_.listener_hit_events;
        if (kLuaHitRateLimitBound) {
            // Packet cc9_hit_rate_limit. 00988949 (009882F0) and 00988957 (00499030) look the pair up
            // (the stored float, 0.0 for a new pair); 00988964..00988969 FCOMIP / JBE
            // compare it with DAT_00F876A4 and skip the whole evaluation when it
            // is still ahead; otherwise it stores clock + 2.0 (00CE3958), or clock
            // + 1e-4 (00CE3C68) for the kinds 8..0Fh, 12h and 13h.
            // SUBSTITUTIONS (labelled): the kind is the event's ordnance_kind, the
            // fired class's bullet_sub_type (the id space the attack-type name
            // table 00E08E58 indexes; a kamikaze shooter's forced 11h is not
            // modelled); the list test at this+1A4h..+1ACh that also shortens the
            // interval to 1e-4 is not modelled.
            const auto key = std::make_pair(hit.victim, hit.shooter);
            float& next = hit_rate_limit_[key];
            const float clock = static_cast<float>(spawn_world_clock_);
            if (!(next <= clock)) {
                ++summary_.listener_hit_throttled;
                continue;
            }
            const int kind = hit.ordnance_kind;
            const bool quick = (kind >= 0x08 && kind <= 0x0F) || kind == 0x12 || kind == 0x13;
            next = static_cast<float>(clock + (quick ? 9.99999974737875e-05 : 2.0));
            ++summary_.listener_hit_throttle_passed;
        }
        const int victim = static_cast<int>(hit.victim + 1);
        const int shooter = static_cast<int>(hit.shooter + 1);
        const GameBulletClassRow* bullet = gunnery->bullet_class_row(hit.bullet_class);
        const std::string type = bullet != nullptr ? bullet->type : std::string();
        std::vector<std::string> callbacks;
        for (const ListenerEntry& entry : listeners_) {
            if (!listener_key_equal(entry.channel, "hit")) continue;
            auto holds = [](const std::vector<int>& set, int v) {
                return set.empty() || std::find(set.begin(), set.end(), v) != set.end();
            };
            if (!holds(entry.entity_ids, victim) || !holds(entry.attacker_ids, shooter)) continue;
            if (!entry.attack_types.empty()) {
                bool any = false;
                for (const std::string& t : entry.attack_types) {
                    if (listener_key_equal(t, type)) { any = true; break; }
                }
                if (!any) continue;
            }
            if (entry.damage_range.size() >= 2
                && (hit.damage < entry.damage_range[0] || hit.damage > entry.damage_range[1])) {
                continue;
            }
            // Packet cc9_hit_listener_filters. The host's hits carry no device entity,
            // so a non-empty targetDevice set never holds the parameter; they start no
            // fire and no leak, so fireCaused and leakCaused are 0.0 against the range
            // (the same two-value bracket damageCaused takes).
            if (entry.hit_device_filter) continue;
            // Packet cc9_hit_attacker_player_index: [src+1Ch], the shot's stamped team
            // (-1 with no ordnance, and for the host's torpedoes and depth charges).
            if (!holds(entry.attacker_player_indices, hit.attacker_player_index)) continue;
            if (entry.fire_range.size() >= 2
                && (0.0f < entry.fire_range[0] || 0.0f > entry.fire_range[1])) {
                continue;
            }
            if (entry.leak_range.size() >= 2
                && (0.0f < entry.leak_range[0] || 0.0f > entry.leak_range[1])) {
                continue;
            }
            if (entry.hit_filters_unmodelled) {
                ++summary_.listener_hit_unmodelled;
                continue;
            }
            if (!entry.callback.empty()) callbacks.push_back(entry.callback);
        }
        for (const std::string& name : callbacks) {
            const int top = ::lua_gettop(state_);
            lua_getfield(state_, LUA_GLOBALSINDEX, name.c_str());
            if (!lua_isfunction(state_, -1)) {
                ::lua_settop(state_, top);
                continue;
            }
            ++summary_.listener_hit_fires;
            const GameUnitRow* row = units_hooks_->unit_row(hit.victim);
            log_.notef("  hit listener 00988510: target \"%s\" type \"%s\" damage %.1f -> %s() "
                "(packet cc9_lua_hit_listeners)", row != nullptr ? row->name.c_str() : "?",
                type.c_str(), static_cast<double>(hit.damage), name.c_str());
            if (::lua_pcall(state_, 0, 0, 0) != 0) {
                const char* message = lua_tolstring(state_, -1, nullptr);
                note_error(message != nullptr ? message : std::string("(no message)"));
            }
            ::lua_settop(state_, top);
        }
    }
}

// Packet cc9_forced_recon_level. 008AA8F0 SetForcedReconLevel(entity, level,
// party): argument 0 through 00888AA0, arguments 1 and 2 as integers (00B66290),
// then 00805CF0(level) on the entity's record for that party; a squadron (18h) or
// LandConvoy (1Ah) forces each +3CCh member instead.
// SUBSTITUTIONS (labelled): the force is published at the next recon pass, not by
// 00805CF0's immediate notify; a LandConvoy forces its own slot (no member vector);
// an entity with no units-host slot is counted unresolved.
int GameMissionLuaHost::run_set_forced_recon_level_008aa8f0(lua_State* state,
    int argument_count) {
    ++summary_.forced_recon_calls;
    const int level = argument_count >= 2 ? static_cast<int>(::lua_tonumber(state, 2)) : 0;
    const int party = argument_count >= 3 ? static_cast<int>(::lua_tonumber(state, 3)) : 0;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.forced_recon_unresolved;
        log_.notef("  SetForcedReconLevel 008aa8f0: entity id %d has no units-host slot "
            "(packet cc9_forced_recon_level)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    std::vector<std::size_t> targets;
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit == bsp::kPlaneSquadronNoUnit || r.squadron_unit != index) continue;
        for (std::size_t member : r.member_units) {
            if (member != bsp::kPlaneSquadronNoUnit) targets.push_back(member);
        }
        break;
    }
    if (targets.empty()) targets.push_back(index);
    for (std::size_t unit : targets) {
        bsp::set_forced_recon_level_00805cf0(unit, party, level);
        ++summary_.forced_recon_units;
    }
    const GameUnitRow* row = units->unit_row(index);
    log_.notef("  SetForcedReconLevel 008aa8f0: \"%s\" level=%d party=%d units=%zu (packet "
        "cc9_forced_recon_level)", row != nullptr ? row->name.c_str() : "?", level, party,
        targets.size());
    log_.implemented("MissionLuaNative::SetForcedReconLevel", "008aa8f0");
    return 0;
}

void GameMissionLuaHost::set_world_border_zones(const bsp::WorldMapBounds& bounds) {
    border_bounds_ = bounds;
    border_zones_ = bsp::build_border_zones_004d5bd0(bounds);
    border_zones_loaded_ = true;
}

// Packet cc9_get_closest_border_zone. 008AECD0 GetClosestBorderZone(position
// [, offset]): argument 0 through the vector reader (008AED9B..), the offset
// 500.0 (00CE397C) unless exactly two arguments are given and argument 1 is a
// number (00B663F0 == 2, 008AEE2F..008AEE65); then
// bsp::get_closest_border_zone_008aecd0 and 0088BA30's x/y/z table.
// SUBSTITUTIONS (labelled): argument 0 is read by the host's 00888760 reader
// (named keys x, y, z); with no zone records (the Map block lacked
// MultiPlayMapSizes, or no load step ran) the image's outputs are unwritten
// stack, and the host answers the position itself, counted `missing`.
int GameMissionLuaHost::run_get_closest_border_zone_008aecd0(lua_State* state,
    int argument_count) {
    ++summary_.border_zone_calls;
    float position[3] = {0.0f, 0.0f, 0.0f};
    if (argument_count >= 1) read_vector3_00888760(state, 1, position);
    float offset = 500.0f;
    if (argument_count == 2 && ::lua_type(state, 2) == LUA_TNUMBER)
        offset = static_cast<float>(::lua_tonumber(state, 2));
    const std::array<float, 3> p{position[0], position[1], position[2]};
    bool found = false;
    std::array<float, 3> out = p;
    if (border_zones_loaded_)
        out = bsp::get_closest_border_zone_008aecd0(border_bounds_, border_zones_, p, offset, found);
    if (!found) ++summary_.border_zone_missing;
    lua_createtable(state, 0, 3);
    ::lua_pushnumber(state, static_cast<lua_Number>(out[0]));
    lua_setfield(state, -2, bsp::kPositionTableKeyX);
    ::lua_pushnumber(state, static_cast<lua_Number>(out[1]));
    lua_setfield(state, -2, bsp::kPositionTableKeyY);
    ::lua_pushnumber(state, static_cast<lua_Number>(out[2]));
    lua_setfield(state, -2, bsp::kPositionTableKeyZ);
    log_.notef("  GetClosestBorderZone 008aecd0: (%.2f, %.2f, %.2f) offset %.1f -> (%.2f, "
        "%.2f, %.2f)%s (packet cc9_get_closest_border_zone)", static_cast<double>(p[0]),
        static_cast<double>(p[1]), static_cast<double>(p[2]), static_cast<double>(offset),
        static_cast<double>(out[0]), static_cast<double>(out[1]), static_cast<double>(out[2]),
        found ? "" : " no zone");
    log_.implemented("MissionLuaNative::GetClosestBorderZone", "008aecd0");
    return 1;
}

// Packet cc9_set_invincible_native. 00897A50 SetInvincible(entity, value):
// argument 0 through 00888AA0; argument 1 is a boolean (00897B6F) giving 1.0 or
// 0.0, otherwise its number, a fraction of maximum health (a nil or missing
// argument reads 0.0, the release). 00897C63 calls vtable[F4h] = 0042ED80, which
// stores unit+150h and passes the value to every child's vtable[F4h].
// bsp::lua_set_invincible_00897a50 (src/unit_damage.cpp) reconstructs the rule.
// 008C1930 OverrideHP(unit, hp): argument 0 through 00888AA0 (ESI = the unit),
// argument 1 as a number. 008C1A71 stores it at unit+36Ch (maximum health) and
// 008C1AA8 calls 00877B90(unit, hp) with the same number. The write is the
// gunnery host's (GameGunneryHost::override_hp_008c1930, kLuaOverrideHpBound).
int GameMissionLuaHost::run_override_hp_008c1930(lua_State* state, int argument_count) {
    float value = 0.0f;
    if (argument_count >= 2) value = static_cast<float>(::lua_tonumber(state, 2));
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || units->gunnery() == nullptr || id <= 0
        || static_cast<std::size_t>(id) > units->count()) {
        log_.notef("  OverrideHP 008c1930: entity id %d has no units-host slot (packet "
            "cc9_override_hp)", id);
        return 0;
    }
    units->gunnery()->override_hp_008c1930(static_cast<std::size_t>(id - 1), value);
    log_.implemented("MissionLuaNative::OverrideHP", "008c1930");
    return 0;
}

int GameMissionLuaHost::run_set_invincible_00897a50(lua_State* state, int argument_count) {
    ++summary_.invincible_calls;
    float value = 0.0f;
    if (argument_count >= 2 && ::lua_type(state, 2) == LUA_TBOOLEAN) {
        value = ::lua_toboolean(state, 2) != 0 ? 1.0f : 0.0f;
    } else if (argument_count >= 2) {
        value = static_cast<float>(::lua_tonumber(state, 2));
    }
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || units->gunnery() == nullptr || id <= 0
        || static_cast<std::size_t>(id) > units->count()) {
        // SUBSTITUTION (labelled): an entity with no units-host slot is not
        // reached; the image would store its +150h too.
        ++summary_.invincible_unresolved;
        log_.notef("  SetInvincible 00897a50: entity id %d has no units-host slot (packet "
            "cc9_set_invincible_native)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    GameGunneryHost& gunnery = *units->gunnery();
    std::vector<std::size_t> targets{index};
    // 0042ED80's fan-out: a squadron's fused slot carries its live members.
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit == bsp::kPlaneSquadronNoUnit || r.squadron_unit != index) continue;
        for (std::size_t member : r.member_units) {
            if (member != bsp::kPlaneSquadronNoUnit && member != index) targets.push_back(member);
        }
        break;
    }
    for (std::size_t unit : targets) {
        gunnery.set_unit_invincibility(unit, value);
        ++summary_.invincible_units;
    }
    const GameUnitRow* row = units->unit_row(index);
    log_.notef("  SetInvincible 00897a50: \"%s\" value=%.3f units=%zu (packet "
        "cc9_set_invincible_native)", row != nullptr ? row->name.c_str() : "?", value,
        targets.size());
    log_.implemented("MissionLuaNative::SetInvincible", "00897a50");
    return 0;
}

// Packet cc9_lua_kill. 008AC5C0 Kill(entity [, hard]); returns no value (the
// native's frame result count is 0).
int GameMissionLuaHost::run_kill_008ac5c0(lua_State* state, int argument_count) {
    ++summary_.kill_calls;
    // 008AC6DF..008AC71B: cause 1, or 2 when a second argument reads true
    // (bsp::kill_cause_from_lua_008ac5c0, src/unit_damage.cpp).
    const bool has_second = argument_count > 1;
    const bool hard = has_second && ::lua_toboolean(state, 2) != 0;
    const int cause = hard ? 2 : 1;
    // 00888AA0 resolves argument 0; the host's entity id is the units-host index
    // plus one. SUBSTITUTION (labelled): an entity with no units-host slot (a
    // script entity such as Mission.CamScript, a path, a marker) is not killed;
    // the image would run 00926D90 on it.
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || units->gunnery() == nullptr || id <= 0
        || static_cast<std::size_t>(id) > units->count()) {
        // Packet cc9_lua_kill_script_entity: a script entity's `Ptr` is its
        // GameScriptEntity; 008AC756's 00926D90 runs on it through the script
        // orders host (Dead set, think lists dropped, self table rebuilt).
        if (kLuaKillScriptEntityBound && script_orders_ != nullptr
            && ::lua_type(state, 1) == LUA_TTABLE) {
            ::lua_getfield(state, 1, "Ptr");
            void* const ptr = ::lua_touserdata(state, -1);
            ::lua_pop(state, 1);
            if (ptr != nullptr && script_orders_->is_script_entity(ptr)) {
                script_orders_->kill_script_entity(ptr, cause);
                log_.notef("  Kill 008ac5c0: script entity id %d killed (00926D90, packet "
                    "cc9_lua_kill_script_entity)", id);
                log_.implemented("MissionLuaNative::Kill", "008ac5c0");
                return 0;
            }
        }
        ++summary_.kill_unresolved;
        log_.notef("  Kill 008ac5c0: entity id %d has no units-host slot, not killed (packet "
            "cc9_lua_kill)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    GameGunneryHost& gunnery = *units->gunnery();
    std::vector<std::size_t> victims;
    // 008AC729: a squadron (IsKindOf 18h) kills its members through 007ED380.
    // The host fuses the squadron with its wing-0 plane in one slot, so the
    // registry record's live members stand for the +3D0h array.
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit == bsp::kPlaneSquadronNoUnit || r.squadron_unit != index) continue;
        ++summary_.kill_squadrons;
        for (std::size_t member : r.member_units) {
            if (member != bsp::kPlaneSquadronNoUnit) victims.push_back(member);
        }
        break;
    }
    // 008AC740: a LandConvoy (1Ah) kills its vector through 00742210.
    // SUBSTITUTION (labelled): the host keeps no convoy member vector, so the
    // convoy's own slot stands for it.
    if (victims.empty()) victims.push_back(index);   // 008AC756, 00926D90
    const GameUnitRow* row = units->unit_row(index);
    for (std::size_t unit : victims) {
        if (gunnery.unit_dead(unit)) {
            // 00926D90 returns when +5Fh is already set.
            ++summary_.kill_already_dead;
            continue;
        }
        // SUBSTITUTION (labelled): the gunnery host's funnel is the host's one
        // death path. It ignores the cause (1 or 2 lands in the image's +70h)
        // and counts the call in its water-depth census.
        gunnery.kill_unit_00926d90(unit, cause);
        ++summary_.kill_units;
    }
    log_.notef("  Kill 008ac5c0: \"%s\" cause=%d victims=%zu (packet cc9_lua_kill)",
        row != nullptr ? row->name.c_str() : "?", cause, victims.size());
    log_.implemented("MissionLuaNative::Kill", "008ac5c0");
    return 0;
}

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
    // Packet cc9_get_property_class_readers: 00927AD0 and 00779BB0 run before
    // any class reader, so their two keys are tested first.
    const int class_pushed = run_get_property_class_readers(state, key);
    if (class_pushed > 0) {
        ++summary_.get_property_served;
        log_.implemented("MissionLuaNative::GetProperty", "0088bf80");
        return class_pushed;
    }
    if (class_pushed == 0) {
        ++summary_.get_property_unserved;
        return 0;
    }

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

float GameMissionLuaHost::read_vehicle_class_nested_number(int index, const char* key,
    const char* nested_key, float fallback) {
    if (state_ == nullptr || index < 0 || key == nullptr || nested_key == nullptr) {
        return fallback;
    }
    const int top = ::lua_gettop(state_);
    float value = fallback;
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_pushinteger(state_, index);
        ::lua_gettable(state_, -2);
        if (::lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, key);
            if (::lua_type(state_, -1) == LUA_TTABLE) {
                ::lua_getfield(state_, -1, nested_key);
                if (::lua_type(state_, -1) == LUA_TNUMBER) {
                    value = static_cast<float>(::lua_tonumber(state_, -1));
                }
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

namespace {

struct LoadoutCacheKey {
    int class_id;
    int equipment;
    bool operator<(const LoadoutCacheKey& o) const {
        return class_id != o.class_id ? class_id < o.class_id : equipment < o.equipment;
    }
};
std::map<LoadoutCacheKey, std::pair<bool, std::vector<bsp::AiPlaneLoadoutEntryFacts>>>&
loadout_cache() {
    static std::map<LoadoutCacheKey, std::pair<bool, std::vector<bsp::AiPlaneLoadoutEntryFacts>>>
        cache;
    return cache;
}
std::map<int, int>& equipment_count_cache() {
    static std::map<int, int> cache;
    return cache;
}

// 00425850's case-insensitive compare, which 004431xx's Type chain uses.
bool type_equals(const std::string& a, const char* b) {
    if (a.size() != std::strlen(b)) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        char x = a[i];
        char y = b[i];
        if (x >= 'A' && x <= 'Z') x = static_cast<char>(x - 'A' + 'a');
        if (y >= 'A' && y <= 'Z') y = static_cast<char>(y - 'A' + 'a');
        if (x != y) return false;
    }
    return true;
}

// One Lua chunk per (class, equipment): each entry as
// "ammo;reload;deviceType;hasBullet;bulletType;dmin;dmax;bmin;bmax;ignition;antiair;
// capturePower;captureDuration;damage;carriedFound;carriedType;carriedBmin;carriedBmax",
// '|' between entries, in pairs() order (00961F57 walks the table with the same
// next()). DamageMax and BlastDamageMax default to their minimum
// (docs/WEAPON_CLASS_DESCRIPTOR.md); `Bullets`, else `BulletClass`, is the table.
// The last seven fields are packet cc9_ai_loadout_carried_terms: MParatrooper's
// CapturePower / CaptureDuration / Damage (007AC780, plain numbers, nil as 0)
// and a DummyKamikazePlane's VehicleClass[KamikazePlaneClass].KamikazeBulletClass
// blast pair and Type (006FF170 +DCh, then the class's +210h).
std::string read_loadout_text(lua_State* state, int class_id, int equipment) {
    char chunk[4096];
    std::snprintf(chunk, sizeof(chunk),
        "local c = type(VehicleClass) == 'table' and VehicleClass[%d] or nil\n"
        "if type(c) ~= 'table' or type(c.Equipments) ~= 'table' then return nil end\n"
        "local e = c.Equipments[%d]\n"
        "if type(e) ~= 'table' then return nil end\n"
        "local BT = type(Bullets) == 'table' and Bullets\n"
        "   or (type(BulletClass) == 'table' and BulletClass or nil)\n"
        "local out = {}\n"
        "for k, v in pairs(e) do\n"
        "  if type(v) == 'table' then\n"
        "    local d = type(DeviceClass) == 'table' and DeviceClass[tonumber(v.Platform) or -1] or nil\n"
        "    local dt = (type(d) == 'table' and type(d.Type) == 'string') and d.Type or ''\n"
        "    local hb, bt, dmin, dmax, bmin, bmax, ign, aa = 0, '', 0, 0, 0, 0, 0, 0\n"
        "    local cp, cd, dg, kf, kt, kmin, kmax = 0, 0, 0, 0, '', 0, 0\n"
        "    if type(d) == 'table' and type(d.Bullet) == 'table' and type(d.Bullet[1]) == 'table' then\n"
        "      local b = BT and BT[d.Bullet[1].Bullet] or nil\n"
        "      if type(b) == 'table' then\n"
        "        hb = 1\n"
        "        bt = type(b.Type) == 'string' and b.Type or ''\n"
        "        dmin = tonumber(b.DamageMin) or 0\n"
        "        dmax = tonumber(b.DamageMax) or dmin\n"
        "        if type(b.Blast) == 'table' then\n"
        "          bmin = tonumber(b.Blast.BlastDamageMin) or 0\n"
        "          bmax = tonumber(b.Blast.BlastDamageMax) or bmin\n"
        "        end\n"
        "        ign = tonumber(b.IgnitionDelay) or 0\n"
        "        aa = (b.AntiAir == true) and 1 or 0\n"
        "        cp = tonumber(b.CapturePower) or 0\n"
        "        cd = tonumber(b.CaptureDuration) or 0\n"
        "        dg = tonumber(b.Damage) or 0\n"
        "        local kc = type(VehicleClass) == 'table' and VehicleClass[tonumber(b.KamikazePlaneClass) or -1] or nil\n"
        "        local kb = (type(kc) == 'table' and BT) and BT[tonumber(kc.KamikazeBulletClass) or -1] or nil\n"
        "        if type(kb) == 'table' then\n"
        "          kf = 1\n"
        "          kt = type(kb.Type) == 'string' and kb.Type or ''\n"
        "          if type(kb.Blast) == 'table' then\n"
        "            kmin = tonumber(kb.Blast.BlastDamageMin) or 0\n"
        "            kmax = tonumber(kb.Blast.BlastDamageMax) or kmin\n"
        "          end\n"
        "        end\n"
        "      end\n"
        "    end\n"
        "    out[#out + 1] = table.concat({math.floor(tonumber(v.Ammo) or 0),\n"
        "      tonumber(v.ReloadTime) or 0, dt, hb, bt, dmin, dmax, bmin, bmax, ign, aa,\n"
        "      cp, cd, dg, kf, kt, kmin, kmax}, ';')\n"
        "  end\n"
        "end\n"
        "return table.concat(out, '|')\n",
        class_id, equipment);
    const int top = ::lua_gettop(state);
    std::string text;
    bool ok = false;
    if (::luaL_loadbuffer(state, chunk, std::strlen(chunk), "bsp_ai_plane_loadout") == 0 &&
        ::lua_pcall(state, 0, 1, 0) == 0 && ::lua_type(state, -1) == LUA_TSTRING) {
        const char* s = lua_tolstring(state, -1, nullptr);
        if (s != nullptr) text = s;
        ok = true;
    }
    ::lua_settop(state, top);
    return ok ? text : std::string("\x01");
}

std::vector<std::string> split_text(const std::string& text, char sep) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    for (;;) {
        const std::size_t at = text.find(sep, start);
        parts.push_back(text.substr(start, at == std::string::npos ? std::string::npos
                                                                     : at - start));
        if (at == std::string::npos) break;
        start = at + 1;
    }
    return parts;
}

}  // namespace

bool game_ai_plane_loadout(int class_id, int equipment,
                           std::vector<bsp::AiPlaneLoadoutEntryFacts>& out) {
    out.clear();
    // 009552E0: a loadout at or below 0 answers null here (the negative arm
    // reads class +134h, which 00A08460 never reaches: 00A0864F JLE).
    if (g_loadout_lua_state == nullptr || class_id < 0 || equipment <= 0) return false;
    const LoadoutCacheKey key{class_id, equipment};
    auto& cache = loadout_cache();
    const auto hit = cache.find(key);
    if (hit != cache.end()) {
        out = hit->second.second;
        return hit->second.first;
    }
    const std::string text = read_loadout_text(g_loadout_lua_state, class_id, equipment);
    const bool present = text != "\x01";
    if (present && !text.empty()) {
        for (const std::string& row : split_text(text, '|')) {
            const std::vector<std::string> f = split_text(row, ';');
            if (f.size() != 18) continue;
            bsp::AiPlaneLoadoutEntryFacts e;
            e.ammo = std::atoi(f[0].c_str());
            e.reload = static_cast<float>(std::atof(f[1].c_str()));
            e.device_is_rack = type_equals(f[2], "BombPlatform") ||
                               type_equals(f[2], "MultiBombPlatform");
            // Device class +F4h lies past a BombPlatform class's E8h allocation;
            // no producer is known, so it reads as 0.0 here (LABELLED).
            e.device_f4 = 0.0f;
            if (f[3] == "1") {
                e.bullet.present = true;
                e.bullet.sub_type = bsp::weapon_class_sub_type_for_lua_type(f[4]);
                e.bullet.damage_min = static_cast<float>(std::atof(f[5].c_str()));
                e.bullet.damage_max = static_cast<float>(std::atof(f[6].c_str()));
                e.bullet.blast_min = static_cast<float>(std::atof(f[7].c_str()));
                e.bullet.blast_max = static_cast<float>(std::atof(f[8].c_str()));
                e.bullet.ignition_delay = static_cast<float>(std::atof(f[9].c_str()));
                e.bullet.anti_air = f[10] == "1";
                if constexpr (bsp::kAiLoadoutCarriedTermsBound) {
                    // 00A08E81: the 0Fh scoring reads these only for sub-type 0Fh.
                    if (e.bullet.sub_type == 0x0F) {
                        e.bullet.paratrooper_terms_known = true;
                        e.bullet.paratrooper_d8 = static_cast<float>(std::atof(f[11].c_str()));
                        e.bullet.paratrooper_f8 = static_cast<float>(std::atof(f[12].c_str()));
                        e.bullet.paratrooper_fc = static_cast<float>(std::atof(f[13].c_str()));
                    }
                    if (f[14] == "1") {
                        e.bullet.carried_kamikaze_known = true;
                        e.bullet.carried_sub_type = bsp::weapon_class_sub_type_for_lua_type(f[15]);
                        e.bullet.carried_blast_min = static_cast<float>(std::atof(f[16].c_str()));
                        e.bullet.carried_blast_max = static_cast<float>(std::atof(f[17].c_str()));
                    }
                }
            }
            out.push_back(e);
        }
    }
    cache[key] = {present, out};
    return present;
}

int game_ai_plane_equipment_count(int class_id) {
    if (g_loadout_lua_state == nullptr || class_id < 0) return 0;
    auto& cache = equipment_count_cache();
    const auto hit = cache.find(class_id);
    if (hit != cache.end()) return hit->second;
    // 00961F57 appends one loadout per Equipments entry, so +128h is the
    // table's entry count, not its length operator.
    char chunk[512];
    std::snprintf(chunk, sizeof(chunk),
        "local c = type(VehicleClass) == 'table' and VehicleClass[%d] or nil\n"
        "if type(c) ~= 'table' or type(c.Equipments) ~= 'table' then return '0' end\n"
        "local n = 0\n"
        "for _ in pairs(c.Equipments) do n = n + 1 end\n"
        "return tostring(n)\n", class_id);
    lua_State* state = g_loadout_lua_state;
    const int top = ::lua_gettop(state);
    int count = 0;
    if (::luaL_loadbuffer(state, chunk, std::strlen(chunk), "bsp_ai_equipment_count") == 0 &&
        ::lua_pcall(state, 0, 1, 0) == 0 && ::lua_type(state, -1) == LUA_TSTRING) {
        const char* s = lua_tolstring(state, -1, nullptr);
        if (s != nullptr) count = std::atoi(s);
    }
    ::lua_settop(state, top);
    cache[class_id] = count;
    return count;
}

std::string GameMissionLuaHost::read_vehicle_class_string(int index, const char* key) {
    // The VehicleClass companion of read_device_class_string.
    std::string value;
    if (state_ == nullptr || index < 0 || key == nullptr) return value;
    const int top = ::lua_gettop(state_);
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
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

std::string GameMissionLuaHost::read_device_class_string(int index, const char* key) {
    // The DeviceClass companion of read_bullet_class_string. `Mesh` is the
    // model the image loads into the weapon class's +50h, which 007325A0 reads
    // for the "fire" Points items. docs/GUN_BARREL_COUNT.md.
    std::string value;
    if (state_ == nullptr || index < 0 || key == nullptr) return value;
    const int top = ::lua_gettop(state_);
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "DeviceClass");
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

bool GameMissionLuaHost::read_resource_file(const std::string& path,
    std::vector<std::uint8_t>& bytes) {
    return resources_ != nullptr && resources_->read_file(path, kScriptReadMode, bytes);
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
        // Packet cc9_device_reload_enabled: 005E2FB2 / 005E3017 write 00E17BF2.
        g_device_reload_enabled_00e17bf2 = flags.reload_payload_on;
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
    std::size_t parties = 0;
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
        if (!kSceneLoadThisTableIdentityBound && entity.class_index >= 0) {
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
        // Packet cc8_ship_drive: 00928F50's mirror, the one other writer of this
        // slot the image has. It is a separate event from the attach - the root
        // entity vtable's slot 11, reached through the adjustor thunk 00951F30 -
        // and it runs when the party is set, which for a scene unit is before
        // the mission's first Think. Both fields are integers (00B67460).
        if (entity.party >= 0) {
            lua_pushinteger(state_, entity.party);
            lua_setfield(state_, -2, "Party");
            ++parties;
        }
        if (entity.race >= 0) {
            lua_pushinteger(state_, entity.race);
            lua_setfield(state_, -2, "Race");
        }
        lua_setfield(state_, -2, key);
        // Packet cc_lua_find_entity: the slot is built for every entity that
        // reaches virtual slot 39; only the entities whose world bucket
        // 0088B1B0 walks enter the name index the FindEntity tail uses.
        if (entity.findable) scene_entity_ids_[entity.name] = entity.id;
        if constexpr (kPendingListDedupBound) {
            // Packet cc9_pending_list_dedup: the load attach stands for the scene
            // read's InitAll, so a node another pusher queued for this id is done.
            load_attached_.insert(entity.id);
            for (std::deque<PendingEntity>::iterator it = pending_entities_.begin();
                 it != pending_entities_.end();) {
                if (it->entity_id == entity.id) {
                    it = pending_entities_.erase(it);
                    ++summary_.dedup_load_dropped;
                } else {
                    ++it;
                }
            }
        }
        ++made;
    }
    ::lua_settop(state_, ::lua_gettop(state_) - 1);
    summary_.self_table_entities = made;
    if constexpr (kSceneLoadThisTableIdentityBound) {
        // Packet cc9_init_identity_gaps. The scene read's InitAll: pass A over
        // every instance (the loop above), then pass B, then pass C.
        for (const SceneEntity& entity : entities) {
            if (entity.class_index < 0) continue;
            // Pass B of every unit class reaches 009292B0 (00822CDB, 007D5DAC,
            // 007F218E, 006D3C2D; section 1.1).
            if (bind_lua_class_009292b0(entity.id, entity.class_index, entity.name)) {
                ++summary_.load_class_bound;
                ++classes;
            }
            log_.implemented("SceneLoad::pass B bind_lua_class", "009292b0");
        }
        for (const SceneEntity& entity : entities) {
            if (entity.class_index >= 0 || entity.marker_class_id < 0) continue;
            const char* const type_name = default_pass_c_type_name(entity.marker_class_id);
            if (type_name == nullptr) continue;  // the class has its own pass C
            // 009295B0 with no kind-3 holder: 009297E4 CALL 00928100. +54h is
            // the authored Party (00927050, kind 1) or 00925E1D's 2.
            const int party = entity.marker_authored_party >= 0
                ? entity.marker_authored_party : kEntityDefaultParty00925e1d;
            if (mirror_identity_00928100(entity.id, party, entity.name, type_name)) {
                ++summary_.load_identity_mirrored;
            }
            log_.implemented("SceneLoad::pass C mirror_identity", "00928100");
        }
    }
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
    log_.notef("thisTable: %zu slot(s) carry 00928f50's `Party` mirror. A slot without it "
        "makes commandhelpers.lua:330 `recon[targetUnit.Party][allegiance]` index a nil, "
        "which is what reverted the GetSelectedUnit binding in packet cc8_ship_moveonpath; "
        "the caller supplies the value and a negative one means it does not know it", parties);
    return made;
}

bool GameMissionLuaHost::push_resolved_entity(lua_State* state, const char* binding_name,
    int argument_count) {
    if (state == nullptr || binding_name == nullptr) return false;
    if (scene_entity_ids_.empty()) return false;
    // FindEntity, and since packet cc8_ship_moveonpath, GetSelectedUnit.
    // FindEntity's argument is a name and 00925a90 answers the scene database's
    // entity of that name. GetSelectedUnit 008AB070 takes no argument: it reads
    // the global at 008AB14D, jumps to the nil arm at 008AB15C when it is null,
    // and otherwise formats the entity's uint16 at +174h (008AB162 MOVZX EAX,
    // word ptr [EAX+174h]) and indexes the same `thisTable` FindEntity's tail
    // uses. The global is 00E188D8, stored at 004C0893 in 004C0890, which this
    // process runs as GameUnitsHost::set_controlled_unit_004c0890, so the
    // selected unit is the controlled one and the key is its entity id.
    // MEASURED, packet cc8_ship_moveonpath, and it is why GetSelectedUnit is
    // still nil here. Binding it is one line - the controlled unit's entity id,
    // `units.controlled_bound` standing for the null test at 008AB15C - and
    // `local/mop_after_usn04.log` is the run that did exactly that. The mission
    // then died on every frame:
    //
    //   script call Think failed: commandhelpers.lua:330: attempt to index
    //   field '?' (a nil value)
    //     commandhelpers.lua:330 in luaGetShipsAround
    //     commandhelpers.lua:13294 in luaCheckMusic
    //     usn_19_coralus.lua:490
    //
    // `luaCheckMusic` returns early while GetSelectedUnit answers nil; with a
    // unit it reaches `luaGetShipsAround`, whose line 330 is
    // `pairs(recon[targetUnit.Party][allegiance])` with
    // `targetUnit = thisTable[target.ID]`. The slot 00928A00 builds here carries
    // ID, Dead, Ptr and Class and NO `Party`, so `recon[nil]` is nil and the
    // index raises. USN04's whole Think aborts from frame 61 on, the 98
    // NavigatorMoveOnPath calls never happen and the world ends at 45 units
    // instead of 57.
    //
    // So this row is a HOLE with a named cause, not an unread native: the
    // missing half is `Party` (and `Name`, which the same file's helpers read)
    // on the thisTable slot, plus whatever fills the `recon` table per party.
    // 008AB070 itself is read - 008AB14D the global 00E188D8, 008AB15C the null
    // arm, 008AB162 MOVZX EAX,word ptr [EAX+174h] then the same registry index.
    int entity_id = 0;
    if (std::strcmp(binding_name, "FindEntity") == 0) {
        if (argument_count < 1 || lua_type(state, 1) != LUA_TSTRING) return false;
        const char* name = lua_tolstring(state, 1, nullptr);
        if (name == nullptr) return false;
        // DIAGNOSTIC, env-gated (BSP_LUA_FIND_ENTITY_MISSES=1): the names
        // FindEntity answers nil for, and the case-folded hits. Prints nothing
        // when unset.
        static const bool trace = [] {
            char* v = nullptr;
            std::size_t n = 0;
            const bool on = _dupenv_s(&v, &n, "BSP_LUA_FIND_ENTITY_MISSES") == 0
                && v != nullptr && v[0] == '1';
            std::free(v);
            return on;
        }();
        std::map<std::string, int>::const_iterator found = scene_entity_ids_.find(name);
        if (kFindEntityCaseInsensitiveBound && found == scene_entity_ids_.end()) {
            // 009251F0's 00438E10 -> _stricmp: the whole string, ASCII case
            // folded. LABELLED: the registry's walk order is not modelled; an
            // exact hit wins, then the first case-folded match in name order.
            for (found = scene_entity_ids_.begin(); found != scene_entity_ids_.end(); ++found) {
                if (_stricmp(found->first.c_str(), name) == 0) break;
            }
            if (trace && !error_replay_ && found != scene_entity_ids_.end()) {
                log_.notef("FindEntity case-folded: \"%s\" -> \"%s\"", name,
                    found->first.c_str());
            }
        }
        if (found == scene_entity_ids_.end()) {
            if (trace && !error_replay_) log_.notef("FindEntity miss: \"%s\"", name);
            return false;
        }
        entity_id = found->second;
    } else if (std::strcmp(binding_name, "GetSelectedUnit") == 0) {
        // Packet cc8_ship_drive. The revert above is lifted: `Party` is on the
        // slot now, so 008AB070 can answer. 008AB14D reads the global 00E188D8,
        // 008AB15C is the nil arm when it is null, and 008AB162 MOVZX EAX,word
        // ptr [EAX+174h] formats the entity's own id into the same thisTable
        // key. The null test is the units host's controlled_bound.
        if (script_orders_ == nullptr) return false;
        const GameUnitsHost& units = script_orders_->units();
        const GameUnitsSummary& summary = units.summary();
        if (!summary.controlled_bound) return false;
        const GameUnitRow* row = units.unit_row(summary.controlled_index);
        if (row == nullptr || row->name.empty()) return false;
        const std::map<std::string, int>::const_iterator found
            = scene_entity_ids_.find(row->name);
        if (found == scene_entity_ids_.end()) return false;
        entity_id = found->second;
    } else {
        return false;
    }
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, entity_id);
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
    log_.notef("summary SEntity::InitAll 00925f20 bound=%d calls=%llu nonempty=%llu "
        "entities=%llu wing_appended=%llu wing_marked=%llu pushes=%llu still_pending=%zu",
        kSEntityInitAllBound ? 1 : 0, summary_.init_all_calls, summary_.init_all_nonempty,
        summary_.init_all_entities, summary_.init_all_wing_appended,
        summary_.init_all_wing_marked,
        summary_.init_all_pushes, pending_entities_.size());
    log_.notef("summary SEntity::InitAll thisTable steps bound=%d class_bound=%llu "
        "squadron_ids=%llu think_names=0",
        kSEntityInitThisTableStepsBound ? 1 : 0, summary_.init_all_class_bound,
        summary_.init_all_squadron_ids);
    log_.notef("summary mission script generated entity party (packet "
        "cc9_generated_entity_party, 00944FD0 -> 00928A00): bound=%d nodes=%llu writes=%llu",
        kGeneratedEntityPartyBound ? 1 : 0, summary_.generated_party_nodes,
        summary_.generated_party_writes);
    log_.notef("summary SEntity::InitAll pass E bound=%d released=%llu",
        kSEntityInitPassEReleaseBound ? 1 : 0, summary_.init_all_holders_released);
    log_.notef("summary SEntity::InitAll pending dedup bound=%d skipped_pending=%llu "
        "skipped_attached=%llu squadron_upgrades=%llu wing_deferred=%llu "
        "wing_append_skipped=%llu load_dropped=%llu",
        kPendingListDedupBound ? 1 : 0, summary_.dedup_skipped_pending,
        summary_.dedup_skipped_attached, summary_.dedup_squadron_upgrades,
        summary_.dedup_wing_deferred, summary_.dedup_wing_append_skipped,
        summary_.dedup_load_dropped);
    log_.notef("summary SEntity::InitAll route pushes retired bound=%d retired=%llu "
        "squadron_annotations=%llu fallback_pushes=%llu",
        kRoutePushesRetiredBound ? 1 : 0, summary_.route_pushes_retired,
        summary_.route_squadron_annotations, summary_.route_fallback_pushes);
    log_.notef("summary SEntity::InitAll load walk bound=%d pushes=%llu mirrored=%llu",
        kLoadTimeInitAllBound ? 1 : 0, summary_.load_init_all_pushes,
        summary_.init_all_identity_mirrored);
    log_.notef("summary SEntity::InitAll load squadron hooks bound=%d squadrons=%llu "
        "(007F4580 / 007F4BA0 at 0046EB4B, packet cc9_load_time_squadron_hooks)",
        kLoadTimeSquadronHooksBound ? 1 : 0, summary_.load_squadron_nodes);
    log_.notef("summary SEntity::InitAll load wing squadron ids bound=%d marked=%llu "
        "(007F4B49 +9D4h -> 007C9770, packet cc9_load_wing_squadron_id)",
        kLoadWingSquadronIdBound ? 1 : 0, summary_.load_wing_marked);
    log_.notef("summary SceneLoad thisTable identity bound=%d class_bound=%llu mirrored=%llu",
        kSceneLoadThisTableIdentityBound ? 1 : 0, summary_.load_class_bound,
        summary_.load_identity_mirrored);
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
    log_.notef("summary mission script listeners bound=%d adds=%llu removes=%llu queries=%llu "
        "registered=%zu kill_deaths=%llu kill_fires=%llu attacker_filtered=%llu (008C6760 / "
        "008C6990 / 008C6BB0 / 009813A0, packet cc9_lua_listeners)", kLuaListenersBound ? 1 : 0,
        summary_.listener_adds, summary_.listener_removes, summary_.listener_queries,
        listeners_.size(), summary_.listener_kill_deaths, summary_.listener_kill_fires,
        summary_.listener_attacker_filtered);
    log_.notef("summary mission script hit listeners bound=%d events=%llu fires=%llu "
        "unmodelled=%llu (00988510, packet cc9_lua_hit_listeners)", kLuaHitListenersBound ? 1 : 0,
        summary_.listener_hit_events, summary_.listener_hit_fires,
        summary_.listener_hit_unmodelled);
    log_.notef("summary mission script hit rate limit bound=%d passed=%llu throttled=%llu "
        "pairs=%zu (00988510 this+168h, packet cc9_hit_rate_limit)", kLuaHitRateLimitBound ? 1 : 0,
        summary_.listener_hit_throttle_passed, summary_.listener_hit_throttled,
        hit_rate_limit_.size());
    log_.notef("summary mission script ship speed bound=%d calls=%llu units=%llu unresolved=%llu "
        "(00890D30 -> 00890E6F, packet cc9_lua_set_ship_speed)", kLuaSetShipSpeedBound ? 1 : 0,
        summary_.ship_speed_calls, summary_.ship_speed_units, summary_.ship_speed_unresolved);
    log_.notef("summary mission script attack target bound=%d calls=%llu fire_arm=%llu "
        "command_arm=%llu pushed=%llu nil=%llu unresolved=%llu (008A6DE0, packet "
        "cc9_unit_get_attack_target)", kLuaUnitGetAttackTargetBound ? 1 : 0,
        summary_.attack_target_calls, summary_.attack_target_fire_arm,
        summary_.attack_target_command_arm, summary_.attack_target_pushed,
        summary_.attack_target_nil, summary_.attack_target_unresolved);
    log_.notef("summary mission script squadron speed bound=%d calls=%llu planes=%llu "
        "unresolved=%llu (0089F780 -> 0074E1E0 -> 007D9E80, packet cc9_squadron_set_speed)",
        kLuaSquadronSetSpeedBound ? 1 : 0, summary_.squadron_speed_calls,
        summary_.squadron_speed_planes, summary_.squadron_speed_unresolved);
    log_.notef("summary mission script class changed bound=%d calls=%llu true=%llu "
        "(008CC4B0, registry+2010h, packet cc9_is_class_changed)",
        kLuaIsClassChangedBound ? 1 : 0, summary_.class_changed_calls,
        summary_.class_changed_true);
    log_.notef("summary mission script submarine depth set bound=%d calls=%llu stored=%llu "
        "unresolved=%llu (00893F40 -> 008528B0, packet cc9_set_submarine_depth_level)",
        kLuaSetSubmarineDepthLevelBound ? 1 : 0, summary_.sub_depth_calls,
        summary_.sub_depth_stored, summary_.sub_depth_unresolved);
    log_.notef("summary mission script air base slot count bound=%d calls=%llu resized=%llu "
        "unresolved=%llu (008963E0 -> 006C7E20, packet cc9_set_air_base_slot_count)",
        kLuaSetAirBaseSlotCountBound ? 1 : 0, summary_.slot_count_calls,
        summary_.slot_count_resized, summary_.slot_count_unresolved);
    log_.notef("summary mission script device reload bound=%d calls=%llu true=%llu "
        "now=%d (008C1350 -> 00E17BF2, packet cc9_device_reload_enabled)",
        kLuaDeviceReloadEnabledBound ? 1 : 0, summary_.device_reload_calls,
        summary_.device_reload_true, lua_device_reload_enabled_00e17bf2() ? 1 : 0);
    log_.notef("summary mission script formation query bound=%d in_formation=%llu true=%llu "
        "leave=%llu left=%llu (008996A0 +284h; 00899EB0 -> 0077C980 -> 0077BD70, packet "
        "cc9_lua_formation_query)", kLuaFormationQueryBound ? 1 : 0,
        summary_.in_formation_calls, summary_.in_formation_true,
        summary_.leave_formation_calls, summary_.leave_formation_left);
    log_.notef("summary mission script untouchable bound=%d calls=%llu marked=%llu "
        "(008AC140 -> +1D4h, packet cc9_add_untouchable_unit)",
        kLuaAddUntouchableUnitBound ? 1 : 0, summary_.untouchable_calls,
        summary_.untouchable_marked);
    log_.notef("summary mission script formation leader bound=%d calls=%llu found=%llu "
        "other=%llu (00899AF0 -> 007788D0, packet cc9_get_formation_leader)",
        kLuaFormationLeaderBound ? 1 : 0, summary_.formation_leader_calls,
        summary_.formation_leader_found, summary_.formation_leader_other);
    log_.notef("summary mission script last catapulted bound=%d calls=%llu nil=%llu "
        "not_ship=%llu (00892860 -> 00953A60, packet cc9_get_last_catapulted)",
        kLuaLastCatapultedBound ? 1 : 0, summary_.last_catapulted_calls,
        summary_.last_catapulted_nil, summary_.last_catapulted_not_ship);
    log_.notef("summary mission script unlimited air bound=%d calls=%llu stored=%llu "
        "(00893C00 -> unit+1280h, packet cc9_submarine_air)", kSubmarineAirBound ? 1 : 0,
        summary_.unlimited_air_calls, summary_.unlimited_air_stored);
    log_.notef("summary mission script aa enable bound=%d calls=%llu disables=%llu "
        "unresolved=%llu (0089C740 -> 0071E050 -> director+221h, packet cc9_lua_aa_enable)",
        kLuaAAEnableBound ? 1 : 0, summary_.aa_enable_calls, summary_.aa_enable_disables,
        summary_.aa_enable_unresolved);
    log_.notef("summary mission script add damage bound=%d calls=%llu units=%llu unresolved=%llu "
        "(0088E000 -> 0095DA00, packet cc9_lua_add_damage)", kLuaAddDamageBound ? 1 : 0,
        summary_.add_damage_calls, summary_.add_damage_units, summary_.add_damage_unresolved);
    log_.notef("summary mission script forced recon bound=%d calls=%llu units=%llu "
        "unresolved=%llu (008AA8F0 -> 00805CF0, packet cc9_forced_recon_level)",
        kForcedReconLevelBound ? 1 : 0, summary_.forced_recon_calls,
        summary_.forced_recon_units, summary_.forced_recon_unresolved);
    log_.notef("summary mission script border zones bound=%d loaded=%d calls=%llu missing=%llu "
        "(008AECD0 / 004C7730, packet cc9_get_closest_border_zone)",
        kLuaClosestBorderZoneBound ? 1 : 0, border_zones_loaded_ ? 1 : 0,
        summary_.border_zone_calls, summary_.border_zone_missing);
    log_.notef("summary mission script set invincible calls=%llu units=%llu unresolved=%llu "
        "(00897A50 -> 0042ED80, packet cc9_set_invincible_native)", summary_.invincible_calls,
        summary_.invincible_units, summary_.invincible_unresolved);
    log_.notef("summary mission script recon publication bound=%d passes=%llu slots=%llu "
        "unchanged=%llu entries=%llu excluded=%llu no_category=%llu (00806B10/00805D90, packet "
        "cc9_recon_publication)", kReconPublishBound ? 1 : 0, summary_.recon_publish_passes,
        summary_.recon_publish_slots, summary_.recon_publish_unchanged,
        summary_.recon_publish_entries, summary_.recon_publish_excluded,
        summary_.recon_publish_no_category);
    log_.notef("summary mission script recon listeners bound=%d changes=%llu fires=%llu "
        "(00980E50, packet cc9_lua_recon_listeners)", kLuaReconListenersBound ? 1 : 0,
        summary_.listener_recon_changes, summary_.listener_recon_fires);
    log_.notef("summary mission script kill bound=%d calls=%llu units=%llu unresolved=%llu "
        "already_dead=%llu squadrons=%llu (008AC5C0, packet cc9_lua_kill)", kLuaKillBound ? 1 : 0,
        summary_.kill_calls, summary_.kill_units, summary_.kill_unresolved,
        summary_.kill_already_dead, summary_.kill_squadrons);
    log_.notef("summary mission getproperty class readers bound=%d unitcommand=%llu named=%llu "
        "nocommand=%llu unnamed=%llu reconlevel=%llu tables=%llu (00927AD0 / 00779BB0, packet "
        "cc9_get_property_class_readers)", kGetPropertyClassReadersBound ? 1 : 0,
        summary_.get_property_unitcommand_asked, summary_.get_property_unitcommand_named,
        summary_.get_property_unitcommand_nocommand, summary_.get_property_unitcommand_unnamed,
        summary_.get_property_reconlevel_asked, summary_.get_property_reconlevel_tables);
    log_.notef("summary mission getproperty ammotype bound=%d asked=%llu served=%llu zero=%llu "
        "(007EF1C0 -> 007EDAD0, packet cc9_get_property_ammotype)",
        kGetPropertySquadronAmmoTypeBound ? 1 : 0, summary_.get_property_ammotype_asked,
        summary_.get_property_ammotype_served, summary_.get_property_ammotype_zero);
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
    g_loadout_lua_state = state_;
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
    last_status_ = status;
    last_chunk_ = chunk_name != nullptr ? chunk_name : "";
    if (status == 0 && chunk_name != nullptr
        && std::strcmp(chunk_name, bsp::kMissionLuaPlatformChunk) == 0) {
        summary_.platform_chunk_ran = true;
    }
    return status;
}

int GameMissionLuaHost::lua_pcall(int nargs, int nresults, int errfunc_index) {
    if (state_ == nullptr) return 1;
    last_status_ = ::lua_pcall(state_, nargs, nresults, errfunc_index);
    return last_status_;
}

std::string GameMissionLuaHost::lua_tolstring_at_top() {
    if (state_ == nullptr) return {};
    const char* text = lua_tolstring(state_, -1, nullptr);
    const std::string message = text != nullptr ? text : std::string();
    // Packet cc9_stage_init_chunk_errors: the top of the stack after a
    // successful load and call is a result, not an error message.
    if (!message.empty() && last_status_ != 0) {
        note_error(message);
        if (phase_ == "global folders") ++summary_.global_folder_errors;
        if (phase_ == "mission chunk") summary_.mission_chunk_error = message;
        log_.notef("  chunk error in %s [%s] status=%d: %s",
            phase_.empty() ? "(none)" : phase_.c_str(),
            last_chunk_.empty() ? "?" : last_chunk_.c_str(), last_status_, message.c_str());
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
