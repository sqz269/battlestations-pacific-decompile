#include "bsp/native_render_effect_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render effect lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
void* at(const void* p,Word offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+offset);
}
Word word(const volatile void* p) noexcept {
    Word result;
    __asm { mov eax,p }
    __asm { mov eax,[eax] }
    __asm { mov result,eax }
    return result;
}
void put(void* p,Word value) noexcept { *static_cast<volatile Word*>(p)=value; }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void require(bool condition,const char* reason) {
    if(!condition)throw std::logic_error(reason);
}
const volatile Word* table(void* owner,NativeRenderEffectLifetimeContext& c) {
    switch(word(owner)) {
    case 0x00d61ec0:return c.actual_post_effect_profile_00d61ec0;
    case 0x00d61eb8:return c.actual_holder_profile_00d61eb8;
    default:throw std::logic_error("effect member has no admitted actual profile");
    }
}
void release_zero(void* owner,NativeRenderEffectLifetimeContext& c) {
    require(table(owner,c)[0]==0x00bd30e0,"effect member virtual0 is not recovered BD30E0");
    // The real invoker reloads current profile and its deleting slot.
    const Word terminal=table(owner,c)[1];
    if(terminal==0x00b4e410) {
        delete_native_render_texture_surface_owner_00b4e410(owner,1,c.texture_holders);
        return;
    }
    require(terminal==0x00b4e430,"effect member deleting slot is not recovered");
    auto& reference=c.actual_post_effects.resolve_actual(owner);
    auto* const actual=dynamic_cast<NativePostEffect20Reference*>(&reference);
    require(actual && actual->storage()==owner &&
        &actual->reference_count==static_cast<std::atomic<std::int32_t>*>(at(owner,4)),
        "post-effect member requires its canonical concrete reference");
    actual->release_zero_references();
    // The terminal may retire native storage and both host companions.
}
void release_slot(void* owner,Word offset,void* captured,
    NativeTextureSurfaceReferenceIncrement decrement,NativeRenderEffectLifetimeContext& c) {
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
    __try { destroy_native_render_effect_base_00b0f5e0(owner,c); }
    __except(cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct Cleanup {
    void* owner;
    NativeRenderEffectLifetimeContext& context;
    bool derived;
    bool armed{true};
    ~Cleanup() noexcept {
        if(armed) {
            if(derived)unwind_base(owner,context);
            else destroy_native_ref_counted_base_00bd30f0(owner);
        }
    }
};
void* delete_base(void* owner,Word flags,NativeRenderEffectLifetimeContext& c) {
    destroy_native_render_effect_base_00b0f5e0(owner,c);
    if((flags&1u)!=0)singleton_lifetime_free(owner);
    return owner;
}
} // namespace

void destroy_native_render_effect_base_00b0f5e0(void* owner,NativeRenderEffectLifetimeContext& c) {
    put(owner,0x00d5e140);
    void* const first=pointer(word(at(owner,8)));
    const auto decrement=c.actual_decrement_00ce2220;
    Cleanup cleanup{owner,c,false};
    release_slot(owner,8,first,decrement,c);
    release_slot(owner,0xc,pointer(word(at(owner,0xc))),decrement,c);
    cleanup.armed=false;destroy_native_ref_counted_base_00bd30f0(owner);
}

void* delete_native_depth_downscale_pass_00b10120(void* p,Word f,NativeRenderEffectLifetimeContext& c) {return delete_base(p,f,c);}
void* delete_native_particle_blend_pass_00b10140(void* p,Word f,NativeRenderEffectLifetimeContext& c) {return delete_base(p,f,c);}
void* delete_native_downscale4x4_pass_00b10160(void* p,Word f,NativeRenderEffectLifetimeContext& c) {return delete_base(p,f,c);}
void* delete_native_downscale2x2_pass_00b10180(void* p,Word f,NativeRenderEffectLifetimeContext& c) {return delete_base(p,f,c);}

void* __fastcall construct_native_bloom_owner_00b54e70(void* owner) noexcept {
    put(owner,0x00ceb130);::new(at(owner,4)) std::atomic<std::int32_t>(1);
    put(at(owner,0xc),0);put(owner,0x00d62150);
    put(at(owner,0x18),0);put(at(owner,0x1c),0);put(at(owner,0x20),0);put(at(owner,0x24),0);
    put(at(owner,8),0);return owner;
}

void destroy_native_bloom_owner_00b54ea0(void* owner,NativeRenderEffectLifetimeContext& c) {
    put(owner,0x00d62150);Cleanup cleanup{owner,c,true};
    for(Word offset:{0x18u,0x1cu,0x20u,0x24u,8u}) {
        void* const captured=pointer(word(at(owner,offset)));
        if(captured)release_slot(owner,offset,captured,c.actual_decrement_00ce2220,c);
    }
    cleanup.armed=false;destroy_native_render_effect_base_00b0f5e0(owner,c);
}

void* delete_native_bloom_owner_00b54f70(void* owner,Word flags,NativeRenderEffectLifetimeContext& c) {
    destroy_native_bloom_owner_00b54ea0(owner,c);
    if((flags&1u)!=0)singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
