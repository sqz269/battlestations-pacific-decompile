#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual preset-tree leaves require MSVC Win32.
#endif

namespace bsp {
// Whole fixed-nil29 ordinary entries. Tree head is DWORD+4; node links are
// DWORD0/4/8 and nil is BYTE29h. Iterator is actual8B owner/node words.
// Native ECX supplies tree/iterator/subtree, EDX is unconsumed. Rotations have
// one stacked node/RET4; successor and extrema have no stack args/plainRET.
// Extrema return the actual selected node. Source void for rotations/successor
// excludes incidental Native EAX/flags. Names describe provisional instances.
void __fastcall increment_native_input_preset_tree_iterator_0069c0f0(
    void* actual_iterator, std::uint32_t unused_edx);
void __fastcall rotate_native_input_preset_tree_right_0069a560(
    void* actual_tree, std::uint32_t unused_edx, void* actual_node);
void __fastcall rotate_native_input_preset_tree_left_0069c1c0(
    void* actual_tree, std::uint32_t unused_edx, void* actual_node);
void* __fastcall maximum_native_input_preset_node_0069a9e0(void* actual_subtree);
void* __fastcall minimum_native_input_preset_node_0069a530(void* actual_subtree);

// Successor's owner diagnostic may return; reload CURRENT node afterward.
// Nil node reports and exits. Parent climb reloads CURRENT iterator.node before
// each parent.right comparison; publish a matched parent before its parent read.
// Rotations reread pivot child after the first node-child store; retain current
// parent/head captures and direction-specific parent-side tests, then ordered
// final pivot-child/node-parent stores. Extrema repeatedly read child/nil29,
// without a new initial-node guard. No payload/color/count/header ownership.
//
// Public accesses are volatile raw DWORD/BYTE operations. Valid reached raw
// storage and finite loops are required. Only accepted current SDK BF6713 is
// called; no callback, node overlay, nil-layout adapter, extra guard, allocator,
// cleanup, catch or rollback. Private Source frames and argument aliases differ.
// Original CRT/handler, FH3/SEH/RTTI/fault/heap/register/flags/binary ABI and game
// equivalence remain unproved. Published nil19/15/99 domains remain unchanged.
} // namespace bsp
