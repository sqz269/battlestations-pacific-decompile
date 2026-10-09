#pragma once

namespace bsp {

// Complete native 00C11C18..00C11C20: no arguments, plain RET, nine bytes.
// Calls the actual canonical C11B31 unlock provider with lock index 10.
// The caller owns the readable fixed descriptor at 00E164C8 and its valid,
// initialized Win32 critical section, acquired by the current thread, through
// the call. This helper supplies no storage, initialization or acquisition.
// No own frame or implicit parent-EBP access. POP ECX establishes ECX=10;
// EAX/EDX and arithmetic flags retain the unlock child's normal-return state.
// No meaningful return value. The genuine child relocation changes placement,
// not this instruction schedule. Ordinary-call Source does not admit the full
// initializer, its SEH table/unwind behavior, original fault PCs or gameplay.
void __cdecl unlock_native_crt_initializer_lock10_00c11c18();

} // namespace bsp
