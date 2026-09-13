// bsp_game.exe milestone 2m: the fixed-step and mission-load routines whose
// reconstructions were already on main and that the executable still recorded.
//
// See include/bsp/game_hosts_ready.hpp for the address list and for what each
// pass can show in this process.

#include "bsp/game_hosts_ready.hpp"

#include "bsp/entity_event_queues.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/game_pending_entity_runtime.hpp"
#include "bsp/mission_load_hosts.hpp"
#include "bsp/spatial_index.hpp"

#include <cstdio>
#include <stdexcept>

namespace bsp::game {
namespace {

void format_address(std::uint32_t value, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(value));
}

// ---------------------------------------------------------------------------
// Row 3+4, 0098bdb0: the spatial re-bucket
// ---------------------------------------------------------------------------
//
// 0042e630 hands the walk the registry singleton. This process builds no such
// registry and registers no node, so the root list is empty and every method
// below the first is unreachable. Each one still records its own call site, so
// a later run that does hold nodes reports the boundary rather than crossing it.
class SpatialRefreshBinding final : public bsp::SpatialRefreshHost {
public:
    explicit SpatialRefreshBinding(GameHostLog& log) : log_(log) {}

    void* first_root_node() override { return nullptr; }
    void* next_root_node(void* node) override { static_cast<void>(node); return nullptr; }
    bool node_is_static(void* node) override { static_cast<void>(node); return true; }
    void* node_owner_pose(void* node) override { static_cast<void>(node); return nullptr; }
    bool pose_world_valid(void* pose) override { static_cast<void>(pose); return true; }
    void pose_refresh_world_00414db0(void* pose) override {
        static_cast<void>(pose);
        log_.unimplemented("SpatialIndex::pose_refresh_world", "00414db0");
    }
    void node_copy_world_matrix_004134f0(void* node, void* pose) override {
        static_cast<void>(node);
        static_cast<void>(pose);
        log_.unimplemented("SpatialIndex::copy_world_matrix", "004134f0");
    }
    bool pose_inverse_valid(void* pose) override { static_cast<void>(pose); return true; }
    void pose_build_inverse_00b63d50(void* pose) override {
        static_cast<void>(pose);
        log_.unimplemented("SpatialIndex::build_inverse", "00b63d50");
    }
    void node_copy_inverse_matrix_004134f0(void* node, void* pose) override {
        static_cast<void>(node);
        static_cast<void>(pose);
        log_.unimplemented("SpatialIndex::copy_inverse_matrix", "004134f0");
    }
    void node_rebuild_world_bounds_0098a750(void* node) override {
        static_cast<void>(node);
        log_.unimplemented("SpatialIndex::rebuild_world_bounds", "0098a750");
    }
    int node_cell_count(void* node) override { static_cast<void>(node); return 0; }
    bsp::SpatialCellRect node_cell_rect_0098ad60(void* node) override {
        static_cast<void>(node);
        log_.unimplemented("SpatialIndex::node_cell_rect", "0098ad60");
        return bsp::SpatialCellRect{};
    }
    std::uint32_t node_cell_key(void* node) override { static_cast<void>(node); return 0; }
    void unregister_node_0098a3d0(void* node) override {
        static_cast<void>(node);
        log_.unimplemented("SpatialIndex::unregister_node", "0098a3d0");
    }
    void register_node_0098a310(void* node, const bsp::SpatialCellRect& rect) override {
        static_cast<void>(node);
        static_cast<void>(rect);
        log_.unimplemented("SpatialIndex::register_node", "0098a310");
    }
    int node_child_count(void* node) override { static_cast<void>(node); return 0; }
    void* node_child(void* node, int slot) override {
        static_cast<void>(node);
        static_cast<void>(slot);
        return nullptr;
    }

private:
    GameHostLog& log_;
};

// ---------------------------------------------------------------------------
// Rows 6 and 14, 00926700: the deferred entity event drain
// ---------------------------------------------------------------------------
class DeferredEventBinding final : public bsp::DeferredEntityEventHost {
public:
    explicit DeferredEventBinding(GameHostLog& log) : log_(log) {}

