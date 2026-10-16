#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual input int/string tree leaves require MSVC Win32.
#endif

namespace bsp {
// Fixed actual node links0/4/8, nil BYTE19h; tree opaque0/head4/count8 and
// checked iterator owner0/node4. No host node/owner/layout/context projection.
// Existing STL identities and typed/AV domains are retained; descriptive
// int/string instantiation names are hypotheses, not recovered symbols.

// Whole0069AE50[99B/38starts]: Native ECX actual8B iterator, EDX unused,
// no stacked args, RET or invalid-parameter tail. After returning null-owner
// diagnostic reread CURRENT node. Nil node diagnoses and returns without
// advance. Otherwise right minimum or progressive CURRENT-node parent climb;
// only iterator.node4 is written. Source void qualifies Native EAX residue.
void __fastcall increment_native_input_int_string_tree_iterator_0069ae50(
    void* actual_iterator, std::uint32_t unused_edx);

// Whole0069A780[82B/30starts] and0069C300[78B/30starts]: Native ECX tree,
// EDX unused, stacked node, RET4. Preserve pivot-child reread AFTER node-child
// store, nil19 branch, parent publication, ONCE captured head/root test,
// reloaded node.parent and direction-specific first side test, final ordered
// pivot-child/node-parent stores. No semantic EAX result; Source returns void.
void __fastcall rotate_native_input_int_string_tree_right_0069a780(
    void* actual_tree, std::uint32_t unused_edx, void* actual_node);
void __fastcall rotate_native_input_int_string_tree_left_0069c300(
    void* actual_tree, std::uint32_t unused_edx, void* actual_node);

// Whole maximum25definedB/10starts in a28B envelope; unreachable3B at
// 0069AACD..CF remain undefined/unpromoted. Minimum27B/10starts. Native ECX
// subtree, EDX unconsumed, RET/EAX selected node. Repeated current child/nil19
// reads, no initial-node null/nil guard. The public Source retains that shape.
void* __fastcall maximum_native_input_int_string_node_0069aac0(void* actual_subtree);
void* __fastcall minimum_native_input_int_string_node_0069aae0(void* actual_subtree);

// Direct raw volatile DWORD/BYTE accesses and accepted returning current-SDK
// BF6713 service. No nil15/99 delegate, template direction/context, callback,
// allocation, new guard, rollback, local EH or noexcept bridge. Diagnostics
// may return or throw; valid storage must support every reached native access.
// Physical register/stack shapes do not establish original binary ABI,
// incidental registers/flags, private-frame aliases, CRT/FH3/SEH/RTTI/fault
// identity, application adoption or game execution. See the packet report.
} // namespace bsp
