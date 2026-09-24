#include "bsp/native_render_resource_init_post658.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource post658 stage requires MSVC Win32.
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
using Previous=NativeRenderResourceInitPassthroughDustState;
using State=NativeRenderResourceInitPost658State;
using Names=NativeRenderResourceInitPost658Names;
void* at(const void* p,U offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
U bits(const void* p) noexcept {return reinterpret_cast<U>(p);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(at(p,offset))=value;}
void* child(const void* p,U offset) noexcept {return reinterpret_cast<void*>(word(p,offset));}
Continuation& continuation(State& a) noexcept {return *a.previous->previous->previous->previous->previous->previous;}
void set_state(State& a,int value) noexcept {
    auto& previous=*a.previous;auto& distortion=*previous.previous;auto& bloom=*distortion.previous;
    auto& dof=*bloom.previous;auto& post=*dof.previous;auto& c=*post.previous;
    a.native_state=value;previous.native_state=value;distortion.native_state=value;bloom.native_state=value;
    dof.native_state=value;post.native_state=value;c.native_state=value;c.entry->unwind_state=value;
}
void site(State& a,U address,int state) noexcept {
    auto& previous=*a.previous;auto& distortion=*previous.previous;auto& bloom=*distortion.previous;
    auto& dof=*bloom.previous;auto& post=*dof.previous;auto& c=*post.previous;
    a.native_site=address;previous.native_site=address;distortion.native_site=address;bloom.native_site=address;
    dof.native_site=address;post.native_site=address;c.native_site=address;c.entry->native_site=address;
    set_state(a,state);
}
void set_mask(State& a,U value) noexcept {
    a.temporary_mask_esp10=value;continuation(a).temporary_mask_esp10=value;
}
void construct_name(std::size_t index,const char* literal,U address,Context& c,State& a) {
    site(a,address,a.native_state);
    construct_native_string_header_0041e870(&a.names[index],c.passes.post_effects.raw_strings,literal);
    a.name_constructed[index]=true;
}
void return_name(std::size_t index,U mask,bool reset_edi,U getter,U release,Context& c,State& a) {
    void* const data=child(&a.names[index],4);
    if(mask)set_mask(a,a.temporary_mask_esp10&~mask);
    // B122BC captures letterbox name data BEFORE B122C3 resets EDI.
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
    a.name_returned[index]=true; // Actual raw header remains stale.
}
enum class Source { service,ebx,ebp,owner34 };
enum class Order { receiver_first,source_first,owner_receiver_offset };
struct ParameterStep {
    U offset,name,accessor,registration,getter,release,words;
    int state;
    Source source;
    Order order;
};
constexpr ParameterStep parameters[]={
    {4,0xb12166,0xb12185,0xb1218c,0xb121ab,0xb121b2,2,65,Source::service,Order::receiver_first},
    {0,0xb121c3,0xb121e2,0xb121e9,0xb1220e,0xb12215,2,66,Source::ebx,Order::receiver_first},
    {0,0xb12226,0xb12247,0xb1224e,0xb12273,0xb1227a,36,67,Source::ebp,Order::source_first},
    {0x228,0xb1228b,0xb122b0,0xb122b7,0xb122df,0xb122e6,1,68,Source::service,Order::receiver_first},
    {8,0xb122f7,0xb1231c,0xb12323,0xb12348,0xb1234f,2,69,Source::owner34,Order::source_first},
    {0x648,0xb12360,0xb12385,0xb1238c,0xb123b1,0xb123b8,2,70,Source::service,Order::receiver_first},
    {0x10,0xb123c9,0xb123ee,0xb123f5,0xb1241a,0xb12421,1,71,Source::owner34,Order::source_first},
    {0x14,0xb12432,0xb12457,0xb1245e,0xb12483,0xb1248a,1,72,Source::owner34,Order::owner_receiver_offset}};
void parameter(std::size_t index,Context& c,const Names& names,State& a) {
    const auto& step=parameters[index];const auto n=index+1;
    const auto& inherited=*a.previous->context_identity;
    const char* literals[]={c.parameter_names[0],inherited.displacement_00d5e2f8,
        inherited.aa_offsets_00d5e2e4,names.letterbox_00d5e2bc,names.noise_offset_00d5e2ac,
        names.noise_scale_00d5e2a0,names.flicker_00d5e294,names.shake_00d5e28c};
    construct_name(n,literals[index],step.name,c,a);
    auto& prior=continuation(a);void* const service=prior.entry->service;
    void* effect=nullptr;const void* source=nullptr;
    if(step.order==Order::receiver_first)effect=child(service,0x658);
    switch(step.source) {
    case Source::service: source=at(service,step.offset);break;
    case Source::ebx: source=reinterpret_cast<void*>(a.ebx_bits);break; // Actual inherited B121CE use.
    case Source::ebp: source=reinterpret_cast<void*>(prior.ebp_bits);break; // Actual inherited B1222D use.
    case Source::owner34:
        a.captured_parameter_owners[index]=child(service,0x34);
        // Shake: current+34, current+658, THEN ADD14, in that order.
        if(step.order==Order::owner_receiver_offset)effect=child(service,0x658);
        source=at(a.captured_parameter_owners[index],step.offset);break;
    }
    if(index==3)a.ebx_bits=bits(source); // B12296, only AFTER inherited EBX was used.
    if(index==5)prior.ebp_bits=bits(source); // B1236B, only AFTER inherited EBP was used.
    if(step.order==Order::source_first)effect=child(service,0x658);
    a.parameter_sources[index]=source;
    site(a,step.accessor,step.state);
    auto* const material=static_cast<NativeMaterialStorage*>(native_post_effect_material_00b4cba0(effect));
    a.parameter_materials[index]=material;
    site(a,step.registration,step.state);
    if(step.words==1)register_native_material_float_00b18b20(*material,&a.names[n],source,c.passes.parameters);
    else if(step.words==2)register_native_material_float2_00b18b00(*material,&a.names[n],source,c.passes.parameters);
    else {
        // Existing full B18AC0: wrapping DWORD4*count, matrix0. Here count9.
        register_native_material_parameter_00b17e10(*material,&a.names[n],source,4u*9u,0,c.passes.parameters);
    }
    return_name(n,0,index==3,step.getter,step.release,c,a);
}
} // namespace

