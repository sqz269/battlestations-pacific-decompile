#include "bsp/native_main_scene_execution_prefix.hpp"
#include "bsp/native_camera_history.hpp"
#include "bsp/native_camera_far_corners.hpp"
#include "bsp/native_camera_frame_command.hpp"
#include "bsp/native_render_diagnostic_labels.hpp"
#include "bsp/native_string_pool_storage.hpp"
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
#include "bsp/native_render_batch_preparation_actual.hpp"
#include "bsp/native_render_job_publication.hpp"
#include "bsp/native_frame_job_lifetime.hpp"
#include "bsp/native_material_entry_dispatch.hpp"
#include "bsp/native_node_visibility_factor.hpp"
#include "bsp/native_render_scene_submission.hpp"
#include "bsp/native_render_queue_execution.hpp"
#include "bsp/native_render_command_execution.hpp"
#include "bsp/native_post_effect_submission.hpp"
#include "bsp/native_material_pass_execution.hpp"
#include "bsp/native_render_command_owner.hpp"
#include "bsp/native_renderer_texture_binding.hpp"
#include "bsp/native_surface_dimensions.hpp"
#include <cstddef>
#include <cstring>
#include <exception>
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

__declspec(naked) void* __fastcall set_native_camera_render_mode_00b6fdf0(
    void*, void*, std::uint32_t) {
    __asm {
        mov eax, ecx // 00b6fdf0
        mov ecx, dword ptr [esp + 4] // 00b6fdf2
        mov edx, 1 // 00b6fdf6
        shl edx, cl // 00b6fdfb
        mov dword ptr [eax + 0198h], ecx // 00b6fdfd
        mov dword ptr [eax + 019ch], edx // 00b6fe03
        ret 4 // 00b6fe09
    }
}

void continue_native_main_scene_camera_gather_00b15e78(void*, void* command,
    const NativeMainSceneCameraGatherContext& c,
    NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_camera_frame_stage || state.camera_gather_started)
        throw std::logic_error("main scene camera/gather requires its fresh reached continuation");
    state.camera_gather_started=true;
    state.active_native_site=0x00b15e7a;
    void* camera=get_native_scene_command_camera_00b1bf40(command);
    auto& targets=c.bloom_targets.targets;
    auto& renderer_cell=targets.actual_group_owner.actual_surface_context.actual_renderer_00f8d394;
    state.active_native_site=0x00b15e81;
    if(static_cast<const volatile void*>(c.camera.renderer_00)!=static_cast<const volatile void*>(&renderer_cell) ||
       c.camera.renderer_00!=&c.prefix.actual_renderer_00f8d394 ||
       c.camera.renderer_profile_04!=c.prefix.actual_renderer_profile_00d5f0a8 ||
       c.camera.renderer_profile_04!=c.bloom_targets.actual_renderer_profile_00d5f0a8 ||
       !c.camera.prepare_08 || !c.camera.viewport_clear_0c ||
       c.camera.prepare_08->synchronization_04!=&targets.actual_synchronization_0108d6dc ||
       c.camera.viewport_clear_0c->synchronization_00!=&targets.actual_synchronization_0108d6dc ||
       !c.camera.prepare_08->frustum_00 ||
       c.camera.prepare_08->frustum_00->length_access.dispatch_bypass_0109dd78!=c.prefix.axes.length_access.dispatch_bypass_0109dd78 ||
       c.camera.prepare_08->frustum_00->length_access.except_00c27489!=c.prefix.axes.length_access.except_00c27489)
        throw std::logic_error("main scene camera requires the same actual renderer, synchronization and CRT domain");
    execute_native_camera_frame_command_00b71360(camera,&c.camera);
    state.camera_frame_completed=true;

    state.active_native_site=0x00b15e88;
    camera=get_native_scene_command_camera_00b1bf40(command);
    state.render_mode_bits=word(static_cast<const unsigned char*>(camera)+0x198);
    state.render_mode_captured=true;
    camera=get_native_scene_command_camera_00b1bf40(command);
    state.active_native_site=0x00b15ea2;
    (void)set_native_camera_render_mode_00b6fdf0(camera,nullptr,6);
    state.render_mode_changed=true;

    camera=get_native_scene_command_camera_00b1bf40(command);
    void* const scene=current_member(camera,0xa4);
    camera=get_native_scene_command_camera_00b1bf40(command);
    state.active_native_site=0x00b15ebf;
    if(c.system.actual_renderer_00f8d394!=c.camera.renderer_00 ||
       c.system.actual_timer_01090ab0!=&c.prefix.actual_clock_01090ab0 ||
       c.system.actual_axes!=&c.prefix.axes ||
       c.system.actual_synchronization!=&targets.actual_synchronization_0108d6dc ||
       c.system.actual_texture_profile_00d61948!=c.bloom_targets.textures.actual_texture_profile_00d61948 ||
       !c.system_frame.prefix_preimage ||
       c.system_frame.sampler.phase!=NativeSamplerLoaderOperation::Phase::fresh)
        throw std::logic_error("main scene system gather requires its fresh frame and same actual provider domain");
    state.system_gather_started=true;
    gather_native_system_constants_00b46a70(scene,camera,c.system,c.system_frame);
    state.active_native_site=0x00b15ec4;
    state.reached_label_stage=true;
}

void unwind_native_main_scene_drsh_label(const NativeMainSceneDrshLabelContext& c,
    NativeMainSceneDrshLabelFrame& frame) noexcept {
    const auto state=frame.native_cleanup_state;
    if(state==-1) return;
    if((state!=0 && state!=2) || frame.cleanup_started) std::terminate();
    frame.native_cleanup_state=-1; // Both proven unwind-map entries go to -1.
    frame.cleanup_started=true;
    try {
        if(state==0) {
            frame.cleanup_native_site=0x00cbc460;
            destroy_native_string_header_0041dd20(frame.temporary_header,c.strings);
        } else {
            frame.cleanup_native_site=0x00cbc46b;
            reset_native_render_diagnostic_label_00b13510(*c.system.actual_service_00f8d39c,c.strings);
        }
        frame.cleanup_completed=true;
    } catch(...) { std::terminate(); }
}

