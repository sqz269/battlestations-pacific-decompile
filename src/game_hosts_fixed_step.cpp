// bsp_game.exe milestone 2g: the fixed step's own body, run in process.
//
// Milestone 2f ran the driver 00875bb0 and left its four blocks as host
// records: the three job waves, the sixteen-call subsystem fan-out, the
// interpolation wave and the tail hook. Both reconstructions have since landed
// (docs/FIXED_STEP_FANOUT.md, docs/FIXED_STEP_JOB_WAVES.md), so this file runs
// them and records only the callees that have no reconstruction of their own.
//
// Two facts decide what a run of this file can show, and both are reported
// rather than worked around:
//
// 1. The world gate 00875e69..00875e7f reads [[game+19CCh]+4ACh]. This process
//    builds no world object (construct_world 004de610 is a load record), so the
//    gate is closed and rows 9..13 of the fan-out are skipped every step. That
//    is the native behaviour for an inactive world, not a substitution.
// 2. The five 68h groups at 00f876c0 are empty. Every element is built by
//    00875890, whose nine call sites are unit, aircraft and objective
//    constructions this process never reaches, and the one routine that would
//    splice a pending element into a group, 00874c90, is itself row 11 behind
//    the closed world gate. So every wave walks five empty groups, queues
//    nothing and dispatches nothing.
//
// See include/bsp/game_hosts_fixed_step.hpp for the address list.

#include "bsp/game_hosts_fixed_step.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/in_mission_subsystem_tick.hpp"

#include <cstdio>

namespace bsp::game {
namespace {

// 00e0819c, the damping scale 004462d0 multiplies the submersion by. The image
// ships 1.0f and nothing in the reconstructed startup writes it.
constexpr float kDynamicsDampingScale = 1.0f;

const char* owner_name(bsp::FixedStepFanoutOwner owner) {
    switch (owner) {
    case bsp::FixedStepFanoutOwner::kPhysics: return "physics";
    case bsp::FixedStepFanoutOwner::kUnit: return "unit";
    case bsp::FixedStepFanoutOwner::kRegistry: return "registry";
    case bsp::FixedStepFanoutOwner::kEngine: return "engine";
    case bsp::FixedStepFanoutOwner::kEntity: return "entity";
    case bsp::FixedStepFanoutOwner::kScript: return "script";
    case bsp::FixedStepFanoutOwner::kSession: return "session";
    case bsp::FixedStepFanoutOwner::kWorld: return "world";
    }
    return "unknown";
}

void format_address(std::uint32_t value, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(value));
}

}  // namespace

GameFixedStepHost::GameFixedStepHost(GameHostLog& log, bsp::GameDynamicsState& dynamics)
    : log_(log), dynamics_(dynamics) {}

void GameFixedStepHost::record(const char* method, std::uint32_t address) {
    char text[16];
    format_address(address, text);
    log_.unimplemented(method, text);
    ++summary_.fanout_records;
}

void GameFixedStepHost::done(const char* method, std::uint32_t address) {
    char text[16];
    format_address(address, text);
    log_.implemented(method, text);
    ++summary_.fanout_concrete;
}

// ---------------------------------------------------------------------------
// The three job waves, 00875cc0..00875dfc, and the interpolation wave
// ---------------------------------------------------------------------------

void GameFixedStepHost::run_job_waves_00875cc0(std::uint8_t run_pass) {
    bsp::run_fixed_step_job_waves(bsp::kFixedSimulationStepFloat, run_pass, *this);
    summary_.wave_groups += 3 * bsp::kJobWaveGroupCount;
    log_.implemented("FixedStep::run_job_waves", "00875cc0");
}

void GameFixedStepHost::run_interpolation_wave_00875670(float leftover,
    std::uint8_t run_pass) {
    // 00875f32..00875fcc. The driver stores the leftover accumulator into the
    // 0Ch-byte descriptor's +8h at 00875f50 and the wave hands it to the job
    // body instead of the 0.05f step.
    static_cast<void>(bsp::run_fixed_step_job_wave(
        bsp::JobWavePhase::kInterpolation, leftover, run_pass, *this));
    summary_.interpolation_groups += bsp::kJobWaveGroupCount;
    log_.implemented("FixedStep::interpolation_wave", "00875670");
}

