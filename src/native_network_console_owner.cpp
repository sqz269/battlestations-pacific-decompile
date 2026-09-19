#include "bsp/native_network_console_owner.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <type_traits>

namespace bsp {
namespace {

struct NativeGuard final {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, section_04) == 4);
static_assert(sizeof(NativeNetworkConsoleStorage) == 0x4003f0);
static_assert(std::is_trivially_destructible_v<NativeNetworkConsoleStorage>);

void profile(NativeNetworkConsoleStorage& owner, std::uint32_t value) noexcept {
    std::memcpy(&owner, &value, sizeof(value));
}

CRITICAL_SECTION* first_section(void* manager) noexcept {
    return *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(manager) + 0x10);
}

volatile std::uint32_t& depth(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}

// The established raw 00411EE0 provider performs the exact tracked decrement
// and LeaveCriticalSection on the captured section, not a fresh getter result.
struct GuardCleanup final {
    NativeGuard& guard;
    bool armed = true;
    ~GuardCleanup() noexcept {
        if (armed) {
            try { destroy_native_singleton_guard_00411ee0(&guard); }
            catch (...) { std::terminate(); }
        }
    }
};

struct BaseCleanup final {
    NativeNetworkConsoleStorage& owner;
    bool armed = true;
    ~BaseCleanup() noexcept {
        if (armed) destroy_native_generic_singleton_base_00412430(&owner);
    }
};

NativeGuard enter_first_section(void* manager) {
    auto* const section = first_section(manager);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        depth(section) = depth(section) + 1u;
    }
    return guard;
}

void leave_first_section(GuardCleanup& cleanup) {
    destroy_native_singleton_guard_00411ee0(&cleanup.guard);
    cleanup.armed = false;
}

} // namespace

NativeNetworkConsoleStorage* construct_native_network_console_base_00a3cf00(
    NativeNetworkConsoleStorage& owner, NativeNetworkConsoleBaseContext& context) {
    BaseCleanup base_cleanup{owner}; // Native state0 before first getter.
    profile(owner, 0x00d23f44u);
    void* const first_manager = get_native_singleton_manager_00415350(
        context.singleton_manager_01090aa0);
    NativeGuard guard = enter_first_section(first_manager);
    GuardCleanup guard_cleanup{guard}; // Native state1 after lock/depth.
    context.current_manager_00f8abdc = &owner;
    void* const second_manager = get_native_singleton_manager_00415350(
        context.singleton_manager_01090aa0);
    auto* const current_owner = context.current_manager_00f8abdc;
    register_native_singleton_object_00bd0c30(second_manager, nullptr, current_owner);
    leave_first_section(guard_cleanup);
    base_cleanup.armed = false;
    return &owner;
}

void destroy_native_network_console_base_00a3cfa0(
    NativeNetworkConsoleStorage& owner, NativeNetworkConsoleBaseContext& context) {
    profile(owner, 0x00d23f44u);
    BaseCleanup base_cleanup{owner}; // Native state0 after profile.
    void* const first_manager = get_native_singleton_manager_00415350(
        context.singleton_manager_01090aa0);
    NativeGuard guard = enter_first_section(first_manager);
    GuardCleanup guard_cleanup{guard};
    void* const second_manager = get_native_singleton_manager_00415350(
        context.singleton_manager_01090aa0);
    auto* const current_owner = context.current_manager_00f8abdc;
    unregister_native_singleton_object_00bcfca0(second_manager, nullptr, current_owner);
    context.current_manager_00f8abdc = nullptr;
    leave_first_section(guard_cleanup);
    destroy_native_generic_singleton_base_00412430(&owner);
    base_cleanup.armed = false;
}

NativeNetworkConsoleStorage* delete_native_network_console_base_00a3d040(
    NativeNetworkConsoleStorage& owner, std::uint32_t flags,
    NativeNetworkConsoleBaseContext& context) {
    auto* const captured = &owner;
    destroy_native_network_console_base_00a3cfa0(owner, context);
    if ((flags & 1u) != 0) delete captured;
    return captured;
}

} // namespace bsp
