#include "bsp/native_property_tree_library_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native property tree library storage requires MSVC Win32.
#endif
namespace bsp {
static_assert(sizeof(void*) == 4);

__declspec(naked) void* __fastcall
initialize_native_property_tree_library_storage_008f5670(void*) noexcept {
    __asm {
        mov edx, ecx
        push edi
        mov dword ptr [edx], 00d1650ch
        xor eax, eax
        lea edi, [edx + 0ch]
        mov ecx, 40h
        mov dword ptr [edx + 4], 00d162c8h
        mov dword ptr [edx + 8], 0
        rep stosd
        mov eax, edx
        pop edi
        ret
    }
}
} // namespace bsp
