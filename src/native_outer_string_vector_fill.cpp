#include "bsp/native_outer_string_vector_fill.hpp"

#include "bsp/native_outer_string_vector_cleanup.hpp"
#include "bsp/native_outer_string_vector_copy_construct.hpp"

#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word), "Native DWORD address width");

Word address(const void* pointer) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}

void* pointer(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
} // namespace

void __fastcall fill_native_outer_string_vector_uninitialized_n_006a6fa0(
    void* destination, Word count, const void* value,
    Word unused_owner, Word unused_tag3, Word unused_tag4,
    NativeStringRawPoolContext& strings) {
    (void)unused_owner;
    (void)unused_tag3;
    (void)unused_tag4;
    const Word first = address(destination);
    Word current = first;
    Word completed_end = first;
    Word remaining = count;
    const void* const fixed_value = value;

    try {
        while (remaining != 0u) {
            // Native state1 includes the null skip. Its placement action
            // C7F560 calls actual00401130 (single RET), so has no cleanup.
            if (current != 0u) {
                construct_native_outer_string_vector_copy_0055b150(
                    pointer(current), 0u, fixed_value, strings);
            }
            --remaining;
            current += 16u;
            completed_end = current;
        }
    } catch (...) {
        Word cleanup = first;
        while (cleanup != completed_end) {
            // Native006A6CE0 and accepted0055AFF0 have the same actual
            // capture/range/CURRENT-free/ordered-clear storage schedule.
            // The native owner register is unused by006A6CE0.
            tidy_native_outer_string_vector_0055aff0(pointer(cleanup), strings);
            cleanup += 16u;
        }
        throw;
    }
}

void* __fastcall fill_native_outer_string_vector_dispatch_006a7500(
    Word unused_owner, Word unused_edx, void* destination,
    Word count, const void* value, NativeStringRawPoolContext& strings) {
    (void)unused_edx;
    const Word first = address(destination);
    const Word original_count = count;
    fill_native_outer_string_vector_uninitialized_n_006a6fa0(
        destination, original_count, value, unused_owner, address(value),
        unused_owner & 0xffffff00u, strings);
    return pointer(first + (original_count << 4u));
}
} // namespace bsp
