// bsp_game.exe milestone 2f: the load past the renderer-owner hosts, the
// mission state entry and the headless in-mission frame.
//
// Two things in this file are the executable's own decisions and are marked as
// such where they are made:
//
// 1. The load is driven by walking bsp::mission_load_host_steps() in its own
//    order rather than by entering bsp::run_mission_scene_load. That driver's
//    first world step hands the caller a bsp::GlobalSubsystemInvocation, which
//    is a bundle of references to twenty configuration contexts that nothing in
//    the repository builds yet, so the driver cannot be entered from here. The
//    order the walk follows is still the reconstruction's recovered table; what
//    the walk does not carry are the driver's own session-mode arms, and those
//    steps are reported as skipped arms rather than as performed.
// 2. The frame runs with no world. Every world container the reconstructions
//    walk is empty, so a routine such as 00481640 or 006dc1a0 runs to
//    completion over nothing. That is a real run of the recovered routine and
//    it is not a claim that the game's world was ticked.
//
// See include/bsp/game_hosts_mission_frame.hpp for the address list.

#include "bsp/game_hosts_mission_frame.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_menu.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/award_trackers.hpp"
#include "bsp/game_frame_control.hpp"
#include "bsp/game_settings.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/mission_load_path.hpp"
#include "bsp/mission_lua_host.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/mission_state_entry.hpp"
#include "bsp/mission_state_frame.hpp"
#include "bsp/render_tail.hpp"
#include "bsp/session_polls.hpp"
#include "bsp/simulation_gate.hpp"
#include "bsp/world_entities.hpp"
#include "bsp/world_ocean.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

namespace bsp::game {
namespace {

// 004e53ad's argument: the global clock in milliseconds, 00f876a4 times the
// double 1000.0 at 00ce47a0.
constexpr float kMillisecondsPerSecond = 1000.0f;

// The two profiler slots the in-mission frame brackets itself with are held at
// 0109db08 and 0109db14, both filled at run time by the counter registration
// 00408720 and therefore zero in the image. The executable picks its own, the
// same substitution milestone 2c recorded for the application frame's slot.
constexpr int kGameBlockSlot = 2;
constexpr int kRenderBlockSlot = 3;

void format_address(std::uint32_t value, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(value));
}

}  // namespace

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct GameMissionFrameHost::Impl {
    Impl(GameHostLog& log_in, GameVfsHost& vfs_in, GameMissionLuaHost& lua_in,
        GameFrameProfiler* profiler_in, std::string language_in)
        : log(log_in), vfs(vfs_in), lua(lua_in), profiler(profiler_in),
          language(std::move(language_in)) {}

    GameHostLog& log;
    GameVfsHost& vfs;
    GameMissionLuaHost& lua;
    GameFrameProfiler* profiler{};
    std::string language;

    GameMissionLoadRunSummary load{};
    GameMissionEntrySummary entry{};
    GameMissionFrameRunSummary frames{};

    // The parts of the game object this milestone owns.
    bsp::MissionSceneLoadState scene_state{};
    bsp::MissionStateEntryState entry_state{};
    bsp::DeviceWaitLatch device_latch{};
    bsp::MissionFrameState frame_state{};
    bsp::WorldTickState world{};
    bsp::InputTickState input{};
    bsp::GameFrameControlState control{};
    bsp::MenuInterfaceState menus{};
    bsp::SoundRequestQueue sound_requests{};
    bsp::ParticleClock particle_clock{};
    bsp::MissionCounterState mission_counters{};
    bsp::AudioSettings audio{};
    bsp::SceneSlotRecord slots[bsp::kSceneSlotRecordCount]{};
    bsp::MissionEntryPlayerSlot entry_slots[bsp::kLocalPlayerSlotCount]{};
    bsp::InputBindingDeviceGroups device_groups{};
    std::vector<bsp::HintCooldown> hint_cooldowns{};
    bool main_menu_manager_released{false};   // 00e198ac
    bool cinematic_hidden{false};             // game+634h
    bool cinematic_allow_simulation{false};   // game+635h
    int script_reentry_depth{0};              // game+644h
    std::int32_t published_mission_id{0};     // [00f8a2fc]+48h
    float world_clock{0.0f};                  // 00f876a4
    bool device_edge_injected{false};         // the executable's own injection

    void record(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.unimplemented(method, text);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.implemented(method, text);
    }
};

// ---------------------------------------------------------------------------
// bsp::InputTickHost, for the load's 00a92c40 and the device wait's own call
// ---------------------------------------------------------------------------

namespace {

class InputTickBinding final : public bsp::InputTickHost {
public:
    explicit InputTickBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}

    void backend_update(float seconds) override {
        static_cast<void>(seconds);
        // 00f8bbf4 vtable +4h. The input backend is the input owner's; this
        // process has the reconstructed action records but no backend object.
        owner_.record("InputTick::backend_update", 0x00f8bbf4u);
    }
    bool take_backend_bindings_dirty() override { return false; }  // backend+D4h
    const bsp::InputBindingDeviceGroups& binding_device_groups() const override {
        return owner_.device_groups;
    }
    void post_update_hook() override {}  // 00f8bbfc, null in this process