bool GameFixedStepHost::next_element(std::size_t group, std::size_t position,
    bsp::JobWaveElementView& view) {
    static_cast<void>(group);
    static_cast<void>(position);
    static_cast<void>(view);
    if (!groups_reported_) {
        groups_reported_ = true;
        log_.notef("fixed-step job waves: the five 68h groups at 00f876c0 are empty. Every "
            "element comes from 00875890, whose nine construction sites are unit, aircraft "
            "and objective paths this process does not run, and the splice 00874c90 is row "
            "11 behind the closed world gate");
    }
    return false;
}

void GameFixedStepHost::queue_job(bsp::JobWavePhase phase, std::size_t group,
    std::size_t position) {
    static_cast<void>(phase);
    static_cast<void>(group);
    static_cast<void>(position);
    // 004c1130 then [[pool+4h]+4h]. The frame job pool is the render owner's.
    ++summary_.jobs_queued;
    log_.unimplemented("FixedStepJobs::queue_job", "00be3020");
}

void GameFixedStepHost::dispatch_group(bsp::JobWavePhase phase, std::size_t group,
    std::uint8_t run_pass) {
    static_cast<void>(phase);
    static_cast<void>(group);
    static_cast<void>(run_pass);
    ++summary_.group_dispatches;
    log_.unimplemented("FixedStepJobs::dispatch_group", "00be3150");
}

void GameFixedStepHost::run_wave1_job_008750a0(std::size_t group, std::size_t position,
    float step) {
    static_cast<void>(group);
    static_cast<void>(position);
    static_cast<void>(step);
    log_.unimplemented("FixedStepJobs::wave1_job", "008750a0");
}

void GameFixedStepHost::run_wave2_job_00875b90(std::size_t group, std::size_t position,
    float step) {
    static_cast<void>(group);
    static_cast<void>(position);
    static_cast<void>(step);
    log_.unimplemented("FixedStepJobs::wave2_job", "00875b90");
}

void GameFixedStepHost::run_wave3_job_00874fe0(std::size_t group, std::size_t position,
    float step) {
    static_cast<void>(group);
    static_cast<void>(position);
    static_cast<void>(step);
    log_.unimplemented("FixedStepJobs::wave3_job", "00874fe0");
}

void GameFixedStepHost::run_interpolation_job_00875160(std::size_t group,
    std::size_t position, float leftover) {
    static_cast<void>(group);
    static_cast<void>(position);
    static_cast<void>(leftover);
    log_.unimplemented("FixedStepJobs::interpolation_job", "00875160");
}

// ---------------------------------------------------------------------------
// The sixteen per-step calls, 00875e0c..00875edf
// ---------------------------------------------------------------------------

void GameFixedStepHost::run_subsystems_00875e0c(float step, bool world_active) {
    bsp::FixedStepWorldGate gate{};
    gate.game_present = true;                  // 00875e6e, the process owns the game object
    gate.world_active = world_active;          // 00875e78, [[game+19CCh]+4ACh]
    const bool open = bsp::fixed_step_world_gate_open(gate);

    ++summary_.steps;
    if (!open) {
        ++summary_.gate_closed_steps;
    }

    if (!first_step_reported_) {
        first_step_reported_ = true;
        log_.notef("fixed-step fan-out 00875e0c, the sixteen per-step calls at %.3f s "
            "(00d0de84); the world gate 00875e69 is %s",
            static_cast<double>(step), open ? "open" : "closed");
        const std::size_t rows = bsp::fixed_step_fanout_step_count();
        for (std::size_t i = 0; i < rows; ++i) {
            const bsp::FixedStepFanoutStep& row = bsp::fixed_step_fanout_step(i);
            const bool runs = open || !row.gated;
            log_.notef("  %2zu %08lx -> %08lx %-42s %-8s %s", i + 1,
                static_cast<unsigned long>(row.call_site),
                static_cast<unsigned long>(row.callee), row.host_method,
                owner_name(row.owner),
                runs ? (row.reconstruction[0] != '\0' ? "concrete" : "record")
                     : "skipped (world gate)");
        }
    }

    const unsigned long long before = summary_.fanout_calls;
    bsp::run_fixed_step_subsystem_fanout_00875e0c(step, gate, *this);
    const unsigned long long ran = summary_.fanout_calls - before;
    if (!open) summary_.gated_calls_skipped += 5;
    log_.notef("  fixed step %llu: %llu of 16 fan-out sites ran, %s", summary_.steps, ran,
        open ? "world gate open" : "world gate closed, rows 9-13 skipped");
}

