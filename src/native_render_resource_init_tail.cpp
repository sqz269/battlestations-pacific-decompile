#include "bsp/native_render_resource_init_tail.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/platform_renderer_activation.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource tail requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
using Context=NativeRenderResourceInitContinuationContext;
using Continuation=NativeRenderResourceInitContinuationState;
using Post=NativeRenderResourceInitPostState;
using Dof=NativeRenderResourceInitDofState;
using Bloom=NativeRenderResourceInitSecondBloomState;
using Distortion=NativeRenderResourceInitDistortionState;
using Passthrough=NativeRenderResourceInitPassthroughDustState;
using Post658=NativeRenderResourceInitPost658State;
using Post65c=NativeRenderResourceInitPost65cState;
using Previous=NativeRenderResourceInitPost664State;
using State=NativeRenderResourceInitTailState;
using Names=NativeRenderResourceInitTailNames;
void* at(const void* p,U offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
U bits(const void* p) noexcept {return reinterpret_cast<U>(p);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
std::uint8_t byte(const void* p,U offset) noexcept {return *static_cast<const volatile std::uint8_t*>(at(p,offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(at(p,offset))=value;}
void* child(const void* p,U offset) noexcept {return reinterpret_cast<void*>(word(p,offset));}
Continuation& continuation(State& a) noexcept {return *a.previous->previous->previous->previous->previous->previous->previous->previous->previous;}
void set_state(State& a,int value) noexcept {
    auto& previous=*a.previous;auto& p65c=*previous.previous;auto& p658=*p65c.previous;auto& passthrough=*p658.previous;
    auto& distortion=*passthrough.previous;auto& bloom=*distortion.previous;auto& dof=*bloom.previous;
    auto& post=*dof.previous;auto& c=*post.previous;
    a.native_state=value;previous.native_state=value;p65c.native_state=value;p658.native_state=value;
    passthrough.native_state=value;distortion.native_state=value;bloom.native_state=value;dof.native_state=value;
    post.native_state=value;c.native_state=value;c.entry->unwind_state=value;
}
void site(State& a,U address,int state) noexcept {
    auto& previous=*a.previous;auto& p65c=*previous.previous;auto& p658=*p65c.previous;auto& passthrough=*p658.previous;
    auto& distortion=*passthrough.previous;auto& bloom=*distortion.previous;auto& dof=*bloom.previous;
    auto& post=*dof.previous;auto& c=*post.previous;
    a.native_site=address;previous.native_site=address;p65c.native_site=address;p658.native_site=address;
    passthrough.native_site=address;distortion.native_site=address;bloom.native_site=address;dof.native_site=address;
    post.native_site=address;c.native_site=address;c.entry->native_site=address;set_state(a,state);
}
void set_mask(State& a,U value) noexcept {
    a.temporary_mask_esp10=value;continuation(a).temporary_mask_esp10=value;
}
void construct_name(std::size_t index,const char* literal,U address,Context& c,State& a) {
    site(a,address,a.native_state);
    construct_native_string_header_0041e870(&a.names[index],c.passes.post_effects.raw_strings,literal);
    a.name_constructed[index]=true;
}
void return_name(std::size_t index,U clear_mask,U getter,U release,Context& c,State& a) {
    void* const data=child(&a.names[index],4);
    if(clear_mask)set_mask(a,a.temporary_mask_esp10&~clear_mask);
    set_state(a,-1);
    if(!data)return;
    const U bytes=word(&a.names[index])+1u;
    a.captured_name_data=data;a.captured_name_bytes=bytes;a.name_return_started[index]=true;
    auto& strings=c.passes.post_effects.raw_strings;
    site(a,getter,-1);
    auto* const pool=native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
    site(a,release,-1);
    return_native_string_pool_00bd1510(pool,data,bytes,strings.actual_small_returns_disabled_01090aa4);
    a.name_returned[index]=true; // Retain the stale raw header.
}
struct ParameterStep {
    U offset,name,accessor,registration,getter,release,vector_count;
    int state;
    bool receiver_first;
};
constexpr ParameterStep parameters[]={
    {0,0xb12d41,0xb12d60,0xb12d67,0xb12d8c,0xb12d93,0,98,true},
    {0x194,0xb12da4,0xb12dc9,0xb12dd0,0xb12df5,0xb12dfc,0,99,true},
    {0x628,0xb12e0d,0xb12e34,0xb12e3b,0xb12e60,0xb12e67,1,100,false},
    {0x638,0xb12e78,0xb12e9f,0xb12ea6,0xb12ecb,0xb12ed2,1,101,true},
    {0xf4,0xb12ee3,0xb12f0a,0xb12f11,0xb12f36,0xb12f3d,9,102,true}};
void parameter(std::size_t index,Context& c,State& a) {
    const auto& step=parameters[index];const auto n=index+1;
    const auto& old_names=*a.previous->previous->names_identity;
    const auto& inherited=*a.previous->previous->previous->previous->context_identity;
    const char* literals[]={c.parameter_names[0],inherited.displacement_00d5e2f8,
        old_names.horizontal_00d5e260,old_names.vertical_00d5e254,inherited.aa_offsets_00d5e2e4};
    construct_name(n,literals[index],step.name,c,a);
    void* const service=continuation(a).entry->service;
    void* effect=nullptr;
    if(step.receiver_first)effect=child(service,0x654);
    // B12D4C consumes inherited EBX after current654, without a fresh LEA.
    const void* const source=index==0?reinterpret_cast<void*>(a.ebx_bits):at(service,step.offset);
    if(!step.receiver_first)effect=child(service,0x654);
    a.parameter_sources[index]=source;
    site(a,step.accessor,step.state);
    auto* const material=static_cast<NativeMaterialStorage*>(native_post_effect_material_00b4cba0(effect));
    a.parameter_materials[index]=material;
    site(a,step.registration,step.state);
    if(!step.vector_count)register_native_material_float2_00b18b00(*material,&a.names[n],source,c.passes.parameters);
    else {
        // Existing complete B18AC0: DWORD4*count,matrix0,RET0C.
        register_native_material_parameter_00b17e10(*material,&a.names[n],source,4u*step.vector_count,0,c.passes.parameters);
    }
    return_name(n,0,step.getter,step.release,c,a);
}
} // namespace

void continue_native_render_resource_init_00b12c9a_fragment(Previous& previous,Context& c,const Names& names,State& a) {
    auto* const p65c=previous.previous;
    auto* const p658=p65c?p65c->previous:nullptr;
    auto* const passthrough=p658?p658->previous:nullptr;
    auto* const distortion=passthrough?passthrough->previous:nullptr;
    auto* const bloom=distortion?distortion->previous:nullptr;
    auto* const dof=bloom?bloom->previous:nullptr;
    auto* const post=dof?dof->previous:nullptr;
    auto* const prior=post?post->previous:nullptr;
    auto* const entry=prior?prior->entry:nullptr;
    if(a.phase!=State::Phase::fresh || previous.phase!=Previous::Phase::awaiting_b12c9a_continuation ||
       previous.native_site!=0x00b12c9au || previous.native_state!=-1 || previous.temporary_mask_esp10 ||
       previous.tail_identity || !previous.names_identity || !previous.field664_published || !p65c ||
       p65c->phase!=Post65c::Phase::awaiting_later_continuation || p65c->post664_identity!=&previous ||
       p65c->native_site!=0x00b12c9au || p65c->native_state!=-1 || p65c->temporary_mask_esp10 || !p65c->names_identity || !p658 ||
       p658->phase!=Post658::Phase::awaiting_later_continuation || p658->post65c_identity!=p65c ||
       p658->native_site!=0x00b12c9au || p658->native_state!=-1 || p658->temporary_mask_esp10 || !passthrough ||
       passthrough->phase!=Passthrough::Phase::awaiting_later_continuation || passthrough->post658_identity!=p658 ||
       passthrough->native_site!=0x00b12c9au || passthrough->native_state!=-1 || passthrough->temporary_mask_esp10 ||
       !passthrough->context_identity || !distortion || distortion->phase!=Distortion::Phase::awaiting_later_continuation ||
       distortion->passthrough_dust_identity!=passthrough || distortion->native_site!=0x00b12c9au || distortion->native_state!=-1 ||
       !bloom || bloom->phase!=Bloom::Phase::awaiting_later_continuation || bloom->distortion_identity!=distortion ||
       bloom->native_site!=0x00b12c9au || bloom->native_state!=-1 || !dof ||
       dof->phase!=Dof::Phase::awaiting_later_continuation || dof->second_bloom_identity!=bloom ||
       dof->native_site!=0x00b12c9au || dof->native_state!=-1 || dof->temporary_mask_esp10 ||
       !post || post->phase!=Post::Phase::awaiting_later_continuation || post->dof_identity!=dof ||
       post->native_site!=0x00b12c9au || post->native_state!=-1 || !prior ||
       prior->phase!=Continuation::Phase::awaiting_later_continuation || prior->post_identity!=post ||
       prior->context_identity!=&c || prior->native_site!=0x00b12c9au || prior->native_state!=-1 || prior->temporary_mask_esp10 ||
       !entry || entry->phase!=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation ||
       entry->native_site!=0x00b12c9au || entry->unwind_state!=-1 || entry->continuation_identity!=prior ||
       previous.entry_identity!=entry || !entry->argument_cells || previous.argument_cells_identity!=entry->argument_cells ||
       previous.ebx_bits!=bits(at(entry->service,4)) || prior->ebp_bits!=0xffffffffu ||
       prior->edi_bits!=bits(previous.raw_post) || prior->spill_esp14!=bits(previous.raw_post) ||
       !names.wave_00d5e1e4 || !names.clear_00d5e1d0 || !c.parameter_names[0] ||
       !p65c->names_identity->horizontal_00d5e260 || !p65c->names_identity->vertical_00d5e254 ||
       !passthrough->context_identity->displacement_00d5e2f8 || !passthrough->context_identity->aa_offsets_00d5e2e4)
        throw std::logic_error("render tail requires the unconsumed B12C9A frontier and same inherited/context identities");
    a.previous=&previous;a.names_identity=&names;a.entry_identity=entry;a.argument_cells_identity=entry->argument_cells;
    a.ebx_bits=previous.ebx_bits;previous.tail_identity=&a;
    a.phase=State::Phase::preparing;previous.phase=Previous::Phase::tail_running;
    p65c->phase=Post65c::Phase::post664_running;p658->phase=Post658::Phase::post65c_running;
    passthrough->phase=Passthrough::Phase::post658_running;distortion->phase=Distortion::Phase::passthrough_dust_running;
    bloom->phase=Bloom::Phase::distortion_running;dof->phase=Dof::Phase::second_bloom_running;
    post->phase=Post::Phase::dof_running;prior->phase=Continuation::Phase::post_running;
    entry->phase=NativeRenderResourceInitEntryState::Phase::continuation_running;
    site(a,0xb12c9a,-1);
    try {
        a.wave.prepare(c.passes.post_effects);
        a.phase=State::Phase::running;
        void* const service=entry->service;
        site(a,0xb12c9c,-1);
        a.raw_wave=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
        prior->edi_bits=bits(a.raw_wave);prior->spill_esp14=bits(a.raw_wave);set_state(a,95);
        if(a.raw_wave) {
            construct_name(0,names.wave_00d5e1e4,0xb12cc5,c,a);
            set_mask(a,a.temporary_mask_esp10|0x200u);
            site(a,0xb12ce8,96);
            a.returned_wave=construct_native_post_effect20_00b4e470(a.raw_wave,0x20,a.names[0],3,nullptr,a.wave);
        }
        const bool return_wave_name=(a.temporary_mask_esp10&0x200u)!=0;
        put(service,0x654,bits(a.returned_wave));a.field654_published=true;
        set_state(a,-1); // B12CFF AFTER the B12CF9 publication. EBP remains inherited -1.
        if(return_wave_name)return_name(0,0x200,0xb12d29,0xb12d30,c,a);
        for(std::size_t i=0;i<5;++i)parameter(i,c,a);
        site(a,0xb12f42,-1);a.captured_frame=static_cast<NativeFrameTargetOwnerStorage*>(child(service,0x1d4));
        a.captured_frame_post=child(service,0x654);
        site(a,0xb12f4f,-1);
        assign_native_post_effect_frame_00b4e2b0(a.captured_frame_post,a.captured_frame,c.entry,
            c.passes.holders.levels.actual_increment_00ce221c);

        site(a,0xb12f54,-1);
        a.clear.prepare(c.passes.post_effects); // Distinct block; keep completed wave acquisitions on failure.
        site(a,0xb12f56,-1);
        a.raw_clear=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
        prior->edi_bits=bits(a.raw_clear);prior->spill_esp14=bits(a.raw_clear);set_state(a,103);
        a.ebx_bits=0x400u; // B12F71 before the conditional, including a null allocation.
        if(a.raw_clear) {
            construct_name(6,names.clear_00d5e1d0,0xb12f84,c,a);
            set_mask(a,a.temporary_mask_esp10|a.ebx_bits);
            site(a,0xb12fa3,104);
            a.returned_clear=construct_native_post_effect20_00b4e470(a.raw_clear,0x20,a.names[6],3,nullptr,a.clear);
        }
        const bool return_clear_name=(a.temporary_mask_esp10&a.ebx_bits)!=0; // B12FAC, before native POPs.
        a.native_edi_restore_observed=true; // B12FB0, BEFORE publication; no fabricated saved EDI.
        put(service,0x660,bits(a.returned_clear));a.field660_published=true;
        set_state(a,-1); // B12FB7 after publication, with one native POP's displacement.
        a.native_ebx_restore_observed=true; // B12FBE, BEFORE branch/name capture; no later EBX use.
        if(return_clear_name)return_name(6,0,0xb12fda,0xb12fe1,c,a); // Native NEVER clears bit400.

        site(a,0xb12fe6,-1);a.first_focus_gate=byte(service,0x1c4);a.first_focus_gate_sampled=true;
        if(a.first_focus_gate) {
            a.captured_focus_batch=child(service,0x20);
            if(a.captured_focus_batch) {
                site(a,0xb12ff6,-1);
                mark_native_render_batch_dirty_00b50010(a.captured_focus_batch);
                a.focus_batch_marked=true;
            }
            site(a,0xb12ffb,-1);a.second_focus_gate=byte(service,0x1c4);a.second_focus_gate_sampled=true;
            if(a.second_focus_gate) {
                a.captured_focus_root_owner=child(service,0x30);
                if(a.captured_focus_root_owner) {
                    site(a,0xb1300b,-1);
                    invalidate_native_render_root_chain_00b4ecc0(a.captured_focus_root_owner);
                    a.focus_root_invalidated=true;
                }
            }
        }
        site(a,0xb13010,-1);a.phase=State::Phase::normal_work_complete_at_b13010;
        previous.phase=Previous::Phase::normal_work_complete;p65c->phase=Post65c::Phase::awaiting_later_continuation;
        p658->phase=Post658::Phase::awaiting_later_continuation;passthrough->phase=Passthrough::Phase::awaiting_later_continuation;
        distortion->phase=Distortion::Phase::awaiting_later_continuation;bloom->phase=Bloom::Phase::awaiting_later_continuation;
        dof->phase=Dof::Phase::awaiting_later_continuation;post->phase=Post::Phase::awaiting_later_continuation;
        prior->phase=Continuation::Phase::awaiting_later_continuation;entry->phase=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation;
        // No native stack/SEH epilogue execution or full-initializer completion mark.
    } catch(...) {
        a.phase=State::Phase::failed;previous.phase=Previous::Phase::failed;p65c->phase=Post65c::Phase::failed;
        p658->phase=Post658::Phase::failed;passthrough->phase=Passthrough::Phase::failed;distortion->phase=Distortion::Phase::failed;
        bloom->phase=Bloom::Phase::failed;dof->phase=Dof::Phase::failed;post->phase=Post::Phase::failed;
        prior->phase=Continuation::Phase::failed;entry->phase=NativeRenderResourceInitEntryState::Phase::failed;
        // Retain publications, names/mask/register observations and all genuine
        // provider acquisitions. No caller repair, rollback, free or retry.
        throw;
    }
}
} // namespace bsp
