#include "bsp/native_pending_registry_getter.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_pending_registry_constructor.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
namespace {
struct NativeGuard {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, profile_00) == 0);
static_assert(offsetof(NativeGuard, section_04) == 4);
static_assert(std::is_trivial_v<NativeGuard>);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

volatile std::uint32_t& tracked_counter(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}
} // namespace

void* get_native_pending_registry_00875280(
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0) {
    void* const initial = actual_registry_publication_00f878cc;
    if (initial) {
        return initial;
    }

    void* const first_manager = get_native_singleton_manager_00415350(
        actual_manager_publication_01090aa0);
    auto* const captured_section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, captured_section};
    if (captured_section) {
        EnterCriticalSection(captured_section);
        auto& depth = tracked_counter(captured_section);
        depth = depth + 1u;
    }

    // Native state0 arms after Enter/increment, including the null-section path.
    try {
        if (!actual_registry_publication_00f878cc) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 8, 8});
            void* constructed;
            // Native state1 starts after allocation returns and ends before
            // publication. Constructor failure first performs its own cleanup.
            try {
                constructed = allocation
                    ? construct_native_pending_registry_00874bc0(
                        allocation, actual_registry_publication_00f878cc)
                    : nullptr;
            } catch (...) {
                singleton_lifetime_free(allocation);
                throw;
            }

            actual_registry_publication_00f878cc = constructed;
            void* const current_manager = get_native_singleton_manager_00415350(
                actual_manager_publication_01090aa0);
            void* const current_registry = actual_registry_publication_00f878cc;
            register_native_singleton_object_00bd0c30(
                current_manager, nullptr, current_registry);
        }

        // Keep guard cleanup active through normal Leave and the final reload.
        if (captured_section) {
            auto& depth = tracked_counter(captured_section);
            depth = depth - 1u;
            LeaveCriticalSection(captured_section);
        }
        return actual_registry_publication_00f878cc;
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(&guard);
        throw;
    }
}

} // namespace bsp
