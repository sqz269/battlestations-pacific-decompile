#include "bsp/native_render_resources_construction.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_renderer_current_depth_surface.hpp"
#include "bsp/native_renderer_surface_save_publish.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* p,std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p)+n);
}
std::uint32_t word(const void* p) noexcept {
    return *static_cast<const volatile std::uint32_t*>(p);
}
void put(void* p,std::uint32_t n) noexcept { *static_cast<volatile std::uint32_t*>(p)=n; }
void byte(void* p,std::uint8_t n) noexcept { *static_cast<volatile std::uint8_t*>(p)=n; }
void* pointer(std::uint32_t n) noexcept { return reinterpret_cast<void*>(n); }
std::uint32_t bits(const void* p) noexcept { return reinterpret_cast<std::uintptr_t>(p); }

void* current_renderer(NativeRenderResourcesConstructionContext& c,
    std::uint32_t slot,std::uint32_t expected) {
    void* const renderer=const_cast<void*>(c.textures.cache.textures.current_renderer_00f8d394);
    if(word(renderer)!=0x00d5f0a8u || !c.textures.renderer_profile ||
        c.textures.renderer_profile[slot/4]!=expected)
        throw std::invalid_argument("unsupported current render-resource renderer dispatch");
    return renderer;
}

void unwind(NativeRenderResourcesConstructionContext& c,
    NativeRenderResourcesConstructionAcquired& a) {
    if(a.unwind_state>=4) {
        const int state=a.unwind_state;
        a.unwind_state=3;
        if(state==5 || state==6)
            destroy_native_string_header_0041dd20(a.native_local_14,c.textures.strings);
        else c.free_00bf65ac(pointer(word(a.native_local_14)));
    }
    while(a.unwind_state>=0) {
        switch(a.unwind_state--) {
        case 3: destroy_native_string_header_vector_004324a0(at(a.owner,0x69c),c.textures.strings);break;
        case 2: destroy_native_renderer_record_vector_thunk_00b14590(at(a.owner,0x68c),c.textures.strings);break;
        case 1: destroy_native_string_header_0041dd20(at(a.owner,0x684),c.textures.strings);break;
        case 0: destroy_native_render_service_base_00b0f0c0(a.owner,c.base);break;
        }
    }
}

void load_default_texture(NativeRenderResourcesConstructionContext& c,
    NativeRenderResourcesConstructionAcquired& a,const void* literal,
    std::uint32_t resize_site,std::uint32_t load_site,std::uint32_t return_site,
    int state,std::uint32_t destination,NativeTextureCacheAcquired& load) {
    auto* const name=a.native_local_14;
    put(name,0);put(at(name,4),0);
    a.native_site=resize_site;
    resize_native_string_header_0041dd40(name,c.textures.strings,9,true);
    void* const data=pointer(word(at(name,4)));
    if(data) std::memmove(data,literal,word(name)+1u);
    void* const renderer=current_renderer(c,0x64,0x00b319b0);
    a.unwind_state=state;
    a.native_site=load_site;
    void* const texture=load_native_renderer_texture_00b319b0(renderer,name,0,c.textures.cache,&load);
    put(at(a.owner,destination),bits(texture));
    void* const released=pointer(word(at(name,4)));
    a.unwind_state=3;
    if(released) {
        const auto size=word(name)+1u;
        a.native_site=return_site;
        auto& strings=c.textures.strings;
        auto* const pool=native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool,released,size,strings.actual_small_returns_disabled_01090aa4);
    }
}
} // namespace

