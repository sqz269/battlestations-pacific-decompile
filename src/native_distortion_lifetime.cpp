#include "bsp/native_distortion_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <cstring>

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
using RawContext=NativeDistortionStorageLifetimeContext;
using ClearAcquired=NativeDistortionStorageClearAcquired;
using LifetimeAcquired=NativeDistortionStorageLifetimeAcquired;
template<class Context> const volatile Word* table(void* child,Context& c) {
    switch(word(child)) {
    case 0xd61eb8:return c.effects.actual_holder_profile_00d61eb8;
    case 0xd61ec0:return c.effects.actual_post_effect_profile_00d61ec0;
    case 0xd5e600:return c.frame_profile_00d5e600;
    case 0xd62d48:return c.scene_profile_00d62d48;
    default:throw std::logic_error("distortion child has unsupported current profile");
    }
}
void release_scene(void* child,NativeDistortionLifetimeContext& c) {
    auto* const scene=c.scene_owner_3c;
    require(scene && &scene->storage==child && &scene->reference_count==static_cast<std::atomic<std::int32_t>*>(at(child,4)),
        "distortion requires its canonical concrete scene companion");
    scene->release_zero_references();
}
void release_scene(void* child,RawContext& c) {
    auto& canonical=c.trees.owners.resolve_actual(child);
    auto* const scene=dynamic_cast<NativeGuiSceneReference*>(&canonical);
    require(scene && &scene->reference_count==static_cast<std::atomic<std::int32_t>*>(at(child,4)),
        "raw distortion requires the same actual scene/count canonical companion");
    scene->release_zero_references();
}
template<class Context> void release_zero(void* child,Context& c) {
    if constexpr(std::is_same_v<Context,RawContext>)
        require(word(at(child,4))==0,"raw distortion terminal requires observed current actual count zero");
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
    case 0xb72580:release_scene(child,c);return;
    default:throw std::logic_error("distortion child has unsupported deleting slot");
    }
}
template<class Context> void release_slot(void* owner,Word offset,void* captured,
    NativeTextureSurfaceReferenceIncrement decrement,Context& c,ClearAcquired* a,
    Word decrement_site,Word terminal_site) {
    if(a){a->current_offset=offset;a->captured_child=captured;a->captured_decrement=decrement;}
    if(captured) {
        if(a)a->active_call_site=decrement_site;
        const long result=decrement(static_cast<volatile long*>(at(captured,4)));
        if(a)a->decrement_result=result;
        if(result==0){if(a)a->active_call_site=terminal_site;release_zero(captured,c);}
        put(at(owner,offset),0);
    }
}
void unlink_camera(void* camera,NativeDistortionLifetimeContext& c,ClearAcquired*) {
    auto* const canonical=c.nodes.attachments.find_actual_node(reinterpret_cast<Word>(camera));
    auto* const reference=dynamic_cast<NativeCameraReference*>(canonical);
    require(reference && &reference->camera_owner().storage.node==camera,
        "distortion requires its canonical concrete camera companion");
    unlink_and_release_render_model_00b6dfa0(*reference);
}
void unlink_camera(void* camera,RawContext& c,ClearAcquired* a) {
    unlink_native_node_tree_00b6dfa0(camera,c.trees,a->camera_unlink);
}
template<class Context> void clear_resources(void* owner,Context& c,ClearAcquired* a) {
    for(Word offset:{0x10u,0x14u}) {
        void* const captured=pointer(word(at(owner,offset)));
        if(captured)release_slot(owner,offset,captured,c.effects.actual_decrement_00ce2220,c,a,0xb4ee8a,0xb4ee9a);
    }
    void* child=pointer(word(at(owner,0x18)));
    const auto decrement=c.effects.actual_decrement_00ce2220;
    const Word offsets[]={0x18,0x1c,0x20,0x38};
    const Word calls[]={0xb4eebb,0xb4eedb,0xb4eefb,0xb4ef1b};
    for(Word i=0;i!=4;++i) {
        if(i)child=pointer(word(at(owner,offsets[i])));
        release_slot(owner,offsets[i],child,decrement,c,a,calls[i],calls[i]+12);
    }
    void* const camera=pointer(word(at(owner,0x34)));
    if(camera) {
        if(a){a->current_offset=0x34;a->captured_child=camera;a->active_call_site=0xb4ef37;}
        unlink_camera(camera,c,a);put(at(owner,0x34),0);
    }
    release_slot(owner,0x3c,pointer(word(at(owner,0x3c))),decrement,c,a,0xb4ef4e,0xb4ef5a);
    release_slot(owner,0x240,pointer(word(at(owner,0x240))),decrement,c,a,0xb4ef71,0xb4ef7d);
}
void admit_context(RawContext& c) {
    require(&c.effects.actual_post_effects==&c.trees.owners &&
        &c.effects.actual_decrement_00ce2220==&c.trees.decrement_00ce2220,
        "raw distortion requires the same actual-owner registry and decrement cell");
}
void prepare_record_lifetime(void* owner) noexcept {
    static_assert(std::is_aggregate_v<NativeDistortionRecordArray>);
    static_assert(std::is_trivially_copyable_v<NativeDistortionRecordArray>);
    static_assert(sizeof(NativeDistortionRecordArray)==12);
    unsigned char saved[sizeof(NativeDistortionRecordArray)];
    std::memcpy(saved,at(owner,0x254),sizeof saved);
    // memcpy implicitly creates the trivial header object while preserving
    // every preimage byte; it introduces no native descriptor initialization.
    std::memcpy(at(owner,0x254),saved,sizeof saved);
}
int cleanup_exception(unsigned long code) noexcept {
    if(code==0xe06d7363u)std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
template<class Context> void unwind_native_distortion(void* owner,Context& c,int state,LifetimeAcquired* a) noexcept {
    __try {
        if(a)a->cleanup_started=true;
        if(state==1) {
            if(a){a->native_eh_state=0;a->records_started=true;a->active_call_site=0xcbfc61;}
            destroy_native_distortion_records_00b4f0a0(*static_cast<NativeDistortionRecordArray*>(at(owner,0x254)));
            if(a)a->records_complete=true;
        }
        if(a){a->native_eh_state=-1;a->base_started=true;a->active_call_site=0xcbfc53;}
        destroy_native_render_effect_base_00b0f5e0(owner,c.effects);
        if(a){a->base_complete=true;a->cleanup_complete=true;}
    } __except(cleanup_exception(GetExceptionCode())) {__assume(0);}
}
template<class Context> struct Cleanup {
    void* owner;Context& context;int state;LifetimeAcquired* acquired;
    ~Cleanup() noexcept {if(state>=0)unwind_native_distortion(owner,context,state,acquired);}
};
template<class Context> void destroy_owner(void* owner,Context& c,LifetimeAcquired* a) {
    put(owner,0xd61f1c);Cleanup<Context> cleanup{owner,c,1,a};
    if(a){a->native_eh_state=1;a->active_call_site=0xb4f17c;a->clear.started=true;}
    clear_resources(owner,c,a?&a->clear:nullptr);
    if(a)a->clear.complete=true;
    auto& records=*static_cast<NativeDistortionRecordArray*>(at(owner,0x254));cleanup.state=0;
    if(a){a->native_eh_state=0;a->records_started=true;a->active_call_site=0xb4f190;}
    resize_native_distortion_records_00b4ee20(records,0);
    void* const backing=pointer(word(&records));
    if(a)a->active_call_site=0xb4f198;
    singleton_lifetime_free(backing);
    if(a)a->records_complete=true;
    cleanup.state=-1;
    if(a){a->native_eh_state=-1;a->base_started=true;a->active_call_site=0xb4f1aa;}
    destroy_native_render_effect_base_00b0f5e0(owner,c.effects);
    if(a){a->base_complete=true;a->complete=true;}
}
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
    clear_resources(owner,c,nullptr);
}
void destroy_native_distortion_owner_00b4f150(void* owner,NativeDistortionLifetimeContext& c) {
    destroy_owner(owner,c,nullptr);
}
void* delete_native_distortion_owner_00b4f540(void* owner,Word flags,NativeDistortionLifetimeContext& c) {
    destroy_native_distortion_owner_00b4f150(owner,c);
    if(flags&1u)singleton_lifetime_free(owner);
    return owner;
}
void clear_native_distortion_resources_00b4ee70(void* owner,RawContext& c,ClearAcquired& a) {
    require(!a.started && !a.camera_unlink.started,"raw distortion clear diagnostics must be fresh");
    admit_context(c);a.started=true;
    clear_resources(owner,c,&a);a.complete=true;
}
void destroy_native_distortion_owner_00b4f150(void* owner,RawContext& c,LifetimeAcquired& a) {
    require(!a.started && !a.clear.started && !a.clear.camera_unlink.started,
        "raw distortion destruction diagnostics must be fresh");
    admit_context(c);prepare_record_lifetime(owner);a.started=true;destroy_owner(owner,c,&a);
}
void* delete_native_distortion_owner_00b4f540(void* owner,const volatile Word& flags,
    RawContext& c,LifetimeAcquired& a) {
    destroy_native_distortion_owner_00b4f150(owner,c,a);
    a.complete=false;
    if(*reinterpret_cast<const volatile std::uint8_t*>(&flags)&1u) {
        a.active_call_site=0xb4f550;a.free_started=true;
        singleton_lifetime_free(owner);a.free_complete=true;
    }
    a.complete=true;return owner;
}
} // namespace bsp
