// cc15 lane L2: the profile block game+650h with its +64h MissionProgress, and
// the hint tracker singleton [00E198D0]. Evidence and uncertainty:
// docs/CC15_OBJECT_PROFILE_HINTS.md. Both objects reuse the existing typed
// bodies (reset_profile_007fdb20, MissionProgressOwner,
// hint_tracker_rebuild_history_00690cd0); nothing here re-derives them.

#include "bsp/game_profile_owner.hpp"

#include "bsp/game_hint_passes.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/lua_script_runtime.hpp"
#include "bsp/lua_state_owner.hpp"
#include "bsp/vfs_lua_scripts.hpp"

#include <exception>
#include <memory>
#include <optional>
#include <string_view>

extern "C" {
#include <lua.h>
}

namespace bsp::game {

// ---------------------------------------------------------------------------
// game+650h
// ---------------------------------------------------------------------------

GameProfileOwner::GameProfileOwner(GameHostLog& log) : log_(log) {}

void GameProfileOwner::reset_to_defaults_007fdb20() {
    // The image hands 007FDB20 the live settings object; the host keeps the
    // settings phase 5 loaded and lets only the profile half take effect.
    discarded_settings_ = GameSettingsBlock{};
    reset_profile_007fdb20(profile_, discarded_settings_, *this);
    ++resets_;
}

MissionProgress* GameProfileOwner::mission_progress() noexcept {
    return profile_.mission_progress_present ? &progress_.get() : nullptr;
}

void GameProfileOwner::destroy_mission_progress_007fd780() {
    progress_.destroy_mission_progress_007fd780();
}

bool GameProfileOwner::construct_mission_progress_00920e10() {
    // 007FDC11 PUSH 24h, operator new, 00920E10, stored at 007FDC41.
    return progress_.construct_mission_progress_00920e10();
}

bool GameProfileOwner::xenon_state_is_two() {
    // XenonSystemManager [00F8ABE8] is not built in single player; the answer
    // only gates the settings half, which this owner discards.
    log_.unimplemented("ProfileReset::xenon_state_is_two", "00f8abe8");
    return false;
}

bool GameProfileOwner::xenon_user_selected() {
    log_.unimplemented("ProfileReset::xenon_user_selected", "00f8abe8");
    return false;
}

void GameProfileOwner::restore_selected_user_controls_008d45d0() {
    log_.unimplemented("ProfileReset::restore_selected_user_controls", "008d45d0");
}

// ---------------------------------------------------------------------------
// [00E198D0]
// ---------------------------------------------------------------------------

GameHintTracker::GameHintTracker(GameHostLog& log, GameProfileOwner& profile)
    : log_(log), profile_(profile) {}

GameHintTracker::~GameHintTracker() = default;

void GameHintTracker::clear_hint_queue_0068ea00() {
    mission_seen_48_.clear();   // 004D05E0 on +48h
    queue_68_.clear();          // 0068E780 until +78h == 0
}

void GameHintTracker::rebuild_history_00690cd0() {
    hint_tracker_rebuild_history_00690cd0(*this);
}

// ---------------------------------------------------------------------------
// 006921B0 over a private Lua state
// ---------------------------------------------------------------------------

namespace {

// One sub-table entry, the value lua_next left on top of the stack. The native
// accessors (00B67800 get_by_name, 00B65FB0 is_nil, 00B662B0 string, 00B66250
// boolean, 00B66290 integer) become the stock 5.1 calls; a non-table entry
// answers nil for every key instead of raising.
class LuaHintEntry final : public HintLuaEntry {
public:
    LuaHintEntry(lua_State* state, int index) : state_(state), index_(index) { load_rows(); }

