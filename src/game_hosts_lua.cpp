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
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/global_script_folders.hpp"
#include "bsp/lua_binding_core.hpp"
#include "bsp/mission_lua_bindings.hpp"
#include "bsp/mission_lua_machine.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/native_string.hpp"
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

// ---------------------------------------------------------------------------
// Milestone 2g: the ten rows of docs/LUA_BINDING_CORE.md
//
// The reconstruction reads its arguments and pushes its results through two
// interfaces, so the executable supplies them over the live lua_State it
// already owns. Index 0 is the first argument, which bsp::mission_binding_
// argument_slot turns into the lua_CFunction stack slot.
// ---------------------------------------------------------------------------

class CoreArgumentReader final : public bsp::LuaBindingArgumentReader {
public:
    explicit CoreArgumentReader(lua_State* state) : state_(state) {}
    int count() override { return lua_gettop(state_); }  // 00b663f0
    int get_integer(int index) override {
        // 00b66290 narrows with the CRT __ftol, which truncates toward zero.
        return static_cast<int>(lua_tointeger(state_, slot(index)));
    }
    double get_number(int index) override {
        return static_cast<double>(lua_tonumber(state_, slot(index)));
    }
    bool get_boolean(int index) override { return lua_toboolean(state_, slot(index)) != 0; }
    std::string get_string(int index) override {
        const char* text = lua_tolstring(state_, slot(index), nullptr);
        return text != nullptr ? std::string(text) : std::string();
    }
    bool is_string(int index) override { return lua_isstring(state_, slot(index)) != 0; }
    bool is_nil(int index) override { return lua_isnil(state_, slot(index)) != 0; }
    bool is_entity_table(int index) override { return entity_at(index) != nullptr; }
    void* entity_at(int index) override {
        // 00888aa0's rule: the entity is the light userdata the table carries in
        // `Ptr`. No table in this process carries one, because nothing creates
        // an entity, so every entity argument answers null. That is the state of
        // the process, not a substitution.
        const int at = slot(index);
        if (lua_istable(state_, at) == 0) return nullptr;
        lua_getfield(state_, at, bsp::kEntitySelfFieldPtr);
        void* pointer = lua_islightuserdata(state_, -1) ? lua_touserdata(state_, -1) : nullptr;
        lua_pop(state_, 1);
        return pointer;
    }

private:
    static int slot(int index) { return bsp::mission_binding_argument_slot(index); }
    lua_State* state_;
};

class CoreResultWriter final : public bsp::LuaBindingResultWriter {
public:
    explicit CoreResultWriter(lua_State* state) : state_(state) {}
    void push_number(int value) override {  // 00b664b0
        lua_pushnumber(state_, static_cast<lua_Number>(value));
    }
    void push_boolean(bool value) override { lua_pushboolean(state_, value ? 1 : 0); }
    void push_nil() override { lua_pushnil(state_); }

private:
    lua_State* state_;
};

// One native step of the ten, as a record or as a value this process owns.
class CoreBindingHost final : public bsp::LuaBindingCoreHost {
public:
    CoreBindingHost(GameMissionLuaHost& owner, lua_State* state)
        : owner_(owner), state_(state) {}

    int game_non_campaign_flag() override {
        return owner_.core_binding_state().non_campaign_flag;
    }
    int game_effective_difficulty() override {
        return owner_.core_binding_state().effective_difficulty;
    }

    void log_prepare_class(int class_id) override {
        static_cast<void>(class_id);
        record("LuaCore::log_prepare_class", "008c9099");
    }

    bool resolve_global_integer(const std::string& dotted_path, int& value) override {
        // 00b68d70 over the globals object. The datatable scripts the load ran
        // built the VehicleClass table on this very state, so the walk has the
        // same answer here as in the game. The rule is a string lookup first and
        // then a base-10 _atol retry, taken only when the conversion is non-zero
        // or the segment is the single character '0'; without it
        // VehicleClass.5 never resolves, because the table builds its rows under
        // integer keys (docs/LUA_BINDING_CORE.md).
        ++owner_.core_binding_state().prepare_class_lookups;
        value = 0;
        std::size_t begin = 0;
        lua_pushvalue(state_, LUA_GLOBALSINDEX);
        bool reached = true;
        while (begin < dotted_path.size()) {
            std::size_t end = begin;
            while (end < dotted_path.size() && dotted_path[end] != '.') ++end;
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
            lua_remove(state_, -2);
            begin = end < dotted_path.size() ? end + 1 : end;
        }
        const bool ok = reached && lua_isnumber(state_, -1) != 0;
        if (ok) {
            value = static_cast<int>(lua_tointeger(state_, -1));
            ++owner_.core_binding_state().prepare_class_resolved;
        }
        lua_pop(state_, 1);
        done("LuaCore::resolve_global_integer", "008c91ba");
        return ok;
    }