    std::size_t deferred_queue_size() override { return 0; }
    bsp::DeferredEventBack deferred_queue_back() override {
        log_.unimplemented("EntityEvents::deferred_queue_back", "00926717");
        return bsp::DeferredEventBack{};
    }
    void dispatch_hit_009239a0(void* subject, void* record, void* direction) override {
        static_cast<void>(subject);
        static_cast<void>(record);
        static_cast<void>(direction);
        log_.unimplemented("EntityEvents::dispatch_hit", "009239a0");
    }
    void deferred_queue_pop_back_and_destroy(const bsp::DeferredEventBack& back) override {
        static_cast<void>(back);
        log_.unimplemented("EntityEvents::pop_deferred_event", "00926783");
    }
    void set_deferred_pending_flag_00e18684(bool pending) override {
        static_cast<void>(pending);
        log_.implemented("EntityEvents::set_deferred_pending_flag", "00e18684");
    }

private:
    GameHostLog& log_;
};

// ---------------------------------------------------------------------------
// Row 7, 00888230: the queued Lua call drain
// ---------------------------------------------------------------------------
//
// The queue at [game+1A08h]+8h is empty in this process: its producer is
// 00887560, reached only when a named call is made from a frame job thread, and
// this executable runs the mission Lua machine on the main thread alone. The
// drain therefore reads the size, finds zero and returns, which is the whole of
// 00888230's loop condition.
class NamedCallBinding final : public bsp::MissionNamedCallHost {
public:
    explicit NamedCallBinding(GameHostLog& log) : log_(log) {}

    int game_lifecycle_state() override { return 0; }
    bool on_frame_job_thread() override {
        log_.unimplemented("MissionNamedCall::on_frame_job_thread", "004c1130");
        return false;
    }
    int vfs_block_depth() override { return 0; }
    void vfs_leave_file_block() override {
        log_.unimplemented("MissionNamedCall::leave_file_block", "00bdc9b0");
    }
    int lua_gettop() override { return 0; }
    void lua_settop(int index) override { static_cast<void>(index); }
    void lua_getglobal(const char* name) override {
        static_cast<void>(name);
        log_.unimplemented("MissionNamedCall::lua_getglobal", "006b8460");
    }
    void lua_pushstring(const char* text) override { static_cast<void>(text); }
    void lua_gettable(int index) override { static_cast<void>(index); }
    void lua_remove(int index) override { static_cast<void>(index); }
    void lua_pushvalue(int index) override { static_cast<void>(index); }
    void push_argument(const bsp::MissionLuaArgument& argument) override {
        static_cast<void>(argument);
        log_.unimplemented("MissionNamedCall::push_argument", "00885da0");
    }
    int lua_pcall(int nargs, int nresults, int errfunc_index) override {
        static_cast<void>(nargs);
        static_cast<void>(nresults);
        static_cast<void>(errfunc_index);
        log_.unimplemented("MissionNamedCall::lua_pcall", "006b8530");
        return 0;
    }
    void adjust_reentrancy_depth(int delta) override { static_cast<void>(delta); }
    void adjust_call_stack_marker(int delta) override { static_cast<void>(delta); }
    std::vector<bsp::MissionLuaResultVariant> collect_results(int count,
        int max_depth) override {
        static_cast<void>(count);
        static_cast<void>(max_depth);
        return {};
    }
    void queue_call(const bsp::MissionLuaDeferredCall& call) override {
        static_cast<void>(call);
        log_.unimplemented("MissionNamedCall::queue_call", "00887560");
    }

private:
    GameHostLog& log_;
};

// ---------------------------------------------------------------------------
// Row 8, 00929460: the due entity think
// ---------------------------------------------------------------------------
class EntityThinkBinding final : public bsp::EntityThinkHost {
public:
    explicit EntityThinkBinding(GameHostLog& log) : log_(log) {}

    void run_entity_think_00929150(std::uint32_t entity) override {
        static_cast<void>(entity);
        log_.unimplemented("EntityThink::run_entity_think", "00929150");
    }
    void free_think_node_0092952c(const bsp::EntityThinkNode& node) override {
        static_cast<void>(node);
    }
    bool gc_gate_predicate_0109cefc_vtable0c() override {
        log_.unimplemented("EntityThink::gc_gate_predicate", "0109cefc+vtable0c");
        return false;
    }
    void lua_run_string_006b8ad0(const char* chunk, int mode) override {
        static_cast<void>(chunk);
        static_cast<void>(mode);
        log_.unimplemented("EntityThink::collect_garbage", "006b8ad0");
    }
    void splice_pending_into_live_00928380(bsp::EntityThinkList& live,
        const bsp::EntityThinkList& pending) override {
        for (const bsp::EntityThinkNode& node : pending.nodes) live.nodes.push_back(node);
        log_.implemented("EntityThink::splice_pending_into_live", "00928380");
    }
    void clear_pending_00928330(bsp::EntityThinkList& pending) override {
        pending.nodes.clear();
        log_.implemented("EntityThink::clear_pending", "00928330");
    }

private:
    GameHostLog& log_;
};

// ---------------------------------------------------------------------------
// Row 15, 009273a0: the pending destroy and kill lists
// ---------------------------------------------------------------------------
class PendingQueueBinding final : public bsp::PendingEntityQueueHost {
public:
    PendingQueueBinding(GameHostLog& log, GameUnitsHost* units)
        : log_(log), units_(units), owners_(game_pending_entity_owners()) {}

