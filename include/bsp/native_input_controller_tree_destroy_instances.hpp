#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual controller tree destruction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Whole 006A6C20..006A6C53, 006A6E70..006A6EA3 and 006A7040..006A7073:
// 52B/23 instructions each. Keep STL_Tree_Destroy identities; descriptive
// Source names are hypotheses. Native ECX=actual12B tree, EDX unconsumed,
// no stacked inputs/plainRET, EAX DWORD0. Added borrowed RawCtx gives these
// Source fastcall interfaces one stack word/RET4, not native ABI identity.
//
// Capture head(+4), then head.left(+0), unconditionally. Actual whole6A6A20
// receives the same tree owners, captured endpoints and private ignored8B
// output, using its existing real NativeInputControllerTreeIterator carrier.
// Only after child success reload CURRENT head for currentCRT free, clear
// tree+4 BEFORE tree+8, return0. No direct opaque0 access; raw aliases can
// overlap it. No guard, key cleanup, retry, rollback, local EH or new context.
std::uint32_t __fastcall destroy_native_input_controller_tree_006a6c20(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);
std::uint32_t __fastcall destroy_native_input_controller_tree_006a6e70(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);
std::uint32_t __fastcall destroy_native_input_controller_tree_006a7040(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);

// Reached actual storage/currentCRT/pool ownership must be valid. Actual
// child failures retain child progress before this entry's free/stores.
// Child unusedEDX is canonical0; original register residue is unmodelled.
// Native zeros EAX BEFORE public stores; compiler Source may do so AFTER,
// retaining semantic return0 without register/flag timing equivalence.
// Source private output/arguments, currentCpp/CRT, original heap/FH3/RTTI/
// SEH/fault/cleanup failure/binaryABI/game behavior remain qualified.
// Current free is noexcept; no noexcept adapter surrounds child failures.
// No EBP reader, owner/lifetime binding, global, adapter/model/callback or
// promotion of6AA460 is introduced. Prior Source/fixtures remain separate.
} // namespace bsp
