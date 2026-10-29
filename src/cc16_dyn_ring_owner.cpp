#include "bsp/cc16_dyn_ring_owner.hpp"

#include <cstring>

namespace bsp::cc16_dyn_ring_owner {
namespace {

template <class T>
T* at_byte_offset(T* base, std::uint32_t bytes) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(base) + bytes;
    return reinterpret_cast<T*>(address);
}

template <class T>
T* at_index(T* base, std::uint32_t index) noexcept {
    return at_byte_offset(base, index * static_cast<std::uint32_t>(sizeof(T)));
}

bool signed_less_32(std::uint32_t left, std::uint32_t right) noexcept {
    return (left ^ 0x80000000u) < (right ^ 0x80000000u);
}

std::uint32_t chain_size(const DynIndexChain& chain) noexcept {
    return static_cast<std::uint32_t>(chain.size_04);
}

void set_chain_size(DynIndexChain& chain, std::uint32_t size) noexcept {
    // Preserve raw ADD/MOV bits without signed-overflow arithmetic.
    std::memcpy(&chain.size_04, &size, sizeof(size));
}

class RingAllocationAdapter final : public cc16_dyn_ring_init::DynRingAllocationCalls {
public:
    explicit RingAllocationAdapter(const cc15_dyn_live::DynLiveCrt& crt) : crt_(crt) {}

    std::uint32_t* allocate_00bf55be(std::uint32_t bytes) override {
        return static_cast<std::uint32_t*>(crt_.operator_new_00bf55be(bytes));
    }