    void vehicle_class_mark_party_required(int class_id, int party) override {
        static_cast<void>(class_id);
        static_cast<void>(party);
        record("LuaCore::vehicle_class_mark_party_required", "008c9297");
    }
    void vehicle_class_get_or_create(int class_id, bool read_race) override {
        static_cast<void>(class_id);
        static_cast<void>(read_race);
        record("LuaCore::vehicle_class_get_or_create", "008c92be");
    }
    void music_director_set_level(int level) override {
        static_cast<void>(level);
        record("LuaCore::music_director_set_level", "008c4e1d");
    }
    void session_send_music_level(int level) override {
        static_cast<void>(level);
        record("LuaCore::session_send_music_level", "008c4e74");
    }
    bool entity_vcall_5c(void* entity, int selector) override {
        static_cast<void>(entity);
        static_cast<void>(selector);
        record("LuaCore::entity_vcall_5c", "008a8a83");
        return false;
    }
    void entity_vcall_2c(void* entity, int party, std::uint32_t entity_field_58) override {
        static_cast<void>(entity);
        static_cast<void>(party);
        static_cast<void>(entity_field_58);
        record("LuaCore::entity_vcall_2c", "008a8ae3");
    }
    void session_route_party_message(void* entity, int party) override {
        static_cast<void>(entity);
        static_cast<void>(party);
        record("LuaCore::session_route_party_message", "008a8ac4");
    }
    void scoring_set_real_play_time_running(bool running) override {
        // scoring+14A4h, a field of the object this process owns.
        owner_.core_binding_state().real_play_time_running = running;
        done("LuaCore::scoring_set_real_play_time_running", "008b8901");
    }
    void scoring_set_final_scoring_function_name(const std::string& name) override {
        owner_.core_binding_state().final_scoring_function = name;  // scoring+147Ch
        done("LuaCore::scoring_set_final_scoring_function_name", "008b8766");
    }
    void message_map_load(const std::string& name, int index) override {
        static_cast<void>(index);
        owner_.note_core_message_map(name);
        record("LuaCore::message_map_load", "008c6308");
    }
    void call_0088b6d0_0076a9f0_00765590(const std::string& name, int index) override {
        static_cast<void>(name);
        static_cast<void>(index);
        record("LuaCore::message_map_session_path", "008c63c3");
    }
    void entity_set_think_script_name(void* entity, const std::string& name) override {
        static_cast<void>(entity);
        owner_.note_core_think_name(name);
        record("LuaCore::entity_set_think_script_name", "008980e8");
    }
    void set_entity_message_suppression(void* entity, bool suppressed) override {
        static_cast<void>(entity);
        static_cast<void>(suppressed);
        record("LuaCore::set_entity_message_suppression", "008cffe4");
    }
    void set_global_message_suppression(bool suppressed) override {
        // *(00F8A0C4)+D0h, the byte 008d0000 stores.
        owner_.core_binding_state().global_messages_suppressed = suppressed;
        done("LuaCore::set_global_message_suppression", "008d0000");
    }
    void message_system_drain_queue() override {
        record("LuaCore::message_system_drain_queue", "008d000a");
    }

private:
    void record(const char* method, const char* address) {
        owner_.host_log().unimplemented(method, address);
    }
    void done(const char* method, const char* address) {
        owner_.host_log().implemented(method, address);
    }

