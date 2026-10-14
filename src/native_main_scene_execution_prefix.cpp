#include "bsp/native_main_scene_execution_prefix.hpp"
#include "bsp/native_camera_history.hpp"
#include "bsp/native_camera_far_corners.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_system_constant_gather.hpp"
#include "bsp/native_render_resource_init_continuation.hpp"
#include "bsp/native_renderer_frame_targets.hpp"
#include "bsp/native_world_configuration_leaves.hpp"
#include "bsp/native_camera_axes.hpp"
#include "bsp/native_camera_configuration_leaves.hpp"
#include "bsp/native_camera_cache_getters.hpp"
#include "bsp/native_camera_general_inverse.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_render_state_leaves.hpp"
#include <cstddef>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native main-scene execution requires MSVC Win32 x87.
#endif
namespace bsp {
__declspec(naked) void* __fastcall get_native_scene_command_camera_00b1bf40(const void*) {
    __asm {
        mov eax, dword ptr [ecx + 028h]
        mov eax, dword ptr [eax + 8]
        ret
    }
}
__declspec(naked) void* __fastcall get_native_camera_clear_color_00b6fe60(const void*, void*, void*) {
    __asm {
        mov ecx, dword ptr [ecx + 0190h]
        mov eax, dword ptr [esp + 4]
        mov dword ptr [eax], ecx
        ret 4
    }
}
namespace {
struct ExecutionAccess {
    const NativeCameraAxesContext* axes;
    const NativeCameraHistoryContext* history;
    const volatile double* direction_scale;
    const volatile std::uint32_t* near_plane;
    void* const volatile* clock_publication;
    NativeMainSceneExecutionPrefixState* state;
};
static_assert(offsetof(ExecutionAccess, axes)==0);
static_assert(offsetof(ExecutionAccess, history)==4);
static_assert(offsetof(ExecutionAccess, direction_scale)==8);
static_assert(offsetof(ExecutionAccess, near_plane)==12);
static_assert(offsetof(ExecutionAccess, clock_publication)==16);
static_assert(offsetof(ExecutionAccess, state)==20);
static_assert(offsetof(NativeMainSceneExecutionPrefixState, near_bits)==0);
static_assert(offsetof(NativeMainSceneExecutionPrefixState, color_word)==4);
static_assert(offsetof(NativeMainSceneExecutionPrefixState, near_changed)==9);
static_assert(offsetof(NativeCameraAxesContext, length_access)==0);

// Native B156F6..B1586C. The caller handles the original current slot2C gate.
// Native scratch offsets and all x87 spills are retained. No EH state has yet
// been armed by this point; all reached raw/CRT callees must return normally.
__declspec(naked) void __fastcall execute_prefix_body(void*, const ExecutionAccess*, void*) {
    __asm {
        sub esp, 090h
        push ebx
        push ebp
        push esi
        push edi
        mov esi, ecx
        mov ebp, edx
        mov edi,dword ptr [esp + 0a4h] // 00b156f6
        mov ecx,edi // 00b156fd
        call get_native_scene_command_camera_00b1bf40
        push eax
        mov eax, dword ptr [ebp + 0ch]
        fld dword ptr [eax]
        pop eax
        movss xmm0,dword ptr [eax + 01d4h] // 00b1570a
        push ecx // 00b15712
        mov ecx,edi // 00b15713
        fstp dword ptr [esp] // 00b15715
        movss dword ptr [esp + 02ch], xmm0
        mov edx, dword ptr [ebp + 014h]
        movss dword ptr [edx], xmm0
        call get_native_scene_command_camera_00b1bf40
        mov ecx,eax // 00b15723
        lea edx, [esp]
        call set_native_camera_near_00b6fbf0
        add esp, 4
        mov edx, dword ptr [ebp + 014h]
        mov byte ptr [edx + 9], 1
        lea eax,[esp + 030h] // 00b1572a
        push eax // 00b1572e
        mov ecx,edi // 00b1572f
        call get_native_scene_command_camera_00b1bf40
        mov ecx,eax // 00b15736
        call get_native_camera_clear_color_00b6fe60
        mov bl,0ffh // 00b1573d
        mov ecx,edi // 00b1573f
        mov byte ptr [esp + 033h], bl
        mov edx, dword ptr [ebp + 014h]
        mov eax, dword ptr [esp + 030h]
        mov dword ptr [edx + 4], eax
        call get_native_scene_command_camera_00b1bf40
        mov ecx,eax // 00b1574a
        mov edx, dword ptr [ebp]
        call get_native_camera_axis_y_00b70ea0
        fld dword ptr [eax] // 00b15751
        fstp dword ptr [esp + 018h] // 00b15753
        mov ecx,edi // 00b15757
        fld dword ptr [eax + 04h] // 00b15759
        fstp dword ptr [esp + 01ch] // 00b1575c
        fld dword ptr [eax + 08h] // 00b15760
        fstp dword ptr [esp + 020h] // 00b15763
        call get_native_scene_command_camera_00b1bf40
        mov ecx,eax // 00b1576c
        mov edx, dword ptr [ebp]
        call get_native_camera_axis_x_00b70fe0
        movss xmm0,dword ptr [eax] // 00b15773
        movss dword ptr [esp + 048h],xmm0 // 00b15777
        movss xmm0,dword ptr [eax + 04h] // 00b1577d
        movss dword ptr [esp + 04ch],xmm0 // 00b15782
        movss xmm0,dword ptr [eax + 08h] // 00b15788
        lea edx,[esi + 019ch] // 00b1578d
        lea ecx,[esp + 034h] // 00b15793
        movss dword ptr [esp + 050h],xmm0 // 00b15797
        push dword ptr [ebp] // axes begins with the SAME CRT access
        call camera_vector_normalize_00419510
        fld dword ptr [esp + 034h] // 00b157a2
        fld st(0) // 00b157a6
        fmul dword ptr [esp + 018h] // 00b157a8
        fld dword ptr [esp + 038h] // 00b157ac
        fld st(0) // 00b157b0
        fmul dword ptr [esp + 01ch] // 00b157b2
        faddp st(2),st(0) // 00b157b6
        fld dword ptr [esp + 03ch] // 00b157b8
        fld st(0) // 00b157bc
        fmul dword ptr [esp + 020h] // 00b157be
        faddp st(3),st(0) // 00b157c2
        fxch st(2) // 00b157c4
        fstp dword ptr [esp + 014h] // 00b157c6
        fld dword ptr [esp + 048h] // 00b157ca
        fmulp st(3), st(0) // 00b157ce
        fmul dword ptr [esp + 04ch] // 00b157d0
        faddp st(2),st(0) // 00b157d4
        fmul dword ptr [esp + 050h] // 00b157d6
        faddp st(1), st(0) // 00b157da
        fstp dword ptr [esp + 018h] // 00b157dc
        fld dword ptr [esp + 018h] // 00b157e0
        push eax
        mov eax, dword ptr [ebp + 8]
        fld qword ptr [eax]
        pop eax
        fmul st(1), st(0) // 00b157ea
        fxch // 00b157ec
        fstp dword ptr [esp + 010h] // 00b157ee
        fmul dword ptr [esp + 014h] // 00b157f2
        fstp dword ptr [esp + 014h] // 00b157f6
        fld dword ptr [esp + 010h] // 00b157fa
        fstp dword ptr [esi + 01a8h] // 00b157fe
        fld dword ptr [esp + 014h] // 00b15804
        fstp dword ptr [esi + 01ach] // 00b15808
        mov ecx, dword ptr [ebp + 010h]
        mov ecx, dword ptr [ecx]
        fld dword ptr [ecx + 04h] // 00b15814
        mov ecx,edi // 00b15817
        fstp dword ptr [esp + 014h] // 00b15819
        call get_native_scene_command_camera_00b1bf40
        fld dword ptr [esp + 014h] // 00b15822
        push eax // 00b15826
        push ecx // 00b15827
        lea ecx,[esi + 0314h] // 00b15828
        fstp dword ptr [esp] // 00b1582e
        mov edx, dword ptr [ebp + 4]
        call update_native_camera_history_00b0db00
        lea edx,[esi + 055ch] // 00b15836
        push edx // 00b1583c
        lea ecx,[esi + 05a8h] // 00b1583d
        call copy_native_camera_matrix_004134f0
        mov ecx,edi // 00b15848
        call get_native_scene_command_camera_00b1bf40
        mov ecx,eax // 00b1584f
        call get_native_camera_view_projection_00b70490
        mov edx,eax // 00b15856
        lea ecx,[esp + 054h] // 00b15858
        call invert_native_camera_matrix_00b632d0
        push eax // 00b15861
        lea ecx,[esi + 05e8h] // 00b15862
        call copy_native_camera_matrix_004134f0
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 090h
        ret 4
    }
}
std::uint32_t word(const void* p) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, p
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}
} // namespace

