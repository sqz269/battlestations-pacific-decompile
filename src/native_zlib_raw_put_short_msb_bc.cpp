#include "bsp/native_zlib_raw_put_short_msb_bc.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This reconstruction requires MSVC Win32 inline assembly.
#endif

// Native 00BCB1F0..00BCB216: complete 39B/16-instruction returning body.
// The same-function prefix transports two genuine Source inputs into the
// Original EAX/ECX contract. It adds no stack frame or Native address credit.
// Keep the buffer reload before the first pending RMW and the actual mutable
// EDI/ESI saves. All raw data/control aliases and wrapping operands remain real.
extern "C" __declspec(naked) void __fastcall
bsp_native_zlib_put_short_msb_00bcb1f0(void*, std::uint32_t) noexcept
{
    __asm {
        mov eax, ecx
        mov ecx, edx

        push esi
        mov esi, dword ptr [eax + 8]
        push edi
        mov edi, dword ptr [eax + 14h]
        mov edx, ecx
        shr edx, 8
        mov byte ptr [esi + edi], dl
        mov edi, dword ptr [eax + 8]
        mov esi, 1
        add dword ptr [eax + 14h], esi
        mov edx, dword ptr [eax + 14h]
        mov byte ptr [edx + edi], cl
        add dword ptr [eax + 14h], esi
        pop edi
        pop esi
        ret
    }
}
