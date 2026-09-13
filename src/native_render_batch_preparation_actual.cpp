#include "bsp/native_render_batch_preparation_actual.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include "bsp/native_render_pointer_slot_sort.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
struct NativeGuard {
    Word profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4 && sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, section_04) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
volatile Word& depth(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile Word*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(NativeGuard& guard) noexcept {
    __try {
        destroy_native_singleton_guard_00411ee0(&guard);
    } __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct GuardCleanup {
    NativeGuard& guard;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard); }
};
struct AllocationCleanup {
    void* allocation;
    bool armed = true;
    ~AllocationCleanup() noexcept { if (armed) singleton_lifetime_free(allocation); }
};
} // namespace

NativeRenderCommandQueueStorage* get_native_render_command_queue_004c11f0(
    NativeRenderBatchPreparationContext& context) {
    auto& publication = context.actual_queue_constructor.actual_queue_00f8d440;
    auto* const initial = publication;
    if (initial) return initial;

    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    auto* const section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        depth(section) = depth(section) + 1u;
    }
    GuardCleanup cleanup{guard}; // C64F00/state0 armed after Enter/depth.
    if (!publication) {
        void* const allocation = singleton_lifetime_allocate({
            SingletonAllocationKind::object, 0x34, 0x34});
        AllocationCleanup allocation_cleanup{allocation}; // C64F08/state1.
        // Begin the trivial C++17 lifetime without value initialization: the
        // full constructor must still observe the allocation's old control_20.
        auto* const storage = allocation
            ? ::new (allocation) NativeRenderCommandQueueStorage
            : nullptr;
        auto* const constructed = allocation
            ? construct_native_render_queue_00b1f280(
                *storage, context.actual_queue_constructor)
            : nullptr;
        allocation_cleanup.armed = false; // Restore state0 BEFORE publication.
        publication = constructed;
        void* const manager = get_native_singleton_manager_00415350(
            context.actual_manager_01090aa0);
        auto* const current_queue = publication;
        register_native_singleton_object_00bd0c30(manager, nullptr, current_queue);
    }
    if (section) {
        depth(section) = depth(section) - 1u;
        LeaveCriticalSection(section);
    }
    cleanup.armed = false;
    return publication;
}

void prepare_native_render_batch_actual_00b51df0(NativeRenderBatchStorage& batch,
    volatile Word& argument_slot, NativeRenderBatchPreparationContext& context) {
    const Word original_index = argument_slot;
    volatile std::uint8_t enabled = 0;
    auto* const queue = get_native_render_command_queue_004c11f0(context);
    (void)read_native_render_batch_sort_configuration_00b1cb30(
        *queue, original_index, enabled, argument_slot);
    if (!enabled) return;

    const volatile auto& actual = batch;
    Word first;
    Word last;
    NativeRenderPointerSlotComparator comparator;
    if (original_index == 0) {
        prepare_native_render_batch_keys_00b51df0(batch);
        first = reinterpret_cast<Word>(actual.entries_0c);
        const Word count = static_cast<Word>(actual.count_10);
        last = first + count * 4u;
        comparator = native_render_entry_unsigned_key_less_00b51b00;
    } else {
        const Word count = static_cast<Word>(actual.count_10);
        const Word end_base = reinterpret_cast<Word>(actual.entries_0c);
        last = end_base + count * 4u;
        first = reinterpret_cast<Word>(actual.entries_0c);
        comparator = native_render_entry_material_depth_less_00b51ab0;
    }
    const auto ideal = static_cast<std::int32_t>(last - first) >> 2;
    sort_native_render_pointer_slots_00b1dce0(
        reinterpret_cast<void*>(first), reinterpret_cast<void*>(last), ideal, comparator);
}

void execute_native_render_preparation_job_00b1bf70(volatile Word& argument_slot,
    NativeRenderBatchPreparationContext& context) {
    auto* const batch = reinterpret_cast<NativeRenderBatchStorage*>(argument_slot);
    const volatile auto& actual = *batch;
    const Word profile = actual.native_vtable_00;
    const Word mode = actual.mode_08;
    const volatile Word* table;
    if (profile == 0x00d5e5acu) table = context.actual_batch_profile_00d5e5ac;
    else if (profile == 0x00d62064u) table = context.actual_base_batch_profile_00d62064;
    else throw std::invalid_argument("Missing current native batch preparation profile");
    if (!table || table[3] != 0x00b51df0u)
        throw std::invalid_argument("Missing current native batch preparation callback binding");
    argument_slot = mode;
    prepare_native_render_batch_actual_00b51df0(*batch, argument_slot, context);
}

NativeRenderPreparationJobDispatch::NativeRenderPreparationJobDispatch(
    NativeRenderBatchPreparationContext& context, const volatile Word* profile,
    NativeFrameJobDispatch& remaining)
    : context_(context), job_profile_(profile), remaining_(remaining) {
    if (!profile || profile[0] != 0x00b1bf70u)
        throw std::invalid_argument("Preparation jobs require the actual D5E160 primary table");
}
void NativeRenderPreparationJobDispatch::execute_current_00(void* owner, Word argument) {
    const Word profile = *static_cast<const volatile Word*>(owner);
    if (profile != 0x00d5e160u) {
        remaining_.execute_current_00(owner, argument);
        return;
    }
    if (job_profile_[0] != 0x00b1bf70u)
        throw std::invalid_argument("Missing current native preparation job callback binding");
    execute_native_render_preparation_job_00b1bf70(argument, context_);
}
} // namespace bsp
