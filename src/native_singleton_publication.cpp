#include "bsp/native_singleton_publication.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_resource_registry_construction.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_singleton_vector_reserve_construct.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {

struct NativeGuard {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, section_04) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

volatile std::uint32_t& tracked_counter(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}

} // namespace

__declspec(noinline) void* get_native_singleton_manager_00415350(
    void* volatile& actual_manager_publication_01090aa0) {
    void* const initial = actual_manager_publication_01090aa0;
    if (initial) {
        return initial;
    }

    // Allocation occurs before native state0 is armed. The saved allocation
    // remains the cleanup target even if construction changes publication.
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::manager, 0x14, 0x14});
    try {
        void* const result = allocation
            ? construct_native_singleton_manager_00bd0960(allocation, nullptr)
            : nullptr;
        actual_manager_publication_01090aa0 = result;
        return result;
    } catch (...) {
        singleton_lifetime_free(allocation);
        throw;
    }
}

__declspec(noinline) void* get_native_resource_registry_00b1b730(
    void* volatile& actual_manager_publication_01090aa0,
    void* volatile& actual_registry_publication_00f8d41c) {
    void* const initial = actual_registry_publication_00f8d41c;
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

    // Native guard state0 starts after Enter and the raw +18 increment.
    try {
        if (!actual_registry_publication_00f8d41c) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x10, 0x10});
            // Native state1 is armed only after allocation returns. It ends
            // before publication and the second lookup/registration calls.
            try {
                if (allocation) {
                    construct_native_resource_registry_00b1aa70(
                        allocation, actual_registry_publication_00f8d41c);
                    *static_cast<volatile std::uint32_t*>(allocation) = 0x00d5e59cu;
                }
            } catch (...) {
                singleton_lifetime_free(allocation);
                throw;
            }

            actual_registry_publication_00f8d41c = allocation;
            void* const current_manager = get_native_singleton_manager_00415350(
                actual_manager_publication_01090aa0);
            void* const current_registry = actual_registry_publication_00f8d41c;
            register_native_singleton_object_00bd0c30(
                current_manager, nullptr, current_registry);
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
    return actual_registry_publication_00f8d41c;
}

} // namespace bsp
