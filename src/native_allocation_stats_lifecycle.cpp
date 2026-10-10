#include "bsp/native_allocation_stats_lifecycle.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/singleton_lifetime.hpp"

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
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);
static_assert(sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, profile_00) == 0);
static_assert(offsetof(NativeGuard, section_04) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(NativeAllocationStatsConstructorContext) == 8);

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

void destroy_native_allocation_stats_base_00be27f0(
    void* actual_receiver, NativeAllocationStatsConstructorContext& context) {
    *static_cast<volatile Word*>(actual_receiver) = 0x00d685e0u;
    BaseCleanup base_cleanup{actual_receiver}; // Source state0 after profile.
    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    auto* const section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        depth(section) = depth(section) + 1u;
    }
    GuardCleanup guard_cleanup{guard}; // Source state1 after enter/increment.
    void* const manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    void* const current_stats = context.actual_allocation_stats_0109cefc;
    unregister_native_singleton_object_00bcfca0(manager, nullptr, current_stats);
    context.actual_allocation_stats_0109cefc = nullptr;
    if (section) {
        depth(section) = depth(section) - 1u;
        LeaveCriticalSection(section);
    }
    destroy_native_generic_singleton_base_00412430(actual_receiver);
    guard_cleanup.armed = false;
    base_cleanup.armed = false;
}

void* delete_native_allocation_stats_base_00be2890(
    void* actual_receiver, std::uint32_t flags,
    NativeAllocationStatsConstructorContext& context) {
    destroy_native_allocation_stats_base_00be27f0(actual_receiver, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(actual_receiver);
    return actual_receiver;
}

void* delete_native_allocation_stats_00be2930(
    void* actual_receiver, std::uint32_t flags,
    NativeAllocationStatsConstructorContext& context) {
    destroy_native_allocation_stats_base_00be27f0(actual_receiver, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(actual_receiver);
    return actual_receiver;
}
} // namespace bsp
