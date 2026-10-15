#include "bsp/native_string_vector_group_cleanup.hpp"

#include "bsp/native_outer_string_vector_cleanup.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
} // namespace

void __fastcall destroy_native_string_vector_group_range_006a6ee0(void* first,
    void* last, Word, Word, NativeStringRawPoolContext& strings) {
    Word current = address(first);
    const Word captured_end = address(last);
    while (current != captured_end) {
        // Same complete capture/range/current-free/clear schedule. The native
        // original-first versus current-header duplicate is ignored by55A3A0.
        tidy_native_outer_string_vector_0055aff0(pointer(current), strings);
        current += 16u;
    }
}

void __fastcall destroy_native_string_vector_group_range_dispatch_006a7190(
    Word receiver_word, Word, void* first, void* last,
    NativeStringRawPoolContext& strings) {
    destroy_native_string_vector_group_range_006a6ee0(first, last, receiver_word,
        address(last), strings);
}
} // namespace bsp
