#include "bsp/native_luminance_owner.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native luminance owner requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word=std::uint32_t;
void* at(const void* p,Word offset=0) noexcept {return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+offset);}
Word word(const volatile void* p) noexcept {
    Word value;
    __asm { mov eax,p }
    __asm { mov eax,[eax] }
    __asm { mov value,eax }
    return value;
}
void* pointer(Word value) noexcept {return reinterpret_cast<void*>(value);}
void put(void* p,Word value) noexcept {*static_cast<volatile Word*>(p)=value;}
void require(bool value,const char* reason) {if(!value)throw std::logic_error(reason);}
NativeTexture2DOwnerContext& textures(NativeRenderEffectLifetimeContext& c) noexcept {
    return c.texture_holders.textures.construction.owners;
}
const volatile Word* member_table(void* p,NativeRenderEffectLifetimeContext& c) {
    switch(word(p)) {
    case 0xd61ec0:return c.actual_post_effect_profile_00d61ec0;
    case 0xd61eb8:return c.actual_holder_profile_00d61eb8;
    case 0xd61948:return textures(c).renderer_notification.accounting_tables.texture_2d_00d61948;
    case 0xd619a0:return textures(c).actual_surface_profile_00d619a0;
    default:throw std::logic_error("luminance member has unsupported actual profile");
    }
}
void release_zero(void* p,NativeRenderEffectLifetimeContext& c) {
    require(member_table(p,c)[0]==0xbd30e0,"luminance member requires actual BD30E0");
    const Word terminal=member_table(p,c)[1]; // BD30E0 reloads current table.
    switch(terminal) {
    case 0xb4e410:delete_native_render_texture_surface_owner_00b4e410(p,1,c.texture_holders);return;
    case 0xb3f590:delete_native_texture_2d_00b3f590(p,1,textures(c));return;
    case 0xb3f5b0:delete_native_surface_00b3f5b0(*static_cast<NativeSurfaceOwnerStorage*>(p),1,textures(c).surfaces);return;
    case 0xb4e430: {
        auto& reference=c.actual_post_effects.resolve_actual(p);
        auto* const actual=dynamic_cast<NativePostEffect20Reference*>(&reference);
        require(actual && actual->storage()==p &&
            &actual->reference_count==static_cast<std::atomic<std::int32_t>*>(at(p,4)),
            "luminance post member requires its canonical concrete reference");
        actual->release_zero_references();return;
    }
    default:throw std::logic_error("luminance member has unsupported deleting slot");
    }
}
void release_slot(void* owner,Word offset,void* captured,NativeTextureSurfaceReferenceIncrement decrement,
    NativeRenderEffectLifetimeContext& c) {
    if(captured) {
        if(decrement(static_cast<volatile long*>(at(captured,4)))==0)release_zero(captured,c);
        put(at(owner,offset),0);
    }
}
int cleanup_exception(unsigned long code) noexcept {
    if(code==0xe06d7363u)std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_base(void* owner,NativeRenderEffectLifetimeContext& c) noexcept {
    __try {destroy_native_render_effect_base_00b0f5e0(owner,c);}
    __except(cleanup_exception(GetExceptionCode())) {__assume(0);}
}
struct BaseCleanup {
    void* owner;NativeRenderEffectLifetimeContext& context;bool armed{true};
    ~BaseCleanup() noexcept {if(armed)unwind_base(owner,context);}
};
NativeMaterialStorage& current_material(void* owner,Word member) {
    void* const post=pointer(word(at(owner,member)));
    require(post!=nullptr,"luminance requires its current post20");
    void* const material=native_post_effect_material_00b4cba0(post);
    require(material!=nullptr,"luminance requires its current material");
    return *static_cast<NativeMaterialStorage*>(material);
}
} // namespace

