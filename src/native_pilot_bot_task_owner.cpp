#include "bsp/native_pilot_bot_task_owner.hpp"

namespace bsp {

void retire_all_native_pilot_bot_tasks_00999e40(
    NativePilotBotTaskOwnerView& owner, NativePilotBotTaskOwnerCalls& calls) {
    auto& active = owner.active_58_5c_60;
    auto& retired = owner.retired_64_68_6c;
    for (std::uint32_t index = 0; index < active.count; ++index) {
        if (retired.count == retired.capacity) {
            //00999E5C/60 publishes2*n+2 BEFORE the allocation at00999E68.
            retired.capacity = retired.capacity * 2u + 2u;
            const NativePilotBotTaskArrayAllocation request{
                retired.capacity, retired.capacity * 4u,
                static_cast<std::size_t>(retired.capacity) * sizeof(NativePilotBotTaskHandle)};
            auto* replacement = calls.allocate_task_array_00bf55be(request);
            for (std::uint32_t i = 0; i < retired.count; ++i) replacement[i] = retired.entries[i];
            if (retired.entries != nullptr) calls.free_task_array_00bf6989(retired.entries); //00999E9F
            retired.entries = replacement; //00999EA7, after CRT free returns
        }
        retired.entries[retired.count] = active.entries[index]; //00999EB7/B9
        ++retired.count;                                       //00999EBB
    }
    if (active.count != 0) active.count = 0; //00999ED6; no hook or task deletion
}

void drain_native_pilot_bot_retired_tasks_00999ee0(
    NativePilotBotTaskOwnerView& owner, NativePilotBotTaskOwnerCalls& calls) {
    auto& retired = owner.retired_64_68_6c;
    while (retired.count != 0) {
        //00999EF3..EFA unconditionally invokes the valid current head's+58h.
        calls.task_hook_58(retired.entries[0]);
        //00999EFC/FF RELOAD the array/head after the hook, before its null guard.
        const NativePilotBotTaskHandle head = retired.entries[0];
        if (head.identity != 0) calls.task_scalar_delete_00(head, 1); //00999F09/0B
        for (std::uint32_t i = 0; i + 1u < retired.count; ++i) {
            retired.entries[i] = retired.entries[i + 1u]; //00999F20/23
        }
        --retired.count; //00999F36; native does not clear the unused tail slot
    }
}

void disable_and_drain_native_pilot_bot_tasks_0099a830(
    NativePilotBotTaskOwnerView& owner, NativePilotBotTaskOwnerCalls& calls) {
    owner.disabled_10 = 1; //0099A833
    owner.enabled_11 = 0;  //0099A837
    retire_all_native_pilot_bot_tasks_00999e40(owner, calls);    //0099A83B
    drain_native_pilot_bot_retired_tasks_00999ee0(owner, calls); //0099A843 tail JMP
}

void destroy_native_pilot_bot_task_owner_members_0099a720(
    NativePilotBotTaskOwnerView& owner, NativePilotBotTaskOwnerCalls& calls) {
    calls.enter_member_destructor_profiles_0099a742();
    auto& active = owner.active_58_5c_60;
    for (std::uint32_t i = 0; i < active.count; ++i) {
        const NativePilotBotTaskHandle task = active.entries[i];
        if (task.identity == 0) continue; //0099A769/76E
        calls.task_scalar_delete_00(task, 1); //0099A776; no+58h call
        active.entries[i] = {0};             //0099A778
    }
    calls.destroy_embedded_plans_00bf7c6e(0x1c, 2, 0x0099a010); //0099A7A3; RET10h helper
    if (owner.retired_64_68_6c.entries != nullptr) {
        calls.free_task_array_00bf6989(owner.retired_64_68_6c.entries); //0099A7B0
    }
    if (active.entries != nullptr) calls.free_task_array_00bf6989(active.entries); //0099A7C0
    calls.destroy_callback_owner_00695870(); //0099A7D2, owner+1Ch
    calls.base_cleanup_00875b30();           //0099A7E1, owner base
    // The native leaves array publications/counts/capacities untouched.
}
}  // namespace bsp