void GameFixedStepHost::simulate_physics_world_00c5c540(float step) {
    static_cast<void>(step);
    ++summary_.fanout_calls;
    // The mitengine Dynamics world at [game+18h]; its own substep loop is a
    // third-party library body with no reconstruction.
    record("FixedStepFanout::simulate_physics_world", 0x00875e0cu);
}

void GameFixedStepHost::apply_dynamics_buoyancy_004462d0(float step) {
    static_cast<void>(step);
    ++summary_.fanout_calls;
    // The one row of the sixteen with a reconstruction on main. The list is the
    // same bsp::GameDynamicsState the frame's fourth tick call walks, and it is
    // empty here, so the walk processes no record and no host method is reached.
    summary_.buoyancy_records
        = bsp::apply_dynamics_buoyancy_004462d0(dynamics_, kDynamicsDampingScale, *this);
    done("FixedStepFanout::apply_dynamics_buoyancy", 0x00875e24u);
}

void GameFixedStepHost::refresh_moved_spatial_nodes_0098bdb0(float step) {
    static_cast<void>(step);
    summary_.fanout_calls += 2;  // 00875e33 the getter and 00875e3a the walk
    record("FixedStepFanout::refresh_moved_spatial_nodes", 0x00875e3au);
}

void GameFixedStepHost::run_fixed_step_callbacks_00874de0(float step) {
    static_cast<void>(step);
    ++summary_.fanout_calls;
    record("FixedStepFanout::run_step_callbacks", 0x00875e3fu);
}

void GameFixedStepHost::drain_deferred_entity_events_00926700() {
    ++summary_.fanout_calls;
    // Rows 6 and 14 are the same callee; the record carries the first site,
    // and the call count says it ran twice per step.
    record("FixedStepFanout::drain_deferred_entity_events", 0x00875e44u);
}

void GameFixedStepHost::drain_queued_lua_calls_00888230() {
    ++summary_.fanout_calls;
    // [game+1A08h], the mission Lua host this process does own; what 00888230
    // drains is the cross-thread call list, which has no reconstruction.
    record("FixedStepFanout::drain_queued_lua_calls", 0x00875e55u);
}

void GameFixedStepHost::run_due_entity_think_00929460(float step) {
    static_cast<void>(step);
    ++summary_.fanout_calls;
    record("FixedStepFanout::run_due_entity_think", 0x00875e64u);
}

void GameFixedStepHost::pump_session_00778450(float step) {
    static_cast<void>(step);
    ++summary_.fanout_calls;
    record("FixedStepFanout::pump_session", 0x00875e91u);
}

void GameFixedStepHost::apply_pending_entity_creates_0077ec20() {
    ++summary_.fanout_calls;
    record("FixedStepFanout::apply_pending_entity_creates", 0x00875e96u);
}

void GameFixedStepHost::flush_pending_tick_registrations_00874c90() {
    ++summary_.fanout_calls;
    record("FixedStepFanout::flush_tick_registrations", 0x00875e9bu);
}

void GameFixedStepHost::init_pending_entities_00925f20(bool flag) {
    static_cast<void>(flag);
    ++summary_.fanout_calls;
    record("FixedStepFanout::init_pending_entities", 0x00875ea2u);
}

void GameFixedStepHost::flush_outbound_session_0076ffc0(float step, std::int32_t mode) {
    static_cast<void>(step);
    static_cast<void>(mode);
    ++summary_.fanout_calls;
    record("FixedStepFanout::flush_outbound_session", 0x00875ebfu);
}

