#include "bsp/native_tracked_critical_section_release.hpp"

#include "bsp/random_threads.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Tracked critical-section release reconstruction requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(TrackedCriticalSection) == 0x1c);
static_assert(offsetof(TrackedCriticalSection, native) == 0);
static_assert(offsetof(TrackedCriticalSection, depth) == 0x18);

// Original complete 64-byte body. The two IAT references bind real Win32
// imports; BF65AC binds the existing malloc/new-handler allocation family's
// real free service. The latter does not reconstruct the game's static CRT.
// Keep the captured owner slot, section, import and signed counter schedule.
__declspec(naked) void __fastcall
release_native_tracked_critical_section_0041cc80(TrackedCriticalSection**) {
    __asm {
        push ebp
        mov ebp, ecx
        push esi
        mov esi, dword ptr [ebp]
        test esi, esi
        jz release_done
        cmp dword ptr [esi + 18h], 0
        jle delete_section
        push edi
        mov edi, dword ptr [LeaveCriticalSection]
    drain_depth:
        add dword ptr [esi + 18h], -1
        push esi
        call edi
        cmp dword ptr [esi + 18h], 0
        jg drain_depth
        pop edi
    delete_section:
        push esi
        call dword ptr [DeleteCriticalSection]
        push esi
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [ebp], 0
    release_done:
        pop esi
        pop ebp
        ret
    }
}
} // namespace bsp
