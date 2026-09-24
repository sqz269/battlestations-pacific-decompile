#include "bsp/native_render_resource_init_post65c.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource post65C stage requires MSVC Win32.
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
using Previous=NativeRenderResourceInitPost658State;
using State=NativeRenderResourceInitPost65cState;
using Names=NativeRenderResourceInitPost65cNames;
void* at(const void* p,U offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
U bits(const void* p) noexcept {return reinterpret_cast<U>(p);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(at(p,offset))=value;}
void* child(const void* p,U offset) noexcept {return reinterpret_cast<void*>(word(p,offset));}
Continuation& continuation(State& a) noexcept {return *a.previous->previous->previous->previous->previous->previous->previous;}
void set_state(State& a,int value) noexcept {
    auto& previous=*a.previous;auto& passthrough=*previous.previous;auto& distortion=*passthrough.previous;
    auto& bloom=*distortion.previous;auto& dof=*bloom.previous;auto& post=*dof.previous;auto& c=*post.previous;
    a.native_state=value;previous.native_state=value;passthrough.native_state=value;distortion.native_state=value;
    bloom.native_state=value;dof.native_state=value;post.native_state=value;c.native_state=value;c.entry->unwind_state=value;
}
void site(State& a,U address,int state) noexcept {
    auto& previous=*a.previous;auto& passthrough=*previous.previous;auto& distortion=*passthrough.previous;
    auto& bloom=*distortion.previous;auto& dof=*bloom.previous;auto& post=*dof.previous;auto& c=*post.previous;
    a.native_site=address;previous.native_site=address;passthrough.native_site=address;distortion.native_site=address;
    bloom.native_site=address;dof.native_site=address;post.native_site=address;c.native_site=address;c.entry->native_site=address;
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
void return_name(std::size_t index,U mask,U getter,U release,Context& c,State& a) {
    void* const data=child(&a.names[index],4);
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
enum class Source { service,ebx,ebp,owner34 };
struct ParameterStep {
    U offset,name,accessor,registration,getter,release,words,vector_count;
    int state;
    Source source;
    bool receiver_first;
};
constexpr ParameterStep parameters[]={
    {4,0xb12563,0xb12585,0xb1258c,0xb125b1,0xb125b8,2,0,76,Source::service,false},
    {0x194,0xb125c9,0xb125ee,0xb125f5,0xb1261a,0xb12621,2,0,77,Source::service,true},
    {0xf4,0xb12632,0xb12659,0xb12660,0xb12685,0xb1268c,36,9,78,Source::service,true},
    {0x628,0xb1269d,0xb126c4,0xb126cb,0xb126f0,0xb126f7,4,1,79,Source::service,false},
    {0x638,0xb12708,0xb1272f,0xb12736,0xb1275b,0xb12762,4,1,80,Source::service,true},
    {0,0xb12773,0xb12792,0xb12799,0xb127be,0xb127c5,1,0,81,Source::ebx,true},
    {8,0xb127d6,0xb127fb,0xb12802,0xb12827,0xb1282e,2,0,82,Source::owner34,false},
    {0,0xb1283f,0xb1285e,0xb12865,0xb1288a,0xb12891,2,0,83,Source::ebp,true},
    {0x10,0xb128a2,0xb128c7,0xb128ce,0xb128f3,0xb128fa,1,0,84,Source::owner34,false},
    {0x14,0xb1290b,0xb12930,0xb12937,0xb1295c,0xb12963,1,0,85,Source::owner34,false}};
void parameter(std::size_t index,Context& c,const Names& names,State& a) {
    const auto& step=parameters[index];const auto n=index+1;
    const auto& old_names=*a.previous->names_identity;
    const auto& inherited=*a.previous->previous->context_identity;
    const char* literals[]={c.parameter_names[0],inherited.displacement_00d5e2f8,inherited.aa_offsets_00d5e2e4,
        names.horizontal_00d5e260,names.vertical_00d5e254,old_names.letterbox_00d5e2bc,
        old_names.noise_offset_00d5e2ac,old_names.noise_scale_00d5e2a0,old_names.flicker_00d5e294,old_names.shake_00d5e28c};
    construct_name(n,literals[index],step.name,c,a);
    auto& prior=continuation(a);void* const service=prior.entry->service;
    void* effect=nullptr;const void* source=nullptr;
    if(step.receiver_first)effect=child(service,0x65c);
    switch(step.source) {
    case Source::service: source=at(service,step.offset);break;
    case Source::ebx: source=reinterpret_cast<void*>(a.ebx_bits);break; // B1277E inherited228, not recomputed.
    case Source::ebp: source=reinterpret_cast<void*>(prior.ebp_bits);break; // B1284A inherited648, not recomputed.
    case Source::owner34:
        a.captured_parameter_owners[index]=child(service,0x34);
        source=at(a.captured_parameter_owners[index],step.offset);break;
    }
    // In this stage every owner34 row forms its offset BEFORE reading65C.
    // Scene/HParams are also source-first; all other rows are receiver-first.
    if(!step.receiver_first)effect=child(service,0x65c);
    a.parameter_sources[index]=source;
    site(a,step.accessor,step.state);
    auto* const material=static_cast<NativeMaterialStorage*>(native_post_effect_material_00b4cba0(effect));
    a.parameter_materials[index]=material;
    site(a,step.registration,step.state);
    if(step.words==1)register_native_material_float_00b18b20(*material,&a.names[n],source,c.passes.parameters);
    else if(step.words==2)register_native_material_float2_00b18b00(*material,&a.names[n],source,c.passes.parameters);
    else {
        // Existing complete B18AC0: DWORD4*vector_count,matrix0,RET0C.
        // Native pushes count9 for AA; count1 for HParams and VParams.
        register_native_material_parameter_00b17e10(*material,&a.names[n],source,4u*step.vector_count,0,c.passes.parameters);
    }
    return_name(n,0,step.getter,step.release,c,a);
}
} // namespace

void continue_native_render_resource_init_00b124a1_fragment(Previous& previous,Context& c,const Names& names,State& a) {
    auto* const passthrough=previous.previous;
    auto* const distortion=passthrough?passthrough->previous:nullptr;
    auto* const bloom=distortion?distortion->previous:nullptr;
    auto* const dof=bloom?bloom->previous:nullptr;
    auto* const post=dof?dof->previous:nullptr;
    auto* const prior=post?post->previous:nullptr;
    auto* const entry=prior?prior->entry:nullptr;
    if(a.phase!=State::Phase::fresh || previous.phase!=Previous::Phase::awaiting_b124a1_continuation ||
       previous.native_site!=0x00b124a1u || previous.native_state!=-1 || previous.temporary_mask_esp10 ||
       previous.post65c_identity || !previous.names_identity || !previous.field658_published || !passthrough ||
       passthrough->phase!=Passthrough::Phase::awaiting_later_continuation || passthrough->post658_identity!=&previous ||
       passthrough->native_site!=0x00b124a1u || passthrough->native_state!=-1 || passthrough->temporary_mask_esp10 ||
       !passthrough->context_identity || !distortion || distortion->phase!=Distortion::Phase::awaiting_later_continuation ||
       distortion->passthrough_dust_identity!=passthrough || distortion->native_site!=0x00b124a1u || distortion->native_state!=-1 ||
       !bloom || bloom->phase!=Bloom::Phase::awaiting_later_continuation || bloom->distortion_identity!=distortion ||
       bloom->native_site!=0x00b124a1u || bloom->native_state!=-1 || !dof ||
       dof->phase!=Dof::Phase::awaiting_later_continuation || dof->second_bloom_identity!=bloom ||
       dof->native_site!=0x00b124a1u || dof->native_state!=-1 || dof->temporary_mask_esp10 ||
       !post || post->phase!=Post::Phase::awaiting_later_continuation || post->dof_identity!=dof ||
       post->native_site!=0x00b124a1u || post->native_state!=-1 || !prior ||
       prior->phase!=Continuation::Phase::awaiting_later_continuation || prior->post_identity!=post ||
       prior->context_identity!=&c || prior->native_site!=0x00b124a1u || prior->native_state!=-1 || prior->temporary_mask_esp10 ||
       !entry || entry->phase!=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation ||
       entry->native_site!=0x00b124a1u || entry->unwind_state!=-1 || entry->continuation_identity!=prior ||
       previous.entry_identity!=entry || !entry->argument_cells || previous.argument_cells_identity!=entry->argument_cells ||
       previous.ebx_bits!=bits(at(entry->service,0x228)) || prior->ebp_bits!=bits(at(entry->service,0x648)) ||
       prior->edi_bits!=0xffffffffu || !names.effect_00d5e26c || !names.horizontal_00d5e260 || !names.vertical_00d5e254 ||
       !c.parameter_names[0] || !passthrough->context_identity->displacement_00d5e2f8 || !passthrough->context_identity->aa_offsets_00d5e2e4 ||
       !previous.names_identity->letterbox_00d5e2bc || !previous.names_identity->noise_offset_00d5e2ac ||
       !previous.names_identity->noise_scale_00d5e2a0 || !previous.names_identity->flicker_00d5e294 || !previous.names_identity->shake_00d5e28c)
        throw std::logic_error("post65C requires the unconsumed B124A1 frontier and same inherited/context identities");
    a.previous=&previous;a.names_identity=&names;a.entry_identity=entry;a.argument_cells_identity=entry->argument_cells;
    a.ebx_bits=previous.ebx_bits;previous.post65c_identity=&a;
    a.phase=State::Phase::preparing;previous.phase=Previous::Phase::post65c_running;
    passthrough->phase=Passthrough::Phase::post658_running;distortion->phase=Distortion::Phase::passthrough_dust_running;
    bloom->phase=Bloom::Phase::distortion_running;dof->phase=Dof::Phase::second_bloom_running;
    post->phase=Post::Phase::dof_running;prior->phase=Continuation::Phase::post_running;
    entry->phase=NativeRenderResourceInitEntryState::Phase::continuation_running;
    site(a,0xb124a1,-1);
    try {
        a.post.prepare(c.passes.post_effects);
        a.phase=State::Phase::running;
        void* const service=entry->service;
        site(a,0xb124a3,-1);
        a.raw_post=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
        prior->edi_bits=bits(a.raw_post);prior->spill_esp14=bits(a.raw_post);set_state(a,73);
        if(a.raw_post) {
            construct_name(0,names.effect_00d5e26c,0xb124cc,c,a);
            set_mask(a,a.temporary_mask_esp10|0x80u);
            site(a,0xb124ef,74);
            a.returned_post=construct_native_post_effect20_00b4e470(a.raw_post,0x20,a.names[0],3,nullptr,a.post);
        }
        prior->edi_bits=0xffffffffu; // B124F8 BEFORE mask80 test/publication.
        const bool cleanup=(a.temporary_mask_esp10&0x80u)!=0;
        put(service,0x65c,bits(a.returned_post));a.field65c_published=true;
        set_state(a,-1); // B12506 AFTER publication.
        if(cleanup)return_name(0,0x80,0xb12530,0xb12537,c,a);
        site(a,0xb1253c,-1);a.captured_texture=child(service,0x67c);
        a.captured_texture_post=child(service,0x65c);
        site(a,0xb1254b,-1);a.captured_texture_material=native_post_effect_material_00b4cba0(a.captured_texture_post);
        site(a,0xb12552,-1);
        set_native_material_texture_unchecked_00b189f0(a.captured_texture_material,3,a.captured_texture,
            c.passes.post_effects.destruction.actual_owners);
        for(std::size_t i=0;i<10;++i)parameter(i,c,names,a);
        site(a,0xb12968,-1);a.captured_frame=static_cast<NativeFrameTargetOwnerStorage*>(child(service,0x1d4));
        a.captured_frame_post=child(service,0x65c);
        site(a,0xb12975,-1);
        assign_native_post_effect_frame_00b4e2b0(a.captured_frame_post,a.captured_frame,c.entry,
            c.passes.holders.levels.actual_increment_00ce221c);
        site(a,0xb1297a,-1);a.phase=State::Phase::awaiting_b1297a_continuation;
        previous.phase=Previous::Phase::awaiting_later_continuation;passthrough->phase=Passthrough::Phase::awaiting_later_continuation;
        distortion->phase=Distortion::Phase::awaiting_later_continuation;bloom->phase=Bloom::Phase::awaiting_later_continuation;
        dof->phase=Dof::Phase::awaiting_later_continuation;post->phase=Post::Phase::awaiting_later_continuation;
        prior->phase=Continuation::Phase::awaiting_later_continuation;entry->phase=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation;
    } catch(...) {
        a.phase=State::Phase::failed;previous.phase=Previous::Phase::failed;passthrough->phase=Passthrough::Phase::failed;
        distortion->phase=Distortion::Phase::failed;bloom->phase=Bloom::Phase::failed;dof->phase=Dof::Phase::failed;
        post->phase=Post::Phase::failed;prior->phase=Continuation::Phase::failed;
        entry->phase=NativeRenderResourceInitEntryState::Phase::failed;
        // Retain actual publications, raw names/mask and all acquired children.
        // Provider failure behavior remains authoritative; no caller repair.
        throw;
    }
}
} // namespace bsp
