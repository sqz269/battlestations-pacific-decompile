#include "bsp/native_render_resource_init_post.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource post configuration requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
using Context=NativeRenderResourceInitContinuationContext;
using Previous=NativeRenderResourceInitContinuationState;
using State=NativeRenderResourceInitPostState;
void* at(const void* p,U offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
U word(const void* p,U offset=0) noexcept {
    return *static_cast<const volatile U*>(at(p,offset));
}
void* child(const void* p,U offset) noexcept {return reinterpret_cast<void*>(word(p,offset));}
void site(State& a,U address,int state) noexcept {
    a.native_site=address;a.native_state=state;
    a.previous->native_site=address;a.previous->native_state=state;
    a.previous->entry->native_site=address;a.previous->entry->unwind_state=state;
}
struct ParameterStep {
    U name_index,source_offset,name,accessor,registration,getter,release;
    int state;
    bool float2,bloom_source,inherited_ebp,receiver_first;
};
constexpr ParameterStep parameters[]={
    {2,0x8c, 0xb115e7,0xb11609,0xb11610,0xb11635,0xb1163c,32,false,false,false,false},
    {3,0x94, 0xb1164d,0xb1166f,0xb11676,0xb1169b,0xb116a2,33,false,false,false,true},
    {4,0x428,0xb116b3,0xb116d7,0xb116de,0xb11703,0xb1170a,34,true, true, false,false},
    {5,0x98, 0xb1171b,0xb1173d,0xb11744,0xb11769,0xb11770,35,false,false,false,true},
    {6,0x9c, 0xb11781,0xb117a3,0xb117aa,0xb117cf,0xb117d6,36,false,false,false,true},
    {11,0,   0xb117e7,0xb11803,0xb1180a,0xb1182f,0xb11836,37,true, false,true, false},
    {7,4,    0xb11847,0xb11866,0xb1186d,0xb11892,0xb11899,38,true, false,false,true}};

void return_name(std::size_t index,const ParameterStep& step,Context& c,State& a) {
    // Native captures data before disarming; current length+1 is captured before
    // 419CC0. No header clear: callbacks can observe the actual stale header.
    void* const data=child(&a.names[index],4);
    a.native_state=-1;a.previous->native_state=-1;a.previous->entry->unwind_state=-1;
    if(!data)return;
    const U bytes=word(&a.names[index])+1u;
    a.captured_name_data=data;a.captured_name_bytes=bytes;
    a.name_return_started[index]=true;
    auto& strings=c.passes.post_effects.raw_strings;
    site(a,step.getter,-1);
    auto* const pool=native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
    site(a,step.release,-1);
    return_native_string_pool_00bd1510(pool,data,bytes,strings.actual_small_returns_disabled_01090aa4);
    a.name_returned[index]=true;
}
void parameter(std::size_t index,Context& c,State& a) {
    const auto& step=parameters[index];
    site(a,step.name,-1);
    construct_native_string_header_0041e870(&a.names[index],c.passes.post_effects.raw_strings,
        c.parameter_names[step.name_index]);
    a.name_constructed[index]=true;
    void* const service=a.previous->entry->service;
    // Match the caller's current +70 and source load ordering. Only bloom_source
    // dereferences a current child. EBP is carried from the previous stage.
    void* effect=nullptr;
    if(step.receiver_first)effect=child(service,0x70);
    const void* const source=step.inherited_ebp?reinterpret_cast<void*>(a.previous->ebp_bits):
        at(step.bloom_source?child(service,0x28):service,step.source_offset);
    if(!step.receiver_first)effect=child(service,0x70);
    a.parameter_sources[index]=source;
    site(a,step.accessor,step.state);
    auto* const material=static_cast<NativeMaterialStorage*>(native_post_effect_material_00b4cba0(effect));
    a.parameter_materials[index]=material;
    site(a,step.registration,step.state);
    if(step.float2)register_native_material_float2_00b18b00(*material,&a.names[index],source,c.passes.parameters);
    else register_native_material_float_00b18b20(*material,&a.names[index],source,c.passes.parameters);
    return_name(index,step,c,a);
}
} // namespace

void continue_native_render_resource_init_00b11599_fragment(Previous& previous,Context& c,State& a) {
    auto* const entry=previous.entry;
    if(a.phase!=State::Phase::fresh || previous.phase!=Previous::Phase::awaiting_b11599_continuation ||
       previous.native_site!=0x00b11599u || previous.native_state!=-1 || previous.temporary_mask_esp10 ||
       !previous.field70_published || previous.context_identity!=&c || previous.post_identity || !entry ||
       entry->phase!=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation ||
       entry->native_site!=0x00b11599u || entry->unwind_state!=-1 ||
       entry->continuation_identity!=&previous || !entry->argument_cells ||
       previous.ebp_bits!=reinterpret_cast<U>(at(entry->service,0xa0)))
        throw std::logic_error("post configuration requires an unconsumed B11599 state and its original context");
    a.previous=&previous;previous.post_identity=&a;
    a.phase=State::Phase::running;previous.phase=Previous::Phase::post_running;
    entry->phase=NativeRenderResourceInitEntryState::Phase::continuation_running;
    try {
        void* const service=entry->service;
        site(a,0xb1159c,-1);
        void* holder=native_shadow_texture_holder_00b4d170(child(service,0x64));
        site(a,0xb115a3,-1);void* texture=native_shadow_holder_texture_00b4cb10(holder);
        site(a,0xb115ae,-1);void* material=native_post_effect_material_00b4cba0(child(service,0x70));
        site(a,0xb115b5,-1);
        set_native_material_texture_unchecked_00b189f0(material,0,texture,c.passes.post_effects.destruction.actual_owners);
        site(a,0xb115bd,-1);holder=native_bloom_output_holder_00b54cd0(child(service,0x28));
        site(a,0xb115c4,-1);texture=native_shadow_holder_texture_00b4cb10(holder);
        site(a,0xb115cf,-1);material=native_post_effect_material_00b4cba0(child(service,0x70));
        site(a,0xb115d6,-1);
        set_native_material_texture_unchecked_00b189f0(material,1,texture,c.passes.post_effects.destruction.actual_owners);
        for(std::size_t i=0;i<7;++i)parameter(i,c,a);
        site(a,0xb118a1,-1);
        auto* const surface=native_render_holder_primary_00b4cb20(child(service,0x4c));
        site(a,0xb118aa,-1);
        set_native_post_effect_color0_00b4cb70(child(service,0x70),surface,c.entry.frame_targets);
        site(a,0xb118af,-1);a.phase=State::Phase::awaiting_b118af_continuation;
        previous.phase=Previous::Phase::awaiting_later_continuation;
        entry->phase=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation;
    } catch(...) {
        a.phase=State::Phase::failed;previous.phase=Previous::Phase::failed;
        entry->phase=NativeRenderResourceInitEntryState::Phase::failed;
        // Retain names, actual publications and every previous child block.
        // No guessed native FH3 cleanup, repeat, rollback or deletion admission.
        throw;
    }
}
} // namespace bsp
