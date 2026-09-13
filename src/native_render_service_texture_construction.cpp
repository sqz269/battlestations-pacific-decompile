#include "bsp/native_render_service_texture_construction.hpp"
#include "bsp/native_material_effect_runtime.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_render_service_texture_vectors.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_texture_saved_dimensions.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*)==4,"Native pointer words require Win32");
void* at(void* base,std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base)+offset);
}
std::uint32_t word(const void* p) noexcept {
    return *static_cast<const volatile std::uint32_t*>(p);
}
void put(void* p,std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(p)=value;
}
void* pointer(std::uint32_t value) noexcept {return reinterpret_cast<void*>(value);}
std::uint32_t pointer_word(const void* p) noexcept {return reinterpret_cast<std::uintptr_t>(p);}
void* name_slot(NativeRenderServiceTextureConstructionAcquired& a,unsigned index) noexcept {
    constexpr unsigned offsets[4]={0,0xc,0x14,0x1c};
    return a.native_locals_10_33+offsets[index];
}
void initialize_name(void* name) noexcept {put(name,0);put(at(name,4),0);}

__declspec(naked) std::uint32_t __fastcall unsigned_divide(std::uint32_t,std::uint32_t) {
    __asm {
        mov eax,ecx
        mov ecx,edx
        xor edx,edx
        div ecx
        ret
    }
}

void unwind(NativeRenderServiceTextureConstructionContext& c,
    NativeRenderServiceTextureConstructionAcquired& a) {
    if(a.unwind_state>=5) {
        void* const name=name_slot(a,static_cast<unsigned>(a.unwind_state-5));
        a.unwind_state=4;
        destroy_native_string_header_0041dd20(name,c.strings);
    }
    while(a.unwind_state>=0) {
        const int state=a.unwind_state--;
        switch(state) {
        case 4: destroy_native_texture_vector24_00b523c0(at(a.owner,0xa4));break;
        case 3: destroy_native_texture_vector12_00b523e0(at(a.owner,0x8c));break;
        case 2: destroy_native_texture_vector24_00b523c0(at(a.owner,0x7c));break;
        case 1: destroy_native_texture_vector24_00b523c0(at(a.owner,0x24));break;
        case 0: destroy_native_ref_counted_base_00bd30f0(a.owner);break;
        }
    }
}
struct Cleanup {
    NativeRenderServiceTextureConstructionContext& context;
    NativeRenderServiceTextureConstructionAcquired& acquired;
    ~Cleanup() noexcept {
        if(acquired.unwind_state<0)return;
        acquired.phase=NativeRenderServiceTextureConstructionAcquired::Phase::failed;
        try {unwind(context,acquired);} catch(...) {std::terminate();}
    }
};

using DimensionCall=std::uint32_t (__fastcall *)(const void*) noexcept;
DimensionCall capture_dimension(void* receiver,unsigned byte_slot,
    NativeRenderServiceTextureConstructionContext& c) {
    const auto current_profile=word(receiver);
    if(current_profile!=0x00d61948u||!c.texture_profile)
        throw std::invalid_argument("unsupported current render-service texture profile");
    const auto current_target=c.texture_profile[byte_slot/4];
    if(current_target==0x00b3ce70u)return &get_native_texture_saved_width_00b3ce70;
    if(current_target==0x00b3ce80u)return &get_native_texture_saved_height_00b3ce80;
    throw std::invalid_argument("unsupported current render-service texture dimension slot");
}

void resize_copy_load(unsigned index,std::uint32_t length,std::uint32_t resize_site,
    std::uint32_t load_site,std::uint32_t owner_offset,
    NativeRenderServiceTextureConstructionContext& c,
    NativeRenderServiceTextureConstructionAcquired& a) {
    auto* const name=name_slot(a,index);
    a.native_site=resize_site;
    resize_native_string_header_0041dd40(name,c.strings,length,true);
    auto* const captured_data=pointer(word(at(name,4)));
    if(captured_data) {
        const auto current_count=word(name)+1u;
        std::memmove(captured_data,c.literal_storage[index],current_count);
    }
    void* const renderer=const_cast<void*>(c.cache.textures.current_renderer_00f8d394);
    const auto current_profile=word(renderer);
    if(current_profile!=0x00d5f0a8u||!c.renderer_profile)
        throw std::invalid_argument("unsupported current texture-loading renderer profile");
    const auto current_target=c.renderer_profile[0x64/4];
    a.unwind_state=5+static_cast<int>(index);
    a.native_site=load_site;
    if(current_target!=0x00b319b0u)
        throw std::invalid_argument("unsupported current renderer texture-loading slot");
    auto* const result=load_native_renderer_texture_00b319b0(renderer,name,0,c.cache,&a.loads[index]);
    put(at(a.owner,owner_offset),pointer_word(result));
    // Native captures the current pointer BEFORE disarming this temporary.
    void* const released=pointer(word(at(name,4)));
    a.unwind_state=4;
    if(released) {
        const auto size=word(name)+1u;
        auto* const pool=native_string_pool_get_or_create_00419cc0(
            c.strings.actual_published_01090aa8,c.strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool,released,size,c.strings.actual_small_returns_disabled_01090aa4);
    }
}
} // namespace

