#include "bsp/native_traceline_render.hpp"
#include "bsp/native_scene_render_traversal.hpp"
#include "bsp/native_camera_cache_getters.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_vector2_math.hpp"
#include "bsp/gui_text_content.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/system_time_constants.hpp"
#include <cstddef>
#include <exception>
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Traceline render requires MSVC Win32 x87/SSE instructions.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*)==4);
static_assert(offsetof(NativeTracelineRenderAccess,crt)==0);
static_assert(offsetof(NativeTracelineRenderAccess,frustum)==4);
static_assert(offsetof(NativeTracelineRenderAccess,mapping)==8);
static_assert(offsetof(NativeTracelineRenderAccess,clock_01090ab0)==12);
static_assert(offsetof(NativeTracelineRenderAccess,clock_virtuals)==16);
static_assert(offsetof(NativeTracelineRenderAccess,services)==20);
static_assert(offsetof(NativeTracelineRenderAccess,negative_zero_00d7a208)==24);
static_assert(offsetof(NativeTracelineRenderAccess,zero_00d7a218)==28);
static_assert(offsetof(NativeTracelineRenderAccess,one_00d7a24c)==32);
static_assert(offsetof(NativeTracelineRenderAccess,epsilon_00d7a238)==36);
static_assert(offsetof(NativeTracelineRenderAccess,alpha_00ce4b48)==40);
static_assert(offsetof(NativeTracelineRenderAccess,half_00d7a280)==44);
static_assert(offsetof(NativeTracelineRenderAccess,entry_cache_0108fe88)==48);
static_assert(offsetof(NativeTracelineRenderAccess,clock_current_14)==52);
static_assert(sizeof(ClockTimestamp)==16 && offsetof(ClockTimestamp,frequency)==8);

const ClockTimestamp* __fastcall clock_bridge(const NativeTracelineRenderAccess* a) {
    FrameClock* const actual=*a->clock_01090ab0;
    return a->clock_current_14(*actual,*a->clock_virtuals);
}
const void* __fastcall sphere_bridge(void* actual,const NativeTracelineRenderAccess* a) {
    const auto target=a->services->profile(actual)[0x48/4];
    if(target==0x00b6e8c0u) return get_native_model_world_sphere_00b6e8c0(actual,a);
    return a->services->call_virtual48(actual,target);
}

// Exact raw leaf 6EF890..6EF894: AL alone is the result; original RET8.
__declspec(naked) std::uint32_t __fastcall accept_native_model_006ef890(
    void*,void*,void*,float) {
    __asm { mov al,1 }
    __asm { ret 8 }
}
std::uint32_t __fastcall visibility_bridge(void* actual,const NativeTracelineRenderAccess* a,
    void* camera,float lod) {
    const auto target=a->services->profile(actual)[0x58/4];
    if(target==0x006ef890u) return accept_native_model_006ef890(actual,nullptr,camera,lod);
    return a->services->call_virtual58(actual,target,camera,lod);
}
void __fastcall render_child_bridge(void* actual,const NativeTracelineRenderAccess* a,
    void* context,float lod,float visibility,std::uint32_t flags) {
    const auto target=a->services->profile(actual)[0x20/4];
    if(const auto body=select_native_render20_body(target,a)) body(actual,a,context,lod,visibility,flags);
    else a->services->call_virtual20(actual,target,context,lod,visibility,flags);
}
void* __fastcall map_bridge(void* actual,const NativeTracelineRenderAccess* a,
    std::uint32_t count,std::uint32_t offset,std::uint8_t read_only) {
    // This packet has the actual B49980 logical stream profile only. Reject a
    // different current profile before treating its storage as that class.
    if(a->services->profile(actual)[0x10/4]!=0x00b49980u) std::terminate();
    return lock_native_logical_vertex_stream_00b49980(actual,*a->mapping,count,offset,read_only);
}
void __fastcall unmap_bridge(void* actual,const NativeTracelineRenderAccess* a) {
    if(a->services->profile(actual)[0x14/4]!=0x00b49a80u) std::terminate();
    unlock_native_logical_vertex_stream_00b49a80(actual,*a->mapping);
}
void* __fastcall geometry_bridge(void* actual,void*,std::uint32_t unused) {
    auto* tail=reinterpret_cast<NativeModelTailStorage*>(static_cast<std::byte*>(actual)+0x174);
    return gui_model_geometry_00b74640(*tail,unused);
}
void* __fastcall stream_bridge(void* actual,void*,std::uint32_t index) {
    return native_mesh_vertex_stream_00b73260(*static_cast<NativeMeshStorage*>(actual),index);
}
void* __fastcall section_bridge(void* actual,void*,std::int32_t index) {
    return gui_geometry_element_00b732c0(actual,index);
}
LONG __stdcall increment_bridge(volatile LONG* count) { return InterlockedIncrement(count); }
void __fastcall collect_bridge(void* command,const NativeTracelineRenderAccess* a,void* entry) {
    a->services->collect_entry_00b1dff0(command,entry);
}
} // namespace

const ClockTimestamp* __fastcall frame_clock_current_00bee050(FrameClock& clock) noexcept {
    return &clock.current;
}

// Complete 00af1c20; each trailing address identifies the native instruction.
__declspec(naked) void* __fastcall copy_native_traceline_vertex_00af1c20(void*,void*,const void*) {
    __asm {
        mov eax,ecx // 00af1c20
        mov ecx,dword ptr [esp + 0x4] // 00af1c22
        fld dword ptr [ecx] // 00af1c26
        fstp dword ptr [eax] // 00af1c28
        fld dword ptr [ecx + 0x4] // 00af1c2a
        fstp dword ptr [eax + 0x4] // 00af1c2d
        fld dword ptr [ecx + 0x8] // 00af1c30
        fstp dword ptr [eax + 0x8] // 00af1c33
        fld dword ptr [ecx + 0xc] // 00af1c36
        fstp dword ptr [eax + 0xc] // 00af1c39
        fld dword ptr [ecx + 0x10] // 00af1c3c
        fstp dword ptr [eax + 0x10] // 00af1c3f
        fld dword ptr [ecx + 0x14] // 00af1c42
        fstp dword ptr [eax + 0x14] // 00af1c45
        mov edx,dword ptr [ecx + 0x18] // 00af1c48
        mov dword ptr [eax + 0x18],edx // 00af1c4b
        fld dword ptr [ecx + 0x1c] // 00af1c4e
        fstp dword ptr [eax + 0x1c] // 00af1c51
        fld dword ptr [ecx + 0x20] // 00af1c54
        fstp dword ptr [eax + 0x20] // 00af1c57
        fld dword ptr [ecx + 0x24] // 00af1c5a
        fstp dword ptr [eax + 0x24] // 00af1c5d
        fld dword ptr [ecx + 0x28] // 00af1c60
        fstp dword ptr [eax + 0x28] // 00af1c63
        fld dword ptr [ecx + 0x2c] // 00af1c66
        fstp dword ptr [eax + 0x2c] // 00af1c69
        ret 0x4 // 00af1c6c
    }
}

// Complete 004142e0; each trailing address identifies the native instruction.
__declspec(naked) void* __fastcall transform_native_point_004142e0(const void*,void*,void*,const void*) {
    __asm {
        sub esp,0xc // 004142e0
        fld dword ptr [ecx + 0x4] // 004142e3
        mov eax,dword ptr [esp + 0x10] // 004142e6
        fstp dword ptr [esp] // 004142ea
        fld dword ptr [ecx] // 004142ed
        fstp dword ptr [esp + 0x4] // 004142ef
        fld dword ptr [ecx + 0x8] // 004142f3
        mov ecx,dword ptr [esp + 0x14] // 004142f6
        fstp dword ptr [esp + 0x8] // 004142fa
        fld dword ptr [ecx + 0x10] // 004142fe
        fld dword ptr [esp] // 00414301
        fld st(0) // 00414304
        fmulp st(2), st(0) // 00414306
        fld dword ptr [ecx] // 00414308
        fld dword ptr [esp + 0x4] // 0041430a
        fld st(0) // 0041430e
        fmulp st(2), st(0) // 00414310
        fxch st(3) // 00414312
        faddp st(1), st(0) // 00414314
        fld dword ptr [ecx + 0x20] // 00414316
        fld dword ptr [esp + 0x8] // 00414319
        fld st(0) // 0041431d
        fmulp st(2), st(0) // 0041431f
        fxch st(2) // 00414321
        faddp st(1), st(0) // 00414323
        fadd dword ptr [ecx + 0x30] // 00414325
        fstp dword ptr [eax] // 00414328
        fld dword ptr [ecx + 0x4] // 0041432a
        fmul st(0), st(3) // 0041432d
        fld dword ptr [ecx + 0x14] // 0041432f
        fmul st(0), st(3) // 00414332
        faddp st(1), st(0) // 00414334
        fld dword ptr [ecx + 0x24] // 00414336
        fmul st(0), st(2) // 00414339
        faddp st(1), st(0) // 0041433b
        fadd dword ptr [ecx + 0x34] // 0041433d
        fstp dword ptr [eax + 0x4] // 00414340
        fld dword ptr [ecx + 0x8] // 00414343
        fmulp st(3), st(0) // 00414346
        fld dword ptr [ecx + 0x18] // 00414348
        fmulp st(2), st(0) // 0041434b
        fxch st(2) // 0041434d
        faddp st(1), st(0) // 0041434f
        fld dword ptr [ecx + 0x28] // 00414351
        fmulp st(2), st(0) // 00414354
        faddp st(1), st(0) // 00414356
        fadd dword ptr [ecx + 0x38] // 00414358
        fstp dword ptr [eax + 0x8] // 0041435b
        add esp,0xc // 0041435e
        ret 0x8 // 00414361
    }
}

// Complete 007bb620; each trailing address identifies the native instruction.
__declspec(naked) float __fastcall maximum_native_basis_squared_007bb620(const void*) {
    __asm {
        sub esp,0x10 // 007bb620
        fld dword ptr [ecx + 0x4] // 007bb623
        fld dword ptr [ecx] // 007bb626
        fld dword ptr [ecx + 0x8] // 007bb628
        fld st(1) // 007bb62b
        fmulp st(2), st(0) // 007bb62d
        fld st(2) // 007bb62f
        fmulp st(3), st(0) // 007bb631
        fxch st(1) // 007bb633
        faddp st(2),st(0) // 007bb635
        fmul st(0), st(0) // 007bb637
        faddp st(1), st(0) // 007bb639
        fstp dword ptr [esp + 0x4] // 007bb63b
        fld dword ptr [ecx + 0x14] // 007bb63f
        fld dword ptr [ecx + 0x10] // 007bb642
        fld dword ptr [ecx + 0x18] // 007bb645
        fld st(1) // 007bb648
        fmulp st(2), st(0) // 007bb64a
        fld st(2) // 007bb64c
        fmulp st(3), st(0) // 007bb64e
        fxch st(1) // 007bb650
        faddp st(2),st(0) // 007bb652
        fmul st(0), st(0) // 007bb654
        faddp st(1), st(0) // 007bb656
        fstp dword ptr [esp + 0x8] // 007bb658
        fld dword ptr [ecx + 0x24] // 007bb65c
        fld dword ptr [ecx + 0x20] // 007bb65f
        fld dword ptr [ecx + 0x28] // 007bb662
        fld st(1) // 007bb665
        fmulp st(2), st(0) // 007bb667
        fld st(2) // 007bb669
        fmulp st(3), st(0) // 007bb66b
        fxch st(1) // 007bb66d
        faddp st(2),st(0) // 007bb66f
        fmul st(0), st(0) // 007bb671
        faddp st(1), st(0) // 007bb673
        fstp dword ptr [esp + 0xc] // 007bb675
        fld dword ptr [esp + 0x8] // 007bb679
        fld dword ptr [esp + 0x4] // 007bb67d
        fcomi st(0),st(1) // 007bb681
        jc l_007bb6a5 // 007bb683
        fstp st(1) // 007bb685
        fld dword ptr [esp + 0xc] // 007bb687
        fxch st(1) // 007bb68b
        fcomip st(0),st(1) // 007bb68d
        fstp st(0) // 007bb68f
        jc l_007bb6c5 // 007bb691
        movss xmm0,dword ptr [esp + 0x4] // 007bb693
        movss dword ptr [esp],xmm0 // 007bb699
        fld dword ptr [esp] // 007bb69e
        add esp,0x10 // 007bb6a1
        ret // 007bb6a4
    l_007bb6a5:
        fstp st(0) // 007bb6a5
        fld dword ptr [esp + 0xc] // 007bb6a7
        fxch st(1) // 007bb6ab
        fcomip st(0),st(1) // 007bb6ad
        fstp st(0) // 007bb6af
        jc l_007bb6c5 // 007bb6b1
        movss xmm0,dword ptr [esp + 0x8] // 007bb6b3
        movss dword ptr [esp],xmm0 // 007bb6b9
        fld dword ptr [esp] // 007bb6be
        add esp,0x10 // 007bb6c1
        ret // 007bb6c4
    l_007bb6c5:
        movss xmm0,dword ptr [esp + 0xc] // 007bb6c5
        movss dword ptr [esp],xmm0 // 007bb6cb
        fld dword ptr [esp] // 007bb6d0
        add esp,0x10 // 007bb6d3
        ret // 007bb6d6
    }
}

