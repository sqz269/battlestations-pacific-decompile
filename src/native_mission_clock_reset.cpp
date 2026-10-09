#include "bsp/native_mission_clock_reset.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include <cstddef>

namespace bsp {
namespace {
// Integer-only address binding. This private layout is not a clock owner or
// an overlay of either the Native globals or the reference-bearing context.
struct ResetAddresses {
    volatile float* clock;
    volatile float* accumulated;
    volatile float* remainder;
    volatile std::uint32_t* step_count;
    volatile float* interpolation;
    const volatile double* step;
    const volatile std::uint32_t* conversion_mode;
};
static_assert(sizeof(void*) == 4 && sizeof(float) == 4 && sizeof(double) == 8);
static_assert(sizeof(std::uint32_t) == 4 && sizeof(ResetAddresses) == 28);
static_assert(offsetof(ResetAddresses, clock) == 0);
static_assert(offsetof(ResetAddresses, accumulated) == 4);
static_assert(offsetof(ResetAddresses, remainder) == 8);
static_assert(offsetof(ResetAddresses, step_count) == 12);
static_assert(offsetof(ResetAddresses, interpolation) == 16);
static_assert(offsetof(ResetAddresses, step) == 20);
static_assert(offsetof(ResetAddresses, conversion_mode) == 24);

// ECX = borrowed addresses; EDX unused; stack DWORD = original input bits.
// ESI survives both existing converter paths and is restored to our caller.
// Its save moves the unchanged argument slot from [ESP+4] to [ESP+8].
__declspec(naked) void __fastcall reset_kernel(
    const ResetAddresses*, void*, std::uint32_t) {
    __asm {
        push esi
        mov esi, ecx
        fld dword ptr [esp + 8]       // 00874640: first floating operation
        movss xmm0, dword ptr [esp + 8] // 00874644: unchanged input bits
        fld st(0)                    // 0087464A: T,T
        mov edx, dword ptr [esi]
        movss dword ptr [edx], xmm0  // 0087464C: clock
        mov edx, dword ptr [esi + 14h]
        fld qword ptr [edx]          // 00874654: S,T,T
        mov edx, dword ptr [esi + 4]
        movss dword ptr [edx], xmm0  // 0087465A: accumulated
        fdiv st(1), st(0)            // 00874662: S,Q,T
        fxch st(1)                   // 00874664: Q,S,T
        mov ecx, dword ptr [esi + 18h]
        call native_crt_truncate_st0_00bf7420 // 00874666: S,T remain live
        mov edx, dword ptr [esi + 0ch]
        mov dword ptr [edx], eax     // 0087466B: store low count word
        fild dword ptr [edx]         // 00874670: signed count,S,T
        fmul st(0), st(1)            // 00874676: count*S,S,T
        fsubp st(2), st(0)           // 00874678: S,T-count*S
        fxch st(1)                   // 0087467A: T-count*S,S
        mov edx, dword ptr [esi + 8]
        fstp dword ptr [edx]         // 0087467C: rounded remainder; S
        fsub dword ptr [edx]         // 00874682: reload that stored remainder
        mov edx, dword ptr [esi + 10h]
        fstp dword ptr [edx]         // 00874688: interpolation
        pop esi
        ret 4 // 0087468E: private kernel argument
    }
}
} // namespace

void reset_native_mission_clock_00874640(
    std::uint32_t input_float_bits,
    const NativeMissionClockResetContext& context) {
    const ResetAddresses addresses{
        &context.clock_00f876a4,
        &context.accumulated_00f876a8,
        &context.remainder_00f876ac,
        &context.step_count_00f876b0,
        &context.interpolation_00f876b4,
        &context.step_00d7a270,
        &context.conversion_mode_0109eea4,
    };
    reset_kernel(&addresses, nullptr, input_float_bits);
}
} // namespace bsp
