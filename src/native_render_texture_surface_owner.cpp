#include "bsp/native_render_texture_surface_owner.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <atomic>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render texture surface owner requires MSVC Win32.
#endif

namespace bsp {
__declspec(naked) void __fastcall native_texture_noop_00b3d640(void*,void*,const void*) noexcept {
    __asm { ret 4 }
}
namespace {
using Word=std::uint32_t;
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p,Word offset=0) noexcept { return pointer(reinterpret_cast<Word>(p)+offset); }
Word word(const volatile void* p) noexcept {
    Word result;
    __asm { mov eax,p }
    __asm { mov eax,[eax] }
    __asm { mov result,eax }
    return result;
}
void put(void* p,Word value) noexcept { *static_cast<volatile Word*>(p)=value; }
std::uint8_t byte(const void* p) noexcept { return *static_cast<const volatile std::uint8_t*>(p); }
Word table_word(const volatile Word* table,Word offset) noexcept { return table[offset/4]; }
NativeTexture2DOwnerContext& owners(NativeRenderTextureSurfaceOwnerContext& c) noexcept {
    return c.textures.construction.owners;
}
const volatile Word* resource_table(void* owner,NativeRenderTextureSurfaceOwnerContext& c) noexcept {
    switch(word(owner)) {
    case 0x00d61948:return owners(c).renderer_notification.accounting_tables.texture_2d_00d61948;
    case 0x00d619a0:return owners(c).actual_surface_profile_00d619a0;
    default:__assume(0);
    }
}
void require_texture_method(void* texture,Word offset,Word entry,
    NativeRenderTextureSurfaceOwnerContext& c) noexcept {
    const Word profile=word(texture);__assume(profile==0x00d61948);
    const Word target=table_word(owners(c).renderer_notification.accounting_tables.texture_2d_00d61948,offset);
    __assume(target==entry);
}
void release_at_zero(void* owner,NativeRenderTextureSurfaceOwnerContext& c) {
    const Word invoker=table_word(resource_table(owner,c),0);__assume(invoker==0x00bd30e0);
    // The complete BD30E0 call reloads current profile and deleting slot.
    if(!owner)return;
    const Word terminal=table_word(resource_table(owner,c),4);
    if(terminal==0x00b3f5b0) {
        delete_native_surface_00b3f5b0(*static_cast<NativeSurfaceOwnerStorage*>(owner),1,owners(c).surfaces);
        return;
    }
    if(terminal==0x00b3f590) { delete_native_texture_2d_00b3f590(owner,1,owners(c));return; }
    __assume(0);
}
struct BaseCleanup {
    void* owner;
    int& state;
    ~BaseCleanup() noexcept {
        if(state==0) {state=-1;destroy_native_ref_counted_base_00bd30f0(owner);}
    }
};
struct ConstructionPhase {
    NativeRenderTextureSurfaceOwnerAcquired& acquired;
    ~ConstructionPhase() noexcept {
        if(acquired.phase!=NativeRenderTextureSurfaceOwnerAcquired::Phase::complete)
            acquired.phase=NativeRenderTextureSurfaceOwnerAcquired::Phase::failed;
    }
};
} // namespace

void* construct_native_render_texture_surface_owner_00b4e020(void* owner,
    const volatile NativeRenderTextureSurfaceOwnerArguments& args,
    NativeRenderTextureSurfaceOwnerContext& c,NativeRenderTextureSurfaceOwnerAcquired& a) {
    if(a.phase!=NativeRenderTextureSurfaceOwnerAcquired::Phase::fresh)
        throw std::invalid_argument("render texture surface owner frame must be fresh");
    a.phase=NativeRenderTextureSurfaceOwnerAcquired::Phase::running;a.owner=owner;
    ConstructionPhase phase{a};BaseCleanup cleanup{owner,a.unwind_state};
    put(owner,0x00ceb130);::new(at(owner,4)) std::atomic<std::int32_t>(1);
    const auto mode=static_cast<std::uint8_t>(args.mode);
    const Word format=args.format,height=args.height;
    *static_cast<volatile std::uint8_t*>(at(owner,0x14))=mode;
    const Word initial_width=args.width;
    put(owner,0x00d61eb8);put(at(owner,8),0);put(at(owner,0xc),0);put(at(owner,0x10),0);
    void* const first_renderer=const_cast<void*>(owners(c).renderer_notification.actual_renderer_00f8d394);
    const Word renderer_profile=word(first_renderer);__assume(renderer_profile==0x00d5f0a8);
    const Word factory_entry=table_word(owners(c).actual_renderer_profile_00d5f0a8,0x88);__assume(factory_entry==0x00b2a070);
    a.texture_arguments={initial_width,height,1,format,0x10};a.unwind_state=0;
    void* const texture=create_native_runtime_texture_2d_00b2a070(first_renderer,a.texture_arguments,c.textures,a.texture_creation);
    const bool separate=byte(at(owner,0x14))!=0;
    put(at(owner,8),reinterpret_cast<Word>(texture));require_texture_method(texture,0x30,0x00b3fd80,c);
    auto* const level=get_native_texture_surface_00b3fd80(texture,0,0,c.levels,a.surface_getter);
    if(!separate)put(at(owner,0xc),reinterpret_cast<Word>(level));
    else {
        put(at(owner,0x10),reinterpret_cast<Word>(level));
        auto* const external=args.external_surface;
        if(external) {
            void* const old=pointer(word(at(owner,0xc)));
            if(old!=external) {
                put(at(owner,0xc),reinterpret_cast<Word>(external));
                (void)c.levels.actual_increment_00ce221c(static_cast<volatile long*>(at(external,4)));
                if(old && c.actual_decrement_00ce2220(static_cast<volatile long*>(at(old,4)))==0)
                    release_at_zero(old,c);
            }
        } else {
            const Word multisample=args.multisample,width=args.width;
            void* const renderer=const_cast<void*>(owners(c).renderer_notification.actual_renderer_00f8d394);
            auto* const target=create_native_renderer_render_target_00b2a7c0(renderer,width,height,format,multisample,c.render_targets,a.target_creation);
            put(at(owner,0xc),reinterpret_cast<Word>(target));
        }
    }
    void* const current_texture=pointer(word(at(owner,8)));
    for(Word& component:a.zero_vector)put(&component,0);
    require_texture_method(current_texture,0x54,0x00b3d640,c);
    native_texture_noop_00b3d640(current_texture,nullptr,a.zero_vector);
    a.unwind_state=-1;a.phase=NativeRenderTextureSurfaceOwnerAcquired::Phase::complete;return owner;
}

void destroy_native_render_texture_surface_owner_00b4e140(void* owner,
    NativeRenderTextureSurfaceOwnerContext& c) {
    put(owner,0x00d61eb8);
    void* current=pointer(word(at(owner,0xc)));
    const auto decrement=c.actual_decrement_00ce2220;
    int state=0;BaseCleanup cleanup{owner,state};
    for(Word offset:{0xcu,0x10u,8u}) {
        if(offset!=0xc)current=pointer(word(at(owner,offset)));
        if(current) {
            if(decrement(static_cast<volatile long*>(at(current,4)))==0)release_at_zero(current,c);
            put(at(owner,offset),0);
        }
    }
    state=-1;destroy_native_ref_counted_base_00bd30f0(owner);
}

void* delete_native_render_texture_surface_owner_00b4e410(void* owner,Word flags,
    NativeRenderTextureSurfaceOwnerContext& c) {
    destroy_native_render_texture_surface_owner_00b4e140(owner,c);
    if((flags&1u)!=0)singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
