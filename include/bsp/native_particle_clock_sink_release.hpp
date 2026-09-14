#pragma once

namespace bsp {

using NativeParticleClockAtomic = long (__stdcall *)(volatile long*);

// 004DDB40: original ECX container is unused; actual sink stays in the
// original public stack slot and RET4. EDX adds the address of the current
// InterlockedDecrement IAT cell 00CE2220, not a captured target pointer.
// Decrement actual sink+4. Only on zero, reload its actual profile and call
// slot0 with sink ECX and no stack arguments (no deleting-destructor flags).
void __fastcall release_native_particle_clock_sink_004ddb40(
    void* unused_container,
    NativeParticleClockAtomic const volatile& actual_decrement_00ce2220,
    void* actual_sink);

// This raw source interface has an additional IAT-cell argument; it is not
// an original binary entry or a host C++ virtual-owner projection.

} // namespace bsp
