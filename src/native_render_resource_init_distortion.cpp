#include "bsp/native_render_resource_init_distortion.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource distortion stage requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
using Context=NativeRenderResourceInitContinuationContext;
using Continuation=NativeRenderResourceInitContinuationState;
using Post=NativeRenderResourceInitPostState;
using Dof=NativeRenderResourceInitDofState;
using Previous=NativeRenderResourceInitSecondBloomState;
using State=NativeRenderResourceInitDistortionState;
void* at(const void* p,U offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
U bits(const void* p) noexcept {return reinterpret_cast<U>(p);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(at(p,offset))=value;}
void* child(const void* p,U offset) noexcept {return reinterpret_cast<void*>(word(p,offset));}
void set_state(State& a,int state) noexcept {
    auto& previous=*a.previous;auto& dof=*previous.previous;
    auto& post=*dof.previous;auto& continuation=*post.previous;
    a.native_state=state;previous.native_state=state;dof.native_state=state;
    post.native_state=state;continuation.native_state=state;continuation.entry->unwind_state=state;
}
void site(State& a,U address,int state) noexcept {
    auto& previous=*a.previous;auto& dof=*previous.previous;
    auto& post=*dof.previous;auto& continuation=*post.previous;
    a.native_site=address;previous.native_site=address;dof.native_site=address;
    post.native_site=address;continuation.native_site=address;continuation.entry->native_site=address;
    set_state(a,state);
}
void require_domains(Context& c,NativeRenderResourceInitDistortionContext& d,State& a) {
    auto& passes=d.pass_companions;
    auto* const lifetime=passes.distortion;
    auto& post=d.distortion.post_effects;
    auto& surfaces=c.entry.frame_targets.actual_surface_context;
    if(!lifetime || &passes.effects!=&c.effects_lifetime ||
       &passes.registry!=&c.effects_lifetime.actual_post_effects ||
       &lifetime->effects!=&c.effects_lifetime || &lifetime->frames!=&c.entry.frame_targets ||
       &lifetime->nodes!=&post.destruction.nodes ||
       lifetime->frame_profile_00d5e600!=c.entry.actual_frame_profile_00d5e600 ||
       &lifetime->scene_owner_3c!=&a.distortion.scene_owner_3c() ||
       &post!=&c.bloom.post_effects || &post.destruction.actual_owners!=&passes.registry ||
       &d.distortion.textures.textures.holders!=&c.effects_lifetime.texture_holders ||
       &d.distortion.parameters!=&c.bloom.parameters ||
       static_cast<const volatile void*>(&d.distortion.textures.actual_renderer_00f8d394)!=
           static_cast<const volatile void*>(&surfaces.actual_renderer_00f8d394) ||
       d.distortion.textures.renderer_profile_00d5f0a8!=c.entry.actual_renderer_profile_00d5f0a8 ||
       &d.depth.factory!=&c.effects_lifetime.texture_holders.render_targets ||
       &d.depth.factory.surfaces!=&surfaces ||
       d.depth.renderer_profile_00d5f0a8!=c.entry.actual_renderer_profile_00d5f0a8 ||
       d.depth.surface_profile_00d619a0!=c.entry.frame_targets.actual_surface_profile_00d619a0 ||
       &d.depth.decrement_iat_00ce2220!=&c.entry.decrement_iat_00ce2220 ||
       &c.effects_lifetime.actual_decrement_00ce2220!=&c.entry.decrement_iat_00ce2220)
        throw std::logic_error("distortion stage requires shared actual domains and its live scene publication cell");
}
} // namespace

