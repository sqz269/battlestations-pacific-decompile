#include "bsp/native_scene_property_array_block_release.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This native register-ABI leaf requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4, "Native array headers have four-byte pointers");

// Exact whole native 39B/13-instruction schedule. The sole CALL relocation
// binds the genuine current cdecl free; no receiver adapter or cleanup hook.
__declspec(naked) void __fastcall
release_native_scene_property_array_block_008f03f0(void*) noexcept {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi]
        test eax, eax
        jz clear_header
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi], 0
    clear_header:
        mov dword ptr [esi + 4], 0
        mov dword ptr [esi], 0
        pop esi
        ret
    }
}
} // namespace bsp
