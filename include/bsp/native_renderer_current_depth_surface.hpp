#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer current depth surface requires MSVC Win32.
#endif

namespace bsp {

// Complete B20090..B20096, seven bytes: MOV EAX,[ECX+198C]; RET.
// Borrow the actual renderer; return its CURRENT raw surface-owner pointer.
// B238D0 publishes the wrapper of device GetDepthStencilSurface at this field;
// D5F0A8+12C selects this getter and B14A10 passes its result to B1FB00.
// No retain/release, COM acquisition, validation or owner lifetime is added.
// Original thiscall has no stack arguments, ECX renderer and EAX pointer.
// This fastcall leaf uses the same physical inputs/return; EDX is unused.
// Descriptive name is a hypothesis. Full renderer ABI/gameplay are unproved.
void* __fastcall get_native_renderer_current_depth_surface_00b20090(
    const void* actual_renderer) noexcept;

} // namespace bsp