    std::string id() override { return string("id"); }
    bool is_nil(std::string_view key) override {
        push(index_, key);
        const bool nil = lua_isnil(state_, -1);
        lua_pop(state_, 1);
        return nil;
    }
    std::string string(std::string_view key) override {
        push(index_, key);
        std::string value = top_string();
        lua_pop(state_, 1);
        return value;
    }
    bool boolean(std::string_view key) override {
        push(index_, key);
        const bool value = lua_toboolean(state_, -1) != 0;
        lua_pop(state_, 1);
        return value;
    }
    std::int32_t integer(std::string_view key) override {
        push(index_, key);
        const auto value = static_cast<std::int32_t>(lua_tointeger(state_, -1));
        lua_pop(state_, 1);
        return value;
    }
    std::size_t row_count() override { return rows_.size(); }
    bool row_is_nil(std::size_t row, std::string_view key) override {
        return !row_field(row, key).has_value();
    }
    std::string row_string(std::size_t row, std::string_view key) override {
        return row_field(row, key).value_or(std::string());
    }

private:
    struct Row {
        std::optional<std::string> title, description, lua_command;
    };

    void push(int table, std::string_view key) {
        if (lua_type(state_, table) != LUA_TTABLE) {
            lua_pushnil(state_);
            return;
        }
        const std::string name(key);
        lua_getfield(state_, table, name.c_str());
    }
    std::string top_string() {
        std::size_t size = 0;
        const char* text = lua_tolstring(state_, -1, &size);
        return text != nullptr ? std::string(text, size) : std::string();
    }
    std::optional<std::string> field_at(int table, const char* key) {
        push(table, key);
        std::optional<std::string> value;
        if (!lua_isnil(state_, -1)) value = top_string();
        lua_pop(state_, 1);
        return value;
    }
    // TitleDescList in the iteration order 0069090F walks (00B67080 / 00B67190).
    void load_rows() {
        push(index_, "TitleDescList");
        const int list = lua_gettop(state_);
        if (lua_type(state_, list) == LUA_TTABLE) {
            lua_pushnil(state_);
            while (lua_next(state_, list) != 0) {
                const int row = lua_gettop(state_);
                rows_.push_back(Row{field_at(row, "Title"), field_at(row, "Description"),
                    field_at(row, "LuaCommand")});
                lua_pop(state_, 1);
            }
        }
        lua_pop(state_, 1);
    }
    std::optional<std::string> row_field(std::size_t row, std::string_view key) const {
        if (row >= rows_.size()) return std::nullopt;
        const Row& r = rows_[row];
        if (key == "Title") return r.title;
        if (key == "Description") return r.description;
        if (key == "LuaCommand") return r.lua_command;
        return std::nullopt;
    }

    lua_State* state_;
    int index_;
    std::vector<Row> rows_;
};

}  // namespace

// HintReloadHost and HintTableHost over the tracker and the private state. The
// selected sub-table sits at stack slot table_; next_entry pops the previous
// value before each lua_next, so key and table stay where lua_next wants them.
class HintTablesReload final : public HintReloadHost, public HintTableHost {
public:
    HintTablesReload(GameHintTracker& tracker, lua_State* state, LuaScriptRuntime& scripts)
        : tracker_(tracker), state_(state), scripts_(scripts) {}

    void history7c_clear() override { tracker_.history_7c_.clear(); }
    bool run_script_with_overrides(std::string_view path) override {
        // 00692261 -> 00B69D40(path, 0): the file, then each VFS override of it.
        for (const GuiLuaChunkOutcome& outcome : scripts_.run_file(state_, std::string(path), false)) {
            if (outcome.called) tracker_.reload_summary_.script_called = true;
        }
        return tracker_.reload_summary_.script_called;
    }
    void definitions_clear_004d7370() override { tracker_.definitions_30_.clear(); }
    void select_table(std::string_view group, std::string_view name) override {
        end_iteration();
        lua_settop(state_, base_);
        lua_getfield(state_, LUA_GLOBALSINDEX, std::string(kHintRootTable).c_str());
        sub_table(group);
        sub_table(name);
        table_ = lua_gettop(state_);
    }
    void load_selected_0068f030(bool other_category) override {
        lua_pushnil(state_);   // the first key
        iterating_ = lua_type(state_, table_) == LUA_TTABLE;
        if (!iterating_) lua_pop(state_, 1);
        hint_tracker_load_definitions_0068f030(*this, other_category);
        end_iteration();
    }
    void rebuild_history_00690cd0() override { tracker_.rebuild_history_00690cd0(); }
    void close_script() override { lua_settop(state_, base_); }