bool execute_native_main_scene_prefix_00b156c0(void* service, void* command,
    NativeMainSceneExecutionPrefixContext& c, NativeMainSceneExecutionPrefixState& state) {
    void* const renderer=c.actual_renderer_00f8d394;
    if(word(renderer)!=0x00d5f0a8 || !c.actual_renderer_profile_00d5f0a8 ||
        c.actual_renderer_profile_00d5f0a8[0x2c/4]!=0x00b1fe20)
        throw std::logic_error("main scene requires current D5F0A8:B1FE20 active-frame slot");
    if(!native_render_is_frame_active_00b1fe20(renderer)) return false;
    state.frame_active=true;
    if(c.history.crt!=&c.axes.length_access ||
       c.history.one_00d7a24c!=c.axes.one_00d7a24c ||
       &c.near_plane_00ce3800!=c.axes.fallback_limit_00ce3800)
        throw std::logic_error("main scene axes/history require the same actual CRT and constant cells");
    const ExecutionAccess access{&c.axes,&c.history,&c.direction_scale_00cec9e0,
        &c.near_plane_00ce3800,&c.actual_clock_01090ab0,&state};
    execute_prefix_body(service,&access,command);
    state.reached_texture_stage=true;
    return true;
}

namespace {
using TextureContext=NativeMainSceneTextureConstantsContext;
static_assert(offsetof(TextureContext, actual_texture_profile_00d61948)==0);
static_assert(offsetof(TextureContext, unsigned_bias_00ce3978)==4);
static_assert(offsetof(TextureContext, half_00d7a280)==8);
static_assert(offsetof(TextureContext, negative_one_00d7a260)==12);
static_assert(offsetof(TextureContext, one_00d7a24c)==16);
static_assert(offsetof(TextureContext, far_corners)==20);
std::uint32_t __fastcall texture_width(const void* actual, const TextureContext* c,
    std::uint32_t captured_profile) {
    if(captured_profile!=0x00d61948 || !c->actual_texture_profile_00d61948 ||
        c->actual_texture_profile_00d61948[15]!=0x00b3ce50)
        throw std::logic_error("main scene requires current D61948:B3CE50 width");
    return get_raw_texture_width_00b3ce50(actual);
}
std::uint32_t __fastcall texture_height(const void* actual, const TextureContext* c,
    std::uint32_t captured_profile) {
    if(captured_profile!=0x00d61948 || !c->actual_texture_profile_00d61948 ||
        c->actual_texture_profile_00d61948[16]!=0x00b3ce60)
        throw std::logic_error("main scene requires current D61948:B3CE60 height");
    return get_raw_texture_height_00b3ce60(actual);
}
// Native B1586D..B15BD1. Added context is above the native scratch frame;
// EBP continues capturing the first texture for the later height dispatch.
__declspec(naked) void __fastcall execute_texture_constants(void*, const TextureContext*, void*) {
    __asm {
        push edx
        sub esp, 090h
        push ebx
        push ebp
        push esi
        push edi
        mov esi, ecx
        mov edi, dword ptr [esp + 0a8h]
        mov ecx,dword ptr [esi + 04ch] // 00b1586d
        call native_shadow_holder_texture_00b4cb10
        mov ecx,dword ptr [esi + 04ch] // 00b15875
        mov ebp,eax // 00b15878
        call native_shadow_holder_texture_00b4cb10
        mov edx,dword ptr [eax] // 00b1587f
        mov ecx,eax // 00b15881
        mov eax, edx // preserve the original captured owner profile
        push eax
        mov edx, dword ptr [esp + 0a4h]
        call texture_width
        test eax,eax // 00b15888
        mov dword ptr [esp + 02ch],eax // 00b1588a
        fild dword ptr [esp + 02ch] // 00b1588e
        jge loc_00b1589a // 00b15892
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax + 4]
        fadd dword ptr [eax]
        pop eax
    loc_00b1589a:
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax + 8]
        fdivr qword ptr [eax]
        pop eax
        mov edx,dword ptr [ebp] // 00b158a0
        mov eax, edx // preserve the original captured owner profile
        mov ecx,ebp // 00b158a6
        fstp dword ptr [esp + 014h] // 00b158a8
        push eax
        mov edx, dword ptr [esp + 0a4h]
        call texture_height
        test eax,eax // 00b158ae
        mov dword ptr [esp + 02ch],eax // 00b158b0
        fild dword ptr [esp + 02ch] // 00b158b4
        jge loc_00b158c0 // 00b158b8
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax + 4]
        fadd dword ptr [eax]
        pop eax
    loc_00b158c0:
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax + 8]
        fdivr qword ptr [eax]
        pop eax
        fstp dword ptr [esp + 010h] // 00b158c6
        fld dword ptr [esp + 014h] // 00b158ca
        fstp dword ptr [esi + 04h] // 00b158ce
        fld dword ptr [esp + 010h] // 00b158d1
        fstp dword ptr [esi + 08h] // 00b158d5
        mov ecx,dword ptr [esi + 04ch] // 00b158d8
        call native_shadow_holder_texture_00b4cb10
        mov ecx,dword ptr [esi + 04ch] // 00b158e0
        mov ebp,eax // 00b158e3
        call native_shadow_holder_texture_00b4cb10
        mov edx,dword ptr [eax] // 00b158ea
        mov ecx,eax // 00b158ec
        mov eax, edx // preserve the original captured owner profile
        push eax
        mov edx, dword ptr [esp + 0a4h]
        call texture_width
        test eax,eax // 00b158f3
        mov dword ptr [esp + 02ch],eax // 00b158f5
        fild dword ptr [esp + 02ch] // 00b158f9
        jge loc_00b15905 // 00b158fd
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax + 4]
        fadd dword ptr [eax]
        pop eax
    loc_00b15905:
        fld1 // 00b15905
        mov edx,dword ptr [ebp] // 00b15907
        fdivrp st(1), st(0) // 00b1590a
        mov eax, edx // preserve the original captured owner profile
        mov ecx,ebp // 00b1590f
        fstp dword ptr [esp + 014h] // 00b15911
        push eax
        mov edx, dword ptr [esp + 0a4h]
        call texture_height
        test eax,eax // 00b15917
        mov dword ptr [esp + 02ch],eax // 00b15919
        fild dword ptr [esp + 02ch] // 00b1591d
        jge loc_00b15929 // 00b15921
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax + 4]
        fadd dword ptr [eax]
        pop eax
    loc_00b15929:
        fld1 // 00b15929
        xorps xmm0,xmm0 // 00b1592b
        fdivrp st(1), st(0) // 00b1592e
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax + 12]
        movss xmm1,dword ptr [eax]
        pop eax
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax + 16]
        movss xmm2,dword ptr [eax]
        pop eax
        fstp dword ptr [esp + 010h] // 00b15940
        fld dword ptr [esp + 014h] // 00b15944
        fstp dword ptr [esi + 0184h] // 00b15948
        fld dword ptr [esp + 010h] // 00b1594e
        fstp dword ptr [esi + 0188h] // 00b15952
        movss dword ptr [esi + 0f4h],xmm1 // 00b15958
        movss dword ptr [esi + 0f8h],xmm1 // 00b15960
        movss dword ptr [esi + 0fch],xmm0 // 00b15968
        movss dword ptr [esi + 0100h],xmm0 // 00b15970
        movss dword ptr [esi + 0104h],xmm0 // 00b15978
        movss dword ptr [esi + 0108h],xmm1 // 00b15980
        movss dword ptr [esi + 010ch],xmm0 // 00b15988
        movss dword ptr [esi + 0110h],xmm0 // 00b15990
        movss dword ptr [esi + 0114h],xmm2 // 00b15998
        movss dword ptr [esi + 0118h],xmm1 // 00b159a0
        movss dword ptr [esi + 011ch],xmm0 // 00b159a8
        movss dword ptr [esi + 0120h],xmm0 // 00b159b0
        movss dword ptr [esi + 0124h],xmm1 // 00b159b8
        movss dword ptr [esi + 0128h],xmm0 // 00b159c0
        movss dword ptr [esi + 012ch],xmm0 // 00b159c8
        movss dword ptr [esi + 0130h],xmm0 // 00b159d0
        movss dword ptr [esi + 0134h],xmm0 // 00b159d8
        movss dword ptr [esi + 0138h],xmm0 // 00b159e0
        movss dword ptr [esi + 013ch],xmm0 // 00b159e8
        movss dword ptr [esi + 0140h],xmm0 // 00b159f0
        movss dword ptr [esi + 0144h],xmm2 // 00b159f8
        movss dword ptr [esi + 0148h],xmm0 // 00b15a00
        movss dword ptr [esi + 014ch],xmm0 // 00b15a08
        movss dword ptr [esi + 0150h],xmm0 // 00b15a10
        movss dword ptr [esi + 0154h],xmm1 // 00b15a18
        movss dword ptr [esi + 0158h],xmm2 // 00b15a20
        movss dword ptr [esi + 015ch],xmm0 // 00b15a28
        movss dword ptr [esi + 0160h],xmm0 // 00b15a30
        movss dword ptr [esi + 0164h],xmm0 // 00b15a38
        movss dword ptr [esi + 0168h],xmm2 // 00b15a40
        movss dword ptr [esi + 016ch],xmm0 // 00b15a48
        movss dword ptr [esi + 0170h],xmm0 // 00b15a50
        movss dword ptr [esi + 0174h],xmm2 // 00b15a58
        movss dword ptr [esi + 0178h],xmm2 // 00b15a60
        movss dword ptr [esi + 017ch],xmm0 // 00b15a68
        movss dword ptr [esi + 0180h],xmm0 // 00b15a70
        fld dword ptr [esi + 0f4h] // 00b15a78
        fmul dword ptr [esi + 0184h] // 00b15a7e
        fstp dword ptr [esi + 0f4h] // 00b15a84
        fld dword ptr [esi + 0f8h] // 00b15a8a
        fmul dword ptr [esi + 0188h] // 00b15a90
        fstp dword ptr [esi + 0f8h] // 00b15a96
        fld dword ptr [esi + 0104h] // 00b15a9c
        fmul dword ptr [esi + 0184h] // 00b15aa2
        fstp dword ptr [esi + 0104h] // 00b15aa8
        fld dword ptr [esi + 0188h] // 00b15aae
        fmul dword ptr [esi + 0108h] // 00b15ab4
        fstp dword ptr [esi + 0108h] // 00b15aba
        fld dword ptr [esi + 0114h] // 00b15ac0
        fmul dword ptr [esi + 0184h] // 00b15ac6
        fstp dword ptr [esi + 0114h] // 00b15acc
        fld dword ptr [esi + 0118h] // 00b15ad2
        fmul dword ptr [esi + 0188h] // 00b15ad8
        fstp dword ptr [esi + 0118h] // 00b15ade
        fld dword ptr [esi + 0124h] // 00b15ae4
        fmul dword ptr [esi + 0184h] // 00b15aea
        fstp dword ptr [esi + 0124h] // 00b15af0
        fld dword ptr [esi + 0128h] // 00b15af6
        fmul dword ptr [esi + 0188h] // 00b15afc
        fstp dword ptr [esi + 0128h] // 00b15b02
        fld dword ptr [esi + 0134h] // 00b15b08
        fmul dword ptr [esi + 0184h] // 00b15b0e
        fstp dword ptr [esi + 0134h] // 00b15b14
        fld dword ptr [esi + 0138h] // 00b15b1a
        fmul dword ptr [esi + 0188h] // 00b15b20
        fstp dword ptr [esi + 0138h] // 00b15b26
        fld dword ptr [esi + 0144h] // 00b15b2c
        fmul dword ptr [esi + 0184h] // 00b15b32
        fstp dword ptr [esi + 0144h] // 00b15b38
        fld dword ptr [esi + 0188h] // 00b15b3e
        fmul dword ptr [esi + 0148h] // 00b15b44
        fstp dword ptr [esi + 0148h] // 00b15b4a
        lea ecx,[esi + 0b4h] // 00b15b50
        fld dword ptr [esi + 0154h] // 00b15b56
        push ecx // 00b15b5c
        fmul dword ptr [esi + 0184h] // 00b15b5d
        mov ecx,edi // 00b15b63
        fstp dword ptr [esi + 0154h] // 00b15b65
        fld dword ptr [esi + 0158h] // 00b15b6b
        fmul dword ptr [esi + 0188h] // 00b15b71
        fstp dword ptr [esi + 0158h] // 00b15b77
        fld dword ptr [esi + 0164h] // 00b15b7d
        fmul dword ptr [esi + 0184h] // 00b15b83
        fstp dword ptr [esi + 0164h] // 00b15b89
        fld dword ptr [esi + 0168h] // 00b15b8f
        fmul dword ptr [esi + 0188h] // 00b15b95
        fstp dword ptr [esi + 0168h] // 00b15b9b
        fld dword ptr [esi + 0174h] // 00b15ba1
        fmul dword ptr [esi + 0184h] // 00b15ba7
        fstp dword ptr [esi + 0174h] // 00b15bad
        fld dword ptr [esi + 0178h] // 00b15bb3
        fmul dword ptr [esi + 0188h] // 00b15bb9
        fstp dword ptr [esi + 0178h] // 00b15bbf
        call get_native_scene_command_camera_00b1bf40
        push eax // 00b15bca
        mov ecx,esi // 00b15bcb
        mov ecx, dword ptr [esp + 0a8h]
        mov ecx, dword ptr [ecx + 20]
        call compute_native_camera_far_corners_00b0eb10
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 094h
        ret 4
    }
}
} // namespace

