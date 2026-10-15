#include "bsp/native_outer_string_vector_copy_construct.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_nested_string_vector_allocation.hpp"
#include "bsp/native_nested_string_vector_copy.hpp"
#include "bsp/native_outer_string_vector_cleanup.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read(const void* base, Word byte_offset) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
void write(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
Word sar_four(Word value) noexcept {
    return (value >> 4) | ((value & 0x80000000u) ? 0xf0000000u : 0u);
}
} // namespace

void* __fastcall construct_native_outer_string_vector_copy_0055b150(
    void* destination, Word, const void* source,
    NativeStringRawPoolContext& strings) {
    const Word initial_begin = read(source, 4u);
    const Word count = initial_begin == 0u ? 0u :
        sar_four(read(source, 8u) - initial_begin);
    write(destination, 4u, 0u);
    write(destination, 8u, 0u);
    write(destination, 12u, 0u);
    if (count == 0u) return destination;

    if (count > 0x0fffffffu)
        throw_native_nested_string_vector_length_error_00556c90();
    const Word fresh = bits(allocate_native_nested_string_vector_00553300(
        count, nullptr));
    write(destination, 4u, fresh);
    write(destination, 8u, fresh);
    write(destination, 12u, fresh + count * 16u);

    const Word captured_end = read(source, 8u);
    const bool begin_after_end = read(source, 4u) > captured_end;
    // Native0055B1C5 arms state0 only after both reads/comparison above.
    try {
        if (begin_after_end) invoke_native_invalid_parameter_00bf6713();
        const Word captured_begin = read(source, 4u);
        if (captured_begin > read(source, 8u))
            invoke_native_invalid_parameter_00bf6713();
        const Word destination_begin = read(destination, 4u);
        const Word dead_tag = captured_begin & 0xffffff00u;
        void* const completed =
            copy_native_nested_string_vector_uninitialized_range_00559e00(
                pointer(captured_begin), pointer(captured_end),
                pointer(destination_begin), bits(destination), dead_tag,
                dead_tag, strings);
        write(destination, 8u, bits(completed));
    } catch (...) {
        tidy_native_outer_string_vector_0055aff0(destination, strings);
        throw;
    }
    return destination;
}
} // namespace bsp