    HintLuaEntry* next_entry() override {
        if (!iterating_) return nullptr;
        if (entry_) {
            entry_.reset();
            lua_pop(state_, 1);   // the previous value; its key stays for lua_next
        }
        if (lua_next(state_, table_) == 0) {
            iterating_ = false;
            return nullptr;
        }
        ++tracker_.reload_summary_.entries;
        entry_ = std::make_unique<LuaHintEntry>(state_, lua_gettop(state_));
        return entry_.get();
    }
    void store_definition_005fdd90(const std::string& id, const HintDefinition& def) override {
        // 005FFB20 then 005FDD90: map[id] = def, a later id replacing an earlier one.
        auto [it, inserted] = tracker_.definitions_30_.insert_or_assign(id, def);
        static_cast<void>(it);
        if (!inserted) ++tracker_.reload_summary_.replaced;
    }

    int base_{0};

private:
    void sub_table(std::string_view key) {
        if (lua_type(state_, -1) != LUA_TTABLE) {
            lua_pop(state_, 1);
            lua_pushnil(state_);
            return;
        }
        lua_getfield(state_, -1, std::string(key).c_str());
        lua_remove(state_, -2);
    }
    void end_iteration() {
        entry_.reset();
        if (iterating_) {
            lua_settop(state_, table_);
            iterating_ = false;
        }
    }

