#pragma once
#include <cstdint>

namespace bsp {
class RandomThreads;

// Borrow the existing application random domain and CURRENT native globals.
// These pointers may alias definition/state storage; loads stay at native sites.
struct NativeParticleTypeStateAccess {
    RandomThreads* random;
    const volatile float* zero_00d7a218;
    const volatile float* one_00d7a24c;
    const volatile double* random_scale_00d5da30;
    const volatile double* base_00d7a210;
    const volatile double* angle_scale_00d5daf8;
    volatile std::uint32_t* floating_counter_00f8d384;
};

// Complete actual PARTICLE Axial/Floating virtual18 initializers. Original ABI:
// ECX definition, stack(actual6Ch state,actual108h record), RET8. The record is
// accepted but unread. EDX adds borrowed access; these are new C++ interfaces.
// Native uint32 draws undergo SIGNED FILD; exact x87/SSE order, spills, NaNs,
// unchecked frame DIV and aliased reloads remain. Incidental EAX is not repaired
// into a result. Dispatch only the actual captured target, never a shape profile.
void __fastcall initialize_native_particle_axial_state_00b059c0(void* actual_definition,
    const NativeParticleTypeStateAccess*, void* actual_state, const void* actual_record);
void __fastcall initialize_native_particle_floating_state_00b077d0(void* actual_definition,
    const NativeParticleTypeStateAccess*, void* actual_state, const void* actual_record);
} // namespace bsp
