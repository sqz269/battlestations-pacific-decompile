#include "bsp/native_settings_renderer.hpp"
#include <cstring>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native settings renderer requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;
void* at(const void* p,U n=0) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
U get(const void* p,U n=0) noexcept{return *static_cast<const volatile U*>(at(p,n));}
void put(void* p,U n,U v) noexcept{*static_cast<volatile U*>(at(p,n))=v;}
I signed_bits(U n) noexcept{I v;std::memcpy(&v,&n,4);return v;}
void append(void* h,U v,NativeSettingsChoiceCalls& c) {
    const U count=get(h,4),capacity=get(h,8);
    if(count==capacity){const U doubled=capacity*2;c.reserve_dwords_0086a220(h,signed_bits(doubled)>1?signed_bits(doubled):1);}
    void* const row=at(reinterpret_cast<void*>(get(h)),get(h,4)*4);
    if(row)put(row,0,v);
    put(h,4,get(h,4)+1);
}
}
void* __fastcall native_renderer_resolution_header_00b1fff0(void* p) noexcept{return at(p,0x1c);}
void* __fastcall native_renderer_antialias_header_00b20000(void* p) noexcept{return at(p,0x28);}
I __fastcall native_renderer_shader_ceiling_00b200b0(const void* p) noexcept{return signed_bits(get(p,0x1b48));}
void __fastcall select_native_renderer_shader_00b200c0(void*,U,I) noexcept{}
void rebuild_native_renderer_antialias_00b295c0(void* renderer,U format,NativeSettingsChoiceCalls& c) {
    void* const h=at(renderer,0x28);c.resize_dwords_0086a430(h,0);append(h,0,c);
    const U captured_format=format;U quality=format;
    using Check=I(__stdcall*)(void*,U,U,U,int,U,U*);
    for(U sample=2;sample<16;++sample) {
        void* const api=reinterpret_cast<void*>(get(renderer,0x1990));
        const auto check=reinterpret_cast<Check>(get(reinterpret_cast<void*>(get(api)),0x2c));
        if(check(api,0,1,captured_format,0,sample,&quality)==0)append(h,sample,c);
    }
}
} // namespace bsp
