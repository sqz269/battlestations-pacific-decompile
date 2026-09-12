#pragma once

#include <cstdint>

namespace bsp {
struct CameraAxesCrtException;
struct LegacyCrtMathRuntime;
struct NativeCrtPointerDecodeContext;

// One stable, canonical identity domain for the original immediate names.
// Bind actual original pointers, or a single explicitly mapped rebuilt domain
// shared by EVERY admitted callback/consumer that can compare name pointers.
// Each pointer denotes the indicated exact NUL-terminated original bytes.
// No per-function string copies and no mutable __umatherr table substitution.
struct NativeCrtLibmNameIdentities {
    const char* const tan_00d571a0;
    const char* const sin_00d571b4;
    const char* const pow_00d571c4;
    const char* const modf_00d571c8;
    const char* const log_00d571d0;
    const char* const log10_00d571d4;
    const char* const floor_00d571f4;
    const char* const exp_00d571fc;
    const char* const cos_00d57204;
    const char* const ceil_00d57210;
    const char* const atan_00d57218;
    const char* const asin_00d57228;
    const char* const acos_00d57230;
    const char* const exp10_00d6a968;
};

struct NativeCrtLibmErrorSupportContext {
    const volatile std::uint32_t& callback_present_0109e1b8;
    const volatile std::uint32_t& encoded_callback_0109ed80;
    const NativeCrtPointerDecodeContext& decoder;
    const LegacyCrtMathRuntime& owning_crt;
    const NativeCrtLibmNameIdentities& names;
};

using NativeCrtLibmMatherr = int (__cdecl*)(CameraAxesCrtException*);

// Complete _matherr C28545[3]: XOR EAX,EAX; RET. Reads no argument storage.
int __cdecl native_crt_default_matherr_00c28545(CameraAxesCrtException*);

// Complete ___libm_error_support C0F0E4[634], qualified naked Win32 ABI.
// Four original cdecl argument words retain their entry ESP+4,+8,+C,+10 slots;
// the added stable context is entry ESP+14. Plain RET; caller cleans 20 bytes.
// No semantic integer/FP return. Do not use an adapter that copies the selector
// before decoding: the current fourth argument slot is read AFTER decoding.
void __cdecl native_crt_libm_error_support_00c0f0e4(
    const double* first, const double* second, double* output,
    std::int32_t selector, const NativeCrtLibmErrorSupportContext& context);

// Context/member identities, canonical mapping and added argument remain valid
// and stable, separate from mutable native state. Actual referenced flag and
// encoded words stay mutable; selection precedes even unknown-selector/26
// dispatch. Nonzero flag uses the full decoder and its actual OS/TLS/PTD domain;
// decoded callback is invoked without a null guard or replacement. It has the
// true cdecl mutable32-byte record ABI, valid code/data lifetime, preserved
// nonvolatiles and required net x87 depth. Clearing the flag cannot retire an
// already captured callback. Existing actual owning-CRT errno accessor applies.
//
// First/second/output may alias each other and valid mutable native state.
// Read/write access and x87 exception preconditions follow each reached route.
// Callbacks may mutate the exact record, including result; errno policy is
// selected by the route, not reselected from the changed type. Final result
// reload follows callback and any errno accessor/store. Private source-frame,
// added context/mapping aliases and inspecting caller machine code are outside
// the domain. No FP-control/status reset, EH guard, validation or rollback.
//
// New code addresses and the added context/name loads
// do not reproduce native fault-site/SEH continuation, private-frame identity,
// or incidental volatile-register/EFLAGS values. This source is not a drop-in
// binary replacement, CRT-startup closure, or evidence of a current callback.
} // namespace bsp
