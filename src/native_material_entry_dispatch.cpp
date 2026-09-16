#include "bsp/native_material_entry_dispatch.hpp"
#include "bsp/light_type_bootstrap.hpp"
#include "bsp/native_material_pass_execution.hpp"
#include "bsp/native_point_light_owner.hpp"
#include "bsp/native_renderer_debug_records24_append.hpp"
#include "bsp/native_shadow_update_leaves.hpp"
#include "bsp/native_traceline_render.hpp"
#include "bsp/native_render_state_leaves.hpp"
#include <cstdlib>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material entry dispatch requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(const void* p, Word offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+offset);
}
__declspec(noinline) Word word(const void* p, Word offset=0) noexcept {
    return *static_cast<const volatile Word*>(at(p,offset));
}
void* pointer(const void* p, Word offset=0) noexcept {
    return reinterpret_cast<void*>(word(p,offset));
}
__declspec(noinline) bool byte_set(const void* p, Word offset) noexcept {
    return *static_cast<const volatile unsigned char*>(at(p,offset))!=0;
}
void require(bool valid, const char* message) {
    if (!valid) throw std::logic_error(message);
}
__declspec(noinline) bool one_above_visibility(const void* entry,
    const volatile float* actual_one) noexcept {
    unsigned char above;
    __asm {
        mov eax, actual_one
        mov edx, entry
        movss xmm0, dword ptr [eax]
        comiss xmm0, dword ptr [edx+18h]
        seta above
    }
    return above!=0;
}
const volatile Word* renderer_profile(void* renderer,
    const NativeMaterialEntryDispatchContext& c) {
    const Word token=word(renderer);
    require(token==0x00d5f0a8 && c.actual_renderer_profile_00d5f0a8,
        "material entry requires current D5F0A8 renderer binding");
    return c.actual_renderer_profile_00d5f0a8;
}
const volatile Word* light_profile(void* light,
    const NativeMaterialEntryDispatchContext& c) {
    const volatile Word* profile=nullptr;
    switch(word(light)) {
    case 0x00d62f58: profile=c.actual_light_profile_00d62f58; break;
    case 0x00d62fb0: profile=c.actual_directional_profile_00d62fb0; break;
    case 0x00d63008: profile=c.actual_point_profile_00d63008; break;
    default: break;
    }
    require(profile!=nullptr,"material entry requires its current concrete light profile");
    return profile;
}
bool light_accepts(Word target, Word token, NativeMaterialEntryDispatchContext& c) {
    require(c.light_types!=nullptr,"material entry requires actual shared light type storage");
    switch(target) {
    case 0x00b7c580: return c.light_types->light_is_type_00b7c580(token);
    case 0x00b7c6d0: return c.light_types->directional_is_type_00b7c6d0(token);
    case 0x00b7c740:
        require(c.point_light_types!=nullptr,"material entry requires actual point-light types");
        return c.point_light_types->is_type_00b7c740(token);
    default: throw std::logic_error("material entry light slot0C lacks a concrete recovered target");
    }
}
void draw_pass(void* pass, void* entry, void* light,
    NativeMaterialEntryDispatchContext& c, NativeMaterialEntryDispatchFrame* frame) {
    require(c.passes && frame && frame->passes && frame->used<frame->capacity,
        "material entry requires its prepared persistent pass frame");
    require(&c.passes->actual_renderer_00f8d394==&c.actual_renderer_00f8d394 &&
        &c.passes->actual_cached_effect_0108fbf4==&c.actual_cached_effect_0108fbf4 &&
        &c.passes->actual_one_00d7a24c==&c.actual_one_00d7a24c &&
        c.passes->actual_renderer_profile_00d5f0a8==c.actual_renderer_profile_00d5f0a8,
        "material entry and pass must borrow the same actual global/profile cells");
    auto* const child=frame->passes[frame->used];
    require(child && child->phase==NativeMaterialPassExecutionFrame::Phase::fresh,
        "material entry cannot replay a used pass frame");
    ++frame->used;
    bind_native_material_pass_geometry_00b44750(pass,entry,light,*c.passes,*child);
}
void select_and_draw(void* effect, void* entry, void* light,
    NativeMaterialEntryDispatchContext& c, NativeMaterialEntryDispatchFrame* frame) {
    void* const camera=pointer(entry,0x10);
    const bool below_one=one_above_visibility(entry,&c.actual_one_00d7a24c);
    const Word mode=word(camera,0x198); // read AFTER COMISS, as in both native arms
    void* const pass=pointer(effect,below_one && mode==0 ? 0x100u : 0xc8u+mode*4u);
    if(pass) draw_pass(pass,entry,light,c,frame);
}
} // namespace

