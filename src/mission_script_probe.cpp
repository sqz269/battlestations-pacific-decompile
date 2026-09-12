// Mission Lua machine probe.
//
// Builds a real Lua 5.1.1 state the way 006b8740 builds the mission machine,
// installs the 560 mission bindings from the table at 00e0b7b8 as stubs that
// log their name and argument count, then runs one installed mission script
// through the 006b89f0 chunk rule and calls the four entry points through the
// 00887750 named-call rule.
//
// The point is to check the recovered host contract against a real script: a
// contract that is wrong shows up here as a nil-call error on a name the table
// does not carry, or as a chunk that never loads. Nothing in this file is a
// reconstruction of a native body; the native rules it follows all live in
// bsp/mission_lua_host.hpp and are cited per step.
//
// Packet cc_mission_natives added the self table and the entity return convention
// (docs/MISSION_LUA_SELF_TABLE.md, docs/LUA_BINDING_ENTITY.md), which is what the six
// previously failing scripts needed, plus --sweep over every installed mission script.
//
// Usage: bsp_mission_script_probe [game-root] [mission-name]
//            [--stub-dofile] [--skip-global-folders] [--skip-lobby-settings]
//            [--no-self-table] [--recon-tables] [--core-bindings] [--navigator-bindings]
//            [--stand-in-random] [--sweep] [--quiet]
// Defaults to the installed copy and "usn/usn_2_java".

#include "bsp/lua_binding_core.hpp"
#include "bsp/lua_binding_navigator.hpp"
#include "bsp/mission_lua_bindings.hpp"
#include "bsp/mission_lua_host.hpp"
#include "bsp/mission_lua_machine.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/recon_values.hpp"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

// config/target.json's installed copy. Overridden by argv[1].
constexpr const char kDefaultGameRoot[]
    = "I:/SteamLibrary/steamapps/common/Battlestations Pacific";

// The name as the scene record's script table at +928h carries it, which is
// what 008860b0 concatenates. The subdirectory is part of the name: see the
// resolution report this probe prints.
constexpr const char kDefaultMissionName[] = "usn/usn_2_java";

// ---------------------------------------------------------------------------
// Call log. One entry per native binding the script actually reached.
// ---------------------------------------------------------------------------

struct NativeCall {
    std::string phase;
    std::string name;
    int argc;
};

// One CreateScript registration. 00898750 passes the current Lua stack range [first, -1] to
// 009290a0, which forwards it to 00887e50 together with the new entity's self key; 00887ee1 and
// 00887f05 normalise both ends the way lua_absindex does, and a first index of zero means "no
// extra arguments". CreateScript sets first to 2 exactly when it saw more than one argument
// (00898945 CMP EAX,1 / 0089894a MOV EDI,2), so the forwarded range is everything after the
// name. The probe keeps registry references because the C stack is gone by the time the engine
// would make the call.
struct CreatedScript {
    std::string name;
    std::string self_key;
    std::vector<int> argument_refs; // luaL_ref values, LUA_REGISTRYINDEX
};

struct ProbeState {
    std::string game_root;
    std::string phase;
    bool follow_dofile{true};
    bool run_global_folders{true};
    bool create_lobby_settings{true};
    bool model_self_table{true};
    bool install_recon_tables{false};
    bool verbose{true};
    // --core-bindings: route the ten bindings of docs/LUA_BINDING_CORE.md through
    // src/lua_binding_core.cpp instead of the stub, so the script sees the value the native
    // would have pushed rather than nothing at all.
    bool core_bindings{false};
    // How many times a reconstructed routine ran, and in how many distinct scripts.
    std::map<std::string, int> core_binding_calls;
    std::map<std::string, std::set<std::string>> core_binding_scripts;
    std::vector<NativeCall> calls;
    std::map<std::string, int> call_counts;
    std::vector<std::string> dofile_paths;
    std::vector<std::string> dofile_missing;
    std::string first_error;
    std::string first_error_phase;

    // The self table, modelled after 00928a00. The probe has no entities, so it mints one on
    // demand per (binding, call) and keeps the id counter the native keeps in the entity's
    // 16-bit field at +174h. kSelfKeyBufferBytes bounds the key either way.
    std::uint16_t next_entity_id{1};
    // Which script, in a sweep, called a binding at all; used for the per-binding survey.
    std::map<std::string, std::map<std::string, int>> calls_by_script;
    std::string current_script;
    // Names registered by CreateScript, in registration order, with the self key each was
    // created against and registry references to the extra arguments; the sweep calls them the
    // way 00887750 would.
    std::vector<CreatedScript> created_scripts;

    // --navigator-bindings: the eight rows of docs/LUA_BINDING_NAVIGATOR.md run for real
    // against the stub entity state below, instead of logging and returning nothing.
    bool navigator_bindings{false};
    // Native steps the navigator host recorded rather than performed.
    std::map<std::string, int> navigator_host_steps;
    // The label a minted entity was created under: the first string argument of the
    // entity-returning row that produced it, which for FindEntity is the ship's authored name.
    std::map<std::uint16_t, std::string> entity_labels;
    // One record per minted entity that a navigator binding actually addressed. The fields are
    // exactly the ones the eight bindings write; nothing else about a ship is modelled.
    struct StubEntity {
        bool skill_level_set{false};
        int skill_level{0};
        bool repair_set{false};
        bool repair_enabled{false};
        bool repair_routed{false};
        bool formation_set{false};
        std::string formation_leader;
        // "attackmove -> <target>" / "moveto -> <target>", in call order.
        std::vector<std::string> orders;
    };
    std::map<std::uint16_t, StubEntity> entity_state;
    // Order in which entities were first addressed, so the report is deterministic.
    std::vector<std::uint16_t> entity_state_order;
    // Role rows SetRoleAvailable set, and which arm each took.
    std::vector<std::string> role_rows;
    // --stand-in-random: see the note above stand_in_random_binding. Not a reconstruction.
    bool stand_in_random{false};
    int stand_in_random_calls{0};
};

ProbeState g_probe;

// 00928a00, reduced to what a probe with no entities can do: mint an id, build the table under
// the key, and seed the three fields the native seeds. `Ptr` is light userdata in the native
// (00b67530 -> lua_pushlightuserdata at 00a67be0); the probe pushes a distinct non-null pointer
// derived from the id so that object_from_lua_table_00888aa0's rule stays meaningful, and never
// dereferences it. `Class` is the one field 00440e10 adds afterwards; the probe points it at a
// row of the global DeviceClass table when the datatable scripts built one, because that is
// where the shipped scripts' `FindEntity(...).Class.Height` reads from.
//
// The value is left on the stack. Returns the key so the caller can log it.
std::string push_new_entity_table(lua_State* L)
{
    const std::uint16_t id = g_probe.next_entity_id++;
    const std::string key = bsp::mission_entity_self_key(id);

    lua_getfield(L, LUA_GLOBALSINDEX, bsp::kMissionSelfTableGlobal);
    if (lua_isnil(L, -1)) { // no self table: behave like the not-found path, 00b66430
        return std::string();
    }
    lua_createtable(L, 0, 4);

    lua_pushstring(L, key.c_str());
    lua_setfield(L, -2, bsp::kEntitySelfFieldId);       // 00928ba5, a string, not a number
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, bsp::kEntitySelfFieldDead);     // 00928bc7
    lua_pushlightuserdata(L, reinterpret_cast<void*>(static_cast<std::uintptr_t>(id) + 1u));
    lua_setfield(L, -2, bsp::kEntitySelfFieldPtr);      // 00928c2f

    // self.Class = <ClassGlobal>[index]: 009292b0 for VehicleClass, 00440e10 for DeviceClass.
    // The probe has no entity kind to select with, so it takes the first row of VehicleClass and
    // falls back to DeviceClass. That is enough for a field read on the class to resolve, which
    // is what the scripts do; it is not the row the native would have chosen.
    const char* class_globals[] = {bsp::kVehicleClassGlobal, bsp::kDeviceClassGlobal};
    bool class_set = false;
    for (std::size_t i = 0; i < 2 && !class_set; ++i) {
        lua_getfield(L, LUA_GLOBALSINDEX, class_globals[i]);
        if (lua_istable(L, -1)) {
            lua_pushnil(L);
            if (lua_next(L, -2) != 0) {
                lua_remove(L, -2);      // drop the key, keep the first class row
                lua_setfield(L, -3, bsp::kEntitySelfFieldClass);
                class_set = true;
            }
        }
        lua_pop(L, 1);                  // the class global
    }
    if (!class_set) {
        lua_createtable(L, 0, 0);
        lua_setfield(L, -2, bsp::kEntitySelfFieldClass);
    }

    // thisTable[key] = table, then leave the table itself as the result, which is what the
    // native's 00b678e0 + 00b663d0 pair produces: a second reference to the stored value.
    lua_pushstring(L, key.c_str());
    lua_pushvalue(L, -2);
    lua_settable(L, -4);
    lua_remove(L, -2);                  // drop thisTable, keep the entity table
    return key;
}

// ---------------------------------------------------------------------------
// --core-bindings: the ten routines of include/bsp/lua_binding_core.hpp, run for real
// ---------------------------------------------------------------------------
//
// The stub below returns nothing for every row. For the ten bindings packet cc_lua_core
// reconstructed, that is now a choice rather than a limit: this adapter gives them the same
// argument surface and the same result surface the native uses, so a script gets the value the
// native would have pushed. The host records every native step the probe cannot perform
// instead of inventing one; nothing here stands in for unrecovered game behaviour.