void* construct_native_luminance_owner_00b50d40(void* owner,const volatile Word& adaptation_bits) {
    const Word captured=adaptation_bits;
    put(owner,0xceb130);::new(at(owner,4)) std::atomic<std::int32_t>(1);
    put(at(owner,8),0);put(at(owner,0xc),0);put(owner,0xd61fe0);put(at(owner,0x240),captured);
    for(Word offset=0x214;offset<=0x23c;offset+=4)put(at(owner,offset),0);
    put(at(owner,0x244),0);put(at(owner,0x248),0);
    *static_cast<volatile std::uint8_t*>(at(owner,0x24c))=0;
    put(at(owner,0x210),0);return owner;
}
void destroy_native_luminance_owner_00b50dd0(void* owner,NativeRenderEffectLifetimeContext& c) {
    put(owner,0xd61fe0);BaseCleanup cleanup{owner,c};
    for(Word offset=0x228;offset<=0x234;offset+=4) {
        void* const captured=pointer(word(at(owner,offset)));
        if(captured)release_slot(owner,offset,captured,c.actual_decrement_00ce2220,c);
    }
    void* first=pointer(word(at(owner,0x210)));
    const auto decrement=c.actual_decrement_00ce2220;
    for(Word offset:{0x210u,0x238u,0x23cu,0x214u,0x218u,0x21cu,0x220u,0x224u,0x248u,0x244u}) {
        if(offset!=0x210)first=pointer(word(at(owner,offset)));
        release_slot(owner,offset,first,decrement,c);
    }
    cleanup.armed=false;destroy_native_render_effect_base_00b0f5e0(owner,c);
}
void* delete_native_luminance_owner_00b50fe0(void* owner,Word flags,NativeRenderEffectLifetimeContext& c) {
    destroy_native_luminance_owner_00b50dd0(owner,c);
    if(flags&1u)singleton_lifetime_free(owner);
    return owner;
}

