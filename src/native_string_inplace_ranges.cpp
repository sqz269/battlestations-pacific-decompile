#include "bsp/native_string_inplace_ranges.hpp"
#include "bsp/native_string_range_assignment.hpp"
#include "bsp/native_string_uninitialized_copy.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word sar_three(Word value) noexcept {
    return (value >> 3) | ((value & 0x80000000u) ? 0xe0000000u : 0u);
}
// The complete0048DA50 body performs exactly one assignment for this captured
// eight-byte source span, including identity skip and all ordered raw reloads.
void assign_one(Word destination, Word source, NativeStringRawPoolContext& strings) {
    assign_native_string_header_range_0048da50(pointer(source), pointer(source + 8u),
        pointer(destination), 0u, 0u, 0u, strings);
}
} // namespace

void* __cdecl copy_native_string_header_range_backward_0044bc50(
    const void* first, const void* last, void* destination_end,
    Word, Word, Word, NativeStringRawPoolContext& strings) {
    const Word source_begin = bits(first);
    Word source = bits(last);
    Word output = bits(destination_end);
    while (source != source_begin) {
        source -= 8u;
        output -= 8u;
        assign_one(output, source, strings);
    }
    return pointer(output);
}

void __cdecl fill_native_string_header_range_0044c770(void* first,
    const void* last, const void* value, NativeStringRawPoolContext& strings) {
    Word current = bits(first);
    const Word end = bits(last);
    const Word source = bits(value);
    while (current != end) {
        assign_one(current, source, strings);
        current += 8u;
    }
}

void* __cdecl copy_native_string_backward_dispatch_0044d850(const void* first,
    const void* last, void* destination_end, NativeStringRawPoolContext& strings) {
    const Word captured_first = bits(first), captured_last = bits(last);
    const Word captured_destination = bits(destination_end);
    copy_native_string_header_range_backward_0044bc50(first, last, destination_end,
        captured_destination, 0u, 0u, strings);
    return pointer(captured_destination - sar_three(captured_last - captured_first) * 8u);
}

void* copy_native_string_uninitialized_dispatch_0044f170(void* owner,
    const void* first, const void* last, void* destination,
    NativeStringRawPoolContext& strings) {
    return copy_native_string_uninitialized_dispatch_00505ad0(
        owner, first, last, destination, strings);
}
} // namespace bsp
