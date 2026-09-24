#include "bsp/native_render_resource_init_second_bloom.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_renderer_current_depth_surface.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource second bloom requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
using Context=NativeRenderResourceInitContinuationContext;
using Continuation=NativeRenderResourceInitContinuationState;
using Post=NativeRenderResourceInitPostState;
using Previous=NativeRenderResourceInitDofState;
using State=NativeRenderResourceInitSecondBloomState;
void* at(const void* p,U offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
U bits(const void* p) noexcept {return reinterpret_cast<U>(p);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(at(p,offset))=value;}
void* child(const void* p,U offset) noexcept {return reinterpret_cast<void*>(word(p,offset));}
void set_state(State& a,int state) noexcept {
    auto& dof=*a.previous;auto& post=*dof.previous;auto& continuation=*post.previous;
    a.native_state=state;dof.native_state=state;post.native_state=state;
    continuation.native_state=state;continuation.entry->unwind_state=state;
}
void site(State& a,U address,int state) noexcept {
    auto& dof=*a.previous;auto& post=*dof.previous;auto& continuation=*post.previous;
    a.native_site=address;dof.native_site=address;post.native_site=address;
    continuation.native_site=address;continuation.entry->native_site=address;
    set_state(a,state);
}
void* current_depth_renderer(Context& c) {
    void* const renderer=c.entry.frame_targets.actual_surface_context.actual_renderer_00f8d394;
    const auto* const table=c.entry.actual_renderer_profile_00d5f0a8;
    if(!renderer || word(renderer)!=0x00d5f0a8u || !table || table[0x12c/4]!=0x00b20090u)
        throw std::logic_error("second bloom requires current D5F0A8 renderer slot12C=B20090");
    return renderer;
}
} // namespace

