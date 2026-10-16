#pragma once

#include "bsp/native_input_int_string_tree_erase.hpp"

namespace bsp {
// Whole006A1B70:200definedB/76starts in two ranges. The outside NOP at
// 006A1BEF remains unpromoted. Native ECX tree, EDX unused, five stacked
// words(output,first.owner/node,last.owner/node), EAXoutput, RET14h. Source
// adds actual borrowed RawCtx after both real8B by-value iterators: RET18h.
// Preserve returning-diagnostic capture order, whole subtree/reset schedule,
// and the complete partial path. The caller advances its actual private FIRST
// argument, then the accepted eraser advances its separate OLD by-value copy.
// No generic range/Kind/layout/context/callback/model bridge or prefix shortcut.
//
// Raw public loads/stores remain volatile. Getter/diagnostic/child exceptions
// propagate with prior iterator/tree/pool progress; no local EH/rollback/guard.
// Existing current SDK/CRT/CurrentCpp/FH3/SEH/RTTI/fault/private-frame and heap
// limits remain. Physical Source ABI differs from Native; no game proof.
NativeInputIntStringTreeIterator* __fastcall
erase_native_input_int_string_tree_range_006a1b70(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputIntStringTreeIterator* actual_output,
    NativeInputIntStringTreeIterator first,
    NativeInputIntStringTreeIterator last,
    NativeStringRawPoolContext& strings);
} // namespace bsp
