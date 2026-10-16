#include "bsp/native_node_raw_transform.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_traceline_render.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
template<class T> T read(const void* p,U offset=0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const unsigned char*>(p)+offset);
}
void* at(void* p,U offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);
}
void store(void* p,U offset,U value) noexcept {
    *reinterpret_cast<volatile U*>(at(p,offset))=value;
}
void require(bool condition,const char* why) { if(!condition)throw std::invalid_argument(why); }
}

NativeNodeMappedRawTransformProfiles::NativeNodeMappedRawTransformProfiles(
    const game::GameNativeReadOnlyData& data) {
    constexpr U addresses[]={0x00d62c88,0x00d62de8,0x00d634f8};
    const volatile U* aliases[]={
        static_cast<const volatile U*>(data.data_at(addresses[0],0x40)),
        static_cast<const volatile U*>(data.data_at(addresses[1],0x40)),
        static_cast<const volatile U*>(data.data_at(addresses[2],0x40))};
    // data_at checks existing held bands and returns the Original VA itself.
    // Finish all availability/identity checks before storing the borrowed set.
    for(U i=0;i<3;++i)
        require(reinterpret_cast<U>(aliases[i])==addresses[i],
            "mapped raw transform requires the genuine Original table aliases");
    for(U i=0;i<3;++i)profiles_[i]=aliases[i];
}

struct NativeNodeMappedRawTransformAccess final {
    static const volatile U* current_profile(
        void* actual,const NativeNodeMappedRawTransformProfiles& profiles) {
        const volatile U* current=read<const volatile U*>(actual);
        require(current==profiles.profiles_[0] || current==profiles.profiles_[1] ||
            current==profiles.profiles_[2],
            "current actual A0 profile is outside the mapped raw transform family");
        return current;
    }
};

void __fastcall invalidate_raw_descendants_00b6da30(void* actual) {
    void* child=read<void*>(actual,0x34);
    while(child) {
        const U flags=read<U>(child,0x5c);
        if(flags&2u) {
            store(child,0x138,read<U>(child,0x138)&0xffffffcfu);
            const bool has_child=read<void*>(child,0x34)!=nullptr;
            store(child,0x5c,flags&0xfffffff5u);
            if(has_child)invalidate_raw_descendants_00b6da30(child);
        }
        child=read<void*>(child,0x3c);
    }
}
namespace {
void require_bounds_target(void* actual,NativeTracelineRenderServices* profiles,
    const char* missing,const char* unsupported) {
    require(profiles!=nullptr,missing);
    const volatile U* profile=profiles->profile(actual);
    require(profile&&profile[0x3c/4]==0x00b6dbc0,unsupported);
}
void require_bounds_target(void* actual,const NativeNodeMappedRawTransformProfiles& profiles,
    const char*,const char* unsupported) {
    const volatile U* profile=NativeNodeMappedRawTransformAccess::current_profile(actual,profiles);
    require(profile[0x3c/4]==0x00b6dbc0,unsupported);
}
template<class Profiles>
void current_bounds_notification(void* actual,const Profiles& profiles) {
    require_bounds_target(actual,profiles,
        "current actual A0 virtual3C requires its canonical profile resolver",
        "current actual A0 virtual3C target is outside the observed native closure");
    notify_raw_bounds_00b6dbc0(actual,profiles);
}
template<class Profiles>
void notify_raw_bounds_body(void* actual,const Profiles& profiles) {
    // The native JMP dispatch is a tail loop. Do not add host recursion here.
    for(;;) {
        store(actual,0x138,read<U>(actual,0x138)&0xffffffc3u);
        actual=read<void*>(actual,0xa0);
        if(!actual)return;
        require_bounds_target(actual,profiles,
            "current enclosing A0 virtual3C requires its canonical profile resolver",
            "current enclosing A0 virtual3C target is outside the observed native closure");
    }
}
template<class Profiles>
void set_raw_local_matrix_body(void* actual,const Profiles& profiles,const void* source) {
    copy_native_camera_matrix_004134f0(at(actual,0xb0),nullptr,source);
    if(read<std::uint8_t>(actual,0x5c)&0xau) {
        void* captured_attachment=read<void*>(actual,0xa0);
        store(actual,0x138,read<U>(actual,0x138)&0xffffffcfu);
        store(actual,0x5c,0);
        if(captured_attachment)current_bounds_notification(captured_attachment,profiles);
        if(read<void*>(actual,0x34))invalidate_raw_descendants_00b6da30(actual);
    }
}
}

void __fastcall notify_raw_bounds_00b6dbc0(void* actual,NativeTracelineRenderServices* profiles) {
    notify_raw_bounds_body(actual,profiles);
}
void __fastcall set_raw_local_matrix_00b6db10(void* actual,
    NativeTracelineRenderServices* profiles,const void* source) {
    set_raw_local_matrix_body(actual,profiles,source);
}
void __fastcall notify_raw_bounds_00b6dbc0(void* actual,
    const NativeNodeMappedRawTransformProfiles& profiles) {
    notify_raw_bounds_body(actual,profiles);
}
void __fastcall set_raw_local_matrix_00b6db10(void* actual,
    const NativeNodeMappedRawTransformProfiles& profiles,const void* source) {
    set_raw_local_matrix_body(actual,profiles,source);
}

} // namespace bsp