void continue_native_main_scene_drsh_label_00b15ec4(
    const NativeMainSceneDrshLabelContext& c, NativeMainSceneDrshLabelFrame& frame,
    NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_label_stage || frame.started || frame.native_cleanup_state!=-1)
        throw std::logic_error("main scene DRSH requires its fresh reached label frame");
    frame.started=true;
    frame.active_native_site=state.active_native_site=0x00b15ec4;
    try {
        if(!c.system.actual_service_00f8d39c || !c.actual_drsh_bytes_00d5e500 ||
           c.system.actual_manager_01090aa0!=&c.strings.actual_manager_publication_01090aa0)
            throw std::logic_error("main scene DRSH requires the same actual service and raw string-manager domain");
        auto* const temporary=frame.temporary_header;
        *static_cast<volatile std::uint32_t*>(temporary)=0;
        *static_cast<volatile std::uint32_t*>(temporary+1)=0;
        frame.active_native_site=state.active_native_site=0x00b15ed4;
        resize_native_string_header_0041dd40(temporary,c.strings,4,true);
        // EBX is captured once at B15ED9; later header pointer changes do not
        // replace this source or the normal return's block argument.
        void* const captured_data=current_member(temporary,4);
        frame.captured_data=captured_data;
        if(captured_data) {
            const std::uint32_t bytes=word(temporary)+1u;
            frame.active_native_site=state.active_native_site=0x00b15eef;
            if(bytes) std::memmove(captured_data,c.actual_drsh_bytes_00d5e500,bytes);
        }
        void* const service=*c.system.actual_service_00f8d39c; // B15EF7
        frame.captured_service=service;
        frame.native_cleanup_state=0; // B15EFE, AFTER publication capture.
        if(service) {
            void* const destination_header=reinterpret_cast<void*>(
                reinterpret_cast<std::uintptr_t>(service)+0x684u);
            frame.destination_header=destination_header;
            if(destination_header!=temporary) {
                const std::uint32_t requested=word(temporary);
                frame.active_native_site=state.active_native_site=0x00b15f21;
                resize_native_string_header_0041dd40(destination_header,c.strings,requested,true);
                if(word(temporary)!=0) {
                    const std::uint32_t bytes=word(destination_header);
                    void* const destination=current_member(destination_header,4);
                    frame.active_native_site=state.active_native_site=0x00b15f38;
                    if(bytes) std::memmove(destination,captured_data,bytes);
                }
            }
        }
        // B15F42 disowns the temporary and arms label reset BEFORE the pool
        // getter/return. A returning-getter failure must not retry that block.
        frame.native_cleanup_state=2;
        if(captured_data) {
            const std::uint32_t bytes=word(temporary)+1u;
            frame.temporary_return_started=true;
            frame.active_native_site=state.active_native_site=0x00b15f57;
            auto* const pool=native_string_pool_get_or_create_00419cc0(
                c.strings.actual_published_01090aa8,c.strings.actual_manager_publication_01090aa0);
            frame.active_native_site=state.active_native_site=0x00b15f5e;
            return_native_string_pool_00bd1510(pool,captured_data,bytes,
                c.strings.actual_small_returns_disabled_01090aa4);
            frame.temporary_return_completed=true;
        }
        frame.active_native_site=state.active_native_site=0x00b15f63;
        frame.completed=true;
        state.reached_drsh_batch_stage=true;
    } catch(...) {
        frame.failed=true;
        unwind_native_main_scene_drsh_label(c,frame);
        throw;
    }
}
namespace {
NativeRenderBatchStorage* main_scene_batch(void* command, std::uint32_t index) noexcept {
    return static_cast<NativeRenderBatchStorage*>(current_member(command,0x0cu+index*4u));
}
NativeRenderBatchPreparationContext& main_scene_preparation(NativeMainSceneBatchContext& c) {
    if(!c.preparation ||
       static_cast<const volatile void*>(&c.preparation->actual_manager_01090aa0)!=
           static_cast<const volatile void*>(&c.label.strings.actual_manager_publication_01090aa0) ||
       static_cast<const volatile void*>(&c.preparation->actual_queue_constructor.actual_renderer_00f8d394)!=
           static_cast<const volatile void*>(c.label.system.actual_renderer_00f8d394))
        throw std::logic_error("main scene batches require the same actual queue, manager and renderer domain");
    return *c.preparation;
}
const volatile std::uint32_t* main_scene_batch_profile(NativeRenderBatchStorage* batch,
    NativeRenderBatchPreparationContext& preparation) {
    const volatile std::uint32_t* profile=nullptr;
    // Capture owner[0] once. The selected table remains live across B1BF40.
    switch(word(batch)) {
    case 0x00d5e5ac: profile=preparation.actual_batch_profile_00d5e5ac; break;
    case 0x00d62064: profile=preparation.actual_base_batch_profile_00d62064; break;
    default: break;
    }
    if(!profile) throw std::logic_error("main scene requires its current concrete batch profile");
    return profile;
}
NativeFrameJobLifetimeBindings& main_scene_job_bindings(NativeMainSceneBatchContext& c) {
    auto& preparation=main_scene_preparation(c);
    if(!c.jobs || !c.jobs->frame_lifetime || !c.preparation_dispatch ||
       &c.jobs->actual_manager_01090aa0!=&preparation.actual_manager_01090aa0 ||
       &c.preparation_dispatch->preparation_context()!=&preparation ||
       &c.jobs->frame_lifetime->execution.job_dispatch()!=c.preparation_dispatch)
        throw std::logic_error("main scene jobs require the same actual preparation and lifetime domain");
    return *c.jobs->frame_lifetime;
}
void main_scene_batch_site(std::uint32_t site, NativeMainSceneDrshLabelFrame& label,
    NativeMainSceneBatchFrame& frame, NativeMainSceneExecutionPrefixState& state) noexcept {
    label.active_native_site=frame.active_native_site=state.active_native_site=site;
}
} // namespace

