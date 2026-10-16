#pragma once

#include "bsp/native_input_preset_tree_erase.hpp"

namespace bsp {
// Whole006A1AA0:200definedB/76starts in two ranges; outside NOP006A1B1F
// is not owned. Native ECX tree, EDX unconsumed, five stacked DWORDs:
// output,FIRST.owner/node,LAST.owner/node. EAXoutput/RET14h. Source adds
// actual borrowed RawCtx after both real8B by-value iterators: RET18h.
// NativeInputPresetTreeIterator is the accepted trivial8B private carrier,
// not a node overlay or a recovered original C++ argument declaration.
//
// Preserve returning-diagnostic captures, CURRENT full-reset head reloads,
// and both full and partial paths. FIRST advances before erasing saved OLD;
// the actual eraser separately advances its own OLD copy. Reload FIRST.node
// before FIRST.owner; capture output/result words before owner/node stores.
// Direct actual preset subtree/successor/eraser only; no generic nil/Kind,
// container/provider/callback, guard, repair, catch or rollback is introduced.
//
// Valid reached tree/sentinel/iterator/output storage and actual RawCtx/current
// CRT ownership are required. Child/diagnostic exceptions retain prior input,
// topology and pool progress. Source private frames/argument aliases, dead EDX
// canonicalization, original heap/CRT/FH3/SEH/RTTI/fault/register/game identity
// remain unproved. Existing NativeKeyboardTreeIterator/AV overload is separate.
NativeInputPresetTreeIterator* __fastcall
erase_native_input_preset_tree_range_006a1aa0(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputPresetTreeIterator* actual_output,
    NativeInputPresetTreeIterator first,
    NativeInputPresetTreeIterator last,
    NativeStringRawPoolContext& strings);
} // namespace bsp
