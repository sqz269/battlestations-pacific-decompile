#include "bsp/native_bloom_initializer.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_system_constant_gather.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native bloom initialization requires MSVC Win32.
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
void put(void* p,Word byte_offset,Word value) noexcept {
    *reinterpret_cast<volatile Word*>(reinterpret_cast<Word>(p)+byte_offset)=value;
}
void require(bool value,const char* reason) {if(!value)throw std::logic_error(reason);}
Word texture_dimension(void* texture,bool height,const NativeBloomTextureContext& c) {
    require(texture && word(texture)==0x00d61948 && c.texture_profile_00d61948 &&
        c.texture_profile_00d61948[height?16:15]==(height?0x00b3ce60u:0x00b3ce50u),
        "bloom dimension getter requires its current actual D61948 profile");
    return height?get_raw_texture_height_00b3ce60(texture):get_raw_texture_width_00b3ce50(texture);
}
NativeMaterialStorage& current_material(void* owner,Word member) {
    void* const post=pointer(word(owner,member));
    require(post!=nullptr,"bloom requires its current post-effect");
    void* const material=native_post_effect_material_00b4cba0(post);
    require(material!=nullptr,"bloom requires its current material");
    return *static_cast<NativeMaterialStorage*>(material);
}
} // namespace

namespace detail {
// B5505F..B55081 arithmetic: the float spill MUST precede the height getter.
void bloom_output_half_texel_x(Word width,const volatile float* bias,
    const volatile double* half,float* saved) noexcept {
    __asm {
        mov eax,width
        test eax,eax
        fild dword ptr width
        jge positive
        mov edx,bias
        fadd dword ptr [edx]
    positive:
        mov edx,half
        fdivr qword ptr [edx]
        mov edx,saved
        fstp dword ptr [edx]
    }
}
// B55084..B550FA arithmetic/stores. Retain one freshly loaded half value on
// x87 for the input dimensions; preserve native sign-test/read ordering.
void bloom_finish_half_texels(void* owner,Word height,const float* saved,
    const volatile float* bias,const volatile double* half) noexcept {
    float output_y,input_x,input_y;
    __asm {
        mov eax,height
        test eax,eax
        fild dword ptr height
        jge output_positive
        mov edx,bias
        fadd dword ptr [edx]
    output_positive:
        mov edx,half
        fld qword ptr [edx]
        fld st(0)
        fdivrp st(2),st(0)
        fxch st(1)
        fstp dword ptr output_y
        mov ecx,owner
        mov edx,saved
        fld dword ptr [edx]
        fstp dword ptr [ecx+428h]
        fld dword ptr output_y
        fstp dword ptr [ecx+42ch]
        fild dword ptr [ecx+10h]
        mov eax,[ecx+10h]
        test eax,eax
        jge input_width_positive
        mov edx,bias
        fadd dword ptr [edx]
    input_width_positive:
        fdivr st(0),st(1)
        mov eax,[ecx+14h]
        test eax,eax
        fstp dword ptr input_x
        fild dword ptr [ecx+14h]
        jge input_height_positive
        mov edx,bias
        fadd dword ptr [edx]
    input_height_positive:
        fdivp st(1),st(0)
        fstp dword ptr input_y
        fld dword ptr input_x
        fstp dword ptr [ecx+430h]
        fld dword ptr input_y
        fstp dword ptr [ecx+434h]
    }
}

// Contiguous native body prefix through B550FA, before the first post20
// allocation. The caller owns state0 cleanup; it frees only the current raw
// holder on failure. This component is not a separate original function.
void initialize_native_bloom_texture_prefix(void* owner,NativeBloomInitializationArguments args,
    NativeBloomTextureContext& c,NativeBloomTextureAcquired& a,int& state) {
    require(!a.entered,"bloom texture prefix requires a fresh persistent frame");a.entered=true;
    put(owner,0x438,args.parameter_438_bits);
    void* texture=native_shadow_holder_texture_00b4cb10(args.input_holder);
    put(owner,0x10,texture_dimension(texture,false,c));
    texture=native_shadow_holder_texture_00b4cb10(args.input_holder);
    put(owner,0x14,texture_dimension(texture,true,c));
    for(Word i=0;i<3;++i) {
        a.current=i;
        a.raw[i]=singleton_lifetime_allocate({SingletonAllocationKind::object,0x18,0x18});
        state=0;
        if(a.raw[i]) {
            a.holder_arguments={args.width,args.height,args.format,0,0,nullptr};
            a.returned[i]=construct_native_render_texture_surface_owner_00b4e020(
                a.raw[i],a.holder_arguments,c.holders,a.holders[i]);
        }
        put(owner,0x18+4*i,reinterpret_cast<Word>(a.returned[i]));a.published[i]=true;state=-1;
    }
    void* const captured_height_texture=native_shadow_holder_texture_00b4cb10(pointer(word(owner,0x18)));
    void* const current_width_texture=native_shadow_holder_texture_00b4cb10(pointer(word(owner,0x18)));
    const Word width=texture_dimension(current_width_texture,false,c);
    bloom_output_half_texel_x(width,&c.unsigned_bias_00ce3978,&c.half_00d7a280,&a.saved_output_x);
    const Word height=texture_dimension(captured_height_texture,true,c);
    bloom_finish_half_texels(owner,height,&a.saved_output_x,&c.unsigned_bias_00ce3978,&c.half_00d7a280);
    a.completed=true;
}
} // namespace detail