// Argument slots follow mission_binding_argument_slot: index 0 is Lua slot 1.
class ProbeArgumentReader final : public bsp::LuaBindingArgumentReader {
public:
    explicit ProbeArgumentReader(lua_State* state) : state_(state) {}
    int count() override { return lua_gettop(state_); }
    int get_integer(int index) override
    {
        // 00b66290 narrows with the CRT __ftol, which truncates toward zero; lua_tointeger
        // does the same for the values a script can pass here.
        return static_cast<int>(lua_tointeger(state_, slot(index)));
    }
    double get_number(int index) override
    {
        return static_cast<double>(lua_tonumber(state_, slot(index)));
    }
    bool get_boolean(int index) override { return lua_toboolean(state_, slot(index)) != 0; }
    std::string get_string(int index) override
    {
        const char* text = lua_tostring(state_, slot(index));
        return text != nullptr ? std::string(text) : std::string();
    }
    bool is_string(int index) override { return lua_isstring(state_, slot(index)) != 0; }
    bool is_nil(int index) override { return lua_isnil(state_, slot(index)) != 0; }
    bool is_entity_table(int index) override { return entity_at(index) != nullptr; }
    void* entity_at(int index) override
    {
        // object_from_lua_table_00888aa0's rule: the entity is the light userdata the table
        // carries in `Ptr`. The probe's pointers are minted by push_new_entity_table and are
        // never dereferenced, here or anywhere else in this file.
        const int at = slot(index);
        if (lua_istable(state_, at) == 0) {
            return nullptr;
        }
        lua_getfield(state_, at, bsp::kEntitySelfFieldPtr);
        void* pointer = lua_islightuserdata(state_, -1) ? lua_touserdata(state_, -1) : nullptr;
        lua_pop(state_, 1);
        return pointer;
    }

private:
    static int slot(int index) { return bsp::mission_binding_argument_slot(index); }
    lua_State* state_;
};

class ProbeResultWriter final : public bsp::LuaBindingResultWriter {
public:
    explicit ProbeResultWriter(lua_State* state) : state_(state) {}
    void push_number(int value) override
    {
        lua_pushnumber(state_, static_cast<lua_Number>(value));
    }
    void push_boolean(bool value) override { lua_pushboolean(state_, value ? 1 : 0); }
    void push_nil() override { lua_pushnil(state_); }

private:
    lua_State* state_;
};

// The two game fields GetDifficulty reads. The probe is not a game, so these are the values a
// campaign load would have left: mission_tree_screens.hpp's launch path writes the effective
// difficulty at game+6ACh and clears the non-campaign flag at game+1FE4h for a campaign
// mission. Choosing the campaign side keeps every branch of the ten that a campaign takes.
const int kProbeNonCampaignFlag = 0;
const int kProbeEffectiveDifficulty = 1;

class ProbeCoreHost final : public bsp::LuaBindingCoreHost {
public:
    explicit ProbeCoreHost(lua_State* state) : state_(state) {}

    int game_non_campaign_flag() override { return kProbeNonCampaignFlag; }
    int game_effective_difficulty() override { return kProbeEffectiveDifficulty; }

    void log_prepare_class(int class_id) override
    {
        static_cast<void>(class_id);
        record("log_prepare_class");
    }

    bool resolve_global_integer(const std::string& dotted_path, int& value) override
    {
        // This one the probe can do for real: the datatable scripts the machine loads build the
        // global VehicleClass table, so the native walk of 00b68d70 over
        // "VehicleClass.<n>.Race" has the same answer here.
        //
        // 00b68d70's rule, already reconstructed by docs/LOCALE_TEXT_LOOKUP.md and implemented
        // for the locale root in src/locale_lua_context.cpp: a segment is looked up as a string
        // first and, when that is nil, converted with the CRT _atol (a base-10 _strtol, not a
        // whole-token validator) and looked up again as a number. The numeric fallback is only
        // taken when the conversion is non-zero or the segment is the single character '0'.
        // Without it `VehicleClass.5` never resolves, because the datatable builds the rows
        // under integer keys. This adapter reproduces the table-kind half of that rule; the
        // pseudo-object half the locale root needs does not apply to a plain global table.
        record("resolve_global_integer");
        value = 0;
        std::size_t begin = 0;
        lua_pushvalue(state_, LUA_GLOBALSINDEX);
        bool reached = true;
        while (begin < dotted_path.size()) {
            std::size_t end = begin;
            while (end < dotted_path.size() && dotted_path[end] != '.') {
                ++end;
            }
            if (end == begin || lua_istable(state_, -1) == 0) {
                reached = false;
                break;
            }
            lua_pushlstring(state_, dotted_path.data() + begin, end - begin);
            lua_gettable(state_, -2);
            if (lua_isnil(state_, -1) != 0) {
                const std::string segment = dotted_path.substr(begin, end - begin);
                const long parsed = std::strtol(segment.c_str(), nullptr, 10);
                const bool numeric = parsed != 0 || segment == "0";
                if (!numeric) {
                    reached = false;
                    break;
                }
                lua_pop(state_, 1);
                lua_pushnumber(state_, static_cast<lua_Number>(parsed));
                lua_gettable(state_, -2);
            }
            lua_remove(state_, -2); // drop the table the segment came from
            begin = end < dotted_path.size() ? end + 1 : end;
        }
        const bool ok = reached && lua_isnumber(state_, -1) != 0;
        if (ok) {
            value = static_cast<int>(lua_tointeger(state_, -1));
        }
        lua_pop(state_, 1);
        return ok;
    }

    void vehicle_class_mark_party_required(int class_id, int party) override
    {
        static_cast<void>(class_id);
        static_cast<void>(party);
        record("vehicle_class_mark_party_required");
    }
    void vehicle_class_get_or_create(int class_id, bool read_race) override
    {
        static_cast<void>(class_id);
        static_cast<void>(read_race);
        record("vehicle_class_get_or_create");
    }
    void music_director_set_level(int level) override
    {
        static_cast<void>(level);
        record("music_director_set_level");
    }
    void session_send_music_level(int level) override
    {
        static_cast<void>(level);
        record("session_send_music_level");
    }
    bool entity_vcall_5c(void* entity, int selector) override
    {
        static_cast<void>(entity);
        static_cast<void>(selector);
        record("entity_vcall_5c");
        // The concrete vtable was never resolved, so the probe takes the branch that sends no
        // session message. Recorded here rather than assumed silently.
        return false;
    }
    void entity_vcall_2c(void* entity, int party, std::uint32_t entity_field_58) override
    {
        static_cast<void>(entity);
        static_cast<void>(party);
        static_cast<void>(entity_field_58);
        record("entity_vcall_2c");
    }
    void session_route_party_message(void* entity, int party) override
    {
        static_cast<void>(entity);
        static_cast<void>(party);
        record("session_route_party_message");
    }
    void scoring_set_real_play_time_running(bool running) override
    {
        real_play_time_running_ = running;
        record("scoring_set_real_play_time_running");
    }
    void scoring_set_final_scoring_function_name(const std::string& name) override
    {
        final_scoring_function_ = name;
        record("scoring_set_final_scoring_function_name");
    }
    void message_map_load(const std::string& name, int index) override
    {
        static_cast<void>(index);
        message_maps_.insert(name);
        record("message_map_load");
    }
    void call_0088b6d0_0076a9f0_00765590(const std::string& name, int index) override
    {
        static_cast<void>(name);
        static_cast<void>(index);
        record("call_0088b6d0_0076a9f0_00765590");
    }
    void entity_set_think_script_name(void* entity, const std::string& name) override
    {
        static_cast<void>(entity);
        think_names_.insert(name);
        record("entity_set_think_script_name");
    }
    void set_entity_message_suppression(void* entity, bool suppressed) override
    {
        static_cast<void>(entity);
        static_cast<void>(suppressed);
        record("set_entity_message_suppression");
    }
    void set_global_message_suppression(bool suppressed) override
    {
        global_messages_suppressed_ = suppressed;
        record("set_global_message_suppression");
    }
    void message_system_drain_queue() override { record("message_system_drain_queue"); }

    // What the probe kept, for the run report.
    bool real_play_time_running() const { return real_play_time_running_; }
    const std::string& final_scoring_function() const { return final_scoring_function_; }
    bool global_messages_suppressed() const { return global_messages_suppressed_; }
    std::size_t message_map_count() const { return message_maps_.size(); }
    std::size_t think_name_count() const { return think_names_.size(); }

private:
    static void record(const char* step) { g_probe.core_binding_calls[step] += 1; }

    lua_State* state_;
    bool real_play_time_running_{false};
    std::string final_scoring_function_;
    bool global_messages_suppressed_{false};
    std::set<std::string> message_maps_;
    std::set<std::string> think_names_;
};

// One host per state; the ten routines keep no state of their own.
ProbeCoreHost* g_core_host = nullptr;