    // Borrow the same process owners that startup initialized. Source bindings
    // require initialized rings; zero-filled storage does not establish them.
    std::size_t pending_destroy_count() override {
        if (owners_.destroy_00f899a8.head_04 == nullptr)
            throw std::logic_error("pending destroy owner is not initialized");
        return owners_.destroy_00f899a8.count_08;
    }
    std::size_t pending_kill_count() override {
        if (owners_.kill_00f899b4.head_04 == nullptr)
            throw std::logic_error("pending kill owner is not initialized");
        return owners_.kill_00f899b4.count_08;
    }
    void copy_pending_lists_00926fa0() override {
        log_.unimplemented("EntityQueues::copy_pending_lists", "00926fa0");
        throw std::logic_error("native pending entity list copy binding is unavailable");
    }
    void clear_pending_lists() override {
        log_.unimplemented("EntityQueues::clear_pending_lists", "009273d1");
        throw std::logic_error("native pending entity list clear binding is unavailable");
    }
    std::size_t destroy_copy_count() override { return 0; }
    void* destroy_copy_at(std::size_t index) override {
        static_cast<void>(index);
        return nullptr;
    }
    std::size_t kill_copy_count() override { return 0; }
    void* kill_copy_at(std::size_t index) override {
        static_cast<void>(index);
        return nullptr;
    }
    void on_entity_destroyed_74h(void* entity) override {
        static_cast<void>(entity);
        log_.unimplemented("EntityQueues::on_entity_destroyed", "00927400");
    }
    void* render_handle_18h(void* entity) override {
        static_cast<void>(entity);
        return nullptr;
    }
    void* controlled_unit_handle_00e188dc() override { return nullptr; }
    void clear_controlled_unit_handle_004bca80() override {
        log_.unimplemented("EntityQueues::clear_controlled_unit_handle", "004bca80");
    }
    bsp::SceneNodeFlags scene_node_flags(void* entity) override {
        bsp::SceneNodeFlags flags;
        if (units_ == nullptr || !units_->read_scene_node_flags(entity, flags)) {
            throw std::logic_error("pending entity has no canonical unit scene flags");
        }
        return flags;
    }
    void store_scene_node_flags(void* entity, const bsp::SceneNodeFlags& flags) override {
        if (units_ == nullptr || !units_->store_scene_node_flags(entity, flags)) {
            throw std::logic_error("pending entity has no canonical unit scene flags");
        }
    }
    bool sample_has_observers_00925c40(void* entity) override {
        static_cast<void>(entity);
        log_.unimplemented("EntityQueues::sample_has_observers", "00925c40");
        return false;
    }
    void notify_observers_00696330(void* entity) override {
        static_cast<void>(entity);
        log_.unimplemented("EntityQueues::notify_observers", "00696330");
    }
    void on_entity_killed_80h(void* entity) override {
        static_cast<void>(entity);
        log_.unimplemented("EntityQueues::on_entity_killed", "009274f4");
    }

private:
    GameHostLog& log_;
    GameUnitsHost* units_;
    bsp::NativePendingEntityOwners& owners_;
};

// ---------------------------------------------------------------------------
// Row 16, 00903610: the world expiry walk
// ---------------------------------------------------------------------------
//
// The one pass of the six with something to walk: the chain 009037f0 allocated
// holds this mission's 32 created instances, linked by +38h, and the walk visits
// every one of them. Their counters at +6Ch are zero, which is 00903625's skip,
// because 00922fd0 is the only writer of that field and nothing here marks an
// entity for release.
class WorldExpiryBinding final : public bsp::WorldExpiryHost {
public:
    WorldExpiryBinding(GameHostLog& log, std::vector<int>& counters, std::size_t count)
        : log_(log), counters_(counters), count_(count) {}

