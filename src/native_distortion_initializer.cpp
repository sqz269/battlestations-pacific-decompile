#include "bsp/native_distortion_initializer.hpp"
#include "bsp/native_renderer_format_check.hpp"
#include "bsp/native_render_resources_remap.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_system_constant_gather.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native distortion initialization requires MSVC Win32.
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
void* pointer(Word bits) noexcept {return reinterpret_cast<void*>(bits);}
void* at(const void* p,Word offset) noexcept {return pointer(reinterpret_cast<Word>(p)+offset);}
void put(void* p,Word offset,Word bits) noexcept {*static_cast<volatile Word*>(at(p,offset))=bits;}
void require(bool value,const char* reason) {if(!value)throw std::logic_error(reason);}
void* renderer(NativeDistortionTextureContext& c) {
    void* const current=c.actual_renderer_00f8d394;
    require(current && word(current)==0xd5f0a8 && c.renderer_profile_00d5f0a8 &&
        c.renderer_profile_00d5f0a8[0xf8/4]==0xb21ec0,"distortion requires actual D5F0A8 format dispatch");
    return current;
}
Word dimension(void* texture,bool height,const NativeBloomTextureContext& c) {
    require(texture && word(texture)==0xd61948 && c.texture_profile_00d61948 &&
        c.texture_profile_00d61948[height?16:15]==(height?0xb3ce60u:0xb3ce50u),
        "distortion requires actual D61948 dimension dispatch");
    return height?get_raw_texture_height_00b3ce60(texture):get_raw_texture_width_00b3ce50(texture);
}
NativeMaterialStorage& material(void* owner,Word member) {
    void* const post=pointer(word(owner,member));require(post!=nullptr,"distortion requires current post");
    void* const result=native_post_effect_material_00b4cba0(post);
    require(result!=nullptr,"distortion requires current material");return *static_cast<NativeMaterialStorage*>(result);
}
void* allocate(Word bytes) {return singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes});}
} // namespace

namespace detail {
void distortion_reciprocal_width(Word value,const volatile float* bias,
    const volatile double* half,bool use_half,float* saved) noexcept {
    __asm {
        mov eax,value
        test eax,eax
        fild dword ptr value
        jge positive
        mov edx,bias
        fadd dword ptr [edx]
    positive:
    }
    if(use_half) {
        __asm { mov edx,half }
        __asm { fdivr qword ptr [edx] }
    } else {
        __asm { fld1 }
        __asm { fdivrp st(1),st(0) }
    }
    __asm { mov edx,saved }
    __asm { fstp dword ptr [edx] }
}
void distortion_reciprocal_pair(void* owner,Word height,const float* saved,
    const volatile float* bias,const volatile double* half,bool use_half) noexcept {
    float y;
    distortion_reciprocal_width(height,bias,half,use_half,&y);
    void* const destination=at(owner,use_half?0x24u:0x2cu);
    __asm {
        mov ecx,destination
        mov edx,saved
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        fld dword ptr y
        fstp dword ptr [ecx+4]
    }
}
bool initialize_native_distortion_texture_prefix(void* owner,Word width,Word height,
    NativeDistortionTextureContext& c,NativeDistortionTextureAcquired& a,int& state) {
    a.entered=true;
    if(check_native_renderer_device_format_00b21ec0(renderer(c),22,16,3,111))put(owner,0x264,111);
    else {
        if(!check_native_renderer_device_format_00b21ec0(renderer(c),22,16,3,114))return false;
        put(owner,0x264,114);
    }
    if(!check_native_renderer_device_format_00b21ec0(renderer(c),22,16,3,112))return false;
    put(owner,0x268,112);
    const bool blending=check_native_renderer_post_blend_render_target_00b20160(c.actual_renderer_00f8d394,112);
    *static_cast<volatile unsigned char*>(at(owner,0x260))=static_cast<unsigned char>(blending);
    a.capabilities_passed=true;a.width=width-(width&7u);a.height=height-(height&7u);
    for(Word i=0;i<3;++i) {
        a.current=i;a.raw[i]=allocate(0x18);state=i==2?1:0;
        if(a.raw[i]) {
            a.holder_arguments={a.width,a.height,word(owner,i==2?0x268u:0x264u),0,0,nullptr};
            a.returned[i]=construct_native_render_texture_surface_owner_00b4e020(a.raw[i],a.holder_arguments,c.textures.holders,a.holders[i]);
        }
        if(i==2)state=-1;
        put(owner,0x10+i*4,reinterpret_cast<Word>(a.returned[i]));a.published[i]=true;
        if(i!=2)state=-1;
    }
    for(bool half:{true,false}) {
        void* const height_texture=native_shadow_holder_texture_00b4cb10(pointer(word(owner,0x10)));
        void* const width_texture=native_shadow_holder_texture_00b4cb10(pointer(word(owner,0x10)));
        const Word w=dimension(width_texture,false,c.textures);
        distortion_reciprocal_width(w,&c.textures.unsigned_bias_00ce3978,&c.textures.half_00d7a280,half,&a.saved_width);
        const Word h=dimension(height_texture,true,c.textures);
        distortion_reciprocal_pair(owner,h,&a.saved_width,&c.textures.unsigned_bias_00ce3978,&c.textures.half_00d7a280,half);
    }
    a.completed=true;return true;
}
} // namespace detail

