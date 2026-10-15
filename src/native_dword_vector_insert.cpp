#include "bsp/native_dword_vector_insert.hpp"
#include "bsp/native_dword_inplace_ranges.hpp"
#include "bsp/native_dword_uninitialized_copy.hpp"
#include "bsp/native_dword_uninitialized_fill.hpp"
#include "bsp/native_dword_vector_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"

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
void add_word(Word location, Word value) noexcept {
    __asm { mov ecx, location }
    __asm { mov eax, value }
    __asm { add dword ptr [ecx], eax }
}
Word sar_two(Word value) noexcept {
    Word result;
    __asm { mov eax, value }
    __asm { sar eax, 2 }
    __asm { mov result, eax }
    return result;
}
} // namespace

void __fastcall insert_native_dword_vector_count_00697990(
    void* receiver, std::uint32_t unused_edx, void* unused_owner, void* position,
    std::uint32_t count, const void* value) {
    (void)unused_edx;
    (void)unused_owner;
    const Word object = address(receiver);
    Word captured_value = read_word(address(value));
    const Word captured_begin = read_word(object + 4u);
    Word capacity = 0;
    if (captured_begin != 0)
        capacity = sar_two(read_word(object + 0xcu) - captured_begin);
    if (count == 0)
        return;

    constexpr Word maximum = 0x3fffffffu;
    Word checked_size = 0;
    if (captured_begin != 0)
        checked_size = sar_two(read_word(object + 8u) - captured_begin);
    if (maximum - checked_size < count)
        throw_native_dword_vector_length_error_00556c20();

    Word required = 0;
    if (captured_begin != 0)
        required = sar_two(read_word(object + 8u) - captured_begin);
    required += count;
    if (capacity < required) {
        const Word half = capacity >> 1;
        if (maximum - half < capacity)
            capacity = 0;
        else
            capacity += half;

        Word growth_required = 0;
        if (captured_begin != 0)
            growth_required = sar_two(read_word(object + 8u) - captured_begin);
        growth_required += count;
        if (capacity < growth_required) {
            Word selected_size = 0;
            if (captured_begin != 0)
                selected_size = sar_two(read_word(object + 8u) - captured_begin);
            capacity = selected_size + count;
        }

        void* const new_backing = allocate_native_dword_vector_00553260(capacity, nullptr);
        const Word new_begin = address(new_backing);
        const Word captured_position = address(position);
        const Word current_begin = read_word(object + 4u);
        try {
            // State0: the native first tag has uninitialized high bytes, but
            // the complete core does not consume it. Use explicit dead0;
            // do not evaluate an indeterminate C++ scalar or add an input.
            void* const prefix_end = copy_native_dword_uninitialized_00696d40(
                pointer(current_begin), pointer(captured_position), new_backing,
                receiver, count, 0u);
            void* const fill_end = fill_native_dword_uninitialized_n_dispatch_00697760(
                receiver, count, prefix_end, count, &captured_value);
            const Word current_end = read_word(object + 8u);
            captured_value &= 0xffffff00u;
            copy_native_dword_uninitialized_00696d40(
                pointer(captured_position), pointer(current_end), fill_end,
                receiver, count, captured_value);

            const Word old_begin = read_word(object + 4u);
            Word old_size = 0;
            if (old_begin != 0)
                old_size = sar_two(read_word(object + 8u) - old_begin);
            const Word total = count + old_size;
            if (old_begin != 0)
                singleton_lifetime_free(pointer(old_begin));
            const Word capacity_end = new_begin + capacity * 4u;
            const Word logical_end = new_begin + total * 4u;
            write_word(object + 0xcu, capacity_end);
            write_word(object + 8u, logical_end);
            write_word(object + 4u, new_begin);
        } catch (...) {
            // Complete native catch00697AF1..00697B05: only saved new backing
            // is freed, followed by the shared rethrow. No element cleanup.
            singleton_lifetime_free(new_backing);
            throw;
        }
        return;
    }

    const Word old_end = read_word(object + 8u);
    const Word captured_position = address(position);
    if (sar_two(old_end - captured_position) < count) {
        const Word delta = count * 4u;
        copy_native_dword_uninitialized_dispatch_00697860(
            receiver, count, pointer(captured_position), pointer(old_end),
            pointer(captured_position + delta));
        const Word current_end = read_word(object + 8u);
        const Word current_tail = sar_two(current_end - captured_position);
        const Word remaining = count - current_tail;
        try {
            // State2 begins only after the preceding tail copy and captures.
            fill_native_dword_uninitialized_n_dispatch_00697760(
                receiver, current_tail, pointer(current_end), remaining, &captured_value);
            add_word(object + 8u, delta);
            const Word published_end = read_word(object + 8u);
            fill_native_dword_inplace_range_006970b0(
                pointer(captured_position), pointer(published_end - delta), &captured_value);
        } catch (...) {
            // INTERIOR00697AFD..00697B05: rethrow ONLY, without entering the
            // new-backing free prefix at00697AF1. No restoration or rollback.
            throw;
        }
        return;
    }

    const Word delta = count * 4u;
    const Word tail_first = old_end - delta;
    void* const copied_end = copy_native_dword_uninitialized_dispatch_00697860(
        receiver, count, pointer(tail_first), pointer(old_end), pointer(old_end));
    write_word(object + 8u, address(copied_end));
    copy_backward_native_dword_range_00697440(
        pointer(captured_position), pointer(tail_first), pointer(old_end));
    fill_native_dword_inplace_range_006970b0(
        pointer(captured_position), pointer(captured_position + delta), &captured_value);
}
} // namespace bsp