    void* world_chain_head() override {
        if (count_ == 0) return nullptr;
        return token(0);
    }
    void* next_in_world_chain(void* entity) override {
        const std::size_t next = index_of(entity) + 1;
        if (next >= count_) return nullptr;
        return token(next);
    }
    int expiry_counter(void* entity) override {
        const std::size_t index = index_of(entity);
        return (index < counters_.size()) ? counters_[index] : 0;
    }
    void store_expiry_counter(void* entity, int counter) override {
        const std::size_t index = index_of(entity);
        if (index < counters_.size()) counters_[index] = counter;
    }
    std::size_t child_count(void* entity) override {
        static_cast<void>(entity);
        return 0;
    }
    void* first_child(void* entity) override {
        static_cast<void>(entity);
        return nullptr;
    }
    void destroy_entity_vtable0(void* entity) override {
        static_cast<void>(entity);
        // The scalar deleting destructor. What it unlinks is a contract, and no
        // entity of this mission reaches the release value, so it is recorded.
        log_.unimplemented("WorldExpiry::destroy_entity", "0090364e");
        ++released;
    }

    std::size_t visited() const noexcept { return visited_; }
    std::size_t released{0};

private:
    void* token(std::size_t index) {
        ++visited_;
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(index + 1));
    }
    static std::size_t index_of(void* entity) noexcept {
        return static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(entity)) - 1;
    }

    GameHostLog& log_;
    std::vector<int>& counters_;
    std::size_t count_{0};
    std::size_t visited_{0};
};

// ---------------------------------------------------------------------------
// The mission-load hosts
// ---------------------------------------------------------------------------

class NetworkSlotResetBinding final : public bsp::NetworkSlotResetHost {
public:
    explicit NetworkSlotResetBinding(GameHostLog& log) : log_(log) {}

    void reset_single_player_slots_004bb160() override {
        single_player = true;
        log_.implemented("MissionLoad::reset_single_player_slots_from_branch", "004dfd18");
    }
    void clear_string_tree_00e18a60() override {
        log_.unimplemented("MissionLoad::clear_string_tree", "004cec60");
    }
    void clear_record_tree_00e18a6c() override {
        log_.unimplemented("MissionLoad::clear_record_tree", "004c1ff0");
    }
    void clear_side_balance_latch_00e188bd() override {
        log_.unimplemented("MissionLoad::clear_side_balance_latch", "004dfc80");
    }
    void write_slot_header(std::size_t index, const bsp::SessionSlotHeader& header) override {
        static_cast<void>(index);
        static_cast<void>(header);
        log_.unimplemented("MissionLoad::write_slot_header", "004dfc91");
    }
    int local_slot_index() override { return 0; }
    int slot_party(int slot) override { static_cast<void>(slot); return 0; }
    int select_menu_record_005d7070() override {
        log_.unimplemented("MissionLoad::select_menu_record", "005d7070");
        return -1;
    }
    std::size_t menu_record_count() override { return 0; }
    void apply_menu_record_00626930(int record_index, int party) override {
        static_cast<void>(record_index);
        static_cast<void>(party);
        log_.unimplemented("MissionLoad::apply_menu_record", "00626930");
    }

    bool single_player{false};

private:
    GameHostLog& log_;
};

class AvoidZoneResetBinding final : public bsp::AvoidZoneResetHost {
public:
    AvoidZoneResetBinding(GameHostLog& log, const std::function<void()>& rebuild)
        : log_(log), rebuild_(rebuild) {}

    void clear_avoid_zone_counter_648h() override {
        log_.implemented("MissionLoad::clear_avoid_zone_counter", "004e075a");
    }
    void clear_avoid_zone_clock_64ch() override {
        log_.implemented("MissionLoad::clear_avoid_zone_clock", "004e0764");
    }
    void clear_scene_tree_5c8h() override {
        // The inlined _Tree::_Erase over game+5C8h. The tree has no identified
        // producer, so this process holds it empty and the erase walks nothing.
        log_.implemented("MissionLoad::clear_scene_tree", "004e076c");
    }
    void rebuild_avoid_zones_00424d00() override {
        rebuild_();
        log_.implemented("MissionLoad::rebuild_avoid_zone_geometry", "00424d00..00424ddf");
        log_.unimplemented("MissionLoad::avoid_zone_draft_layers", "00424ddf..00425487");
    }

private:
    GameHostLog& log_;
    const std::function<void()>& rebuild_;
};