void* construct_native_render_resources_00b14a10(void* owner,
    NativeRenderResourcesConstructionContext& c,NativeRenderResourcesConstructionAcquired& a) {
    if(a.phase!=NativeRenderResourcesConstructionAcquired::Phase::fresh ||
        a.black.phase!=NativeTextureCacheAcquired::Phase::not_started || a.black.wrapper_started ||
        a.noise.phase!=NativeTextureCacheAcquired::Phase::not_started || a.noise.wrapper_started ||
        a.texture_helper.phase!=NativeRenderServiceTextureConstructionAcquired::Phase::fresh)
        throw std::logic_error("render-resource construction requires a fresh persistent operation");
    if(!c.allocate_00bf681b || !c.free_00bf65ac ||
        &c.base.actual_manager_01090aa0!=&c.textures.strings.actual_manager_publication_01090aa0 ||
        c.frame_targets.actual_surface_context.actual_string_pool_00419cc0.actual_storage()!=&c.textures.cache.strings ||
        reinterpret_cast<const volatile void*>(&c.frame_targets.actual_surface_context.actual_renderer_00f8d394)!=
            reinterpret_cast<const volatile void*>(&c.textures.cache.textures.current_renderer_00f8d394))
        throw std::invalid_argument("render-resource construction requires one actual allocation/manager domain");
    for(const auto& load:a.texture_helper.loads)
        if(load.phase!=NativeTextureCacheAcquired::Phase::not_started || load.wrapper_started)
            throw std::logic_error("render-resource texture helper child already used");
    a.phase=NativeRenderResourcesConstructionAcquired::Phase::running;
    a.owner=owner;
    try {
        a.native_site=0x00b14a31;
        publish_native_render_service_base_00b0f020(owner,c.base);
        put(owner,0x00d5e480u);
        std::uint32_t x0=0,x1=0,x2=0,x3=0;
        x0 = c.constants.actual_00ce54a0; // 00B14A3C
        x1 = 0; // 00B14A44
        x2 = c.constants.actual_00ce3804; // 00B14A47
        x3 = c.constants.actual_00ce4bc4; // 00B14A4F
        put(at(owner,0x10),0); // 00B14A59
        put(at(owner,0x14),0); // 00B14A5C
        put(at(owner,0x84),x0); // 00B14A5F
        x0 = c.constants.actual_00ce74f8; // 00B14A67
        put(at(owner,0x88),x0); // 00B14A6F
        x0 = c.constants.actual_00ce8198; // 00B14A77
        put(at(owner,0x8c),x0); // 00B14A7F
        x0 = c.constants.actual_00ce6650; // 00B14A87
        put(at(owner,0x90),x0); // 00B14A8F
        x0 = c.constants.actual_00d7a24c; // 00B14A97
        put(at(owner,0x9c),x2); // 00B14A9F
        x2 = c.constants.actual_00ce3854; // 00B14AA7
        put(at(owner,0x94),x0); // 00B14AAF
        put(at(owner,0x98),x1); // 00B14AB7
        put(at(owner,0xa0),x2); // 00B14ABF
        x2 = c.constants.actual_00ce38b8; // 00B14AC7
        put(at(owner,0xa4),x2); // 00B14ACF
        x2 = c.constants.actual_00ce3800; // 00B14AD7
        put(at(owner,0xac),x0); // 00B14ADF
        byte(at(owner,0xa8),0); // 00B14AE7
        put(at(owner,0x230),x0); // 00B14AED
        put(at(owner,0x234),x2); // 00B14AF5
        x2 = c.constants.actual_00ce3d08; // 00B14AFD
        put(at(owner,0x23c),x3); // 00B14B05
        x3 = c.constants.actual_00d1f980; // 00B14B0D
        put(at(owner,0x238),x2); // 00B14B15
        put(at(owner,0x240),x2); // 00B14B1D
        put(at(owner,0x244),x1); // 00B14B25
        put(at(owner,0x248),x1); // 00B14B2D
        put(at(owner,0x24c),x1); // 00B14B35
        put(at(owner,0x250),x0); // 00B14B3D
        put(at(owner,0x254),x0); // 00B14B45
        put(at(owner,0x264),x1); // 00B14B4D
        put(at(owner,0x268),x0); // 00B14B55
        x0 = c.constants.actual_00ce3d34; // 00B14B5D
        put(at(owner,0x258),x3); // 00B14B65
        put(at(owner,0x25c),x2); // 00B14B6D
        put(at(owner,0x260),x2); // 00B14B75
        put(at(owner,0x294),x1); // 00B14B7D
        x1 = c.constants.actual_00ce89d0; // 00B14B85
        put(at(owner,0x28c),x0); // 00B14B8D
        x0 = c.constants.actual_00ce89cc; // 00B14B95
        put(at(owner,0x29c),x1); // 00B14B9D
        x1 = c.constants.actual_00ce89c8; // 00B14BA5
        put(at(owner,0x290),x0); // 00B14BAD
        x0 = c.constants.actual_00d7a238; // 00B14BB5
        put(at(owner,0x2a0),x1); // 00B14BBD
        x1 = c.constants.actual_00ce3d30; // 00B14BC5
        a.unwind_state = 0; // 00B14BD3
        put(at(owner,0x298),x0); // 00B14BD7
        put(at(owner,0x2a4),x1); // 00B14BDF
        put(at(owner,0x2a8),x0); // 00B14BE7
        a.native_site = 0x00b14bef; initialize_native_render_service_parameters_00b0cd80(at(owner,0x2ac),c.parameters); // 00B14BEF
        x0 = c.constants.actual_00ce7628; // 00B14BF4
        put(at(owner,0x314),x0); // 00B14BFC
        x0 = c.constants.actual_00ce6a04; // 00B14C04
        put(at(owner,0x318),0); // 00B14C0C
        put(at(owner,0x59c),x0); // 00B14C18
        put(at(owner,0x5a0),x0); // 00B14C20
        put(at(owner,0x5a4),x0); // 00B14C28
        put(at(owner,0x684),0); // 00B14C30
        put(at(owner,0x688),0); // 00B14C32
        put(at(owner,0x690),0); // 00B14C35
        put(at(owner,0x694),0); // 00B14C3B
        put(at(owner,0x698),0); // 00B14C41
        put(at(owner,0x6a0),0); // 00B14C47
        put(at(owner,0x6a4),0); // 00B14C4D
        put(at(owner,0x6a8),0); // 00B14C53
        x0 = 0; // 00B14C59
        put(at(owner,0x224),x0); // 00B14C5C
        x0 = c.constants.actual_00d7a24c; // 00B14C64
        a.unwind_state = 3; // 00B14C6E
        put(at(owner,0x50),0); // 00B14C73
        byte(at(owner,0x218),0); // 00B14C76
        byte(at(owner,0x219),0); // 00B14C7C
        byte(at(owner,0x21a),0); // 00B14C82
        byte(at(owner,0x21b),0); // 00B14C88
        byte(at(owner,0x21c),0); // 00B14C8E
        put(at(owner,0x220),1); // 00B14C94
        put(at(owner,0x228),x0); // 00B14C9E
        byte(at(owner,0x680),0); // 00B14CA6
        byte(at(owner,0x681),0); // 00B14CAC
        byte(at(owner,0x1c4),0); // 00B14CB2
        byte(at(owner,0xb0),0); // 00B14CB8
        put(at(owner,0x44),0); // 00B14CBE
        put(at(owner,0x48),0); // 00B14CC1
        put(at(owner,0x18),0); // 00B14CC4
        put(at(owner,0x20),0); // 00B14CC7
        put(at(owner,0x24),0); // 00B14CCA
        put(at(owner,0x28),0); // 00B14CCD
        put(at(owner,0x38),0); // 00B14CD0
        put(at(owner,0x3c),0); // 00B14CD3
        put(at(owner,0x40),0); // 00B14CD6
        put(at(owner,0x1cc),0); // 00B14CD9
        put(at(owner,0x1d4),0); // 00B14CDF
        put(at(owner,0x1c8),0); // 00B14CE5
        put(at(owner,0x650),0); // 00B14CEB
        put(at(owner,0x658),0); // 00B14CF1
        put(at(owner,0x65c),0); // 00B14CF7
        put(at(owner,0x664),0); // 00B14CFD
        put(at(owner,0x4c),0); // 00B14D03
        put(at(owner,0x54),0); // 00B14D06
        put(at(owner,0x58),0); // 00B14D09
        put(at(owner,0x5c),0); // 00B14D0C
        put(at(owner,0x2c),0); // 00B14D0F
        put(at(owner,0x1d0),0); // 00B14D12
        put(at(owner,0x74),0); // 00B14D18
        put(at(owner,0x80),0); // 00B14D1B
        put(at(owner,0x654),0); // 00B14D21
        put(at(owner,0x7c),0); // 00B14D27
        put(at(owner,0x78),0); // 00B14D2A
        put(at(owner,0x30),0); // 00B14D2D
        put(at(owner,0x668),0); // 00B14D30
        put(at(owner,0x67c),0); // 00B14D36
        put(at(owner,0x66c),0); // 00B14D3C
        put(at(owner,0x670),0); // 00B14D42
        put(at(owner,0x674),0); // 00B14D48
        put(at(owner,0x678),0); // 00B14D4E
        put(at(owner,0x1c),0); // 00B14D54
        put(at(owner,0x60),0); // 00B14D57
        put(at(owner,0x64),0); // 00B14D5A
        put(at(owner,0x6c),0); // 00B14D5D
        put(at(owner,0x68),0); // 00B14D60
        put(at(owner,0x660),0); // 00B14D63
        put(at(owner,0xc),0); // 00B14D69
        put(at(owner,0x1c0),0); // 00B14D6C
        a.native_site=0x00b14d72;
        void* allocation=c.allocate_00bf681b(0x40);
        put(a.native_local_14,bits(allocation));
        a.unwind_state=4;
        a.native_site=0x00b14d89;
        void* frame=allocation ? construct_native_frame_target_owner_00b1fbb0(allocation) : nullptr;
        put(at(owner,0x1d4),bits(frame));
        void* renderer=current_renderer(c,0x128,0x00b24dc0);
        a.unwind_state=3;
        a.native_site=0x00b14dac;
        void* surface=native_renderer_field197c_00b24dc0(renderer,nullptr,0);
        a.native_site=0x00b14db6;
        set_native_frame_target_color_00b1fab0(
            *static_cast<NativeFrameTargetOwnerStorage*>(pointer(word(at(owner,0x1d4)))),
            0,static_cast<NativeSurfaceOwnerStorage*>(surface),c.frame_targets);
        renderer=current_renderer(c,0x12c,0x00b20090);
        a.native_site=0x00b14dc9;
        surface=get_native_renderer_current_depth_surface_00b20090(renderer);
        a.native_site=0x00b14dd2;
        set_native_frame_target_depth_00b1fb00(
            *static_cast<NativeFrameTargetOwnerStorage*>(pointer(word(at(owner,0x1d4)))),
            static_cast<NativeSurfaceOwnerStorage*>(surface),c.frame_targets);
        load_default_texture(c,a,c.black_00d5e474,0x00b14de7,0x00b14e20,0x00b14e40,
            5,0x668,a.black);
        load_default_texture(c,a,c.noise_00d5e468,0x00b14e5c,0x00b14e95,0x00b14eb5,
            6,0x67c,a.noise);
        a.native_site=0x00b14ec6;
        allocation=c.allocate_00bf681b(0xcc);
        put(a.native_local_14,bits(allocation));
        a.unwind_state=7;
        a.native_site=0x00b14edd;
        void* helper=allocation ? construct_native_render_service_textures_00b52550(
            allocation,c.textures,a.texture_helper) : nullptr;
        a.unwind_state=3;
        put(at(owner,0x34),bits(helper));
        a.native_site=0x00b14ef3;
        resize_native_string_header_0041dd40(at(owner,0x684),c.textures.strings,4,false);
        void* const marker=pointer(word(at(owner,0x688)));
        if(marker) std::memmove(marker,c.marker_00d5e460,word(at(owner,0x684)));
        a.native_site=0x00b14f12;
        allocation=c.allocate_00bf681b(0x24);
        put(a.native_local_14,bits(allocation));
        a.unwind_state=8;
        a.native_site=0x00b14f29;
        helper=allocation ? construct_native_cockpit_helper_00b3c800(allocation,0x24,
            c.cockpit_near_00d7a2f0,c.constants.actual_00ce38b8,c.cockpit_release,
            std::move(c.cockpit_admission)) : nullptr;
        put(at(owner,0xc),bits(helper));
        a.unwind_state=-1;
        a.phase=NativeRenderResourcesConstructionAcquired::Phase::complete;
        return owner;
    } catch(...) {
        a.failure_site=a.native_site;
        a.phase=NativeRenderResourcesConstructionAcquired::Phase::failed;
        try {unwind(c,a);} catch(...) {std::terminate();}
        throw;
    }
}
} // namespace bsp