// Runs the reconstructed routine for `name` when --core-bindings is on. Returns -1 when the
// name is not one of the ten, which is the caller's signal to fall back to the stub.
int run_core_binding(lua_State* L, const char* name)
{
    if (!g_probe.core_bindings || g_core_host == nullptr) {
        return -1;
    }
    ProbeArgumentReader args(L);
    ProbeResultWriter results(L);
    int produced = -1;
    if (std::strcmp(name, "SETLOG") == 0) {
        produced = bsp::lua_binding_setlog();
    } else if (std::strcmp(name, "GetDifficulty") == 0) {
        produced = bsp::lua_binding_get_difficulty(results, *g_core_host);
    } else if (std::strcmp(name, "PrepareClass") == 0) {
        produced = bsp::lua_binding_prepare_class(args, *g_core_host);
    } else if (std::strcmp(name, "Music_Control_SetLevel") == 0) {
        produced = bsp::lua_binding_music_control_set_level(args, *g_core_host);
    } else if (std::strcmp(name, "SetParty") == 0) {
        produced = bsp::lua_binding_set_party(args, results, *g_core_host);
    } else if (std::strcmp(name, "Scoring_RealPlayTimeRunning") == 0) {
        produced = bsp::lua_binding_scoring_real_play_time_running(args, results, *g_core_host);
    } else if (std::strcmp(name, "Scoring_SetFinalScoringFunctionName") == 0) {
        produced = bsp::lua_binding_scoring_set_final_scoring_function_name(args, *g_core_host);
    } else if (std::strcmp(name, "LoadMessageMap") == 0) {
        produced = bsp::lua_binding_load_message_map(args, *g_core_host);
    } else if (std::strcmp(name, "SetThink") == 0) {
        produced = bsp::lua_binding_set_think(args, *g_core_host);
    } else if (std::strcmp(name, "EnableMessages") == 0) {
        produced = bsp::lua_binding_enable_messages(args, *g_core_host);
    }
    if (produced >= 0 && !g_probe.current_script.empty()) {
        g_probe.core_binding_scripts[name].insert(g_probe.current_script);
    }
    return produced;
}

// ---------------------------------------------------------------------------
// --navigator-bindings: the eight routines of include/bsp/lua_binding_navigator.hpp
// ---------------------------------------------------------------------------
//
// The stub returns nothing for every row, so in docs/GAME_EXECUTABLE.md milestone 2l the
// mission's own order function addresses 21 ships and moves none of them. These adapters give
// the eight bindings the argument surface the native has and a stub entity state to write to,
// so the question "which ship receives which order" gets an answer from the script itself
// rather than from a reading of it. The state is only the fields the eight bindings write; the
// host records every native step the probe cannot perform and invents no game behaviour.

// The probe's `Ptr` is `id + 1` (push_new_entity_table), so a pointer maps back to its id.
std::uint16_t probe_entity_id(void* entity)
{
    const std::uintptr_t raw = reinterpret_cast<std::uintptr_t>(entity);
    return raw == 0 ? std::uint16_t{0} : static_cast<std::uint16_t>(raw - 1u);
}

// The authored name the entity was minted under, or its self key when the row that minted it
// took no string argument.
std::string probe_entity_label(void* entity)
{
    if (entity == nullptr) {
        return "(none)";
    }
    const std::uint16_t id = probe_entity_id(entity);
    const std::map<std::uint16_t, std::string>::const_iterator found = g_probe.entity_labels.find(id);
    if (found != g_probe.entity_labels.end() && !found->second.empty()) {
        return found->second;
    }
    return bsp::mission_entity_self_key(id);
}

ProbeState::StubEntity& probe_entity_state(void* entity)
{
    const std::uint16_t id = probe_entity_id(entity);
    if (g_probe.entity_state.find(id) == g_probe.entity_state.end()) {
        g_probe.entity_state_order.push_back(id);
    }
    return g_probe.entity_state[id];
}

void record_navigator_step(const char* step) { g_probe.navigator_host_steps[step] += 1; }

// 0088A810's four reads, over the probe's Lua stack.
class ProbeCommandTargetSource final : public bsp::LuaCommandTargetSource {
public:
    explicit ProbeCommandTargetSource(lua_State* state) : state_(state) {}

    bool argument_id_field_is_nil(int index) override
    {
        const int at = bsp::mission_binding_argument_slot(index);
        if (lua_istable(state_, at) == 0) {
            return true;  // 00b67910 on a non-table answers an unbound object, 00b65fb0 true
        }
        lua_getfield(state_, at, bsp::kEntitySelfFieldId);
        const bool is_nil = lua_isnil(state_, -1) != 0;
        lua_pop(state_, 1);
        return is_nil;
    }

    void* argument_entity(int index) override
    {
        const int at = bsp::mission_binding_argument_slot(index);
        if (lua_istable(state_, at) == 0) {
            return nullptr;
        }
        lua_getfield(state_, at, bsp::kEntitySelfFieldPtr);
        void* pointer = lua_islightuserdata(state_, -1) ? lua_touserdata(state_, -1) : nullptr;
        lua_pop(state_, 1);
        return pointer;
    }

    // 00888760 walks the value's elements. The probe reads slots 1..3 of an array-style table,
    // which is the shape the shipped scripts pass; anything else leaves the position at zero
    // and is reported as a step the probe could not perform.
    bool argument_vector3(int index, float out[3]) override
    {
        const int at = bsp::mission_binding_argument_slot(index);
        if (lua_istable(state_, at) == 0) {
            record_navigator_step("lua_object_read_vector3_00888760 (not a table)");
            return false;
        }
        for (int component = 0; component < 3; ++component) {
            lua_rawgeti(state_, at, component + 1);
            if (lua_isnumber(state_, -1) == 0) {
                lua_pop(state_, 1);
                record_navigator_step("lua_object_read_vector3_00888760 (missing component)");
                return false;
            }
            out[component] = static_cast<float>(lua_tonumber(state_, -1));
            lua_pop(state_, 1);
        }
        return true;
    }

    std::uint16_t entity_object_id(void* entity) override { return probe_entity_id(entity); }

private:
    lua_State* state_;
};

// The probe is not a game: every step below that would reach game state is recorded, and the
// three that only touch the entity's own fields are applied to the stub record.
class ProbeNavigatorHost final : public bsp::LuaBindingNavigatorHost {
public:
    explicit ProbeNavigatorHost(lua_State* state) : state_(state) {}

    void* argument_entity(int index) override
    {
        const int at = bsp::mission_binding_argument_slot(index);
        if (lua_istable(state_, at) == 0) {
            return nullptr;
        }
        lua_getfield(state_, at, bsp::kEntitySelfFieldPtr);
        void* pointer = lua_islightuserdata(state_, -1) ? lua_touserdata(state_, -1) : nullptr;
        lua_pop(state_, 1);
        return pointer;
    }

    int argument_integer(int index) override
    {
        return static_cast<int>(lua_tointeger(state_, bsp::mission_binding_argument_slot(index)));
    }

    bool argument_boolean(int index) override
    {
        return lua_toboolean(state_, bsp::mission_binding_argument_slot(index)) != 0;
    }

    // 00888d20 reads `Ptr` without 00888aa0's entity-table validation, so the probe does the
    // same: any table carrying light userdata answers.
    void* argument_ptr_field(int index) override { return argument_entity(index); }

    void entity_issue_command(void* entity, std::uint32_t command_object,
                              const bsp::SceneCommandTarget& target, int flags) override
    {
        record_navigator_step("entity_issue_command_0077d600");
        const char* name = command_object == bsp::kCommandObjectAttackMove ? "attackmove" : "moveto";
        std::string order(name);
        order += " -> ";
        if (target.kind == 1) {
            order += probe_entity_label(target.object);
        } else if (target.position_valid != 0) {
            char buffer[96];
            std::snprintf(buffer, sizeof(buffer), "position %.2f %.2f %.2f",
                          static_cast<double>(target.position[0]),
                          static_cast<double>(target.position[1]),
                          static_cast<double>(target.position[2]));
            order += buffer;
        } else {
            order += "unresolved target (origin)";
        }
        if (flags != bsp::kNavigatorIssueFlags) {
            order += " [flags differ]";
        }
        probe_entity_state(entity).orders.push_back(order);
    }

    // 008162b0 for MDestroyer; a predicate the probe cannot evaluate. Answering true is the
    // arm that does something, which is what makes the rest of 0077c8d0 observable; the false
    // arm is recorded as unexercised in the report rather than silently chosen.
    bool entity_command_is_available(void* entity, const char* command_name, void* target) override
    {
        (void)entity;
        (void)command_name;
        (void)target;
        record_navigator_step("entity_command_is_available_vtable16c (answered true)");
        return true;
    }

    int entity_route_slot(void* entity) override
    {
        (void)entity;
        record_navigator_step("entity_route_slot_1ac");
        return -1;  // out of 0..7, so 00905300 is not reached and nothing is invented
    }

    void slot_counter_increment(int slot) override
    {
        (void)slot;
        record_navigator_step("slot_counter_increment_00905300");
    }

    std::uint16_t entity_object_id(void* entity) override { return probe_entity_id(entity); }

