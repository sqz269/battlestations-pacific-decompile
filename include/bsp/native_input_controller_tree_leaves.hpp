#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual controller-tree leaves require MSVC Win32.
#endif

namespace bsp {
// Whole fixed-nil21 controller instances: links DWORD0/4/8, tree head DWORD4,
// actual eight-byte iterator owner/node. No color, payload, pool context,
// allocator, count or container model is consumed by these five functions.
// Preserve established STL identities, including the left Unk_N24 name.
// Descriptive controller C++ names are hypotheses, not recovered symbols.

// Native ECX actual iterator, incoming EDX unused, no stack words, plain RET
// or nil-node diagnostic tail. Owner diagnostic may return; reload CURRENT
// iterator.node afterward. During the parent climb reload CURRENT node before
// parent.right comparison; publish parent before reading its parent. Source
// void excludes incidental native EAX/register/flag and private-frame identity.
void __fastcall increment_native_input_controller_tree_iterator_0069ad50(
    void* actual_iterator, std::uint32_t unused_edx);

// Native ECX tree, incoming EDX unused, one stacked node and RET4. Reread the
// pivot child AFTER the first node-child store; test CURRENT child nil21,
// publish pivot.parent, capture CURRENT head once for root comparison. On
// nonroot reload node.parent: right tests parent.right first, left parent.left.
// Side/root publication precedes ordered pivot-child then node-parent stores.
// Source void excludes native incidental EAX; no rollback or snapshot is added.
void __fastcall rotate_native_input_controller_tree_right_0069a6a0(
    void* actual_tree, std::uint32_t unused_edx, void* actual_node);
void __fastcall rotate_native_input_controller_tree_left_0069c280(
    void* actual_tree, std::uint32_t unused_edx, void* actual_node);

// Native ECX subtree, EDX unconsumed, plain RET/EAX selected actual node. Read
// successive CURRENT child/nil21 fields; no initial-node guard. Maximum has
// 25 defined bytes/10 starts in two ranges, excluding its undefined three bytes;
// minimum has 27 bytes/10 starts, including the native LEA alignment instruction.
void* __fastcall maximum_native_input_controller_node_0069aa60(void* actual_subtree);
void* __fastcall minimum_native_input_controller_node_0069aa80(void* actual_subtree);

// All public raw DWORD/BYTE accesses are volatile. Reached storage must be
// valid and walks finite. Only accepted current-SDK BF6713 is called; it may
// return or throw. No nil19/29/15/99 delegate, nil parameter, callback, extra
// guard, owning action, local EH, catch or noexcept bridge. Old typed/AV
// interfaces remain separate. New Source private arguments/frames differ;
// original CRT/heap/handler/FH3/RTTI/SEH/fault/register/flags/binary ABI and game
// equivalence remain unproved. Whole controller eraser/pair remain outside.
} // namespace bsp
