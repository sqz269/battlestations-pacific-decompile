#include "bsp/native_render_resource_init_dof.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource DOF initialization requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
using Context=NativeRenderResourceInitContinuationContext;
using Continuation=NativeRenderResourceInitContinuationState;
using Previous=NativeRenderResourceInitPostState;
using State=NativeRenderResourceInitDofState;
using Names=NativeRenderResourceInitDofNames;
void* at(const void* p,U offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(at(p,offset))=value;}
void* child(const void* p,U offset) noexcept {return reinterpret_cast<void*>(word(p,offset));}
U bits(const void* p) noexcept {return reinterpret_cast<U>(p);}
void set_state(State& a,int state) noexcept {
    a.native_state=state;a.previous->native_state=state;
    a.previous->previous->native_state=state;a.previous->previous->entry->unwind_state=state;
}
void site(State& a,U address,int state) noexcept {
    a.native_site=address;a.previous->native_site=address;
    a.previous->previous->native_site=address;a.previous->previous->entry->native_site=address;
    set_state(a,state);
}
void set_mask(State& a,U value) noexcept {
    a.temporary_mask_esp10=value;a.previous->previous->temporary_mask_esp10=value;
}
void construct_name(std::size_t index,const char* literal,U address,Context& c,State& a) {
    site(a,address,a.native_state);
    construct_native_string_header_0041e870(&a.names[index],c.passes.post_effects.raw_strings,literal);
    a.name_constructed[index]=true;
}
void return_name(std::size_t index,U mask,U getter,U release,Context& c,State& a) {
    // Capture current data first. Effect cleanup then clears its mask bit;
    // every name disarms before current length+1/current singleton are read.
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
    a.name_returned[index]=true; // Header intentionally remains stale.
}
struct ParameterStep {
    U name_index,source_offset,name,accessor,registration,getter,release;
    int state;
    U words;
    bool bloom_source,inherited_ebp,receiver_first;
};
constexpr ParameterStep parameters[]={
    {2,0x8c, 0xb1198c,0xb119ae,0xb119b5,0xb119da,0xb119e1,42,1,false,false,false},
    {3,0x94, 0xb119f2,0xb11a14,0xb11a1b,0xb11a40,0xb11a47,43,1,false,false,true},
    {4,0x428,0xb11a58,0xb11a7c,0xb11a83,0xb11aa8,0xb11aaf,44,2,true, false,false},
    {5,0x98, 0xb11ac0,0xb11ae2,0xb11ae9,0xb11b0e,0xb11b15,45,1,false,false,true},
    {6,0x9c, 0xb11b26,0xb11b48,0xb11b4f,0xb11b74,0xb11b7b,46,1,false,false,true},
    {7,4,    0xb11b8c,0xb11bab,0xb11bb2,0xb11bd7,0xb11bde,47,2,false,false,false},
    {11,0,   0xb11bef,0xb11c0b,0xb11c12,0xb11c37,0xb11c3e,48,2,false,true, true},
    {10,0x18c,0xb11c4f,0xb11c71,0xb11c78,0xb11c9d,0xb11ca4,49,2,false,false,true},
    {12,0x26c,0xb11cb2,0xb11cd3,0xb11cda,0xb11cf9,0xb11d00,50,4,false,false,false},
    {13,0x27c,0xb11d11,0xb11d35,0xb11d3c,0xb11d61,0xb11d68,51,4,false,false,false}};
void parameter(std::size_t index,Context& c,const Names& names,State& a) {
    const auto& step=parameters[index];const auto n=index+1;
    const char* const literal=step.name_index==12?names.focal_plane_00d5e338:
        step.name_index==13?names.focal_plane2_00d5e324:c.parameter_names[step.name_index];
    construct_name(n,literal,step.name,c,a);
    auto& continuation=*a.previous->previous;
    void* const service=continuation.entry->service;
    void* effect=nullptr;
    if(step.receiver_first)effect=child(service,0x74);
    const void* const source=step.inherited_ebp?reinterpret_cast<void*>(continuation.ebp_bits):
        at(step.bloom_source?child(service,0x28):service,step.source_offset);
    if(!step.receiver_first)effect=child(service,0x74);
    a.parameter_sources[index]=source;
    site(a,step.accessor,step.state);
    auto* const material=static_cast<NativeMaterialStorage*>(native_post_effect_material_00b4cba0(effect));
    a.parameter_materials[index]=material;
    site(a,step.registration,step.state);
    if(step.words==1)register_native_material_float_00b18b20(*material,&a.names[n],source,c.passes.parameters);
    else if(step.words==2)register_native_material_float2_00b18b00(*material,&a.names[n],source,c.passes.parameters);
    else {
        // Native B18AC0: wrapping DWORD 4*vector_count, matrix0, RET0C.
        // Both B107F0 sites push vector_count1. Reuse the established exact
        // wrapper expansion from native_render_pass_initializers.cpp.
        register_native_material_parameter_00b17e10(*material,&a.names[n],source,4u*1u,0,c.passes.parameters);
    }
    return_name(n,0,step.getter,step.release,c,a);
}
} // namespace