// Complete 00415550; each trailing address identifies the native instruction.
__declspec(naked) float __fastcall max_native_float_by_ref_00415550(const float*,const float*) {
    __asm {
        sub esp,0x8 // 00415550
        fld dword ptr [ecx] // 00415553
        fstp dword ptr [esp] // 00415555
        fld dword ptr [edx] // 00415558
        fstp dword ptr [esp + 0x4] // 0041555a
        fld dword ptr [esp + 0x4] // 0041555e
        fld dword ptr [esp] // 00415562
        fcomip st(0),st(1) // 00415565
        fstp st(0) // 00415567
        jbe l_0041557c // 00415569
        movss xmm0,dword ptr [esp] // 0041556b
        movss dword ptr [esp],xmm0 // 00415570
        fld dword ptr [esp] // 00415575
        add esp,0x8 // 00415578
        ret // 0041557b
    l_0041557c:
        movss xmm0,dword ptr [esp + 0x4] // 0041557c
        movss dword ptr [esp],xmm0 // 00415582
        fld dword ptr [esp] // 00415587
        add esp,0x8 // 0041558a
        ret // 0041558d
    }
}

// Complete 00b6d810; each trailing address identifies the native instruction.
__declspec(naked) void* __fastcall get_native_node_notification_context_00b6d810(const void*) {
    __asm {
        mov eax,dword ptr [ecx + 0xa0] // 00b6d810
        ret // 00b6d816
    }
}

// Complete 00b6dcb0; each trailing address identifies the native instruction.
__declspec(naked) float __fastcall get_native_node_bounds_scalar_00b6dcb0(const void*) {
    __asm {
        fld dword ptr [ecx + 0x50] // 00b6dcb0
        ret // 00b6dcb3
    }
}

// Complete 007c1180; each trailing address identifies the native instruction.
__declspec(naked) void* __fastcall transform_native_sphere_007c1180(const void*,const NativeTracelineRenderAccess*,void*,const void*) {
    __asm {
        push edx // borrowed access outside the original local frame
        sub esp,0xc // 007c1180
        push esi // 007c1183
        push edi // 007c1184
        mov edi,dword ptr [esp + 32] // 007c1185
        mov esi,ecx // 007c1189
        mov ecx,edi // 007c118b
        call maximum_native_basis_squared_007bb620 // 007c118d
        mov ecx,dword ptr [esp+20]
        mov ecx,dword ptr [ecx]
        call native_crt_sqrt_st0_00bf7030 // 007c1192
        fstp dword ptr [esp + 32] // 007c1197
        fld dword ptr [esp + 32] // 007c119b
        push edi // 007c119f
        fmul dword ptr [esi + 0xc] // 007c11a0
        lea eax,[esp + 12] // 007c11a3
        push eax // 007c11a7
        mov ecx,esi // 007c11a8
        fstp dword ptr [esp + 40] // 007c11aa
        call transform_native_point_004142e0 // 007c11ae
        mov ecx,eax // 007c11b3
        mov eax,dword ptr [esp + 28] // 007c11b5
        fld dword ptr [ecx] // 007c11b9
        fstp dword ptr [eax] // 007c11bb
        pop edi // 007c11bd
        fld dword ptr [ecx + 0x4] // 007c11be
        pop esi // 007c11c1
        fstp dword ptr [eax + 0x4] // 007c11c2
        fld dword ptr [ecx + 0x8] // 007c11c5
        fstp dword ptr [eax + 0x8] // 007c11c8
        fld dword ptr [esp + 24] // 007c11cb
        fstp dword ptr [eax + 0xc] // 007c11cf
        add esp,0xc // 007c11d2
        pop edx // discard the added access slot
        ret 0x8 // 007c11d5
    }
}

// Complete 00b6e8c0; each trailing address identifies the native instruction.
__declspec(naked) const void* __fastcall get_native_model_world_sphere_00b6e8c0(void*,const NativeTracelineRenderAccess*) {
    __asm {
        push edx // borrowed access outside the original local frame
        sub esp,0x10 // 00b6e8c0
        push esi // 00b6e8c3
        mov esi,ecx // 00b6e8c4
        test byte ptr [esi + 0x138],0x30 // 00b6e8c6
        jnz l_00b6e918 // 00b6e8cd
        test byte ptr [esi + 0x5c],0x2 // 00b6e8cf
        jnz l_00b6e8da // 00b6e8d3
        call refresh_native_camera_world_00b6db70 // 00b6e8d5
    l_00b6e8da:
        lea eax,[esi + 0xf0] // 00b6e8da
        push eax // 00b6e8e0
        lea ecx,[esp + 8] // 00b6e8e1
        push ecx // 00b6e8e5
        lea ecx,[esi + 0x8] // 00b6e8e6
        mov edx,dword ptr [esp+28]
        call transform_native_sphere_007c1180 // 00b6e8e9
        fld dword ptr [eax] // 00b6e8ee
        fstp dword ptr [esi + 0x13c] // 00b6e8f0
        fld dword ptr [eax + 0x4] // 00b6e8f6
        fstp dword ptr [esi + 0x140] // 00b6e8f9
        fld dword ptr [eax + 0x8] // 00b6e8ff
        fstp dword ptr [esi + 0x144] // 00b6e902
        fld dword ptr [eax + 0xc] // 00b6e908
        fstp dword ptr [esi + 0x148] // 00b6e90b
        or dword ptr [esi + 0x138],0x30 // 00b6e911
    l_00b6e918:
        lea eax,[esi + 0x13c] // 00b6e918
        pop esi // 00b6e91e
        add esp,0x10 // 00b6e91f
        pop edx // discard the added access slot
        ret // 00b6e922
    }
}

// Complete 00b651e0; each trailing address identifies the native instruction.
__declspec(naked) std::uint32_t __fastcall classify_native_plane_sphere_00b651e0(const void*,const NativeTracelineRenderAccess*,const void*,std::uint32_t) {
    __asm {
        push edx // borrowed access outside the original local frame
        sub esp,0x10 // 00b651e0
        push ebx // 00b651e3
        mov ebx,dword ptr [esp + 32] // 00b651e4
        push esi // 00b651e8
        mov esi,ecx // 00b651e9
        push edi // 00b651eb
        mov edi,dword ptr [esi + 0x140] // 00b651ec
        xor edx,edx // 00b651f2
        test edi,edi // 00b651f4
        jle l_00b652a7 // 00b651f6
        mov eax,dword ptr [esp + 36] // 00b651fc
        movss xmm0,dword ptr [eax + 0x4] // 00b65200
        push eax
        mov eax,dword ptr [esp+32]
        mov eax,dword ptr [eax+24]
        movss xmm1,dword ptr [eax] // 00b65205
        pop eax
        movss dword ptr [esp + 36],xmm0 // 00b6520d
        movss xmm0,dword ptr [eax] // 00b65213
        fld dword ptr [esp + 36] // 00b65217
        movss dword ptr [esp + 12],xmm0 // 00b6521b
        movss xmm0,dword ptr [eax + 0x8] // 00b65221
        fld dword ptr [esp + 12] // 00b65226
        movss dword ptr [esp + 16],xmm0 // 00b6522a
        movss xmm0,dword ptr [eax + 0xc] // 00b65230
        fld dword ptr [esp + 16] // 00b65235
        movss dword ptr [esp + 20],xmm0 // 00b65239
        fld dword ptr [esp + 20] // 00b6523f
        subss xmm1,xmm0 // 00b65243
        movss dword ptr [esp + 24],xmm1 // 00b65247
        fld dword ptr [esp + 24] // 00b6524d
        xor ecx,ecx // 00b65251
        add esi,0x8 // 00b65253
    l_00b65256:
        fld dword ptr [esi + -0x8] // 00b65256
        fmul st(0), st(4) // 00b65259
        fld dword ptr [esi + -0x4] // 00b6525b
        fmul st(0), st(6) // 00b6525e
        faddp st(1), st(0) // 00b65260
        fld dword ptr [esi] // 00b65262
        fmul st(0), st(4) // 00b65264
        faddp st(1), st(0) // 00b65266
        fstp dword ptr [esp + 40] // 00b65268
        fld dword ptr [esp + 40] // 00b6526c
        fsub dword ptr [esi + 0x4] // 00b65270
        fstp dword ptr [esp + 40] // 00b65273
        fld dword ptr [esp + 40] // 00b65277
        fxch st(1) // 00b6527b
        fcomi st(0),st(1) // 00b6527d
        ja l_00b652b2 // 00b6527f
        fxch st(1) // 00b65281
        fcomip st(0),st(2) // 00b65283
        jbe l_00b65290 // 00b65285
        mov eax,0x1 // 00b65287
        shl eax,cl // 00b6528c
        or ebx,eax // 00b6528e
    l_00b65290:
        add edx,0x1 // 00b65290
        add ecx,0x2 // 00b65293
        add esi,0x14 // 00b65296
        cmp edx,edi // 00b65299
        jl l_00b65256 // 00b6529b
        fstp st(4) // 00b6529d
        fstp st(2) // 00b6529f
        fstp st(0) // 00b652a1
        fstp st(1) // 00b652a3
        fstp st(0) // 00b652a5
    l_00b652a7:
        pop edi // 00b652a7
        pop esi // 00b652a8
        mov eax,ebx // 00b652a9
        pop ebx // 00b652ab
        add esp,0x10 // 00b652ac
        pop edx // discard the added access slot
        ret 0x8 // 00b652af
    l_00b652b2:
        fstp st(1) // 00b652b2
        pop edi // 00b652b4
        fstp st(4) // 00b652b5
        pop esi // 00b652b7
        fstp st(2) // 00b652b8
        mov eax,0xaaa // 00b652ba
        fstp st(0) // 00b652bf
        pop ebx // 00b652c1
        fstp st(1) // 00b652c2
        fstp st(0) // 00b652c4
        add esp,0x10 // 00b652c6
        pop edx // discard the added access slot
        ret 0x8 // 00b652c9
    }
}

