#include "bsp/native_gui_manager_publication.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_platform_focus_owners.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
struct NativeGuard {
    std::uint32_t profile;
    CRITICAL_SECTION* section;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, section) == 4);

volatile std::uint32_t& depth(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}

struct GuardCleanup {
    NativeGuard& guard;
    bool armed = true;
    ~GuardCleanup() noexcept {
        if (!armed) return;
        armed = false;
        try {
            destroy_native_singleton_guard_00411ee0(&guard);
        } catch (...) {
            std::terminate();
        }
    }
};
} // namespace

NativeGuiManagerPublicationCalls::NativeGuiManagerPublicationCalls(
    NativePlatformFocusOwnersContext& owners, void* provider_state,
    CurrentScalarZero current_scalar_zero)
    : owners_(owners), provider_state_(provider_state),
      current_scalar_zero_(current_scalar_zero) {
    if (!current_scalar_zero_)
        throw std::invalid_argument("GUI publication teardown requires current scalar dispatch");
}

NativePlatformFocusOwnersContext& NativeGuiManagerPublicationCalls::actual_owners() const noexcept {
    return owners_;
}

void NativeGuiManagerPublicationCalls::call_current_scalar_zero(
    void* actual_owner, std::uint32_t flags) const {
    current_scalar_zero_(provider_state_, actual_owner, flags, owners_);
}

void destroy_native_gui_manager_publication_00736250(
    const NativeGuiManagerPublicationCalls& calls) {
    auto& owners = calls.actual_owners();
    void* volatile& published = owners.actual_gui_00f8bc5c;
    if (!published) return;

    void* const first_manager = get_native_singleton_manager_00415350(
        owners.actual_manager_01090aa0);
    auto* const captured_section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, captured_section};
    if (captured_section) {
        EnterCriticalSection(captured_section);
        auto& count = depth(captured_section);
        count = count + 1u;
    }
    // Native state0 starts only after Enter and the captured section's +18 ADD.
    GuardCleanup cleanup{guard};
    if (published) {
        void* const current_manager = get_native_singleton_manager_00415350(
            owners.actual_manager_01090aa0);
        void* const unregister_argument = published;
        unregister_native_singleton_object_00bcfca0(
            current_manager, nullptr, unregister_argument);
        if (void* const current_owner = published)
            calls.call_current_scalar_zero(current_owner, 1u);
        published = nullptr;
    }
    if (captured_section) {
        auto& count = depth(captured_section);
        count = count - 1u;
        LeaveCriticalSection(captured_section);
    }
    // Native state0 remains armed through the normal Leave call.
    cleanup.armed = false;
}
} // namespace bsp