void continue_native_render_resource_init_00b118af_fragment(Previous& previous,Context& c,
    const Names& names,State& a) {
    auto* const continuation=previous.previous;
    auto* const entry=continuation?continuation->entry:nullptr;
    if(a.phase!=State::Phase::fresh || previous.phase!=Previous::Phase::awaiting_b118af_continuation ||
       previous.native_site!=0x00b118afu || previous.native_state!=-1 || previous.dof_identity ||
       !continuation || continuation->phase!=Continuation::Phase::awaiting_later_continuation ||
       continuation->post_identity!=&previous || continuation->context_identity!=&c ||
       continuation->native_site!=0x00b118afu || continuation->native_state!=-1 || continuation->temporary_mask_esp10 ||
       !entry || entry->phase!=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation ||
       entry->native_site!=0x00b118afu || entry->unwind_state!=-1 || entry->continuation_identity!=continuation ||
       !entry->argument_cells || continuation->ebp_bits!=bits(at(entry->service,0xa0)) ||
       !names.effect_00d5e348 || !names.focal_plane_00d5e338 || !names.focal_plane2_00d5e324)
        throw std::logic_error("DOF initialization requires an unconsumed B118AF state, original context and actual literals");
    a.previous=&previous;previous.dof_identity=&a;a.phase=State::Phase::preparing;
    previous.phase=Previous::Phase::dof_running;continuation->phase=Continuation::Phase::post_running;
    entry->phase=NativeRenderResourceInitEntryState::Phase::continuation_running;
    site(a,0xb118af,-1); // Host preparation precedes the first native instruction.
    try {
        a.post.prepare(c.passes.post_effects);
        a.phase=State::Phase::running;
        void* const service=entry->service;
        site(a,0xb118b1,-1);
        a.raw_post=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
        continuation->edi_bits=bits(a.raw_post);continuation->spill_esp14=bits(a.raw_post);
        set_state(a,39);
        if(a.raw_post) {
            construct_name(0,names.effect_00d5e348,0xb118da,c,a);
            set_mask(a,a.temporary_mask_esp10|0x10u);
            site(a,0xb118fa,40);
            a.returned_post=construct_native_post_effect20_00b4e470(a.raw_post,0x20,a.names[0],3,nullptr,a.post);
        }
        const bool cleanup=(a.temporary_mask_esp10&0x10u)!=0; // B11903 before publication.
        put(service,0x74,bits(a.returned_post));a.field74_published=true;
        set_state(a,-1);
        if(cleanup)return_name(0,0x10,0xb11932,0xb11939,c,a);
        site(a,0xb11941,-1);void* holder=native_shadow_texture_holder_00b4d170(child(service,0x64));
        site(a,0xb11948,-1);void* texture=native_shadow_holder_texture_00b4cb10(holder);
        site(a,0xb11953,-1);void* material=native_post_effect_material_00b4cba0(child(service,0x74));
        site(a,0xb1195a,-1);
        set_native_material_texture_unchecked_00b189f0(material,0,texture,c.passes.post_effects.destruction.actual_owners);
        site(a,0xb11962,-1);holder=native_bloom_output_holder_00b54cd0(child(service,0x28));
        site(a,0xb11969,-1);texture=native_shadow_holder_texture_00b4cb10(holder);
        site(a,0xb11974,-1);material=native_post_effect_material_00b4cba0(child(service,0x74));
        site(a,0xb1197b,-1);
        set_native_material_texture_unchecked_00b189f0(material,1,texture,c.passes.post_effects.destruction.actual_owners);
        for(std::size_t i=0;i<10;++i)parameter(i,c,names,a);
        site(a,0xb11d70,-1);
        auto* const surface=native_render_holder_primary_00b4cb20(child(service,0x4c));
        site(a,0xb11d79,-1);
        set_native_post_effect_color0_00b4cb70(child(service,0x74),surface,c.entry.frame_targets);
        site(a,0xb11d7e,-1);a.phase=State::Phase::awaiting_b11d7e_continuation;
        previous.phase=Previous::Phase::awaiting_later_continuation;
        continuation->phase=Continuation::Phase::awaiting_later_continuation;
        entry->phase=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation;
    } catch(...) {
        a.phase=State::Phase::failed;previous.phase=Previous::Phase::failed;
        continuation->phase=Continuation::Phase::failed;entry->phase=NativeRenderResourceInitEntryState::Phase::failed;
        // Keep provider preparation/acquisition records, all previous blocks,
        // name headers and actual publications. Child providers retain their
        // own existing failure semantics; this caller adds no cleanup/retry.
        throw;
    }
}
} // namespace bsp
