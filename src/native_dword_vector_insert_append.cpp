#include "bsp/native_dword_vector_insert_append.hpp"
#include "bsp/native_dword_uninitialized_fill.hpp"
#include "bsp/native_dword_vector_insert.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read_word(Word location) noexcept {
    Word value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
void write_word(Word location, Word value) noexcept {
    __asm { mov eax, location }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax], ecx }
}
Word sar_two(Word value) noexcept {
    Word result;
    __asm { mov eax, value }
    __asm { sar eax, 2 }
    __asm { mov result, eax }
    return result;
}
} // namespace

void* __fastcall insert_native_dword_vector_one_00697dc0(
    void* actual_header, std::uint32_t unused_edx, void* output,
    void* iterator_owner, void* position, const void* value_address) {
    (void)unused_edx;
    const Word object = address(actual_header);
    const Word captured_owner = address(iterator_owner);
    const Word initial_begin = read_word(object + 4u);
    Word index = 0;
    if (initial_begin != 0) {
        const Word initial_end = read_word(object + 8u);
        if (sar_two(initial_end - initial_begin) != 0) {
            if (initial_begin > initial_end)
                invoke_native_invalid_parameter_00bf6713();
            if (captured_owner == 0 || captured_owner != object)
                invoke_native_invalid_parameter_00bf6713();
            index = sar_two(address(position) - initial_begin);
        }
    }

    insert_native_dword_vector_count_00697990(
        actual_header, 0u, iterator_owner, position, 1u, value_address);
    const Word new_begin = read_word(object + 4u);
    if (new_begin > read_word(object + 8u))
        invoke_native_invalid_parameter_00bf6713();
    const Word result = new_begin + index * 4u;
    if (result > read_word(object + 8u) || result < read_word(object + 4u))
        invoke_native_invalid_parameter_00bf6713();
    write_word(address(output) + 4u, result);
    write_word(address(output), object);
    return output;
}

void __fastcall append_native_checked_dword_vector_00697f40(
    void* actual_header, std::uint32_t unused_edx, const void* value_address) {
    (void)unused_edx;
    const Word object = address(actual_header);
    const Word initial_begin = read_word(object + 4u);
    Word size = 0;
    if (initial_begin != 0)
        size = sar_two(read_word(object + 8u) - initial_begin);
    if (initial_begin != 0 &&
        size < sar_two(read_word(object + 0xcu) - initial_begin)) {
        const Word captured_end = read_word(object + 8u);
        // Only the native tag's low byte is initialized; the core ignores the
        // entire tag. Dead0 avoids an indeterminate C++ scalar evaluation.
        fill_native_dword_uninitialized_006970d0(
            pointer(captured_end), 1u, value_address, actual_header,
            value_address, 0u);
        write_word(object + 8u, captured_end + 4u);
        return;
    }

    const Word captured_end = read_word(object + 8u);
    if (initial_begin > captured_end)
        invoke_native_invalid_parameter_00bf6713();
    Word private_output[2]; // Written by InsertOne; never read by this wrapper.
    insert_native_dword_vector_one_00697dc0(
        actual_header, 0u, private_output, actual_header,
        pointer(captured_end), value_address);
}
} // namespace bsp