void continue_native_main_scene_batches_00b15f63(void* command,
    NativeMainSceneBatchContext& c, NativeMainSceneDrshLabelFrame& label,
    NativeMainSceneBatchFrame& frame, NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_drsh_batch_stage || state.reached_excluded_node_stage ||
       !label.completed || label.failed || label.native_cleanup_state!=2 ||
       label.cleanup_started || frame.started)
        throw std::logic_error("main scene batches require the fresh reached DRSH state2 frame");
    frame.started=true;
    main_scene_batch_site(0x00b15f63,label,frame,state);
    try {
        const auto& system=c.label.system;
        if(!system.actual_synchronization || !system.actual_renderer_00f8d394 ||
           !system.actual_service_00f8d39c ||
           system.actual_manager_01090aa0!=&c.label.strings.actual_manager_publication_01090aa0)
            throw std::logic_error("main scene batches require their retained actual DRSH domain");
        bool parallel=false;
        if(system.actual_synchronization->mode_00==0) {
            main_scene_batch_site(0x00b15f73,label,frame,state);
            if(native_render_batch_count_00b51b20(main_scene_batch(command,0))>50) {
                main_scene_batch_site(0x00b15f84,label,frame,state);
                parallel=native_render_batch_count_00b51b20(main_scene_batch(command,1))>50;
            }
        }
        frame.parallel=parallel;
        if(parallel) {
            for(std::uint32_t index=0;index<2;++index) {
                auto* const initial=main_scene_batch(command,index);
                static_cast<volatile NativeRenderBatchStorage*>(initial)->mode_08=index;
                main_scene_batch_site(index==0?0x00b15f98:0x00b15fbd,label,frame,state);
                auto& bindings=main_scene_job_bindings(c);
                auto* const owner=get_native_frame_jobs_actual_004c1130(*c.jobs);
                auto* const captured_batch=main_scene_batch(command,index);
                auto* const captured_pool=reinterpret_cast<NativeFrameJobPoolStorage*>(
                    reinterpret_cast<std::uintptr_t>(owner)+4u);
                main_scene_batch_site(index==0?0x00b15fa3:0x00b15fc8,label,frame,state);
                auto* const job=get_native_preparation_job_actual_00b0ffb0(*c.jobs);
                main_scene_batch_site(index==0?0x00b15fb1:0x00b15fd6,label,frame,state);
                bindings.execution.require_enqueue_virtual_04(*captured_pool);
                enqueue_native_frame_job_00be3020(*captured_pool,job,
                    reinterpret_cast<std::uint32_t>(captured_batch));
                ++frame.submissions_completed;
            }
            main_scene_batch_site(0x00b15fd8,label,frame,state);
            auto& bindings=main_scene_job_bindings(c);
            auto* const owner=get_native_frame_jobs_actual_004c1130(*c.jobs);
            auto* const pool=reinterpret_cast<NativeFrameJobPoolStorage*>(
                reinterpret_cast<std::uintptr_t>(owner)+4u);
            main_scene_batch_site(0x00b15fe8,label,frame,state);
            bindings.execution.require_dispatch_virtual_08(*pool);
            dispatch_native_frame_jobs_00be3150(*pool,1,bindings.scope,bindings.execution);
            frame.dispatch_completed=true;
        }
        for(std::uint32_t index=0;index<2;++index) {
            if(!parallel) {
                auto* const batch=main_scene_batch(command,index);
                main_scene_batch_site(index==0?0x00b16023:0x00b1604a,label,frame,state);
                auto& preparation=main_scene_preparation(c);
                const auto* const profile=main_scene_batch_profile(batch,preparation);
                if(profile[3]!=0x00b51df0)
                    throw std::logic_error("main scene requires current B51DF0 serial preparation");
                volatile std::uint32_t argument=index;
                prepare_native_render_batch_actual_00b51df0(*batch,argument,preparation);
                ++frame.serial_preparations_completed;
            }
            // The serial path reloads the batch AFTER its preparation; the
            // parallel path reloads after dispatch. Capture profile BEFORE
            // the camera getter and read its execution slot only afterward.
            auto* const batch=main_scene_batch(command,index);
            auto& preparation=main_scene_preparation(c);
            const auto* const captured_profile=main_scene_batch_profile(batch,preparation);
            main_scene_batch_site(parallel?(index==0?0x00b15ff1:0x00b16007):
                (index==0?0x00b16030:0x00b16057),label,frame,state);
            void* const camera=get_native_scene_command_camera_00b1bf40(command);
            const std::uint32_t target=captured_profile[2];
            main_scene_batch_site(parallel?(index==0?0x00b15ffe:0x00b16014):
                (index==0?0x00b1603e:0x00b16066),label,frame,state);
            if(target!=0x00b55550 || !c.entries ||
               &c.entries->actual_renderer_00f8d394!=system.actual_renderer_00f8d394)
                throw std::logic_error("main scene requires its same actual B55550 material-entry domain");
            execute_native_render_batch_entries_00b55550(batch,index,camera,*c.entries,frame.entries[index]);
            ++frame.executions_completed;
        }
        main_scene_batch_site(0x00b16068,label,frame,state);
        frame.completed=true;
        state.reached_excluded_node_stage=true;
    } catch(...) {
        frame.failed=true;
        label.failed=true;
        unwind_native_main_scene_drsh_label(c.label,label);
        throw;
    }
}
namespace {
// Keep FLD1 before the saved-camera word read, and FSTP32 after the current
// node load, exactly as B1051E..B10532. No x87 value crosses a C++ adapter.
__declspec(naked) std::uint32_t __fastcall raise_excluded_node_and_capture_flags(
    void*, const void*) {
    __asm {
        push esi
        fld1 // 00b1051e
        mov esi, dword ptr [edx + 0188h]
        push 0
        push ecx
        mov ecx, dword ptr [ecx + 01c0h]
        fstp dword ptr [esp] // 00b1052f
        call set_native_node_visibility_factor_00b6da70
        mov eax, esi
        pop esi
        ret
    }
}
__declspec(naked) void __fastcall clear_current_excluded_node_visibility(void*) {
    __asm {
        fldz // 00b10596
        push 0
        push ecx
        mov ecx, dword ptr [ecx + 01c0h]
        fstp dword ptr [esp] // 00b105a1
        call set_native_node_visibility_factor_00b6da70
        ret
    }
}
__declspec(naked) void __fastcall restore_current_camera_near(void*, const void*) {
    __asm {
        push esi
        mov esi, ecx
        fld dword ptr [edx] // B160BA: borrowed saved word, source operand relocation.
        push ecx
        mov ecx, esi
        fstp dword ptr [esp] // 00b160c1
        call get_native_scene_command_camera_00b1bf40
        mov ecx, eax
        mov edx, esp
        call set_native_camera_near_00b6fbf0
        add esp, 4
        pop esi
        ret
    }
}
NativeRenderCommandQueueStorage* excluded_node_queue(NativeMainSceneExcludedNodeContext& c) {
    if(!c.queue ||
       &c.queue->actual_manager_01090aa0!=&c.label.strings.actual_manager_publication_01090aa0 ||
       static_cast<const volatile void*>(&c.queue->actual_queue_constructor.actual_renderer_00f8d394)!=
           static_cast<const volatile void*>(c.label.system.actual_renderer_00f8d394))
        throw std::logic_error("excluded node requires the same actual queue, manager and renderer cells");
    return get_native_render_command_queue_004c11f0(*c.queue);
}
void require_excluded_submission(NativeMainSceneExcludedNodeContext& c,
    NativeMainSceneExcludedNodeFrame& frame) {
    if(!c.submission || !frame.submission ||
       &c.submission->strings.actual_published_01090aa8!=&c.label.strings.actual_published_01090aa8 ||
       &c.submission->strings.actual_small_returns_disabled_01090aa4!=&c.label.strings.actual_small_returns_disabled_01090aa4 ||
       &c.submission->strings.actual_manager_publication_01090aa0!=&c.label.strings.actual_manager_publication_01090aa0 ||
       frame.submission->allocation || frame.submission->command || frame.submission->appended || frame.submission->completed)
        throw std::logic_error("excluded node requires its fresh actual drawable submission and string domain");
}
void require_excluded_execution_domain(NativeMainSceneExcludedNodeContext& c) {
    if(!c.execution) return; // The raw B1E6D0 empty path needs no context.
    if(&c.execution->actual_commands.actual_renderer_00f8d394!=c.label.system.actual_renderer_00f8d394 ||
       &c.execution->actual_commands.actual_service_00f8d39c!=c.label.system.actual_service_00f8d39c ||
       &c.execution->actual_commands.actual_strings.actual_published_01090aa8!=&c.label.strings.actual_published_01090aa8 ||
       &c.execution->actual_commands.actual_strings.actual_small_returns_disabled_01090aa4!=&c.label.strings.actual_small_returns_disabled_01090aa4 ||
       &c.execution->actual_commands.actual_strings.actual_manager_publication_01090aa0!=&c.label.strings.actual_manager_publication_01090aa0 ||
       &c.execution->actual_owners!=&c.submission->commands.owners || !c.execution->actual_lifetime ||
       &c.execution->actual_lifetime->batch_references!=&c.submission->commands.batch_references ||
       &c.execution->actual_lifetime->models!=&c.submission->commands.models)
        throw std::logic_error("excluded node execution must retain its actual submission owner and lifetime domains");
}
void excluded_restore_site(std::uint32_t site, NativeMainSceneDrshLabelFrame& label,
    NativeMainSceneExcludedRestoreFrame& frame, NativeMainSceneExecutionPrefixState& state) noexcept {
    label.active_native_site=frame.active_native_site=state.active_native_site=site;
}
} // namespace

