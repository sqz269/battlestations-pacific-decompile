#include "bsp/native_allocator_base_cleanup.hpp"

#include <cstdlib>

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

// Preserve all seven physical instructions, including caller cleanup after
// the actual current Source CRT free call. Its import form/size is build-owned.
__declspec(naked) void __fastcall cleanup_native_allocator_base_00bf6454(
    void*, std::uint32_t) {
    __asm {
        cmp dword ptr [ecx + 8], 0            // 00BF6454
        mov dword ptr [ecx], 00d69370h         // 00BF6458
        je cleanup_complete                   // 00BF645E
        push dword ptr [ecx + 4]              // 00BF6460
        call free                             // 00BF6463
        pop ecx                               // 00BF6468
    cleanup_complete:
        ret                                   // 00BF6469
    }
}

} // namespace bsp
