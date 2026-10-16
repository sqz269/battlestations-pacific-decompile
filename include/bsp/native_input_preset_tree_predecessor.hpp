#pragma once

#include "bsp/native_input_preset_tree_erase.hpp"

#include <cstdint>

namespace bsp {

// Whole0069AEC0..0069AF48:137B/48starts, including two7B alignment LEAs.
// Established analysis identity: STL_TreeIter_Dec_0069aec0; Source name is
// descriptive, not a recovered symbol. Native ECX points to the real8B
// owner0/node4 iterator, EDX is unconsumed, no stacked inputs, plain RET.
// EAX is incidental and differs by path; this Source API is semantic void.
// Reuse the published carrier; links0/4/8 and literal nil29 are raw storage.
void __fastcall decrement_native_input_preset_tree_iterator_0069aec0(
    NativeInputPresetTreeIterator* actual_iterator, std::uint32_t unused_edx);

// Owner0 is a nullness diagnostic only. The current SDK diagnostic may
// return, throw or terminate. After its initial return, reload CURRENTnode.
// End.right is captured and published before testing that captured node.
// Climbing publishes parent before reading its parent and reloads CURRENT
// iterator.node for each left comparison and the final nil test. Both Native
// tail diagnostics return without a subsequent store if the handler returns.
// Raw reached storage must be valid and walks finite; no endpoint repair.
// Only node4 is written. No payload/heap/pool/RawCtx/locale/callback/guard/EH.
// Volatile Win32 public loads/stores preserve their Source order. The current
// SDK policy, semantic void, canonical unused EDX, compiler frames/registers/
// flags, Original handler/FH3/SEH/fault/ABI/runtime/game identity are qualified.
} // namespace bsp
