#include "bsp/native_front_end_frame_publication.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_front_end_frame_construction.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {
struct NativeFrameGuard {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeFrameGuard) == 8);
static_assert(offsetof(NativeFrameGuard, section_04) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

volatile std::uint32_t& tracked_counter(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}
} // namespace

__declspec(noinline) void* get_native_front_end_frame_004c1ac0(
    void* volatile& actual_manager_publication_01090aa0,
    void* volatile& actual_frame_publication_00e18d80) {
    void* const initial = actual_frame_publication_00e18d80;
    if (initial) return initial;

    void* const first_manager = get_native_singleton_manager_00415350(
        actual_manager_publication_01090aa0);
    auto* const captured_section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeFrameGuard guard{0x00ce37fcu, captured_section};
    if (captured_section) {
        EnterCriticalSection(captured_section);
        auto& depth = tracked_counter(captured_section);
        depth = depth + 1u;
    }

    // Native state0 starts only after Enter and the physical +18 increment.
    try {
        if (!actual_frame_publication_00e18d80) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x164, 0x164});
            void* constructed;
            // Native state1 starts after allocation and ends before publish.
            try {
                constructed = allocation
                    ? construct_native_front_end_frame_00517d10(allocation)
                    : nullptr;
            } catch (...) {
                singleton_lifetime_free(allocation);
                throw;
            }

            actual_frame_publication_00e18d80 = constructed;
            void* const current_frame = actual_frame_publication_00e18d80;
            void* const adjusted = current_frame
                ? static_cast<void*>(static_cast<std::byte*>(current_frame) + 8)
                : nullptr;
            void* const second_manager = get_native_singleton_manager_00415350(
                actual_manager_publication_01090aa0);
            register_native_singleton_object_00bd0c30(
                second_manager, nullptr, adjusted);
        }
        if (captured_section) {
            auto& depth = tracked_counter(captured_section);
            depth = depth - 1u;
            LeaveCriticalSection(captured_section);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(&guard);
        throw;
    }
    return actual_frame_publication_00e18d80;
}

} // namespace bsp