    GameMissionLuaHost& owner_;
    lua_State* state_;
};

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
    // Milestone 2g: the ten rows packet cc_lua_core reconstructed run their own
    // routine instead of the record. A row that is not one of the ten answers
    // -1 and falls through to the policy below, which is still the rule for the
    // other 550.
    const int produced = host->run_core_binding(static_cast<std::size_t>(row), state);
    if (produced >= 0) return produced;
    host->note_native_call(static_cast<std::size_t>(row), argc);
    // The nineteen entity-returning rows of the table end in one recovered tail
    // (docs/LUA_BINDING_ENTITY.md): they push thisTable[key] for the entity they
    // resolved, or, at 0089903C, nil when the lookup produced nothing. This
    // process resolves no entity, so every one of them takes the nil arm, which
    // is a recovered result rather than a substitute: a binding that returns one
    // value is different from one that returns none, and the shipped scripts
    // assign from these.
    const bsp::MissionLuaBinding& binding =
        bsp::mission_lua_bindings()[static_cast<std::size_t>(row)];
    if (bsp::mission_binding_returns_entity(binding.name)) {
        host->note_entity_return();
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

void GameMissionLuaHost::note_error(const std::string& message) {
    if (summary_.first_error.empty() && !message.empty()) {
        summary_.first_error = message;
        summary_.first_error_phase = phase_;
    }
}

void GameMissionLuaHost::set_game_fields(int non_campaign_flag,
    int effective_difficulty) noexcept {
    core_.non_campaign_flag = non_campaign_flag;
    core_.effective_difficulty = effective_difficulty;
}

GameMissionCoreBindings& GameMissionLuaHost::core_binding_state() noexcept { return core_; }

const GameMissionCoreBindings& GameMissionLuaHost::core_bindings() const noexcept {
    return core_;
}

GameHostLog& GameMissionLuaHost::host_log() noexcept { return log_; }

void GameMissionLuaHost::note_core_message_map(const std::string& name) {
    if (std::find(core_message_maps_.begin(), core_message_maps_.end(), name)
        == core_message_maps_.end()) {
        core_message_maps_.push_back(name);
    }
    core_.message_maps = core_message_maps_.size();
}

void GameMissionLuaHost::note_core_think_name(const std::string& name) {
    if (std::find(core_think_names_.begin(), core_think_names_.end(), name)
        == core_think_names_.end()) {
        core_think_names_.push_back(name);
    }
    core_.think_names = core_think_names_.size();
}

int GameMissionLuaHost::run_core_binding(std::size_t row, lua_State* state) {
    if (state == nullptr || row >= bsp::mission_lua_binding_count()) return -1;
    const bsp::MissionLuaBinding& binding = bsp::mission_lua_bindings()[row];
    const int argc = ::lua_gettop(state);
    CoreArgumentReader args(state);
    CoreResultWriter results(state);
    CoreBindingHost host(*this, state);

    int produced = -1;
    switch (binding.address) {
    case 0x0088c620u:  // SETLOG: 25 instructions that touch no game state
        produced = bsp::lua_binding_setlog();
        break;
    case 0x008ae030u:  // GetDifficulty
        produced = bsp::lua_binding_get_difficulty(results, host);
        core_.difficulty_asked = true;
        core_.difficulty_reported = bsp::lua_binding_difficulty_value(
            core_.non_campaign_flag, core_.effective_difficulty);
        break;
    case 0x008c8f70u:  // PrepareClass
        produced = bsp::lua_binding_prepare_class(args, host);
        break;
    case 0x008c4d10u:  // Music_Control_SetLevel
        produced = bsp::lua_binding_music_control_set_level(args, host);
        break;
    case 0x008a8930u:  // SetParty
        ++core_.party_calls;
        if (args.entity_at(0) == nullptr) ++core_.entity_arguments_missing;
        produced = bsp::lua_binding_set_party(args, results, host);
        break;
    case 0x008b87f0u:  // Scoring_RealPlayTimeRunning
        produced = bsp::lua_binding_scoring_real_play_time_running(args, results, host);
        break;
    case 0x008b8640u:  // Scoring_SetFinalScoringFunctionName
        produced = bsp::lua_binding_scoring_set_final_scoring_function_name(args, host);
        break;
    case 0x008c61c0u:  // LoadMessageMap
        produced = bsp::lua_binding_load_message_map(args, host);
        break;
    case 0x00897fb0u:  // SetThink
        if (args.entity_at(0) == nullptr) ++core_.entity_arguments_missing;
        produced = bsp::lua_binding_set_think(args, host);
        break;
    case 0x008cfe40u:  // EnableMessages
        produced = bsp::lua_binding_enable_messages(args, host);
        break;
    default:
        return -1;
    }
    ++core_.calls;
    note_native_call(row, argc, true);
    return produced;
}

void GameMissionLuaHost::report_core_bindings() {
    if (core_.calls == 0) {
        log_.notef("mission lua core bindings: none of the ten reconstructed rows was called");
        return;
    }
    log_.notef("mission lua core bindings: %llu calls through src/lua_binding_core.cpp; "
        "GetDifficulty answered %d (game+1FE4h=%d game+6ACh=%d)%s", core_.calls,
        core_.difficulty_reported, core_.non_campaign_flag, core_.effective_difficulty,
        core_.difficulty_asked ? "" : " (not asked)");
    log_.notef("  PrepareClass resolved %llu of %llu VehicleClass.<id>.Race paths on the live "
        "datatable globals", core_.prepare_class_resolved, core_.prepare_class_lookups);
    log_.notef("  scoring real_play_time_running=%d final_function=\"%s\" message_maps=%zu "
        "think_names=%zu set_party=%zu entity_arguments_missing=%zu global_messages_suppressed=%d",
        core_.real_play_time_running ? 1 : 0, core_.final_scoring_function.c_str(),
        core_.message_maps, core_.think_names, core_.party_calls,
        core_.entity_arguments_missing, core_.global_messages_suppressed ? 1 : 0);
}

void GameMissionLuaHost::note_native_call(std::size_t row, int argument_count) {
    note_native_call(row, argument_count, false);
}

void GameMissionLuaHost::note_native_call(std::size_t row, int argument_count, bool concrete) {
    const bsp::MissionLuaBinding* rows = bsp::mission_lua_bindings();
    if (row >= bsp::mission_lua_binding_count()) return;
    const bsp::MissionLuaBinding& binding = rows[row];
    ++summary_.native_calls;
    auto found = native_index_.find(binding.name);
    if (found == native_index_.end()) {
        native_index_.emplace(binding.name, summary_.natives.size());
        GameMissionNativeCall record;
        record.name.assign(binding.name);
        record.address = binding.address;
        record.calls = 1;
        record.last_argument_count = argument_count;
        summary_.natives.push_back(record);
        // A binding the scripts reached is one native body this process does
        // not have, unless it is one of the ten rows milestone 2g routes
        // through src/lua_binding_core.cpp. The record carries the row's own
        // address, so the report names the routine rather than the table.
        char address[16];
        std::snprintf(address, sizeof(address), "%08lx",
            static_cast<unsigned long>(binding.address));
        char method[96];
        std::snprintf(method, sizeof(method), "MissionLuaNative::%s", binding.name);
        if (concrete) {
            log_.implemented(method, address);
        } else {
            log_.unimplemented(method, address);
        }
        log_.notef("  native %-28s argc=%d phase=%s %s", binding.name, argument_count,
            phase_.empty() ? "(none)" : phase_.c_str(),
            concrete ? "[reconstructed]" : "");
        return;
    }
    GameMissionNativeCall& record = summary_.natives[found->second];
    ++record.calls;
    record.last_argument_count = argument_count;
    char address[16];
    std::snprintf(address, sizeof(address), "%08lx", static_cast<unsigned long>(binding.address));
    char method[96];
    std::snprintf(method, sizeof(method), "MissionLuaNative::%s", binding.name);
    if (concrete) {
        log_.implemented(method, address);
    } else {
        log_.unimplemented(method, address);
    }
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

void GameMissionLuaHost::publish_lobby_settings_005e2f00() {
    if (state_ == nullptr) return;
    // 005e2f00 reads and writes the thirteen slots of 00e08908 through the
    // LuaObject API, taking each value from session state. This process has no
    // session, so the table is created with zeroed fields and the sync itself
    // is a record: without the table the multiplayer scripts index a nil global
    // at their first line, which is why the step exists at all.
    lua_createtable(state_, 0, static_cast<int>(bsp::kLobbySettingsFieldCount));
    std::size_t fields = 0;
    for (std::size_t slot = 0; slot < bsp::kLobbySettingsSlotCount; ++slot) {
        const char* field = bsp::lobby_settings_field_name(slot);
        if (field == nullptr) continue;  // slot 0Dh, the null pointer at 00e08940
        lua_pushinteger(state_, 0);
        lua_setfield(state_, -2, field);
        ++fields;
    }
    lua_setfield(state_, LUA_GLOBALSINDEX, bsp::kLobbySettingsTable);
    summary_.lobby_settings_published = true;
    log_.unimplemented("MissionLua::sync_lobby_settings", "005e2f00");
    log_.notef("LobbySettings created with %zu fields, all zero: the values are the "
        "session owner's (005e2f00 syncs them, this process does not)", fields);
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
        lua_getfield(state_, LUA_GLOBALSINDEX, name.c_str());
        if (!lua_isnil(state_, -1)) {
            if (::lua_pcall(state_, 0, 0, 0) != 0) {
                const char* message = lua_tolstring(state_, -1, nullptr);
                run.error = message != nullptr ? message : "(no message)";
            }
        }
        ::lua_settop(state_, top);
        note_error(run.error);
    }
    log_.notef("  entry point %-20s defined=%d dispatched=%d status=%d %s", name.c_str(),
        run.defined ? 1 : 0, run.dispatched ? 1 : 0, run.pcall_status,
        run.error.empty() ? "" : run.error.c_str());
    summary_.entry_points.push_back(run);
    return run.dispatched && run.pcall_status == 0;
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
