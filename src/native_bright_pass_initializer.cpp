#include "bsp/native_bright_pass_initializer.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native bright-pass initialization requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
Word word(const void* p,Word byte_offset=0) noexcept {
    Word value;
    __asm { mov eax,p }
    __asm { add eax,byte_offset }
    __asm { mov eax,[eax] }
    __asm { mov value,eax }
    return value;
}
void* pointer(Word value) noexcept {return reinterpret_cast<void*>(value);}
void put(void* p,Word offset,Word value) noexcept {
    *reinterpret_cast<volatile Word*>(reinterpret_cast<Word>(p)+offset)=value;
}
void require(bool value,const char* reason) {if(!value)throw std::logic_error(reason);}
NativeMaterialStorage& current_material(void* owner) {
    void* const post=pointer(word(owner,8));
    require(post!=nullptr,"bright-pass requires its current actual post-effect");
    void* const material=native_post_effect_material_00b4cba0(post);
    require(material!=nullptr,"bright-pass requires its current actual material");
    return *static_cast<NativeMaterialStorage*>(material);
}
} // namespace

NativeBrightPassInitializationBlock::~NativeBrightPassInitializationBlock() noexcept {
    if(phase_!=Phase::idle)std::terminate();
}
void NativeBrightPassInitializationBlock::prepare(NativeBrightPassInitializationContext& c) {
    require(phase_==Phase::idle && c.effect_name_00d62138,
        "bright-pass requires idle host storage and the actual effect name");
    for(const char* name:c.parameter_names)require(name!=nullptr,"bright-pass requires actual parameter names");
    context_=&c;phase_=Phase::preparing;
    try {
        post_effect_.prepare(c.post_effects);
        acquired_.native.effect_name_header=&effect_name_;
        acquired_.native.parameter_name_header=&parameter_name_;
        acquired_.last_parameter_name_header=&last_parameter_name_;
        phase_=Phase::prepared;
    } catch(...) {context_=nullptr;phase_=Phase::idle;throw;}
}
void NativeBrightPassInitializationBlock::cancel_preparation() noexcept {
    if(phase_!=Phase::prepared)std::terminate();
    post_effect_.cancel_preparation();acquired_={};context_=nullptr;phase_=Phase::idle;
}
void NativeBrightPassInitializationBlock::settle() noexcept {
    if(post_effect_.phase()==NativePostEffect20ConstructionBlock::Phase::prepared)
        post_effect_.cancel_preparation();
    phase_=Phase::settled;
}
void NativeBrightPassInitializationBlock::reset_after_host_quiescence() noexcept {
    if(phase_!=Phase::settled || post_effect_.phase()!=NativePostEffect20ConstructionBlock::Phase::idle)
        std::terminate();
    effect_name_.~NativeString();new(&effect_name_) NativeString;
    parameter_name_.~NativeString();new(&parameter_name_) NativeString;
    last_parameter_name_.~NativeString();new(&last_parameter_name_) NativeString;
    acquired_={};context_=nullptr;phase_=Phase::idle;
}
void NativeBrightPassInitializationBlock::return_effect_name() {
    acquired_.native.effect_name_return_started=true;
    destroy_native_string_header_0041dd20(&effect_name_,context_->post_effects.raw_strings);
}
void NativeBrightPassInitializationBlock::return_parameter_name(bool last) {
    acquired_.native.parameter_name_return_started=true;
    ++acquired_.parameter_returns_started;
    if(last)acquired_.last_parameter_return_started=true;
    destroy_native_string_header_0041dd20(last?&last_parameter_name_:&parameter_name_,
        context_->post_effects.raw_strings);
}
void NativeBrightPassInitializationBlock::register_parameter(Word index) {
    constexpr Word lengths[]={19,17,11,13,13};
    const bool last=index==4;
    NativeString& name=last?last_parameter_name_:parameter_name_;
    put(&name,0,0);put(&name,4,0);
    resize_native_string_header_0041dd40(&name,context_->post_effects.raw_strings,lengths[index],true);
    if(void* const data=pointer(word(&name,4)))
        std::memcpy(data,context_->parameter_names[index],word(&name)+1u);
    acquired_.native.native_state=static_cast<int>(index)+3;
    register_native_material_float_00b18b20(current_material(acquired_.native.receiver),&name,
        pointer(reinterpret_cast<Word>(acquired_.native.receiver)+0x210+4*index),context_->parameters);
    acquired_.native.native_state=-1;return_parameter_name(last);
}
void NativeBrightPassInitializationBlock::unwind() {
    auto& a=acquired_.native;
    // FuncInfo DF8D24, nine-state map DF8D48. State2 shares the masked
    // name return but goes to -1; it is unvisited on the normal body.
    while(a.native_state>=0) {
        const int action=a.native_state;a.native_state=action==1?0:-1;
        try {
            if(action==0) {
                if(post_effect_.acquired().native_completed)a.completed_post_preserved_after_host_failure=true;
                else {a.post_raw_free_started=true;singleton_lifetime_free(a.raw_post_effect);}
            } else if(action==1 || action==2) {
                if(acquired_.effect_name_mask&1u) {
                    acquired_.effect_name_mask&=~1u;return_effect_name();
                }
            } else if(action>=3 && action<=7)return_parameter_name(action==7);
            else if(action==8) {a.holder_raw_free_started=true;singleton_lifetime_free(a.raw_holder);}
            else std::terminate();
        } catch(...) {unwind();throw;}
    }
}