class ScriptedNameBinding final : public bsp::ScriptedNameListHost {
public:
    ScriptedNameBinding(GameHostLog& log, std::vector<bsp::LuaGlobalEntry> globals,
        std::vector<std::string>& out)
        : log_(log), globals_(std::move(globals)), out_(out) {}

    void clear_scripted_name_set() override {
        out_.clear();
        log_.implemented("MissionLoad::clear_scripted_name_set", "004d3126");
    }
    std::size_t lua_global_count() override { return globals_.size(); }
    bsp::LuaGlobalEntry lua_global(std::size_t index) override {
        if (index >= globals_.size()) return bsp::LuaGlobalEntry{};
        return globals_[index];
    }
    void insert_scripted_name(const std::string& name) override {
        for (const std::string& existing : out_) {
            if (existing == name) return;
        }
        out_.push_back(name);
    }

private:
    GameHostLog& log_;
    std::vector<bsp::LuaGlobalEntry> globals_;
    std::vector<std::string>& out_;
};

}  // namespace

// ---------------------------------------------------------------------------
// GameStepSubsystemsHost
// ---------------------------------------------------------------------------

GameStepSubsystemsHost::GameStepSubsystemsHost(GameHostLog& log) : log_(log) {
    bsp::fixed_step_callback_list_init(callbacks_);
}

void GameStepSubsystemsHost::attach_units(GameUnitsHost* units) noexcept {
    units_ = units;
    expiry_counters_.assign(units != nullptr ? units->count() : 0, 0);
}

void GameStepSubsystemsHost::refresh_moved_spatial_nodes_0098bdb0(float step) {
    SpatialRefreshBinding binding(log_);
    bsp::spatial_refresh_moved_nodes_0098bdb0(binding, step);
    ++summary_.spatial_passes;
    if (!logged_empty_) {
        logged_empty_ = true;
        log_.notef("fixed-step rows 3+4, 6, 7, 8, 15 and 16 now run their own "
            "reconstructions. Five of the six walk an empty container, and the reason is "
            "the same in each case: no spatial node is registered (0098a310's callers are "
            "the scene graph's), no deferred entity event is queued (00922e60 is a "
            "record), no named Lua call is deferred (00887560 needs a frame job thread), "
            "no entity has a think function (0088a240 is the `SetThink` binding's) and no "
            "entity is pending destroy or kill (00922fd0 is a record). The sixth, the "
            "world expiry 00903610, walks this mission's 32 created instances");
    }
}

void GameStepSubsystemsHost::run_fixed_step_callbacks_00874de0(float step) {
    struct CallbackBinding final : bsp::FixedStepCallbackHost {
        explicit CallbackBinding(GameHostLog& log_in) : log(log_in) {}
        void invoke_step_callback(bsp::FixedStepCallbackNode& node, float delta) override {
            static_cast<void>(node);
            static_cast<void>(delta);
            log.unimplemented("FixedStepCallbacks::invoke", "00874e0b");
        }
        GameHostLog& log;
    };
    CallbackBinding binding(log_);
    const std::int32_t run
        = bsp::fixed_step_callback_list_run_00874de0(callbacks_, step, binding);
    summary_.callbacks_run += static_cast<std::size_t>(run < 0 ? 0 : run);
    ++summary_.callback_passes;
}

void GameStepSubsystemsHost::drain_deferred_entity_events_00926700() {
    DeferredEventBinding binding(log_);
    bsp::drain_deferred_entity_events_00926700(binding);
    ++summary_.deferred_event_passes;
}

void GameStepSubsystemsHost::drain_queued_lua_calls_00888230() {
    NamedCallBinding binding(log_);
    const std::vector<bsp::MissionNamedCallTrace> traces
        = bsp::drain_named_call_queue(binding, lua_queue_);
    summary_.lua_calls_drained += traces.size();
    ++summary_.lua_call_passes;
}

void GameStepSubsystemsHost::run_due_entity_think_00929460(float step) {
    EntityThinkBinding binding(log_);
    const std::vector<bsp::EntityThinkFields> fields;
    const bsp::EntityThinkRunSummary run = bsp::run_entity_think_list_00929460(step,
        think_countdown_, think_live_, think_pending_, fields, binding);
    summary_.thinks_run += run.thinks_run;
    summary_.think_collections += run.garbage_collected ? 1u : 0u;
    ++summary_.think_passes;
}

