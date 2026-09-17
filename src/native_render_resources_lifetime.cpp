#include "bsp/native_render_resources_lifetime.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <atomic>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource destruction requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word=std::uint32_t;
using Decrement=NativeRenderServiceTextureDecrement;
static_assert(sizeof(void*)==4 && sizeof(long)==4);
void* at(void* owner,Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(owner)+offset);
}
Word word(void* owner,Word offset=0) noexcept {
    return *static_cast<const volatile Word*>(at(owner,offset));
}
void put(void* owner,Word offset,Word value) noexcept {
    *static_cast<volatile Word*>(at(owner,offset))=value;
}
void* child(void* owner,Word offset) noexcept {return reinterpret_cast<void*>(word(owner,offset));}
void terminal(void* captured,NativeRenderResourcesLifetimeContext& c) {
    if(word(captured)==0x00d5e600u) {
        const auto* table=c.actual_frame_table_00d5e600;
        if(!table || table[0]!=0x00bd30e0u || word(captured)!=0x00d5e600u || table[1]!=0x00b1fcf0u)
            throw std::logic_error("unsupported current frame-target terminal");
        delete_native_frame_target_owner_00b1fcf0(*static_cast<NativeFrameTargetOwnerStorage*>(captured),1,c.frame_targets);
        return;
    }
    RenderCommandReference* reference;
    if(word(captured)==0x00d61854u) {
        auto* owner=c.construction.helper_owner();
        reference=c.construction.helper_reference();
        if(!owner || &owner->storage!=captured || !reference)
            throw std::logic_error("render resources require their canonical cockpit companion");
    } else reference=&c.textures.actual_owners.resolve_actual(captured);
    auto* count=std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(captured,4)));
    if(&reference->reference_count!=count)
        throw std::logic_error("render-resource child companion must borrow actual+04");
    // Existing companions check current slot0 and the refreshed deleting slot.
    // The decrement already happened; never decrement a second time here.
    reference->release_zero_references();
}
void release(void* service,Word offset,void* captured,Decrement decrement,
    NativeRenderResourcesLifetimeContext& c) {
    if(!captured)return;
    if(!decrement)throw std::logic_error("render-resource current CE2220 import is unbound");
    if(decrement(static_cast<volatile long*>(at(captured,4)))==0)terminal(captured,c);
    put(service,offset,0); // Overwrite any returning callback's parent-field write.
}
void unwind(void* service,int state,NativeRenderResourcesLifetimeContext& c) noexcept {
    try {
        while(state>=0) {
            switch(state--) {
            case 3: destroy_native_string_header_vector_004324a0(at(service,0x69c),c.strings);break;
            case 2: destroy_native_renderer_record_vector_thunk_00b14590(at(service,0x68c),c.strings);break;
            case 1: destroy_native_string_header_0041dd20(at(service,0x684),c.strings);break;
            case 0: destroy_native_render_service_base_00b0f0c0(service,c.base);break;
            }
        }
    } catch(...) {std::terminate();}
}
} // namespace

void release_native_render_resources_00b0f6e0(void* service,NativeRenderResourcesLifetimeContext& c) {
    release_native_render_service_texture_auxiliaries_00b52270(child(service,0x34),c.textures); // B0F6E9
    void* const first=child(service,0x50); // B0F6EE BEFORE current import capture.
    const auto decrement=c.textures.decrement_iat_00ce2220; // B0F6F1
    release(service,0x50,first,decrement,c);
    // Each row starts with its own current parent load, after the prior clear.
    // The repeated1D4 is native B0F912 and B0F956, not deduplicated metadata.
    constexpr Word fields[]={0x1d0,0x44,0x48,0x4c,0x2c,0x650,0x658,0x65c,0x664,
        0x18,0x20,0x24,0x28,0x38,0x3c,0x40,0x1cc,0x1d4,0x1c8,0x1d4,0x70,
        0x74,0x80,0x1c,0x60,0x64,0x6c,0x68,0x654,0x78,0x7c,0x30,0x660,
        0x10,0x66c,0x670,0x674,0x678,0x54,0x5c,0x58};
    for(Word offset:fields)release(service,offset,child(service,offset),decrement,c);
    *static_cast<volatile std::uint8_t*>(at(service,0x1c4))=0; // B0FBEF
}
void destroy_native_render_resources_00b14f60(void* service,NativeRenderResourcesLifetimeContext& c) {
    put(service,0,0x00d5e480u);
    int state=3;
    try {
        release_native_render_resources_00b0f6e0(service,c); // B14F8E
        void* const first=child(service,0x34); // B14F93
        const auto decrement=c.textures.decrement_iat_00ce2220; // B14F96, new epoch.
        release(service,0x34,first,decrement,c);
        release(service,0x668,child(service,0x668),decrement,c);
        release(service,0x0c,child(service,0x0c),decrement,c);
        void* const begin=child(service,0x6a0); // B14FF5, BEFORE state transition.
        state=2;
        if(begin) {
            void* const end=child(service,0x6a4);
            destroy_native_string_header_range_00432050(begin,end,c.strings);
            singleton_lifetime_free(child(service,0x6a0)); // CURRENT after callback.
        }
        put(service,0x6a0,0);put(service,0x6a4,0);put(service,0x6a8,0);
        state=1;
        destroy_native_renderer_record_vector_00b14500(at(service,0x68c),c.strings);
        void* const marker=child(service,0x688); // B1503D BEFORE state transition.
        state=0;
        if(marker) {
            const Word bytes=word(service,0x684)+1u;
            auto* const pool=native_string_pool_get_or_create_00419cc0(
                c.strings.actual_published_01090aa8,c.strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool,marker,bytes,c.strings.actual_small_returns_disabled_01090aa4);
        }
        state=-1;
        destroy_native_render_service_base_00b0f0c0(service,c.base);
    } catch(...) {unwind(service,state,c);throw;}
}
void* delete_native_render_resources_00b151c0(void* service,Word flags,NativeRenderResourcesLifetimeContext& c) {
    destroy_native_render_resources_00b14f60(service,c);
    if(flags&1u)singleton_lifetime_free(service);
    return service;
}
} // namespace bsp
