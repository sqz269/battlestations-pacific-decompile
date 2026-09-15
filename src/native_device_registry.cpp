#include "bsp/native_device_registry.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native device registry reconstruction requires MSVC Win32.
#endif

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

NativeDeviceRegistryStorage* volatile device_registry_publication_00e17bf4 = nullptr;

} // namespace

NativeDeviceRegistryStorage* get_native_device_registry_00441780(
    void* volatile& actual_manager_publication_01090aa0,
    NativeDeviceRegistryStorage* volatile& actual_registry_publication_00e17bf4) {
    NativeDeviceRegistryStorage* const initial = actual_registry_publication_00e17bf4;
    if (initial != nullptr) {
        return initial;
    }

    void* const first_manager = get_native_singleton_manager_00415350(
        actual_manager_publication_01090aa0);
    auto* const captured_section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, captured_section};
    if (captured_section != nullptr) {
        EnterCriticalSection(captured_section);
        auto& depth = tracked_counter(captured_section);
        depth = depth + 1u;
    }

    // Native state0 begins only after Enter and the raw +18 increment.
    try {
        if (actual_registry_publication_00e17bf4 == nullptr) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x10, 0x10});
            auto* fresh = static_cast<NativeDeviceRegistryStorage*>(allocation);
            if (fresh != nullptr) {
                fresh->native_vtable_00 = kNativeDeviceRegistryDeletingProfile;
                fresh->classes_04.data_00 = 0;
                fresh->classes_04.count_04 = 0;
                fresh->classes_04.capacity_08 = 0;
            }

            actual_registry_publication_00e17bf4 = fresh;
            void* const current_manager = get_native_singleton_manager_00415350(
                actual_manager_publication_01090aa0);
            NativeDeviceRegistryStorage* const current_registry =
                actual_registry_publication_00e17bf4;
            register_native_singleton_object_00bd0c30(
                current_manager, nullptr, current_registry);
        }

        if (captured_section != nullptr) {
            auto& depth = tracked_counter(captured_section);
            depth = depth - 1u;
            LeaveCriticalSection(captured_section);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(&guard);
        throw;
    }
    return actual_registry_publication_00e17bf4;
}

void unwind_native_device_registry_0043ebf0(
    NativeDeviceRegistryStorage& owner,
    NativeDeviceRegistryStorage* volatile& actual_registry_publication_00e17bf4) noexcept {
    actual_registry_publication_00e17bf4 = nullptr;
    owner.native_vtable_00 = kNativeSingletonBaseProfile;
}

void destroy_native_device_registry_00441360(
    NativeDeviceRegistryStorage& owner,
    NativeDeviceRegistryStorage* volatile& actual_registry_publication_00e17bf4) {
    try {
        resize_native_device_registry_array_00440180(owner.classes_04, 0);
        singleton_lifetime_free(reinterpret_cast<void*>(
            static_cast<std::uintptr_t>(owner.classes_04.data_00)));
    } catch (...) {
        unwind_native_device_registry_0043ebf0(
            owner, actual_registry_publication_00e17bf4);
        throw;
    }
    unwind_native_device_registry_0043ebf0(
        owner, actual_registry_publication_00e17bf4);
}

NativeDeviceRegistryStorage* delete_native_device_registry_00441840(
    NativeDeviceRegistryStorage* owner, std::uint32_t flags,
    NativeDeviceRegistryStorage* volatile& actual_registry_publication_00e17bf4) {
    destroy_native_device_registry_00441360(
        *owner, actual_registry_publication_00e17bf4);
    if ((flags & 1u) != 0) {
        singleton_lifetime_free(owner);
    }
    return owner;
}

NativeDeviceRegistryStorage* volatile& process_native_device_registry_00e17bf4() noexcept {
    return device_registry_publication_00e17bf4;
}

NativeDeviceRegistryStorage* get_process_native_device_registry_00441780(
    void* volatile& actual_manager_publication_01090aa0) {
    return get_native_device_registry_00441780(
        actual_manager_publication_01090aa0,
        process_native_device_registry_00e17bf4());
}

} // namespace bsp
