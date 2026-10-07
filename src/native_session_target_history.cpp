#include "bsp/native_session_target_history.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Session target history reconstruction requires MSVC Win32.
#endif

namespace bsp {
void initialize_native_session_target_history_00782f40(
    NativeSessionTargetHistoryStorage* history, std::uint32_t original_count,
    std::uint32_t original_initial_float_word) {
    const std::uint32_t bytes = original_count > 0x3fffffffu
        ? 0xffffffffu : original_count * 4u;
    volatile auto& h = *history;
    h.count_04 = original_count;
    h.initial_float_word_08 = original_initial_float_word;
    h.sample_float_words_0c = static_cast<std::uint32_t*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes})); // BF55BE -> BF681B

    std::int32_t signed_count;
    std::memcpy(&signed_count, &original_count, sizeof signed_count);
    std::int32_t index = 0;
    if (signed_count >= 4) {
        const std::int32_t limit = signed_count - 3;
        do {
            static_cast<volatile std::uint32_t*>(h.sample_float_words_0c)[index] = original_initial_float_word;
            static_cast<volatile std::uint32_t*>(h.sample_float_words_0c)[index + 1] = original_initial_float_word;
            static_cast<volatile std::uint32_t*>(h.sample_float_words_0c)[index + 2] = original_initial_float_word;
            static_cast<volatile std::uint32_t*>(h.sample_float_words_0c)[index + 3] = original_initial_float_word;
            index += 4;
        } while (index < limit);
    }
    while (index < signed_count) {
        static_cast<volatile std::uint32_t*>(h.sample_float_words_0c)[index] = original_initial_float_word;
        ++index;
    }
    // Keep the integer store between the original x87 instructions. Neither
    // the fields at +4/+8 nor an intermediate host floating value is read.
    __asm {
        mov eax, history
        fild dword ptr original_count
        mov dword ptr [eax + 10h], 0
        fmul dword ptr original_initial_float_word
        fstp dword ptr [eax + 14h]
    }
}

NativeSessionTargetHistoryStorage* scalar_delete_native_session_target_history_00783970(
    NativeSessionTargetHistoryStorage* history, std::uint32_t flags) noexcept {
    volatile auto& h = *history;
    auto* const captured_array = h.sample_float_words_0c;
    h.profile_00 = 0x00d04268u;
    singleton_lifetime_free(captured_array); // BF6989 -> BF65AC
    if ((flags & 1u) != 0) singleton_lifetime_free(history);
    return history;
}
} // namespace bsp
