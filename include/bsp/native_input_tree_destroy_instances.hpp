#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual input tree destruction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Five whole 52-byte/23-instruction STL_Tree_Destroy instances. These names
// describe consumed contracts, not recovered C++ symbols. Native ECX is the
// actual 12B tree, incoming EDX unconsumed, no stacked inputs/plain RET, EAX=0.
// Preserve the established library identities and all earlier typed/AV APIs.
//
// Capture tree.head(+4) then head.left(+0) before actual full range erasure.
// The same tree supplies both iterator owners; private 8B output is ignored.
// After success reload CURRENT head and free it through the accepted current
// CRT service; clear head(+4) BEFORE count(+8), return DWORD zero. No direct
// opaque(+0) access; reached raw aliases are not an immutability promise.
// No guard, retry, rollback, local EH, count shortcut or extra payload cleanup.
//
// These two bind whole 0069FE70, retaining its physical owner/node word API.
// No pool/context parameter is introduced for unowned scalar payload.
std::uint32_t __fastcall destroy_native_input_default_scalar_tree_006a0d60(
    void* tree, std::uint32_t unused_edx);
std::uint32_t __fastcall destroy_native_input_default_scalar_tree_006a14e0(
    void* tree, std::uint32_t unused_edx);

// These three bind whole 006A1B70 with its actual trivial 8B iterator carrier.
// The borrowed actual pool context adds one Source stack word/RET4, unlike
// Native plain RET. It is forwarded unchanged; the range owns payload cleanup.
std::uint32_t __fastcall destroy_native_input_int_string_tree_006a21c0(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);
std::uint32_t __fastcall destroy_native_input_int_string_tree_006a24f0(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);
std::uint32_t __fastcall destroy_native_input_int_string_tree_006a2b70(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);

// Valid reached raw tree/head/node storage, unsigned32 address wrap, matching
// current CRT and actual borrowed pool ownership are required. Child failure
// propagates after child progress and before this entry's head free/clears.
// Child unused EDX is canonical zero, not native register residue. Source
// returns semantic zero; native EAX is zeroed before the public stores while
// compiler/private register scheduling may differ. Private output/argument
// frames and aliases, original register/flags/CRT/heap/FH3/RTTI/SEH/fault/
// cleanup-failure/binary ABI and game equivalence remain unproved. Current
// free is noexcept; no noexcept adapter is added around range exceptions.
// Existing default/int-string/controller/preset Source and fixtures remain
// separate; these bodies do not promote any containing owner or unwind action.
} // namespace bsp