NativeDistortionInitializationBlock::~NativeDistortionInitializationBlock() noexcept {
    if(phase_!=Phase::idle)std::terminate();
}
void NativeDistortionInitializationBlock::prepare(NativeDistortionInitializationContext& c) {
    require(phase_==Phase::idle,"distortion block requires idle storage");
    require(&c.scenes.nodes==&c.post_effects.cameras.nodes &&
        &c.scenes.nodes.require_raw_name_pool()==&c.post_effects.raw_strings &&
        &c.scenes.one_00d7a24c==&c.post_effects.node_constants.one_00d7a24c &&
        &c.textures.actual_renderer_00f8d394==&c.post_effects.actual_renderer_00f8d394 &&
        c.textures.renderer_profile_00d5f0a8==c.post_effects.renderer_profile_00d5f0a8,
        "distortion providers require SAME canonical node/string/renderer domains");
    require(c.damp_name_00d61fcc && c.sample_offset_name_00d61fbc && c.sample_offsets_name_00d5e40c &&
        c.sample_weights_name_00d61fac && c.bump_fade_name_00d61f9c && c.bump_to_disp_name_00d61f80 &&
        c.texel_offset_name_00d61f70 && c.bump_height_name_00d61f64 && c.refraction_name_00d61f50 &&
        c.scene_name_00d61f40 && c.camera_name_00d61f30 && c.passthrough_name_00d5e448 &&
        c.scene_color_offset_name_00d5e430,"distortion requires actual literal views");
    context_=&c;phase_=Phase::preparing;
    try {
        for(auto& post:posts_)post.prepare(c.post_effects);
        camera_scene_=c.post_effects.cameras.nodes.scenes.reserve_binding();
        camera_lifetime_=c.post_effects.cameras.nodes.attachments.reserve_binding();
        for(Word i=0;i<2;++i)viewport_admissions_[i]=c.post_effects.viewports.admit(viewport_records_[i]);
        acquired_.effect_name_header=&effect_name_;acquired_.parameter_name_header=&parameter_name_;acquired_.last_name_header=&last_name_;
        phase_=Phase::prepared;
    } catch(...) {cancel_preparation();throw;}
}
void NativeDistortionInitializationBlock::cancel_preparation() noexcept {
    if(phase_!=Phase::preparing && phase_!=Phase::prepared)std::terminate();
    camera_scene_.cancel();camera_lifetime_.cancel();for(auto& admission:viewport_admissions_)admission.cancel();
    for(auto& record:viewport_records_)context_->post_effects.viewports.forget_quiescent(record);
    for(auto& post:posts_)if(post.phase()==NativePostEffect20ConstructionBlock::Phase::prepared)post.cancel_preparation();
    acquired_={};context_=nullptr;phase_=Phase::idle;
}
void NativeDistortionInitializationBlock::settle() noexcept {
    camera_scene_.cancel();camera_lifetime_.cancel();for(auto& admission:viewport_admissions_)admission.cancel();
    for(auto& post:posts_)if(post.phase()==NativePostEffect20ConstructionBlock::Phase::prepared)post.cancel_preparation();
    phase_=Phase::settled;
}
NativePostEffect20ConstructionBlock& NativeDistortionInitializationBlock::post(Word i) {
    require(i<3,"distortion post index outside actual three children");return posts_[i];
}
void NativeDistortionInitializationBlock::retire_camera(void* block,NativeCameraReference& reference) noexcept {
    auto& self=*static_cast<NativeDistortionInitializationBlock*>(block);
    if(self.acquired_.camera_reference!=&reference || self.acquired_.camera_retired)std::terminate();
    self.acquired_.camera_retired=true;
}
void NativeDistortionInitializationBlock::reset_after_host_quiescence() noexcept {
    if(phase_!=Phase::settled)std::terminate();
    for(auto& post:posts_)if(post.phase()!=NativePostEffect20ConstructionBlock::Phase::idle)std::terminate();
    if(camera_reference_ && !acquired_.camera_retired)std::terminate();
    if(camera_owner_ && camera_owner_->phase!=NativeCameraOwner::Phase::dead)std::terminate();
    for(auto& record:viewport_records_) {
        auto phase=record.phase();
        if(phase!=NativeViewportRegistry::Phase::unused && phase!=NativeViewportRegistry::Phase::cancelled && phase!=NativeViewportRegistry::Phase::retired)std::terminate();
    }
    camera_reference_.reset();camera_owner_.reset();
    for(auto& record:viewport_records_)context_->post_effects.viewports.forget_quiescent(record);
    effect_name_.~NativeString();new(&effect_name_)NativeString;
    parameter_name_.~NativeString();new(&parameter_name_)NativeString;
    last_name_.~NativeString();new(&last_name_)NativeString;
    scene_publication_=nullptr;acquired_={};context_=nullptr;phase_=Phase::idle;
}
void NativeDistortionInitializationBlock::build_name(NativeString& header,const char* name,Word length) {
    put(&header,0,0);put(&header,4,0);
    resize_native_string_header_0041dd40(&header,context_->post_effects.raw_strings,length,true);
    if(void* const destination=pointer(word(&header,4)))std::memcpy(destination,name,word(&header)+1u);
}
void NativeDistortionInitializationBlock::return_name(NativeString& header) {
    acquired_.last_return_header=&header;++acquired_.name_returns_started;
    destroy_native_string_header_0041dd20(&header,context_->post_effects.raw_strings);
}
void NativeDistortionInitializationBlock::create_post(Word index,Word member,const char* name,
    Word length,int allocation_state,int name_state,bool last) {
    auto& a=acquired_;NativeString& header=last?parameter_name_:effect_name_;
    const Word mask=last?0x10u:(1u<<index);
    a.raw_posts[index]=allocate(0x20);a.native_state=allocation_state;
    if(a.raw_posts[index]) {
        build_name(header,name,length);a.name_mask|=mask;a.native_state=name_state;
        a.returned_posts[index]=construct_native_post_effect20_00b4e470(a.raw_posts[index],0x20,header,3,nullptr,posts_[index]);
    }
    put(a.receiver,member,reinterpret_cast<Word>(a.returned_posts[index]));a.posts_published[index]=true;a.native_state=-1;
    if(a.name_mask&mask){if(!last)a.name_mask&=~mask;return_name(header);}
}
void NativeDistortionInitializationBlock::parameter(Word member,Word source,const char* name,
    Word length,Word count,int state,bool last) {
    auto& header=last?last_name_:parameter_name_;build_name(header,name,length);acquired_.native_state=state;
    register_native_material_parameter_00b17e10(material(acquired_.receiver,member),&header,
        at(acquired_.receiver,source),count,0,context_->parameters);
    acquired_.native_state=-1;return_name(header);
}
void NativeDistortionInitializationBlock::unwind() {
    auto& a=acquired_;
    while(a.native_state>=0) {
        const int action=a.native_state;
        a.native_state=action==3?2:action==10?9:action==17?16:action==21?20:action==25?24:-1;
        try {
            if(action==0 || action==1) {
                a.textures.raw_free_started=true;singleton_lifetime_free(a.textures.raw[a.textures.current]);
            } else if(action==2 || action==9 || action==24) {
                const Word index=action==2?0u:action==9?1u:2u;
                if(posts_[index].acquired().native_completed)a.completed_post_preserved[index]=true;
                else {a.post_raw_free_started[index]=true;singleton_lifetime_free(a.raw_posts[index]);}
            } else if(action==3 || action==4 || action==10 || action==11 || action==17 || action==18 || action==21 || action==22 || action==25 || action==26) {
                const Word mask=action<=4?1u:action<=11?2u:action<=18?4u:action<=22?8u:16u;
                if(a.name_mask&mask){a.name_mask&=~mask;return_name(mask==16?parameter_name_:effect_name_);}
            } else if((action>=5 && action<=8) || (action>=12 && action<=15))return_name(parameter_name_);
            else if(action==16){a.scene_raw_free_started=true;singleton_lifetime_free(a.raw_scene);}
            else if(action==19){a.frame_raw_free_started=true;singleton_lifetime_free(a.raw_frame);}
            else if(action==20) {
                if(a.camera_native_completed)continue;
                camera_owner_.reset();a.camera_slot_return_started=true;
                context_->post_effects.cameras.pool_0108ffb0.return_raw_slot_00b711e0(a.raw_camera);
            } else if(action==23){a.viewport_raw_free_started=true;singleton_lifetime_free(a.raw_viewport);}
            else if(action==27)return_name(last_name_);
            else std::terminate();
        } catch(...) {unwind();throw;}
    }
}

