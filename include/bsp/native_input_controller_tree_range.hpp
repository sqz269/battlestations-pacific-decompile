#pragma once

#include "bsp/native_input_controller_tree_erase.hpp"

namespace bsp {
// Whole006A6A20:200definedB/76starts in two ranges; outside NOP006A6A9F
// is not Source. Native ECX tree/unused EDX, five stacked output/first/last
// words, EAX output/RET14h. Source reuses the published real8B trivial carrier
// and adds borrowed actual RawCtx after both by-value iterators: RET18h.
//
// Preserve capture order across returning owner diagnostics. Full range calls
// the actual controller subtree, then resets current head/root/count/extrema
// in Native order. Partial range saves OLD before actual FIRST advance; the
// actual eraser advances a separate OLD by-value copy. Its private output is
// ignored; recapture CURRENT first.node BEFORE first.owner on successful return.
// No prefix shortcut, generic range, layout/context/callback/container bridge,
// repaired ownership/endpoints, extra guard, local EH or rollback is introduced.
// Existing NativeKeyboardTreeIterator AV/typed overload remains unchanged.
//
// Raw public reads/writes are volatile. Exceptions retain prior iterator/tree/
// child/pool progress. Source private arguments/frames, borrowed RawCtx, current
// SDK/CRT/Cpp/FH3/SEH/RTTI/fault/heap/register/flags and binary ABI/game identity
// remain qualified; this interface is not a drop-in Native replacement.
NativeInputControllerTreeIterator* __fastcall
erase_native_input_controller_tree_range_006a6a20(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputControllerTreeIterator* actual_output,
    NativeInputControllerTreeIterator first,
    NativeInputControllerTreeIterator last,
    NativeStringRawPoolContext& strings);
} // namespace bsp
