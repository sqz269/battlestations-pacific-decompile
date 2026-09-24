#include "bsp/native_gui_page_default.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI page default construction requires MSVC Win32.
#endif
namespace bsp {
void* construct_native_gui_page_default_00aa3840(void* page,
    NativeGuiPageDefaultBindings& bindings) {
    const volatile std::uint32_t* const first=&bindings.scalar_00d7a2f0;
    const volatile std::uint32_t* const second=&bindings.scalar_00ce3804;
    const volatile std::uint32_t* const one=&bindings.one_00d7a24c;
    construct_native_gui_widget_base_00aa9390(page,1,bindings.one_00d7a24c);
    // MOVSS transports raw bits without arithmetic or additional spills.
    // The first current load precedes the stamp; the later loads follow the
    // intervening stores, including when supplied cells alias actual storage.
    __asm {
        mov esi,page
        mov eax,first
        movss xmm0,dword ptr [eax]
        mov dword ptr [esi],00d5be38h
        xor eax,eax
        mov dword ptr [esi+100h],eax
        mov dword ptr [esi+104h],eax
        mov dword ptr [esi+108h],eax
        movss dword ptr [esi+10ch],xmm0
        mov ecx,second
        movss xmm0,dword ptr [ecx]
        movss dword ptr [esi+110h],xmm0
        mov ecx,one
        movss xmm0,dword ptr [ecx]
        movss dword ptr [esi+114h],xmm0
        xorps xmm0,xmm0
        mov dword ptr [esi+118h],eax
        movss dword ptr [esi+11ch],xmm0
    }
    return page;
}
} // namespace bsp
