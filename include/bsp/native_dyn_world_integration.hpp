#pragma once
#include "bsp/system_camera_axes.hpp"
namespace bsp {
// Complete raw world-list kernels. Original C41550: ESI world/stack dt/RET4;
// C5B1B0: EBX world/stack dt/RET4. New explicit C++ interfaces, MSVC Win32 only.
// Preserve the native x87/SSE instruction schedule, double spills, signed sleep
// countdown and zero-products in the constrained-inertia path. Valid original
// world/body/motion records and exclusive mutation are required.
void native_dyn_integrate_velocities_00c41550(void* world,float dt);
// Borrows the actual CRT dispatch word/exception handler for the existing float
// sqrt boundary. No replacement math mode, allocator or global physics owner.
void native_dyn_integrate_positions_00c5b1b0(void* world,float dt,const CameraAxesCrtAccess&);
} // namespace bsp
