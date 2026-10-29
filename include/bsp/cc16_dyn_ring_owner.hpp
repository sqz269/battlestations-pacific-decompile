#pragma once

#include "bsp/cc15_dyn_live.hpp"
#include "bsp/cc16_dyn_ring_init.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp::cc16_dyn_ring_owner {

using DynIndexChain = cc15_dyn_live::DynVector4;

struct DynIndexChainArray {
    DynIndexChain* data_00;
    std::uint32_t size_04;
    std::uint32_t capacity_08;
};

struct DynIndexPair {
    std::uint32_t previous_00;
    std::uint32_t next_04;
};

struct DynIndexPairArray {
    DynIndexPair* data_00;
    std::uint32_t size_04;
    std::uint32_t capacity_08;
};

// Native storage observed in 00C4D5A0. Names are hypotheses. In the reused
// ring view, opaque_04 is the current anchor in this parent; the owner input's
// first dword is not interpreted here. Cursor +30h/+34h are not zeroed by this
// constructor: their preparation belongs to the required 00C4D370 contract.
struct DynRingOwner {
    cc16_dyn_ring_init::DynRingIndices ring_00;
    DynIndexChain scratch_24;
    std::uint32_t next_cursor_30;
    std::uint32_t previous_cursor_34;
    DynIndexChainArray chains_38;
    DynIndexPairArray pairs_44;
};

static_assert(sizeof(DynIndexChainArray) == 0x0c);
static_assert(sizeof(DynIndexPair) == 0x08);
static_assert(sizeof(DynIndexPairArray) == 0x0c);
static_assert(offsetof(DynRingOwner, scratch_24) == 0x24);
static_assert(offsetof(DynRingOwner, next_cursor_30) == 0x30);
static_assert(offsetof(DynRingOwner, previous_cursor_34) == 0x34);
static_assert(offsetof(DynRingOwner, chains_38) == 0x38);
static_assert(offsetof(DynRingOwner, pairs_44) == 0x44);
static_assert(sizeof(DynRingOwner) == 0x50);

// Required in-house dependencies whose bodies are not supplied by this packet.
// Their actual receiver storage is passed intact; there are no fallback bodies.
struct DynRingOwnerCalls {
    virtual ~DynRingOwnerCalls() = default;
    // 00C4D370: ESI = owner, no stack arguments, plain RET; EAX is ignored.
    // Its own allocations must use the same CRT custody as this constructor.
    virtual void prepare_chain_00c4d370(
        DynRingOwner& owner, const cc15_dyn_live::DynLiveCrt& crt) = 0;
    // 00C41230 / 00C41190: ECX = owner, no stack arguments, plain RET.
    // This parent tests the whole EAX value for zero, rather than just AL.
    virtual std::uint32_t test_previous_00c41230(DynRingOwner& owner) = 0;
    virtual std::uint32_t test_next_00c41190(DynRingOwner& owner) = 0;
};

// 00C4D5A0..00C4D971: two stack arguments (destination, input owner), RET 8,
// EAX = destination. This C++ interface does not supply that binary ABI/FH3
// transport. All buffers, leaf allocation, and vector copies share the supplied
// CRT. The original unchecked/wrapping memory and loop behavior is retained.
DynRingOwner* construct_dyn_ring_owner_00c4d5a0(
    DynRingOwner& destination,
    const cc16_dyn_ring_init::DynRingOwnerCount& input_owner,
    const cc15_dyn_live::DynLiveCrt& crt,
    DynRingOwnerCalls& calls);

}  // namespace bsp::cc16_dyn_ring_owner
