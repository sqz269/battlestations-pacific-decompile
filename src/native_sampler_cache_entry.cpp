#include "bsp/native_sampler_cache_entry.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T read(const void* p,std::uint32_t offset=0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const char*>(p)+offset);
}
template<class T> bool unresolved(const std::optional<T>& value) noexcept {
    return value&&(value->phase==T::Phase::running||value->phase==T::Phase::failed);
}
}
NativeSamplerCacheEntryOperation::~NativeSamplerCacheEntryOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativeSamplerCacheEntryOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::running||unresolved(pump)||unresolved(continuation))std::terminate();
    phase=Phase::diagnostic_retired;
}
NativeSamplerDefaultLoadOperation::~NativeSamplerDefaultLoadOperation(){
    if(phase==Phase::running||phase==Phase::failed||requested_live||options_live)std::terminate();
}
void NativeSamplerDefaultLoadOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::running||requested_live||options_live||unresolved(cache_child))std::terminate();
    phase=Phase::diagnostic_retired;
}
void* load_native_sampler_cache_00b1a4f0(void* cache,const void* name,const void* options,
    std::uint8_t retain_new,std::uint8_t load_if_missing,NativeSamplerCacheEntryContext& c,
    NativeSamplerCacheEntryOperation& a) {
    using Phase=NativeSamplerCacheEntryOperation::Phase;
    if(a.phase!=Phase::fresh)throw std::logic_error("sampler cache entry is one-shot");
    a.phase=Phase::running;a.context=&c;a.captured_cache=cache;a.input_name=name;a.options=options;
    try {
        a.captured_platform=c.actual_platform_0109cf04;
        a.pump.emplace();
        std::memcpy(&a.pump->message,&c.native_message_preimage,sizeof(MSG));
        a.native_site=0x00b1a518;
        pump_native_platform_load_messages_00beccd0(a.captured_platform,c.messages,*a.pump);
        a.continuation.emplace();a.native_site=0x00b1a51d;
        a.result=continue_native_sampler_cache_after_pump_00b1a51d(cache,name,options,
            retain_new,load_if_missing,c.cache,*a.continuation);
        a.phase=Phase::complete;return a.result;
    }catch(...){a.phase=Phase::failed;throw;}
}
void* load_native_sampler_with_default_options_00b1b4d0(NativeSamplerLoaderSingletonStorage* receiver,
    const void* name,NativeSamplerCacheEntryContext& c,NativeSamplerDefaultLoadOperation& a) {
    using Phase=NativeSamplerDefaultLoadOperation::Phase;
    if(a.phase!=Phase::fresh)throw std::logic_error("default sampler load is one-shot");
    a.phase=Phase::running;a.receiver=receiver;a.input_name=name;a.context=&c;
    try {
        a.requested[0]=0;a.requested[1]=0;a.requested_live=true;
        if(a.requested!=name) {
            a.native_site=0x00b1b50f;
            resize_native_string_header_0041dd40(a.requested,c.cache.strings,read<std::uint32_t>(name),true);
            a.captured_requested_data=read<char*>(a.requested,4);
            if(read<std::uint32_t>(name)) {
                const auto count=read<std::uint32_t>(a.requested);
                const void* const source=read<void*>(name,4);
                a.native_site=0x00b1b526;
                std::memmove(a.captured_requested_data,source,count);
            }
        }
        a.native_site=0x00b1b536;lowercase_native_string_header_004bcc00(a.requested);
        a.options[0]=0;a.options[1]=0;a.options_live=true;
        a.native_site=0x00b1b54b;resize_native_string_header_0041dd40(a.options,c.cache.strings,7,true);
        if(void* const output=read<void*>(a.options,4)) {
            const auto bytes=read<std::uint32_t>(a.options)+1u;
            a.native_site=0x00b1b566;std::memmove(output,c.actual_default_text_00ce3c5c,bytes);
        }
        a.cache_child.emplace();a.native_site=0x00b1b584;
        a.result=load_native_sampler_cache_00b1a4f0(reinterpret_cast<char*>(receiver)+4,
            a.requested,a.options,1,1,c,*a.cache_child);
        if(char* const data=read<char*>(a.options,4)) {
            const auto bytes=read<std::uint32_t>(a.options)+1u;
            a.native_site=0x00b1b5a2;c.cache.strings.release(data,bytes);
        }
        a.options_live=false;
        if(a.captured_requested_data) {
            const auto bytes=read<std::uint32_t>(a.requested)+1u;
            a.native_site=0x00b1b5c5;c.cache.strings.release(a.captured_requested_data,bytes);
        }
        a.requested_live=false;a.phase=Phase::complete;return a.result;
    }catch(...){a.phase=Phase::failed;throw;}
}
} // namespace bsp
