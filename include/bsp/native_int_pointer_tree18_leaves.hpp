#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native IntPointerTree18 leaves require MSVC Win32.
#endif

namespace bsp {

// Complete raw tree leaves. Tree header: untouched allocator+0, head+4,
// count+8. Nodes: links+0/+4/+8, unowned key/value+0C/+10, color+14, nil+15,
// remaining padding+16/+17. Checked iterator: owner+0, node+4.
// No std::map projection, payload destruction or new ownership policy.

// 8697B0[28]/8697D0[27]: ECX subtree, EAX extreme node, RET. EDX unconsumed.
void* __fastcall maximum_native_int_pointer_tree18_008697b0(void* subtree);
void* __fastcall minimum_native_int_pointer_tree18_008697d0(void* subtree);

// 869810[82]/86A2F0[78]: ECX tree, EDX unconsumed, stack pivot, RET4.
// Complete root/left/right-parent cases and nil-child parent-update branch.
// Preserve current rereads and write order; no semantic return value.
void __fastcall rotate_right_native_int_pointer_tree18_00869810(
    void* tree, void* unused_edx, void* pivot);
void __fastcall rotate_left_native_int_pointer_tree18_0086a2f0(
    void* tree, void* unused_edx, void* pivot);

// 869A20[99]: ECX actual iterator, RET. Null-owner validation may return;
// then reread current node. Nil current node tails the real invalid service.
// Right-subtree minimum or parent climb; only node+4 is written.
void __fastcall increment_native_int_pointer_tree18_00869a20(void* iterator);

// 86AA60[53]: ECX tree retained through recursion, EDX unconsumed, stack
// subtree, RET4. Recurse right, capture left, free current, continue left.
// Read no key/value and perform no payload release or header updates.
void __fastcall erase_subtree_native_int_pointer_tree18_0086aa60(
    void* tree, void* unused_edx, void* subtree);

// 86AC00[55]: no consumed inputs, EAX raw18h allocation, RET. Preserve the
// original LEA/TEST/store branches; write links and color/nil only. A null
// allocation is not a successful result. Uses actual source CRT allocation.
void* __cdecl allocate_native_int_pointer_tree18_node_0086ac00();

// Memory belongs to actual singleton_lifetime_allocate/free. Validation uses
// fixed SDK _invalid_parameter_noinfo, including a returning installed handler.
// Original CRT heap/encoded-handler domain and provider register/throw identity
// are explicit boundaries. No new EH cleanup, original binary compatibility
// or gameplay claim. Descriptive tree names are hypotheses, not symbols.
} // namespace bsp