NativeLuminanceInitializationBlock::~NativeLuminanceInitializationBlock() noexcept {
    if(phase_!=Phase::idle)std::terminate();
}
void NativeLuminanceInitializationBlock::prepare(NativeLuminanceInitializationContext& c) {
    require(phase_==Phase::idle && c.sample_offsets_name_00d5e40c && c.adaptation_name_00d61ff4,
        "luminance requires idle host storage and actual parameter names");
    for(const char* name:c.effect_names)require(name!=nullptr,"luminance requires actual effect names");
    context_=&c;phase_=Phase::preparing;std::size_t prepared=0;
    try {
        for(auto& post:posts_){post.prepare(c.post_effects);++prepared;}
        acquired_.common_name_header=&common_name_;acquired_.adaptation_name_header=&adaptation_name_;
        phase_=Phase::prepared;
    } catch(...) {
        while(prepared)posts_[--prepared].cancel_preparation();
        context_=nullptr;phase_=Phase::idle;throw;
    }
}
void NativeLuminanceInitializationBlock::cancel_preparation() noexcept {
    if(phase_!=Phase::prepared)std::terminate();
    for(auto& post:posts_)post.cancel_preparation();
    acquired_={};context_=nullptr;phase_=Phase::idle;
}
void NativeLuminanceInitializationBlock::settle() noexcept {
    for(auto& post:posts_)if(post.phase()==NativePostEffect20ConstructionBlock::Phase::prepared)post.cancel_preparation();
    phase_=Phase::settled;
}
void NativeLuminanceInitializationBlock::reset_after_host_quiescence() noexcept {
    if(phase_!=Phase::settled)std::terminate();
    for(auto& post:posts_)if(post.phase()!=NativePostEffect20ConstructionBlock::Phase::idle)std::terminate();
    common_name_.~NativeString();new(&common_name_) NativeString;
    adaptation_name_.~NativeString();new(&adaptation_name_) NativeString;
    acquired_={};context_=nullptr;phase_=Phase::idle;
}
void NativeLuminanceInitializationBlock::build_name(NativeString& name,const char* bytes,Word length) {
    put(&name,0);put(at(&name,4),0);
    resize_native_string_header_0041dd40(&name,context_->post_effects.raw_strings,length,true);
    if(void* const data=pointer(word(at(&name,4))))std::memcpy(data,bytes,word(&name)+1u);
}
void NativeLuminanceInitializationBlock::return_name(NativeString& name) {
    acquired_.last_return_header=&name;++acquired_.name_returns_started;
    destroy_native_string_header_0041dd20(&name,context_->post_effects.raw_strings);
}
namespace detail {
namespace {
void create_luminance_holder(Word index,Word dimension,int state,
    NativeRenderTextureSurfaceOwnerContext& context,NativeLuminanceInitializationAcquired& a) {
    a.current_holder=index;
    a.raw_holders[index]=singleton_lifetime_allocate({SingletonAllocationKind::object,0x18,0x18});
    a.native_state=state;
    if(a.raw_holders[index]) {
        const NativeRenderTextureSurfaceOwnerArguments args{dimension,dimension,114,0,0,nullptr};
        a.returned_holders[index]=construct_native_render_texture_surface_owner_00b4e020(
            a.raw_holders[index],args,context,a.holders[index]);
    }
    // First four loop publications precede disarming; final two follow it.
    if(index>=4)a.native_state=-1;
    put(at(a.receiver,0x228+4*index),reinterpret_cast<Word>(a.returned_holders[index]));a.holders_published[index]=true;
    a.native_state=-1;
}
} // namespace
// Normal B51090..B51180 prefix, also used independently by the bounded
// differential probe. Its caller owns the original states0/1/2 cleanup.
void initialize_native_luminance_holder_prefix(void* owner,
    NativeRenderTextureSurfaceOwnerContext& context,NativeLuminanceInitializationAcquired& a) {
    a.receiver=owner;
    for(Word i=0;i<4;++i)create_luminance_holder(i,1u<<(2*i),0,context,a);
    create_luminance_holder(4,1,1,context,a);create_luminance_holder(5,1,2,context,a);
}
} // namespace detail
void NativeLuminanceInitializationBlock::create_post(Word index,int allocation_state) {
    constexpr Word lengths[]={17,17,19,19,22,24};
    auto& a=acquired_;a.current_post=index;
    a.raw_posts[index]=singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,0x20});
    a.native_state=allocation_state;
    if(a.raw_posts[index]) {
        build_name(common_name_,context_->effect_names[index],lengths[index]);
        a.name_mask|=1u<<index;a.native_state=allocation_state+1;
        a.returned_posts[index]=construct_native_post_effect20_00b4e470(a.raw_posts[index],0x20,
            common_name_,3,nullptr,posts_[index]);
    }
    put(at(a.receiver,0x210+4*index),reinterpret_cast<Word>(a.returned_posts[index]));a.posts_published[index]=true;
    a.native_state=-1;
    if(a.name_mask&(1u<<index)) {
        if(index<5)a.name_mask&=~(1u<<index); // Native final bit20 remains set.
        return_name(common_name_);
    }
}
void NativeLuminanceInitializationBlock::bind_sample(Word index,const void* input,Word output_member,int state) {
    const Word member=0x210+4*index;
    void* const texture=native_shadow_holder_texture_00b4cb10(input);
    set_native_material_texture_unchecked_00b189f0(&current_material(acquired_.receiver,member),0,
        texture,context_->post_effects.destruction.actual_owners);
    build_name(common_name_,context_->sample_offsets_name_00d5e40c,14);acquired_.native_state=state;
    register_native_material_parameter_00b17e10(current_material(acquired_.receiver,member),&common_name_,
        at(acquired_.receiver,0x10),64,0,context_->parameters);
    acquired_.native_state=-1;return_name(common_name_);
    void* const output=pointer(word(at(acquired_.receiver,output_member)));
    require(output!=nullptr,"luminance sample requires current output holder");
    auto* const surface=native_render_holder_primary_00b4cb20(output);
    void* const post=pointer(word(at(acquired_.receiver,member)));
    require(post!=nullptr,"luminance sample requires current post frame");
    set_native_post_effect_color0_00b4cb70(post,surface,context_->post_effects.destruction.frame_targets);
}
void NativeLuminanceInitializationBlock::unwind() {
    auto& a=acquired_;
    while(a.native_state>=0) {
        const int action=a.native_state;
        const bool named=action==4 || action==7 || action==11 || action==15 || action==19 || action==23;
        a.native_state=named?action-1:-1;
        try {
            if(action<=2) {a.holder_free_started[a.current_holder]=true;singleton_lifetime_free(a.raw_holders[a.current_holder]);}
            else if(action==3 || action==6 || action==10 || action==14 || action==18 || action==22) {
                const Word index=a.current_post;
                if(posts_[index].acquired().native_completed)a.completed_posts_preserved[index]=true;
                else {a.post_free_started[index]=true;singleton_lifetime_free(a.raw_posts[index]);}
            } else if(named || action==5 || action==8 || action==12 || action==16 || action==20 || action==24) {
                const Word mask=1u<<a.current_post;
                if(a.name_mask&mask){a.name_mask&=~mask;return_name(common_name_);}
            } else if(action==9 || action==13 || action==17 || action==21)return_name(common_name_);
            else if(action==25)return_name(adaptation_name_);
            else std::terminate();
        } catch(...) {unwind();throw;}
    }
}
void initialize_native_luminance_00b51090(void* owner,std::size_t bytes,const void* input,
    NativeLuminanceInitializationBlock& block) {
    using Block=NativeLuminanceInitializationBlock;
    require(owner && !(reinterpret_cast<Word>(owner)&3u) && bytes>=0x250 && block.phase_==Block::Phase::prepared,
        "luminance initialization requires existing aligned250h storage and prepared companions");
    auto& c=*block.context_;auto& a=block.acquired_;a.receiver=owner;block.phase_=Block::Phase::executing;
    try {
        detail::initialize_native_luminance_holder_prefix(owner,c.holders,a);
        block.create_post(0,3);block.create_post(1,6);block.bind_sample(1,input,0x234,9);
        for(Word i=2;i<=4;++i) {
            block.create_post(i,static_cast<int>(4*i+2));
            const Word input_member=0x23c-4*i;
            block.bind_sample(i,pointer(word(at(owner,input_member))),input_member-4,static_cast<int>(4*i+5));
        }
        block.create_post(5,22);
        block.build_name(block.adaptation_name_,c.adaptation_name_00d61ff4,18);a.native_state=25;
        register_native_material_float_00b18b20(current_material(owner,0x224),&block.adaptation_name_,at(owner,0x240),c.parameters);
        a.native_state=-1;block.return_name(block.adaptation_name_);
        auto& owners=c.holders.textures.construction.owners;
        void* const renderer=const_cast<void*>(owners.renderer_notification.actual_renderer_00f8d394);
        require(renderer && word(renderer)==0xd5f0a8 && owners.actual_renderer_profile_00d5f0a8[0x88/4]==0xb2a070,
            "luminance readback requires actual renderer texture factory");
        void* const texture=create_native_runtime_texture_2d_00b2a070(renderer,a.readback_arguments,c.holders.textures,a.readback_texture);
        put(at(owner,0x244),reinterpret_cast<Word>(texture));
        require(texture && word(texture)==0xd61948 &&
            owners.renderer_notification.accounting_tables.texture_2d_00d61948[0x30/4]==0xb3fd80,
            "luminance readback requires actual texture surface getter");
        auto* const surface=get_native_texture_surface_00b3fd80(texture,0,0,c.holders.levels,a.readback_surface);
        put(at(owner,0x248),reinterpret_cast<Word>(surface));a.completed=true;block.settle();
    } catch(...) {
        try {block.unwind();}catch(...){block.settle();throw;}
        block.settle();throw;
    }
}
} // namespace bsp