    void session_route_formation_message(void* follower, std::uint16_t leader_object_id) override
    {
        record_navigator_step("session_route_formation_message_0077c8d0_type76");
        ProbeState::StubEntity& state = probe_entity_state(follower);
        state.formation_set = true;
        state.formation_leader = probe_entity_label(
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(leader_object_id) + 1u));
    }

    void entity_set_skill_level(void* entity, int level) override
    {
        ProbeState::StubEntity& state = probe_entity_state(entity);
        state.skill_level_set = true;
        state.skill_level = level;
    }

    bool entity_is_kind_of(void* entity, int class_id) override
    {
        (void)entity;
        (void)class_id;
        record_navigator_step("entity_is_kind_of_vtable5c (answered false)");
        return false;  // the local-write arm; the routed arm needs a session this probe lacks
    }

    void entity_set_repair_enabled_field(void* entity, bool enabled) override
    {
        ProbeState::StubEntity& state = probe_entity_state(entity);
        state.repair_set = true;
        state.repair_enabled = enabled;
        state.repair_routed = false;
    }

    void session_route_repair_enable_message(void* entity, bool enabled) override
    {
        record_navigator_step("session_route_repair_enable_message_008ad330_type9f");
        ProbeState::StubEntity& state = probe_entity_state(entity);
        state.repair_set = true;
        state.repair_enabled = enabled;
        state.repair_routed = true;
    }

    // The probe is a single process with no session: [00E188A8]+1FE4h is 0 there, which is the
    // direct arm. Same choice the core host makes for the campaign fields.
    int game_session_mode() override
    {
        record_navigator_step("game_session_mode_1fe4 (answered 0)");
        return 0;
    }

    int game_effective_game_mode() override
    {
        record_navigator_step("game_effective_game_mode_004bca50");
        return 0;
    }

    void role_owner_set_role_available(void* owner, int role, int value) override
    {
        record_navigator_step("role_owner_set_role_available_vtable148");
        char buffer[128];
        std::snprintf(buffer, sizeof(buffer), "%s role %d = %d",
                      probe_entity_label(owner).c_str(), role, value);
        g_probe.role_rows.push_back(buffer);
    }

    void session_route_role_message(void* owner, int role, int value) override
    {
        (void)owner;
        (void)role;
        (void)value;
        record_navigator_step("session_route_role_message_008ab850_type4c");
    }

    void game_assign_party_player_slots(int value) override
    {
        (void)value;
        record_navigator_step("game_assign_party_player_slots_004c3840");
    }

private:
    lua_State* state_;
};

ProbeNavigatorHost* g_navigator_host = nullptr;

// What the eight bindings did, per ship, in the order the script first addressed each.
void report_navigator_bindings()
{
    std::cout << "\nnavigator bindings: stub entity state after the mission's own orders\n";
    if (g_probe.entity_state_order.empty()) {
        std::cout << "  no entity was addressed by one of the eight\n";
    }
    for (std::size_t i = 0; i < g_probe.entity_state_order.size(); ++i) {
        const std::uint16_t id = g_probe.entity_state_order[i];
        const ProbeState::StubEntity& state = g_probe.entity_state[id];
        void* pointer = reinterpret_cast<void*>(static_cast<std::uintptr_t>(id) + 1u);
        std::cout << "  " << probe_entity_label(pointer);
        if (state.skill_level_set) {
            std::cout << "  skill=" << state.skill_level;
        }
        if (state.repair_set) {
            std::cout << "  repair=" << (state.repair_enabled ? "on" : "off")
                      << (state.repair_routed ? " (routed)" : " (local +378h)");
        }
        if (state.formation_set) {
            std::cout << "  follows=" << state.formation_leader;
        }
        std::cout << "\n";
        for (std::size_t order = 0; order < state.orders.size(); ++order) {
            std::cout << "      order " << state.orders[order] << "\n";
        }
    }
    if (!g_probe.role_rows.empty()) {
        std::cout << "  SetRoleAvailable rows:\n";
        for (std::size_t i = 0; i < g_probe.role_rows.size(); ++i) {
            std::cout << "      " << g_probe.role_rows[i] << "\n";
        }
    }
    std::cout << "native steps the navigator host recorded rather than performed:\n";
    for (const std::pair<const std::string, int>& row : g_probe.navigator_host_steps) {
        std::cout << "  " << row.second << "  " << row.first << "\n";
    }
}

// Runs the reconstructed routine for `name` when --navigator-bindings is on. Returns -1 when
// the name is not one of the eight, which is the caller's signal to fall back to the stub.
int run_navigator_binding(lua_State* L, const char* name)
{
    if (!g_probe.navigator_bindings || g_navigator_host == nullptr) {
        return -1;
    }
    ProbeCommandTargetSource targets(L);
    int produced = -1;
    if (std::strcmp(name, "NavigatorAttackMove") == 0) {
        produced = bsp::lua_binding_navigator_attack_move(targets, *g_navigator_host);
    } else if (std::strcmp(name, "NavigatorMoveToRange") == 0
               || std::strcmp(name, "NavigatorMoveToPos") == 0
               || std::strcmp(name, "NavigatorDirectMoveToRange") == 0) {
        produced = bsp::lua_binding_navigator_move_to(targets, *g_navigator_host);
    } else if (std::strcmp(name, "JoinFormation") == 0) {
        produced = bsp::lua_binding_join_formation(*g_navigator_host);
    } else if (std::strcmp(name, "SetSkillLevel") == 0) {
        produced = bsp::lua_binding_set_skill_level(*g_navigator_host);
    } else if (std::strcmp(name, "RepairEnable") == 0) {
        bsp::RepairEnableArm arm = bsp::RepairEnableArm::kLocalFieldWrite;
        produced = bsp::lua_binding_repair_enable(*g_navigator_host, arm);
    } else if (std::strcmp(name, "SetRoleAvailable") == 0) {
        bsp::SetRoleAvailableArm arm = bsp::SetRoleAvailableArm::kDirectCall;
        produced = bsp::lua_binding_set_role_available(*g_navigator_host, arm);
    }
    return produced;
}

// ---------------------------------------------------------------------------
// --stand-in-random: a probe fixture, NOT a reconstruction
// ---------------------------------------------------------------------------
//
// usn_2_java's luaInit reaches its order function only through luaPickRnd
// (Scripts/global/commandhelpers.lua:1003), which compares the result of luaRnd() and so of
// the native row `random` 0088C160. Stubbed, that row pushes nothing, luaRnd returns nil, and
// luaInit dies at that comparison before a single ship is ordered -- which is the
// "status=2 ... attempt to compare number with nil" line of docs/GAME_EXECUTABLE.md
// milestone 2l.
//
// 0088C160 was not reconstructed by this packet and is not in its lease. All that was read of
// it is the argument-count switch at 0088C1xx through 00B663F0: zero arguments draw against
// the float at 00D11318, one argument against arg0+1, two against arg0+1 and arg1, each
// through 00BD2F10 + 00BF7420 and pushed as an integer. The generator, its seeding and the
// exact bound convention were NOT read.
//
// So this is a deterministic fixture with the native's argument shape and none of its
// numerics: it exists so the rest of luaInit runs and the eight navigator bindings can be
// observed against real script control flow. Any report that depends on it says so. It is off
// by default, and the default run keeps reproducing milestone 2l's failure exactly.
int stand_in_random_binding(lua_State* L)
{
    static std::uint32_t state = 0x13579bdfu;  // fixed seed: the run must be reproducible
    state = state * 1664525u + 1013904223u;
    const std::uint32_t draw = (state >> 16) & 0x7fffu;
    g_probe.stand_in_random_calls += 1;

    const int argc = lua_gettop(L);
    if (argc == 0) {
        lua_pushnumber(L, static_cast<lua_Number>(draw));
        return 1;
    }
    long low = 0;
    long high = static_cast<long>(lua_tointeger(L, 1));
    if (argc >= 2) {
        low = static_cast<long>(lua_tointeger(L, 1));
        high = static_cast<long>(lua_tointeger(L, 2));
    }
    if (high < low) {
        const long swap = low;
        low = high;
        high = swap;
    }
    const long span = high - low + 1;
    const long value = span > 0 ? low + static_cast<long>(draw % static_cast<std::uint32_t>(span))
                                : low;
    lua_pushnumber(L, static_cast<lua_Number>(value));
    return 1;
}

// Every binding is installed as this one C function plus an index upvalue.
// 006b8610 uses nup = 0 because each native row is a distinct function; the
// probe needs identity at call time, so it carries the row index instead. That
// is the only divergence from the native registration, and it is invisible to
// the script: the global is still a plain C closure on a plain global name.
int binding_stub(lua_State* L)
{
    const int index = static_cast<int>(lua_tointeger(L, lua_upvalueindex(1)));
    const int argc = lua_gettop(L);
    const bsp::MissionLuaBinding& row = bsp::mission_lua_bindings()[static_cast<std::size_t>(index)];
    g_probe.calls.push_back(NativeCall{g_probe.phase, row.name, argc});
    g_probe.call_counts[row.name] += 1;
    if (!g_probe.current_script.empty()) {
        g_probe.calls_by_script[row.name][g_probe.current_script] += 1;
    }
    if (g_probe.stand_in_random && std::strcmp(row.name, "random") == 0) {
        return stand_in_random_binding(L);
    }
    const int navigator_results = run_navigator_binding(L, row.name);
    if (navigator_results >= 0) {
        return navigator_results;
    }
    const int core_results = run_core_binding(L, row.name);
    if (core_results >= 0) {
        return core_results;
    }
    if (!g_probe.model_self_table || !bsp::mission_binding_returns_entity(row.name)) {
        return 0;
    }
    // The entity tail, docs/LUA_BINDING_ENTITY.md: one value, or nil when the lookup found
    // nothing (0089903c). 00b66400 makes the count `lua_gettop - base`, which for this tail is
    // always exactly one because the handler pushes exactly one value above its arguments.
    const std::string key = push_new_entity_table(L);
    if (key.empty()) {
        lua_pushnil(L);
        return 1;
    }
    // The id push_new_entity_table just consumed, labelled with the row's first string
    // argument when it has one: for FindEntity that is the ship's authored name, which is what
    // lets --navigator-bindings report orders per ship instead of per anonymous key.
    if (argc >= 1 && lua_isstring(L, 1) != 0) {
        g_probe.entity_labels[static_cast<std::uint16_t>(g_probe.next_entity_id - 1u)]
            = lua_tostring(L, 1);
    }
    // CreateScript registers a named Lua function against the entity it just created; the
    // engine calls it later through 00887750 with that entity's table as `this`, followed by
    // the stack range 00898945/0089894a selects, which is slots 2..top when more than one
    // argument was given and nothing at all when only the name was.
    if (std::strcmp(row.name, "CreateScript") == 0 && argc >= 1 && lua_isstring(L, 1)) {
        CreatedScript created;
        created.name = lua_tostring(L, 1);
        created.self_key = key;
        if (argc > 1) {
            for (int slot = 2; slot <= argc; ++slot) {
                lua_pushvalue(L, slot);
                created.argument_refs.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
            }
        }
        g_probe.created_scripts.push_back(created);
    }
    return 1;
}