void dispatch_native_material_entry_00b45360(void* effect, void* entry,
    NativeMaterialEntryDispatchContext& c, NativeMaterialEntryDispatchFrame* frame) {
    void* const initial_camera=pointer(entry,0x10);
    const Word initial_mode=word(initial_camera,0x198);
    if(initial_mode==2) {
        void* const special=pointer(effect,0x138);
        if(special) { draw_pass(special,entry,nullptr,c,frame); return; }
    }
    if(byte_set(pointer(effect,0xc4),0x16) &&
        one_above_visibility(entry,&c.actual_one_00d7a24c) && initial_mode==6) return;

    const bool debug_sphere=byte_set(effect,8);
    void* const model=pointer(entry,0x0c);
    void* const collection=pointer(model,0x170);
    if(debug_sphere && initial_mode==0) {
        const Word selector=word(entry,0x1c);
        void* const renderer=c.actual_renderer_00f8d394;
        const volatile Word* const captured_renderer_profile=renderer_profile(renderer,c);
        require(c.models && c.models->services && c.sphere_records,
            "material entry requires canonical model sphere and record guard domains");
        const volatile Word* const model_profile=c.models->services->profile(model);
        const Word model_target=model_profile[0x48/4];
        const void* const sphere=model_target==0x00b6e8c0
            ? get_native_model_world_sphere_00b6e8c0(model,c.models)
            : c.models->services->call_virtual48(model,model_target);
        // Native camera/selector stack words survive the no-argument sphere call.
        // Load +C8 from the CAPTURED renderer table only after that call returns.
        require(captured_renderer_profile[0xc8/4]==0x00b29270,
            "material entry requires captured renderer sphere-record producer");
        append_native_renderer_debug_record24_00b29270(
            renderer,*c.sphere_records,sphere,initial_camera,selector);
    }
    if(!collection) { select_and_draw(effect,entry,nullptr,c,frame); return; }
    if(word(collection,0x20)==0) return;
    Word index=0;
    do {
        void* const head=pointer(collection,0x1c);
        void* const front=pointer(head);
        if(front==head) _invalid_parameter_noinfo();
        void* const light=pointer(front,8); // keep captured node after returning validation
        const volatile Word* const captured_profile=light_profile(light,c);
        require(c.light_types!=nullptr,"material entry requires actual directional type cell");
        const Word token=get_native_override_context_word_00b7aae0(
            c.light_types->storage().directional_0109019c.own_id);
        const Word target=captured_profile[0x0c/4]; // current slot AFTER getter
        if(light_accepts(target,token,c)) select_and_draw(effect,entry,light,c,frame);
        ++index;
    } while(index<word(collection,0x20));
}

void execute_native_render_batch_entries_00b55550(void* batch,
    std::uint32_t unused_index, void* unused_camera,
    NativeMaterialEntryDispatchContext& c, NativeMaterialEntryDispatchFrame* frame) {
    (void)unused_index; (void)unused_camera;
    void* const renderer=c.actual_renderer_00f8d394;
    const volatile Word* const profile=renderer_profile(renderer,c);
    require(profile[0x2c/4]==0x00b1fe20,"material batch requires current renderer active-frame getter");
    if(!native_render_is_frame_active_00b1fe20(renderer)) return;
    c.actual_cached_effect_0108fbf4=nullptr;
    Word index=0;
    if(static_cast<std::int32_t>(word(batch,0x10))<=0) return;
    do {
        void* const entry=pointer(pointer(batch,0x0c),index*4u);
        void* const effect=pointer(pointer(pointer(entry,4),0x20),0x7c);
        const Word effect_token=word(effect);
        require(effect_token==0x00d61a00 && c.actual_effect_profile_00d61a00 &&
            c.actual_effect_profile_00d61a00[0x14/4]==0x00b45360,
            "material batch requires current D61A00 effect dispatch");
        dispatch_native_material_entry_00b45360(effect,entry,c,frame);
        ++index;
    } while(static_cast<std::int32_t>(index)<static_cast<std::int32_t>(word(batch,0x10)));
}
} // namespace bsp
