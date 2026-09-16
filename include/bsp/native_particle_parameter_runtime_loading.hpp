#pragma once

namespace bsp {
class NativeWeakHandlePool;

// Borrow the SAME actual F8D344 pool used by parameter returns and four native
// numeric pointer cells. No text, allocation callback, or legacy owner facade.
// The pointer members remain current: numeric kernels load through their
// original member addresses. The referenced pool owns its real native storage.
struct NativeParticleParameterRuntimeRawContext {
    NativeWeakHandlePool& parameter_pool_00f8d344;
    const volatile double* tangent_scale_00cf1450;
    const volatile float* default_slope_00d5dca0;
    const volatile double* integral_half_00d7a280;
    const volatile double* integral_quarter_00d7a348;
};

// AFBF60[583], ECX actual10h builder, RET/EAX actual0Ch pooled payload.
// Mutates Hermite key coefficients in place. Leaves payload multiplier+0 and
// physical slab index+C untouched; count/capacity are BYTEs at+8/+9. Uses the
// fixed source CRT for segment storage. Unknown kind abandons its newly
// allocated slot and returns null; failures after construction retain native
// partial effects. Caller retains the builder and owns the returned slot.
void* convert_native_particle_parameter_00afbf60(
    void* actual_builder, NativeParticleParameterRuntimeRawContext&);

// AFFCB0[106]/AFFD20[80]: original ECX parameter, stack time, RET4/ST0.
// EDX supplies the new borrowed context; it is not an original argument.
// Integer-only entry binding preserves caller-owned x87 stack values and adds
// no floating-point spill. Unchecked segment search follows the native body.
float __fastcall integrate_native_particle_parameter_hermite_00affcb0(
    const void*, const NativeParticleParameterRuntimeRawContext*, float);
float __fastcall integrate_native_particle_parameter_linear_00affd20(
    const void*, const NativeParticleParameterRuntimeRawContext*, float);
} // namespace bsp
