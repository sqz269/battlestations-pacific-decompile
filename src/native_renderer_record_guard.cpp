#include "bsp/native_renderer_record_guard.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include "bsp/random_threads.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstring>
#include <exception>

namespace bsp {
namespace {
template<class T> T load(const void* base, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(base) + offset, sizeof value);
    return value;
}
template<class T> void store(void* base, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(base) + offset, &value, sizeof value);
}
struct NativeGuard {
    std::uint32_t profile;
    TrackedCriticalSection* section;
};
static_assert(sizeof(NativeGuard) == 8);
static_assert(offsetof(TrackedCriticalSection, depth) == 0x18);
static_assert(sizeof(TrackedCriticalSection) == 0x1c);

void adjust_depth(TrackedCriticalSection* section, std::uint32_t delta) noexcept {
    store(section, 0x18, load<std::uint32_t>(section, 0x18) + delta);
}
} // namespace

void destroy_native_renderer_record_guard_base_00b22360(
    void* owner, NativeRendererRecordGuardContext& context) noexcept {
    context.actual_guard_0108d5a0 = nullptr;
    store(owner, 0, std::uint32_t{0x00ce3818});
}

void* construct_native_renderer_record_guard_00b23750(
    void* owner, NativeRendererRecordGuardContext& context) {
    // Native constructor state0 precedes the derived profile store.
    try {
        store(owner, 0, std::uint32_t{0x00d5e60c});
        auto* section = create_native_tracked_critical_section_00bd1860();
        store(owner, 4, section);
    } catch (...) {
        destroy_native_renderer_record_guard_base_00b22360(owner, context);
        throw;
    }
    return owner;
}

void* get_native_renderer_record_guard_00b25be0(
    NativeRendererRecordGuardContext& context) {
    void* const first = context.actual_guard_0108d5a0;
    if (first != nullptr) return first;

    void* const manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    auto* const section = load<TrackedCriticalSection*>(manager, 0x10);
    NativeGuard guard{0x00ce37fc, section};
    if (section != nullptr) {
        EnterCriticalSection(&section->native);
        adjust_depth(section, 1);
    }
    // Native state0 arms after entry. State1 covers only construction after a
    // successful allocation return; publication and registration use state0.
    try {
        if (context.actual_guard_0108d5a0 == nullptr) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 8, 8});
            void* owner = nullptr;
            try {
                if (allocation != nullptr) {
                    owner = construct_native_renderer_record_guard_00b23750(
                        allocation, context);
                }
            } catch (...) {
                singleton_lifetime_free(allocation);
                throw;
            }
            context.actual_guard_0108d5a0 = owner;
            void* const current_manager = get_native_singleton_manager_00415350(
                context.actual_manager_01090aa0);
            void* const current_owner = context.actual_guard_0108d5a0;
            register_native_singleton_object_00bd0c30(
                current_manager, nullptr, current_owner);
        }
        if (section != nullptr) {
            adjust_depth(section, std::uint32_t{0xffffffff});
            LeaveCriticalSection(&section->native);
        }
    } catch (...) {
        // Native state0 remains armed through normal LeaveCriticalSection.
        // A second source C++ exception while unwinding cannot replace the first.
        try {
            destroy_native_singleton_guard_00411ee0(&guard);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    return context.actual_guard_0108d5a0;
}

void* delete_native_renderer_record_guard_00b26130(
    void* owner, std::uint32_t flags, NativeRendererRecordGuardContext& context) {
    store(owner, 0, std::uint32_t{0x00d5e60c});
    auto** const section_slot = reinterpret_cast<TrackedCriticalSection**>(
        static_cast<std::byte*>(owner) + 4);
    release_native_tracked_critical_section_0041cc80(section_slot);
    const bool free_owner = (flags & 1u) != 0;
    destroy_native_renderer_record_guard_base_00b22360(owner, context);
    if (free_owner) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
