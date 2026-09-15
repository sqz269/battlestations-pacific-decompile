#include "bsp/gui_page_root.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(NativeNodeStorage) == 0x174);
static_assert(sizeof(NativeGroupTailStorage) == 0x14);
static_assert(offsetof(NativeGroupTailStorage, enabled_175) == 1);
static_assert(offsetof(NativeGroupTailStorage, attached_nodes_178) == 4);
static_assert(offsetof(NativeGroupTailStorage, attached_count_17c) == 8);
static_assert(offsetof(NativeGroupTailStorage, attached_capacity_180) == 12);
static_assert(offsetof(NativeGroupTailStorage, scalar_184) == 16);

namespace {
void require_group_slot(void* slot, std::size_t bytes) {
    if (!slot || bytes < 0x18cu || reinterpret_cast<std::uintptr_t>(slot) % alignof(NativeNodeStorage))
        throw std::invalid_argument("native cGroup construction requires one aligned18Ch pool slot");
}
NativeGroupStorageView finish_group_construction(void* slot, NativeNodeStorage& node,
    NativeGroupConstants constants) {
    auto& group = *::new (static_cast<std::byte*>(slot) + 0x174) NativeGroupTailStorage;
    const auto one = constants.one_00d7a24c;
    node.vtable_00 = 0x00d634f8u;
    group.attached_nodes_178 = nullptr;
    group.attached_count_17c = 0;
    group.attached_capacity_180 = 0;
    node.auxiliary_flags_138 |= 2u;
    group.scalar_184 = one;
    group.enabled_175 = 1;
    const std::uint32_t zero = 0;
    for (std::size_t offset = 0; offset < 12; offset += 4)
        std::memcpy(node.untouched_08.data() + offset, &zero, 4);
    const auto bound = constants.bound_00ce4970;
    std::memcpy(node.untouched_08.data() + 12, &bound, 4);
    return {node, group};
}
} // namespace

NativeGroupStorageView construct_native_group_00b8f5e0(void* slot, std::size_t bytes,
    const NativeString& name, SizedStoragePool& strings, NativeGroupConstants constants) {
    require_group_slot(slot, bytes);
    auto& node = construct_native_node_00b6f5a0(slot, bytes, name, strings);
    return finish_group_construction(slot, node, constants);
}

NativeGroupStorageView construct_native_group_00b8f5e0(void* slot, std::size_t bytes,
    const void* name, NativeStringRawPoolContext& strings, const NativeNodeRawConstants& constants) {
    require_group_slot(slot, bytes);
    auto& node = construct_native_node_00b6f5a0(slot, bytes, name, strings, constants);
    return finish_group_construction(slot, node,
        {constants.one_00d7a24c, constants.positive_bound_00ce4970});
}
} // namespace bsp