    void free_00bf6989(std::uint32_t* block) override {
        crt_.free_00bf6989(block);
    }

private:
    const cc15_dyn_live::DynLiveCrt& crt_;
};

// 00C4D654..00C4D730. ESI is exactly zero: MOV ESI,ECX; SUB ESI,ECX.
void prepend_previous(DynRingOwner& owner, const cc15_dyn_live::DynLiveCrt& crt) {
    DynIndexChain& scratch = owner.scratch_24;
    if (chain_size(scratch) == scratch.capacity_08) {
        const std::uint32_t capacity = scratch.capacity_08 * 2u + 1u;
        scratch.capacity_08 = capacity;
        auto* const grown = static_cast<std::uint32_t*>(
            crt.operator_new_00bf55be(capacity * 4u));
        if (grown != nullptr) {
            *grown = owner.previous_cursor_34;
        }
        // The prefix-copy loop is empty because its bound ESI is zero.
        for (std::uint32_t index = 0; index < chain_size(scratch); ++index) {
            std::uint32_t* const slot = at_index(grown, index + 1u);
            if (slot != nullptr) {
                *slot = *at_index(scratch.data_00, index);
            }
        }
        if (scratch.data_00 != nullptr) {
            crt.free_00bf6989(scratch.data_00);
        }
        set_chain_size(scratch, chain_size(scratch) + 1u);
        scratch.data_00 = grown;
    } else {
        std::uint32_t* const end_slot = at_index(scratch.data_00, chain_size(scratch));
        if (end_slot != nullptr) {
            *end_slot = *at_byte_offset(end_slot, 0xfffffffcu);
        }
        std::uint32_t index = chain_size(scratch) - 1u;
        while (index > 0) {
            std::uint32_t* const slot = at_index(scratch.data_00, index);
            const std::uint32_t value = *at_byte_offset(slot, 0xfffffffcu);
            --index;
            *slot = value;
        }
        // No null check here. In particular, size==0 with spare capacity is
        // not repaired: the preceding subtraction and shift loop wrap.
        *scratch.data_00 = owner.previous_cursor_34;
        set_chain_size(scratch, chain_size(scratch) + 1u);
    }
}

// 00C4D74C..00C4D7B5: the append growth rule is capacity*2+2.
void append_next(DynRingOwner& owner, const cc15_dyn_live::DynLiveCrt& crt) {
    DynIndexChain& scratch = owner.scratch_24;
    if (chain_size(scratch) == scratch.capacity_08) {
        const std::uint32_t capacity = scratch.capacity_08 * 2u + 2u;
        scratch.capacity_08 = capacity;
        auto* const grown = static_cast<std::uint32_t*>(
            crt.operator_new_00bf55be(capacity * 4u));
        for (std::uint32_t index = 0; index < chain_size(scratch); ++index) {
            std::uint32_t* const slot = at_index(grown, index);
            if (slot != nullptr) {
                *slot = *at_index(scratch.data_00, index);
            }
        }
        if (scratch.data_00 != nullptr) {
            crt.free_00bf6989(scratch.data_00);
        }
        scratch.data_00 = grown;
    }
    std::uint32_t* const slot = at_index(scratch.data_00, chain_size(scratch));
    if (slot != nullptr) {
        *slot = owner.next_cursor_30;
    }
    set_chain_size(scratch, chain_size(scratch) + 1u);
}

// 00C4D7BA..00C4D83D: capture the pair before any allocator call.
void append_pair(DynRingOwner& owner, const cc15_dyn_live::DynLiveCrt& crt) {
    const std::uint32_t next = owner.next_cursor_30;
    const std::uint32_t previous = owner.previous_cursor_34;
    DynIndexPairArray& pairs = owner.pairs_44;
    if (pairs.size_04 == pairs.capacity_08) {
        const std::uint32_t capacity = pairs.capacity_08 * 2u + 2u;
        pairs.capacity_08 = capacity;
        auto* const grown = static_cast<DynIndexPair*>(
            crt.operator_new_00bf55be(capacity * 8u));
        for (std::uint32_t index = 0; index < pairs.size_04; ++index) {
            DynIndexPair* const slot = at_index(grown, index);
            if (slot != nullptr) {
                const DynIndexPair* const source = at_index(pairs.data_00, index);
                slot->previous_00 = source->previous_00;
                slot->next_04 = source->next_04;
            }
        }
        if (pairs.data_00 != nullptr) {
            crt.free_00bf6989(pairs.data_00);
        }
        pairs.data_00 = grown;
    }
    DynIndexPair* const slot = at_index(pairs.data_00, pairs.size_04);
    if (slot != nullptr) {
        slot->previous_00 = previous;
        slot->next_04 = next;
    }
    ++pairs.size_04;
}

// 00C4D859..00C4D92F: deep-copy each old chain, then free that old child.
void append_chain(DynRingOwner& owner, const cc15_dyn_live::DynLiveCrt& crt) {
    DynIndexChainArray& chains = owner.chains_38;
    if (chains.size_04 == chains.capacity_08) {
        const std::uint32_t capacity = chains.capacity_08 * 2u + 2u;
        chains.capacity_08 = capacity;
        auto* const grown = static_cast<DynIndexChain*>(
            crt.operator_new_00bf55be(capacity * 12u));
        for (std::uint32_t index = 0; index < chains.size_04; ++index) {
            DynIndexChain* const slot = at_index(grown, index);
            if (slot != nullptr) {
                cc15_dyn_live::vector4_copy_construct_0040d7b0(
                    slot, at_index(chains.data_00, index), crt);
            }
            // The original leaves this old child pointer intact after free.
            std::uint32_t* const old_child = at_index(chains.data_00, index)->data_00;
            if (old_child != nullptr) {
                crt.free_00bf6989(old_child);
            }
        }
        if (chains.data_00 != nullptr) {
            crt.free_00bf6989(chains.data_00);
        }
        chains.data_00 = grown;
    }
    DynIndexChain* const slot = at_index(chains.data_00, chains.size_04);
    if (slot != nullptr) {
        cc15_dyn_live::vector4_copy_construct_0040d7b0(slot, &owner.scratch_24, crt);
    }
    ++chains.size_04;
}

// FuncInfo 00E04138, unwind map 00E04100: states4..0 clean +44,+38,+24,
// +18,+0C. This is the observed helper effect, not new helper-body credit.
// States5/6 call verified RET-only00401130 and add no private-buffer rollback.
void unwind_constructed_members(DynRingOwner& owner,
                                const cc15_dyn_live::DynLiveCrt& crt) noexcept {
    if (owner.pairs_44.data_00 != nullptr) {
        crt.free_00bf6989(owner.pairs_44.data_00);
    }
    if (owner.chains_38.data_00 != nullptr) {
        for (std::uint32_t index = 0; index < owner.chains_38.size_04; ++index) {
            std::uint32_t* const child = at_index(owner.chains_38.data_00, index)->data_00;
            if (child != nullptr) {
                crt.free_00bf6989(child);
            }
        }
        crt.free_00bf6989(owner.chains_38.data_00);
    }
    if (owner.scratch_24.data_00 != nullptr) {
        crt.free_00bf6989(owner.scratch_24.data_00);
    }
    if (owner.ring_00.previous_18 != nullptr) {
        crt.free_00bf6989(owner.ring_00.previous_18);
    }
    if (owner.ring_00.next_0c != nullptr) {
        crt.free_00bf6989(owner.ring_00.next_0c);
    }
}

}  // namespace