bool initialize_native_distortion_00b4f560(void* owner,std::size_t bytes,Word width,Word height,
    NativeDistortionInitializationBlock& block) {
    require(owner && !(reinterpret_cast<Word>(owner)&3u) && bytes>=0x26c &&
        block.phase_==NativeDistortionInitializationBlock::Phase::prepared,
        "distortion requires an existing26Ch owner and prepared persistent block");
    auto& a=block.acquired_;auto& c=*block.context_;auto& p=c.post_effects;
    a.receiver=owner;block.phase_=NativeDistortionInitializationBlock::Phase::executing;
    try {
        if(!detail::initialize_native_distortion_texture_prefix(owner,width,height,c.textures,a.textures,a.native_state)) {
            a.capability_failure=true;block.settle();return false;
        }
        block.create_post(0,0x1c,c.damp_name_00d61fcc,18,2,3);
        block.parameter(0x1c,0x24,c.sample_offset_name_00d61fbc,13,4,5);
        block.parameter(0x1c,0x40,c.sample_offsets_name_00d5e40c,14,64,6);
        block.parameter(0x1c,0x140,c.sample_weights_name_00d61fac,14,64,7);
        block.parameter(0x1c,0x244,c.bump_fade_name_00d61f9c,15,1,8);
        block.create_post(1,0x20,c.bump_to_disp_name_00d61f80,26,9,10);
        block.parameter(0x20,0x24,c.sample_offset_name_00d61fbc,13,2,12);
        block.parameter(0x20,0x2c,c.texel_offset_name_00d61f70,12,2,13);
        block.parameter(0x20,0x248,c.bump_height_name_00d61f64,11,1,14);
        block.parameter(0x20,0x24c,c.refraction_name_00d61f50,16,1,15);
        auto* const surface=native_render_holder_primary_00b4cb20(pointer(word(owner,0x18)));
        set_native_post_effect_color0_00b4cb70(pointer(word(owner,0x20)),surface,p.destruction.frame_targets);
        a.raw_scene=allocate(0x24);a.native_state=16;
        if(a.raw_scene) {
            block.build_name(block.effect_name_,c.scene_name_00d61f40,12);a.name_mask|=4;a.native_state=17;
            a.scene=construct_native_gui_scene_00b724e0(a.raw_scene,c.scenes,block.effect_name_);
            block.scene_publication_=a.scene;
        }
        put(owner,0x3c,a.scene?reinterpret_cast<Word>(&a.scene->storage):0);a.scene_published=true;a.native_state=-1;
        if(a.name_mask&4u){a.name_mask&=~4u;block.return_name(block.effect_name_);}
        a.raw_frame=allocate(0x40);a.native_state=19;
        if(a.raw_frame)a.frame=construct_native_frame_target_owner_00b1fbb0(a.raw_frame);
        a.native_state=-1;put(owner,0x38,reinterpret_cast<Word>(a.frame));a.frame_published=true;
        a.raw_camera=p.cameras.pool_0108ffb0.allocate_raw_slot_00b71770();a.native_state=20;
        void* camera=nullptr;
        if(a.raw_camera) {
            block.camera_owner_.emplace(a.raw_camera,NativeCameraPool::slot_bytes,p.cameras,std::move(block.camera_scene_));
            a.camera_owner=&*block.camera_owner_;
            block.build_name(block.effect_name_,c.camera_name_00d61f30,13);a.name_mask|=8;a.native_state=21;
            camera=construct_native_camera_00b71a80(*block.camera_owner_,&block.effect_name_,p.node_constants,std::move(block.viewport_admissions_[0]));
            a.camera_native_completed=true;
        }
        put(owner,0x34,reinterpret_cast<Word>(camera));a.camera_published=true;a.native_state=-1;
        if(camera) {
            block.camera_reference_.emplace(*block.camera_owner_,NativeCameraCompanionDisposal{&block,&NativeDistortionInitializationBlock::retire_camera},std::move(block.camera_lifetime_));
            a.camera_reference=&*block.camera_reference_;
        }
        if(a.name_mask&8u){a.name_mask&=~8u;block.return_name(block.effect_name_);}
        a.raw_viewport=allocate(0x34);a.native_state=23;
        if(a.raw_viewport)a.viewport=initialize_native_viewport_owner_00b1f850(a.raw_viewport,p.cameras.viewport);
        a.native_state=-1;
        if(a.viewport)p.viewports.constructed(block.viewport_admissions_[1],*a.viewport);
        require(a.viewport!=nullptr,"distortion requires its returned actual viewport");
        const Word dimensions[2]={a.textures.width,a.textures.height};
        set_native_viewport_dimensions_00b1f940(*a.viewport,dimensions);
        void* const current_camera=pointer(word(owner,0x34));
        auto* const reference=dynamic_cast<NativeCameraReference*>(p.cameras.nodes.attachments.find_actual_node(reinterpret_cast<Word>(current_camera)));
        require(reference && &reference->camera_owner().storage.node==current_camera,"distortion requires its current canonical camera");
        set_native_camera_viewport_00b71990(reference->camera_owner(),a.viewport);
        a.viewport_creator_release_started=true;
        if(p.destruction.actual_decrement_00ce2220(&a.viewport->references_04)==0) {
            require(word(a.viewport)==0xd5e5f8 && p.viewport_profile_00d5e5f8[0]==0xbd30e0 &&
                p.viewport_profile_00d5e5f8[1]==0xb1f8f0,"distortion viewport requires current concrete deleting profile");
            invoke_native_viewport_deleting_destructor_00bd30e0(a.viewport);
        }
        block.create_post(2,0x240,c.passthrough_name_00d5e448,21,24,25,true);
        block.parameter(0x240,0x24,c.scene_color_offset_name_00d5e430,23,2,27,true);
        a.completed=true;block.settle();return true;
    } catch(...) {
        try {block.unwind();}catch(...){block.settle();throw;}
        block.settle();throw;
    }
}
} // namespace bsp
