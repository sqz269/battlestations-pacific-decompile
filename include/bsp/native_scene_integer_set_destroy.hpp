#pragma once

#include "bsp/native_scene_integer_set_erase_range.hpp"

namespace bsp {

// Stable metadata is disjoint from all actual cells/payload it describes.
// range.actual_parameters20 binds this destructor's five outgoing words;
// actual_range_output8 separately binds its private8B result, not the range's
// own partial-erase private output8B. Child range backing/metadata is required
// only on reached paths under the complete actual range-provider contract.
struct NativeSceneIntegerSetDestroyFrame {
    const NativeSceneIntegerSetEraseRangeFrame& range;
    void* actual_range_output8;
};

// Three complete52B/23op bodies. Native ECX tree, incoming EDX unused, plain
// RET; EAX cleared to0, no semantic result established. Capture current head
// then its left link before outgoing stores: last-node, last-owner, first-node,
// first-owner, private-output. Call actual complete0046B170 with explicit raw
// backing, ignore its result, then reload CURRENT tree.head for actual current-
// CRT free. Finally publish head0 then count0. No initial guard, old-head free,
// rollback, nominal owner/key cleanup, string context or nil19h call is added.
void destroy_native_scene_integer_set_0046b530(void* actual_tree,
    const NativeSceneIntegerSetDestroyFrame&);
void destroy_native_scene_integer_set_0046c4c0(void* actual_tree,
    const NativeSceneIntegerSetDestroyFrame&);
void destroy_native_scene_integer_set_0046ce90(void* actual_tree,
    const NativeSceneIntegerSetDestroyFrame&);

// Ordinary Source with new explicit frame metadata; reached cells/payload may
// deliberately alias, while metadata/bindings remain stable. Valid backing and
// actual current-CRT releases are required. No original physical private-frame,
// register/stack/CRT/FH3/SEH binary ABI, direct native differential, application
// admission or game parity follows from this Source interface or its build.
// Correct native STL_Tree_Destroy names are retained; family names are inferred
// from actual raw storage/providers, not recovered class/ownership identities.

} // namespace bsp
