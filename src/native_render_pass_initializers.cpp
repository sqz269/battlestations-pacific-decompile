#include "bsp/native_render_pass_initializers.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render pass initialization requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
Word word(const void* p,Word byte_offset=0) noexcept {
    Word result;
    __asm { mov eax,p }
    __asm { add eax,byte_offset }
    __asm { mov eax,[eax] }
    __asm { mov result,eax }
    return result;
}
void put(void* p,Word offset,Word value) noexcept {
    *reinterpret_cast<volatile Word*>(reinterpret_cast<Word>(p)+offset)=value;
}
void* pointer(Word value) noexcept {return reinterpret_cast<void*>(value);}
void require(bool value,const char* reason) {if(!value)throw std::logic_error(reason);}
NativeMaterialStorage& current_material(void* owner) {
    void* const post=pointer(word(owner,8));
    require(post!=nullptr,"render pass requires its current actual post-effect");
    void* const material=native_post_effect_material_00b4cba0(post);
    require(material!=nullptr,"render pass requires its current actual material");
    return *static_cast<NativeMaterialStorage*>(material);
}
} // namespace

namespace detail {
// Shared full arithmetic/store fragment from B5414B..B541C2 and
// B54397..B5440B. Keep the original x87 stack, single-precision spills/reloads,
// unsigned conversion, zero stores and caller's precision/rounding controls.
// This is an implementation helper, not an additional recovered native entry.
void store_native_render_pass_sample_offsets(void* owner,Word width,Word height,
    bool twice,const volatile double* half,const volatile double* bias,void* name) noexcept {
    Word denominator;
    float x,y;
    __asm {
        mov eax,width
        cmp twice,0
        je width_ready
        add eax,eax
    width_ready:
        test eax,eax
        mov denominator,eax
        fild dword ptr denominator
        jge width_positive
        mov edx,bias
        fadd qword ptr [edx]
    width_positive:
        mov edx,half
        fld qword ptr [edx]
        fld st(0)
        mov eax,height
        cmp twice,0
        je height_ready
        add eax,eax
    height_ready:
        test eax,eax
        fdivrp st(2),st(0)
        mov denominator,eax
        fxch st(1)
        fstp dword ptr x
        fild dword ptr denominator
        jge height_positive
        mov edx,bias
        fadd qword ptr [edx]
    height_positive:
        fdivp st(1),st(0)
        xorps xmm0,xmm0
        mov edx,owner
        movss dword ptr [edx+18h],xmm0
        movss dword ptr [edx+1ch],xmm0
        mov ecx,name
        mov dword ptr [ecx],0
        mov dword ptr [ecx+4],0
        fstp dword ptr y
        fld dword ptr x
        fstp dword ptr [edx+10h]
        fld dword ptr y
        fstp dword ptr [edx+14h]
    }
}
} // namespace detail

__declspec(naked) NativeSurfaceOwnerStorage* __fastcall native_render_holder_primary_00b4cb20(const void*) noexcept {
    __asm { mov eax,[ecx+0ch] }
    __asm { ret }
}
void set_native_post_effect_color0_00b4cb70(void* post,NativeSurfaceOwnerStorage* surface,
    NativeFrameTargetOwnerContext& context) {
    auto* const frame=static_cast<NativeFrameTargetOwnerStorage*>(pointer(word(post,8)));
    set_native_frame_target_color_00b1fab0(*frame,0,surface,context);
}

NativeRenderPassInitializationBlock::~NativeRenderPassInitializationBlock() noexcept {
    if(phase_!=Phase::idle)std::terminate();
}
void NativeRenderPassInitializationBlock::prepare(NativeRenderPassInitializationContext& c) {
    require(phase_==Phase::idle && c.depth_name_00d620c0 && c.particle_name_00d5e41c &&
        c.sample_offsets_name_00d5e40c,"render pass requires idle host storage and actual names");
    context_=&c;phase_=Phase::preparing;
    try {
        post_effect_.prepare(c.post_effects);
        acquired_.effect_name_header=&effect_name_;
        acquired_.parameter_name_header=&parameter_name_;
        phase_=Phase::prepared;
    } catch(...) {context_=nullptr;phase_=Phase::idle;throw;}
}
void NativeRenderPassInitializationBlock::cancel_preparation() noexcept {
    if(phase_!=Phase::prepared)std::terminate();
    post_effect_.cancel_preparation();acquired_={};context_=nullptr;phase_=Phase::idle;
}
void NativeRenderPassInitializationBlock::set_state(int value) noexcept {acquired_.native_state=value;}
void NativeRenderPassInitializationBlock::settle() noexcept {
    if(post_effect_.phase()==NativePostEffect20ConstructionBlock::Phase::prepared)
        post_effect_.cancel_preparation();
    phase_=Phase::settled;
}
void NativeRenderPassInitializationBlock::reset_after_host_quiescence() noexcept {
    if(phase_!=Phase::settled || post_effect_.phase()!=NativePostEffect20ConstructionBlock::Phase::idle)
        std::terminate();
    effect_name_.~NativeString();new(&effect_name_) NativeString;
    parameter_name_.~NativeString();new(&parameter_name_) NativeString;
    acquired_={};context_=nullptr;phase_=Phase::idle;
}
void NativeRenderPassInitializationBlock::return_effect_name() {
    acquired_.effect_name_return_started=true;
    destroy_native_string_header_0041dd20(&effect_name_,context_->post_effects.raw_strings);
}
void NativeRenderPassInitializationBlock::return_parameter_name() {
    acquired_.parameter_name_return_started=true;
    destroy_native_string_header_0041dd20(&parameter_name_,context_->post_effects.raw_strings);
}
void NativeRenderPassInitializationBlock::unwind(bool& name_owned) {
    // DF8C18 / DF8C64: 0->-1/free post;1->0/masked name;2->-1/same
    // name;3->-1/parameter name;4->-1/free holder. No published-child rollback.
    // State2 is in the native map but not visited on these normal bodies.
    while(acquired_.native_state>=0) {
        const int action=acquired_.native_state;set_state(action==1?0:-1);
        try {
            switch(action) {
            case 0:
                if(post_effect_.acquired().native_completed)
                    acquired_.completed_post_preserved_after_host_failure=true;
                else {
                    acquired_.post_raw_free_started=true;
                    singleton_lifetime_free(acquired_.raw_post_effect);
                }
                break;
            case 1:case 2:
                if(name_owned){name_owned=false;return_effect_name();}
                break;
            case 3:return_parameter_name();break;
            case 4:
                acquired_.holder_raw_free_started=true;
                singleton_lifetime_free(acquired_.raw_holder);break;
            default:std::terminate();
            }
        } catch(...) {unwind(name_owned);throw;}
    }
}