void continue_native_main_scene_texture_constants_00b1586d(void* service,
    void* command, const TextureContext& c, NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_texture_stage || state.reached_bloom_stage)
        throw std::logic_error("main scene texture continuation requires its fresh reached prefix");
    if(!c.far_corners || c.far_corners->one_00d7a24c!=c.one_00d7a24c ||
       c.far_corners->negative_one_00d7a260!=c.negative_one_00d7a260 ||
       c.far_corners->half_00d7a280!=c.half_00d7a280)
        throw std::logic_error("main scene constants require the same actual far-corner cells");
    execute_texture_constants(service,&c,command);
    state.reached_bloom_stage=true;
}

namespace {
struct BloomScalarAccess {
    const TextureContext* textures;
    const volatile double* adaptation_scale;
    const volatile std::uint32_t* negative_half;
    const volatile std::uint32_t* half_float;
};
static_assert(offsetof(BloomScalarAccess, textures)==0);
static_assert(offsetof(BloomScalarAccess, adaptation_scale)==4);
static_assert(offsetof(BloomScalarAccess, negative_half)==8);
static_assert(offsetof(BloomScalarAccess, half_float)==12);
void* __fastcall bloom_output_holder_actual(const void* actual) noexcept {
    return native_bloom_output_holder_00b54cd0(actual);
}
// Native B15BD2..B15DDB: complete active/inactive bloom arithmetic and shared
// screen constants. Native scratch slots/spills and opcode directions retained.
__declspec(naked) void __fastcall execute_bloom_constants(void*, const BloomScalarAccess*, void*) {
    __asm {
        push edx
        sub esp, 090h
        push ebx
        push ebp
        push esi
        push edi
        mov esi, ecx
        mov edi, dword ptr [esp + 0a8h]
        cmp byte ptr [esi + 021ah],00h // 00b15bd2
        jz loc_00b15d1d // 00b15bd9
        mov ecx,dword ptr [esi + 02ch] // 00b15bdf
        call bloom_output_holder_actual
        mov ecx,eax // 00b15be7
        call native_shadow_holder_texture_00b4cb10
        mov ecx,dword ptr [esi + 02ch] // 00b15bee
        mov ebp,eax // 00b15bf1
        call bloom_output_holder_actual
        mov ecx,eax // 00b15bf8
        call native_shadow_holder_texture_00b4cb10
        mov edx,dword ptr [eax] // 00b15bff
        mov ecx,eax // 00b15c01
        mov eax, edx // original captured owner profile
        push eax
        mov edx, dword ptr [esp + 0a4h]
        mov edx, dword ptr [edx]
        call texture_width
        test eax,eax // 00b15c08
        mov dword ptr [esp + 02ch],eax // 00b15c0a
        fild dword ptr [esp + 02ch] // 00b15c0e
        jge loc_00b15c1a // 00b15c12
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax]
        mov eax, dword ptr [eax + 4]
        fadd dword ptr [eax]
        pop eax
    loc_00b15c1a:
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax]
        mov eax, dword ptr [eax + 8]
        fdivr qword ptr [eax]
        pop eax
        mov edx,dword ptr [ebp] // 00b15c20
        mov eax, edx // original captured owner profile
        mov ecx,ebp // 00b15c26
        fstp dword ptr [esp + 014h] // 00b15c28
        push eax
        mov edx, dword ptr [esp + 0a4h]
        mov edx, dword ptr [edx]
        call texture_height
        test eax,eax // 00b15c2e
        mov dword ptr [esp + 02ch],eax // 00b15c30
        fild dword ptr [esp + 02ch] // 00b15c34
        jge loc_00b15c40 // 00b15c38
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax]
        mov eax, dword ptr [eax + 4]
        fadd dword ptr [eax]
        pop eax
    loc_00b15c40:
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax]
        mov eax, dword ptr [eax + 8]
        fld qword ptr [eax]
        pop eax
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax]
        mov eax, dword ptr [eax + 16]
        movss xmm1,dword ptr [eax]
        pop eax
        fld st(0) // 00b15c4e
        xorps xmm0,xmm0 // 00b15c50
        fdivrp st(2),st(0) // 00b15c53
        fxch // 00b15c55
        fstp dword ptr [esp + 010h] // 00b15c57
        fld dword ptr [esp + 014h] // 00b15c5b
        fstp dword ptr [esi + 018ch] // 00b15c5f
        fld dword ptr [esp + 010h] // 00b15c65
        fstp dword ptr [esi + 0190h] // 00b15c69
        fld dword ptr [esi + 0258h] // 00b15c6f
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax + 4]
        fld qword ptr [eax]
        pop eax
        fdiv st(1),st(0) // 00b15c7b
        fxch // 00b15c7d
        fstp dword ptr [esi + 026ch] // 00b15c7f
        fld dword ptr [esi + 025ch] // 00b15c85
        fdiv st(0),st(1) // 00b15c8b
        fld1 // 00b15c8d
        fld st(0) // 00b15c8f
        fdivrp st(2),st(0) // 00b15c91
        fxch // 00b15c93
        fstp dword ptr [esi + 0270h] // 00b15c95
        fld dword ptr [esi + 0264h] // 00b15c9b
        fsubr st(0),st(1) // 00b15ca1
        fstp dword ptr [esp + 014h] // 00b15ca3
        fld dword ptr [esp + 014h] // 00b15ca7
        fldz // 00b15cab
        fcomi st(0),st(1) // 00b15cad
        fstp st(1) // 00b15caf
        jbe loc_00b15cb8 // 00b15cb1
        movaps xmm2,xmm0 // 00b15cb3
        jmp loc_00b15cc6 // 00b15cb6
    loc_00b15cb8:
        movss xmm2,dword ptr [esp + 014h] // 00b15cb8
        comiss xmm2,xmm1 // 00b15cbe
        jbe loc_00b15cc6 // 00b15cc1
        movaps xmm2,xmm1 // 00b15cc3
    loc_00b15cc6:
        fld dword ptr [esi + 0260h] // 00b15cc6
        movss dword ptr [esi + 0274h],xmm2 // 00b15ccc
        fdivrp st(3),st(0) // 00b15cd4
        fld st(1) // 00b15cd6
        fdivrp st(3),st(0) // 00b15cd8
        fxch st(2) // 00b15cda
        fstp dword ptr [esi + 0278h] // 00b15cdc
        fsub dword ptr [esi + 0268h] // 00b15ce2
        fstp dword ptr [esp + 014h] // 00b15ce8
        fld dword ptr [esp + 014h] // 00b15cec
        fxch // 00b15cf0
        fcomip st(0),st(1) // 00b15cf2
        fstp st(0) // 00b15cf4
        jbe loc_00b15d05 // 00b15cf6
        movaps xmm2,xmm0 // 00b15cf8
        movss dword ptr [esi + 027ch],xmm2 // 00b15cfb
        jmp loc_00b15d2e // 00b15d03
    loc_00b15d05:
        movss xmm2,dword ptr [esp + 014h] // 00b15d05
        comiss xmm2,xmm1 // 00b15d0b
        jbe loc_00b15d13 // 00b15d0e
        movaps xmm2,xmm1 // 00b15d10
    loc_00b15d13:
        movss dword ptr [esi + 027ch],xmm2 // 00b15d13
        jmp loc_00b15d2e // 00b15d1b
    loc_00b15d1d:
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax]
        mov eax, dword ptr [eax + 16]
        movss xmm1,dword ptr [eax]
        pop eax
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax]
        mov eax, dword ptr [eax + 8]
        fld qword ptr [eax]
        pop eax
        xorps xmm0,xmm0 // 00b15d2b
    loc_00b15d2e:
        fld dword ptr [esi + 04h] // 00b15d2e
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax + 12]
        movss xmm2,dword ptr [eax]
        pop eax
        fadd st(0),st(1) // 00b15d39
        mov ecx,edi // 00b15d3b
        fstp dword ptr [esp + 014h] // 00b15d3d
        fadd dword ptr [esi + 08h] // 00b15d41
        movss dword ptr [esi + 01d8h],xmm2 // 00b15d44
        push eax
        mov eax, dword ptr [esp + 0a4h]
        mov eax, dword ptr [eax + 8]
        movss xmm2,dword ptr [eax]
        pop eax
        movss dword ptr [esi + 01ech],xmm2 // 00b15d54
        movss xmm2,dword ptr [esp + 014h] // 00b15d5c
        fstp dword ptr [esp + 010h] // 00b15d62
        movss dword ptr [esi + 0208h],xmm2 // 00b15d66
        movss xmm2,dword ptr [esp + 010h] // 00b15d6e
        movss dword ptr [esi + 01dch],xmm0 // 00b15d74
        movss dword ptr [esi + 01e0h],xmm0 // 00b15d7c
        movss dword ptr [esi + 01e4h],xmm0 // 00b15d84
        movss dword ptr [esi + 01e8h],xmm0 // 00b15d8c
        movss dword ptr [esi + 01f0h],xmm0 // 00b15d94
        movss dword ptr [esi + 01f4h],xmm0 // 00b15d9c
        movss dword ptr [esi + 01f8h],xmm0 // 00b15da4
        movss dword ptr [esi + 01fch],xmm0 // 00b15dac
        movss dword ptr [esi + 0200h],xmm1 // 00b15db4
        movss dword ptr [esi + 0204h],xmm0 // 00b15dbc
        movss dword ptr [esi + 020ch],xmm2 // 00b15dc4
        movss dword ptr [esi + 0210h],xmm0 // 00b15dcc
        movss dword ptr [esi + 0214h],xmm1 // 00b15dd4
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 094h
        ret 4
    }
}
void* current_member(const void* actual, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(word(static_cast<const unsigned char*>(actual)+offset));
}
} // namespace

