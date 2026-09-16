#include "bsp/native_occlusion_query_poll.hpp"

namespace bsp {

// Keep the original indirect COM call, post-call ordering and AL-only result.
// EAX's upper 24 bits retain the HRESULT on the nonnull path; null returns
// EAX=1. The source entry adds no compiler prologue, layout or hidden argument.
__declspec(naked) bool __fastcall poll_native_occlusion_query_00b5fca0(void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 10h]
        test eax, eax
        jz absent
        mov ecx, dword ptr [eax]
        push 1
        push 4
        lea edx, [esi + 0Ch]
        push edx
        push eax
        mov eax, dword ptr [ecx + 1Ch]
        call eax
        test eax, eax
        jnz result
        mov dword ptr [esi + 8], 2
    result:
        xor ecx, ecx
        test eax, eax
        setz cl
        mov al, cl
        pop esi
        ret
    absent:
        mov al, 1
        pop esi
        ret
    }
}

} // namespace bsp