// Complete 00b71530; each trailing address identifies the native instruction.
__declspec(naked) std::uint32_t __fastcall classify_native_camera_sphere_00b71530(void*,const NativeTracelineRenderAccess*,const void*) {
    __asm {
        push edx // borrowed access outside the original local frame
        sub esp,0x60 // 00b71530
        push esi // 00b71533
        mov esi,ecx // 00b71534
        mov eax,dword ptr [esi + 0x2f0] // 00b71536
        test al,0x4 // 00b7153c
        jnz l_00b7156a // 00b7153e
        or eax,0x4 // 00b71540
        mov dword ptr [esi + 0x2f0],eax // 00b71543
        call get_native_camera_view_projection_00b70490 // 00b71549
        push eax // 00b7154e
        lea ecx,[esp + 8] // 00b7154f
        mov edx,dword ptr [esp+104]
        mov edx,dword ptr [edx+4]
        call extract_native_camera_frustum_00b653f0 // 00b71553
        push 0x7 // 00b71558
        lea eax,[esp + 8] // 00b7155a
        push eax // 00b7155e
        lea ecx,[esi + 0x2f4] // 00b7155f
        call assign_native_camera_frustum_planes_00b658e0 // 00b71565
    l_00b7156a:
        mov ecx,dword ptr [esp + 108] // 00b7156a
        push 0x0 // 00b7156e
        push ecx // 00b71570
        lea ecx,[esi + 0x2f4] // 00b71571
        mov edx,dword ptr [esp+108]
        call classify_native_plane_sphere_00b651e0 // 00b71577
        pop esi // 00b7157c
        add esp,0x60 // 00b7157d
        pop edx // discard the added access slot
        ret 0x4 // 00b71580
    }
}

// Complete 00b51a20; each trailing address identifies the native instruction.
__declspec(naked) void __fastcall initialize_native_render_entry_00b51a20(void*,const NativeTracelineRenderAccess*,float,void*,void*,void*,void*,float,float,std::uint32_t) {
    __asm {
        push edx // borrowed access outside the original local frame
        movss xmm0,dword ptr [esp + 8] // 00b51a20
        mov eax,dword ptr [esp + 16] // 00b51a26
        mov edx,dword ptr [esp + 36] // 00b51a2a
        sub esp,0xc // 00b51a2e
        push ebx // 00b51a31
        mov ebx,dword ptr [esp + 28] // 00b51a32
        push esi // 00b51a36
        mov esi,ecx // 00b51a37
        mov ecx,dword ptr [esp + 44] // 00b51a39
        movss dword ptr [esi],xmm0 // 00b51a3d
        movss xmm0,dword ptr [esp + 48] // 00b51a41
        push edi // 00b51a47
        mov edi,dword ptr [esp + 44] // 00b51a48
        movss dword ptr [esi + 0x18],xmm0 // 00b51a4c
        movss xmm0,dword ptr [esp + 56] // 00b51a51
        push eax
        mov eax,dword ptr [esp+28]
        mov eax,dword ptr [eax+28]
        comiss xmm0,dword ptr [eax] // 00b51a57
        pop eax
        mov dword ptr [esi + 0x4],ebx // 00b51a5e
        mov dword ptr [esi + 0x8],eax // 00b51a61
        mov dword ptr [esi + 0xc],edi // 00b51a64
        mov dword ptr [esi + 0x10],ecx // 00b51a67
        mov dword ptr [esi + 0x1c],edx // 00b51a6a
        jbe l_00b51a7d // 00b51a6d
        pop edi // 00b51a6f
        movss dword ptr [esi + 0x14],xmm0 // 00b51a70
        pop esi // 00b51a75
        pop ebx // 00b51a76
        add esp,0xc // 00b51a77
        pop edx // discard the added access slot
        ret 0x20 // 00b51a7a
    l_00b51a7d:
        call get_native_camera_view_00b6fcb0 // 00b51a7d
        push eax // 00b51a84
        lea eax,[esp + 16] // 00b51a85
        push eax // 00b51a89
        mov ecx,edi // 00b51a8d
        mov edx,dword ptr [esp+32]
        call sphere_bridge // 00b51a8f
        mov ecx,eax // 00b51a91
        call transform_native_point_004142e0 // 00b51a93
        fld dword ptr [ebx + 0x34] // 00b51a98
        fadd dword ptr [esp + 20] // 00b51a9b
        pop edi // 00b51a9f
        fstp dword ptr [esi + 0x14] // 00b51aa0
        pop esi // 00b51aa3
        pop ebx // 00b51aa4
        add esp,0xc // 00b51aa5
        pop edx // discard the added access slot
        ret 0x20 // 00b51aa8
    }
}

// Complete 00b72f80; each trailing address identifies the native instruction.
__declspec(naked) void __fastcall render_native_mesh_entries_00b72f80(void*,const NativeTracelineRenderAccess*,void*,void*,float,float,std::uint32_t) {
    __asm {
        push edx // borrowed access outside the original local frame
        sub esp,0x18 // 00b72f80
        mov eax,dword ptr [esp + 32] // 00b72f83
        push ebx // 00b72f87
        push ebp // 00b72f88
        push esi // 00b72f89
        mov esi,dword ptr [eax + 0x8] // 00b72f8a
        mov eax,dword ptr [esp + 60] // 00b72f8d
        mov ebp,eax // 00b72f91
        and eax,0x555 // 00b72f93
        cmp eax,0x555 // 00b72f98
        push edi // 00b72f9d
        mov edi,ecx // 00b72f9e
        mov dword ptr [esp + 16],esi // 00b72fa0
        jz l_00b72fe1 // 00b72fa4
        cmp byte ptr [edi + 0x80],0x0 // 00b72fa6
        jz l_00b72fe1 // 00b72fad
        mov ebx,dword ptr [esp + 52] // 00b72faf
        test byte ptr [ebx + 0x5c],0x2 // 00b72fb3
        jnz l_00b72fc0 // 00b72fb7
        mov ecx,ebx // 00b72fb9
        call refresh_native_camera_world_00b6db70 // 00b72fbb
    l_00b72fc0:
        add ebx,0xf0 // 00b72fc0
        push ebx // 00b72fc6
        lea ecx,[esp + 28] // 00b72fc7
        push ecx // 00b72fcb
        lea ecx,[edi + 0x84] // 00b72fcc
        mov edx,dword ptr [esp+48]
        call transform_native_sphere_007c1180 // 00b72fd2
        push eax // 00b72fd7
        mov ecx,esi // 00b72fd8
        mov edx,dword ptr [esp+44]
        call classify_native_camera_sphere_00b71530 // 00b72fda
        mov ebp,eax // 00b72fdf
    l_00b72fe1:
        cmp ebp,0xaaa // 00b72fe1
        jz l_00b73251 // 00b72fe7
        cmp dword ptr [edi + 0x50],0x0 // 00b72fed
        jz l_00b7314f // 00b72ff1
        mov edx,dword ptr [edi + 0x50] // 00b72ff7
        lea ecx,[edi + 0x10] // 00b72ffa
        shl edx,0x4 // 00b72ffd
        add edx,ecx // 00b73000
        cmp ecx,edx // 00b73002
        jz l_00b73251 // 00b73004
        fld dword ptr [esp + 56] // 00b7300a
    l_00b7300e:
        fld dword ptr [ecx] // 00b7300e
        fxch st(1) // 00b73010
        fcomi st(0),st(1) // 00b73012
        fstp st(1) // 00b73014
        jc l_00b7301f // 00b73016
        fld dword ptr [ecx + 0x4] // 00b73018
        fcomip st(0),st(1) // 00b7301b
        jnc l_00b73032 // 00b7301d
    l_00b7301f:
        add ecx,0x10 // 00b7301f
        cmp ecx,edx // 00b73022
        jnz l_00b7300e // 00b73024
        pop edi // 00b73026
        fstp st(0) // 00b73027
        pop esi // 00b73029
        pop ebp // 00b7302a
        pop ebx // 00b7302b
        add esp,0x18 // 00b7302c
        pop edx // discard the added access slot
        ret 0x14 // 00b7302f
    l_00b73032:
        mov ebx,dword ptr [ecx + 0x8] // 00b73032
        fstp st(0) // 00b73035
        cmp ebx,dword ptr [ecx + 0xc] // 00b73037
        mov dword ptr [esp + 64],ecx // 00b7303a
        mov dword ptr [esp + 20],ebx // 00b7303e
        jg l_00b73251 // 00b73042
        jmp l_00b73050 // 00b73048
    l_00b73050:
        cmp ebx,dword ptr [edi + 0x58] // 00b73050
        jge l_00b73135 // 00b73053
        mov edx,dword ptr [edi + 0x54] // 00b73059
        mov eax,dword ptr [edx + ebx*0x4] // 00b7305c
        cmp dword ptr [eax + 0x18],0x0 // 00b7305f
        jz l_00b73135 // 00b73063
        push eax
        mov eax,dword ptr [esp+44]
        mov eax,dword ptr [eax+48]
        mov esi,dword ptr [eax] // 00b73069
        pop eax
        add esi,0x4 // 00b7306f
        lea ecx,[esi + 0x4] // 00b73072
        push ecx // 00b73075
        call increment_bridge // 00b73076
        fldz // 00b7307c
        mov ecx,dword ptr [esp + 16] // 00b7307e
        push ebp // 00b73082
        sub esp,0x8 // 00b73083
        fstp dword ptr [esp + 4] // 00b73086
        lea edx,[eax + eax*0x4] // 00b7308a
        mov eax,dword ptr [esi] // 00b7308d
        fld dword ptr [esp + 72] // 00b7308f
        fstp dword ptr [esp + 0] // 00b73093
        push ecx // 00b73096
        fld dword ptr [esp + 72] // 00b73097
        lea esi,[eax + edx*0x8 + -0x28] // 00b7309b
        mov edx,dword ptr [esp + 68] // 00b7309f
        mov eax,dword ptr [edi + 0x54] // 00b730a3
        mov ecx,dword ptr [eax + ebx*0x4] // 00b730a6
        push edx // 00b730a9
        push edi // 00b730aa
        push ecx // 00b730ab
        push ecx // 00b730ac
        mov ecx,esi // 00b730ad
        fstp dword ptr [esp + 0] // 00b730af
        mov edx,dword ptr [esp+72]
        call initialize_native_render_entry_00b51a20 // 00b730b2
        mov edx,dword ptr [esp + 48] // 00b730b7
        mov ecx,dword ptr [edx + 0x10] // 00b730bb
        push esi // 00b730be
        mov edx,dword ptr [esp+44]
        call collect_bridge // 00b730bf
        mov eax,dword ptr [edi + 0x54] // 00b730c4
        mov ecx,dword ptr [eax + ebx*0x4] // 00b730c7
        mov esi,dword ptr [ecx + 0x38] // 00b730ca
        test esi,esi // 00b730cd
        jz l_00b73131 // 00b730cf
    l_00b730d1:
        push eax
        mov eax,dword ptr [esp+44]
        mov eax,dword ptr [eax+48]
        mov ebx,dword ptr [eax] // 00b730d1
        pop eax
        add ebx,0x4 // 00b730d7
        lea edx,[ebx + 0x4] // 00b730da
        push edx // 00b730dd
        call increment_bridge // 00b730de
        fldz // 00b730e4
        mov ecx,dword ptr [ebx] // 00b730e6
        mov edx,dword ptr [esp + 16] // 00b730e8
        push ebp // 00b730ec
        sub esp,0x8 // 00b730ed
        fstp dword ptr [esp + 4] // 00b730f0
        lea eax,[eax + eax*0x4] // 00b730f4
        fld dword ptr [esp + 72] // 00b730f7
        lea ebx,[ecx + eax*0x8 + -0x28] // 00b730fb
        mov eax,dword ptr [esp + 64] // 00b730ff
        fstp dword ptr [esp + 0] // 00b73103
        fld dword ptr [esp + 68] // 00b73106
        push edx // 00b7310a
        push eax // 00b7310b
        push edi // 00b7310c
        push esi // 00b7310d
        push ecx // 00b7310e
        mov ecx,ebx // 00b7310f
        fstp dword ptr [esp + 0] // 00b73111
        mov edx,dword ptr [esp+72]
        call initialize_native_render_entry_00b51a20 // 00b73114
        mov ecx,dword ptr [esp + 48] // 00b73119
        mov ecx,dword ptr [ecx + 0x10] // 00b7311d
        push ebx // 00b73120
        mov edx,dword ptr [esp+44]
        call collect_bridge // 00b73121
        mov esi,dword ptr [esi + 0x38] // 00b73126
        test esi,esi // 00b73129
        jnz l_00b730d1 // 00b7312b
        mov ebx,dword ptr [esp + 20] // 00b7312d
    l_00b73131:
        mov ecx,dword ptr [esp + 64] // 00b73131
    l_00b73135:
        add ebx,0x1 // 00b73135
        cmp ebx,dword ptr [ecx + 0xc] // 00b73138
        mov dword ptr [esp + 20],ebx // 00b7313b
        jle l_00b73050 // 00b7313f
        pop edi // 00b73145
        pop esi // 00b73146
        pop ebp // 00b73147
        pop ebx // 00b73148
        add esp,0x18 // 00b73149
        pop edx // discard the added access slot
        ret 0x14 // 00b7314c
    l_00b7314f:
        mov ebx,dword ptr [edi + 0x54] // 00b7314f
        mov edx,dword ptr [edi + 0x58] // 00b73152
        mov eax,ebx // 00b73155
        lea eax,[eax + edx*0x4] // 00b73157
        cmp ebx,eax // 00b7315a
        mov dword ptr [esp + 64],ebx // 00b7315c
        mov dword ptr [esp + 20],eax // 00b73160
        jz l_00b73251 // 00b73164
        lea ebx,[ebx] // 00b7316a
    l_00b73170:
        mov ecx,dword ptr [ebx] // 00b73170
        cmp dword ptr [ecx + 0x18],0x0 // 00b73172
        jz l_00b73240 // 00b73176
        push eax
        mov eax,dword ptr [esp+44]
        mov eax,dword ptr [eax+48]
        mov esi,dword ptr [eax] // 00b7317c
        pop eax
        add esi,0x4 // 00b73182
        lea edx,[esi + 0x4] // 00b73185
        push edx // 00b73188
        call increment_bridge // 00b73189
        fldz // 00b7318f
        mov ecx,dword ptr [esi] // 00b73191
        mov edx,dword ptr [esp + 16] // 00b73193
        push ebp // 00b73197
        sub esp,0x8 // 00b73198
        fstp dword ptr [esp + 4] // 00b7319b
        lea eax,[eax + eax*0x4] // 00b7319f
        fld dword ptr [esp + 72] // 00b731a2
        lea esi,[ecx + eax*0x8 + -0x28] // 00b731a6
        mov eax,dword ptr [esp + 64] // 00b731aa
        fstp dword ptr [esp + 0] // 00b731ae
        mov ecx,dword ptr [ebx] // 00b731b1
        fld dword ptr [esp + 68] // 00b731b3
        push edx // 00b731b7
        push eax // 00b731b8
        push edi // 00b731b9
        push ecx // 00b731ba
        push ecx // 00b731bb
        mov ecx,esi // 00b731bc
        fstp dword ptr [esp + 0] // 00b731be
        mov edx,dword ptr [esp+72]
        call initialize_native_render_entry_00b51a20 // 00b731c1
        mov edx,dword ptr [esp + 48] // 00b731c6
        mov ecx,dword ptr [edx + 0x10] // 00b731ca
        push esi // 00b731cd
        mov edx,dword ptr [esp+44]
        call collect_bridge // 00b731ce
        mov eax,dword ptr [ebx] // 00b731d3
        mov esi,dword ptr [eax + 0x38] // 00b731d5
        test esi,esi // 00b731d8
        jz l_00b73240 // 00b731da
        lea esp,[esp + 0] // 00b731dc
    l_00b731e0:
        push eax
        mov eax,dword ptr [esp+44]
        mov eax,dword ptr [eax+48]
        mov ebx,dword ptr [eax] // 00b731e0
        pop eax
        add ebx,0x4 // 00b731e6
        lea ecx,[ebx + 0x4] // 00b731e9
        push ecx // 00b731ec
        call increment_bridge // 00b731ed
        fldz // 00b731f3
        mov ecx,dword ptr [esp + 16] // 00b731f5
        push ebp // 00b731f9
        sub esp,0x8 // 00b731fa
        fstp dword ptr [esp + 4] // 00b731fd
        lea edx,[eax + eax*0x4] // 00b73201
        mov eax,dword ptr [ebx] // 00b73204
        fld dword ptr [esp + 72] // 00b73206
        fstp dword ptr [esp + 0] // 00b7320a
        push ecx // 00b7320d
        fld dword ptr [esp + 72] // 00b7320e
        lea ebx,[eax + edx*0x8 + -0x28] // 00b73212
        mov edx,dword ptr [esp + 68] // 00b73216
        push edx // 00b7321a
        push edi // 00b7321b
        push esi // 00b7321c
        push ecx // 00b7321d
        mov ecx,ebx // 00b7321e
        fstp dword ptr [esp + 0] // 00b73220
        mov edx,dword ptr [esp+72]
        call initialize_native_render_entry_00b51a20 // 00b73223
        mov eax,dword ptr [esp + 48] // 00b73228
        mov ecx,dword ptr [eax + 0x10] // 00b7322c
        push ebx // 00b7322f
        mov edx,dword ptr [esp+44]
        call collect_bridge // 00b73230
        mov esi,dword ptr [esi + 0x38] // 00b73235
        test esi,esi // 00b73238
        jnz l_00b731e0 // 00b7323a
        mov ebx,dword ptr [esp + 64] // 00b7323c
    l_00b73240:
        add ebx,0x4 // 00b73240
        cmp ebx,dword ptr [esp + 20] // 00b73243
        mov dword ptr [esp + 64],ebx // 00b73247
        jnz l_00b73170 // 00b7324b
    l_00b73251:
        pop edi // 00b73251
        pop esi // 00b73252
        pop ebp // 00b73253
        pop ebx // 00b73254
        add esp,0x18 // 00b73255
        pop edx // discard the added access slot
        ret 0x14 // 00b73258
    }
}

