#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene integer set decrement requires MSVC Win32.
#endif

namespace bsp {

// Complete 004679F0..00467A78 [137B]. Native ECX=actual iterator8, plain
// RET or two tail JMPs to BF6713; no semantic result, incoming EDX unused.
// Iterator DWORDs are owner+0 and live node+4. Nodes have links0/4/8 and nil11h.
// A zero owner invokes the concrete returning current-CRT provider, then the
// CURRENT node is read. End publishes its right link before validating it.
// Left descent publishes its rightmost node. Ascent reloads iterator.node,
// publishes each parent, then reloads that parent's parent. Tail validation
// checks CURRENT iterator.node; failure reports and returns without publishing
// the captured candidate. No nil19h record-map routine is invoked.
void __fastcall decrement_native_scene_integer_set_iterator_004679f0(
    void* actual_iterator8);

// Complete 12-byte wrappers, native ECX iterator/plain RET; each decrements
// actual backing and returns that same original backing address in EAX.
void* __fastcall decrement_native_scene_integer_set_iterator_00468210(
    void* actual_iterator8);
void* __fastcall decrement_native_scene_integer_set_iterator_004688f0(
    void* actual_iterator8);

// Ordinary Source retains raw volatile loads/stores and returning-handler
// effects. Valid backing for every taken access is required. No snapshot,
// semantic owner, callback, STL replacement, extra validation or noexcept.
// Current CRT policy does not reproduce encoded native handler selection,
// Watson/debugger delivery, native private-frame aliases, mangling, fault/SEH
// identity or binary ABI. No direct native differential or game proof claimed.
// Established native STL_TreeIter_Dec/STL_IterWrap library names are retained.

} // namespace bsp
