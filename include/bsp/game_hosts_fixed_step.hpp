#pragma once
// bsp_game.exe milestone 2g: the body of one fixed simulation step.
//
// Addresses: 00875e0c..00875edf (the sixteen per-step subsystem calls and their
// world gate 00875e69..00875e7f), 00875cc0..00875dfc (the three job waves over
// the five 68h groups at 00f876c0), 00875f32..00875fcc (the interpolation wave)
// and 00875fd1..00875ff7 (the tail hook). Milestone 2f ran the driver 00875bb0
// and recorded all four of those blocks as opaque host methods; this file runs
// the reconstructions bsp/fixed_step_fanout.hpp and bsp/fixed_step_job_waves.hpp
// put behind them.
//
// Nothing here is a reconstruction of native code. Every method is one call site
// of bsp::FixedStepFanoutHost, bsp::FixedStepJobWaveHost or
// bsp::GameDynamicsBuoyancyHost, satisfied either by a reconstruction already on
// main or by the explicit unimplemented policy in GameHostLog.
//
// Evidence: docs/FIXED_STEP_FANOUT.md, docs/FIXED_STEP_JOB_WAVES.md,
// docs/IN_MISSION_SUBSYSTEM_TICK.md, docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>

#include "bsp/fixed_step_fanout.hpp"
#include "bsp/fixed_step_job_waves.hpp"
#include "bsp/game_dynamics_list.hpp"

namespace bsp::game {

class GameHostLog;
class GameStepSubsystemsHost;

// What the run's fixed steps did.
struct GameFixedStepSummary {
    unsigned long long steps{0};             // fixed steps whose body ran
    unsigned long long fanout_calls{0};      // sites reached, gated ones excluded
    unsigned long long fanout_concrete{0};   // of those, run by a reconstruction
    unsigned long long fanout_records{0};    // of those, the unimplemented policy
    unsigned long long gate_closed_steps{0}; // steps whose world gate was closed
    unsigned long long gated_calls_skipped{0};
    unsigned long long wave_groups{0};       // wave x group visits inside the loop
    unsigned long long interpolation_groups{0};
    unsigned long long elements_seen{0};     // elements the group walk yielded
    unsigned long long jobs_queued{0};
    unsigned long long group_dispatches{0};
    unsigned long long tail_gate_open{0};
    unsigned long long tail_gate_closed{0};
    std::size_t buoyancy_records{0};         // records the last 004462d0 walked
};

// The sixteen per-step calls, the four waves and the tail hook, as one owner.
// Held for the whole run because the counters are per run and the dynamics list
// behind call 2 is the mission frame host's.
class GameFixedStepHost final : public bsp::FixedStepFanoutHost,
                                public bsp::FixedStepJobWaveHost,
                                private bsp::GameDynamicsBuoyancyHost {
public:
    GameFixedStepHost(GameHostLog& log, bsp::GameDynamicsState& dynamics);

    // Milestone 2m: the six rows whose reconstructions are on main. Attached on
    // the load's own load_scene_contents row, because row 16 walks the entity
    // chain that step creates.
    void attach_subsystems(GameStepSubsystemsHost* subsystems) noexcept;

    // 00875cc0..00875dfc, waves 1..3 over the five groups, inside the step loop.
    void run_job_waves_00875cc0(std::uint8_t run_pass);
    // 00875e0c..00875edf, the sixteen subsystem calls with the world gate.
    void run_subsystems_00875e0c(float step, bool world_active);
    // 00875f32..00875fcc, the interpolation wave, once per frame after the loop.
    void run_interpolation_wave_00875670(float leftover, std::uint8_t run_pass);
    // 00875fd1..00875ff7, the four-test tail gate and its hook.
    void run_tail_00875fd1();

    void report();
    const GameFixedStepSummary& summary() const noexcept { return summary_; }

private:
    // --- bsp::FixedStepFanoutHost, one method per callee -------------------
    void simulate_physics_world_00c5c540(float step) override;
    void apply_dynamics_buoyancy_004462d0(float step) override;
    void refresh_moved_spatial_nodes_0098bdb0(float step) override;
    void run_fixed_step_callbacks_00874de0(float step) override;
    void drain_deferred_entity_events_00926700() override;
    void drain_queued_lua_calls_00888230() override;
    void run_due_entity_think_00929460(float step) override;
    void pump_session_00778450(float step) override;
    void apply_pending_entity_creates_0077ec20() override;
    void flush_pending_tick_registrations_00874c90() override;
    void init_pending_entities_00925f20(bool flag) override;
    void flush_outbound_session_0076ffc0(float step, std::int32_t mode) override;
    void flush_pending_entity_queues_009273a0() override;
    void release_expired_world_objects_00903610() override;
    void run_tail_hook_00a317f0() override;

    // --- bsp::FixedStepJobWaveHost ----------------------------------------
    bool next_element(std::size_t group, std::size_t position,
        bsp::JobWaveElementView& view) override;
    void queue_job(bsp::JobWavePhase phase, std::size_t group,
        std::size_t position) override;
    void dispatch_group(bsp::JobWavePhase phase, std::size_t group,
        std::uint8_t run_pass) override;
    void run_wave1_job_008750a0(std::size_t group, std::size_t position,
        float step) override;
    void run_wave2_job_00875b90(std::size_t group, std::size_t position,
        float step) override;
    void run_wave3_job_00874fe0(std::size_t group, std::size_t position,
        float step) override;
    void run_interpolation_job_00875160(std::size_t group, std::size_t position,
        float leftover) override;

    // --- bsp::GameDynamicsBuoyancyHost, behind call 2 ----------------------
    void body_world_transform_00c32000(std::uint32_t body, bsp::DynamicsVec3& axis,
        bsp::DynamicsVec3& center) override;
    bsp::DynamicsVec3 body_box_extent_00c31f90(std::uint32_t body) override;
    float water_height_0078cf20(float x, float z) override;
    float body_buoyancy_scalar_00c31fc0(std::uint32_t body) override;
    void apply_buoyancy_force_00c32050(std::uint32_t body, float force) override;
    void set_body_damping_00c37de0(std::uint32_t body, float damping) override;

    void record(const char* method, std::uint32_t address);
    void done(const char* method, std::uint32_t address);

    GameHostLog& log_;
    bsp::GameDynamicsState& dynamics_;
    GameStepSubsystemsHost* subsystems_{nullptr};
    GameFixedStepSummary summary_{};
    bool first_step_reported_{false};
    bool groups_reported_{false};
};

}  // namespace bsp::game