void execute_native_excluded_node_00b10500(void* service,void* command,void* targets,
    NativeMainSceneExcludedNodeContext& c, NativeMainSceneExcludedNodeFrame& frame) {
    if(frame.started) throw std::logic_error("excluded node requires its fresh persistent frame");
    frame.started=true;
    frame.active_native_site=0x00b10503;
    if(*static_cast<const volatile unsigned char*>(static_cast<const void*>(
        static_cast<const unsigned char*>(service)+0x22c))==0) {
        frame.completed=true;
        return;
    }
    frame.entered=true;
    try {
        frame.active_native_site=0x00b10519;
        void* camera=get_native_scene_command_camera_00b1bf40(command);
        frame.active_native_site=0x00b10532;
        const std::uint32_t flags=raise_excluded_node_and_capture_flags(service,camera);
        frame.saved_clear_flags=flags;
        frame.visibility_raised=true;
        const std::uint32_t zero=0;
        frame.active_native_site=0x00b1053b;
        camera=get_native_scene_command_camera_00b1bf40(command);
        frame.active_native_site=0x00b10542;
        (void)set_native_camera_clear_flags_00b6fe10(camera,&zero);
        frame.flags_cleared=true;
        frame.active_native_site=0x00b10549;
        camera=get_native_scene_command_camera_00b1bf40(command);
        void* const scene=current_member(camera,0xa4);
        frame.active_native_site=0x00b10554;
        auto* const captured_queue=excluded_node_queue(c);
        void* const node=current_member(service,0x1c0);
        frame.active_native_site=0x00b1056b;
        camera=get_native_scene_command_camera_00b1bf40(command);
        frame.active_native_site=0x00b10576;
        require_excluded_submission(c,frame);
        (void)submit_native_drawable_00b1f5e0(*captured_queue,scene,camera,targets,node,
            *c.submission,*frame.submission);
        frame.submitted=true;
        frame.active_native_site=0x00b1057b;
        auto* const current_queue=excluded_node_queue(c);
        frame.active_native_site=0x00b10582;
        require_excluded_execution_domain(c);
        execute_last_native_render_command_00b1e6d0(*current_queue,c.execution,frame.execution);
        frame.executed=true;
        frame.active_native_site=0x00b1058a;
        camera=get_native_scene_command_camera_00b1bf40(command);
        frame.active_native_site=0x00b10591;
        (void)set_native_camera_clear_flags_00b6fe10(camera,&flags);
        frame.flags_restored=true;
        frame.active_native_site=0x00b105a4;
        clear_current_excluded_node_visibility(service);
        frame.visibility_cleared=true;
        frame.active_native_site=0x00b105af;
        frame.completed=true;
    } catch(...) {
        frame.failed=true; // B10500 has no local restoration or ownership cleanup.
        throw;
    }
}

