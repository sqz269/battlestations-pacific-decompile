#include "bsp/native_main_scene_execution_prefix.hpp"
#include "bsp/native_camera_history.hpp"
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
} // namespace bsp
