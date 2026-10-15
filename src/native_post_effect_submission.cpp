#include "bsp/native_post_effect_submission.hpp"
#include "bsp/native_camera_frame_command.hpp"
#include "bsp/native_frame_target_getters.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_material_pass_execution.hpp"
#include "bsp/native_mesh_remaining_fields.hpp"
#include "bsp/native_renderer_frame_targets.hpp"
#include "bsp/native_surface_dimensions.hpp"
#include "bsp/native_viewport_owner.hpp"
#include "bsp/gui_widget_owner.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera viewport access requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
void* at(const void* p,Word n=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+n);
}
__declspec(noinline) Word word(const void* p,Word n=0) noexcept {
    return *static_cast<const volatile Word*>(at(p,n));
}
void* pointer(const void* p,Word n=0) noexcept {return reinterpret_cast<void*>(word(p,n));}
void put(void* p,Word n,Word v) noexcept {*static_cast<volatile Word*>(at(p,n))=v;}
void require(bool valid,const char* why) {if(!valid)throw std::logic_error(why);}
const volatile Word* renderer_profile(void* renderer,NativePostEffectExecutionContext& c) {
    require(word(renderer)==0x00d5f0a8 && c.camera.renderer_profile_04,
        "post effect requires its current D5F0A8 renderer profile");
    return c.camera.renderer_profile_04;
}
void require_domains(NativePostEffectExecutionContext& c) {
    const volatile void* const renderer_cell=c.camera.renderer_00;
    const auto* const sync=&c.targets.actual_synchronization_0108d6dc;
    require(renderer_cell && c.camera.prepare_08 && c.camera.viewport_clear_0c &&
        renderer_cell==static_cast<const volatile void*>(&c.mapping.actual_renderer_00f8d394) &&
        renderer_cell==static_cast<const volatile void*>(&c.materials.actual_renderer_00f8d394) &&
        renderer_cell==static_cast<const volatile void*>(&c.targets.actual_group_owner.actual_surface_context.actual_renderer_00f8d394) &&
        sync==&c.mapping.actual_synchronization_0108d6dc && sync==&c.materials.synchronization &&
        sync==c.camera.prepare_08->synchronization_04 && sync==c.camera.viewport_clear_0c->synchronization_00 &&
        c.camera.renderer_profile_04==c.materials.actual_renderer_profile_00d5f0a8 &&
        c.vertex_profile_00d61d6c==c.materials.actual_vertex_profile_00d61d6c,
        "post effect requires one actual renderer, synchronization and current-profile domain");
}
Word surface_dimension(void* surface,Word slot,NativePostEffectExecutionContext& c) {
    require(word(surface)==0x00d619a0 && c.surface_profile_00d619a0,
        "post effect requires its current D619A0 surface profile");
    const Word target=c.surface_profile_00d619a0[slot/4];
    if(slot==0x20) {
        require(target==0x00b3cd20,"post effect requires actual surface height");
        return get_native_surface_height_00b3cd20(surface);
    }
    require(target==0x00b3cd10,"post effect requires actual surface width");
    return get_native_surface_width_00b3cd10(surface);
}
void require_stream(void* stream,Word slot,Word target,NativePostEffectExecutionContext& c) {
    require(word(stream)==0x00d61d6c && c.vertex_profile_00d61d6c &&
        c.vertex_profile_00d61d6c[slot/4]==target,
        "post effect requires its current concrete logical vertex map/unmap profile");
}
// All operations are raw MOVSS bit copies or XORPS zero. Use volatile DWORD
// accesses in the same order, preserving even nonnumeric borrowed cell bits.
void write_vertices(void* post,void* out,const NativePostEffectVertexConstants& c) {
    const bool triangle=word(post,0x1c)==3; // B4DD79, AFTER the map callback.
    const Word x1=c.word_00d7a260;
    const Word half=c.word_00ce3800;
    const Word one=c.word_00d7a24c;
    put(out,4,x1);put(out,8,half);put(out,0xc,one);
    if(triangle) {
        Word temp=c.word_00ce3d50;
        put(out,0,temp);put(out,0x1c,temp);
        temp=c.word_00ce3850;put(out,0x20,temp);
        temp=c.word_00ce3854;
        put(out,0x24,half);put(out,0x28,one);put(out,0x3c,x1);put(out,0x40,half);
        put(out,0x38,temp);put(out,0x44,one);put(out,0x10,x1);put(out,0x14,one);put(out,0x2c,x1);
        temp=c.word_00ce7d7c;put(out,0x30,temp);
        temp=c.word_00ce3958;put(out,0x48,temp);put(out,0x4c,one);put(out,0x18,0);put(out,0x34,one);put(out,0x50,temp);
    } else {
        put(out,0,x1);put(out,0x20,one);put(out,0x28,one);put(out,0x1c,x1);put(out,0x24,half);
        put(out,0x38,one);put(out,0x44,one);put(out,0x3c,x1);put(out,0x40,half);
        put(out,0x54,one);put(out,0x58,one);put(out,0x60,one);put(out,0x5c,half);
        put(out,0x14,one);put(out,0x10,0);put(out,0x2c,0);put(out,0x30,0);
        put(out,0x48,one);put(out,0x4c,one);put(out,0x64,one);put(out,0x68,0);put(out,0x34,one);
        Word temp=c.word_00ce3958;put(out,0x50,temp);
        temp=c.word_00ce3854;put(out,0x18,0);put(out,0x6c,temp);
    }
}
} // namespace

