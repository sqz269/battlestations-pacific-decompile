#pragma once

#include <cstdint>

namespace bsp::cc16_dyn_ambient_math {

// Ambient-x87 variants of the already reconstructed native bodies. MSVC Win32
// only. Arguments cross the wrapper as raw 32-bit words, without FP conversions.
// No x87 control/status or MXCSR reset is performed. See the packet document for
// native ABI, store epochs, exception behavior and the relocated-code boundary.
// Normal return needs four free x87 entries for CCW and seven for barycentric.

// Native 00C377F0..00C37857 returns full EAX = 0 or 1, RET 18h. This public
// function uses the ordinary C++ cdecl ABI; it is not a binary entrypoint.
std::uint32_t __cdecl ccw_2d_ambient_00c377f0(
    float x0, float y0, float x1, float y1, float x2, float y2) noexcept;

// Native 00C37860..00C37984 receives u/v/w pointers in EAX/ECX/EDX, then
// eight stack floats in this order, RET 20h. Outputs must be writable. They may
// alias each other: u, then v, then w are stored; retained x87 values determine w.
// The wrapper makes private native argument/scratch slots. Pointers into those
// private slots are not representable through this public C++ interface.
void __cdecl barycentric_2d_ambient_00c37860(
    float* u_out, float* v_out, float* w_out,
    float x0, float y0, float x1, float y1,
    float x2, float y2, float px, float py) noexcept;

}  // namespace bsp::cc16_dyn_ambient_math