// 006b8720 is the machine's panic function; its body was not read by this
// packet, so the probe installs its own rather than guess. Reaching it means
// an unprotected error escaped, which is a probe bug, not a finding.
int probe_panic(lua_State* L)
{
    const char* message = lua_tostring(L, -1);
    std::cerr << "lua panic: " << (message != nullptr ? message : "(no message)") << "\n";
    return 0;
}

bool read_file(const std::string& path, std::vector<char>& output)
{
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) {
        return false;
    }
    input.seekg(0, std::ios::end);
    const std::streamoff size = input.tellg();
    if (size < 0) {
        return false;
    }
    input.seekg(0, std::ios::beg);
    output.assign(static_cast<std::size_t>(size), '\0');
    if (size > 0) {
        input.read(output.data(), size);
    }
    return static_cast<bool>(input);
}

// 006b89f0. top = lua_gettop; luaL_loadbuffer; on success lua_pcall(0,
// LUA_MULTRET, 0) with errfunc 0, so a chunk gets no traceback; on failure
// lua_tolstring the message even when nothing will read it; then restore the
// entry top. Returns true when the chunk failed.
bool run_chunk(lua_State* L, const std::vector<char>& bytes, const char* chunk_name,
    std::string& error_out)
{
    const int top = lua_gettop(L);
    int status = luaL_loadbuffer(L, bytes.data(), bytes.size(), chunk_name);
    if (status == 0) {
        status = lua_pcall(L, 0, LUA_MULTRET, bsp::kLuaNoErrorHandler);
    }
    const bool failed = status != 0;
    if (lua_gettop(L) > top) {
        if (failed) {
            const char* message = lua_tolstring(L, -1, nullptr);
            error_out = message != nullptr ? message : "(no message)";
        }
        lua_settop(L, top);
    }
    return failed;
}

void note_error(const std::string& message)
{
    if (g_probe.first_error.empty() && !message.empty()) {
        g_probe.first_error = message;
        g_probe.first_error_phase = g_probe.phase;
    }
}

bool run_script_file(lua_State* L, const std::string& vfs_path)
{
    std::vector<char> bytes;
    const std::string disk_path = g_probe.game_root + "/" + vfs_path;
    if (!read_file(disk_path, bytes)) {
        return false;
    }
    std::string error;
    if (run_chunk(L, bytes, vfs_path.c_str(), error)) {
        note_error(error);
        std::cout << "    chunk error in " << vfs_path << ": " << error << "\n";
        if (error.find("nesting of [[") != std::string::npos) {
            // Stock lua-5.1.1 sets LUA_COMPAT_LSTR to 1, which turns a nested
            // long bracket into a lex error. The executable's copy of the same
            // lexer function (00a71350) carries that function's other two
            // messages and not this one, so the game built with the value 2.
            std::cout << "      the matched library must be built with "
                         "LUA_COMPAT_LSTR=2; see cmake/startup.cmake\n";
        }
    }
    return true;
}

// DoFile is NOT one of the 560 rows. 00b6a303 installs it as a global from the
// LuaStateOwner layer with callback 00b69e00, which runs the named script
// through 00b69d40. The probe follows it so the script reaches its real depth;
// --stub-dofile turns it back into a logging stub to show the difference.
int dofile_stub(lua_State* L)
{
    const int argc = lua_gettop(L);
    const char* path = lua_tolstring(L, 1, nullptr);
    const std::string vfs_path = path != nullptr ? path : "";
    g_probe.calls.push_back(NativeCall{g_probe.phase, "DoFile", argc});
    g_probe.call_counts["DoFile"] += 1;
    g_probe.dofile_paths.push_back(vfs_path);
    if (!g_probe.follow_dofile || vfs_path.empty()) {
        return 0;
    }
    const std::string saved_phase = g_probe.phase;
    g_probe.phase = "DoFile " + vfs_path;
    if (!run_script_file(L, vfs_path)) {
        g_probe.dofile_missing.push_back(vfs_path);
        std::cout << "    DoFile could not open " << vfs_path << "\n";
    }
    g_probe.phase = saved_phase;
    return 0;
}

// 006b8740 step 4. The pair table at 00cf8350 is walked in order and each row
// is run as lua_pushcclosure(open, 0); lua_pushstring(name); lua_call(1, 0).
// luaopen_package is absent, so the state has no require and no loadlib.
void open_standard_libraries(lua_State* L)
{
    const lua_CFunction openers[] = {
        luaopen_base, luaopen_table, luaopen_io, luaopen_os,
        luaopen_string, luaopen_math, luaopen_debug,
    };
    const std::size_t opener_count = sizeof(openers) / sizeof(openers[0]);
    if (opener_count != bsp::kMissionLuaStandardLibraryCount) {
        std::cerr << "library table size disagrees with the header\n";
        return;
    }
    for (std::size_t i = 0; i < opener_count; ++i) {
        lua_pushcclosure(L, openers[i], 0);
        lua_pushstring(L, bsp::kMissionLuaStandardLibraries[i].name);
        lua_call(L, 1, 0);
    }
}

// 00884be0 step 3 through 006b8610: lua_pushcclosure(fn, 0) then
// lua_setfield(LUA_GLOBALSINDEX, name). Every binding is a plain global; there
// is no namespace table and no metatable.
void register_bindings(lua_State* L)
{
    const bsp::MissionLuaBinding* rows = bsp::mission_lua_bindings();
    const std::size_t count = bsp::mission_lua_binding_count();
    for (std::size_t i = 0; i < count; ++i) {
        lua_pushinteger(L, static_cast<lua_Integer>(i));
        lua_pushcclosure(L, binding_stub, 1);
        lua_setfield(L, LUA_GLOBALSINDEX, rows[i].name);
    }
}

// 00887750, reduced to the no-argument, no-self-key case the four mission
// entry points use. debugtrap is fetched as the error handler before the name
// is resolved, the name is split on '.', and the stack is restored afterwards.
//
// use_error_handler = false reruns the same call with errfunc 0. The native
// always passes the handler; the probe needs the bare form only to show what
// the handler discards, because 008c8390 returns 0 results and Lua 5.1 then
// replaces the error object with nil.
bool call_entry_point(lua_State* L, const std::string& name, std::string& error_out,
    bool use_error_handler)
{
    const int saved_top = lua_gettop(L);
    int error_handler = bsp::kLuaNoErrorHandler;
    if (use_error_handler) {
        lua_getfield(L, LUA_GLOBALSINDEX, bsp::kMissionLuaErrorHandler);
        error_handler = lua_gettop(L);
    }

    const std::vector<std::string> segments = bsp::split_lua_entry_point_name(name);
    if (segments.empty()) {
        lua_settop(L, saved_top);
        return false;
    }
    lua_getfield(L, LUA_GLOBALSINDEX, segments[0].c_str());
    for (std::size_t i = 1; i < segments.size(); ++i) {
        lua_pushstring(L, segments[i].c_str());
        lua_gettable(L, -2);
        lua_remove(L, -2);
    }
    if (lua_isnil(L, -1)) {
        lua_settop(L, saved_top);
        return false;
    }
    const int status = lua_pcall(L, 0, LUA_MULTRET, error_handler);
    if (status != 0) {
        const char* message = lua_tolstring(L, -1, nullptr);
        error_out = message != nullptr ? message : "(no message)";
    }
    lua_settop(L, saved_top);
    return status == 0;
}

