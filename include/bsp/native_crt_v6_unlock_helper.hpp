#pragma once

namespace bsp {

// Whole ordinary 00BF9E9C..00BF9EA4: nine bytes/four instructions, zero
// incoming arguments, plain RET. Provisional behavior name; preserve Original
// LIBCRT_unmatched_00bf9e9c name/comment/prototype preimages in the evidence.
// Directly calls actual canonical C11B31 unlock with lock index 4; this is
// not an alias through the distinct C1C60B calloc unlock helper.
// Caller owns readable DWORD descriptor 00E16498 and its valid, initialized
// Win32 critical section, acquired by the current thread, through the call.
// Page admission alone supplies no initialization or acquisition. This helper
// adds no context, provider, global, default, owner, guard or EBP reader.
// PUSH4/CALL/POP ECX/RET preserves ECX=4 and the actual child's normal-return
// EAX/EDX/arithmetic flags; nonvolatiles inherit the actual child contract.
// No meaningful return value. Genuine child relocation/new code placement
// qualifies fault PCs and private return frames. This does not admit whole
// V6 heap, SEH4/compiler frames, SBH, bootstrap, runtime or game equivalence.
void __cdecl unlock_native_crt_v6_heap_00bf9e9c();

} // namespace bsp
