#include "bsp/native_allocator_owned_message_constructor.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>

// Inline assembly needs the callable CRT symbol rather than the intrinsic.
#pragma function(strlen)

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);
static_assert(sizeof(std::size_t) == 4);

// Preserve the complete owned schedule with current callable Source CRT
// bindings. Actual emitted call forms and linked identities remain build-owned.
__declspec(naked) void* __fastcall construct_native_allocator_owned_message_00bf6340(
    void*, std::uint32_t, const void*) {
    __asm {
        push ebx                              // 00BF6340
        mov ebx, dword ptr [esp + 8]          // 00BF6341
        push esi                              // 00BF6345
        push edi                              // 00BF6346
        mov edi, ecx                          // 00BF6347
        mov dword ptr [edi], 00d69370h        // 00BF6349
        mov eax, dword ptr [ebx]              // 00BF634F
        test eax, eax                         // 00BF6351
        jz null_message                       // 00BF6353
        push eax                              // 00BF6355
        call strlen                           // 00BF6356
        mov esi, eax                          // 00BF635B
        inc esi                               // 00BF635D
        push esi                              // 00BF635E
        call malloc                           // 00BF635F
        test eax, eax                         // 00BF6364
        pop ecx                               // 00BF6366
        pop ecx                               // 00BF6367
        mov dword ptr [edi + 4], eax          // 00BF6368
        jz message_constructed                // 00BF636B
        push dword ptr [ebx]                  // 00BF636D
        push esi                              // 00BF636F
        push eax                              // 00BF6370
        call strcpy_s                         // 00BF6371
        add esp, 0ch                          // 00BF6376
        jmp message_constructed               // 00BF6379
    null_message:
        and dword ptr [edi + 4], 0            // 00BF637B
    message_constructed:
        mov dword ptr [edi + 8], 1            // 00BF637F
        mov eax, edi                          // 00BF6386
        pop edi                               // 00BF6388
        pop esi                               // 00BF6389
        pop ebx                               // 00BF638A
        ret 4                                 // 00BF638B
    }
}

} // namespace bsp