// 00887750 with a non-empty self key: block 008878c7..0089790f pushes thisTable[key] as the
// call's first argument. The four steps are NamedCallSelfStep in the header, emitted in order.
// Everything else is call_entry_point's path, so only the self push and the argument count
// differ. This is how a script registered by CreateScript receives its `this`, which is how the
// shipped scripts get the global `Mission` (`Mission = this` on the first line of luaInit).
bool call_named_with_self(lua_State* L, const std::string& name, const std::string& self_key,
    const std::vector<int>& argument_refs, std::string& error_out, bool use_error_handler = true)
{
    const int saved_top = lua_gettop(L);
    int error_handler = bsp::kLuaNoErrorHandler;
    if (use_error_handler) {
        lua_getfield(L, LUA_GLOBALSINDEX, bsp::kMissionLuaErrorHandler);
        error_handler = lua_gettop(L);
    }

    const std::vector<std::string> segments = bsp::split_lua_entry_point_name(name);
    if (segments.empty()) {
        lua_settop(L, saved_top);
        return false;
    }
    lua_getfield(L, LUA_GLOBALSINDEX, segments[0].c_str());
    for (std::size_t i = 1; i < segments.size(); ++i) {
        lua_pushstring(L, segments[i].c_str());
        lua_gettable(L, -2);
        lua_remove(L, -2);
    }
    if (lua_isnil(L, -1)) {
        lua_settop(L, saved_top);
        return false;
    }

    const bsp::MissionNamedCallSelf self = bsp::mission_named_call_self(&self_key);
    int argc = 0;
    if (self.pushes_self) {
        lua_getfield(L, LUA_GLOBALSINDEX, bsp::kMissionSelfTableGlobal); // 006b8460
        lua_pushstring(L, self.self_key.c_str());                        // 006b8120
        lua_gettable(L, -2);                                             // 006b8470
        lua_remove(L, -2);                                               // 006b7ea0
        argc = 1;
    }
    // The forwarded stack range, 00887ee1..00887f19. The native reads it out of the live Lua
    // stack of the C function that registered the script; the probe replays it from the
    // registry references it took at registration time.
    for (std::size_t i = 0; i < argument_refs.size(); ++i) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, argument_refs[i]);
        ++argc;
    }

    const int status = lua_pcall(L, argc, LUA_MULTRET, error_handler);
    if (status != 0) {
        const char* message = lua_tolstring(L, -1, nullptr);
        error_out = message != nullptr ? message : "(no message)";
    }
    lua_settop(L, saved_top);
    return status == 0;
}

// 004e01f7..004e02ac in BSP_Game_LoadMissionScene: create the global `thisTable` when it is
// currently nil (00b65fb0 is-nil, then 00b67580 assigns a fresh table), and clear the global
// `recon` (004e0305, 00b67350 set-nil). Both run over the mission LuaStateOwner at game+1A0Ch.
void create_mission_self_table(lua_State* L)
{
    lua_getfield(L, LUA_GLOBALSINDEX, bsp::kMissionSelfTableGlobal);
    const bool is_nil = lua_isnil(L, -1) != 0;
    lua_pop(L, 1);
    if (bsp::mission_self_table_needs_creation(is_nil)) {
        lua_createtable(L, 0, 0);
        lua_setfield(L, LUA_GLOBALSINDEX, bsp::kMissionSelfTableGlobal);
    }
    lua_pushnil(L);
    lua_setfield(L, LUA_GLOBALSINDEX, bsp::kMissionReconGlobal);
}

// 00803A40 BSP_ReconTables_Install, already reconstructed in src/recon_values.cpp over the
// game+1A08 mission host instance: the global `recon`, integer children 0..2, the relations
// enemy/neutral/unknown/own in that order, then 008039E0's nineteen category tables under each.
// The probe rebuilds the same shape on its own state; the reconstruction cannot be called here
// because it drives a ReconValuesHost this probe does not have.
//
// Its sole caller is 004DC6A0 BSP_Game_ConstructGlobalSubsystems, a bring-up step, while
// 004E0305 nils `recon` on every mission load. Those two facts do not compose: something must
// rebuild the table per mission and this packet did not find it. --recon-tables is the switch
// that shows which of the two remaining sweep failures the missing rebuild accounts for.
void install_recon_tables(lua_State* L)
{
    lua_createtable(L, 3, 0);
    for (int party = 0; party <= 2; ++party) {          // 00803750, integer children 0..2
        lua_createtable(L, 0, 4);
        const char* relations[] = {"enemy", "neutral", "unknown", "own"}; // 008037d0, in order
        for (std::size_t r = 0; r < 4; ++r) {
            lua_createtable(L, 0, static_cast<int>(bsp::recon_category_names_00e0b590.size()));
            for (std::size_t c = 0; c < bsp::recon_category_names_00e0b590.size(); ++c) {
                lua_createtable(L, 0, 0);               // 008039e0, one table per category
                lua_setfield(L, -2, bsp::recon_category_names_00e0b590[c]);
            }
            lua_setfield(L, -2, relations[r]);
        }
        lua_rawseti(L, -2, party);
    }
    lua_setfield(L, LUA_GLOBALSINDEX, bsp::kMissionReconGlobal);
}

bool file_exists(const std::string& path)
{
    std::ifstream input(path.c_str(), std::ios::binary);
    return static_cast<bool>(input);
}

// 00886370, reduced to the disk case. The native enumerates .luab first and
// then runs a .lua only when its computed compiled name did not appear in the
// first pass. The install ships no .luab under either folder, so every .lua
// runs; the probe keeps the rule anyway so the ordering stays honest. The
// native's enumeration order comes from the VFS provider list, which this
// probe cannot reproduce, so it sorts by name instead. That divergence only
// matters for scripts that redefine each other's globals.
std::size_t run_script_folder(lua_State* L, const std::string& directory)
{
    namespace fs = std::filesystem;
    const fs::path root = fs::path(g_probe.game_root) / directory;
    std::error_code ec;
    if (!fs::is_directory(root, ec)) {
        return 0;
    }
    std::vector<std::string> compiled;
    std::vector<std::string> source;
    for (const fs::directory_entry& entry : fs::directory_iterator(root, ec)) {
        if (!entry.is_regular_file(ec)) {
            continue;
        }
        const std::string stem = entry.path().stem().string();
        const std::string extension = entry.path().extension().string();
        if (extension == ".luab") {
            compiled.push_back(stem);
        } else if (extension == ".lua") {
            source.push_back(stem);
        }
    }
    std::sort(compiled.begin(), compiled.end());
    std::sort(source.begin(), source.end());

    std::size_t ran = 0;
    for (const std::string& stem : compiled) {
        if (run_script_file(L, directory + "/" + stem + ".luab")) {
            ++ran;
        }
    }
    for (const std::string& stem : source) {
        if (std::find(compiled.begin(), compiled.end(), stem) != compiled.end()) {
            continue; // shadowed by the compiled pass, even when that load failed
        }
        if (run_script_file(L, directory + "/" + stem + ".lua")) {
            ++ran;
        }
    }
    return ran;
}

// ---------------------------------------------------------------------------
// --sweep: every installed mission script through the same state
// ---------------------------------------------------------------------------

struct MissionOutcome {
    std::string name;
    bool chunk_ran{false};
    bool all_entry_points_ok{true};
    std::string failing_step;   // the entry point or "chunk"
    std::string error;
    // Errors from functions CreateScript registered. These run from the frame loop, not from
    // the load path, so they are reported apart from the load result.
    std::vector<std::pair<std::string, std::string>> script_errors;
};

// The four entry-point globals, cleared before each mission so that a script which omits one
// does not inherit the previous script's. The native gets a fresh state per mission load; the
// sweep reuses one state because the global folders cost about 200k lines to rebuild. That is
// the sweep's one divergence and it is confined to the global namespace: clearing the four
// names restores the 0045f440/0045f520 defined-check to the same answer a fresh state gives.
void clear_entry_points(lua_State* L)
{
    for (std::size_t i = 0; i < bsp::kMissionLuaEntryPointCount; ++i) {
        const std::vector<std::string> segments
            = bsp::split_lua_entry_point_name(bsp::kMissionLuaEntryPoints[i].name);
        if (segments.size() != 1) {
            continue; // a dotted name lives in a table the mission did not create
        }
        lua_pushnil(L);
        lua_setfield(L, LUA_GLOBALSINDEX, segments[0].c_str());
    }
}

// One mission: 008860b0's chunk, then the entry points through the 0045f440/0045f520 guard,
// then every function CreateScript registered, called the way 00887750 calls it.
MissionOutcome run_one_mission(lua_State* L, const std::string& mission_name)
{
    MissionOutcome outcome;
    outcome.name = mission_name;
    clear_entry_points(L);
    create_mission_self_table(L);
    if (g_probe.install_recon_tables) {
        install_recon_tables(L);
    }
    g_probe.created_scripts.clear();
    g_probe.current_script = mission_name;

    g_probe.phase = "mission chunk";
    if (!run_script_file(L, bsp::mission_script_path(mission_name))) {
        outcome.failing_step = "chunk";
        outcome.error = "could not open or load the chunk";
        g_probe.current_script.clear();
        return outcome;
    }
    outcome.chunk_ran = true;

    for (std::size_t i = 0; i < bsp::kMissionLuaEntryPointCount; ++i) {
        const bsp::MissionLuaEntryPoint& entry = bsp::kMissionLuaEntryPoints[i];
        g_probe.phase = entry.name;
        lua_getfield(L, LUA_GLOBALSINDEX, entry.name);
        const bool defined = !lua_isnil(L, -1);
        lua_pop(L, 1);
        if (!defined) {
            continue;
        }
        std::string error;
        if (!call_entry_point(L, entry.name, error, true)) {
            std::string bare;
            call_entry_point(L, entry.name, bare, false);
            outcome.all_entry_points_ok = false;
            outcome.failing_step = entry.name;
            outcome.error = bare.empty() ? error : bare;
            g_probe.current_script.clear();
            return outcome;
        }
    }

    // The scripts CreateScript registered. The native reaches these from the frame loop rather
    // than from the load path, so a failure here is a load-time contract claim only in the weak
    // sense: it shows the self argument resolves. `Mission = this` happens on this call.
    const std::vector<CreatedScript> scripts = g_probe.created_scripts;
    for (const CreatedScript& script : scripts) {
        g_probe.phase = "CreateScript:" + script.name;
        std::string error;
        if (!call_named_with_self(L, script.name, script.self_key, script.argument_refs, error)
            && !error.empty()) {
            std::string bare;
            call_named_with_self(L, script.name, script.self_key, script.argument_refs, bare,
                false);
            // Not a load failure: CreateScript bodies run from the frame loop, not from
            // 008860b0 or the entry points. Recorded separately so the sweep's load result
            // stays about what the load path actually reaches.
            outcome.script_errors.emplace_back(script.name, bare.empty() ? error : bare);
        }
    }
    g_probe.current_script.clear();
    return outcome;
}

