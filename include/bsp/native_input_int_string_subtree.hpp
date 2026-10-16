#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual input int/string subtree destruction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Whole006A0AA0..006A0AF1[82B/34starts], STL _Tree::_Erase(_Nodeptr)
// instantiation hypothesis: actual1Ch node, int key+C, pooled mapped string
// length+10h/data+14h, nil+19h. Preserve the existing STL identity/provenance.
// Native ECX is opaque and only forwarded to recursive calls; EDX is unused;
// stacked node, RET4, no semantic result (EAX residue). This Source retains
// both physical register slots and adds borrowed RawCtx: RET8, new Source ABI.
// Recursive unused EDX is canonical0; Native ambient EDX/frame bits are not
// reproduced or consumed. RawCtx owns no storage and introduces no provider.
//
// Test current nil19; recurse CURRENT right8. After return capture CURRENT
// data14 BEFORE CURRENT left0; only nonnull data reads CURRENT length10+1
// with DWORD wrap. Call actual419CC0 then actualBD1510 for that captured block
// even when the return later frees a large block or skips a disabled small
// return. Free original node with current CRT, then continue at captured left.
// No 41DD20 delegation/reread, header clear, count reset, null/cycle guard,
// rollback, callback, local EH or noexcept promise. Getter failure propagates
// after completed right-subtree effects, before current-node free/left walk.
//
// Valid reached storage/sentinels and provider-compatible allocations are
// required. Current C++/CRT/Windows services and this added-context interface
// do not establish original register/private-frame/FH3/SEH/fault identity,
// binary replacement, application adoption or game validation.
void __fastcall destroy_native_input_int_string_subtree_006a0aa0(
    void* opaque_tree, std::uint32_t unused_edx, void* actual_node,
    NativeStringRawPoolContext& strings);

} // namespace bsp
