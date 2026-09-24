#include "bsp/native_render_resource_init_post664.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource post664 stage requires MSVC Win32.
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
using Previous=NativeRenderResourceInitPost65cState;
using State=NativeRenderResourceInitPost664State;
using Names=NativeRenderResourceInitPost664Names;
void* at(const void* p,U offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
U bits(const void* p) noexcept {return reinterpret_cast<U>(p);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(at(p,offset))=value;}
void* child(const void* p,U offset) noexcept {return reinterpret_cast<void*>(word(p,offset));}
Continuation& continuation(State& a) noexcept {return *a.previous->previous->previous->previous->previous->previous->previous->previous;}
void set_state(State& a,int value) noexcept {
    auto& previous=*a.previous;auto& p658=*previous.previous;auto& passthrough=*p658.previous;
    auto& distortion=*passthrough.previous;auto& bloom=*distortion.previous;auto& dof=*bloom.previous;
    auto& post=*dof.previous;auto& c=*post.previous;
    a.native_state=value;previous.native_state=value;p658.native_state=value;passthrough.native_state=value;
    distortion.native_state=value;bloom.native_state=value;dof.native_state=value;post.native_state=value;
    c.native_state=value;c.entry->unwind_state=value;
}
void site(State& a,U address,int state) noexcept {
    auto& previous=*a.previous;auto& p658=*previous.previous;auto& passthrough=*p658.previous;
    auto& distortion=*passthrough.previous;auto& bloom=*distortion.previous;auto& dof=*bloom.previous;
    auto& post=*dof.previous;auto& c=*post.previous;
    a.native_site=address;previous.native_site=address;p658.native_site=address;passthrough.native_site=address;
    distortion.native_site=address;bloom.native_site=address;dof.native_site=address;post.native_site=address;
    c.native_site=address;c.entry->native_site=address;set_state(a,state);
}
void set_mask(State& a,U value) noexcept {
    a.temporary_mask_esp10=value;continuation(a).temporary_mask_esp10=value;
}
void construct_name(std::size_t index,const char* literal,U address,Context& c,State& a) {
    site(a,address,a.native_state);
    construct_native_string_header_0041e870(&a.names[index],c.passes.post_effects.raw_strings,literal);
    a.name_constructed[index]=true;
}
void return_name(std::size_t index,U mask,U getter,U release,Context& c,State& a) {
    void* const data=child(&a.names[index],4); // Capture BEFORE mask clear/disarm.
    if(mask)set_mask(a,a.temporary_mask_esp10&~mask);
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
    a.name_returned[index]=true; // Preserve the stale raw header.
}
struct ParameterStep {
    U offset,name,accessor,registration,getter,release,words;
    int state;
    bool receiver_first;
};
constexpr ParameterStep parameters[]={
    {0x1a8,0xb12a21,0xb12a46,0xb12a4d,0xb12a72,0xb12a79,2,89,false},
    {0x1b0,0xb12a8a,0xb12aaf,0xb12ab6,0xb12adb,0xb12ae2,1,90,false},
    {0x238,0xb12af3,0xb12b18,0xb12b1f,0xb12b44,0xb12b4b,1,91,true},
    {0x240,0xb12b5c,0xb12b81,0xb12b88,0xb12bad,0xb12bb4,1,92,false},
    {0x23c,0xb12bc5,0xb12bea,0xb12bf1,0xb12c16,0xb12c1d,1,93,false},
    {4,0xb12c2e,0xb12c50,0xb12c57,0xb12c7c,0xb12c83,2,94,true}};
void parameter(std::size_t index,Context& c,const Names& names,State& a) {
    const auto& step=parameters[index];const auto n=index+1;
    const char* literals[]={names.camera_velocity_00d5e230,names.camera_speed_00d5e220,
        names.min_speed_00d5e214,names.max_blur_00d5e208,names.divider_00d5e1fc,c.parameter_names[0]};
    construct_name(n,literals[index],step.name,c,a);
    void* const service=continuation(a).entry->service;
    void* effect=nullptr;
    if(step.receiver_first)effect=child(service,0x664);
    const void* source=at(service,step.offset);
    if(index==5) {
        a.ebx_bits=bits(source); // B12C39, AFTER current664 capture; inherited EBX100 ends here.
        source=reinterpret_cast<void*>(a.ebx_bits);
    }
    if(!step.receiver_first)effect=child(service,0x664);
    a.parameter_sources[index]=source;
    site(a,step.accessor,step.state);
    auto* const material=static_cast<NativeMaterialStorage*>(native_post_effect_material_00b4cba0(effect));
    a.parameter_materials[index]=material;
    site(a,step.registration,step.state);
    if(step.words==1)register_native_material_float_00b18b20(*material,&a.names[n],source,c.passes.parameters);
    else register_native_material_float2_00b18b00(*material,&a.names[n],source,c.passes.parameters);
    return_name(n,0,step.getter,step.release,c,a);
}
} // namespace

