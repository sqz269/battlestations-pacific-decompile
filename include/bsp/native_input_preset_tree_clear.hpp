#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual preset-tree clear requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Whole006A1230..006A125A,43B/16starts. Native ECXtree/unusedEDX, plainRET;
// Source adds borrowed RawCtx/RET4. Capture current head then root, invoke
// actual preset-key subtree. After success reload head for root reset, reload
// head AGAIN before count clear/left reset, then reload head for right reset.
// Preserve these distinct captures and ordered stores under native aliases.
void __fastcall clear_native_input_preset_tree_006a1230(
    void* tree, std::uint32_t unused_edx, NativeStringRawPoolContext& strings);

// Actual reached tree/head/node storage and matching rawpool/currentCRT
// ownership are required. Subtree failure propagates before reset, retaining
// previous effects. No count/head cache, guard, rollback, EH scope, key/mapped
// destructor or substitute container. New Source ABI/private frames, unused
// EDX0, original CRT/FH3/SEH/RTTI/fault/register and game identity unproved.
// Existing typed/AV projections are retained separately.
} // namespace bsp
