#pragma once

#include "bsp/native_resource_registry_node_leaves.hpp"

namespace bsp {

// Five live DWORD cells for native stacked left/parent/right/pair/color.
// This alias shares argument-cell mechanics, not nominal key/value types.
// Metadata must be disjoint from native payload, argument and local cells.
using NativeSceneRecordMapNodeArguments = NativeResourceRegistryNodeArguments;

// Complete 00469F50..00469FBF; native ECX actual1Ch node, five stacked words,
// RET14/EAX original node. Reuses the instruction-equivalent accepted B1ACA0
// raw body: links0/4/8, string length/data C/10, mapped DWORD14, color18,
// nil19=0; padding1A/1B untouched. Preserve capture/alias checks before stores,
// current pair reads after resize, and late color LOW-byte read. No nominal
// mapped-value ownership, local cleanup or rollback is added.
void* construct_native_scene_record_map_node_00469f50(void* actual_node,
    const NativeSceneRecordMapNodeArguments&, NativeStringRawPoolContext&);

// Complete 00468B30..00468B4C; ECX actual8h string header, RET, no semantic
// result. Reuses canonical raw0041DD20. Capture data before length+1 (DWORD
// wrap); nonnull data resolves current pool then returns the captured block.
// Null data skips length/getter/pool. Header remains untouched; failures escape.
// This ordinary C++ context interface does not reproduce the original ABI.
void destroy_native_scene_record_map_string_header_00468b30(
    void* actual_header, NativeStringRawPoolContext&);

} // namespace bsp