    std::uint8_t device_query_1c(bsp::InputDevice&, std::uint32_t) override { return 0; }
    std::uint8_t device_query_20(bsp::InputDevice&, std::uint32_t) override { return 0; }
    float device_value_24(bsp::InputDevice&, std::uint32_t) override { return 0.0f; }
    float crt_sqrt_00bf7030(float sum_of_squares) override {
        return sum_of_squares <= 0.0f ? 0.0f : std::sqrt(sum_of_squares);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::WorldTickHost. Every container the recovered walks read is empty, so the
// per-entity methods below are unreachable in this process and never record.
// ---------------------------------------------------------------------------

class WorldTickBinding final : public bsp::WorldTickHost {
public:
    explicit WorldTickBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}

    float world_clock() override { return owner_.world_clock; }

    void mission_events_pre_pass_00982540() override {
        owner_.record("MissionEvents::pre_pass", 0x00982540u);
    }
    void mission_events_periodic_00977990() override {
        owner_.record("MissionEvents::periodic", 0x00977990u);
    }
    void mission_events_poll_0096d540() override {
        owner_.record("MissionEvents::poll_zones", 0x0096d540u);
    }
    void mission_events_poll_00968550() override {
        owner_.record("MissionEvents::poll_triggers", 0x00968550u);
    }
    float mission_event_start_time(std::size_t) override { return 0.0f; }
    float mission_event_priority(std::size_t) override { return 0.0f; }
    bool mission_event_ready_005b71d0(std::size_t) override { return false; }
    void mission_event_destroy(std::size_t) override {}
    void mission_event_apply_00974070(std::size_t index) override {
        static_cast<void>(index);
        owner_.record("MissionEvents::apply", 0x00974070u);
        ++owner_.frames.mission_events_applied;
    }

    void bot_retarget_begin_0075b430(int) override {
        owner_.record("BotScheduler::retarget_begin", 0x0075b430u);
    }
    void bot_slot_prepare_00914390(std::size_t) override {
        owner_.record("BotScheduler::slot_prepare", 0x00914390u);
    }
    void bot_slot_dispatch_0076a9f0(std::uint32_t, std::uint32_t, std::size_t) override {
        owner_.record("BotScheduler::slot_dispatch", 0x0076a9f0u);
    }
    void bot_think_pass_a_00911e80(std::size_t) override {
        owner_.record("BotScheduler::think_a", 0x00911e80u);
    }
    void bot_think_pass_b_00912a60(std::size_t) override {
        owner_.record("BotScheduler::think_b", 0x00912a60u);
    }

    void marker_update(void*, float) override {
        owner_.record("Markers::update_marker", 0x006dc27fu);
    }
    bsp::MarkerColor marker_get_color(void*) override { return {}; }
    void marker_set_color(void*, const bsp::MarkerColor&) override {}
    void marker_set_scale_006dbac0(void*, int, int, float) override {}
    void marker_set_highlight_level(void*, float) override {}

    void entity_manager_update(float scaled_delta) override {
        static_cast<void>(scaled_delta);
        // The sub-manager at entityManager+8h, virtual +4h. Its object is the
        // world's; this process ticks the recovered walk over no entities.
        ++owner_.frames.units_ticked;
        owner_.record("EntityManager::submanager_update", 0x00481664u);
    }

    void power_ups_pre_pass_008eac80() override {
        owner_.record("PowerUps::pre_pass", 0x008eac80u);
    }
    void power_up_expire_008e8c30(void*) override {
        owner_.record("PowerUps::expire", 0x008e8c30u);
    }
    void power_up_notify_ready_009789a0(std::size_t, std::size_t) override {
        owner_.record("PowerUps::notify_ready", 0x009789a0u);
    }
    void power_ups_post_pass_00613760() override {
        owner_.record("PowerUps::post_pass", 0x00613760u);
    }

    void activate_entity_subtree_00922fd0(std::size_t) override {
        owner_.record("Entities::activate_subtree", 0x00922fd0u);
    }

    bool input_action_pressed(int action) override {
        // The 004c43c0 edge test. The executable's action table holds only the
        // press-start record, which the front end owns, so no in-mission action
        // is ever pressed here.
        static_cast<void>(action);
        return false;
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::MenuRequestServicer for 006840f0
// ---------------------------------------------------------------------------

class MenuServicerBinding final : public bsp::MenuRequestServicer {
public:
    explicit MenuServicerBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}
    void service_menu_channel(std::size_t, std::int32_t, std::int32_t) override {
        owner_.record("MenuRequests::service_channel", 0x006840f0u);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::GameFrameControlHost, for the mission completion poll 004d7ea0 only.
// Every method records when it is reached; the poll reaches exactly one.
// ---------------------------------------------------------------------------

class FrameControlBinding final : public bsp::GameFrameControlHost {
public:
    explicit FrameControlBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}

    void pre_tick_console_commands() override {
        owner_.record("FrameControl::pre_tick_console", 0x004e2200u);
    }
    void copy_local_player_slots(bsp::LocalPlayerSlot (&slots)[bsp::kLocalPlayerSlotCount])
        override {
        for (std::size_t i = 0; i < bsp::kLocalPlayerSlotCount; ++i) {
            slots[i] = bsp::LocalPlayerSlot{};
        }
    }
    float mission_time_filter_007713a0(float delta) override { return delta; }
    bool max_step_clamp_disabled() override { return false; }
    bool input_action_held(int) override { return false; }
    bool input_action_pressed(int) override { return false; }

    void request_02_004d8000() override { owner_.record("Drain::request_02", 0x004d8000u); }
    void request_04_004e4000() override { owner_.record("Drain::request_04", 0x004e4000u); }
    void request_06_notify_00e198ac() override {
        owner_.record("Drain::request_06", 0x00e198acu);
    }
    void request_07_004bfc70() override { owner_.record("Drain::request_07", 0x004bfc70u); }
    void request_09_notify_00e198b4() override {
        owner_.record("Drain::request_09", 0x00e198b4u);
    }
    void request_0a_0b_004dfb70() override { owner_.record("Drain::request_0a", 0x004dfb70u); }
    void request_0e_004c6b00() override { owner_.record("Drain::request_0e", 0x004c6b00u); }
    void request_0f_004d7970() override {
        // BSP_Game_EndScene, the debrief handler docs/MISSION_RESULT_DECISION.md
        // reconstructs. It commits or discards the score record through the
        // profile, which this process does not own.
        owner_.record("Drain::request_0f_end_scene", 0x004d7970u);
    }
    bool request_10_teardown_004e458a(bsp::GameFrameControlState&) override {
        owner_.record("Drain::request_10_teardown", 0x004e458au);
        return false;
    }
    void request_12_resume_004cd0f0() override {
        owner_.record("Drain::request_12", 0x004cd0f0u);
    }
    void request_14_004bac20() override { owner_.record("Drain::request_14", 0x004bac20u); }
    void request_16_notify_00e198b8() override {
        owner_.record("Drain::request_16", 0x00e198b8u);
    }
    bool network_session_active() override { return false; }
    void post_drain_00a95960(float) override {
        owner_.record("Drain::post_drain", 0x00a95960u);
    }
    void update_cutscene_playback_004c6b20(float) override {
        owner_.record("FrameControl::cutscene_playback", 0x004c6b20u);
    }
    void mission_hud_update(float) override {
        owner_.record("FrameControl::mission_hud_update", 0x00f88c20u);
    }
    void accumulate_frame_statistics_0053c510() override {
        owner_.record("FrameControl::frame_statistics", 0x0053c510u);
    }
    void update_presence_context_004c0170() override {
        owner_.record("FrameControl::presence_context", 0x004c0170u);
    }
    void update_device_wait_screen_004db920() override {
        owner_.record("FrameControl::device_wait", 0x004db920u);
    }
    void set_front_end_pending_flag(bool) override {}
    bsp::MissionResult mission_result() override {
        // game+7188h. 004dfe83 released the previous result during the load and
        // nothing in this process builds a new one, so the poll stops here and
        // the debrief request 0Fh is never enqueued.
        owner_.done("MissionCompletion::mission_result", 0x004d7ea0u);
        return bsp::MissionResult{};
    }
    void world_final_tick(float) override {
        owner_.record("MissionCompletion::world_final_tick", 0x00903670u);
    }
    void world_post_tick() override {
        owner_.record("MissionCompletion::world_post_tick", 0x00903670u);
    }
    void show_mission_result_gui(float) override {
        owner_.record("MissionCompletion::result_gui", 0x004d7f1du);
    }
    void close_mission_result() override {
        owner_.record("MissionCompletion::close_result", 0x004cd610u);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::MissionDeviceWaitHost (004db920)
// ---------------------------------------------------------------------------

class DeviceWaitBinding final : public bsp::MissionDeviceWaitHost {
public:
    DeviceWaitBinding(GameMissionFrameHost::Impl& owner, bool& entered)
        : owner_(owner), entered_(entered) {}

    bool any_dynamic_device_button_down() override {
        // 00a91020 with ECX = the device table 00f8bbf4. This process has no
        // input backend, so nothing would ever report a button down and the
        // single-player arm would wait forever. The executable therefore
        // injects one device-down sample here, which the recovered latch at
        // table+DDh turns into exactly one rising edge. **This is the
        // executable's own injection, not recovered behaviour**, and it is the
        // same substitution --press-start-frame makes for the title page.
        owner_.record("DeviceWait::any_button_down", 0x00a91020u);
        if (owner_.device_edge_injected) return false;
        owner_.device_edge_injected = true;
        owner_.log.notef("device-wait edge injected: the state 0Ch arm waits on "
            "00a91020, which needs an input backend this process does not build");
        return true;
    }
    bool menu_command_screen_busy() override { return false; }  // 00425d10 +25Ch
    bool gui_root_busy() override { return false; }             // 00f8abe8 +3E8h
    void update_input_manager(float delta_seconds) override {
        InputTickBinding input(owner_);
        bsp::update_input_manager_00a92c40(owner_.input, delta_seconds, input);
        owner_.done("DeviceWait::update_input_manager", 0x00a92c40u);
    }
    void dispatch_session_event(int event_tag) override {
        static_cast<void>(event_tag);
        owner_.record("DeviceWait::dispatch_session_event", 0x0075b430u);
    }
    void enter_mission_state() override { entered_ = true; }

private:
    GameMissionFrameHost::Impl& owner_;
    bool& entered_;
};

// ---------------------------------------------------------------------------
// bsp::MissionStateEntryHost (004da6c0)
// ---------------------------------------------------------------------------

class DynamicsReleaseBinding final : public bsp::DynamicsReleaseHost {
public:
    explicit DynamicsReleaseBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}
    void release_dynamics_handle_00c34f70(std::uint32_t) override {
        owner_.record("Dynamics::release_handle", 0x00c34f70u);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

class MissionEntryBinding final : public bsp::MissionStateEntryHost {
public:
    explicit MissionEntryBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}

    void enter_scope(const char* label) override {
        static_cast<void>(label);  // 004254b0, a no-op body in this build
    }
    void release_deferred_dynamics() override {
        bsp::GameDynamicsList list{};
        DynamicsReleaseBinding release(owner_);
        owner_.entry.dynamics_released
            = bsp::release_all_dynamics_00447060(list, release);
        owner_.done("MissionEntry::release_deferred_dynamics", 0x00447060u);
    }
    void mark_local_slot_ready(std::size_t slot_index, std::uint16_t value) override {
        static_cast<void>(slot_index);
        static_cast<void>(value);
        owner_.done("MissionEntry::mark_local_slot_ready", 0x004da71eu);
    }
    void set_audio_environment_level(float level, std::uint32_t bus_mask) override {
        static_cast<void>(level);
        static_cast<void>(bus_mask);
        owner_.record("MissionEntry::set_audio_environment_level", 0x00a7a440u);
    }
    void apply_in_game_interface(bool loading) override {
        // 004c9ca0 with 0 on this path: the entry arm tears the loading element
        // down, releases interface/textures/allbutingame.ats, selects front-end
        // layout set 3 and writes the engine-movie latch at game+1EE0h. All
        // four belong to the GUI and renderer owners.
        owner_.entry.interface_request = loading ? "allocate (1)" : "tear down (0)";
        owner_.record("MissionEntry::apply_in_game_interface", 0x004c9ca0u);
    }
    void check_multiplayer_player_count() override {
        owner_.record("MissionEntry::check_player_count", 0x004d87b0u);
    }
    std::uint32_t game_state() override { return owner_.entry_state.game_state; }
    void set_cinematic_mode(bool hide, bool allow_simulation, bool third) override {
        static_cast<void>(third);
        const bsp::CinematicModeFlags flags
            = bsp::cinematic_flags_004cd0f0(hide, allow_simulation);
        owner_.cinematic_hidden = flags.hud_hidden;
        owner_.cinematic_allow_simulation = flags.simulate_while_hidden;
        owner_.entry.cinematic_cleared = !flags.hud_hidden && !flags.simulate_while_hidden;
        owner_.done("MissionEntry::set_cinematic_mode", 0x004cd0f0u);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::MissionFrameHost, the in-mission branch of 004e4a40
// ---------------------------------------------------------------------------

class MissionFrameBinding final : public bsp::MissionFrameHost {
public:
    explicit MissionFrameBinding(GameMissionFrameHost::Impl& owner)
        : owner_(owner), world_(owner), control_(owner), servicer_(owner) {}

    // --- the mission-start one-shot --------------------------------------
    bool session_counts_mission_start_004b6260() override {
        owner_.done("MissionFrame::session_counts_mission_start", 0x004b6260u);
        // No session object, so session+F4h is null and the routine answers no.
        return bsp::session_counts_mission_004b6260(false, false);
    }
    void adjust_mission_start_counters_004bcaa0(bool add) override {
        bsp::adjust_mission_counters_004bcaa0(owner_.mission_counters, add);
        owner_.done("MissionFrame::adjust_mission_counters", 0x004bcaa0u);
    }

    // --- the warning director and the input effect sets ------------------
    void update_warning_manager_00987590(float scaled_delta) override {
        static_cast<void>(scaled_delta);
        WorldTickBinding& world = world_;
        const bsp::MissionEventTickResult result
            = bsp::update_mission_events_00987590(owner_.world, world);
        static_cast<void>(result);
        owner_.done("MissionFrame::update_mission_events", 0x00987590u);
    }
    int run_input_effect_sets_004e4e6c(float expiry_reference) override {
        const int erased
            = bsp::run_input_effect_lists_004e4e6c(owner_.input, expiry_reference);
        owner_.frames.input_entries_erased += static_cast<unsigned long long>(erased);
        owner_.done("MissionFrame::run_input_effect_sets", 0x004e4e6cu);
        return erased;
    }

    // --- frame bookkeeping -----------------------------------------------
    void record_action_deadlines_004d8cd0() override {
        WorldTickBinding& world = world_;
        bsp::record_action_deadlines_004d8cd0(owner_.world, world);
        owner_.done("MissionFrame::record_action_deadlines", 0x004d8cd0u);
    }
    void multiplayer_tick_00778560(float raw_delta) override {
        static_cast<void>(raw_delta);
        owner_.record("MissionFrame::multiplayer_tick", 0x00778560u);
    }
    void begin_game_profile_block() override { begin_block(kGameBlockSlot, "game"); }
    void end_game_profile_block() override { end_block(kGameBlockSlot, "game"); }

    // --- the simulation gate ---------------------------------------------
    void register_game_block_label() override {
        owner_.record("MissionFrame::register_block_label", 0x0041e870u);
    }
    void update_in_mission_subsystems_004c40a0() override {
        // The fixed four-call opener of every simulated frame. Packet
        // cc_mission_tick owns it; it had not merged when this ran.
        owner_.record("MissionFrame::update_in_mission_subsystems", 0x004c40a0u);
    }
    void begin_engine_movie_004cce50() override {
        owner_.record("MissionFrame::begin_engine_movie", 0x004cce50u);
    }
    bsp::SimulationGateBranch pause_gate_branch_004e5153() override {
        bsp::PauseDecisionInputs inputs{};
        inputs.multiplayer_session = false;
        inputs.hud_hidden = owner_.cinematic_hidden;
        inputs.top_populated_level = 0;
        const bsp::SimulationGateBranch branch
            = bsp::decide_simulation_gate_branch(inputs);
        owner_.done("MissionFrame::pause_gate_branch", 0x004e5153u);
        return branch;
    }
    bool tutorial_hint_step_available() override { return false; }
    void advance_tutorial_hint_0054e440() override {
        owner_.record("MissionFrame::advance_tutorial_hint", 0x0054e440u);
    }
    void toggle_pause_menu_004db030() override {
        owner_.record("MissionFrame::toggle_pause_menu", 0x004db030u);
    }
    bool in_game_interface_active() override {
        // 00e198c4 and its +3Ch. The in-mission HUD manager 0068a990 builds is
        // a record in the load, so the process has no such object.
        owner_.record("MissionFrame::in_game_interface_active", 0x004e524cu);
        return false;
    }
    void update_in_game_interface_0068c1f0() override {
        owner_.record("MissionFrame::update_in_game_interface", 0x0068c1f0u);
    }
    void update_interface_only_004c40f0() override {
        // 004c40f0's own reconstruction needs the front-end screen registry the
        // load released at 00e198ac. With no manager it would have nothing to
        // pump, so the call is recorded rather than run over an empty registry
        // that would make the pump look like it did something.
        ++owner_.frames.interface_updates;
        owner_.record("MissionFrame::update_interface_only", 0x004c40f0u);
    }

    // --- the seven hint passes -------------------------------------------
    void hint_tick_cooldowns_0068ec10(float raw_delta) override {
        const std::size_t surviving = bsp::tick_hint_cooldowns_0068ec10(
            owner_.hint_cooldowns.empty() ? nullptr : owner_.hint_cooldowns.data(),
            owner_.hint_cooldowns.size(), raw_delta);
        owner_.hint_cooldowns.resize(surviving);
        owner_.done("MissionFrame::hint_tick_cooldowns", 0x0068ec10u);
    }
    void hint_drain_queued_00692b00() override {
        bsp::QueuedHintDrainInputs inputs{};
        static_cast<void>(bsp::should_drain_queued_hint_00692b00(inputs));
        owner_.done("MissionFrame::hint_drain_queued", 0x00692b00u);
    }
    void hint_unit_class_00692b60() override {
        owner_.record("MissionFrame::hint_unit_class", 0x00692b60u);
    }
    void hint_weapon_006926f0() override {
        owner_.record("MissionFrame::hint_weapon", 0x006926f0u);
    }
    void hint_environment_00692580() override {
        owner_.record("MissionFrame::hint_environment", 0x00692580u);
    }
    void hint_zone_first_get_00692fd0() override {
        owner_.record("MissionFrame::hint_zone", 0x00692fd0u);
    }
    void hint_strategic_map_00692960() override {
        owner_.record("MissionFrame::hint_strategic_map", 0x00692960u);
    }

    // --- the world tick ---------------------------------------------------
    void update_bot_scheduler_00914ef0(float scaled_delta) override {
        static_cast<void>(scaled_delta);
        WorldTickBinding& world = world_;
        bsp::update_bot_scheduler_00914ef0(owner_.world, world);
        owner_.done("MissionFrame::update_bot_scheduler", 0x00914ef0u);
    }
    void update_markers_006dc1a0(float scaled_delta) override {
        static_cast<void>(scaled_delta);
        WorldTickBinding& world = world_;
        bsp::update_markers_006dc1a0(owner_.world, world);
        owner_.done("MissionFrame::update_markers", 0x006dc1a0u);
    }
    void update_entity_manager_00481640(float scaled_delta) override {
        static_cast<void>(scaled_delta);
        WorldTickBinding& world = world_;
        bsp::update_entity_manager_00481640(owner_.world, world);
        owner_.done("MissionFrame::update_entity_manager", 0x00481640u);
    }
    void update_rain_descriptor_00865ab0() override {
        owner_.record("MissionFrame::update_rain_descriptor", 0x00865ab0u);
    }
    void update_ocean_00bbddd0(float) override {
        owner_.record("MissionFrame::update_ocean", 0x00bbddd0u);
    }
    void update_decals_00740e10(float scaled_delta) override {
        static_cast<void>(bsp::update_decal_manager_00740e10(owner_.world.decals,
            scaled_delta));
        owner_.done("MissionFrame::update_decals", 0x00740e10u);
    }
    void update_00f89b3c_0094c8f0(float) override {
        owner_.record("MissionFrame::update_00f89b3c", 0x0094c8f0u);
    }
    void update_power_ups_008eb110() override {
        WorldTickBinding& world = world_;
        bsp::update_power_ups_008eb110(owner_.world, world);
        owner_.done("MissionFrame::update_power_ups", 0x008eb110u);
    }
    void update_effect_manager_00867ee0(float) override {
        owner_.record("MissionFrame::update_effect_manager", 0x00867ee0u);
    }
    void flush_entity_activations_00903670() override {
        WorldTickBinding& world = world_;
        bsp::flush_entity_activations_00903670(owner_.world, world);
        owner_.done("MissionFrame::flush_entity_activations", 0x00903670u);
    }
    bool check_mission_completion_004d7ea0() override {
        FrameControlBinding& control = control_;
        const bool enqueued
            = bsp::check_mission_completion_004d7ea0(owner_.control, control);
        owner_.done("MissionFrame::check_mission_completion", 0x004d7ea0u);
        if (enqueued) owner_.frames.completion_requested = true;
        return enqueued;
    }

    // --- the particle clock ----------------------------------------------
    void set_particle_clock_time_00b19a10(float time_milliseconds) override {
        bsp::set_particle_clock_time_00b19a10(owner_.particle_clock, time_milliseconds);
        owner_.done("MissionFrame::set_particle_clock_time", 0x00b19a10u);
    }

    // --- post-simulation ---------------------------------------------------
    void set_foliage_shader_time_00af0450(float) override {
        owner_.record("MissionFrame::set_foliage_shader_time", 0x00af0450u);
    }
    void build_foliage_visible_set_00af0c50() override {
        // The recovered builder takes the camera at [[game+19F0h]+A8h], which is
        // the renderer's.
        owner_.record("MissionFrame::build_foliage_visible_set", 0x00af0c50u);
    }
    void apply_gui_visibility_004c6c70() override {
        bsp::GuiVisibilityInputs inputs{};
        static_cast<void>(bsp::decide_gui_visibility_004c6c70(inputs));
        owner_.done("MissionFrame::apply_gui_visibility", 0x004c6c70u);
    }
    bool interface_manager_present() override {
        // 00e198ac, which the load released at 004dfd90.
        return !owner_.main_menu_manager_released;
    }
    void update_interface_music_00685c80(float) override {
        owner_.record("MissionFrame::update_interface_music", 0x00685c80u);
    }
    void update_multiplayer_interface_004d80d0() override {
        owner_.record("MissionFrame::update_multiplayer_interface", 0x004d80d0u);
    }
    bool service_pending_menu_requests_006840f0() override {
        MenuServicerBinding& servicer = servicer_;
        const bool serviced
            = bsp::service_pending_menu_requests_006840f0(owner_.menus, servicer);
        owner_.done("MissionFrame::service_pending_menu_requests", 0x006840f0u);
        return serviced;
    }
    void pump_peer_queues_00776230() override {
        owner_.record("MissionFrame::pump_peer_queues", 0x00776230u);
    }
    void apply_sound_requests_00941140() override {
        static_cast<void>(bsp::apply_sound_request_00941140(owner_.sound_requests, false));
        owner_.done("MissionFrame::apply_sound_requests", 0x00941140u);
    }
    void update_front_end_screens_004d8620() override {
        owner_.record("MissionFrame::update_front_end_screens", 0x004d8620u);
    }

    // --- render ------------------------------------------------------------
    void begin_render_profile_block() override { begin_block(kRenderBlockSlot, "render"); }
    void render_004ca440() override {
        owner_.record("MissionFrame::render", 0x004ca440u);
    }
    void end_render_profile_block() override { end_block(kRenderBlockSlot, "render"); }
    void finish_render_frame_004ca1f0() override {
        owner_.record("MissionFrame::finish_render_frame", 0x004ca1f0u);
    }

    // --- tail --------------------------------------------------------------
    bool frame_metrics_enabled() override { return false; }  // 00e1aed4
    void submit_frame_metrics_00757ce0(float) override {
        owner_.record("MissionFrame::submit_frame_metrics", 0x00757ce0u);
    }

private:
    void begin_block(int slot, const char* label) {
        static_cast<void>(label);
        if (owner_.profiler == nullptr) {
            owner_.record("MissionFrame::begin_profile_block", 0x00be3640u);
            return;
        }
        bsp::profiler_begin_frame_slot_00be3640(owner_.profiler->counters(), slot,
            *owner_.profiler);
        owner_.done("MissionFrame::begin_profile_block", 0x00be3640u);
    }
    void end_block(int slot, const char* label) {
        static_cast<void>(label);
        if (owner_.profiler == nullptr) {
            owner_.record("MissionFrame::end_profile_block", 0x00be3660u);
            return;
        }
        bsp::profiler_end_frame_slot_00be3660(owner_.profiler->counters(), slot,
            *owner_.profiler);
        owner_.done("MissionFrame::end_profile_block", 0x00be3660u);
    }

    GameMissionFrameHost::Impl& owner_;
    WorldTickBinding world_;
    FrameControlBinding control_;
    MenuServicerBinding servicer_;
};

}  // namespace

// ---------------------------------------------------------------------------
// GameMissionFrameHost
// ---------------------------------------------------------------------------

GameMissionFrameHost::GameMissionFrameHost(GameHostLog& log, GameVfsHost& vfs,
    GameMissionLuaHost& lua, GameFrameProfiler* profiler, std::string language)
    : impl_(std::make_unique<Impl>(log, vfs, lua, profiler, std::move(language))) {}

GameMissionFrameHost::~GameMissionFrameHost() = default;

const GameMissionLoadRunSummary& GameMissionFrameHost::load_summary() const noexcept {
    return impl_->load;
}
const GameMissionEntrySummary& GameMissionFrameHost::entry_summary() const noexcept {
    return impl_->entry;
}
const GameMissionFrameRunSummary& GameMissionFrameHost::frame_summary() const noexcept {
    return impl_->frames;
}

void GameMissionFrameHost::run_scene_load_004dfb70(const std::string& scene_path,
    const std::string& script_name, const std::string& locale_tables,
    std::int32_t mission_id) {
    Impl& host = *impl_;
    host.load.ran = true;
    host.load.short_name = bsp::derive_scene_short_name(scene_path);
    host.load.script_name = script_name;
    host.load.mission_id = mission_id;

    host.scene_state.state = bsp::kMissionSceneLoadRequest;
    host.scene_state.scene_path = scene_path;
    host.scene_state.script_name = script_name;
    host.scene_state.locale_table_list = locale_tables;
    host.scene_state.mission_id = mission_id;
    host.scene_state.have_scene_record = true;

    // 004e087b picks the script slot; single player always resolves to 8, and
    // 004e0a50 reads the raw slot to choose the arm.
    host.load.script_slot = bsp::normalise_mission_script_slot(
        host.scene_state.script_slot, host.scene_state.script_slot_forced,
        host.scene_state.session_mode);
    host.load.engine_movie_arm = bsp::mission_uses_engine_movie(host.scene_state.script_slot);

    // The five numbered VFS file blocks the load opens around its phases.
    const bsp::MissionLoadPhase phases[] = {
        bsp::MissionLoadPhase::WorldConstruction, bsp::MissionLoadPhase::MissionScript,
        bsp::MissionLoadPhase::StageInit, bsp::MissionLoadPhase::EngineMovie,
        bsp::MissionLoadPhase::RendererHandoff,
    };
    for (const bsp::MissionLoadPhase phase : phases) {
        host.load.blocks.push_back(
            bsp::mission_load_phase_block_name(phase, host.load.short_name));
    }

    host.log.notef("mission load resumes past the renderer owners: scene=%s short=%s "
        "script=%s slot=%d engine_movie_arm=%d", scene_path.c_str(),
        host.load.short_name.c_str(), script_name.c_str(), host.load.script_slot,
        host.load.engine_movie_arm ? 1 : 0);

    std::size_t count = 0;
    const bsp::MissionLoadHostStep* steps = bsp::mission_load_host_steps(count);
    for (std::size_t i = 0; i < count; ++i) {
        const bsp::MissionLoadHostStep& step = steps[i];
        if (std::strcmp(step.host, "MissionSceneLoadHost") != 0) continue;
        ++host.load.steps;
        const std::string method(step.method);
        char address[16];
        format_address(step.address, address);
        char label[128];
        std::snprintf(label, sizeof(label), "MissionLoad::%s", step.method);

        // The arms the driver never takes on a single-player load. They are
        // reported as skipped rather than as performed or as missing.
        if (method == "reset_network_slots" || method == "detach_network_menu_manager"
            || method == "assign_party_player_slots" || method == "activate_slot"
            || method == "dispatch_session_ready_event") {
            ++host.load.skipped_arms;
            continue;
        }

        if (method == "set_cinematic_mode") {
            const bsp::CinematicModeFlags flags = bsp::cinematic_flags_004cd0f0(false, false);
            host.cinematic_hidden = flags.hud_hidden;
            host.cinematic_allow_simulation = flags.simulate_while_hidden;
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "reset_single_player_slots") {
            // 004bb160 points game+18CCh at the eight 118h records based at
            // game+1008h; 004bb440 then claims one for the local player and
            // writes its +0Eh device byte, which is what the state 0Ch handler
            // counts.
            for (std::size_t slot = 0; slot < bsp::kSceneSlotRecordCount; ++slot) {
                host.slots[slot] = bsp::SceneSlotRecord{};
            }
            host.slots[0].in_use = true;
            host.load.slots_reset = bsp::kSceneSlotRecordCount;
            for (std::size_t slot = 0; slot < bsp::kLocalPlayerSlotCount; ++slot) {
                host.entry_slots[slot] = bsp::MissionEntryPlayerSlot{};
                host.entry_slots[slot].device_bound = true;
            }
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "release_main_menu_manager") {
            host.main_menu_manager_released = true;
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "release_mission_result") {
            host.control.mission_result = bsp::MissionResult{};
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "load_scene_file") {
            // The header pass already ran against the same bytes when milestone
            // 2e built the record; the load runs it again, this time against the
            // live database, which is the scene-graph owner's. The first run is
            // reported by the mission host.
            host.record(label, step.address);
            ++host.load.records;
            continue;
        }
        if (method == "lua_reset_state") {
            host.lua.publish_lobby_settings_005e2f00();
            ++host.load.concrete;
            continue;
        }
        if (method == "register_locale_table") {
            const std::vector<std::string> names
                = bsp::split_locale_table_list(locale_tables);
            host.load.locale_tables = names.size();
            for (const std::string& name : names) {
                host.log.notef("  locale table %s requested by the scene record", name.c_str());
            }
            host.record(label, step.address);
            ++host.load.records;
            continue;
        }
        if (method == "input_update") {
            InputTickBinding input(host);
            bsp::update_input_manager_00a92c40(host.input, 0.0f, input);
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "global_subsystems") {
            // 004dc6a0 is the world owner's and needs a GlobalSubsystemContext
            // this process cannot build, so the construction itself is a
            // record. One step inside it does run: 004dc72f calls 00886900,
            // and the mission chunk depends on the globals those two folders
            // leave behind (docs/MISSION_LUA_MACHINE.md, gap 1).
            host.record(label, step.address);
            ++host.load.records;
            if (!host.lua.started()) host.lua.start_machine_00884be0();
            host.lua.run_global_script_folders_00886900();
            continue;
        }
        if (method == "run_mission_script") {
            if (!host.lua.started()) host.lua.start_machine_00884be0();
            ++host.script_reentry_depth;  // game+644h, raised across the group
            host.lua.run_mission_script_008860b0(script_name);
            ++host.load.concrete;
            continue;
        }
        if (method == "lua_call_entry_point_a") {
            host.lua.call_entry_point(bsp::kLuaPrecacheUnits, false);
            host.lua.call_entry_point(bsp::kLuaStageInitMulti, false);
            ++host.load.concrete;
            continue;
        }
        if (method == "lua_call_entry_point_b") {
            if (host.load.engine_movie_arm) {
                host.lua.call_entry_point(bsp::kLuaEngineMovieInit, true);
            } else {
                host.lua.call_entry_point(bsp::kLuaStageInit, true);
            }
            if (host.script_reentry_depth > 0) --host.script_reentry_depth;
            ++host.load.concrete;
            continue;
        }
        if (method == "publish_mission_id") {
            host.published_mission_id = mission_id;
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }

        host.record(label, step.address);
        ++host.load.records;
    }

    // 004e086b, immediately after 0077f5e0.
    host.scene_state.state = bsp::kGameStateSceneReady;
    host.load.state_after = bsp::kGameStateSceneReady;
    host.entry_state.game_state = bsp::kGameStateSceneReady;
    host.log.notef("mission load finished: %zu steps, %zu concrete, %zu records, "
        "%zu arms not taken; game state = 0x%02X", host.load.steps, host.load.concrete,
        host.load.records, host.load.skipped_arms, host.load.state_after);
    host.log.notef("  numbered VFS blocks: %s %s %s %s %s", host.load.blocks[0].c_str(),
        host.load.blocks[1].c_str(), host.load.blocks[2].c_str(),
        host.load.blocks[3].c_str(), host.load.blocks[4].c_str());
}

bool GameMissionFrameHost::enter_mission_state_004da6c0() {
    Impl& host = *impl_;
    if (host.load.state_after != bsp::kGameStateSceneReady) return false;

    bsp::MissionDeviceWaitInputs inputs{};
    inputs.local_view_mode = host.scene_state.session_mode;
    inputs.session_flag_29c = false;
    inputs.game_field_624 = 0;
    inputs.unbound_player_slots = bsp::count_unbound_player_slots(host.entry_slots);

    bool entered = false;
    DeviceWaitBinding device_wait(host, entered);
    host.entry.device_wait_ran = true;
    const bool wait_entered
        = bsp::run_mission_device_wait(inputs, host.device_latch, device_wait);
    host.done("MissionState::device_wait", 0x004db920u);

    if (!entered && !wait_entered) {
        host.log.notef("state 0Ch did not enter: %zu player slot(s) still unbound",
            inputs.unbound_player_slots);
        return false;
    }

    host.entry_state.game_state = bsp::kGameStateSceneReady;
    host.entry_state.local_view_mode = host.scene_state.session_mode;
    host.entry_state.local_slot_index = 0;
    MissionEntryBinding entry(host);
    const bool ran = bsp::run_mission_state_entry(host.entry_state, host.audio, entry);
    host.done("MissionState::enter_mission_state", 0x004da6c0u);
    host.entry.entered = ran
        && host.entry_state.game_state == static_cast<std::uint32_t>(bsp::GameStateId::kInMission);
    host.entry.state = host.entry_state.game_state;

    // The frame reads what the entry left behind.
    host.frame_state.state = host.entry_state.game_state;
    host.frame_state.scaled_delta = host.entry_state.scaled_frame_delta;
    host.frame_state.cinematic_hidden = host.cinematic_hidden;
    host.frame_state.cinematic_allow_simulation = host.cinematic_allow_simulation;
    host.frame_state.warning_manager_present = true;  // game+21E0h, the owned queue
    host.control.state = host.entry_state.game_state;

    host.log.notef("mission state entry: state=0x%02X entered=%d dynamics_released=%zu "
        "interface=%s cinematic_cleared=%d one_shots{aborted=%d end_latch=%d "
        "networked=%d dropped=%d not_enough_players=%d}",
        host.entry.state, host.entry.entered ? 1 : 0, host.entry.dynamics_released,
        host.entry.interface_request.empty() ? "(none)" : host.entry.interface_request.c_str(),
        host.entry.cinematic_cleared ? 1 : 0,
        host.entry_state.one_shots.scene_ended_by_abort ? 1 : 0,
        host.entry_state.one_shots.end_scene_body_done ? 1 : 0,
        host.entry_state.one_shots.session_was_networked ? 1 : 0,
        host.entry_state.one_shots.session_dropped ? 1 : 0,
        host.entry_state.one_shots.not_enough_players ? 1 : 0);
    return host.entry.entered;
}

bool GameMissionFrameHost::run_mission_frame_004e4a40(float raw_delta) {
    Impl& host = *impl_;
    if (!host.entry.entered) return false;

    host.world_clock += raw_delta;
    host.frame_state.raw_delta = raw_delta;
    host.frame_state.scaled_delta = raw_delta;  // no time dilation in this process
    host.frame_state.global_time = host.world_clock;
    host.world.game.elapsed = host.world_clock;

    MissionFrameBinding binding(host);
    const bsp::MissionFrameResult result
        = bsp::run_mission_frame(host.frame_state, binding);
    ++host.frames.frames;
    if (result.simulated) ++host.frames.simulated;
    if (result.paused) ++host.frames.paused;
    host.frames.menu_drain_iterations
        += static_cast<unsigned long long>(result.menu_drain_iterations);
    host.frames.script_calls = host.lua.summary().native_calls;
    if (result.mission_completion_requested) host.frames.completion_requested = true;

    host.log.notef("  mission frame %llu simulated=%d paused=%d units=%llu events=%llu "
        "script_calls=%llu erased=%d", host.frames.frames, result.simulated ? 1 : 0,
        result.paused ? 1 : 0, host.frames.units_ticked,
        host.frames.mission_events_applied, host.frames.script_calls,
        result.input_entries_erased);
    return !result.mission_completion_requested;
}

void GameMissionFrameHost::report(long requested_frames) {
    Impl& host = *impl_;
    host.frames.requested = requested_frames;
    // docs/MISSION_RESULT_DECISION.md's exit is reached only through 004d7ea0,
    // which needs a mission-result object at game+7188h. 004dfe83 released the
    // previous one during the load and nothing in this process builds a new
    // one, so no frame can enqueue request 0Fh.
    host.frames.exit_reachable = host.frames.completion_requested;
    host.frames.exit_note = host.frames.completion_requested
        ? "004d7ea0 enqueued the debrief request 0Fh"
        : "no mission-result object at game+7188h, so 004d7ea0 never enqueues 0Fh "
          "and the debrief path of 004d7970 is unreachable in this process";
    host.log.notef("summary mission frames requested=%ld ran=%llu simulated=%llu paused=%llu "
        "units=%llu events=%llu script_calls=%llu interface_updates=%llu",
        host.frames.requested, host.frames.frames, host.frames.simulated,
        host.frames.paused, host.frames.units_ticked, host.frames.mission_events_applied,
        host.frames.script_calls, host.frames.interface_updates);
    host.log.notef("summary mission exit reachable=%d: %s",
        host.frames.exit_reachable ? 1 : 0, host.frames.exit_note.c_str());
}

}  // namespace bsp::game
