#include "bsp/native_allocator_base_copy_constructor.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>

// Inline assembly needs the callable CRT symbol rather than the intrinsic.
#pragma function(strlen)

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);
static_assert(sizeof(std::size_t) == 4);

// Retain every owned instruction and branch. CRT identifiers bind to current
// Source providers; their implementations and addresses are not Native proof.
__declspec(naked) void* __fastcall construct_native_allocator_base_copy_00bf63a6(
    void*, std::uint32_t, const void*) {
    __asm {
        push ebx                              // 00BF63A6
        mov ebx, dword ptr [esp + 8]           // 00BF63A7
        push esi                              // 00BF63AB
        mov esi, ecx                          // 00BF63AC
        mov dword ptr [esi], 00d69370h         // 00BF63AE
        mov eax, dword ptr [ebx + 8]           // 00BF63B4
        mov dword ptr [esi + 8], eax           // 00BF63B7
        test eax, eax                         // 00BF63BA
        mov eax, dword ptr [ebx + 4]           // 00BF63BC
        push edi                              // 00BF63BF
        jz shallow_message                    // 00BF63C0
        test eax, eax                         // 00BF63C2
        jz null_owned_message                 // 00BF63C4
        push eax                              // 00BF63C6
        call strlen                           // 00BF63C7
        mov edi, eax                          // 00BF63CC
        inc edi                               // 00BF63CE
        push edi                              // 00BF63CF
        call malloc                           // 00BF63D0
        test eax, eax                         // 00BF63D5
        pop ecx                               // 00BF63D7
        pop ecx                               // 00BF63D8
        mov dword ptr [esi + 4], eax           // 00BF63D9
        jz copy_complete                      // 00BF63DC
        push dword ptr [ebx + 4]              // 00BF63DE
        push edi                              // 00BF63E1
        push eax                              // 00BF63E2
        call strcpy_s                         // 00BF63E3
        add esp, 0ch                          // 00BF63E8
        jmp copy_complete                     // 00BF63EB
    null_owned_message:
        and dword ptr [esi + 4], 0            // 00BF63ED
        jmp copy_complete                     // 00BF63F1
    shallow_message:
        mov dword ptr [esi + 4], eax           // 00BF63F3
    copy_complete:
        pop edi                               // 00BF63F6
        mov eax, esi                          // 00BF63F7
        pop esi                               // 00BF63F9
        pop ebx                               // 00BF63FA
        ret 4                                 // 00BF63FB
    }
}

} // namespace bsp
