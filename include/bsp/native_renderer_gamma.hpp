#pragma once

#include "bsp/native_crt_pow_dispatch.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {
// Stable borrowed references to actual state and relocated native literals.
// Literal values are the original double10/20/255/65535 and float1; their
// separate native access order is retained. Pow uses the actual current
// dispatch cell and complete selected provider, with its owning CRT domain.
struct NativeRendererGammaContext {
    NativeRendererSynchronizationGlobals& synchronization;
    const NativeCrtPowDispatchContext& power;
    const volatile double& ten_00ce3dc0;
    const volatile double& twenty_00ce3d88;
    const volatile double& levels_00ce4b48;
    const volatile double& maximum_00ce5f90;
    const volatile float& one_00d7a24c;
};
static_assert(sizeof(NativeRendererGammaContext) == 28);

// Complete B21960[474] behavior with a new source interface. Native ECX
// renderer, actual float callee slot at ESP+4, RET4. Source borrows that live
// four-byte slot by reference instead of copying its value: the first x87
// input load and later MOVSS cache-publication load remain distinct reads.
// Native raw owner/device/COM ABI and optional synchronization contracts
// apply. Inputs on the ramp-generation path use the complete power schedule;
// no host pow, clamp, rounding normalization or HRESULT check is substituted.
// This operation invokes the supplied actual device's SetGammaRamp method.
// Caller owns and keeps every referenced state/provider alive and stable.
void set_native_renderer_gamma_00b21960(void* actual_renderer,
    const volatile float& actual_requested_slot,
    const NativeRendererGammaContext&);

// Source C++ EH preserves the guard's armed/disarmed lifetime; no cache or
// x87-control rollback is added. Original FH3/private stack-slot aliases,
// hardware-fault/SEH identity, provider nonlocal-exit registers, original
// caller/drop-in ABI and game/monitor validation are not established. As in
// the existing synchronization domain, cleanup must not become enabled
// after entry was skipped with an uninitialized guard record.
} // namespace bsp
