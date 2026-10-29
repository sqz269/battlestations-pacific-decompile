#include "bsp/cc16_dyn_ring_init.hpp"

namespace bsp::cc16_dyn_ring_init {
namespace {

// LEA/ADD in the image wrap in a 32-bit address register. Integer address
// arithmetic also preserves its per-slot null checks without null-pointer math.
std::uint32_t* word_at_byte_offset(std::uint32_t* base,
                                  std::uint32_t byte_offset) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(base) + byte_offset;
    return reinterpret_cast<std::uint32_t*>(address);
}

std::uint32_t* word_at_index(std::uint32_t* base, std::uint32_t index) noexcept {
    return word_at_byte_offset(base, index * 4u);
}

bool signed_less_32(std::uint32_t left, std::uint32_t right) noexcept {
    return (left ^ 0x80000000u) < (right ^ 0x80000000u);
}

// The two inlined vector resizes have the same instruction sequence. Sizes and
// capacities use unsigned comparisons; shrinking retains the allocation.
void resize_indices(std::uint32_t*& elements, std::uint32_t& size,
                    std::uint32_t& capacity, std::uint32_t requested,
                    DynRingAllocationCalls& calls) {
    if (requested > size) {
        if (requested > capacity) {
            const std::uint32_t byte_count = requested * 4u;
            capacity = requested;  // Before CALL, including when it throws.
            std::uint32_t* const grown = calls.allocate_00bf55be(byte_count);
            for (std::uint32_t index = 0; index < size; ++index) {
                std::uint32_t* const slot = word_at_index(grown, index);
                if (slot != nullptr) {
                    *slot = *word_at_index(elements, index);
                }
            }
            std::uint32_t* const old = elements;
            if (old != nullptr) {
                calls.free_00bf6989(old);
            }
            elements = grown;
        }
        for (std::uint32_t index = size; index < requested; ++index) {
            std::uint32_t* const slot = word_at_index(elements, index);
            if (slot != nullptr) {
                *slot = 0;
            }
        }
    }
    size = requested;
}

}  // namespace

void initialize_dyn_ring_indices_00c33510(DynRingIndices& state,
                                         DynRingAllocationCalls& calls) {
    const std::uint32_t requested = state.owner_08->count_04;
    state.count_00 = requested;  // 00C33510..00C33518

    // 00C3351A..00C3358A: previous array, then 00C3358D..00C335FF: next.
    resize_indices(state.previous_18, state.previous_size_1c,
                   state.previous_capacity_20, requested, calls);
    resize_indices(state.next_0c, state.next_size_10, state.next_capacity_14,
                   state.count_00, calls);

    // 00C33602 JLE and 00C3362A JL are signed, unlike the resize branches.
    if (signed_less_32(0, state.count_00)) {
        std::uint32_t byte_offset = 0;
        std::uint32_t next_index = 1;
        do {
            *word_at_byte_offset(state.previous_18, byte_offset) = next_index - 2u;
            *word_at_byte_offset(state.next_0c, byte_offset) = next_index;
            ++next_index;
            byte_offset += 4u;
        } while (signed_less_32(next_index - 1u, state.count_00));
    }

    // 00C3362C..00C33643 run even if the signed loop was skipped. Keep the
    // count reload after the previous-array store and the wrapping last index.
    const std::uint32_t last_previous = state.count_00 - 1u;
    *state.previous_18 = last_previous;
    const std::uint32_t last_next = state.count_00 - 1u;
    *word_at_index(state.next_0c, last_next) = 0;
}

}  // namespace bsp::cc16_dyn_ring_init
