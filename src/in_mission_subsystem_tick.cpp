#include "bsp/in_mission_subsystem_tick.hpp"

namespace bsp {
namespace {

// The step table. Order is the native call order inside 004c40a0; `callee` is
// zero for the virtual dispatch at 004c40ce, whose slot 0Ch of vtable 00CE7784
// holds 00904bf0 (read from the image at 00CE7790).
constexpr InMissionTickStep kSteps[] = {
    {"step_fixed_simulation_00875bb0", 0x004c40adu, 0x00875bb0u,
        MissionFrameOwner::kUnit, "bsp::run_fixed_step_driver_00875bb0"},
    {"build_local_player_unit_lists_004c3cb0", 0x004c40b4u, 0x004c3cb0u,
        MissionFrameOwner::kHud, "bsp::local_player_unit_lists_run_004c3cb0"},
    {"update_world_entities_00904bf0", 0x004c40ceu, 0x00000000u,
        MissionFrameOwner::kUnit, ""},
    {"update_game_dynamics_00447b80", 0x004c40ddu, 0x00447b80u,
        MissionFrameOwner::kUnit, "bsp::tick_game_dynamics_frame_00447b80"},
};

} // namespace

std::size_t in_mission_tick_step_count() noexcept
{
    return sizeof(kSteps) / sizeof(kSteps[0]);
}

const InMissionTickStep& in_mission_tick_step(std::size_t index) noexcept
{
    if (index >= in_mission_tick_step_count()) {
        index = in_mission_tick_step_count() - 1;
    }
    return kSteps[index];
}

// 004c40a0..004c40e3. The scaled delta is reloaded from game+21F0h before each
// of the three calls that take it; nothing between the calls can change it, so
// one value is faithful.
void run_in_mission_subsystem_tick_004c40a0(float scaled_delta, InMissionTickHost& host)
{
    host.step_fixed_simulation_00875bb0(scaled_delta);   // 004c40ad
    host.build_local_player_unit_lists_004c3cb0();       // 004c40b4
    host.update_world_entities_00904bf0(scaled_delta);   // 004c40ce
    host.update_game_dynamics_00447b80(scaled_delta);    // 004c40dd
}

// 00875bb1..00875c02, in listing order. The fourth test only bites in view
// mode 2.
bool fixed_step_gate_open(const FixedStepGate& gate) noexcept
{
    if (!gate.game_present) {
        return false;
    }
    if (!gate.local_player_slot_present) {
        return false;
    }
    if (gate.slot_ready_10h < 1) { // 00875bdc CMP word, 00875be1 SETLE
        return false;
    }
    if (gate.view_mode_1fe4 == kFixedStepGatedViewMode &&
        gate.session_count_9c < kFixedStepGatedMinimum) {
        return false;
    }
    return true;
}

std::uint32_t advance_fixed_step_clock_00875bb0(
    FixedStepClock& clock, const FixedStepGate& gate, float delta) noexcept
{
    if (!fixed_step_gate_open(gate)) {
        // 00876003..0087601b: three of the clocks are zeroed, and only three.
        // The step count, the stepping flag and the buffer index survive.
        clock.accumulator = 0.0f;
        clock.simulation_clock = 0.0f;
        clock.raw_accumulated = 0.0f;
        return 0;
    }

    const float step = static_cast<float>(kFixedSimulationStepSeconds);

    clock.simulation_clock -= clock.accumulator; // 00875c08
    clock.accumulator += delta;                  // 00875c2f
    clock.raw_accumulated += delta;              // 00875c3d

    std::uint32_t steps = 0;
    if (step <= clock.accumulator) { // 00875c4f, the test is made twice
        while (step <= clock.accumulator) {
            clock.simulation_clock += step;          // 00875c68
            clock.step_count += 1;                   // 00875c6e
            clock.stepping = true;                   // 00875c86
            clock.previous_buffer = clock.buffer_index;              // 00875c5f
            clock.buffer_index = static_cast<std::uint16_t>(1u - clock.buffer_index); // 00875c7a
            clock.interpolation_left = 0.0f;         // 00875ef0
            clock.accumulator -= step;               // 00875efa
            steps += 1;
        }
        // 00875f0e, reached only from the loop: a frame that ran no step never
        // clears the flag (00875c51 jumps past this write).
        clock.stepping = false;
    }

    clock.simulation_clock += clock.accumulator; // 00875f26

    if (delta > 0.0f && clock.accumulator > 0.0f) { // 00875f1d, 00875f3a
        clock.interpolation_left = clock.accumulator; // 00875fc1
    }
    return steps;
}

std::uint32_t run_fixed_step_driver_00875bb0(
    FixedStepClock& clock, const FixedStepGate& gate, float delta,
    bool world_active, std::uint8_t run_pass, FixedStepHost& host)
{
    if (!fixed_step_gate_open(gate)) {
        clock.accumulator = 0.0f;
        clock.simulation_clock = 0.0f;
        clock.raw_accumulated = 0.0f;
        return 0;
    }

    const float step = static_cast<float>(kFixedSimulationStepSeconds);

    clock.simulation_clock -= clock.accumulator;
    clock.accumulator += delta;
    clock.raw_accumulated += delta;

    std::uint32_t steps = 0;
    if (step <= clock.accumulator) {
        while (step <= clock.accumulator) {
            clock.simulation_clock += step;
            clock.step_count += 1;
            clock.stepping = true;
            clock.previous_buffer = clock.buffer_index;
            clock.buffer_index = static_cast<std::uint16_t>(1u - clock.buffer_index);

            host.advance_step_countdown_008079b0(kFixedSimulationStepFloat); // 00875ca5
            host.run_step_job_waves(run_pass);                                // 00875cc0
            host.run_step_subsystems(kFixedSimulationStepFloat, world_active); // 00875e0c

            clock.interpolation_left = 0.0f;
            clock.accumulator -= step;
            steps += 1;
        }
        clock.stepping = false;
    }

    clock.simulation_clock += clock.accumulator;

    if (delta > 0.0f && clock.accumulator > 0.0f) {
        host.run_interpolation_wave_00875670(clock.accumulator, run_pass); // 00875f43
        clock.interpolation_left = clock.accumulator;
    }
    return steps;
}

// 004c3cb7..004c3cdc. The latch write is the routine's only unconditional
// effect; everything after it is the classification this packet documents but
// does not reconstruct.
bool local_player_unit_lists_run_004c3cb0(UnitListsGate& gate) noexcept
{
    if (gate.already_built) {
        return false;
    }
    // 004c3cc9 JL and 004c3cd1 CMP 7 / JG: the accepted range is 0..7.
    if (gate.slot_index < 0 || gate.slot_index >= kUnitListsSlotBound) {
        return false;
    }
    gate.already_built = true; // 004c3cdc
    return true;
}

} // namespace bsp
