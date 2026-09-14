#include "bsp/native_system_constant_gather.hpp"
#include "bsp/native_camera_cache_getters.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_fog_access.hpp"
#include "bsp/native_ambient_color.hpp"
#include "bsp/native_light_environment_queries.hpp"
#include "bsp/native_instance_group_upload.hpp"
#include "bsp/native_shader_constants.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/material_constants.hpp"
#include <cstddef>
#include <cstdlib>
#include <stdexcept>

namespace bsp {
namespace {
using Context=NativeSystemConstantGatherContext;
using Frame=NativeSystemConstantGatherFrame;
static_assert(sizeof(Context)==68);
static_assert(offsetof(Frame,prefix_preimage)==0);
void require(bool condition) {
    if(!condition) throw std::logic_error("native system gather current virtual profile is unsupported");
}
void __fastcall matrix_bridge(float* destination,void*,const float* source) {
    write_system_matrix_00b404a0(destination,source);
}
const void* __fastcall service_matrix(const void* actual) {
    return get_system_camera_service_matrix_00b0d100(actual);
}
const void* __fastcall service_region(const void* actual) {
    return render_time_region_00b0cf30(actual);
}
const void* __fastcall fog_directional(const void* owner,void*,std::uint32_t index) {
    return get_native_fog_directional_color_00b84fd0(owner,index);
}
void* __fastcall particle_bridge(const Context* c,Frame* frame) {
    return get_native_sampler_loader_singleton_004de4b0(*c->actual_manager_01090aa0,
        *c->actual_sampler_00f8d420,frame->sampler);
}
const void* __fastcall timer_bridge(const void* timer,const Context* c,std::uint32_t captured_profile) {
    require(captured_profile==0x00d68d50 &&
        c->actual_timer_profile_00d68d50 && c->actual_timer_profile_00d68d50[7]==0x00bee070);
    return get_raw_timer_interval_00bee070(timer);
}
void* __fastcall shadow_bridge(const void* shadow,const Context* c,std::uint32_t captured_profile) {
    require(captured_profile==0x00d5b5d8 &&
        c->actual_shadow_profile_00d5b5d8 && c->actual_shadow_profile_00d5b5d8[2]==0x00a8fcf0);
    return get_raw_shadow_depth_texture_00a8fcf0(shadow,c->actual_shadow_target_00f8bbf0);
}
std::uint32_t __fastcall width_bridge(const void* texture,const Context* c,std::uint32_t captured_profile) {
    require(captured_profile==0x00d61948 &&
        c->actual_texture_profile_00d61948 && c->actual_texture_profile_00d61948[15]==0x00b3ce50);
    return get_raw_texture_width_00b3ce50(texture);
}
std::uint32_t __fastcall height_bridge(const void* texture,const Context* c,std::uint32_t captured_profile) {
    require(captured_profile==0x00d61948 &&
        c->actual_texture_profile_00d61948 && c->actual_texture_profile_00d61948[16]==0x00b3ce60);
    return get_raw_texture_height_00b3ce60(texture);
}
}

__declspec(naked) void* __fastcall get_raw_shadow_depth_texture_00a8fcf0(const void*,void* const volatile*) {
    __asm {
        mov eax,[edx]
        cmp byte ptr [eax+0ch],0
        jz fallback
        cmp byte ptr [eax+0dh],0
        jz fallback
        mov ecx,eax
        jmp get_raw_shadow_target_depth_00a8fdb0
    fallback:
        mov eax,[ecx+384h]
        ret
    }
}
__declspec(naked) void* __fastcall get_raw_shadow_color_texture_00a8fd10(const void*,void* const volatile*) {
    __asm {
        mov eax,[edx]
        cmp byte ptr [eax+0ch],0
        jz fallback
        cmp byte ptr [eax+0dh],0
        jz fallback
        mov ecx,eax
        jmp get_raw_shadow_target_color_00a8fd90
    fallback:
        mov eax,[ecx+384h]
        ret
    }
}
__declspec(naked) void* __fastcall get_raw_shadow_target_color_00a8fd90(const void*) noexcept {
    __asm { mov eax,[ecx+10h]
        ret
    }
}
__declspec(naked) void* __fastcall get_raw_shadow_target_depth_00a8fdb0(const void*) noexcept {
    __asm { mov eax,[ecx+18h]
        ret
    }
}
__declspec(naked) std::uint32_t __fastcall get_raw_texture_width_00b3ce50(const void*) noexcept {
    __asm { mov eax,[ecx+28h]
        ret
    }
}
__declspec(naked) std::uint32_t __fastcall get_raw_texture_height_00b3ce60(const void*) noexcept {
    __asm { mov eax,[ecx+2ch]
        ret
    }
}
__declspec(naked) float __fastcall get_raw_foliage_manager_time_00af0460(const void*) {
    __asm { fld dword ptr [ecx+2ch]
        ret
    }
}
__declspec(naked) float __fastcall get_raw_foliage_time_00ad5700(const void*) {
    __asm { fld dword ptr [ecx+8]
        ret
    }
}
__declspec(naked) std::uint8_t __fastcall get_raw_foliage_byte_00ad5740(const void*) noexcept {
    __asm { mov al,[ecx+11h]
        ret
    }
}
__declspec(naked) void* __fastcall get_raw_light_shadow_00b7aab0(const void*) noexcept {
    __asm { mov eax,[ecx+174h]
        ret
    }
}
__declspec(naked) const void* __fastcall get_raw_timer_interval_00bee070(const void*) noexcept {
    __asm { lea eax,[ecx+40h]
        ret
    }
}

// Full original instruction schedule. Added stack arguments only bind actual
// globals/providers and the initialized prefix preimage. Numeric game targets
// are never executed. The original prefix occupies ESP+14..ESP+4E3.
__declspec(naked) void __fastcall gather_native_system_constants_00b46a70(void*,void*,
    const NativeSystemConstantGatherContext&,NativeSystemConstantGatherFrame&) {
    __asm {
        sub esp,0x4d4 // 00b46a70
        push ebx // 00b46a76
        push ebp // 00b46a77
        push esi // 00b46a78
        mov ebp,ecx // 00b46a79
        mov ecx,dword ptr [esp+0x4e4]
        mov ecx,dword ptr [ecx+0]
        mov ecx,dword ptr [ecx+0] // 00b46a7b
        push edi // 00b46a81
        mov edi,edx // 00b46a82
        mov dword ptr [esp + 0x10],ebp // 00b46a84
        // Added bitwise copy of caller-supplied native prefix preimage.
        push ecx
        push edx
        push esi
        push edi
        mov eax,dword ptr [esp+4fch]
        mov esi,dword ptr [eax]
        lea edi,[esp+24h]
        mov ecx,134h
        rep movsd
        pop edi
        pop esi
        pop edx
        pop ecx
        call service_matrix // 00b46a88
        push eax // 00b46a8d
        lea ecx,[esp + 0x18] // 00b46a8e
        call matrix_bridge // 00b46a92
        mov bl,0x2 // 00b46a97
        test byte ptr [edi + 0x5c],bl // 00b46a99
        jnz l_00b46aa5 // 00b46a9c
        mov ecx,edi // 00b46a9e
        call refresh_native_camera_world_00b6db70 // 00b46aa0
    l_00b46aa5:
        movss xmm0,dword ptr [edi + 0x120] // 00b46aa5
        movss xmm1,dword ptr [edi + 0x124] // 00b46aad
        movss xmm2,dword ptr [edi + 0x128] // 00b46ab5
        mov ecx,edi // 00b46abd
        movss dword ptr [esp + 0x74],xmm0 // 00b46abf
        movss dword ptr [esp + 0x78],xmm1 // 00b46ac5
        movss dword ptr [esp + 0x7c],xmm2 // 00b46acb
        call get_native_camera_view_00b6fcb0 // 00b46ad1
        push eax // 00b46ad6
        lea ecx,[esp + 0x88] // 00b46ad7
        call matrix_bridge // 00b46ade
        test byte ptr [edi + 0x5c],bl // 00b46ae3
        jnz l_00b46aef // 00b46ae6
        mov ecx,edi // 00b46ae8
        call refresh_native_camera_world_00b6db70 // 00b46aea
    l_00b46aef:
        lea eax,[edi + 0xf0] // 00b46aef
        push eax // 00b46af5
        lea ecx,[esp + 0xc8] // 00b46af6
        call matrix_bridge // 00b46afd
        mov ecx,edi // 00b46b02
        call get_native_camera_view_projection_00b70490 // 00b46b04
        push eax // 00b46b09
        lea ecx,[esp + 0x108] // 00b46b0a
        call matrix_bridge // 00b46b11
        mov ecx,edi // 00b46b16
        call get_native_camera_projection_00b6fcf0 // 00b46b18
        push eax // 00b46b1d
        lea ecx,[esp + 0x188] // 00b46b1e
        call matrix_bridge // 00b46b25
        mov ecx,edi // 00b46b2a
        call get_native_camera_inverse_view_projection_00b70510 // 00b46b2c
        push eax // 00b46b31
        lea ecx,[esp + 0x148] // 00b46b32
        call matrix_bridge // 00b46b39
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+0]
        pop eax // 00b46b3e
        movss dword ptr [esp + 0x1c4],xmm0 // 00b46b46
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+4]
        pop eax // 00b46b4f
        movss dword ptr [esp + 0x1c8],xmm0 // 00b46b57
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+8]
        pop eax // 00b46b60
        movss dword ptr [esp + 0x1cc],xmm0 // 00b46b68
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+12]
        pop eax // 00b46b71
        movss dword ptr [esp + 0x1d0],xmm0 // 00b46b79
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+16]
        pop eax // 00b46b82
        movss dword ptr [esp + 0x1d4],xmm0 // 00b46b8a
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+20]
        pop eax // 00b46b93
        movss dword ptr [esp + 0x1d8],xmm0 // 00b46b9b
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+24]
        pop eax // 00b46ba4
        movss dword ptr [esp + 0x1dc],xmm0 // 00b46bac
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+28]
        pop eax // 00b46bb5
        movss dword ptr [esp + 0x1e0],xmm0 // 00b46bbd
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+32]
        pop eax // 00b46bc6
        movss dword ptr [esp + 0x1e4],xmm0 // 00b46bce
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+36]
        pop eax // 00b46bd7
        movss dword ptr [esp + 0x1e8],xmm0 // 00b46bdf
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+40]
        pop eax // 00b46be8
        movss dword ptr [esp + 0x1ec],xmm0 // 00b46bf0
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+44]
        pop eax // 00b46bf9
        movss dword ptr [esp + 0x1f0],xmm0 // 00b46c01
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+48]
        pop eax // 00b46c0a
        movss dword ptr [esp + 0x1f4],xmm0 // 00b46c12
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+52]
        pop eax // 00b46c1b
        movss dword ptr [esp + 0x1f8],xmm0 // 00b46c23
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+56]
        pop eax // 00b46c2c
        movss dword ptr [esp + 0x1fc],xmm0 // 00b46c34
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+28]
        movss xmm0,dword ptr [eax+60]
        pop eax // 00b46c3d
        mov ecx,edi // 00b46c45
        movss dword ptr [esp + 0x200],xmm0 // 00b46c47
        mov edx,dword ptr [esp+4e8h]
        mov edx,dword ptr [edx+44]
        call get_native_camera_axis_x_00b70fe0 // 00b46c50
        movss xmm0,dword ptr [eax] // 00b46c55
        movss dword ptr [esp + 0x204],xmm0 // 00b46c59
        movss xmm0,dword ptr [eax + 0x4] // 00b46c62
        movss dword ptr [esp + 0x208],xmm0 // 00b46c67
        movss xmm0,dword ptr [eax + 0x8] // 00b46c70
        mov ecx,edi // 00b46c75
        movss dword ptr [esp + 0x20c],xmm0 // 00b46c77
        mov edx,dword ptr [esp+4e8h]
        mov edx,dword ptr [edx+44]
        call get_native_camera_axis_y_00b70ea0 // 00b46c80
        movss xmm0,dword ptr [eax] // 00b46c85
        mov ecx,dword ptr [esp+0x4e8]
        mov ecx,dword ptr [ecx+8]
        mov ecx,dword ptr [ecx+0] // 00b46c89
        movss dword ptr [esp + 0x214],xmm0 // 00b46c8f
        movss xmm0,dword ptr [eax + 0x4] // 00b46c98
        movss dword ptr [esp + 0x218],xmm0 // 00b46c9d
        movss xmm0,dword ptr [eax + 0x8] // 00b46ca6
        movss dword ptr [esp + 0x21c],xmm0 // 00b46cab
        movss xmm0,dword ptr [ecx + 0x4] // 00b46cb4
        movss dword ptr [esp + 0x224],xmm0 // 00b46cb9
        mov ecx,dword ptr [esp+4e8h]
        mov edx,dword ptr [esp+4ech]
        call particle_bridge // 00b46cc2
        movss xmm0,dword ptr [eax + 0x18] // 00b46cc7
        mov ecx,dword ptr [esp+0x4e8]
        mov ecx,dword ptr [ecx+20]
        mov ecx,dword ptr [ecx+0] // 00b46ccc
        movss dword ptr [esp + 0x228],xmm0 // 00b46cd2
        call get_raw_foliage_manager_time_00af0460 // 00b46cdb
        fstp float ptr [esp + 0x22c] // 00b46ce0
        mov ecx,dword ptr [esp+0x4e8]
        mov ecx,dword ptr [ecx+24]
        mov ecx,dword ptr [ecx+0] // 00b46ce7
        call get_raw_foliage_time_00ad5700 // 00b46ced
        fstp float ptr [esp + 0x230] // 00b46cf2
        mov ecx,dword ptr [esp+0x4e8]
        mov ecx,dword ptr [ecx+8]
        mov ecx,dword ptr [ecx+0] // 00b46cf9
        mov edx,dword ptr [ecx] // 00b46cff
        // Numeric table target is read and checked through the borrowed profile in the following bridge. // 00b46d01
        push edx // Preserve the original one-time profile capture; bridge RET4.
        mov edx,dword ptr [esp+4ech]
        call timer_bridge // 00b46d04
        fild qword ptr [eax] // 00b46d06
        fild qword ptr [eax + 0x8] // 00b46d08
        mov ecx,dword ptr [esp+0x4e8]
        mov ecx,dword ptr [ecx+0]
        mov ecx,dword ptr [ecx+0] // 00b46d0b
        fdivp st(1),st(0) // 00b46d11
        fstp float ptr [esp + 0x234] // 00b46d13
        call service_region // 00b46d1a
        movzx ecx,byte ptr [eax + 0x24] // 00b46d1f
        cvtsi2ss xmm0,ecx // 00b46d23
        mov ecx,dword ptr [esp+0x4e8]
        mov ecx,dword ptr [ecx+0]
        mov ecx,dword ptr [ecx+0] // 00b46d27
        movss dword ptr [esp + 0x238],xmm0 // 00b46d2d
        call service_region // 00b46d36
        fld float ptr [edi + 0x1c4] // 00b46d3b
        fld1 // 00b46d41
        movss xmm0,dword ptr [eax + 0x28] // 00b46d43
        fdivrp st(1),st(0) // 00b46d48
        movzx edx,byte ptr [edi + 0x174] // 00b46d4a
        mov ecx,dword ptr [esp+0x4e8]
        mov ecx,dword ptr [ecx+24]
        mov ecx,dword ptr [ecx+0] // 00b46d51
        movss dword ptr [esp + 0x23c],xmm0 // 00b46d57
        cvtsi2ss xmm0,edx // 00b46d60
        movss dword ptr [esp + 0x4c4],xmm0 // 00b46d64
        fstp float ptr [esp + 0x240] // 00b46d6d
        call get_raw_foliage_byte_00ad5740 // 00b46d74
        mov esi,dword ptr [edi + 0x184] // 00b46d79
        test esi,esi // 00b46d7f
        movzx eax,al // 00b46d81
        cvtsi2ss xmm0,eax // 00b46d84
        movss dword ptr [esp + 0x4c8],xmm0 // 00b46d88
        jz l_00b46ed1 // 00b46d91
        mov ecx,esi // 00b46d97
        call load_native_fog_scalar_80_00b84da0 // 00b46d99
        fstp float ptr [esp + 0x494] // 00b46d9e
        mov ecx,esi // 00b46da5
        call load_native_fog_scalar_84_00b84db0 // 00b46da7
        fstp float ptr [esp + 0x498] // 00b46dac
        mov ecx,esi // 00b46db3
        call load_native_fog_scalar_88_00b84e20 // 00b46db5
        fstp float ptr [esp + 0x4a4] // 00b46dba
        mov ecx,esi // 00b46dc1
        call load_native_fog_scalar_8c_00b84e30 // 00b46dc3
        fstp float ptr [esp + 0x4a8] // 00b46dc8
        mov ecx,esi // 00b46dcf
        call load_native_fog_scalar_90_00b84e40 // 00b46dd1
        fstp float ptr [esp + 0x4ac] // 00b46dd6
        mov ecx,esi // 00b46ddd
        call load_native_fog_scalar_6c_00b84cb0 // 00b46ddf
        fstp float ptr [esp + 0x244] // 00b46de4
        mov ecx,esi // 00b46deb
        call load_native_fog_scalar_70_00b84cc0 // 00b46ded
        fstp float ptr [esp + 0x248] // 00b46df2
        mov ecx,esi // 00b46df9
        call load_native_fog_scalar_68_00b84ca0 // 00b46dfb
        fstp float ptr [esp + 0x24c] // 00b46e00
        mov ecx,esi // 00b46e07
        call load_native_fog_scalar_74_00b84cd0 // 00b46e09
        fstp float ptr [esp + 0x254] // 00b46e0e
        mov ecx,esi // 00b46e15
        call load_native_fog_scalar_78_00b84ce0 // 00b46e17
        fstp float ptr [esp + 0x258] // 00b46e1c
        mov ecx,esi // 00b46e23
        call load_native_fog_scalar_7c_00b84cf0 // 00b46e25
        fstp float ptr [esp + 0x25c] // 00b46e2a
        xor ebp,ebp // 00b46e31
        lea ebx,[esp + 0x274] // 00b46e33
        lea ebx,[ebx] // 00b46e3a
    l_00b46e40:
        push ebp // 00b46e40
        mov ecx,esi // 00b46e41
        call fog_directional // 00b46e43
        mov ecx,dword ptr [eax] // 00b46e48
        mov dword ptr [ebx],ecx // 00b46e4a
        mov edx,dword ptr [eax + 0x4] // 00b46e4c
        mov dword ptr [ebx + 0x4],edx // 00b46e4f
        mov ecx,dword ptr [eax + 0x8] // 00b46e52
        mov dword ptr [ebx + 0x8],ecx // 00b46e55
        mov edx,dword ptr [eax + 0xc] // 00b46e58
        mov dword ptr [ebx + 0xc],edx // 00b46e5b
        add ebp,0x1 // 00b46e5e
        add ebx,0x10 // 00b46e61
        cmp ebp,0x4 // 00b46e64
        jc l_00b46e40 // 00b46e67
        mov ecx,dword ptr [edi + 0x184] // 00b46e69
        call native_ambient_color_address_00b84c60 // 00b46e6f
        mov ecx,dword ptr [eax] // 00b46e74
        mov dword ptr [esp + 0x264],ecx // 00b46e76
        mov edx,dword ptr [eax + 0x4] // 00b46e7d
        mov dword ptr [esp + 0x268],edx // 00b46e80
        mov ecx,dword ptr [eax + 0x8] // 00b46e87
        mov dword ptr [esp + 0x26c],ecx // 00b46e8a
        mov edx,dword ptr [eax + 0xc] // 00b46e91
        mov ecx,dword ptr [edi + 0x184] // 00b46e94
        mov dword ptr [esp + 0x270],edx // 00b46e9a
        call get_native_fog_underwater_color_00b84c90 // 00b46ea1
        mov ecx,dword ptr [eax] // 00b46ea6
        mov ebp,dword ptr [esp + 0x10] // 00b46ea8
        mov dword ptr [esp + 0x4b4],ecx // 00b46eac
        mov edx,dword ptr [eax + 0x4] // 00b46eb3
        mov dword ptr [esp + 0x4b8],edx // 00b46eb6
        mov ecx,dword ptr [eax + 0x8] // 00b46ebd
        mov dword ptr [esp + 0x4bc],ecx // 00b46ec0
        mov edx,dword ptr [eax + 0xc] // 00b46ec7
        mov dword ptr [esp + 0x4c0],edx // 00b46eca
    l_00b46ed1:
        test ebp,ebp // 00b46ed1
        jz l_00b475b4 // 00b46ed3
        mov ecx,ebp // 00b46ed9
        call native_scene_lighting_owner_00b72110 // 00b46edb
        mov esi,eax // 00b46ee0
        test esi,esi // 00b46ee2
        jz l_00b475b4 // 00b46ee4
        mov eax,dword ptr [esi + 0x1c] // 00b46eea
        mov ebx,dword ptr [eax] // 00b46eed
        cmp ebx,eax // 00b46eef
        jnz l_00b46ef8 // 00b46ef1
        call _invalid_parameter_noinfo // 00b46ef3
    l_00b46ef8:
        cmp dword ptr [edi + 0x198],0x3 // 00b46ef8
        mov ecx,dword ptr [esi + 0x10] // 00b46eff
        mov ebx,dword ptr [ebx + 0x8] // 00b46f02
        mov dword ptr [esp + 0x10],ecx // 00b46f05
        jnz l_00b46f66 // 00b46f09
        call get_native_light_environment_mode3_ambient_00b7aa30 // 00b46f0b
        mov ecx,dword ptr [eax] // 00b46f10
        mov dword ptr [esp + 0x2c4],ecx // 00b46f12
        mov edx,dword ptr [eax + 0x4] // 00b46f19
        mov dword ptr [esp + 0x2c8],edx // 00b46f1c
        mov ecx,dword ptr [eax + 0x8] // 00b46f23
        mov dword ptr [esp + 0x2cc],ecx // 00b46f26
        mov edx,dword ptr [eax + 0xc] // 00b46f2d
        mov dword ptr [esp + 0x2d0],edx // 00b46f30
        mov eax,dword ptr [ebx + 0x1b4] // 00b46f37
        mov dword ptr [esp + 0x2d4],eax // 00b46f3d
        mov ecx,dword ptr [ebx + 0x1b8] // 00b46f44
        mov dword ptr [esp + 0x2d8],ecx // 00b46f4a
        mov edx,dword ptr [ebx + 0x1bc] // 00b46f51
        mov dword ptr [esp + 0x2dc],edx // 00b46f57
        mov eax,dword ptr [ebx + 0x1c0] // 00b46f5e
        jmp l_00b46fbf // 00b46f64
    l_00b46f66:
        call get_native_light_environment_ambient_00b7aa20 // 00b46f66
        mov ecx,dword ptr [eax] // 00b46f6b
        mov dword ptr [esp + 0x2c4],ecx // 00b46f6d
        mov edx,dword ptr [eax + 0x4] // 00b46f74
        mov dword ptr [esp + 0x2c8],edx // 00b46f77
        mov ecx,dword ptr [eax + 0x8] // 00b46f7e
        mov dword ptr [esp + 0x2cc],ecx // 00b46f81
        mov edx,dword ptr [eax + 0xc] // 00b46f88
        mov dword ptr [esp + 0x2d0],edx // 00b46f8b
        mov eax,dword ptr [ebx + 0x184] // 00b46f92
        mov dword ptr [esp + 0x2d4],eax // 00b46f98
        mov ecx,dword ptr [ebx + 0x188] // 00b46f9f
        mov dword ptr [esp + 0x2d8],ecx // 00b46fa5
        mov edx,dword ptr [ebx + 0x18c] // 00b46fac
        mov dword ptr [esp + 0x2dc],edx // 00b46fb2
        mov eax,dword ptr [ebx + 0x190] // 00b46fb9
    l_00b46fbf:
        mov dword ptr [esp + 0x2e0],eax // 00b46fbf
        mov ecx,dword ptr [ebx + 0x194] // 00b46fc6
        mov dword ptr [esp + 0x2e4],ecx // 00b46fcc
        mov edx,dword ptr [ebx + 0x198] // 00b46fd3
        mov dword ptr [esp + 0x2e8],edx // 00b46fd9
        mov eax,dword ptr [ebx + 0x19c] // 00b46fe0
        mov dword ptr [esp + 0x2ec],eax // 00b46fe6
        mov ecx,dword ptr [ebx + 0x1a0] // 00b46fed
        mov dword ptr [esp + 0x2f0],ecx // 00b46ff3
        movss xmm0,dword ptr [ebx + 0x1e0] // 00b46ffa
        movss dword ptr [esp + 0x354],xmm0 // 00b47002
        movss xmm0,dword ptr [ebx + 0x1e4] // 00b4700b
        movss dword ptr [esp + 0x358],xmm0 // 00b47013
        movss xmm0,dword ptr [ebx + 0x1e8] // 00b4701c
        movss dword ptr [esp + 0x35c],xmm0 // 00b47024
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+36]
        movss xmm0,dword ptr [eax+0]
        pop eax // 00b4702d
        movss dword ptr [esp + 0x2b4],xmm0 // 00b47035
        xor ebp,ebp // 00b4703e
        lea esi,[esp + 0x2f4] // 00b47040
        jmp l_00b47050 // 00b47047
    l_00b47050:
        mov ecx,dword ptr [esp + 0x10] // 00b47050
        push ebp // 00b47054
        call get_native_light_environment_cube_face_00b7aa40 // 00b47055
        mov edx,dword ptr [eax] // 00b4705a
        mov dword ptr [esi],edx // 00b4705c
        mov ecx,dword ptr [eax + 0x4] // 00b4705e
        mov dword ptr [esi + 0x4],ecx // 00b47061
        mov edx,dword ptr [eax + 0x8] // 00b47064
        mov dword ptr [esi + 0x8],edx // 00b47067
        mov eax,dword ptr [eax + 0xc] // 00b4706a
        mov dword ptr [esi + 0xc],eax // 00b4706d
        add ebp,0x1 // 00b47070
        add esi,0x10 // 00b47073
        cmp ebp,0x6 // 00b47076
        jc l_00b47050 // 00b47079
        mov ecx,ebx // 00b4707b
        call get_raw_light_shadow_00b7aab0 // 00b4707d
        mov esi,eax // 00b47082
        test esi,esi // 00b47084
        jz l_00b475b4 // 00b47086
        movss xmm0,dword ptr [esi + 0x144] // 00b4708c
        movss dword ptr [esp + 0x364],xmm0 // 00b47094
        movss xmm0,dword ptr [esi + 0x154] // 00b4709d
        movss dword ptr [esp + 0x368],xmm0 // 00b470a5
        movss xmm0,dword ptr [esi + 0x164] // 00b470ae
        movss dword ptr [esp + 0x36c],xmm0 // 00b470b6
        movss xmm0,dword ptr [esi + 0x174] // 00b470bf
        movss dword ptr [esp + 0x370],xmm0 // 00b470c7
        movss xmm0,dword ptr [esi + 0x148] // 00b470d0
        movss dword ptr [esp + 0x374],xmm0 // 00b470d8
        movss xmm0,dword ptr [esi + 0x158] // 00b470e1
        movss dword ptr [esp + 0x378],xmm0 // 00b470e9
        movss xmm0,dword ptr [esi + 0x168] // 00b470f2
        movss dword ptr [esp + 0x37c],xmm0 // 00b470fa
        movss xmm0,dword ptr [esi + 0x178] // 00b47103
        movss dword ptr [esp + 0x380],xmm0 // 00b4710b
        movss xmm0,dword ptr [esi + 0x14c] // 00b47114
        movss dword ptr [esp + 0x384],xmm0 // 00b4711c
        movss xmm0,dword ptr [esi + 0x15c] // 00b47125
        movss dword ptr [esp + 0x388],xmm0 // 00b4712d
        movss xmm0,dword ptr [esi + 0x16c] // 00b47136
        movss dword ptr [esp + 0x38c],xmm0 // 00b4713e
        movss xmm0,dword ptr [esi + 0x17c] // 00b47147
        movss dword ptr [esp + 0x390],xmm0 // 00b4714f
        movss xmm0,dword ptr [esi + 0x150] // 00b47158
        movss dword ptr [esp + 0x394],xmm0 // 00b47160
        movss xmm0,dword ptr [esi + 0x160] // 00b47169
        movss dword ptr [esp + 0x398],xmm0 // 00b47171
        movss xmm0,dword ptr [esi + 0x170] // 00b4717a
        movss dword ptr [esp + 0x39c],xmm0 // 00b47182
        movss xmm0,dword ptr [esi + 0x180] // 00b4718b
        movss dword ptr [esp + 0x3a0],xmm0 // 00b47193
        movss xmm0,dword ptr [esi + 0x184] // 00b4719c
        movss dword ptr [esp + 0x3a4],xmm0 // 00b471a4
        movss xmm0,dword ptr [esi + 0x194] // 00b471ad
        movss dword ptr [esp + 0x3a8],xmm0 // 00b471b5
        movss xmm0,dword ptr [esi + 0x1a4] // 00b471be
        movss dword ptr [esp + 0x3ac],xmm0 // 00b471c6
        movss xmm0,dword ptr [esi + 0x1b4] // 00b471cf
        movss dword ptr [esp + 0x3b0],xmm0 // 00b471d7
        movss xmm0,dword ptr [esi + 0x188] // 00b471e0
        movss dword ptr [esp + 0x3b4],xmm0 // 00b471e8
        movss xmm0,dword ptr [esi + 0x198] // 00b471f1
        movss dword ptr [esp + 0x3b8],xmm0 // 00b471f9
        movss xmm0,dword ptr [esi + 0x1a8] // 00b47202
        movss dword ptr [esp + 0x3bc],xmm0 // 00b4720a
        movss xmm0,dword ptr [esi + 0x1b8] // 00b47213
        movss dword ptr [esp + 0x3c0],xmm0 // 00b4721b
        movss xmm0,dword ptr [esi + 0x18c] // 00b47224
        movss dword ptr [esp + 0x3c4],xmm0 // 00b4722c
        movss xmm0,dword ptr [esi + 0x19c] // 00b47235
        movss dword ptr [esp + 0x3c8],xmm0 // 00b4723d
        movss xmm0,dword ptr [esi + 0x1ac] // 00b47246
        movss dword ptr [esp + 0x3cc],xmm0 // 00b4724e
        movss xmm0,dword ptr [esi + 0x1bc] // 00b47257
        movss dword ptr [esp + 0x3d0],xmm0 // 00b4725f
        movss xmm0,dword ptr [esi + 0x190] // 00b47268
        movss dword ptr [esp + 0x3d4],xmm0 // 00b47270
        movss xmm0,dword ptr [esi + 0x1a0] // 00b47279
        movss dword ptr [esp + 0x3d8],xmm0 // 00b47281
        movss xmm0,dword ptr [esi + 0x1b0] // 00b4728a
        movss dword ptr [esp + 0x3dc],xmm0 // 00b47292
        movss xmm0,dword ptr [esi + 0x1c0] // 00b4729b
        movss dword ptr [esp + 0x3e0],xmm0 // 00b472a3
        movss xmm0,dword ptr [esi + 0x1c4] // 00b472ac
        movss dword ptr [esp + 0x3e4],xmm0 // 00b472b4
        movss xmm0,dword ptr [esi + 0x1d4] // 00b472bd
        movss dword ptr [esp + 0x3e8],xmm0 // 00b472c5
        movss xmm0,dword ptr [esi + 0x1e4] // 00b472ce
        movss dword ptr [esp + 0x3ec],xmm0 // 00b472d6
        movss xmm0,dword ptr [esi + 0x1f4] // 00b472df
        movss dword ptr [esp + 0x3f0],xmm0 // 00b472e7
        movss xmm0,dword ptr [esi + 0x1c8] // 00b472f0
        movss dword ptr [esp + 0x3f4],xmm0 // 00b472f8
        movss xmm0,dword ptr [esi + 0x1d8] // 00b47301
        movss dword ptr [esp + 0x3f8],xmm0 // 00b47309
        movss xmm0,dword ptr [esi + 0x1e8] // 00b47312
        movss dword ptr [esp + 0x3fc],xmm0 // 00b4731a
        movss xmm0,dword ptr [esi + 0x1f8] // 00b47323
        movss dword ptr [esp + 0x400],xmm0 // 00b4732b
        movss xmm0,dword ptr [esi + 0x1cc] // 00b47334
        movss dword ptr [esp + 0x404],xmm0 // 00b4733c
        movss xmm0,dword ptr [esi + 0x1dc] // 00b47345
        movss dword ptr [esp + 0x408],xmm0 // 00b4734d
        movss xmm0,dword ptr [esi + 0x1ec] // 00b47356
        movss dword ptr [esp + 0x40c],xmm0 // 00b4735e
        movss xmm0,dword ptr [esi + 0x1fc] // 00b47367
        movss dword ptr [esp + 0x410],xmm0 // 00b4736f
        movss xmm0,dword ptr [esi + 0x1d0] // 00b47378
        movss dword ptr [esp + 0x414],xmm0 // 00b47380
        movss xmm0,dword ptr [esi + 0x1e0] // 00b47389
        movss dword ptr [esp + 0x418],xmm0 // 00b47391
        movss xmm0,dword ptr [esi + 0x1f0] // 00b4739a
        movss dword ptr [esp + 0x41c],xmm0 // 00b473a2
        movss xmm0,dword ptr [esi + 0x200] // 00b473ab
        movss dword ptr [esp + 0x420],xmm0 // 00b473b3
        movss xmm0,dword ptr [esi + 0x204] // 00b473bc
        movss dword ptr [esp + 0x424],xmm0 // 00b473c4
        movss xmm0,dword ptr [esi + 0x214] // 00b473cd
        movss dword ptr [esp + 0x428],xmm0 // 00b473d5
        movss xmm0,dword ptr [esi + 0x224] // 00b473de
        movss dword ptr [esp + 0x42c],xmm0 // 00b473e6
        movss xmm0,dword ptr [esi + 0x234] // 00b473ef
        movss dword ptr [esp + 0x430],xmm0 // 00b473f7
        movss xmm0,dword ptr [esi + 0x208] // 00b47400
        movss dword ptr [esp + 0x434],xmm0 // 00b47408
        movss xmm0,dword ptr [esi + 0x218] // 00b47411
        movss dword ptr [esp + 0x438],xmm0 // 00b47419
        movss xmm0,dword ptr [esi + 0x228] // 00b47422
        movss dword ptr [esp + 0x43c],xmm0 // 00b4742a
        movss xmm0,dword ptr [esi + 0x238] // 00b47433
        movss dword ptr [esp + 0x440],xmm0 // 00b4743b
        movss xmm0,dword ptr [esi + 0x20c] // 00b47444
        movss dword ptr [esp + 0x444],xmm0 // 00b4744c
        movss xmm0,dword ptr [esi + 0x21c] // 00b47455
        movss dword ptr [esp + 0x448],xmm0 // 00b4745d
        movss xmm0,dword ptr [esi + 0x22c] // 00b47466
        movss dword ptr [esp + 0x44c],xmm0 // 00b4746e
        movss xmm0,dword ptr [esi + 0x23c] // 00b47477
        movss dword ptr [esp + 0x450],xmm0 // 00b4747f
        movss xmm0,dword ptr [esi + 0x210] // 00b47488
        movss dword ptr [esp + 0x454],xmm0 // 00b47490
        movss xmm0,dword ptr [esi + 0x220] // 00b47499
        movss dword ptr [esp + 0x458],xmm0 // 00b474a1
        movss xmm0,dword ptr [esi + 0x230] // 00b474aa
        movss dword ptr [esp + 0x45c],xmm0 // 00b474b2
        movss xmm0,dword ptr [esi + 0x240] // 00b474bb
        movss dword ptr [esp + 0x460],xmm0 // 00b474c3
        movss xmm0,dword ptr [esi + 0x38] // 00b474cc
        movss xmm1,dword ptr [esi + 0x3c] // 00b474d1
        movss xmm2,dword ptr [esi + 0x40] // 00b474d6
        movss dword ptr [esp + 0x464],xmm0 // 00b474db
        movss dword ptr [esp + 0x468],xmm1 // 00b474e4
        movss dword ptr [esp + 0x46c],xmm2 // 00b474ed
        movss xmm0,dword ptr [esi + 0x390] // 00b474f6
        movss xmm1,dword ptr [esi + 0x394] // 00b474fe
        movss xmm2,dword ptr [esi + 0x398] // 00b47506
        movss xmm3,dword ptr [esi + 0x39c] // 00b4750e
        movss dword ptr [esp + 0x474],xmm0 // 00b47516
        movss dword ptr [esp + 0x478],xmm1 // 00b4751f
        movss dword ptr [esp + 0x47c],xmm2 // 00b47528
        movss dword ptr [esp + 0x480],xmm3 // 00b47531
        mov edx,dword ptr [esi] // 00b4753a
        // Numeric table target is read and checked through the borrowed profile in the following bridge. // 00b4753c
        mov ecx,esi // 00b4753f
        push edx // Captured at B4753A; do not reload shadow[0].
        mov edx,dword ptr [esp+4ech]
        call shadow_bridge // 00b47541
        mov edx,dword ptr [eax] // 00b47543
        mov ecx,eax // 00b47545
        // Numeric table target is read and checked through the borrowed profile in the following bridge. // 00b47547
        push edx // Captured at B47543; do not reload texture[0].
        mov edx,dword ptr [esp+4ech]
        call width_bridge // 00b4754a
        test eax,eax // 00b4754c
        mov dword ptr [esp + 0x10],eax // 00b4754e
        fild dword ptr [esp + 0x10] // 00b47552
        jge l_00b4755e // 00b47556
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+40]
        fadd float ptr [eax+0]
        pop eax // 00b47558
    l_00b4755e:
        fstp float ptr [esp + 0x4d4] // 00b4755e
        mov edx,dword ptr [esi] // 00b47565
        // Numeric table target is read and checked through the borrowed profile in the following bridge. // 00b47567
        mov ecx,esi // 00b4756a
        push edx // Captured at B47565; do not reload shadow[0].
        mov edx,dword ptr [esp+4ech]
        call shadow_bridge // 00b4756c
        mov edx,dword ptr [eax] // 00b4756e
        mov ecx,eax // 00b47570
        // Numeric table target is read and checked through the borrowed profile in the following bridge. // 00b47572
        push edx // Captured at B4756E; do not reload texture[0].
        mov edx,dword ptr [esp+4ech]
        call height_bridge // 00b47575
        test eax,eax // 00b47577
        mov dword ptr [esp + 0x10],eax // 00b47579
        fild dword ptr [esp + 0x10] // 00b4757d
        jge l_00b47589 // 00b47581
        push eax
        mov eax,dword ptr [esp+0x4ec]
        mov eax,dword ptr [eax+40]
        fadd float ptr [eax+0]
        pop eax // 00b47583
    l_00b47589:
        fstp float ptr [esp + 0x4d8] // 00b47589
        fld float ptr [esp + 0x4d4] // 00b47590
        fld1 // 00b47597
        fld st(0) // 00b47599
        fdivrp st(2),st(0) // 00b4759b
        fxch // 00b4759d
        fstp float ptr [esp + 0x4dc] // 00b4759f
        fdiv float ptr [esp + 0x4d8] // 00b475a6
        fstp float ptr [esp + 0x4e0] // 00b475ad
    l_00b475b4:
        mov ecx,edi // 00b475b4
        call get_native_camera_context_00b6feb0 // 00b475b6
        test eax,eax // 00b475bb
        jz l_00b475fd // 00b475bd
        mov ecx,edi // 00b475bf
        call get_native_camera_context_00b6feb0 // 00b475c1
        movss xmm0,dword ptr [eax] // 00b475c6
        movss dword ptr [esp + 0x484],xmm0 // 00b475ca
        movss xmm0,dword ptr [eax + 0x4] // 00b475d3
        movss dword ptr [esp + 0x488],xmm0 // 00b475d8
        movss xmm0,dword ptr [eax + 0x8] // 00b475e1
        movss dword ptr [esp + 0x48c],xmm0 // 00b475e6
        movss xmm0,dword ptr [eax + 0xc] // 00b475ef
        movss dword ptr [esp + 0x490],xmm0 // 00b475f4
    l_00b475fd:
        mov ecx,dword ptr [esp+0x4e8]
        mov ecx,dword ptr [ecx+32]
        mov ecx,dword ptr [ecx+0] // 00b475fd
        push ecx // 00b47603
        mov ecx,dword ptr [esp+0x4ec]
        mov ecx,dword ptr [ecx+4]
        mov ecx,dword ptr [ecx+0] // 00b47604
        lea edx,[esp + 0x18] // 00b4760a
        push edx // 00b4760e
        push 0x0 // 00b4760f
        mov edx,dword ptr [esp+4f4h]
        mov edx,dword ptr [edx+48]
        call set_native_vertex_shader_constants_f_00b21820 // 00b47611
        mov eax,dword ptr [esp+0x4e8]
        mov eax,dword ptr [eax+32]
        mov eax,dword ptr [eax+0] // 00b47616
        push eax // 00b4761b
        lea ecx,[esp + 0x18] // 00b4761c
        push ecx // 00b47620
        mov ecx,dword ptr [esp+0x4f0]
        mov ecx,dword ptr [ecx+4]
        mov ecx,dword ptr [ecx+0] // 00b47621
        push 0x0 // 00b47627
        mov edx,dword ptr [esp+4f4h]
        mov edx,dword ptr [edx+48]
        call set_native_pixel_shader_constants_f_00b218c0 // 00b47629
        pop edi // 00b4762e
        pop esi // 00b4762f
        pop ebp // 00b47630
        pop ebx // 00b47631
        add esp,0x4d4 // 00b47632
        ret 8 // 00b47638
    }
}
} // namespace bsp