std::vector<std::string> installed_mission_names()
{
    namespace fs = std::filesystem;
    std::vector<std::string> names;
    const fs::path root = fs::path(g_probe.game_root) / "Scripts" / "missions";
    std::error_code ec;
    if (!fs::is_directory(root, ec)) {
        return names;
    }
    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(root, ec)) {
        if (!entry.is_regular_file(ec) || entry.path().extension() != ".lua") {
            continue;
        }
        const fs::path relative = fs::relative(entry.path(), root, ec);
        std::string name = relative.generic_string();
        name.erase(name.size() - 4); // ".lua"
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

int run_sweep(lua_State* L)
{
    const std::vector<std::string> names = installed_mission_names();
    std::cout << "sweep          : " << names.size() << " installed mission scripts\n";

    // 00885fb0 runs every content variant after the base file. The rule is
    // append_lua_script_overrides_00bdef90; on the installed copy Scripts/missions holds 299
    // files and every one of them ends in ".lua", so the variant pass adds nothing here and
    // the sweep's per-mission count is the base file alone.
    std::cout << "variant rule   : no variant-suffixed mission script ships; the pass is empty\n";

    std::vector<MissionOutcome> failures;
    std::vector<MissionOutcome> script_failures;
    std::size_t ok = 0;
    for (const std::string& name : names) {
        const MissionOutcome outcome = run_one_mission(L, name);
        if (outcome.chunk_ran && outcome.all_entry_points_ok) {
            ++ok;
        } else {
            failures.push_back(outcome);
        }
        if (!outcome.script_errors.empty()) {
            script_failures.push_back(outcome);
        }
    }

    std::cout << "\nloaded and ran cleanly: " << ok << " / " << names.size() << "\n";
    if (!failures.empty()) {
        std::cout << "load failures (" << failures.size() << "):\n";
        for (const MissionOutcome& failure : failures) {
            std::cout << "  " << failure.name << "  at " << failure.failing_step << ": "
                      << failure.error << "\n";
        }
    }
    if (!script_failures.empty()) {
        std::size_t error_count = 0;
        for (const MissionOutcome& outcome : script_failures) {
            error_count += outcome.script_errors.size();
        }
        std::cout << "CreateScript bodies that raised when called with `this` (" << error_count
                  << " in " << script_failures.size()
                  << " scripts; these run from the frame loop, not from the load path):\n";
        for (const MissionOutcome& outcome : script_failures) {
            for (const std::pair<std::string, std::string>& entry : outcome.script_errors) {
                std::cout << "  " << outcome.name << "  " << entry.first << ": " << entry.second
                          << "\n";
            }
        }
    }

    // The per-binding survey: how many distinct scripts reached each binding, over the whole
    // sweep. A binding no script reaches at load time is not dead; it is reached from the frame
    // loop or from a script the install does not ship.
    std::vector<std::pair<std::size_t, std::string>> ranked;
    for (const std::pair<const std::string, std::map<std::string, int>>& row
            : g_probe.calls_by_script) {
        ranked.emplace_back(row.second.size(), row.first);
    }
    std::sort(ranked.begin(), ranked.end(),
        [](const std::pair<std::size_t, std::string>& a,
           const std::pair<std::size_t, std::string>& b) {
            return a.first != b.first ? a.first > b.first : a.second < b.second;
        });
    if (g_probe.core_bindings) {
        std::cout << "\nreconstructed bindings, scripts reached / calls made:\n";
        for (const std::pair<const std::string, std::set<std::string>>& row
                : g_probe.core_binding_scripts) {
            std::cout << "  " << row.second.size() << "  " << row.first << "  ("
                      << g_probe.call_counts[row.first] << " calls)\n";
        }
        std::cout << "native steps the host recorded rather than performed:\n";
        for (const std::pair<const std::string, int>& row : g_probe.core_binding_calls) {
            std::cout << "  " << row.second << "  " << row.first << "\n";
        }
    }
    if (g_probe.navigator_bindings) {
        report_navigator_bindings();
    }

    std::cout << "\nbindings reached at load time: " << ranked.size() << " of "
              << bsp::mission_lua_binding_count() << "\n";
    std::cout << "scripts binding\n";
    for (const std::pair<std::size_t, std::string>& row : ranked) {
        std::cout << "  " << row.first << "  " << row.second
                  << (bsp::mission_binding_returns_entity(row.second.c_str()) ? "  [entity]" : "")
                  << "\n";
    }
    return failures.empty() ? 0 : 2;
}

} // namespace