DynRingOwner* construct_dyn_ring_owner_00c4d5a0(
    DynRingOwner& destination,
    const cc16_dyn_ring_init::DynRingOwnerCount& input_owner,
    const cc15_dyn_live::DynLiveCrt& crt,
    DynRingOwnerCalls& calls) {
    DynRingOwner& owner = destination;
    owner.ring_00.owner_08 = &input_owner;
    owner.ring_00.next_0c = nullptr;
    owner.ring_00.next_size_10 = 0;
    owner.ring_00.next_capacity_14 = 0;
    owner.ring_00.previous_18 = nullptr;
    owner.ring_00.previous_size_1c = 0;
    owner.ring_00.previous_capacity_20 = 0;
    owner.scratch_24 = {nullptr, 0, 0};
    owner.chains_38 = {nullptr, 0, 0};
    owner.pairs_44 = {nullptr, 0, 0};

    try {
        RingAllocationAdapter allocation(crt);
        cc16_dyn_ring_init::initialize_dyn_ring_indices_00c33510(owner.ring_00, allocation);
        owner.ring_00.opaque_04 = 0;

        while (!signed_less_32(owner.ring_00.count_00, 3u)) {
            calls.prepare_chain_00c4d370(owner, crt);
            std::uint32_t remaining = owner.ring_00.count_00 - 3u;
            while (remaining != 0) {
                if (calls.test_previous_00c41230(owner) != 0) {
                    owner.previous_cursor_34 = *at_index(
                        owner.ring_00.previous_18, owner.previous_cursor_34);
                    prepend_previous(owner, crt);
                    --remaining;
                } else if (calls.test_next_00c41190(owner) != 0) {
                    owner.next_cursor_30 = *at_index(
                        owner.ring_00.next_0c, owner.next_cursor_30);
                    append_next(owner, crt);
                    --remaining;
                } else {
                    append_pair(owner, crt);
                    break;
                }
            }

            *at_index(owner.ring_00.next_0c, owner.previous_cursor_34) = owner.next_cursor_30;
            *at_index(owner.ring_00.previous_18, owner.next_cursor_30) = owner.previous_cursor_34;
            append_chain(owner, crt);
            owner.ring_00.count_00 += 2u - chain_size(owner.scratch_24);
            if (chain_size(owner.scratch_24) != 0) {
                set_chain_size(owner.scratch_24, 0);
            }
            owner.ring_00.opaque_04 = owner.next_cursor_30;
        }
        return &owner;
    } catch (...) {
        unwind_constructed_members(owner, crt);
        throw;
    }
}

}  // namespace bsp::cc16_dyn_ring_owner
