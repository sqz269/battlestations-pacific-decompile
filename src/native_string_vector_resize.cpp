#include "bsp/native_string_vector_resize.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_vector_erase.hpp"
#include "bsp/native_string_vector_insert.hpp"

namespace bsp {
namespace {

using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeString) == 8);

Word address(const void* value) noexcept {
    return reinterpret_cast<Word>(value);
}

void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}

Word word(Word base, Word offset) noexcept {
    return *reinterpret_cast<const volatile Word*>(base + offset);
}

void put(Word base, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(base + offset) = value;
}

Word sar_three(Word value) noexcept {
    return (value >> 3) | ((value & 0x80000000u) ? 0xe0000000u : 0u);
}

Word size_from_captured_begin(Word header, Word begin) noexcept {
    return begin == 0u ? 0u : sar_three(word(header, 8) - begin);
}

} // namespace

void __fastcall resize_native_string_header_vector_0049e050(void* header,
    Word, Word requested_count, Word owned_fill_length, void* owned_fill_data,
    NativeStringRawPoolContext& strings) {
    // These are the actual parameter-header bits, not a newly copied string.
    // NativeString has no implicit storage destructor; ownership is explicit.
    NativeString owned_fill;
    const Word fill_address = address(&owned_fill);
    put(fill_address, 0, owned_fill_length);
    put(fill_address, 4, address(owned_fill_data));

    const Word header_address = address(header);
    const Word captured_begin = word(header_address, 4);
    // Native state0 is armed only after that first receiver-begin read.
    try {
        const Word initial_size = size_from_captured_begin(header_address,
            captured_begin);
        if (initial_size < requested_count) {
            const Word old_size = size_from_captured_begin(header_address,
                captured_begin);
            const Word captured_end = word(header_address, 8);
            if (captured_begin > captured_end)
                invoke_native_invalid_parameter_00bf6713();
            insert_native_string_header_vector_count_0044f410(header, 0u,
                header, pointer(captured_end), requested_count - old_size,
                &owned_fill, strings);
        } else if (captured_begin != 0u) {
            const Word captured_end = word(header_address, 8);
            const Word fresh_size = sar_three(captured_end - captured_begin);
            if (requested_count < fresh_size) {
                if (captured_begin > captured_end)
                    invoke_native_invalid_parameter_00bf6713();
                const Word current_begin = word(header_address, 4);
                if (current_begin > word(header_address, 8))
                    invoke_native_invalid_parameter_00bf6713();
                const Word target = current_begin + requested_count * 8u;
                if (target > word(header_address, 8) ||
                    target < word(header_address, 4))
                    invoke_native_invalid_parameter_00bf6713();
                Word output[2];
                erase_native_string_header_vector_range_004954f0(header, 0u,
                    output, header, pointer(target), header,
                    pointer(captured_end), strings);
            }
        }
    } catch (...) {
        //00C63540 selects the callee-owned parameter header and tail-delegates
        // actual0041DD20. A Source cleanup exception escapes without retry.
        destroy_native_string_header_0041dd20(&owned_fill, strings);
        throw;
    }
    // State-1 before the normal pool return; do not catch/retry this destructor.
    destroy_native_string_header_0041dd20(&owned_fill, strings);
}

} // namespace bsp
