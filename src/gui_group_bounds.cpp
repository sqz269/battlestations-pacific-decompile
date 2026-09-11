#include "bsp/gui_group_bounds.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include <cstddef>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error GUI native group bounds require MSVC Win32 x87 assembly.
#endif

namespace bsp {
static_assert(offsetof(GuiGroupBoundsCrtAccess, sse2_conversion_0109eea4) == 4);
static_assert(offsetof(NativeNodeStorage, untouched_08) == 8);
static_assert(offsetof(NativeNodeStorage, auxiliary_flags_138) == 0x138);
static_assert(offsetof(NativeGroupTailStorage, enabled_175) == 1);
namespace {
// ECX actual slot; EDX unused; stack sphere pointer; RET4, as original.
__declspec(naked) void __fastcall write_group_bounds_kernel(void*, void*, const void*) {
    __asm {
        and dword ptr [ecx + 138h], 0ffffffcfh
        mov eax, dword ptr [esp + 4]
        mov byte ptr [ecx + 175h], 0
        fld dword ptr [eax]
        fstp dword ptr [ecx + 8]
        fld dword ptr [eax + 4]
        fstp dword ptr [ecx + 0ch]
        fld dword ptr [eax + 8]
        fstp dword ptr [ecx + 10h]
        fld dword ptr [eax + 0ch]
        fstp dword ptr [ecx + 14h]
        ret 4
    }
}
__declspec(naked) float __fastcall radius_kernel(const volatile double*, const GuiGroupBoundsCrtAccess*) {
    __asm {
        push esi
        mov esi, edx
        sub esp, 4
        fld qword ptr [ecx] // AC5F00: original input remains binary64 here
        mov ecx, dword ptr [esi]
        call native_crt_sqrt_st0_00bf7030
        fstp dword ptr [esp] // AC5F20
        fld dword ptr [esp] // AC5F24
        mov ecx, dword ptr [esi + 4] // fresh actual runtime global AFTER sqrt
        call native_crt_truncate_st0_00bf7420
        cvtsi2ss xmm0, eax // AC5F34
        movss dword ptr [esp], xmm0 // AC5F39 (native after PUSH destination)
        fld dword ptr [esp] // host float return only; finite, exact binary32
        add esp, 4
        pop esi
        ret
    }
}
}
void set_native_gui_group_bounds_00b8e6c0(NativeGroupOwner& owner, const void* sphere) {
    if (owner.phase != NativeGroupOwner::Phase::live ||
        owner.storage.node.vtable_00 != 0x00d634f8u ||
        &owner.node.storage != &owner.storage.node ||
        reinterpret_cast<std::byte*>(&owner.storage.group) !=
            reinterpret_cast<std::byte*>(&owner.storage.node) + 0x174)
        throw std::invalid_argument("group bounds require the same live actual cGroup owner");
    write_group_bounds_kernel(&owner.storage.node, nullptr, sphere);
}
float gui_screen_root_radius_00ac5f00(const volatile double& squared_radius,
    const GuiGroupBoundsCrtAccess& access) {
    if (!access.sqrt || !access.sqrt->dispatch_bypass_0109dd78 ||
        !access.sqrt->except_00c27489 || !access.sse2_conversion_0109eea4)
        throw std::invalid_argument("GUI radius requires actual CRT and conversion-global bindings");
    return radius_kernel(&squared_radius, &access);
}
} // namespace bsp
