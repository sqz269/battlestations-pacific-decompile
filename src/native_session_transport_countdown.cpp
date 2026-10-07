#include "bsp/native_session_transport_countdown.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Session transport countdown reconstruction requires MSVC Win32.
#endif

namespace bsp {
void tick_native_transport_countdown_00782840(void* transport,
    std::uint32_t step_word,
    const NativeSessionTransportCountdownBindings& bindings) noexcept {
    const volatile float* const threshold = &bindings.minimum_countdown_d7a260;
    std::uint32_t captured_word;
    __asm {
        mov ecx, transport
        mov edx, threshold
        movss xmm0, dword ptr [ecx + 48h]
        comiss xmm0, dword ptr [edx]
        movss dword ptr captured_word, xmm0
        jbe countdown_done
        fld dword ptr captured_word
        fsub dword ptr step_word
        fstp dword ptr [ecx + 48h]
    countdown_done:
    }
}

void tick_native_session_transport_countdowns_0076c4a0(void* session,
    std::uint32_t step_word,
    const NativeSessionTransportCountdownBindings& bindings) noexcept {
    const volatile double* const maximum = &bindings.maximum_step_d7a280;
    const volatile float* const replacement = &bindings.limited_step_ce3800;
    const auto* const context = &bindings;
    __asm {
        mov eax, maximum
        fld qword ptr [eax]
        fld dword ptr step_word
        fcomip st(0), st(1)
        fstp st(0)
        jbe step_ready
        mov eax, replacement
        movss xmm0, dword ptr [eax]
        movss dword ptr step_word, xmm0
    step_ready:
        mov eax, session
        mov ecx, dword ptr [eax + 18ch]
        test ecx, ecx
        jz primary_countdown
        push context
        fld dword ptr step_word
        push ecx
        fstp dword ptr [esp]
        push ecx
        call tick_native_transport_countdown_00782840
        add esp, 0ch
    primary_countdown:
        mov eax, session
        mov ecx, dword ptr [eax + 188h]
        test ecx, ecx
        jz countdowns_done
        push context
        fld dword ptr step_word
        push ecx
        fstp dword ptr [esp]
        push ecx
        call tick_native_transport_countdown_00782840
        add esp, 0ch
    countdowns_done:
    }
}
} // namespace bsp
