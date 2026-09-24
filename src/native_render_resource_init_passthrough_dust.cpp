#include "bsp/native_render_resource_init_passthrough_dust.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource passthrough and dust stage requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
using Context=NativeRenderResourceInitContinuationContext;
using Continuation=NativeRenderResourceInitContinuationState;
using Post=NativeRenderResourceInitPostState;
using Dof=NativeRenderResourceInitDofState;
using Bloom=NativeRenderResourceInitSecondBloomState;
using Previous=NativeRenderResourceInitDistortionState;
using State=NativeRenderResourceInitPassthroughDustState;
using StageContext=NativeRenderResourceInitPassthroughDustContext;
void* at(const void* p,U offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
U bits(const void* p) noexcept {return reinterpret_cast<U>(p);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(at(p,offset))=value;}
void* child(const void* p,U offset) noexcept {return reinterpret_cast<void*>(word(p,offset));}
Continuation& continuation(State& a) noexcept {return *a.previous->previous->previous->previous->previous;}
void set_state(State& a,int value) noexcept {
    auto& previous=*a.previous;auto& bloom=*previous.previous;auto& dof=*bloom.previous;
    auto& post=*dof.previous;auto& c=*post.previous;
    a.native_state=value;previous.native_state=value;bloom.native_state=value;
    dof.native_state=value;post.native_state=value;c.native_state=value;c.entry->unwind_state=value;
}
void site(State& a,U address,int state) noexcept {
    auto& previous=*a.previous;auto& bloom=*previous.previous;auto& dof=*bloom.previous;
    auto& post=*dof.previous;auto& c=*post.previous;
    a.native_site=address;previous.native_site=address;bloom.native_site=address;
    dof.native_site=address;post.native_site=address;c.native_site=address;c.entry->native_site=address;
    set_state(a,state);
}
void set_mask(State& a,U value) noexcept {
    a.temporary_mask_esp10=value;continuation(a).temporary_mask_esp10=value;
}
void require_domains(Context& c,StageContext& d) {
    auto& p=c.passes.post_effects;auto& q=d.dust.construction;
    if(!d.effect_00d5e314 || !d.displacement_00d5e2f8 || !d.aa_offsets_00d5e2e4 ||
       !c.parameter_names[0] || !d.dust.effect_name_00d620ac ||
       &q.destruction.actual_owners!=&p.destruction.actual_owners ||
       &q.destruction.nodes!=&p.destruction.nodes ||
       &q.destruction.frame_targets!=&c.entry.frame_targets ||
       &q.destruction.actual_decrement_00ce2220!=&c.entry.decrement_iat_00ce2220 ||
       q.destruction.actual_frame_profile_00d5e600!=c.entry.actual_frame_profile_00d5e600 ||
       &q.raw_strings!=&p.raw_strings || &q.strings!=&p.strings ||
       &q.actual_renderer_00f8d394!=&p.actual_renderer_00f8d394 ||
       q.renderer_profile_00d5f0a8!=p.renderer_profile_00d5f0a8 ||
       q.surface_profile_00d619a0!=p.surface_profile_00d619a0 ||
       q.viewport_profile_00d5e5f8!=p.viewport_profile_00d5e5f8 ||
       &q.meshes!=&p.meshes || &q.sections!=&p.sections || &q.materials!=&p.materials ||
       &q.material_effects!=&p.material_effects || &q.declaration_companions!=&p.declaration_companions ||
       &q.declarations!=&p.declarations || &q.streams!=&p.streams || &q.layouts!=&p.layouts ||
       &q.models!=&p.models || &q.cameras!=&p.cameras || &q.viewports!=&p.viewports ||
       &q.draw_entries!=&p.draw_entries || &q.node_constants!=&p.node_constants)
        throw std::logic_error("passthrough and dust require the same actual domains and original literal views");
}
void construct_name(std::size_t index,const char* literal,U address,Context& c,State& a) {
    site(a,address,a.native_state);
    construct_native_string_header_0041e870(&a.names[index],c.passes.post_effects.raw_strings,literal);
    a.name_constructed[index]=true;
}
void return_name(std::size_t index,U mask,bool reset_edi,U getter,U release,Context& c,State& a) {
    void* const data=child(&a.names[index],4);
    if(mask)set_mask(a,a.temporary_mask_esp10&~mask);
    // B12017 captures data BEFORE B1201B replaces EDI withFFFFFFFF.
    if(reset_edi)continuation(a).edi_bits=0xffffffffu;
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
    a.name_returned[index]=true;
}
struct ParameterStep {
    U offset,name,accessor,registration,getter,release;
    int state;
};
constexpr ParameterStep parameters[]={
    {0x4,0xb11f8f,0xb11fae,0xb11fb5,0xb11fd4,0xb11fdb,59},
    {0x194,0xb11fe9,0xb1200b,0xb12012,0xb12034,0xb1203b,60},
    {0xf4,0xb12049,0xb1206d,0xb12074,0xb12093,0xb1209a,61}};
void parameter(std::size_t index,Context& c,StageContext& d,State& a) {
    const auto& step=parameters[index];const auto n=index+1;
    const char* const literal=index==0?c.parameter_names[0]:
        index==1?d.displacement_00d5e2f8:d.aa_offsets_00d5e2e4;
    construct_name(n,literal,step.name,c,a);
    auto& prior=continuation(a);void* const service=prior.entry->service;
    // First two rows load current+650 BEFORE forming the source. Third row
    // sets EBP to service+F4 BEFORE its current+650 load at B1205C.
    void* effect=nullptr;
    if(index<2)effect=child(service,0x650);
    const void* const source=at(service,step.offset);
    if(index==1)a.ebx_bits=bits(source);
    if(index==2) {prior.ebp_bits=bits(source);effect=child(service,0x650);}
    a.parameter_sources[index]=source;
    site(a,step.accessor,step.state);
    auto* const material=static_cast<NativeMaterialStorage*>(native_post_effect_material_00b4cba0(effect));
    a.parameter_materials[index]=material;
    site(a,step.registration,step.state);
    if(index<2)register_native_material_float2_00b18b00(*material,&a.names[n],source,c.passes.parameters);
    else {
        // Existing complete B18AC0: 4*vector_count DWORDs, matrix0, RET0C.
        // B1204E supplies vector_count9; preserve a borrowed36-word record.
        register_native_material_parameter_00b17e10(*material,&a.names[n],source,4u*9u,0,c.passes.parameters);
    }
    return_name(n,0,index==1,step.getter,step.release,c,a);
}
} // namespace

