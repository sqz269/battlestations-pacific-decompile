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
#include <cstring>
#include <exception>
#include <intrin.h>

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
    const Word profile = 0x00ce3818u;
    _ReadWriteBarrier();
    std::memcpy(actual_receiver, &profile, 4);
    _ReadWriteBarrier();
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

namespace bsp {
void* construct_native_application_singleton_base_00bea810(
    void* actual_receiver, NativeApplicationSingletonBaseContext& context) {
    BaseCleanup base_cleanup{actual_receiver}; // Source state0 before profile.
    *static_cast<volatile Word*>(actual_receiver) = 0x00d68bc4u;
    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    auto* const section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        depth(section) = depth(section) + 1u;
    }
    GuardCleanup guard_cleanup{guard}; // Source state1 after Enter/depth.
    context.actual_application_e1ae90 = actual_receiver;
    void* const manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    void* const current_application = context.actual_application_e1ae90;
    register_native_singleton_object_00bd0c30(manager, nullptr, current_application);
    if (section) {
        depth(section) = depth(section) - 1u;
        LeaveCriticalSection(section);
    }
    guard_cleanup.armed = false;
    base_cleanup.armed = false;
    return actual_receiver;
}
} // namespace bsp

namespace bsp {
void* construct_native_application_loop_base_00bea970(
    void* actual_receiver, NativeApplicationSingletonBaseContext& context) {
    void* const original_receiver = actual_receiver;
    (void)construct_native_application_singleton_base_00bea810(original_receiver, context);
    *static_cast<volatile Word*>(original_receiver) = 0x00d68bc8u;
    static_cast<volatile unsigned char*>(original_receiver)[0x04] = 0;
    return original_receiver;
}
} // namespace bsp

#include "bsp/native_shader_preload.hpp"

namespace bsp {
void* construct_native_application_00737970(
    void* actual_receiver, NativeApplicationSingletonBaseContext& context) {
    void* const original_receiver = actual_receiver;
    (void)construct_native_application_loop_base_00bea970(original_receiver, context);
    *static_cast<volatile Word*>(original_receiver) = 0x00cfeab0u;
    auto* const bytes = static_cast<unsigned char*>(original_receiver);
    auto* const systems = reinterpret_cast<volatile NativeApplicationPointerVectorStorage*>(bytes + 0x08);
    systems->data_00 = nullptr;
    systems->count_04 = 0;
    systems->capacity_08 = 0;
    *reinterpret_cast<void* volatile*>(bytes + 0x14) = nullptr;
    static_cast<volatile unsigned char*>(original_receiver)[0x19] = 0;
    static_cast<volatile unsigned char*>(original_receiver)[0x1a] = 0;
    static_cast<volatile unsigned char*>(original_receiver)[0x18] = 1;
    return original_receiver;
}
} // namespace bsp

namespace bsp {
void retire_native_application_singleton_base_00bea8b0(
    void* actual_receiver, NativeApplicationSingletonBaseContext& context) {
    *static_cast<volatile Word*>(actual_receiver) = 0x00d68bc4u;
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
    GuardCleanup guard_cleanup{guard}; // Source state1, including null section.
    void* const manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    void* const current_application = context.actual_application_e1ae90;
    unregister_native_singleton_object_00bcfca0(manager, nullptr, current_application);
    context.actual_application_e1ae90 = nullptr;
    if (section) {
        depth(section) = depth(section) - 1u;
        LeaveCriticalSection(section);
    }
    guard_cleanup.armed = false;
    destroy_native_generic_singleton_base_00412430(actual_receiver);
    base_cleanup.armed = false;
}
} // namespace bsp

namespace bsp {
void retire_native_application_base_00bea990(
    void* actual_receiver, NativeApplicationSingletonBaseContext& context) {
    *static_cast<volatile Word*>(actual_receiver) = 0x00d68bc8u;
    retire_native_application_singleton_base_00bea8b0(actual_receiver, context);
}
} // namespace bsp

#include "bsp/native_application_pointer_vector_resize.hpp"

namespace bsp {
namespace {

void unwind_application_base(void* actual_receiver,
                             NativeApplicationSingletonBaseContext& context) noexcept {
    __try {
        retire_native_application_base_00bea990(actual_receiver, context);
    } __except (cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}

struct ApplicationVectorRetirementCleanup {
    void* actual_receiver;
    NativeApplicationSingletonBaseContext& context;
    bool armed = true;
    ~ApplicationVectorRetirementCleanup() noexcept {
        if (armed) unwind_application_base(actual_receiver, context);
    }
};

} // namespace

void retire_native_application_007379a0(
    void* actual_receiver,
    NativeApplicationPointerVectorStorage& actual_rows,
    const NativeApplicationPointerVectorAllocation& allocation,
    NativeApplicationSingletonBaseContext& context) {
    ApplicationVectorRetirementCleanup cleanup{actual_receiver, context};
    resize_native_application_pointer_vector_00735f30(actual_rows, 0, allocation);
    void* const current_data =
        static_cast<volatile NativeApplicationPointerVectorStorage&>(actual_rows).data_00;
    allocation.free_00bf6989(current_data);
    cleanup.armed = false;
    retire_native_application_base_00bea990(actual_receiver, context);
}

} // namespace bsp

namespace bsp {

void* retire_native_application_scalar_00737e00(
    void* actual_receiver,
    NativeApplicationPointerVectorStorage& actual_rows,
    const NativeApplicationPointerVectorAllocation& allocation,
    NativeApplicationSingletonBaseContext& context,
    const volatile std::uint8_t& actual_selector,
    void (*free_object_00bf65ac)(void*)) {
    void* const captured_receiver = actual_receiver;
    retire_native_application_007379a0(captured_receiver, actual_rows, allocation, context);
    const std::uint8_t selector = actual_selector;
    if ((selector & 1u) != 0) {
        free_object_00bf65ac(captured_receiver);
    }
    return captured_receiver;
}

} // namespace bsp
