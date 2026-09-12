#include "bsp/native_renderer_gamma.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <exception>

namespace bsp {
namespace {
// B219D3..B21B00 math/device path. The 628h reserved/saved frame keeps every
// native scratch/ramp offset. EBP is a new stable binding register; the added
// pointer argument references the original live requested slot. No float
// value is copied by a C++ call or an SSE arithmetic expression.
__declspec(naked) void __fastcall generate_and_set_gamma(
    void*, const NativeRendererGammaContext&, const volatile float*) {
    __asm {
        sub esp, 620h
        push ebp
        push edi
        mov ebp, edx
        mov edi, ecx
        fld dword ptr [edi + 196ch]
        mov eax, dword ptr [esp + 62ch]
        fld dword ptr [eax]
        fld st(0)
        fxch st(2)
        fucomip st(0), st(2)
        fstp st(1)
        lahf
        test ah, 44h
        jnp equal_request
        mov eax, dword ptr [ebp + 8]
        fadd qword ptr [eax]
        mov eax, dword ptr [esp + 62ch]
        movss xmm0, dword ptr [eax]
        movss dword ptr [edi + 196ch], xmm0
        mov eax, dword ptr [ebp + 0ch]
        fdiv qword ptr [eax]
        fstp dword ptr [esp + 8]
        fld dword ptr [esp + 8]
        fldz
        fcomip st(0), st(1)
        fstp st(0)
        jbe upper_clamp
        xorps xmm0, xmm0
    store_normalized:
        movss dword ptr [esp + 8], xmm0
    normalized:
        fld dword ptr [esp + 8]
        push esi
        fadd st(0), st(0)
        xor esi, esi
        fld1
        mov dword ptr [esp + 0ch], esi
        fdivrp st(1), st(0)
        fstp dword ptr [esp + 1ch]
    next_entry:
        fild dword ptr [esp + 0ch]
        mov eax, dword ptr [ebp + 10h]
        fdiv qword ptr [eax]
        fstp dword ptr [esp + 0ch]
        fld dword ptr [esp + 0ch]
        fld dword ptr [esp + 1ch]
        push dword ptr [ebp + 4]
        call dispatch_native_crt_pow_00bfeb10
        add esp, 4
        fstp dword ptr [esp + 0ch]
        fld dword ptr [esp + 0ch]
        add esi, 1
        mov eax, dword ptr [ebp + 14h]
        fmul qword ptr [eax]
        fnstcw word ptr [esp + 0ch]
        movzx eax, word ptr [esp + 0ch]
        or eax, 0c00h
        cmp esi, 100h
        mov dword ptr [esp + 10h], eax
        fldcw word ptr [esp + 10h]
        fistp dword ptr [esp + 10h]
        mov ax, word ptr [esp + 10h]
        mov word ptr [esp + esi*2 + 41eh], ax
        mov word ptr [esp + esi*2 + 21eh], ax
        fldcw word ptr [esp + 0ch]
        mov word ptr [esp + esi*2 + 1eh], ax
        mov dword ptr [esp + 0ch], esi
        jl next_entry
        cmp byte ptr [edi + 1b54h], 0
        mov edi, dword ptr [edi + 1a10h]
        mov ecx, dword ptr [edi]
        mov eax, dword ptr [ecx + 54h]
        pop esi
        lea edx, [esp + 1ch]
        push edx
        jz uncalibrated
        push 1
        push 0
        push edi
        call eax
        jmp done
    upper_clamp:
        movss xmm0, dword ptr [esp + 8]
        mov eax, dword ptr [ebp + 18h]
        movss xmm1, dword ptr [eax]
        comiss xmm0, xmm1
        jbe store_normalized
        movss dword ptr [esp + 8], xmm1
        jmp normalized
    uncalibrated:
        push 0
        push 0
        push edi
        call eax
        jmp done
    equal_request:
        fstp st(0)
    done:
        pop edi
        pop ebp
        add esp, 620h
        ret 4
    }
}

int terminate_cpp_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) noexcept {
    __try {
        destroy_native_renderer_optional_guard_00b21110(guard, globals);
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct GuardCleanup {
    const NativeRendererOptionalGuardStorage& guard;
    NativeRendererSynchronizationGlobals& globals;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard, globals); }
};
} // namespace

void set_native_renderer_gamma_00b21960(void* actual_renderer,
    const volatile float& actual_requested_slot,
    const NativeRendererGammaContext& bindings) {
    NativeRendererOptionalGuardStorage guard;
    auto& globals = bindings.synchronization;
    const bool entry_enabled = globals.mode_00 != 0;
    if (entry_enabled) {
        guard.renderer_04 = actual_renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(actual_renderer, globals);
    }
    const bool enabled = *(static_cast<volatile unsigned char*>(actual_renderer) + 0x1b53) != 0;
    GuardCleanup cleanup{guard, globals}; // State0 arms after the enabled-byte read.
    if (enabled) generate_and_set_gamma(actual_renderer, bindings, &actual_requested_slot);
    const bool leave_enabled = globals.mode_00 != 0;
    cleanup.armed = false; // State-1 before normal leave, including disabled/equal paths.
    if (leave_enabled) {
        __assume(entry_enabled);
        std::uint32_t saved_word;
        const void* saved_renderer;
        const auto* record = &guard;
        __asm {
            mov eax, record
            mov ecx, dword ptr [eax]
            mov saved_word, ecx
            mov ecx, dword ptr [eax + 4]
            mov saved_renderer, ecx
        }
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, saved_word, globals);
    }
}
} // namespace bsp
