#include "bsp/native_allocator_base_message_constructor.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

// Retain every audited instruction, including the DWORD read-modify-write and
// its flags. The explicit second fastcall argument occupies EDX but is unused;
// the two actual Native argument words therefore remain at ESP+4 and ESP+8.
__declspec(naked) void* __fastcall construct_native_allocator_base_message_00bf638e(
    void*, std::uint32_t, const void*, std::uint32_t) {
    __asm {
        mov eax, ecx                         // 00BF638E
        mov ecx, dword ptr [esp + 4]          // 00BF6390
        mov dword ptr [eax], 00d69370h        // 00BF6394
        mov ecx, dword ptr [ecx]              // 00BF639A
        and dword ptr [eax + 8], 0            // 00BF639C
        mov dword ptr [eax + 4], ecx          // 00BF63A0
        ret 8                                // 00BF63A3
    }
}

} // namespace bsp
