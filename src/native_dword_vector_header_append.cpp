#include "bsp/native_dword_vector_header_append.hpp"

#include "bsp/native_dword_vector_counted_fill.hpp"
#include "bsp/native_dword_vector_header_insert_one.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word bits(const void* value) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
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

void __fastcall append_native_checked_dword_vector_headers_00698980(
    void* receiver, std::uint32_t /*unused_edx*/, const void* fill_header) {
    const Word object = bits(receiver);
    const Word initial_begin = load(object + 4u);
    Word size = 0u;
    if (initial_begin != 0u)
        size = sar_four(load(object + 8u) - initial_begin);

    if (initial_begin != 0u) {
        const Word capacity = sar_four(load(object + 12u) - initial_begin);
        if (size < capacity) {
            const Word fill_bits = bits(fill_header);
            const Word captured_end = load(object + 8u);
            fill_native_dword_vector_counted_00698040(
                pointer(captured_end), 1u, fill_header, object, fill_bits, 0u);
            store(object + 8u, captured_end + 16u);
            return;
        }
    }

    const Word captured_end = load(object + 8u);
    if (initial_begin > captured_end)
        invoke_native_invalid_parameter_00bf6713();

    Word output[2];
    insert_native_dword_vector_header_one_006988f0(
        receiver, 0u, output, receiver, pointer(captured_end), fill_header);
}

} // namespace bsp
