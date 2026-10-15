#include "bsp/native_outer_string_vector_insert.hpp"

#include "bsp/native_checked_string_vector_ranges.hpp"
#include "bsp/native_nested_string_vector_allocation.hpp"
#include "bsp/native_outer_insert_range_services.hpp"
#include "bsp/native_outer_string_vector_cleanup.hpp"
#include "bsp/native_outer_string_vector_cleanup_wrappers.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_string_vector_copy_construct.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word address(const void* value) noexcept {
    return reinterpret_cast<Word>(value);
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}
Word word(Word base, Word byte_offset) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
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

void __fastcall unwind_native_outer_insert_value_00c7f470(
    void* temporary, NativeStringRawPoolContext& strings) {
    destroy_native_string_header_vector_004324a0(temporary, strings);
}

void __fastcall insert_native_outer_string_vector_count_006a5c50(
    void* header, Word, const void*, void* position, Word count,
    const void* value, NativeStringRawPoolContext& strings) {
    const Word vector = address(header), position_address = address(position);
    Word temporary[4]; // Opaque+0 is neither initialized nor consumed.
    construct_native_string_header_vector_copy_00557bc0(
        temporary, 0u, value, strings);
    const Word temporary_address = address(temporary);
    const Word initial_begin = word(vector, 4u);
    Word normal_temporary_begin = 0u;

    // State0 arms only after the initial receiver-begin read, even for count0.
    try {
        const Word capacity = initial_begin == 0u ? 0u :
            sar_four(word(vector, 12u) - initial_begin);
        if (count != 0u) {
            const Word checked_size = size_from_captured_begin(vector, initial_begin);
            if (0x0fffffffu - checked_size < count)
                throw_native_nested_string_vector_length_error_00556c90();

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

                const Word fresh = address(allocate_native_nested_string_vector_00553300(
                    new_capacity, nullptr));
                const Word prefix_begin = word(vector, 4u);
                Word completed = fresh, old_begin = 0u;
                // State1 covers the new prefix/fill/suffix only. The accepted
                // transfer child moves inner allocations, not deep copies.
                try {
                    completed = address(transfer_native_inner_vectors_to_empty_006a3200(
                        pointer(prefix_begin), position, pointer(fresh), header,
                        fresh, 0u, strings));
                    const Word filled_end = address(fill_native_inner_vectors_dispatch_006a5170(
                        header, 0u, pointer(completed), count, temporary, strings));
                    const Word suffix_end = word(vector, 8u);
                    completed = filled_end;
                    transfer_native_inner_vectors_to_empty_006a3200(position,
                        pointer(suffix_end), pointer(completed), header,
                        completed, 0u, strings);
                    // Native captures this begin before disabling state1.
                    old_begin = word(vector, 4u);
                } catch (...) {
                    //006A5DC5: child handles its own partial current phase;
                    // this scope owns only the previously completed phases.
                    destroy_native_outer_string_vector_range_dispatch_0055ac60(
                        header, 0u, pointer(fresh), pointer(completed), strings);
                    singleton_lifetime_free(pointer(fresh));
                    throw;
                }

                // State0 resumes before old cleanup; do not free the new
                // backing from the construction catch if old cleanup fails.
                const Word new_size = count + size_from_captured_begin(vector, old_begin);
                if (old_begin != 0u) {
                    const Word old_end = word(vector, 8u);
                    destroy_native_outer_string_vector_range_0055a3a0(
                        pointer(old_begin), pointer(old_end), header, completed, strings);
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
                    transfer_native_inner_vectors_dispatch_006a4b50(header, 0u,
                        position, pointer(old_end), pointer(position_address + count_bytes), strings);
                    const Word fill_begin = word(vector, 8u);
                    const Word remaining = count - sar_four(fill_begin - position_address);
                    Word assigned_end = 0u;
                    // State3 starts only after the transfer and count reload.
                    try {
                        fill_native_inner_vectors_dispatch_006a5170(header, 0u,
                            pointer(fill_begin), remaining, temporary, strings);
                        put(vector, 8u, word(vector, 8u) + count_bytes);
                        assigned_end = word(vector, 8u) - count_bytes;
                    } catch (...) {
                        //006A5E4E deliberately reloads CURRENT receiver end.
                        const Word current_end = word(vector, 8u);
                        destroy_native_outer_string_vector_range_dispatch_0055ac60(
                            header, 0u, pointer(position_address + count_bytes),
                            pointer(current_end + count_bytes), strings);
                        throw;
                    }
                    // State0 resumes before assignment to the existing gap.
                    fill_native_checked_string_vector_range_006a2a00(
                        position, pointer(assigned_end), temporary, strings);
                } else {
                    const Word tail_begin = old_end - count_bytes;
                    const Word transferred_end = address(
                        transfer_native_inner_vectors_dispatch_006a4b50(header, 0u,
                            pointer(tail_begin), pointer(old_end), pointer(old_end), strings));
                    put(vector, 8u, transferred_end);
                    swap_native_inner_vectors_backward_dispatch_006a4b80(position,
                        pointer(tail_begin), pointer(old_end));
                    fill_native_checked_string_vector_range_006a2a00(position,
                        pointer(position_address + count_bytes), temporary, strings);
                }
            }
        }
        //006A5EA8 captures begin while state0 is still armed.
        normal_temporary_begin = word(temporary_address, 4u);
    } catch (...) {
        unwind_native_outer_insert_value_00c7f470(temporary, strings);
        throw;
    }

    // Native state-1 precedes normal cleanup. Unlike4324A0, this schedule
    // captures the range and frees the cached begin without clearing fields.
    if (normal_temporary_begin != 0u) {
        const Word temporary_end = word(temporary_address, 8u);
        destroy_native_string_header_range_00432050(
            pointer(normal_temporary_begin), pointer(temporary_end), strings);
        singleton_lifetime_free(pointer(normal_temporary_begin));
    }
}
} // namespace bsp
