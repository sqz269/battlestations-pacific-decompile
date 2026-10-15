#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene integer-set cleanup requires MSVC Win32.
#endif

namespace bsp {

// Complete00468300. Native ECX tree (forwarded but not dereferenced), one
// stacked node, RET4; EDX unused. Nodes have left0,parent4,right8,keyC,
// color10,nil11 in14h bytes. Visit right recursively, capture left, free the
// current node through the real current-CRT provider, then inspect captured
// left.nil. An initial nil node is inspected and retained. No key destruction.
void __fastcall destroy_native_scene_integer_set_subtree_00468300(
    void* tree, std::uint32_t unused_edx, void* node);

// Complete00468980. Native ECX tree, RET; EDX unused. Destroy current root,
// then reset current head.parent, count, captured head.left, current head.right.
// Tree+0 and the sentinel allocation survive. No semantic return value.
void __fastcall clear_native_scene_integer_set_00468980(
    void* tree, std::uint32_t unused_edx);

// Borrow valid reached native-layout storage, including nil sentinels, and
// current-CRT compatible node allocations. These Source interfaces do not
// establish original register/CRT/heap ABI identity or game behavior.
} // namespace bsp