NativeBloomInitializationBlock::~NativeBloomInitializationBlock() noexcept {
    if(phase_!=Phase::idle)std::terminate();
}
void NativeBloomInitializationBlock::prepare(NativeBloomInitializationContext& c) {
    require(phase_==Phase::idle && c.blur_name_00d62180 && c.bloom_name_00d62164 &&
        c.sample_offsets_name_00d5e40c && c.sample_weights_name_00d61fac && c.texture_offset_name_00d62170,
        "bloom requires idle persistent storage and actual names");
    context_=&c;phase_=Phase::preparing;
    try {
        for(auto& post:posts_)post.prepare(c.post_effects);
        acquired_.common_name_header=&common_name_;acquired_.last_name_header=&last_name_;
        phase_=Phase::prepared;
    } catch(...) {
        for(auto& post:posts_)if(post.phase()==NativePostEffect20ConstructionBlock::Phase::prepared)post.cancel_preparation();
        context_=nullptr;phase_=Phase::idle;throw;
    }
}
void NativeBloomInitializationBlock::cancel_preparation() noexcept {
    if(phase_!=Phase::prepared)std::terminate();
    for(auto& post:posts_)post.cancel_preparation();
    acquired_={};context_=nullptr;phase_=Phase::idle;
}
void NativeBloomInitializationBlock::settle() noexcept {
    for(auto& post:posts_)if(post.phase()==NativePostEffect20ConstructionBlock::Phase::prepared)post.cancel_preparation();
    phase_=Phase::settled;
}
void NativeBloomInitializationBlock::reset_after_host_quiescence() noexcept {
    if(phase_!=Phase::settled)std::terminate();
    for(auto& post:posts_)if(post.phase()!=NativePostEffect20ConstructionBlock::Phase::idle)std::terminate();
    common_name_.~NativeString();new(&common_name_) NativeString;
    last_name_.~NativeString();new(&last_name_) NativeString;
    acquired_={};context_=nullptr;phase_=Phase::idle;
}
void NativeBloomInitializationBlock::build_name(NativeString& name,const char* bytes,Word length) {
    put(&name,0,0);put(&name,4,0);
    resize_native_string_header_0041dd40(&name,context_->post_effects.raw_strings,length,true);
    if(void* const data=pointer(word(&name,4)))std::memcpy(data,bytes,word(&name)+1u);
}
void NativeBloomInitializationBlock::return_name(NativeString& name) {
    acquired_.last_return_header=&name;++acquired_.name_returns_started;
    destroy_native_string_header_0041dd20(&name,context_->post_effects.raw_strings);
}
void NativeBloomInitializationBlock::create_post(Word index,Word member,const char* name,
    Word length,int allocation_state,int name_state) {
    auto& a=acquired_;
    a.raw_posts[index]=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
    a.native_state=allocation_state;
    if(a.raw_posts[index]) {
        build_name(common_name_,name,length);
        a.name_mask|=1u<<index;a.native_state=name_state;
        a.returned_posts[index]=construct_native_post_effect20_00b4e470(a.raw_posts[index],0x20,
            common_name_,3,nullptr,posts_[index]);
    }
    put(a.receiver,member,reinterpret_cast<Word>(a.returned_posts[index]));a.posts_published[index]=true;
    a.native_state=-1;
    if(a.name_mask&(1u<<index)) {
        if(index==0)a.name_mask&=~1u; // Native clears blur bit before normal return; bloom bit remains set.
        return_name(common_name_);
    }
}
void NativeBloomInitializationBlock::register_parameter(Word member,Word source,const char* name,
    Word vector_count,int state,bool last) {
    NativeString& header=last?last_name_:common_name_;
    build_name(header,name,14);acquired_.native_state=state;
    register_native_material_parameter_00b17e10(current_material(acquired_.receiver,member),&header,
        pointer(reinterpret_cast<Word>(acquired_.receiver)+source),vector_count*4u,0,context_->parameters);
    acquired_.native_state=-1;return_name(header);
}
void NativeBloomInitializationBlock::unwind() {
    auto& a=acquired_;
    while(a.native_state>=0) {
        const int action=a.native_state;a.native_state=action==2?1:(action==8?7:-1);
        try {
            if(action==0) {
                a.textures.raw_free_started=true;singleton_lifetime_free(a.textures.raw[a.textures.current]);
            } else if(action==1 || action==7) {
                const Word index=action==1?0u:1u;
                if(posts_[index].acquired().native_completed)a.completed_post_preserved[index]=true;
                else {a.post_raw_free_started[index]=true;singleton_lifetime_free(a.raw_posts[index]);}
            } else if(action==2 || action==3 || action==8 || action==9) {
                const Word mask=action<7?1u:2u;
                if(a.name_mask&mask){a.name_mask&=~mask;return_name(common_name_);}
            } else if(action==4 || action==5 || action==6 || action==10 || action==11) {
                return_name(common_name_);
            } else if(action==12)return_name(last_name_);
            else std::terminate();
        } catch(...) {unwind();throw;}
    }
}