// Complete 00b748e0; each trailing address identifies the native instruction.
__declspec(naked) void __fastcall render_native_generated_model_00b748e0(void*,const NativeTracelineRenderAccess*,void*,float,float,std::uint32_t) {
    __asm {
        push edx // borrowed access outside the original local frame
        sub esp,0x34 // 00b748e0
        mov eax,dword ptr [esp + 60] // 00b748e3
        push ebp // 00b748e7
        mov ebp,dword ptr [eax + 0x8] // 00b748e8
        mov eax,dword ptr [eax + 0xc] // 00b748eb
        push esi // 00b748ee
        mov esi,ecx // 00b748ef
        movss xmm0,dword ptr [esi + 0xac] // 00b748f1
        push eax
        mov eax,dword ptr [esp+64]
        mov eax,dword ptr [eax+28]
        comiss xmm0,dword ptr [eax] // 00b748f9
        pop eax
        mov dword ptr [esp + 12],eax // 00b74900
        jbe l_00b74b53 // 00b74904
        push ebx // 00b7490a
        mov ebx,0x2 // 00b7490b
        cmp dword ptr [ebp + 0x198],ebx // 00b74910
        jnz l_00b749c2 // 00b74916
        test byte ptr [ebp + 0x5c],bl // 00b7491c
        jnz l_00b74928 // 00b7491f
        mov ecx,ebp // 00b74921
        call refresh_native_camera_world_00b6db70 // 00b74923
    l_00b74928:
        test byte ptr [esi + 0x5c],bl // 00b74928
        movss xmm0,dword ptr [ebp + 0x120] // 00b7492b
        movss dword ptr [esp + 36],xmm0 // 00b74933
        movss xmm0,dword ptr [ebp + 0x124] // 00b74939
        movss dword ptr [esp + 40],xmm0 // 00b74941
        movss xmm0,dword ptr [ebp + 0x128] // 00b74947
        movss dword ptr [esp + 44],xmm0 // 00b7494f
        jnz l_00b7495e // 00b74955
        mov ecx,esi // 00b74957
        call refresh_native_camera_world_00b6db70 // 00b74959
    l_00b7495e:
        fld dword ptr [esi + 0x120] // 00b7495e
        lea ecx,[esp + 48] // 00b74964
        fstp dword ptr [esp + 24] // 00b74968
        fld dword ptr [esi + 0x124] // 00b7496c
        fstp dword ptr [esp + 28] // 00b74972
        fld dword ptr [esi + 0x128] // 00b74976
        fstp dword ptr [esp + 32] // 00b7497c
        fld dword ptr [esp + 24] // 00b74980
        fsub dword ptr [esp + 36] // 00b74984
        fstp dword ptr [esp + 48] // 00b74988
        fld dword ptr [esp + 28] // 00b7498c
        fsub dword ptr [esp + 40] // 00b74990
        fstp dword ptr [esp + 52] // 00b74994
        fld dword ptr [esp + 32] // 00b74998
        fsub dword ptr [esp + 44] // 00b7499c
        fstp dword ptr [esp + 56] // 00b749a0
        mov edx,dword ptr [esp+64]
        mov edx,dword ptr [edx]
        call camera_vector_length_00419440 // 00b749a4
        mov ecx,esi // 00b749a9
        fstp dword ptr [esp + 12] // 00b749ab
        call get_native_node_bounds_scalar_00b6dcb0 // 00b749af
        fld dword ptr [esp + 12] // 00b749b4
        fcomip st(0),st(1) // 00b749b8
        fstp st(0) // 00b749ba
        ja l_00b74b52 // 00b749bc
    l_00b749c2:
        mov ebx,dword ptr [esp + 84] // 00b749c2
        mov ecx,ebx // 00b749c6
        and ecx,0x555 // 00b749c8
        cmp ecx,0x555 // 00b749ce
        push edi // 00b749d4
        mov edi,ebx // 00b749d5
        jz l_00b749f5 // 00b749d7
        test byte ptr [esi + 0x138],0x3 // 00b749d9
        jz l_00b749f5 // 00b749e0
        mov ecx,esi // 00b749e7
        mov edx,dword ptr [esp+68]
        call sphere_bridge // 00b749e9
        push eax // 00b749eb
        mov ecx,ebp // 00b749ec
        mov edx,dword ptr [esp+72]
        call classify_native_camera_sphere_00b71530 // 00b749ee
        mov edi,eax // 00b749f3
    l_00b749f5:
        cmp edi,0xaaa // 00b749f5
        jz l_00b74b02 // 00b749fb
        movss xmm0,dword ptr [esp + 80] // 00b74a01
        mov ecx,esi // 00b74a07
        movss dword ptr [esp + 16],xmm0 // 00b74a09
        call get_native_node_notification_context_00b6d810 // 00b74a0f
        test eax,eax // 00b74a14
        jnz l_00b74ab6 // 00b74a16
        mov ebx,dword ptr [esp + 20] // 00b74a1c
        mov ecx,ebx // 00b74a20
        call get_native_camera_view_projection_00b70490 // 00b74a22
        push eax // 00b74a29
        lea ecx,[esp + 56] // 00b74a2d
        push ecx // 00b74a31
        mov ecx,esi // 00b74a32
        mov edx,dword ptr [esp+76]
        call sphere_bridge // 00b74a34
        mov ecx,eax // 00b74a36
        mov edx,dword ptr [esp+76]
        call transform_native_sphere_007c1180 // 00b74a38
        fld dword ptr [esp + 64] // 00b74a3d
        push eax
        mov eax,dword ptr [esp+72]
        mov eax,dword ptr [eax+32]
        movss xmm0,dword ptr [eax] // 00b74a41
        pop eax
        fstp qword ptr [esp + 28] // 00b74a49
        movss dword ptr [esp + 16],xmm0 // 00b74a4d
        xorps xmm0,xmm0 // 00b74a53
        movss dword ptr [esp + 24],xmm0 // 00b74a56
        push eax
        mov eax,dword ptr [esp+72]
        mov eax,dword ptr [eax+36]
        movss xmm0,dword ptr [eax] // 00b74a5c
        pop eax
        lea edx,[esp + 20] // 00b74a64
        lea ecx,[esp + 60] // 00b74a68
        movss dword ptr [esp + 20],xmm0 // 00b74a6c
        call max_native_float_by_ref_00415550 // 00b74a72
        fdivr qword ptr [esp + 28] // 00b74a77
        lea edx,[esp + 16] // 00b74a7b
        push edx // 00b74a7f
        lea edx,[esp + 28] // 00b74a80
        fstp dword ptr [esp + 24] // 00b74a84
        fld dword ptr [ebx + 0x4c] // 00b74a88
        mov ecx,dword ptr [esp + 24] // 00b74a8b
        and ecx,0x7fffffff // 00b74a8f
        mov dword ptr [esp + 24],ecx // 00b74a95
        fmul dword ptr [esp + 24] // 00b74a99
        fld1 // 00b74a9d
        lea ecx,[esp + 24] // 00b74a9f
        fsubrp st(1), st(0) // 00b74aa3
        fstp dword ptr [esp + 24] // 00b74aa5
        call clamp_native_float_004155b0 // 00b74aa9
        mov ebx,dword ptr [esp + 88] // 00b74aae
        fstp dword ptr [esp + 16] // 00b74ab2
    l_00b74ab6:
        fld dword ptr [esp + 16] // 00b74ab8
        push ecx // 00b74abf
        fstp dword ptr [esp + 0] // 00b74ac0
        push ebp // 00b74ac3
        mov ecx,esi // 00b74ac4
        mov edx,dword ptr [esp+76]
        call visibility_bridge // 00b74ac6
        test al,al // 00b74ac8
        jz l_00b74b02 // 00b74aca
        mov ecx,dword ptr [esi + 0x180] // 00b74acc
        test ecx,ecx // 00b74ad2
        jz l_00b74b02 // 00b74ad4
        fld dword ptr [esi + 0xac] // 00b74ad6
        mov eax,dword ptr [esp + 76] // 00b74adc
        fmul dword ptr [esp + 84] // 00b74ae0
        push edi // 00b74ae4
        sub esp,0x8 // 00b74ae5
        fstp dword ptr [esp + 100] // 00b74ae8
        fld dword ptr [esp + 100] // 00b74aec
        fstp dword ptr [esp + 4] // 00b74af0
        fld dword ptr [esp + 28] // 00b74af4
        fstp dword ptr [esp + 0] // 00b74af8
        push esi // 00b74afb
        push eax // 00b74afc
        mov edx,dword ptr [esp+88]
        call render_native_mesh_entries_00b72f80 // 00b74afd
    l_00b74b02:
        mov edi,dword ptr [esi + 0x34] // 00b74b02
        test edi,edi // 00b74b05
        jz l_00b74b51 // 00b74b07
        lea esp,[esp + 0] // 00b74b09
    l_00b74b10:
        mov ecx,dword ptr [edi + 0x48] // 00b74b10
        test dword ptr [ebp + 0x19c],ecx // 00b74b13
        jz l_00b74b4a // 00b74b19
        fld dword ptr [esi + 0xac] // 00b74b1b
        fmul dword ptr [esp + 84] // 00b74b23
        mov eax,dword ptr [esp + 76] // 00b74b27
        push ebx // 00b74b2e
        fstp dword ptr [esp + 92] // 00b74b2f
        sub esp,0x8 // 00b74b33
        fld dword ptr [esp + 100] // 00b74b36
        mov ecx,edi // 00b74b3a
        fstp dword ptr [esp + 4] // 00b74b3c
        fld dword ptr [esp + 92] // 00b74b40
        fstp dword ptr [esp + 0] // 00b74b44
        push eax // 00b74b47
        mov edx,dword ptr [esp+84]
        call render_child_bridge // 00b74b48
    l_00b74b4a:
        mov edi,dword ptr [edi + 0x3c] // 00b74b4a
        test edi,edi // 00b74b4d
        jnz l_00b74b10 // 00b74b4f
    l_00b74b51:
        pop edi // 00b74b51
    l_00b74b52:
        pop ebx // 00b74b52
    l_00b74b53:
        pop esi // 00b74b53
        pop ebp // 00b74b54
        add esp,0x34 // 00b74b55
        pop edx // discard the added access slot
        ret 0x10 // 00b74b58
    }
}

