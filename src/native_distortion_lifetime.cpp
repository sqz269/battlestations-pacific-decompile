#include "bsp/native_distortion_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native distortion lifetime requires MSVC Win32.
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
void put(void* p,Word value) noexcept {*static_cast<volatile Word*>(p)=value;}
void* pointer(Word bits) noexcept {return reinterpret_cast<void*>(bits);}
std::int32_t signed_word(const volatile void* p) noexcept {return static_cast<std::int32_t>(word(p));}
void require(bool value,const char* why) {if(!value)throw std::logic_error(why);}
const volatile Word* table(void* child,NativeDistortionLifetimeContext& c) {
    switch(word(child)) {
    case 0xd61eb8:return c.effects.actual_holder_profile_00d61eb8;
    case 0xd61ec0:return c.effects.actual_post_effect_profile_00d61ec0;
    case 0xd5e600:return c.frame_profile_00d5e600;
    case 0xd62d48:return c.scene_profile_00d62d48;
    default:throw std::logic_error("distortion child has unsupported current profile");
    }
}
void release_zero(void* child,NativeDistortionLifetimeContext& c) {
    const volatile Word* first=table(child,c);
    require(first && first[0]==0xbd30e0,"distortion child requires actual BD30E0");
    const Word terminal=table(child,c)[1];
    switch(terminal) {
    case 0xb4e410:delete_native_render_texture_surface_owner_00b4e410(child,1,c.effects.texture_holders);return;
    case 0xb1fcf0:delete_native_frame_target_owner_00b1fcf0(*static_cast<NativeFrameTargetOwnerStorage*>(child),1,c.frames);return;
    case 0xb4e430: {
        auto& canonical=c.effects.actual_post_effects.resolve_actual(child);
        auto* const post=dynamic_cast<NativePostEffect20Reference*>(&canonical);
        require(post && post->storage()==child && &post->reference_count==static_cast<std::atomic<std::int32_t>*>(at(child,4)),
            "distortion requires canonical post20 companion");
        post->release_zero_references();return;
    }
    case 0xb72580: {
        auto* const scene=c.scene_owner_3c;
        require(scene && &scene->storage==child && &scene->reference_count==static_cast<std::atomic<std::int32_t>*>(at(child,4)),
            "distortion requires its canonical concrete scene companion");
        scene->release_zero_references();return;
    }
    default:throw std::logic_error("distortion child has unsupported deleting slot");
    }
}
void release_slot(void* owner,Word offset,void* captured,NativeTextureSurfaceReferenceIncrement decrement,
    NativeDistortionLifetimeContext& c) {
    if(captured) {
        if(decrement(static_cast<volatile long*>(at(captured,4)))==0)release_zero(captured,c);
        put(at(owner,offset),0);
    }
}
int cleanup_exception(unsigned long code) noexcept {
    if(code==0xe06d7363u)std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_native_distortion(void* owner,NativeDistortionLifetimeContext& c,int state) noexcept {
    __try {
        if(state==1)destroy_native_distortion_records_00b4f0a0(*static_cast<NativeDistortionRecordArray*>(at(owner,0x254)));
        destroy_native_render_effect_base_00b0f5e0(owner,c.effects);
    } __except(cleanup_exception(GetExceptionCode())) {__assume(0);}
}
struct Cleanup {
    void* owner;NativeDistortionLifetimeContext& context;int state{1};
    ~Cleanup() noexcept {if(state>=0)unwind_native_distortion(owner,context,state);}
};
} // namespace