void initialize_native_bloom_00b54f90(void* owner,std::size_t bytes,
    NativeBloomInitializationArguments args,NativeBloomInitializationBlock& block) {
    require(owner && !(reinterpret_cast<Word>(owner)&3u) && bytes>=0x43c &&
        block.phase_==NativeBloomInitializationBlock::Phase::prepared,
        "bloom requires an existing43Ch owner and prepared persistent block");
    auto& a=block.acquired_;auto& c=*block.context_;a.receiver=owner;
    block.phase_=NativeBloomInitializationBlock::Phase::executing;
    try {
        detail::initialize_native_bloom_texture_prefix(owner,args,c.textures,a.textures,a.native_state);
        block.create_post(0,0x24,c.blur_name_00d62180,12,1,2);
        block.register_parameter(0x24,0x228,c.sample_offsets_name_00d5e40c,16,4);
        block.register_parameter(0x24,0x328,c.sample_weights_name_00d61fac,16,5);
        block.register_parameter(0x24,0x430,c.texture_offset_name_00d62170,1,6);
        void* const texture=native_shadow_holder_texture_00b4cb10(args.input_holder);
        set_native_material_texture_unchecked_00b189f0(&current_material(owner,0x24),0,texture,
            c.post_effects.destruction.actual_owners);
        block.create_post(1,8,c.bloom_name_00d62164,10,7,8);
        block.register_parameter(8,0x28,c.sample_offsets_name_00d5e40c,16,10);
        block.register_parameter(8,0x128,c.sample_weights_name_00d61fac,16,11);
        block.register_parameter(8,0x428,c.texture_offset_name_00d62170,1,12,true);
        a.completed=true;block.settle();
    } catch(...) {
        try {block.unwind();}catch(...){block.settle();throw;}
        block.settle();throw;
    }
}
} // namespace bsp
