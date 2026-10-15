#include "bsp/native_outer_vector_resize.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_outer_string_vector_insert.hpp"
#include "bsp/native_outer_vector_erase_range.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word address(const void* value) noexcept {
    return reinterpret_cast<Word>(value);
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}
Word word(Word base, Word byte_offset) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
Word sar_four(Word value) noexcept {
    return (value >> 4u) | ((value & 0x80000000u) ? 0xf0000000u : 0u);
}
Word size_from_captured_begin(Word header, Word begin) noexcept {
    return begin == 0u ? 0u : sar_four(word(header, 8u) - begin);
}
} // namespace

void __fastcall unwind_native_outer_resize_parameter_00c7f4b0(
    void* owned_fill, NativeStringRawPoolContext& strings) {
    destroy_native_string_header_vector_004324a0(owned_fill, strings);
}

void __fastcall resize_native_outer_vector_006a6350(void* header, Word,
    Word requested_count, Word owned_fill_opaque, Word owned_fill_begin,
    Word owned_fill_end, Word owned_fill_capacity,
    NativeStringRawPoolContext& strings) {
    // Transfer the native by-value parameter's actual ownership as bits.
    // This array is the single canonical header; no deep copy is introduced.
    Word fill[4] = {owned_fill_opaque, owned_fill_begin,
        owned_fill_end, owned_fill_capacity};
    const Word fill_address = address(fill), vector = address(header);
    const Word initial_begin = word(vector, 4u);
    Word normal_fill_begin = 0u;
    // Original state0 arms after that first receiver-begin read.
    try {
        const Word initial_size = size_from_captured_begin(vector, initial_begin);
        if (initial_size < requested_count) {
            const Word captured_size = size_from_captured_begin(vector, initial_begin);
            const Word captured_end = word(vector, 8u);
            if (initial_begin > captured_end)
                invoke_native_invalid_parameter_00bf6713();
            insert_native_outer_string_vector_count_006a5c50(header, 0u,
                header, pointer(captured_end), requested_count - captured_size,
                fill, strings);
        } else if (initial_begin != 0u) {
            const Word captured_end = word(vector, 8u);
            const Word compared_size = sar_four(captured_end - initial_begin);
            if (requested_count < compared_size) {
                if (initial_begin > captured_end)
                    invoke_native_invalid_parameter_00bf6713();
                // Reload begin after a returning first diagnostic, and retain
                // it across later diagnostics when deriving the first endpoint.
                const Word current_begin = word(vector, 4u);
                if (current_begin > word(vector, 8u))
                    invoke_native_invalid_parameter_00bf6713();
                const Word first = current_begin + requested_count * 16u;
                const Word compared_end = word(vector, 8u);
                Word output[2];
                output[1] = current_begin; // Native private scratch preset.
                if (first > compared_end || first < word(vector, 4u))
                    invoke_native_invalid_parameter_00bf6713();
                erase_native_outer_vector_range_006a5bf0(header, 0u, output,
                    header, pointer(first), header, pointer(captured_end), strings);
            }
        }
        normal_fill_begin = word(fill_address, 4u);
    } catch (...) {
        unwind_native_outer_resize_parameter_00c7f4b0(fill, strings);
        throw;
    }

    // Owner state-1 precedes the normal destructor. Use captured begin and
    // freshly captured parameter end, free cached begin, and do not clear.
    if (normal_fill_begin != 0u) {
        const Word fill_end = word(fill_address, 8u);
        destroy_native_string_header_range_00432050(
            pointer(normal_fill_begin), pointer(fill_end), strings);
        singleton_lifetime_free(pointer(normal_fill_begin));
    }
}
} // namespace bsp