void continue_native_render_resource_init_00b120b9_fragment(Previous& previous,Context& c,
    const Names& names,State& a) {
    auto* const distortion=previous.previous;
    auto* const bloom=distortion?distortion->previous:nullptr;
    auto* const dof=bloom?bloom->previous:nullptr;
    auto* const post=dof?dof->previous:nullptr;
    auto* const prior=post?post->previous:nullptr;
    auto* const entry=prior?prior->entry:nullptr;
    if(a.phase!=State::Phase::fresh || previous.phase!=Previous::Phase::awaiting_b120b9_continuation ||
       previous.native_site!=0x00b120b9u || previous.native_state!=-1 || previous.temporary_mask_esp10 ||
       previous.post658_identity || !previous.context_identity || !previous.field650_published || !distortion ||
       distortion->phase!=Distortion::Phase::awaiting_later_continuation || distortion->passthrough_dust_identity!=&previous ||
       distortion->native_site!=0x00b120b9u || distortion->native_state!=-1 || !bloom ||
       bloom->phase!=Bloom::Phase::awaiting_later_continuation || bloom->distortion_identity!=distortion ||
       bloom->native_site!=0x00b120b9u || bloom->native_state!=-1 || !dof ||
       dof->phase!=Dof::Phase::awaiting_later_continuation || dof->second_bloom_identity!=bloom ||
       dof->native_site!=0x00b120b9u || dof->native_state!=-1 || dof->temporary_mask_esp10 ||
       !post || post->phase!=Post::Phase::awaiting_later_continuation || post->dof_identity!=dof ||
       post->native_site!=0x00b120b9u || post->native_state!=-1 || !prior ||
       prior->phase!=Continuation::Phase::awaiting_later_continuation || prior->post_identity!=post ||
       prior->context_identity!=&c || prior->native_site!=0x00b120b9u || prior->native_state!=-1 ||
       prior->temporary_mask_esp10 || !entry ||
       entry->phase!=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation ||
       entry->native_site!=0x00b120b9u || entry->unwind_state!=-1 || entry->continuation_identity!=prior ||
       previous.entry_identity!=entry || !entry->argument_cells || previous.argument_cells_identity!=entry->argument_cells ||
       previous.ebx_bits!=bits(at(entry->service,0x194)) || prior->ebp_bits!=bits(at(entry->service,0xf4)) ||
       prior->edi_bits!=0xffffffffu || !names.effect_00d5e2cc || !names.letterbox_00d5e2bc ||
       !names.noise_offset_00d5e2ac || !names.noise_scale_00d5e2a0 || !names.flicker_00d5e294 || !names.shake_00d5e28c ||
       !c.parameter_names[0] || !previous.context_identity->displacement_00d5e2f8 || !previous.context_identity->aa_offsets_00d5e2e4)
        throw std::logic_error("post658 requires an unconsumed B120B9 frontier, inherited registers and original identities");
    a.previous=&previous;a.names_identity=&names;a.entry_identity=entry;a.argument_cells_identity=entry->argument_cells;
    a.ebx_bits=previous.ebx_bits;previous.post658_identity=&a;
    a.phase=State::Phase::preparing;previous.phase=Previous::Phase::post658_running;
    distortion->phase=Distortion::Phase::passthrough_dust_running;bloom->phase=Bloom::Phase::distortion_running;
    dof->phase=Dof::Phase::second_bloom_running;post->phase=Post::Phase::dof_running;
    prior->phase=Continuation::Phase::post_running;entry->phase=NativeRenderResourceInitEntryState::Phase::continuation_running;
    site(a,0xb120b9,-1);
    try {
        a.post.prepare(c.passes.post_effects);
        a.phase=State::Phase::running;
        void* const service=entry->service;
        site(a,0xb120bb,-1);
        a.raw_post=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
        prior->edi_bits=bits(a.raw_post);prior->spill_esp14=bits(a.raw_post);set_state(a,62);
        if(a.raw_post) {
            construct_name(0,names.effect_00d5e2cc,0xb120e1,c,a);
            set_mask(a,a.temporary_mask_esp10|0x40u);
            site(a,0xb120fe,63);
            a.returned_post=construct_native_post_effect20_00b4e470(a.raw_post,0x20,a.names[0],3,nullptr,a.post);
        }
        prior->edi_bits=0xffffffffu; // B12107 BEFORE mask test/publication.
        const bool cleanup=(a.temporary_mask_esp10&0x40u)!=0;
        put(service,0x658,bits(a.returned_post));a.field658_published=true;
        set_state(a,-1); // B12115 AFTER publication.
        if(cleanup)return_name(0,0x40,false,0xb12136,0xb1213d,c,a);
        site(a,0xb12142,-1);a.captured_texture=child(service,0x67c);
        a.captured_texture_post=child(service,0x658);
        site(a,0xb12151,-1);a.captured_texture_material=native_post_effect_material_00b4cba0(a.captured_texture_post);
        site(a,0xb12158,-1);
        set_native_material_texture_unchecked_00b189f0(a.captured_texture_material,3,a.captured_texture,
            c.passes.post_effects.destruction.actual_owners);
        for(std::size_t i=0;i<8;++i)parameter(i,c,names,a);
        site(a,0xb1248f,-1);a.captured_frame=static_cast<NativeFrameTargetOwnerStorage*>(child(service,0x1d4));
        a.captured_frame_post=child(service,0x658);
        site(a,0xb1249c,-1);
        assign_native_post_effect_frame_00b4e2b0(a.captured_frame_post,a.captured_frame,c.entry,
            c.passes.holders.levels.actual_increment_00ce221c);
        site(a,0xb124a1,-1);a.phase=State::Phase::awaiting_b124a1_continuation;
        previous.phase=Previous::Phase::awaiting_later_continuation;distortion->phase=Distortion::Phase::awaiting_later_continuation;
        bloom->phase=Bloom::Phase::awaiting_later_continuation;dof->phase=Dof::Phase::awaiting_later_continuation;
        post->phase=Post::Phase::awaiting_later_continuation;prior->phase=Continuation::Phase::awaiting_later_continuation;
        entry->phase=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation;
    } catch(...) {
        a.phase=State::Phase::failed;previous.phase=Previous::Phase::failed;distortion->phase=Distortion::Phase::failed;
        bloom->phase=Bloom::Phase::failed;dof->phase=Dof::Phase::failed;post->phase=Post::Phase::failed;
        prior->phase=Continuation::Phase::failed;entry->phase=NativeRenderResourceInitEntryState::Phase::failed;
        // Preserve publications, borrowed sources and raw/name/provider state.
        // Existing child failure policy stays authoritative; no caller repair.
        throw;
    }
}
} // namespace bsp
