#include "bsp/native_diagnostic_sink_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native diagnostic sink reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
struct NativeGuard {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, section_04) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

std::uint32_t& tracked_counter(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<std::uint32_t*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}
} // namespace

void destroy_native_singleton_guard_00411ee0(void* actual_8byte_guard) {
    CRITICAL_SECTION* captured;
    std::memcpy(&captured, static_cast<std::byte*>(actual_8byte_guard) + 4, 4);
    const std::uint32_t profile = 0x00ce37fcu;
    std::memcpy(actual_8byte_guard, &profile, 4);
    if (captured) {
        --tracked_counter(captured);
        LeaveCriticalSection(captured);
    }
}

NativeDiagnosticSinkStorage* native_diagnostic_sink_get_or_create_004c14c0(
    NativeDiagnosticSinkStorage* volatile& actual_published_0109cf14,
    SoundLifetimeAccess actual_lifetime) {
    if (auto* owner = actual_published_0109cf14) return owner;

    auto* const captured = static_cast<CRITICAL_SECTION*>(
        actual_lifetime.get_manager_00415350().native_system_section_10());
    NativeGuard guard{0x00ce37fcu, captured};
    if (captured) {
        EnterCriticalSection(captured);
        ++tracked_counter(captured);
    }
    // Native state 0 arms only after entry and the counter increment. There is
    // no owned-allocation state: a registration exception leaves publication.
    try {
        if (!actual_published_0109cf14) {
            void* allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 4, sizeof(NativeDiagnosticSinkStorage)});
            auto* owner = allocation
                ? ::new (allocation) NativeDiagnosticSinkStorage : nullptr;
            if (owner) owner->native_vtable_00 = 0x00ce752cu;
            actual_published_0109cf14 = owner;
            auto manager = actual_lifetime.get_manager_00415350();
            manager->register_object(actual_published_0109cf14);
        }
        // The normal path uses the captured section directly and keeps state 0
        // armed through Leave. Its final publication read occurs after Leave.
        if (captured) {
            --tracked_counter(captured);
            LeaveCriticalSection(captured);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(&guard);
        throw;
    }
    return actual_published_0109cf14;
}

NativeDiagnosticSinkStorage* delete_native_diagnostic_sink_004bbca0(
    NativeDiagnosticSinkStorage& owner, std::uint32_t flags,
    NativeDiagnosticSinkStorage* volatile& actual_published_0109cf14) noexcept {
    const bool should_free = (flags & 1u) != 0;
    auto* const original_address = &owner;
    actual_published_0109cf14 = nullptr;
    owner.native_vtable_00 = 0x00ce3818u;
    if (should_free) {
        owner.~NativeDiagnosticSinkStorage();
        singleton_lifetime_free(original_address);
    }
    return original_address;
}

} // namespace bsp
