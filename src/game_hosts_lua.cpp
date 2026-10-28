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
#include "bsp/session_context_cc13.hpp"  // packet cc13-mp2c
#include "bsp/lobby_option_defaults_cc13.hpp"  // packet cc13-mp3 (MP3c)
#include "bsp/lua_exec_guard.hpp"
#include "bsp/kamikaze_task_host.hpp"   // packet cc9_lua52_kamikaze_wiring
#include "bsp/game_hosts_mission_frame.hpp"
#include "bsp/game_hosts_mission_result.hpp"  // packet cc13-fix20

#include "bsp/air_operations.hpp"
#include "bsp/plane_task_reach_sw3.hpp"
#include "bsp/mothership_failure.hpp"
#include "bsp/get_failure_runway_hangar_cc13.hpp"
#include "bsp/forced_recon_mission_clear_cc13.hpp"
#include "bsp/game_hosts_ai.hpp"
#include "bsp/bullet_engagement_range.hpp"  // packet cc9_ai_plane_loadout_arm
#include "bsp/objective_units.hpp"
#include "bsp/plane_squadron_host.hpp"
#include "bsp/plane_cached_speed_b1c_cc13.hpp"   // packet cc13-torprun

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/gameplay_seg41_cc13.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/land_convoy_dead_cc13.hpp"  // packet cc13-dead
#include "bsp/ship_ai_attackmove_substates.hpp"
#include "bsp/game_hosts_ship_ai.hpp"  // packet cc9_unit_get_attack_target
#include "bsp/ship_damaged_gfx_remove_cc13.hpp"  // packet cc13-gunC C1b
#include "bsp/entity_orders.hpp"     // packet cc9_unit_get_attack_target
#include "bsp/game_hosts_gunnery.hpp"  // packet cc9_get_property_class_readers
#include "bsp/recon_sensor_pass.hpp"   // packet cc9_get_property_class_readers
#include "bsp/game_hosts_hud.hpp"
#include "bsp/hud_movie_camera.hpp"
#include "bsp/archive_text_writer.hpp"   // packet cc9_lua49_checkpoint
#include "bsp/profile_checkpoint.hpp"   // packet cc9_lua49_checkpoint
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/global_script_folders.hpp"
#include "bsp/lua_binding_mission_2.hpp"
#include "bsp/scene_unit_creators.hpp"
#include "bsp/lua_spawn_new.hpp"
#include "bsp/mission_lobby_settings.hpp"
#include "bsp/mission_lua_bindings.hpp"
#include "bsp/mission_lua_machine.hpp"
#include "bsp/mission_result.hpp"     // packet cc13-ver8, unit_death_00959450
#include "bsp/mission_ver_cc13.hpp"   // packet cc13-ver8
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
#include "bsp/cc13_seg87.hpp"
#include "bsp/native_lua_class_setters.hpp"
#include "bsp/native_lua_field_setters.hpp"
#include "bsp/native_lua_object_boolean.hpp"
#include "bsp/recon_level_forced_cc13.hpp"
#include "bsp/recon_values.hpp"
#include "bsp/recon_slot_lists.hpp"  // packet cc9_recon_publication
#include "bsp/shipyard_production.hpp"  // packet cc9_lua_add_shipyard_stock
#include "bsp/plane_retreat_task.hpp"   // packet cc13_natives1, 00489C40
#include "bsp/script_natives_fix5_cc13.hpp"   // packet cc13-fix5
#include "bsp/vfs_locale_runtime.hpp"
#include "bsp/vfs_provider_manager.hpp"
#include "bsp/unit_kind_query.hpp"   // packet p16_torpedo_stock_property
#include "bsp/cc13_lua_natives_wire.hpp"   // packet cc13-luawire
#include "bsp/unit_weapons.hpp"            // packet cc13-luawire, 008BE7E0's class tests
#include "bsp/plane_issue_threshold_cc13.hpp"   // packet cc13-issuea0, 008A1F40
#include "bsp/racked_round_entity_cc13.hpp"     // packet cc13-rackround

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
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <optional>
#include <type_traits>

