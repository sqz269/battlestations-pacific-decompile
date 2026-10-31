#include "bsp/native_gui_icon_delayed_loads_getter_storage.hpp"
#include "bsp/native_gui_icon_delayed_loads.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <new>
#include <type_traits>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(NativeGuiIconDelayedLoadsStorage) == 0x10);
static_assert(std::is_trivial_v<NativeGuiIconDelayedLoadsStorage>);
static_assert(std::is_trivial_v<NativeGuiIconDelayedLoadsGetterStorageFrame>);
Word bits(const void* pointer) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}
void* pointer_from_bits(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
Word read_word(const void* base, Word byte_offset) {
    Word result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void adjust_depth(void* actual_section, Word delta) {
    __asm {
        mov eax, actual_section
        mov edx, delta
        add dword ptr [eax + 18h], edx
    }
}
volatile Word& live(Word& word) { return word; }
} // namespace

NativeGuiIconDelayedLoadsStorage*
get_native_gui_icon_delayed_loads_storage_00ab5700(
    NativeGuiIconDelayedLoadsStorage* volatile& publication,
    void* volatile& manager_publication,
    NativeGuiIconDelayedLoadsGetterStorageFrame& frame,
    NativeGuiIconDelayedLoadsGetterStorageAcquired& acquired) {
    // Fresh/disjoint/retained metadata is a Source caller precondition, not a
    // Native receiver/profile/count/capacity guard or an Origin capability.
    auto& a = acquired.observation_;
    a.phase = NativeGuiIconDelayedLoadsGetterStorageAcquired::Phase::entered;
    try {
        auto* const initial = publication; // AB5715, capture even across callbacks.
        a.initial_read = true;
        a.initial_publication_bits = bits(initial);
        if (initial) { // AB5722: no current-owner or array admission here.
            a.fast_return = true;
            a.result_captured = true;
            a.returned_publication_bits = bits(initial);
            a.phase = NativeGuiIconDelayedLoadsGetterStorageAcquired::Phase::complete;
            return initial;
        }
        a.first_manager_entered = true;
        void* const first_manager = get_native_singleton_manager_00415350(manager_publication);
        a.first_manager_returned = true;
        a.first_manager_bits = bits(first_manager);
        void* const captured_section = pointer_from_bits(read_word(first_manager, 0x10)); // 572A
        a.section_captured = true;
        a.captured_first_section_bits = bits(captured_section);
        live(frame.guard_profile_home) = 0x00ce37fc; // 572F
        live(frame.guard_section_home) = bits(captured_section); // 5737
        a.guard_homes_written = true;
        if (captured_section) {
            a.enter_entered = true;
            EnterCriticalSection(static_cast<CRITICAL_SECTION*>(captured_section)); // 573E
            a.enter_returned = true;
            adjust_depth(captured_section, 1u); // 5744, CURRENT +18 DWORD.
            a.depth_incremented = true;
        }
        a.state0_reached = true; // 5748; observation, never EH permission.
        auto* const rechecked = publication; // 574C
        a.publication_rechecked = true;
        a.rechecked_publication_bits = bits(rechecked);
        if (!rechecked) {
            a.allocation_entered = true;
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x10, 0x10}); // 5756
            a.allocation_returned = true;
            a.allocated_bits = bits(allocation);
            auto* const created = allocation
                ? ::new (allocation) NativeGuiIconDelayedLoadsStorage : nullptr;
            if (created) {
                created->native_vtable_00 = 0x00d5c4b8; // 5762
                created->entries_04 = nullptr; // 5768
                created->count_08 = 0; // 576B
                created->capacity_0c = 0; // 576E
                a.object_stores_completed = true;
            }
            publication = created; // 5775, including a successful null allocation.
            a.publication_stored = true;
            a.second_manager_entered = true;
            void* const second_manager = get_native_singleton_manager_00415350(manager_publication); // 577A
            a.second_manager_returned = true;
            a.second_manager_bits = bits(second_manager);
            auto* const pushed_argument = publication; // 577F, AFTER second getter.
            a.registration_argument_captured = true;
            a.pushed_registration_argument_bits = bits(pushed_argument);
            a.registration_entered = true;
            register_native_singleton_object_00bd0c30(second_manager, nullptr, pushed_argument); // 5788
            a.registration_returned = true; // Not a registry/custody receipt.
        }
        if (captured_section) { // 578D: use captured ESI, not mutable guard+4.
            adjust_depth(captured_section, 0xffffffffu); // 5791
            a.depth_decremented = true;
            a.leave_entered = true;
            LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(captured_section)); // 5796
            a.leave_returned = true;
        }
        auto* const result = publication; // 579C, late current return after leave.
        a.result_captured = true;
        a.returned_publication_bits = bits(result);
        a.phase = NativeGuiIconDelayedLoadsGetterStorageAcquired::Phase::complete;
        return result;
    } catch (...) {
        a.phase = NativeGuiIconDelayedLoadsGetterStorageAcquired::Phase::unknown_source_escape;
        // Retain reached guard/depth/allocation/publication/registration debt.
        // No local cleanup/leave/EH411EE0, automatic failed-frame destruction,
        // compensation, retry, exception-origin assertion or new terminal policy.
        throw;
    }
}
} // namespace bsp
