#pragma once

namespace bsp {

// Full C302A0[74], including zero and nonzero child-count branches. Original
// ECX source, one stacked float, RET4. Source adds EDX pointing to the actual
// readable8-byte CE47A0 double storage; it is dereferenced ONLY after a nonzero
// captured source+14 count, at the original FDIVR point. No literal fallback.
// Preserve x87 operation/CW/FISTPqword/lowDWORD/unsignedDIV order. No retain,
// release, child access or validation; source+08 is the selected remainder.
// A usable source+0C float is required only on the nonzero branch. It is not
// initialized by C30470. Nonzero child/rate production is C30570, separate.
// New explicit Win32 source ABI; native register/stack/FH3/hardware-fault
// equivalence is not claimed. Normal CW restoration is not unwind cleanup.
void __fastcall set_native_texture_source_time_00c302a0(void* actual_source,
    const void* actual_milliseconds_scale_00ce47a0, float milliseconds) noexcept;

} // namespace bsp
