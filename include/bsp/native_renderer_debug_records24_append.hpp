#pragma once
#include <cstdint>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer sphere record append requires MSVC Win32.
#endif
namespace bsp {
struct NativeRendererRecordGuardContext;

// Full B25750[90]. ECX actual12B header, EDX unconsumed, stack live24B input,
// RET4. Equality-only capacity growth, wrapped signed doubled capacity, genuine
// B229D0 reserve. Reload used then data; DWORD data+used*18. Computed null skips
// all input reads. Four ordered x87 FLD/FSTP pairs and two ordered DWORD copies
// preserve aliases and FP state. Finally increment CURRENT used after stores.
void __fastcall append_native_renderer_records24_00b25750(
    void* actual_header, std::uint32_t unused_edx, const void* live_record24);

// Full B29270[184]. Original ECX renderer, stack(sphere16,camera,selector), RET0C.
// Use SAME R53 guard publication/manager. Capture guard+4 and enter/increment;
// MOVSS-read sphere components and capture arguments before arming state0;
// append actual renderer+1D0C, then decrement/leave while state0 remains armed.
// Sphere is copied, camera is borrowed without retain, selector is a raw DWORD.
void append_native_renderer_debug_record24_00b29270(void* actual_renderer,
    NativeRendererRecordGuardContext&, const void* live_sphere16,
    void* actual_camera, std::uint32_t selector);

// New public source context interface. Original private-frame/FH3/SEH and
// hardware-fault compatibility are unproved; no renderer owner is constructed.
} // namespace bsp
