#include "bsp/fixed_step_fanout.hpp"

// 00875E0C..00875EDF and 00875FD1..00875FF7 of BSP_Game_RunFixedSimulationSteps
// (00875BB0, body 00875BB0..0087601E). docs/FIXED_STEP_FANOUT.md.
//
// The sequence is a straight line with one two-sided branch: rows 9..13 are
// skipped when the world gate is closed (00875E70 and 00875E7F both JE to
// 00875EC4). Nothing else in the block tests anything.

namespace bsp {
namespace {

// The table is the listing, transcribed once. `callee_this` is the ECX the site
// sets; the reconstruction column names what already exists on main.
constexpr FixedStepFanoutStep kSteps[kFixedStepFanoutStepCount] = {
    {"simulate_physics_world_00c5c540", 0x00875e0c, 0x00c5c540, "[game+18h]", true, false,
     FixedStepFanoutOwner::kPhysics, ""},
    {"apply_dynamics_buoyancy_004462d0", 0x00875e24, 0x004462d0, "[game+30h]", true, false,
     FixedStepFanoutOwner::kUnit, "apply_dynamics_buoyancy_004462d0"},
    {"refresh_moved_spatial_nodes_0098bdb0", 0x00875e33, 0x0042e630, "", false, false,
     FixedStepFanoutOwner::kRegistry, ""},
    {"refresh_moved_spatial_nodes_0098bdb0", 0x00875e3a, 0x0098bdb0, "EAX of 0042E630", true, false,
     FixedStepFanoutOwner::kRegistry, ""},
    {"run_fixed_step_callbacks_00874de0", 0x00875e3f, 0x00874de0, "", false, false,
     FixedStepFanoutOwner::kEngine, ""},
    {"drain_deferred_entity_events_00926700", 0x00875e44, 0x00926700, "", false, false,
     FixedStepFanoutOwner::kEntity, ""},
    {"drain_queued_lua_calls_00888230", 0x00875e55, 0x00888230, "[game+1A08h]", false, false,
     FixedStepFanoutOwner::kScript, ""},
    {"run_due_entity_think_00929460", 0x00875e64, 0x00929460, "", true, false,
     FixedStepFanoutOwner::kScript, ""},
    {"pump_session_00778450", 0x00875e91, 0x00778450, "game+1EF0h", true, true,
     FixedStepFanoutOwner::kSession, ""},
    {"apply_pending_entity_creates_0077ec20", 0x00875e96, 0x0077ec20, "", false, true,
     FixedStepFanoutOwner::kSession, ""},
    {"flush_pending_tick_registrations_00874c90", 0x00875e9b, 0x00874c90, "", false, true,
     FixedStepFanoutOwner::kEngine, ""},
    {"init_pending_entities_00925f20", 0x00875ea2, 0x00925f20, "CL = 0", false, true,
     FixedStepFanoutOwner::kEntity, ""},
    {"flush_outbound_session_0076ffc0", 0x00875ebf, 0x0076ffc0, "game+1EF0h", true, true,
     FixedStepFanoutOwner::kSession, ""},
    {"drain_deferred_entity_events_00926700", 0x00875ec4, 0x00926700, "", false, false,
     FixedStepFanoutOwner::kEntity, ""},
    {"flush_pending_entity_queues_009273a0", 0x00875ec9, 0x009273a0, "", false, false,
     FixedStepFanoutOwner::kEntity, ""},
    {"release_expired_world_objects_00903610", 0x00875eda, 0x00903610, "[game+19CCh]", false, false,
     FixedStepFanoutOwner::kWorld, ""},
};

} // namespace

bool fixed_step_world_gate_open(const FixedStepWorldGate& gate) noexcept
{
    // 00875E6E TEST EAX,EAX / JE, then 00875E78 CMP byte [ECX+4ACh],0 / JE.
    return gate.game_present && gate.world_active;
}

bool fixed_step_tail_hook_runs(const FixedStepTailGate& gate) noexcept
{
    // 00875FD7, 00875FE6, 00875FEC, 00875FF5: four JE to the same exit.
    return gate.enabled_00f8ab04 && gate.interface_manager && gate.manager_sub_object
           && gate.sub_object_flag_05;
}

std::size_t fixed_step_fanout_step_count() noexcept
{
    return kFixedStepFanoutStepCount;
}

const FixedStepFanoutStep& fixed_step_fanout_step(std::size_t index) noexcept
{
    if (index >= kFixedStepFanoutStepCount) {
        index = kFixedStepFanoutStepCount - 1;
    }
    return kSteps[index];
}

void run_fixed_step_subsystem_fanout_00875e0c(
    float step, const FixedStepWorldGate& gate, FixedStepFanoutHost& host)
{
    host.simulate_physics_world_00c5c540(step);          // 00875E0C
    host.apply_dynamics_buoyancy_004462d0(step);         // 00875E24
    host.refresh_moved_spatial_nodes_0098bdb0(step);     // 00875E33 + 00875E3A
    host.run_fixed_step_callbacks_00874de0(step);        // 00875E3F
    host.drain_deferred_entity_events_00926700();        // 00875E44
    host.drain_queued_lua_calls_00888230();              // 00875E55
    host.run_due_entity_think_00929460(step);            // 00875E64

    if (fixed_step_world_gate_open(gate)) {              // 00875E69..00875E7F
        host.pump_session_00778450(step);                // 00875E91
        host.apply_pending_entity_creates_0077ec20();    // 00875E96
        host.flush_pending_tick_registrations_00874c90();// 00875E9B
        host.init_pending_entities_00925f20(kFanoutInitAllFlag);           // 00875EA2
        host.flush_outbound_session_0076ffc0(step, kFanoutOutboundSessionMode); // 00875EBF
    }

    host.drain_deferred_entity_events_00926700();        // 00875EC4
    host.flush_pending_entity_queues_009273a0();         // 00875EC9
    host.release_expired_world_objects_00903610();       // 00875EDA
}

bool run_fixed_step_tail_00875fd1(
    const FixedStepTailGate& gate, FixedStepFanoutHost& host)
{
    if (!fixed_step_tail_hook_runs(gate)) {
        return false;
    }
    host.run_tail_hook_00a317f0(); // 00875FF7
    return true;
}

} // namespace bsp
