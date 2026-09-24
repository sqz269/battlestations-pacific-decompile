#include "bsp/native_render_resource_init_continuation.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <atomic>
#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource continuation requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
using Context=NativeRenderResourceInitContinuationContext;
using State=NativeRenderResourceInitContinuationState;
void* at(const void* p,U offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
void put(void* p,U offset,U value) noexcept {*static_cast<volatile U*>(at(p,offset))=value;}
void* child(const void* p,U offset) noexcept {return reinterpret_cast<void*>(word(p,offset));}
U bits(const void* p) noexcept {return reinterpret_cast<U>(p);}
U quotient(U value,U divisor) noexcept {
    std::int32_t signed_value;
    std::memcpy(&signed_value,&value,sizeof(value));
    return static_cast<U>(signed_value/static_cast<std::int32_t>(divisor));
}
U x87_float_copy(const volatile U& source) noexcept {
    const volatile U* address=&source;U result;
    __asm {mov eax,address}
    __asm {fld dword ptr [eax]}
    __asm {fstp dword ptr [result]}
    return result;
}
void terminal_frame(void* old,NativeRenderResourceInitEntryContext& c) {
    const auto* table=c.actual_frame_profile_00d5e600;
    if(word(old)!=0x00d5e600u || !table || table[0]!=0x00bd30e0u ||
       word(old)!=0x00d5e600u || table[1]!=0x00b1fcf0u)
        throw std::logic_error("unsupported current post-effect frame terminal");
    delete_native_frame_target_owner_00b1fcf0(*static_cast<NativeFrameTargetOwnerStorage*>(old),1,c.frame_targets);
}
template<class A,class B> bool same_cell(A& a,B& b) noexcept {
    return reinterpret_cast<const volatile void*>(&a)==reinterpret_cast<const volatile void*>(&b);
}
void validate(Context& c) {
    auto& post=c.passes.post_effects;auto& holders=c.passes.holders;
    auto& surfaces=c.entry.frame_targets.actual_surface_context;
    if(&c.downscale.common!=&c.passes || &c.luminance.post_effects!=&post ||
       &c.bright.post_effects!=&post || &c.bloom.post_effects!=&post ||
       &c.luminance.holders!=&holders || &c.bright.holders!=&holders ||
       &c.bloom.textures.holders!=&holders || &c.effects_lifetime.texture_holders!=&holders ||
       &c.effects_lifetime.actual_post_effects!=&post.destruction.actual_owners ||
       &c.luminance.parameters!=&c.passes.parameters || &c.bright.parameters!=&c.passes.parameters ||
       &c.bloom.parameters!=&c.passes.parameters || &post.destruction.frame_targets!=&c.entry.frame_targets ||
       &holders.levels.surfaces!=&surfaces || &holders.render_targets.surfaces!=&surfaces ||
       &holders.textures.construction.owners.surfaces!=&surfaces ||
       !same_cell(post.actual_renderer_00f8d394,surfaces.actual_renderer_00f8d394) ||
       !same_cell(c.entry.decrement_iat_00ce2220,holders.actual_decrement_00ce2220) ||
       !same_cell(c.entry.decrement_iat_00ce2220,post.destruction.actual_decrement_00ce2220) ||
       !same_cell(c.entry.decrement_iat_00ce2220,c.effects_lifetime.actual_decrement_00ce2220))
        throw std::logic_error("render-resource continuation requires one canonical provider domain");
}
void site(State& a,U address,int state) noexcept {
    a.native_site=address;a.native_state=state;
    a.entry->native_site=address;a.entry->unwind_state=state;
}
void* renderer(Context& c) noexcept {
    return c.entry.frame_targets.actual_surface_context.actual_renderer_00f8d394;
}
void* renderer_slot(Context& c,U offset,U expected) {
    void* const current=renderer(c);const auto* table=c.entry.actual_renderer_profile_00d5f0a8;
    if(!current || word(current)!=0x00d5f0a8u || !table || table[offset/4]!=expected)
        throw std::logic_error("unsupported current continuation renderer dispatch");
    return current;
}
void prepare(Context& c,State& a) {
    for(auto& block:a.passes)block.prepare(c.passes);
    a.luminance.prepare(c.luminance);a.bright.prepare(c.bright);a.bloom.prepare(c.bloom);
    for(auto& block:a.posts)block.prepare(c.passes.post_effects);
}
void* allocate(U size,State& a,U address) {
    site(a,address,a.native_state);
    return singleton_lifetime_allocate({SingletonAllocationKind::object,size,size});
}
void* holder(std::size_t index,U allocation_site,U constructor_site,int state,
    U format,U multisample,U mode,Context& c,State& a) {
    void* const raw=allocate(0x18,a,allocation_site);
    a.raw_holders[index]=raw;a.spill_esp20=bits(raw);site(a,constructor_site,state);
    if(!raw)return nullptr;
    auto& args=a.holder_arguments[index];
    // B10AF9/B10B3E reload +50 only AFTER the respective allocator returns.
    auto* const external=index>=3?static_cast<NativeSurfaceOwnerStorage*>(child(a.entry->service,0x50)):nullptr;
    args={a.entry->dimensions_esp18[0],a.entry->dimensions_esp18[1],format,multisample,mode,external};
    void* result=construct_native_render_texture_surface_owner_00b4e020(raw,args,c.passes.holders,a.holders[index]);
    a.returned_holders[index]=result;return result;
}
void assign_shared(void* service,U offset,void* old,void* incoming,bool surface,
    NativeTextureSurfaceReferenceIncrement increment,Context& c,State& a,U retain_site,U release_site,U terminal_site) {
    if(old==incoming)return;
    put(service,offset,bits(incoming));
    if(incoming) {
        site(a,retain_site,-1);
        if(!increment)throw std::logic_error("continuation captured increment import is unbound");
        increment(static_cast<volatile long*>(at(incoming,4)));
    }
    if(!old)return;
    site(a,release_site,-1);
    const auto decrement=c.entry.decrement_iat_00ce2220;
    if(!decrement)throw std::logic_error("continuation current decrement import is unbound");
    if(decrement(static_cast<volatile long*>(at(old,4)))!=0)return;
    site(a,terminal_site,-1);
    const auto* table=surface?c.entry.frame_targets.actual_surface_profile_00d619a0:
        c.effects_lifetime.actual_holder_profile_00d61eb8;
    const U profile=surface?0x00d619a0u:0x00d61eb8u;
    const U deleting=surface?0x00b3f5b0u:0x00b4e410u;
    if(word(old)!=profile || !table || table[0]!=0x00bd30e0u || word(old)!=profile || table[1]!=deleting)
        throw std::logic_error("unsupported current continuation shared-owner terminal");
    if(surface)delete_native_surface_00b3f5b0(*static_cast<NativeSurfaceOwnerStorage*>(old),1,c.entry.frame_targets.actual_surface_context);
    else delete_native_render_texture_surface_owner_00b4e410(old,1,c.passes.holders);
}
void* inline_pass(U size,U profile,std::size_t index,U allocation_site,State& a) {
    void* const raw=allocate(size,a,allocation_site);a.raw_passes[index]=raw;
    if(raw) {
        put(raw,0,0x00ceb130u);
        ::new(at(raw,4)) std::atomic<std::int32_t>(1); // SAME native +04 cell.
        put(raw,8,0);put(raw,0xc,0);put(raw,0,profile);
    }
    return raw;
}
void name(std::size_t index,const char* text,U address,Context& c,State& a) {
    site(a,address,a.native_state);
    construct_native_string_header_0041e870(&a.names[index],c.passes.post_effects.raw_strings,text);
    a.name_constructed[index]=true;
}
void return_name(std::size_t index,U mask,U getter_site,U return_site,Context& c,State& a) {
    // Native captures the current data pointer, then clears mask/disarms before
    // calling the current singleton. The stale header must not be cleared.
    void* const data=child(&a.names[index],4);
    if(mask)a.temporary_mask_esp10&=~mask;
    a.native_state=-1;a.entry->unwind_state=-1;
    if(!data)return;
    const U size=word(&a.names[index])+1u;
    a.captured_name_data=data;a.captured_name_bytes=size;
    a.name_return_started[index]=true;
    auto& strings=c.passes.post_effects.raw_strings;
    site(a,getter_site,-1);
    auto* const pool=native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
    site(a,return_site,-1);
    return_native_string_pool_00bd1510(pool,data,size,strings.actual_small_returns_disabled_01090aa4);
}
struct PostStep {U field,allocation,name,construction,getter,release;int state;U mask;};
constexpr PostStep post_steps[]={
    {0x68,0xb10c8f,0xb10cb5,0xb10cd4,0xb10d06,0xb10d0d,6,1},
    {0x6c,0xb10d77,0xb10d9d,0xb10dba,0xb10dec,0xb10df3,10,2},
    {0x80,0xb11023,0xb1104c,0xb1106c,0xb110a7,0xb110ae,16,4},
    {0x70,0xb1150c,0xb11535,0xb11555,0xb1158d,0xb11594,29,8}};
void post(std::size_t index,Context& c,State& a) {
    const auto& s=post_steps[index];void* const raw=allocate(0x20,a,s.allocation);
    a.raw_posts[index]=raw;a.edi_bits=bits(raw);
    if(index<2)a.spill_esp20=bits(raw);else a.spill_esp14=bits(raw);
    a.native_state=s.state;a.entry->unwind_state=s.state;
    void* returned=nullptr;
    if(raw) {
        name(index,c.effect_names[index],s.name,c,a);
        if(index==0)a.temporary_mask_esp10=1;else a.temporary_mask_esp10|=s.mask;
        site(a,s.construction,s.state+1);
        returned=construct_native_post_effect20_00b4e470(raw,0x20,a.names[index],3,nullptr,a.posts[index]);
        a.returned_posts[index]=returned;
    }
    put(a.entry->service,s.field,bits(returned));
    if(index==3)a.field70_published=true;
    a.native_state=-1;a.entry->unwind_state=-1;
    if(a.temporary_mask_esp10&s.mask)return_name(index,s.mask,s.getter,s.release,c,a);
}
struct ParameterStep {U field,source_field,name,accessor,registration,getter,release;int state;bool vector,bloom_source;};
constexpr ParameterStep parameter_steps[]={
    {0x68,4,0xb10d1e,0xb10d3d,0xb10d44,0xb10d69,0xb10d70,9,true,false},
    {0x6c,4,0xb10e04,0xb10e20,0xb10e27,0xb10e4e,0xb10e55,13,true,false},
    {0x80,0x8c,0xb110fd,0xb1111f,0xb11126,0xb11145,0xb1114c,19,false,false},
    {0x80,0x94,0xb1115d,0xb11182,0xb11189,0xb111ae,0xb111b5,20,false,false},
    {0x80,0x428,0xb111c3,0xb111e8,0xb111ef,0xb1120e,0xb11215,21,true,true},
    {0x80,0x98,0xb11226,0xb1124b,0xb11252,0xb11277,0xb1127e,22,false,false},
    {0x80,0x9c,0xb1128f,0xb112b4,0xb112bb,0xb112e0,0xb112e7,23,false,false},
    {0x80,4,0xb112f8,0xb1131a,0xb11321,0xb11346,0xb1134d,24,true,false},
    {0x80,0x250,0xb1135e,0xb11383,0xb1138a,0xb113af,0xb113b6,25,false,false},
    {0x80,0x254,0xb113c7,0xb113ec,0xb113f3,0xb11418,0xb1141f,26,false,false},
    {0x80,0x18c,0xb11430,0xb11455,0xb1145c,0xb11481,0xb11488,27,true,false},
    {0x80,0xa0,0xb11499,0xb114be,0xb114c5,0xb114ea,0xb114f1,28,true,false}};
void parameter(std::size_t index,Context& c,State& a) {
    const auto& s=parameter_steps[index];const auto n=index+4;
    name(n,c.parameter_names[index],s.name,c,a);
    // All providers retain this actual source address; no copied parameter data.
    const void* const source=at(s.bloom_source?child(a.entry->service,0x28):a.entry->service,s.source_field);
    if(index==0 || index==11)a.ebp_bits=bits(source);
    void* const effect=child(a.entry->service,s.field);
    site(a,s.accessor,s.state);
    auto* material=static_cast<NativeMaterialStorage*>(native_post_effect_material_00b4cba0(effect));
    site(a,s.registration,s.state);
    if(s.vector)register_native_material_float2_00b18b00(*material,&a.names[n],source,c.passes.parameters);
    else register_native_material_float_00b18b20(*material,&a.names[n],source,c.passes.parameters);
    return_name(n,0,s.getter,s.release,c,a);
}
} // namespace

