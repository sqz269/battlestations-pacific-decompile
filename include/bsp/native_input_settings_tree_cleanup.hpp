#pragma once
#include "bsp/native_input_keyboard_apply.hpp"
#include "bsp/native_string.hpp"

namespace bsp {
// Actual native checked trees: opaque/header/count, nodes left/parent/right.
// Complete normal subtree cleanup and nested device/controller payload release.
// Subtree entry takes ECX tree and one stack node (RET4). Device payload entry
// takes ECX self and no stack arguments. These explicit-service APIs are new ABIs.
void destroy_native_input_device_subtree_006a7540(void* tree, void* node, NativeStringStorage&);
void destroy_native_input_device_record_0055b690(void* self, NativeStringStorage&);

// Original ECX tree; stack output, first owner/node, last owner/node; EAX output,
// RET14h. Full-range reset and partial-range successor/transplant/rebalance paths.
// Payload destruction precedes node release; current count is decremented after
// release. Output receives owner before node, after capturing the resulting node.
void* erase_native_input_device_tree_range_006a7aa0(void*, void*,
    NativeKeyboardTreeIterator, NativeKeyboardTreeIterator, NativeStringStorage&);
void* erase_native_input_controller_tree_range_006a6a20(void*, void*,
    NativeKeyboardTreeIterator, NativeKeyboardTreeIterator, NativeStringStorage&);
void* erase_native_input_default_tree_range_0069fe70(void*, void*,
    NativeKeyboardTreeIterator, NativeKeyboardTreeIterator, NativeStringStorage&);
void* erase_native_input_preset_tree_range_006a1aa0(void*, void*,
    NativeKeyboardTreeIterator, NativeKeyboardTreeIterator, NativeStringStorage&);

// 0055B230: same iterator ABI; outer signed-key node20h/nil1Dh owns an
// inner scalar tree with node18h/nil15h. Full and partial range erasure.
void* erase_native_input_scale_tree_range_0055b230(void*, void*,
    NativeKeyboardTreeIterator, NativeKeyboardTreeIterator, NativeStringStorage&);

// Full-range4CEF40 contract consumed by game lifetime. Reuses the existing
// actual int-only node14h/nil11 implementation used by device-record cleanup.
// Rejects other ranges;does not expose a new generic partial-erase interface.
void* clear_native_input_int_only_full_range_004cef40(void* tree,void* output,
    void* first_owner,void* first_node,void* last_owner,void* last_node,NativeStringStorage&);

// Valid consistent owned trees and forward ranges are required. Release callbacks
// must not mutate tree topology or source iterator storage. Source CRT/string and
// exception services retain their existing boundaries. Original private-stack/FH3
// aliasing, exception ABI, malformed-tree and hardware-fault behavior are unvalidated.
} // namespace bsp
