#include "bsp/native_land_task_constructor.hpp"

namespace bsp {

void bind_native_land_approach_to_nonnull_task_009f9980(
    const NativeLandTaskConstructorView& task) noexcept {
    task.command_alias_410 = task.command_004;  // 009F998F
    task.gun_alias_414 = task.gun_314;          // 009F999D
    task.control_alias_418 = task.control_38c; // 009F99A0
}

NativePilotBotTaskHandle construct_native_land_task_009b3240(
    const NativeLandTaskConstructorView& task, const void* owner,
    const void* original_block, NativeLandTaskExecutableProfiles profiles,
    NativeLandTaskConstructorCalls& calls) {
    const NativePilotBotTaskHandle original_task = task.task;
    calls.construct_base_0099c6f0(task, owner, 3u);       // 009B3265
    const void* const plane = calls.owner_plane_50(owner); // 009B326A
    calls.construct_composite_009b2e50(task, plane, original_block); // 009B3283

    const void* const control_plane = task.retained.plane_3fc; // 009B3288
    task.task_profile_000 = profiles.task_00d1ffa0;       // 009B3294
    task.approach_profile_3f8 = profiles.approach_00d1ff94; // 009B329A
    task.registry_profile_4b0 = profiles.registry_00d1ff90; // 009B32A0

    const void* state;
    if (!calls.plane_embedded_72c_predicate_38(control_plane)) { // 009B32B4
        state = task.park_620;                          // 009B32BA
    } else {
        const void* const leader_plane = task.retained.plane_3fc; // 009B32C2
        state = calls.plane_is_leader_007b8ad0(leader_plane) // 009B32C8
            ? task.moveto_4c4 : task.follow_500;         // 009B32CF/D7
    }
    task.current_state_310 = state;                    // 009B32DD
    calls.enter_state_04(state);                       // 009B32E8
    bind_native_land_approach_to_nonnull_task_009f9980(task); // 009B32ED
    return original_task;                             // 009B32F7
}

} // namespace bsp
