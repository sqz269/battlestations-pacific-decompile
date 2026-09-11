#include "bsp/native_resource_registry_base.hpp"

#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource registry base entries require MSVC Win32.
#endif

namespace bsp {

__declspec(naked) void* __fastcall construct_native_resource_registry_base_00b197e0(
    void*) {
    __asm {
        mov eax, ecx
        mov dword ptr [eax], 0x00d5e550
        ret
    }
}

__declspec(naked) void* __fastcall delete_native_resource_registry_base_00b197f0(
    void*, void* volatile*, std::uint32_t) {
    __asm {
        test byte ptr [esp + 4], 1
        push esi
        mov esi, ecx
        // Original ten-byte MOV [F8D41C],0, retaining its length and flags.
        // New binding: MOV DWORD PTR [EDX+disp32(0)],0.
        _emit 0xc7
        _emit 0x82
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        mov dword ptr [esi], 0x00ce3818
        jz retained
        push esi
        call singleton_lifetime_free
        add esp, 4
    retained:
        mov eax, esi
        pop esi
        ret 4
    }
}

} // namespace bsp
