#include "bsp/native_locale_manager_retirement.hpp"
#include "bsp/native_locale_manager_base_retirement.hpp"
#include "bsp/native_locale_map_clear.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <cstddef>
#include <cstring>
#include <exception>
#include <intrin.h>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);
static_assert(sizeof(NativeLocaleManagerRetirementContext) == 12);

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
void* pointer(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
void* at(void* storage, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(storage) + offset;
}
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_action(void* receiver,
    const NativeLocaleManagerRetirementContext& context, int state) noexcept {
    __try {
        if (state == 2) {
            // Own C86963 ->73C330, an exact JMP73C240.
            clear_native_locale_map_0073c240(at(receiver, 0x14),
                context.strings, context.actual_initialized_locale_pool_00e1768c);
        } else if (state == 1) {
            destroy_native_string_header_vector_004324a0(
                at(receiver, 4), context.strings);
        } else {
            destroy_native_locale_manager_base_007365e0(
                receiver, context.publication);
        }
    } __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct RetirementCleanup {
    void* receiver;
    const NativeLocaleManagerRetirementContext& context;
    int state = 2;
    ~RetirementCleanup() noexcept {
        while (state >= 0) {
            const int action = state--;
            unwind_action(receiver, context, action);
        }
    }
};
} // namespace

void destroy_native_locale_manager_0073c9f0(
    void* actual_receiver, const NativeLocaleManagerRetirementContext& context) {
    store_word(actual_receiver, 0, 0x00cff140u);
    const Word first_sidecar = load_word(actual_receiver, 0x4034);
    const bool first_nonnull = first_sidecar != 0;
    RetirementCleanup cleanup{actual_receiver, context}; // Own state2 now.
    if (first_nonnull) singleton_lifetime_free(pointer(first_sidecar));
    store_word(actual_receiver, 0x4034, 0);
    store_word(actual_receiver, 0x4038, 0);
    store_word(actual_receiver, 0x403c, 0);
    const Word second_sidecar = load_word(actual_receiver, 0x4024);
    if (second_sidecar != 0) singleton_lifetime_free(pointer(second_sidecar));
    store_word(actual_receiver, 0x4024, 0);
    store_word(actual_receiver, 0x4028, 0);
    store_word(actual_receiver, 0x402c, 0);
    const Word language_data = load_word(actual_receiver, 0x401c);
    if (language_data != 0) {
        const Word language_size = load_word(actual_receiver, 0x4018) + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            context.strings.actual_published_01090aa8,
            context.strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, pointer(language_data),
            language_size, context.strings.actual_small_returns_disabled_01090aa4);
    }
    cleanup.state = 1; // Disarm normal map cleanup BEFORE the call.
    clear_native_locale_map_0073c240(at(actual_receiver, 0x14),
        context.strings, context.actual_initialized_locale_pool_00e1768c);
    const Word names_begin = load_word(actual_receiver, 8); // Still state1.
    cleanup.state = 0;
    if (names_begin != 0) {
        const Word names_end = load_word(actual_receiver, 0xc);
        destroy_native_string_header_range_00432050(
            pointer(names_begin), pointer(names_end), context.strings);
        const Word current_names_begin = load_word(actual_receiver, 8);
        singleton_lifetime_free(pointer(current_names_begin));
    }
    store_word(actual_receiver, 8, 0);
    store_word(actual_receiver, 0xc, 0);
    store_word(actual_receiver, 0x10, 0);
    cleanup.state = -1; // Base has its OWN selected cleanup; never retry it.
    destroy_native_locale_manager_base_007365e0(
        actual_receiver, context.publication);
}

void* delete_native_locale_manager_0073caf0(void* actual_receiver,
    const volatile std::uint32_t& actual_flags,
    const NativeLocaleManagerRetirementContext& context) {
    void* const original_receiver = actual_receiver;
    destroy_native_locale_manager_0073c9f0(original_receiver, context);
    _ReadWriteBarrier();
    const unsigned char low_flags =
        *reinterpret_cast<const volatile unsigned char*>(&actual_flags);
    _ReadWriteBarrier();
    if ((low_flags & 1u) != 0) singleton_lifetime_free(original_receiver);
    return original_receiver; // Address bits only; do not inspect freed storage.
}

} // namespace bsp
