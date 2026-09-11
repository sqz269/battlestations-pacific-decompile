#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/mission_state_frame.hpp"

// The fixed four-call sequence that opens every simulated mission frame.
//
// docs/MISSION_STATE_FRAME.md step 9 is one call, 004e5133 -> 004c40a0, and it
// had no reconstruction. 004c40a0 is the whole simulated part of an in-mission
// frame: four calls, no test, no local state. docs/IN_MISSION_SUBSYSTEM_TICK.md
// carries the evidence; the dynamics list behind call 4 is
// bsp/game_dynamics_list.hpp.
//
// MissionFrameOwner comes from bsp/mission_state_frame.hpp and is reused here so
// a caller tags both tables the same way.
//
// Every name below is a hypothesis, not a recovered symbol.

namespace bsp {

// ---------------------------------------------------------------------------
// 004c40a0, __thiscall void(GGame*), body 004c40a0..004c40e3, RET (no stack
// argument). Sole caller 004e4a40 at 004e5133.
// ---------------------------------------------------------------------------

// The one field the sequence reads, reloaded before each call that takes it.
inline constexpr std::size_t kInMissionScaledDeltaOffset = 0x21f0; // 004c40a3

// One row per host method, in native call order. `callee` is zero for the
// virtual dispatch, whose slot is kWorldUpdateVtableSlot below.
struct InMissionTickStep {
    const char* host_method;
    std::uint32_t call_site;
    std::uint32_t callee;
    MissionFrameOwner owner;
    const char* reconstruction; // "" when nothing on main implements it yet
};

std::size_t in_mission_tick_step_count() noexcept;
const InMissionTickStep& in_mission_tick_step(std::size_t index) noexcept;

// 004c40bf..004c40ce: ECX = [game+19CCh] (the world object, vtable 00CE7784),
// EDX = [[ECX]+0Ch]. That slot holds 00904bf0 BSP_World_UpdateEntities, read
// from the image at 00CE7790.
inline constexpr std::size_t kWorldObjectOffset = 0x19cc;      // 004c40bf
inline constexpr std::size_t kWorldUpdateVtableSlot = 0x0c;    // 004c40c7
inline constexpr std::size_t kGameDynamicsListOffset = 0x30;   // 004c40d7

struct InMissionTickHost {
    virtual ~InMissionTickHost() = default;

    // 004c40ad, 00875bb0. The site passes game in ECX; the callee discards it
    // (00875bb1 MOV ECX,[00E188A8]). run_fixed_step_driver_00875bb0 below is
    // the same routine split into the rule and its host.
    virtual void step_fixed_simulation_00875bb0(float scaled_delta) = 0;

    // 004c40b4, 004c3cb0, ECX = game, no argument. Does work only on the first
    // simulated frame after a scene load; see the latch below.
    virtual void build_local_player_unit_lists_004c3cb0() = 0;

    // 004c40ce, [game+19CCh]->vtable[0Ch] = 00904bf0, one float.
    // docs/GAME_WORLD_ENTITIES.md already describes the callee.
    virtual void update_world_entities_00904bf0(float scaled_delta) = 0;

