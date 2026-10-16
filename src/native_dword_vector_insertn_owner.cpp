#include "bsp/native_dword_vector_insertn_owner.hpp"

#include "bsp/native_dword_vector_copy_construct.hpp"
#include "bsp/native_dword_vector_counted_fill.hpp"
#include "bsp/native_dword_vector_destroy_range.hpp"
#include "bsp/native_dword_vector_fill_objects.hpp"
#include "bsp/native_dword_vector_header_allocation.hpp"
#include "bsp/native_dword_vector_swap_backward.hpp"
#include "bsp/native_dword_vector_transfer_range.hpp"
#include "bsp/native_word_vector_tidy.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

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
Word load(const void* header, Word byte_offset) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<std::uintptr_t>(bits(header) + byte_offset));
}
void store(void* header, Word byte_offset, Word value) {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::uintptr_t>(bits(header) + byte_offset)) = value;
}
Word sar_four(Word value) {
    return (value >> 4u) | ((value & 0x80000000u) ? 0xf0000000u : 0u);
}
void free_private_begin(void* header) {
    const auto captured_begin = load(header, 4u);
    if (captured_begin != 0u) {
        singleton_lifetime_free(pointer(captured_begin));
    }
}
} // namespace

void __fastcall insert_native_dword_vector_header_count_00698400(
    void* receiver, std::uint32_t /*unused_edx*/,
    std::uint32_t /*unused_iterator_owner_bits*/, void* position,
    std::uint32_t count, const void* fill_header) {
    alignas(4) std::byte private_fill[16];
    construct_native_checked_dword_vector_copy_00557590(
        private_fill, 0u, fill_header);
    const auto initial_begin = load(receiver, 4u);

    // State0 begins only after successful construction and the first begin
    // read. This lexical catch is logical C7EC60, not a native EBP reader.
    try {
        auto capacity = initial_begin == 0u ? 0u
            : sar_four(load(receiver, 12u) - initial_begin);
        if (count == 0u) {
            free_private_begin(private_fill);
            return;
        }

        const auto size_from_initial_begin = [&]() -> Word {
            return initial_begin == 0u ? 0u
                : sar_four(load(receiver, 8u) - initial_begin);
        };
        const auto size_for_bound = size_from_initial_begin();
        if (0x0fffffffu - size_for_bound < count) {
            throw_native_dword_vector_header_length_error_006978d0();
        }

        const auto size_for_capacity = size_from_initial_begin();
        if (capacity < size_for_capacity + count) {
            const auto half = capacity >> 1u;
            capacity = 0x0fffffffu - half < capacity ? 0u : capacity + half;
            const auto size_for_growth = size_from_initial_begin();
            if (capacity < size_for_growth + count) {
                capacity = size_from_initial_begin() + count;
            }
            void* const allocated_base =
                allocate_native_dword_vector_header_storage_00696890(
                    capacity, nullptr);
            void* const captured_first = pointer(load(receiver, 4u));
            void* completed = allocated_base;

            // State1 persists through common normal private-begin free.
            try {
                completed = transfer_native_dword_vector_range_006980e0(
                    captured_first, position, allocated_base, bits(receiver),
                    bits(allocated_base), 0u);
                void* const after_fill =
                    fill_native_dword_vector_counted_dispatch_00698310(
                        completed, count, private_fill);
                void* const captured_suffix_end = pointer(load(receiver, 8u));
                completed = after_fill;
                transfer_native_dword_vector_range_006980e0(
                    position, captured_suffix_end, after_fill, bits(receiver),
                    bits(after_fill), 0u);

                const auto current_begin = load(receiver, 4u);
                const auto final_size = current_begin == 0u ? 0u
                    : sar_four(load(receiver, 8u) - current_begin);
                const auto final_count = count + final_size;
                if (current_begin != 0u) {
                    void* const current_end = pointer(load(receiver, 8u));
                    destroy_native_dword_vector_header_range_004d49b0(
                        pointer(current_begin), current_end, bits(receiver),
                        bits(completed));
                    singleton_lifetime_free(pointer(load(receiver, 4u)));
                }
                store(receiver, 12u, bits(allocated_base) + (capacity << 4u));
                store(receiver, 8u, bits(allocated_base) + (final_count << 4u));
                store(receiver, 4u, bits(allocated_base));
                free_private_begin(private_fill);
                return;
            } catch (...) {
                // 0069856E: suffix return never advances this completion.
                destroy_native_dword_vector_header_range_dispatch_004d9e40(
                    allocated_base, completed);
                singleton_lifetime_free(allocated_base);
                throw;
            }
        }

        const auto old_end = load(receiver, 8u);
        const auto position_bits = bits(position);
        const auto old_tail = sar_four(old_end - position_bits);
        const auto inserted_bytes = count << 4u;
        if (old_tail < count) {
            transfer_native_dword_vector_range_dispatch_00698350(
                position, pointer(old_end), pointer(position_bits + inserted_bytes));
            const auto current_end = load(receiver, 8u);
            const auto current_tail = sar_four(current_end - position_bits);
            Word assigned_end;
            try {
                // State3 starts immediately before counted construction.
                fill_native_dword_vector_counted_dispatch_00698310(
                    pointer(current_end), count - current_tail, private_fill);
                store(receiver, 8u, load(receiver, 8u) + inserted_bytes);
                assigned_end = load(receiver, 8u) - inserted_bytes;
            } catch (...) {
                // 006985F7 uses CURRENT receiver end, not captured old end.
                const auto failed_current_end = load(receiver, 8u);
                const auto failed_offset = count << 4u;
                destroy_native_dword_vector_header_range_dispatch_004d9e40(
                    pointer(position_bits + failed_offset),
                    pointer(failed_current_end + failed_offset));
                throw;
            }
            // State0 restored BEFORE assignment; no short-tail rollback here.
            fill_native_checked_dword_vector_objects_00698010(
                position, pointer(assigned_end), private_fill);
        } else {
            const auto selected_tail = old_end - inserted_bytes;
            void* const returned_end =
                transfer_native_dword_vector_range_dispatch_00698350(
                    pointer(selected_tail), pointer(old_end), pointer(old_end));
            store(receiver, 8u, bits(returned_end));
            swap_native_dword_vector_headers_backward_dispatch_00698380(
                position, pointer(selected_tail), pointer(old_end));
            fill_native_checked_dword_vector_objects_00698010(
                position, pointer(position_bits + inserted_bytes), private_fill);
        }
        free_private_begin(private_fill);
    } catch (...) {
        tidy_native_word_vector_storage_004cdce0(private_fill);
        throw;
    }
}
} // namespace bsp