void assign_native_post_effect_frame_00b4e2b0(void* effect,
    NativeFrameTargetOwnerStorage* incoming,NativeRenderResourceInitEntryContext& c,
    NativeTextureSurfaceReferenceIncrement const volatile& increment_cell) {
    void* const old=child(effect,8);
    if(old==incoming)return;
    put(effect,8,bits(incoming));
    if(incoming) {
        const auto increment=increment_cell;
        if(!increment)throw std::logic_error("post-effect current increment import is unbound");
        increment(static_cast<volatile long*>(at(incoming,4)));
    }
    if(old) {
        const auto decrement=c.decrement_iat_00ce2220;
        if(!decrement)throw std::logic_error("post-effect current decrement import is unbound");
        if(decrement(static_cast<volatile long*>(at(old,4)))==0)terminal_frame(old,c);
    }
}
void* native_bloom_output_holder_00b54cd0(const void* bloom) noexcept {return child(bloom,0x20);}

void continue_native_render_resource_init_00b109bc_fragment(NativeRenderResourceInitEntryState& entry,
    Context& c,State& a) {
    if(a.phase!=State::Phase::fresh || entry.phase!=NativeRenderResourceInitEntryState::Phase::awaiting_b109bc_continuation ||
       entry.native_site!=0x00b109bcu || entry.continuation_identity || !entry.argument_cells)
        throw std::logic_error("render-resource continuation requires an unconsumed B109BC entry");
    a.entry=&entry;entry.continuation_identity=&a;a.phase=State::Phase::preparing;
    try {
        validate(c);prepare(c,a);
        a.phase=State::Phase::running;entry.phase=NativeRenderResourceInitEntryState::Phase::continuation_running;
        void* const service=entry.service;
        a.aligned_width_esp58=entry.aligned_width_eax; // B109BE, after native PUSH18.
        void* result=holder(0,0xb109c2,0xb109ee,1,0x15,0,0,c,a);
        a.native_state=-1;put(service,0x4c,bits(result));
        result=holder(1,0xb10a03,0xb10a2f,2,0x15,0,0,c,a);
        a.native_state=-1;put(service,0x38,bits(result));
        result=holder(2,0xb10a44,0xb10a70,3,0x15,0,0,c,a);
        a.captured_increment_ebp=c.passes.holders.levels.actual_increment_00ce221c; // B10A79 before +3C.
        put(service,0x3c,bits(result));a.native_state=-1;
        // B10A82 captures old+40 before the incoming+4C load.
        void* const old40=child(service,0x40);
        void* const incoming40=child(service,0x4c);
        assign_shared(service,0x40,old40,incoming40,false,a.captured_increment_ebp,c,a,0xb10a9e,0xb10aa8,0xb10ab8);
        const U multisample=entry.argument_cells->word_00; // B10ABA, retained EDI for both holders.
        site(a,0xb10ad4,-1);
        auto* surface=create_native_renderer_render_target_00b2a7c0(renderer(c),entry.dimensions_esp18[0],
            entry.dimensions_esp18[1],0x71,multisample,c.passes.holders.render_targets,a.target);
        put(service,0x50,bits(surface));
        result=holder(3,0xb10ade,0xb10b0e,4,0x71,multisample,1,c,a);
        a.native_state=-1;put(service,0x44,bits(result));
        result=holder(4,0xb10b23,0xb10b53,5,0x71,multisample,1,c,a);
        a.native_state=-1;put(service,0x48,bits(result));
        void* current=renderer_slot(c,0x88,0x00b2a070);
        a.half_texture_arguments={quotient(entry.dimensions_esp18[0],2),quotient(entry.dimensions_esp18[1],2),1,0x71,0x10};
        site(a,0xb10b8e,-1);
        void* texture=create_native_runtime_texture_2d_00b2a070(current,a.half_texture_arguments,c.passes.holders.textures,a.half_texture);
        put(service,0x54,bits(texture));
        if(word(texture)!=0x00d61948u || !c.actual_texture_profile_00d61948 || c.actual_texture_profile_00d61948[0x30/4]!=0x00b3fd80u)
            throw std::logic_error("unsupported current half-texture surface getter");
        site(a,0xb10b9e,-1);
        surface=get_native_texture_surface_00b3fd80(texture,0,0,c.passes.holders.levels,a.half_surface);
        put(service,0x5c,bits(surface));
        void* const incoming58=child(service,0x50);void* const old58=child(service,0x58);
        assign_shared(service,0x58,old58,incoming58,true,a.captured_increment_ebp,c,a,0xb10bb8,0xb10bc2,0xb10bd2);
        result=inline_pass(0x20,0x00d5e164,0,0xb10bd6,a);a.ebp_bits=0;
        const NativeDepthDownscalePassArguments depth_args{child(service,0x3c),quotient(entry.dimensions_esp18[0],2),quotient(entry.dimensions_esp18[1],2),0x73};
        put(service,0x60,bits(result));site(a,0xb10c20,-1);
        initialize_native_depth_downscale_pass_00b540b0(result,0x20,depth_args,a.passes[0]);
        result=inline_pass(0x20,0x00d5e178,1,0xb10c27,a);
        const U current_multisample=entry.argument_cells->word_00; // B10C50, after allocation.
        put(service,0x64,bits(result));
        NativeParticleBlendPassArguments particle{};
        particle.width=entry.dimensions_esp18[0];particle.height=entry.dimensions_esp18[1];
        particle.format=0x71;particle.multisample=current_multisample;particle.mode=1;
        particle.external_surface=static_cast<NativeSurfaceOwnerStorage*>(child(service,0x50));
        site(a,0xb10c70,-1);void* input=native_shadow_texture_holder_00b4d170(child(service,0x60));
        site(a,0xb10c77,-1);particle.texture1=native_shadow_holder_texture_00b4cb10(input);
        particle.texture0=child(service,0x54);particle.unused_first_word=word(service,0x44);
        site(a,0xb10c88,-1);
        initialize_native_particle_blend_pass_00b542d0(child(service,0x64),0x20,particle,a.passes[1]);
        post(0,c,a);parameter(0,c,a);post(1,c,a);parameter(1,c,a);
        result=inline_pass(0x220,0x00d5e18c,2,0xb10e5f,a);
        input=child(service,0x64);put(service,0x18,bits(result));
        a.quarter_height=quotient(entry.aligned_height_esp24,4);a.quarter_width=quotient(a.aligned_width_esp58,4);
        a.edi_bits=a.quarter_height;a.ebp_bits=a.quarter_width;
        site(a,0xb10eb0,-1);input=native_shadow_texture_holder_00b4d170(input);
        site(a,0xb10eb9,-1);
        initialize_native_downscale4x4_pass_00b544f0(child(service,0x18),0x220,
            {input,a.quarter_width,a.quarter_height,0x71},c.downscale,a.passes[2]);
        result=inline_pass(0x90,0x00d5e1a0,3,0xb10ec3,a);put(service,0x1c,bits(result));
        a.half_aligned_height_esp1d8=quotient(entry.aligned_height_esp24,2);
        a.spill_esp20=quotient(a.aligned_width_esp58,2);
        site(a,0xb10f17,-1);input=native_shadow_texture_holder_00b4d170(child(service,0x64));
        site(a,0xb10f20,-1);
        initialize_native_downscale2x2_pass_00b546f0(child(service,0x1c),0x90,
            {input,a.spill_esp20,a.half_aligned_height_esp1d8,0x71},c.downscale,a.passes[3]);
        result=allocate(0x250,a,0xb10f2a);a.raw_passes[4]=result;a.spill_esp14=bits(result);
        site(a,0xb10f47,14);if(result)result=construct_native_luminance_owner_00b50d40(result,c.actual_00ce6650);
        input=child(service,0x18);a.native_state=-1;put(service,0x20,bits(result));
        site(a,0xb10f5d,-1);input=native_shadow_texture_holder_00b4d170(input);
        site(a,0xb10f66,-1);initialize_native_luminance_00b51090(child(service,0x20),0x250,input,a.luminance);
        result=inline_pass(0x224,0x00d5e1b4,5,0xb10f70,a);
        input=child(service,0x18);put(service,0x24,bits(result));
        site(a,0xb10fa5,-1);input=native_shadow_texture_holder_00b4d170(input);
        site(a,0xb10fae,-1);initialize_native_bright_pass_00b54940(child(service,0x24),0x224,
            {input,a.quarter_width,a.quarter_height,0x15},a.bright);
        result=allocate(0x43c,a,0xb10fb8);a.raw_passes[6]=result;a.spill_esp14=bits(result);
        site(a,0xb10fd5,15);if(result)result=construct_native_bloom_owner_00b54e70(result);
        const U bloom_parameter=x87_float_copy(c.actual_00ce3854); // B10FDE before +28 publication.
        put(service,0x28,bits(result));
        const U bloom_height=quotient(entry.aligned_height_esp24,8);
        input=child(service,0x24);const U bloom_width=quotient(a.aligned_width_esp58,8);
        site(a,0xb11013,-1);input=native_shadow_texture_holder_00b4d170(input);
        site(a,0xb1101c,-1);initialize_native_bloom_00b54f90(child(service,0x28),0x43c,
            {input,bloom_width,bloom_height,0x15,bloom_parameter},a.bloom);
        post(2,c,a);
        site(a,0xb110b6,-1);texture=native_shadow_holder_texture_00b4cb10(child(service,0x44));
        site(a,0xb110c4,-1);void* material=native_post_effect_material_00b4cba0(child(service,0x80));
        site(a,0xb110cb,-1);set_native_material_texture_unchecked_00b189f0(material,0,texture,c.passes.post_effects.destruction.actual_owners);
        site(a,0xb110d3,-1);input=native_bloom_output_holder_00b54cd0(child(service,0x28));
        site(a,0xb110da,-1);texture=native_shadow_holder_texture_00b4cb10(input);
        site(a,0xb110e8,-1);material=native_post_effect_material_00b4cba0(child(service,0x80));
        site(a,0xb110ef,-1);set_native_material_texture_unchecked_00b189f0(material,1,texture,c.passes.post_effects.destruction.actual_owners);
        for(std::size_t index=2;index<12;++index)parameter(index,c,a);
        site(a,0xb114f9,-1);surface=native_render_holder_primary_00b4cb20(child(service,0x4c));
        site(a,0xb11505,-1);set_native_post_effect_color0_00b4cb70(child(service,0x80),surface,c.entry.frame_targets);
        post(3,c,a);
        site(a,0xb11599,-1);a.phase=State::Phase::awaiting_b11599_continuation;
        entry.phase=NativeRenderResourceInitEntryState::Phase::awaiting_later_continuation;
    } catch(...) {
        a.phase=State::Phase::failed;entry.phase=NativeRenderResourceInitEntryState::Phase::failed;
        // Keep actual publications, names and all failed child/companion frames.
        // No guessed native FH3 unwind, rollback or destruction admission.
        throw;
    }
}
} // namespace bsp
