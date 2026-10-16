#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual preset-tree destruction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Three whole52B/23-start entries;156B/69starts total. Native ECXactualtree,
// EDXunconsumed,no stacked arguments/plainRET,EAX0. Source adds the actual
// borrowed RawCtx on stack/RET4. Names describe the same consumed preset tree;
// correct STL identities and existing AV overloads remain separate.
//
// Capture initial head then head.left BEFORE actual whole6A1AA0 using the
// same tree as both owners, saved endpoints and a real private8B output.
// After success free CURRENTtree+4; clear+4 BEFORE+8; return0; opaque0 retained.
// No cached-head free, guard, key/pair cleanup, count shortcut or rollback.
// Range failure propagates before this entry's free/final clears.
std::uint32_t __fastcall destroy_native_input_preset_tree_006a2060(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);
std::uint32_t __fastcall destroy_native_input_preset_tree_006a24b0(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);
std::uint32_t __fastcall destroy_native_input_preset_tree_006a2b00(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);

// Requires valid reached owned storage and compatible currentCRT, with tree
// writable through post-free clears. Raw volatile DWORD accesses wrap32.
// Child EDX is canonically0; private8B/carrier/argument frames and recursive
// origins are new Source interfaces. Native EAX0 precedes zero stores; Source
// may compute0 afterward. Original heap/CRT/FH3/SEH/RTTI/fault/flags/frame ABI
// and game equivalence remain unproved. Readonly EBP-based caller actions are
// not Source helpers; no action, caller, member or owner is promoted here.
} // namespace bsp
