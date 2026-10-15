#include "bsp/native_avoid_singleton.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {

struct NativeCriticalGuardPrefix {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4 && sizeof(NativeCriticalGuardPrefix) == 8);
static_assert(offsetof(NativeCriticalGuardPrefix, section_04) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

std::uint32_t load_word(const void* storage, std::size_t offset = 0) noexcept {
    _ReadWriteBarrier();
    std::uint32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + offset, sizeof(value));
    _ReadWriteBarrier();
    return value;
}
void store_word(void* storage, std::size_t offset, std::uint32_t value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(static_cast<unsigned char*>(storage) + offset, &value, sizeof(value));
    _ReadWriteBarrier();
}
Cc13AvoidZoneManagerNative* load_avoid_publication(
    const NativeAvoidSingletonContext& context) noexcept {
    return reinterpret_cast<Cc13AvoidZoneManagerNative*>(
        load_word(context.actual_00e17624_publication_cell));
}

struct CurrentCriticalUnwindGuard {
    NativeCriticalGuardPrefix* prefix;
    bool active = true;
    ~CurrentCriticalUnwindGuard() noexcept {
        if (active) destroy_native_singleton_guard_00411ee0(prefix);
    }
};
struct CurrentAllocationUnwindGuard {
    void* allocation;
    bool active = true;
    ~CurrentAllocationUnwindGuard() noexcept {
        if (active) singleton_lifetime_free(allocation);
    }
};

} // namespace

__declspec(noinline) Cc13AvoidZoneManagerNative* __cdecl
get_native_avoid_manager_004218e0(const NativeAvoidSingletonContext& context) {
    auto* const initial = load_avoid_publication(context);
    if (initial != nullptr) return initial;

    void* const first_manager = get_native_singleton_manager_00415350(
        *context.actual_01090aa0_publication_cell);
    auto* const captured_section = reinterpret_cast<CRITICAL_SECTION*>(
        load_word(first_manager, 0x10));
    NativeCriticalGuardPrefix prefix{0x00ce37fc, captured_section};
    if (captured_section != nullptr) {
        EnterCriticalSection(captured_section);
        store_word(captured_section, 0x18, load_word(captured_section, 0x18) + 1u);
    }

    // Native state0 is armed only after successful enter and depth increment.
    CurrentCriticalUnwindGuard critical_cleanup{&prefix};
    if (load_avoid_publication(context) == nullptr) {
        // Allocation precedes Native state1. A throwing allocator leaves only
        // the captured critical guard active; no unreturned block is freed.
        void* const allocation = singleton_lifetime_allocate({
            SingletonAllocationKind::object, 0x78, 0x78});
        Cc13AvoidZoneManagerNative* constructed;
        {
            CurrentAllocationUnwindGuard allocation_cleanup{allocation};
            const NativeAvoidManagerLifetimeContext lifetime{
                context.actual_00e188a8_publication_cell,
                context.actual_00e17624_publication_cell};
            constructed = allocation != nullptr
                ? construct_native_avoid_manager_00421500(
                    static_cast<Cc13AvoidZoneManagerNative*>(allocation), &lifetime)
                : nullptr;
            // End this guard's scope BEFORE publication/relookup/registration,
            // so current EH also restores only the critical guard here.
            allocation_cleanup.active = false;
        }
        store_word(context.actual_00e17624_publication_cell, 0,
            reinterpret_cast<std::uint32_t>(constructed));
        void* const current_manager = get_native_singleton_manager_00415350(
            *context.actual_01090aa0_publication_cell);
        auto* const current_avoid = load_avoid_publication(context);
        register_native_singleton_object_00bd0c30(current_manager, nullptr, current_avoid);
    }

    // Normal release uses the original captured section, not a new lookup or
    // a projected lock. Keep cleanup armed until Leave and final reload finish.
    if (captured_section != nullptr) {
        store_word(captured_section, 0x18, load_word(captured_section, 0x18) - 1u);
        LeaveCriticalSection(captured_section);
    }
    auto* const result = load_avoid_publication(context);
    critical_cleanup.active = false;
    return result;
}

} // namespace bsp