void continue_native_main_scene_bloom_targets_00b15bd2(void* service,
    void* command, const NativeMainSceneBloomTargetContext& c,
    NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_bloom_stage || state.bloom_targets_started)
        throw std::logic_error("main scene bloom/targets require their fresh reached continuation");
    if(!c.textures.far_corners || c.textures.far_corners->near_00ce3800!=&c.half_float_00ce3800)
        throw std::logic_error("main scene bloom requires the same actual half-float cell");
    state.bloom_targets_started=true;
    state.active_native_site=0x00b15bd2;
    const BloomScalarAccess access{&c.textures,&c.adaptation_scale_00ce3cb8,
        &c.negative_half_00ce69d0,&c.half_float_00ce3800};
    execute_bloom_constants(service,&access,command);

    state.active_native_site=0x00b15ddc;
    void* camera=get_native_scene_command_camera_00b1bf40(command);
    state.clear_flags_bits=word(static_cast<const unsigned char*>(camera)+0x188);
    state.clear_flags_captured=true;
    const std::uint32_t clear_flags=7;
    camera=get_native_scene_command_camera_00b1bf40(command);
    state.active_native_site=0x00b15df6;
    (void)set_native_camera_clear_flags_00b6fe10(camera,&clear_flags);
    state.clear_flags_changed=true;
    // BL is FF from B1573D and survives every preceding callee. The original
    // four byte stores prepare this full DWORD before the next camera getter.
    const std::uint32_t white=0xffffffffu;
    camera=get_native_scene_command_camera_00b1bf40(command);
    state.active_native_site=0x00b15e19;
    set_native_camera_clear_color_00b6fe50(camera,nullptr,white);
    state.clear_color_changed=true;

    auto& groups=c.targets.actual_group_owner;
    state.active_native_site=0x00b15e21;
    auto* surface=native_render_holder_primary_00b4cb20(current_member(service,0x38));
    auto* frame=static_cast<NativeFrameTargetOwnerStorage*>(current_member(service,0x1c8));
    state.active_native_site=0x00b15e30;
    set_native_frame_target_color_00b1fab0(*frame,0,surface,groups);
    state.active_native_site=0x00b15e38;
    surface=native_render_holder_primary_00b4cb20(current_member(service,0x3c));
    frame=static_cast<NativeFrameTargetOwnerStorage*>(current_member(service,0x1c8));
    state.active_native_site=0x00b15e46;
    set_native_frame_target_color_00b1fab0(*frame,1,surface,groups);
    state.active_native_site=0x00b15e4e;
    surface=native_render_holder_primary_00b4cb20(current_member(service,0x40));
    frame=static_cast<NativeFrameTargetOwnerStorage*>(current_member(service,0x1c8));
    state.active_native_site=0x00b15e5c;
    set_native_frame_target_color_00b1fab0(*frame,2,surface,groups);

    // Original publication -> captured profile -> current frame -> slot98.
    void* renderer=groups.actual_surface_context.actual_renderer_00f8d394;
    const std::uint32_t captured_profile=word(renderer);
    frame=static_cast<NativeFrameTargetOwnerStorage*>(current_member(service,0x1c8));
    state.active_native_site=0x00b15e76;
    if(captured_profile!=0x00d5f0a8 || !c.actual_renderer_profile_00d5f0a8 ||
       c.actual_renderer_profile_00d5f0a8[0x98/4]!=0x00b24e70)
        throw std::logic_error("main scene requires current D5F0A8:B24E70 frame-target binding");
    bind_native_renderer_frame_targets_00b24e70(renderer,frame,c.targets);
    state.active_native_site=0x00b15e78;
    state.reached_camera_frame_stage=true;
}
} // namespace bsp