void reserve_native_distortion_records_00b4ed70(NativeDistortionRecordArray& records,std::int32_t requested) {
    if(requested<1)requested=1;
    if(signed_word(at(&records,8))>=requested)return;
    const Word bytes=static_cast<Word>(requested)*8u;
    void* const raw=singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes});
    Word index=0;
    if(signed_word(at(&records,4))>0) {
        void* destination=raw;
        do {
            if(destination) {
                void* const source=pointer(word(&records));
                put(destination,word(at(source,index*8u)));
                put(at(destination,4),word(at(source,index*8u+4u)));
            }
            ++index;destination=at(destination,8);
        } while(static_cast<std::int32_t>(index)<signed_word(at(&records,4)));
    }
    singleton_lifetime_free(pointer(word(&records)));
    put(&records,reinterpret_cast<Word>(raw));put(at(&records,8),static_cast<Word>(requested));
}
void resize_native_distortion_records_00b4ee20(NativeDistortionRecordArray& records,std::int32_t requested) {
    if(requested>signed_word(at(&records,8)))reserve_native_distortion_records_00b4ed70(records,requested);
    Word index=word(at(&records,4));
    while(static_cast<std::int32_t>(index)<requested) {
        void* const destination=at(pointer(word(&records)),index*8u);
        if(destination){put(destination,0);put(at(destination,4),0);}
        ++index;
    }
    while(requested<signed_word(at(&records,4)))put(at(&records,4),word(at(&records,4))-1u);
    put(at(&records,4),static_cast<Word>(requested));
}
void destroy_native_distortion_records_00b4f0a0(NativeDistortionRecordArray& records) {
    resize_native_distortion_records_00b4ee20(records,0);
    singleton_lifetime_free(pointer(word(&records)));
}
void* construct_native_distortion_owner_00b4f0c0(void* owner,const NativeDistortionOwnerConstants& c) {
    const Word first=c.bits_00cf4848;
    put(owner,0xceb130);::new(at(owner,4)) std::atomic<std::int32_t>(1);
    put(at(owner,8),0);put(at(owner,0xc),0);put(owner,0xd61f1c);put(at(owner,0x244),first);
    put(at(owner,0x248),c.bits_00ce3958);put(at(owner,0x24c),c.bits_00d1f3c4);
    for(Word offset:{0x254u,0x258u,0x25cu,0x10u,0x14u,0x18u,0x1cu,0x20u,0x240u})put(at(owner,offset),0);
    *static_cast<volatile std::uint8_t*>(at(owner,0x250))=0;
    *static_cast<volatile std::uint8_t*>(at(owner,0x251))=0;
    return owner;
}
void clear_native_distortion_resources_00b4ee70(void* owner,NativeDistortionLifetimeContext& c) {
    for(Word offset:{0x10u,0x14u}) {
        void* const captured=pointer(word(at(owner,offset)));
        if(captured)release_slot(owner,offset,captured,c.effects.actual_decrement_00ce2220,c);
    }
    void* child=pointer(word(at(owner,0x18)));
    const auto decrement=c.effects.actual_decrement_00ce2220;
    for(Word offset:{0x18u,0x1cu,0x20u,0x38u}) {
        if(offset!=0x18)child=pointer(word(at(owner,offset)));
        release_slot(owner,offset,child,decrement,c);
    }
    void* const camera=pointer(word(at(owner,0x34)));
    if(camera) {
        auto* const canonical=c.nodes.attachments.find_actual_node(reinterpret_cast<Word>(camera));
        auto* const reference=dynamic_cast<NativeCameraReference*>(canonical);
        require(reference && &reference->camera_owner().storage.node==camera,
            "distortion requires its canonical concrete camera companion");
        unlink_and_release_render_model_00b6dfa0(*reference);put(at(owner,0x34),0);
    }
    release_slot(owner,0x3c,pointer(word(at(owner,0x3c))),decrement,c);
    release_slot(owner,0x240,pointer(word(at(owner,0x240))),decrement,c);
}
void destroy_native_distortion_owner_00b4f150(void* owner,NativeDistortionLifetimeContext& c) {
    put(owner,0xd61f1c);Cleanup cleanup{owner,c};
    clear_native_distortion_resources_00b4ee70(owner,c);
    auto& records=*static_cast<NativeDistortionRecordArray*>(at(owner,0x254));cleanup.state=0;
    resize_native_distortion_records_00b4ee20(records,0);
    singleton_lifetime_free(pointer(word(&records)));
    cleanup.state=-1;destroy_native_render_effect_base_00b0f5e0(owner,c.effects);
}
void* delete_native_distortion_owner_00b4f540(void* owner,Word flags,NativeDistortionLifetimeContext& c) {
    destroy_native_distortion_owner_00b4f150(owner,c);
    if(flags&1u)singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
