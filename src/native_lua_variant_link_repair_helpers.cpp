#include "bsp/native_lua_variant_link_repair_helpers.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

// Complete 78-byte / 30-operation Original schedule. Keep every late reload
// and the POP ESI between the root CMP and its JNE. Emission is build-owned.
__declspec(naked) void* __fastcall repair_native_lua_variant_links_006edc80(
    void*, std::uint32_t, void*) {
    __asm {
        mov edx, dword ptr [esp + 4]          // 006EDC80
        mov eax, dword ptr [edx + 8]          // 006EDC84
        push esi                              // 006EDC87
        mov esi, dword ptr [eax]              // 006EDC88
        mov dword ptr [edx + 8], esi          // 006EDC8A
        mov esi, dword ptr [eax]              // 006EDC8D
        cmp byte ptr [esi + 31h], 0           // 006EDC8F
        jne left_skip_moved_parent            // 006EDC93
        mov dword ptr [esi + 4], edx          // 006EDC95
    left_skip_moved_parent:
        mov esi, dword ptr [edx + 4]          // 006EDC98
        mov dword ptr [eax + 4], esi          // 006EDC9B
        mov ecx, dword ptr [ecx + 4]          // 006EDC9E
        cmp edx, dword ptr [ecx + 4]          // 006EDCA1
        pop esi                               // 006EDCA4
        jne left_nonroot                      // 006EDCA5
        mov dword ptr [ecx + 4], eax          // 006EDCA7
        mov dword ptr [eax], edx              // 006EDCAA
        mov dword ptr [edx + 4], eax          // 006EDCAC
        ret 4                                 // 006EDCAF
    left_nonroot:
        mov ecx, dword ptr [edx + 4]          // 006EDCB2
        cmp edx, dword ptr [ecx]              // 006EDCB5
        jne left_other_parent                 // 006EDCB7
        mov dword ptr [ecx], eax              // 006EDCB9
        mov dword ptr [eax], edx              // 006EDCBB
        mov dword ptr [edx + 4], eax          // 006EDCBD
        ret 4                                 // 006EDCC0
    left_other_parent:
        mov dword ptr [ecx + 8], eax          // 006EDCC3
        mov dword ptr [eax], edx              // 006EDCC6
        mov dword ptr [edx + 4], eax          // 006EDCC8
        ret 4                                 // 006EDCCB
    }
}

// Complete mirrored 82-byte / 30-operation Original schedule. Captured EAX
// and EDX survive all three RET4 arms; no Source child or guard is added.
__declspec(naked) void* __fastcall repair_native_lua_variant_links_006edcd0(
    void*, std::uint32_t, void*) {
    __asm {
        mov edx, dword ptr [esp + 4]          // 006EDCD0
        mov eax, dword ptr [edx]              // 006EDCD4
        push esi                              // 006EDCD6
        mov esi, dword ptr [eax + 8]          // 006EDCD7
        mov dword ptr [edx], esi              // 006EDCDA
        mov esi, dword ptr [eax + 8]          // 006EDCDC
        cmp byte ptr [esi + 31h], 0           // 006EDCDF
        jne right_skip_moved_parent           // 006EDCE3
        mov dword ptr [esi + 4], edx          // 006EDCE5
    right_skip_moved_parent:
        mov esi, dword ptr [edx + 4]          // 006EDCE8
        mov dword ptr [eax + 4], esi          // 006EDCEB
        mov ecx, dword ptr [ecx + 4]          // 006EDCEE
        cmp edx, dword ptr [ecx + 4]          // 006EDCF1
        pop esi                               // 006EDCF4
        jne right_nonroot                     // 006EDCF5
        mov dword ptr [ecx + 4], eax          // 006EDCF7
        mov dword ptr [eax + 8], edx          // 006EDCFA
        mov dword ptr [edx + 4], eax          // 006EDCFD
        ret 4                                 // 006EDD00
    right_nonroot:
        mov ecx, dword ptr [edx + 4]          // 006EDD03
        cmp edx, dword ptr [ecx + 8]          // 006EDD06
        jne right_other_parent                // 006EDD09
        mov dword ptr [ecx + 8], eax          // 006EDD0B
        mov dword ptr [eax + 8], edx          // 006EDD0E
        mov dword ptr [edx + 4], eax          // 006EDD11
        ret 4                                 // 006EDD14
    right_other_parent:
        mov dword ptr [ecx], eax              // 006EDD17
        mov dword ptr [eax + 8], edx          // 006EDD19
        mov dword ptr [edx + 4], eax          // 006EDD1C
        ret 4                                 // 006EDD1F
    }
}

} // namespace bsp
