#pragma once

namespace bsp {

// B7D160, complete 191-byte leaf. ECX = output six floats, EDX = input
// {center.x, center.y, center.z, radius}; EAX = original output; plain RET.
// Preserve the original x87/SSE schedule, including every intermediate float
// store/reload. Inputs are raw valid storage; output may overlap the input.
float* __fastcall project_native_sphere_bounds_00b7d160(
    float* destination, const float* sphere) noexcept;

// B7D220, complete 62-byte wrapper. Native ECX = output, [ESP+4] = sphere,
// EAX = output, RET4. The ignored EDX parameter lets a C++ __fastcall caller
// supply that original register/stack arrangement. It is not a native argument.
// The projection uses a private six-float temporary, then six ordered x87
// load/store copies. No null/radius/FP-mode check is added.
float* __fastcall set_native_bounds_from_sphere_00b7d220(
    float* destination, void* ignored_edx, const float* sphere) noexcept;

} // namespace bsp
