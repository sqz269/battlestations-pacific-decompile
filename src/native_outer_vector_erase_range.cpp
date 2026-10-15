#include "bsp/native_outer_vector_erase_range.hpp"
#include "bsp/native_checked_string_vector_ranges.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_outer_string_vector_cleanup.hpp"

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
} // namespace

void* __fastcall erase_native_outer_vector_range_006a5bf0(void* header, Word,
    void* output_iterator, const void* first_owner, void* first_position,
    const void* last_owner, const void* last_position,
    NativeStringRawPoolContext& strings) {
    const Word owner = bits(first_owner);
    if (owner == 0u || owner != bits(last_owner))
        invoke_native_invalid_parameter_00bf6713();

    const Word vector = bits(header), position = bits(first_position);
    if (position != bits(last_position)) {
        const Word old_end = read(vector, 8u);
        const Word new_end = bits(copy_native_checked_string_vector_range_006a29c0(
            last_position, pointer(old_end), first_position, strings));
        const Word current_end = read(vector, 8u);
        destroy_native_outer_string_vector_range_0055a3a0(pointer(new_end),
            pointer(current_end), header, bits(output_iterator), strings);
        write(vector, 8u, new_end);
    }
    const Word output = bits(output_iterator);
    write(output, 0u, owner);
    write(output, 4u, position);
    return output_iterator;
}

void __fastcall clear_native_outer_vector_via_erase_006a6310(void* header,
    NativeStringRawPoolContext& strings) {
    const Word vector = bits(header);
    const Word captured_end = read(vector, 8u);
    if (read(vector, 4u) > captured_end)
        invoke_native_invalid_parameter_00bf6713();
    const Word captured_begin = read(vector, 4u);
    if (captured_begin > read(vector, 8u))
        invoke_native_invalid_parameter_00bf6713();
    Word output_iterator[2];
    erase_native_outer_vector_range_006a5bf0(header, 0u, output_iterator,
        header, pointer(captured_begin), header, pointer(captured_end), strings);
}
} // namespace bsp
