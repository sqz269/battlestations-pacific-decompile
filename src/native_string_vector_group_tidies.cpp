#include "bsp/native_string_vector_group_tidies.hpp"

#include "bsp/native_outer_string_vector_cleanup.hpp"
#include "bsp/native_string_vector_group_cleanup.hpp"
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

void __fastcall tidy_native_outer_string_vector_stacked_006a6ce0(Word, Word,
    void* header, NativeStringRawPoolContext& strings) {
    // The complete native storage schedule is identical; only entry/private
    // stack mechanics differ. Incoming owner ECX and EDX are not consumed.
    tidy_native_outer_string_vector_0055aff0(header, strings);
}

void __fastcall tidy_native_string_vector_group_vector_006a75f0(void* header,
    NativeStringRawPoolContext& strings) {
    const Word header_address = address(header);
    const Word begin = word(header_address, 4);
    if (begin != 0u) {
        const Word end = word(header_address, 8);
        destroy_native_string_vector_group_range_006a6ee0(pointer(begin),
            pointer(end), header_address, header_address, strings);
        singleton_lifetime_free(pointer(word(header_address, 4)));
    }
    put(header_address, 4, 0u);
    put(header_address, 8, 0u);
    put(header_address, 12, 0u);
}

void __fastcall tidy_native_string_vector_group_vector_006a7960(void* header,
    NativeStringRawPoolContext& strings) {
    tidy_native_string_vector_group_vector_006a75f0(header, strings);
}
} // namespace bsp
