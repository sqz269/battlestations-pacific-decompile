#include "bsp/native_air_ops_holder_geometry.hpp"
#include "bsp/native_traceline_render.hpp"

namespace bsp {

// Preserve the complete native instruction sequence, with only its CALL bound
// to the already reconstructed complete raw 004142E0 Source implementation.
__declspec(naked) void* __fastcall native_air_ops_holder_local_less_t_006bcc90(
    const void*, void*, void*, const void*) {
    __asm {
        sub esp,0xc //006BCC90
        push esi //006BCC93
        mov esi,ecx //006BCC94
        lea eax,[esi+0x48] //006BCC96
        push eax //006BCC99
        lea ecx,[esp+0x8] //006BCC9A
        push ecx //006BCC9E
        mov ecx,[esp+0x20] //006BCC9F
        call transform_native_point_004142e0 //006BCCA3
        fld dword ptr [esp+0x4] //006BCCA8
        fsub dword ptr [esi+0xa4] //006BCCAC
        mov eax,[esp+0x14] //006BCCB2
        fstp dword ptr [eax] //006BCCB6
        fld dword ptr [esp+0x8] //006BCCB8
        fsub dword ptr [esi+0xa8] //006BCCBC
        fstp dword ptr [eax+0x4] //006BCCC2
        fld dword ptr [esp+0xc] //006BCCC5
        fsub dword ptr [esi+0xac] //006BCCC9
        pop esi //006BCCCF
        fstp dword ptr [eax+0x8] //006BCCD0
        add esp,0xc //006BCCD3
        ret 0x8 //006BCCD6
    }
}

} // namespace bsp
