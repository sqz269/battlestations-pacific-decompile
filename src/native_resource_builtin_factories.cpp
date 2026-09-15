#include "bsp/native_resource_builtin_factories.hpp"
#include "bsp/native_group_pool.hpp"
#include "bsp/native_node_pool_allocation.hpp"

namespace bsp {
NativeNodeStorage* create_native_plain_node_00b866c0(void* pool, const void* name,
    NativeStringRawPoolContext& strings, const NativeNodeRawConstants& constants) {
    void* const slot = allocate_native_node_00b6ed70(pool);
    if (!slot) return nullptr;
    try {
        return &construct_native_node_00b6f5a0(slot, native_node_pool_slot_bytes,
            name, strings, constants);
    } catch (...) {
        return_native_node_00b6e670(slot, pool);
        throw;
    }
}

NativeNodeStorage* create_native_resource_group_00b86780(const void* name,
    NativeStringRawPoolContext& strings, const NativeNodeRawConstants& constants) {
    void* const slot = allocate_native_group_slot_00b8f450();
    if (!slot) return nullptr;
    try {
        return &construct_native_group_00b8f5e0(slot, NativeGroupPool::slot_bytes,
            name, strings, constants).node;
    } catch (...) {
        return_native_group_slot_00b8eeb0(slot);
        throw;
    }
}
} // namespace bsp
