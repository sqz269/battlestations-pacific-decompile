#include "bsp/native_string_vector_insert.hpp"

#include "bsp/native_render_resource_alias_nodes.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_inplace_ranges.hpp"
#include "bsp/native_string_uninitialized_copy.hpp"
#include "bsp/native_string_uninitialized_fill.hpp"
#include "bsp/native_string_vector_allocation.hpp"
#include "bsp/native_string_vector_operations.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {

using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeString) == 8);

Word address(const void* value) noexcept {
    return reinterpret_cast<Word>(value);
}

void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}

Word word(Word base, Word offset) noexcept {
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer(base + offset), sizeof value);
    _ReadWriteBarrier();
    return value;
}

void put(Word base, Word offset, Word value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(pointer(base + offset), &value, sizeof value);
    _ReadWriteBarrier();
}

Word sar_three(Word value) noexcept {
    return (value >> 3) | ((value & 0x80000000u) ? 0xe0000000u : 0u);
}

Word size_from_captured_begin(Word header, Word begin) noexcept {
    return begin == 0u ? 0u : sar_three(word(header, 8) - begin);
}

} // namespace

void __fastcall insert_native_string_header_vector_count_0044f410(void* header,
    Word, const void*, void* position, Word count, const void* value,
    NativeStringRawPoolContext& strings) {
    const Word header_address = address(header);
    const Word position_address = address(position);
    NativeString temporary;
    // Same actual-header inline schedule as0044F433..0044F46A. No owner cleanup
    // is armed if this construction fails; accepted0044BCB0 adds no such cleanup.
    construct_native_render_alias_string_0044bcb0(&temporary, value, strings);
    const Word captured_begin = word(header_address, 4);

    // Native state0 is armed AFTER the first vector begin read above.
    try {
        const Word capacity = captured_begin == 0u ? 0u :
            sar_three(word(header_address, 12) - captured_begin);
        if (count != 0u) {
            const Word checked_size = size_from_captured_begin(header_address,
                captured_begin);
            if (0x1fffffffu - checked_size < count)
                throw_native_string_vector_length_error_00431f70();

            const Word compared_size = size_from_captured_begin(header_address,
                captured_begin);
            if (capacity < compared_size + count) {
                const Word half = capacity >> 1;
                Word new_capacity = 0x1fffffffu - half < capacity ? 0u :
                    capacity + half;
                const Word candidate_size = size_from_captured_begin(
                    header_address, captured_begin);
                if (new_capacity < candidate_size + count) {
                    const Word selected_size = size_from_captured_begin(
                        header_address, captured_begin);
                    new_capacity = selected_size + count;
                }

                const Word fresh = address(allocate_native_string_vector_0042bf40(
                    new_capacity, nullptr));
                const Word prefix_begin = word(header_address, 4);
                Word completed = fresh;
                // Native state1 covers only construction into the new backing.
                try {
                    completed = address(copy_native_string_uninitialized_range_0044d0c0(
                        pointer(prefix_begin), position, pointer(fresh), header,
                        strings));
                    const Word filled_end = address(
                        fill_native_string_uninitialized_n_dispatch_0044db60(
                            header, pointer(completed), count, &temporary, strings));
                    const Word suffix_end = word(header_address, 8);
                    completed = filled_end;
                    // Native ignores this final return and never publishes it.
                    copy_native_string_uninitialized_range_0044d0c0(position,
                        pointer(suffix_end), pointer(completed), header, strings);
                } catch (...) {
                    destroy_native_string_range_dispatch_00432360(header,
                        pointer(fresh), pointer(completed), strings);
                    singleton_lifetime_free(pointer(fresh));
                    throw;
                }

                // State0 resumes BEFORE old cleanup. A failure below does not
                // enter the new-backing catch or free the completed new buffer.
                const Word old_begin = word(header_address, 4);
                const Word old_size = size_from_captured_begin(header_address,
                    old_begin);
                const Word new_size = count + old_size;
                if (old_begin != 0u) {
                    const Word old_end = word(header_address, 8);
                    destroy_native_string_header_range_00432050(pointer(old_begin),
                        pointer(old_end), strings);
                    singleton_lifetime_free(pointer(word(header_address, 4)));
                }
                put(header_address, 12, fresh + new_capacity * 8u);
                put(header_address, 8, fresh + new_size * 8u);
                put(header_address, 4, fresh);
            } else {
                const Word old_end = word(header_address, 8);
                const Word tail_count = sar_three(old_end - position_address);
                const Word byte_count = count * 8u;
                if (tail_count < count) {
                    copy_native_string_uninitialized_dispatch_0044f170(header,
                        position, pointer(old_end),
                        pointer(position_address + byte_count), strings);
                    const Word fill_begin = word(header_address, 8);
                    const Word fill_count = count -
                        sar_three(fill_begin - position_address);
                    Word assignment_end;
                    // State3 begins only after the first tail copy and these
                    // CURRENT-end/count captures, before the fill call.
                    try {
                        fill_native_string_uninitialized_n_dispatch_0044db60(header,
                            pointer(fill_begin), fill_count, &temporary, strings);
                        put(header_address, 8,
                            word(header_address, 8) + byte_count);
                        assignment_end = word(header_address, 8) - byte_count;
                    } catch (...) {
                        const Word cleanup_end = word(header_address, 8) + byte_count;
                        const Word cleanup_begin = position_address + byte_count;
                        destroy_native_string_range_dispatch_00432360(header,
                            pointer(cleanup_begin), pointer(cleanup_end), strings);
                        throw;
                    }
                    // State0: assignment failure leaves the published new end.
                    fill_native_string_header_range_0044c770(position,
                        pointer(assignment_end), &temporary, strings);
                } else {
                    const Word cut = old_end - byte_count;
                    const Word completed_end = address(
                        copy_native_string_uninitialized_dispatch_0044f170(header,
                            pointer(cut), pointer(old_end), pointer(old_end), strings));
                    put(header_address, 8, completed_end);
                    copy_native_string_backward_dispatch_0044d850(position,
                        pointer(cut), pointer(old_end), strings);
                    fill_native_string_header_range_0044c770(position,
                        pointer(position_address + byte_count), &temporary, strings);
                }
            }
        }
    } catch (...) {
        //00C60300 tail-delegates actual0041DD20 for the private temporary.
        // Cleanup failure escapes immediately in the current C++ domain.
        destroy_native_string_header_0041dd20(&temporary, strings);
        throw;
    }
    // Native0044F6BA disables temporary cleanup BEFORE its normal pool return.
    destroy_native_string_header_0041dd20(&temporary, strings);
}

} // namespace bsp
