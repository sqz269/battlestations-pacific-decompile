#include "bsp/native_group_vector_erase_resize.hpp"
#include "bsp/native_group_vector_insert.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_outer_string_vector_cleanup.hpp"
#include "bsp/native_outer_string_vector_cleanup_wrappers.hpp"
#include "bsp/native_outer_vector_assignment.hpp"
#include "bsp/native_string_vector_group_cleanup.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(Word base, Word byte_offset) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void write(Word base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
Word sar_four(Word value) noexcept {
    return (value >> 4u) | ((value & 0x80000000u) ? 0xf0000000u : 0u);
}
Word size_from_begin(Word header, Word begin) noexcept {
    return begin == 0u ? 0u : sar_four(read(header, 8u) - begin);
}
} // namespace

void* __fastcall erase_native_group_vector_range_006a7630(void* header, Word,
    void* output_iterator, const void* first_owner, void* first_position,
    const void* last_owner, const void* last_position,
    NativeStringRawPoolContext& strings) {
    const Word owner = bits(first_owner);
    if (owner == 0u || owner != bits(last_owner))
        invoke_native_invalid_parameter_00bf6713();
    const Word vector = bits(header), position = bits(first_position);
    if (position != bits(last_position)) {
        const Word old_end = read(vector, 8u);
        const Word new_end = bits(copy_native_outer_vector_range_006a6c60(
            last_position, pointer(old_end), first_position, strings));
        const Word current_end = read(vector, 8u);
        destroy_native_string_vector_group_range_006a6ee0(pointer(new_end),
            pointer(current_end), vector, bits(output_iterator), strings);
        write(vector, 8u, new_end);
    }
    const Word output = bits(output_iterator);
    write(output, 0u, owner);
    write(output, 4u, position);
    return output_iterator;
}

void __fastcall unwind_native_group_resize_parameter_00c7f5e0(
    void* owned_fill, NativeStringRawPoolContext& strings) {
    tidy_native_outer_string_vector_0055b110(owned_fill, strings);
}

void __fastcall resize_native_group_vector_006a79a0(void* header, Word,
    Word requested_count, Word owned_fill_opaque, Word owned_fill_begin,
    Word owned_fill_end, Word owned_fill_capacity,
    NativeStringRawPoolContext& strings) {
    Word fill[4] = {owned_fill_opaque, owned_fill_begin,
        owned_fill_end, owned_fill_capacity};
    const Word fill_address = bits(fill), vector = bits(header);
    const Word initial_begin = read(vector, 4u);
    Word normal_fill_begin = 0u;
    // Original owner state0 starts only after the initial receiver read.
    try {
        const Word initial_size = size_from_begin(vector, initial_begin);
        if (initial_size < requested_count) {
            const Word captured_size = size_from_begin(vector, initial_begin);
            const Word captured_end = read(vector, 8u);
            if (initial_begin > captured_end)
                invoke_native_invalid_parameter_00bf6713();
            insert_native_group_vector_count_006a7690(header, 0u, header,
                pointer(captured_end), requested_count - captured_size,
                fill, strings);
        } else if (initial_begin != 0u) {
            const Word captured_end = read(vector, 8u);
            if (requested_count < sar_four(captured_end - initial_begin)) {
                if (initial_begin > captured_end)
                    invoke_native_invalid_parameter_00bf6713();
                const Word current_begin = read(vector, 4u);
                if (current_begin > read(vector, 8u))
                    invoke_native_invalid_parameter_00bf6713();
                const Word first = current_begin + requested_count * 16u;
                const Word compared_end = read(vector, 8u);
                Word output[2];
                output[1] = current_begin;
                if (first > compared_end || first < read(vector, 4u))
                    invoke_native_invalid_parameter_00bf6713();
                erase_native_group_vector_range_006a7630(header, 0u, output,
                    header, pointer(first), header, pointer(captured_end), strings);
            }
        }
        normal_fill_begin = read(fill_address, 4u);
    } catch (...) {
        unwind_native_group_resize_parameter_00c7f5e0(fill, strings);
        throw;
    }
    // State-1 before cleanup: captured begin, CURRENT end, free cached begin.
    if (normal_fill_begin != 0u) {
        const Word current_fill_end = read(fill_address, 8u);
        destroy_native_outer_string_vector_range_0055a3a0(
            pointer(normal_fill_begin), pointer(current_fill_end), fill,
            requested_count, strings);
        singleton_lifetime_free(pointer(normal_fill_begin));
    }
}
} // namespace bsp
