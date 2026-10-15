#include "bsp/native_group_vector_insert.hpp"

#include "bsp/native_group_vector_transfer.hpp"
#include "bsp/native_nested_string_vector_allocation.hpp"
#include "bsp/native_outer_string_vector_cleanup.hpp"
#include "bsp/native_outer_string_vector_cleanup_wrappers.hpp"
#include "bsp/native_outer_string_vector_copy_construct.hpp"
#include "bsp/native_outer_string_vector_fill.hpp"
#include "bsp/native_outer_vector_assignment.hpp"
#include "bsp/native_string_vector_group_cleanup.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word), "Native DWORD address width");

Word address(const void* value) noexcept {
    return reinterpret_cast<Word>(value);
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}
Word word(Word base, Word byte_offset) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void put(Word base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
Word sar_four(Word value) noexcept {
    return (value >> 4u) | ((value & 0x80000000u) ? 0xf0000000u : 0u);
}
Word size_from_captured_begin(Word header, Word begin) noexcept {
    return begin == 0u ? 0u : sar_four(word(header, 8u) - begin);
}
} // namespace

void* __fastcall allocate_native_group_vector_0069b360(
    Word count, void* unused_edx) {
    return allocate_native_nested_string_vector_00553300(count, unused_edx);
}

[[noreturn]] void __cdecl throw_native_group_vector_length_error_0069f1a0() {
    throw_native_nested_string_vector_length_error_00556c90();
}

void __fastcall insert_native_group_vector_count_006a7690(
    void* header, Word unused_edx, const void* unused_iterator_owner,
    void* position, Word count, const void* value,
    NativeStringRawPoolContext& strings) {
    (void)unused_edx;
    (void)unused_iterator_owner;
    const Word vector = address(header);
    const Word position_address = address(position);
    Word prototype[4]; // Opaque+0 is neither initialized nor consumed.
    construct_native_outer_string_vector_copy_0055b150(
        prototype, 0u, value, strings);
    const Word prototype_address = address(prototype);
    const Word initial_begin = word(vector, 4u);
    Word normal_prototype_begin = 0u;
    Word normal_dead_word = count;

    // State0 starts only after prototype construction and receiver-begin read.
    try {
        const Word capacity = initial_begin == 0u ? 0u :
            sar_four(word(vector, 12u) - initial_begin);
        if (count != 0u) {
            const Word checked_size = size_from_captured_begin(vector, initial_begin);
            if (0x0fffffffu - checked_size < count)
                throw_native_group_vector_length_error_0069f1a0();

            const Word compared_size = size_from_captured_begin(vector, initial_begin);
            if (capacity < compared_size + count) {
                const Word half = capacity >> 1u;
                Word new_capacity = 0x0fffffffu - half < capacity ? 0u :
                    capacity + half;
                const Word candidate_size = size_from_captured_begin(vector, initial_begin);
                if (new_capacity < candidate_size + count) {
                    const Word selected_size = size_from_captured_begin(vector, initial_begin);
                    new_capacity = selected_size + count;
                }

                const Word fresh = address(allocate_native_group_vector_0069b360(
                    new_capacity, nullptr));
                const Word prefix_begin = word(vector, 4u);
                Word completed = fresh;
                Word old_begin = 0u;
                try {
                    completed = address(transfer_native_group_vector_to_empty_006a6d20(
                        pointer(prefix_begin), position, pointer(fresh), vector,
                        fresh, 0u, strings));
                    const Word filled_end = address(fill_native_outer_string_vector_dispatch_006a7500(
                        vector, 0u, pointer(completed), count, prototype, strings));
                    const Word suffix_end = word(vector, 8u);
                    completed = filled_end;
                    normal_dead_word = completed;
                    transfer_native_group_vector_to_empty_006a6d20(position,
                        pointer(suffix_end), pointer(completed), vector,
                        completed, 0u, strings);
                    // Captured before state1 is disarmed, as at006A77B1.
                    old_begin = word(vector, 4u);
                } catch (...) {
                    destroy_native_string_vector_group_range_dispatch_006a7190(
                        vector, 0u, pointer(fresh), pointer(completed), strings);
                    singleton_lifetime_free(pointer(fresh));
                    throw;
                }

                // State0 owns only the prototype from this point onward.
                const Word new_size = count + size_from_captured_begin(vector, old_begin);
                if (old_begin != 0u) {
                    const Word old_end = word(vector, 8u);
                    destroy_native_string_vector_group_range_006a6ee0(
                        pointer(old_begin), pointer(old_end), vector, completed, strings);
                    singleton_lifetime_free(pointer(word(vector, 4u)));
                }
                put(vector, 12u, fresh + new_capacity * 16u);
                put(vector, 8u, fresh + new_size * 16u);
                put(vector, 4u, fresh);
            } else {
                const Word old_end = word(vector, 8u);
                const Word tail = sar_four(old_end - position_address);
                const Word count_bytes = count * 16u;
                if (tail < count) {
                    transfer_native_group_vector_dispatch_006a70e0(vector, 0u,
                        position, pointer(old_end), pointer(position_address + count_bytes), strings);
                    const Word fill_begin = word(vector, 8u);
                    const Word remaining = count - sar_four(fill_begin - position_address);
                    Word assigned_end = 0u;
                    try {
                        fill_native_outer_string_vector_dispatch_006a7500(vector, 0u,
                            pointer(fill_begin), remaining, prototype, strings);
                        put(vector, 8u, word(vector, 8u) + count_bytes);
                        assigned_end = word(vector, 8u) - count_bytes;
                    } catch (...) {
                        //006A788E reloads CURRENT receiver end, not old_end.
                        const Word current_end = word(vector, 8u);
                        destroy_native_string_vector_group_range_dispatch_006a7190(
                            vector, 0u, pointer(position_address + count_bytes),
                            pointer(current_end + count_bytes), strings);
                        throw;
                    }
                    fill_native_outer_vector_range_006a6ca0(
                        position, pointer(assigned_end), prototype, strings);
                } else {
                    const Word tail_begin = old_end - count_bytes;
                    const Word transferred_end = address(
                        transfer_native_group_vector_dispatch_006a70e0(vector, 0u,
                            pointer(tail_begin), pointer(old_end), pointer(old_end), strings));
                    put(vector, 8u, transferred_end);
                    move_native_group_vector_backward_006a7110(position,
                        pointer(tail_begin), pointer(old_end));
                    fill_native_outer_vector_range_006a6ca0(position,
                        pointer(position_address + count_bytes), prototype, strings);
                }
            }
        }
        normal_prototype_begin = word(prototype_address, 4u);
    } catch (...) {
        tidy_native_outer_string_vector_0055b110(prototype, strings);
        throw;
    }

    // State-1: normal cleanup frees the cached begin and does not clear fields.
    if (normal_prototype_begin != 0u) {
        const Word prototype_end = word(prototype_address, 8u);
        destroy_native_outer_string_vector_range_0055a3a0(
            pointer(normal_prototype_begin), pointer(prototype_end), prototype,
            normal_dead_word, strings);
        singleton_lifetime_free(pointer(normal_prototype_begin));
    }
}
} // namespace bsp
