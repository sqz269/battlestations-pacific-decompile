#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
// Actual 18h history storage allocated by 007839D0. Float fields and array
// elements retain their raw four-byte representations; there are no defaults
// or automatic lifetime operations. profile_00 is not a host executable table.
struct NativeSessionTargetHistoryStorage {
    std::uint32_t profile_00;
    std::uint32_t count_04;
    std::uint32_t initial_float_word_08;
    std::uint32_t* sample_float_words_0c;
    std::uint32_t index_10;
    std::uint32_t total_float_word_14;
};
static_assert(sizeof(NativeSessionTargetHistoryStorage) == 0x18);
static_assert(offsetof(NativeSessionTargetHistoryStorage, sample_float_words_0c) == 0xc);
static_assert(offsetof(NativeSessionTargetHistoryStorage, index_10) == 0x10);
static_assert(offsetof(NativeSessionTargetHistoryStorage, total_float_word_14) == 0x14);

// Complete [00782F40,00782FD8): original ECX=history; stack count,float word;
// RET8. No native identity return is established. This is a new source ABI.
// Preserve profile, saturating unsigned allocation bytes, signed fill bounds,
// fresh array reloads and original argument words. The final signed FILD,
// index=0, FMUL and FSTP retain ambient x87 state. Verification covers masked
// exceptions with an available x87 slot; no control-word normalization occurs.
// Existing canonical allocation service only; no catch or partial rollback.
void initialize_native_session_target_history_00782f40(
    NativeSessionTargetHistoryStorage* actual_history, std::uint32_t original_count,
    std::uint32_t original_initial_float_word);

// Complete [00783970,0078399B): original ECX=history, stack flags, RET4;
// EAX returns the captured history even when flags bit0 requests its release.
// Capture array before numeric D04268 stamp, free array, optionally free self;
// do not clear fields. Self release requires a matching canonical allocation.
// Stored Ghidra body still truncates at 00783981; complete native bytes prove
// the remaining tail. Historical CRT/EH/native vtable ABI are not supplied.
NativeSessionTargetHistoryStorage* scalar_delete_native_session_target_history_00783970(
    NativeSessionTargetHistoryStorage* actual_history, std::uint32_t flags) noexcept;
} // namespace bsp