void continue_native_main_scene_excluded_restore_00b16068(void* service,void* command,
    NativeMainSceneExcludedNodeContext& c, NativeMainSceneDrshLabelFrame& label,
    const NativeMainSceneBatchFrame& batch, NativeMainSceneExcludedRestoreFrame& frame,
    NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_excluded_node_stage || state.reached_post_drsh_stage ||
       !batch.completed || batch.failed || !label.completed || label.failed ||
       label.native_cleanup_state!=2 || label.cleanup_started || frame.started)
        throw std::logic_error("main scene excluded/restore requires completed batches and live DRSH state2");
    frame.started=true;
    excluded_restore_site(0x00b16068,label,frame,state);
    try {
        if(*static_cast<const volatile unsigned char*>(static_cast<const void*>(
            static_cast<const unsigned char*>(service)+0x22d))!=0) {
            void* const targets=current_member(service,0x1c8);
            frame.excluded_called=true;
            excluded_restore_site(0x00b1607b,label,frame,state);
            execute_native_excluded_node_00b10500(service,command,targets,c,frame.excluded);
        }
        const std::uint32_t mode=state.render_mode_bits;
        excluded_restore_site(0x00b16087,label,frame,state);
        void* camera=get_native_scene_command_camera_00b1bf40(command);
        excluded_restore_site(0x00b1608e,label,frame,state);
        (void)set_native_camera_render_mode_00b6fdf0(camera,nullptr,mode);
        frame.render_mode_restored=true;
        excluded_restore_site(0x00b16097,label,frame,state);
        label.native_cleanup_state=-1; // Native normal path disarms BEFORE reset.
        label.cleanup_started=true;
        label.cleanup_native_site=0x00b160a2;
        excluded_restore_site(0x00b160a2,label,frame,state);
        reset_native_render_diagnostic_label_00b13510(*c.label.system.actual_service_00f8d39c,c.label.strings);
        label.cleanup_completed=true;
        frame.label_reset_completed=true;
        const std::uint32_t flags=state.clear_flags_bits;
        excluded_restore_site(0x00b160ae,label,frame,state);
        camera=get_native_scene_command_camera_00b1bf40(command);
        excluded_restore_site(0x00b160b5,label,frame,state);
        (void)set_native_camera_clear_flags_00b6fe10(camera,&flags);
        frame.clear_flags_restored=true;
        excluded_restore_site(0x00b160ba,label,frame,state);
        restore_current_camera_near(command,&state.near_bits);
        frame.near_restored=true;
        excluded_restore_site(0x00b160d0,label,frame,state);
        frame.completed=true;
        state.reached_post_drsh_stage=true;
    } catch(...) {
        frame.failed=true;
        label.failed=true;
        unwind_native_main_scene_drsh_label(c.label,label); // State-1 does nothing.
        throw;
    }
}
void continue_native_main_scene_post_gather_00b160d0(void* service,void* command,
    NativeMainScenePostGatherContext& c,const NativeMainSceneDrshLabelFrame& label,
    const NativeMainSceneExcludedRestoreFrame& restored,NativeMainScenePostGatherFrame& frame,
    NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_post_drsh_stage || state.reached_scne_label_stage ||
       !restored.completed || restored.failed || label.native_cleanup_state!=-1 ||
       !label.cleanup_completed || label.failed || frame.started)
        throw std::logic_error("post DRSH requires completed restoration and disarmed label state");
    if(c.system.actual_renderer_00f8d394!=c.post.camera.renderer_00 ||
       c.system.actual_service_00f8d39c!=&c.post.materials.actual_diagnostics_00f8d39c ||
       c.system.actual_synchronization!=&c.post.materials.synchronization ||
       !frame.system.prefix_preimage || frame.system.sampler.phase!=NativeSamplerLoaderOperation::Phase::fresh)
        throw std::logic_error("post DRSH requires its fresh system frame and same actual renderer/service domain");
    frame.started=true;
    try {
        frame.active_native_site=state.active_native_site=0x00b160d8;
        void* const owner=current_member(service,0x60);
        if(word(owner)!=0x00d5e164 || !c.depth_profile_00d5e164 ||
           c.depth_profile_00d5e164[0x10/4]!=0x00b4dfb0)
            throw std::logic_error("post DRSH requires current D5E164 depth-downscale draw");
        draw_native_post_effect_owner_00b4dfb0(owner,c.post,frame.post);
        frame.depth_draw_completed=true;
        frame.active_native_site=state.active_native_site=0x00b160dc;
        void* camera=get_native_scene_command_camera_00b1bf40(command);
        void* const scene=current_member(camera,0xa4);
        frame.active_native_site=state.active_native_site=0x00b160e9;
        camera=get_native_scene_command_camera_00b1bf40(command);
        frame.active_native_site=state.active_native_site=0x00b160f2;
        gather_native_system_constants_00b46a70(scene,camera,c.system,frame.system);
        frame.system_gather_completed=true;
        const std::uint32_t color=state.color_word;
        frame.active_native_site=state.active_native_site=0x00b160fe;
        camera=get_native_scene_command_camera_00b1bf40(command);
        frame.active_native_site=state.active_native_site=0x00b16105;
        set_native_camera_clear_color_00b6fe50(camera,nullptr,color);
        frame.color_restored=true;
        frame.active_native_site=state.active_native_site=0x00b1610a;
        frame.completed=true;state.reached_scne_label_stage=true;
    } catch(...) {frame.failed=true;throw;}
}
void unwind_native_main_scene_scne_label(const NativeMainSceneScneLabelContext& c,
    NativeMainSceneScneLabelFrame& frame) noexcept {
    const auto state=frame.native_cleanup_state;
    if(state==-1) return;
    if((state!=3 && state!=5) || frame.cleanup_started) std::terminate();
    frame.native_cleanup_state=-1; // Both proven unwind-map entries go to -1.
    frame.cleanup_started=true;
    try {
        if(state==3) {
            frame.cleanup_native_site=0x00cbc473;
            destroy_native_string_header_0041dd20(frame.temporary_header,c.domains.strings);
        } else {
            frame.cleanup_native_site=0x00cbc47e;
            reset_native_render_diagnostic_label_00b13510(*c.domains.system.actual_service_00f8d39c,c.domains.strings);
        }
        frame.cleanup_completed=true;
    } catch(...) { std::terminate(); }
}