void continue_native_render_resource_init_00b1297a_fragment(Previous& previous,Context& c,const Names& names,State& a) {
    auto* const p658=previous.previous;
    auto* const passthrough=p658?p658->previous:nullptr;
    auto* const distortion=passthrough?passthrough->previous:nullptr;
    auto* const bloom=distortion?distortion->previous:nullptr;
    auto* const dof=bloom?bloom->previous:nullptr;
    auto* const post=dof?dof->previous:nullptr;
    auto* const prior=post?post->previous:nullptr;
    auto* const entry=prior?prior->entry:nullptr;
    if(a.phase!=State::Phase::fresh || previous.phase!=Previous::Phase::awaiting_b1297a_continuation ||
       previous.native_site!=0x00b1297au || previous.native_state!=-1 || previous.temporary_mask_esp10 ||
       previous.post664_identity || !previous.names_identity || !previous.field65c_published || !p658 ||
       p658->phase!=Post658::Phase::awaiting_later_continuation || p658->post65c_identity!=&previous ||
       p658->native_site!=0x00b1297au || p658->native_state!=-1 || p658->temporary_mask_esp10 || !passthrough ||
       passthrough->phase!=Passthrough::Phase::awaiting_later_continuation || passthrough->post658_identity!=p658 ||
       passthrough->native_site!=0x00b1297au || passthrough->native_state!=-1 || passthrough->temporary_mask_esp10 ||
       !passthrough->context_identity || !distortion || distortion->phase!=Distortion::Phase::awaiting_later_continuation ||
       distortion->passthrough_dust_identity!=passthrough || distortion->native_site!=0x00b1297au || distortion->native_state!=-1 ||
       !bloom || bloom->phase!=Bloom::Phase::awaiting_later_continuation || bloom->distortion_identity!=distortion ||
       bloom->native_site!=0x00b1297au || bloom->native_state!=-1 || !dof ||
       dof->phase!=Dof::Phase::awaiting_later_continuation || dof->second_bloom_identity!=bloom ||
       dof->native_site!=0x00b1297au || dof->native_state!=-1 || dof->temporary_mask_esp10 ||
       !post || post->phase!=Post::Phase::awaiting_later_continuation || post->dof_identity!=dof ||
       post->native_site!=0x00b1297au || post->native_state!=-1 || !prior ||
       prior->phase!=Continuation::Phase::awaiting_later_continuation || prior->post_identity!=post ||
       prior->context_identity!=&c || prior->native_site!=0x00b1297au || prior->native_state!=-1 || prior->temporary_mask_esp10 ||
       !entry || entry->phase!=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation ||
       entry->native_site!=0x00b1297au || entry->unwind_state!=-1 || entry->continuation_identity!=prior ||
       previous.entry_identity!=entry || !entry->argument_cells || previous.argument_cells_identity!=entry->argument_cells ||
       previous.ebx_bits!=bits(at(entry->service,0x228)) || prior->ebp_bits!=bits(at(entry->service,0x648)) ||
       prior->edi_bits!=0xffffffffu || !names.effect_00d5e240 || !names.camera_velocity_00d5e230 ||
       !names.camera_speed_00d5e220 || !names.min_speed_00d5e214 || !names.max_blur_00d5e208 ||
       !names.divider_00d5e1fc || !c.parameter_names[0])
        throw std::logic_error("post664 requires the unconsumed B1297A frontier and same inherited/context identities");
    a.previous=&previous;a.names_identity=&names;a.entry_identity=entry;a.argument_cells_identity=entry->argument_cells;
    a.ebx_bits=previous.ebx_bits;previous.post664_identity=&a;
    a.phase=State::Phase::preparing;previous.phase=Previous::Phase::post664_running;
    p658->phase=Post658::Phase::post65c_running;passthrough->phase=Passthrough::Phase::post658_running;
    distortion->phase=Distortion::Phase::passthrough_dust_running;bloom->phase=Bloom::Phase::distortion_running;
    dof->phase=Dof::Phase::second_bloom_running;post->phase=Post::Phase::dof_running;
    prior->phase=Continuation::Phase::post_running;entry->phase=NativeRenderResourceInitEntryState::Phase::continuation_running;
    site(a,0xb1297a,-1);
    try {
        a.post.prepare(c.passes.post_effects);
        a.phase=State::Phase::running;
        void* const service=entry->service;
        site(a,0xb1297c,-1);
        a.raw_post=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
        prior->edi_bits=bits(a.raw_post);prior->spill_esp14=bits(a.raw_post);set_state(a,86);
        a.ebx_bits=0x100u; // B12997 even on the null-allocation branch.
        if(a.raw_post) {
            construct_name(0,names.effect_00d5e240,0xb129aa,c,a);
            set_mask(a,a.temporary_mask_esp10|a.ebx_bits); // DWORD OR, not the previous stage's byte mask.
            site(a,0xb129c9,87);
            a.returned_post=construct_native_post_effect20_00b4e470(a.raw_post,0x20,a.names[0],3,nullptr,a.post);
        }
        prior->ebp_bits=0xffffffffu; // B129D2 BEFORE DWORD mask test and publication; EDI stays raw.
        const bool cleanup=(a.temporary_mask_esp10&a.ebx_bits)!=0;
        put(service,0x664,bits(a.returned_post));a.field664_published=true;
        set_state(a,-1); // B129DF AFTER publication.
        if(cleanup)return_name(0,0x100,0xb12a09,0xb12a10,c,a);
        for(std::size_t i=0;i<6;++i)parameter(i,c,names,a);
        site(a,0xb12c88,-1);a.captured_frame=static_cast<NativeFrameTargetOwnerStorage*>(child(service,0x1d4));
        a.captured_frame_post=child(service,0x664);
        site(a,0xb12c95,-1);
        assign_native_post_effect_frame_00b4e2b0(a.captured_frame_post,a.captured_frame,c.entry,
            c.passes.holders.levels.actual_increment_00ce221c);
        site(a,0xb12c9a,-1);a.phase=State::Phase::awaiting_b12c9a_continuation;
        previous.phase=Previous::Phase::awaiting_later_continuation;p658->phase=Post658::Phase::awaiting_later_continuation;
        passthrough->phase=Passthrough::Phase::awaiting_later_continuation;distortion->phase=Distortion::Phase::awaiting_later_continuation;
        bloom->phase=Bloom::Phase::awaiting_later_continuation;dof->phase=Dof::Phase::awaiting_later_continuation;
        post->phase=Post::Phase::awaiting_later_continuation;prior->phase=Continuation::Phase::awaiting_later_continuation;
        entry->phase=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation;
    } catch(...) {
        a.phase=State::Phase::failed;previous.phase=Previous::Phase::failed;p658->phase=Post658::Phase::failed;
        passthrough->phase=Passthrough::Phase::failed;distortion->phase=Distortion::Phase::failed;
        bloom->phase=Bloom::Phase::failed;dof->phase=Dof::Phase::failed;post->phase=Post::Phase::failed;
        prior->phase=Continuation::Phase::failed;entry->phase=NativeRenderResourceInitEntryState::Phase::failed;
        // Retain actual publications, raw names/mask and all acquired children.
        // Provider failure behavior remains authoritative; no caller repair.
        throw;
    }
}
} // namespace bsp
