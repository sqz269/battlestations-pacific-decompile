#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene integer set increment requires MSVC Win32.
#endif

namespace bsp {

// Complete 00466AE0..00466B42 [99B]. Native ECX actual iterator8, owner+0,
// live node+4; incoming EDX unused, plain RET or validation tail JMP, no result.
// A zero owner invokes returning current CRT, then CURRENT node is loaded.
// Nil11h current reports and returns without another publication. Otherwise
// descend right then leftmost, or ascend with CURRENT node/parent.right reads,
// intermediate parent publication before parent.parent reload, and unconditional
// final parent publication. No decrement-style final validation is introduced.
void __fastcall increment_native_scene_integer_set_iterator_00466ae0(
    void* actual_iterator8);

// Complete 12B wrappers: native ECX actual iterator, plain RET; each calls the
// concrete nil11h increment and returns its original backing address in EAX.
void* __fastcall increment_native_scene_integer_set_iterator_00467b30(
    void* actual_iterator8);
void* __fastcall increment_native_scene_integer_set_iterator_00468340(
    void* actual_iterator8);

// Distinct ordinary raw14h-node Source, no nil19h body call or owner snapshot.
// Every external load/store and returning-handler effect is retained, including
// alias-sensitive parent reloads. Valid taken raw backing remains required.
// No callback, STL container, added validation, normalization or noexcept.
// Current SDK CRT policy does not reproduce native encoded-handler selection,
// Watson/debugger/FH3/private-frame/fault identity. Native binary ABI, direct
// differential, application adoption and game behavior remain unproved.
// Correct native STL_TreeIter_Inc/STL_IterWrap library names are preserved.

} // namespace bsp