    GameHintTracker& tracker_;
    lua_State* state_;
    LuaScriptRuntime& scripts_;
    int table_{0};
    bool iterating_{false};
    std::unique_ptr<LuaHintEntry> entry_;
};

void GameHintTracker::reload_hint_tables_006921b0(GameVfsHost& vfs) {
    reload_summary_ = ReloadSummary{};
    // The host registers no content suffixes (as GameGlobalSubsystemsHost);
    // 0108FF20 / 0108FF24 stay x360comp 0 and an empty region.
    const std::vector<std::string> suffixes{};
    VfsLuaScriptFiles files(vfs.context(), suffixes);
    LuaScriptRuntime runtime(files);
    const LuaRuntimeGlobals globals{};
    try {
        // 006921FF 00B66BD0, 00692214 00B6A020(1): base library only.
        PcStorageLuaOwner lua(files.owner_environment(runtime, globals));
        lua.open_storage_archive_00b6a020(1);
        lua_State* const state = lua.storage_lua_38();
        HintTablesReload host(*this, state, runtime);
        host.base_ = lua_gettop(state);
        HintReloadState fields{state_.force_next_04, last_unit_class_18_, state_.last_timed_1c,
            state_.last_timed_seconds_24, state_.active_60};
        hint_tracker_reload_006921b0(fields, host);
        state_.force_next_04 = fields.force_next_04;
        last_unit_class_18_ = fields.last_unit_class_18;
        state_.last_timed_1c = fields.last_timed_1c;
        state_.last_timed_seconds_24 = fields.last_timed_seconds_24;
        state_.active_60 = fields.active_60;
        lua.close_storage_archive_00b65e80();   // 00692578 00B65E80
    } catch (const std::exception& failure) {
        log_.notef("hint tracker: 006921b0 failed: %s", failure.what());
        log_.unimplemented("HintTracker::reload_hint_tables", "006921b0");
        return;
    }
    for (const auto& [id, def] : definitions_30_) {
        static_cast<void>(id);
        if (def.loader_flag_50) ++reload_summary_.other_category;
        if (def.hidden) ++reload_summary_.hidden;
        if (def.critical) ++reload_summary_.critical;
        if (def.mission_specific) ++reload_summary_.mission_specific;
    }
    log_.implemented("HintTracker::reload_hint_tables", "006921b0");
    log_.notef("hint tracker: 006921b0 script=%d entries=%zu definitions=%zu replaced=%zu "
               "other=%zu hidden=%zu critical=%zu missionspecific=%zu history94=%zu",
        reload_summary_.script_called ? 1 : 0, reload_summary_.entries, definitions_30_.size(),
        reload_summary_.replaced, reload_summary_.other_category, reload_summary_.hidden,
        reload_summary_.critical, reload_summary_.mission_specific, history_94_.size());
}

const HintDefinition* GameHintTracker::find_definition_0068e450(const std::string& name) {
    const auto found = definitions_30_.find(name);
    return found != definitions_30_.end() ? &found->second : nullptr;
}

// The remaining HintStoreHost hooks belong to routines this lane does not wire
// (00690B20, 00690C30, 0068EE00); reaching one is recorded. Under
// kHintTriggerPassesCc15Bound the three TriggerHint and the passes read are
// the image's (src/game_hint_passes.cpp).
bool GameHintTracker::has_been_shown_0068e600(const std::string& name) {
    if constexpr (kHintTriggerPassesCc15Bound) {
        return has_been_shown_cc15_0068e600(name);
    } else {
        log_.unimplemented("HintTracker::has_been_shown", "0068e600");
        return false;
    }
}

bool GameHintTracker::profile_has_seen_007f8e00(const std::string& name) {
    if constexpr (kHintTriggerPassesCc15Bound) {
        return profile_has_seen_cc15_007f8e00(name);
    } else {
        log_.unimplemented("HintTracker::profile_has_seen", "007f8e00");
        return false;
    }
}

void GameHintTracker::profile_mark_seen_007fae20(const std::string& name) {
    if constexpr (kHintTriggerPassesCc15Bound) {
        profile_mark_seen_cc15_007fae20(name);
    } else {
        log_.unimplemented("HintTracker::profile_mark_seen", "007fae20");
    }
}

void GameHintTracker::profile_unmark_007fae60(const std::string&) {
    log_.unimplemented("HintTracker::profile_unmark", "007fae60");
}

void GameHintTracker::mission_seen_push_back_48(const std::string& name) {
    mission_seen_48_.push_back(name);   // 004D2660 on +48h
}

bool GameHintTracker::player_unit_present_e188d8() {
    log_.unimplemented("HintTracker::player_unit_present", "00e188d8");
    return false;
}

std::int32_t GameHintTracker::player_unit_class_170() {
    log_.unimplemented("HintTracker::player_unit_class", "00e188d8+170h");
    return 0;
}

bool GameHintTracker::history94_has_key(const std::string& key) {
    return history_94_.find(key) != history_94_.end();
}

void GameHintTracker::history94_push_back(const std::string& key, const std::string& name) {
    history_94_[key].push_back(name);
}

void GameHintTracker::history94_assign_single(const std::string& key, const std::string& name) {
    history_94_[key] = std::vector<std::string>{name};
}

std::vector<std::string> GameHintTracker::profile_shown_names_65c() {
    // *(game+65Ch) is the head of the profile's list at +08h (+0Ch head).
    return profile_.profile().string_list_08;
}

namespace {
GameHintTrackerSlot* g_hint_tracker_slot = nullptr;
}  // namespace

GameHintTrackerSlot::GameHintTrackerSlot(GameHostLog& log, GameProfileOwner& profile)
    : log_(log), profile_(profile) {
    g_hint_tracker_slot = this;
}

GameHintTrackerSlot::~GameHintTrackerSlot() {
    if (g_hint_tracker_slot == this) g_hint_tracker_slot = nullptr;
}

GameHintTrackerSlot* GameHintTrackerSlot::process_slot() noexcept { return g_hint_tracker_slot; }

GameHintTracker& GameHintTrackerSlot::get_004e1ca0() {
    if (!tracker_) {
        tracker_ = std::make_unique<GameHintTracker>(log_, profile_);
        log_.implemented("HintTracker::construct", "004e18e0");
    }
    return *tracker_;
}

} // namespace bsp::game
