#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shadow-entry iteration requires MSVC Win32.
#endif

namespace bsp {

// Complete AD7440[188]. Native ECX actual owner, no public stack arguments,
// RET; source EDX is unused. Traverse owner+64h then owner+54h descriptors,
// reading begin/end at descriptor+4/+8 and actual four-byte entry slots.
// Keep captured cursor/end values and every current reload/returning check.
// Fixed AE2CA0/AE0750 callees are separately emitted exact native RET bodies.
// This does not establish a complete owner layout or construct an owner.
void __fastcall dispatch_native_entry_pass_00ad7440(
    void* actual_owner, void* unused_edx);

// Validation calls use the real SDK _invalid_parameter_noinfo service through
// a fixed cdecl wrapper. Its current source-CRT handler may return; traversal
// then follows the original continuation. No arbitrary callback, throw/fatal
// replacement, range snapshot, allocation, retain or synchronization is added.
// Original BF6713/BF66EF encoded-handler globals, Watson and service exception/
// register identities remain unproved. The caller supplies valid actual32-bit
// storage/lifetimes; these checks are not a new memory-safety guarantee.

} // namespace bsp