// Complete 00af26a0; each trailing address identifies the native instruction.
__declspec(naked) void __fastcall render_native_traceline_00af26a0(void*,const NativeTracelineRenderAccess*,void*,float,float,std::uint32_t) {
    __asm {
        push edx // borrowed access outside the original local frame
        sub esp,0x80 // 00af26a0
        mov eax,dword ptr [esp + 136] // 00af26a6
        push esi // 00af26ad
        mov esi,dword ptr [eax + 0x8] // 00af26ae
        cmp dword ptr [esi + 0x198],0x0 // 00af26b1
        push edi // 00af26b8
        mov edi,ecx // 00af26b9
        mov dword ptr [esp + 68],esi // 00af26bb
        jnz l_00af315e // 00af26bf
        mov ecx,dword ptr [esp+136] // 00af26c5: same current clock binding
        call clock_bridge // 00af26d0
        fild qword ptr [eax] // 00af26d2
        cmp dword ptr [edi + 0x190],0x2 // 00af26d4
        fild qword ptr [eax + 0x8] // 00af26db
        fdivp st(1), st(0) // 00af26de
        fstp dword ptr [edi + 0x198] // 00af26e0
        jl l_00af315e // 00af26e6
        mov ecx,edi // 00af26f1
        mov edx,dword ptr [esp+136]
        call sphere_bridge // 00af26f3
        push eax // 00af26f5
        mov ecx,esi // 00af26f6
        mov edx,dword ptr [esp+140]
        call classify_native_camera_sphere_00b71530 // 00af26f8
        cmp eax,0xaaa // 00af26fd
        jz l_00af315e // 00af2702
        push 0x0 // 00af2708
        mov ecx,edi // 00af270a
        call geometry_bridge // 00af270c
        push 0x0 // 00af2711
        mov ecx,eax // 00af2713
        mov dword ptr [esp + 96],eax // 00af2715
        call stream_bridge // 00af2719
        mov edx,dword ptr [edi + 0x190] // 00af271e
        lea ecx,[edx + edx*0x1] // 00af2724
        lea edx,[edx + edx*0x1 + -0x2] // 00af2727
        mov dword ptr [esp + 72],edx // 00af272b
        mov edx,dword ptr [edi + 0x184] // 00af272f
        cmp byte ptr [edx + 0x58],0x0 // 00af2735
        mov dword ptr [esp + 132],eax // 00af2739
        mov dword ptr [esp + 76],ecx // 00af2740
        jz l_00af275b // 00af2744
        cmp byte ptr [edi + 0x1a8],0x0 // 00af2746
        jnz l_00af275b // 00af274d
        add ecx,0xa // 00af274f
        add dword ptr [esp + 72],0xa // 00af2752
        mov dword ptr [esp + 76],ecx // 00af2757
    l_00af275b:
        push ebx // 00af275d
        push ebp // 00af275e
        push 0x0 // 00af275f
        push 0x0 // 00af2761
        push ecx // 00af2763
        mov ecx,eax // 00af2764
        mov edx,dword ptr [esp+156]
        call map_bridge // 00af2769
        mov ebp,dword ptr [edi + 0x18c] // 00af276b
        mov ebx,dword ptr [edi + 0x188] // 00af2771
        mov ecx,dword ptr [edi + 0x184] // 00af2777
        fld dword ptr [ecx + 0x24] // 00af277d
        mov esi,eax // 00af2780
        fdiv dword ptr [ecx + 0x2c] // 00af2782
        lea edx,[ebp + ebp*0x4] // 00af2785
        lea eax,[ebx + edx*0x4] // 00af2789
        mov dword ptr [esp + 24],eax // 00af278c
        lea eax,[ebp + 0x1] // 00af2790
        cdq // 00af2793
        idiv dword ptr [ecx + 0x20] // 00af2794
        push eax
        mov eax,dword ptr [esp+148]
        mov eax,dword ptr [eax+24]
        movss xmm7,dword ptr [eax] // 00af2797
        pop eax
        mov dword ptr [esp + 64],ebp // 00af279f
        mov dword ptr [esp + 60],0x0 // 00af27a3
        fstp dword ptr [esp + 20] // 00af27ab
        lea edx,[edx + edx*0x4] // 00af27af
        fld dword ptr [ebx + edx*0x4] // 00af27b2
        lea eax,[ebx + edx*0x4] // 00af27b5
        mov edx,dword ptr [esp + 24] // 00af27b8
        fsub dword ptr [edx] // 00af27bc
        fstp dword ptr [esp + 32] // 00af27be
        fld dword ptr [eax + 0x4] // 00af27c2
        fsub dword ptr [edx + 0x4] // 00af27c5
        fstp dword ptr [esp + 36] // 00af27c8
        fld dword ptr [eax + 0x8] // 00af27cc
        mov eax,dword ptr [edi + 0x190] // 00af27cf
        fsub dword ptr [edx + 0x8] // 00af27d5
        lea edx,[eax + -0x6] // 00af27d8
        mov dword ptr [esp + 68],edx // 00af27db
        movzx edx,byte ptr [ecx + 0x33] // 00af27df
        fstp dword ptr [esp + 40] // 00af27e3
        mov dword ptr [esp + 24],edx // 00af27e7
        mov dword ptr [esp + 28],eax // 00af27eb
        sub eax,dword ptr [esp + 68] // 00af27ef
        fild dword ptr [esp + 24] // 00af27f3
        mov edx,dword ptr [edi + 0x1b0] // 00af27f7
        sub eax,0x1 // 00af27fd
        cmp dword ptr [esp + 28],0x0 // 00af2800
        fmul dword ptr [edi + 0x19c] // 00af2805
        mov dword ptr [esp + 24],eax // 00af280b
        mov eax,dword ptr [ecx + 0x4c] // 00af280f
        mov edx,dword ptr [eax + edx*0x4] // 00af2812
        fdiv dword ptr [ecx + 0x28] // 00af2815
        movss xmm3,dword ptr [edx + 0x14] // 00af2818
        movss xmm4,dword ptr [edx + 0x1c] // 00af281d
        mov edx,dword ptr [edi + 0x1b4] // 00af2822
        mov eax,dword ptr [eax + edx*0x4] // 00af2828
        movss xmm5,dword ptr [eax + 0x14] // 00af282b
        movss xmm6,dword ptr [eax + 0x1c] // 00af2830
        mov eax,dword ptr [ecx + 0x30] // 00af2835
        mov dword ptr [esp + 72],eax // 00af2838
        fstp dword ptr [esp + 44] // 00af283c
        fild dword ptr [esp + 24] // 00af2840
        fld1 // 00af2844
        fld st(0) // 00af2846
        fdivrp st(2),st(0) // 00af2848
        fxch st(1) // 00af284a
        fstp dword ptr [esp + 24] // 00af284c
        fild dword ptr [esp + 28] // 00af2850
        fdivr st(0), st(1) // 00af2854
        fstp dword ptr [esp + 16] // 00af2856
        jle l_00af2a54 // 00af285a
        mov eax,dword ptr [esp + 68] // 00af2860
        fld dword ptr [esp + 20] // 00af2864
        fld dword ptr [esp + 44] // 00af2868
        neg eax // 00af286c
        fld dword ptr [esp + 24] // 00af286e
        mov dword ptr [esp + 28],eax // 00af2872
        fld dword ptr [esp + 16] // 00af2876
        lea ecx,[esi + 0x14] // 00af287a
    l_00af287d:
        lea eax,[ebp + 0x1] // 00af287d
        fild dword ptr [esp + 60] // 00af2880
        mov ebp,dword ptr [edi + 0x184] // 00af2884
        cdq // 00af288a
        idiv dword ptr [ebp + 0x20] // 00af288b
        fmul st(0), st(1) // 00af288e
        fsub st(0), st(5) // 00af2890
        fstp dword ptr [esp + 20] // 00af2892
        fld dword ptr [esp + 20] // 00af2896
        fmul st(0), st(0) // 00af289a
        fsubr st(0), st(5) // 00af289c
        fstp dword ptr [esp + 20] // 00af289e
        mov dword ptr [esp + 24],edx // 00af28a2
        mov edx,dword ptr [esp + 60] // 00af28a6
        cmp edx,dword ptr [esp + 68] // 00af28aa
        jle l_00af28c0 // 00af28ae
        fild dword ptr [esp + 28] // 00af28b0
        fmul st(0), st(2) // 00af28b4
        fsubr st(0), st(5) // 00af28b6
        fmul dword ptr [esp + 20] // 00af28b8
        fstp dword ptr [esp + 20] // 00af28bc
    l_00af28c0:
        mov ebp,dword ptr [esp + 64] // 00af28c0
        mov eax,dword ptr [edi + 0x184] // 00af28c4
        fld dword ptr [eax + 0x28] // 00af28ca
        lea ebp,[ebp + ebp*0x4] // 00af28cd
        add ebp,ebp // 00af28d1
        add ebp,ebp // 00af28d3
        fsub dword ptr [ebx + ebp*0x1 + 0x10] // 00af28d5
        fmul st(0), st(3) // 00af28d9
        fmul dword ptr [esp + 20] // 00af28db
        fnstcw word ptr [esp + 20] // 00af28df
        movzx eax,word ptr [esp + 20] // 00af28e3
        or eax,0xc00 // 00af28e8
        mov dword ptr [esp + 16],eax // 00af28ed
        fldcw word ptr [esp + 16] // 00af28f1
        fistp dword ptr [esp + 16] // 00af28f5
        mov al,byte ptr [esp + 16] // 00af28f9
        mov byte ptr [esp + 75],al // 00af28fd
        mov eax,dword ptr [edi + 0x18c] // 00af2901
        fldcw word ptr [esp + 20] // 00af2907
        add eax,edx // 00af290b
        mov dword ptr [esp + 16],eax // 00af290d
        fild dword ptr [esp + 16] // 00af2911
        fmul st(0), st(4) // 00af2915
        fstp dword ptr [esp + 60] // 00af2917
        fld dword ptr [ebx + ebp*0x1] // 00af291b
        lea eax,[ebx + ebp*0x1 + 0x4] // 00af291e
        fstp dword ptr [esi] // 00af2922
        mov dword ptr [esp + 16],eax // 00af2924
        fld dword ptr [eax] // 00af2928
        lea eax,[ebx + ebp*0x1 + 0x8] // 00af292a
        fstp dword ptr [ecx + -0x10] // 00af292e
        mov dword ptr [esp + 44],eax // 00af2931
        fld dword ptr [eax] // 00af2935
        fstp dword ptr [ecx + -0xc] // 00af2937
        movss xmm0,dword ptr [esp + 32] // 00af293a
        movss xmm1,dword ptr [esp + 36] // 00af2940
        movss xmm2,dword ptr [esp + 40] // 00af2946
        movss dword ptr [ecx + -0x8],xmm0 // 00af294c
        movss dword ptr [ecx + -0x4],xmm1 // 00af2951
        movss dword ptr [ecx],xmm2 // 00af2956
        mov eax,dword ptr [esp + 72] // 00af295a
        mov dword ptr [ecx + 0x4],eax // 00af295e
        movss dword ptr [ecx + 0x8],xmm3 // 00af2961
        fld dword ptr [esp + 60] // 00af2966
        fst dword ptr [ecx + 0xc] // 00af296a
        movss dword ptr [ecx + 0x10],xmm5 // 00af296d
        fstp dword ptr [ecx + 0x14] // 00af2972
        mov eax,dword ptr [edi + 0x188] // 00af2975
        fld dword ptr [eax + ebp*0x1 + 0xc] // 00af297b
        fstp dword ptr [ecx + 0x18] // 00af297f
        fld dword ptr [ebx + ebp*0x1] // 00af2982
        mov eax,dword ptr [esp + 16] // 00af2985
        fstp dword ptr [ecx + 0x1c] // 00af2989
        fld dword ptr [eax] // 00af298c
        mov eax,dword ptr [esp + 44] // 00af298e
        fstp dword ptr [ecx + 0x20] // 00af2992
        fld dword ptr [eax] // 00af2995
        fstp dword ptr [ecx + 0x24] // 00af2997
        movss dword ptr [ecx + 0x28],xmm0 // 00af299a
        movss dword ptr [ecx + 0x2c],xmm1 // 00af299f
        movss dword ptr [ecx + 0x30],xmm2 // 00af29a4
        mov eax,dword ptr [esp + 72] // 00af29a9
        mov dword ptr [ecx + 0x34],eax // 00af29ad
        movss dword ptr [ecx + 0x38],xmm4 // 00af29b0
        movss xmm0,dword ptr [esp + 60] // 00af29b5
        movss dword ptr [ecx + 0x3c],xmm0 // 00af29bb
        movss dword ptr [ecx + 0x40],xmm6 // 00af29c0
        movss dword ptr [ecx + 0x44],xmm0 // 00af29c5
        mov eax,dword ptr [edi + 0x188] // 00af29ca
        movaps xmm0,xmm7 // 00af29d0
        subss xmm0,dword ptr [eax + ebp*0x1 + 0xc] // 00af29d3
        movss dword ptr [ecx + 0x48],xmm0 // 00af29d9
        mov ebx,dword ptr [edi + 0x188] // 00af29de
        mov eax,dword ptr [esp + 24] // 00af29e4
        add dword ptr [esp + 28],0x1 // 00af29e8
        lea eax,[eax + eax*0x4] // 00af29ed
        fld dword ptr [ebx + eax*0x4] // 00af29f0
        lea eax,[ebx + eax*0x4] // 00af29f3
        fsub dword ptr [ebx + ebp*0x1] // 00af29f6
        add edx,0x1 // 00af29f9
        add esi,0x60 // 00af29fc
        add ecx,0x60 // 00af29ff
        cmp edx,dword ptr [edi + 0x190] // 00af2a02
        fstp dword ptr [esp + 48] // 00af2a08
        fld dword ptr [eax + 0x4] // 00af2a0c
        mov dword ptr [esp + 60],edx // 00af2a0f
        fsub dword ptr [ebx + ebp*0x1 + 0x4] // 00af2a13
        fstp dword ptr [esp + 52] // 00af2a17
        fld dword ptr [eax + 0x8] // 00af2a1b
        fsub dword ptr [ebx + ebp*0x1 + 0x8] // 00af2a1e
        mov ebp,dword ptr [esp + 24] // 00af2a22
        mov dword ptr [esp + 64],ebp // 00af2a26
        fstp dword ptr [esp + 56] // 00af2a2a
        fld dword ptr [esp + 48] // 00af2a2e
        fstp dword ptr [esp + 32] // 00af2a32
        fld dword ptr [esp + 52] // 00af2a36
        fstp dword ptr [esp + 36] // 00af2a3a
        fld dword ptr [esp + 56] // 00af2a3e
        fstp dword ptr [esp + 40] // 00af2a42
        jl l_00af287d // 00af2a46
        fstp st(4) // 00af2a4c
        fstp st(2) // 00af2a4e
        fstp st(0) // 00af2a50
        fstp st(1) // 00af2a52
    l_00af2a54:
        mov ecx,dword ptr [edi + 0x184] // 00af2a54
        fstp st(0) // 00af2a5a
        cmp byte ptr [ecx + 0x58],0x0 // 00af2a5c
        jz l_00af30fe // 00af2a60
        cmp byte ptr [edi + 0x1a8],0x0 // 00af2a66
        jnz l_00af30fe // 00af2a6d
        fld dword ptr [edi + 0x1a4] // 00af2a73
        or bl,0xff // 00af2a79
        fstp dword ptr [esp + 16] // 00af2a7c
        mov byte ptr [esp + 20],bl // 00af2a80
        fld dword ptr [ecx + 0x78] // 00af2a84
        fstp dword ptr [esp + 44] // 00af2a87
        fld dword ptr [esp + 44] // 00af2a8b
        fld dword ptr [esp + 16] // 00af2a8f
        fcomi st(0),st(1) // 00af2a93
        jbe l_00af2ad4 // 00af2a95
        fsubrp st(1), st(0) // 00af2a97
        fnstcw word ptr [esp + 20] // 00af2a99
        fstp dword ptr [esp + 16] // 00af2a9d
        movzx eax,word ptr [esp + 20] // 00af2aa1
        fld dword ptr [esp + 16] // 00af2aa6
        or eax,0xc00 // 00af2aaa
        fdiv dword ptr [ecx + 0x7c] // 00af2aaf
        mov dword ptr [esp + 16],eax // 00af2ab2
        push eax
        mov eax,dword ptr [esp+148]
        mov eax,dword ptr [eax+40]
        fmul qword ptr [eax] // 00af2ab6
        pop eax
        fldcw word ptr [esp + 16] // 00af2abc
        fistp dword ptr [esp + 16] // 00af2ac0
        mov cl,byte ptr [esp + 16] // 00af2ac4
        mov bl,cl // 00af2ac8
        fldcw word ptr [esp + 20] // 00af2aca
        mov byte ptr [esp + 20],cl // 00af2ace
        jmp l_00af2ad8 // 00af2ad2
    l_00af2ad4:
        fstp st(0) // 00af2ad4
        fstp st(0) // 00af2ad6
    l_00af2ad8:
        lea edx,[esi + -0x60] // 00af2ad8
        push edx // 00af2adb
        mov ecx,esi // 00af2adc
        call copy_native_traceline_vertex_00af1c20 // 00af2ade
        mov eax,dword ptr [edi + 0x184] // 00af2ae3
        mov ecx,dword ptr [eax + 0x64] // 00af2ae9
        mov dword ptr [esi + 0x18],ecx // 00af2aec
        mov byte ptr [esi + 0x1b],bl // 00af2aef
        mov edx,dword ptr [edi + 0x184] // 00af2af2
        mov eax,dword ptr [edx + 0x5c] // 00af2af8
        movss xmm0,dword ptr [eax + 0x14] // 00af2afb
        movss dword ptr [esi + 0x24],xmm0 // 00af2b00
        movss dword ptr [esi + 0x1c],xmm0 // 00af2b05
        mov ecx,dword ptr [edi + 0x184] // 00af2b0a
        mov edx,dword ptr [ecx + 0x5c] // 00af2b10
        movss xmm0,dword ptr [edx + 0x18] // 00af2b13
        lea eax,[esi + -0x30] // 00af2b18
        lea ecx,[esi + 0x30] // 00af2b1b
        push eax // 00af2b1e
        movss dword ptr [esi + 0x28],xmm0 // 00af2b1f
        movss dword ptr [esi + 0x20],xmm0 // 00af2b24
        call copy_native_traceline_vertex_00af1c20 // 00af2b29
        mov ecx,dword ptr [edi + 0x184] // 00af2b2e
        mov edx,dword ptr [ecx + 0x64] // 00af2b34
        mov dword ptr [esi + 0x48],edx // 00af2b37
        mov byte ptr [esi + 0x4b],bl // 00af2b3a
        mov eax,dword ptr [edi + 0x184] // 00af2b3d
        mov ecx,dword ptr [eax + 0x5c] // 00af2b43
        movss xmm0,dword ptr [ecx + 0x1c] // 00af2b46
        movss dword ptr [esi + 0x54],xmm0 // 00af2b4b
        movss dword ptr [esi + 0x4c],xmm0 // 00af2b50
        mov edx,dword ptr [edi + 0x184] // 00af2b55
        mov eax,dword ptr [edx + 0x5c] // 00af2b5b
        movss xmm0,dword ptr [eax + 0x18] // 00af2b5e
        movss dword ptr [esi + 0x58],xmm0 // 00af2b63
        movss dword ptr [esi + 0x50],xmm0 // 00af2b68
        mov ecx,dword ptr [edi + 0x184] // 00af2b6d
        fld dword ptr [ecx + 0x6c] // 00af2b73
        lea ebp,[esi + 0xc] // 00af2b76
        mov edx,ebp // 00af2b79
        fstp dword ptr [esp + 16] // 00af2b7b
        lea ecx,[esp + 88] // 00af2b7f
        push eax
        mov eax,dword ptr [esp+148]
        mov eax,dword ptr [eax]
        xchg eax,dword ptr [esp]
        call camera_vector_normalize_00419510 // 00af2b83
        fld dword ptr [esp + 16] // 00af2b88
        fld st(0) // 00af2b8c
        lea ebx,[esi + 0x60] // 00af2b8e
        fmul dword ptr [eax] // 00af2b91
        mov ecx,ebx // 00af2b93
        fstp dword ptr [esp + 32] // 00af2b95
        fld dword ptr [eax + 0x4] // 00af2b99
        fmul st(0), st(1) // 00af2b9c
        fstp dword ptr [esp + 36] // 00af2b9e
        fmul dword ptr [eax + 0x8] // 00af2ba2
        lea eax,[esi + 0x30] // 00af2ba5
        push eax // 00af2ba8
        fstp dword ptr [esp + 44] // 00af2ba9
        call copy_native_traceline_vertex_00af1c20 // 00af2bad
        fld dword ptr [ebx] // 00af2bb2
        push eax
        mov eax,dword ptr [esp+148]
        mov eax,dword ptr [eax+24]
        movss xmm2,dword ptr [eax] // 00af2bb4
        pop eax
        fsub dword ptr [esp + 32] // 00af2bbc
        movaps xmm0,xmm2 // 00af2bc0
        movaps xmm1,xmm2 // 00af2bc3
        fstp dword ptr [ebx] // 00af2bc6
        fld dword ptr [ebx + 0x4] // 00af2bc8
        fsub dword ptr [esp + 36] // 00af2bcb
        fstp dword ptr [ebx + 0x4] // 00af2bcf
        fld dword ptr [ebx + 0x8] // 00af2bd2
        fsub dword ptr [esp + 40] // 00af2bd5
        fstp dword ptr [ebx + 0x8] // 00af2bd9
        subss xmm0,dword ptr [ebp] // 00af2bdc
        subss xmm1,dword ptr [ebp + 0x4] // 00af2be1
        subss xmm2,dword ptr [ebp + 0x8] // 00af2be6
        movss dword ptr [esi + 0x6c],xmm0 // 00af2beb
        movss dword ptr [esi + 0x70],xmm1 // 00af2bf0
        movss dword ptr [esi + 0x74],xmm2 // 00af2bf5
        mov edx,dword ptr [edi + 0x184] // 00af2bfa
        mov eax,dword ptr [edx + 0x5c] // 00af2c00
        movss xmm0,dword ptr [eax + 0x14] // 00af2c03
        movss dword ptr [esi + 0x84],xmm0 // 00af2c08
        movss dword ptr [esi + 0x7c],xmm0 // 00af2c10
        mov ecx,dword ptr [edi + 0x184] // 00af2c15
        mov edx,dword ptr [ecx + 0x5c] // 00af2c1b
        movss xmm0,dword ptr [edx + 0x20] // 00af2c1e
        lea ebx,[esi + 0x90] // 00af2c23
        push esi // 00af2c29
        mov ecx,ebx // 00af2c2a
        movss dword ptr [esi + 0x88],xmm0 // 00af2c2c
        movss dword ptr [esi + 0x80],xmm0 // 00af2c34
        call copy_native_traceline_vertex_00af1c20 // 00af2c3c
        fld dword ptr [ebx] // 00af2c41
        fsub dword ptr [esp + 32] // 00af2c43
        push eax
        mov eax,dword ptr [esp+148]
        mov eax,dword ptr [eax+24]
        movss xmm0,dword ptr [eax] // 00af2c47
        pop eax
        movaps xmm1,xmm0 // 00af2c4f
        movaps xmm2,xmm0 // 00af2c52
        fstp dword ptr [ebx] // 00af2c55
        movaps xmm3,xmm0 // 00af2c57
        fld dword ptr [ebx + 0x4] // 00af2c5a
        fsub dword ptr [esp + 36] // 00af2c5d
        fstp dword ptr [ebx + 0x4] // 00af2c61
        fld dword ptr [ebx + 0x8] // 00af2c64
        fsub dword ptr [esp + 40] // 00af2c67
        fstp dword ptr [ebx + 0x8] // 00af2c6b
        subss xmm1,dword ptr [ebp] // 00af2c6e
        subss xmm2,dword ptr [ebp + 0x4] // 00af2c73
        subss xmm3,dword ptr [ebp + 0x8] // 00af2c78
        mov ebp,dword ptr [esp + 76] // 00af2c7d
        movss dword ptr [esi + 0x9c],xmm1 // 00af2c81
        movss dword ptr [esi + 0xa0],xmm2 // 00af2c89
        movss dword ptr [esi + 0xa4],xmm3 // 00af2c91
        mov eax,dword ptr [edi + 0x184] // 00af2c99
        mov ecx,dword ptr [eax + 0x5c] // 00af2c9f
        movss xmm1,dword ptr [ecx + 0x1c] // 00af2ca2
        movss dword ptr [esi + 0xb4],xmm1 // 00af2ca7
        movss dword ptr [esi + 0xac],xmm1 // 00af2caf
        mov edx,dword ptr [edi + 0x184] // 00af2cb7
        mov eax,dword ptr [edx + 0x5c] // 00af2cbd
        movss xmm1,dword ptr [eax + 0x20] // 00af2cc0
        movss dword ptr [esi + 0xb8],xmm1 // 00af2cc5
        movss dword ptr [esi + 0xb0],xmm1 // 00af2ccd
        test byte ptr [ebp + 0x5c],0x2 // 00af2cd5
        jnz l_00af2cea // 00af2cd9
        mov ecx,ebp // 00af2cdb
        call refresh_native_camera_world_00b6db70 // 00af2cdd
        push eax
        mov eax,dword ptr [esp+148]
        mov eax,dword ptr [eax+24]
        movss xmm0,dword ptr [eax] // 00af2ce2
        pop eax
    l_00af2cea:
        movss xmm2,dword ptr [ebp + 0xf0] // 00af2cea
        movaps xmm1,xmm0 // 00af2cf2
        subss xmm1,xmm2 // 00af2cf5
        movss dword ptr [esp + 48],xmm1 // 00af2cf9
        fld dword ptr [esp + 48] // 00af2cff
        movss dword ptr [esp + 64],xmm1 // 00af2d03
        fsub dword ptr [ebp + 0x100] // 00af2d09
        movss dword ptr [esp + 76],xmm2 // 00af2d0f
        movss xmm2,dword ptr [ebp + 0xf4] // 00af2d15
        movaps xmm1,xmm0 // 00af2d1d
        fstp dword ptr [esp + 128] // 00af2d20
        subss xmm1,xmm2 // 00af2d27
        movss dword ptr [esp + 52],xmm1 // 00af2d2b
        fld dword ptr [esp + 52] // 00af2d31
        movss dword ptr [esp + 24],xmm1 // 00af2d35
        fsub dword ptr [ebp + 0x104] // 00af2d3b
        movss xmm1,dword ptr [ebp + 0xf8] // 00af2d41
        subss xmm0,xmm1 // 00af2d49
        movss dword ptr [esp + 56],xmm0 // 00af2d4d
        fstp dword ptr [esp + 132] // 00af2d53
        movss dword ptr [esp + 16],xmm2 // 00af2d5a
        fld dword ptr [esp + 56] // 00af2d60
        movss dword ptr [esp + 44],xmm1 // 00af2d64
        fsub dword ptr [ebp + 0x108] // 00af2d6a
        movss dword ptr [esp + 28],xmm0 // 00af2d70
        push ebx // 00af2d76
        lea ecx,[esi + 0xc0] // 00af2d77
        fstp dword ptr [esp + 140] // 00af2d7d
        fld dword ptr [esp + 80] // 00af2d84
        fld st(0) // 00af2d88
        fsub dword ptr [ebp + 0x100] // 00af2d8a
        fstp dword ptr [esp + 108] // 00af2d90
        fld dword ptr [esp + 20] // 00af2d94
        fld st(0) // 00af2d98
        fsub dword ptr [ebp + 0x104] // 00af2d9a
        fstp dword ptr [esp + 112] // 00af2da0
        fld dword ptr [esp + 48] // 00af2da4
        fld st(0) // 00af2da8
        fsub dword ptr [ebp + 0x108] // 00af2daa
        fstp dword ptr [esp + 116] // 00af2db0
        fld dword ptr [esp + 68] // 00af2db4
        fstp dword ptr [esp + 52] // 00af2db8
        fld dword ptr [esp + 28] // 00af2dbc
        fstp dword ptr [esp + 56] // 00af2dc0
        fld dword ptr [esp + 32] // 00af2dc4
        fstp dword ptr [esp + 60] // 00af2dc8
        fld dword ptr [esp + 52] // 00af2dcc
        fadd dword ptr [ebp + 0x100] // 00af2dd0
        fstp dword ptr [esp + 120] // 00af2dd6
        fld dword ptr [ebp + 0x104] // 00af2dda
        fadd dword ptr [esp + 56] // 00af2de0
        fstp dword ptr [esp + 124] // 00af2de4
        fld dword ptr [ebp + 0x108] // 00af2de8
        fadd dword ptr [esp + 60] // 00af2dee
        fstp dword ptr [esp + 128] // 00af2df2
        fld dword ptr [ebp + 0x100] // 00af2df9
        faddp st(3),st(0) // 00af2dff
        fxch st(2) // 00af2e01
        fstp dword ptr [esp + 92] // 00af2e03
        fadd dword ptr [ebp + 0x104] // 00af2e07
        fstp dword ptr [esp + 96] // 00af2e0d
        fadd dword ptr [ebp + 0x108] // 00af2e11
        fstp dword ptr [esp + 100] // 00af2e17
        call copy_native_traceline_vertex_00af1c20 // 00af2e1b
        fld dword ptr [esi + 0x30] // 00af2e20
        fadd dword ptr [esi] // 00af2e23
        fstp dword ptr [esp + 48] // 00af2e25
        fld dword ptr [esi + 0x34] // 00af2e29
        fadd dword ptr [esi + 0x4] // 00af2e2c
        fstp dword ptr [esp + 52] // 00af2e2f
        fld dword ptr [esi + 0x38] // 00af2e33
        fadd dword ptr [esi + 0x8] // 00af2e36
        fstp dword ptr [esp + 56] // 00af2e39
        fld dword ptr [esp + 48] // 00af2e3d
        push eax
        mov eax,dword ptr [esp+148]
        mov eax,dword ptr [eax+44]
        fld qword ptr [eax] // 00af2e41
        pop eax
        fmul st(1), st(0) // 00af2e47
        fxch st(1) // 00af2e49
        fstp dword ptr [esp + 32] // 00af2e4b
        fld dword ptr [esp + 52] // 00af2e4f
        fmul st(0), st(1) // 00af2e53
        fstp dword ptr [esp + 36] // 00af2e55
        fmul dword ptr [esp + 56] // 00af2e59
        movss xmm0,dword ptr [esp + 128] // 00af2e5d
        mov bl,byte ptr [esp + 20] // 00af2e66
        movss dword ptr [esi + 0x12c],xmm0 // 00af2e6a
        movss xmm0,dword ptr [esp + 132] // 00af2e72
        fstp dword ptr [esp + 40] // 00af2e7b
        fld dword ptr [esp + 32] // 00af2e7f
        movss dword ptr [esi + 0x130],xmm0 // 00af2e83
        movss xmm0,dword ptr [esp + 136] // 00af2e8b
        lea eax,[esi + 0x120] // 00af2e94
        movss dword ptr [eax + 0x14],xmm0 // 00af2e9a
        mov ecx,dword ptr [edi + 0x184] // 00af2e9f
        mov edx,dword ptr [ecx + 0x68] // 00af2ea5
        mov dword ptr [eax + 0x18],edx // 00af2ea8
        mov byte ptr [eax + 0x1b],bl // 00af2eab
        fstp dword ptr [eax] // 00af2eae
        fld dword ptr [esp + 36] // 00af2eb0
        push eax // 00af2eb4
        fstp dword ptr [eax + 0x4] // 00af2eb5
        fld dword ptr [esp + 44] // 00af2eb8
        fstp dword ptr [eax + 0x8] // 00af2ebc
        mov ecx,dword ptr [edi + 0x184] // 00af2ebf
        fld dword ptr [ecx + 0x70] // 00af2ec5
        fstp dword ptr [eax + 0x2c] // 00af2ec8
        mov edx,dword ptr [edi + 0x184] // 00af2ecb
        mov ecx,dword ptr [edx + 0x60] // 00af2ed1
        movss xmm0,dword ptr [ecx + 0x14] // 00af2ed4
        movss dword ptr [eax + 0x24],xmm0 // 00af2ed9
        movss dword ptr [eax + 0x1c],xmm0 // 00af2ede
        mov edx,dword ptr [edi + 0x184] // 00af2ee3
        mov ecx,dword ptr [edx + 0x60] // 00af2ee9
        movss xmm0,dword ptr [ecx + 0x20] // 00af2eec
        lea ecx,[esi + 0xf0] // 00af2ef1
        movss dword ptr [eax + 0x28],xmm0 // 00af2ef7
        movss dword ptr [eax + 0x20],xmm0 // 00af2efc
        call copy_native_traceline_vertex_00af1c20 // 00af2f01
        movss xmm0,dword ptr [esp + 104] // 00af2f06
        movss dword ptr [esi + 0x15c],xmm0 // 00af2f0c
        movss xmm0,dword ptr [esp + 108] // 00af2f14
        movss dword ptr [esi + 0x160],xmm0 // 00af2f1a
        movss xmm0,dword ptr [esp + 112] // 00af2f22
        movss dword ptr [esi + 0x164],xmm0 // 00af2f28
        mov edx,dword ptr [edi + 0x184] // 00af2f30
        mov eax,dword ptr [edx + 0x68] // 00af2f36
        movss xmm0,dword ptr [esp + 32] // 00af2f39
        movss xmm1,dword ptr [esp + 36] // 00af2f3f
        movss xmm2,dword ptr [esp + 40] // 00af2f45
        mov dword ptr [esi + 0x168],eax // 00af2f4b
        mov byte ptr [esi + 0x16b],bl // 00af2f51
        movss dword ptr [esi + 0x150],xmm0 // 00af2f57
        movss dword ptr [esi + 0x154],xmm1 // 00af2f5f
        movss dword ptr [esi + 0x158],xmm2 // 00af2f67
        mov ecx,dword ptr [edi + 0x184] // 00af2f6f
        fld dword ptr [ecx + 0x70] // 00af2f75
        fstp dword ptr [esi + 0x17c] // 00af2f78
        mov edx,dword ptr [edi + 0x184] // 00af2f7e
        mov eax,dword ptr [edx + 0x60] // 00af2f84
        movss xmm3,dword ptr [eax + 0x1c] // 00af2f87
        movss dword ptr [esi + 0x174],xmm3 // 00af2f8c
        movss dword ptr [esi + 0x16c],xmm3 // 00af2f94
        mov ecx,dword ptr [edi + 0x184] // 00af2f9c
        mov edx,dword ptr [ecx + 0x60] // 00af2fa2
        movss xmm3,dword ptr [edx + 0x20] // 00af2fa5
        movss dword ptr [esi + 0x178],xmm3 // 00af2faa
        movss dword ptr [esi + 0x170],xmm3 // 00af2fb2
        movss xmm3,dword ptr [esp + 116] // 00af2fba
        movss dword ptr [esi + 0x18c],xmm3 // 00af2fc0
        movss xmm3,dword ptr [esp + 120] // 00af2fc8
        movss dword ptr [esi + 0x190],xmm3 // 00af2fce
        movss xmm3,dword ptr [esp + 124] // 00af2fd6
        movss dword ptr [esi + 0x194],xmm3 // 00af2fdc
        mov eax,dword ptr [edi + 0x184] // 00af2fe4
        mov ecx,dword ptr [eax + 0x68] // 00af2fea
        mov dword ptr [esi + 0x198],ecx // 00af2fed
        mov byte ptr [esi + 0x19b],bl // 00af2ff3
        movss dword ptr [esi + 0x180],xmm0 // 00af2ff9
        movss dword ptr [esi + 0x184],xmm1 // 00af3001
        movss dword ptr [esi + 0x188],xmm2 // 00af3009
        mov edx,dword ptr [edi + 0x184] // 00af3011
        fld dword ptr [edx + 0x70] // 00af3017
        fstp dword ptr [esi + 0x1ac] // 00af301a
        mov eax,dword ptr [edi + 0x184] // 00af3020
        mov ecx,dword ptr [eax + 0x60] // 00af3026
        movss xmm3,dword ptr [ecx + 0x14] // 00af3029
        movss dword ptr [esi + 0x1a4],xmm3 // 00af302e
        movss dword ptr [esi + 0x19c],xmm3 // 00af3036
        mov edx,dword ptr [edi + 0x184] // 00af303e
        mov eax,dword ptr [edx + 0x60] // 00af3044
        movss xmm3,dword ptr [eax + 0x18] // 00af3047
        movss dword ptr [esi + 0x1a8],xmm3 // 00af304c
        movss dword ptr [esi + 0x1a0],xmm3 // 00af3054
        movss xmm3,dword ptr [esp + 88] // 00af305c
        movss dword ptr [esi + 0x1bc],xmm3 // 00af3062
        movss xmm3,dword ptr [esp + 92] // 00af306a
        movss dword ptr [esi + 0x1c0],xmm3 // 00af3070
        movss xmm3,dword ptr [esp + 96] // 00af3078
        movss dword ptr [esi + 0x1c4],xmm3 // 00af307e
        mov ecx,dword ptr [edi + 0x184] // 00af3086
        mov edx,dword ptr [ecx + 0x68] // 00af308c
        mov dword ptr [esi + 0x1c8],edx // 00af308f
        mov byte ptr [esi + 0x1cb],bl // 00af3095
        movss dword ptr [esi + 0x1b0],xmm0 // 00af309b
        movss dword ptr [esi + 0x1b4],xmm1 // 00af30a3
        movss dword ptr [esi + 0x1b8],xmm2 // 00af30ab
        mov eax,dword ptr [edi + 0x184] // 00af30b3
        fld dword ptr [eax + 0x70] // 00af30b9
        fstp dword ptr [esi + 0x1dc] // 00af30bc
        mov ecx,dword ptr [edi + 0x184] // 00af30c2
        mov edx,dword ptr [ecx + 0x60] // 00af30c8
        movss xmm0,dword ptr [edx + 0x1c] // 00af30cb
        movss dword ptr [esi + 0x1d4],xmm0 // 00af30d0
        movss dword ptr [esi + 0x1cc],xmm0 // 00af30d8
        mov eax,dword ptr [edi + 0x184] // 00af30e0
        mov ecx,dword ptr [eax + 0x60] // 00af30e6
        movss xmm0,dword ptr [ecx + 0x18] // 00af30e9
        movss dword ptr [esi + 0x1d8],xmm0 // 00af30ee
        movss dword ptr [esi + 0x1d0],xmm0 // 00af30f6
    l_00af30fe:
        mov ecx,dword ptr [esp + 100] // 00af30fe
        push 0x0 // 00af3102
        call section_bridge // 00af3104
        mov edx,dword ptr [esp + 84] // 00af3109
        xor ecx,ecx // 00af310d
        mov dword ptr [eax + 0xc],ecx // 00af310f
        mov dword ptr [eax + 0x14],ecx // 00af3112
        mov ecx,dword ptr [esp + 80] // 00af3115
        mov dword ptr [eax + 0x18],ecx // 00af3119
        mov ecx,dword ptr [esp + 140] // 00af311c
        mov dword ptr [eax + 0x10],edx // 00af3123
        mov edx,dword ptr [esp+144]
        call unmap_bridge // 00af312b
        fld dword ptr [esp + 160] // 00af312d
        mov ecx,dword ptr [esp + 164] // 00af3134
        mov edx,dword ptr [esp + 152] // 00af313b
        push ecx // 00af3142
        sub esp,0x8 // 00af3143
        fstp dword ptr [esp + 4] // 00af3146
        mov ecx,edi // 00af314a
        fld dword ptr [esp + 168] // 00af314c
        fstp dword ptr [esp + 0] // 00af3153
        push edx // 00af3156
        mov edx,dword ptr [esp+160]
        call render_native_generated_model_00b748e0 // 00af3157
        pop ebp // 00af315c
        pop ebx // 00af315d
    l_00af315e:
        pop edi // 00af315e
        pop esi // 00af315f
        add esp,0x80 // 00af3160
        pop edx // discard the added access slot
        ret 0x10 // 00af3166
    }
}

} // namespace bsp
