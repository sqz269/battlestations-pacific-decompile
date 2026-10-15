#include "bsp/native_avoid_container_tidy.hpp"

#include "bsp/native_avoid_manager_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);

// Preserve the actual raw byte/stack schedule. The only substitutions are
// fixed calls to the existing real Source free and complete hull-list clear.
__declspec(naked) void __fastcall tidy_native_avoid_body_vector_0041ddf0(void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 4]
        test eax, eax
        jz clear_words
        push eax
        call singleton_lifetime_free
        add esp, 4
    clear_words:
        mov dword ptr [esi + 4], 0
        mov dword ptr [esi + 8], 0
        mov dword ptr [esi + 0ch], 0
        pop esi
        ret
    }
}

__declspec(naked) void __fastcall tidy_native_avoid_hull_list_0041f230(void*) {
    __asm {
        push esi
        mov esi, ecx
        call clear_native_avoid_hull_list_0041e200
        mov eax, dword ptr [esi + 4]
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 4], 0
        pop esi
        ret
    }
}

__declspec(naked) void __fastcall tidy_native_avoid_hull_list_0041e3a0(void*) {
    __asm {
        push esi
        mov esi, ecx
        call clear_native_avoid_hull_list_0041e200
        mov eax, dword ptr [esi + 4]
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 4], 0
        pop esi
        ret
    }
}

} // namespace bsp
