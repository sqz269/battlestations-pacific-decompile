#pragma once

namespace bsp {

// Whole native 00C1C60B..00C1C613: nine bytes, four instructions, zero
// incoming arguments, plain RET. Provisional behavior name; Original live
// identity LIBCRT_unmatched_00c1c60b is retained in the evidence preimage.
// Calls the published actual canonical C11B31 unlock with lock index 4.
// Caller owns readable descriptor 00E16498 and its valid, initialized Win32
// critical section, acquired by the current thread, through the call. Page
// admission alone does not initialize or acquire a lock. No added owner,
// context, storage, provider, guard, initialization or acquisition is supplied.
// Separate PUSH4/CALL/POP ECX/RET retains ECX=4 and the child's normal-return
// EAX/EDX/arithmetic flags; nonvolatiles inherit the actual child contract.
// No meaningful return value or private parent-EBP access. The actual child
// relocation/new placement qualifies fault PCs and private return frames.
// This ordinary helper does not cover the preceding C1C606 private-frame
// fragment, its complete 14B action, calloc C1C50F, FH3/SEH4 or game behavior.
void __cdecl unlock_native_crt_calloc_lock4_00c1c60b();

} // namespace bsp