    // 004c40dd, 00447b80, ECX = [game+30h], one float.
    // bsp/game_dynamics_list.hpp reconstructs the callee.
    virtual void update_game_dynamics_00447b80(float scaled_delta) = 0;
};

// The four calls, in order, each with the same scaled delta. No branch: the
// native routine tests nothing.
void run_in_mission_subsystem_tick_004c40a0(float scaled_delta, InMissionTickHost& host);

// ---------------------------------------------------------------------------
// Call 1, 00875bb0: the fixed-step simulation driver
// __thiscall void(ignored, float), body 00875bb0..0087601e, RET 4.
// ---------------------------------------------------------------------------

// 00d7a270 holds the double 0.05; 00d0de84 holds the float 0.05f the per-step
// callees receive and 00447b80 divides the leftover accumulator by.
inline constexpr double kFixedSimulationStepSeconds = 0.05;    // 00d7a270
inline constexpr float kFixedSimulationStepFloat = 0.05f;      // 00d0de84

// 00875bb1..00875c02, the four gate tests in listing order. All four must pass
// or the three clocks are zeroed (00876003..0087601b) and nothing runs.
struct FixedStepGate {
    bool game_present{};             // [00E188A8] != 0,               00875bb7
    bool local_player_slot_present{};// [game+18CCh + 18ECh*4] != 0,   00875bc5
    std::int16_t slot_ready_10h{};   // that slot's +10h short, >= 1,  00875bdc
    std::int32_t view_mode_1fe4{};   // game+1FE4h,                    00875bec
    std::int32_t session_count_9c{}; // [[game+207Ch]+9Ch],            00875bfb
};

// 00875bec..00875c02: mode 2 additionally wants at least eight of whatever
// [[game+207Ch]+9Ch] counts.
inline constexpr std::int32_t kFixedStepGatedViewMode = 2; // 00875bf3
inline constexpr std::int32_t kFixedStepGatedMinimum = 8;  // 00875bfb

bool fixed_step_gate_open(const FixedStepGate& gate) noexcept;

// The clock globals 00875bb0 owns, in one place. The names are roles, not
// symbols; the addresses are the evidence.
struct FixedStepClock {
    float accumulator{};        // 00f876ac, the leftover is the interpolation numerator
    float simulation_clock{};   // 00f876a4, the deadline base 004d87b0 reads
    float raw_accumulated{};    // 00f876a8, never decremented
    std::uint32_t step_count{}; // 00f876b0
    bool stepping{};            // 00f876a1, set during the loop, cleared after
    float interpolation_left{}; // 00f876b4, zeroed per step, set after the wave
    std::uint16_t buffer_index{};   // 00e0b6cc low word, flipped per step (00875c9a)
    std::uint16_t previous_buffer{};// 00f876b8, the value before the flip (00875c93)
};

// 00875c08..00875fcc as one rule. Returns the number of fixed steps the loop
// ran. The order is the listing's: the clock is pulled back by the previous
// leftover before the new delta is folded in (00875c08..00875c3d), every step
// adds a whole step to the clock and flips the buffer, and the leftover is
// added back afterwards (00875f26). A closed gate zeroes the three clocks
// (00876003..0087601b) and returns 0.
std::uint32_t advance_fixed_step_clock_00875bb0(
    FixedStepClock& clock, const FixedStepGate& gate, float delta) noexcept;

// The parts of 00875bb0 that are calls, one method per block. Each runs once
// per fixed step except run_interpolation_wave_00875670, which runs once per
// frame after the loop.
struct FixedStepHost {
    virtual ~FixedStepHost() = default;
    // 00875ca5, 008079b0, the countdown at 00f874b8.
    virtual void advance_step_countdown_008079b0(float step) = 0;
    // 00875cc0..00875dfc, the three job waves over the five 68h-stride group
    // heads at 00f876c8. `run_pass` is the byte at 00e0b6ce that every wave
    // hands to the pool's run call (00875d06, 00875d76, 00875de6). It is a
    // different field from the flipped index at 00e0b6cc, which only the word
    // store at 00875c9a touches.
    virtual void run_step_job_waves(std::uint8_t run_pass) = 0;
    // 00875e0c..00875edf, the sixteen per-step calls. 00875e24 inside this
    // block is apply_dynamics_buoyancy_004462d0 of bsp/game_dynamics_list.hpp.
    // `world_active` is [[game+19CCh]+4ACh], which gates five of the sixteen.
    virtual void run_step_subsystems(float step, bool world_active) = 0;
    // 00875f32..00875fcc, the same five groups with the leftover accumulator in
    // the job descriptor's +8h (00875f50) and the same run byte (00875fad).
    virtual void run_interpolation_wave_00875670(float leftover, std::uint8_t run_pass) = 0;
};

// The whole of call 1: the gate, the clock rule and the loop body. The step
// body is the host's; the count and the clock are the rule's.
std::uint32_t run_fixed_step_driver_00875bb0(
    FixedStepClock& clock, const FixedStepGate& gate, float delta,
    bool world_active, std::uint8_t run_pass, FixedStepHost& host);

// ---------------------------------------------------------------------------
// Call 2, 004c3cb0: the local-player unit lists
// __thiscall void(GGame*), body 004c3cb0..004c409d.
// ---------------------------------------------------------------------------

// 004c3cb7..004c3cdc. The latch is cleared only by BSP_Game_LoadMissionScene
// 004dfb70, at 004e0360 and 004e05a2, so the body runs once per scene load and
// the tick's call is a no-op on every frame after the first.
inline constexpr std::size_t kUnitListsBuiltLatchOffset = 0x193c;   // 004c3cb7
inline constexpr std::size_t kLocalPlayerSlotArrayOffset = 0x18cc;  // 004c3cee
inline constexpr std::size_t kLocalPlayerSlotSelectorOffset = 0x18ec; // 004c3cc3
// bsp/game_frame_control.hpp already declares kLocalPlayerSlotCount for the
// same eight-slot array; this is the bound as 004c3cd0 tests it, signed.
inline constexpr std::int32_t kUnitListsSlotBound = 8;               // 004c3cd1 CMP 7, JG

// The eight {count, head, tail} heads 004bfdf0 touches and the walks append to,
// in address order. 004c404c..004c407d merges into the last one.
inline constexpr std::size_t kUnitListHeadOffsets[] = {
    0x1964, 0x1970, 0x197c, 0x1988, 0x1994, 0x19a0, 0x19ac, 0x19b8,
}; // 004bfdf3..004bfe40

struct UnitListsGate {
    bool already_built{};       // game+193Ch
    std::int32_t slot_index{};  // game+18ECh
};

// 004c3cbe..004c3cdc: the guard and the latch write, which is all of this
// routine that is a rule. Returns true when the body runs, and sets the latch.
bool local_player_unit_lists_run_004c3cb0(UnitListsGate& gate) noexcept;

} // namespace bsp
