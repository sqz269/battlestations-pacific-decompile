#include "bsp/native_group_vector_transfer.hpp"
#include "bsp/native_outer_string_vector_cleanup.hpp"
#include "bsp/native_outer_string_vector_cleanup_wrappers.hpp"
#include "bsp/native_outer_string_vector_copy_construct.hpp"
#include "bsp/native_string_vector_group_tidies.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
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
} // namespace

void* __fastcall transfer_native_group_vector_to_empty_006a6d20(
    void* first, void* last, void* destination, Word owner_word, Word, Word,
    NativeStringRawPoolContext& strings) {
    Word source_cursor = address(first);
    const Word captured_last = address(last);
    Word destination_cursor = address(destination);
    const Word destination_first = destination_cursor;
    Word completed_end = destination_cursor;
    Word temporary[4]; // Native opaque temporary+0 is never initialized.
    const Word temporary_address = address(temporary);
    put(temporary_address, 4u, 0u);
    put(temporary_address, 8u, 0u);
    put(temporary_address, 12u, 0u);
    Word cached_temporary_begin = 0u;

    // FH3 state0 owns the temporary. Try states1..3 cover the loop and
    // state2's placement-delete action is RET-only00401130: no Source work.
    try {
        try {
            while (source_cursor != captured_last) {
                if (destination_cursor != 0u) {
                    construct_native_outer_string_vector_copy_0055b150(
                        pointer(destination_cursor), 0u, temporary, strings);
                    cached_temporary_begin = word(temporary_address, 4u);
                }
                const Word source_begin = word(source_cursor, 4u);
                const Word old_destination_begin = word(destination_cursor, 4u);
                put(destination_cursor, 4u, source_begin);
                put(source_cursor, 4u, old_destination_begin);

                const Word old_destination_end = word(destination_cursor, 8u);
                const Word source_end = word(source_cursor, 8u);
                put(destination_cursor, 8u, source_end);
                put(source_cursor, 8u, old_destination_end);

                const Word source_capacity = word(source_cursor, 12u);
                const Word old_destination_capacity = word(destination_cursor, 12u);
                put(destination_cursor, 12u, source_capacity);
                destination_cursor += 16u;
                put(source_cursor, 12u, old_destination_capacity);
                completed_end = destination_cursor;
                source_cursor += 16u;
            }
        } catch (...) {
            //006A6DB0 captures original destination and the completed end.
            // Owner is loaded only on a nonempty prefix in the native frame;
            // stacked6CE0 consumes neither it nor its incoming EDX word.
            Word current = destination_first;
            if (current != completed_end) {
                do {
                    tidy_native_outer_string_vector_stacked_006a6ce0(
                        owner_word, 0u, pointer(current), strings);
                    current += 16u;
                } while (current != completed_end);
            }
            throw;
        }
    } catch (...) {
        //00C7F530: current temporary fields through actual0055B110. This
        // also runs if prefix cleanup exits exceptionally; later work stops.
        tidy_native_outer_string_vector_0055b110(temporary, strings);
        throw;
    }

    //006A6DD8 disables state0 BEFORE manual normal cleanup. Do not replace
    // this with a tidy: native frees cached begin, not reloaded CURRENT begin.
    if (cached_temporary_begin != 0u) {
        const Word current_temporary_end = word(temporary_address, 8u);
        destroy_native_outer_string_vector_range_0055a3a0(
            pointer(cached_temporary_begin), pointer(current_temporary_end),
            temporary, destination_cursor, strings);
        singleton_lifetime_free(pointer(cached_temporary_begin));
    }
    return pointer(destination_cursor);
}

void* __fastcall transfer_native_group_vector_dispatch_006a70e0(
    Word receiver_word, Word, void* first, void* last, void* destination,
    NativeStringRawPoolContext& strings) {
    const Word dead_tag = receiver_word & 0xffffff00u;
    return transfer_native_group_vector_to_empty_006a6d20(first, last,
        destination, receiver_word, address(destination), dead_tag, strings);
}

void __cdecl swap_native_group_vector_backward_006a6e20(void* first,
    void* last, void* destination_end, Word, Word, Word) {
    const Word captured_first = address(first);
    Word source_end = address(last);
    Word destination = address(destination_end);
    if (source_end == captured_first) return;
    bool more;
    do {
        const Word source_begin = word(source_end, 0xfffffff4u);
        const Word destination_begin = word(destination, 0xfffffff4u);
        source_end -= 16u;
        destination -= 16u;
        more = source_end != captured_first;
        put(destination, 4u, source_begin);
        put(source_end, 4u, destination_begin);

        const Word source_finish = word(source_end, 8u);
        const Word destination_finish = word(destination, 8u);
        put(destination, 8u, source_finish);
        put(source_end, 8u, destination_finish);

        const Word source_capacity = word(source_end, 12u);
        const Word destination_capacity = word(destination, 12u);
        put(destination, 12u, source_capacity);
        put(source_end, 12u, destination_capacity);
    } while (more);
}

void* __cdecl move_native_group_vector_backward_006a7110(void* first,
    void* last, void* destination_end) {
    swap_native_group_vector_backward_006a6e20(first, last, destination_end,
        address(destination_end), address(destination_end), 0u);
    const Word byte_count = sar_four(address(last) - address(first)) << 4u;
    return pointer(address(destination_end) - byte_count);
}
} // namespace bsp
