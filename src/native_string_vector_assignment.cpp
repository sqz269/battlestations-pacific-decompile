#include "bsp/native_string_vector_assignment.hpp"

#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_string_range_assignment.hpp"
#include "bsp/native_string_uninitialized_copy.hpp"
#include "bsp/native_string_vector_allocation.hpp"
#include "bsp/native_string_vector_erase.hpp"
#include "bsp/native_string_vector_operations.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read(const void* base, Word byte_offset) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void write(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
Word sar_three(Word value) noexcept {
    return (value >> 3) | ((value & 0x80000000u) ? 0xe0000000u : 0u);
}
} // namespace

void* assign_native_string_header_vector_00506c80(void* destination,
    const void* source, NativeStringRawPoolContext& strings) {
    if (destination == source) return destination;
    const Word source_begin = read(source, 4);
    if (source_begin == 0) {
        clear_native_string_header_vector_00498ff0(destination, strings);
        return destination;
    }
    const Word source_end = read(source, 8);
    const Word source_count = sar_three(source_end - source_begin);
    if (source_count == 0) {
        clear_native_string_header_vector_00498ff0(destination, strings);
        return destination;
    }
    const Word destination_begin = read(destination, 4);
    const Word destination_size = destination_begin != 0
        ? sar_three(read(destination, 8) - destination_begin) : 0u;
    if (source_count <= destination_size) {
        void* const copied_end = assign_native_string_range_dispatch_0048f400(
            pointer(source_begin), pointer(source_end), pointer(destination_begin), strings);
        void* const current_end = pointer(read(destination, 8));
        destroy_native_string_header_range_00432050(copied_end, current_end, strings);
        const Word current_source_begin = read(source, 4);
        if (current_source_begin == 0) {
            write(destination, 8, read(destination, 4));
        } else {
            const Word current_source_end = read(source, 8);
            const Word current_destination_begin = read(destination, 4);
            write(destination, 8, current_destination_begin +
                sar_three(current_source_end - current_source_begin) * 8u);
        }
        return destination;
    }
    const Word destination_capacity = destination_begin != 0
        ? sar_three(read(destination, 12) - destination_begin) : 0u;
    if (source_count <= destination_capacity) {
        // Native rereads end for this size, even after the earlier size test.
        const Word current_size = destination_begin != 0
            ? sar_three(read(destination, 8) - destination_begin) : 0u;
        const Word source_split = source_begin + current_size * 8u;
        assign_native_string_range_dispatch_0048f400(pointer(source_begin),
            pointer(source_split), pointer(destination_begin), strings);
        void* const current_destination_end = pointer(read(destination, 8));
        const void* const current_source_end = pointer(read(source, 8));
        void* const completed_end = copy_native_string_uninitialized_dispatch_00505ad0(
            destination, pointer(source_split), current_source_end,
            current_destination_end, strings);
        write(destination, 8, bits(completed_end));
        return destination;
    }
    if (destination_begin != 0) {
        void* const old_end = pointer(read(destination, 8));
        destroy_native_string_range_dispatch_00432360(destination,
            pointer(destination_begin), old_end, strings);
        // Native frees the CURRENT begin after element destruction.
        singleton_lifetime_free(pointer(read(destination, 4)));
    }
    const Word current_source_begin = read(source, 4);
    const Word requested_count = current_source_begin != 0
        ? sar_three(read(source, 8) - current_source_begin) : 0u;
    const Word bought = buy_native_checked_string_vector_00432090(
        destination, nullptr, requested_count);
    if ((bought & 0xffu) != 0) {
        void* const current_destination_begin = pointer(read(destination, 4));
        const void* const current_source_end = pointer(read(source, 8));
        const void* const fresh_source_begin = pointer(read(source, 4));
        void* const completed_end = copy_native_string_uninitialized_dispatch_00505ad0(
            destination, fresh_source_begin, current_source_end,
            current_destination_begin, strings);
        write(destination, 8, bits(completed_end));
    }
    return destination;
}
} // namespace bsp