int main(int argc, char** argv)
{
    std::string mission_name = kDefaultMissionName;
    g_probe.game_root = kDefaultGameRoot;
    bool sweep = false;
    int positional = 0;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--stub-dofile") {
            g_probe.follow_dofile = false;
        } else if (argument == "--skip-global-folders") {
            g_probe.run_global_folders = false;
        } else if (argument == "--skip-lobby-settings") {
            g_probe.create_lobby_settings = false;
        } else if (argument == "--no-self-table") {
            g_probe.model_self_table = false;
        } else if (argument == "--recon-tables") {
            g_probe.install_recon_tables = true;
        } else if (argument == "--core-bindings") {
            g_probe.core_bindings = true;
        } else if (argument == "--navigator-bindings") {
            g_probe.navigator_bindings = true;
        } else if (argument == "--stand-in-random") {
            g_probe.stand_in_random = true;
        } else if (argument == "--sweep") {
            sweep = true;
        } else if (argument == "--quiet") {
            g_probe.verbose = false;
        } else if (positional == 0) {
            g_probe.game_root = argument;
            positional = 1;
        } else if (positional == 1) {
            mission_name = argument;
            positional = 2;
        }
    }

    std::cout << "game root      : " << g_probe.game_root << "\n";
    std::cout << "mission name   : " << mission_name << "\n";
    std::cout << "DoFile         : " << (g_probe.follow_dofile ? "followed" : "stubbed") << "\n";
    std::cout << "core bindings  : "
              << (g_probe.core_bindings ? "the ten of docs/LUA_BINDING_CORE.md run for real"
                                        : "stubbed with every other row")
              << "\n";
    std::cout << "nav bindings   : "
              << (g_probe.navigator_bindings
                      ? "the eight of docs/LUA_BINDING_NAVIGATOR.md run for real"
                      : "stubbed with every other row")
              << "\n\n";

    // -- the machine, 006b8740 ----------------------------------------------
    lua_State* L = luaL_newstate();
    if (L == nullptr) {
        std::cerr << "luaL_newstate failed\n";
        return 1;
    }
    lua_atpanic(L, probe_panic);
    // The reconstructed routines read and push through this state; the host outlives every
    // binding call and is released with the state at the end of main.
    ProbeCoreHost core_host(L);
    g_core_host = &core_host;
    ProbeNavigatorHost navigator_host(L);
    g_navigator_host = &navigator_host;
    lua_gc(L, bsp::kLuaGcSetPause, bsp::kMissionLuaGcPause);
    open_standard_libraries(L);

    std::cout << "libraries      : " << bsp::kMissionLuaStandardLibraryCount
              << " opened, package/require absent\n";
    lua_getfield(L, LUA_GLOBALSINDEX, "require");
    const bool has_require = !lua_isnil(L, -1);
    lua_pop(L, 1);
    std::cout << "require global : " << (has_require ? "PRESENT (unexpected)" : "absent") << "\n";

    // -- the bindings, 00884be0 step 3 --------------------------------------
    register_bindings(L);
    const std::size_t binding_count = bsp::mission_lua_binding_count();
    std::cout << "bindings       : " << binding_count << " installed as plain globals, first \""
              << bsp::mission_lua_bindings()[0].name << "\" last \""
              << bsp::mission_lua_bindings()[binding_count - 1].name << "\"\n";

    // DoFile, from the LuaStateOwner layer at 00b6a303, not from the table.
    lua_pushcclosure(L, dofile_stub, 0);
    lua_setfield(L, LUA_GLOBALSINDEX, bsp::kMissionLuaStateOwnerGlobal);
    std::cout << "DoFile global  : installed from the owner layer (00b6a303), not from the table\n\n";

    // -- 00884be0 step 2, the platform chunk --------------------------------
    g_probe.phase = "platform chunk";
    {
        const char* source = bsp::kMissionLuaPlatformChunk;
        const std::vector<char> bytes(source, source + std::strlen(source));
        std::string error;
        if (run_chunk(L, bytes, source, error)) {
            note_error(error);
            std::cout << "platform chunk error: " << error << "\n";
        }
    }

    // -- 00884be0 step 4, fundamentals --------------------------------------
    g_probe.phase = "fundamentals";
    const bool fundamentals_ok = run_script_file(L, "Scripts/fundamentals.lua");
    std::cout << "fundamentals   : " << (fundamentals_ok ? "ran" : "MISSING on disk") << "\n";

    // -- 005e2f00, the LobbySettings table ----------------------------------
    // The native reads and writes the global table named by 00cf1f28 through
    // the LuaObject API, one slot per row of the pointer array at 00e08908.
    // The probe only creates the table and gives every recovered field a
    // value, because the values come from game state this probe has none of.
    // Without it the multiplayer mission scripts index a nil global at their
    // first line, which is what puts 005e2f00 before the entry points.
    if (g_probe.create_lobby_settings) {
        lua_createtable(L, 0, static_cast<int>(bsp::kLobbySettingsFieldCount));
        for (std::size_t slot = 0; slot < bsp::kLobbySettingsSlotCount; ++slot) {
            const char* field = bsp::lobby_settings_field_name(slot);
            if (field == nullptr) {
                continue; // slot 0Dh, the null pointer at 00e08940
            }
            lua_pushinteger(L, 0);
            lua_setfield(L, -2, field);
        }
        lua_setfield(L, LUA_GLOBALSINDEX, bsp::kLobbySettingsTable);
        std::cout << "LobbySettings  : created with " << bsp::kLobbySettingsFieldCount
                  << " fields (005e2f00)\n";
    }

    // -- 00886900, the global script folders --------------------------------
    // Not part of 00884be0: this runs earlier, from 004dc6a0 and 004e3aa0. It
    // is here because the mission chunk's entry points call globals only these
    // folders define. --skip-global-folders shows what breaks without it.
    if (g_probe.run_global_folders) {
        g_probe.phase = "Scripts/global/";
        const std::size_t global_count = run_script_folder(L, "Scripts/global");
        g_probe.phase = "Scripts/datatables/autoload/";
        const std::size_t autoload_count = run_script_folder(L, "Scripts/datatables/autoload");
        std::cout << "global folders : " << global_count << " from Scripts/global/, "
                  << autoload_count << " from Scripts/datatables/autoload/\n";
    } else {
        std::cout << "global folders : SKIPPED (--skip-global-folders)\n";
    }

    // -- 004e01f7, the self table -------------------------------------------
    // BSP_Game_LoadMissionScene creates the global `thisTable` when it is nil and clears the
    // global `recon`. Every entity-returning binding reads this table and the named-call self
    // argument is fetched out of it, so it has to exist before the mission chunk runs.
    if (g_probe.model_self_table) {
        create_mission_self_table(L);
        std::cout << "self table     : global \"" << bsp::kMissionSelfTableGlobal
                  << "\" created, \"" << bsp::kMissionReconGlobal << "\" cleared (004e01f7)\n";
        std::cout << "entity tail    : " << bsp::kEntityReturningBindingCount
                  << " bindings return thisTable[tostring(id)] (docs/LUA_BINDING_ENTITY.md)\n";
    } else {
        std::cout << "self table     : SKIPPED (--no-self-table)\n";
    }

    if (sweep) {
        const int status = run_sweep(L);
        lua_close(L);
        return status;
    }

    // -- the mission script path, 008860b0 ----------------------------------
    const std::string script_path = bsp::mission_script_path(mission_name);
    const bool script_present = file_exists(g_probe.game_root + "/" + script_path);
    std::cout << "script path    : \"" << script_path << "\" "
              << (script_present ? "present" : "ABSENT") << "\n";
    {
        // The bare short name derive_scene_short_name produces has no
        // subdirectory. Showing both settles which one the record must carry.
        const std::string bare = mission_name.substr(mission_name.find_last_of('/') + 1);
        const std::string bare_path = bsp::mission_script_path(bare);
        std::cout << "bare-name path : \"" << bare_path << "\" "
                  << (file_exists(g_probe.game_root + "/" + bare_path) ? "present" : "ABSENT")
                  << "\n\n";
    }

    // -- the mission chunk, 00885110 through 006b89f0 -----------------------
    g_probe.phase = "mission chunk";
    const std::size_t calls_before_chunk = g_probe.calls.size();
    const bool chunk_ran = run_script_file(L, script_path);
    if (!chunk_ran) {
        std::cout << "mission chunk could not be opened; stopping\n";
        lua_close(L);
        return 1;
    }
    std::cout << "load-time native calls (" << (g_probe.calls.size() - calls_before_chunk) << "):\n";
    for (std::size_t i = calls_before_chunk; i < g_probe.calls.size(); ++i) {
        std::cout << "  " << g_probe.calls[i].name << "  argc=" << g_probe.calls[i].argc
                  << "  [" << g_probe.calls[i].phase << "]\n";
    }
    if (!g_probe.dofile_paths.empty()) {
        std::cout << "DoFile targets:\n";
        for (const std::string& path : g_probe.dofile_paths) {
            std::cout << "  " << path << " "
                      << (file_exists(g_probe.game_root + "/" + path) ? "(resolved at the VFS root)"
                                                                      : "(NOT at the VFS root)")
                      << "\n";
        }
    }

    // -- the four entry points, 0045f440 / 0045f520 -------------------------
    std::cout << "\nentry points:\n";
    for (std::size_t i = 0; i < bsp::kMissionLuaEntryPointCount; ++i) {
        const bsp::MissionLuaEntryPoint& entry = bsp::kMissionLuaEntryPoints[i];
        g_probe.phase = entry.name;
        const std::size_t before = g_probe.calls.size();
        std::string error;
        lua_getfield(L, LUA_GLOBALSINDEX, entry.name);
        const bool defined = !lua_isnil(L, -1);
        lua_pop(L, 1);
        if (!defined) {
            std::cout << "  " << entry.name << ": not defined, skipped (the 0045f440/0045f520 guard)\n";
            continue;
        }
        const bool ok = call_entry_point(L, entry.name, error, true);
        std::cout << "  " << entry.name << ": " << (ok ? "ok" : "FAILED")
                  << ", " << (g_probe.calls.size() - before) << " native calls"
                  << (entry.threadsafe ? "  [0045f440]" : "  [0045f520]") << "\n";
        for (std::size_t c = before; c < g_probe.calls.size(); ++c) {
            std::cout << "      called " << g_probe.calls[c].name
                      << "  argc=" << g_probe.calls[c].argc << "\n";
        }
        if (!ok) {
            std::cout << "      through debugtrap: \"" << error << "\"\n";
            std::string bare_error;
            call_entry_point(L, entry.name, bare_error, false);
            std::cout << "      with errfunc 0   : \"" << bare_error << "\"\n";
            note_error(bare_error.empty() ? error : bare_error);
        }
    }

    // -- the scripts CreateScript registered, 00887750 with a self key -------
    if (!g_probe.created_scripts.empty()) {
        std::cout << "\nCreateScript registrations (" << g_probe.created_scripts.size()
                  << "), called with thisTable[key] as `this`:\n";
        const std::vector<CreatedScript> scripts = g_probe.created_scripts;
        for (const CreatedScript& script : scripts) {
            g_probe.phase = "CreateScript:" + script.name;
            std::string error;
            const bool ok
                = call_named_with_self(L, script.name, script.self_key, script.argument_refs, error);
            if (!ok && !error.empty()) {
                std::string bare;
                call_named_with_self(L, script.name, script.self_key, script.argument_refs, bare,
                    false);
                if (!bare.empty()) {
                    error = bare;
                }
            }
            std::cout << "  " << script.name << "(this=thisTable[\"" << script.self_key << "\"]"
                      << (script.argument_refs.empty()
                              ? ""
                              : ", +" + std::to_string(script.argument_refs.size()) + " forwarded")
                      << "): "
                      << (ok ? "ok" : (error.empty() ? "not defined, skipped" : "FAILED")) << "\n";
            if (!ok && !error.empty()) {
                std::cout << "      " << error << "\n";
                note_error(error);
            }
        }
        lua_getfield(L, LUA_GLOBALSINDEX, "Mission");
        std::cout << "  global Mission: " << (lua_istable(L, -1) ? "a table" : lua_typename(L, lua_type(L, -1)))
                  << "  (the scripts assign it from `this` inside luaInit)\n";
        lua_pop(L, 1);
    }

    // -- summary ------------------------------------------------------------
    std::cout << "\ndistinct natives called: " << g_probe.call_counts.size() << "\n";
    std::size_t unresolved = 0;
    for (const std::pair<const std::string, int>& entry : g_probe.call_counts) {
        if (bsp::find_mission_lua_binding(entry.first.c_str()) == nullptr) {
            ++unresolved;
            std::cout << "  not in the 00e0b7b8 table: " << entry.first << "\n";
        }
    }
    std::cout << "natives outside the table: " << unresolved << "\n";
    if (g_probe.navigator_bindings) {
        report_navigator_bindings();
    }
    if (g_probe.first_error.empty()) {
        std::cout << "first error: none\n";
    } else {
        std::cout << "first error [" << g_probe.first_error_phase << "]: " << g_probe.first_error << "\n";
    }

    lua_close(L);
    return 0;
}
