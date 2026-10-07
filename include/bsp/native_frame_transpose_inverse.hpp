#pragma once

namespace bsp {
// Complete0085DEA0..0085DF5B: original ECX destination, EDX source, RET.
// Sequential real004134F0 copy, mixed x87/SSE transpose, then genuine0042D0D0
// normalize=false kernel and three SSE actual-negative-zero subtractions.
// Borrow the actual00D7A208 cell (80000000); no constant/provider is created.
// Source adds that cell pointer as one stack argument and returns RET4.
// Raw views may alias or be unaligned; they must support every reached access.
// Keep ambient x87/MXCSR. Ordinary fixture scope masks exceptions and disables
// DAZ/FTZ. No semantic EAX return, general matrix inversion, normalized branch,
// native binary replacement, caller lifetime or game behavior is established.
void __fastcall transpose_inverse_native_frame_0085dea0(
    void* destination, const void* source,
    const volatile float* actual_negative_zero_00d7a208);
} // namespace bsp