void continue_native_main_scene_scne_label_00b1610a(
    const NativeMainSceneScneLabelContext& c, NativeMainSceneScneLabelFrame& frame,
    NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_scne_label_stage || state.reached_scne_target_stage || frame.started || frame.native_cleanup_state!=-1)
        throw std::logic_error("main scene SCNE requires its fresh reached label frame");
    frame.started=true;
    frame.active_native_site=state.active_native_site=0x00b1610a;
    try {
        if(!c.domains.system.actual_service_00f8d39c || !c.actual_scne_bytes_00d5e4f8 ||
           c.domains.system.actual_manager_01090aa0!=&c.domains.strings.actual_manager_publication_01090aa0)
            throw std::logic_error("main scene SCNE requires the same actual service and raw string-manager domain");
        auto* const temporary=frame.temporary_header;
        *static_cast<volatile std::uint32_t*>(temporary)=0;
        *static_cast<volatile std::uint32_t*>(temporary+1)=0;
        frame.active_native_site=state.active_native_site=0x00b1611a;
        resize_native_string_header_0041dd40(temporary,c.domains.strings,4,true);
        // EBX is captured once at B1611F; later header pointer changes do not
        // replace this source or the normal return's block argument.
        void* const captured_data=current_member(temporary,4);
        frame.captured_data=captured_data;
        if(captured_data) {
            const std::uint32_t bytes=word(temporary)+1u;
            frame.active_native_site=state.active_native_site=0x00b16135;
            if(bytes) std::memmove(captured_data,c.actual_scne_bytes_00d5e4f8,bytes);
        }
        void* const service=*c.domains.system.actual_service_00f8d39c; // B1613D
        frame.captured_service=service;
        frame.native_cleanup_state=3; // B16144, AFTER publication capture.
        if(service) {
            void* const destination_header=reinterpret_cast<void*>(
                reinterpret_cast<std::uintptr_t>(service)+0x684u);
            frame.destination_header=destination_header;
            if(destination_header!=temporary) {
                const std::uint32_t requested=word(temporary);
                frame.active_native_site=state.active_native_site=0x00b1616b;
                resize_native_string_header_0041dd40(destination_header,c.domains.strings,requested,true);
                if(word(temporary)!=0) {
                    const std::uint32_t bytes=word(destination_header);
                    void* const destination=current_member(destination_header,4);
                    frame.active_native_site=state.active_native_site=0x00b16182;
                    if(bytes) std::memmove(destination,captured_data,bytes);
                }
            }
        }
        // B1618C disowns the temporary and arms label reset BEFORE the pool
        // getter/return. A returning-getter failure must not retry that block.
        frame.native_cleanup_state=5;
        if(captured_data) {
            const std::uint32_t bytes=word(temporary)+1u;
            frame.temporary_return_started=true;
            frame.active_native_site=state.active_native_site=0x00b161a1;
            auto* const pool=native_string_pool_get_or_create_00419cc0(
                c.domains.strings.actual_published_01090aa8,c.domains.strings.actual_manager_publication_01090aa0);
            frame.active_native_site=state.active_native_site=0x00b161a8;
            return_native_string_pool_00bd1510(pool,captured_data,bytes,
                c.domains.strings.actual_small_returns_disabled_01090aa4);
            frame.temporary_return_completed=true;
        }
        frame.active_native_site=state.active_native_site=0x00b161ad;
        frame.completed=true;
        state.reached_scne_target_stage=true;
    } catch(...) {
        frame.failed=true;
        unwind_native_main_scene_scne_label(c,frame);
        throw;
    }
}
void continue_native_main_scene_scne_targets_00b161ad(void* service,void* command,
    const NativeMainSceneScneTargetContext& c,NativeMainSceneScneLabelFrame& label,
    NativeMainSceneScneTargetFrame& frame,NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_scne_target_stage || state.reached_scne_batch_stage ||
       !label.completed || label.failed || label.native_cleanup_state!=5 ||
       label.cleanup_started || frame.started)
        throw std::logic_error("SCNE targets require completed label and armed state5");
    frame.started=true;
    try {
        frame.active_native_site=state.active_native_site=0x00b161ad;
        auto& groups=c.targets.actual_group_owner;
        auto& renderer_cell=groups.actual_surface_context.actual_renderer_00f8d394;
        const auto& system=c.label.domains.system;
        if(static_cast<const volatile void*>(c.camera.renderer_00)!=static_cast<const volatile void*>(&renderer_cell) ||
           system.actual_renderer_00f8d394!=c.camera.renderer_00 ||
           system.actual_synchronization!=&c.targets.actual_synchronization_0108d6dc ||
           !c.camera.prepare_08 || !c.camera.viewport_clear_0c || !system.actual_axes ||
           c.camera.prepare_08->synchronization_04!=system.actual_synchronization ||
           c.camera.viewport_clear_0c->synchronization_00!=system.actual_synchronization ||
           !c.camera.prepare_08->frustum_00 ||
           c.camera.prepare_08->frustum_00->length_access.dispatch_bypass_0109dd78!=system.actual_axes->length_access.dispatch_bypass_0109dd78 ||
           c.camera.prepare_08->frustum_00->length_access.except_00c27489!=system.actual_axes->length_access.except_00c27489)
            throw std::logic_error("SCNE targets require the same actual renderer, synchronization and CRT domain");
        const auto selected=*reinterpret_cast<const volatile std::uint8_t*>(
            static_cast<const unsigned char*>(service)+0x22d);
        frame.active_native_site=state.active_native_site=0x00b161be;
        auto* const surface=native_render_holder_primary_00b4cb20(current_member(service,selected?0x48:0x44));
        auto* targets=static_cast<NativeFrameTargetOwnerStorage*>(current_member(service,0x1cc));
        frame.active_native_site=state.active_native_site=0x00b161cb;
        set_native_frame_target_color_00b1fab0(*targets,0,surface,groups);
        frame.color_installed=true;
        // B161D0/D6 capture publication/profile; B161D8 reloads +1CC BEFORE
        // B161DE reads the captured profile's current slot98.
        void* const renderer=renderer_cell;
        const std::uint32_t profile=word(renderer);
        targets=static_cast<NativeFrameTargetOwnerStorage*>(current_member(service,0x1cc));
        frame.active_native_site=state.active_native_site=0x00b161e5;
        if(profile!=0x00d5f0a8 || !c.camera.renderer_profile_04 ||
           c.camera.renderer_profile_04[0x98/4]!=0x00b24e70)
            throw std::logic_error("SCNE targets require current D5F0A8:B24E70 binding");
        bind_native_renderer_frame_targets_00b24e70(renderer,targets,c.targets);
        frame.targets_bound=true;
        frame.active_native_site=state.active_native_site=0x00b161e9;
        void* const camera=get_native_scene_command_camera_00b1bf40(command);
        frame.active_native_site=state.active_native_site=0x00b161f0;
        execute_native_camera_frame_command_00b71360(camera,&c.camera);
        frame.camera_executed=true;
        frame.active_native_site=state.active_native_site=0x00b161f5;
        frame.completed=true;state.reached_scne_batch_stage=true;
    } catch(...) {
        frame.failed=true;
        unwind_native_main_scene_scne_label(c.label,label);
        throw;
    }
}
namespace {
void main_scene_scne_batch_site(std::uint32_t site, NativeMainSceneScneLabelFrame& label,
    NativeMainSceneBatchFrame& frame, NativeMainSceneExecutionPrefixState& state) noexcept {
    label.active_native_site=frame.active_native_site=state.active_native_site=site;
}
} // namespace