__declspec(naked) void* __fastcall get_native_camera_viewport_00b6fde0(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 180h] // B6FDE0: actual borrowed pointer.
        ret // B6FDE6: no native stack arguments or ownership effect.
    }
}

void draw_native_post_effect_00b4dcc0(void* post,std::uint32_t,
    NativePostEffectExecutionContext& c,NativePostEffectDrawFrame& frame) {
    require(!frame.started,"post effect requires a fresh persistent frame");
    require_domains(c);
    frame.started=true;
    try {
        frame.active_native_site=0x00b4dccd;
        void* const height_surface=native_frame_targets_get_color_surface_00b1f6d0(pointer(post,8),0);
        frame.active_native_site=0x00b4dcd9;
        void* const width_surface=native_frame_targets_get_color_surface_00b1f6d0(pointer(post,8),0);
        frame.active_native_site=0x00b4dce7;
        const Word height=surface_dimension(height_surface,0x20,c);
        frame.active_native_site=0x00b4dcf2;
        const Word width=surface_dimension(width_surface,0x1c,c);
        const Word dimensions[2]={width,height};
        frame.active_native_site=0x00b4dd04;
        auto* const viewport=static_cast<NativeViewportOwner*>(get_native_camera_viewport_00b6fde0(pointer(post,0xc)));
        frame.active_native_site=0x00b4dd0b;
        set_native_viewport_dimensions_00b1f940(*viewport,dimensions);
        void* renderer=*c.camera.renderer_00;
        const volatile Word* profile=renderer_profile(renderer,c);
        void* camera=pointer(post,0xc);
        frame.active_native_site=0x00b4dd22;
        require(profile[0xa0/4]==0x00b285a0,"post effect requires current renderer camera preparation");
        prepare_native_renderer_camera_00b285a0(renderer,c.camera.prepare_08,camera);
        // Capture BOTH renderer and its table BEFORE the current viewport getter.
        renderer=*c.camera.renderer_00;
        camera=pointer(post,0xc);
        profile=renderer_profile(renderer,c);
        frame.active_native_site=0x00b4dd2f;
        void* const current_viewport=get_native_camera_viewport_00b6fde0(camera);
        frame.active_native_site=0x00b4dd3d;
        require(profile[0xa4/4]==0x00b26770,"post effect requires captured renderer viewport binding");
        bind_native_renderer_viewport_00b26770(renderer,c.camera.viewport_clear_0c,current_viewport);
        renderer=*c.camera.renderer_00;
        profile=renderer_profile(renderer,c);
        auto* const targets=static_cast<NativeFrameTargetOwnerStorage*>(pointer(post,8));
        frame.active_native_site=0x00b4dd51;
        require(profile[0x98/4]==0x00b24e70,"post effect requires current renderer frame targets");
        bind_native_renderer_frame_targets_00b24e70(renderer,targets,c.targets);
        frame.active_native_site=0x00b4dd5a;
        void* const geometry=gui_model_geometry_00b74640(
            *static_cast<NativeModelTailStorage*>(at(pointer(post,0x10),0x174)),0);
        frame.active_native_site=0x00b4dd61;
        void* const stream=native_mesh_vertex_stream_unchecked_00b73260(geometry,0);
        const Word count=word(post,0x1c);
        frame.captured_stream=stream;
        frame.active_native_site=0x00b4dd77;
        require_stream(stream,0x10,0x00b49980,c);
        void* const mapped=lock_native_logical_vertex_stream_00b49980(stream,c.mapping,count,0,0);
        frame.captured_mapping=mapped;frame.mapped=true;
        write_vertices(post,mapped,c.vertices);
        frame.vertices_written=true;
        frame.active_native_site=0x00b4dec9;
        require_stream(stream,0x14,0x00b49a80,c);
        unlock_native_logical_vertex_stream_00b49a80(stream,c.mapping);
        frame.unmapped=true;
        void* const pass=pointer(pointer(pointer(post,0x14),0x7c),0x9c);
        void* const entry=pointer(post,0x18);
        frame.active_native_site=0x00b4dee0;
        require(frame.material!=nullptr,"post effect requires its prepared material execution frame");
        execute_native_material_pass_current08(pass,entry,c.materials,*frame.material);
        frame.active_native_site=0x00b4deea;frame.completed=true;
    } catch(...) {frame.failed=true;throw;}
}

void draw_native_post_effect_owner_00b4dfb0(void* owner,
    NativePostEffectExecutionContext& context,NativePostEffectDrawFrame& frame) {
    draw_native_post_effect_00b4dcc0(pointer(owner,8),0,context,frame);
}

} // namespace bsp