void continue_native_render_resource_init_00b11ef4_fragment(Previous& previous,Context& c,
    StageContext& d,State& a) {
    auto* const bloom=previous.previous;
    auto* const dof=bloom?bloom->previous:nullptr;
    auto* const post=dof?dof->previous:nullptr;
    auto* const prior=post?post->previous:nullptr;
    auto* const entry=prior?prior->entry:nullptr;
    if(a.phase!=State::Phase::fresh || previous.phase!=Previous::Phase::awaiting_b11ef4_continuation ||
       previous.native_site!=0x00b11ef4u || previous.native_state!=-1 || previous.passthrough_dust_identity ||
       !previous.context_identity || !previous.frame_published || !bloom ||
       bloom->phase!=Bloom::Phase::awaiting_later_continuation || bloom->distortion_identity!=&previous ||
       bloom->native_site!=0x00b11ef4u || bloom->native_state!=-1 || !dof ||
       dof->phase!=Dof::Phase::awaiting_later_continuation || dof->second_bloom_identity!=bloom ||
       dof->native_site!=0x00b11ef4u || dof->native_state!=-1 || dof->temporary_mask_esp10 ||
       !post || post->phase!=Post::Phase::awaiting_later_continuation || post->dof_identity!=dof ||
       post->native_site!=0x00b11ef4u || post->native_state!=-1 || !prior ||
       prior->phase!=Continuation::Phase::awaiting_later_continuation || prior->post_identity!=post ||
       prior->context_identity!=&c || prior->native_site!=0x00b11ef4u || prior->native_state!=-1 ||
       prior->temporary_mask_esp10 || prior->ebp_bits!=prior->spill_esp20 || !entry ||
       entry->phase!=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation ||
       entry->native_site!=0x00b11ef4u || entry->unwind_state!=-1 || entry->continuation_identity!=prior ||
       previous.entry_identity!=entry || !entry->argument_cells || previous.argument_cells_identity!=entry->argument_cells)
        throw std::logic_error("passthrough and dust require an unconsumed B11EF4 state and original identities");
    require_domains(c,d);
    a.previous=&previous;a.context_identity=&d;a.entry_identity=entry;
    a.argument_cells_identity=entry->argument_cells;previous.passthrough_dust_identity=&a;
    a.phase=State::Phase::preparing;previous.phase=Previous::Phase::passthrough_dust_running;
    bloom->phase=Bloom::Phase::distortion_running;dof->phase=Dof::Phase::second_bloom_running;
    post->phase=Post::Phase::dof_running;prior->phase=Continuation::Phase::post_running;
    entry->phase=NativeRenderResourceInitEntryState::Phase::continuation_running;
    site(a,0xb11ef4,-1);
    try {
        a.post.prepare(c.passes.post_effects);
        a.dust.prepare(d.dust);
        a.phase=State::Phase::running;
        void* const service=entry->service;
        site(a,0xb11ef6,-1);
        a.raw_post=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
        prior->edi_bits=bits(a.raw_post);prior->spill_esp14=bits(a.raw_post);set_state(a,56);
        if(a.raw_post) {
            construct_name(0,d.effect_00d5e314,0xb11f1f,c,a);
            set_mask(a,a.temporary_mask_esp10|0x20u);
            site(a,0xb11f3f,57);
            a.returned_post=construct_native_post_effect20_00b4e470(a.raw_post,0x20,a.names[0],3,nullptr,a.post);
        }
        const bool cleanup=(a.temporary_mask_esp10&0x20u)!=0; // B11F48 before publication.
        put(service,0x650,bits(a.returned_post));a.field650_published=true;
        set_state(a,-1); // B11F53 AFTER actual result publication.
        if(cleanup)return_name(0,0x20,false,0xb11f7a,0xb11f81,c,a);
        for(std::size_t i=0;i<3;++i)parameter(i,c,d,a);
        site(a,0xb1209f,-1);
        a.captured_frame=static_cast<NativeFrameTargetOwnerStorage*>(child(service,0x1d4));
        a.captured_post=child(service,0x650);
        site(a,0xb120ac,-1);
        assign_native_post_effect_frame_00b4e2b0(a.captured_post,a.captured_frame,c.entry,
            c.passes.holders.levels.actual_increment_00ce221c);
        site(a,0xb120b1,-1);a.captured_dust_receiver=child(service,0x34);
        site(a,0xb120b4,-1);
        initialize_native_post_effect_dust_00b52860(a.captured_dust_receiver,0xcc,a.dust);
        site(a,0xb120b9,-1);a.phase=State::Phase::awaiting_b120b9_continuation;
        previous.phase=Previous::Phase::awaiting_later_continuation;
        bloom->phase=Bloom::Phase::awaiting_later_continuation;dof->phase=Dof::Phase::awaiting_later_continuation;
        post->phase=Post::Phase::awaiting_later_continuation;prior->phase=Continuation::Phase::awaiting_later_continuation;
        entry->phase=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation;
    } catch(...) {
        a.phase=State::Phase::failed;previous.phase=Previous::Phase::failed;bloom->phase=Bloom::Phase::failed;
        dof->phase=Dof::Phase::failed;post->phase=Post::Phase::failed;prior->phase=Continuation::Phase::failed;
        entry->phase=NativeRenderResourceInitEntryState::Phase::failed;
        // Preserve actual publications, name/mask state and both real child
        // blocks. Their existing failure policies are authoritative.
        throw;
    }
}
} // namespace bsp
