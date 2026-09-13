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
#include <stdexcept>

#include "bsp/game_hosts.hpp"
#include "bsp/game_observer_runtime.hpp"
#include "bsp/game_hosts_fixed_step.hpp"
#include "bsp/game_hosts_hud.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_menu.hpp"
#include "bsp/game_hosts_ready.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/game_hosts_mission_result.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/game_hosts_ship_ai.hpp"
#include "bsp/game_hosts_trajectory.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_hosts_world.hpp"
#include "bsp/award_trackers.hpp"
#include "bsp/game_dynamics_list.hpp"
#include "bsp/game_frame_control.hpp"
#include "bsp/in_mission_subsystem_tick.hpp"
#include "bsp/game_settings.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/lua_binding_mission_2.hpp"
#include "bsp/mission_load_path.hpp"
#include "bsp/mission_lua_host.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/mission_state_entry.hpp"
#include "bsp/mission_state_frame.hpp"
#include "bsp/render_tail.hpp"
#include "bsp/session_polls.hpp"
#include "bsp/session_participant_pools.hpp"
#include "bsp/simulation_gate.hpp"
#include "bsp/world_entities.hpp"
#include "bsp/world_ocean.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

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

// Milestone 2g. How many frames the exit path is given after the mission leaves
// game state 0Dh before the run gives up on it. The recovered path needs two:
// one to dispatch request 10h and lower the suspension the teardown arm raised,
// and one to dispatch the request 04h the state 11h arm enqueued. The bound
// exists so a path that stalls ends the run instead of spinning.
constexpr unsigned long long kMaxExitFrames = 8;

