#include "bsp/native_texture_surface_getter.hpp"
#include "bsp/native_texture_surface_cache_storage.hpp"
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture surface getter requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p,Word offset=0) noexcept {
    return pointer(reinterpret_cast<Word>(p)+offset);
}
Word word(const void* p) noexcept {
    Word result;
    __asm { mov eax,p }
    __asm { mov eax,[eax] }
    __asm { mov result,eax }
    return result;
}
void put(void* p,Word value) noexcept { *static_cast<volatile Word*>(p)=value; }
template<class T> T method(void* p,Word offset) noexcept {
    return reinterpret_cast<T>(word(at(pointer(word(p)),offset)));
}
int cleanup_exception(unsigned long code) noexcept {
    if(code==0xe06d7363u)std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void return_slot(NativeTextureSurfaceGetterAcquired& a) noexcept {
    a.unwind_state=-1;
    __try {
        return_d3d9_surface_slot_00b3dcc0(a.raw_slot);
        a.raw_slot_returned=true;
    } __except(cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct Cleanup {
    NativeTextureSurfaceGetterAcquired& acquired;
    ~Cleanup() noexcept {
        if(acquired.phase!=NativeTextureSurfaceGetterAcquired::Phase::complete) {
            acquired.phase=NativeTextureSurfaceGetterAcquired::Phase::failed;
            if(acquired.unwind_state==0)return_slot(acquired);
        }
    }
};
} // namespace

NativeSurfaceOwnerStorage* get_native_texture_surface_00b3fd80(void* texture,
    Word mip,Word unused,NativeTextureSurfaceGetterContext& c,
    NativeTextureSurfaceGetterAcquired& a) {
    (void)unused;
    if(a.phase!=NativeTextureSurfaceGetterAcquired::Phase::fresh)
        throw std::invalid_argument("texture surface getter frame must be fresh");
    a.phase=NativeTextureSurfaceGetterAcquired::Phase::running;
    a.surface_output=reinterpret_cast<IDirect3DSurface9*>(mip);
    Cleanup cleanup{a};
    const auto increment=c.actual_increment_00ce221c;
    const auto count=static_cast<std::int32_t>(word(at(texture,0x44)));
    if(count>0) {
        void* const data=pointer(word(at(texture,0x40)));
        for(Word index=0;static_cast<std::int32_t>(index)<count;++index) {
            if(word(at(data,index*8u))==mip) {
                auto* const hit=static_cast<NativeSurfaceOwnerStorage*>(
                    pointer(word(at(data,index*8u+4u))));
                (void)increment(static_cast<volatile long*>(at(hit,4)));
                if(hit) {
                    a.owner=hit;a.phase=NativeTextureSurfaceGetterAcquired::Phase::complete;
                    return hit;
                }
                break;
            }
        }
    }
    void* const texture_com=pointer(word(at(texture,0x10)));
    using GetSurface=HRESULT (__stdcall*)(void*,UINT,IDirect3DSurface9**);
    a.get_surface_result=method<GetSurface>(texture_com,0x48)(texture_com,mip,
        const_cast<IDirect3DSurface9**>(&a.surface_output));
    a.raw_slot=c.surfaces.actual_surface_pool_0108db00.allocate_raw_slot_00b3ed40();
    a.owner=nullptr;a.unwind_state=0;
    if(a.raw_slot) {
        const Word flags=word(at(texture,0x1c));
        a.owner=construct_native_surface_00b3f630(a.raw_slot,a.surface_output,flags,
            static_cast<std::uint8_t>((flags>>8)&0xffffff01u),c.surfaces);
    }
    void* const surface=a.surface_output;
    using Release=ULONG (__stdcall*)(void*);
    const auto release=method<Release>(surface,8);
    a.unwind_state=-1;(void)release(surface);
    if((word(at(texture,0x1c))&1u)==0) {
        (void)increment(static_cast<volatile long*>(at(a.owner,4)));
        Word capacity=word(at(texture,0x48));
        auto* const cache=static_cast<NativeTextureSurfaceCacheStorage*>(at(texture,0x40));
        if(word(at(cache,4))==capacity) {
            const auto doubled=static_cast<std::int32_t>(capacity+capacity);
            reserve_native_texture_surface_cache_00b3d9b0(*cache,doubled>1?doubled:1);
        }
        void* const entry=pointer(word(cache)+word(at(cache,4))*8u);
        if(entry) {
            put(entry,mip);put(at(entry,4),reinterpret_cast<Word>(a.owner));
        }
        put(at(cache,4),word(at(cache,4))+1u);
    }
    a.phase=NativeTextureSurfaceGetterAcquired::Phase::complete;return a.owner;
}
} // namespace bsp
