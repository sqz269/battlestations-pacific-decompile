#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
// SOURCE identity, not a native task layout or an owning smart pointer. Caller
// supplies a stable live mapping; nonzero identities cannot be reused while any
// active/retired task reference or required callback can still reach them.
struct NativePilotBotTaskHandle { std::uintptr_t identity; };

// Borrowed SOURCE fields, not a binary-layout overlay. Count <= capacity;
// occupied entries and both storage arrays must remain valid and disjoint.
struct NativePilotBotTaskArrayView {
    NativePilotBotTaskHandle*& entries;
    std::uint32_t& count;
    std::uint32_t& capacity;
};
struct NativePilotBotTaskOwnerView {
    std::uint8_t& disabled_10;
    std::uint8_t& enabled_11;
    NativePilotBotTaskArrayView active_58_5c_60;
    NativePilotBotTaskArrayView retired_64_68_6c;
};
struct NativePilotBotTaskArrayAllocation {
    std::uint32_t capacity;
    std::uint32_t native_bytes;  // four native pointer bytes per entry
    std::size_t host_bytes;     // actual SOURCE handle storage
};

// Every service is REQUIRED and bound to this same owner. No task layout,
// allocator, observer, native profile or cleanup substitute is supplied.
class NativePilotBotTaskOwnerCalls {
   public:
    virtual ~NativePilotBotTaskOwnerCalls() = default;
    virtual NativePilotBotTaskHandle* allocate_task_array_00bf55be(
        const NativePilotBotTaskArrayAllocation&) = 0;
    virtual void free_task_array_00bf6989(NativePilotBotTaskHandle*) = 0;
    virtual void task_hook_58(NativePilotBotTaskHandle) = 0;
    virtual void task_scalar_delete_00(NativePilotBotTaskHandle, std::uint32_t flags) = 0;
    // Native0099A742/748 stamps owner00D1F348 and callback subobject00D1F32C.
    // Actual profile/layout publication is external to this source facade.
    virtual void enter_member_destructor_profiles_0099a742() = 0;
    // Actual owner+84h storage, reverse order, two1Ch records; dtor0099A010
    // tail-jumps004B7EF0. Existing game-array adapter excludes this pair.
    virtual void destroy_embedded_plans_00bf7c6e(std::uint32_t stride,
        std::uint32_t count, std::uint32_t native_destructor) = 0;
    virtual void destroy_callback_owner_00695870() = 0;  // actual owner+1Ch
    virtual void base_cleanup_00875b30() = 0;           // actual owner base
};

// Complete conditional normal control flow, original ECX=bot, plain RET (830
// tail-jumps EE0). New C++ signatures are not original entry/profile ABI.
// Allocation must succeed with disjoint live storage. Capacity2*n+2 and its
// four-byte native request must be representable. Callbacks must return normally
// and cannot reenter these methods or change array base/count/capacity. A hook
// may replace/clear ONLY the current retired head; the scalar target is reloaded
// afterward. Its replacement must be a valid live handle. Other structural
// mutation, faults, exceptions/private EH and object-lifetime proofs are excluded.
void retire_all_native_pilot_bot_tasks_00999e40(
    NativePilotBotTaskOwnerView&, NativePilotBotTaskOwnerCalls&);
void drain_native_pilot_bot_retired_tasks_00999ee0(
    NativePilotBotTaskOwnerView&, NativePilotBotTaskOwnerCalls&);
void disable_and_drain_native_pilot_bot_tasks_0099a830(
    NativePilotBotTaskOwnerView&, NativePilotBotTaskOwnerCalls&);

// Separate ONE-SHOT member cleanup: active delete1 WITHOUT58; embedded plans;
// retired array free WITHOUT retired task deletion; active array free; callback;
// base. Native pointer/count/capacity fields are not normalized afterward. This
// view/storage becomes unusable, and this API does not free the owner itself.
void destroy_native_pilot_bot_task_owner_members_0099a720(
    NativePilotBotTaskOwnerView&, NativePilotBotTaskOwnerCalls&);
}  // namespace bsp
