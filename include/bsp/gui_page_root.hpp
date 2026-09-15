#pragma once
#include "bsp/native_node_construction.hpp"

namespace bsp {

// cGroup, the native188h plain GUI page root (also used outside GUI). The
// allocation is18Ch; trailing slab index188h is not part of this constructor.
struct NativeGroupTailStorage {
    std::uint8_t untouched_174;
    std::uint8_t enabled_175;
    std::array<std::byte, 2> untouched_176;
    CameraTransform** attached_nodes_178;
    std::int32_t attached_count_17c;
    std::int32_t attached_capacity_180;
    std::uint32_t scalar_184;
};
struct NativeGroupStorageView {
    NativeNodeStorage& node;
    NativeGroupTailStorage& group;
};
struct NativeGroupConstants {
    const volatile std::uint32_t& one_00d7a24c;
    const volatile std::uint32_t& bound_00ce4970;
};

// ECX actual raw slot, stack NativeString*, EAX same slot, RET4. Complete
// constructor over a supplied aligned actual18Ch allocation. Preserves all
// constructor-unwritten bytes and the pool index. Caller owns storage on error.
// Allocation00B8F450, type bootstrap, virtual/lifetime binding and deletion are
// separate required services; this routine does not fabricate a live root owner.
NativeGroupStorageView construct_native_group_00b8f5e0(void* actual_slot,
    std::size_t slot_bytes, const NativeString&, SizedStoragePool&, NativeGroupConstants);

// Same complete106-byte constructor for the resource factory's actual8h name
// header and current raw string pool. The base and tail read the same current
// constant cells in native order. No name copy or companion is synthesized.
NativeGroupStorageView construct_native_group_00b8f5e0(void* actual_slot,
    std::size_t slot_bytes, const void* actual_name_header,
    NativeStringRawPoolContext&, const NativeNodeRawConstants&);

} // namespace bsp
