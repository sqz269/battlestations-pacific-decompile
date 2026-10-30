#include "bsp/native_locale_manager_constructor.hpp"
#include "bsp/gui_startup.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
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
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);
static_assert(sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, section_04) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(NativeLocaleManagerConstructorContext) == 8);
static_assert(sizeof(LocaleTableManagerImage) == 0x4040);

Word load_word(const void* storage, std::size_t offset) noexcept {
    _ReadWriteBarrier();
    Word value;
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + offset,
        sizeof(value));
    _ReadWriteBarrier();
    return value;
}
void store_word(void* storage, std::size_t offset, Word value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(static_cast<unsigned char*>(storage) + offset, &value,
        sizeof(value));
    _ReadWriteBarrier();
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

void* construct_native_locale_manager_base_00736540(
    void* actual_receiver, const NativeLocaleManagerConstructorContext& context) {
    BaseCleanup base_cleanup{actual_receiver}; // Native state0 before CFEA5C.
    store_word(actual_receiver, 0, 0x00cfea5cu);
    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    Word const section_bits = load_word(first_manager, 0x10);
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(
        static_cast<std::uintptr_t>(section_bits));
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        store_word(section, 0x18, load_word(section, 0x18) + 1u);
    }
    GuardCleanup guard_cleanup{guard}; // State1 only after Enter/increment.
    context.actual_locale_00f8bc4c = actual_receiver;
    void* const second_manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    void* const current_locale = context.actual_locale_00f8bc4c;
    register_native_singleton_object_00bd0c30(
        second_manager, nullptr, current_locale);
    if (section) {
        store_word(section, 0x18, load_word(section, 0x18) - 1u);
        LeaveCriticalSection(section);
    }
    guard_cleanup.armed = false;
    base_cleanup.armed = false;
    return actual_receiver;
}

void* construct_native_locale_manager_0073c960(
    void* actual_receiver, const NativeLocaleManagerConstructorContext& context) {
    construct_native_locale_manager_base_00736540(actual_receiver, context);
    // Native derived EH state stays -1; no derived/member cleanup is armed.
    store_word(actual_receiver, 0, 0x00cff140u);
    store_word(actual_receiver, 0x8, 0u);
    store_word(actual_receiver, 0xc, 0u);
    store_word(actual_receiver, 0x10, 0u);
    std::memset(static_cast<std::byte*>(actual_receiver) + 0x14, 0, 0x4000);
    store_word(actual_receiver, 0x4014, 0u);
    store_word(actual_receiver, 0x4018, 0u);
    store_word(actual_receiver, 0x401c, 0u);
    store_word(actual_receiver, 0x4024, 0u);
    store_word(actual_receiver, 0x4028, 0u);
    store_word(actual_receiver, 0x402c, 0u);
    store_word(actual_receiver, 0x4034, 0u);
    store_word(actual_receiver, 0x4038, 0u);
    store_word(actual_receiver, 0x403c, 0u);
    return actual_receiver;
}
} // namespace bsp
