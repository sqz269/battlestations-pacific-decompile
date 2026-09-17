#include "bsp/native_shader_binary_cache.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/native_material_compiler_providers.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shader binary cache requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
using Cache=NativeShaderBinaryCacheStorage;
using Row=NativeShaderBinaryCacheRecordStorage;
using Op=NativeShaderBinaryCacheOperation;
template<class T> T read(const void* p,U offset=0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const char*>(p)+offset);
}
template<class T> void put(void* p,U offset,T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<char*>(p)+offset)=value;
}
U entry(void* stream,U slot) noexcept {return read<U>(read<void*>(stream),slot);}
void require(bool condition,const char* message) {if(!condition)throw std::logic_error(message);}
void start(Op& a,U address,Cache* cache) {
    require(a.phase==Op::Phase::fresh,"shader cache operation is one-shot");
    a.phase=Op::Phase::running;a.function=address;a.cache=cache;
}
U read_count(void* stream,NativeShaderBinaryCacheContext& c) {
    require(entry(stream,0x38)==0x00be4300,"unsupported shader-cache stream count entry");
    return read_native_stream_dword_slot38_00be4300(stream,nullptr,c.reads.streams);
}
void write_count(void* stream,U count,NativeShaderBinaryCacheContext& c) {
    require(entry(stream,0x54)==0x00be40d0,"unsupported shader-cache stream write entry");
    (void)write_native_compiler_stream_word_00be40d0(stream,count,nullptr,c.actual_physical_profile_00d691b0);
}
Row* row(Cache* cache,U offset) noexcept {
    return reinterpret_cast<Row*>(reinterpret_cast<U>(read<void*>(cache,0xc))+offset);
}
void load(Cache* cache,NativeShaderBinaryCacheContext& c,Op& a) {
    if(c.source_mode_0108d6f1)return;
    put<U>(cache,0,0);a.native_site=0x00b35378;
    const U count=read_count(read<void*>(cache,8),c);
    put<U>(cache,4,count);
    const auto product=static_cast<std::uint64_t>(count)*16u;
    const U multiplied=product>0xffffffffu ? 0xffffffffu : static_cast<U>(product);
    a.allocation_bytes=multiplied>0xfffffffbu ? 0xffffffffu : multiplied+4u;
    a.native_site=0x00b3539c;
    auto* allocation=c.allocate_array_00bf55be(a.allocation_bytes);a.allocation=allocation;
    Row* rows=nullptr;
    if(allocation) {
        put<U>(allocation,0,count);rows=reinterpret_cast<Row*>(static_cast<char*>(allocation)+4);
        for(U i=0;i<count;++i) {
            initialize_native_shader_cache_record_00b34c70(rows+i);++a.constructed_records;
        }
    }
    put<Row*>(cache,0xc,rows);
    U index=0,offset=0;
    while(index<read<U>(cache,4)) {
        a.index=index;a.native_site=0x00b353f5;
        void* stream=read<void*>(cache,8);
        require(entry(stream,0x60)==0x00be45f0,"unsupported shader-cache string entry");
        auto* returned=read_native_shader_cache_string_00be45f0(stream,&a.temporary,nullptr,c.reads);
        a.temporary_live=true;
        auto* destination=row(cache,offset);
        if(destination!=returned) {
            a.native_site=0x00b35412;
            resize_native_string_header_0041dd40(destination,c.reads.strings,read<U>(returned),true);
            if(read<U>(returned)) {
                a.native_site=0x00b35428;
                std::memmove(read<void*>(destination,4),read<void*>(returned,4),read<U>(destination));
            }
        }
        a.native_site=0x00b3544b;
        destroy_native_string_header_0041dd20(&a.temporary,c.reads.strings);a.temporary_live=false;
        a.native_site=0x00b35461;
        const U size=read_count(read<void*>(cache,8),c);
        put<U>(row(cache,offset),0xc,size);
        a.native_site=0x00b35472;
        auto* bytes=c.allocate_array_00bf55be(read<U>(row(cache,offset),0xc));
        put<void*>(row(cache,offset),8,bytes);
        auto* current=row(cache,offset);const U current_size=read<U>(current,0xc);
        stream=read<void*>(cache,8);auto* data=read<void*>(current,8);
        const U target=entry(stream,0x24);a.native_site=0x00b3549a;
        c.reads.streams.source_read(target,stream,data,current_size,nullptr);
        ++index;offset+=16u;a.completed_records=index;
    }
    a.index=index;
}
Cache* construct(void* raw,NativeShaderBinaryCacheContext& c,Op& a) {
    auto* cache=static_cast<Cache*>(raw);a.cache=cache;
    put<U>(cache,4,0);put<void*>(cache,8,nullptr);put<Row*>(cache,0xc,nullptr);
    if(!c.source_mode_0108d6f1) {
        a.native_site=0x00b38aac;
        construct_native_string_header_0041e870(&a.temporary,c.reads.strings,c.actual_filename_00d60d30);
        a.temporary_live=true;
        const U flags=c.load_variants_0108d6f0 ? 5u : 2u;
        auto* manager=c.actual_vfs_0109ceec;const U target=entry(manager,4);
        a.native_site=0x00b38ad6;
        auto* stream=c.vfs.open_manager_entry(target,manager,&a.temporary,flags);
        put<void*>(cache,8,stream);
        a.native_site=0x00b38af6;
        destroy_native_string_header_0041dd20(&a.temporary,c.reads.strings);a.temporary_live=false;
        if(c.load_variants_0108d6f0) {
            a.native_site=0x00b38b14;write_count(read<void*>(cache,8),0,c);
        }else {a.native_site=0x00b38b2b;load(cache,c,a);}
    }
    return cache;
}
}
NativeShaderBinaryCacheOperation::~NativeShaderBinaryCacheOperation() {
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
Row* initialize_native_shader_cache_record_00b34c70(void* raw) noexcept {
    put<U>(raw,0,0);put<U>(raw,4,0);put<U>(raw,8,0);return static_cast<Row*>(raw);
}
void destroy_native_shader_cache_record_00b34c80(Row& record,NativeShaderBinaryCacheContext& c) {
    if(auto* bytes=read<void*>(&record,8)) {c.free_array_00bf6989(bytes);put<void*>(&record,8,nullptr);}
    destroy_native_string_header_0041dd20(&record,c.reads.strings);
}
void* read_native_shader_cache_string_00be45f0(void* stream,void* output,U* actual,
    NativeResourceStreamReadContext& c) {
    require(entry(stream,0x48)==0x00be4620,"unsupported shader-cache underlying string reader");
    (void)read_native_stream_string_00be4620(stream,output,actual,c);return output;
}
void load_native_shader_cache_records_00b35340(Cache* cache,NativeShaderBinaryCacheContext& c,Op& a) {
    start(a,0x00b35340,cache);
    try {load(cache,c,a);a.phase=Op::Phase::complete;}catch(...) {a.phase=Op::Phase::failed;throw;}
}
Cache* construct_native_shader_binary_cache_00b38a70(void* raw,NativeShaderBinaryCacheContext& c,Op& a) {
    start(a,0x00b38a70,static_cast<Cache*>(raw));
    try {auto* result=construct(raw,c,a);a.phase=Op::Phase::complete;return result;}
    catch(...) {a.phase=Op::Phase::failed;throw;}
}
void destroy_native_shader_binary_cache_00b352b0(Cache* cache,NativeShaderBinaryCacheContext& c) {
    if(c.source_mode_0108d6f1)return;
    if(c.load_variants_0108d6f0) {
        auto* stream=read<void*>(cache,8);const U target=entry(stream,0x1c);
        (void)c.reads.streams.source_seek(target,stream,0,0,0);
        stream=read<void*>(cache,8);const U count=read<U>(cache,4);write_count(stream,count,c);
    }
    if(auto* rows=read<Row*>(cache,0xc)) {
        auto* allocation=reinterpret_cast<char*>(rows)-4;U count=read<U>(allocation);
        while(count) {--count;destroy_native_shader_cache_record_00b34c80(rows[count],c);}
        c.free_array_00bf6989(allocation);put<Row*>(cache,0xc,nullptr);
    }
    if(auto* stream=read<void*>(cache,8)) {
        auto* count=reinterpret_cast<volatile long*>(static_cast<char*>(stream)+4);
        if(c.decrement_00ce2220(count)==0) {
            const U table=read<U>(stream);const U target=read<U>(reinterpret_cast<void*>(table));
            c.reads.streams.source_zero_reference(target,stream,table);
        }
        put<void*>(cache,8,nullptr);
    }
}
void create_native_shader_binary_cache_00b3a600(NativeShaderBinaryCacheContext& c,Op& a) {
    start(a,0x00b3a600,nullptr);
    try {
        a.native_site=0x00b3a618;auto* raw=c.allocate_object_00bf681b(16);a.allocation=raw;
        Cache* result=nullptr;
        if(raw) {a.native_site=0x00b3a631;result=construct(raw,c,a);}
        c.actual_cache_0108d6ec=result;a.phase=Op::Phase::complete;
    }catch(...) {a.phase=Op::Phase::failed;throw;}
}
void release_native_shader_binary_cache_00b3b140(NativeShaderBinaryCacheContext& c) {
    if(auto* captured=c.actual_cache_0108d6ec) {
        destroy_native_shader_binary_cache_00b352b0(captured,c);
        c.free_object_00bf65ac(captured);c.actual_cache_0108d6ec=nullptr;
    }
}
} // namespace bsp