void NativeRenderPassInitializationBlock::execute(void* owner,std::size_t bytes,bool particle,
    const NativeDepthDownscalePassArguments& depth,const NativeParticleBlendPassArguments& blend) {
    require(owner && !(reinterpret_cast<Word>(owner)&3u) && bytes>=0x20 && phase_==Phase::prepared,
        "render pass requires aligned existing20h storage and a prepared persistent frame");
    auto& c=*context_;auto& a=acquired_;a.receiver=owner;phase_=Phase::executing;
    bool name_owned=false;
    try {
        a.raw_post_effect=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
        set_state(0);
        if(a.raw_post_effect) {
            construct_native_string_cstring_0041e870(&effect_name_,
                particle?c.particle_name_00d5e41c:c.depth_name_00d620c0,c.post_effects.raw_strings);
            a.effect_name_constructed=true;name_owned=true;set_state(1);
            a.returned_post_effect=construct_native_post_effect20_00b4e470(a.raw_post_effect,0x20,
                effect_name_,3,nullptr,post_effect_);
        }
        put(owner,8,reinterpret_cast<Word>(a.returned_post_effect));a.post_published=true;
        set_state(-1);
        if(name_owned)return_effect_name(); // Native bit stays set; state-1 prevents retry.

        if(particle) {
            set_native_material_texture_unchecked_00b189f0(&current_material(owner),0,
                blend.texture0,c.post_effects.destruction.actual_owners);
            set_native_material_texture_unchecked_00b189f0(&current_material(owner),1,
                blend.texture1,c.post_effects.destruction.actual_owners);
        }
        const Word width=particle?blend.width:depth.width;
        const Word height=particle?blend.height:depth.height;
        detail::store_native_render_pass_sample_offsets(owner,width,height,!particle,
            &c.half_00d7a280,&c.unsigned_bias_00d57da0,&parameter_name_);
        resize_native_string_header_0041dd40(&parameter_name_,c.post_effects.raw_strings,14,true);
        if(void* const data=pointer(word(&parameter_name_,4)))
            std::memcpy(data,c.sample_offsets_name_00d5e40c,word(&parameter_name_)+1u);
        set_state(3);
        // B18AC0 is a complete wrapper: wrapping 4*vector_count and matrix0
        // into the existing actual B17E10/B44D60 registration. Here count is1.
        register_native_material_parameter_00b17e10(current_material(owner),&parameter_name_,
            pointer(reinterpret_cast<Word>(owner)+0x10),4,0,c.parameters);
        set_state(-1);return_parameter_name();
        if(!particle) {
            void* const texture=native_shadow_holder_texture_00b4cb10(depth.input_holder);
            set_native_material_texture_unchecked_00b189f0(&current_material(owner),0,
                texture,c.post_effects.destruction.actual_owners);
        }
        a.raw_holder=singleton_lifetime_allocate({SingletonAllocationKind::object,0x18,0x18});
        set_state(4);
        if(a.raw_holder) {
            if(particle)a.holder_arguments={width,height,blend.format,blend.multisample,blend.mode,blend.external_surface};
            else a.holder_arguments={width,height,depth.format,0,0,nullptr};
            a.returned_holder=construct_native_render_texture_surface_owner_00b4e020(
                a.raw_holder,a.holder_arguments,c.holders,a.holder);
        }
        set_state(-1);put(owner,0xc,reinterpret_cast<Word>(a.returned_holder));a.holder_published=true;
        require(a.returned_holder!=nullptr,"render pass primary getter requires a returned holder");
        auto* const surface=native_render_holder_primary_00b4cb20(a.returned_holder);
        void* const current_post=pointer(word(owner,8));
        require(current_post!=nullptr,"render pass color binding requires its current post-effect");
        set_native_post_effect_color0_00b4cb70(current_post,surface,c.post_effects.destruction.frame_targets);
        a.completed=true;settle();
    } catch(...) {
        try {unwind(name_owned);}catch(...){settle();throw;}
        settle();throw;
    }
}
void initialize_native_depth_downscale_pass_00b540b0(void* owner,std::size_t bytes,
    NativeDepthDownscalePassArguments args,NativeRenderPassInitializationBlock& block) {
    block.execute(owner,bytes,false,args,{});
}
void initialize_native_particle_blend_pass_00b542d0(void* owner,std::size_t bytes,
    NativeParticleBlendPassArguments args,NativeRenderPassInitializationBlock& block) {
    block.execute(owner,bytes,true,{},args);
}
} // namespace bsp
