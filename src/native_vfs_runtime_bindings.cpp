#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/native_vfs_open_route.hpp"
#include "bsp/native_vfs_lookup_routes.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_physical_stream_open.hpp"
#include "bsp/native_physical_stream_conversion.hpp"
#include "bsp/native_filestore_open.hpp"
#include "bsp/native_vfs_open_logging.hpp"
#include "bsp/native_memory_stream.hpp"
#include <stdexcept>
namespace bsp {
namespace {
std::uintptr_t slot(std::uintptr_t table,unsigned offset) {
    return reinterpret_cast<const volatile std::uintptr_t*>(table)[offset/4];
}
void require(std::uintptr_t table,std::uintptr_t expected) {
    if(table!=expected)throw std::invalid_argument("Unimplemented native VFS class identity");
}
[[noreturn]] void unsupported() {throw std::invalid_argument("Unimplemented current native VFS method");}
void require_stream(std::uintptr_t table) {
    if(table!=0x00d642c0 && table!=0x00d691b0 && table!=0x00d68db0)unsupported();
}
std::uint32_t memory_seek_result(void* stream,std::uint32_t distance_low,
    std::uint32_t distance_high,std::uint32_t origin) {
    // Existing BEF540 is a naked original-ABI body declared void. Preserve
    // its native EAX too because BF10E0 forwards the source's register result.
    auto entry=&native_memory_stream_seek_00bef540;
    std::uint32_t result;
    __asm {
        push origin
        push distance_high
        push distance_low
        mov ecx,stream
        mov edx,distance_high
        mov eax,entry
        call eax
        mov result,eax
    }
    return result;
}
}
NativeVfsRuntimeBindings::NativeVfsRuntimeBindings(NativeVfsOpenRouteContext& route,
    NativeVfsLookupRouteContext& lookup,NativePhysicalStreamOpenContext& physical,
    NativeStoredStreamConversionContext& store,NativeVfsOpenLoggingContext& logging,
    NativeRetainedMemoryOwnerContext& memory)
    :route_(route),lookup_(lookup),physical_(physical),store_(store),logging_(logging),memory_(memory),
     previous_(route.native_bindings),previous_substreams_(store.adopted_substreams) {
    route_.native_bindings=this;
    store_.adopted_substreams=this;
}
NativeVfsRuntimeBindings::~NativeVfsRuntimeBindings() {
    if(route_.native_bindings==this)route_.native_bindings=previous_;
    if(store_.adopted_substreams==this)store_.adopted_substreams=previous_substreams_;
}
void* NativeVfsRuntimeBindings::open(std::uintptr_t table,void* manager,const NativeString& name,std::uint32_t flags) {
    require(table,0x00d685b4);if(slot(table,4)!=0x00bdf310)unsupported();
    return open_native_vfs_resource_00bdf310(manager,&name,flags,route_);
}
std::uint8_t NativeVfsRuntimeBindings::exists(std::uintptr_t table,void* manager,NativeString& name) {
    require(table,0x00d685b4);if(slot(table,8)!=0x00bdd440)unsupported();
    return exists_native_vfs_file_00bdd440(manager,&name,lookup_);
}
std::uint8_t NativeVfsRuntimeBindings::is_open(std::uintptr_t table,void* stream) {
    require_stream(table);
    return source_is_open(slot(table,0x18),stream);
}
std::uint8_t NativeVfsRuntimeBindings::source_is_open(std::uintptr_t entry,void* stream) {
    switch(entry) {
    case 0x00bef4c0:return native_memory_stream_open_00bef4c0(stream,nullptr);
    case 0x00bf5020:return valid_native_physical_stream_00bf5020(stream);
    case 0x00bf1090:return open_native_adopted_substream_00bf1090(stream,*this);
    default:unsupported();
    }
}
std::uint64_t NativeVfsRuntimeBindings::length(std::uintptr_t table,void* stream) {
    require_stream(table);
    switch(slot(table,0x30)) {
    case 0x00bef600:return static_cast<std::uint64_t>(native_memory_stream_length_00bef600(stream,nullptr));
    case 0x00bf4f90:return size_native_physical_stream_00bf4f90(stream);
    case 0x00bf10a0:return length_native_adopted_substream_00bf10a0(stream);
    default:unsupported();
    }
}
void NativeVfsRuntimeBindings::read(std::uintptr_t table,void* stream,void* bytes,std::uint32_t count,std::uint32_t* actual) {
    require_stream(table);
    source_read(slot(table,0x24),stream,bytes,count,actual);
}
void NativeVfsRuntimeBindings::source_read(std::uintptr_t entry,void* stream,void* bytes,
    std::uint32_t count,std::uint32_t* actual) {
    switch(entry) {
    case 0x00bef590:native_memory_stream_read_00bef590(stream,nullptr,bytes,count,actual);return;
    case 0x00bf5030:(void)read_native_physical_stream_00bf5030(stream,bytes,count,actual,physical_);return;
    case 0x00bf1000:(void)read_native_adopted_substream_00bf1000(stream,bytes,count,actual,*this);return;
    default:unsupported();
    }
}
std::uint32_t NativeVfsRuntimeBindings::source_seek(std::uintptr_t entry,void* stream,
    std::uint32_t low,std::uint32_t high,std::uint32_t origin) {
    switch(entry) {
    case 0x00bef540:return memory_seek_result(stream,low,high,origin);
    case 0x00bf4f20:return static_cast<std::uint32_t>(seek_native_physical_stream_00bf4f20(stream,low,high,origin));
    case 0x00bf10e0:return seek_native_adopted_substream_00bf10e0(stream,low,high,origin,*this);
    default:unsupported();
    }
}
void NativeVfsRuntimeBindings::source_write(std::uintptr_t entry,void* stream,const void* bytes,
    std::uint32_t count,std::uint32_t* actual) {
    if(entry==0x00bf1040) {
        (void)write_native_adopted_substream_00bf1040(stream,bytes,count,actual,*this);
        return;
    }
    unsupported(); // Physical/memory write leaves need their own numeric binding.
}
void NativeVfsRuntimeBindings::zero_reference(std::uintptr_t table,void* stream) {
    require_stream(table);
    source_zero_reference(slot(table,0),stream,table);
}
void NativeVfsRuntimeBindings::source_zero_reference(std::uintptr_t entry,void* stream,std::uintptr_t) {
    switch(entry) {
    case 0x00bd30e0:
    {
        if(!stream)return; // BD30E0 accepts null before reading the current table.
        // The original invoker reloads current slot4 and supplies flag1.
        const auto table=capture_native_lua_vfs_table(stream);require_stream(table);
        const auto terminal=slot(table,4);
        if(terminal==0x00bb8f90) {delete_native_memory_stream_00bb8f90(stream,1,memory_);return;}
        if(terminal==0x00bf1240) {delete_native_adopted_substream_00bf1240(stream,1,*this);return;}
        unsupported();
    }
    case 0x00bf55a0:recycle_native_physical_stream_00bf55a0(stream,physical_);return;
    default:unsupported();
    }
}
void* NativeVfsRuntimeBindings::provider_open(void* provider,const void* name,std::uint32_t flags) {
    const auto table=capture_native_lua_vfs_table(provider);
    if(table!=0x00d69168 && table!=0x00d689e8)unsupported();
    switch(slot(table,8)) {
    case 0x00bf4ba0:return open_native_physical_provider_00bf4ba0(provider,name,flags,physical_);
    case 0x00be5fa0:return open_native_file_store_00be5fa0(provider,name,flags,physical_.physical.invalid_parameters,store_);
    default:unsupported();
    }
}
std::uint32_t NativeVfsRuntimeBindings::stream_size_low(void* stream,std::uint32_t argument) {
    const auto table=capture_native_lua_vfs_table(stream);require_stream(table);
    switch(slot(table,0x2c)) {
    case 0x00be41a0:return native_stream_size_low_00be41a0(stream,reinterpret_cast<std::uint32_t*>(argument),*this);
    case 0x00bf4fa0:return query_native_physical_file_size_00bf4fa0(stream,argument);
    default:unsupported();
    }
}
void NativeVfsRuntimeBindings::log_open(void* manager,const void* name,void* stream,std::uint32_t mount_byte) {
    log_native_vfs_opened_resource_00bde9c0(manager,name,stream,mount_byte,logging_);
}
} // namespace bsp