void GameFixedStepHost::flush_pending_entity_queues_009273a0() {
    ++summary_.fanout_calls;
    record("FixedStepFanout::flush_pending_entity_queues", 0x00875ec9u);
}

void GameFixedStepHost::release_expired_world_objects_00903610() {
    ++summary_.fanout_calls;
    record("FixedStepFanout::release_expired_world_objects", 0x00875edau);
}

void GameFixedStepHost::run_tail_hook_00a317f0() {
    // 00a317f0 is a single RET in this build, so a no-op host is faithful. The
    // record exists so a run can say the four-test gate opened.
    log_.implemented("FixedStepFanout::tail_hook", "00875ff7");
}

void GameFixedStepHost::run_tail_00875fd1() {
    // 00875fd1..00875ff5. None of the four tests can hold here: 00f8ab04 has no
    // writer in this process and the in-mission interface manager 00e198c4 is
    // the front-end owner's, which the load released as a record.
    bsp::FixedStepTailGate gate{};
    gate.enabled_00f8ab04 = false;
    gate.interface_manager = false;
    gate.manager_sub_object = false;
    gate.sub_object_flag_05 = false;
    if (bsp::run_fixed_step_tail_00875fd1(gate, *this)) {
        ++summary_.tail_gate_open;
    } else {
        ++summary_.tail_gate_closed;
    }
}

// ---------------------------------------------------------------------------
// bsp::GameDynamicsBuoyancyHost, behind call 2. The list is empty, so none of
// these is reached; each records its own native site if it ever is.
// ---------------------------------------------------------------------------

void GameFixedStepHost::body_world_transform_00c32000(std::uint32_t body,
    bsp::DynamicsVec3& axis, bsp::DynamicsVec3& center) {
    static_cast<void>(body);
    axis = bsp::DynamicsVec3{};
    center = bsp::DynamicsVec3{};
    log_.unimplemented("DynamicsBuoyancy::body_world_transform", "00c32000");
}

bsp::DynamicsVec3 GameFixedStepHost::body_box_extent_00c31f90(std::uint32_t body) {
    static_cast<void>(body);
    log_.unimplemented("DynamicsBuoyancy::body_box_extent", "00c31f90");
    return bsp::DynamicsVec3{};
}

float GameFixedStepHost::water_height_0078cf20(float x, float z) {
    static_cast<void>(x);
    static_cast<void>(z);
    log_.unimplemented("DynamicsBuoyancy::water_height", "0078cf20");
    return 0.0f;
}

float GameFixedStepHost::body_buoyancy_scalar_00c31fc0(std::uint32_t body) {
    static_cast<void>(body);
    log_.unimplemented("DynamicsBuoyancy::body_buoyancy_scalar", "00c31fc0");
    return 0.0f;
}

void GameFixedStepHost::apply_buoyancy_force_00c32050(std::uint32_t body, float force) {
    static_cast<void>(body);
    static_cast<void>(force);
    log_.unimplemented("DynamicsBuoyancy::apply_force", "00c32050");
}

void GameFixedStepHost::set_body_damping_00c37de0(std::uint32_t body, float damping) {
    static_cast<void>(body);
    static_cast<void>(damping);
    log_.unimplemented("DynamicsBuoyancy::set_body_damping", "00c37de0");
}

void GameFixedStepHost::report() {
    log_.notef("summary fixed step body steps=%llu fanout_sites=%llu concrete=%llu "
        "records=%llu gate_closed_steps=%llu gated_sites_skipped=%llu",
        summary_.steps, summary_.fanout_calls, summary_.fanout_concrete,
        summary_.fanout_records, summary_.gate_closed_steps,
        summary_.gated_calls_skipped);
    log_.notef("summary fixed step waves groups=%llu interpolation_groups=%llu "
        "elements=%llu queued=%llu dispatches=%llu tail_open=%llu tail_closed=%llu",
        summary_.wave_groups, summary_.interpolation_groups, summary_.elements_seen,
        summary_.jobs_queued, summary_.group_dispatches, summary_.tail_gate_open,
        summary_.tail_gate_closed);
}

}  // namespace bsp::game
