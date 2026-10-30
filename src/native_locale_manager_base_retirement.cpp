#include "bsp/native_locale_manager_base_retirement.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"

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

void destroy_native_locale_manager_base_007365e0(
    void* actual_receiver, const NativeLocaleManagerConstructorContext& context) {
    store_word(actual_receiver, 0, 0x00cfea5cu);
    BaseCleanup base_cleanup{actual_receiver}; // Own state0 AFTER profile store.
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
    GuardCleanup guard_cleanup{guard}; // State1 on BOTH section paths.
    void* const second_manager = get_native_singleton_manager_00415350(
        context.actual_manager_01090aa0);
    void* const current_locale = context.actual_locale_00f8bc4c;
    unregister_native_singleton_object_00bcfca0(
        second_manager, nullptr, current_locale);
    context.actual_locale_00f8bc4c = nullptr;
    if (section) {
        store_word(section, 0x18, load_word(section, 0x18) - 1u);
        LeaveCriticalSection(section); // State1 stays armed through this call.
    }
    destroy_native_generic_singleton_base_00412430(actual_receiver);
    guard_cleanup.armed = false;
    base_cleanup.armed = false;
}

} // namespace bsp