void continue_native_render_resource_init_00b11e40_fragment(Previous& previous,Context& c,
    NativeRenderResourceInitDistortionContext& d,State& a) {
    auto* const dof=previous.previous;
    auto* const post=dof?dof->previous:nullptr;
    auto* const continuation=post?post->previous:nullptr;
    auto* const entry=continuation?continuation->entry:nullptr;
    if(a.phase!=State::Phase::fresh || previous.phase!=Previous::Phase::awaiting_b11e40_continuation ||
       previous.native_site!=0x00b11e40u || previous.native_state!=-1 || previous.distortion_identity ||
       !previous.bloom_published || !dof || dof->phase!=Dof::Phase::awaiting_later_continuation ||
       dof->second_bloom_identity!=&previous || dof->native_site!=0x00b11e40u || dof->native_state!=-1 ||
       dof->temporary_mask_esp10 || !post || post->phase!=Post::Phase::awaiting_later_continuation ||
       post->dof_identity!=dof || post->native_site!=0x00b11e40u || post->native_state!=-1 ||
       !continuation || continuation->phase!=Continuation::Phase::awaiting_later_continuation ||
       continuation->post_identity!=post || continuation->context_identity!=&c ||
       continuation->native_site!=0x00b11e40u || continuation->native_state!=-1 || continuation->temporary_mask_esp10 ||
       !entry || entry->phase!=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation ||
       entry->native_site!=0x00b11e40u || entry->unwind_state!=-1 || entry->continuation_identity!=continuation ||
       !entry->argument_cells || continuation->ebp_bits!=continuation->spill_esp20 ||
       continuation->edi_bits!=continuation->half_aligned_height_esp1d8)
        throw std::logic_error("distortion stage requires an unconsumed B11E40 state and original context chain");
    require_domains(c,d,a);
    a.previous=&previous;a.context_identity=&d;a.entry_identity=entry;
    a.argument_cells_identity=entry->argument_cells;previous.distortion_identity=&a;
    a.phase=State::Phase::preparing;previous.phase=Previous::Phase::distortion_running;
    dof->phase=Dof::Phase::second_bloom_running;post->phase=Post::Phase::dof_running;
    continuation->phase=Continuation::Phase::post_running;
    entry->phase=NativeRenderResourceInitEntryState::Phase::continuation_running;
    site(a,0xb11e40,-1);
    try {
        a.distortion.prepare(d.distortion);
        a.phase=State::Phase::running;
        void* const service=entry->service;
        site(a,0xb11e45,-1);
        a.raw_distortion=singleton_lifetime_allocate({SingletonAllocationKind::object,0x26c,0x26c});
        continuation->spill_esp14=bits(a.raw_distortion);set_state(a,54);
        if(a.raw_distortion) {
            site(a,0xb11e62,54);
            a.returned_distortion=construct_native_distortion_owner_00b4f0c0(a.raw_distortion,d.constants);
            // Host metadata only, under the explicit valid cleanup-preimage
            // admission. Do not wait for init success: false/reentrant release
            // must already have the one real canonical companion over actual+04.
            a.pass_reference.emplace(a.returned_distortion,d.pass_companions);
        }
        const U height=continuation->edi_bits;
        const U width=continuation->ebp_bits;
        void* const receiver=a.returned_distortion;
        set_state(a,-1);put(service,0x30,bits(receiver));a.distortion_published=true;
        site(a,0xb11e79,-1);
        a.initializer_result=initialize_native_distortion_00b4f560(receiver,0x26c,width,height,a.distortion);
        a.initializer_returned=true;
        if(!a.initializer_result) {
            site(a,0xb11e82,-1);
            a.captured_false_owner=child(service,0x30);
            continuation->edi_bits=bits(a.captured_false_owner);
            if(a.captured_false_owner) {
                site(a,0xb11e8d,-1);
                a.captured_decrement=c.entry.decrement_iat_00ce2220;
                if(!a.captured_decrement)throw std::logic_error("unbound current distortion decrement target");
                a.decrement_started=true;
                a.decrement_result=a.captured_decrement(static_cast<volatile long*>(at(a.captured_false_owner,4)));
                a.decrement_returned=true;
                if(a.decrement_result==0) {
                    site(a,0xb11e9d,-1);a.terminal_started=true;
                    // Lookup has no native effects. The actual captured owner's
                    // canonical terminal reloads its current profile/virtual0
                    // AFTER the decrement callback and handles BD30E0's reload.
                    // It can be a callback replacement with a different context.
                    d.pass_companions.registry.resolve_actual(a.captured_false_owner).release_zero_references();
                }
                site(a,0xb11e9f,-1);
                put(service,0x30,0);a.parent_cleared=true;
            }
        }
        site(a,0xb11ea8,-1);
        a.raw_frame=singleton_lifetime_allocate({SingletonAllocationKind::object,0x40,0x40});
        continuation->spill_esp14=bits(a.raw_frame);set_state(a,55);
        if(a.raw_frame) {
            site(a,0xb11ec5,55);
            a.returned_frame=construct_native_frame_target_owner_00b1fbb0(a.raw_frame);
        }
        // Native B11ECE/B11ED5/B11EDA/B11EE0 order, including unused word08.
        a.original_third_argument=a.argument_cells_identity->word_08;
        a.original_height=*static_cast<const volatile U*>(&entry->dimensions_esp18[1]);
        put(service,0x1c8,bits(a.returned_frame));a.frame_published=true;
        a.original_width=*static_cast<const volatile U*>(&entry->dimensions_esp18[0]);
        set_state(a,-1);site(a,0xb11eef,-1);
        resize_native_render_resources_depth_00b0fc10(service,a.original_width,a.original_height,
            a.original_third_argument,d.depth,a.depth);
        site(a,0xb11ef4,-1);a.phase=State::Phase::awaiting_b11ef4_continuation;
        previous.phase=Previous::Phase::awaiting_later_continuation;
        dof->phase=Dof::Phase::awaiting_later_continuation;post->phase=Post::Phase::awaiting_later_continuation;
        continuation->phase=Continuation::Phase::awaiting_later_continuation;
        entry->phase=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation;
    } catch(...) {
        a.phase=State::Phase::failed;previous.phase=Previous::Phase::failed;dof->phase=Dof::Phase::failed;
        post->phase=Post::Phase::failed;continuation->phase=Continuation::Phase::failed;
        entry->phase=NativeRenderResourceInitEntryState::Phase::failed;
        // Preserve publications, canonical metadata and child acquisition state.
        // Existing provider unwind remains authoritative; no caller repair/free.
        throw;
    }
}
} // namespace bsp
