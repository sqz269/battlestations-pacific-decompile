#include "bsp/native_settings_vector_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native settings vectors require MSVC Win32.
#endif
namespace bsp {
namespace {
template<class T> T read(void* p,std::size_t offset){T v;std::memcpy(&v,static_cast<char*>(p)+offset,sizeof v);return v;}
}
int NativeSettingsVectorLifetimeCalls::register_shutdown_00bf6ff5(void (*callback)()){return std::atexit(callback);}
void NativeSettingsVectorLifetimeCalls::free_00bf6989(void* p){singleton_lifetime_free(p);}
int register_native_settings_resolutions_00cd2d60(NativeSettingsVectorLifetimeCalls& calls,void (*callback)()){
    return calls.register_shutdown_00bf6ff5(callback);
}
int register_native_settings_antialias_00cd2d70(NativeSettingsVectorLifetimeCalls& calls,void (*callback)()){
    return calls.register_shutdown_00bf6ff5(callback);
}
void shutdown_native_settings_resolutions_00cdee40(void* header,NativeSettingsVectorLifetimeCalls& calls){
    if(read<std::int32_t>(header,8)<0)calls.reserve_pairs_008d4750(header,0);
    auto remaining=read<std::int32_t>(header,4);
    while(remaining>0)--remaining; // EAX only; no per-iteration header stores.
    void* const backing=read<void*>(header,0);
    const std::int32_t zero=0;std::memcpy(static_cast<char*>(header)+4,&zero,4);
    calls.free_00bf6989(backing);
}
void shutdown_native_settings_antialias_00cdee80(void* header,NativeSettingsVectorLifetimeCalls& calls){
    calls.resize_dwords_0086a430(header,0);
    calls.free_00bf6989(read<void*>(header,0));
}
} // namespace bsp
