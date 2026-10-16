#pragma once

namespace bsp {

// Complete ordinary bodies at 0069D1F0 / 0069D240 / 0069D290 / 0069D320.
// Original: no consumed inputs, no stacked arguments, plain RET, EAX = block.
// Incoming ECX/EDX are unconsumed. These zero-input cdecl interfaces do not
// reproduce incidental registers/flags, private frames, or the original heap ABI.
// Existing STL library names remain authoritative; the C++ labels are hypotheses.
// Uses the accepted current-CRT singleton allocation service at the exact native
// byte size. Original CRT/new-handler/exception and raw-fault parity are unproved.
// Only link DWORDs 0/4/8 and the color/nil bytes are initialized. Caller-side
// self-links, nil=1, count/header publication, and payload initialization are absent.
void* __cdecl buy_native_input_device_tree_head_0069d1f0();
void* __cdecl buy_native_input_preset_tree_head_0069d240();
void* __cdecl buy_native_input_default_scalar_tree_head_0069d290();
void* __cdecl buy_native_input_controller_tree_head_0069d320();

} // namespace bsp
