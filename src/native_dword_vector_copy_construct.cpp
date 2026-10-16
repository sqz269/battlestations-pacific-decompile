#include "bsp/native_dword_vector_copy_construct.hpp"

#include "bsp/native_dword_vector_allocation.hpp"
#include "bsp/native_dword_vector_copy_primitives.hpp"
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

void* __fastcall construct_native_checked_dword_vector_copy_00557590(
    void* destination, std::uint32_t unused_edx, const void* source) {
    (void)unused_edx;
    const Word object = address(destination);
    const Word input = address(source);
    const Word initial_begin = read_word(input + 4u);
    Word count = 0;
    if (initial_begin != 0)
        count = sar_two(read_word(input + 8u) - initial_begin);

    write_word(object + 4u, 0u);
    write_word(object + 8u, 0u);
    write_word(object + 0xcu, 0u);
    if (count == 0)
        return destination;
    if (count > 0x3fffffffu)
        throw_native_dword_vector_length_error_00556c20();
    void* const allocated = allocate_native_dword_vector_00553260(count, nullptr);
    const Word begin = address(allocated);
    write_word(object + 4u, begin);
    write_word(object + 8u, begin);
    write_word(object + 0xcu, begin + count * 4u);

    const Word captured_end = read_word(input + 8u);
    const bool initial_order_invalid = read_word(input + 4u) > captured_end;
    try {
        if (initial_order_invalid)
            invoke_native_invalid_parameter_00bf6713();
        const Word saved_first = read_word(input + 4u);
        if (saved_first > read_word(input + 8u))
            invoke_native_invalid_parameter_00bf6713();
        const Word current_destination_begin = read_word(object + 4u);
        const Word dead_metadata = saved_first & 0xffffff00u;
        void* const end = copy_native_dword_uninitialized_00555000(
            pointer(saved_first), pointer(captured_end),
            pointer(current_destination_begin), destination,
            dead_metadata, dead_metadata);
        write_word(object + 8u, address(end));
    } catch (...) {
        //557654: CURRENT header cleanup,not the saved allocation above.
        tidy_native_dword_vector_004cb130(destination);
        throw;
    }
    return destination;
}
} // namespace bsp
