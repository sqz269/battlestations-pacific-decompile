#include "bsp/native_gui_widget_transform.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_particle_axial_loading.hpp"
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI widget transform requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word=std::uint32_t;
void* at(const void* p,Word offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+offset);
}
Word word(const void* p,Word offset=0) noexcept {
    return *static_cast<const volatile Word*>(at(p,offset));
}
} // namespace
void recompose_native_gui_widget_transform_00aa7220(void* widget,
    NativeGuiWidgetTransformScratch& scratch,NativeGuiWidgetTransformBindings& bindings) {
    void* const workspace=scratch.bytes;
    const volatile float* const one=&bindings.one_00d7a24c;
    const volatile float* const negative_zero=&bindings.negative_zero_00d7a208;
    const volatile float* const y_scale=&bindings.y_scale_00e12fc4;
    Word captured_vtable;
    // EBX is the original ESP after both saved registers (scratch-8). Before
    // native AA72C1 PUSH EDI, its ESP-relative operands therefore add four.
    // The y-scale stays live in ST0 until AA72C4; do not spill/reload it.
    __asm {
        mov esi,widget
        mov ebx,workspace
        sub ebx,8
        mov ecx,one
        movss xmm1,dword ptr [ecx] // AA7226
        fld dword ptr [esi+10h]
        movss xmm2,dword ptr [esi+0ch]
        mov ecx,y_scale
        fld dword ptr [ecx] // AA7239
        movss xmm3,dword ptr [esi+14h]
        fld st(0)
        mov ecx,negative_zero
        movss xmm0,dword ptr [ecx] // AA7246
        fmulp st(2),st(0)
        subss xmm0,dword ptr [esi+48h]
        mov eax,dword ptr [esi+4ch] // AA7255 captures node
        fxch st(1)
        movss dword ptr [ebx+0c8h],xmm2
        fstp dword ptr [ebx+0dch]
        movss dword ptr [ebx+8],xmm0
        xorps xmm0,xmm0
        fld dword ptr [esi+18h]
        movss xmm2,dword ptr [ebx+0dch]
        fchs
        fmul dword ptr [esi+20h]
        movss dword ptr [ebx+0cch],xmm2
        movss xmm2,dword ptr [esi+28h]
        movss dword ptr [ebx+18h],xmm2
        fstp dword ptr [ebx+0ch]
        movss dword ptr [ebx+0d0h],xmm3
        fld dword ptr [esi+1ch]
        movss xmm2,dword ptr [ebx+0ch]
        movss xmm3,dword ptr [esi+2ch]
        fchs
        fmul dword ptr [esi+24h]
        movss dword ptr [ebx+88h],xmm2
        mov edi,dword ptr [eax] // AA72C2 captures this node's vtable
        fmulp st(1),st(0)
        movss dword ptr [ebx+98h],xmm1
        movss dword ptr [ebx+9ch],xmm0
        movss dword ptr [ebx+0a0h],xmm0
        fstp dword ptr [ebx+10h]
        movss dword ptr [ebx+0a4h],xmm0
        movss xmm2,dword ptr [ebx+10h]
        movss dword ptr [ebx+0a8h],xmm0
        movss dword ptr [ebx+0ach],xmm1
        movss dword ptr [ebx+0b0h],xmm0
        movss dword ptr [ebx+0b4h],xmm0
        movss dword ptr [ebx+0b8h],xmm0
        movss dword ptr [ebx+0bch],xmm0
        movss dword ptr [ebx+0c0h],xmm1
        movss dword ptr [ebx+0c4h],xmm0
        movss dword ptr [ebx+0d4h],xmm1
        movss dword ptr [ebx+1ch],xmm0
        movss dword ptr [ebx+20h],xmm0
        movss dword ptr [ebx+24h],xmm0
        movss dword ptr [ebx+28h],xmm0
        movss dword ptr [ebx+2ch],xmm3
        movss dword ptr [ebx+30h],xmm0
        movss dword ptr [ebx+34h],xmm0
        movss dword ptr [ebx+38h],xmm0
        movss dword ptr [ebx+3ch],xmm0
        movss dword ptr [ebx+40h],xmm1
        movss dword ptr [ebx+44h],xmm0
        movss dword ptr [ebx+48h],xmm0
        movss dword ptr [ebx+4ch],xmm0
        movss dword ptr [ebx+50h],xmm0
        movss dword ptr [ebx+54h],xmm1
        movss dword ptr [ebx+58h],xmm1
        movss dword ptr [ebx+5ch],xmm0
        movss dword ptr [ebx+60h],xmm0
        movss dword ptr [ebx+64h],xmm0
        movss dword ptr [ebx+68h],xmm0
        movss dword ptr [ebx+6ch],xmm1
        movss dword ptr [ebx+70h],xmm0
        movss dword ptr [ebx+74h],xmm0
        movss dword ptr [ebx+78h],xmm0
        movss dword ptr [ebx+7ch],xmm0
        movss dword ptr [ebx+80h],xmm1
        movss dword ptr [ebx+84h],xmm0
        movss dword ptr [ebx+8ch],xmm2
        movss dword ptr [ebx+90h],xmm0
        movss dword ptr [ebx+94h],xmm1
        mov captured_vtable,edi
    }
    // Default initialization starts the array lifetime without clearing bytes.
    auto* const rotation=::new(at(workspace,0x15c)) CameraMatrix;
    const float& angle=*static_cast<const float*>(workspace);
    build_gui_rotation_z_00b64780(*rotation,angle,negative_zero,one); // AA7423
    void* const first=multiply_native_camera_matrices_00413920(
        at(workspace,0x50),nullptr,at(workspace,0x19c),at(workspace,0x10)); // AA7442
    void* const second=multiply_native_camera_matrices_00413920(
        first,nullptr,at(workspace,0x11c),rotation); // AA7449
    void* const final=multiply_native_camera_matrices_00413920(
        second,nullptr,at(workspace,0xdc),at(workspace,0x90)); // AA7450
    void* const current_receiver=reinterpret_cast<void*>(word(widget,0x4c)); // AA7455
    const Word target=word(reinterpret_cast<void*>(captured_vtable),0x38); // AA7459
    bindings.dispatch.call_virtual38(target,current_receiver,final); // AA745C
}
} // namespace bsp
