#include "bsp/native_pilot_bot_task_owner.hpp"
#include "bsp/command_execution.hpp"

namespace bsp {
namespace {
// Shared SOURCE copy of the native normal array-growth block. All admitted
// arrays/placement destinations are live; native null/fault allocation excluded.
void grow_full_task_array(NativePilotBotTaskArrayView& array,
    NativePilotBotTaskOwnerCalls& calls) {
    if (array.count != array.capacity) return;
    array.capacity = array.capacity * 2u + 2u; // publish before allocation
    const NativePilotBotTaskArrayAllocation request{
        array.capacity, array.capacity * 4u,
        static_cast<std::size_t>(array.capacity) * sizeof(NativePilotBotTaskHandle)};
    auto* replacement = calls.allocate_task_array_00bf55be(request);
    for (std::uint32_t i = 0; i < array.count; ++i) replacement[i] = array.entries[i];
    if (array.entries != nullptr) calls.free_task_array_00bf6989(array.entries);
    array.entries = replacement; // after actual free returns
}

void retire_current_active_head(NativePilotBotTaskOwnerView& owner,
    NativePilotBotTaskOwnerCalls& calls) {
    auto& active = owner.active_58_5c_60;
    auto& retired = owner.retired_64_68_6c;
    //0099A0CA/0099A548 captures the active BASE after the task predicates and
    // before possible retired allocation.0099A128/0099A5A6 later reads its head.
    auto* const active_base = active.entries;
    grow_full_task_array(retired, calls);
    retired.entries[retired.count] = active_base[0];
    ++retired.count; //0099A12C/0099A5AA
    for (std::uint32_t i = 0; i + 1u < active.count; ++i) {
        active.entries[i] = active.entries[i + 1u]; //0099A140/0099A5C0
    }
    --active.count; //0099A156/0099A5D6; unused tail remains untouched
}
}  // namespace

void append_native_pilot_bot_active_task_0099a020(
    NativePilotBotTaskOwnerView& owner, NativePilotBotTaskOwnerCalls& calls,
    NativePilotBotTaskHandle task) {
    auto& active = owner.active_58_5c_60;
    //0099A02F publishes2*n+2;0099A038 allocate;0099A06F free;0099A077 publish.
    grow_full_task_array(active, calls);
    active.entries[active.count] = task; //0099A08C
    ++active.count;                     //0099A08E
}

void prepend_native_pilot_bot_active_task_00999f50(
    NativePilotBotTaskOwnerView& owner, NativePilotBotTaskOwnerCalls& calls,
    NativePilotBotTaskHandle task) {
    auto& active = owner.active_58_5c_60;
    if (active.count == active.capacity) {
        // This is not the append growth helper: capacity2*n+1, incoming first,
        // and count-before-base publication are distinct native observations.
        active.capacity = active.capacity * 2u + 1u; //00999F61, before allocate
        const NativePilotBotTaskArrayAllocation request{
            active.capacity, active.capacity * 4u,
            static_cast<std::size_t>(active.capacity) * sizeof(NativePilotBotTaskHandle)};
        auto* replacement = calls.allocate_task_array_00bf55be(request); //00999F6A
        replacement[0] = task; //00999F78/7C, incoming stack task before old prefix
        for (std::uint32_t i = 0; i < active.count; ++i) {
            replacement[i + 1u] = active.entries[i]; //00999F94..9C
        }
        if (active.entries != nullptr) calls.free_task_array_00bf6989(active.entries); //00999FB2
        ++active.count;                //00999FBA, after ADD ESP4 at00999FB7
        active.entries = replacement; //00999FBE, AFTER count publication
    } else {
        // REQUIRES count>0 in this valid spare-capacity domain. Native00999FD0
        // reads the old tail first, and00999FD8 underflows for count0; no repair.
        active.entries[active.count] = active.entries[active.count - 1u]; //00999FD0/D3
        std::uint32_t index = active.count - 1u;                         //00999FD5/D8
        while (index != 0) {
            active.entries[index] = active.entries[index - 1u]; //00999FE9/EC
            --index;
        }
        active.entries[0] = task; //00999FF3/F7, incoming stack task written last
        ++active.count;           //00999FF9
    }
}

void retire_native_pilot_bot_leading_tasks_0099a0a0(
    NativePilotBotTaskOwnerView& owner, NativePilotBotTaskOwnerCalls& calls,
    NativePilotBotActiveRetirementCalls& task_calls) {
    auto& active = owner.active_58_5c_60;
    while (active.count != 0) {
        const NativePilotBotTaskHandle head = active.entries[0];
        if (!task_calls.task_predicate_34(head)) break; //0099A0BA, no null guard
        retire_current_active_head(owner, calls);
    }
}

void retire_native_pilot_bot_finished_tasks_0099a4c0(
    NativePilotBotTaskOwnerView& owner, NativePilotBotTaskOwnerCalls& calls,
    NativePilotBotActiveRetirementCalls& task_calls) {
    const auto controller = task_calls.current_command_controller_114(); //0099A4D4
    if (task_calls.current_command_token_0071be40(controller) == kCommandStop) { //0099A4D8/DD
        retire_native_pilot_bot_leading_tasks_0099a0a0(owner, calls, task_calls); //0099A4E6
    }
    auto& active = owner.active_58_5c_60;
    while (active.count != 0) {
        const NativePilotBotTaskHandle captured = active.entries[0]; //0099A500
        if (captured.identity == 0) break;                          //0099A502/504
        if (!task_calls.task_predicate_38(captured)) break;         //0099A511/515
        if (task_calls.task_predicate_34(captured)) break;          //0099A522/526
        if (active.count <= 1u && task_calls.task_state_40(captured) == 1u) break; //0099A52C..53C
        retire_current_active_head(owner, calls); // reload head, not captured predicate task
    }
    if (active.count == 0) {
        task_calls.install_current_command_task_0099a170(owner, calls); //0099A5EB tail
    }
}

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
