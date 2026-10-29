#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp::cc16_dyn_ring_init {

// Names describe the observed storage and are hypotheses, not recovered symbols.
struct DynRingOwnerCount {
    std::uint32_t opaque_00;
    std::uint32_t count_04;
};

// Native Win32 view of the object passed in ESI to 00C33510. The two arrays hold
// four-byte indices; +04h is untouched and only owner_08->count_04 is read.
struct DynRingIndices {
    std::uint32_t count_00;
    std::uint32_t opaque_04;
    const DynRingOwnerCount* owner_08;
    std::uint32_t* next_0c;
    std::uint32_t next_size_10;
    std::uint32_t next_capacity_14;
    std::uint32_t* previous_18;
    std::uint32_t previous_size_1c;
    std::uint32_t previous_capacity_20;
};

static_assert(sizeof(void*) == 4, "Dyn ring storage is the original Win32 layout");
static_assert(offsetof(DynRingOwnerCount, count_04) == 0x04);
static_assert(offsetof(DynRingIndices, owner_08) == 0x08);
static_assert(offsetof(DynRingIndices, next_0c) == 0x0c);
static_assert(offsetof(DynRingIndices, next_size_10) == 0x10);
static_assert(offsetof(DynRingIndices, next_capacity_14) == 0x14);
static_assert(offsetof(DynRingIndices, previous_18) == 0x18);
static_assert(offsetof(DynRingIndices, previous_size_1c) == 0x1c);
static_assert(offsetof(DynRingIndices, previous_capacity_20) == 0x20);
static_assert(sizeof(DynRingIndices) == 0x24);

// Required providers for the two original external call kinds. No allocator or
// free implementation is supplied here. Allocation sizes are byte counts after
// the original uint32 multiplication by four, including wraparound.
struct DynRingAllocationCalls {
    virtual ~DynRingAllocationCalls() = default;
    virtual std::uint32_t* allocate_00bf55be(std::uint32_t byte_count) = 0;
    virtual void free_00bf6989(std::uint32_t* block) = 0;
};

// 00C33510..00C33645: ESI = &state, no stack arguments, plain RET. There is no
// semantic return value. This C++ interface does not implement that register ABI.
// The caller must supply native valid storage. In particular, the image does not
// guard zero counts, allocation failure, or the final two endpoint stores.
void initialize_dyn_ring_indices_00c33510(DynRingIndices& state,
                                         DynRingAllocationCalls& calls);

}  // namespace bsp::cc16_dyn_ring_init
