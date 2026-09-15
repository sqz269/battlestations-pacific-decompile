#pragma once

#include <cstdint>

namespace bsp {

// Complete native 00C11B31..00C11B45, one raw 32-bit cdecl argument, plain RET.
// Uses the actual fixed DWORD at 00E16478 + (index * 8), with x86 wrapping.
// The caller owns the readable canonical descriptor and its valid, initialized,
// currently owned Win32 critical section for the duration of LeaveCriticalSection.
// Canonical page admission alone supplies no initialized lock. No range/null
// check, lazy initialization, replacement lock, return policy or fault conversion.
// Original instruction/frame/flag schedule is retained except the genuine import
// operand relocation. New code placement does not reproduce original fault PCs
// or establish PTD, initialization, acquisition, SEH or gameplay closure.
void __cdecl unlock_native_crt_canonical_00c11b31(std::int32_t actual_lock_index);

} // namespace bsp
