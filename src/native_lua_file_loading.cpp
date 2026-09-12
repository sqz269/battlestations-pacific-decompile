#include "bsp/native_lua_file_loading.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <Windows.h>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
namespace bsp {
namespace {
std::uint64_t length(void* stream,NativeLuaVfsDispatch& dispatch){return dispatch.length(capture_native_lua_vfs_table(stream),stream);}
}
void load_native_lua_chunk_00b66ca0(NativeLuaStateStorage& owner,const NativeString& path,std::uint32_t obfuscated,const NativeLuaFileServices& services){
    void* const manager=services.manager_0109ceec;
    auto& dispatch=services.vfs;
    void* const stream=dispatch.open(capture_native_lua_vfs_table(manager),manager,path,2);
    if(!stream || !dispatch.is_open(capture_native_lua_vfs_table(stream),stream) || !length(stream,dispatch))return;
    const auto allocated=static_cast<std::uint32_t>(length(stream,dispatch));
    auto* const bytes=static_cast<unsigned char*>(singleton_lifetime_allocate({SingletonAllocationKind::object,allocated,allocated}));
    const auto read_table=capture_native_lua_vfs_table(stream);std::uint32_t ignored_read_count;
    const auto requested=static_cast<std::uint32_t>(dispatch.length(read_table,stream));
    dispatch.read(read_table,stream,bytes,requested,&ignored_read_count);
    if(static_cast<std::uint8_t>(obfuscated)){
        bool prefix=true;std::uint32_t index=0;
        if(length(stream,dispatch))do {
            if(prefix){if(bytes[index]==1)prefix=false;bytes[index]=0x20;}
            else bytes[index]=static_cast<unsigned char>((bytes[index]<<4)|(bytes[index]>>4));
            ++index;
        }while(static_cast<std::uint64_t>(static_cast<std::int64_t>(static_cast<std::int32_t>(index)))<length(stream,dispatch));
    }
    const char* const chunk_name=path.data()?path.data():"";
    const auto chunk_size=static_cast<std::uint32_t>(length(stream,dispatch));
    (void)luaL_loadbuffer(owner.state_04,reinterpret_cast<const char*>(bytes),chunk_size,chunk_name);
    if(InterlockedDecrement(reinterpret_cast<volatile LONG*>(static_cast<char*>(stream)+4))==0)
        dispatch.zero_reference(capture_native_lua_vfs_table(stream),stream);
    lua_call(owner.state_04,0,LUA_MULTRET);singleton_lifetime_free(bytes);
}
void run_native_lua_file_00b69d40(NativeLuaStateStorage& owner,const NativeString& path,std::uint32_t obfuscated,NativeStringStorage& strings,const NativeLuaFileServices& services){
    if(!services.append_overrides_00bdef90)throw std::invalid_argument("native Lua file loading requires override append service");
    load_native_lua_chunk_00b66ca0(owner,path,obfuscated,services);
    NativeStringVectorStorage paths{nullptr,0,0};
    __try {
        services.append_overrides_00bdef90(services.manager_0109ceec,path,paths);
        auto cursor=reinterpret_cast<std::uintptr_t>(paths.data_00);
        const auto end=cursor+static_cast<std::uint32_t>(paths.count_04)*8u;
        while(cursor!=end){load_native_lua_chunk_00b66ca0(owner,*reinterpret_cast<NativeString*>(cursor),obfuscated,services);cursor+=8u;}
    } __finally {destroy_native_string_vector_004d0fa0(paths,strings);}
}
int do_native_lua_file_00b69e00(lua_State* state,NativeStringStorage& strings,const NativeLuaFileServices& services){
    NativeLuaStateStorage owner;construct_native_lua_borrowed_state_00b66c00(&owner,*state,services.do_file_00b69e00);
    __try {
        NativeLuaObjectStorage frame;native_lua_call_frame_00b679b0(owner,&frame);
        __try {
            NativeLuaObjectStorage argument;native_lua_argument_at_00b677e0(frame,&argument,0);
            __try {
                NativeString path;path.assign_0041e870(strings,lua_tolstring(argument.owner_00->state_04,argument.index_08,nullptr));
                __try {run_native_lua_file_00b69d40(owner,path,0,strings,services);}
                __finally {destroy_native_string_header_0041dd20(&path,strings);}
            } __finally {
                if(AbnormalTermination())destroy_native_lua_object_00b67700(argument);
                else if(argument.kind_04)release_native_lua_tracked_object_00b66de0(argument.owner_00,argument,argument.index_08,1);
            }
        } __finally {destroy_native_lua_object_00b67700(frame);}
    } __finally {
        if(AbnormalTermination())close_native_lua_state_00b669a0(owner);
        else if(owner.state_04 && owner.owns_00)lua_close(owner.state_04);
    }
    return 0;
}
} // namespace bsp