NativeRenderServiceTextureConstructionContext::NativeRenderServiceTextureConstructionContext(
    NativeTextureCacheContext& cache_input,NativeStringRawPoolContext& string_input,
    NativeVfsNameResolutionContext& names,const volatile std::uint32_t* renderer,
    const volatile std::uint32_t* texture,const void* kosz,const void* szor,
    const void* csikok,const void* splotch)
    :cache(cache_input),strings(string_input),renderer_profile(renderer),texture_profile(texture),
     literal_storage{kosz,szor,csikok,splotch} {
    bind_native_texture_vfs_name_resolution(cache.textures,names);
}

void* construct_native_render_service_textures_00b52550(void* owner,
    NativeRenderServiceTextureConstructionContext& c,
    NativeRenderServiceTextureConstructionAcquired& a) {
    if(a.phase!=NativeRenderServiceTextureConstructionAcquired::Phase::fresh)
        throw std::logic_error("render-service texture construction cannot replay");
    for(const auto& load:a.loads)
        if(load.phase!=NativeTextureCacheAcquired::Phase::not_started||load.wrapper_started)
            throw std::logic_error("render-service texture child already used");
    a.phase=NativeRenderServiceTextureConstructionAcquired::Phase::running;
    a.owner=owner;
    put(owner,0x00ceb130);                         // B5256D
    put(a.native_locals_10_33+8,pointer_word(owner)); // B52574
    put(at(owner,4),1);                           // B52578
    put(owner,0x00d62074);                         // B52581
    a.unwind_state=0;
    Cleanup cleanup{c,a};
    put(at(owner,0x24),0);put(at(owner,0x28),0);put(at(owner,0x2c),0);
    put(at(owner,0x7c),0);put(at(owner,0x80),0);put(at(owner,0x84),0);
    put(at(owner,0x8c),0);put(at(owner,0x90),0);put(at(owner,0x94),0);
    put(at(owner,0xa4),0);put(at(owner,0xa8),0);put(at(owner,0xac),0);
    a.unwind_state=4;
    initialize_name(name_slot(a,0));
    resize_copy_load(0,11,0x00b525dd,0x00b52616,0x18,c,a);

    void* texture=pointer(word(at(owner,0x18))); // Re-read AFTER string return.
    auto call=capture_dimension(texture,0x4c,c);
    a.native_site=0x00b5264b;
    auto height=call(texture);
    call=capture_dimension(texture,0x48,c);     // Capture BEFORE spill.
    put(a.native_locals_10_33,height);          // B52655, first released header.
    a.native_site=0x00b52659;
    auto width=call(texture);
    auto quotient=unsigned_divide(width,word(a.native_locals_10_33));
    put(at(owner,0x30),0);
    initialize_name(name_slot(a,1));
    put(at(owner,0x1c),quotient);
    resize_copy_load(1,11,0x00b52679,0x00b526b2,0x3c,c,a);

    texture=pointer(word(at(owner,0x3c)));
    call=capture_dimension(texture,0x4c,c);
    a.native_site=0x00b526e7;
    height=call(texture);
    call=capture_dimension(texture,0x48,c);
    put(a.native_locals_10_33,height);          // B526F1, SAME first header.
    a.native_site=0x00b526f5;
    width=call(texture);
    quotient=unsigned_divide(width,word(a.native_locals_10_33));
    put(at(owner,0x68),0);
    initialize_name(name_slot(a,2));
    put(at(owner,0x40),quotient);
    resize_copy_load(2,10,0x00b52715,0x00b5274e,0x70,c,a);

    texture=pointer(word(at(owner,0x70)));
    call=capture_dimension(texture,0x48,c);
    a.native_site=0x00b5277e;
    width=call(texture);
    put(at(owner,0x74),width>>1);
    put(at(owner,0x88),0);
    initialize_name(name_slot(a,3));
    resize_copy_load(3,11,0x00b5279b,0x00b527d4,0x98,c,a);

    texture=pointer(word(at(owner,0x98)));
    call=capture_dimension(texture,0x4c,c);
    a.native_site=0x00b5280e;
    height=call(texture);
    call=capture_dimension(texture,0x48,c);
    a.native_site=0x00b52819;
    width=call(texture);
    quotient=unsigned_divide(width,height);
    put(at(owner,0xb0),0);
    put(at(owner,0x9c),quotient);
    a.unwind_state=-1;
    a.phase=NativeRenderServiceTextureConstructionAcquired::Phase::complete;
    return owner;
}
} // namespace bsp
