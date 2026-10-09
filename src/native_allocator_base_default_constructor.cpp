#include "bsp/native_allocator_base_default_constructor.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

// Preserve the complete five-operation body, including both prior DWORD
// reads, their ordered zero writes, the receiver result and final AND flags.
__declspec(naked) void* __fastcall construct_native_allocator_base_default_00bf632f(
    void*, std::uint32_t) {
    __asm {
        mov eax, ecx                          // 00BF632F
        and dword ptr [eax + 4], 0             // 00BF6331
        and dword ptr [eax + 8], 0             // 00BF6335
        mov dword ptr [eax], 00d69370h         // 00BF6339
        ret                                   // 00BF633F
    }
}

} // namespace bsp
