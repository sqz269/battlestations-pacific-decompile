#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
// SOURCE identity, not a native task layout or an owning smart pointer. Caller
// supplies a stable live mapping; nonzero identities cannot be reused while any
// active/retired task reference or required callback can still reach them.
struct NativePilotBotTaskHandle { std::uintptr_t identity; };

// Borrowed SOURCE fields, not a binary-layout overlay. Count <= capacity;
// occupied entries and allocated storage must remain valid and disjoint.
// An empty zero-capacity array may have a null base before required allocation.
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

// Borrowed identity for the CURRENT plane's squadron+348h command controller,
// not a native layout or a command class. Its actual lifetime/context is external.
struct NativePilotBotCommandControllerHandle { std::uintptr_t identity; };

// REQUIRED observations bound to the same live owner. Slot34/38 answers are the
// native AL booleans; slot40 is the full EAX result. Each callback receives the
// captured task even if a preceding callback replaces the current head slot.
class NativePilotBotActiveRetirementCalls {
   public:
    virtual ~NativePilotBotActiveRetirementCalls() = default;
    // Actual bot+50h -> plane+9D4h -> squadron virtual+114h, whose established
    // profile entry007ECFD0 returns squadron+348h. Live plane/squadron required.
    virtual NativePilotBotCommandControllerHandle current_command_controller_114() = 0;
    // Actual0071BE40: mode1 reads+54h, mode2 reads+188h, other modes return0.
    // Answer the original interned singleton identity, not a command-class enum.
    virtual std::uint32_t current_command_token_0071be40(
        NativePilotBotCommandControllerHandle) = 0;
    virtual bool task_predicate_34(NativePilotBotTaskHandle) = 0;
    virtual bool task_predicate_38(NativePilotBotTaskHandle) = 0;
    virtual std::uint32_t task_state_40(NativePilotBotTaskHandle) = 0;
    // COMPLETE actual0099A170 contract is external: current controller/token,
    // descriptor/target probes, all admitted factory arms, actual live task arena,
    // and nonzero-result append0099A020. The lossy AttackCommandHost projection
    // cannot stand in for this provider. Native install does not drain retired.
    virtual void install_current_command_task_0099a170(
        NativePilotBotTaskOwnerView&, NativePilotBotTaskOwnerCalls&) = 0;
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

// Complete conditional normal SOURCE flow: original append ECX=bot/stack task,
// RET4; retire-leading ECX=bot/RET; retire-finished ECX=bot/RET or tail0099A170.
// The same successful disjoint allocation/representable2*n+2 domain applies.
// Providers return normally and cannot reenter these operations or change array
// bases/counts/capacities. A predicate may replace ONLY the current active head
// slot, keeping the captured and replacement task mappings live throughout the
// operation; this admitted SOURCE observation is not native mutation evidence.
// Leading retirement requires a valid nonnull head for slot34; finished retirement
// preserves its native null-head guard. No retirement path calls58/delete/drain.
// The explicitly invoked empty-list installer may append through0099A020; it
// must preserve the complete native producer contract and deferred retired FIFO.
// Other structural mutation, private EH/faults, ABI and pointee lifetimes excluded.
void append_native_pilot_bot_active_task_0099a020(
    NativePilotBotTaskOwnerView&, NativePilotBotTaskOwnerCalls&, NativePilotBotTaskHandle);
void retire_native_pilot_bot_leading_tasks_0099a0a0(
    NativePilotBotTaskOwnerView&, NativePilotBotTaskOwnerCalls&,
    NativePilotBotActiveRetirementCalls&);
void retire_native_pilot_bot_finished_tasks_0099a4c0(
    NativePilotBotTaskOwnerView&, NativePilotBotTaskOwnerCalls&,
    NativePilotBotActiveRetirementCalls&);

// Separate ONE-SHOT member cleanup: active delete1 WITHOUT58; embedded plans;
// retired array free WITHOUT retired task deletion; active array free; callback;
// base. Native pointer/count/capacity fields are not normalized afterward. This
// view/storage becomes unusable, and this API does not free the owner itself.
void destroy_native_pilot_bot_task_owner_members_0099a720(
    NativePilotBotTaskOwnerView&, NativePilotBotTaskOwnerCalls&);
}  // namespace bsp
