// The dynamics world's construction. docs/DYN_WORLD_SETTINGS.md.
//
// Two routines and one call site, read in full for this packet:
//   004DDB90  the game constructor's world block, 004DE13E..004DE1DB
//   00C420E0  the world factory, 00C420E0..00C421A3
//   00C41AD0  the world constructor's descriptor copy, 00C41AF2..00C41B4A
//
// Nothing here is a binary-compatible replacement; the native world is a 48Ch-byte object
// whose full layout is not recovered.

#include "bsp/dyn_world_settings.hpp"

namespace bsp {

DynWorldDescriptor dyn_world_descriptor_004ddb90() noexcept {
    // The stores are listed in image order. Six of the sixteen slots are written before
    // the ECX push at 004DE191 and the rest after it, which is why a linear read of the
    // listing sees two different displacements for one descriptor.
    DynWorldDescriptor desc{};
    desc.solver_float_30 = kDynWorldSolverFloat30;  // 004DE13E, 00CE7480
    desc.solver_float_34 = kDynWorldSolverFloat34;  // 004DE14C, 00D7A24C
    desc.solver_float_38 = kDynWorldSolverFloat38;  // 004DE15A, 00CE746C
    desc.fixed_substep = kDynWorldFixedSubstep;     // 004DE168, 00CE7638
    desc.solver_float_2c = kDynWorldSolverFloat2c;  // 004DE17A, 00D7A2F0
    desc.solver_float_3c = kDynWorldSolverFloat3c;  // 004DE183, 00CE3800
    desc.sleep_countdown_reload = kDynWorldSleepCountdownReload;  // 004DE197, imm 14h
    desc.solver_word_20 = 0;                        // 004DE19F, EBX
    desc.solver_word_24 = kDynWorldSolverWord24;    // 004DE1A3, imm 0Ah
    desc.substep_budget = kDynWorldSubstepBudget;   // 004DE1AB, EDI
    desc.solver_float_28 = 0.0f;                    // 004DE1AF, zeroed XMM0
    desc.gravity.x = 0.0f;                          // 004DE1B5
    desc.gravity.y = kDynWorldGravityY;             // 004DE1BB, 00CE6848
    desc.gravity.z = 0.0f;                          // 004DE1C1
    desc.sleep_linear_speed = 0.0f;                 // 004DE1C7
    desc.sleep_angular_speed = 0.0f;                // 004DE1CD
    return desc;
}

DynWorldSettings dyn_world_construct_00c41ad0(const DynWorldDescriptor& desc) noexcept {
    DynWorldSettings world{};
    world.fixed_substep = desc.fixed_substep;                  // 00C41AF2
    world.gravity = desc.gravity;                              // 00C41AF7..00C41B03
    world.substep_budget = desc.substep_budget;                // 00C41B2E
    world.sleep_linear_speed = desc.sleep_linear_speed;        // 00C41B3A
    world.sleep_angular_speed = desc.sleep_angular_speed;      // 00C41B44
    world.sleep_countdown_reload = desc.sleep_countdown_reload;  // 00C41B4A
    // The remaining nine copies (00C41B09, 00C41B0F, 00C41B16, 00C41B1C, 00C41B22,
    // 00C41B28, 00C41B34) land in world+10h..+28h and world+38h, whose readers are in the
    // collision, group and LCP phases this packet does not cover. They are carried in the
    // descriptor so a later packet can name them without re-reading the constructor.
    return world;
}

DynWorldSettings dyn_world_settings_game() noexcept {
    return dyn_world_construct_00c41ad0(dyn_world_descriptor_004ddb90());
}

DynWorldStepConstants dyn_world_step_constants(const DynWorldSettings& settings) noexcept {
    DynWorldStepConstants step{};
    step.gravity = settings.gravity;
    step.sleep_linear_speed = settings.sleep_linear_speed;
    step.sleep_angular_speed = settings.sleep_angular_speed;
    step.sleep_countdown_reload = settings.sleep_countdown_reload;
    return step;
}

DynWorldSubstepPlan dyn_world_substep_plan(const DynWorldSettings& settings,
                                           float& accumulator, float dt) noexcept {
    // The native budget at world+34h is an int (00C5C63C MOV, 00C5C661 CMP dword,
    // 00C5C674 SUB dword). dyn_world_substep_plan_00c5c540 takes it as a float and
    // decrements it through an integer cast, which reproduces the same sequence for every
    // integral value passed by value; see the Corrections section of the doc.
    return dyn_world_substep_plan_00c5c540(settings.fixed_substep,
                                           static_cast<float>(settings.substep_budget),
                                           accumulator, dt);
}

}  // namespace bsp
