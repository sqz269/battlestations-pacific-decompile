#include "bsp/native_render_service_base.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
struct NativeGuard {
    Word profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4 && sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, section_04) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
volatile Word& depth(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile Word*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(NativeGuard& guard) noexcept {
    __try {
        destroy_native_singleton_guard_00411ee0(&guard);
    } __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct BaseCleanup {
    void* receiver;
    bool armed = true;
    ~BaseCleanup() noexcept {
        if (armed) destroy_native_generic_singleton_base_00412430(receiver);
    }
};
struct GuardCleanup {
    NativeGuard& guard;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard); }
};
} // namespace

void destroy_native_generic_singleton_base_00412430(void* actual_receiver) noexcept {
    *static_cast<volatile Word*>(actual_receiver) = 0x00ce3818u;
}

void* publish_native_render_service_base_00b0f020(
    void* actual_receiver, NativeRenderServiceBaseContext& context) {
    BaseCleanup base_cleanup{actual_receiver}; // Native state0 before profile.
    *static_cast<volatile Word*>(actual_receiver) = 0x00d5e158u;
    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    auto* const section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        depth(section) = depth(section) + 1u;
    }
    GuardCleanup guard_cleanup{guard}; // Native state1 after Enter/depth.
    context.actual_service_00f8d39c = actual_receiver;
    void* const manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    void* const current_service = context.actual_service_00f8d39c;
    register_native_singleton_object_00bd0c30(manager, nullptr, current_service);
    if (section) {
        depth(section) = depth(section) - 1u;
        LeaveCriticalSection(section);
    }
    guard_cleanup.armed = false;
    base_cleanup.armed = false;
    return actual_receiver;
}

void destroy_native_render_service_base_00b0f0c0(
    void* actual_receiver, NativeRenderServiceBaseContext& context) {
    *static_cast<volatile Word*>(actual_receiver) = 0x00d5e158u;
    BaseCleanup base_cleanup{actual_receiver}; // Native state0 after profile.
    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    auto* const section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        depth(section) = depth(section) + 1u;
    }
    GuardCleanup guard_cleanup{guard};
    void* const manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    void* const current_service = context.actual_service_00f8d39c;
    unregister_native_singleton_object_00bcfca0(manager, nullptr, current_service);
    context.actual_service_00f8d39c = nullptr;
    if (section) {
        depth(section) = depth(section) - 1u;
        LeaveCriticalSection(section);
    }
    guard_cleanup.armed = false;
    destroy_native_generic_singleton_base_00412430(actual_receiver);
    base_cleanup.armed = false;
}

const void* render_time_region_00b0cf30(const void* actual_service) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<Word>(actual_service) + 0x84u);
}

const void* get_system_camera_service_matrix_00b0d100(
    const void* actual_service) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<Word>(actual_service) + 0x1d8u);
}
} // namespace bsp