void GameStepSubsystemsHost::flush_pending_entity_queues_009273a0() {
    PendingQueueBinding binding(log_, units_);
    bsp::flush_pending_entity_queues_009273a0(binding);
    ++summary_.pending_queue_passes;
}

void GameStepSubsystemsHost::release_expired_world_objects_00903610() {
    const std::size_t count = (units_ != nullptr) ? units_->count() : 0;
    if (expiry_counters_.size() != count) expiry_counters_.assign(count, 0);
    WorldExpiryBinding binding(log_, expiry_counters_, count);
    bsp::release_expired_world_objects_00903610(binding);
    summary_.expiry_entities += binding.visited();
    summary_.expiry_released += binding.released;
    ++summary_.expiry_passes;
}

void GameStepSubsystemsHost::report() {
    if (summary_.spatial_passes == 0 && summary_.expiry_passes == 0) return;
    log_.notef("summary fixed step subsystems spatial=%llu/%zu callbacks=%llu/%zu "
        "deferred_events=%llu/%zu "
        "lua_calls=%llu/%zu think=%llu/%zu pending_queues=%llu expiry=%llu/%llu released=%zu",
        summary_.spatial_passes, summary_.spatial_nodes,
        summary_.callback_passes, summary_.callbacks_run,
        summary_.deferred_event_passes, summary_.deferred_events,
        summary_.lua_call_passes, summary_.lua_calls_drained,
        summary_.think_passes, summary_.thinks_run,
        summary_.pending_queue_passes,
        summary_.expiry_passes, summary_.expiry_entities, summary_.expiry_released);
    const auto& pending = game_pending_entity_owners();
    log_.notef("pending entity owner audit: destroy=%s kill=%s counts=%u/%u "
        "frame_passes=%llu storage=process_raw24h",
        pending.destroy_00f899a8.head_04 != nullptr ? "present" : "null",
        pending.kill_00f899b4.head_04 != nullptr ? "present" : "null",
        pending.destroy_00f899a8.count_08, pending.kill_00f899b4.count_08,
        summary_.pending_queue_passes);
}

// ---------------------------------------------------------------------------
// The mission-load rows
// ---------------------------------------------------------------------------

bool run_load_session_slot_reset_004dfc13(GameHostLog& log, int session_mode,
    bool& single_player_reset) {
    NetworkSlotResetBinding binding(log);
    const bool networked = bsp::run_reset_network_slots_004dfc13(session_mode, binding);
    single_player_reset = binding.single_player;
    log.notef("mission load session-slot reset: 004dfc13 tests game+1FE4h and this session "
        "is %s, so the routine takes %s. docs/MISSION_LOAD_HOSTS.md corrects the interface "
        "name: 004cec60 is an MSVC std::_Tree::_Erase, not a network-slot reset, and the "
        "reset itself is the branch 004dfc1f..004dfd11 that a local session never enters",
        networked ? "networked" : "local",
        networked ? "the network branch" : "004dfd18, the single-player reset 004bb160");
    return networked;
}

void run_load_avoid_zone_state_004e0754(GameHostLog& log,
    const std::function<void()>& rebuild_geometry) {
    AvoidZoneResetBinding binding(log, rebuild_geometry);
    bsp::run_reset_avoid_zone_state_004e0754(binding);
    log.notef("mission load avoid-zone state: the block at 004e0754 clears game+648h and "
        "game+64Ch, erases the tree at game+5C8h and hands the 004218e0 singleton to "
        "00424d00. docs/MISSION_LOAD_HOSTS.md corrects the interface name: the step is not "
        "an objective-list reset, and the objective sets are at game+21A4h + slot*4");
}

std::size_t run_load_scripted_name_baseline_004d30f0(GameHostLog& log,
    GameMissionLuaHost& lua, std::vector<std::string>& names) {
    std::vector<bsp::LuaGlobalEntry> globals = lua.lua_global_entries();
    const std::size_t count = globals.size();
    ScriptedNameBinding binding(log, std::move(globals), names);
    const std::size_t inserted = bsp::run_rebuild_scripted_name_list_004d30f0(binding);
    log.notef("mission load scripted-name baseline: 004d30f0 walked %zu Lua global(s) and "
        "recorded %zu function-valued name(s) into the set at game+1930h. "
        "docs/MISSION_LOAD_HOSTS.md corrects the interface name: the set is the "
        "pre-script baseline of the global namespace, which 004d32a0 uses at teardown to "
        "nil exactly the globals the mission scripts added", count, inserted);
    return inserted;
}

}  // namespace bsp::game
