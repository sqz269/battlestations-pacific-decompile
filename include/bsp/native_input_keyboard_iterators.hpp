#pragma once
#include "bsp/native_input_keyboard_apply.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
// Actual checked integer-set header, node14h/nil11h. Original ECX tree;
// stack output/key; EAX output, RET8. Owner is published before node.
NativeKeyboardTreeIterator* find_native_input_hack_00546840(void* tree,
    NativeKeyboardTreeIterator* output, const std::int32_t* key);

// Original ECX iterator, no stack arguments, RET. The owner is retained;
// ascent publishes each intermediate node exactly as the native iterator does.
void advance_native_input_sensitivity_00552770(NativeKeyboardTreeIterator*);
void advance_native_input_codes_00552d40(NativeKeyboardTreeIterator*);
void advance_native_input_device_005540c0(NativeKeyboardTreeIterator*);

// Shared checked-tree successor mechanism, also used by the class frame tree
// (008772B0, nil byte +55h). The iterator keeps the same two-pointer shape.
void advance_native_checked_tree_iterator(NativeKeyboardTreeIterator*,
    std::uint32_t nil_offset, const SingletonLifetimeCallbacks&);

// Original ECX iterator, stack signed distance, EAX iterator, RET4.
// Zero distance returns without validation. Nonzero traversal uses the actual
// bit-vector count/begin/end, including signed backward DWORD-boundary movement.
NativeKeyboardBitIterator* advance_native_input_bit_0048d3b0(
    NativeKeyboardBitIterator*, std::int32_t distance);

// New source ABIs. Valid consistent owned trees/bit ranges and live aligned
// storage are required. Original CRT invalid-parameter handler identity, FH3,
// malformed storage, private-stack aliases and hardware faults are unvalidated.
} // namespace bsp
