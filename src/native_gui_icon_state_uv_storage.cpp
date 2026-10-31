#include "bsp/native_gui_icon_state_uv_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source supplier requires MSVC Win32 inline assembly.
#endif

namespace bsp {
void set_native_gui_icon_state_uv_storage_00ab1680(void* actual_40b_state,
    volatile std::uint32_t (&actual_argument_homes)[4],
    const volatile float& actual_one_00d7a24c) {
    void* const captured_receiver = actual_40b_state;
    volatile std::uint32_t* const argument_homes = &actual_argument_homes[0];
    const volatile float* const one_address = &actual_one_00d7a24c;
    __asm {
        // Source ABI address bindings only; no native payload/value sampling.
        mov ecx, captured_receiver
        mov esi, argument_homes
        mov edi, one_address

        mov eax, dword ptr [esi]
        mov edx, dword ptr [esi + 4]
        movss xmm0, dword ptr [ecx + 20h]
        movss xmm1, dword ptr [edi]
        xorps xmm2, xmm2
        mov dword ptr [ecx + 0ch], eax
        mov eax, dword ptr [esi + 8]
        mov dword ptr [ecx + 10h], edx
        mov edx, dword ptr [esi + 0ch]
        mov dword ptr [ecx + 14h], eax
        ucomiss xmm0, xmm1
        lahf
        test ah, 44h
        mov dword ptr [ecx + 18h], edx
        jp uv_skip_v
        movss xmm0, dword ptr [ecx + 28h]
        ucomiss xmm0, xmm2
        lahf
        test ah, 44h
        jp uv_skip_v
        fld dword ptr [ecx + 18h]
        movss xmm0, dword ptr [ecx + 10h]
        fstp dword ptr [ecx + 10h]
        movss dword ptr [ecx + 18h], xmm0
    uv_skip_v:
        movss xmm0, dword ptr [ecx + 1ch]
        ucomiss xmm0, xmm1
        lahf
        test ah, 44h
        jp uv_done
        movss xmm0, dword ptr [ecx + 24h]
        ucomiss xmm0, xmm2
        lahf
        test ah, 44h
        jp uv_done
        fld dword ptr [ecx + 14h]
        movss xmm0, dword ptr [ecx + 0ch]
        fstp dword ptr [ecx + 0ch]
        movss dword ptr [ecx + 14h], xmm0
    uv_done:
    }
}
} // namespace bsp
