#include "bsp/native_world_expiry_pass.hpp"
#include "bsp/native_world_child_retirement.hpp"

#if defined(_MSC_VER) && defined(_M_IX86)

namespace bsp {

__declspec(naked) void __fastcall
release_native_world_expired_objects_00903610(void*) {
    __asm {
        push ebx                       // 00903610
        mov ebx, ecx                   // 00903611
        mov eax, dword ptr [ebx + 4]    // 00903613
        push esi                       // 00903616
        mov esi, dword ptr [eax]        // 00903617
        push edi                       // 00903619
        xor edi, edi                   // 0090361A
        test esi, esi                  // 0090361C
        jz done                        // 0090361E

    entity_loop:
        mov eax, dword ptr [esi + 6ch]  // 00903620
        test eax, eax                  // 00903623
        jle next_entity                // 00903625
        add eax, 1                     // 00903627
        cmp eax, 3                     // 0090362A
        mov dword ptr [esi + 6ch], eax  // 0090362D
        jl age_entity                  // 00903630
        cmp dword ptr [esi + 50h], 0    // 00903632
        jz release_own                 // 00903636

    release_children:
        mov ecx, dword ptr [esi + 48h]  // 00903638
        call retire_native_world_child_subtree_009035e0 // 0090363B
        cmp dword ptr [esi + 50h], 0    // 00903640
        jnz release_children           // 00903644

    release_own:
        mov edx, dword ptr [esi]        // 00903646
        mov eax, dword ptr [edx]        // 00903648
        push 1                         // 0090364A
        mov ecx, esi                   // 0090364C
        call eax                       // 0090364E
        test edi, edi                  // 00903650
        jz resume_head                 // 00903652
        mov esi, dword ptr [edi + 38h]  // 00903654
        jmp check_entity               // 00903657

    resume_head:
        mov ecx, dword ptr [ebx + 4]    // 00903659
        mov esi, dword ptr [ecx]        // 0090365C
        jmp check_entity               // 0090365E

    age_entity:
        mov edi, esi                   // 00903660
    next_entity:
        mov esi, dword ptr [esi + 38h]  // 00903662
    check_entity:
        test esi, esi                  // 00903665
        jnz entity_loop                // 00903667

    done:
        pop edi                        // 00903669
        pop esi                        // 0090366A
        pop ebx                        // 0090366B
        ret                            // 0090366C
    }
}

} // namespace bsp

#endif
