#pragma once

namespace bsp {

// Complete native 00BF9E1E..00BF9E26: no arguments, plain RET, nine bytes.
// Calls the actual canonical C11B31 unlock provider with lock index 4. The
// caller owns the readable fixed descriptor at 00E16498 and its valid,
// initialized, already-acquired Win32 critical section through the call.
// No lock/storage initialization, acquisition, replacement or own EH frame.
// POP ECX establishes ECX=4 on normal return; EAX/EDX and arithmetic flags
// retain the unlock child's residual state. No meaningful return value.
// The direct child relocation changes code placement, not the instruction
// schedule. This ordinary-call leaf does not admit the containing free body,
// gunnery destructor, SEH table/unwind behavior, original fault PCs or gameplay.
void __cdecl unlock_native_crt_free_lock4_00bf9e1e();

} // namespace bsp
