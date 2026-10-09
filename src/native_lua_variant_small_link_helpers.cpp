#include "bsp/native_lua_variant_small_link_helpers.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);

// Complete 28-byte / 11-instruction Original span. Ordinary entry paths
// skip the three-byte LEA at 006ED9DD; retain its exact displacement encoding.
// Final emitted extent, branches and bytes remain for integrator build review.
__declspec(naked) void* __fastcall follow_native_lua_variant_links_006ed9d0(void*) {
    __asm {
        mov eax, ecx                          // 006ED9D0
        mov edx, dword ptr [eax + 8]          // 006ED9D2
        cmp byte ptr [edx + 31h], 0           // 006ED9D5
        jne short links_8_done                // 006ED9D9
        jmp short links_8_advance             // 006ED9DB
        _emit 08dh                            // 006ED9DD: LEA ECX,[ECX+disp8(0)]
        _emit 049h
        _emit 000h
    links_8_advance:
        mov eax, edx                          // 006ED9E0
        mov edx, dword ptr [eax + 8]          // 006ED9E2
        cmp byte ptr [edx + 31h], 0           // 006ED9E5
        je short links_8_advance              // 006ED9E9
    links_8_done:
        ret                                   // 006ED9EB
    }
}

// Complete 27-byte / 10-instruction Original span. The six-byte identity
// LEA executes only on the initial-zero path; the loop backedge bypasses it.
// Explicit bytes prevent shortening its disp32(0) encoding.
__declspec(naked) void* __fastcall follow_native_lua_variant_links_006ed9f0(void*) {
    __asm {
        mov eax, ecx                          // 006ED9F0
        mov edx, dword ptr [eax]              // 006ED9F2
        cmp byte ptr [edx + 31h], 0           // 006ED9F4
        jne short links_0_done                // 006ED9F8
        _emit 08dh                            // 006ED9FA: LEA EBX,[EBX+disp32(0)]
        _emit 09bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    links_0_advance:
        mov eax, edx                          // 006EDA00
        mov edx, dword ptr [eax]              // 006EDA02
        cmp byte ptr [edx + 31h], 0           // 006EDA04
        je short links_0_advance              // 006EDA08
    links_0_done:
        ret                                   // 006EDA0A
    }
}

} // namespace bsp
