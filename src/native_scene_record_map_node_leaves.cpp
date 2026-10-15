#include "bsp/native_scene_record_map_node_leaves.hpp"

#include "bsp/native_string.hpp"

namespace bsp {

void* construct_native_scene_record_map_node_00469f50(void* actual_node,
    const NativeSceneRecordMapNodeArguments& arguments,
    NativeStringRawPoolContext& strings) {
    return construct_native_resource_registry_node_00b1aca0(actual_node, arguments, strings);
}

void destroy_native_scene_record_map_string_header_00468b30(
    void* actual_header, NativeStringRawPoolContext& strings) {
    destroy_native_string_header_0041dd20(actual_header, strings);
}

} // namespace bsp
