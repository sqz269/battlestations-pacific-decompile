#pragma once
#include "bsp/system_camera_axes.hpp"

namespace bsp {
// Bind the owning runtime's actual global and per-thread errno accessor. In a
// rebuilt process the latter is the host CRT's _errno; inside the original game
// it is 00BFFB8B. The runtime and global must outlive every adapter call.
struct LegacyCrtMathRuntime {
    const volatile std::uint32_t* matherr_bypass_00e16bd0;
    int* (__cdecl* errno_location_00bffb8b)();
};
void bind_legacy_crt_math_runtime(const LegacyCrtMathRuntime&);

// Recovered VS2005 __87except, including masked x87 status generation, Win32
// RaiseException and continuation record mutation, default _matherr, and errno.
// Exact cdecl argument layout fits CameraAxes87Except. Requires the binding
// above and the same x87 entry environment as the original CRT sqrt handler.
// saved_control_word is read only: continuation changes the local word loaded
// into x87, never the caller's saved word. MXCSR is not read or modified.
void __cdecl legacy_crt_87except_00c27489(std::int32_t operation,
    CameraAxesCrtException*, std::uint16_t* saved_control_word);
}
