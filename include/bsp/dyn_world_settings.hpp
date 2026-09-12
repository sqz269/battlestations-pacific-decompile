#pragma once

#include <cstdint>

#include "bsp/rigid_body_integration.hpp"
#include "bsp/world_ocean.hpp"

// The dynamics world the ship controller's hull body lives in: where it is created, what
// descriptor it is created from, and which of its fields the substep schedule and the two
// integration phases read.
//
// docs/DYN_WORLD_SETTINGS.md carries the addresses, the original ABI and the uncertainty.
// Everything here is a semantic C++ interface for MSVC Win32, not a drop-in binary
// replacement, and every descriptive name is a hypothesis rather than a recovered symbol.
//
// The chain, all of it read for this packet:
//
//   004DDB90  the game object's constructor. Publishes itself at DAT_00E188A8
//             (004DE105), builds a 40h-byte world descriptor on its stack at ESP+1Ch,
//             calls the world factory with the Dyn engine from game+14h in ECX
//             (004DE1D3) and stores the returned world at game+18h (004DE1DB).
//   00C420E0  __thiscall(engine, const desc*): operator_new(48Ch) at 00C420F8, then
//             00C41AD0 at 00C4211B, then the new world is appended to the engine's
//             world vector.
//   00C41AD0  the world constructor, __cdecl world*(world*, const desc*). Fifteen
//             copies at 00C41AF2..00C41B4A are the whole of the descriptor's effect.
//   00C5C540  the substep schedule: world+00h (00C5C651), world+34h (00C5C63C) and the
//             accumulator world+48h.
//   00C41550 / 00C5B1B0  the two integration phases: world+04h..+0Ch (gravity) and
//             world+3Ch..+44h (the sleep block).
//
// This packet does not read the collision, group or LCP phases, so the eight further
// descriptor fields are recorded by offset with their authored values and no contract.

namespace bsp {

// The 40h-byte descriptor 004DDB90 builds on its stack (base ESP+1Ch at 004DE176) and
// hands to the world factory. Offsets are the native byte offsets inside that block.
// Names come from the world fields 00C41AD0 copies them into, not from the call site.
struct DynWorldDescriptor {
    float fixed_substep{0.0f};               // desc+00h -> world+00h
    OceanVec3 gravity{};                     // desc+04h..+0Ch -> world+04h..+0Ch
    std::int32_t substep_budget{0};          // desc+10h -> world+34h
    float sleep_linear_speed{0.0f};          // desc+14h -> world+3Ch
    float sleep_angular_speed{0.0f};         // desc+18h -> world+40h
    std::int32_t sleep_countdown_reload{0};  // desc+1Ch -> world+44h
    std::uint32_t solver_word_20{0};         // desc+20h -> world+10h, contract unread
    std::int32_t solver_word_24{0};          // desc+24h -> world+38h, contract unread
    float solver_float_28{0.0f};             // desc+28h -> world+14h, contract unread
    float solver_float_2c{0.0f};             // desc+2Ch -> world+18h, contract unread
    float solver_float_30{0.0f};             // desc+30h -> world+1Ch, contract unread
    float solver_float_34{0.0f};             // desc+34h -> world+20h, contract unread
    float solver_float_38{0.0f};             // desc+38h -> world+24h, contract unread
    float solver_float_3c{0.0f};             // desc+3Ch -> world+28h, contract unread
};

// The world fields this packet established a reader for. The world object is 48Ch bytes
// (operator_new(0x48c) at 00C420F8); this is the slice the step reads, not a layout.
struct DynWorldSettings {
    float fixed_substep{0.0f};               // world+00h, read by 00C5C540 at 00C5C651
    OceanVec3 gravity{};                     // world+04h..+0Ch, read by 00C41550
    std::int32_t substep_budget{0};          // world+34h, read by 00C5C540 at 00C5C63C
    float sleep_linear_speed{0.0f};          // world+3Ch, read by 00C5B1B0
    float sleep_angular_speed{0.0f};         // world+40h, read by 00C5B1B0
    std::int32_t sleep_countdown_reload{0};  // world+44h, read by 00C5B1B0
    // world+48h is the accumulator the schedule owns; it is per-frame state, not a
    // setting, so it stays with the caller.
};

// ---------------------------------------------------------------------------
// The authored values, each with the image address it is loaded from
// ---------------------------------------------------------------------------

// 00CE7638, loaded at 004DE160 and stored at 004DE168. Bit for bit the game-step float at
// 00D0DE84 that 00875E0C passes into 00C5C540, which is what makes the schedule take
// exactly one substep of the whole game step.
inline constexpr float kDynWorldFixedSubstep = 0.05f;
// 00CE6848, loaded at 004DE189 and stored at 004DE1BB. Gravity is (0, -10, 0); the x and
// z slots take the zeroed XMM0 of 004DE180 at 004DE1B5 and 004DE1C1.
inline constexpr float kDynWorldGravityY = -10.0f;
// EDI, set to 1 at 004DE10B and stored at 004DE1AB. An int, not a float: 00C5C63C reads
// it with MOV, 00C5C661 compares it with CMP dword and 00C5C674 decrements it with SUB.
inline constexpr std::int32_t kDynWorldSubstepBudget = 1;
// The immediates at 004DE197 (14h) and 004DE1A3 (0Ah).
inline constexpr std::int32_t kDynWorldSleepCountdownReload = 20;
inline constexpr std::int32_t kDynWorldSolverWord24 = 10;
// 00D7A2F0 (004DE17A), 00CE7480 (004DE13E), 00D7A24C (004DE14C), 00CE746C (004DE15A),
// 00CE3800 (004DE183). Their readers are in the phases this packet does not cover.
inline constexpr float kDynWorldSolverFloat2c = 0.1f;
inline constexpr float kDynWorldSolverFloat30 = 0.85f;
inline constexpr float kDynWorldSolverFloat34 = 1.0f;
inline constexpr float kDynWorldSolverFloat38 = 0.02f;
inline constexpr float kDynWorldSolverFloat3c = 0.5f;
// 00D0DE84, the argument 00875E0C pushes into 00C5C540 once per fixed game step.
inline constexpr float kDynWorldGameStep = 0.05f;

// ---------------------------------------------------------------------------
// The rules
// ---------------------------------------------------------------------------

// The descriptor 004DDB90 fills at 004DE13E..004DE1CD. Every slot it does not name keeps
// the zero the same block writes (EBX and the zeroed XMM0).
DynWorldDescriptor dyn_world_descriptor_004ddb90() noexcept;

// 00C41AD0, __cdecl world*(world*, const desc*); 00C420E0 pushes the descriptor at
// 00C42119 and the fresh world at 00C4211A. The fifteen copies at 00C41AF2..00C41B4A are
// the whole of the descriptor's effect on the world.
DynWorldSettings dyn_world_construct_00c41ad0(const DynWorldDescriptor& desc) noexcept;

// The world the running game holds at game+18h: the two rules above composed.
DynWorldSettings dyn_world_settings_game() noexcept;

// The slice of the world the two integration phases read, in the shape
// bsp/rigid_body_integration.hpp already declares for them.
DynWorldStepConstants dyn_world_step_constants(const DynWorldSettings& settings) noexcept;

// What 00C5C540 does with one game step of `dt` in this world. This is
// dyn_world_substep_plan_00c5c540 with the world's own fields supplied; it exists so a
// caller cannot pass the budget as a float, because the native field is an int.
DynWorldSubstepPlan dyn_world_substep_plan(const DynWorldSettings& settings,
                                           float& accumulator, float dt) noexcept;

}  // namespace bsp
