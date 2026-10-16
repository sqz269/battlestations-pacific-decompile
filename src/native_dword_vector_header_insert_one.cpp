#include "bsp/native_dword_vector_header_insert_one.hpp"

#include "bsp/native_dword_vector_insertn_owner.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word bits(const void* pointer) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}
void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
Word load(Word address) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<std::uintptr_t>(address));
}
void store(Word address, Word value) {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::uintptr_t>(address)) = value;
}
Word sar_four(Word value) {
    return (value >> 4u) | ((value & 0x80000000u) ? 0xf0000000u : 0u);
}
} // namespace

void* __fastcall insert_native_dword_vector_header_one_006988f0(
    void* receiver, std::uint32_t /*unused_edx*/, void* output,
    void* iterator_owner, void* position, const void* fill_header) {
    const Word object = bits(receiver);
    const Word captured_owner = bits(iterator_owner);
    const Word initial_begin = load(object + 4u);
    Word index = 0u;
    if (initial_begin != 0u) {
        const Word initial_end = load(object + 8u);
        if (sar_four(initial_end - initial_begin) != 0u) {
            if (initial_begin > initial_end)
                invoke_native_invalid_parameter_00bf6713();
            if (captured_owner == 0u || captured_owner != object)
                invoke_native_invalid_parameter_00bf6713();
            index = sar_four(bits(position) - initial_begin);
        }
    }

    insert_native_dword_vector_header_count_00698400(
        receiver, 0u, captured_owner, position, 1u, fill_header);

    const Word new_begin = load(object + 4u);
    if (new_begin > load(object + 8u))
        invoke_native_invalid_parameter_00bf6713();
    const Word result = new_begin + (index << 4u);
    if (result > load(object + 8u))
        invoke_native_invalid_parameter_00bf6713();
    else if (result < load(object + 4u))
        invoke_native_invalid_parameter_00bf6713();

    const Word output_address = bits(output);
    store(output_address + 4u, result);
    store(output_address, object);
    return pointer(output_address);
}

} // namespace bsp