namespace bsp::game {
namespace {
// Packet cc15-lua1: the VehicleClass row mark (0096490C, 00964959) and the lobby
// settings globals (00B67350, 00B67580) run the native 14h-object field setters
// over this state instead of stock push/setfield pairs. Each setter pushes the
// key with lua_pushlstring, then the value, then lua_settable on the object.
constexpr bool kLuaObjectFieldSettersCc13Bound = false;

// Packet cc13-yardskill: 00844FC0's launch bag `Skill` is the shipyard's own skill.
// 0084503C..00845046 call shipyard->vtable[12Ch]() (EDI = the shipyard), which is
// 006D1EC0 `MOV EAX,[ECX+390h]; RET`, and 00845049..00845055 store it under key
// 00CF8838 "Skill" (008F3710). OFF keeps the record default 1. A SetSkillLevel
// value outside 0..5 on the shipyard is stored raw by the image, while the host's
// existing 0..5 gate (game_hosts_units.cpp:20406, older than this switch) keeps 1.
// ON (cc9-recon1): reached only on JM08 (12 shipyard creates; jm08_l65_j8_p2 pair
// gameplay identical); no WINS row creates from a shipyard.
constexpr bool kShipyardLaunchSkillCc13Bound = true;

// Packet cc13-D3: SpawnNew 00949750 has no argument type test. A non-table argument
// goes to 00B67800 (lua_gettable 00A67C20), which raises for nil, number or boolean
// and reads nil fields from a string. OFF keeps the host's refusal (counted in
// spawn_new_rejected). ON also takes the request serial at the image's point,
// 00949F2B after the field reads, so neither that raise nor the live
// kSpawnNewMissingClassRaisesBound raise burns a serial; OFF takes it before the
// reads. Live: MissionLuaNative SpawnNew 0094C480 ->
// GameMissionLuaHost::run_spawn_new_00949750.
// ON (cc9-recon2, unreached): no log since 2026-10-15 has spawn_new_rejected > 0,
// and the one missing-class raise (PRCP05, calls=1) has no later SpawnNew whose
// serial could shift.
constexpr bool kSpawnNewNonTableArgumentCc13Bound = true;

// Packet cc13-atktgt: UnitGetAttackTarget 008A6DE0's arm by director kind. The
// image asks director->vtable[48h](2) (008A6F0D). A squadron's director is the
// block 00D0BD98 that 007F5009 stores at +348h, whose +48h is 0084D8F0, true only
// for 0 and 1, so a squadron always takes the command arm (0071BE40 -> category
// 1/2 -> 0071EB60 -> 00521EA0) and answers its PilotSetTarget target. OFF: the
// fire arm whenever the ship AI host has a row for the unit, which it keeps for
// every unit ("not_ship" included), so a squadron answers nil and
// luaAirfieldManager re-picks its target on every pass.
// ON since cc9-physics26 (docs/OCEAN_WAVE_FIELD.md section 114): reached through the
// squadron stand-in (cc13-atkfix); IJN20's level bombers stop reinstalling.
constexpr bool kUnitGetAttackTargetDirectorKindCc13Bound = true;

// The setters' NativeString key: 0041E870 over the CRT-backed storage, released
// when the call returns.
struct ScopedNativeKey {
    NativeString key;
    explicit ScopedNativeKey(const char* text) { key.assign_0041e870(crt_string_storage(), text); }
    ~ScopedNativeKey() { key.release_to(crt_string_storage()); }
    ScopedNativeKey(const ScopedNativeKey&) = delete;
    ScopedNativeKey& operator=(const ScopedNativeKey&) = delete;
};

// An untracked native object over the value at stack index `index` (kind 2).
NativeLuaObjectStorage stack_object(NativeLuaStateStorage& owner, int index) {
    NativeLuaObjectStorage object{};
    object.owner_00 = &owner;
    object.kind_04 = 2;
    object.index_08 = index;
    return object;
}

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
    luaopen_base, luaopen_table, bsp::luaopen_io_guarded, bsp::luaopen_os_guarded,
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

// Packet s54_class_id (docs/SHIP_AI_OPEN_ITEMS.md 270). 00964790
// BSP_VehicleClass_GetOrCreate, on a cache miss, reads VehicleClass[classIndex]
// and writes row.ID = classIndex (009648EC..0096490C, 00B67460 with EDI) and
// row.Got = true (00964938..00964959, 00B673A0); a non-zero LandingShip first
// resolves that class the same way (009648E7). This installation's
// vehicleclasses.lua authors no `ID`, so every script test `unit.Class.ID == n`
// reads nil without it. True: the first time a class row is seeded as a unit's
// `Class`, it gets both writes. LABELLED: the host's identity type map stands in
// for registry+10h, and the seed points stand in for the factory's call sites;
// the ship's second recursion (descriptor +0C0h != -1) is not modelled.
// False: the rows stay as loaded (before). ON by mechanism (SHIP_AI_OPEN_ITEMS 270):
// JM08 w1 fills Mission.Dakota and loses its early completion; USN01 sets the
// Class 162 / 313 squadrons' altitudes and still wins; JM06 identical.
constexpr bool kVehicleClassRowIdBound = true;

// Packet cc9_l63_entry_isfunction (docs/SQUADRON_LAND_TASK.md 5hv.7). 0045F440
// and 0045F520 gate the entry-point call on 00B66200 (BSP_LuaObject_IsFunction:
// lua_type == 6), not on "not nil". True: global_is_defined answers whether the
// global is a function. False: any non-nil value counts, as before.
constexpr bool kEntryPointIsFunctionBound = true;   // ON: SQUADRON_LAND_TASK 5hv.8

// Packet cc9_lua49_checkpoint (docs/CC13_PROFILE_SAVE.md; docs/SQUADRON_LAND_TASK.md
// 5fk). SaveCheckpoint 008ACB70: argument 0 to 00438A00 on the profile
// manager, then PushBoolean(1) (008ACB70's tail) whatever happened.
// 00438A00 runs 00438020 (src/profile_checkpoint.cpp), now or as the storage
// continuation 004389F0. LoadCheckpoint 008ACA30: 004379C0 on the calling
// context, which pushes BSP_Chk_Save[id]'s copy or nil, and the result count.
// SUBSTITUTIONS, labelled: the id is the mission script's file stem, not
// game+2198h; the unlock record is 00436690's defaults (0, -1, -1, "") with
// the difficulty from game+6ACh; the play time is 0.0; the archive is written
// only to a file beside the executable (the build or export tree), never the
// game installation or a profile; 008ACB70's gamepad-force and input-manager
// calls are not run. OFF: both rows stay unimplemented records.
constexpr bool kMissionCheckpointBound = false;

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
// Packet cc9_lua49_movie_modifier: a Lua value as 007A0EB0's LuaObject
// wrappers see it, for the modifier and event parses (bsp::cc13cam). Tables
// are copied two levels deep (the curves are arrays of numbers); an entity
// table is userdata to the image and is not read by either parse.
bsp::cc13cam::ModifierLuaValue movie_lua_value(lua_State* state, int index, int depth) {
    using Value = bsp::cc13cam::ModifierLuaValue;
    Value v;
    const int type = lua_type(state, index);
    if (type == LUA_TNUMBER) {
        v.kind = Value::Kind::Number;
        v.number = lua_tonumber(state, index);
        v.string = lua_tostring(state, index);   // 00B662B0's text of a number
    } else if (type == LUA_TSTRING) {
        // lua_isnumber is true for a numeric string, which 00B66050 accepts.
        v.kind = lua_isnumber(state, index) ? Value::Kind::Number : Value::Kind::String;
        v.number = lua_tonumber(state, index);
        v.string = lua_tostring(state, index);
    } else if (type == LUA_TTABLE) {
        v.kind = Value::Kind::Table;
        if (depth > 0) {
            const int abs_index = index < 0 ? lua_gettop(state) + index + 1 : index;
            const int n = static_cast<int>(lua_objlen(state, abs_index));
            for (int i = 1; i <= n; ++i) {
                lua_rawgeti(state, abs_index, i);
                v.array.push_back(movie_lua_value(state, -1, depth - 1));
                lua_pop(state, 1);
            }
            lua_pushnil(state);
            while (lua_next(state, abs_index) != 0) {
                if (lua_type(state, -2) == LUA_TSTRING) {
                    v.fields.emplace_back(lua_tostring(state, -2),
                                          movie_lua_value(state, -1, depth - 1));
                }
                lua_pop(state, 1);
            }
        }
    } else if (type != LUA_TNIL && type != LUA_TNONE) {
        v.kind = Value::Kind::Other;
    }
    return v;
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
            if (bsp::kMovieKeyframeModifierBound && std::string(key) == "modifier") {
                if (lua_type(state, -1) == LUA_TTABLE) in.modifier = movie_lua_value(state, -1, 3);
            } else if (!lua_isnil(state, -1)) {
                in.unsupported_keys.push_back(key);
            }
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
        if (bsp::kMovieKeyframeModifierBound && std::string(key) == "event") {
            if (lua_type(state, -1) == LUA_TTABLE) in.event = movie_lua_value(state, -1, 2);
        } else if (std::string(key) == "finishscript") {
            // Packet cc13-finscr: always kept (007A1F76 IsString), so the OFF log can
            // name it; OFF still counts the key as unsupported.
            if (lua_type(state, -1) == LUA_TSTRING) in.finishscript = std::string(lua_tostring(state, -1));
            if constexpr (!kMovieCameraFinishScriptCc13Bound) {
                if (!lua_isnil(state, -1)) in.unsupported_keys.push_back(key);
            }
        } else if (!lua_isnil(state, -1)) {
            in.unsupported_keys.push_back(key);
        }
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
    // Packet cc13-gunC C1b.
    const bool damaged_gfx_row = bsp::kShipDamagedGfxRemoveCc13Bound
        && dispatch_row.address == 0x0088f710u;
    const bool aa_enable_row = kLuaAAEnableBound && dispatch_row.address == 0x0089c740u;
    const bool ship_speed_row = kLuaSetShipSpeedBound && dispatch_row.address == 0x00890d30u;
    // Packet cc13_natives1.
    const bool set_failure_row = kLuaSetFailureBound && dispatch_row.address == 0x0088eff0u;
    const bool get_failure_row = kLuaSetFailureBound && dispatch_row.address == 0x0088f1d0u;
    // Packet cc13-fix11.
    const bool dead_meat_row = kLuaSetDeadMeatBound && dispatch_row.address == 0x008ac7b0u;
    // Packet cc13-fix12.
    const bool unit_selectable_row = kLuaIsUnitSelectableBound
        && dispatch_row.address == 0x008ab580u;
    const bool entity_speed_row = kLuaGetEntitySpeedBound
        && dispatch_row.address == 0x008c0820u;
    // Packet cc13-fix18.
    const bool clear_forced_recon_row = kLuaClearForcedReconLevelBound
        && dispatch_row.address == 0x008aab90u;
    const bool crew_level_row = kLuaSetCrewLevelBound && dispatch_row.address == 0x00894f20u;
    // Packet cc13-smbind: no switch, nothing in this process reads what they store.
    const bool support_manager_bann_row = dispatch_row.address == 0x0088c750u;
    const bool support_manager_permit_row = dispatch_row.address == 0x0088c8e0u;
    const bool support_manager_enable_row = dispatch_row.address == 0x0088cbb0u;
    // Packet cc13-fix10.
    const bool proximity_trigger_row
        = kLuaProximityTriggersBound && dispatch_row.address == 0x008c9730u;
    // Packet cc13-fix19: RemoveTrigger (0074FBB0) serves every Lua trigger kind.
    const bool remove_trigger_row
        = (kLuaProximityTriggersBound || kLuaSquadOrdnanceDroppedTriggerBound)
        && dispatch_row.address == 0x008c5fd0u;
    const bool ordnance_trigger_row
        = kLuaSquadOrdnanceDroppedTriggerBound && dispatch_row.address == 0x008c9a10u;
    // Packet cc9_unit_get_attack_target.
    const bool attack_target_row = kLuaUnitGetAttackTargetBound
        && dispatch_row.address == 0x008a6de0u;
    // Packet cc9_squadron_set_speed.
    const bool squadron_speed_row = kLuaSquadronSetSpeedBound
        && dispatch_row.address == 0x0089f780u;
    // Packet cc13_natives1.
    const bool force_gunner_row = kLuaSquadronForceGunnerModeBound
        && dispatch_row.address == 0x0089f950u;
    // Packet cc9_is_class_changed.
    const bool class_changed_row = kLuaIsClassChangedBound
        && dispatch_row.address == 0x008cc4b0u;
    // Packet cc9_set_submarine_depth_level.
    const bool sub_depth_row = kLuaSetSubmarineDepthLevelBound
        && dispatch_row.address == 0x00893f40u;
    // Packet cc13-luawire (bsp/cc13_lua_natives_wire.hpp).
    const bool has_fired_row = kLuaHasFiredBound && dispatch_row.address == 0x008be7e0u;
    const bool water_load_row = kLuaAddWaterLoadBound && dispatch_row.address == 0x00893380u;
    const bool break_ship_row = kLuaBreakShipBound && dispatch_row.address == 0x008914f0u;
    const bool player_commands_row = kLuaUnitSetPlayerCommandsEnabledBound
        && dispatch_row.address == 0x008a6c30u;
    const bool visibility_row = kLuaSetVisibilityBound && dispatch_row.address == 0x008a13d0u;
    const bool avoidance_row = kLuaSquadronEnableVehicleAvoidanceBound
        && dispatch_row.address == 0x008a0000u;
    const bool base_unsupported_row = kLuaSquadronSetBaseUnsupportedBound
        && dispatch_row.address == 0x0089faf0u;
    const bool cheat_turbo_row = kLuaSetCheatTurboBound && dispatch_row.address == 0x008a62a0u;
    // Packet cc9_lua_submarine_on_surface.
    const bool sub_surface_row = kLuaGetSubmarineOnSurfaceBound
        && dispatch_row.address == 0x008942c0u;
    // Packet cc13_natives1.
    const bool truly_dead_row = kLuaTrulyDeadBound && dispatch_row.address == 0x00899500u;
    // Packet cc9_set_air_base_slot_count.
    const bool slot_count_row = kLuaSetAirBaseSlotCountBound
        && dispatch_row.address == 0x008963e0u;
    // Packet cc9_lua_add_air_base_stock.
    const bool add_stock_row = kLuaAddAirBaseStockBound
        && dispatch_row.address == 0x00896a90u;
    // Packet cc9_lua_add_shipyard_stock.
    const bool shipyard_stock_row = kLuaAddShipyardStockBound
        && dispatch_row.address == 0x00896cc0u;
    // Packet cc9_device_reload_enabled.
    const bool device_reload_row = kLuaDeviceReloadEnabledBound
        && dispatch_row.address == 0x008c1350u;
    // Packet cc13-fix18.
    const bool reload_time_mul_row = kLuaDeviceReloadTimeMulBound
        && dispatch_row.address == 0x008c14b0u;
    // Packet cc9_squadron_travel_alt.
    const bool travel_alt_row = kSquadronTravelAltBound
        && dispatch_row.address == 0x0089f550u;
    // Packet cc13_natives1.
    const bool release_alt_row = kLuaSquadronSetReleaseAltBound
        && dispatch_row.address == 0x008a24e0u;
    // Packet cc13-fix4.
    const bool land_and_kill_row = kLuaSquadronLandAndKillBound
        && dispatch_row.address == 0x008a20e0u;
    // Packet cc13-issuea0: its only reader is the ON arm of the 007EEF40 gate.
    const bool drop_all_bomb_row = bsp::kPlaneIssueThresholdZeroCc13Bound
        && dispatch_row.address == 0x008a1f40u;
    // Packet cc13-fix5.
    const bool remove_all_stock_row = kLuaRemoveAllAirBasePlanesBound
        && dispatch_row.address == 0x008975b0u;
    const bool start_landing2_row = kLuaStartLanding2Bound
        && dispatch_row.address == 0x0089ac30u;
    const bool squadron_plane_row = kLuaGetSquadronPlaneBound
        && dispatch_row.address == 0x0089cfe0u;
    const bool disband_row = kLuaDisbandFormationBound
        && dispatch_row.address == 0x0089a020u;
    const bool land_convoy_row
        = (kLuaLandConvoyStartBound && dispatch_row.address == 0x00895570u)
        || (kLuaLandConvoyStopBound && dispatch_row.address == 0x008956e0u)
        || (kLuaLandConvoySetSpeedBound && dispatch_row.address == 0x00895850u);
    const bool launch_slot_row = kLuaLaunchAirBaseSlotBound
        && dispatch_row.address == 0x00896750u;
    // Packet cc9_squadron_attack_alt.
    const bool attack_alt_row = kSquadronAttackAltBound
        && dispatch_row.address == 0x008a22b0u;
    // Packet cc9_get_closest_border_zone.
    const bool border_zone_row = kLuaClosestBorderZoneBound
        && dispatch_row.address == 0x008aecd0u;
    // Packet cc13_natives1.
    const bool in_border_zone_row = kLuaIsInBorderZoneBound
        && dispatch_row.address == 0x008af020u;
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
    // Packet cc13-fix14b.
    const bool num_catapulted_row = (kLuaGetNumCatapultedBound || bsp::kShipCatapultLaunchBound)
        && dispatch_row.address == 0x00892aa0u;
    // Packet p17_catapult_adapter.
    const bool use_catapult_row = bsp::kShipCatapultLaunchBound
        && dispatch_row.address == 0x00891d50u;
    const bool set_catapult_stock_row = bsp::kShipCatapultLaunchBound
        && dispatch_row.address == 0x00892c30u;
    // Packet cc9_lua55_catapult_stock.
    const bool catapult_stock_row = kLuaGetCatapultStockBound
        && dispatch_row.address == 0x00892de0u;
    // Packet cc9_add_untouchable_unit.
    const bool untouchable_row = kLuaAddUntouchableUnitBound
        && dispatch_row.address == 0x008ac140u;
    // Packet p16_lua_natives.
    const bool untouchable_remove_row = kLuaUntouchableClearBound
        && dispatch_row.address == 0x008ac2b0u;
    const bool untouchable_is_row = kLuaUntouchableClearBound
        && dispatch_row.address == 0x008ac420u;
    // Packet cc9_submarine_air.
    const bool unlimited_air_row = kSubmarineAirBound
        && dispatch_row.address == 0x00893c00u;
    const bool ready_row = dispatch_row.address == 0x00895d20u;
    // Packet cc9_lua54_prcp01.
    const bool simplified_row = kSimplifiedReconNativesBound
        && (dispatch_row.address == 0x008b2320u || dispatch_row.address == 0x008b24a0u
            || dispatch_row.address == 0x008b25f0u || dispatch_row.address == 0x008b2770u);
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
    // Packet cc9_lua49_checkpoint.
    const bool checkpoint_row = kMissionCheckpointBound
        && (dispatch_row.address == 0x008acb70u || dispatch_row.address == 0x008aca30u);
    // Packet cc13-fix20. Both need the frame host's mission result object.
    GameMissionResultHost* mission_result = host->mission_result();
    const bool set_mission_completed_row = kLuaScoringSetMissionCompletedBound
        && mission_result != nullptr && dispatch_row.address == 0x008b8ad0u;
    const bool play_bink_movie_row = kLuaPlayBinkMovieBound
        && mission_result != nullptr && dispatch_row.address == 0x0089a480u;
    const bool handled = set_mission_completed_row || play_bink_movie_row
        || checkpoint_row || avoidance_setting || objective_row || get_property_row || kill_row
        || add_listener_row || remove_listener_row || listener_active_row || set_invincible_row
        || set_failure_row || get_failure_row || dead_meat_row
        || unit_selectable_row || entity_speed_row
        || clear_forced_recon_row || crew_level_row
        || support_manager_bann_row || support_manager_permit_row || support_manager_enable_row
        || proximity_trigger_row || remove_trigger_row || ordnance_trigger_row
        || forced_recon_row || add_damage_row || aa_enable_row || ship_speed_row
        || damaged_gfx_row
        || override_hp_row
        || attack_target_row || squadron_speed_row || class_changed_row || sub_depth_row
        || has_fired_row || water_load_row || break_ship_row || player_commands_row
        || visibility_row || avoidance_row || base_unsupported_row || cheat_turbo_row
        || sub_surface_row
        || force_gunner_row
        || slot_count_row || add_stock_row || shipyard_stock_row || device_reload_row
        || reload_time_mul_row
        || remove_all_stock_row || start_landing2_row || squadron_plane_row || disband_row
        || land_convoy_row || launch_slot_row
        || unlimited_air_row
        || in_formation_row || leave_formation_row || travel_alt_row || border_zone_row
        || untouchable_row || untouchable_remove_row || untouchable_is_row
        || truly_dead_row
        || attack_alt_row
        || ready_row
        || simplified_row || catapult_stock_row || num_catapulted_row
        // Packet cc13-fix21: census only. Both rows are answered by their early
        // returns below whatever `handled` says; listing them here stops
        // note_native_call recording them as UNIMPLEMENTED.
        || last_catapulted_row || formation_leader_row
        || launch_row || generate_row || spawn_new_row || scoring_play_time_row
        || spawn_id_requested_row || spawn_id_remove_row
        || in_border_zone_row
        || select_unit_row || movie_add_row || force_select_row
        || release_alt_row || land_and_kill_row || drop_all_bomb_row
        || (orders != nullptr && GameScriptOrdersHost::handles(dispatch_row.name));
    // The replay of a failed named call, which the executable makes only to
    // recover the error message, must not count a second time.
    if (!host->error_replay()) {
        host->note_native_call(static_cast<std::size_t>(row), argc, handled);
        if constexpr (bsp::game::kVehicleClassPreloadCc15Bound) {
            // cc15-vcp2: PrepareClass 008C8F70 (unrouted, see the orders
            // table) ends each argument with 00964790(arg, DL=1) at 008C92BE,
            // whose miss path marks the row's ID and Got. Only that mark is
            // modelled here; the race/party half (resolve_global_integer) stays
            // unbound. The registry map is the identity in this host.
            if (dispatch_row.address == 0x008c8f70u) {
                for (int index = 1; index <= argc; ++index) {
                    if (lua_type(state, index) != LUA_TNUMBER) continue;
                    host->mark_scene_vehicle_class_row_00964790(
                        static_cast<int>(lua_tointeger(state, index)), "008C92BE");
                }
            }
        }
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
        // DIAGNOSTIC: the objective's display text, argument 3 of 008CD440
        // (commandhelpers.lua luaObj_Add passes obj.Text there). Reads the
        // stack only: no Lua call, no draw.
        if (is_add) host->note_objective_text(name, objective_argument_string(state, 3));
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
    if (add_stock_row) {
        if (!host->error_replay()) host->run_add_air_base_stock_00896a90(state, argc);
        return 0;
    }
    if (remove_all_stock_row) {
        if (!host->error_replay()) host->run_remove_all_air_base_planes_008975b0(state, argc);
        return 0;
    }
    if (start_landing2_row && !host->error_replay()) {
        return host->run_start_landing2_0089ac30(state, argc);
    }
    if (squadron_plane_row && !host->error_replay()) {
        return host->run_get_squadron_plane_0089cfe0(state, argc);
    }
    if (disband_row) {
        if (!host->error_replay()) host->run_disband_formation_0089a020(state, argc);
        return 0;
    }
    if (land_convoy_row) {
        if (!host->error_replay()) host->run_land_convoy_native(state, argc, dispatch_row.address);
        return 0;
    }
    if (launch_slot_row && !host->error_replay()) {
        return host->run_launch_air_base_slot_00896750(state, argc);
    }
    if (shipyard_stock_row) {
        if (!host->error_replay()) host->run_add_shipyard_stock_00896cc0(state, argc);
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
    if (reload_time_mul_row) {
        if (!host->error_replay()) host->run_set_device_reload_time_mul_008c14b0(state, argc);
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
    if (release_alt_row) {
        if (!host->error_replay()) host->run_squadron_set_release_alt_008a24e0(state, argc);
        return 0;
    }
    if (land_and_kill_row) {
        if (!host->error_replay()) host->run_squadron_land_and_kill_008a20e0(state, argc);
        return 0;
    }
    if (drop_all_bomb_row) {
        if (!host->error_replay()) {
            host->run_squadron_set_drop_all_bomb_percent_008a1f40(state, argc);
        }
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
    if constexpr (kLuaGetNumCatapultedBound || bsp::kShipCatapultLaunchBound) {
        if (num_catapulted_row && !host->error_replay()) {
            return host->run_get_num_catapulted_00892aa0(state, argc);
        }
    }
    if (use_catapult_row) {
        if (!host->error_replay()) host->run_ship_use_catapult_00891d50(state, argc);
        return 0;
    }
    if (set_catapult_stock_row) {
        if (!host->error_replay()) host->run_set_catapult_stock_00892c30(state, argc);
        return 0;
    }
    if (catapult_stock_row && !host->error_replay()) {
        return host->run_get_catapult_stock_00892de0(state, argc);
    }
    if (untouchable_row) {
        if (!host->error_replay()) host->run_add_untouchable_unit_008ac140(state, argc);
        return 0;
    }
    if (untouchable_remove_row) {
        if (!host->error_replay()) host->run_remove_untouchable_unit_008ac2b0(state, argc);
        return 0;
    }
    if (untouchable_is_row && !host->error_replay()) {
        return host->run_is_unit_untouchable_008ac420(state, argc);
    }
    if (border_zone_row && !host->error_replay()) {
        return host->run_get_closest_border_zone_008aecd0(state, argc);
    }
    if (in_border_zone_row && !host->error_replay()) {
        return host->run_is_in_border_zone_008af020(state, argc);
    }
    if (leave_formation_row) {
        if (!host->error_replay()) host->run_leave_formation_00899eb0(state, argc);
        return 0;
    }
    if (sub_depth_row) {
        if (!host->error_replay()) host->run_set_submarine_depth_level_00893f40(state, argc);
        return 0;
    }
    if (has_fired_row && !host->error_replay()) {
        return host->run_has_fired_008be7e0(state, argc);
    }
    if (water_load_row) {
        if (!host->error_replay()) host->run_add_water_load_00893380(state, argc);
        return 0;
    }
    if (break_ship_row) {
        if (!host->error_replay()) host->run_break_ship_008914f0(state, argc);
        return 0;
    }
    if (player_commands_row) {
        if (!host->error_replay()) host->run_unit_set_player_commands_enabled_008a6c30(state, argc);
        return 0;
    }
    if (visibility_row) {
        if (!host->error_replay()) host->run_set_visibility_008a13d0(state, argc);
        return 0;
    }
    if (avoidance_row) {
        if (!host->error_replay()) host->run_squadron_enable_vehicle_avoidance_008a0000(state, argc);
        return 0;
    }
    if (base_unsupported_row) {
        if (!host->error_replay()) host->run_squadron_set_base_unsupported_0089faf0(state, argc);
        return 0;
    }
    if (cheat_turbo_row) {
        if (!host->error_replay()) host->run_set_cheat_turbo_008a62a0(state, argc);
        return 0;
    }
    if (class_changed_row && !host->error_replay()) {
        return host->run_is_class_changed_008cc4b0(state, argc);
    }
    if (sub_surface_row && !host->error_replay()) {
        return host->run_get_submarine_on_surface_008942c0(state, argc);
    }
    if (truly_dead_row && !host->error_replay()) {
        return host->run_truly_dead_00899500(state, argc);
    }
    if (squadron_speed_row) {
        if (!host->error_replay()) host->run_squadron_set_speed_0089f780(state, argc);
        return 0;
    }
    if (force_gunner_row) {
        if (!host->error_replay()) host->run_squadron_force_gunner_mode_0089f950(state, argc);
        return 0;
    }
    if (attack_target_row && !host->error_replay()) {
        return host->run_unit_get_attack_target_008a6de0(state, argc);
    }
    if (ship_speed_row) {
        if (!host->error_replay()) host->run_set_ship_speed_00890d30(state, argc);
        return 0;
    }
    if (set_mission_completed_row) {
        // 008B8AD0: the count 00B663F0, arg 0 through 00B66290 (lua_tonumber
        // then __ftol, so truncated) and the flag through 00B66250
        // (lua_toboolean, no type gate). The mode decides which are used.
        if (!host->error_replay()) {
            mission_result->lua_scoring_set_mission_completed_008b8ad0(argc,
                static_cast<int>(::lua_tonumber(state, 1)), lua_toboolean(state, 1) != 0,
                lua_toboolean(state, 2) != 0);
        }
        return 0;  // 00B66400: nothing pushed
    }
    if (play_bink_movie_row) {
        // 0089A480: arg 0 through 00B662B0; arg 2 through 00B66250 only when
        // the count is above 2 (BL starts at 0, 0089A5A2). Labelled: a non-string
        // arg 0 is passed as an empty name.
        if (!host->error_replay()) {
            const char* name = lua_tostring(state, 1);
            mission_result->lua_play_bink_movie_0089a480(name != nullptr ? name : "",
                argc > 2 && lua_toboolean(state, 3) != 0);
        }
        return 0;  // 00B66400: nothing pushed
    }
    if (set_failure_row) {
        if (!host->error_replay()) host->run_set_failure_0088eff0(state, argc);
        return 0;
    }
    if (get_failure_row && !host->error_replay()) {
        return host->run_get_failure_0088f1d0(state, argc);
    }
    if (dead_meat_row) {
        if (!host->error_replay()) host->run_set_dead_meat_008ac7b0(state, argc);
        return 0;
    }
    if (unit_selectable_row && !host->error_replay()) {
        return host->run_is_unit_selectable_008ab580(state, argc);
    }
    if (entity_speed_row && !host->error_replay()) {
        return host->run_get_entity_speed_008c0820(state, argc);
    }
    if (clear_forced_recon_row) {
        if (!host->error_replay()) host->run_clear_forced_recon_level_008aab90(state, argc);
        return 0;
    }
    if (crew_level_row) {
        if (!host->error_replay()) host->run_set_crew_level_00894f20(state, argc);
        return 0;
    }
    if (support_manager_bann_row || support_manager_permit_row) {
        if (host->error_replay()) return 0;
        return host->run_support_manager_set_banned_0088c750_0088c8e0(support_manager_bann_row);
    }
    if (support_manager_enable_row) {
        if (host->error_replay()) return 0;
        return host->run_support_manager_enable_0088cbb0(state, argc);
    }
    if (proximity_trigger_row) {
        if (!host->error_replay()) host->run_add_proximity_trigger_008c9730(state, argc);
        return 0;
    }
    if (remove_trigger_row) {
        if (host->error_replay()) return 0;
        return host->run_remove_trigger_008c5fd0(state, argc);
    }
    if (ordnance_trigger_row) {
        if (!host->error_replay()) host->run_add_squad_ordnance_dropped_trigger_008c9a10(state, argc);
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
    if (damaged_gfx_row) {
        if (!host->error_replay()) host->run_set_damaged_gfx_level_0088f710(state, argc);
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
    if (simplified_row && !host->error_replay()) {
        return host->run_simplified_multiplier(state, argc, dispatch_row.address);
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
    if (checkpoint_row) {
        if (host->error_replay()) return 0;
        return host->mission_checkpoint_binding(state, dispatch_row.address == 0x008acb70u);
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

// The run log the os.execute / io.popen guard writes its refusal into.
static GameHostLog* g_exec_refusal_log = nullptr;
// Packet cc9_player_air_ops_launch: the host whose class table answers class+134h.
static GameMissionLuaHost* g_class_arm_host = nullptr;

GameMissionLuaHost::GameMissionLuaHost(GameHostLog& log, GameVfsHost& vfs)
    : log_(log), vfs_(vfs) {
    // The guard's refusal line into this run's log (stderr is discarded).
    g_exec_refusal_log = &log_;
    bsp::g_lua_exec_refused_sink = [](const char* command) {
        if (g_exec_refusal_log != nullptr)
            g_exec_refusal_log->notef("bsp: refused a mission script's process launch: %s", command);
    };
    g_class_arm_host = this;
    // Packet cc9_shipyard_create_unit: gunnery30's build arm creates through this host.
    bsp::shipyard_set_create_unit([](const bsp::ShipyardBuildRequest& request) {
        return g_class_arm_host != nullptr ? g_class_arm_host->create_shipyard_unit_00844fc0(request)
                                           : bsp::kShipyardNone;
    });
    script_orders_set_class_default_arm_reader([](int vehicle_class) {
        // class+134h `DefaultEquipment` (00961F0A), 0 when nil (00961F30).
        return g_class_arm_host != nullptr
            ? g_class_arm_host->read_vehicle_class_integer(vehicle_class, "DefaultEquipment",
                                                           nullptr, 0)
            : 0;
    });
    // 006C5950's class restore (006C5AD9) reads the same class+134h; its reader
    // is consulted only under kAirOpsReturnSlotClassArmCc13Bound (and the sw3
    // reach count).
    bsp::set_air_ops_class_default_arm_reader(
        [](std::uint32_t vehicle_class, void*) -> std::int32_t {
            return g_class_arm_host != nullptr
                ? g_class_arm_host->read_vehicle_class_integer(static_cast<int>(vehicle_class),
                                                               "DefaultEquipment", nullptr, 0)
                : 0;
        },
        nullptr);
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
    if (g_exec_refusal_log == &log_) {
        g_exec_refusal_log = nullptr;
        bsp::g_lua_exec_refused_sink = nullptr;
    }
    if (g_class_arm_host == this) {
        g_class_arm_host = nullptr;
        script_orders_set_class_default_arm_reader(nullptr);
        bsp::set_air_ops_class_default_arm_reader(nullptr, nullptr);
        bsp::shipyard_set_create_unit(nullptr);
    }
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
    note_vehicle_class_read(index);   // cc15-vcp1 census
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
            row.death_efx_id = static_cast<int>(number("DeathEfx"));
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
    // Harness diagnostic, not image behaviour: the summary keeps only the first
    // error, so a later error raised inside a listener or delayed callback was
    // silent. Log each one, capped so a per-frame failure cannot flood the log.
    if (!message.empty()) ++lua_errors_total_;   // cc15-vcp3 summary
    static int logged = 0;
    if (!message.empty() && logged < 50) {
        ++logged;
        log_.notef("lua error (%d of at most 50, phase %s): %s", logged,
            phase_.c_str(), message.c_str());
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
    // Packet cc9_lua52_kamikaze_wiring: the released rounds live with the world.
    bsp::dummy_kamikaze_rounds().clear();
    // Packet p17_catapult_adapter: the ships' launch records live with the world.
    bsp::ship_catapult_registry().clear();
    if (orders == nullptr) {
        // Packet cc13-reconclear: the image's forced recon records live in the units.
        if constexpr (bsp::kForcedReconMissionClearCc13Bound) {
            bsp::clear_forced_recon_levels();
        }
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
    // Packet cc9_lua46_set_party_unit: SetParty on a unit re-runs the mirror.
    orders->set_party_mirror([](void* context) -> std::size_t {
        return static_cast<GameMissionLuaHost*>(context)->mirror_party_race_00928f50();
    }, this);
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

std::string GameMissionLuaHost::read_stationary_class_string(const std::string& name,
    const char* key) {
    std::string value;
    if (state_ == nullptr || name.empty() || key == nullptr) return value;
    const int top = ::lua_gettop(state_);
    // The same 00851CB0 StationaryClass/type-name lookup as the existing
    // presence reader above, followed by the installed field itself.
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "StationaryClass");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_getfield(state_, -1, name.c_str());
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

// Packet cc9_lua54_prcp01. The setters read argument 1 through GetNumber
// (008B2320 -> 008B243C) and push nothing; the getters push the stored float.
int GameMissionLuaHost::run_simplified_multiplier(lua_State* state, int argument_count,
                                                  std::uint32_t address) {
    if (state == nullptr) return 0;
    const bool sonar = address == 0x008b25f0u || address == 0x008b2770u;
    float& value = sonar ? simplified_sonar_78_ : simplified_recon_74_;
    if (address == 0x008b2320u || address == 0x008b25f0u) {
        value = argument_count >= 1 ? static_cast<float>(::lua_tonumber(state, 1)) : 0.0f;
        log_.notef("  %s %08x: +%s = %.3f", sonar ? "SetSimplifiedSonarMultiplier"
            : "SetSimplifiedReconMultiplier", static_cast<unsigned>(address),
            sonar ? "78h" : "74h", static_cast<double>(value));
        log_.implemented(sonar ? "MissionLuaNative::SetSimplifiedSonarMultiplier"
            : "MissionLuaNative::SetSimplifiedReconMultiplier",
            sonar ? "008b25f0" : "008b2320");
        return 0;
    }
    ::lua_pushnumber(state, static_cast<lua_Number>(value));
    log_.implemented(sonar ? "MissionLuaNative::GetSimplifiedSonarMultiplier"
        : "MissionLuaNative::GetSimplifiedReconMultiplier",
        sonar ? "008b2770" : "008b24a0");
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
    // class+134h is `DefaultEquipment`: 00961F0A pushes the key (00D1AB10) and
    // 00961F45 stores its integer at +134h, 0 when nil (00961F30). The old
    // reading here ("not authored under any key") kept 0, which the release
    // path's own DefaultEquipment read masked. Packet cc9_plane_scene_equipment
    // (SQUADRON_LAND_TASK 5dk.1) reads it with kPlaneSceneEquipmentBound, where
    // the launched squadron's bag `Equipment` decides its loadout.
    request.class_default_arm = bsp::kPlaneSceneEquipmentBound
        ? read_vehicle_class_integer(static_cast<int>(request.vehicle_class),
                                     "DefaultEquipment", nullptr, 0)
        : 0;
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

// Packet s52_bsm09_cloud (docs/SHIP_AI_OPEN_ITEMS.md 256/257). A held-back row
// whose class has no unit creator in this process and no `thisTable` identity
// row (find_scene_class_lua_identity) gets the answer the load path gives the
// same class (game_hosts_scene_contents.cpp: "creator ... is not
// reconstructed"): nothing is made. The case met is BSM09's luaIntroMovie7
// (bsm_09:1102-1173), GenerateObject on 72 `Cloud` (3Dh) rows. 0046DB4B calls
// descriptor[1] = 004E9E40 for them, which reads CloudType and builds a cloud
// through the manager [[00E188A8]+21D0h] vtable[8] (docs/ENTITY_IDENTITY.md
// part 3): presentation and no unit; src/entity_identity.cpp models the body,
// but this host has no cloud manager to create into, so it is not called. No script reads
// the result. True: the row is marked spawned and the call returns nothing.
// False: the row goes to create_unit_from_scene_record_0046db4b, which makes a
// unit with no creator and throws at its observer tables (the run dies).
// ON by section 257: USN01 control gameplay identical (s52_c0_u1 / s52_c1_u1);
// BSM09 runs past luaIntroMovie7 to its EndMission at 917.90 s (s52_c1_b9).
constexpr bool kGenerateObjectRecordOnlyCreatorBound = true;
// Packet cc13-hiddencarry: spawns that took the pool entry's carry, and of them
// the ones whose authored command the commands host issued.
unsigned long long g_hidden_spawn_carry_applied = 0;
unsigned long long g_hidden_spawn_carry_commands = 0;
// Host-only: carried spawns that took no `thisTable` slot, whose command was dropped.
unsigned long long g_hidden_spawn_carry_unattached = 0;
unsigned long long g_generate_object_record_only = 0;

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
    // kGenerateObjectRespawnsDeadBound narrows the deviation to a live earlier
    // instance: once the script sees it Dead, the call instantiates again.
    bool respawn = false;
    if constexpr (kGenerateObjectEveryCallBound) {
        // Packet p16_lua_natives: 0046D930 has no already-created arm.
        if (entry->spawned) {
            respawn = true;
            ++summary_.generate_object_every_call;
        }
    }
    if constexpr (kGenerateObjectRespawnsDeadBound) {
        if (!respawn && entry->spawned && push_resolved_entity_by_id(state, entry->entity_id)) {
            ::lua_getfield(state, -1, "Dead");
            respawn = ::lua_toboolean(state, -1) != 0;
            ::lua_settop(state, ::lua_gettop(state) - 2);
        }
        if (respawn) {
            ++summary_.generate_object_respawned;
            log_.notef("  GenerateObject 00944fd0: \"%s\": the earlier instance (id %d) is "
                "Dead, so a new one is made, as 0046D930 does on every call (packet "
                "cc9_lua55_genobj)", name.c_str(), entry->entity_id);
        }
    }
    if (entry->spawned && !respawn) {
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
    if (record.class_id == 0x1a) {
        // Packet cc9_lua46_generate_land_convoy. A held-back LandConvoy (JM05's
        // Event5Convoy and Event3Convoy 01..03): 0046DB4B's descriptor[1] is the
        // LandConvoy creator (004F2700, a 3CCh FixedInstance, no vehicle class),
        // and its slot-39 attach 00743450 (vtable 00CEA570 +9Ch) builds the
        // members from Type1..TypeN. Before, the record went to create_units as a
        // unit and threw (no creator for the observer tables), killing the run.
        return generate_land_convoy_00743450(state, *entry, record, placed);
    }
    if (kGenerateObjectRecordOnlyCreatorBound
        && bsp::find_scene_unit_creator(record.class_id) == nullptr
        && bsp::find_scene_class_lua_identity(record.class_id) == nullptr) {
        entry->spawned = true;
        ++g_generate_object_record_only;
        if (g_generate_object_record_only <= 4) {
            log_.notef("  GenerateObject 00944fd0: \"%s\" (%s, class %02Xh) has no unit "
                "creator and no thisTable row; recorded as at load, nothing made "
                "(004E9E40 for Cloud, presentation; packet s52_bsm09_cloud, %llu so far)",
                name.c_str(), record.class_name.c_str(), record.class_id,
                g_generate_object_record_only);
        }
        log_.implemented("MissionLuaNative::GenerateObject", "00944fd0");
        return 0;
    }
    // Packet cc13-hiddencarry. 0046DC10's creator call (0046DE9A) and its InitAll
    // (0046DEDD) read the held-back record's own bag; the hold-back kept what
    // they read on the pool entry. The command lands in the record as the
    // creator's 004E6B30 queue does, and the StartSpeed is what create_units'
    // 0082356C arm seeds the ring from, before the command is resolved below.
    const bool carried = kSceneHiddenSpawnCarryCc13Bound && entry->carry_present;
    if (carried) {
        if (entry->carry_command_queued) {
            record.command = entry->carry_command;
            record.command_target = entry->carry_command_target;
        }
        if (entry->carry_start_speed_present) {
            record.start_speed_present = true;
            record.start_speed_type = entry->carry_start_speed_type;
            record.start_speed_float = entry->carry_start_speed_float;
            record.start_speed_int = entry->carry_start_speed_int;
        }
    }
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
        // Packet cc13-hiddencarry, HOST-ONLY: the image has no such exit (its
        // 009453BB resolve runs regardless), so a carried spawn that ends here
        // keeps its StartSpeed seed but its command is never issued. Counted.
        if (carried) ++g_hidden_spawn_carry_unattached;
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
    // Packet cc13-hiddencarry: 0046AAB0 at 009453BB, after the spawn's InitAll
    // (0046DEDD) and 00874D00 (00945311), resolves the record the creator
    // queued. The target lookup is 00925A90 over the entities that exist at that
    // moment, which is what the commands host's registry holds after this
    // spawn's create_units, so a CommandTarget naming a unit not yet spawned
    // resolves to nothing here as it does there.
    if (carried) {
        ++g_hidden_spawn_carry_applied;
        if (entry->carry_command_queued && units_hooks_ != nullptr
            && units_hooks_->issue_generated_authored_command(units_before, name)) {
            ++g_hidden_spawn_carry_commands;
        }
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

// Packet cc9_lua46_generate_land_convoy: GenerateObject of a held-back
// LandConvoy, the same three steps a load-time convoy takes in this host
// (src/game_hosts_mission_frame.cpp: the marker seed collect_scene_markers makes,
// 00925F20's pass A over it, then 00743450 and 007420B0/007AF150), run at the
// call instead of the load walk. SUBSTITUTIONS, labelled:
//  - the convoy entity is a scene marker of this host, with an id from
//    kGeneratedSceneMarkerIdBase (the load-time markers count up from 50000);
//  - the members are units created by build_land_convoy_roster_00743450 and,
//    as for a load-time convoy, are not pushed onto the pending list;
//  - a convoy whose Path does not resolve keeps its members standing, as at load.
int GameMissionLuaHost::generate_land_convoy_00743450(lua_State* state,
    bsp::game::SceneSpawnPoolEntry& entry, const bsp::game::GameSceneEntityRecord& held,
    bool placed) {
    bsp::game::GameSceneEntityRecord record = held;
    record.generated = true;   // the instantiate pass took it (0046DB4B)
    record.created = false;    // a marker: no unit is made of the convoy itself
    record.skipped_because.clear();
    const int id = kGeneratedSceneMarkerIdBase + generated_scene_markers_++;
    const float position[3] = {record.world[12], record.world[13], record.world[14]};
    // 00928760's push, then the marker fields run_scene_load_init_all_0046eb4b sets.
    push_pending_entity_00926be0(id, record.name, -1);
    if (PendingEntity* const node = find_pending(id)) {
        node->findable = true;          // the LandConvoy row of find_scene_class_lua_identity
        node->marker_class_id = record.class_id;
        node->marker_authored_party = record.party;
        node->party = record.party;
        node->race = record.race;
    }
    script_orders_->register_scene_marker(id, record.name, position, record.class_id);
    if (units_hooks_ != nullptr) units_hooks_->register_scene_marker_frame(id, record.world);
    run_sentity_init_all_00925f20(false, 0x0046dbe8u);  // 0046DBE6 XOR CL,CL
    const bool attached = init_all_attached(id);
    std::size_t members = 0;
    bool moving = false;
    if constexpr (bsp::game::kLandConvoyMembersBound) {
        if (units_hooks_ != nullptr) members = units_hooks_->build_land_convoy_roster_00743450(record);
        if constexpr (bsp::game::kLandConvoyMovementBound) {
            if (const std::vector<bsp::game::GameSceneEntityRecord>* scene =
                    bsp::game::live_scene_entities(); scene != nullptr && units_hooks_ != nullptr) {
                moving = units_hooks_->bind_land_convoy_motion(record, *scene);
            }
        }
    }
    entry.spawned = true;
    entry.entity_id = id;
    ++summary_.generate_object_created;
    log_.notef("  GenerateObject 00944fd0: \"%s\" (LandConvoy) -> marker id %d at "
        "(%.1f %.1f %.1f)%s, thisTable=%d, members=%zu, moving=%d (0046DB4B 004F2700, "
        "00743450, packet cc9_lua46_generate_land_convoy)", record.name.c_str(), id,
        position[0], position[1], position[2], placed ? " placed" : " authored frame",
        attached ? 1 : 0, members, moving ? 1 : 0);
    log_.implemented("MissionLuaNative::GenerateObject", "00944fd0");
    if (attached && push_resolved_entity_by_id(state, id)) return 1;
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
    // kSpawnNewNonTableArgumentCc13Bound (cc13-D3, OFF): the image does not test the
    // type. Its field reads are 00B67800 BSP_LuaObject_GetByName, whose lua_gettable
    // (00A67C20) raises "attempt to index a <type> value" for a nil, number or boolean
    // argument and reads nil fields through the string metatable for a string, which
    // then queues the absent-field record. ON drops the refusal so the host's own
    // lua_getfield reads do the same; OFF keeps the refusal.
    if (argument_count < 1 && !kSpawnNewNonTableArgumentCc13Bound) {
        ++summary_.spawn_new_rejected;
        return 0;
    }
    if (::lua_type(state, 1) != LUA_TTABLE && !kSpawnNewNonTableArgumentCc13Bound) {
        ++summary_.spawn_new_rejected;
        return 0;
    }
    ++summary_.spawn_new_calls;

    bsp::SpawnNewRequest request;
    // ON takes the serial where the image does (00949F2B, after the last 00B67800
    // read at 00949E59), so a raise in the reads - the non-table argument at the
    // first read (00949834) or the missing VehicleClass row (00944210 via 00945C30
    // at 00949A53) - burns no serial. OFF takes it here, as before.
    if constexpr (!kSpawnNewNonTableArgumentCc13Bound) {
        request.serial = bsp::next_spawn_request_serial_00949f2b();
    }
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
            if constexpr (kSpawnNewMissingClassRaisesBound) {
                // Packet p18_spawn_missing_class: 00944210 -> 00964790 indexes the nil
                // VehicleClass row (00B67800 -> 00A67C20 lua_gettable) and Lua raises.
                if (!read_vehicle_class_row(row.type_class_id).found) {
                    ++summary_.spawn_missing_class_raises;
                    log_.notef("  SpawnNew 00949750: member %d Type %d has no VehicleClass row; "
                        "00964790's lua_gettable on the nil row raises (packet "
                        "p18_spawn_missing_class)", i, row.type_class_id);
                    return luaL_error(state, "attempt to index a nil value");
                }
            }
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

    if constexpr (kSpawnNewNonTableArgumentCc13Bound) {
        // 00949F2B: argument 1 has been indexed by now, so the reads below
        // cannot raise and their order against the serial is unobservable.
        request.serial = bsp::next_spawn_request_serial_00949f2b();
    }

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
    // Packet cc13-ver5, kKillListenerDeadFirstBound: `hit` first, as the image fires it
    // inside the hit (0077CE60 -> 00988510), ahead of the row-15 flush.
    if constexpr (kKillListenerDeadFirstBound) dispatch_hit_listeners_00988510();
    // Packet cc13-ver5: `entityKilled` before `kill`, as 0077D1A0 fires it inside the
    // destroy, before the flush that reaches 009813A0.
    // Packet cc13-fix3: `exitzone` first of all, as 007F31E6 runs before 007F31A0's Kill(4).
    if constexpr (kLuaExitZoneListenersBound) {
        if (kLuaListenersBound) dispatch_exit_zone_listeners_00982120();
    }
    if constexpr (kLuaEntityKilledListenersBound) {
        if (kLuaListenersBound) dispatch_entity_killed_listeners_00986480();
    }
    if (kLuaListenersBound) dispatch_kill_listeners_009813a0();
    if constexpr (!kKillListenerDeadFirstBound) dispatch_hit_listeners_00988510();
    if (kLuaListenersBound && kLuaReconListenersBound) dispatch_recon_listeners_00980e50();
    if (kReconPublishBound) publish_recon_slot_tables_00806b10();
    if constexpr (bsp::kDropKamikazeTaskBound) run_dummy_kamikaze_rounds_006fec00(step_seconds);
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
// 0071C4F0 tests this host's own Map bounds under kSpawnPlacementMapBoundsBound
// (packet p16_lua_natives); without it, or with none loaded, nothing is outside.
class SpawnPlacementUnits final : public bsp::SpawnPlacementWorld {
public:
    SpawnPlacementUnits(const GameUnitsHost& units, const bsp::WorldMapBounds* bounds)
        : units_(units), bounds_(bounds) {}
    bool point_outside_map_0071c4f0(const float* p) const override {
        if (!kSpawnPlacementMapBoundsBound || bounds_ == nullptr || p == nullptr) return false;
        return bsp::point_outside_world_map_0071c4f0(*bounds_, {p[0], p[1], p[2]});
    }
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
    const bsp::WorldMapBounds* bounds_;
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
        placement_world.emplace(script_orders_->units(), world_border_bounds());
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
                // Packet cc13-spawnsq: 009486BF stores the same 009420A0 bag at
                // +0C0h on the plane arm too, so pass A gives the squadron its
                // `Party` (00928A1E) and 007F491A hands it to the wing. Counted
                // whether or not the switch is on.
                if (PendingEntity* node = find_pending(static_cast<int>(entity))) {
                    ++summary_.spawn_squadron_party_nodes;
                    node->spawn_squadron_record = true;   // log only: counts its wing
                    if constexpr (kSpawnNewSquadronPartyCc13Bound) {
                        node->party = request.party;
                        node->race = member.race > 0 ? member.race
                                                     : (request.party == 0 ? 2 : 1);
                        node->generated_party = true;
                    }
                }
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

// Packet cc9_lua52_kamikaze_wiring. 006FEC00 for every released
// DummyKamikazePlane round. SUBSTITUTION, labelled: the image ticks the round in
// the bomb pass of the fixed step; here it is stepped once per mission frame,
// ahead of the spawn queue, and the unit it makes takes the same route as an
// air-ops squadron (create, AutoAttackTarget, the SEntity init push).
void GameMissionLuaHost::run_dummy_kamikaze_rounds_006fec00(float step_seconds) {
    if (script_orders_ == nullptr) return;
    const std::vector<bsp::DummyKamikazeSpawn> spawns =
        bsp::dummy_kamikaze_rounds().step(step_seconds);
    for (const bsp::DummyKamikazeSpawn& s : spawns) {
        const std::size_t units_before = script_orders_->units().count();
        std::string name;
        const std::uint32_t entity =
            script_orders_->create_dummy_kamikaze_squadron_006fec00(s, name);
        if (entity == 0u) continue;
        // 006FEE41-006FEE5F: AutoAttackTarget only with a target at +98h;
        // 007F4BA0 turns it into the attack order (007F15F0 -> 007EEC50).
        if (s.target_plus_one > 0) {
            script_orders_->queue_auto_attack_target_007f15f0(
                entity, static_cast<std::uint32_t>(s.target_plus_one));
        }
        if constexpr (kSEntityInitAllBound) {
            route_push_squadron(static_cast<int>(entity), name, s.plane_class, units_before);
        } else if (attach_created_entity_00928a00(static_cast<int>(entity), name,
                                                  s.plane_class)) {
            attach_wing_member_tables(units_before, entity, s.plane_class);
        }
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
    // cc13-tier1: the bag's `State` (7 on 006C5050's flag arm) and Dynamics/Ceiling
    // (tuning+210h, read at 006C52BF) go to the creator, which reads them only
    // under kAirOpsLaunchFlagAirborneBound. plane_globals_ is filled once by the
    // settings load (007E2A20) before any mission script and never cleared; with
    // no table it keeps its zero, the +210h default in game_tuning_singleton.cpp.
    const std::uint32_t entity = script_orders_->create_air_ops_squadron_006c5050(
        request.type, request.wing_count, request.equipment, request.home_base, name,
        wing, request.state, plane_globals_.dynamics_ceiling);
    if (entity == 0u) return 0u;
    if constexpr (bsp::kAirOpsPlayerLaunchBound) {
        // Packet cc9_player_air_ops_launch: the bag's `AutoAttackTarget`
        // (006C5050), which the squadron's 007F4BA0 turns into an attack order.
        if (request.has_auto_attack_target && request.auto_attack_target > 0) {
            script_orders_->queue_auto_attack_target_007f15f0(entity,
                static_cast<std::uint32_t>(request.auto_attack_target));
        }
    }
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
        // kAirOpsOwnerByInstanceBound: the live instance's skill (SetSkillLevel on
        // a regenerated carrier lands there, not on the dead row of the same name).
        const std::size_t owner = bsp::air_ops_owner_row(units.count(), request.home_base,
            [&](std::size_t i) -> const std::string* {
                const GameUnitRow* row = units.unit_row(i);
                return row != nullptr ? &row->name : nullptr;
            },
            [&](std::size_t i) { return units.gunnery() != nullptr && units.gunnery()->unit_dead(i); });
        if (owner < units.count()) owner_skill = units.skill_level(owner);
        if (PendingEntity* node = find_pending(static_cast<int>(entity))) {
            node->launch_skill = owner_skill;
            // Packet cc13-lsqparty: 006C5050's bag `Party` (the owner's +54h),
            // which 00927050's kind-1 arm reads at pass A (00928A1E). The
            // value is the air-ops record's party (game_hosts_script_orders.cpp
            // `squadron.party = record.party`), whose fused slot is this
            // squadron's leader. Counted whether or not the switch is on.
            const bsp::PlaneSquadronHostRecord* launched = nullptr;
            for (const bsp::PlaneSquadronHostRecord& r :
                 bsp::plane_squadron_registry().records()) {
                if (r.from_air_ops_launch && r.squadron_unit != bsp::kPlaneSquadronNoUnit
                    && r.squadron_unit + 1 == static_cast<std::size_t>(entity)) {
                    launched = &r;
                }
            }
            if (launched != nullptr && launched->party >= 0) {
                ++summary_.launched_party_nodes;
                node->launched_record = true;   // log only: counts its wing
                if constexpr (kLaunchedSquadronPartyCc13Bound) {
                    node->party = launched->party;
                    node->launched_party = true;
                }
            }
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

void GameMissionLuaHost::mark_vehicle_class_row_00964790(int class_index, const char* site) {
    if (!kVehicleClassRowIdBound || state_ == nullptr || class_index < 0) return;
    if (!vehicle_class_rows_marked_.insert(class_index).second) return;   // the cache hit
    vehicle_class_mark_site_.emplace(class_index, site);
    const int base = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (lua_istable(state_, -1)) {
        lua_rawgeti(state_, -1, class_index);
        if (lua_istable(state_, -1)) {
            // 009648A0..009648E7: LandingShip (00CEB7BC) read as an integer; a
            // non-zero value resolves that class first.
            lua_getfield(state_, -1, "LandingShip");
            const int landing = lua_isnumber(state_, -1)
                ? static_cast<int>(::lua_tointeger(state_, -1)) : 0;
            ::lua_settop(state_, ::lua_gettop(state_) - 1);
            if (landing != 0) {
                ::lua_settop(state_, base);
                mark_vehicle_class_row_00964790(landing);
                lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
                lua_rawgeti(state_, -1, class_index);
            }
            if (lua_istable(state_, -1)) {
                if constexpr (kLuaObjectFieldSettersCc13Bound) {
                    NativeLuaStateStorage owner;
                    construct_native_lua_state_00b66bd0(&owner);
                    owner.state_04 = state_;
                    NativeLuaObjectStorage row = stack_object(owner, ::lua_gettop(state_));
                    const ScopedNativeKey id("ID"), got("Got");
                    native_lua_set_integer_00b67460(row, id.key, class_index);   // 0096490C
                    native_lua_set_boolean_00b673a0(row, got.key, 1);            // 00964959
                } else {
                    lua_pushinteger(state_, class_index);   // 0096490C 00B67460
                    lua_setfield(state_, -2, "ID");
                    lua_pushboolean(state_, 1);             // 00964959 00B673A0
                    lua_setfield(state_, -2, "Got");
                }
                ++summary_.class_rows_marked;
            }
        }
    }
    ::lua_settop(state_, base);
    if constexpr (bsp::game::kVehicleClassPreloadCc15Bound) {
        // cc15-vcp2: 00964790 step 14 (00965182). A ship descriptor
        // (vtable[18h](6)) whose +0C0h, Catapult.LaunchedClass (009604AC,
        // -1 when Catapult is not a table), is not -1 resolves that class
        // too, with DL = 1. UNCERTAIN: the ship test is read as the eight
        // ship-sized branches of the 22-way Type match, Destroyer to
        // MotherShip.
        static constexpr const char* kShipTypes[] = {"Destroyer", "Cruiser", "LandingShip",
            "Cargo", "BattleShip", "Submarine", "TorpedoBoat", "MotherShip"};
        const std::string type = read_vehicle_class_string(class_index, "Type");
        bool ship = false;
        for (const char* name : kShipTypes) ship = ship || type == name;
        if (ship) {
            const int launched = read_vehicle_class_integer(class_index, "Catapult",
                "LaunchedClass", -1);
            if (launched != -1) mark_vehicle_class_row_00964790(launched, "00965182");
        }
        mark_vehicle_class_device_bullets_00960230(class_index);
    }
}

// cc15-vcp6. The Lua load in 00964790 step 13 (vtable[8h]) reaches the base
// reader 00960230. For each Platforms[i] it resolves every Gun[j] device
// through 00443090 (009614F9), then DefaultGun (or the first gun) at 00961591.
// The device reader at 0044334D ([vt+8h]: 007327B0, or the same body through
// BombPlatform 006E01E1 or Catapult 006EB6D1) passes each Bullet[k].Bullet to
// 006EA910 at 00731955. The plane reader 007D1F70 adds KamikazeBulletClass at
// 007D2BFF. A new bullet's reader (006EACFD, [vt+18h]) resolves a vehicle class
// with DL = 1 for three kinds: DummyKamikazePlane KamikazePlaneClass (006FF1E5),
// DummySubmarine SubmarineClass (007000C5) and DummyTarget
// DummyTargetVehicleClass (00701115). All of this runs inside the parent's
// resolve, so those classes are marked before 004E0A9B whenever the parent is.
// UNCERTAIN: KamikazeBulletClass is read here for any row, while the image
// reads it only for the plane family. Every row that has it in this
// installation names a Type "Kamikaze" bullet, which resolves no class.
void GameMissionLuaHost::mark_vehicle_class_device_bullets_00960230(int class_index) {
    if (state_ == nullptr) return;
    const int base = ::lua_gettop(state_);
    std::vector<int> devices;
    std::vector<int> bullets;
    const auto integers = [this](int table, std::vector<int>& out) {
        ::lua_pushnil(state_);
        while (::lua_next(state_, table) != 0) {
            if (::lua_isnumber(state_, -1)) out.push_back(static_cast<int>(::lua_tointeger(state_, -1)));
            ::lua_settop(state_, ::lua_gettop(state_) - 1);
        }
    };
    lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (lua_istable(state_, -1)) {
        lua_rawgeti(state_, -1, class_index);
        if (lua_istable(state_, -1)) {
            const int row = ::lua_gettop(state_);
            lua_getfield(state_, row, "KamikazeBulletClass");
            if (::lua_isnumber(state_, -1)) bullets.push_back(static_cast<int>(::lua_tointeger(state_, -1)));
            ::lua_settop(state_, row);
            lua_getfield(state_, row, "Platforms");
            if (lua_istable(state_, -1)) {
                const int platforms = ::lua_gettop(state_);
                ::lua_pushnil(state_);
                while (::lua_next(state_, platforms) != 0) {
                    const int platform = ::lua_gettop(state_);
                    if (lua_istable(state_, platform)) {
                        lua_getfield(state_, platform, "Gun");
                        if (lua_istable(state_, -1)) integers(::lua_gettop(state_), devices);
                        ::lua_settop(state_, platform);
                        lua_getfield(state_, platform, "DefaultGun");
                        if (::lua_isnumber(state_, -1) && ::lua_tointeger(state_, -1) > -1) {
                            devices.push_back(static_cast<int>(::lua_tointeger(state_, -1)));
                        }
                    }
                    ::lua_settop(state_, platform - 1);
                }
            }
        }
    }
    ::lua_settop(state_, base);
    lua_getfield(state_, LUA_GLOBALSINDEX, "DeviceClass");
    if (lua_istable(state_, -1)) {
        const int table = ::lua_gettop(state_);
        for (const int device : devices) {
            lua_rawgeti(state_, table, device);
            if (lua_istable(state_, -1)) {
                lua_getfield(state_, -1, "Bullet");
                if (lua_istable(state_, -1)) {
                    const int list = ::lua_gettop(state_);
                    ::lua_pushnil(state_);
                    while (::lua_next(state_, list) != 0) {
                        if (lua_istable(state_, -1)) {
                            lua_getfield(state_, -1, "Bullet");
                            if (::lua_isnumber(state_, -1)) {
                                bullets.push_back(static_cast<int>(::lua_tointeger(state_, -1)));
                            }
                        }
                        ::lua_settop(state_, list + 1);
                    }
                }
            }
            ::lua_settop(state_, table);
        }
    }
    ::lua_settop(state_, base);
    struct DummyKind { const char* type; const char* key; const char* site; };
    static constexpr DummyKind kKinds[] = {
        {"DummyKamikazePlane", "KamikazePlaneClass", "006FF1E5"},
        {"DummySubmarine", "SubmarineClass", "007000C5"},
        {"DummyTarget", "DummyTargetVehicleClass", "00701115"},
    };
    std::vector<std::pair<int, const char*>> linked;
    lua_getfield(state_, LUA_GLOBALSINDEX, "Bullets");
    if (lua_istable(state_, -1)) {
        const int table = ::lua_gettop(state_);
        for (const int bullet : bullets) {
            lua_rawgeti(state_, table, bullet);
            if (lua_istable(state_, -1)) {
                const int row = ::lua_gettop(state_);
                lua_getfield(state_, row, "Type");
                const char* type = ::lua_isstring(state_, -1) ? ::lua_tostring(state_, -1) : nullptr;
                for (const DummyKind& kind : kKinds) {
                    if (type == nullptr || _stricmp(type, kind.type) != 0) continue;
                    lua_getfield(state_, row, kind.key);
                    if (::lua_isnumber(state_, -1)) {
                        linked.emplace_back(static_cast<int>(::lua_tointeger(state_, -1)), kind.site);
                    }
                }
            }
            ::lua_settop(state_, table);
        }
    }
    ::lua_settop(state_, base);
    for (const auto& [linked_class, site] : linked) mark_vehicle_class_row_00964790(linked_class, site);
}

int GameMissionLuaHost::vehicle_class_nil_index_hook(lua_State* state) {
    // cc15-vcp3, harness diagnostic, not image behaviour: __index on
    // VehicleClass runs only for an absent key, and it returns nothing, which
    // is the nil a plain table gives. Only purged keys are counted.
    auto* host = static_cast<GameMissionLuaHost*>(lua_touserdata(state, lua_upvalueindex(1)));
    if (host != nullptr && lua_type(state, 2) == LUA_TNUMBER) {
        const auto row = host->vehicle_class_script_nil_rows_.find(
            static_cast<int>(lua_tointeger(state, 2)));
        if (row != host->vehicle_class_script_nil_rows_.end()) {
            ++row->second;
            ++host->vehicle_class_script_nil_reads_;
        }
    }
    return 0;
}

void GameMissionLuaHost::census_vehicle_class_purge_0095ca70(const char* site, bool purge) {
    // 0095CA70 walks VehicleClass with 00B67080 / 00B67190 and collects every
    // key whose value's Got (00CE452C) fails 00B66000, then runs
    // "VehicleClass[%d]=nil" (00D1A964) for each. This census only counts.
    // UNCERTAIN: 00B66000 is read here as "is the boolean true".
    if (state_ == nullptr) return;
    const int base = ::lua_gettop(state_);
    std::size_t rows = 0;
    std::size_t would_nil = 0;
    std::string kept;
    vehicle_class_would_nil_.clear();
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_pushnil(state_);
        while (::lua_next(state_, -2) != 0) {
            ++rows;
            bool got = false;
            if (::lua_type(state_, -1) == LUA_TTABLE) {
                ::lua_getfield(state_, -1, "Got");
                got = ::lua_type(state_, -1) == LUA_TBOOLEAN && ::lua_toboolean(state_, -1) != 0;
                ::lua_settop(state_, ::lua_gettop(state_) - 1);
            }
            const int index = ::lua_type(state_, -2) == LUA_TNUMBER
                ? static_cast<int>(::lua_tointeger(state_, -2)) : -1;
            if (got) {
                // cc15-vcp2: the site of a scene-time mark; a bare index is a
                // lazy host mark.
                const auto marker = vehicle_class_mark_site_.find(index);
                const char* at = marker != vehicle_class_mark_site_.end() ? marker->second : nullptr;
                if (kept.size() < 900) {
                    kept += " " + std::to_string(index) + (at != nullptr ? std::string("@") + at : "");
                }
            } else {
                ++would_nil;
                vehicle_class_would_nil_.emplace(index, 0);
            }
            ::lua_settop(state_, ::lua_gettop(state_) - 1);
        }
    }
    ::lua_settop(state_, base);
    ++vehicle_class_censuses_;
    vehicle_class_rows_would_nil_ = would_nil;
    log_.notef("vehicle class purge census at %s (0095CA70, %s): rows=%zu would_nil=%zu "
        "kept=%zu:%s", site, purge ? "PURGED" : "log only", rows, would_nil,
        rows - would_nil, kept.c_str());
    if (!purge) return;
    // cc15-vcp3: 0095CA70's second loop. Each index is formatted with
    // "VehicleClass[%d]=nil" (00D1A964) into a 2Ah-byte buffer and run by the
    // Lua machine (006B8AD0(buffer, 0, 0, 2)): a chunk, not a raw store.
    for (const auto& [index, reads] : vehicle_class_would_nil_) {
        static_cast<void>(reads);
        char chunk[0x2a];
        std::snprintf(chunk, sizeof chunk, "VehicleClass[%d]=nil", index);
        if (::luaL_loadbuffer(state_, chunk, std::strlen(chunk), chunk) != 0
            || ::lua_pcall(state_, 0, 0, 0) != 0) {
            ++vehicle_class_purge_failures_;
            ::lua_settop(state_, base);
            continue;
        }
        ++vehicle_class_purged_;
        vehicle_class_script_nil_rows_.emplace(index, 0);
    }
    // The harness hook that counts later script reads of the purged keys.
    // A table that already has a metatable is left alone.
    lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (lua_istable(state_, -1) && !lua_getmetatable(state_, -1)) {
        lua_createtable(state_, 0, 1);
        lua_pushlightuserdata(state_, this);
        lua_pushcclosure(state_, &GameMissionLuaHost::vehicle_class_nil_index_hook, 1);
        lua_setfield(state_, -2, "__index");
        lua_setmetatable(state_, -2);
    }
    ::lua_settop(state_, base);
    log_.notef("vehicle class purge at %s: purged=%zu failures=%zu (VehicleClass[%%d]=nil, "
        "00D1A964 through 006B8AD0)", site, vehicle_class_purged_, vehicle_class_purge_failures_);
}

void GameMissionLuaHost::note_vehicle_class_read(int index) {
    if (vehicle_class_censuses_ == 0) return;
    const auto row = vehicle_class_would_nil_.find(index);
    if (row == vehicle_class_would_nil_.end()) return;
    ++row->second;
    ++vehicle_class_later_reads_;
}

bool GameMissionLuaHost::attach_created_entity_00928a00(int entity_id,    const std::string& name, int class_index, bool seed_class, bool findable) {
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
        mark_vehicle_class_row_00964790(class_index);   // kVehicleClassRowIdBound
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
            if (kShipyardSceneAttachBound) bsp::shipyards().bind_entity_id(node.entity_id, node.name);
            host_.load_attached_.insert(node.entity_id);
        }
        if (host_.attach_created_entity_00928a00(node.entity_id, node.name,
                node.class_index, !kSEntityInitThisTableStepsBound, node.findable)) {
            host_.init_all_attached_.insert(node.entity_id);
            if (node.wing_member) ++host_.summary_.wing_member_tables;
            // Packet cc13-lsqparty: a launched squadron's node is written as a
            // generated one is (launched_party is only set with the switch on).
            if ((node.load_scene || node.generated_party || node.launched_party) &&
                (node.party >= 0 || node.race >= 0)) {
                host_.write_party_race_fields(node.entity_id, node.party, node.race);
                if (node.generated_party) {
                    ++host_.summary_.generated_party_writes;
                    host_.log_.notef("  generated entity party: thisTable[%d] (\"%s\") Party=%d "
                        "Race=%d written at pass A (packet cc9_generated_entity_party)",
                        node.entity_id, node.name.c_str(), node.party, node.race);
                } else if (node.launched_party) {
                    host_.log_.notef("  launched squadron party: thisTable[%d] (\"%s\") Party=%d "
                        "written at pass A (006C5050 bag Party, packet cc13-lsqparty)",
                        node.entity_id, node.name.c_str(), node.party);
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
                // Packet cc13-lsqparty2: the same 007F491A copy for a launched
                // squadron (launched_party is only set with its switch on).
                if (kGeneratedWingPartyBound && (node.generated_party || node.launched_party)) {
                    plane.party = node.party;
                    plane.race = node.race;
                    plane.generated_party = node.generated_party;
                    plane.launched_party = node.launched_party;
                }
                if (node.launched_record) ++host_.summary_.launched_wing_nodes;
                if (node.spawn_squadron_record) ++host_.summary_.spawn_squadron_wing_nodes;
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
            // Packet cc13-lsqparty2: the same 007F491A copy for a launched
            // squadron (launched_party is only set with its switch on).
            if (kGeneratedWingPartyBound && (node.generated_party || node.launched_party)) {
                plane.party = node.party;
                plane.race = node.race;
                plane.generated_party = node.generated_party;
                plane.launched_party = node.launched_party;
            }
            if (node.launched_record) ++host_.summary_.launched_wing_nodes;
            if (node.spawn_squadron_record) ++host_.summary_.spawn_squadron_wing_nodes;
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
    mark_vehicle_class_row_00964790(class_index);   // kVehicleClassRowIdBound
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

void GameMissionLuaHost::note_objective_text(const std::string& objective,
                                             const std::string& text) {
    log_.notef("mission objective: %s (id \"%s\", 008CD440 argument 3)", text.c_str(),
        objective.c_str());
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
    if (get_property_key_is(key, "TorpedoStock")) {
        // Packet p16_torpedo_stock_property. 00815870 (ship vtables +138h, and
        // 00758340's first call for the mother ship): 00779BB0 first (neither
        // of its keys), then 00815891 _stricmp(key, "TorpedoStock") == 0 ->
        // 00815850's value -> 00B664B0 (push integer).
        ++summary_.get_property_torpedo_stock_asked;
        if constexpr (!kGetPropertyTorpedoStockBound) {
            return -1;
        } else {
            GameUnitsHost* units = units_hooks_;
            const int id = air_ops_entity_id(state);
            if (units == nullptr || units->gunnery() == nullptr || id <= 0
                || static_cast<std::size_t>(id) > units->count()) {
                return 0;
            }
            const std::size_t index = static_cast<std::size_t>(id - 1);
            std::int32_t total = 0;
            if (!bsp::unit_kind_is_ship(units->unit_class_id(index))
                || !units->gunnery()->torpedo_stock_total_00815850(index, total)) {
                ++summary_.get_property_torpedo_stock_not_ship;
                return 0;
            }
            ++summary_.get_property_torpedo_stock_served;
            ::lua_pushinteger(state, static_cast<lua_Integer>(total));
            return 1;
        }
    }
    if (get_property_key_is(key, "state")) {
        // Packet p16_torpedo_stock_property. 007EF20E 00425850(key, "state"):
        // A counts the members whose (plane+72Ch)->vtable[38h] is true (the
        // free-flight mode, 0074E210 +1D4h == 7, kept here as +900h), B the
        // rest; 2 when B == 0, else (A != 0). Only a PlaneSquadronGen reaches
        // 007EF1C0; every other entity pushes nothing for this key.
        ++summary_.get_property_squadron_state_asked;
        if constexpr (!kGetPropertySquadronStateBound) {
            return -1;
        } else {
            GameUnitsHost* units = units_hooks_;
            const int id = air_ops_entity_id(state);
            if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
                return 0;
            }
            const std::size_t index = static_cast<std::size_t>(id - 1);
            const bsp::PlaneSquadronHostRecord* sq =
                bsp::plane_squadron_registry().find_by_member_unit(index);
            if (sq == nullptr || sq->squadron_unit != index) return 0;
            int free_flight = 0;
            int other = 0;
            for (const std::size_t member : sq->member_units) {
                int state_900 = 0;
                bool landed = false;
                bool enabled = false;
                unsigned int command_class = 0;
                if (!units->plane_order_view(member, state_900, landed, enabled, command_class)) {
                    continue;
                }
                if (state_900 == 7) ++free_flight; else ++other;
            }
            const int value = other == 0 ? 2 : (free_flight != 0 ? 1 : 0);
            ++summary_.get_property_squadron_state_served;
            if (value == 2) ++summary_.get_property_squadron_state_two;
            ::lua_pushinteger(state, static_cast<lua_Integer>(value));
            return 1;
        }
    }
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
    // Forced levels (SetForcedReconLevel) replace a party's level only under
    // kReconLevelForcedCc13Bound; OFF they are not consulted.
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
        if constexpr (bsp::kReconLevelForcedCc13Bound) {
            // 00779C12..00779C1F: record+10h set -> record+8h, the forced level as
            // SetForcedReconLevel stored it (00805D0E), in place of the computed one.
            int forced = 0;
            if (bsp::forced_recon_level_raw(party, index, forced)) level = forced;
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
            if (kLuaKillAttackerBound) {
                // Packet cc9_lua51_listener_args: keep both sets for 0096ACE0.
                entry.last_attacker_set = attacker;
                entry.last_attacker_ids = ignored;
                listener_read_int_set_fn(state, 3, "lastAttackerPlayerIndex",
                    entry.last_attacker_player_indices);
            }
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
        if (kLuaEntityKilledListenersBound && listener_key_equal(entry.channel, "entityKilled")) {
            // Packet cc13-ver5. 00972F00: callback, entity (+0Ch, 009721C0) and
            // killReason (+1Ch, 00970FF0, a table of strings).
            listener_read_entity_set(state, 3, "entity", entry.entity_ids);
            ::lua_getfield(state, 3, "killReason");
            if (::lua_type(state, -1) == LUA_TTABLE) {
                const int reasons = ::lua_gettop(state);
                ::lua_pushnil(state);
                while (::lua_next(state, reasons) != 0) {
                    if (::lua_type(state, -1) == LUA_TSTRING) {
                        const char* text = ::lua_tolstring(state, -1, nullptr);
                        if (text != nullptr) entry.kill_reasons.emplace_back(text);
                    }
                    ::lua_pop(state, 1);
                }
            }
            ::lua_pop(state, 1);
        }
        if (kLuaExitZoneListenersBound && listener_key_equal(entry.channel, "exitzone")) {
            // Packet cc13-fix3. 009726D0: callback, entity (+0Ch, 009721C0) and exited
            // (+1Ch, a table of booleans).
            listener_read_entity_set(state, 3, "entity", entry.entity_ids);
            ::lua_getfield(state, 3, "exited");
            if (::lua_type(state, -1) == LUA_TTABLE) {
                const int values = ::lua_gettop(state);
                ::lua_pushnil(state);
                while (::lua_next(state, values) != 0) {
                    if (::lua_type(state, -1) == LUA_TBOOLEAN) {
                        entry.exited_values.push_back(::lua_toboolean(state, -1) != 0 ? 1 : 0);
                    }
                    ::lua_pop(state, 1);
                }
            }
            ::lua_pop(state, 1);
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

// Packet cc13-ver5, under kLuaEntityKilledListenersBound. 00986480's `entityKilled`
// channel, reached from every unit destroy (0077D1A0 -> 0077D253). The victim is the
// unit's entity (id = index + 1) and the reason is 00E0CF04[entity+70h], the cause the
// host's kill recorded (kill_cause_00926d90, as the `Dead` publisher in
// game_hosts_script_orders.cpp reads it: 1 "harm" unless a kill passed another).
// 00968FF0 tests the entity set, then the killReason set; an empty set matches all.
void GameMissionLuaHost::dispatch_entity_killed_listeners_00986480() {
    if (units_hooks_ == nullptr || state_ == nullptr) return;
    const std::vector<std::pair<std::size_t, float>> deaths = units_hooks_->destroyed_units();
    if (listener_entity_killed_seen_.size() < units_hooks_->count()) {
        listener_entity_killed_seen_.resize(units_hooks_->count(), false);
    }
    // 00E0CF04: none, harm, soft, sell, exitzone, landed, editor.
    static const char* const kKillReasons[] = {
        "none", "harm", "soft", "sell", "exitzone", "landed", "editor"};
    for (const auto& death : deaths) {
        const std::size_t unit = death.first;
        if (unit >= listener_entity_killed_seen_.size() || listener_entity_killed_seen_[unit]) {
            continue;
        }
        // The same squadron hold as the `kill` dispatcher (SUBSTITUTION, labelled there).
        bool squadron_alive = false;
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == unit
                && r.live_count() > 0) {
                squadron_alive = true;
                break;
            }
        }
        if (squadron_alive) continue;
        listener_entity_killed_seen_[unit] = true;
        ++summary_.listener_entity_killed_deaths;
        const int victim = static_cast<int>(unit + 1);
        const int cause = units_hooks_->kill_cause_00926d90(unit);
        const char* const kReason = (cause >= 0 && cause < 7) ? kKillReasons[cause] : "harm";
        // 0097B8C0 collects every matching callback first, then 00887E50 calls each.
        std::vector<std::string> callbacks;
        for (const ListenerEntry& entry : listeners_) {
            if (!listener_key_equal(entry.channel, "entityKilled")) continue;
            if (!entry.entity_ids.empty()
                && std::find(entry.entity_ids.begin(), entry.entity_ids.end(), victim)
                    == entry.entity_ids.end()) {
                continue;
            }
            if (!entry.kill_reasons.empty()) {
                bool reason = false;
                for (const std::string& r : entry.kill_reasons) {
                    if (listener_key_equal(r, kReason)) { reason = true; break; }
                }
                if (!reason) continue;
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
            ++summary_.listener_entity_killed_fires;
            const GameUnitRow* row = units_hooks_->unit_row(unit);
            log_.notef("  entityKilled listener 00986480: entity \"%s\" reason %s -> %s() "
                "(packet cc13-ver5)", row != nullptr ? row->name.c_str() : "?", kReason,
                name.c_str());
            int pushed = 0;
            if (kLuaListenerCallbackArgsBound) {
                if (!push_resolved_entity_by_id(state_, victim)) lua_pushnil(state_);
                ::lua_pushstring(state_, kReason);
                pushed = 2;
            }
            if (::lua_pcall(state_, pushed, 0, 0) != 0) {
                const char* message = lua_tolstring(state_, -1, nullptr);
                note_error(message != nullptr ? message : std::string("(no message)"));
            }
            ::lua_settop(state_, top);
        }
    }
}

void GameMissionLuaHost::note_exit_zone_00982120(std::size_t unit, bool exited) {
    if constexpr (kLuaExitZoneListenersBound) {
        exit_zone_pending_.emplace_back(unit, exited);
    } else {
        static_cast<void>(unit);
        static_cast<void>(exited);
    }
}

// Packet cc13-fix3, under kLuaExitZoneListenersBound. 00982120's `exitzone` channel.
// The plane producer is 007F31A0, whose 007F31E6 call fires (plane, 1) before the
// plane's Kill(4) at 007F3418; the host records that kill as cause 4 (the units host's
// retreat leave-map arm, note_kill_cause(..., 4)), so each such death is one event. The
// entity is plane->vtable[140h]() = 007B97E0, plane+9D4h, the squadron (id = squadron
// slot + 1). Other producers queue through note_exit_zone_00982120. 00968710 tests the
// entity set (+0Ch), then the exited set (+1Ch); an empty set matches all.
void GameMissionLuaHost::dispatch_exit_zone_listeners_00982120() {
    if (units_hooks_ == nullptr || state_ == nullptr) return;
    std::vector<std::pair<std::size_t, bool>> events;
    events.swap(exit_zone_pending_);
    const std::vector<std::pair<std::size_t, float>> deaths = units_hooks_->destroyed_units();
    if (listener_exit_zone_seen_.size() < units_hooks_->count()) {
        listener_exit_zone_seen_.resize(units_hooks_->count(), false);
    }
    for (const auto& death : deaths) {
        const std::size_t unit = death.first;
        if (unit >= listener_exit_zone_seen_.size() || listener_exit_zone_seen_[unit]) continue;
        listener_exit_zone_seen_[unit] = true;
        if (units_hooks_->kill_cause_00926d90(unit) != 4) continue;
        const bsp::PlaneSquadronHostRecord* sq =
            bsp::plane_squadron_registry().find_by_member_unit(unit);
        if (sq == nullptr) continue;   // not a plane: 007F31A0 is the squadron's arm
        const std::size_t entity = sq->squadron_unit != bsp::kPlaneSquadronNoUnit
            ? sq->squadron_unit : unit;
        // 007F31CF CMP [squadron+361h],0 / JNE past the call. SUBSTITUTION (labelled at
        // the switch): only the deaths at the squadron's first exit clock fire.
        const auto first = exit_zone_first_clock_.emplace(entity, death.second).first;
        if (first->second != death.second) {
            ++summary_.listener_exit_zone_latched;
            continue;
        }
        events.emplace_back(entity, true);
    }
    for (const auto& event : events) {
        ++summary_.listener_exit_zone_events;
        const int victim = static_cast<int>(event.first + 1);
        const int exited = event.second ? 1 : 0;
        // 0097B8C0 collects every matching callback first, then 00887E50 calls each.
        std::vector<std::string> callbacks;
        for (const ListenerEntry& entry : listeners_) {
            if (!listener_key_equal(entry.channel, "exitzone")) continue;
            if (!entry.entity_ids.empty()
                && std::find(entry.entity_ids.begin(), entry.entity_ids.end(), victim)
                    == entry.entity_ids.end()) {
                continue;
            }
            if (!entry.exited_values.empty()
                && std::find(entry.exited_values.begin(), entry.exited_values.end(), exited)
                    == entry.exited_values.end()) {
                continue;
            }
            if (!entry.callback.empty()) callbacks.push_back(entry.callback);
        }
        const GameUnitRow* row = units_hooks_->unit_row(event.first);
        log_.notef("  exitzone 00982120: entity \"%s\" exited=%d matches=%zu (packet cc13-fix3)",
            row != nullptr ? row->name.c_str() : "?", exited, callbacks.size());
        for (const std::string& name : callbacks) {
            const int top = ::lua_gettop(state_);
            lua_getfield(state_, LUA_GLOBALSINDEX, name.c_str());
            if (!lua_isfunction(state_, -1)) {
                ::lua_settop(state_, top);
                continue;
            }
            ++summary_.listener_exit_zone_fires;
            log_.notef("  exitzone listener 00982120: entity \"%s\" exited=%d -> %s() "
                "(packet cc13-fix3)", row != nullptr ? row->name.c_str() : "?", exited,
                name.c_str());
            int pushed = 0;
            if (kLuaListenerCallbackArgsBound) {
                // 00884A90 the entity, 00884A30 the byte as a boolean.
                if (!push_resolved_entity_by_id(state_, victim)) lua_pushnil(state_);
                ::lua_pushboolean(state_, exited);
                pushed = 2;
            }
            if (::lua_pcall(state_, pushed, 0, 0) != 0) {
                const char* message = lua_tolstring(state_, -1, nullptr);
                note_error(message != nullptr ? message : std::string("(no message)"));
            }
            ::lua_settop(state_, top);
        }
    }
}

// ---------------------------------------------------------------------------
// Packet cc13-fix10, under kLuaProximityTriggersBound (docs/CC13_FIX10.md).
// ---------------------------------------------------------------------------

// 008C9730 AddProximityTrigger(entity, name?, callback, target, range). The binding
// reads argument 3 through 0088A810, argument 4 as a number (00B66270), makes the
// trigger with 007505C0 (new 60h, 0074FEE0: +5Ch = range * range as a float, +11h = 1,
// +18h = 2), reads argument 1 through 008C9350 (nil leaves the name disengaged) and
// argument 2 through 00B662B0, stores both with 00750790 (+1Ch/+24h, +28h), and hands
// the trigger to the list entity(0)->vtable[60h]() through 0074FAB0 -> vtable[4h]
// 0071C470 -> 00876020, which links it after every sub-node of priority 2 or less.
// Returns 0.
// SUBSTITUTIONS (labelled): the list is this host's per-unit list, keyed by the
// entity's ID; an entity that is not a unit of this host is counted unresolved and
// nothing is stored (the image would attach to any entity's element). 0074FEE0's
// observer on an entity target (00694A60, OnTargetGone 0074FFE0) is the target id the
// record keeps; target_gone_drain_009273a0 answers for it (packet cc13-fix19).
int GameMissionLuaHost::run_add_proximity_trigger_008c9730(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.proximity_trigger_adds;
    ProximityTrigger trigger;
    // 0088A810 on argument 3 (Lua slot 4): `ID` not nil -> 00888AA0 entity, kind 1,
    // the position left at the zero point and position_valid 0; a null entity leaves
    // kind 0 and position_valid 0 (the zero point). `ID` nil -> 00888760 Vector3,
    // position_valid 1.
    bool id_nil = true;
    int target_id = 0;
    if (::lua_type(state, 4) == LUA_TTABLE) {
        ::lua_getfield(state, 4, "ID");
        const int type = ::lua_type(state, -1);
        id_nil = type == LUA_TNIL;
        if (type == LUA_TNUMBER || type == LUA_TSTRING) {
            target_id = static_cast<int>(::lua_tonumber(state, -1));
        }
        ::lua_pop(state, 1);
    }
    if (!id_nil) {
        // 00888AA0's resolve, as GameScriptOrdersHost::entity_from_argument: a unit
        // of this host or a scene marker (NavPoint, building) the find-entity pass
        // made; spawn_ref_entity_frame_008f8680 answers for exactly those.
        float frame[16];
        if (spawn_ref_entity_frame_008f8680(target_id, frame)) {
            trigger.target_kind_44 = 1;
            trigger.target_id = target_id;
        }
    } else if (::lua_type(state, 4) == LUA_TTABLE) {
        // 00888760, read as GameScriptOrdersHost::argument_vector3 reads it.
        static const char* const kKeys[3] = {"x", "y", "z"};
        for (int lane = 0; lane < 3; ++lane) {
            ::lua_getfield(state, 4, kKeys[lane]);
            if (::lua_type(state, -1) == LUA_TNUMBER) {
                trigger.target_point[lane] = static_cast<float>(::lua_tonumber(state, -1));
            }
            ::lua_pop(state, 1);
        }
        trigger.target_point_valid_45 = true;
    }
    // 00B66270 on argument 4, FSTP float; 0074FF4E FMUL ST0,ST0 then FSTP float.
    const float range = static_cast<float>(::lua_tonumber(state, 5));
    trigger.radius_sq_5c = bsp::gameplay_seg41_cc13::trigger_radius_squared(range);
    // 008C9350 on argument 1: nil leaves +24h clear.
    if (!lua_isnoneornil(state, 2)) {
        trigger.named_24 = true;
        trigger.name_1c = listener_string_argument(state, 2);
    }
    // 00B662B0 on argument 2.
    trigger.callback_28 = listener_string_argument(state, 3);
    // 00888AA0 on argument 0, then vtable[60h]().
    const int id = air_ops_entity_id(state);
    if (units_hooks_ == nullptr || id <= 0
        || static_cast<std::size_t>(id) > units_hooks_->count()) {
        ++summary_.proximity_trigger_unresolved;
        log_.notef("  AddProximityTrigger 008c9730: entity id %d has no units-host slot; "
            "trigger \"%s\" -> %s() not stored (packet cc13-fix10)", id,
            trigger.name_1c.c_str(), trigger.callback_28.c_str());
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    const GameUnitRow* row = units_hooks_->unit_row(index);
    log_.notef("  AddProximityTrigger 008c9730: \"%s\" name=%s%s%s -> %s() target=%s%d "
        "range=%.1f (packet cc13-fix10)", row != nullptr ? row->name.c_str() : "?",
        trigger.named_24 ? "\"" : "", trigger.named_24 ? trigger.name_1c.c_str() : "nil",
        trigger.named_24 ? "\"" : "", trigger.callback_28.c_str(),
        trigger.target_kind_44 != 0 ? "entity " : (trigger.target_point_valid_45 ? "point " : "zero "),
        trigger.target_id, static_cast<double>(range));
    // 00876020: every trigger is priority 2, so the new one goes to the end.
    proximity_triggers_[index].push_back(std::move(trigger));
    log_.implemented("MissionLuaNative::AddProximityTrigger", "008c9730");
    return 0;
}

// 008C5FD0 RemoveTrigger(entity, name): list = entity(0)->vtable[60h]() (008C60ED),
// name = 00B662B0 argument 1, then 0074FBB0 walks the list (+1Ch, next +0Ch): every
// node whose vtable[8h](1) answers (the Lua trigger base 0074FCD0), whose name is
// engaged (+24h) and equal (00435C40: same length, _stricmp) gets vtable[10h]
// (0071C4A0: +10h = 1, +11h = 0). All matches are retired, not only the first. Pushes
// whether any matched (00B66xxx boolean) and returns 1.
// SUBSTITUTION (labelled): only the proximity and (packet cc13-fix19) squad-ordnance
// triggers are this host's, each stored only while its switch is on; a line-of-sight
// trigger of the same name (unbound) answers false here.
int GameMissionLuaHost::run_remove_trigger_008c5fd0(lua_State* state, int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.remove_trigger_calls;
    const int id = air_ops_entity_id(state);
    const std::string name = listener_string_argument(state, 2);
    bool removed = false;
    if (id > 0) {
        const auto found = proximity_triggers_.find(static_cast<std::size_t>(id - 1));
        if (found != proximity_triggers_.end()) {
            for (ProximityTrigger& trigger : found->second) {
                if (!trigger.named_24 || !listener_key_equal(trigger.name_1c, name)) continue;
                trigger.expired_10 = true;   // 0071C4A0
                trigger.enabled_11 = false;
                removed = true;
            }
        }
    }
    if (removed) ++summary_.remove_trigger_matched;
    const GameUnitRow* row = units_hooks_ != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units_hooks_->count()
        ? units_hooks_->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    log_.notef("  RemoveTrigger 008c5fd0: \"%s\" \"%s\" -> %s (packet cc13-fix10)",
        row != nullptr ? row->name.c_str() : "?", name.c_str(), removed ? "true" : "false");
    ::lua_pushboolean(state, removed ? 1 : 0);
    log_.implemented("MissionLuaNative::RemoveTrigger", "008c5fd0");
    return 1;
}

// Wave 2 of the fixed step (00875B90 -> 008759B0) over each watched unit's triggers.
// 008759B0 takes the next sub-node before it runs the current one, runs vtable[0Ch]
// when +11h is set (00750000 for a proximity trigger), and releases the node when
// +10h is set (unlink, vtable[0](1)). A trigger fires once: 0074FDD0 ends in
// vtable[10h], so the same walk releases it. A trigger added by a callback is
// appended (priority 2) and is reached in this walk only when the walk had not yet
// taken its last node, as in the image.
// SUBSTITUTIONS (labelled): the walk runs once per fixed step after this host's
// motion step (the image runs it in wave 2, after wave 1's element steps and before
// the fan-out); the units are walked in index order (the image walks the job groups'
// element order); wave 2's admission (+5Eh clear, +BDh set) is unit_active(). Packet
// cc13-fix19b: a squadron's list is keyed by its fused wing-0 slot, but the image
// tests the squadron entity. Its +5Eh is set by pass K (009274D2 MOV byte [ESI+5Eh],1)
// once the squadron has been Killed, and the squadron is Killed by 007F3970 when
// +3CCh reaches 0 (007F3A16..007F3A2A); Lua Kill of a squadron Kills only its members
// (008AC732 007ED380, then 008AC737 JMP past 00926D90). Other direct Kills of a
// squadron object are not enumerated. So a fused slot whose squadron still holds a
// plane (plane_squadron_live_fused_slot) is admitted after the wing-0 plane dies, and
// (cc13-fix19c) a fused slot is refused once the drain has noticed the squadron's
// Kill (target_gone_notified_, the +5Eh stand-in), which also covers the up-to-one-
// step window where gunnery counts every member dead before compaction.
void GameMissionLuaHost::run_proximity_triggers_008759b0(float step) {
    static_cast<void>(step);   // 00750000 is RET 4 and never reads it
    if (units_hooks_ == nullptr || state_ == nullptr) return;
    for (auto& entry : proximity_triggers_) {
        const std::size_t unit = entry.first;
        if (!units_hooks_->unit_active(unit)
            && bsp::plane_squadron_live_fused_slot(unit) == nullptr) continue;
        if (unit < target_gone_notified_.size() && target_gone_notified_[unit]) {
            bool squadron_slot = false;
            for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
                if (r.squadron_unit == unit) {
                    squadron_slot = true;
                    break;
                }
            }
            if (squadron_slot) continue;
        }
        std::list<ProximityTrigger>& list = entry.second;
        for (auto it = list.begin(); it != list.end();) {
            const auto next = std::next(it);
            if (it->enabled_11) {
                // vtable[0Ch]: 00750000 (00D00580) or 007504F0 (00D005C0).
                if (it->kind == ProximityTrigger::Kind::SquadOrdnanceDropped) {
                    check_squad_ordnance_trigger_007504f0(unit, *it);
                } else {
                    check_proximity_trigger_00750000(unit, *it);
                }
            }
            if (it->expired_10) list.erase(it);
            it = next;
        }
    }
}

// Packet cc13-fix19, under kLuaSquadOrdnanceDroppedTriggerBound (docs/CC13_FIX19.md).
// 008C9A10 AddSquadOrdnanceDroppedTrigger(entity, name?, callback). The binding makes
// the trigger first (00750660: new 34h, vtable 00D005C0, +11h = 1, +14h = 1, +18h = 2,
// +30h = 0), reads argument 1 through 008C9350 (nil leaves the name disengaged) and
// argument 2 through 00B662B0, stores both with 00750790, then hands the trigger to
// entity(0)->vtable[60h]() through 0074FAB0 -> vtable[4h] 007504C0: 00876020 (after
// every sub-node of priority 2 or less) and +30h = element+28h. Returns 0. No
// argument is a number, so nothing else is read.
// SUBSTITUTIONS (labelled): the list is this host's per-unit list, as for
// AddProximityTrigger; the entity must be a squadron of the host's registry (the
// squadron's slot keys the list), because 007504F0 reads the squadron's member array.
// Every use in this installation's scripts passes a squadron. Anything else is counted
// unresolved and nothing is stored (the image would attach it and read +3CCh of a
// non-squadron).
int GameMissionLuaHost::run_add_squad_ordnance_dropped_trigger_008c9a10(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.ordnance_trigger_adds;
    ProximityTrigger trigger;
    trigger.kind = ProximityTrigger::Kind::SquadOrdnanceDropped;
    // 008C9350 on argument 1: nil leaves +24h clear.
    if (!lua_isnoneornil(state, 2)) {
        trigger.named_24 = true;
        trigger.name_1c = listener_string_argument(state, 2);
    }
    // 00B662B0 on argument 2.
    trigger.callback_28 = listener_string_argument(state, 3);
    // 00888AA0 on argument 0, then vtable[60h]().
    const int id = air_ops_entity_id(state);
    bool squadron = false;
    if (units_hooks_ != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units_hooks_->count()) {
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit == static_cast<std::size_t>(id - 1)) {
                squadron = true;
                break;
            }
        }
    }
    if (!squadron) {
        ++summary_.ordnance_trigger_unresolved;
        log_.notef("  AddSquadOrdnanceDroppedTrigger 008c9a10: entity id %d is not a squadron "
            "of this host; trigger \"%s\" -> %s() not stored (packet cc13-fix19)", id,
            trigger.name_1c.c_str(), trigger.callback_28.c_str());
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    const GameUnitRow* row = units_hooks_->unit_row(index);
    log_.notef("  AddSquadOrdnanceDroppedTrigger 008c9a10: \"%s\" name=%s%s%s -> %s() "
        "(packet cc13-fix19)", row != nullptr ? row->name.c_str() : "?",
        trigger.named_24 ? "\"" : "", trigger.named_24 ? trigger.name_1c.c_str() : "nil",
        trigger.named_24 ? "\"" : "", trigger.callback_28.c_str());
    // 00876020: every Lua trigger is priority 2, so the new one goes to the end.
    proximity_triggers_[index].push_back(std::move(trigger));
    log_.implemented("MissionLuaNative::AddSquadOrdnanceDroppedTrigger", "008c9a10");
    return 0;
}

// 007504F0 (vtable 00D005C0 slot 0Ch), __thiscall(trigger, float step), RET 4; the step
// is not read. +30h (the squadron, from 007504C0) null -> vtable[10h], a silent
// retire. Else 007ED790(ECX = +30h, 0): for i < +3CCh, 007B9140(ECX = [+3D0h + 4i], 0)
// answers true for a kind-17h plane (MPlaneKamikaze, 007B9150) or when a rack device
// (+974h, count +994h) answers vtable[210h](2Ah, 0) (006E3FE0: an attached round
// whose vtable[5Ch](2Ah) is true; the flag-0 call skips the reload arm). Any true
// keeps the trigger; all false (or no member) -> 0074FDD0: the callback, then the
// retire. One-shot.
// SUBSTITUTIONS (labelled): +30h is never null here (the squadron slot keys the list).
// The squadron's live members (registry members the gunnery host does not count
// dead, as the cc9 spent-ordnance latch reads 007ED790) stand for +3D0h/+3CCh; with
// none the trigger is not tested, standing for the squadron's +5Eh (its Kill when
// 007F3970 removes the last plane drains at row 15, before the next walk), so a
// squadron that dies never fires it. The per-plane test is 007B9140's kind-17h arm
// (unit_is_kind_of 17h) and plane_holds_rack_round_007b9140 (the rack walk; its
// flag-1 arm needs [00E17BF2], 0 in single player, so it answers as flag 0).
void GameMissionLuaHost::check_squad_ordnance_trigger_007504f0(std::size_t unit,
    ProximityTrigger& trigger) {
    const bsp::PlaneSquadronHostRecord* record = nullptr;
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit == unit) {
            record = &r;
            break;
        }
    }
    GameGunneryHost* gunnery = units_hooks_->gunnery();
    if (record == nullptr || gunnery == nullptr) return;
    std::vector<std::size_t> live;
    for (const std::size_t m : record->member_units) {
        if (m != bsp::kPlaneSquadronNoUnit && m < units_hooks_->count() && !gunnery->unit_dead(m))
            live.push_back(m);
    }
    if (live.empty()) return;
    ++summary_.ordnance_trigger_checks;
    for (const std::size_t m : live) {
        // 007ED7B3 007B9140(member, 0); 007ED7BA JNE: the first true ends the test.
        if (units_hooks_->unit_is_kind_of(m, 0x17)) return;
        if (units_hooks_->plane_holds_rack_round_007b9140(m)) return;
    }
    // 00750514 0074FDD0.
    ++summary_.ordnance_trigger_fires;
    const GameUnitRow* row = units_hooks_->unit_row(unit);
    log_.notef("  squad ordnance trigger 007504f0: \"%s\" %s%s%s -> %s() live=%zu "
        "(packet cc13-fix19)", row != nullptr ? row->name.c_str() : "?",
        trigger.named_24 ? "\"" : "", trigger.named_24 ? trigger.name_1c.c_str() : "nil",
        trigger.named_24 ? "\"" : "", trigger.callback_28.c_str(), live.size());
    fire_lua_trigger_0074fdd0(unit, trigger);
}

// Packet cc13-fix19, under kLuaProximityTriggersBound. 009273A0 pass K, steps d-f
// (009274CE..0092751F): an entity whose +5Eh is clear gets +5Eh set, then 00925C40
// (inlined at 009274DE) hands it to 00696330, which runs slot +4h of every callback
// owner observing it. A proximity trigger's owner sub-object (+30h, adjustor table
// 00D00568) answers with 0074FFE0: when the notifier is 00521EA0(+44h), its command
// target, vtable[10h] of the trigger (0071C4A0: +10h = 1, +11h = 0). No callback.
// The walk releases the node on its next pass over the watched unit's list.
// SUBSTITUTIONS (labelled): the notices come from the host's Kill sites that reach
// this hook: the host's inline pass-K sites (through on_entity_killed_00928c80: the
// ship KillDepth and kamikaze removals) and Lua Kill's victims (008AC5C0 -> 00926D90).
// Other Kill callers are not fed (the host's gunnery funnel does not tell a Kill from
// a Destroy). A squadron is noticed at the drain when the host's registry shows no
// live member after it has held one (cc13-fix19b: sticky, because compaction empties
// the record), standing for 007F3970's Kill of the squadron when its last plane goes
// (007F3A16 CMP [ESI+3CCh],0 / JNE, 007F3A2A 00926D90). Packet cc13-fix19c: every
// registry squadron is polled at every drain, watched or not, as the image's Kill
// does not depend on an observer; the notice then sets the squadron's +5Eh stand-in
// (target_gone_notified_), so a trigger made later is never retired by it, as in the
// image (pass K handles an entity once). Lua Kill of a squadron (008AC72F JE /
// 008AC732 007ED380 / 008AC737 JMP 008AC75B) Kills only the members, never the
// squadron itself, so the squadron's notice comes from this poll alone: a notice for
// the fused wing-0 slot from a plane-level Kill site is not the squadron's and is
// dropped here (cc13-fix19c; before, a kamikaze wing-0 plane retired triggers aimed
// at a squadron whose other planes lived). RESIDUAL (labelled, cc13-fix19d): the
// wing-0 PLANE shares the fused slot's id with its squadron. So a trigger aimed at
// that plane, which used to retire on the plane's Lua Kill, kamikaze or KillDepth,
// now retires only when the whole squadron goes. No reachable script in this
// installation aims at a single plane this way. The squadron targets are PRCP01's
// usn_03:961 Mission.HornetKillers[1] and JM02's prcpjm02.lua:1123
// luaPickRnd(Mission.Bombers) (docs/CC13_FIX19.md, item 5).
// Once per unit, standing for the +5Eh test (009274C8).
void GameMissionLuaHost::target_gone_00925c40(std::size_t unit_index) {
    if constexpr (!kLuaProximityTriggersBound) {
        static_cast<void>(unit_index);
    } else {
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit == unit_index) return;
        }
        target_gone_mark_(unit_index);
    }
}

void GameMissionLuaHost::target_gone_mark_(std::size_t unit_index) {
    if constexpr (!kLuaProximityTriggersBound) {
        static_cast<void>(unit_index);
    } else {
        if (target_gone_notified_.size() <= unit_index) {
            target_gone_notified_.resize(unit_index + 1, false);
        }
        if (target_gone_notified_[unit_index]) return;
        target_gone_notified_[unit_index] = true;
        target_gone_pending_.push_back(unit_index);
    }
}

void GameMissionLuaHost::target_gone_drain_009273a0() {
    if constexpr (kLuaProximityTriggersBound) {
        // The squadrons first: the registry's view of 007F3970's Kill, for every
        // registry squadron (cc13-fix19c), watched or not. A record that never had a
        // member is not taken as gone.
        GameGunneryHost* gunnery = units_hooks_ != nullptr ? units_hooks_->gunnery() : nullptr;
        if (gunnery != nullptr) {
            for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
                const std::size_t target = r.squadron_unit;
                if (target == bsp::kPlaneSquadronNoUnit) continue;
                if (target < target_gone_notified_.size() && target_gone_notified_[target]) continue;
                // Packet cc13-fix19b: the compaction (kSquadronRemovesDeadBound, and the
                // stock return's 007F3970) clears a dead or landed member to NoUnit, so
                // "had a member" is kept sticky; without it an emptied record never read
                // as gone.
                // TIMING SUBSTITUTION (labelled, cc13-fix19c): a member the gunnery host
                // counts dead but has not yet compacted is not live either. For a plane
                // that dies, the image drops +3CCh at its destroy flush (007BCAA0 ->
                // 007F3970), so this can notice the squadron's Kill up to one units step
                // early. The other 007F3970 callers Kill and remove at once, so they
                // leave no window: the landed plane (007F3CD9 00926D90 cause 5, then
                // 007F3CE7) and Lua SquadronLandAndKill (008A223F, then 008A2249).
                bool live = false;
                for (const std::size_t m : r.member_units) {
                    if (m == bsp::kPlaneSquadronNoUnit || m >= units_hooks_->count()) continue;
                    if (squadron_target_resolved_.size() <= target) {
                        squadron_target_resolved_.resize(target + 1, false);
                    }
                    squadron_target_resolved_[target] = true;
                    if (!gunnery->unit_dead(m)) live = true;
                }
                // 007F3A16 CMP [ESI+3CCh],0 / JNE; 007F3A2A 00926D90 (the squadron).
                if (!live && target < squadron_target_resolved_.size()
                    && squadron_target_resolved_[target]) {
                    target_gone_mark_(target);
                }
            }
        }
        std::vector<std::size_t> pending;
        pending.swap(target_gone_pending_);
        for (const std::size_t gone : pending) {
            ++summary_.target_gone_notices;
            const int gone_id = static_cast<int>(gone + 1);
            for (auto& entry : proximity_triggers_) {
                for (ProximityTrigger& trigger : entry.second) {
                    // 0074FFE0: notifier == 00521EA0(+44h). Only a proximity trigger
                    // with an entity target registered the observer (0074FEE0).
                    if (trigger.kind != ProximityTrigger::Kind::Proximity
                        || trigger.target_kind_44 == 0 || trigger.target_id != gone_id) continue;
                    if (trigger.enabled_11) {
                        ++summary_.target_gone_retired;
                        const GameUnitRow* row = units_hooks_ != nullptr
                            ? units_hooks_->unit_row(entry.first) : nullptr;
                        log_.notef("  proximity trigger 0074ffe0: \"%s\" %s%s%s target id %d "
                            "gone, retired (packet cc13-fix19)",
                            row != nullptr ? row->name.c_str() : "?",
                            trigger.named_24 ? "\"" : "",
                            trigger.named_24 ? trigger.name_1c.c_str() : "nil",
                            trigger.named_24 ? "\"" : "", gone_id);
                    }
                    trigger.expired_10 = true;   // vtable[10h], 0071C4A0
                    trigger.enabled_11 = false;
                }
            }
        }
    }
}

// 00750000: the watched unit's +FCh/+104h after the pose refresh, the descriptor
// +44h through 006F7DD0, the planar x/z test (fires when +5Ch > d2; a NaN never
// fires). A hit calls
// 0074FDD0: the variant vector {00884A90 entity, the name (00886A10) or nil (tag 6)},
// 00887E50(host, 0, +28h, args, 0, -1), then vtable[10h] (0071C4A0).
// SUBSTITUTIONS (labelled): an entity target's position is the unit's pose
// translation or the scene marker's authored position; a target this host cannot
// place skips the test. 00887E50's queued and `game+1FE4h == 2` arms are not
// modelled: the call runs inline. Packet cc13-fix19b: a watched squadron (its fused
// wing-0 slot) is posed at its current leader, +3D0h[0] after compaction, standing
// for the squadron's +FCh (docs/CONSTRUCT_WORLD.md "the squadron's position +FCh |
// the leader's pose"; the +FCh writer is unread); while wing 0 lives that is the slot.
void GameMissionLuaHost::check_proximity_trigger_00750000(std::size_t unit,
    ProximityTrigger& trigger) {
    float right[3], up[3], forward[3], watched[3];
    std::size_t posed = unit;
    if (const bsp::PlaneSquadronHostRecord* sq = bsp::plane_squadron_live_fused_slot(unit)) {
        posed = sq->flight_leader();
    }
    if (!units_hooks_->unit_pose(posed, right, up, forward, watched)) return;
    float target[3] = {0.0f, 0.0f, 0.0f};   // the zero point 00F87574
    if (trigger.target_kind_44 != 0) {
        // 006F7DD0's entity arm: (00414DB0 when +C8h is clear) then +FCh/+100h/+104h,
        // the translation row of +CCh.
        float frame[16];
        if (!spawn_ref_entity_frame_008f8680(trigger.target_id, frame)) return;
        target[0] = frame[12];
        target[1] = frame[13];
        target[2] = frame[14];
    } else if (trigger.target_point_valid_45) {
        target[0] = trigger.target_point[0];
        target[1] = trigger.target_point[1];
        target[2] = trigger.target_point[2];
    }
    ++summary_.proximity_trigger_checks;
    // 00750042 / 0075004E: FSUB float, FSTP float.
    const float dx = static_cast<float>(static_cast<double>(watched[0]) - target[0]);
    const float dz = static_cast<float>(static_cast<double>(watched[2]) - target[2]);
    // 0075005E..0075006C: (dx*dx + 0*0) + dz*dz, FSTP float. ASSUMED 24-bit precision
    // control (each FMUL/FADD rounds to float; docs/CC13_FPU_PRECISION.md): the thread
    // that runs wave-2 jobs is not established. Under 53 bits the sum rounds once
    // (bsp::gameplay_seg41_cc13::proximity_check_00750000); at most one ulp apart.
    const float dx2 = static_cast<float>(static_cast<double>(dx) * dx);
    const float dz2 = static_cast<float>(static_cast<double>(dz) * dz);
    const float d2 = static_cast<float>(static_cast<double>(dx2) + dz2);
    // 00750077 FCOMIP ST0 = +5Ch, ST1 = d2; JBE skips, so an unordered compare never fires.
    if (!(trigger.radius_sq_5c > d2)) return;
    // 0074FDD0.
    ++summary_.proximity_trigger_fires;
    const GameUnitRow* row = units_hooks_->unit_row(unit);
    log_.notef("  proximity trigger 00750000: \"%s\" %s%s%s -> %s() (packet cc13-fix10)",
        row != nullptr ? row->name.c_str() : "?", trigger.named_24 ? "\"" : "",
        trigger.named_24 ? trigger.name_1c.c_str() : "nil", trigger.named_24 ? "\"" : "",
        trigger.callback_28.c_str());
    fire_lua_trigger_0074fdd0(unit, trigger);
}

// 0074FDD0, shared by every Lua trigger kind (packet cc13-fix19 split it out of
// 00750000's body): the variant vector {00884A90 entity of (+4)->+28h, the name
// (00886A10) or nil (tag 6)}, 00887E50(host, 0, +28h, args, 0, -1), then vtable[10h]
// (0071C4A0). SUBSTITUTIONS (labelled, from packet cc13-fix10): 00887E50's queued and
// `game+1FE4h == 2` arms are not modelled: the call runs inline.
void GameMissionLuaHost::fire_lua_trigger_0074fdd0(std::size_t unit, ProximityTrigger& trigger) {
    const int top = ::lua_gettop(state_);
    lua_getfield(state_, LUA_GLOBALSINDEX, trigger.callback_28.c_str());
    if (lua_isfunction(state_, -1)) {
        if (!push_resolved_entity_by_id(state_, static_cast<int>(unit + 1))) {
            lua_pushnil(state_);
        }
        if (trigger.named_24) {
            ::lua_pushlstring(state_, trigger.name_1c.data(), trigger.name_1c.size());
        } else {
            lua_pushnil(state_);
        }
        if (::lua_pcall(state_, 2, 0, 0) != 0) {
            const char* message = lua_tolstring(state_, -1, nullptr);
            note_error(message != nullptr ? message : std::string("(no message)"));
        }
    } else {
        ++summary_.proximity_trigger_callback_missing;
    }
    ::lua_settop(state_, top);
    trigger.expired_10 = true;   // vtable[10h], 0071C4A0
    trigger.enabled_11 = false;
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
        if constexpr (kKillListenerHarmOnlyBound) {
            // Packet cc13-ver8: 00959475 / 007F3B19 CMP [unit+70h],1 / JNE past 009813A0,
            // and 00959460..0095946F the 1.0f clock gate (unit path only).
            auto cause_of = [&](std::size_t u) {
                const auto it = lua_kill_cause_.find(u);
                return it != lua_kill_cause_.end() ? it->second
                                                   : units_hooks_->kill_cause_00926d90(u);
            };
            int cause = cause_of(unit);
            bool squadron_slot = false;
            for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
                if (r.squadron_unit == bsp::kPlaneSquadronNoUnit || r.squadron_unit != unit) continue;
                // 007F3A2A: the squadron is killed with its last plane's +70h.
                squadron_slot = true;
                float last_time = death.second;
                std::size_t last = unit;
                for (const std::size_t m : r.member_units) {
                    for (const auto& other : deaths) {
                        if (other.first != m) continue;
                        if (other.second > last_time || (other.second == last_time && m > last)) {
                            last_time = other.second;
                            last = m;
                        }
                        break;
                    }
                }
                cause = cause_of(last);
                break;
            }
            // The world's active byte is set throughout a mission here, and the veto is
            // not modelled (labelled in kKillListenerHarmOnlyBound).
            bool reported = false;
            if (squadron_slot) {
                reported = bsp::squadron_death_reports_kill_007f3b10(cause, true, false);
            } else {
                bsp::UnitDeathInputs in;
                in.mission_clock = death.second;
                in.unit_party_70 = cause;   // +70h is the kill cause (00926E02)
                in.world_gate_4ac = true;
                reported = bsp::unit_death_00959450(in).reports_kill;
            }
            if (!reported) {
                if (cause != 1) {
                    ++summary_.listener_kill_not_harm;
                } else {
                    ++summary_.listener_kill_grace;
                }
                continue;
            }
        }
        ++summary_.listener_kill_deaths;
        const int victim = static_cast<int>(unit + 1);
        if constexpr (kKillListenerDeadFirstBound) {
            // Packet cc13-ver5: 0095945B (Dead, KillReason via 00929800) runs before
            // 00959507 (this channel). Same slot test as the Dead publisher: the self
            // table's Ptr must be this unit's id.
            if (push_resolved_entity_by_id(state_, victim)) {
                lua_getfield(state_, -1, "Ptr");
                const bool own_slot = lua_touserdata(state_, -1)
                    == reinterpret_cast<void*>(static_cast<std::uintptr_t>(victim));
                ::lua_settop(state_, ::lua_gettop(state_) - 1);
                if (own_slot) {
                    static const char* const kKillReasons[] = {
                        "none", "harm", "soft", "sell", "exitzone", "landed", "editor"};
                    const int cause = units_hooks_->kill_cause_00926d90(unit);
                    lua_pushboolean(state_, 1);
                    lua_setfield(state_, -2, "Dead");
                    ::lua_pushstring(state_, (cause >= 0 && cause < 7) ? kKillReasons[cause]
                                                                       : "harm");
                    lua_setfield(state_, -2, "KillReason");
                    ++summary_.listener_kill_dead_first;
                }
                ::lua_settop(state_, ::lua_gettop(state_) - 1);
            }
        }
        // Packet cc9_lua51_listener_args: the boxed lastAttacker (+2C8h, one based, 0 =
        // none) and +2D8h. The unit constructor 0077EED0 leaves +2C8h = 0 (0077EFDD)
        // and +2D8h = -1 (0077EFEF OR EAX,-1; 0077F012), and only 0077CE60's block
        // write changes them, so a victim no attributed hit reached boxes (nil, -1).
        // SUBSTITUTION (labelled): +2D8h otherwise is the host's credited slot from
        // its 0077CE60 rule, whose owning slot is the gunnery host's side stand-in.
        const GameGunneryHost* gunnery = units_hooks_->gunnery();
        int attacker = 0;
        int attacker_slot = -1;
        if ((kLuaKillAttackerBound || kLuaListenerCallbackArgsBound) && gunnery != nullptr) {
            attacker = static_cast<int>(gunnery->kill_last_attacker(unit));
            if (attacker != 0) {
                attacker_slot = gunnery->kill_credited_slot(unit);
                ++summary_.listener_kill_with_attacker;
            }
        }
        std::vector<std::string> callbacks;
        auto evaluate = [&](int attacker_id) {
        for (const ListenerEntry& entry : listeners_) {
            if (!listener_key_equal(entry.channel, "kill")) continue;
            if (!entry.entity_ids.empty()
                && std::find(entry.entity_ids.begin(), entry.entity_ids.end(), victim)
                    == entry.entity_ids.end()) {
                continue;   // 0096AD0D: the `entity` set rejects the victim
            }
            if (kLuaKillAttackerBound) {
                // 0096ACE0: +1Ch against the attacker, +2Ch against +2D8h.
                if (entry.last_attacker_set
                    && std::find(entry.last_attacker_ids.begin(), entry.last_attacker_ids.end(),
                           attacker_id) == entry.last_attacker_ids.end()) {
                    continue;
                }
                if (!entry.last_attacker_player_indices.empty()
                    && std::find(entry.last_attacker_player_indices.begin(),
                           entry.last_attacker_player_indices.end(), attacker_slot)
                        == entry.last_attacker_player_indices.end()) {
                    continue;
                }
                if (entry.attacker_filters_set) ++summary_.listener_kill_attacker_matched;
            } else if (entry.attacker_filters_set) {
                // SUBSTITUTION (labelled): the host's death row carries no
                // attacker entity, so a non-empty lastAttacker or
                // lastAttackerPlayerIndex set is not matched.
                ++summary_.listener_attacker_filtered;
                continue;
            }
            if (!entry.callback.empty()) callbacks.push_back(entry.callback);
        }
        };
        evaluate(attacker);
        if (kLuaKillAttackerBound && callbacks.empty() && attacker > 0) {
            // 00981AA4: nothing matched; 00981AC2..00981AE9: a plane attacker
            // (IsKindOf(0Fh)) is replaced by vtable[140h]() = plane+9D4h, its
            // squadron, and 00981C31 evaluates the channel again with it.
            // SUBSTITUTION (labelled): a plane is a member of a squadron registry
            // record; the wing-0 plane shares its squadron's slot, so it already
            // matched as the squadron in the first pass.
            const std::size_t shooter = static_cast<std::size_t>(attacker - 1);
            for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
                if (r.squadron_unit == bsp::kPlaneSquadronNoUnit || r.squadron_unit == shooter) continue;
                if (std::find(r.member_units.begin(), r.member_units.end(), shooter)
                        == r.member_units.end()) {
                    continue;
                }
                attacker = static_cast<int>(r.squadron_unit + 1);
                evaluate(attacker);
                ++summary_.listener_kill_second_passes;
                if (!callbacks.empty()) ++summary_.listener_kill_second_pass_fires;
                break;
            }
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
            const GameUnitRow* by = attacker > 0
                ? units_hooks_->unit_row(static_cast<std::size_t>(attacker - 1)) : nullptr;
            log_.notef("  kill listener 009813a0: victim \"%s\" attacker \"%s\" slot %d -> %s() "
                "(packet cc9_lua_listeners)", row != nullptr ? row->name.c_str() : "?",
                by != nullptr ? by->name.c_str() : "-", attacker_slot, name.c_str());
            int pushed = 0;
            if (kLuaListenerCallbackArgsBound) {
                // 009818E8..00981977: victim (00884A90), lastAttacker (00884AC0; id 0
                // boxes nil), +2D8h (00884A00).
                if (!push_resolved_entity_by_id(state_, victim)) lua_pushnil(state_);
                if (attacker <= 0 || !push_resolved_entity_by_id(state_, attacker)) {
                    lua_pushnil(state_);
                }
                ::lua_pushnumber(state_, static_cast<lua_Number>(attacker_slot));
                pushed = 3;
                ++summary_.listener_args_pushed;
            }
            if (::lua_pcall(state_, pushed, 0, 0) != 0) {
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
    if (bsp::kReconDetectionRecordsBound) {
        // Packet cc9_g51_recon_records: the records' own delegate calls, in pass
        // order. A unit record is not reset by 00807490 (that loop walks only the
        // class-18h squadron list), so it notifies on a net change only; the own
        // arm's 00805AF0(1.0f) notifies once, 0 -> 2. Supersedes the reset-cycle
        // and own-step substitutions below.
        if (!any_recon) return;
        for (const bsp::ReconLevelTransition& t : pass.transitions()) {
            if (t.unit >= count || gunnery->unit_dead(t.unit)) continue;   // 00980E50's live-unit gate
            fire_recon_listeners_00980e50(t.unit, static_cast<int>(t.old_level),
                                          static_cast<int>(t.new_level), t.party);
        }
        return;
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
            if (kForcedReconSyncNotifyBound && forced) {
                // The record's effective level is its +8h whatever the coverage.
                published = forced_level == bsp::ReconDetectionLevel::identified ? 2
                    : forced_level == bsp::ReconDetectionLevel::blip ? 1 : 0;
            }
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
                fire_recon_listeners_00980e50(u, transition.first, transition.second, party);
            }
        }
    }
}

void GameMissionLuaHost::fire_recon_listeners_00980e50(std::size_t u, int old, int level,
    int party) {
    {
        {
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
// Packet cc13-reconload: `apply` false is a dry run for the load census. It
// walks the same buckets and counts what would be published, but touches no
// Lua, no generation, no last-published content, no squadron category cache
// and no summary counter.
GameMissionLuaHost::ReconPublishCounts GameMissionLuaHost::publish_recon_slot_tables_00806b10(
    bool apply) {
    ReconPublishCounts counts;
    if (units_hooks_ == nullptr || state_ == nullptr) return counts;
    const GameGunneryHost* gunnery = units_hooks_->gunnery();
    if (gunnery == nullptr) return counts;
    const bsp::ReconSensorPassState& pass = gunnery->recon_sensor_pass_state();
    if (pass.passes == recon_publish_generation_) return counts;
    if (apply) {
        recon_publish_generation_ = pass.passes;
        ++summary_.recon_publish_passes;
    }

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
            if (apply) recon_squadron_category_.emplace(unit, category);
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
                    if (apply) ++summary_.recon_publish_excluded;
                    continue;
                }
                const int category = category_of(unit);
                if (category < 0 || category >= static_cast<int>(kCategories)) {
                    if (apply) ++summary_.recon_publish_no_category;
                    continue;
                }
                buckets[r][static_cast<std::size_t>(category)].push_back(
                    static_cast<int>(unit + 1));
            }
        }
        if (!any_triple) continue;   // no slot rebuilt for this party yet
        ++counts.slots;
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
            if (apply) ++summary_.recon_publish_unchanged;
            continue;
        }
        ++counts.published;
        counts.entries += content.size();
        if (!apply) continue;
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
    return counts;
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

namespace {
// Defined with the part 7b helpers below.
const bsp::PlaneSquadronHostRecord* luawire_squadron_record(std::size_t index);
}  // namespace

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
// clear is GameUnitsHost::unit_active (unit_flag_005d under kLuaAttackTargetDeadNilBound);
// an entity with no units-host slot answers nil.
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
    bool fire_arm = ship_row != nullptr;
    // Packet cc13-atkfix: the slot is a squadron registry record's fused slot,
    // the host's stand-in for a PlaneSquadronGen entity. Counted always, so a
    // pair can see the ON arm's input whichever way the switch is set.
    const bool squadron_standin = luawire_squadron_record(index) != nullptr;
    if (squadron_standin) ++summary_.attack_target_squadron_standin;
    if constexpr (kUnitGetAttackTargetDirectorKindCc13Bound) {
        // A squadron's director is the block 00D0BD98, whose +48h 0084D8F0 is
        // false for 2, so the image takes the command arm whatever row exists.
        // SUBSTITUTION, labelled (packet cc13-atkfix): the host has no squadron
        // slot. A squadron's Lua entity resolves to its wing-0 plane's slot,
        // which is kind 0Fh, never 18h, so the record test stands in for the
        // image's IsKindOf(18h). Wing planes 1.. and lone planes have no record
        // naming their slot and keep the fire arm, as a plane's director does.
        if (units->unit_is_kind_of(index, 0x18)) {
            fire_arm = false;
        } else if (squadron_standin) {
            fire_arm = false;
        }
    }
    if (fire_arm) {
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
    // kLuaAttackTargetDeadNilBound: the image's test is +5Dh clear (008A6F58), not +5Ch.
    const bool live = target_plus_one != 0 &&
        (kLuaAttackTargetDeadNilBound ? !units->unit_flag_005d(target_plus_one - 1)
                                      : units->unit_active(target_plus_one - 1));
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

// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 4). 0089F950
// SquadronForceGunnerMode(entity, flag), 0089F950-0089FAE4 RET: 0089FA4E 00888AA0
// on argument 0 (ESI), 0089FA80 00B66250 on argument 1, 0089FA9C MOV [ESI+379h],BL.
// No class or null test, no result. The squadron constructor 007F2C60 clears the
// byte (007F2D25). Readers: 0064A1E0 (HUD root update, the controlled unit when
// IsKindOf(18h)) and 00647080 (which keeps interface 23h while it is set).
// SUBSTITUTIONS (labelled): the byte is kept per units-host slot here rather than
// on the entity; an entity with no slot is counted unresolved (the image would
// write through whatever 00888AA0 returned).
int GameMissionLuaHost::run_squadron_force_gunner_mode_0089f950(lua_State* state,
    int argument_count) {
    ++summary_.force_gunner_calls;
    const bool flag = argument_count >= 2 && ::lua_toboolean(state, 2) != 0;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.force_gunner_unresolved;
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    if (force_gunner_379_.size() <= index) force_gunner_379_.resize(index + 1, 0);
    force_gunner_379_[index] = flag ? 1 : 0;
    if (flag) ++summary_.force_gunner_set;
    const GameUnitRow* row = units->unit_row(index);
    log_.notef("  SquadronForceGunnerMode 0089f950: \"%s\" +379h=%d (packet cc13_natives1)",
        row != nullptr ? row->name.c_str() : "?", flag ? 1 : 0);
    log_.implemented("MissionLuaNative::SquadronForceGunnerMode", "0089f950");
    return 0;
}

bool GameMissionLuaHost::squadron_force_gunner_mode_379(std::size_t unit_index) const {
    return unit_index < force_gunner_379_.size() && force_gunner_379_[unit_index] != 0;
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

// Packet cc9_lua_submarine_on_surface. 008942C0 GetSubmarineOnSurface(entity)
// (008942C0-00894436, RET): argument 0 through 00888AA0 (008943BF), then 00852820 with
// ECX = the entity and no null or class test (008943D6), and its byte pushed through
// 00B66450; one result. 00852820 (00852820-0085285B): y at +100h (after 00414DB0 when
// +C8h is clear) above (+1204h + +1200h) / 3.0, i.e. NOT 00852860.
// SUBSTITUTIONS (labelled): an entity with no units-host slot answers false (the image
// would dereference null); a slot whose depth bands were never seeded (any class that
// names no depth key, every surface ship) keeps 00E0B578's defaults (0, -20), as the
// gunnery and ship AI hosts do.
int GameMissionLuaHost::run_get_submarine_on_surface_008942c0(lua_State* state,
                                                             int argument_count) {
    (void)argument_count;
    ++summary_.sub_surface_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    bool on_surface = false;
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.sub_surface_unresolved;
    } else {
        const std::size_t index = static_cast<std::size_t>(id - 1);
        float x = 0.0f, y = 0.0f, z = 0.0f;
        units->unit_position_00fc(index, x, y, z);                     // +100h
        float band0 = 0.0f, band1 = -20.0f;                           // 00E0B578 defaults
        const bool seeded = units->submarine_band_y(index, 0, band0);  // +1200h
        units->submarine_band_y(index, 1, band1);                     // +1204h
        if (!seeded) ++summary_.sub_surface_unseeded;
        on_surface = !bsp::ship_ai_attackmove_altitude_gate_00852860(y, band1, band0);
    }
    if (on_surface) ++summary_.sub_surface_true;
    ::lua_pushboolean(state, on_surface ? 1 : 0);
    log_.implemented("MissionLuaNative::GetSubmarineOnSurface", "008942c0");
    return 1;
}

// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 2). 00899500
// TrulyDead(entity), 00899500-00899695 RET, ECX the Lua state: 008995EF
// 00B677E0 (argument 0), 00899609 00B67800 GetByName("Ptr", 00CFAD08), 00899618
// 00B662D0 ToUserdata and 0089964A 00B66450 with `pointer == null`; one result.
// There is no class test. The image nulls `Ptr` only in 00928C80 (the on-killed
// slot vtable[80h], reached from the kill list's drain), so a unit that is `Dead`
// but still in the world (a sinking hull) answers false.
// SUBSTITUTIONS (labelled): the host's self tables carry the entity id as light
// userdata and nothing in the host nulls it yet (the units host records 00928C80),
// so every entity table answers false until that hook exists; a non-table
// argument, where the image's GetByName has no table to index, answers true.
// CORRECTION (packet cc13-tranche2, 2026-10-28): the hook exists. Under
// kLuaTrulyDeadOnKilledBound (true) on_entity_killed_00928c80 below sets
// self.Ptr = lightuserdata(0) as 00928D04 does, so a killed entity answers true;
// "nothing in the host nulls it" holds only for the OFF arm.
int GameMissionLuaHost::run_truly_dead_00899500(lua_State* state, int argument_count) {
    ++summary_.truly_dead_calls;
    void* ptr = nullptr;
    if (argument_count >= 1 && ::lua_type(state, 1) == LUA_TTABLE) {
        ::lua_getfield(state, 1, "Ptr");
        ptr = ::lua_touserdata(state, -1);
        ::lua_settop(state, argument_count);
    }
    if (ptr == nullptr) ++summary_.truly_dead_true;
    ::lua_pushboolean(state, ptr == nullptr ? 1 : 0);
    log_.implemented("MissionLuaNative::TrulyDead", "00899500");
    return 1;
}

// Packet p16_truly_dead. 00928C80's Lua half: gated on the cached key (00928C9B, an
// attached entity), 00928D04 sets self.Ptr = lightuserdata(0). The entity id is the
// units-host index + 1 (kMissionLuaEntityKeyFormat).
void GameMissionLuaHost::on_entity_killed_00928c80(std::size_t unit_index) {
    // Packet cc13-fix19: both callers are the host's inline pass K (009274A1..0092751F),
    // which run step e (00925C40's observer notice, 009274DE) and step f (this
    // vtable[80h] hook, 0092751F) back to back with nothing between, so step e's notice
    // is queued here, first.
    if constexpr (kLuaProximityTriggersBound) target_gone_00925c40(unit_index);
    if constexpr (!kLuaTrulyDeadOnKilledBound) {
        static_cast<void>(unit_index);
    } else {
        lua_State* state = state_;
        if (state == nullptr) return;
        const int top = ::lua_gettop(state);
        if (push_resolved_entity_by_id(state, static_cast<int>(unit_index + 1))) {
            ::lua_pushlightuserdata(state, nullptr);
            ::lua_setfield(state, -2, "Ptr");
            ++summary_.truly_dead_ptr_nulled;
            if constexpr (kLuaLastPositionOnKilledBound) {
                // 00928D49..00928E7E: self.LastPosition = {x = +FCh, y = +100h, z = +104h}.
                float right[3], up[3], forward[3], at[3] = {0.0f, 0.0f, 0.0f};
                if (units_hooks_ != nullptr) {
                    units_hooks_->unit_pose(unit_index, right, up, forward, at);
                }
                ::lua_newtable(state);
                ::lua_pushnumber(state, at[0]);
                ::lua_setfield(state, -2, "x");
                ::lua_pushnumber(state, at[1]);
                ::lua_setfield(state, -2, "y");
                ::lua_pushnumber(state, at[2]);
                ::lua_setfield(state, -2, "z");
                ::lua_setfield(state, -2, "LastPosition");
                ++summary_.last_position_written;
            }
        }
        ::lua_settop(state, top);
    }
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

namespace {

// The twenty C strings at 00E08E58 (read from the PE), indexed by a projectile
// class's sub-type [[shot+4]+8]: the hit listener's attackType and 006E8250's table.
constexpr const char* kWeaponNameTable00e08e58[] = {
    "NONE", "BULLET", "MACHINEGUN", "AAMACHINEGUN", "ARTILLERY",
    "LIGHTARTILLERY", "MEDIUMARTILLERY", "HEAVYARTILLERY", "EXTRAPOLATED",
    "BOMB", "TORPEDO", "DEPTHCHARGE", "DUMMYTARGET", "DUMMYKAMIKAZEPLANE",
    "DUMMYSUBMARINE", "PARATROOPER", "FLAK", "KAMIKAZE", "ROCKET", "WATERMINE"};
constexpr int kWeaponNameTableCount00e08e58 =
    static_cast<int>(sizeof(kWeaponNameTable00e08e58) / sizeof(kWeaponNameTable00e08e58[0]));
static_assert(kWeaponNameTableCount00e08e58 == 0x14, "006E8250 walks twenty entries");

// 00988B09 MOV ECX,[EAX*4+00E08E58h] (and 009890DC for the squadron pass): the hit
// channel's attackType box for an ordnance kind; "" outside the table. Packet
// cc13-atkkind: shared by the callback's argument 4 and the attackType test.
const char* hit_attack_type_name_00e08e58(int kind) {
    return kind >= 0 && kind < kWeaponNameTableCount00e08e58 ? kWeaponNameTable00e08e58[kind]
                                                             : "";
}

// 006E8250 (src/cc13_seg33a.cpp weapon_name_index_006e8250) over the table above,
// whose entries are all non-null and non-empty: a null name matches nothing, any
// other name by _stricmp (00BF7FBF, case-insensitive); no match answers 0, the same as
// a match at index 0.
int weapon_name_index_006e8250(const char* name) {
    if (name == nullptr) return 0;
    for (int i = 0; i < kWeaponNameTableCount00e08e58; ++i) {
        if (::_stricmp(name, kWeaponNameTable00e08e58[i]) == 0) return i;
    }
    return 0;
}

}  // namespace

// Packet cc13-luawire, under kLuaHasFiredBound. 008BE7E0 HasFired(unit, weaponName,
// window), the sequence of bsp::lua_has_fired_008be7e0 (src/unit_weapons.cpp, checked
// against the listing) over this process: vtable[5Ch](2) on argument 0; its director,
// or for a plane (kind 0Fh) with none, the director of [unit+9D4h]; the stamp
// [director+1D0h+index*4] for the name's 006E8250 index; true when the stamp is above
// float(double([00F876A4]) - window) (FSUBR at 24-bit precision, FCOMIP/JA: a NaN
// answers false). One boolean is pushed on every path (008BE9FA).
// SUBSTITUTIONS (labelled):
//  * the director is a units-host slot: the unit's own for a unit that is not kind 0Fh
//    (00720180's two callers construct a ship's director and a squadron's command
//    block), and for a plane its squadron's slot in the plane-squadron registry
//    (+9D4h); a plane in no squadron answers false;
//  * the stamp [director+1D0h+index*4] is the latest gun+474h (GameGunRow
//    last_fire_seconds, written at each gun shot, 0072F830) over the guns of the
//    director's units whose class sub-type (bullet_sub_type, classDesc+8h) is the
//    index; with none, the 00720180 seed -1e10f. Not covered: a dual-purpose gun's
//    second ammunition, whose shot carries its own class (it stamps the gun's
//    sub-type here), and the rack drops 006E4D50 -> 006E55D2, which stamp too;
//  * [00F876A4] is this host's world clock (spawn_world_clock_), summed from the same
//    steps as the gunnery clock the gun+474h values are written with;
//  * an argument 0 with no units-host slot answers false and is counted, where the
//    image dereferences 00888AA0's answer without a test.
int GameMissionLuaHost::run_has_fired_008be7e0(lua_State* state, int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.has_fired_calls;
    GameUnitsHost* units = units_hooks_;
    GameGunneryHost* gunnery = units != nullptr ? units->gunnery() : nullptr;
    const int id = air_ops_entity_id(state);
    bool fired = false;
    constexpr std::size_t kNoDirector = static_cast<std::size_t>(-1);
    // vtable[114h], or for a plane [unit+9D4h]'s.
    const auto director_of = [&](std::size_t unit) -> std::size_t {
        if (!units->unit_is_kind_of(unit, bsp::kClassTestPlane)) return unit;
        const bsp::PlaneSquadronHostRecord* record =
            bsp::plane_squadron_registry().find_by_member_unit(unit);
        return record != nullptr && record->squadron_unit != bsp::kPlaneSquadronNoUnit
            ? record->squadron_unit : kNoDirector;
    };
    if (gunnery == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.has_fired_unresolved;
    } else {
        const std::size_t index = static_cast<std::size_t>(id - 1);
        const std::size_t director = units->unit_is_kind_of(index, bsp::kClassTestHasFiredEntity)
            ? director_of(index) : kNoDirector;
        if (director == kNoDirector) {
            ++summary_.has_fired_no_director;
        } else {
            const int name_index = weapon_name_index_006e8250(::lua_tostring(state, 2));
            float stamp = bsp::kDirectorFireStampUnset;
            for (const GameGunRow& gun : gunnery->guns()) {
                if (gun.last_fire_seconds < 0.0f || gun.bullet_sub_type != name_index) continue;
                if (director_of(gun.unit_index) != director) continue;
                if (gun.last_fire_seconds > stamp) stamp = gun.last_fire_seconds;
            }
            const float difference = static_cast<float>(
                static_cast<double>(spawn_world_clock_)
                - static_cast<double>(::lua_tonumber(state, 3)));
            fired = stamp > difference;
        }
    }
    if (fired) ++summary_.has_fired_true;
    ::lua_pushboolean(state, fired ? 1 : 0);
    log_.implemented("MissionLuaNative::HasFired", "008be7e0");
    return 1;
}

// Packet cc13-luawire, under kLuaAddWaterLoadBound. 00893380 AddWaterLoad(unit, side,
// amount), checked against the listing (src/cc13_left55_seg56a.cpp lua_add_water_load):
// argument 1 an integer (00B66290); the hull-local point x = Width ([class+A4h]) *
// (odd side: 1, even: -1) * 0.5, y = 0, z = Length ([class+A0h]) * (side < 2: 1,
// side < 4: 0, else -1) * 0.5 (00893510..00893557); argument 2 stored as a float;
// unless amount > 0 (FCOMIP/JBE, a NaN skips) and the unit is not kind 8, nothing.
// count = trunc(clamp(float(amount / 10.0), 0, 63)) (008935A6..00893603); a non-zero
// count builds message 90h (0080FF80) and routes it with flags 7 (0077C2A0). The
// receiver 008221A7 -> 0074F440 takes the point through the pose at unit+CCh
// (004142E0: ((x*m0 + y*m4) + z*m8) + m12 per lane) before 0074F090. Returns no value.
// SUBSTITUTIONS (labelled):
//  * the 90h receiver is GameUnitsHost::add_leak_0074f440 on the routing call itself
//    (the host's same-tick message delivery), with the point already in the world by
//    the units host's own pose, which add_leak_0074f440 expects;
//  * Width and Length are the VehicleClass keys for the unit's type
//    (vehicle_class_launch_keys, the keys 008206F0 reads off [unit+538h]);
//  * an argument 0 with no units-host slot is counted and skipped.
int GameMissionLuaHost::run_add_water_load_00893380(lua_State* state, int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.water_load_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.water_load_unresolved;
        log_.notef("  AddWaterLoad 00893380: entity id %d has no units-host slot (packet "
            "cc13-luawire)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    const GameUnitRow* row = units->unit_row(index);
    const int side = static_cast<int>(::lua_tonumber(state, 2));
    // 008934D2..008934E6 and 008934E9..00893506.
    const int x_sign = (side % 2 != 0) ? 1 : -1;
    const int z_sign = side < 2 ? 1 : (side < 4 ? 0 : -1);
    const GameUnitsHost::VehicleClassLaunchKeys keys =
        units->vehicle_class_launch_keys(row != nullptr ? row->type_id : -1);
    const float local_x = static_cast<float>(
        static_cast<double>(keys.width_00a4) * x_sign * 0.5);   // [00D7A280]
    const float local_z = static_cast<float>(
        static_cast<double>(keys.length_00a0) * z_sign * 0.5);
    const float amount = static_cast<float>(::lua_tonumber(state, 3));   // 0089356F
    if (!(amount > 0.0f) || units->unit_is_kind_of(index, 8)) {           // 0089358D, 008935A0
        ++summary_.water_load_skipped;
        log_.implemented("MissionLuaNative::AddWaterLoad", "00893380");
        return 0;
    }
    float scaled = static_cast<float>(static_cast<double>(amount) / 10.0);   // [00CE3DC0]
    if (0.0f > scaled) {
        scaled = 0.0f;
    } else if (static_cast<double>(scaled) > 63.0) {   // [00D099A0]
        scaled = 63.0f;                                // [00D099A8]
    }
    const auto count = static_cast<std::int32_t>(static_cast<std::int64_t>(scaled));
    bool applied = false;
    if (count != 0) {
        ++summary_.water_load_routed;
        float right[3], up[3], forward[3], origin[3];
        if (units->unit_pose(index, right, up, forward, origin)) {
            float world[3];
            for (int lane = 0; lane < 3; ++lane) {
                const float across = local_x * right[lane];
                const float along = local_z * forward[lane];
                const float lift = 0.0f * up[lane];
                world[lane] = ((across + lift) + along) + origin[lane];
            }
            applied = units->add_leak_0074f440(index, static_cast<std::uint32_t>(count), world);
            if (applied) ++summary_.water_load_applied;
        }
    }
    log_.notef("  AddWaterLoad 00893380: \"%s\" side=%d amount=%.1f count=%d local=(%.2f, 0, "
        "%.2f) applied=%d (packet cc13-luawire)", row != nullptr ? row->name.c_str() : "?",
        side, static_cast<double>(amount), count, static_cast<double>(local_x),
        static_cast<double>(local_z), applied ? 1 : 0);
    log_.implemented("MissionLuaNative::AddWaterLoad", "00893380");
    return 0;
}

// Packet cc13-luawire, under kLuaBreakShipBound. 008914F0 BreakShip(unit), checked
// against the listing (src/options_seg56_cc13.cpp lua_break_ship): argument 0 through
// 00888AA0; [unit+1018h] (0080E490) must answer +370h (0092BEA0) and +374h (0092BE90)
// non-zero; then 00814520: 004A5AA0 on [game+21D0h] when the class has breakup pieces
// (0080F9F0), and the tail jump to 00935D30, whose 11 stream-1 draws, cleared halves
// and 00926D90(1) kill are modelled here. Returns no value.
// 00935D30's draws (docs in src/cc13_segmix.cpp): front sx, sy, sz = U(-1, 1),
// a = U(0, 1.5), b = U(0, 1.5), d = U(10, 20); back sy, sz, a, b, d. Then, before the kill,
// 0092A240 (BSP_TickableEntityAttachFires) once per piece, front (00936905, ECX = EDI)
// then back (0093697F, ECX = ESI), unconditionally. With w = float([[piece+348h]+A0h]
// x 0.2) (00CE6648, double) and n = trunc(clamp(float(w / 10.0), 1, 2)) (00CE3DC0,
// 00D7A24C, 00CE3958), it draws 00BD2F10(1, -w, w) once per puff (0092A34A), only when
// [[piece+348h]+55Ch] (the class `DeathEfx`, 00831BE8) and [[00E188A8]+19FCh] (the game
// view node) are non-null. piece+348h is the owner unit's class [unit+538h] (0092B185).
// Only the draws are taken; the offsets and fires they make are presentation. The
// gunnery host's death 98h records the same eleven and likely lacks these puff draws
// (a pre-existing record, not changed here).
// SUBSTITUTIONS (labelled):
//  * the halves are the units host's front and back nodes
//    (ship_death_message_class().front_back) less this host's cleared mark, which a
//    BreakShip here sets; a breakup the gunnery host's death 98h made is not seen, so a
//    BreakShip on that wreck takes the draws again (its kill is a no-op on the dead
//    unit); a non-ship answers no halves;
//  * the draws are GameGunneryHost::death_mode_draw_00bd2f10(1, ...), the shared
//    stream-1 generator in call order as in the image (keyed death_mode under the
//    BSP_GUNNERY_RNG_STREAMS=1 measurement option, not ship_break_in_two);
//  * 004A5AA0, the pieces, the re-parented guns, the effect and the shape masks are
//    presentation and are recorded, not modelled;
//  * 0092A240's class is the unit's VehicleClass row: a non-zero `DeathEfx` id stands
//    for a resolved +55Ch handle, and the game view node is taken as present (it is
//    set in any rendered mission);
//  * the kill is the host's 00926D90 funnel with cause 1, as the Kill runner uses it;
//  * an argument 0 with no units-host slot is counted and skipped.
int GameMissionLuaHost::run_break_ship_008914f0(lua_State* state, int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.break_ship_calls;
    GameUnitsHost* units = units_hooks_;
    GameGunneryHost* gunnery = units != nullptr ? units->gunnery() : nullptr;
    const int id = air_ops_entity_id(state);
    if (gunnery == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.break_ship_unresolved;
        log_.notef("  BreakShip 008914f0: entity id %d has no units-host slot (packet "
            "cc13-luawire)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    const GameUnitRow* row = units->unit_row(index);
    const bool halves = units->ship_death_message_class(index).front_back
        && break_ship_cleared_.count(index) == 0;
    if (!halves) {
        ++summary_.break_ship_no_halves;
    } else {
        // 00814529..00814547: 004A5AA0 when 0080F9F0 answers (the class has breakup
        // pieces); the registry is presentation and the test is not modelled.
        log_.unimplemented("BreakShip::breakup_registry_004a5aa0", "004a5aa0");
        for (int piece = 0; piece < 2; ++piece) {
            for (int axis = piece == 0 ? 0 : 1; axis < 3; ++axis)
                static_cast<void>(gunnery->death_mode_draw_00bd2f10(1, index, -1.0f, 1.0f));
            static_cast<void>(gunnery->death_mode_draw_00bd2f10(1, index, 0.0f, 1.5f));
            static_cast<void>(gunnery->death_mode_draw_00bd2f10(1, index, 0.0f, 1.5f));
            static_cast<void>(gunnery->death_mode_draw_00bd2f10(1, index, 10.0f, 20.0f));
        }
        // 00936905 / 0093697F: 0092A240 on the front piece, then the back piece.
        const GameVehicleClassRow cls = read_vehicle_class_row(row != nullptr ? row->type_id : -1);
        if (cls.found && cls.death_efx_id != 0) {
            const float w = static_cast<float>(static_cast<double>(cls.length) * 0.2);  // 0092A2AD
            float puffs = static_cast<float>(static_cast<double>(w) / 10.0);           // 0092A2CB
            if (1.0f > puffs) {
                puffs = 1.0f;                                                       // 0092A2E7
            } else if (puffs > 2.0f) {
                puffs = 2.0f;                                                       // 0092A6BF
            }
            const int n = static_cast<int>(puffs);                                  // 0092A307 FISTP
            for (int piece = 0; piece < 2; ++piece) {
                for (int puff = 0; puff < n; ++puff) {
                    static_cast<void>(gunnery->death_mode_draw_00bd2f10(1, index, -0.0f - w, w));
                    ++summary_.break_ship_puff_draws;
                }
            }
        }
        log_.unimplemented("BreakShip::break_in_two_pieces_00935d30", "00935d30");
        break_ship_cleared_.insert(index);   // 00935D30 step 8: +370h / +374h = 0
        ++summary_.break_ship_broken;
        // 00935D30 step 8: 00926D90(1), which returns when +5Fh is already set.
        if (!gunnery->unit_dead(index)) {
            if constexpr (kKillListenerHarmOnlyBound) lua_kill_cause_[index] = 1;
            gunnery->kill_unit_00926d90(index, 1);
            if constexpr (kLuaProximityTriggersBound) target_gone_00925c40(index);
        }
    }
    log_.notef("  BreakShip 008914f0: \"%s\" halves=%d (packet cc13-luawire)",
        row != nullptr ? row->name.c_str() : "?", halves ? 1 : 0);
    log_.implemented("MissionLuaNative::BreakShip", "008914f0");
    return 0;
}

// Packet cc13-luawire, under kLuaUnitSetPlayerCommandsEnabledBound. 008A6C30
// UnitSetPlayerCommandsEnabled(unit, mask), checked against the listing
// (src/cc13_left55_seg56a.cpp lua_unit_set_player_commands_enabled): argument 0 through
// 00888AA0, its director vtable[114h] (fetched before the integer), argument 1 as an
// integer, then 0071DD30(director, mask), which builds message 5Ah sub-kind 2 and routes
// it with flags 7 (0077C2A0) to [director+34h]; 0071C1E0's arm 2 stores the mask at
// director+1CCh. Returns no value. No RNG.
// Readers in the image: 0071C060 (BSP_Unit_DirectorFlagsAllow; its 16 callers are the
// HUD unit pick screen 00527260 / 00526720 and the order screen 005F9840 / 005FB080)
// and the network snapshot 00721280; 0071BD30 and the getter 00720E10 have no
// references. No host code reads the store, so ON changes nothing but the record line
// and this host's map.
// SUBSTITUTIONS (labelled):
//  * the director is a units-host slot: the unit itself, or for a plane (kind 0Fh) its
//    registry squadron slot. Unlike HasFired's runner there is no kind-02h gate first,
//    because 008A6C30 has no class test: the entity goes straight to vtable[114h];
//  * the message is applied at the call, not on its routed delivery;
//  * a unit with no slot or no director is counted and skipped, where the image would
//    send through a null director.
int GameMissionLuaHost::run_unit_set_player_commands_enabled_008a6c30(lua_State* state,
    int argument_count) {
    ++summary_.player_commands_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const std::int32_t mask = argument_count >= 2
        ? static_cast<std::int32_t>(::lua_tointeger(state, 2)) : 0;
    std::size_t director = static_cast<std::size_t>(-1);
    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
        const std::size_t index = static_cast<std::size_t>(id - 1);
        director = index;
        if (units->unit_is_kind_of(index, bsp::kClassTestPlane)) {
            const bsp::PlaneSquadronHostRecord* record =
                bsp::plane_squadron_registry().find_by_member_unit(index);
            director = record != nullptr && record->squadron_unit != bsp::kPlaneSquadronNoUnit
                ? record->squadron_unit : static_cast<std::size_t>(-1);
        }
    }
    if (director == static_cast<std::size_t>(-1)) {
        ++summary_.player_commands_unresolved;
        log_.notef("  UnitSetPlayerCommandsEnabled 008a6c30: entity id %d has no director "
            "slot (packet cc13-luawire)", id);
        return 0;
    }
    director_player_commands_1cc_[director] = mask;   // 0071C1E0 arm 2: director+1CCh
    ++summary_.player_commands_stored;
    const GameUnitRow* row = units->unit_row(director);
    log_.notef("  UnitSetPlayerCommandsEnabled 008a6c30: director \"%s\" +1CCh = 0x%X "
        "(packet cc13-luawire)", row != nullptr ? row->name.c_str() : "?",
        static_cast<unsigned>(mask));
    log_.implemented("MissionLuaNative::UnitSetPlayerCommandsEnabled", "008a6c30");
    return 0;
}

namespace {

// Part 7b. The registry record whose fused slot is `index`: the host's stand-in
// for the image's IsKindOf(18h) (PlaneSquadronGen) test on the entity.
const bsp::PlaneSquadronHostRecord* luawire_squadron_record(std::size_t index) {
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == index) return &r;
    }
    return nullptr;
}

}  // namespace

// Packet cc13-luawire 7b, under kLuaSetVisibilityBound. 008A13D0 SetVisibility(entity,
// visible), checked against the listing (src/engine_seg56_cc13.cpp lua_set_visibility):
// argument 1 as a boolean, argument 0 through 00888D20. A squadron (vtable[5Ch](18h)):
// for i below [+3CCh], member = i <= 4 ? [+3D0h+4i] : null, then 00951F40(visible) on
// it (008A154D). Otherwise a kind-5 entity (any unit): 00951F40 on itself (008A1578).
// 00951F40 attaches (1) or detaches (0) part +4A4h's spatial node, which is the units
// host's plane_hit_node_detached. Returns no value. No RNG.
// SUBSTITUTIONS (labelled):
//  * the squadron test is a registry record whose fused slot is the entity, and its
//    members are the record's member_units that are still active, standing in for the
//    compacted +3D0h array (as SquadronSetSpeed's runner);
//  * GUARDED, not in the image: a sixth or later member is null there and 00951F40 is
//    called on it with no null test, which crashes. Here it is counted and logged and
//    nothing is called;
//  * an entity with no slot, or neither a squadron nor a unit, is counted unresolved.
int GameMissionLuaHost::run_set_visibility_008a13d0(lua_State* state, int argument_count) {
    ++summary_.visibility_calls;
    const bool visible = argument_count >= 2 && ::lua_toboolean(state, 2) != 0;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.visibility_unresolved;
        log_.notef("  SetVisibility 008a13d0: entity id %d has no units-host slot (packet "
            "cc13-luawire 7b)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    const GameUnitRow* row = units->unit_row(index);
    const char* name = row != nullptr ? row->name.c_str() : "?";
    std::size_t planes = 0;
    if (const bsp::PlaneSquadronHostRecord* record = luawire_squadron_record(index)) {
        std::size_t live = 0;
        for (std::size_t member : record->member_units) {
            if (member == bsp::kPlaneSquadronNoUnit || !units->unit_active(member)) continue;
            if (live++ >= 5) {
                // 008A1530: i > 4 gives a null member and 00951F40 dereferences it.
                ++summary_.visibility_null_members;
                log_.notef("  SetVisibility 008a13d0: \"%s\" member %zu is past +3D0h's five "
                    "slots; the image calls 00951F40 on null here (crash), GUARDED and "
                    "skipped (packet cc13-luawire 7b)", name, live - 1);
                continue;
            }
            if (units->set_plane_visible_00951f40(member, visible)) ++planes;
        }
    } else if (units->unit_is_kind_of(index, 0x05)) {
        if (units->set_plane_visible_00951f40(index, visible)) ++planes;
    } else {
        ++summary_.visibility_unresolved;
        log_.notef("  SetVisibility 008a13d0: \"%s\" is neither a squadron nor a unit "
            "(packet cc13-luawire 7b)", name);
        return 0;
    }
    summary_.visibility_planes += planes;
    log_.notef("  SetVisibility 008a13d0: \"%s\" visible=%d planes=%zu (packet cc13-luawire "
        "7b)", name, visible ? 1 : 0, planes);
    log_.implemented("MissionLuaNative::SetVisibility", "008a13d0");
    return 0;
}

// Packet cc13-luawire 7b, under kLuaSquadronEnableVehicleAvoidanceBound. 008A0000
// SquadronEnableVehicleAvoidance(entity, on), checked against the listing
// (src/options_seg56_cc13.cpp lua_squadron_enable_vehicle_avoidance): argument 0 through
// 00888AA0, argument 1 as a boolean, then [+3A4h] = on ? v | 2 : v & 0x000000FD. No class
// test, no result, no RNG. The reader is the pilot gate 009A1959-009A1969 (the units
// host's kLuaSquadronEnableVehicleAvoidanceBound arm). The second gate 009A1974-009A1987
// (CL = 1) also tests the squadron +2FCh and +3A4h bit 0, which the host does not read
// yet; a sibling native that writes bit 0 would need it.
// SUBSTITUTIONS (labelled): the mask is kept per units-host slot (the image writes the
// entity whatever its class; only a squadron's is ever read); an entity with no slot is
// counted unresolved. The reader takes squadron_slot_of(plane), which falls back to the
// plane's own slot where the image's null +2FCh passes the test; a plane with no
// squadron is unreachable in the census (every BSM09 caller passes a squadron).
int GameMissionLuaHost::run_squadron_enable_vehicle_avoidance_008a0000(lua_State* state,
    int argument_count) {
    ++summary_.avoidance_calls;
    const bool on = argument_count >= 2 && ::lua_toboolean(state, 2) != 0;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const bool stored = units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count()
        && units->set_squadron_vehicle_avoidance_008a0000(static_cast<std::size_t>(id - 1), on);
    if (!stored) {
        ++summary_.avoidance_unresolved;
        log_.notef("  SquadronEnableVehicleAvoidance 008a0000: entity id %d has no units-host "
            "slot (packet cc13-luawire 7b)", id);
        return 0;
    }
    const GameUnitRow* row = units->unit_row(static_cast<std::size_t>(id - 1));
    log_.notef("  SquadronEnableVehicleAvoidance 008a0000: \"%s\" on=%d (packet cc13-luawire "
        "7b)", row != nullptr ? row->name.c_str() : "?", on ? 1 : 0);
    log_.implemented("MissionLuaNative::SquadronEnableVehicleAvoidance", "008a0000");
    return 0;
}

// Packet cc13-luawire 7b, under kLuaSquadronSetBaseUnsupportedBound. 0089FAF0
// SquadronSetBaseUnsupported(squadron, base), checked against the listing
// (src/cc13_left55_seg56a.cpp lua_squadron_set_base_unsupported): both arguments through
// 00888AA0, the block is base+72Ch for class 45h, else base+1188h (the inlined 006BCD20
// without its null case), then 006CA730 pushes the squadron onto the block's std::list
// at +B4h. No result, no RNG. The readers are the units host's 006C4790 sites (007F1748,
// 0099A404, 009B3537).
// SUBSTITUTIONS (labelled): the list is a set of (deck name, squadron record name) pairs
// in the units host, so a repeated push is one entry (006C4790 answers any match either
// way); a squadron with no registry record or a base with no slot is counted unresolved,
// where the image would push whatever 00888AA0 returned.
int GameMissionLuaHost::run_squadron_set_base_unsupported_0089faf0(lua_State* state,
    int argument_count) {
    ++summary_.base_unsupported_calls;
    GameUnitsHost* units = units_hooks_;
    const int squadron_id = air_ops_entity_id(state);
    const int base_id = argument_count >= 2 ? listener_entity_id_at(state, 2) : 0;
    const bool valid = units != nullptr && squadron_id > 0 && base_id > 0
        && static_cast<std::size_t>(squadron_id) <= units->count()
        && static_cast<std::size_t>(base_id) <= units->count();
    if (!valid || !units->add_base_unsupported_006ca730(static_cast<std::size_t>(squadron_id - 1),
            static_cast<std::size_t>(base_id - 1))) {
        ++summary_.base_unsupported_unresolved;
        log_.notef("  SquadronSetBaseUnsupported 0089faf0: squadron id %d base id %d has no "
            "squadron record or base slot (packet cc13-luawire 7b)", squadron_id, base_id);
        return 0;
    }
    ++summary_.base_unsupported_added;
    const GameUnitRow* sq = units->unit_row(static_cast<std::size_t>(squadron_id - 1));
    const GameUnitRow* base = units->unit_row(static_cast<std::size_t>(base_id - 1));
    log_.notef("  SquadronSetBaseUnsupported 0089faf0: \"%s\" on \"%s\"'s +B4h list (packet "
        "cc13-luawire 7b)", sq != nullptr ? sq->name.c_str() : "?",
        base != nullptr ? base->name.c_str() : "?");
    log_.implemented("MissionLuaNative::SquadronSetBaseUnsupported", "0089faf0");
    return 0;
}

// Packet cc13-luawire 7b, under kLuaSetCheatTurboBound. 008A62A0 SetCheatTurbo(entity,
// value), checked against the listing (src/cc13_left55_seg56a.cpp lua_set_cheat_turbo):
// argument 0, then argument 1 as a number stored as float (FSTP 008A63D5). A squadron
// (18h) writes +340h on members 0..4 and stops at the first null slot, without reading
// the member count (008A6406-008A641F); otherwise a kind-5 entity gets +340h itself.
// No result, no RNG. The readers are the units host's kLuaSetCheatTurboBound sites
// (0099D479's dtScale, 1/dtScale and dt/dtScale; 009FBB20). The dt slot [ESP+6Ch] is
// also read at 0099DAC4 and 0099DBBF; those are taken as covered by tin.pending, not
// traced. ASSUMPTION, labelled: no constructor writes +340h, so the host's 0 takes the
// plane allocation as zeroed.
// SUBSTITUTIONS (labelled): the squadron test and members as SetVisibility's runner
// (active member_units, at most five); an entity with no slot, or neither a squadron nor
// a unit, is counted unresolved.
int GameMissionLuaHost::run_set_cheat_turbo_008a62a0(lua_State* state, int argument_count) {
    ++summary_.cheat_turbo_calls;
    const float value = argument_count >= 2 ? static_cast<float>(::lua_tonumber(state, 2)) : 0.0f;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.cheat_turbo_unresolved;
        log_.notef("  SetCheatTurbo 008a62a0: entity id %d has no units-host slot (packet "
            "cc13-luawire 7b)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    const GameUnitRow* row = units->unit_row(index);
    const char* name = row != nullptr ? row->name.c_str() : "?";
    std::size_t planes = 0;
    if (const bsp::PlaneSquadronHostRecord* record = luawire_squadron_record(index)) {
        std::size_t live = 0;
        for (std::size_t member : record->member_units) {
            if (member == bsp::kPlaneSquadronNoUnit || !units->unit_active(member)) continue;
            if (live++ >= 5) break;  // 008A641F: slots 0..4 only
            if (units->set_cheat_turbo_340(member, value)) ++planes;
        }
    } else if (units->unit_is_kind_of(index, 0x05)) {
        if (units->set_cheat_turbo_340(index, value)) ++planes;
    } else {
        ++summary_.cheat_turbo_unresolved;
        log_.notef("  SetCheatTurbo 008a62a0: \"%s\" is neither a squadron nor a unit (packet "
            "cc13-luawire 7b)", name);
        return 0;
    }
    summary_.cheat_turbo_planes += planes;
    log_.notef("  SetCheatTurbo 008a62a0: \"%s\" +340h=%g planes=%zu (packet cc13-luawire "
        "7b)", name, static_cast<double>(value), planes);
    log_.implemented("MissionLuaNative::SetCheatTurbo", "008a62a0");
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

// Packet cc9_lua_add_air_base_stock. 00896A90 AddAirBaseStock / AddAirBasePlanes(entity,
// class, count): 00888AA0 on argument 0, 006BCD20 with DL = 0 (00896BAA..00896BAE), the
// class 00964790 over argument 1 (00896BD8), 0095BA60(class+70h, entity+54h) (the party
// preload, 00896BF5), 00964790 again, then 006CA770(class, argument 2) on the block at
// 00896C54 (argument index EBX = 2). Returns no value.
// SUBSTITUTION (labelled, cc9-lua59 after docs/CC13_SEG56.md): the 0095BA60 party preload
// mark at 00896BF5 is not applied. It sets the class's party byte in the vehicle-class
// preload bitmap (byte[base + class_index * 3 + party] = 1, include/bsp/vehicle_class.hpp),
// which the image reads when it preloads class resources; this host loads no class
// resources from that bitmap, so the mark has nothing to feed. It is recorded per call.
// SUBSTITUTIONS (labelled): the block is the deck registry's entry for the entity and the
// class is its id (the deck's stock list holds ids); an entity with no deck is counted
// unresolved, where the image calls 006CA770 on a null block. 006CA770's replication
// (0077C7B0, no peer offline) and its notice 00984EB0 (contract: unread) are records.
int GameMissionLuaHost::run_add_air_base_stock_00896a90(lua_State* state, int argument_count) {
    ++summary_.stock_add_calls;
    const int class_id = argument_count >= 2 ? static_cast<int>(::lua_tonumber(state, 2)) : 0;
    const int added = argument_count >= 3 ? static_cast<int>(::lua_tonumber(state, 3)) : 0;
    const int id = air_ops_entity_id(state);
    bsp::AirOpsDeck* deck = id > 0 ? bsp::air_ops_decks().find_mutable_by_entity_id(id) : nullptr;
    if (deck == nullptr) {
        ++summary_.stock_add_unresolved;
        log_.notef("  AddAirBaseStock 00896a90: entity id %d has no deck (packet "
            "cc9_lua_add_air_base_stock)", id);
        return 0;
    }
    log_.unimplemented("VehicleClass::mark_party_preload", "0095ba60");   // 00896BF5
    bsp::AirOpsStockEntry created{};
    const bsp::AirOpsStockAddResult result = bsp::air_base_stock_add_006ca770(
        deck->stock.data(), static_cast<int>(deck->stock.size()),
        static_cast<int>(deck->stock.size()) + 1, static_cast<std::uint32_t>(class_id), added,
        &created);
    if (result.created_entry) {
        deck->stock.push_back(created);
        ++summary_.stock_add_created;
    }
    log_.unimplemented("AirOps::stock_notice", "00984eb0");
    log_.notef("  AddAirBaseStock 00896a90: entity id %d class %d +%d -> %d%s (packet "
        "cc9_lua_add_air_base_stock)", id, class_id, added, result.entry_count,
        result.created_entry ? " (new entry)" : "");
    log_.implemented("MissionLuaNative::AddAirBaseStock", "00896a90");
    return 0;
}

// Packet cc13-fix5 (docs/CC13_FIX_5.md section 1). 008975B0
// RemoveAllAirBasePlanes(base), lua_CFunction, no result (008976DA 00B66400):
// 008976AF 00888AA0 on argument 0, 008976CA 006BCD20 with DL = 0 (008976C6), and
// 008976D1 006C8840 on the block; the sequence is air_base_remove_all_stock_006c8840
// (src/script_natives_fix5_cc13.cpp).
// SUBSTITUTIONS (labelled), as for AddAirBaseStock: the block is the deck registry's
// entry for the entity, and an entity with no deck does nothing (the image calls
// 006C8840 on a null block). The live counts 006BF230 subtracts for a launched slot
// are the orders host's air_ops_squadron_plane_count, the reader the deck tick uses.
// The slot clear's GUI notify (00696350), its slot message to the peers (006BD520,
// 0077C7B0, no peer offline) and the stock notice (00984EB0) are records.
int GameMissionLuaHost::run_remove_all_air_base_planes_008975b0(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    const int id = air_ops_entity_id(state);
    bsp::AirOpsDeck* deck = id > 0 ? bsp::air_ops_decks().find_mutable_by_entity_id(id) : nullptr;
    if (deck == nullptr) {
        log_.notef("  RemoveAllAirBasePlanes 008975b0: entity id %d has no deck (packet "
            "cc13-fix5)", id);
        return 0;
    }
    std::vector<std::int32_t> live(deck->slots.size(), 0);
    for (std::size_t i = 0; i < deck->slots.size(); ++i) {
        const std::uint32_t squadron = deck->slots[i].launched_squadron;
        if (squadron != 0u && script_orders_ != nullptr) {
            live[i] = script_orders_->air_ops_squadron_plane_count(squadron);
        }
    }
    std::string before;
    for (const bsp::AirOpsStockEntry& e : deck->stock) {
        char item[32];
        std::snprintf(item, sizeof(item), " %u:%d", e.vehicle_class, e.count);
        before += item;
    }
    const bsp::AirBaseRemoveAllStockResult result =
        bsp::air_base_remove_all_stock_006c8840(*deck, live.data(), nullptr);
    if (result.slots_cleared != 0u) {
        log_.unimplemented("AirOps::slot_cleared_notify_and_message", "006bd520");
    }
    if (result.entries != 0u) log_.unimplemented("AirOps::stock_notice", "00984eb0");
    log_.notef("  RemoveAllAirBasePlanes 008975b0: entity id %d stock [%s ] removed=%d "
        "slots_cleared=%zu (packet cc13-fix5)", id, before.c_str(), result.planes_removed,
        result.slots_cleared);
    log_.implemented("MissionLuaNative::RemoveAllAirBasePlanes", "008975b0");
    return 0;
}

// Packet cc13-fix5 (docs/CC13_FIX_5.md section 3). 0089AC30 StartLanding2(unit),
// lua_CFunction: 0089AD2F 00888AA0 on argument 0 (no kind test), 0089AD48..0089AD50
// CALL [EDX+238h] on it, 0089AD57 00B664B0 pushes EAX as an integer; one result.
// Slot 238h holds 0074A4C0 only in the MLandingShip vtable (00CFFC68 is its only
// .rdata copy) and 008206F0 in the eight other ship vtables (00CF92E8, 00CFA9B0,
// 00CFB970, 00CFC608, 00D01868, 00D098B0, 00D0C1B8, 00D0C880).
// SUBSTITUTIONS (labelled): the two bodies are the ship AI host's models, as
// 00A11B80's 94h routing uses them; a non-ship argument (its slot 238h is another
// class's virtual, unread) and an unresolved one push 0 and are counted.
int GameMissionLuaHost::run_start_landing2_0089ac30(lua_State* state, int argument_count) {
    static_cast<void>(argument_count);
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    int answer = 0;
    const char* path = "unresolved";
    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
        const std::size_t index = static_cast<std::size_t>(id - 1);
        GameShipAiHost* ship_ai = units->ship_ai();
        if (ship_ai != nullptr && units->unit_is_kind_of(index, 0x0c)) {
            answer = ship_ai->landing_ship_request_landing_0074a4c0(index);
            path = "0074a4c0";
        } else if (ship_ai != nullptr && units->unit_is_kind_of(index, 0x06)) {
            answer = ship_ai->transport_launch_craft_008206f0(index);
            path = "008206f0";
        } else {
            path = "not a ship";
            log_.unimplemented("StartLanding2::non_ship_vtable_238", "0089ad50");
        }
    }
    ::lua_pushinteger(state, answer);
    log_.notef("  StartLanding2 0089ac30: entity id %d -> %s answer=%d (packet cc13-fix5)", id,
        path, answer);
    log_.implemented("MissionLuaNative::StartLanding2", "0089ac30");
    return 1;
}

// Packet cc13-fix5 (docs/CC13_FIX_5.md section 4). 0089CFE0 GetSquadronPlane(squadron, i),
// lua_CFunction: 00888AA0 on argument 0 (EDI), 0089D111 00B66290 argument 1 as an
// integer (ESI); 0089D128 JL and 0089D130 CMP [EDI+3CCh] JGE skip to the result count
// with nothing pushed; 0089D13C CMP ESI,5 JGE and a null [EDI+ESI*4+3D0h] push nil;
// else thisTable[member+174h] is pushed. Results: 0 or 1.
// SUBSTITUTIONS (labelled): the squadron is the registry record whose members hold the
// argument's unit, or held it when it died (find_by_member_or_departed_unit, the
// object the image keeps after its lead dies); +3D0h/+3CCh are its live members in
// order; a member's +174h is its unit index plus one, the `thisTable` key. An argument
// with no record pushes nothing (the image reads a non-squadron's +3CCh).
int GameMissionLuaHost::run_get_squadron_plane_0089cfe0(lua_State* state, int argument_count) {
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const int i = argument_count >= 2 ? static_cast<int>(::lua_tonumber(state, 2)) : 0;
    if (script_orders_ != nullptr) script_orders_->resolve_plane_squadron_members();
    const bsp::PlaneSquadronHostRecord* record = nullptr;
    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
        record = bsp::plane_squadron_registry().find_by_member_or_departed_unit(
            static_cast<std::size_t>(id - 1));
    }
    std::vector<std::size_t> live;
    if (record != nullptr) {
        for (const std::size_t member : record->member_units) {
            if (member != bsp::kPlaneSquadronNoUnit && member < units->count()) live.push_back(member);
        }
    }
    int results = 0;
    const char* answer = "nothing";
    if (record != nullptr && i >= 0 && i < static_cast<int>(live.size())) {
        results = 1;
        answer = "nil";
        if (i >= 5 || !push_resolved_entity_by_id(state, static_cast<int>(live[i]) + 1)) {
            ::lua_pushnil(state);
        } else {
            answer = "plane";
        }
    }
    log_.notef("  GetSquadronPlane 0089cfe0: entity id %d index %d of %zu -> %s (packet "
        "cc13-fix5)", id, i, live.size(), answer);
    log_.implemented("MissionLuaNative::GetSquadronPlane", "0089cfe0");
    return results;
}

// Packet cc13-fix5 (docs/CC13_FIX_5.md section 5). 0089A020 DisbandFormation(unit),
// lua_CFunction, no result: 0089A11F 00888AA0 on argument 0, 0089A138 0077CA60 on it.
// 0077CA60: nothing unless [unit+284h]; a snapshot of the group's members by
// 0070D060 over +4F8h, skipping nulls and the leader [group+14h], with the leader
// appended last; then for each one whose +284h is still set, the slot counter bump
// for +1ACh < 8 (00905300) and 77h with a null target through 0077C2A0 (class 7),
// whose receiver is 0077BD70's leave, the one LeaveFormation's 0077C980 sends.
// SUBSTITUTIONS (labelled), as in run_leave_formation_00899eb0: each leave runs at the
// call instead of through the session queue (unless cc13-jm08's switch posts them,
// below), in the snapshot's order; the member
// order is the units host's index order (the +4F8h array order is not carried); the
// slot counter is not modelled.
int GameMissionLuaHost::run_disband_formation_0089a020(lua_State* state, int argument_count) {
    static_cast<void>(argument_count);
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const bool valid = units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count();
    const std::int32_t group = valid
        ? units->unit_formation_group_0284(static_cast<std::size_t>(id - 1)) : -1;
    std::size_t left = 0;
    std::size_t posted = 0;
    if (group >= 0) {
        const std::size_t leader = units->formation_leader_0014(group);
        std::vector<std::size_t> snapshot;
        for (std::size_t u = 0; u < units->count(); ++u) {
            if (u != leader && units->unit_formation_group_0284(u) == group) snapshot.push_back(u);
        }
        if (leader < units->count()) snapshot.push_back(leader);
        // Packet cc13-jm08 (kDeadLeaderDirectorStateCopyBound): when any member
        // meets a posted 0071C150 (its own, or the group leader's), every leave is
        // posted, in snapshot order, behind the kill's 77h and the successor's 5Dh,
        // as 0077C2A0 queues them in the image; they run at the next pump. The
        // +284h test is the image's, at the post. Otherwise unchanged.
        bool defer = false;
        for (const std::size_t u : snapshot) {
            if (units->formation_leave_waits_for_handover(u)) defer = true;
        }
        for (const std::size_t u : snapshot) {
            if (units->unit_formation_group_0284(u) < 0) continue;
            if (!defer || !units->post_formation_leave_0077c980(u)) {
                units->leave_group_on_destroy_0077bd70(u);
            } else {
                ++posted;
            }
            ++left;
        }
    }
    const GameUnitRow* row = valid ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    log_.notef("  DisbandFormation 0089a020: \"%s\" group=%d left=%zu (0077CA60 -> 77h -> "
        "0077BD70, packet cc13-fix5)", row != nullptr ? row->name.c_str() : "?",
        static_cast<int>(group), left);
    if (posted != 0) {
        log_.notef("  DisbandFormation 0089a020: %zu leave(s) posted behind a leader handover "
            "(packet cc13-jm08)", posted);
    }
    log_.implemented("MissionLuaNative::DisbandFormation", "0089a020");
    return 0;
}

// Packet cc13-fix5 (docs/CC13_FIX_5.md section 6). The LandConvoy natives, lua_CFunction,
// no result: argument 0 through 00888D20 (0089566F, 008957DF, 0089594F), then
// 00895688 00741120 (+3A8h = 0, Start), 008957F8 00741130 (+3A8h = 1, Stop), or
// 00895980 00B66270 on argument 1 and 008959A3 00741140 (+368h = speed, SetSpeed).
// SUBSTITUTIONS (labelled): the convoy is the units host's bound motion state named
// like the argument (its `Name` field, else the scene marker's name for its `ID`);
// an argument that names no bound convoy stores nothing (the image writes through
// whatever pointer Ptr holds).
int GameMissionLuaHost::run_land_convoy_native(lua_State* state, int argument_count,
    std::uint32_t address) {
    std::string name;
    if (::lua_type(state, 1) == LUA_TTABLE) {
        lua_getfield(state, 1, "Name");
        if (const char* text = ::lua_tostring(state, -1)) name = text;
        ::lua_settop(state, ::lua_gettop(state) - 1);
    }
    const int id = air_ops_entity_id(state);
    if (name.empty() && id > 0) {
        if (const PendingEntity* node = find_pending(id)) name = node->name;
    }
    bool stored = false;
    const char* what = "?";
    float speed = 0.0f;
    if (units_hooks_ != nullptr && !name.empty()) {
        if (address == 0x00895570u) {
            what = "Start";
            stored = units_hooks_->set_land_convoy_stopped_3a8(name, false);
        } else if (address == 0x008956e0u) {
            what = "Stop";
            stored = units_hooks_->set_land_convoy_stopped_3a8(name, true);
        } else if (address == 0x00895850u) {
            what = "SetSpeed";
            speed = argument_count >= 2 ? static_cast<float>(::lua_tonumber(state, 2)) : 0.0f;
            stored = units_hooks_->set_land_convoy_speed_368(name, speed);
        }
    }
    log_.notef("  LandConvoy%s %08x: \"%s\" (id %d) speed=%.2f stored=%d (packet cc13-fix5)",
        what, address, name.c_str(), id, static_cast<double>(speed), stored ? 1 : 0);
    if (address == 0x00895570u) log_.implemented("MissionLuaNative::LandConvoyStart", "00895570");
    if (address == 0x008956e0u) log_.implemented("MissionLuaNative::LandConvoyStop", "008956e0");
    if (address == 0x00895850u) {
        log_.implemented("MissionLuaNative::LandConvoySetSpeed", "00895850");
    }
    return 0;
}

// Packet cc13-fix5 (docs/CC13_FIX_5.md section 8). 00896750 LaunchAirBaseSlot(base,
// slot[, now]), lua_CFunction. 00888AA0 and 006BCD20 for the block; argument 1 as an
// integer; argument 2 through 00B66250 only when 00B663F0 counts three or more,
// else 0. A slot whose +2Ch is 2 returns with nothing pushed. Otherwise
// 00896915..00896964 route 83h {slot - 1, now} (0077C2A0(msg, 0, 0)), 00896972 sets
// slot+34h, 00896976 00874D00 (CL = 1), 00896983 006CDC70(0.0) on the block,
// 0089698A 00874D00 (CL = 1), and with slot+28h set the result is
// thisTable[[slot+28h]+174h].
// SUBSTITUTIONS (labelled):
// - the block is the deck registry's entry for the entity; a missing deck or a slot
//   number outside the array (the image indexes it unchecked) does nothing;
// - 83h is delivered at the call, after the +34h store, which is where the first
//   extra step's session pump delivers it in the image; its receiver is
//   air_ops_launch_slot_message_006ca8e0;
// - of 006CDC70(0) only its 006C64B0 is run, for this deck: the queue head that
//   is served is launched by 006C7490(slot, 0), the 89h 006CD6C0 case 8 runs, at
//   the call; the step-0 slot ticks and the other sub-updates are not run;
// - the two 00874D00 calls run through the host's runner (kRunExtraFixedStepBound);
// - the squadron's handle is its entity id, which push_resolved_entity_by_id turns
//   into the `thisTable` slot (nil when that slot is not built yet).
// The 83h/85h/89h peer messages, 006BF150's publish and 00984EB0 are records.
int GameMissionLuaHost::run_launch_air_base_slot_00896750(lua_State* state,
    int argument_count) {
    const int id = air_ops_entity_id(state);
    const int slot_number = argument_count >= 2 ? static_cast<int>(::lua_tonumber(state, 2)) : 0;
    const bool now = argument_count >= 3 && ::lua_toboolean(state, 3) != 0;
    const int index = slot_number - 1;
    bsp::AirOpsDeck* deck = id > 0 ? bsp::air_ops_decks().find_mutable_by_entity_id(id) : nullptr;
    if (deck == nullptr || index < 0 || static_cast<std::size_t>(index) >= deck->slots.size()) {
        log_.notef("  LaunchAirBaseSlot 00896750: entity id %d slot %d unresolved (deck %s) "
            "(packet cc13-fix5)", id, slot_number, deck != nullptr ? "present" : "missing");
        return 0;
    }
    if (deck->slots[static_cast<std::size_t>(index)].state == bsp::AirOpsSlotState::kLaunching) {
        log_.notef("  LaunchAirBaseSlot 00896750: entity id %d slot %d already in state 2 "
            "(packet cc13-fix5)", id, slot_number);
        return 0;
    }
    deck->slots[static_cast<std::size_t>(index)].launch_requested = true;   // 00896972
    const bsp::AirOpsLaunchSlotMessageResult message =
        bsp::air_ops_launch_slot_message_006ca8e0(*deck, index, now);
    log_.unimplemented("AirOps::launch_slot_peer_messages", "006bf150");
    if (kRunExtraFixedStepBound && extra_fixed_step_ != nullptr) {
        extra_fixed_step_->run_extra_fixed_step_00874d00(true, 0x00896976u);
    }
    // The extra step can register decks (GenerateObject), so the deck is found again.
    deck = bsp::air_ops_decks().find_mutable_by_entity_id(id);
    int served = -1;
    if (deck != nullptr) {
        served = bsp::air_ops_queued_slot_wait_006c64b0(*deck);
        if (served >= 0) bsp::air_ops_launch_start_006c7490(*deck, served);
    }
    if (kRunExtraFixedStepBound && extra_fixed_step_ != nullptr) {
        extra_fixed_step_->run_extra_fixed_step_00874d00(true, 0x0089698au);
    }
    deck = bsp::air_ops_decks().find_mutable_by_entity_id(id);
    const std::uint32_t squadron = deck != nullptr && static_cast<std::size_t>(index) < deck->slots.size()
        ? deck->slots[static_cast<std::size_t>(index)].launched_squadron : 0u;
    log_.notef("  LaunchAirBaseSlot 00896750: entity id %d slot %d now=%d -> %s, served=%d, "
        "stock left %d, squadron %u (packet cc13-fix5)", id, slot_number, now ? 1 : 0,
        message.started ? "launched" : "queued", served + 1, message.stock_left, squadron);
    log_.implemented("MissionLuaNative::LaunchAirBaseSlot", "00896750");
    if (squadron == 0u) return 0;
    if (!push_resolved_entity_by_id(state, static_cast<int>(squadron))) ::lua_pushnil(state);
    return 1;
}

// Packet cc9_lua_add_shipyard_stock (docs/SQUADRON_LAND_TASK.md 5ej). 00896CC0
// AddShipyardStock(entity, class, count [, names]): the contract is at
// kLuaAddShipyardStockBound. 0084ACB0 (GUNNERY 136, bsp::shipyard_add_stock_0084acb0)
// adds the count to the record whose class+70h is the id, else pushes a new record
// (008485F0, 008499A0).
// SUBSTITUTIONS (labelled): the object is the bsp::shipyards() record bound to the
// entity id, else the one named like the entity's row; an entity with neither is
// counted unresolved, where the image calls 0084ACB0 on any object 00888AA0 returns.
int GameMissionLuaHost::run_add_shipyard_stock_00896cc0(lua_State* state, int argument_count) {
    ++summary_.shipyard_stock_calls;
    const int class_id = argument_count >= 2 ? static_cast<int>(::lua_tonumber(state, 2)) : 0;
    const int count = argument_count >= 3 ? static_cast<int>(::lua_tonumber(state, 3)) : 0;
    std::string names;
    if (argument_count >= 4) {
        const char* text = ::lua_tostring(state, 4);
        if (text != nullptr) names = text;
    }
    const int id = air_ops_entity_id(state);
    bsp::ShipyardState* yard = id > 0 ? bsp::shipyards().find_mutable_by_entity_id(id) : nullptr;
    std::string row_name;
    if (units_hooks_ != nullptr && id > 0 && static_cast<std::size_t>(id) <= units_hooks_->count()) {
        if (const GameUnitRow* row = units_hooks_->unit_row(static_cast<std::size_t>(id - 1))) {
            row_name = row->name;
        }
    }
    if (yard == nullptr && !row_name.empty()) yard = bsp::shipyards().find_mutable(row_name);
    if (yard == nullptr) {
        ++summary_.shipyard_stock_unresolved;
        log_.notef("  AddShipyardStock 00896cc0: entity id %d \"%s\" has no shipyard record "
            "(class %d +%d) (packet cc9_lua_add_shipyard_stock)", id, row_name.c_str(), class_id,
            count);
        return 0;
    }
    const std::size_t before = yard->stock.size();
    const int total = bsp::shipyard_add_stock_0084acb0(*yard, static_cast<std::uint32_t>(class_id),
        count, names);
    if (yard->stock.size() > before) ++summary_.shipyard_stock_created;
    log_.notef("  AddShipyardStock 00896cc0: \"%s\" class %d +%d -> %d names=\"%s\"%s (packet "
        "cc9_lua_add_shipyard_stock)", yard->owner_name.c_str(), class_id, count, total,
        names.c_str(), yard->stock.size() > before ? " (new record)" : "");
    log_.implemented("MissionLuaNative::AddShipyardStock", "00896cc0");
    return 0;
}

// Packet cc9_shipyard_create_unit (GUNNERY 136 (D), docs/SQUADRON_LAND_TASK.md 5ek).
// 00844FC0's creation, for gunnery30's build arm (bsp::shipyard_create_unit()): a bag
// with `Type` = the class, the shipyard's `Skill` / `Race` / `Party` / `OwnerPlayer`
// and `ShipYardLaunch` = 1; the creator by kind (plane 004F0AD0 with `WingCount` 1,
// destroyer 004F0520, mother ship 004F0860, submarine 004F05F0, torpedo boat
// 004F06C0, landing ship 004F0790); the request's frame, its y on the water; then
// `moveonpath` 00E08F80 on the hangar's path (008454B4..008454C9).
// The creation runs the SpawnNew member path (0094879A's InitAll after it).
// SUBSTITUTIONS (labelled): Party is the shipyard row's; Race follows SpawnNew's
// default (2 for party 0, else 1) and Skill / OwnerPlayer the record defaults (1, 9),
// since this process keeps no bag on the shipyard unit (with
// kShipyardLaunchSkillCc13Bound, Skill is the shipyard's own); the plane's `State` 6 and
// `VelocitySI` 0 and the submarine's `Dive` 0 have no record field, so a plane is
// made airborne as every created plane here is; the water height is 0, the ocean
// sampler's level with no wave field over the point; the bag ref at unit+C0h
// (00845440) and the hangar observer 00694A60 are not modelled.
std::size_t GameMissionLuaHost::create_shipyard_unit_00844fc0(
    const bsp::ShipyardBuildRequest& r) {
    ++summary_.shipyard_units_requested;
    if (script_orders_ == nullptr || units_hooks_ == nullptr) return bsp::kShipyardNone;
    bsp::game::GameSceneEntityRecord record;
    switch (r.creator) {
    case bsp::ShipyardCreator::PlaneSquadron: record.class_name = "PlaneSquadronGen"; record.class_id = 0x18; break;
    case bsp::ShipyardCreator::Destroyer: record.class_name = "DestroyerGen"; record.class_id = 0x07; break;
    case bsp::ShipyardCreator::MotherShip: record.class_name = "MotherShipGen"; record.class_id = 0x09; break;
    case bsp::ShipyardCreator::Submarine: record.class_name = "SubmarineGen"; record.class_id = 0x08; break;
    case bsp::ShipyardCreator::TorpedoBoat: record.class_name = "TBoatGen"; record.class_id = 0x0e; break;
    case bsp::ShipyardCreator::LandingShip: record.class_name = "LandingShipGen"; record.class_id = 0x0c; break;
    default:
        log_.notef("  shipyard create 00844fc0: \"%s\" class %u has no creator (00845416)",
            r.shipyard_name.c_str(), r.class_id);
        return bsp::kShipyardNone;
    }
    int party = -1;
    std::size_t shipyard_index = bsp::kShipyardNone;
    for (std::size_t i = 0; i < units_hooks_->count(); ++i) {
        const GameUnitRow* row = units_hooks_->unit_row(i);
        if (row != nullptr && row->name == r.shipyard_name) {
            party = row->party;
            shipyard_index = i;
            break;
        }
    }
    const bool plane = r.creator == bsp::ShipyardCreator::PlaneSquadron;
    ++shipyard_units_serial_;
    char suffix[24];
    std::snprintf(suffix, sizeof(suffix), " #Y%u", shipyard_units_serial_);
    record.name = (r.gui_name.empty() ? r.shipyard_name : r.gui_name) + suffix;
    record.type_id = static_cast<int>(r.class_id);
    record.party = party;
    record.race = party == 0 ? 2 : 1;
    record.shipyard_launch = true;
    if constexpr (kShipyardLaunchSkillCc13Bound) {
        // Packet cc13-yardskill. The image reads the live shipyard (00844FC0 runs on
        // it), so its +390h is whatever the scene `Skill` or a later SetSkillLevel
        // left. Here that is the shipyard row's pilot_skill_index, read now, at
        // create time. A shipyard row that is not found keeps the record default 1.
        if (shipyard_index != bsp::kShipyardNone) {
            record.bag_skill = units_hooks_->skill_level(shipyard_index);
            record.bag_skill_source = "00844FC0";
        }
    }
    record.bag_equipment = plane && r.equipment >= 0 ? r.equipment : -1;
    record.created = true;
    for (int i = 0; i < 16; ++i) record.world[i] = r.frame[i];
    if (r.snap_to_water) record.world[13] = 0.0f;
    bsp::game::scene_spawn_pool().add(record);
    if (bsp::game::SceneSpawnPoolEntry* entry = bsp::game::scene_spawn_pool().find(record.name)) {
        entry->wing_count_present = plane;
        entry->wing_count_raw = plane ? 1 : 0;
    }
    const std::size_t units_before = script_orders_->units().count();
    const std::uint32_t entity = script_orders_->create_unit_from_scene_record_0046db4b(record);
    if (bsp::game::SceneSpawnPoolEntry* entry = bsp::game::scene_spawn_pool().find(record.name)) {
        entry->spawned = entity != 0u;
        entry->entity_id = static_cast<int>(entity);
    }
    if (entity == 0u) {
        log_.notef("  shipyard create 00844fc0: \"%s\" class %u reached no creator",
            record.name.c_str(), r.class_id);
        return bsp::kShipyardNone;
    }
    if constexpr (kSEntityInitAllBound) {
        if (plane) {
            route_push_squadron(static_cast<int>(entity), record.name, record.type_id, units_before);
        } else {
            route_push_entity(static_cast<int>(entity), record.name, record.type_id);
            if (PendingEntity* node = find_pending(static_cast<int>(entity))) {
                node->party = record.party;
                node->race = record.race;
                node->generated_party = true;
            }
        }
        run_sentity_init_all_00925f20(false, 0x00844fc0u);
    }
    const std::size_t index = static_cast<std::size_t>(entity) - 1;
    const bool ordered = script_orders_->issue_shipyard_moveonpath_008454b4(index, r.path_name);
    ++summary_.shipyard_units_made;
    log_.notef("  shipyard create 00844fc0: \"%s\" %s type %u party %d at (%.1f %.1f %.1f) -> "
        "entity %u; moveonpath %s %s (packet cc9_shipyard_create_unit)", record.name.c_str(),
        record.class_name.c_str(), r.class_id, party, static_cast<double>(record.world[12]),
        static_cast<double>(record.world[13]), static_cast<double>(record.world[14]), entity,
        r.path_name.c_str(), ordered ? "issued" : "NOT issued (no such path marker)");
    log_.implemented("Shipyard::create_unit", "00844fc0");
    return index;
}

namespace {
// 00E17BF2. One byte in the image, so one value per process here.
bool g_device_reload_enabled_00e17bf2 = false;
}  // namespace

bool lua_device_reload_enabled_00e17bf2() noexcept {
    return kLuaDeviceReloadEnabledBound && g_device_reload_enabled_00e17bf2;
}

namespace {
// 00E08178, 1.0 from the mission load (004DFBEB). One float per process here.
float g_device_reload_time_mul_00e08178 = 1.0f;
}  // namespace

float lua_device_reload_time_mul_00e08178() noexcept {
    return kLuaDeviceReloadTimeMulBound ? g_device_reload_time_mul_00e08178 : 1.0f;
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

// Packet p16_lua_natives. 008AC2B0 RemoveUntouchableUnit(unit): 00888AA0 (008AC3AF),
// vtable[140h]() (008AC3D0), byte +1D4h = 0 (008AC3D6). No return value.
int GameMissionLuaHost::run_remove_untouchable_unit_008ac2b0(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.untouchable_remove_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const bool valid = units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count();
    const GameUnitRow* row = valid ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    bool was = false;
    if (valid) {
        const std::size_t index = static_cast<std::size_t>(id - 1);
        if (index < g_unit_untouchable_1d4.size()) {
            was = g_unit_untouchable_1d4[index];
            g_unit_untouchable_1d4[index] = false;
        }
        ++summary_.untouchable_cleared;
    }
    log_.notef("  RemoveUntouchableUnit 008ac2b0: \"%s\" (unit %d) was=%d t=%.2f (packet "
        "p16_lua_natives)", row != nullptr ? row->name.c_str() : "?", id, was ? 1 : 0,
        static_cast<double>(spawn_world_clock_));
    log_.implemented("MissionLuaNative::RemoveUntouchableUnit", "008ac2b0");
    return 0;
}

// Packet p16_lua_natives. 008AC420 IsUnitUntouchable(unit): vtable[140h]() non-null and
// its byte +1D4h set, pushed as a boolean (one result).
int GameMissionLuaHost::run_is_unit_untouchable_008ac420(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.untouchable_is_calls;
    const int id = air_ops_entity_id(state);
    const bool on = id > 0 && lua_unit_untouchable_1d4(static_cast<std::size_t>(id - 1));
    if (on) ++summary_.untouchable_is_true;
    ::lua_pushboolean(state, on ? 1 : 0);
    log_.implemented("MissionLuaNative::IsUnitUntouchable", "008ac420");
    return 1;
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

// Packet cc13-fix18 (docs/CC13_FIX18.md section 5), under kLuaDeviceReloadTimeMulBound.
// 008C14B0 SetDeviceReloadTimeMul(mul): 008C15AF 00B66270 reads argument 0 as a number,
// 008C15B4 FSTP float [00E08178] (the double rounded to float); no entity, no class test,
// no clamp (jm03 passes 0, which stops the countdown), no result. No RNG.
int GameMissionLuaHost::run_set_device_reload_time_mul_008c14b0(lua_State* state,
    int argument_count) {
    ++summary_.reload_time_mul_calls;
    const double mul = argument_count >= 1 ? ::lua_tonumber(state, 1) : 0.0;
    g_device_reload_time_mul_00e08178 = static_cast<float>(mul);   // 008C15B4
    log_.notef("  SetDeviceReloadTimeMul 008c14b0: mul=%g -> 00E08178 (packet cc13-fix18)",
        static_cast<double>(g_device_reload_time_mul_00e08178));
    log_.implemented("MissionLuaNative::SetDeviceReloadTimeMul", "008c14b0");
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

// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 7). 008A24E0
// SquadronSetReleaseAlt(squadron, altitude[, force]), 008A24E0-008A2702 RET:
// 008A25E6 00888AA0 on argument 0 (ESI), 008A2617 00B66270 on argument 1, and
// argument 2 through 00B66250 (008A265F) only when 008A2634 00B663F0 counts
// exactly three arguments; then the five stores 008A2692..008A26B9. No result.
// SUBSTITUTION (labelled): an entity with no squadron slot stores nothing.
int GameMissionLuaHost::run_squadron_set_release_alt_008a24e0(lua_State* state,
    int argument_count) {
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const float altitude = argument_count >= 2 ? static_cast<float>(::lua_tonumber(state, 2)) : 0.0f;
    const bool force = argument_count == 3 && ::lua_toboolean(state, 3) != 0;
    bool stored = false;
    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
        stored = units->set_squadron_release_alt_008a24e0(static_cast<std::size_t>(id - 1),
            altitude, force);
    }
    const GameUnitRow* row = stored ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    log_.notef("  SquadronSetReleaseAlt 008a24e0: \"%s\" alt=%.1f force=%d stored=%d (packet "
        "cc13_natives1)", row != nullptr ? row->name.c_str() : "?",
        static_cast<double>(altitude), force ? 1 : 0, stored ? 1 : 0);
    log_.implemented("MissionLuaNative::SquadronSetReleaseAlt", "008a24e0");
    return 0;
}

// Packet cc13-issuea0. 008A1F40 SquadronSetDropAllBombPercent(squadron, percent):
// argument 0 through 00888AA0 (008A203F), argument 1 through 00B66270 (008A2070),
// then FSTP float [squadron+390h] (008A2091). No result. Bound only under
// bsp::kPlaneIssueThresholdZeroCc13Bound, the reader's switch.
int GameMissionLuaHost::run_squadron_set_drop_all_bomb_percent_008a1f40(lua_State* state,
    int argument_count) {
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const float percent = argument_count >= 2 ? static_cast<float>(::lua_tonumber(state, 2)) : 0.0f;
    bool stored = false;
    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
        stored = units->set_squadron_drop_all_bomb_percent_008a1f40(
            static_cast<std::size_t>(id - 1), percent);
    }
    const GameUnitRow* row = stored ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    log_.notef("  SquadronSetDropAllBombPercent 008a1f40: \"%s\" percent=%g stored=%d (packet "
        "cc13-issuea0)", row != nullptr ? row->name.c_str() : "?",
        static_cast<double>(percent), stored ? 1 : 0);
    log_.implemented("MissionLuaNative::SquadronSetDropAllBombPercent", "008a1f40");
    return 0;
}

// Packet cc13-fix4 (docs/CC13_VER_10.md section 4). 008A20E0
// SquadronLandAndKill(squadron), lua_CFunction, no result (008A2254 00B66400).
// 008A21C8 00888AA0 on argument 0 and the class test 18h (PlaneSquadronGen);
// 008A220F 007F1B70 frees the launching slot on every carrier and airfield; from
// +3CCh - 1 down to 0, 008A223F 00926D90(plane, 5) then 008A2249 007F3970(plane,
// 1). The sequence is squadron_land_and_kill_008a20e0 (src/air_operations.cpp).
// SUBSTITUTIONS (labelled):
// - the plane handle is units-host index + 1 and the +3D0h array is the registry
//   record's live members in order; the squadron handle is squadron_unit + 1, the
//   value the deck slots hold (as the stock return 007F1D4E passes it);
// - the block handle is the deck registry index (every carrier and airfield);
// - 007F3970's own removal and its squadron Kill (007F3A2A) are not run here: the
//   units host's death sweep (007BCAA0 -> 007F3970, kill sweep of the next motion
//   step) removes each dead member, and the fused squadron slot's Dead is held
//   until the record's live count is 0, the host's stand-in for 007F3A2A;
// - the gunnery host's kill ignores the cause, so the published KillReason is
//   not "landed" (5): the units host keeps the cause and has no public writer
//   (adoption note in docs/CC13_VER_10.md section 7);
// - a non-squadron argument (the image dereferences null) and a squadron with no
//   record do nothing.
int GameMissionLuaHost::run_squadron_land_and_kill_008a20e0(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || units->gunnery() == nullptr || id <= 0
        || static_cast<std::size_t>(id) > units->count()) {
        log_.notef("  SquadronLandAndKill 008a20e0: entity id %d has no units-host slot (packet "
            "cc13-fix4)", id);
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
    if (record == nullptr) {
        log_.notef("  SquadronLandAndKill 008a20e0: entity id %d is not a squadron (packet "
            "cc13-fix4)", id);
        return 0;
    }
    std::vector<std::uint32_t> planes;
    for (std::size_t member : record->member_units) {
        if (member != bsp::kPlaneSquadronNoUnit) planes.push_back(static_cast<std::uint32_t>(member + 1));
    }
    bsp::AirOpsDeckRegistry& decks = bsp::air_ops_decks();
    std::vector<std::uint32_t> blocks;
    for (std::size_t d = 0; d < decks.size(); ++d) blocks.push_back(static_cast<std::uint32_t>(d));

    struct Host final : bsp::SquadronLandAndKillHost {
        Host(bsp::AirOpsDeckRegistry& d, GameGunneryHost& g, std::map<std::size_t, int>& causes)
            : decks(d), gunnery(g), lua_kill_cause(causes) {}
        void release_squadron_slot(std::uint32_t block, std::uint32_t squadron) override {
            bsp::AirOpsDeck* deck = decks.mutable_at(block);
            if (deck != nullptr) released += bsp::air_ops_release_squadron_slot_006c65b0(*deck, squadron);
        }
        void kill_plane(std::uint32_t plane, std::int32_t reason) override {
            if (plane == 0u) return;
            const std::size_t unit = plane - 1u;
            // 00926D90 returns when +5Fh is already set.
            if (gunnery.unit_dead(unit)) return;
            // 007F3B19 CMP [unit+70h],1: a cause-5 death does not report `kill`.
            if constexpr (kKillListenerHarmOnlyBound) lua_kill_cause[unit] = reason;  // cc13-ver8
            gunnery.kill_unit_00926d90(unit, reason);
            ++killed;
        }
        bsp::AirOpsDeckRegistry& decks;
        GameGunneryHost& gunnery;
        std::map<std::size_t, int>& lua_kill_cause;
        std::size_t released{0};
        std::size_t killed{0};
    } host(decks, *units->gunnery(), lua_kill_cause_);

    const std::string name = record->name;
    std::int32_t plane_count = static_cast<std::int32_t>(planes.size());
    bsp::squadron_land_and_kill_008a20e0(host,
        static_cast<std::uint32_t>(record->squadron_unit + 1u), blocks.data(),
        static_cast<int>(blocks.size()), planes.data(), &plane_count);
    log_.notef("  SquadronLandAndKill 008a20e0: squadron %s planes=%zu killed=%zu "
        "slots_released=%zu (packet cc13-fix4)", name.c_str(), planes.size(), host.killed,
        host.released);
    log_.implemented("MissionLuaNative::SquadronLandAndKill", "008a20e0");
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
    if constexpr (bsp::kShipCatapultLaunchBound) {
        // Packet p17_catapult_adapter: 00953A60 on the host's record; a dead
        // child's slot reads 0 (the observer handle), which pushes nil.
        bsp::ShipCatapultClass cls;
        if (bsp::ShipCatapultRecord* r = catapult_record(static_cast<std::size_t>(id - 1), cls)) {
            const std::uint32_t child = bsp::ShipCatapultRegistry::last_00953a60(*r);
            if (child != 0u && push_resolved_entity_by_id(state, static_cast<int>(child))) {
                ++summary_.last_catapulted_live;
                return 1;
            }
        }
    }
    ++summary_.last_catapulted_nil;
    ::lua_pushnil(state);
    return 1;
}

// Packet cc13-fix14b. 00892AA0 GetNumCatapulted(ship) (__fastcall, ECX the Lua
// state, RET): 00892B9F 00888AA0 on argument 0; 00892BB6 CMP ESI,EBP (EBP zeroed at 00892AC3) /
// JE to 00892BD3 PUSH EBP; 00892BC3 vtable[5Ch](6), JE on false to the same zero;
// otherwise 00892BCB 00953920 (ECX the ship), which counts the ten launch slots
// +550h, +568h .. +628h whose first dword is non-zero; 00892BD8 00B664B0
// PushInteger. One integer result on every path. LABELLED: no path in this host
// fills a launch slot (the catapult launch 006EC8E0 and 00957450's slot write are
// not built), so a ship counts 0 until the catapult adapter lands; the count is
// bsp::ShipCatapultOffsets' slots, empty from the constructor.
int GameMissionLuaHost::run_get_num_catapulted_00892aa0(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.num_catapulted_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const bool valid = units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count();
    const bool ship = valid
        && units->unit_is_kind_of(static_cast<std::size_t>(id - 1), bsp::kUnitKindQueryShip);
    // 00953920 over the slots this host keeps: none is occupied without the
    // catapult adapter (packet p17_catapult_adapter).
    lua_Integer count = 0;
    if constexpr (bsp::kShipCatapultLaunchBound) {
        bsp::ShipCatapultClass cls;
        if (ship) {
            if (bsp::ShipCatapultRecord* r = catapult_record(static_cast<std::size_t>(id - 1), cls)) {
                count = bsp::ShipCatapultRegistry::count_00953920(*r);
            }
        }
    }
    if (!ship) ++summary_.num_catapulted_not_ship;
    if (summary_.num_catapulted_calls <= 8) {
        const GameUnitRow* row = valid ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
        log_.notef("  GetNumCatapulted 00892aa0: \"%s\" ship=%d -> %d (%s, packet cc13-fix14b)",
            row != nullptr ? row->name.c_str() : "?", ship ? 1 : 0, static_cast<int>(count),
            ship ? "no launch slot is filled in this host" : "not a ship: 0");
    }
    log_.implemented("MissionLuaNative::GetNumCatapulted", "00892aa0");
    ::lua_pushinteger(state, count);
    return 1;
}

// Packet cc9_lua55_catapult_stock. 00892DE0 GetCatapultStock(entity): 00888AA0 on
// argument 0, vtable[5Ch](5), then (x < 0) - 1 & x on unit+638h, i.e. max(x, 0);
// any other entity, or none, pushes 0 (00892F1F). One integer result.
// LABELLED: unit+638h is the init value [unit+538h]+C8h (0095E5B0), the class
// Catapult.LaunchStock, read here from the entity's `Class` table; the catapult
// launch decrement (00957450) and SetCatapultStock (009539A0) are not built.
int GameMissionLuaHost::run_get_catapult_stock_00892de0(lua_State* state,
    int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.catapult_stock_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    const bool valid = units != nullptr && id > 0
        && static_cast<std::size_t>(id) <= units->count();
    const bool kind5 = valid && units->unit_is_kind_of(static_cast<std::size_t>(id - 1), 5);
    lua_Integer stock = 0;
    if (kind5 && push_resolved_entity_by_id(state, id)) {
        const int base = ::lua_gettop(state);
        ::lua_getfield(state, -1, "Class");
        if (lua_istable(state, -1)) {
            ::lua_getfield(state, -1, "Catapult");
            if (lua_istable(state, -1)) {
                ::lua_getfield(state, -1, "LaunchStock");
                if (::lua_type(state, -1) == LUA_TNUMBER) stock = ::lua_tointeger(state, -1);
            }
        }
        ::lua_settop(state, base - 1);
    }
    if constexpr (bsp::kShipCatapultLaunchBound) {
        // Packet p17_catapult_adapter: unit+638h is the host's record, which the
        // launch (00957450) and SetCatapultStock (009539A0) write.
        bsp::ShipCatapultClass cls;
        if (kind5) {
            if (bsp::ShipCatapultRecord* r = catapult_record(static_cast<std::size_t>(id - 1), cls)) {
                stock = r->slots.pending_638;
            }
        }
    }
    if (stock < 0) stock = 0;
    if (summary_.catapult_stock_calls <= 8) {
        const GameUnitRow* row = valid ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
        log_.notef("  GetCatapultStock 00892de0: \"%s\" kind5=%d -> %d (class "
            "Catapult.LaunchStock, packet cc9_lua55_catapult_stock)",
            row != nullptr ? row->name.c_str() : "?", kind5 ? 1 : 0, static_cast<int>(stock));
    }
    log_.implemented("MissionLuaNative::GetCatapultStock", "00892de0");
    ::lua_pushinteger(state, stock);
    return 1;
}

// Packet p17_catapult_adapter (include/bsp/ship_catapult_host.hpp). The class
// Catapult block as 00960230 reads it into class +C0h..+CCh, and the ship's record
// with dead children dropped from their slots.
bsp::ShipCatapultRecord* GameMissionLuaHost::catapult_record(std::size_t unit,
                                                             bsp::ShipCatapultClass& cls) {
    GameUnitsHost* units = units_hooks_;
    if (units == nullptr || unit >= units->count()) return nullptr;
    const GameUnitRow* row = units->unit_row(unit);
    if (row == nullptr) return nullptr;
    cls.launched_class = static_cast<std::int32_t>(
        read_vehicle_class_nested_number(row->type_id, "Catapult", "LaunchedClass", -1.0f));
    cls.equipment = static_cast<std::int32_t>(
        read_vehicle_class_nested_number(row->type_id, "Catapult", "Equipment", 0.0f));
    cls.launch_stock = static_cast<std::int32_t>(
        read_vehicle_class_nested_number(row->type_id, "Catapult", "LaunchStock", 0.0f));
    cls.max_launched = static_cast<std::int32_t>(
        read_vehicle_class_nested_number(row->type_id, "Catapult", "MaxLaunchedPlanes", 0.0f));
    bsp::ShipCatapultRecord& r = bsp::ship_catapult_registry().record(unit, cls);
    const std::vector<std::pair<std::size_t, float>> destroyed = units->destroyed_units();
    bsp::ShipCatapultRegistry::prune(r, [&](std::uint32_t entity) {
        if (entity == 0u || entity > units->count()) return false;
        for (const auto& d : destroyed) {
            if (d.first + 1u == entity) return false;
        }
        return true;
    });
    return &r;
}

// Packet p17_catapult_adapter. 00891D50 ShipUseCatapult(ship [, heading]): 00888D20
// on argument 0, the heading 0.0 or argument 1, message 7Bh; the ship's 7Bh arm
// (00821F12) runs 008141E0, whose gate is bsp::ShipCatapultRegistry::gate_008141e0.
// A launch creates the LaunchedClass squadron (WingCount 1, State 7) above the ship
// along its heading and registers it as 00957450 does. LABELLED: see
// include/bsp/ship_catapult_host.hpp (no catapult device, no held State 3, no
// takeoff timeline; the plane appears 30 m above the ship's origin).
int GameMissionLuaHost::run_ship_use_catapult_00891d50(lua_State* state, int argument_count) {
    static_cast<void>(argument_count);
    ++summary_.use_catapult_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || script_orders_ == nullptr || id <= 0
        || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.use_catapult_no_catapult;
        return 0;
    }
    const std::size_t ship = static_cast<std::size_t>(id - 1);
    bsp::ShipCatapultClass cls;
    bsp::ShipCatapultRecord* r = catapult_record(ship, cls);
    const GameUnitRow* row = units->unit_row(ship);
    if (r == nullptr || row == nullptr) {
        ++summary_.use_catapult_no_catapult;
        return 0;
    }
    const bsp::ShipCatapultVerdict verdict = bsp::ShipCatapultRegistry::gate_008141e0(*r, cls);
    if (verdict != bsp::ShipCatapultVerdict::kLaunch) {
        if (verdict == bsp::ShipCatapultVerdict::kNoCatapult) ++summary_.use_catapult_no_catapult;
        if (verdict == bsp::ShipCatapultVerdict::kNoStock) ++summary_.use_catapult_no_stock;
        if (verdict == bsp::ShipCatapultVerdict::kTooManyInAir) ++summary_.use_catapult_in_air;
        ++r->refusals;
        if (r->refusals <= 4) {
            log_.notef("  ShipUseCatapult 00891d50: \"%s\" refused (%s; stock=%d live=%d max=%d "
                "class=%d, packet p17_catapult_adapter)", row->name.c_str(),
                verdict == bsp::ShipCatapultVerdict::kNoCatapult ? "no catapult"
                    : verdict == bsp::ShipCatapultVerdict::kNoStock ? "no stock" : "too many in air",
                static_cast<int>(r->slots.pending_638),
                static_cast<int>(bsp::ShipCatapultRegistry::count_00953920(*r)),
                static_cast<int>(cls.max_launched), static_cast<int>(cls.launched_class));
        }
        return 0;
    }
    // 006EC8E0's bag: Type = LaunchedClass (the instance override +530h is -1 here),
    // WingCount 1, Party, State (7 here, LABELLED), Equipment.
    bsp::DummyKamikazeSpawn spawn;
    spawn.carrier = ship;
    spawn.plane_class = cls.launched_class;
    spawn.party = row->party;
    spawn.dummy_id = r->launches + 1u;
    const float yaw = static_cast<float>(static_cast<double>(row->heading_degrees) * 3.14159265358979 / 180.0);
    apply_scene_yaw_00467050(spawn.world.data(), yaw);
    spawn.world[3] = 0.0f; spawn.world[7] = 0.0f; spawn.world[11] = 0.0f; spawn.world[15] = 1.0f;
    spawn.world[12] = row->position[0];
    spawn.world[13] = row->position[1] + 30.0f;
    spawn.world[14] = row->position[2];
    const std::size_t units_before = units->count();
    std::string name;
    const std::uint32_t entity =
        script_orders_->create_dummy_kamikaze_squadron_006fec00(spawn, name, "_cat%02u");
    if (entity == 0u) {
        ++summary_.use_catapult_spawn_failed;
        return 0;
    }
    if constexpr (kSEntityInitAllBound) {
        route_push_squadron(static_cast<int>(entity), name, cls.launched_class, units_before);
    } else if (attach_created_entity_00928a00(static_cast<int>(entity), name, cls.launched_class)) {
        attach_wing_member_tables(units_before, entity, cls.launched_class);
    }
    const int slot = bsp::ShipCatapultRegistry::add_child_00957450(*r, entity);
    ++r->launches;
    ++summary_.use_catapult_launches;
    log_.notef("  ShipUseCatapult 00891d50: \"%s\" launched %s (class %d) slot %d, stock now %d "
        "(008141E0 -> 006EC8E0 -> 00957450, packet p17_catapult_adapter)", row->name.c_str(),
        name.c_str(), static_cast<int>(cls.launched_class), slot,
        static_cast<int>(r->slots.pending_638));
    log_.implemented("MissionLuaNative::ShipUseCatapult", "00891d50");
    return 0;
}

// Packet p17_catapult_adapter. 00892C30 SetCatapultStock(ship, n): 00888AA0 on
// argument 0; a non-null entity passing vtable[5Ch](5) gets 009539A0(integer
// argument 1): nothing when the class LaunchStock is 0, else +638h = clamp(n, 0,
// LaunchStock). No result.
int GameMissionLuaHost::run_set_catapult_stock_00892c30(lua_State* state, int argument_count) {
    ++summary_.set_catapult_stock_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) return 0;
    const std::size_t unit = static_cast<std::size_t>(id - 1);
    if (!units->unit_is_kind_of(unit, 5)) return 0;
    const std::int32_t requested = argument_count >= 2
        ? static_cast<std::int32_t>(::lua_tointeger(state, 2)) : 0;
    bsp::ShipCatapultClass cls;
    if (bsp::ShipCatapultRecord* r = catapult_record(unit, cls)) {
        r->slots.pending_638 = bsp::ship_catapult_stock_clamp_009539a0(
            r->slots.pending_638, requested, cls.launch_stock);
    }
    log_.implemented("MissionLuaNative::SetCatapultStock", "00892c30");
    return 0;
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
    bool posted = false;
    if (group >= 0) {
        // Packet cc13-jm08 (kDeadLeaderDirectorStateCopyBound): a unit whose own
        // 0071C150, or its group leader's, is still posted has its leave posted
        // behind it and the successor's 5Dh, as 0077C980 does in the image; it runs
        // at the next pump. Otherwise the leave stays synchronous.
        const std::size_t u = static_cast<std::size_t>(id - 1);
        posted = units->formation_leave_waits_for_handover(u)
            && units->post_formation_leave_0077c980(u);
        if (!posted) units->leave_group_on_destroy_0077bd70(u);
        ++summary_.leave_formation_left;
    }
    const GameUnitRow* row = valid ? units->unit_row(static_cast<std::size_t>(id - 1)) : nullptr;
    log_.notef("  LeaveFormation 00899eb0: \"%s\" group=%d left=%d (0077C980 -> 77h -> "
        "0077BD70, packet cc9_lua_formation_query)", row != nullptr ? row->name.c_str() : "?",
        static_cast<int>(group), group >= 0 ? 1 : 0);
    if (posted) {
        log_.notef("  LeaveFormation 00899eb0: leave posted behind a leader handover "
            "(packet cc13-jm08)");
    }
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

// Packet cc13-gunC C1b (include/bsp/ship_damaged_gfx_remove_cc13.hpp). 0088F710
// SetDamagedGFXLevel(entity, level): 00888AA0 resolves the entity, 00B66290 the level
// (lua_tonumber, FSTP to float, _ftol 00BF7420: (int)(float)x), then vtable[1B4h](level)
// at 0088F851; returns no value. SUBSTITUTION (labelled): an entity with no units-host
// slot is not reached (the image has no null test there).
int GameMissionLuaHost::run_set_damaged_gfx_level_0088f710(lua_State* state, int argument_count) {
    const int level = argument_count >= 2
        ? static_cast<int>(static_cast<float>(::lua_tonumber(state, 2))) : 0;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || units->gunnery() == nullptr || id <= 0
        || static_cast<std::size_t>(id) > units->count()) {
        log_.notef("  SetDamagedGFXLevel 0088f710: entity id %d has no units-host slot "
            "(packet cc13-gunC C1b)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    const int guns = units->gunnery()->set_damaged_gfx_level_00819cc0(index, level);
    const GameUnitRow* row = units->unit_row(index);
    log_.notef("  SetDamagedGFXLevel 0088f710: \"%s\" level=%d guns=%d (packet cc13-gunC C1b)",
        row != nullptr ? row->name.c_str() : "?", level, guns);
    log_.implemented("MissionLuaNative::SetDamagedGFXLevel", "0088f710");
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
            // table 00E08E58 indexes); the list test at this+1A4h..+1ACh that also
            // shortens the interval to 1e-4 is not modelled. Neither forced-11h arm
            // is modelled: 00988604..00988639 (with [[esi+538h]+510h] or +514h > 0.0,
            // 0098862A stores 11h and 00988639 sets BL = 1) and 0098864D..00988658
            // (the attacker's vtable[5Ch](17h) true, 00988658 stores 11h). The
            // null-shot arm 0098869E (kind 0) is moot: every host hit carries a class.
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
        int victim = static_cast<int>(hit.victim + 1);
        if (kLuaHitTargetSquadronBound) {
            // 00988530..0098859B: target = hitEntity->vtable[140h](), a plane's squadron.
            if (const bsp::PlaneSquadronHostRecord* sqn =
                    bsp::plane_squadron_registry().find_by_member_unit(hit.victim)) {
                if (sqn->squadron_unit != bsp::kPlaneSquadronNoUnit) {
                    victim = static_cast<int>(sqn->squadron_unit + 1);
                }
            }
        }
        const GameBulletClassRow* bullet = gunnery->bullet_class_row(hit.bullet_class);
        const std::string type = bullet != nullptr ? bullet->type : std::string();
        // Packet cc13-atkkind: the string the callback's argument 4 carries.
        const std::string subkind = hit_attack_type_name_00e08e58(hit.ordnance_kind);
        std::vector<std::string> callbacks;
        auto evaluate = [&](int shooter) {
        for (const ListenerEntry& entry : listeners_) {
            if (!listener_key_equal(entry.channel, "hit")) continue;
            auto holds = [](const std::vector<int>& set, int v) {
                return set.empty() || std::find(set.begin(), set.end(), v) != set.end();
            };
            if (!holds(entry.entity_ids, victim) || !holds(entry.attacker_ids, shooter)) continue;
            if (!entry.attack_types.empty()) {
                // Packet cc13-atkkind: both answers are taken, and a disagreement is
                // counted whichever the switch picks (log-only). The count is of
                // candidate disagreements per listener evaluation, taken before the
                // damage/device/player/fire/leak tests and on the squadron second pass
                // too; it is not a count of fires that would change.
                bool by_type = false;
                bool by_subkind = false;
                for (const std::string& t : entry.attack_types) {
                    if (listener_key_equal(t, type)) by_type = true;
                    if (listener_key_equal(t, subkind)) by_subkind = true;
                }
                if (by_type != by_subkind) ++summary_.listener_hit_attack_type_subkind_differs;
                bool any = false;
                if constexpr (kHitListenerAttackTypeSubKindCc13Bound) {
                    any = by_subkind;   // 00988B09: 00E08E58[[shot+4]+8]
                } else {
                    any = by_type;
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
        };
        int boxed_attacker = static_cast<int>(hit.shooter + 1);
        evaluate(boxed_attacker);
        if (kLuaHitSquadronSecondPassBound && callbacks.empty()) {
            // 0098901A: no subscription matched; 00989047..00989060: a plane
            // attacker is replaced by vtable[140h]() = plane+9D4h, its squadron,
            // and 009892E4 evaluates the channel again.
            for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
                if (r.squadron_unit == bsp::kPlaneSquadronNoUnit || r.squadron_unit == hit.shooter) continue;
                if (std::find(r.member_units.begin(), r.member_units.end(), hit.shooter)
                        == r.member_units.end()) {
                    continue;
                }
                boxed_attacker = static_cast<int>(r.squadron_unit + 1);
                evaluate(boxed_attacker);
                ++summary_.listener_hit_second_passes;
                if (!callbacks.empty()) ++summary_.listener_hit_second_pass_fires;
                break;
            }
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
            int pushed = 0;
            if (kLuaListenerCallbackArgsBound) {
                // Packet cc9_lua51_listener_args, 00988D87..00988F2C: target,
                // targetDevice, attacker (00884A90), attackType (00884FC0),
                // attackerPlayerIndex (00884A00), damage, fire, leak (008849D0).
                // SUBSTITUTIONS (labelled): the target is the hit unit (the image
                // boxes hitEntity->vtable[140h](), a plane's squadron); the device is
                // nil (the host builds no device entities); the two forced
                // attackType 11h arms are not modelled: 0098862A (BL = 1 at 00988639,
                // when [[esi+538h]+510h] or +514h > 0.0) and 00988658 (the attacker's
                // vtable[5Ch](17h) true). 0098869E's kind 0 for a null shot is moot,
                // since every host hit carries a class.
                if (!push_resolved_entity_by_id(state_, victim)) lua_pushnil(state_);
                lua_pushnil(state_);
                if (!push_resolved_entity_by_id(state_, boxed_attacker)) lua_pushnil(state_);
                // The attack-type name table 00E08E58 (char*[], read from the PE),
                // indexed by the ordnance kind [[src+4]+8], the event's ordnance_kind.
                ::lua_pushstring(state_, hit_attack_type_name_00e08e58(hit.ordnance_kind));
                ::lua_pushnumber(state_, static_cast<lua_Number>(hit.attacker_player_index));
                ::lua_pushnumber(state_, static_cast<lua_Number>(hit.damage));
                ::lua_pushnumber(state_, static_cast<lua_Number>(hit.fire_caused));
                ::lua_pushnumber(state_, static_cast<lua_Number>(hit.leak_caused));
                pushed = 8;
                ++summary_.listener_args_pushed;
            }
            if (::lua_pcall(state_, pushed, 0, 0) != 0) {
                const char* message = lua_tolstring(state_, -1, nullptr);
                note_error(message != nullptr ? message : std::string("(no message)"));
            }
            ::lua_settop(state_, top);
        }
        // Packet cc15-wB4: 009895D8..00989992, the default hit warning, inside the
        // arm past the +168h rate limit (00988969 JBE 009889AF) and after both
        // listener passes (00988F5D, 0098952A). Behind kWarningMessageBankCc15Bound.
        game_warning_report_hit_00989604(hit.victim, hit.shooter, hit.attacker_player_index);
    }
}

// Packet cc9_forced_recon_level. 008AA8F0 SetForcedReconLevel(entity, level,
// party): argument 0 through 00888AA0, arguments 1 and 2 as integers (00B66290),
// then 00805CF0(level) on the entity's record for that party (008AAA76); a squadron
// (18h) or LandConvoy (1Ah) then forces each +3CCh member as well (008AAAB8). The own
// record is forced only under kForcedReconOwnRecordBound.
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
    // 008AAA76: the entity's own record first (kForcedReconOwnRecordBound).
    if (kForcedReconOwnRecordBound) targets.push_back(index);
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
        if (kForcedReconSyncNotifyBound && party >= 0 && party < 3) {
            // 00805D02-00805D38: old = the effective level, new = the forced one; a
            // change notifies at once. The effective level is the one the pass
            // dispatch last published for (unit, party) (SUBSTITUTION, labelled).
            const std::size_t count = units->count();
            if (recon_listener_levels_.size() < count * 3) {
                recon_listener_levels_.resize(count * 3, 0);
            }
            int& last = recon_listener_levels_[unit * 3 + static_cast<std::size_t>(party)];
            const int now = level <= 0 ? 0 : level == 1 ? 1 : 2;
            if (now == last) continue;   // 00805D38 JE: no change, no notify
            const int old = last;
            last = now;
            ++summary_.forced_recon_sync_notifies;
            const GameGunneryHost* gunnery = units->gunnery();
            bool any_recon = false;
            for (const ListenerEntry& entry : listeners_) {
                if (listener_key_equal(entry.channel, "recon")) { any_recon = true; break; }
            }
            // 00980E50's live-unit gate, as in the pass dispatch.
            if (!kLuaListenersBound || !kLuaReconListenersBound || !any_recon ||
                state_ == nullptr || (gunnery != nullptr && gunnery->unit_dead(unit))) continue;
            fire_recon_listeners_00980e50(unit, old, now, party);
        }
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

// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 3). 008AF020
// IsInBorderZone(entity | position), 008AF020-008AF257, RET, ECX the Lua state.
// 008AF11E 008889C0 asks whether argument 0 is an entity handle: yes, 008AF153
// 00888AA0 and the translation +FCh/+100h/+104h (00414DB0 first when +C8h is
// clear); no, 008AF1AC 00888760. 008AF1EF 0071C4F0 with ECX = [00E188A8]; when
// it answers outside, 008AF1FF 00489C40 (same ECX) and 008AF20C 00B664B0 push
// the record's +4h, its side (0 or 1 after the twelve 004C7150 calls). Inside,
// nothing is pushed and the script reads nil. A side of 0 is a true value in Lua.
// SUBSTITUTIONS (labelled): the entity test is the host's `ID` field read (an
// entity table with an ID resolving to a units-host slot), and the translation is
// the slot's +FCh row; with no zone records the image would dereference null, the
// host pushes nothing and counts it.
int GameMissionLuaHost::run_is_in_border_zone_008af020(lua_State* state,
    int argument_count) {
    ++summary_.in_border_zone_calls;
    float position[3] = {0.0f, 0.0f, 0.0f};
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
        units->unit_position_00fc(static_cast<std::size_t>(id - 1), position[0], position[1],
            position[2]);
    } else if (argument_count >= 1) {
        read_vector3_00888760(state, 1, position);
    }
    const std::array<float, 3> p{position[0], position[1], position[2]};
    log_.implemented("MissionLuaNative::IsInBorderZone", "008af020");
    if (!border_zones_loaded_) {
        ++summary_.in_border_zone_no_zone;   // no Map bounds: read as inside
        return 0;
    }
    if (!bsp::point_outside_world_map_0071c4f0(border_bounds_, p)) return 0;
    ++summary_.in_border_zone_outside;
    const bsp::BorderZoneRecord* zone
        = bsp::border_zone_at_point_00489c40(border_bounds_, border_zones_, p);
    if (zone == nullptr) {
        ++summary_.in_border_zone_no_zone;
        return 0;
    }
    ::lua_pushinteger(state, static_cast<lua_Integer>(zone->side));
    log_.notef("  IsInBorderZone 008af020: (%.2f, %.2f, %.2f) outside the map, side %d "
        "(packet cc13_natives1)", static_cast<double>(p[0]), static_cast<double>(p[1]),
        static_cast<double>(p[2]), zone->side);
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

namespace {
// Packet cc13_natives1: the case-blind compares of 008132C0 (_stricmp, 00425850),
// 008135C0, 00953DA0 and 00953E60.
bool failure_name_equals(const std::string& name, const char* literal) {
    const std::size_t n = std::strlen(literal);
    if (name.size() != n) return false;
    for (std::size_t i = 0; i < n; ++i) {
        if (std::tolower(static_cast<unsigned char>(name[i]))
            != std::tolower(static_cast<unsigned char>(literal[i]))) return false;
    }
    return true;
}
// The class test 008C6DD0 uses for a ship (IsKindOf 4).
constexpr int kFailureShipKindOfClassId = 4;
}  // namespace

// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 1). 0088EFF0
// SetFailure(entity, name, ...), 0088EFF0-0088F1CB RET: 0088F0EE 00888AA0 on
// argument 0 (ESI, no null or class test), 0088F121 00B662B0 on argument 1 into a
// NativeString, 0088F14C CALL [vtable+194h](name, frame); no result. The ship
// vtables (eight, +194h = 008132C0):
//   "Explosion" (_stricmp): 008133A2..008133D0 read argument 2 and 3 as integers,
//     4 and 5 as numbers and 6 as an integer into message 6Ah +24h/+28h/+2Ch/+30h/
//     +34h (008133D5); 008198A0 schedules 00818110, whose step applies
//     vtable[1ACh](msg+30h). A call with no extra arguments explodes for 0.
//   "SteeringJam": message 6Bh only (008134C4) -> 0081999D unit+9E4h = 1. No
//     record and no duration: nothing in the image retires it.
//   "EngineJam": 00813530 007619B0 message 6Ch -> 008199C7 unit+9E5h = 1, then
//     0081356F 0093BD80 on unit+A20h pushes {5, "EngineJam", argument 2 when it is a
//     number, else 1e10 (00CE4970)}; 0093C520 runs it down and 00813660's 6Eh
//     clears the byte.
//   "Periscope": 00813595 009373C0 on [unit+1018h] unless +122Ch == 2.
// Other classes: 00953DA0 sends 7Dh for "InferiorFailure" (+720h); the airfield's
// 006D2210 tests RunwayFailure / HangarFailure first.
// SUBSTITUTIONS (labelled): the ship test is IsKindOf(4) through the units host,
// standing in for the eight vtables; the messages apply at the call, not on the
// session pump; the Explosion keeps only msg+30h; the non-ship arms are counted
// and not applied.
int GameMissionLuaHost::run_set_failure_0088eff0(lua_State* state, int argument_count) {
    ++summary_.set_failure_calls;
    const char* text = argument_count >= 2 ? ::lua_tostring(state, 2) : nullptr;
    const std::string name = text != nullptr ? text : "";
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || units->gunnery() == nullptr || id <= 0
        || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.set_failure_unresolved;
        log_.notef("  SetFailure 0088eff0: \"%s\" on entity id %d with no units-host slot "
            "(packet cc13_natives1)", name.c_str(), id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    int arm = 0;
    // Packet p11_carrier_runway_failure (bsp/mothership_failure.hpp): the
    // mothership's +194h 00758750 takes the two air-ops names before 008132C0;
    // the duration is not read. Arms 10 / 11.
    const bsp::MothershipFailureArm carrier_arm = bsp::kCarrierRunwayFailureBound
        && units->unit_is_kind_of(index, bsp::kMothershipKindOfClassId)
        ? bsp::mothership_failure_arm(name) : bsp::MothershipFailureArm::none;
    if (carrier_arm != bsp::MothershipFailureArm::none) {
        bsp::AirOpsDeck* deck = bsp::air_ops_decks().find_mutable_by_entity_id(id);
        bsp::mothership_apply_failure_message(deck, carrier_arm, true);
        arm = carrier_arm == bsp::MothershipFailureArm::runway ? 10 : 11;
        log_.notef("  SetFailure 0088eff0: mothership 00758750 %s on deck=%s "
            "(packet p11_carrier_runway_failure)",
            carrier_arm == bsp::MothershipFailureArm::runway ? "72h runway" : "74h hangar",
            deck != nullptr ? "yes" : "none");
    } else if (units->unit_is_kind_of(index, kFailureShipKindOfClassId)) {
        // 0093BD80: more than two arguments and argument 2 a number, else 1e10.
        const float seconds = argument_count >= 3 && ::lua_type(state, 3) == LUA_TNUMBER
            ? static_cast<float>(::lua_tonumber(state, 3)) : 1e10f;
        // 008133C5: argument 5, msg+30h; a missing argument reads 0.
        const float damage = argument_count >= 6
            ? static_cast<float>(::lua_tonumber(state, 6)) : 0.0f;
        arm = units->gunnery()->script_set_failure_008132c0(index, name, seconds, damage);
    }
    if (arm != 0) ++summary_.set_failure_applied;
    else ++summary_.set_failure_other;
    const GameUnitRow* row = units->unit_row(index);
    log_.notef("  SetFailure 0088eff0: \"%s\" on \"%s\" arm=%d (packet cc13_natives1)",
        name.c_str(), row != nullptr ? row->name.c_str() : "?", arm);
    log_.implemented("MissionLuaNative::SetFailure", "0088eff0");
    return 0;
}

// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 1). 0088F1D0
// GetFailure(entity, name), 0088F1D0-0088F3BE RET: 0088F2CE 00888AA0, 0088F301
// 00B662B0, 0088F331 CALL [vtable+198h](name, frame, &out) RET 0Ch, 0088F373
// 00B66450 with out's low byte; one result. `out` is never written before the call,
// so an unmatched name pushes a stale stack byte. Ship 008135C0: SteeringJam +9E4h,
// EngineJam +9E5h, Periscope +122Ch == 2, else 00953E60: InferiorFailure +720h.
// Airfield 006D2350: RunwayFailure +748h, HangarFailure +749h, else 00953E60.
// Mothership 007589D0 (vtable 00D01630 +198h): RunwayFailure +11A4h, HangarFailure
// +11A5h (air-ops block+1Ch / +1Dh), else 008135C0.
// SUBSTITUTIONS (labelled): an unmatched name answers false; Periscope answers false
// (not modelled); RunwayFailure and HangarFailure answer false unless
// kGetFailureRunwayHangarCc13Bound (bsp/get_failure_runway_hangar_cc13.hpp) reads the
// deck's flags; +720h is the units host's inferior arm of the airfield destruction rule.
int GameMissionLuaHost::run_get_failure_0088f1d0(lua_State* state, int argument_count) {
    ++summary_.get_failure_calls;
    const char* text = argument_count >= 2 ? ::lua_tostring(state, 2) : nullptr;
    const std::string name = text != nullptr ? text : "";
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    bool out = false;
    if (units != nullptr && id > 0 && static_cast<std::size_t>(id) <= units->count()) {
        const std::size_t index = static_cast<std::size_t>(id - 1);
        const bool ship = units->unit_is_kind_of(index, kFailureShipKindOfClassId);
        bool deck_arm = false;
        if constexpr (bsp::kGetFailureRunwayHangarCc13Bound) {
            // 007589D0 / 006D2350: the two names before 008135C0 / 00953E60, runway first.
            const bsp::MothershipFailureArm arm =
                units->unit_is_kind_of(index, bsp::kMothershipKindOfClassId)
                || units->unit_is_kind_of(index, static_cast<int>(bsp::kAirOpsClassIdAirfield))
                ? bsp::mothership_failure_arm(name) : bsp::MothershipFailureArm::none;
            if (arm != bsp::MothershipFailureArm::none) {
                deck_arm = true;
                // block+1Ch / +1Dh; no registered deck reads clear.
                const bsp::AirOpsDeck* deck = bsp::air_ops_decks().find_by_entity_id(id);
                out = deck != nullptr && (arm == bsp::MothershipFailureArm::runway
                    ? deck->runway_failure : deck->hangar_failure);
            }
        }
        if (deck_arm) {
        } else if (ship && units->gunnery() != nullptr && failure_name_equals(name, "SteeringJam")) {
            out = units->gunnery()->ship_jam_flag(index, true);
        } else if (ship && units->gunnery() != nullptr
                   && failure_name_equals(name, "EngineJam")) {
            out = units->gunnery()->ship_jam_flag(index, false);
        } else if (failure_name_equals(name, "InferiorFailure")) {
            out = units->inferior_failure_720(index);
        }
    }
    if (out) ++summary_.get_failure_true;
    ::lua_pushboolean(state, out ? 1 : 0);
    log_.implemented("MissionLuaNative::GetFailure", "0088f1d0");
    return 1;
}

// Packet cc13-fix12 (docs/CC13_FIX12.md), under kLuaIsUnitSelectableBound. 008AB580
// IsUnitSelectable(entity), a binding with no stack arguments: 008AB67F 00888AA0
// resolves argument 0 (ESI, null when unresolved); 008AB696 MOV DL,1 / MOV ECX,ESI /
// CALL 00645160, which is EAX = EDX; EDX = [[00E188A8]+18ECh]; PUSH EAX; CALL 00645060
// and adds nothing else; 008AB6A4 00B66450 PushBoolean(AL); one result. The predicate's
// inputs (selectable_inputs_00645060) already take the local player's team and a shut
// spectator door, which is what 00645060 does with allowSpectate 1 while game+2194h is
// clear (0064506A..0064507C). A null unit answers false at 00645082, as an unresolved
// argument does here. LABELLED: with no HUD host attached nothing is pushed (nil), the
// row's behaviour while unbound; the image always answers.
int GameMissionLuaHost::run_is_unit_selectable_008ab580(lua_State* state, int argument_count) {
    (void)argument_count;
    ++summary_.unit_selectable_calls;
    std::size_t unit = 0;
    if (!objective_argument_unit(state, 0, unit)) {
        ++summary_.unit_selectable_unresolved;
        ::lua_pushboolean(state, 0);
        log_.implemented("MissionLuaNative::IsUnitSelectable", "008ab580");
        return 1;
    }
    bool reached = false;
    const bool selectable = bsp::game::hud_unit_selectable_00645060(unit, reached);
    if (!reached) {
        ++summary_.unit_selectable_no_hud;
        log_.unimplemented("MissionLuaNative::IsUnitSelectable", "008ab580");
        return 0;
    }
    if (selectable) ++summary_.unit_selectable_true;
    ::lua_pushboolean(state, selectable ? 1 : 0);
    log_.implemented("MissionLuaNative::IsUnitSelectable", "008ab580");
    return 1;
}

namespace {
// 007B8E60 FLD [unit+0B1Ch]. +B1Ch is ctl+6Ch (ctl = unit+AB0h), stored at 007D807C by
// 007D8020 BSP_PlaneFlightController_UpdateSpeedTerms from the controller's world
// velocity ctl+18h, the host's plane_world_velocity (packet cc13-fix12c). That differs
// from motion.linear_velocity while the plane is deck-parented: 007C71E0's 007D9CE0
// (007C7277) leaves ctl+18h relative to the deck owner. The chain, main thread, 24-bit
// precision control, so each x87 op rounds to float: 007D8024-007D803B
// s2 = (y*y + x*x) + z*z, 007D803D FSTP float; 007D8041-007D804F FCOMI against the double
// 1e-10 at 00CE3820, JBE (s2 <= 1e-10 or unordered) -> 0.0 (007D8064 XORPS); else
// 007D8051 00BF7030 sqrt, 007D8056 FSTP float. LABELLED: computed when the native runs,
// where the image reads the value 007D8020 cached at the last 007D9C10 / 007D9C80 call.
// The chain is bsp::plane_cached_speed_b1c_007d8020 (plane_cached_speed_b1c_cc13.hpp),
// shared with the game_hosts_units.cpp readers (cc13-torprun).
bool plane_cached_speed_007b8e60(const GameUnitsHost& units, std::size_t index, float& out) {
    float v[3] = {0.0f, 0.0f, 0.0f};
    if (!units.unit_plane_world_velocity(index, v)) return false;
    out = bsp::plane_cached_speed_b1c_007d8020(v);
    return true;
}
}  // namespace

// Packet cc13-fix12 (docs/CC13_FIX12.md), under kLuaGetEntitySpeedBound. 008C0820
// GetEntitySpeed(entity), a binding with no stack arguments: 008C091F 00888AA0
// resolves argument 0; 008C0936..008C093D MOV EDX,[ESI] / MOV EAX,[EDX+38h] / CALL EAX
// with no null test; 008C093F FSTP float (the result is a float); 008C094F 00B66480
// pushes it; one result. Slot 38h by class, first match:
//   PlaneSquadronGen 00D087C0 slot 00D087F8 = 007ED470: ECX = [squadron+3D0h], JMP its
//     vtable[38h], so the leader plane's 007B8E60 (registry flight_leader(), slot 0);
//   the nine ship vtables, 0080E0F0: ECX = [unit+1018h], JMP 0092D730, the signed body
//     forward speed (GameUnitsHost::unit_forward_speed_0092d730);
//   the nine plane vtables, 007B8E60: FLD [unit+0B1Ch] (plane_cached_speed_007b8e60).
// LABELLED: an unresolved entity or a squadron with no leader faults or reads a
// stale slot in the image; other classes' slot-38h bodies were not read. Those
// push nothing (nil), the row's behaviour while unbound. No RNG draw.
int GameMissionLuaHost::run_get_entity_speed_008c0820(lua_State* state, int argument_count) {
    (void)argument_count;
    ++summary_.entity_speed_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.entity_speed_unresolved;
        log_.unimplemented("MissionLuaNative::GetEntitySpeed", "008c0820");
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    float speed = 0.0f;
    const bsp::PlaneSquadronHostRecord* squadron = nullptr;
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == index) {
            squadron = &r;
            break;
        }
    }
    if (squadron != nullptr) {
        const std::size_t leader = squadron->flight_leader();   // 007ED470 +3D0h
        if (leader == bsp::kPlaneSquadronNoUnit || leader >= units->count()
            || !plane_cached_speed_007b8e60(*units, leader, speed)) {
            ++summary_.entity_speed_unresolved;
            log_.unimplemented("MissionLuaNative::GetEntitySpeed", "008c0820");
            return 0;
        }
        ++summary_.entity_speed_squadron;
    } else if (units->unit_is_kind_of(index, bsp::kUnitKindQueryShip)) {
        speed = units->unit_forward_speed_0092d730(index);       // 0080E0F0
        ++summary_.entity_speed_ship;
    } else if (units->unit_is_kind_of(index, bsp::kUnitKindQueryPlane)
               && plane_cached_speed_007b8e60(*units, index, speed)) {
        ++summary_.entity_speed_plane;                           // 007B8E60
    } else {
        ++summary_.entity_speed_unmodelled;
        log_.unimplemented("MissionLuaNative::GetEntitySpeed", "008c0820");
        return 0;
    }
    ::lua_pushnumber(state, static_cast<lua_Number>(speed));     // 00B66480
    log_.implemented("MissionLuaNative::GetEntitySpeed", "008c0820");
    return 1;
}

// Packet cc13-fix18 (docs/CC13_FIX18.md section 2), under kLuaClearForcedReconLevelBound.
// 008AAB90 ClearForcedReconLevel(entity, party), __cdecl binding, no result (008AAD2D
// 00B66400). 008AAC90 00888AA0 resolves argument 0 with no null test; 008AACC1 00B66290
// reads argument 1, the observing party, unchecked (008AACD8 IMUL 34h). 008AACE2 00803B50
// on the entity's own record, unconditionally; then vtable[5Ch](18h), a squadron: each of
// the +3CCh members at +3D0h + 4i (a slot past the fifth reads a null base, 008AAD04),
// 008AAD16; otherwise vtable[5Ch](1Ah), a LandConvoy: each element of the +394h vector
// (008AAD83-008AADE4, the checked-iterator walk), 008AADD2. Host: the slot itself, then
// the live members of the squadron registry record whose fused slot it is (as
// run_set_forced_recon_level_008aa8f0 forces them). LABELLED: an unresolved entity faults
// in the image and is counted here; a LandConvoy's element vector is not carried (its own
// slot is cleared, as SetForcedReconLevel forces only that); 00803B50's notify reaches the
// listeners at the next records rebuild (bsp::clear_forced_recon_level_00803b50). No RNG.
int GameMissionLuaHost::run_clear_forced_recon_level_008aab90(lua_State* state,
    int argument_count) {
    ++summary_.clear_forced_recon_calls;
    const int party = argument_count >= 2 ? static_cast<int>(::lua_tonumber(state, 2)) : 0;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.clear_forced_recon_unresolved;
        log_.unimplemented("MissionLuaNative::ClearForcedReconLevel", "008aab90");
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    std::vector<std::size_t> targets{index};                     // 008AACE2
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit == bsp::kPlaneSquadronNoUnit || r.squadron_unit != index) continue;
        for (std::size_t member : r.member_units) {              // 008AAD16
            if (member != bsp::kPlaneSquadronNoUnit) targets.push_back(member);
        }
        break;
    }
    std::size_t was_forced = 0;
    for (std::size_t unit : targets) {
        ++summary_.clear_forced_recon_records;
        if (bsp::clear_forced_recon_level_00803b50(unit, party)) ++was_forced;
    }
    summary_.clear_forced_recon_was_forced += was_forced;
    const GameUnitRow* row = units->unit_row(index);
    log_.notef("  ClearForcedReconLevel 008aab90: \"%s\" party=%d records=%zu forced=%zu "
        "(00803B50, packet cc13-fix18)", row != nullptr ? row->name.c_str() : "?", party,
        targets.size(), was_forced);
    log_.implemented("MissionLuaNative::ClearForcedReconLevel", "008aab90");
    return 0;
}

namespace {
// 006E6210 __fastcall(ECX = crew level) -> skill, RET. m = ([00E188A8]+1FE4h != 0):
// 0 -> m (006E6214-006E6225), 1 -> 2m + 1 (006E623A), 2 -> 2m + 2 (006E6253), else
// (level == 3) * 4 + 1 (006E6258-006E6265). Single player here, so m = 0. The standalone
// body is bsp::skill_from_crew_level_006e6210 (src/cc13_seg33a.cpp, bsp_core), which reads
// the session word through a Seg33aHost this host does not implement.
int crew_level_skill_006e6210(int level) {
    const int m = 0;
    if (level == 0) return m;
    if (level == 1) return 2 * m + 1;
    if (level == 2) return 2 * m + 2;
    return (level == 3 ? 4 : 0) + 1;
}
}  // namespace

// Packet cc13-fix18 (docs/CC13_FIX18.md section 6), under kLuaSetCrewLevelBound. 00894F20
// SetCrewLevel(entity, level), __cdecl binding, no result (00895082 00B66400). 00895022
// 00888AA0 resolves argument 0; 00895053 00B66290 reads argument 1; 0089506C
// 006E6210(level); 0089507C entity->vtable[128h](skill), no null test. Slot 128h: ships
// and planes 006D1EB0 (the +38Ch sub-object's vtable[0]), the squadron 007ECF80 (every
// member's slot 128h, nothing on the squadron). Host: GameUnitsHost::
// set_unit_skill_index_006d1eb0 on the unit itself, or on each member of a squadron;
// not set_skill_level_007b8ae0, whose registry fan-out re-skills a plane's siblings
// and leaves a squadron's members alone (packet cc13-fix18b). LABELLED: an unresolved entity faults in the image and is
// counted here; the session word is taken as 0 (single player). No RNG.
int GameMissionLuaHost::run_set_crew_level_00894f20(lua_State* state, int argument_count) {
    ++summary_.crew_level_calls;
    const int level = argument_count >= 2 ? static_cast<int>(::lua_tonumber(state, 2)) : 0;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || id <= 0 || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.crew_level_unresolved;
        log_.unimplemented("MissionLuaNative::SetCrewLevel", "00894f20");
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    const int skill = crew_level_skill_006e6210(level);             // 0089506C
    // 0089507C vtable[128h]. The squadron (the registry record whose fused slot this
    // is, as in run_clear_forced_recon_level_008aab90) takes 007ECF80: i < +3CCh,
    // [+3D0h+4i]->vtable[128h](skill) (007ECF86-007ECFB9), and stores nothing on the
    // squadron itself. A ship or a plane, a squadron member included, takes 006D1EB0
    // on itself only.
    std::size_t written = 0;
    const bsp::PlaneSquadronHostRecord* squadron = nullptr;
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == index) {
            squadron = &r;
            break;
        }
    }
    if (squadron != nullptr) {
        for (std::size_t member : squadron->member_units) {         // 007ECFAB
            if (member == bsp::kPlaneSquadronNoUnit) continue;
            if (units->set_unit_skill_index_006d1eb0(member, skill)) ++written;
        }
    } else if (units->set_unit_skill_index_006d1eb0(index, skill)) {
        ++written;                                                  // 006D1EB0
    }
    const GameUnitRow* row = units->unit_row(index);
    log_.notef("  SetCrewLevel 00894f20: \"%s\" level=%d skill=%d units=%zu (006E6210, "
        "vtable[128h], packet cc13-fix18)", row != nullptr ? row->name.c_str() : "?", level,
        skill, written);
    log_.implemented("MissionLuaNative::SetCrewLevel", "00894f20");
    return 0;
}

// Packet cc13-smbind. 0088C750 BannSupportmanager() and 0088C8E0 PermitSupportmanager(),
// __cdecl bindings that read no argument and push nothing (00B66400). Both are
// bsp::seg56_options_cc13::lua_bann/permit_support_manager (src/options_seg56_cc13.cpp),
// whose Seg56Host this host does not implement. 0088C82E MOV CL,BL (EBX = 1, 0088C76F) /
// 0088C9BE XOR CL,CL, then CALL 00A38DA0 (session_spawn_player_vehicles_00a38da0,
// src/cc13_sweepC2.cpp): [00F8AB6A] = CL; 00652A50(CL) and 00652A40(CL) on the panel
// ([00E198C4]+BCh); when banning, 0068B3F0 on the interface and, outside session mode 2
// and only in effective game mode 4 (004BCA50), the player-slot refill. Then, when
// [[00E188A8]+1FE4h] == 1, message D7h (0075B430, byte +18h = banned) goes out through
// 0076A9F0. Host: the flag is stored in support_manager_banned_00f8ab6a_. LABELLED: the
// panel setters and 0068B3F0 are not applied (this process has no support-manager panel);
// the refill does not run (a single-player campaign mission's effective mode is 8,
// game_hosts_hud.hpp) and no message is sent (session word 0, single player, as
// crew_level_skill_006e6210 assumes). LABELLED: the image's reader, the 00A25B90
// SpawnGroup gate (bsp::planner_quick_spawn_00a25b90; 00A25BAB returns when banned), is
// not applied, because the host's planner spawn (game_hosts_ai.cpp:3992 ->
// AiPlanners::quick_spawn_named_group :5941) only records. Whoever binds that spawn reads
// the byte through support_manager_banned_00f8ab6a(). No RNG.
int GameMissionLuaHost::run_support_manager_set_banned_0088c750_0088c8e0(bool banned) {
    if (banned) {
        ++summary_.support_manager_bann_calls;
    } else {
        ++summary_.support_manager_permit_calls;
    }
    support_manager_banned_00f8ab6a_ = banned ? 1 : 0;              // 00A38DCF / 00A39003 arm
    if (banned) {
        log_.notef("  BannSupportmanager 0088c750: [00F8AB6A]=1 (00A38DA0, packet cc13-smbind)");
        log_.implemented("MissionLuaNative::BannSupportmanager", "0088c750");
    } else {
        log_.notef("  PermitSupportmanager 0088c8e0: [00F8AB6A]=0 (00A38DA0, packet cc13-smbind)");
        log_.implemented("MissionLuaNative::PermitSupportmanager", "0088c8e0");
    }
    return 0;
}

// Packet cc13-smbind. 0088CBB0 SupportManagerEnable(entity, enable), __cdecl binding, no
// result (0088CD24 00B66400). bsp::seg56_options_cc13::lua_support_manager_enable.
// 0088CCAE 00888AA0 resolves argument 0 (no null test); 0088CCE0 00B66250 reads argument 1
// (lua_toboolean, no type gate). Enable: 0088CD08 0065BBA0 on the panel ([00E198C4]+BCh),
// which is 0065B1F0, std::list::remove of every node whose +8h is the entity from the list
// at +1F8h. Disable: 0088CD1B 00665EF0 appends the entity to that list. Host: the list is
// support_panel_disabled_list_1f8_, keyed by the entity's ID. LABELLED: an unresolved
// entity is counted and nothing is stored (the image would edit the list with whatever
// 00888AA0 returned); nothing in this process reads the list. No RNG.
int GameMissionLuaHost::run_support_manager_enable_0088cbb0(lua_State* state,
    int argument_count) {
    ++summary_.support_manager_enable_calls;
    const bool enable = argument_count >= 2 && lua_toboolean(state, 2) != 0;
    const int id = air_ops_entity_id(state);
    if (id <= 0) {
        ++summary_.support_manager_enable_unresolved;
        log_.unimplemented("MissionLuaNative::SupportManagerEnable", "0088cbb0");
        return 0;
    }
    std::vector<int>& list = support_panel_disabled_list_1f8_;
    if (enable) {
        list.erase(std::remove(list.begin(), list.end(), id), list.end());  // 0065B1F0
    } else {
        list.push_back(id);                                                  // 00665EF0
    }
    log_.notef("  SupportManagerEnable 0088cbb0: id=%d enable=%d list=%zu (%s, packet "
        "cc13-smbind)", id, enable ? 1 : 0, list.size(), enable ? "0065BBA0" : "00665EF0");
    log_.implemented("MissionLuaNative::SupportManagerEnable", "0088cbb0");
    return 0;
}

// Packet cc13-fix11 (docs/CC13_FIX11.md), under kLuaSetDeadMeatBound. 008AC7B0
// SetDeadMeat(entity), 008AC7B0-008AC9C2, a binding with no stack arguments (RET).
// The standalone body is bsp::lua_set_dead_meat (src/options_seg56_cc13.cpp); its
// Seg56Host has 86 pure methods this host does not implement, so its three arms
// are carried here in the image's order:
//   008AC8AF 00888AA0 resolves argument 0 (no null or class test);
//   008AC8CF vtable[5Ch](0Fh), a plane: 008AC8FB vtable[194h]("spinning" 00D059EC,
//     0), the plane's 007BBFA0;
//   008AC92E vtable[5Ch](6), a ship: 008AC94B 00BD2F10(ECX = 1, -1.0f [00D7A260],
//     1.0f FLD1) stored as pivot x with y = z = 0, then 008AC96A 008110F0(&pivot).
//     008110F0 never reads the pivot (docs/UNIT_DAMAGE_AND_DEATH.md "Sink"), so the
//     draw only advances the stream. 008110F0 returns on +5Dh (008110F7) or on
//     +150h > 0 (00811101 COMISS / JA: a NaN does not refuse), else 00811111
//     vtable[70h](1) = 0077D1A0, then zeroes +828h / +82Ch and releases +740h;
//   anything else: 008AC97A vtable[70h](1).
// 008AC980 00B66400: nothing pushed, the result count is 0.
// SUBSTITUTIONS (labelled):
// * the class tests are the units host's IsKindOf(0Fh) and IsKindOf(6) on the slot;
//   a squadron's slot is fused with its wing-0 plane, so it answers as that plane
//   rather than taking the image's else arm on the squadron object;
// * an entity with no units-host slot is counted and nothing runs (the image would
//   run the else arm's vtable[70h](1) on it);
// * the plane arm is counted and NOT applied: the host runs 007BBFA0's "spinning"
//   only inside the units host's death-mode block (007CA8A0 -> 007BBFA0 -> 007D0B80
//   on a plane the gunnery host already killed) and has no entry for a live plane,
//   so the arm's 007BC1F2 stream-1 U(0, 1) draw is not taken either;
// * the draw is GameGunneryHost::death_mode_draw_00bd2f10(1, ...): by default the
//   shared stream-1 generator in call order, as in the image; under the measurement
//   option BSP_GUNNERY_RNG_STREAMS=1 it is keyed (death_mode, unit), which no other
//   ship draw uses;
// * +5Dh is the gunnery host's unit_dead() (dead, or health <= 0), +150h its stored
//   invincibility (unit_invincibility, read whatever the floor switch says);
// * vtable[70h](1), for the sink and for the else arm, is
//   GameGunneryHost::destroy_unit_0077d1a0(index, 1), the host's death funnel; its
//   own counter (summary "airfield destroy kills") includes these calls, and the
//   0077D1E4 destroy message, +828h / +82Ch and the +740h release are not held.
int GameMissionLuaHost::run_set_dead_meat_008ac7b0(lua_State* state, int argument_count) {
    (void)argument_count;
    ++summary_.dead_meat_calls;
    GameUnitsHost* units = units_hooks_;
    const int id = air_ops_entity_id(state);
    if (units == nullptr || units->gunnery() == nullptr || id <= 0
        || static_cast<std::size_t>(id) > units->count()) {
        ++summary_.dead_meat_unresolved;
        log_.notef("  SetDeadMeat 008ac7b0: entity id %d has no units-host slot, nothing run "
            "(packet cc13-fix11)", id);
        return 0;
    }
    const std::size_t index = static_cast<std::size_t>(id - 1);
    GameGunneryHost& gunnery = *units->gunnery();
    const GameUnitRow* row = units->unit_row(index);
    const char* name = row != nullptr ? row->name.c_str() : "?";
    if (units->unit_is_kind_of(index, 0x0F)) {                 // 008AC8CF
        ++summary_.dead_meat_plane_unapplied;
        log_.notef("  SetDeadMeat 008ac7b0: plane \"%s\" vtable[194h](\"spinning\", 0) not "
            "applied, no host entry for 007BBFA0 on a live plane (packet cc13-fix11)", name);
    } else if (units->unit_is_kind_of(index, 6)) {             // 008AC92E
        const float pivot_x = gunnery.death_mode_draw_00bd2f10(1, index, -1.0f, 1.0f);  // 008AC94B
        const bool refused = bsp::sink_is_refused_008110f0(gunnery.unit_dead(index),
            gunnery.unit_invincibility(index));                // 008110F7 / 00811108
        if (refused) {
            ++summary_.dead_meat_sink_refused;
        } else {
            gunnery.destroy_unit_0077d1a0(index, 1);           // 00811111
            ++summary_.dead_meat_sunk;
        }
        log_.notef("  SetDeadMeat 008ac7b0: ship \"%s\" pivot_x=%.4f sink=%s (008110F0, packet "
            "cc13-fix11)", name, static_cast<double>(pivot_x), refused ? "refused" : "destroyed");
    } else {
        gunnery.destroy_unit_0077d1a0(index, 1);               // 008AC97A
        ++summary_.dead_meat_destroyed;
        log_.notef("  SetDeadMeat 008ac7b0: \"%s\" vtable[70h](1) (packet cc13-fix11)", name);
    }
    log_.implemented("MissionLuaNative::SetDeadMeat", "008ac7b0");
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
        if constexpr (bsp::game::kLandConvoyBodiesBound) {
            // Packet cc13-convoy: 008AC740 IsKindOf 1Ah -> 008AC74E 00742210(flag), then
            // 008AC753 JMPs past 008AC756's 00926D90. A LandConvoy is a scene marker
            // here (no units-host slot); it is named as run_land_convoy_native names it.
            // SUBSTITUTION (labelled): "a convoy roster of that name exists" stands for
            // the IsKindOf 1Ah test.
            std::string name;
            if (::lua_type(state, 1) == LUA_TTABLE) {
                lua_getfield(state, 1, "Name");
                if (const char* text = ::lua_tostring(state, -1)) name = text;
                ::lua_settop(state, ::lua_gettop(state) - 1);
            }
            if (name.empty() && id > 0) {
                if (const PendingEntity* node = find_pending(id)) name = node->name;
            }
            // 008AC748 CMP ESI,EBP / SETNE: EBP is 0 (008AC5E3 XOR EBP,EBP, not written
            // again before 008AC796) and the cause is 1 or 2, so the flag is always true
            // and 00742210 kills every member with cause 2, whatever `hard` was.
            const bool vector_hard = cause != 0;
            std::vector<GameUnitsHost::LandConvoyVictim> victims;
            if (units != nullptr && units->gunnery() != nullptr && !name.empty()
                && units->land_convoy_kill_member_vector_00742210(name, vector_hard, victims)) {
                GameGunneryHost& gunnery = *units->gunnery();
                for (const GameUnitsHost::LandConvoyVictim& victim : victims) {
                    if (gunnery.unit_dead(victim.unit)) {
                        ++summary_.kill_already_dead;    // 00926D90 returns on +5Fh
                        continue;
                    }
                    if constexpr (kKillListenerHarmOnlyBound) lua_kill_cause_[victim.unit] = victim.cause;
                    gunnery.kill_unit_00926d90(victim.unit, victim.cause);
                    if constexpr (kLuaProximityTriggersBound) target_gone_00925c40(victim.unit);
                    ++summary_.kill_units;
                }
                log_.notef("  Kill 008ac5c0: LandConvoy \"%s\" Lua cause=%d, members=%zu through "
                    "00742210 with cause 2, the convoy not killed (packet cc13-convoy)", name.c_str(),
                    cause, victims.size());
                log_.implemented("MissionLuaNative::Kill", "008ac5c0");
                return 0;
            }
        }
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
        if constexpr (bsp::kRackedRoundEntityCc13Bound) {
            // Packet cc13-rackround (include/bsp/racked_round_entity_cc13.hpp): a
            // racked torpedo round GetSquadTorpedoes handed out. 008AC756's 00926D90
            // releases the round, so the rack holds none (kill_racked_round). A
            // second Kill of the same round returns on +5Fh.
            if (units != nullptr && ::lua_type(state, 1) == LUA_TTABLE) {
                ::lua_getfield(state, 1, "Ptr");
                void* const ptr = ::lua_touserdata(state, -1);
                ::lua_settop(state, -2);
                if (bsp::RackedRoundProxy* const round = bsp::find_racked_round_proxy(ptr)) {
                    int before = 0;
                    const bool emptied = !round->dead
                        && units->kill_racked_round(round->plane, round->rack, before);
                    log_.notef("  Kill 008ac5c0: racked round of unit %zu rack %d, %s (00926D90, "
                        "packet cc13-rackround)", round->plane, round->rack,
                        round->dead ? "already dead" : (emptied ? "rack emptied" : "rack already empty"));
                    round->dead = true;
                    log_.implemented("MissionLuaNative::Kill", "008ac5c0");
                    return 0;
                }
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
    // convoy's own slot stands for it. A LandConvoy has no units-host slot (it is
    // a scene marker), so it never reaches here; under kLandConvoyBodiesBound the
    // no-slot arm above runs 00742210 on its roster (packet cc13-convoy).
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
        if constexpr (kKillListenerHarmOnlyBound) lua_kill_cause_[unit] = cause;  // cc13-ver8
        gunnery.kill_unit_00926d90(unit, cause);
        // Packet cc13-fix19: 00926D90 put the unit on the kill list, whose pass K
        // notifies its observers at row 15 (target_gone_drain_009273a0).
        if constexpr (kLuaProximityTriggersBound) target_gone_00925c40(unit);
        ++summary_.kill_units;
    }
    log_.notef("  Kill 008ac5c0: \"%s\" cause=%d victims=%zu (packet cc9_lua_kill)",
        row != nullptr ? row->name.c_str() : "?", cause, victims.size());
    log_.implemented("MissionLuaNative::Kill", "008ac5c0");
    return 0;
}

// Packet cc13-dead. 00929800 on a LandConvoy marker (include/bsp/land_convoy_dead_cc13.hpp
// has the chain from 00926D90). SUBSTITUTIONS (labelled): the convoy is found by name
// in the FindEntity name index (the host's convoys are keyed by name; the members
// carry "-I", "-II" suffixes, so the name is the convoy marker's). The pending list is no source: InitAll's 00926335 clears it,
// so after load it never holds a convoy (cc9-gunnery53: the first ON pair found no
// self table for JM05's RadarStationLandConvoy 01, attached at load through
// 0074347E -> 0077E830 -> 00928A00). The name index keeps the last entity of a
// name, so two convoys sharing a name can pair the wrong objects; the image
// follows its own pointer. The own-slot test (`Ptr` is the marker id, as the 00928A00 seed writes it)
// stands for 00929800's +178h test. OMITTED (labelled): 009298DB's think call and the
// 00928300 think-list drops, for a marker with a think registration (+1D8h).
bool GameMissionLuaHost::publish_land_convoy_dead_00929800(const std::string& convoy, int cause) {
    if constexpr (!bsp::game::kLandConvoyDeadWired) {
        static_cast<void>(convoy);
        static_cast<void>(cause);
        return false;
    } else {
        if (state_ == nullptr) return false;
        int id = 0;
        const std::map<std::string, int>::const_iterator found = scene_entity_ids_.find(convoy);
        if (found != scene_entity_ids_.end()) id = found->second;
        if (id <= 0 || !push_resolved_entity_by_id(state_, id)) {
            log_.notef("  LandConvoy \"%s\": 00929800 found no self table, Dead not written "
                "(packet cc13-dead)", convoy.c_str());
            return false;
        }
        lua_getfield(state_, -1, "Ptr");
        const bool own_slot = lua_touserdata(state_, -1)
            == reinterpret_cast<void*>(static_cast<std::uintptr_t>(id));
        ::lua_settop(state_, ::lua_gettop(state_) - 1);
        if (own_slot) {
            // 00E0CF04[+70h] (009298C2): none, harm, soft, sell, exitzone, landed, editor.
            static const char* const kKillReasons[] = {
                "none", "harm", "soft", "sell", "exitzone", "landed", "editor"};
            // SUBSTITUTION (labelled): the image indexes the table unchecked and
            // reads past it for an out-of-range cause; here such a cause reads
            // "harm" (none reaches here: 007422A0 passes 2).
            const char* const reason = (cause >= 0 && cause < 7) ? kKillReasons[cause] : "harm";
            lua_pushboolean(state_, 1);
            lua_setfield(state_, -2, "Dead");                 // 00929896
            ::lua_pushstring(state_, reason);
            lua_setfield(state_, -2, "KillReason");           // 009298C2
            log_.notef("  LandConvoy \"%s\": marker id %d Dead=true KillReason=%s (00929800 "
                "via 0077D270 -> 00929B60, packet cc13-dead)", convoy.c_str(), id, reason);
            log_.implemented("MissionEntity::on_killed_lua_00929800", "00929800");
        }
        ::lua_settop(state_, ::lua_gettop(state_) - 1);
        return own_slot;
    }
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
    note_vehicle_class_read(index);   // cc15-vcp1 census
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

bool GameMissionLuaHost::read_vehicle_class_part_anim(int index, const char* key,
    float& first, float& second, float& third, bool& has_third) {
    first = 0.0f;
    second = 0.0f;
    third = 0.0f;
    has_third = false;
    if (state_ == nullptr || index < 0 || key == nullptr) return false;
    const int top = ::lua_gettop(state_);
    bool present = false;
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_pushinteger(state_, index);
        ::lua_gettable(state_, -2);
        if (::lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, "PartAnims");
            if (::lua_type(state_, -1) == LUA_TTABLE) {
                ::lua_getfield(state_, -1, key);
                if (::lua_type(state_, -1) == LUA_TTABLE) {
                    present = true;
                    float* const slots[3] = {&first, &second, &third};
                    for (int i = 0; i < 3; ++i) {
                        ::lua_rawgeti(state_, -1, i + 1);
                        if (::lua_type(state_, -1) == LUA_TNUMBER) {
                            *slots[i] = static_cast<float>(::lua_tonumber(state_, -1));
                            if (i == 2) has_third = true;
                        }
                        ::lua_pop(state_, 1);
                    }
                }
            }
        }
    }
    ::lua_settop(state_, top);
    return present;
}

float GameMissionLuaHost::read_vehicle_class_number(int index, const char* key,
    float fallback) {
    // The same plain `VehicleClass[index][key]` lookup as
    // read_vehicle_class_integer, kept as a number. 009623A9's caller squares
    // the result; this reader does not, because the Lua field is the modifier
    // and class+B8h is its square.
    if (state_ == nullptr || index < 0 || key == nullptr) return fallback;
    note_vehicle_class_read(index);   // cc15-vcp1 census
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

bool GameMissionLuaHost::read_vehicle_class_boolean(int index, const char* key,
    bool fallback) {
    if (state_ == nullptr || index < 0 || key == nullptr) return fallback;
    const int top = ::lua_gettop(state_);
    bool value = fallback;
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "VehicleClass");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_pushinteger(state_, index);
        ::lua_gettable(state_, -2);
        if (::lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, key);
            if (::lua_type(state_, -1) == LUA_TBOOLEAN) {
                value = ::lua_toboolean(state_, -1) != 0;
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

float GameMissionLuaHost::read_bullet_class_nested_number(int index, const char* key,
    const char* nested_key, float fallback) {
    // Packet cc9_lua53_kamikaze_impact: Bullets[index].Blast.BlastDamageMin and
    // its siblings, one table deeper than read_bullet_class_number reaches.
    if (state_ == nullptr || index < 0 || key == nullptr || nested_key == nullptr) {
        return fallback;
    }
    const int top = ::lua_gettop(state_);
    float value = fallback;
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "Bullets");
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
    note_vehicle_class_read(index);   // cc15-vcp1 census
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

float GameMissionLuaHost::read_device_class_number(int index, const char* key,
                                                  float fallback) {
    // The number companion of read_device_class_string (packet
    // cc9_plane_level_bomb_task: a bomb platform's `RepeatTime`, desc+E0h).
    float value = fallback;
    if (state_ == nullptr || index < 0 || key == nullptr) return value;
    const int top = ::lua_gettop(state_);
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "DeviceClass");
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

bool GameMissionLuaHost::read_device_class_bool(int index, const char* key, bool fallback) {
    bool value = fallback;
    if (state_ == nullptr || index < 0 || key == nullptr) return value;
    const int top = ::lua_gettop(state_);
    ::lua_getfield(state_, LUA_GLOBALSINDEX, "DeviceClass");
    if (::lua_type(state_, -1) == LUA_TTABLE) {
        ::lua_pushinteger(state_, index);
        ::lua_gettable(state_, -2);
        if (::lua_type(state_, -1) == LUA_TTABLE) {
            ::lua_getfield(state_, -1, key);
            if (::lua_type(state_, -1) == LUA_TBOOLEAN) {
                value = ::lua_toboolean(state_, -1) != 0;
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
        if constexpr (kLuaObjectFieldSettersCc13Bound) {
            NativeLuaStateStorage owner;
            construct_native_lua_state_00b66bd0(&owner);
            owner.state_04 = state_;
            NativeLuaObjectStorage globals;
            native_lua_globals_00b67980(owner, &globals);
            const ScopedNativeKey key(name);
            native_lua_set_nil_00b67350(globals, key.key);
        } else {
            lua_pushnil(state_);
            lua_setfield(state_, LUA_GLOBALSINDEX, name);
        }
        nil_global_ = true;
        log_.implemented("MissionLua::lobby_settings_set_global_nil", "00b67350");
    }
    void set_global_new_table(const char* name) override {
        if (state_ == nullptr) return;
        if constexpr (kLuaObjectFieldSettersCc13Bound) {
            // 00B67580 creates the table with (0, 0); the host's presize is not
            // observable from Lua.
            NativeLuaStateStorage owner;
            construct_native_lua_state_00b66bd0(&owner);
            owner.state_04 = state_;
            NativeLuaObjectStorage globals;
            native_lua_globals_00b67980(owner, &globals);
            const ScopedNativeKey key(name);
            native_lua_set_new_table_00b67580(globals, key.key);
        } else {
            lua_createtable(state_, 0, static_cast<int>(bsp::kLobbySettingsFieldCount));
            lua_setfield(state_, LUA_GLOBALSINDEX, name);
        }
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
        if constexpr (bsp::kSkirmishSessionCc13Bound) {
            // Packet cc13-mp3 (MP3c): 005E2FE2's store to 00E0C978, which the powerup
            // manager's gate reads.
            bsp::lobby_session_state().powerups_enabled_00e0c978 = flags.powerups_enabled;
        }
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
        if constexpr (bsp::kSkirmishSessionCc13Bound) {
            // Packet cc13-mp3 (MP3c): a --skirmish run loads the registry (008D2F50).
            const bsp::LobbyOptionRegistry& registry = bsp::lobby_session_state().registry;
            if (registry.loaded) return registry.option_number(slot, option);  // 00575FD0
            log_.unimplemented("MissionLua::lobby_settings_option_number", "008d2f50");
            return 0;
        } else {
            static_cast<void>(slot);
            static_cast<void>(option);
            log_.unimplemented("MissionLua::lobby_settings_option_number", "008d2f50");
            return 0;
        }
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
    // From the session context (packet cc13-mp2c). Campaign identity: the OFF arm is
    // the old literals false, false, 8.
    if constexpr (bsp::kSkirmishSessionCc13Bound) {
        const bsp::SessionContext& context = bsp::session_context();
        state.network_session = context.session_word_1fe4 != 0;
        state.game_mode_forced = context.forced_61c;
        state.effective_game_mode = bsp::effective_game_mode_004bca50(context);
        // Packet cc13-mp3 (MP3c): 00E1955C as the lobby left it (zero in a campaign run).
        const bsp::LobbySessionState& lobby = bsp::lobby_session_state();
        for (std::size_t slot = 0; slot < bsp::kLobbySettingsStoredOptionCount; ++slot) {
            state.stored_option[slot] = lobby.stored_options_00e1955c[slot];
        }
    } else {
        state.network_session = false;      // game+1FE4h
        state.game_mode_forced = false;     // game+61Ch
        state.effective_game_mode = 8;      // 004bca50, the single-player campaign
    }
    LobbySettingsBinding sync(state_, log_);
    LobbySettingsOptionsBinding options(log_);
    bsp::lobby_settings_sync_005e2f00(sync, options, state);
    summary_.lobby_settings_published = sync.table_created();
    log_.implemented("MissionLua::sync_lobby_settings", "005e2f00");
    if constexpr (bsp::kSkirmishSessionCc13Bound) {
        // Packet cc13-skirmish-gate: 005E2F59 sets the global nil on every call, the
        // published path included, so nil_global() is always true; the table is
        // what tells the 005E2F93 early-out from a publish.
        log_.notef("LobbySettings: %s (game+1FE4h = %d), fields nil=%zu number=%zu "
            "string=%zu; mode flags powerups=%d reload_payload=%d map=%d, command points %.1f",
            sync.table_created() ? "a table was published"
                                 : "the global is left nil by the 005e2f93 early-out",
            static_cast<int>(bsp::session_context().session_word_1fe4),
            sync.nil_fields(), sync.number_fields(), sync.string_fields(),
            sync.flags().powerups_enabled ? 1 : 0, sync.flags().reload_payload_on ? 1 : 0,
            sync.flags().map_enabled ? 1 : 0, static_cast<double>(sync.command_points()));
    } else {
        log_.notef("LobbySettings: %s (single player, game+1FE4h = 0), fields nil=%zu number=%zu "
            "string=%zu; mode flags powerups=%d reload_payload=%d map=%d, command points %.1f",
            sync.nil_global() ? "the global is set nil by the 005e2f93 early-out"
                              : "a table was published",
            sync.nil_fields(), sync.number_fields(), sync.string_fields(),
            sync.flags().powerups_enabled ? 1 : 0, sync.flags().reload_payload_on ? 1 : 0,
            sync.flags().map_enabled ? 1 : 0, static_cast<double>(sync.command_points()));
    }
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

int GameMissionLuaHost::mission_checkpoint_binding(lua_State* state, bool save) {
    // The id: the mission script's file stem (SUBSTITUTION for game+2198h).
    std::string id = summary_.mission_script_path;
    const std::size_t slash = id.find_last_of("/\\");
    if (slash != std::string::npos) id.erase(0, slash + 1);
    const std::size_t dot = id.rfind('.');
    if (dot != std::string::npos) id.erase(dot);
    if (!save) {
        // 008ACB22 CALL 004379C0, then 008ACB2B the result count.
        const int top = ::lua_gettop(state);
        try {
            const std::optional<float> play_time = bsp::load_checkpoint_004379c0(state, id, state);
            log_.notef("LoadCheckpoint 008ACA30: id=%s %s", id.c_str(),
                play_time ? "table pushed" : "nil pushed");
        } catch (const std::exception& e) {
            ::lua_settop(state, top);
            ::lua_pushnil(state);
            log_.notef("LoadCheckpoint 008ACA30: id=%s failed: %s", id.c_str(), e.what());
        }
        log_.implemented("MissionLuaNative::LoadCheckpoint", "008aca30");
        return ::lua_gettop(state) - top;
    }
    const int top = ::lua_gettop(state);
    if (top >= 1 && ::lua_type(state, 1) == LUA_TTABLE) {
        bsp::CheckpointRecordFields record;   // 00436690's defaults
        record.difficulty = bsp::game::game_effective_difficulty_6ac();
        bsp::ArchiveStringSink sink;
        bsp::ArchiveTextOutput output;
        output.buffered_0109cecc = &sink;
        bsp::ArchiveTextWriter writer(output);
        try {
            bsp::save_checkpoint_00438020(state, id, record, 0.0f, state, 1, &writer, true);
            // Beside the executable only: the build or export tree.
            char* program = nullptr;
            std::string path;   // empty: no executable path, nothing written
            if (_get_pgmptr(&program) == 0 && program != nullptr) {
                std::string dir(program);
                const std::size_t cut = dir.find_last_of("/\\");
                if (cut != std::string::npos) path = dir.substr(0, cut + 1) + "checkpoint_" + id + ".txt";
            }
            std::FILE* f = nullptr;
            if (!path.empty() && fopen_s(&f, path.c_str(), "wb") == 0 && f != nullptr) {
                std::fwrite(sink.str().data(), 1, sink.str().size(), f);
                std::fclose(f);
            }
            log_.notef("SaveCheckpoint 008ACB70: id=%s archive %zu bytes -> %s", id.c_str(),
                sink.str().size(), path.c_str());
        } catch (const std::exception& e) {
            log_.notef("SaveCheckpoint 008ACB70: id=%s failed: %s", id.c_str(), e.what());
        }
    } else {
        log_.notef("SaveCheckpoint 008ACB70: argument 0 is not a table; nothing saved");
    }
    ::lua_settop(state, top);
    ::lua_pushboolean(state, 1);   // 008ACB70's PushBoolean(1)
    log_.implemented("MissionLuaNative::SaveCheckpoint", "008acb70");
    return 1;
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
        // Packet cc9_shipyard_scene_attach: the shipyard records likewise.
        if (kShipyardSceneAttachBound) bsp::shipyards().bind_entity_id(entity.id, entity.name);
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
            mark_vehicle_class_row_00964790(entity.class_index);   // kVehicleClassRowIdBound
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
        "cc9_generated_entity_party, 00944FD0 -> 00928A00): bound=%d nodes=%llu writes=%llu "
        "launched_party_nodes=%llu launched_wing_nodes=%llu spawn_squadron_party_nodes=%llu "
        "spawn_squadron_wing_nodes=%llu spawn_squadron_bound=%d",
        kGeneratedEntityPartyBound ? 1 : 0, summary_.generated_party_nodes,
        summary_.generated_party_writes, summary_.launched_party_nodes,
        summary_.launched_wing_nodes, summary_.spawn_squadron_party_nodes,
        summary_.spawn_squadron_wing_nodes, kSpawnNewSquadronPartyCc13Bound ? 1 : 0);
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
        // The shipped end path sets neither of the two above. commandhelpers.lua's
        // luaMissionFailed (9551), luaMissionFailedNew (10123, 10369) and
        // luaMissionCompleted/CompletedNew (9921, 10425) write Mission.MissionStatus
        // (false / true) and Mission.MusicEndTime (GameTime() + 40 / + 35), so
        // MissionStatus=nil means no scripted end ran.
        {"MissionStatus", "MissionStatus"},
        {"MusicEndTime", "MusicEndTime"},
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
    if constexpr (kVehicleClassRowIdBound) {
        log_.notef("summary mission vehicle class rows marked=%zu (00964790 ID/Got, "
            "009648EC..00964959, packet s54_class_id)", summary_.class_rows_marked);
    }
    if (vehicle_class_censuses_ != 0) {
        std::string read_rows;
        for (const auto& [index, reads] : vehicle_class_would_nil_) {
            if (reads != 0 && read_rows.size() < 900) {
                read_rows += " " + std::to_string(index) + "x" + std::to_string(reads);
            }
        }
        log_.notef("summary vehicle class purge census (0095CA70, log only, cc15-vcp1): "
            "censuses=%zu rows_would_nil=%zu later_host_reads=%zu per_class:%s",
            vehicle_class_censuses_, vehicle_class_rows_would_nil_,
            vehicle_class_later_reads_, read_rows.c_str());
        std::string script_rows;
        for (const auto& [index, reads] : vehicle_class_script_nil_rows_) {
            if (reads != 0 && script_rows.size() < 900) {
                script_rows += " " + std::to_string(index) + "x" + std::to_string(reads);
            }
        }
        log_.notef("summary vehicle class purge (cc15-vcp3): purged=%zu failures=%zu "
            "script_nil_reads=%zu lua_errors=%zu per_class:%s", vehicle_class_purged_,
            vehicle_class_purge_failures_, vehicle_class_script_nil_reads_, lua_errors_total_,
            script_rows.c_str());
    }
    log_.notef("summary mission script listeners bound=%d adds=%llu removes=%llu queries=%llu "
        "registered=%zu kill_deaths=%llu kill_fires=%llu attacker_filtered=%llu (008C6760 / "
        "008C6990 / 008C6BB0 / 009813A0, packet cc9_lua_listeners)", kLuaListenersBound ? 1 : 0,
        summary_.listener_adds, summary_.listener_removes, summary_.listener_queries,
        listeners_.size(), summary_.listener_kill_deaths, summary_.listener_kill_fires,
        summary_.listener_attacker_filtered);
    if constexpr (kLuaEntityKilledListenersBound) {
        log_.notef("summary mission script entityKilled listeners bound=1 deaths=%llu "
            "fires=%llu (00986480, packet cc13-ver5)", summary_.listener_entity_killed_deaths,
            summary_.listener_entity_killed_fires);
    }
    if constexpr (kLuaExitZoneListenersBound) {
        log_.notef("summary mission script exitzone listeners bound=1 events=%llu latched=%llu "
            "fires=%llu (00982120, packet cc13-fix3)", summary_.listener_exit_zone_events,
            summary_.listener_exit_zone_latched, summary_.listener_exit_zone_fires);
    }
    if constexpr (kLuaProximityTriggersBound) {
        log_.notef("summary mission script proximity triggers bound=1 adds=%llu "
            "unresolved=%llu checks=%llu fires=%llu callback_missing=%llu removes=%llu "
            "matched=%llu (008C9730 / 00750000 / 008C5FD0, packet cc13-fix10)",
            summary_.proximity_trigger_adds, summary_.proximity_trigger_unresolved,
            summary_.proximity_trigger_checks, summary_.proximity_trigger_fires,
            summary_.proximity_trigger_callback_missing, summary_.remove_trigger_calls,
            summary_.remove_trigger_matched);
        log_.notef("summary mission script trigger target gone notices=%llu retired=%llu "
            "(009273A0 pass K -> 0074FFE0, packet cc13-fix19)", summary_.target_gone_notices,
            summary_.target_gone_retired);
    }
    if constexpr (kLuaSquadOrdnanceDroppedTriggerBound) {
        log_.notef("summary mission script squad ordnance triggers bound=1 adds=%llu "
            "unresolved=%llu checks=%llu fires=%llu (008C9A10 / 007504F0, packet cc13-fix19)",
            summary_.ordnance_trigger_adds, summary_.ordnance_trigger_unresolved,
            summary_.ordnance_trigger_checks, summary_.ordnance_trigger_fires);
    }
    if constexpr (kKillListenerDeadFirstBound) {
        log_.notef("summary mission script kill dead-first bound=1 published=%llu "
            "(0095945B before 00959507, packet cc13-ver5)", summary_.listener_kill_dead_first);
    }
    if constexpr (kKillListenerHarmOnlyBound) {
        log_.notef("summary mission script kill harm-only bound=1 not_harm=%llu grace=%llu "
            "(00959475 / 007F3B19 cause 1, 00959460 clock > 1.0, packet cc13-ver8)",
            summary_.listener_kill_not_harm, summary_.listener_kill_grace);
    }
    log_.notef("summary mission script hit listeners bound=%d events=%llu fires=%llu "
        "unmodelled=%llu (00988510, packet cc9_lua_hit_listeners) attack_type_subkind_differs=%llu",
        kLuaHitListenersBound ? 1 : 0,
        summary_.listener_hit_events, summary_.listener_hit_fires,
        summary_.listener_hit_unmodelled, summary_.listener_hit_attack_type_subkind_differs);
    log_.notef("summary mission script hit second pass bound=%d passes=%llu fired=%llu "
        "(00988510 0098901A..009892E4, plane vtable[140h] 007B97E0 = +9D4h, packet "
        "cc9_lua50_hit_second_pass)", kLuaHitSquadronSecondPassBound ? 1 : 0,
        summary_.listener_hit_second_passes, summary_.listener_hit_second_pass_fires);
    log_.notef("summary mission script listener args bound=%d kill_attacker_bound=%d pushed=%llu "
        "kill_with_attacker=%llu attacker_matched=%llu kill_second_passes=%llu kill_second_fired=%llu "
        "(009813A0 / 00988510, packet cc9_lua51_listener_args)", kLuaListenerCallbackArgsBound ? 1 : 0,
        kLuaKillAttackerBound ? 1 : 0, summary_.listener_args_pushed,
        summary_.listener_kill_with_attacker, summary_.listener_kill_attacker_matched,
        summary_.listener_kill_second_passes, summary_.listener_kill_second_pass_fires);
    log_.notef("summary mission script hit rate limit bound=%d passed=%llu throttled=%llu "
        "pairs=%zu (00988510 this+168h, packet cc9_hit_rate_limit)", kLuaHitRateLimitBound ? 1 : 0,
        summary_.listener_hit_throttle_passed, summary_.listener_hit_throttled,
        hit_rate_limit_.size());
    log_.notef("summary mission script ship speed bound=%d calls=%llu units=%llu unresolved=%llu "
        "(00890D30 -> 00890E6F, packet cc9_lua_set_ship_speed)", kLuaSetShipSpeedBound ? 1 : 0,
        summary_.ship_speed_calls, summary_.ship_speed_units, summary_.ship_speed_unresolved);
    log_.notef("summary mission script failures bound=%d jam_motion=%d set=%llu applied=%llu "
        "other=%llu unresolved=%llu get=%llu true=%llu (0088EFF0 / 0088F1D0 -> 008132C0 / "
        "008135C0, packet cc13_natives1)", kLuaSetFailureBound ? 1 : 0,
        kShipJamMotionBound ? 1 : 0, summary_.set_failure_calls,
        summary_.set_failure_applied, summary_.set_failure_other,
        summary_.set_failure_unresolved, summary_.get_failure_calls,
        summary_.get_failure_true);
    if constexpr (kLuaSetDeadMeatBound) {
        log_.notef("summary mission script dead meat calls=%llu sunk=%llu sink_refused=%llu "
            "destroyed=%llu plane_unapplied=%llu unresolved=%llu (008AC7B0 -> 008110F0 / "
            "vtable[70h], packet cc13-fix11)", summary_.dead_meat_calls, summary_.dead_meat_sunk,
            summary_.dead_meat_sink_refused, summary_.dead_meat_destroyed,
            summary_.dead_meat_plane_unapplied, summary_.dead_meat_unresolved);
    }
    if constexpr (kSpawnNewMissingClassRaisesBound) {
        log_.notef("summary mission script spawn missing class raises=%llu (00949750 -> "
            "00944210 -> 00964790, packet p18_spawn_missing_class)",
            summary_.spawn_missing_class_raises);
    }
    if constexpr (kLuaIsUnitSelectableBound) {
        log_.notef("summary mission script unit selectable calls=%llu true=%llu "
            "unresolved=%llu no_hud=%llu (008AB580 -> 00645160 / 00645060, packet "
            "cc13-fix12)", summary_.unit_selectable_calls, summary_.unit_selectable_true,
            summary_.unit_selectable_unresolved, summary_.unit_selectable_no_hud);
    }
    if constexpr (kLuaGetEntitySpeedBound) {
        log_.notef("summary mission script entity speed calls=%llu squadron=%llu ship=%llu "
            "plane=%llu unresolved=%llu unmodelled=%llu (008C0820 -> vtable[38h] 007ED470 / "
            "0080E0F0 / 007B8E60, packet cc13-fix12)", summary_.entity_speed_calls,
            summary_.entity_speed_squadron, summary_.entity_speed_ship,
            summary_.entity_speed_plane, summary_.entity_speed_unresolved,
            summary_.entity_speed_unmodelled);
    }
    if constexpr (kLuaClearForcedReconLevelBound) {
        log_.notef("summary mission script clear forced recon calls=%llu records=%llu "
            "was_forced=%llu unresolved=%llu (008AAB90 -> 00803B50, packet cc13-fix18)",
            summary_.clear_forced_recon_calls, summary_.clear_forced_recon_records,
            summary_.clear_forced_recon_was_forced, summary_.clear_forced_recon_unresolved);
    }
    if constexpr (kLuaSetCrewLevelBound) {
        log_.notef("summary mission script crew level calls=%llu unresolved=%llu (00894F20 "
            "-> 006E6210 -> vtable[128h], packet cc13-fix18)", summary_.crew_level_calls,
            summary_.crew_level_unresolved);
    }
    if constexpr (kLuaDeviceReloadTimeMulBound) {
        log_.notef("summary mission script reload time mul calls=%llu mul=%g (008C14B0 -> "
            "00E08178, packet cc13-fix18)", summary_.reload_time_mul_calls,
            static_cast<double>(lua_device_reload_time_mul_00e08178()));
    }
    log_.notef("summary mission script support manager bann=%llu permit=%llu enable=%llu "
        "unresolved=%llu (0088C750 / 0088C8E0 -> 00A38DA0, 0088CBB0 -> 0065BBA0 / 00665EF0, "
        "packet cc13-smbind)", summary_.support_manager_bann_calls,
        summary_.support_manager_permit_calls, summary_.support_manager_enable_calls,
        summary_.support_manager_enable_unresolved);
    log_.notef("summary mission script attack target bound=%d calls=%llu fire_arm=%llu "
        "command_arm=%llu pushed=%llu nil=%llu unresolved=%llu (008A6DE0, packet "
        "cc9_unit_get_attack_target) squadron_standin=%llu", kLuaUnitGetAttackTargetBound ? 1 : 0,
        summary_.attack_target_calls, summary_.attack_target_fire_arm,
        summary_.attack_target_command_arm, summary_.attack_target_pushed,
        summary_.attack_target_nil, summary_.attack_target_unresolved,
        summary_.attack_target_squadron_standin);
    log_.notef("summary mission script squadron speed bound=%d calls=%llu planes=%llu "
        "unresolved=%llu (0089F780 -> 0074E1E0 -> 007D9E80, packet cc9_squadron_set_speed)",
        kLuaSquadronSetSpeedBound ? 1 : 0, summary_.squadron_speed_calls,
        summary_.squadron_speed_planes, summary_.squadron_speed_unresolved);
    log_.notef("summary mission script squadron force gunner mode bound=%d calls=%llu set=%llu "
        "unresolved=%llu (0089F950 -> entity+379h, packet cc13_natives1)",
        kLuaSquadronForceGunnerModeBound ? 1 : 0, summary_.force_gunner_calls,
        summary_.force_gunner_set, summary_.force_gunner_unresolved);
    log_.notef("summary mission script class changed bound=%d calls=%llu true=%llu "
        "(008CC4B0, registry+2010h, packet cc9_is_class_changed)",
        kLuaIsClassChangedBound ? 1 : 0, summary_.class_changed_calls,
        summary_.class_changed_true);
    log_.notef("summary mission script submarine depth set bound=%d calls=%llu stored=%llu "
        "unresolved=%llu (00893F40 -> 008528B0, packet cc9_set_submarine_depth_level)",
        kLuaSetSubmarineDepthLevelBound ? 1 : 0, summary_.sub_depth_calls,
        summary_.sub_depth_stored, summary_.sub_depth_unresolved);
    // Packet cc13-luawire: printed only when bound, so an OFF log is unchanged.
    if constexpr (kLuaHasFiredBound) {
        log_.notef("summary mission script has fired calls=%llu true=%llu unresolved=%llu "
            "no_director=%llu (008BE7E0, packet cc13-luawire)", summary_.has_fired_calls,
            summary_.has_fired_true, summary_.has_fired_unresolved,
            summary_.has_fired_no_director);
    }
    if constexpr (kLuaAddWaterLoadBound) {
        log_.notef("summary mission script add water load calls=%llu routed=%llu applied=%llu "
            "skipped=%llu unresolved=%llu (00893380 -> 0074F440, packet cc13-luawire)",
            summary_.water_load_calls, summary_.water_load_routed, summary_.water_load_applied,
            summary_.water_load_skipped, summary_.water_load_unresolved);
    }
    if constexpr (kLuaBreakShipBound) {
        log_.notef("summary mission script break ship calls=%llu broken=%llu no_halves=%llu "
            "unresolved=%llu puff_draws=%llu (008914F0 -> 00814520 -> 00935D30 -> 0092A240, "
            "packet cc13-luawire)", summary_.break_ship_calls, summary_.break_ship_broken,
            summary_.break_ship_no_halves, summary_.break_ship_unresolved,
            summary_.break_ship_puff_draws);
    }
    if constexpr (kLuaUnitSetPlayerCommandsEnabledBound) {
        log_.notef("summary mission script player commands calls=%llu stored=%llu "
            "unresolved=%llu (008A6C30 -> 0071DD30 -> director+1CCh, no host reader, packet "
            "cc13-luawire)", summary_.player_commands_calls, summary_.player_commands_stored,
            summary_.player_commands_unresolved);
    }
    if constexpr (kLuaSetVisibilityBound) {
        log_.notef("summary mission script set visibility calls=%llu planes=%llu "
            "null_members_guarded=%llu unresolved=%llu (008A13D0 -> 00951F40, packet "
            "cc13-luawire 7b)", summary_.visibility_calls, summary_.visibility_planes,
            summary_.visibility_null_members, summary_.visibility_unresolved);
    }
    if constexpr (kLuaSquadronEnableVehicleAvoidanceBound) {
        log_.notef("summary mission script vehicle avoidance calls=%llu unresolved=%llu "
            "(008A0000 -> +3A4h bit 2 -> 009A1959, packet cc13-luawire 7b)",
            summary_.avoidance_calls, summary_.avoidance_unresolved);
    }
    if constexpr (kLuaSquadronSetBaseUnsupportedBound) {
        log_.notef("summary mission script base unsupported calls=%llu added=%llu "
            "unresolved=%llu (0089FAF0 -> 006CA730 -> 006C4790, packet cc13-luawire 7b)",
            summary_.base_unsupported_calls, summary_.base_unsupported_added,
            summary_.base_unsupported_unresolved);
    }
    if constexpr (kLuaSetCheatTurboBound) {
        log_.notef("summary mission script cheat turbo calls=%llu planes=%llu unresolved=%llu "
            "(008A62A0 -> +340h -> 0099D479 / 009FBB20, packet cc13-luawire 7b)",
            summary_.cheat_turbo_calls, summary_.cheat_turbo_planes,
            summary_.cheat_turbo_unresolved);
    }
    log_.notef("summary mission script submarine on surface bound=%d calls=%llu true=%llu "
        "unseeded=%llu unresolved=%llu (008942C0 -> 00852820, packet "
        "cc9_lua_submarine_on_surface)", kLuaGetSubmarineOnSurfaceBound ? 1 : 0,
        summary_.sub_surface_calls, summary_.sub_surface_true, summary_.sub_surface_unseeded,
        summary_.sub_surface_unresolved);
    log_.notef("summary mission script truly dead bound=%d calls=%llu true=%llu (00899500, "
        "Ptr == null, packet cc13_natives1)", kLuaTrulyDeadBound ? 1 : 0,
        summary_.truly_dead_calls, summary_.truly_dead_true);
    log_.notef("summary mission script air base slot count bound=%d calls=%llu resized=%llu "
        "unresolved=%llu (008963E0 -> 006C7E20, packet cc9_set_air_base_slot_count)",
        kLuaSetAirBaseSlotCountBound ? 1 : 0, summary_.slot_count_calls,
        summary_.slot_count_resized, summary_.slot_count_unresolved);
    log_.notef("summary mission script air base stock bound=%d calls=%llu new_entries=%llu "
        "unresolved=%llu (00896A90 -> 006CA770, packet cc9_lua_add_air_base_stock)",
        kLuaAddAirBaseStockBound ? 1 : 0, summary_.stock_add_calls,
        summary_.stock_add_created, summary_.stock_add_unresolved);
    log_.notef("summary mission script shipyard stock bound=%d calls=%llu new_records=%llu "
        "unresolved=%llu (00896CC0 -> 0084ACB0, packet cc9_lua_add_shipyard_stock)",
        kLuaAddShipyardStockBound ? 1 : 0, summary_.shipyard_stock_calls,
        summary_.shipyard_stock_created, summary_.shipyard_stock_unresolved);
    log_.notef("summary shipyard create requested=%llu made=%llu (00844FC0 through the "
        "SpawnNew path, packet cc9_shipyard_create_unit)", summary_.shipyard_units_requested,
        summary_.shipyard_units_made);
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
    log_.notef("summary mission script untouchable clear bound=%d remove_calls=%llu cleared=%llu "
        "is_calls=%llu is_true=%llu (008AC2B0, 008AC420, packet p16_lua_natives)",
        kLuaUntouchableClearBound ? 1 : 0, summary_.untouchable_remove_calls,
        summary_.untouchable_cleared, summary_.untouchable_is_calls, summary_.untouchable_is_true);
    log_.notef("summary mission truly dead ptr_nulled=%llu bound=%d (00928C80, packet "
        "p16_truly_dead)", summary_.truly_dead_ptr_nulled, kLuaTrulyDeadOnKilledBound ? 1 : 0);
    log_.notef("summary mission last position written=%llu bound=%d (00928D49..00928E7E, packet "
        "cc9-recon1-lastpos)", summary_.last_position_written,
        kLuaLastPositionOnKilledBound ? 1 : 0);
    log_.notef("summary mission generate object calls=%llu repeat=%llu respawned_dead=%llu "
        "every_call=%llu bound=%d (00944FD0, packet p16_lua_natives)", summary_.generate_object_calls,
        summary_.generate_object_repeat, summary_.generate_object_respawned,
        summary_.generate_object_every_call, kGenerateObjectEveryCallBound ? 1 : 0);
    log_.notef("summary mission generate object hidden carry bound=%d carried=%zu "
        "applied=%llu commands=%llu (0046DC10 creator 0046DE9A / InitAll 0046DEDD, "
        "0046AAB0 at 009453BB, packet cc13-hiddencarry) unattached_host_only=%llu",
        kSceneHiddenSpawnCarryCc13Bound ? 1 : 0, scene_spawn_pool().carried_count(),
        g_hidden_spawn_carry_applied, g_hidden_spawn_carry_commands,
        g_hidden_spawn_carry_unattached);
    log_.notef("summary mission script formation leader bound=%d calls=%llu found=%llu "
        "other=%llu (00899AF0 -> 007788D0, packet cc9_get_formation_leader)",
        kLuaFormationLeaderBound ? 1 : 0, summary_.formation_leader_calls,
        summary_.formation_leader_found, summary_.formation_leader_other);
    log_.notef("summary mission script last catapulted bound=%d calls=%llu nil=%llu "
        "not_ship=%llu (00892860 -> 00953A60, packet cc9_get_last_catapulted)",
        kLuaLastCatapultedBound ? 1 : 0, summary_.last_catapulted_calls,
        summary_.last_catapulted_nil, summary_.last_catapulted_not_ship);
    log_.notef("summary mission script ship catapult bound=%d use_calls=%llu launches=%llu "
        "no_catapult=%llu no_stock=%llu in_air=%llu spawn_failed=%llu set_stock_calls=%llu "
        "last_live=%llu (00891D50 -> 008141E0 -> 00957450, packet p17_catapult_adapter)",
        bsp::kShipCatapultLaunchBound ? 1 : 0, summary_.use_catapult_calls,
        summary_.use_catapult_launches, summary_.use_catapult_no_catapult,
        summary_.use_catapult_no_stock, summary_.use_catapult_in_air,
        summary_.use_catapult_spawn_failed, summary_.set_catapult_stock_calls,
        summary_.last_catapulted_live);
    if constexpr (kLuaGetNumCatapultedBound) {
        log_.notef("summary mission script num catapulted bound=1 calls=%llu not_ship=%llu "
            "(00892AA0 -> 00953920, packet cc13-fix14b; ships count 0, no launch slot "
            "is filled)", summary_.num_catapulted_calls, summary_.num_catapulted_not_ship);
    }
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
        "unresolved=%llu sync_notifies=%llu (008AA8F0 -> 00805CF0, packet cc9_forced_recon_level)",
        kForcedReconLevelBound ? 1 : 0, summary_.forced_recon_calls,
        summary_.forced_recon_units, summary_.forced_recon_unresolved,
        summary_.forced_recon_sync_notifies);
    log_.notef("summary mission script border zones bound=%d loaded=%d calls=%llu missing=%llu "
        "(008AECD0 / 004C7730, packet cc9_get_closest_border_zone)",
        kLuaClosestBorderZoneBound ? 1 : 0, border_zones_loaded_ ? 1 : 0,
        summary_.border_zone_calls, summary_.border_zone_missing);
    log_.notef("summary mission script in border zone bound=%d calls=%llu outside=%llu "
        "no_zone=%llu (008AF020 -> 0071C4F0 / 00489C40, packet cc13_natives1)",
        kLuaIsInBorderZoneBound ? 1 : 0, summary_.in_border_zone_calls,
        summary_.in_border_zone_outside, summary_.in_border_zone_no_zone);
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
    log_.notef("summary mission getproperty torpedostock bound=%d asked=%llu served=%llu "
        "not_ship=%llu; state bound=%d asked=%llu served=%llu two=%llu (00815870, 007EF1C0, "
        "packet p16_torpedo_stock_property)",
        kGetPropertyTorpedoStockBound ? 1 : 0, summary_.get_property_torpedo_stock_asked,
        summary_.get_property_torpedo_stock_served, summary_.get_property_torpedo_stock_not_ship,
        kGetPropertySquadronStateBound ? 1 : 0, summary_.get_property_squadron_state_asked,
        summary_.get_property_squadron_state_served, summary_.get_property_squadron_state_two);
    log_.notef("summary mission airops gates: ready_calls=%llu ready_true=%llu "
        "launch_calls=%llu started=%llu queued=%llu (00895d20, 0089e3c0)",
        summary_.air_ops_ready_calls, summary_.air_ops_ready_true,
        summary_.air_ops_launch_calls, summary_.air_ops_launch_started,
        summary_.air_ops_launch_queued);
    // cc9-sweep3 reach counters for cc13's plane-task / air-ops switches (log only).
    const auto reach = [this](const char* name, const bsp::PlaneTaskReachPair& pair) {
        log_.notef("summary mission plane task reach (sw3) %s: calls=%llu differs=%llu inner=%llu",
            name, pair.calls.load(), pair.differs.load(), pair.inner.load());
    };
    const bsp::PlaneTaskReachSw3& r = bsp::g_plane_task_reach_sw3;
    reach("strafe_countdown 009CCF06 (kStrafeX87FaithfulCc13Bound)", r.strafe_countdown);
    reach("strafe_in_range 009CCF58 (kStrafeX87FaithfulCc13Bound)", r.strafe_in_range);
    reach("strafe_off_nose 00996510 (kStrafeX87FaithfulCc13Bound)", r.strafe_off_nose);
    reach("strafe_distance 00996300 (kStrafeX87FaithfulCc13Bound)", r.strafe_distance);
    reach("follow_wait 009BE3E0 (kFollowWaitFloatHelpersCc13Bound)", r.follow_wait);
    reach("interp_hi 004190A9 (kInterpClamped00419010NanCc13Bound)", r.interp_hi);
    reach("retreat_zone 009C8D40 (kRetreatX87FaithfulCc13Bound)", r.retreat_zone);
    reach("retreat_countdown 009C9FB4 (kRetreatX87FaithfulCc13Bound)", r.retreat_countdown);
    reach("retreat_arm 009CA02C (kRetreatX87FaithfulCc13Bound)", r.retreat_arm);
    reach("retreat_inside 009C9EA0 (kRetreatX87FaithfulCc13Bound)", r.retreat_inside);
    reach("retreat_steer 009C9310 (kRetreatX87FaithfulCc13Bound)", r.retreat_steer);
    reach("dive_ceiling 009C7F3C (kDiveBombNanComparesCc13Bound)", r.dive_ceiling);
    reach("dive_breakoff 009C8B4D (kDiveBombNanComparesCc13Bound)", r.dive_breakoff);
    reach("airops_arrive 006C56D0 (kAirOpsArriveSquadronWalkCc13Bound)", r.airops_arrive);
    reach("airops_return 006C5AD9 (kAirOpsReturnSlotClassArmCc13Bound)", r.airops_return);
    reach("vec3_divide 0042AE80 (kVector3DivideInPlacePc24Cc13Bound)", r.vec3_divide);
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
    // 00B6621C: lua_type(...) == 6 (LUA_TFUNCTION).
    const bool defined = kEntryPointIsFunctionBound ? lua_isfunction(state_, -1)
                                                    : !lua_isnil(state_, -1);
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
