#include "bsp/native_gui_widget_resolved_setter.hpp"
#include "bsp/game_native_gui_text_state.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_gui_frame_box_constructed_storage.hpp"
#include <cstddef>

namespace bsp {
namespace {
struct SetterCall {
    const volatile double* actual_d7a258;
    const NativeGuiWidgetResolvedPositionData* resolved;
    NativeGuiWidgetTransformScratch* transform_scratch;
    NativeGuiWidgetTransformBindings* transform;
    GuiWidgetBounds* bounds_scratch;
    const volatile double* actual_half;
};
static_assert(sizeof(void*) == 4 && sizeof(double) == 8);
static_assert(offsetof(SetterCall, actual_d7a258) == 0);
static_assert(sizeof(SetterCall) == 24);

__declspec(noinline) void* __fastcall resolved_getter_bridge(
    void* parent, const SetterCall* call, void* output) {
    return get_native_gui_widget_resolved_position_00aa6750(
        parent, output, *call->resolved);
}
__declspec(noinline) void __fastcall resolved_transform_bridge(
    void* widget, const SetterCall* call) {
    recompose_native_gui_widget_transform_00aa7220(widget,
        *call->transform_scratch, *call->transform);
}
__declspec(noinline) void __fastcall resolved_bounds_bridge(
    void* widget, const SetterCall* call) {
    refresh_native_gui_widget_local_bounds_00aa70e0(widget,
        *call->bounds_scratch, *call->actual_half);
}

// Raw private ABI: ECX widget / EDX SetterCall / one stacked XYZ pointer.
// The saved EBP holds borrowed context across the actual C++ call bridges.
// Its extra save shifts ONLY the two original incoming-argument reads by 4;
// every original scratch operand retains its ESP-relative displacement.
__declspec(naked) void resolved_setter_kernel_00aa8240() {
    __asm {
        push ebp
        mov ebp,edx
        sub esp,2ch
        push esi
        mov esi,ecx
        cmp dword ptr [esi+70h],0
        jz resolved_no_parent
        mov ecx,dword ptr [esi+70h]
        fld dword ptr [ecx+20h]
        lea eax,[esp+24h]
        fmul dword ptr [ecx+18h]
        push eax
        fstp dword ptr [esp+8]
        fld dword ptr [ecx+24h]
        fmul dword ptr [ecx+1ch]
        fstp dword ptr [esp+0ch]
        mov edx,ebp
        call resolved_getter_bridge
        mov edx,dword ptr [ebp]
        mov ecx,dword ptr [esp+38h]
        fld dword ptr [ecx]
        fsub dword ptr [eax]
        fstp dword ptr [esp+0ch]
        fld dword ptr [ecx+4]
        fsub dword ptr [eax+4]
        fstp dword ptr [esp+10h]
        fld dword ptr [ecx+8]
        mov ecx,esi
        fsub dword ptr [eax+8]
        fstp dword ptr [esp+14h]
        fld dword ptr [esp+0ch]
        fadd dword ptr [esp+4]
        fstp dword ptr [esp+18h]
        fld dword ptr [esp+10h]
        fadd dword ptr [esp+8]
        fstp dword ptr [esp+1ch]
        fld dword ptr [esp+14h]
        fadd qword ptr [edx]
        fstp dword ptr [esp+20h]
        fld dword ptr [esp+18h]
        fstp dword ptr [esi+0ch]
        fld dword ptr [esp+1ch]
        fstp dword ptr [esi+10h]
        fld dword ptr [esp+20h]
        fstp dword ptr [esi+14h]
        mov edx,ebp
        call resolved_transform_bridge
        mov ecx,esi
        mov edx,ebp
        call resolved_bounds_bridge
        pop esi
        add esp,2ch
        pop ebp
        ret 4
    resolved_no_parent:
        mov eax,dword ptr [esp+38h]
        movss xmm0,dword ptr [eax]
        movss xmm1,dword ptr [eax+4]
        movss xmm2,dword ptr [eax+8]
        movss dword ptr [esi+0ch],xmm0
        movss dword ptr [esi+10h],xmm1
        movss dword ptr [esi+14h],xmm2
        mov edx,ebp
        call resolved_transform_bridge
        mov ecx,esi
        mov edx,ebp
        call resolved_bounds_bridge
        pop esi
        add esp,2ch
        pop ebp
        ret 4
    }
}
} // namespace

NativeGuiWidgetResolvedSetterBindings::NativeGuiWidgetResolvedSetterBindings(
    const game::GameNativeReadOnlyData& data,
    game::GameNativeGuiTextState& gui_state,
    NativeGuiFrameBoxConstructedTransformDispatch& dispatch)
    : resolved_(data),
      transform_{*static_cast<const volatile float*>(data.data_at(0x00d7a24c, 4)),
          *static_cast<const volatile float*>(data.data_at(0x00d7a208, 4)),
          gui_state.widget_y_scale_00e12fc4(), dispatch},
      actual_d7a258_(static_cast<const volatile double*>(data.data_at(0x00d7a258, 8))),
      actual_half_00d7a280_(static_cast<const volatile double*>(data.data_at(0x00d7a280, 8))) {}

void set_native_gui_widget_resolved_position_00aa8240(void* widget,
    const void* xyz, NativeGuiWidgetResolvedSetterBindings& bindings,
    NativeGuiWidgetTransformScratch& transform_scratch, GuiWidgetBounds& bounds_scratch) {
    const SetterCall call{bindings.actual_d7a258_, &bindings.resolved_,
        &transform_scratch, &bindings.transform_, &bounds_scratch,
        bindings.actual_half_00d7a280_};
    const SetterCall* const actual_call = &call;
    __asm {
        mov ecx,widget
        mov edx,actual_call
        push xyz
        call resolved_setter_kernel_00aa8240
    }
}
} // namespace bsp