void continue_native_render_resource_init_00b11d7e_fragment(Previous& previous,Context& c,State& a) {
    auto* const post=previous.previous;
    auto* const continuation=post?post->previous:nullptr;
    auto* const entry=continuation?continuation->entry:nullptr;
    if(a.phase!=State::Phase::fresh || previous.phase!=Previous::Phase::awaiting_b11d7e_continuation ||
       previous.native_site!=0x00b11d7eu || previous.native_state!=-1 || previous.temporary_mask_esp10 ||
       previous.second_bloom_identity || !previous.field74_published || !post ||
       post->phase!=Post::Phase::awaiting_later_continuation || post->dof_identity!=&previous ||
       post->native_site!=0x00b11d7eu || post->native_state!=-1 || !continuation ||
       continuation->phase!=Continuation::Phase::awaiting_later_continuation ||
       continuation->post_identity!=post || continuation->context_identity!=&c ||
       continuation->pass_companions_identity!=&c.pass_companions ||
       continuation->native_site!=0x00b11d7eu || continuation->native_state!=-1 || continuation->temporary_mask_esp10 ||
       !entry || entry->phase!=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation ||
       entry->native_site!=0x00b11d7eu || entry->unwind_state!=-1 || entry->continuation_identity!=continuation ||
       !entry->argument_cells || continuation->ebp_bits!=bits(at(entry->service,0xa0)))
        throw std::logic_error("second bloom requires an unconsumed B11D7E state and original context chain");
    a.previous=&previous;previous.second_bloom_identity=&a;a.phase=State::Phase::preparing;
    previous.phase=Previous::Phase::second_bloom_running;post->phase=Post::Phase::dof_running;
    continuation->phase=Continuation::Phase::post_running;
    entry->phase=NativeRenderResourceInitEntryState::Phase::continuation_running;
    site(a,0xb11d7e,-1);
    try {
        a.bloom.prepare(c.bloom); // Independent of the retained service+28 block.
        a.phase=State::Phase::running;
        void* const service=entry->service;
        site(a,0xb11d80,-1);
        a.raw_frame=singleton_lifetime_allocate({SingletonAllocationKind::object,0x40,0x40});
        continuation->spill_esp14=bits(a.raw_frame);set_state(a,52);
        if(a.raw_frame) {
            site(a,0xb11d9d,52);
            a.returned_frame=construct_native_frame_target_owner_00b1fbb0(a.raw_frame);
        }
        a.captured_color_holder=child(service,0x44); // B11DA6 BEFORE +1CC publication.
        set_state(a,-1);put(service,0x1cc,bits(a.returned_frame));a.frame_published=true;
        site(a,0xb11db6,-1);
        a.captured_color=native_render_holder_primary_00b4cb20(a.captured_color_holder);
        site(a,0xb11dc4,-1);
        set_native_frame_target_color_00b1fab0(*static_cast<NativeFrameTargetOwnerStorage*>(child(service,0x1cc)),
            0,a.captured_color,c.entry.frame_targets);
        site(a,0xb11dd7,-1);a.captured_renderer=current_depth_renderer(c);
        a.captured_depth=get_native_renderer_current_depth_surface_00b20090(a.captured_renderer);
        site(a,0xb11de0,-1);
        set_native_frame_target_depth_00b1fb00(*static_cast<NativeFrameTargetOwnerStorage*>(child(service,0x1cc)),
            static_cast<NativeSurfaceOwnerStorage*>(a.captured_depth),c.entry.frame_targets);
        site(a,0xb11dea,-1);
        a.raw_bloom=singleton_lifetime_allocate({SingletonAllocationKind::object,0x43c,0x43c});
        continuation->spill_esp14=bits(a.raw_bloom);set_state(a,53);
        if(a.raw_bloom) {
            site(a,0xb11e07,53);
            a.returned_bloom=construct_native_bloom_owner_00b54e70(a.raw_bloom);
        }
        // B11E10..B11E21: keep FLD1/FSTP, with the retained dimension reads
        // and current +1C capture between them. No provider call while ST0 lives.
        const U* half_height=&continuation->half_aligned_height_esp1d8;
        const U* half_width=&continuation->spill_esp20;
        U height,width,parameter;void* input_pass;
        __asm {
            fld1
            mov eax,half_height
            mov edx,[eax]
            mov height,edx
            mov eax,half_width
            mov edx,[eax]
            mov width,edx
            mov ecx,service
            mov ecx,[ecx+1ch]
            mov input_pass,ecx
            fstp dword ptr [parameter]
        }
        continuation->edi_bits=height;continuation->ebp_bits=width;
        a.captured_input_pass=input_pass;a.arguments={nullptr,width,height,0x71,parameter};
        if(a.returned_bloom) {
            a.bloom_binding_started=true;
            // Host metadata after genuine cleanup-slot construction and the
            // complete x87/input capture block; no metadata call while ST0 lives.
            // On failure retain the unpublished raw owner and prepared block.
            a.bloom_reference.emplace(a.returned_bloom,c.pass_companions);
        }
        set_state(a,-1);put(service,0x2c,bits(a.returned_bloom));a.bloom_published=true;
        site(a,0xb11e32,-1);
        a.arguments.input_holder=native_shadow_texture_holder_00b4d170(a.captured_input_pass);
        site(a,0xb11e3b,-1);
        initialize_native_bloom_00b54f90(child(service,0x2c),0x43c,a.arguments,a.bloom);
        site(a,0xb11e40,-1);a.phase=State::Phase::awaiting_b11e40_continuation;
        previous.phase=Previous::Phase::awaiting_later_continuation;
        post->phase=Post::Phase::awaiting_later_continuation;
        continuation->phase=Continuation::Phase::awaiting_later_continuation;
        entry->phase=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation;
    } catch(...) {
        a.phase=State::Phase::failed;previous.phase=Previous::Phase::failed;post->phase=Post::Phase::failed;
        continuation->phase=Continuation::Phase::failed;entry->phase=NativeRenderResourceInitEntryState::Phase::failed;
        // Preserve all publications and provider preparation/acquisition records.
        // Child failure semantics stay authoritative; this caller adds no cleanup.
        throw;
    }
}
} // namespace bsp