void format_address(std::uint32_t value, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(value));
}

}  // namespace

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct GameMissionFrameHost::Impl {
    Impl(GameHostLog& log_in, GameVfsHost& vfs_in, GameMissionLuaHost& lua_in,
        bsp::SessionParticipantPools& participants_in,
        GameFrameProfiler* profiler_in, std::string language_in)
        : log(log_in), vfs(vfs_in), lua(lua_in), participants(participants_in), profiler(profiler_in),
          language(std::move(language_in)) {
        // Both take references to members declared below, which are already
        // default constructed when this body runs.
        fixed_step = std::make_unique<GameFixedStepHost>(log, dynamics);
        result = std::make_unique<GameMissionResultHost>(log, control);
        // Milestone 2m: the six fan-out rows whose reconstructions are on main.
        step_subsystems = std::make_unique<GameStepSubsystemsHost>(log);
        fixed_step->attach_subsystems(step_subsystems.get());
    }

    ~Impl() { release_units(); }

    void release_units() noexcept {
        // HUD persists across frames; Lua persists until after this frame dies.
        // Release every outward borrow before destroying the canonical slots.
        if (hud != nullptr && units != nullptr) hud->detach_world_2k();
        lua.attach_script_orders(nullptr);
        if (step_subsystems != nullptr) step_subsystems->attach_units(nullptr);
        if (units != nullptr) units->set_ship_ai(nullptr);
        ship_ai.reset();
        script_orders.reset();
        world_host.reset();
        units.reset();
    }

    GameHostLog& log;
    GameVfsHost& vfs;
    GameMissionLuaHost& lua;
    GameObserverRuntime* observer_runtime{nullptr};
    bsp::SessionParticipantPools& participants; // persistent GameMissionHost owner
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
    bsp::InputBindingDeviceGroups device_groups{};
    std::vector<bsp::HintCooldown> hint_cooldowns{};
    bool main_menu_manager_released{false};   // 00e198ac
    bool cinematic_hidden{false};             // game+634h
    bool cinematic_allow_simulation{false};   // game+635h
    int script_reentry_depth{0};              // game+644h
    std::int32_t published_mission_id{0};     // [00f8a2fc]+48h
    float world_clock{0.0f};                  // 00f876a4
    bool device_edge_injected{false};         // the executable's own injection
    // Packet cc_mission_tick's reconstruction of the four-call opener of every
    // simulated frame, and the dynamics list behind its fourth call.
    bsp::FixedStepClock fixed_clock{};        // 00f876a1..00f876b8
    bsp::GameDynamicsState dynamics{};        // game+30h
    bool unit_lists_built{false};             // game+193Ch
    unsigned long long fixed_steps{0};
    // Milestone 2g: the fixed step's own body (00875cc0, 00875e0c, 00875670,
    // 00875fd1) and the mission result object at game+7188h with the path out
    // of state 0Dh. Both are constructed in the body above.
    std::unique_ptr<GameFixedStepHost> fixed_step;
    std::unique_ptr<GameMissionResultHost> result;
    // Milestone 2m: the six fan-out rows' own reconstructions.
    std::unique_ptr<GameStepSubsystemsHost> step_subsystems;
    // Milestone 2h: 004d4df0, the scene contents pass. Built on the load walk's
    // own row, because the "2_" file block it opens belongs to that step.
    std::unique_ptr<GameSceneContentsHost> scene_contents;
    // Milestone 2i: the created units and the world chain the walk reads. Both
    // are built on the load's own load_scene_contents row, because that is the
    // step that creates the instances.
    std::unique_ptr<GameUnitsHost> units;
    std::unique_ptr<GameWorldHost> world_host;
    // Milestone 2m: the host the eight reconstructed binding bodies run over.
    // Built with the units, because every one of the eight addresses an entity.
    std::unique_ptr<GameScriptOrdersHost> script_orders;
    // Milestone 2n: one ship AI controller per created unit, and the weapon
    // director's automatic target selector beside it. Built with the units,
    // because the order slot the controller publishes lives on the unit.
    std::unique_ptr<GameShipAiHost> ship_ai;
    long order_frame{-1};             // --order-frame N
    // Milestone 2o, --ai-drive <name>=<throttle>,<rudder>, engaged on the same
    // frame as the player order.
    std::string ai_drive_unit;
    float ai_drive_throttle{0.0f};
    float ai_drive_rudder{0.0f};
    float order_throttle{0.0f};
    float order_rudder{0.0f};
    float order_speed{0.0f};
    bool order_speed_set{false};
    bool order_issued{false};
    // Milestone 2l: --order <command>[:<entity>], the same frame, issued
    // through the recovered command path instead of the order ring.
    std::string order_command;
    std::string order_command_target;
    std::string order_command_unit;   // milestone 2n, --order-unit <name>
    float mission_frame_seconds{0.0f};  // --mission-frame-seconds S
    // Milestone 2j, --trajectory-csv <path>: one row per unit per fixed step.
    std::string trajectory_csv_path;
    bool trajectory_csv_tried{false};
    GameTrajectoryCsv trajectory_csv;
    // Milestone 2h: the in-mission HUD, owned by GameMenuHost because its
    // screens register into that object's copy of the registry at 00e18b60.
    GameHudHost* hud{nullptr};
    long complete_frame{-1};                  // --mission-complete-frame N
    bool complete_injected{false};
    unsigned long long exit_frames{0};        // frames run after state 0Dh ended
    bool exit_path_finished{false};
    bool drain_skip_reported{false};

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
// bsp::FixedStepHost and bsp::GameDynamicsFrameHost, behind calls 1 and 4 of
// the in-mission subsystem tick. Every list they walk is empty here.
// ---------------------------------------------------------------------------

class FixedStepBinding final : public bsp::FixedStepHost {
public:
    explicit FixedStepBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}
    void advance_step_countdown_008079b0(float) override {
        owner_.record("FixedStep::advance_countdown", 0x008079b0u);
    }
    void run_step_job_waves(std::uint8_t run_pass) override {
        // Milestone 2g: the three waves over the five 68h groups at 00f876c0.
        owner_.fixed_step->run_job_waves_00875cc0(run_pass);
    }
    void run_step_subsystems(float step, bool world_active) override {
        // Milestone 2g: the sixteen per-step calls and their world gate. The
        // tail hook's own gate is tested once per step alongside them, because
        // 00875fd1 is the last thing the driver does with the same clock.
        owner_.fixed_step->run_subsystems_00875e0c(step, world_active);
        // Milestone 2i: the ship motion virtual 00825f20. Its own caller is not
        // established - docs/SHIP_MOTION.md names three call sites (0085542f,
        // 00749b2c, 00644a38) and reads none - so the position in the frame is
        // the executable's decision. It is run here because 00825f20's order
        // ring is written for the fixed step's 0.05 s period
        // (kUnitStateMessageTickSeconds == kFixedSimulationStepFloat), which is
        // the step this call receives.
        if (owner_.units != nullptr) {
            // Milestone 2j, --trajectory-csv: the file is opened before the
            // first step so its step 0 block holds the pose the scene placed,
            // with no motion in it. A consumer that aligns on its first sample
            // would otherwise fold that step's displacement and rotation into
            // the alignment. Executable plumbing; it reads what the motion path
            // wrote and changes nothing.
            if (!owner_.trajectory_csv_path.empty() && !owner_.trajectory_csv_tried
                && owner_.units->count() > 0) {
                owner_.trajectory_csv_tried = true;
                if (owner_.trajectory_csv.open(owner_.trajectory_csv_path, owner_.log)) {
                    owner_.trajectory_csv.append_step(0, 0.0f, owner_.units->units());
                }
            }
            owner_.units->motion_step_00825f20(step);
            if (owner_.trajectory_csv.is_open()) {
                const GameUnitsSummary& units = owner_.units->summary();
                owner_.trajectory_csv.append_step(units.motion_steps,
                    units.simulated_seconds, owner_.units->units());
            }
        }
    }
    void run_interpolation_wave_00875670(float leftover, std::uint8_t run_pass) override {
        owner_.fixed_step->run_interpolation_wave_00875670(leftover, run_pass);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

class DynamicsFrameBinding final : public bsp::GameDynamicsFrameHost {
public:
    explicit DynamicsFrameBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}
    void select_visible_model_group_00710bb0(std::uint32_t, std::size_t) override {
        owner_.record("Dynamics::select_model_group", 0x00710bb0u);
    }
    std::size_t model_group_count(std::uint32_t) override { return 0; }
    bsp::DynamicsTransform34 interpolated_body_transform_00c43ea0(
        std::uint32_t, float) override {
        owner_.record("Dynamics::interpolated_transform", 0x00c43ea0u);
        return {};
    }
    void refresh_node_world_matrix_00b6db70(std::uint32_t) override {
        owner_.record("Dynamics::refresh_node_matrix", 0x00b6db70u);
    }
    void node_world_matrix_column_y(std::uint32_t, float out[4]) override {
        for (int i = 0; i < 4; ++i) out[i] = 0.0f;
    }
    bsp::DynamicsVec3 body_linear_velocity_00c31f40(std::uint32_t) override {
        owner_.record("Dynamics::body_velocity", 0x00c31f40u);
        return {};
    }
    bsp::DynamicsSplashSettings splash_settings_00424c40() override {
        owner_.record("Dynamics::splash_settings", 0x00424c40u);
        return {};
    }
    void spawn_water_entry_effect(int, const bsp::DynamicsVec3&) override {
        owner_.record("Dynamics::spawn_water_entry_effect", 0x008685e0u);
    }
    void set_node_world_transform_vtable34(
        std::uint32_t, const bsp::DynamicsTransform34&) override {
        owner_.record("Dynamics::set_node_transform", 0x00448130u);
    }
    void unlink_and_release_node_00b6dfa0(std::uint32_t) override {
        owner_.record("Dynamics::release_node", 0x00b6dfa0u);
    }
    void queue_body_release_00c34f70(std::uint32_t) override {
        owner_.record("Dynamics::queue_body_release", 0x00c34f70u);
    }
    void set_node_visibility_00b6da70(std::uint32_t, float) override {
        owner_.record("Dynamics::set_node_visibility", 0x00b6da70u);
    }

private:
    GameMissionFrameHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::InMissionTickHost, step 9 of the frame (004e5133 -> 004c40a0)
// ---------------------------------------------------------------------------

class InMissionTickBinding final : public bsp::InMissionTickHost {
public:
    explicit InMissionTickBinding(GameMissionFrameHost::Impl& owner) : owner_(owner) {}

    void step_fixed_simulation_00875bb0(float scaled_delta) override {
        // The four-test gate at 00875bb1..00875c02.
        // Read the selected identity from the same pool as the entry and AI.
        // The existing +10h entry-ready projection remains outside that
        // pool's three-byte scope; it is not device ownership at +0Eh.
        bsp::FixedStepGate gate{};
        gate.game_present = true;
        bsp::ParticipantRecordId local_record;
        gate.local_player_slot_present = owner_.participants.active_record(0, local_record);
        gate.slot_ready_10h = static_cast<std::int16_t>(bsp::kLocalSlotReadyValue);
        gate.view_mode_1fe4 = owner_.scene_state.session_mode;
        gate.session_count_9c = 0;
        FixedStepBinding host(owner_);
        // The world gate of the fan-out, 00875e69..00875e7f: [[game+19CCh]+4ACh].
        // construct_world 004de610 is a load record here, so there is no world
        // object and the byte is zero; rows 9..13 are skipped every step.
        const bool world_active = false;
        const std::uint32_t steps = bsp::run_fixed_step_driver_00875bb0(
            owner_.fixed_clock, gate, scaled_delta, world_active, 0, host);
        owner_.fixed_steps += steps;
        // 00875fd1..00875ff7, the tail hook's four-test gate. The driver
        // reconstruction stops after the interpolation wave and carries no host
        // method for it, so the executable runs it here, at the driver's own
        // tail position, and only when the driver's gate let the body run.
        if (bsp::fixed_step_gate_open(gate)) {
            owner_.fixed_step->run_tail_00875fd1();
        }
        owner_.done("InMissionTick::fixed_step_driver", 0x00875bb0u);
    }

    void build_local_player_unit_lists_004c3cb0() override {
        // Milestone 2i: milestone 2f ran the guard and recorded the body,
        // because the eight list heads are filled from a registry no unit had
        // reached. The world host now runs the whole routine: the guard, the
        // clear, the three walks and the merge tail.
        if (owner_.world_host != nullptr) {
            owner_.world_host->build_local_player_unit_lists_004c3cb0();
            owner_.unit_lists_built = true;
            return;
        }
        bsp::UnitListsGate gate{};
        gate.already_built = owner_.unit_lists_built;
        gate.slot_index = 0;
        const bool ran = bsp::local_player_unit_lists_run_004c3cb0(gate);
        owner_.unit_lists_built = gate.already_built;
        owner_.done("InMissionTick::unit_lists_guard", 0x004c3cb0u);
        // The guard is the whole of this routine that is a rule; the eight list
        // heads it fills are the world's.
        if (ran) owner_.record("InMissionTick::build_unit_lists", 0x004bfdf0u);
    }

    void update_world_entities_00904bf0(float scaled_delta) override {
        // Milestone 2i: 004c40ce, the walk over [[world+4]] with the gate at
        // entity+5Ch and the sibling link at entity+38h, then 00904600.
        if (owner_.world_host != nullptr) {
            owner_.world_host->run_world_entity_update_00904bf0(scaled_delta);
            return;
        }
        owner_.record("InMissionTick::update_world_entities", 0x00904bf0u);
    }

    void update_game_dynamics_00447b80(float scaled_delta) override {
        DynamicsFrameBinding host(owner_);
        const float alpha = bsp::dynamics_interpolation_alpha(
            owner_.fixed_clock.interpolation_left, bsp::kFixedSimulationStepFloat);
        static_cast<void>(bsp::tick_game_dynamics_frame_00447b80(
            owner_.dynamics, scaled_delta, alpha, host));
        owner_.done("InMissionTick::update_game_dynamics", 0x00447b80u);
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
            bsp::ParticipantRecordId record;
            bsp::ParticipantRecordBytes bytes;
            if (!owner_.participants.active_record(i, record)
                || !owner_.participants.read_record_bytes(record, bytes)
                || !bytes.claimed_08.available || !bytes.ai_held_09.available) {
                throw std::logic_error("Frame participant +8/+9 view is unavailable");
            }
            slots[i] = {true, bytes.claimed_08.value != 0, bytes.ai_held_09.value != 0};
        }
    }
    float mission_time_filter_007713a0(float delta) override { return delta; }
    bool max_step_clamp_disabled() override { return false; }
    bool input_action_held(int) override { return false; }
    bool input_action_pressed(int) override { return false; }

    void request_02_004d8000() override { owner_.record("Drain::request_02", 0x004d8000u); }
    void request_04_004e4000() override {
        // The front-end shell entry, which after a mission is the debrief front
        // end. 004e4000 itself is reconstructed and milestone 2c runs it from
        // the main-menu path over the menu host's screen registry; this host
        // owns none of that state, so the dispatch is recorded and the run ends
        // here rather than re-entering another owner's shell.
        owner_.record("Drain::request_04_front_end_shell", 0x004e4000u);
        owner_.result->note_front_end_entered();
    }
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
        // BSP_Game_EndScene. Milestone 2g owns the scoring records and the
        // mission progress object, so the handler runs for real.
        owner_.result->run_end_scene_004d7970(false);
    }
    bool request_10_teardown_004e458a(bsp::GameFrameControlState& state) override {
        return owner_.result->run_teardown_arm_004e458a(state);
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
        // game+7188h. Milestone 2f had nothing here, because 004dfe83 released
        // the previous result during the load; --mission-complete-frame builds
        // one through 004cd390, and this is what the poll then reads.
        owner_.done("MissionCompletion::mission_result", 0x004d7ea0u);
        return owner_.result->result_projection();
    }
    void world_final_tick(float) override {
        // [game+19CCh] vtable +Ch, which the image fills with 00904bf0.
        owner_.record("MissionCompletion::world_final_tick", 0x00904bf0u);
    }
    void world_post_tick() override {
        owner_.record("MissionCompletion::world_post_tick", 0x00903670u);
    }
    void show_mission_result_gui(float local_z) override {
        // 004d7f1d..004d7f42: the result's +8h float, 004f8a20 with it as the
        // movie widget's local Z, then 004f8970 registering 004f89d0.
        owner_.record("MissionCompletion::play_end_movie", 0x004d7f32u);
        owner_.record("MissionCompletion::set_movie_completion", 0x004d7f42u);
        owner_.log.notef("mission completion poll: the end movie would play at local "
            "Z %.1f (result+8h); the movie player is the front-end owner's",
            static_cast<double>(local_z));
    }
    void close_mission_result() override {
        owner_.record("MissionCompletion::close_result_gui", 0x004d7f72u);
        owner_.result->release_result_004cc510();
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
        // 004c9ca0 with 0 on this path. Milestone 2j runs the arm rather than
        // recording it: its front-end frame step is the committing 00518250 the
        // load's own row is not.
        owner_.entry.interface_request = loading ? "allocate (1)" : "tear down (0)";
        if (owner_.hud != nullptr) {
            owner_.hud->apply_in_game_interface_004c9ca0(loading);
        } else {
            owner_.record("MissionEntry::apply_in_game_interface", 0x004c9ca0u);
        }
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
        // The fixed four-call opener of every simulated frame, reconstructed by
        // packet cc_mission_tick and merged during this packet's turn.
        InMissionTickBinding tick(owner_);
        bsp::run_in_mission_subsystem_tick_004c40a0(owner_.frame_state.scaled_delta, tick);
        owner_.done("MissionFrame::update_in_mission_subsystems", 0x004c40a0u);
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
        // 00e198c4 and its +3Ch. Milestone 2h builds the manager, so the gate
        // answers from the object rather than from its absence.
        owner_.done("MissionFrame::in_game_interface_active", 0x004e524cu);
        return owner_.hud != nullptr && owner_.hud->manager_active();
    }
    void update_in_game_interface_0068c1f0() override {
        if (owner_.hud == nullptr) {
            owner_.record("MissionFrame::update_in_game_interface", 0x0068c1f0u);
            return;
        }
        // bsp::update_in_game_interface_0068c1f0, packet cc_hud_updates. Almost
        // everything it reads belongs to a screen class, to the camera at
        // game+19FCh or to the controlled unit at 00e188d8, none of which this
        // process owns, so the recovered control flow runs over records.
        owner_.hud->update_in_game_interface_0068c1f0();
        owner_.done("MissionFrame::update_in_game_interface", 0x0068c1f0u);
    }
    void update_interface_only_004c40f0() override {
        // Milestone 2h: the registry is no longer empty. The load's HUD step
        // registered the 42 in-mission screens into it, so this call services
        // the interface request Init pushed (006840f0 then 00684600 into the
        // manager's own 0068aca0) and then runs the pump 004f8830, which is
        // what enters the level-1 screens and commits their pages' visibility.
        ++owner_.frames.interface_updates;
        if (owner_.hud == nullptr) {
            owner_.record("MissionFrame::update_interface_only", 0x004c40f0u);
            return;
        }
        owner_.hud->apply_pending_interface_0068aca0();
        owner_.hud->update_interface_only_004c40f0(owner_.frame_state.raw_delta);
        owner_.done("MissionFrame::update_interface_only", 0x004c40f0u);
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
        if (enqueued) {
            owner_.frames.completion_requested = true;
            // 004d7f67 latches the drain suspension with the enqueue, and the
            // only two writers that clear it are 004c7ed0, reached from the
            // completion 004d7f42 just registered, and the state 11h arm at
            // 004e506b, which this state cannot reach while 0Fh is still
            // queued. No clip is in flight here, so the executable runs the
            // registered completion itself; without it the drain would never
            // dispatch the request the poll just made.
            owner_.result->run_movie_completion_004f89d0();
        }
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

// Packet cc_lua_find_entity. Not a native structure: this is the executable's
// carrier for the two facts a non-unit scene entity contributes, the `thisTable`
// slot and the authored pose `GetPosition` answers with.
struct GameSceneMarkerSeed {
    int id{0};
    std::string name;
    std::string class_name;
    int class_id{-1};
    bool findable{false};
    std::uint32_t attach{0};
    float position[3]{0.0f, 0.0f, 0.0f};
};

// Packet cc_lua_find_entity: the scene entities that are not units but that
// BSP_SEntity_InitAll still hands to entity virtual slot 39. The class filter is
// bsp::find_scene_class_lua_identity, whose rows carry the vtable and the
// function at its +9Ch as read from the shipped image; a class with no row is
// one whose creator installs its vtable through a factory the scan does not
// follow, and those are the unit classes the unit host already owns.
std::vector<GameSceneMarkerSeed> collect_scene_markers(
    const std::vector<GameSceneEntityRecord>& entities, int first_id) {
    std::vector<GameSceneMarkerSeed> markers;
    int next = first_id;
    for (const GameSceneEntityRecord& entity : entities) {
        if (entity.name.empty()) continue;
        // `created` is false for these: their creators are records, so the
        // instance was never built. `generated` is the gate's own answer that
        // the instantiate pass takes the entity, which is what puts it on the
        // pending list InitAll walks.
        if (!entity.generated || entity.created) continue;
        const bsp::SceneClassLuaIdentityRow* row
            = bsp::find_scene_class_lua_identity(entity.class_id);
        if (row == nullptr) continue;
        GameSceneMarkerSeed seed;
        seed.id = next++;
        seed.name = entity.name;
        seed.class_name = entity.class_name;
        seed.class_id = entity.class_id;
        seed.findable = row->findable_by_name;
        seed.attach = row->attach;
        // 008A7C3C reads the world matrix translation row at entity+0FCh.
        seed.position[0] = entity.world[12];
        seed.position[1] = entity.world[13];
        seed.position[2] = entity.world[14];
        markers.push_back(seed);
    }
    return markers;
}

void report_scene_markers(GameHostLog& log,
    const std::vector<GameSceneMarkerSeed>& markers) {
    log.notef("scene markers: %zu non-unit scene entit%s reach entity virtual slot 39 "
        "through 00925f20's first pass at 0092604e, so each carries a `thisTable` slot "
        "and, when its world bucket is one of the fourteen 0088b1b0 walks, answers "
        "FindEntity. The creators themselves (004e99b0 for NavPoint) stay records: what "
        "the executable supplies is the slot and the authored pose, not the instance",
        markers.size(), markers.size() == 1 ? "y" : "ies");
    for (const GameSceneMarkerSeed& marker : markers) {
        log.notef("  scene marker %-20s class=%-16s id=%d findable=%d attach=%08lx "
            "pos=(%.1f,%.1f,%.1f)", marker.name.c_str(), marker.class_name.c_str(),
            marker.id, marker.findable ? 1 : 0,
            static_cast<unsigned long>(marker.attach),
            static_cast<double>(marker.position[0]),
            static_cast<double>(marker.position[1]),
            static_cast<double>(marker.position[2]));
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// GameMissionFrameHost
// ---------------------------------------------------------------------------

GameMissionFrameHost::GameMissionFrameHost(GameHostLog& log, GameVfsHost& vfs,
    GameMissionLuaHost& lua, bsp::SessionParticipantPools& participants,
    GameFrameProfiler* profiler, std::string language,
    GameHudHost* hud)
    : impl_(std::make_unique<Impl>(log, vfs, lua, participants, profiler, std::move(language))) {
    impl_->hud = hud;
}

GameMissionFrameHost::~GameMissionFrameHost() = default;

void GameMissionFrameHost::bind_observer_runtime(GameObserverRuntime& runtime) {
    if (!runtime.has_live_dispatch_owner())
        throw std::logic_error("mission frame observer binding requires a live dispatch owner");
    if (impl_->observer_runtime != nullptr && impl_->observer_runtime != &runtime)
        throw std::logic_error("mission frame observer runtime cannot change while the host lives");
    if (impl_->units != nullptr) impl_->units->bind_observer_runtime(runtime);
    impl_->observer_runtime = &runtime;
}

const GameMissionLoadRunSummary& GameMissionFrameHost::load_summary() const noexcept {
    return impl_->load;
}
const GameMissionEntrySummary& GameMissionFrameHost::entry_summary() const noexcept {
    return impl_->entry;
}
const GameMissionFrameRunSummary& GameMissionFrameHost::frame_summary() const noexcept {
    return impl_->frames;
}

const GameSceneContentsSummary* GameMissionFrameHost::scene_contents_summary()
    const noexcept {
    return impl_->scene_contents ? &impl_->scene_contents->summary() : nullptr;
}

void GameMissionFrameHost::run_scene_load_004dfb70(const std::string& scene_path,
    const std::string& script_name, const std::string& locale_tables,
    std::int32_t mission_id, const bsp::SceneRecord* scene_record,
    const std::int32_t* participant_count) {
    Impl& host = *impl_;
    if (scene_record && !participant_count) {
        throw std::logic_error("Scene participant count is unavailable from its header properties");
    }
    const std::int32_t scene_slot_count = participant_count ? *participant_count : 0;
    if (scene_record && scene_slot_count > static_cast<std::int32_t>(bsp::kSceneSlotRecordCount)) {
        throw std::invalid_argument("Scene participant count exceeds the native eight-record pool");
    }
    host.load.ran = true;
    host.load.short_name = bsp::derive_scene_short_name(scene_path);
    host.load.script_name = script_name;
    host.load.mission_id = mission_id;

    host.scene_state.state = bsp::kMissionSceneLoadRequest;
    host.scene_state.scene_path = scene_path;
    host.scene_state.script_name = script_name;
    host.scene_state.locale_table_list = locale_tables;
    host.scene_state.mission_id = mission_id;
    host.scene_state.have_scene_record = scene_record != nullptr;
    host.load.participant_scene_present = scene_record != nullptr;
    host.load.participant_count_available = participant_count != nullptr;
    host.load.participant_scene_slots = scene_slot_count;

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
        // `assign_party_player_slots` stays here on the evidence of
        // docs/MISSION_LOAD_HOSTS.md: 004e044d gates 004c3840 on
        // game+1FE4h == 1 and this session's field is 0, so the party rule never
        // runs on a local load, whatever its reconstruction can do.
        if (method == "detach_network_menu_manager"
            || method == "assign_party_player_slots" || method == "activate_slot"
            || method == "dispatch_session_ready_event") {
            ++host.load.skipped_arms;
            continue;
        }

        // Milestone 2m. The three rows packet cc2_mission_load_hosts
        // reconstructed and renamed. Both spellings are accepted because the
        // interface's own names are being corrected on main at the same time.
        if (method == "reset_network_slots" || method == "erase_native_string_set") {
            bool single_player_reset = false;
            run_load_session_slot_reset_004dfc13(host.log,
                host.scene_state.session_mode, single_player_reset);
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "reset_objective_list" || method == "rebuild_avoid_zone_table"
            || method == "reset_avoid_zone_state") {
            run_load_avoid_zone_state_004e0754(host.log, [&host]() {
                if (!host.scene_contents || !host.ship_ai)
                    throw std::logic_error("Avoid-zone load precedes scene/AI creation");
                host.ship_ai->load_avoid_zone_geometry(*host.scene_contents, host.lua,
                    host.scene_state.script_slot,
                    host.scene_state.script_slot_forced ? 1u : 0u,
                    host.scene_state.session_mode);
            });
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "rebuild_scripted_name_list"
            || method == "record_script_function_baseline") {
            std::vector<std::string> names;
            const std::size_t inserted = run_load_scripted_name_baseline_004d30f0(
                host.log, host.lua, names);
            host.load.scripted_names = inserted;
            host.done(label, step.address);
            ++host.load.concrete;
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
            // 004DFD18 -> 004DFD57 -> 004DFD5C: reset both pools, claim the
            // first player record, then publish current[0]. Neither service
            // writes +0E. The later 004E0851 store is skipped in mode 0.
            bsp::ParticipantRecordId claimed;
            const auto result = host.participants.reset_and_claim_local_flags(
                host.scene_state.have_scene_record,
                scene_slot_count, claimed);
            if (result != bsp::ParticipantClaimResult::Claimed) {
                throw std::logic_error("Local participant reset/claim is unavailable");
            }
            host.load.participant_local_claimed = true;
            host.load.slots_reset = bsp::kSceneSlotRecordCount;
            host.log.notef("participant pools: scene_present=%d scene_slots=%d "
                "current[0]=player[%zu] remaining=mission[1..7] device_bytes=retained "
                "mode=%d", host.scene_state.have_scene_record ? 1 : 0,
                scene_slot_count, claimed.index, host.scene_state.session_mode);
            host.done(label, step.address);
            ++host.load.concrete;
            continue;
        }
        if (method == "release_main_menu_manager") {
            host.main_menu_manager_released = true;
            // Milestone 2h: 004dfd96 calls the manager's vtable slot 0, which is
            // the deleting destructor 00687300; the destroy body 00686c90 runs
            // BSP_FrontEndManager_Deactivate 00683aa0, and that routine's tail
            // publishes the empty level-4 sets. Milestone 2f nulled the global
            // and stopped there, which is why the front-end pages stayed on
            // screen through the whole mission.
            if (host.hud != nullptr) host.hud->release_main_menu_manager_00686c90();
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
        if (method == "create_hud_manager") {
            // 004e0452 / 004e046c / 0068cc70. The manager and its Init are not
            // reconstructed and stay records; what the HUD host runs is the part
            // of Init that goes through recovered code, the 42 screen
            // registrations, their pages and the INTF_SCENE3D request.
            if (host.hud != nullptr) {
                host.hud->build_manager_0068a990();
                ++host.load.concrete;
            } else {
                host.record(label, step.address);
                ++host.load.records;
            }
            continue;
        }
        if (method == "load_scene_contents") {
            host.release_units();
            // 004d4df0 at 004e03e5. Milestone 2f recorded this step; it now runs
            // the reconstruction of the routine, which runs both scene-file
            // passes over the selected mission's .scn. Every host method inside
            // it that needs the renderer, the scene graph, the world, the
            // terrain or sound keeps the unimplemented policy with its own
            // address, so this step being concrete is not a claim about those.
            if (!host.scene_contents) {
                host.scene_contents = std::make_unique<GameSceneContentsHost>(host.log,
                    host.vfs);
            }
            // Milestone 2m: 0095c640's three reads of the live VehicleClass
            // table, which the recovered global-script step already loaded.
            host.scene_contents->attach_lua(&host.lua);
            // game+614h and game+61Ch are the two fields 004bca50 reads as the
            // raw game mode and its forced flag; MissionSceneLoadState names the
            // same pair script_slot / script_slot_forced, because 004e087b picks
            // the mission script slot out of the same field.
            host.scene_contents->run_load_scene_contents_004d4df0(scene_path,
                host.scene_state.scene_override, host.scene_state.script_slot,
                host.scene_state.script_slot_forced,
                host.scene_state.session_mode != 0);
            host.done(label, step.address);
            ++host.load.concrete;
            // Milestone 2i: the instances the instantiate pass created become
            // one state object each, and the chain 009037f0 allocates is built
            // over them so the world walk has something to walk. Every unit's
            // authored `Command` token is then queued, and the first created
            // instance becomes the controlled unit through 004c0890.
            host.units = std::make_unique<GameUnitsHost>(host.log, host.lua);
            if (host.observer_runtime != nullptr)
                host.units->bind_observer_runtime(*host.observer_runtime);
            // Milestone 2j: the gameplay settings singleton's rudder curve block
            // is filled before any unit exists, because 0083b5e0 runs from the
            // settings object's own construction and every ship reads the one
            // block. This process has one Lua state, so it runs the script into
            // the mission machine and says so.
            host.units->load_gameplay_settings_0083b5e0();
            host.units->create_units(host.scene_contents->entities());
            host.world_host = std::make_unique<GameWorldHost>(host.log, *host.units);
            host.world_host->build_entity_chains_009037f0();
            host.units->issue_authored_commands();
            if (host.units->count() > 0) host.units->set_controlled_unit_004c0890(0);
            // Milestone 2l: one `thisTable` slot per created instance, which is
            // what 00928a00 builds for an entity and what the entity tail at
            // 0089903c pushes. Without it the mission's own script resolves
            // every FindEntity to nil and its order loops run over empty tables.
            std::vector<GameSceneMarkerSeed> markers;
            {
                std::vector<GameMissionLuaHost::SceneEntity> entities;
                entities.reserve(host.units->count());
                for (std::size_t unit = 0; unit < host.units->count(); ++unit) {
                    const GameUnitRow* row = host.units->unit_row(unit);
                    if (row == nullptr || row->name.empty()) continue;
                    entities.push_back(GameMissionLuaHost::SceneEntity{row->name,
                        static_cast<int>(unit) + 1, row->type_id, true});
                }
                // Packet cc_lua_find_entity: the pending-entity pass 00925F20
                // calls entity virtual slot 39 on every node at 0092604E, not
                // only on units, so a scene entity of any class whose vtable
                // carries an attach there has a `thisTable` slot too. Without
                // these rows `FindEntity("EscapePoint")` resolved the name to
                // nothing and usn_2_java's phase-2 distance test raised.
                markers = collect_scene_markers(host.scene_contents->entities(),
                    static_cast<int>(host.units->count()) + 1);
                for (const GameSceneMarkerSeed& marker : markers) {
                    entities.push_back(GameMissionLuaHost::SceneEntity{marker.name,
                        marker.id, -1, marker.findable});
                }
                host.lua.attach_scene_entities_00928a00(entities);
                if (!markers.empty()) report_scene_markers(host.log, markers);
            }
            // Milestone 2m: with the slots built, the eight binding bodies
            // src/lua_binding_navigator.cpp reconstructs can run over the
            // created instances instead of being counted as records.
            host.script_orders = std::make_unique<GameScriptOrdersHost>(host.log,
                *host.units);
            for (const GameSceneMarkerSeed& marker : markers) {
                host.script_orders->register_scene_marker(marker.id, marker.name,
                    marker.position);
            }
            host.lua.attach_script_orders(host.script_orders.get());
            // Milestone 2n: the ship AI controller family at 00d21598 over the
            // same created units. 009f50e0's three gates read unit+5Ch, +5Dh and
            // +61h, and 009f3dd0 reads the director 0071be40 answers for, so the
            // host is built after the authored commands were issued.
            host.ship_ai = std::make_unique<GameShipAiHost>(host.log, *host.units);
            host.ship_ai->bind_session_participants(host.participants);
            host.ship_ai->register_units(host.lua, host.scene_state.session_mode);
            host.units->set_ship_ai(host.ship_ai.get());
            // Milestone 2m: row 16 of the fan-out walks the entity chain this
            // step created, so the subsystem host learns about it here.
            if (host.step_subsystems != nullptr) {
                host.step_subsystems->attach_units(host.units.get());
            }
            // Milestone 2k: the two HUD screens that show the world read the same
            // created units. Once 004c0890 has bound one, the interface request
            // can carry it, which is what raises the markers screen and keeps the
            // minimap: 0068aca0's 20h arm is a unit-kind classifier with a
            // payload and publishes 25h INTF_CAPTAIN's level-1 set for a ship.
            if (host.hud != nullptr) {
                host.hud->attach_world_2k(*host.units, host.lua);
                host.hud->request_scene_interface_for_unit_004cc460();
            }
            continue;
        }
        if (method == "apply_in_game_interface") {
            // 004c9ca0(1) at 004e1873, the load's own row: the arm that puts the
            // loading element up. It exists here so the entry's 004c9ca0(0) has
            // the element the native teardown reads without a null check.
            if (host.hud != nullptr) {
                host.hud->apply_in_game_interface_004c9ca0(true);
                ++host.load.concrete;
            } else {
                host.record(label, step.address);
                ++host.load.records;
            }
            continue;
        }
        if (method == "select_front_end_layout") {
            // Milestone 2i: 004c1ac0(3,0) then 00518250(3,0). Milestone 2h
            // reported the front-end frame as still on screen during the
            // mission and left it at milestone 2b's caveat; this row is what
            // the game does about it, and the answer is that it does not
            // release them, because the call does not commit.
            if (host.hud != nullptr) {
                host.hud->select_front_end_layout_00518250();
                ++host.load.concrete;
            } else {
                host.record(label, step.address);
                ++host.load.records;
            }
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
        if (method == "sync_lobby_settings_from_lua") {
            host.lua.publish_lobby_settings_005e2f00();
            // 004e0305 is the next thing the same pass does: it creates the
            // `thisTable` self table and clears `recon`
            // (docs/MISSION_LUA_SELF_TABLE.md). The inventory has no row of its
            // own for it, so it runs here, where the listing runs it.
            host.lua.create_self_table_004e0305();
            // Packet cc_lua_find_entity: 004E0305 nils `recon`, and on this path
            // the native gets a table back through 00806B10's 006B8190 descent.
            // That routine runs off the recon slot lists, which this process does
            // not build, so the executable installs the shell 00803A40 builds and
            // says the publication is the record.
            host.lua.install_recon_tables_00803a40();
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
    bsp::MissionEntryPlayerSlot current_slots[bsp::kLocalPlayerSlotCount];
    host.entry.participant_view_available = host.participants.try_entry_slots(current_slots);
    host.entry.player_count_arm = bsp::mission_entry_uses_player_count_arm(
        inputs.local_view_mode, inputs.session_flag_29c);
    if (host.entry.participant_view_available) {
        inputs.unbound_player_slots = bsp::count_unbound_player_slots(current_slots);
        host.entry.unbound_player_slots = inputs.unbound_player_slots;
    } else if (host.entry.player_count_arm && inputs.local_view_mode == 1) {
        host.log.note("state 0Ch participant +9/+0E view is unavailable for the count arm");
        return false;
    }
    host.log.notef("mission participant entry: view_available=%d unbound=%zu "
        "player_count_arm=%d mode=%d", host.entry.participant_view_available ? 1 : 0,
        host.entry.unbound_player_slots, host.entry.player_count_arm ? 1 : 0,
        inputs.local_view_mode);

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
    // game+1FE4h and game+1EE3h, the two fields 004d7970 and the drain's 10h
    // arm read on the way out of the mission.
    host.result->set_session_mode(host.scene_state.session_mode);
    host.result->set_session_networked(host.entry_state.one_shots.session_was_networked);
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

    // Milestone 2l: the mission's own orders. usn_2_java.lua's `luaStageInit`
    // calls CreateScript("luaInit"), and `luaInit` is where the mission issues
    // every order it issues: NavigatorMoveToRange, NavigatorAttackMove,
    // JoinFormation and the rest of the Navigator family. The script manager
    // that would call it is three records, so the executable calls it here,
    // once, on the frame the mission enters state 0Dh.
    // Packet cc_lua_binding_audit: 00898750 now runs its own body, and that body
    // calls the named global itself (0089898B / 009290A0), on the frame the mission
    // hands the name over. The stand-in above would then run `luaInit` a second
    // time, so it runs only while the binding is still a record.
    if (host.script_orders == nullptr || host.script_orders->timers().scripts_created == 0) {
        host.lua.run_created_scripts();
    } else {
        host.log.notef("script objects: CreateScript 00898750 ran its own body %zu time(s) "
            "and called each named global from inside it, so milestone 2l's one-shot "
            "stand-in is not run",
            host.script_orders->timers().scripts_created);
    }
    return host.entry.entered;
}

void GameMissionFrameHost::set_mission_complete_frame(long frame) noexcept {
    impl_->complete_frame = frame;
}

void GameMissionFrameHost::set_mission_key(std::string key) {
    // game+2198h, the key 009205e0 commits the scoring record under. The
    // mission start writes it (docs/MISSION_TREE_BRIEFING_SCREENS.md).
    impl_->result->set_mission_key(std::move(key));
}

void GameMissionFrameHost::set_player_order(long frame, float throttle,
    float rudder) noexcept {
    impl_->order_frame = frame;
    impl_->order_throttle = throttle;
    impl_->order_rudder = rudder;
}

void GameMissionFrameHost::set_player_commanded_speed(float speed) noexcept {
    impl_->order_speed = speed;
    impl_->order_speed_set = true;
}

void GameMissionFrameHost::set_player_command(std::string token, std::string target,
    std::string unit) {
    impl_->order_command = std::move(token);
    impl_->order_command_target = std::move(target);
    impl_->order_command_unit = std::move(unit);
}

void GameMissionFrameHost::set_ai_drive(std::string unit, float throttle, float rudder) {
    impl_->ai_drive_unit = std::move(unit);
    impl_->ai_drive_throttle = throttle;
    impl_->ai_drive_rudder = rudder;
}

void GameMissionFrameHost::set_trajectory_csv(std::string path) {
    impl_->trajectory_csv_path = std::move(path);
}

void GameMissionFrameHost::set_mission_frame_seconds(float seconds) noexcept {
    impl_->mission_frame_seconds = seconds;
}

bool GameMissionFrameHost::in_mission_phase() const noexcept {
    return impl_->control.state
        == static_cast<std::uint32_t>(bsp::GameStateId::kInMission);
}

bool GameMissionFrameHost::exit_path_finished() const noexcept {
    return impl_->exit_path_finished;
}

bool GameMissionFrameHost::run_mission_frame_004e4a40(float raw_delta_in) {
    Impl& host = *impl_;
    if (!host.entry.entered) return false;

    // Milestone 2i, --mission-frame-seconds S. Executable plumbing, the same
    // kind as --frames: it replaces the wall-clock frame interval so a headless
    // run accumulates simulated time deterministically and the fixed-step
    // driver's own clock at 00f876a4 does not depend on the frame rate.
    const float raw_delta
        = host.mission_frame_seconds > 0.0f ? host.mission_frame_seconds : raw_delta_in;

    host.world_clock += raw_delta;
    host.frame_state.raw_delta = raw_delta;
    host.frame_state.scaled_delta = raw_delta;  // no time dilation in this process
    host.frame_state.global_time = host.world_clock;
    host.world.game.elapsed = host.world_clock;
    host.result->set_mission_clock(host.world_clock);

    // 004e4d02, the first thing OnMove does after the console pre-tick: the
    // request drain, skipped while game+5ECh is set. Milestone 2f ran only the
    // in-mission branch and never drained, because nothing could enqueue.
    {
        FrameControlBinding control(host);
        if (host.control.drain_suspended) {
            if (!host.drain_skip_reported) {
                host.drain_skip_reported = true;
                host.log.notef("the request drain is suspended at game+5ECh, so 004e4430 "
                    "does not run this frame");
            }
        } else {
            bsp::drain_state_requests_004e4430(host.control, control);
        }
        // 004e504b, the state 11h arm of the frame bookkeeping: it lowers the
        // suspension the teardown arm raised and asks for the front end.
        const std::size_t queued = host.control.requests.count;
        const bool waiting = host.control.state
            == static_cast<std::uint32_t>(bsp::GameStateId::kMissionEndWait);
        bsp::update_mission_end_wait(host.control, control);
        if (waiting) {
            const bool enqueued = host.control.requests.count > queued;
            host.result->note_mission_end_wait(enqueued);
            host.log.notef("mission end wait: state 11h, game+7184h clear, so 004e506b "
                "cleared the drain suspension and %s request 04h",
                enqueued ? "enqueued" : "did not enqueue");
        }
    }

    // The mission left state 0Dh: the in-mission branch of 004e4a40 no longer
    // runs, and what is left is the exit path's own frames.
    if (host.control.state
        != static_cast<std::uint32_t>(bsp::GameStateId::kInMission)) {
        ++host.exit_frames;
        host.frames.exit_frames = host.exit_frames;
        host.log.notef("  mission exit frame %llu: game state 0x%02X, requests queued=%zu",
            host.exit_frames, host.control.state, host.control.requests.count);
        const bool front_end = host.control.state
            == static_cast<std::uint32_t>(bsp::GameStateId::kFrontEnd);
        if (front_end || host.exit_frames >= kMaxExitFrames) {
            host.exit_path_finished = true;
            host.frames.exit_completed = front_end;
            return false;
        }
        return true;
    }

    // --order-frame N with --order throttle=<f>,rudder=<f>: one player order to
    // the controlled unit, through the same 00816a40 the authored command
    // takes. A running game issues it from the HUD's own order path; no input
    // backend exists here, so the executable issues it and says so.
    if (host.order_frame >= 0 && !host.order_issued && host.units != nullptr
        && host.frames.frames + 1 >= static_cast<unsigned long long>(host.order_frame)) {
        host.order_issued = true;
        host.frames.player_order_issued = true;
        if (!host.order_command.empty()) {
            // Milestone 2l: the command form. 0046aab0 resolves the name
            // against the 26-row registry and the whole hop chain runs, which
            // is the path the authored scene command takes and not the ring.
            host.units->issue_player_command(host.order_command,
                host.order_command_target, host.order_command_unit);
        } else {
            host.units->issue_player_order(host.order_throttle, host.order_rudder);
        }
        // Milestone 2o: the diagnostic stand-in for the state step, engaged
        // after the order that chose the state, so the two take effect on the
        // same in-mission frame.
        if (!host.ai_drive_unit.empty()) {
            host.units->enable_ai_drive(host.ai_drive_unit, host.ai_drive_throttle,
                host.ai_drive_rudder);
        }
        if (host.order_speed_set && host.units->controlled_bound()) {
            // Milestone 2m: the store luaMW_SetShipSpeed 00890d30 makes. This
            // mission's script never calls it, and neither does
            // luaMW_NavigatorMoveOnPath 008a3600, so the switch is the only way
            // the executable can put a commanded speed on a unit.
            const std::size_t unit = host.units->controlled_index();
            host.units->store_commanded_speed_00890e6f(unit, host.order_speed);
            const bsp::CruiseSpeedSetting pair = host.units->commanded_speed(unit);
            host.log.notef("commanded speed issued to the controlled unit: "
                "navigatorParams+24h = %.3f m/s, +28h = %.3f (the mission clock). "
                "00836e59 now answers active, so the director's idle tail re-issues "
                "`cruise` instead of `stop`, and 009e12bd turns the speed into a "
                "throttle by dividing it by 0080fc30's reference speed",
                static_cast<double>(pair.speed), static_cast<double>(pair.enable));
        }
    }

    // --mission-complete-frame N: the script's own PlayBinkMovie(name, true),
    // injected on the frame the switch names because no script on this
    // installation reaches it in a headless run.
    if (host.complete_frame > 0 && !host.complete_injected
        && host.frames.frames + 1 >= static_cast<unsigned long long>(host.complete_frame)) {
        host.complete_injected = true;
        host.frames.complete_injected = true;
        host.log.notef("mission complete injected on frame %llu: the executable calls the "
            "path a script's PlayBinkMovie(name, true) takes", host.frames.frames + 1);
        host.result->play_bink_movie_0089a480(host.load.short_name, true);
    }

    MissionFrameBinding binding(host);
    const bsp::MissionFrameResult result
        = bsp::run_mission_frame(host.frame_state, binding);
    // Packet cc_lua_binding_audit: row 8 of the fixed-step fan-out, 00875E64 /
    // 00929460, over the script entities CreateScript made. The fan-out's own row
    // runs against an empty list because nothing else registers a think; this pass
    // is the same reconstructed rule with the mission's script entities in it, and
    // it is what re-enters `luaTimetable` when a luaDelay expires.
    if (result.simulated && host.script_orders != nullptr) {
        host.script_orders->run_script_timers(raw_delta);
    }
    ++host.frames.frames;
    if (result.simulated) ++host.frames.simulated;
    if (result.paused) ++host.frames.paused;
    host.frames.menu_drain_iterations
        += static_cast<unsigned long long>(result.menu_drain_iterations);
    host.frames.script_calls = host.lua.summary().native_calls;
    if (result.mission_completion_requested) host.frames.completion_requested = true;

    // Milestone 2i: what the world walk and the units did this frame.
    if (host.world_host != nullptr) {
        const GameWorldSummary& world_summary = host.world_host->summary();
        host.frames.world_walks = world_summary.walks;
        host.frames.entities_walked = world_summary.entities_walked;
        host.frames.entities_updated = world_summary.entities_updated;
        host.frames.merged_unit_list = world_summary.list_counts[7];
    }
    if (host.units != nullptr) {
        const GameUnitsSummary& units = host.units->summary();
        host.frames.units = units.units;
        host.frames.unit_motion_ticks = units.motion_ticks;
        host.frames.simulated_seconds = units.simulated_seconds;
        host.frames.controlled_distance = units.controlled_distance;
        host.frames.total_path_length = units.total_path_length;
        host.frames.controlled_unit = units.controlled_name;
    }

    host.log.notef("  mission frame %llu simulated=%d paused=%d units=%llu events=%llu "
        "script_calls=%llu erased=%d", host.frames.frames, result.simulated ? 1 : 0,
        result.paused ? 1 : 0, host.frames.units_ticked,
        host.frames.mission_events_applied, host.frames.script_calls,
        result.input_entries_erased);
    // The world walk's own per-frame line, and the controlled unit's trajectory
    // every ten in-mission frames.
    if (host.world_host != nullptr) host.world_host->log_frame(host.frames.frames);
    if (host.units != nullptr && host.frames.frames % 10 == 0) {
        host.units->log_controlled_trajectory(host.frames.frames);
    }
    // A frame that asked to leave state 0Dh is not the last frame any more: the
    // request it enqueued is dispatched by the next frame's drain, which is
    // where the exit path runs.
    return true;
}

void GameMissionFrameHost::report(long requested_frames) {
    Impl& host = *impl_;
    host.frames.requested = requested_frames;
    const GameMissionExitSummary& exit = host.result->summary();
    // Milestone 2f reported this as unreachable because nothing built a result
    // object. What makes the path reachable is the producer, not the poll:
    // 0089a390 calls 004d7970 itself when the script asks for the debrief.
    host.frames.exit_reachable = exit.end_scene_ran || host.frames.completion_requested;
    if (host.frames.exit_completed) {
        host.frames.exit_note = "the mission ended through 004d7970: request 10h, the "
            "teardown arm 004e458a, state 11h and request 04h";
    } else if (exit.end_scene_ran) {
        host.frames.exit_note = "004d7970 ran but the drain did not reach request 04h";
    } else if (host.frames.completion_requested) {
        host.frames.exit_note = "004d7ea0 enqueued the debrief request 0Fh";
    } else {
        host.frames.exit_note = "no mission-result object at game+7188h, so nothing "
            "asked to leave state 0Dh; --mission-complete-frame builds one";
    }
    host.fixed_step->report();
    if (host.step_subsystems != nullptr) host.step_subsystems->report();
    host.result->report();
    if (host.hud != nullptr) host.hud->report();
    if (host.world_host != nullptr) host.world_host->report();
    if (host.script_orders != nullptr) host.script_orders->report();
    // Packet cc_lua_find_entity: the shipped script's own `Mission` table, which
    // is how far the run carried the mission rather than how far the host ran.
    host.lua.report_mission_script_state();
    if (host.ship_ai != nullptr) host.ship_ai->report();
    if (host.units != nullptr) host.units->report();
    if (!host.trajectory_csv_path.empty()) {
        host.log.notef("summary mission trajectory csv=%s rows=%llu",
            host.trajectory_csv_path.c_str(), host.trajectory_csv.rows());
        host.trajectory_csv.close();
    }
    if (host.scene_contents) {
        // Milestone 2h. The scene contents pass created unit records, and the
        // frame's unit passes ticked none of them. That is not an empty scene:
        // every container those passes walk hangs off the world object
        // construct_world 004de610 would build, and that step is still a load
        // record, so the entity manager reference at game+21A0h is null and the
        // fixed step's world gate at 00875e69 is closed.
        const GameSceneContentsSummary& scene = host.scene_contents->summary();
        host.log.notef("summary mission scene contents mode=%d entities=%zu generated=%zu "
            "rejected=%zu created=%zu registration_bodies=%zu party_class_marks=%zu "
            "property_groups=%zu enum_tables=%zu", scene.effective_game_mode,
            scene.instantiate_entities, scene.generated, scene.rejected, scene.created,
            scene.registration_bodies, scene.party_class_marks, scene.property_groups,
            scene.enum_tables);
        // Milestone 2i supersedes milestone 2h's "every unit pass ticked 0 of
        // them" for the world walk and the motion virtual: the executable owns
        // the chain header 009037f0 allocates, so 00904bf0 and 00825f20 now run
        // over the created units. The passes that still tick nothing are the
        // ones that read the world object itself, which construct_world 004de610
        // does not build: the entity manager at game+21A0h is null and the fixed
        // step's world gate 00875e69 reads a world that does not exist.
        host.log.notef("summary mission unit passes: %zu unit record(s) exist; the world walk "
            "00904bf0 updated %llu of them and the motion virtual 00825f20 ticked %llu, "
            "while every pass that reads the world object itself still ticks none, because "
            "construct_world 004de610 is a load record", scene.created,
            host.frames.entities_updated, host.frames.unit_motion_ticks);
    }
    host.log.notef("summary mission fixed steps=%llu at %.3f s each (00875bb0's own clock "
        "at 00f876a4/00f876ac)", host.fixed_steps,
        static_cast<double>(bsp::kFixedSimulationStepFloat));
    host.log.notef("summary mission frames requested=%ld ran=%llu simulated=%llu paused=%llu "
        "units=%llu events=%llu script_calls=%llu interface_updates=%llu",
        host.frames.requested, host.frames.frames, host.frames.simulated,
        host.frames.paused, host.frames.units_ticked, host.frames.mission_events_applied,
        host.frames.script_calls, host.frames.interface_updates);
    host.log.notef("summary mission exit frames=%llu injected=%d completed=%d state=0x%02X",
        host.frames.exit_frames, host.frames.complete_injected ? 1 : 0,
        host.frames.exit_completed ? 1 : 0, host.control.state);
    host.log.notef("summary mission exit reachable=%d: %s",
        host.frames.exit_reachable ? 1 : 0, host.frames.exit_note.c_str());
    if (host.world_host != nullptr && host.units != nullptr) {
        host.log.notef("summary mission world units=%zu walked=%llu updated=%llu "
            "motion_ticks=%llu simulated=%.2f s controlled=%s moved=%.2f total_path=%.2f",
            host.frames.units, host.frames.entities_walked, host.frames.entities_updated,
            host.frames.unit_motion_ticks,
            static_cast<double>(host.frames.simulated_seconds),
            host.frames.controlled_unit.empty() ? "(none)"
                                                : host.frames.controlled_unit.c_str(),
            static_cast<double>(host.frames.controlled_distance),
            static_cast<double>(host.frames.total_path_length));
    }
}

}  // namespace bsp::game
