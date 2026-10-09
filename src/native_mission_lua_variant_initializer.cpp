#include "bsp/native_mission_lua_variant_initializer.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua variant initializer requires MSVC Win32.
#endif

namespace bsp {

// 008849B0..008849CA, all 27 bytes / 8 instructions. Preserve raw DWORD stores,
// their order, full EAX receiver return and ECX zeroing without a typed owner.
// 00D0E6F4h is deliberately an immediate, not a Source function/table binding.
__declspec(naked) void* __fastcall
initialize_native_mission_lua_variant_008849b0(void*) {
    __asm {
        mov eax, ecx                         // 008849B0
        xor ecx, ecx                         // 008849B2
        mov dword ptr [eax], 00d0e6f4h       // 008849B4
        mov dword ptr [eax + 4], 0ffffffffh  // 008849BA
        mov dword ptr [eax + 8], ecx         // 008849C1
        mov dword ptr [eax + 0ch], ecx       // 008849C4
        mov dword ptr [eax + 10h], ecx       // 008849C7
        ret                                 // 008849CA
    }
}

} // namespace bsp