void continue_native_main_scene_scne_batches_00b161f5(void* command,
    NativeMainSceneScneBatchContext& scne, NativeMainSceneScneLabelFrame& label,
    NativeMainSceneBatchFrame& frame, NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_scne_batch_stage || state.reached_scne_capture_stage ||
       !label.completed || label.failed || label.native_cleanup_state!=5 ||
       label.cleanup_started || frame.started)
        throw std::logic_error("main scene batches require the fresh reached SCNE state5 frame");
    auto& c=scne.providers;
    if(&c.label!=&scne.label.domains)
        throw std::logic_error("SCNE batches require their retained label domain object");
    frame.started=true;
    main_scene_scne_batch_site(0x00b161f5,label,frame,state);
    try {
        const auto& system=c.label.system;
        if(!system.actual_synchronization || !system.actual_renderer_00f8d394 ||
           !system.actual_service_00f8d39c ||
           system.actual_manager_01090aa0!=&c.label.strings.actual_manager_publication_01090aa0)
            throw std::logic_error("main scene batches require their retained actual SCNE domain");
        bool parallel=false;
        if(system.actual_synchronization->mode_00==0) {
            main_scene_scne_batch_site(0x00b16205,label,frame,state);
            if(native_render_batch_count_00b51b20(main_scene_batch(command,0))>50) {
                main_scene_scne_batch_site(0x00b16216,label,frame,state);
                parallel=native_render_batch_count_00b51b20(main_scene_batch(command,1))>50;
            }
        }
        frame.parallel=parallel;
        if(parallel) {
            for(std::uint32_t index=0;index<2;++index) {
                auto* const initial=main_scene_batch(command,index);
                static_cast<volatile NativeRenderBatchStorage*>(initial)->mode_08=index;
                main_scene_scne_batch_site(index==0?0x00b1622a:0x00b16250,label,frame,state);
                auto& bindings=main_scene_job_bindings(c);
                auto* const owner=get_native_frame_jobs_actual_004c1130(*c.jobs);
                auto* const captured_batch=main_scene_batch(command,index);
                auto* const captured_pool=reinterpret_cast<NativeFrameJobPoolStorage*>(
                    reinterpret_cast<std::uintptr_t>(owner)+4u);
                main_scene_scne_batch_site(index==0?0x00b16235:0x00b1625b,label,frame,state);
                auto* const job=get_native_preparation_job_actual_00b0ffb0(*c.jobs);
                main_scene_scne_batch_site(index==0?0x00b16244:0x00b1626a,label,frame,state);
                bindings.execution.require_enqueue_virtual_04(*captured_pool);
                enqueue_native_frame_job_00be3020(*captured_pool,job,
                    reinterpret_cast<std::uint32_t>(captured_batch));
                ++frame.submissions_completed;
            }
            main_scene_scne_batch_site(0x00b1626c,label,frame,state);
            auto& bindings=main_scene_job_bindings(c);
            auto* const owner=get_native_frame_jobs_actual_004c1130(*c.jobs);
            auto* const pool=reinterpret_cast<NativeFrameJobPoolStorage*>(
                reinterpret_cast<std::uintptr_t>(owner)+4u);
            main_scene_scne_batch_site(0x00b1627c,label,frame,state);
            bindings.execution.require_dispatch_virtual_08(*pool);
            dispatch_native_frame_jobs_00be3150(*pool,1,bindings.scope,bindings.execution);
            frame.dispatch_completed=true;
        }
        for(std::uint32_t index=0;index<2;++index) {
            if(!parallel) {
                auto* const batch=main_scene_batch(command,index);
                main_scene_scne_batch_site(index==0?0x00b162b9:0x00b162e0,label,frame,state);
                auto& preparation=main_scene_preparation(c);
                const auto* const profile=main_scene_batch_profile(batch,preparation);
                if(profile[3]!=0x00b51df0)
                    throw std::logic_error("main scene requires current B51DF0 serial preparation");
                volatile std::uint32_t argument=index;
                prepare_native_render_batch_actual_00b51df0(*batch,argument,preparation);
                ++frame.serial_preparations_completed;
            }
            // The serial path reloads the batch AFTER its preparation; the
            // parallel path reloads after dispatch. Capture profile BEFORE
            // the camera getter and read its execution slot only afterward.
            auto* const batch=main_scene_batch(command,index);
            auto& preparation=main_scene_preparation(c);
            const auto* const captured_profile=main_scene_batch_profile(batch,preparation);
            main_scene_scne_batch_site(parallel?(index==0?0x00b16286:0x00b1629d):
                (index==0?0x00b162c6:0x00b162ed),label,frame,state);
            void* const camera=get_native_scene_command_camera_00b1bf40(command);
            const std::uint32_t target=captured_profile[2];
            main_scene_scne_batch_site(parallel?(index==0?0x00b16293:0x00b162aa):
                (index==0?0x00b162d4:0x00b162fc),label,frame,state);
            if(target!=0x00b55550 || !c.entries ||
               &c.entries->actual_renderer_00f8d394!=system.actual_renderer_00f8d394)
                throw std::logic_error("main scene requires its same actual B55550 material-entry domain");
            execute_native_render_batch_entries_00b55550(batch,index,camera,*c.entries,frame.entries[index]);
            ++frame.executions_completed;
        }
        main_scene_scne_batch_site(0x00b162fe,label,frame,state);
        frame.completed=true;
        state.reached_scne_capture_stage=true;
    } catch(...) {
        frame.failed=true;
        label.failed=true;
        unwind_native_main_scene_scne_label(scne.label,label);
        throw;
    }
}
namespace {
struct SurfaceCopyRectangle {
    std::int32_t left, top;
    std::uint32_t right, bottom;
};
using SurfaceGetDevice = std::int32_t (__stdcall*)(void*, void**);
using DeviceStretchRectangle = std::int32_t (__stdcall*)(void*, void*,
    const SurfaceCopyRectangle*, void*, const SurfaceCopyRectangle*, std::uint32_t);
using SurfaceCopyRelease = std::uint32_t (__stdcall*)(void*);
const volatile std::uint32_t* surface_copy_profile(void* surface,
    const NativeSurfaceCopyContext& c) {
    if(word(surface)!=0x00d619a0 || !c.actual_surface_profile_00d619a0)
        throw std::logic_error("surface copy requires current actual D619A0 profile");
    return c.actual_surface_profile_00d619a0;
}
} // namespace

void copy_native_surface_00b3cda0(void* destination,void* source,
    const NativeSurfaceCopyContext& c) {
    void* captured_device=nullptr;
    void* const initial_destination=current_member(destination,0x2c);
    auto get_device=reinterpret_cast<SurfaceGetDevice>(word(
        static_cast<const unsigned char*>(current_member(initial_destination,0))+0x0c));
    (void)get_device(initial_destination,&captured_device);
    const auto* width_profile=surface_copy_profile(destination,c);
    const std::uint32_t width_target=width_profile[0x1c/4];
    SurfaceCopyRectangle rectangle{};
    if(width_target!=0x00b3cd10)
        throw std::logic_error("surface copy requires current B3CD10 width");
    const std::uint32_t width=get_native_surface_width_00b3cd10(destination);
    const auto* height_profile=surface_copy_profile(destination,c);
    const std::uint32_t height_target=height_profile[0x20/4];
    rectangle.right=width;
    if(height_target!=0x00b3cd20)
        throw std::logic_error("surface copy requires current B3CD20 height");
    const std::uint32_t height=get_native_surface_height_00b3cd20(destination);
    void* const destination_surface=current_member(destination,0x2c);
    void* const source_surface=current_member(source,0x2c);
    rectangle.bottom=height;
    auto stretch=reinterpret_cast<DeviceStretchRectangle>(word(
        static_cast<const unsigned char*>(current_member(captured_device,0))+0x88));
    (void)stretch(captured_device,source_surface,nullptr,destination_surface,&rectangle,2);
    auto release=reinterpret_cast<SurfaceCopyRelease>(word(
        static_cast<const unsigned char*>(current_member(captured_device,0))+8));
    (void)release(captured_device);
}

void capture_native_render_holder_00b4caf0(void* holder,const NativeSurfaceCopyContext& c) {
    if(*reinterpret_cast<const volatile std::uint8_t*>(static_cast<const unsigned char*>(holder)+0x14)==0)
        return;
    void* const source=current_member(holder,0x0c);
    void* const destination=current_member(holder,0x10);
    copy_native_surface_00b3cda0(destination,source,c);
}