void initialize_native_bright_pass_00b54940(void* owner,std::size_t bytes,
    NativeDepthDownscalePassArguments args,NativeBrightPassInitializationBlock& block) {
    using Block=NativeBrightPassInitializationBlock;
    require(owner && !(reinterpret_cast<Word>(owner)&3u) && bytes>=0x224 && block.phase_==Block::Phase::prepared,
        "bright-pass requires aligned existing224h storage and a prepared persistent frame");
    auto& c=*block.context_;auto& a=block.acquired_.native;
    a.receiver=owner;block.phase_=Block::Phase::executing;
    try {
        a.raw_post_effect=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
        a.native_state=0;
        if(a.raw_post_effect) {
            construct_native_string_cstring_0041e870(&block.effect_name_,c.effect_name_00d62138,c.post_effects.raw_strings);
            a.effect_name_constructed=true;block.acquired_.effect_name_mask=1;a.native_state=1;
            a.returned_post_effect=construct_native_post_effect20_00b4e470(a.raw_post_effect,0x20,
                block.effect_name_,3,nullptr,block.post_effect_);
        }
        put(owner,8,reinterpret_cast<Word>(a.returned_post_effect));a.post_published=true;a.native_state=-1;
        if(block.acquired_.effect_name_mask&1u)block.return_effect_name(); // Native bit remains set.
        for(Word index=0;index<5;++index)block.register_parameter(index);
        void* const texture=native_shadow_holder_texture_00b4cb10(args.input_holder);
        set_native_material_texture_unchecked_00b189f0(&current_material(owner),0,
            texture,c.post_effects.destruction.actual_owners);
        a.raw_holder=singleton_lifetime_allocate({SingletonAllocationKind::object,0x18,0x18});
        a.native_state=8;
        if(a.raw_holder) {
            a.holder_arguments={args.width,args.height,args.format,0,0,nullptr};
            a.returned_holder=construct_native_render_texture_surface_owner_00b4e020(
                a.raw_holder,a.holder_arguments,c.holders,a.holder);
        }
        a.native_state=-1;put(owner,0xc,reinterpret_cast<Word>(a.returned_holder));a.holder_published=true;
        require(a.returned_holder!=nullptr,"bright-pass primary getter requires a returned holder");
        auto* const surface=native_render_holder_primary_00b4cb20(a.returned_holder);
        void* const current_post=pointer(word(owner,8));
        require(current_post!=nullptr,"bright-pass color binding requires its current post-effect");
        set_native_post_effect_color0_00b4cb70(current_post,surface,c.post_effects.destruction.frame_targets);
        a.completed=true;block.settle();
    } catch(...) {
        try {block.unwind();}catch(...){block.settle();throw;}
        block.settle();throw;
    }
}
} // namespace bsp
