#include "bsp/native_scene_record_map_node_allocate.hpp"

namespace bsp {

void* allocate_native_scene_record_map_node_0046a340(
    const NativeSceneRecordMapNodeAllocationFrame& frame,
    NativeStringRawPoolContext& strings, NativeSceneRecordMapNodeAcquired& acquired) {
    // Full 0046A340/B1AD10 bodies, their catch/placement actions, EH maps and
    // 00469F50/B1ACA0 constructors are equivalent after the verified address
    // relocations. Reuse the complete concrete provider without a new service.
    return allocate_native_resource_registry_node_00b1ad10(frame, strings, acquired);
}

} // namespace bsp