void continue_native_main_scene_scne_capture_00b162fe(void* service,
    const NativeMainSceneScneCaptureContext& c,NativeMainSceneScneLabelFrame& label,
    NativeMainSceneScneCaptureFrame& frame,NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_scne_capture_stage || state.reached_post_label_stage ||
       !label.completed || label.failed || label.native_cleanup_state!=5 ||
       label.cleanup_started || frame.started)
        throw std::logic_error("SCNE capture requires its fresh reached state5 frame");
    frame.started=true;
    const auto site=[&](std::uint32_t at) noexcept {
        label.active_native_site=frame.active_native_site=state.active_native_site=at;
    };
    site(0x00b162fe);
    try {
        const auto& system=c.label.domains.system;
        auto& textures=c.textures;
        auto& notification=textures.actual_texture_2d_owner.renderer_notification;
        if(!system.actual_renderer_00f8d394 || !system.actual_service_00f8d39c ||
           system.actual_synchronization!=&textures.actual_synchronization_0108d6dc ||
           static_cast<const volatile void*>(system.actual_renderer_00f8d394)!=
               static_cast<const volatile void*>(&notification.actual_renderer_00f8d394) ||
           &notification!=&textures.actual_cube_owner.renderer_notification ||
           &notification!=&textures.actual_volume_owner.renderer_notification ||
           system.actual_manager_01090aa0!=&c.label.domains.strings.actual_manager_publication_01090aa0)
            throw std::logic_error("SCNE capture requires the same actual renderer and label domains");
        const auto selected=*reinterpret_cast<const volatile std::uint8_t*>(
            static_cast<const unsigned char*>(service)+0x22d);
        void* const holder=current_member(service,selected?0x48:0x44);
        site(0x00b1630f);
        capture_native_render_holder_00b4caf0(holder,c.surface_copy);
        frame.capture_completed=true;
        for(std::uint32_t sampler=0;sampler<8;++sampler) {
            void* const renderer=*system.actual_renderer_00f8d394;
            const std::uint32_t profile=word(renderer);
            site(0x00b16330);
            if(profile!=0x00d5f0a8 || !c.actual_renderer_profile_00d5f0a8 ||
               c.actual_renderer_profile_00d5f0a8[0x130/4]!=0x00b24710)
                throw std::logic_error("SCNE capture requires current D5F0A8:B24710 binding");
            bind_native_renderer_texture_00b24710(renderer,sampler,nullptr,textures);
            ++frame.samplers_completed;
        }
        site(0x00b1633e);
        label.native_cleanup_state=-1;
        label.cleanup_started=true;
        label.cleanup_native_site=0x00b16349;
        site(0x00b16349);
        reset_native_render_diagnostic_label_00b13510(*system.actual_service_00f8d39c,c.label.domains.strings);
        label.cleanup_completed=frame.label_reset_completed=true;
        site(0x00b1634e);
        frame.completed=true;
        state.reached_post_label_stage=true;
    } catch(...) {
        frame.failed=true;label.failed=true;
        unwind_native_main_scene_scne_label(c.label,label);
        throw;
    }
}
void unwind_native_main_scene_post_label(const NativeMainScenePostLabelContext& c,
    NativeMainScenePostLabelFrame& frame) noexcept {
    const auto cleanup_state=frame.native_cleanup_state;
    if(cleanup_state==-1) return;
    if((cleanup_state!=6 && cleanup_state!=8) || frame.cleanup_started) std::terminate();
    frame.native_cleanup_state=-1;
    frame.cleanup_started=true;
    try {
        if(cleanup_state==6) {
            frame.cleanup_native_site=0x00cbc486;
            destroy_native_string_header_0041dd20(frame.temporary_header,c.domains.strings);
        } else {
            frame.cleanup_native_site=0x00cbc491;
            reset_native_render_diagnostic_label_00b13510(*c.domains.system.actual_service_00f8d39c,c.domains.strings);
        }
        frame.cleanup_completed=true;
    } catch(...) { std::terminate(); }
}

void continue_native_main_scene_post_label_00b1634e(void* original_service,
    const NativeMainScenePostLabelContext& c,NativeMainScenePostLabelFrame& frame,
    NativeMainSceneExecutionPrefixState& state) {
    if(!state.reached_post_label_stage || state.reached_post_body_stage ||
       frame.started || frame.native_cleanup_state!=-1)
        throw std::logic_error("main scene POST requires its fresh reached label frame");
    frame.started=true;
    frame.active_native_site=state.active_native_site=0x00b16356;
    try {
        *reinterpret_cast<volatile std::uint8_t*>(static_cast<unsigned char*>(original_service)+0x680)=0;
        frame.service_flag_cleared=true;
        auto* const temporary=frame.temporary_header;
        *static_cast<volatile std::uint32_t*>(temporary)=0;
        *static_cast<volatile std::uint32_t*>(temporary+1)=0;
        frame.active_native_site=state.active_native_site=0x00b16365;
        if(!c.domains.system.actual_service_00f8d39c || !c.actual_post_bytes_00d5e4f0 ||
           c.domains.system.actual_manager_01090aa0!=&c.domains.strings.actual_manager_publication_01090aa0)
            throw std::logic_error("main scene POST requires the same actual service and raw string-manager domain");
        resize_native_string_header_0041dd40(temporary,c.domains.strings,4,true);
        void* const captured_data=current_member(temporary,4); // B1636A EBX.
        frame.captured_data=captured_data;
        if(captured_data) {
            const std::uint32_t bytes=word(temporary)+1u;
            frame.active_native_site=state.active_native_site=0x00b16380;
            if(bytes) std::memmove(captured_data,c.actual_post_bytes_00d5e4f0,bytes);
        }
        void* const service=*c.domains.system.actual_service_00f8d39c; // B16388.
        frame.captured_service=service;
        frame.native_cleanup_state=6; // B1638F, AFTER publication capture.
        if(service) {
            void* const destination_header=reinterpret_cast<void*>(
                reinterpret_cast<std::uintptr_t>(service)+0x684u);
            frame.destination_header=destination_header;
            if(destination_header!=temporary) {
                const std::uint32_t requested=word(temporary);
                frame.active_native_site=state.active_native_site=0x00b163b8;
                resize_native_string_header_0041dd40(destination_header,c.domains.strings,requested,true);
                if(word(temporary)!=0) {
                    const std::uint32_t bytes=word(destination_header);
                    void* const destination=current_member(destination_header,4);
                    frame.active_native_site=state.active_native_site=0x00b163cf;
                    if(bytes) std::memmove(destination,captured_data,bytes);
                }
            }
        }
        frame.native_cleanup_state=8; // B163D9, BEFORE normal pool getter/return.
        if(captured_data) {
            const std::uint32_t bytes=word(temporary)+1u;
            frame.temporary_return_started=true;
            frame.active_native_site=state.active_native_site=0x00b163ee;
            auto* const pool=native_string_pool_get_or_create_00419cc0(
                c.domains.strings.actual_published_01090aa8,c.domains.strings.actual_manager_publication_01090aa0);
            frame.active_native_site=state.active_native_site=0x00b163f5;
            return_native_string_pool_00bd1510(pool,captured_data,bytes,
                c.domains.strings.actual_small_returns_disabled_01090aa4);
            frame.temporary_return_completed=true;
        }
        frame.active_native_site=state.active_native_site=0x00b163fa;
        frame.completed=true;
        state.reached_post_body_stage=true;
    } catch(...) {
        frame.failed=true;
        unwind_native_main_scene_post_label(c,frame);
        throw;
    }
}
} // namespace bsp
