#pragma once
#include "bsp/random_threads.hpp"

namespace bsp {
// Borrow the application's existing thread/stream domain and CURRENT globals.
// No default seed, cached constants or replacement random owner is supplied.
struct NativeParticleUnitRandomAccess {
    RandomThreads* random;
    const volatile float* unsigned_correction_00ce3978;
    const volatile double* unit_scale_00d63b80;
};
// Complete BD2F40..BD2F84. Native ECX stream, ST0 float; EDX adds access.
// Keeps the native pre-draw guard, signed FILD/unsigned correction and final
// float32 spill, then returns the same canonical RandomState draw.
float __fastcall native_particle_unit_random_00bd2f40(
    RandomStream, const NativeParticleUnitRandomAccess*);
} // namespace bsp
