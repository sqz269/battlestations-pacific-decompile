#include "bsp/native_shader_compiler_lookups.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Op=NativeShaderCompilerLookupOperation;
template<class T> T read(const void* p,std::size_t offset=0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const char*>(p)+offset);
}
void begin(Op& a,std::uint32_t fn) {
    if(a.phase!=Op::Phase::fresh)throw std::logic_error("compiler lookup operation is one-shot");
    a.function=fn;a.phase=Op::Phase::running;
}
void increment_cursor(Op& a) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(a.cache)=read<std::uint32_t>(a.cache)+1u;
}
}
NativeShaderCompilerLookupOperation::~NativeShaderCompilerLookupOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativeShaderCompilerLookupOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed)phase=Phase::diagnostic_retired;
}
std::uint32_t find_native_sampler_ordinal_00b347e0(const NativeShaderDescriptorStorage& descriptor,
    const NativeString* query,Op& a) {
    begin(a,0x00b347e0);a.descriptor=&descriptor;a.query=query;
    try {
        while(a.index<read<std::uint32_t>(&descriptor,0xc8)) {
            a.data=read<void*>(&descriptor,0xc4);
            a.row=read<const void*>(a.data,a.index*4u);
            a.row_length=read<std::uint32_t>(a.row,4);
            a.query_length=read<std::uint32_t>(query);
            if(a.row_length==a.query_length) {
                a.comparison=0;
                if(a.row_length!=0) {
                    a.query_data=read<const char*>(query,4);
                    a.row_data=read<const char*>(a.row,8);
                    a.native_site=0x00b34825;
                    a.comparison=::_stricmp(a.row_data,a.query_data);
                }
                if(a.comparison==0){a.phase=Op::Phase::complete;return a.index;}
            }
            ++a.index;++a.completed_rows;
        }
        a.phase=Op::Phase::complete;return 0xffffffffu;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
NativeShaderBinaryCacheRecordStorage* find_native_shader_binary_cache_record_00b34890(
    NativeShaderBinaryCacheStorage* cache,const NativeString* query,
    const volatile std::uint8_t& gate,Op& a) {
    begin(a,0x00b34890);a.cache=cache;a.query=query;a.source_mode_0108d6f1=&gate;
    try {
        if(gate!=0){a.phase=Op::Phase::complete;return nullptr;}
        a.index=read<std::uint32_t>(cache);
        while(a.index<read<std::uint32_t>(cache,4)) {
            a.current_cursor=read<std::uint32_t>(cache);
            a.query_length=read<std::uint32_t>(query);
            a.data=read<void*>(cache,0xc);
            a.row=reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(a.data)+a.current_cursor*16u);
            a.row_length=read<std::uint32_t>(a.row);
            if(a.query_length==a.row_length) {
                a.comparison=0;
                if(a.query_length!=0) {
                    a.row_data=read<const char*>(a.row,4);
                    a.query_data=read<const char*>(query,4);
                    a.native_site=0x00b348dd;
                    a.comparison=::_stricmp(a.query_data,a.row_data);
                }
                if(a.comparison==0) {
                    increment_cursor(a);
                    a.current_cursor=read<std::uint32_t>(cache);
                    a.data=read<void*>(cache,0xc);
                    a.phase=Op::Phase::complete;
                    return reinterpret_cast<NativeShaderBinaryCacheRecordStorage*>(
                        reinterpret_cast<std::uintptr_t>(a.data)+a.current_cursor*16u-16u);
                }
            }
            increment_cursor(a);++a.index;++a.completed_rows;
        }
        a.phase=Op::Phase::complete;return nullptr;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
void apply_native_descriptor_render_states_00b34920(void* pass,
    const NativeShaderDescriptorStorage& descriptor,Op& a) {
    begin(a,0x00b34920);a.pass=pass;a.descriptor=&descriptor;
    try {
        while(a.index<read<std::uint32_t>(&descriptor,0xbc)) {
            a.data=read<void*>(&descriptor,0xb8);
            a.sampled_value=read<std::uint32_t>(a.data,a.index*8u+4u);
            a.sampled_state=read<std::uint32_t>(a.data,a.index*8u);
            a.row=reinterpret_cast<const char*>(a.data)+a.index*8u;
            a.native_site=0x00b34954;
            set_native_material_render_state_00b5ec40(pass,a.sampled_state,a.sampled_value);
            ++a.index;++a.completed_rows;
        }
        a.phase=Op::Phase::complete;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
