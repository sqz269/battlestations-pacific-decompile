#include "bsp/native_post_effect_draw_record.hpp"
#include "bsp/native_traceline_render.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(float) == 4,
    "The original post-effect draw-record entry is MSVC Win32.");

__declspec(naked) void* __fastcall
construct_native_post_effect_draw_record_00b51bd0(void*,
    const NativeTracelineRenderAccess*, float, void*, void*, void*, void*, float) {
    __asm {
        push edx // Added borrowed access outside the original stack frame.
        fldz // B51BD0: exact positive-zero depth, retained until its native spill.
        mov eax,dword ptr [esp + 18h] // B51BD2: original camera argument.
        mov edx,dword ptr [esp + 10h] // B51BD6: original geometry argument.
        push esi // B51BDA
        push 555h // B51BDB
        sub esp,8 // B51BE0
        fstp dword ptr [esp + 4] // B51BE3: depth argument.
        mov esi,ecx // B51BE7: captured original allocation.
        fld dword ptr [esp + 2ch] // B51BE9: visibility x87 load.
        mov ecx,dword ptr [esp + 24h] // B51BED: original model argument.
        fstp dword ptr [esp] // B51BF1: visibility x87 spill.
        push eax // B51BF4: camera.
        mov eax,dword ptr [esp + 20h] // B51BF5: original section argument.
        fld dword ptr [esp + 1ch] // B51BF9: leading x87 load.
        push ecx // B51BFD: model.
        push edx // B51BFE: geometry.
        push eax // B51BFF: section.
        push ecx // B51C00: reserve the leading argument word.
        mov ecx,esi // B51C01
        fstp dword ptr [esp] // B51C03: leading x87 spill.
        mov edx,dword ptr [esp + 24h] // Added access, after original argument stores.
        call initialize_native_render_entry_00b51a20 // B51C06; original RET20h.
        mov eax,esi // B51C0B: exact caller allocation identity.
        pop esi // B51C0D
        pop edx // Discard the added access slot.
        ret 18h // B51C0E: six original DWORD arguments.
    }
}

} // namespace bsp
