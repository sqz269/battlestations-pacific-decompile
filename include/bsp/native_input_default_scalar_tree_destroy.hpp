#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual default scalar-tree destruction requires MSVC Win32.
#endif

namespace bsp {
// Whole006A1DF0..006A1E23:52B/23starts. Preserve the established STL_Tree_Destroy
// identity; "default scalar" describes its consumed InputSettings+54h tree.
// Native ECX=actual12B tree, EDX unconsumed, no stack arguments/plainRET,
// EAX=0. This Source retains that input/return shape, not the original frame.
//
// Capture head at+4 then head.left. Pass those captures and the same tree as
// both iterator owners to actual whole0069FE70, with a private8B output that
// is ignored. Only after success reload CURRENT tree+4 and free it; clear
// tree+4 BEFORE tree+8, then return0. Opaque+0 remains untouched. Child failure
// propagates before this entry's free/stores; no local EH/rollback or guard.
std::uint32_t __fastcall destroy_native_input_default_scalar_tree_006a1df0(
    void* tree, std::uint32_t unused_edx);

// Requires valid reached actual tree/head/node storage and matching currentCRT
// ownership, with the tree writable through post-free clears. Accesses use
// volatile rawDWORDs and unsigned32 address wrap; no host tree or provider.
// Child EDX is canonically0: original unused register residue is not modelled.
// Original private output/argument aliases, recursive origins, original heap,
// CRT/FH3/SEH/RTTI/fault/ambient state and game compatibility remain unproved.
// CurrentCpp child exception transport and established progression are reused.
// Existing AV/typed domains and whole-range/full-prefix records stay separate.
} // namespace bsp
