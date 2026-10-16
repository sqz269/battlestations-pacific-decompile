#pragma once

#include "bsp/native_input_preset_tree_erase.hpp"

namespace bsp {

// Whole00699EF0..00699F18:41B/18 instructions. Retain STL_inst_00699ef0;
// the descriptive Source name is a hypothesis. Native ECX is the actual left
// 8B iterator, EDX is unused, and one stacked right iterator gives RET4.
// AL is equality; Native upper EAX retains left.node bits and is incidental.
// The actual producer at006A175E tests AL only. Source returns bool, not fullEAX.
//
// Capture left.owner; its zero case skips right.owner, then calls the accepted
// actual invalid-parameter entry. The handler may return or throw. Only after
// return read CURRENT left.node before CURRENT right.node; no revalidation.
// Both actual carrier addresses and every reached DWORD must remain valid.
// Handler mutations of node words are observed; no new owner/node guard,
// snapshot, context, pool, allocation, repair, catch or rollback is supplied.
bool __fastcall equal_native_input_preset_tree_iterators_00699ef0(
    const NativeInputPresetTreeIterator* actual_left,
    std::uint32_t unused_edx,
    const NativeInputPresetTreeIterator* actual_right);

// New C++ private argument/frame timing is qualified even though register/stack
// input shape and RET4 agree. The published diagnostic uses current SDK policy.
// Original CRT/handler/private-stack aliases, registers/flags, FH3/SEH/RTTI,
// faults, heap identity, drop-in ABI and game behavior remain unproved. Existing
// AV/STL helpers, the preset predecessor and the whole map owner are unchanged.
} // namespace bsp
