#include "bsp/native_gui_widget_resolved_position.hpp"
#include "bsp/game_native_readonly_data.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(double) == 8);

// Private Source ABI: ECX actual widget, EDX authenticated D7A258 cell,
// one stacked output pointer; EAX output, RET4. Original AA6750 never touches
// EDX. The concrete self-call retains it, with no extra stack words/spills.
// Only the final qword-memory addressing differs from the original body.
__declspec(naked) void resolved_position_kernel_00aa6750() {
    __asm {
        sub esp, 0x20
        push esi
        mov esi, ecx
        cmp dword ptr [esi + 0x70], 0
        jz resolved_no_parent
        mov ecx, dword ptr [esi + 0x70]
        fld dword ptr [ecx + 0x20]
        lea eax, [esp + 0x18]
        fmul dword ptr [ecx + 0x18]
        push eax
        fstp dword ptr [esp + 0x8]
        fld dword ptr [ecx + 0x24]
        fmul dword ptr [ecx + 0x1c]
        fstp dword ptr [esp + 0xc]
        call resolved_position_kernel_00aa6750
        fld dword ptr [eax]
        fadd dword ptr [esi + 0xc]
        fstp dword ptr [esp + 0xc]
        fld dword ptr [eax + 0x4]
        fadd dword ptr [esi + 0x10]
        fstp dword ptr [esp + 0x10]
        fld dword ptr [eax + 0x8]
        mov eax, dword ptr [esp + 0x28]
        fadd dword ptr [esi + 0x14]
        pop esi
        fstp dword ptr [esp + 0x10]
        fld dword ptr [esp + 0x8]
        fsub dword ptr [esp]
        fstp dword ptr [eax]
        fld dword ptr [esp + 0xc]
        fsub dword ptr [esp + 0x4]
        fstp dword ptr [eax + 0x4]
        fld dword ptr [esp + 0x10]
        fsub qword ptr [edx]
        fstp dword ptr [eax + 0x8]
        add esp, 0x20
        ret 0x4
    resolved_no_parent:
        mov eax, dword ptr [esp + 0x28]
        fld dword ptr [esi + 0xc]
        fstp dword ptr [eax]
        fld dword ptr [esi + 0x10]
        fstp dword ptr [eax + 0x4]
        fld dword ptr [esi + 0x14]
        pop esi
        fstp dword ptr [eax + 0x8]
        add esp, 0x20
        ret 0x4
    }
}
} // namespace

NativeGuiWidgetResolvedPositionData::NativeGuiWidgetResolvedPositionData(
    const game::GameNativeReadOnlyData& actual_data)
    : actual_d7a258_(static_cast<const volatile double*>(
        actual_data.data_at(0x00d7a258, 8))) {}

void* get_native_gui_widget_resolved_position_00aa6750(void* actual_widget,
    void* actual_output_xyz, const NativeGuiWidgetResolvedPositionData& binding) {
    const volatile double* const actual_cell = binding.actual_d7a258_;
    void* result;
    __asm {
        mov ecx, actual_widget
        mov edx, actual_cell
        push actual_output_xyz
        call resolved_position_kernel_00aa6750
        mov result, eax
    }
    return result;
}
} // namespace bsp
