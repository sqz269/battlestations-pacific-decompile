#include "bsp/native_outer_string_vector_cleanup.hpp"

#include "bsp/native_string_vector_copy_construct.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word word(Word base, Word offset) noexcept {
    return *reinterpret_cast<const volatile Word*>(base + offset);
}
void put(Word base, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(base + offset) = value;
}
} // namespace

void __fastcall destroy_native_outer_string_vector_range_0055a3a0(void* first,
    void* last, const void*, Word, NativeStringRawPoolContext& strings) {
    Word current = address(first);
    const Word captured_end = address(last);
    while (current != captured_end) {
        // Exact captured string range/current-begin free/ordered clears.
        //00432050 ignores the original-first/current-header metadata change.
        tidy_native_string_header_vector_00432440(pointer(current), strings);
        current += 16u;
    }
}

void __fastcall tidy_native_outer_string_vector_0055aff0(void* header,
    NativeStringRawPoolContext& strings) {
    const Word header_address = address(header);
    const Word begin = word(header_address, 4);
    if (begin != 0u) {
        const Word end = word(header_address, 8);
        destroy_native_outer_string_vector_range_0055a3a0(pointer(begin),
            pointer(end), header, header_address, strings);
        singleton_lifetime_free(pointer(word(header_address, 4)));
    }
    put(header_address, 4, 0u);
    put(header_address, 8, 0u);
    put(header_address, 12, 0u);
}
} // namespace bsp
