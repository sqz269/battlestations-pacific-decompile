#include "bsp/native_lua_bootstrap.hpp"
#include <cstring>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
namespace bsp {
int native_lua_panic_00b669c0(lua_State* state){
    (void)lua_tolstring(state,lua_gettop(state),nullptr);return 0;
}
namespace {
struct Library {const char* name;lua_CFunction open;};
const Library libraries[]={
    {"",luaopen_base},{LUA_LOADLIBNAME,luaopen_package},{LUA_TABLIBNAME,luaopen_table},
    {LUA_IOLIBNAME,luaopen_io},{LUA_OSLIBNAME,luaopen_os},{LUA_STRLIBNAME,luaopen_string},
    {LUA_MATHLIBNAME,luaopen_math},{LUA_DBLIBNAME,luaopen_debug}
};
void platform_chunk(NativeLuaStateStorage& owner,NativeStringStorage& strings,const char* code,bool current_release_length=false){
    NativeString temporary;temporary.assign_0041e870(strings,code);
    char* const data=temporary.data();const auto length=temporary.length();
    if(luaL_loadstring(owner.state_04,data?data:"")==0)(void)lua_pcall(owner.state_04,0,LUA_MULTRET,0);
    if(data)strings.release(data,(current_release_length?temporary.length():length)+1u);
}
}
void open_native_lua_state_00b6a020(NativeLuaStateStorage& owner,std::uint32_t mask,
    NativeStringStorage& strings,const NativeLuaBootstrapInputs& inputs){
    if(!inputs.get_fundamentals_00884770 || !inputs.do_file_00b69e00)
        throw std::invalid_argument("native Lua bootstrap requires canonical fundamentals and DoFile services");
    owner.owns_00=1;owner.state_04=luaL_newstate();lua_atpanic(owner.state_04,&native_lua_panic_00b669c0);
    std::uint32_t bit=1;
    for(const auto& library:libraries){
        if(bit==1 || (mask&bit)){
            lua_pushcclosure(owner.state_04,library.open,0);lua_pushstring(owner.state_04,library.name);
            lua_call(owner.state_04,1,0);
        }
        bit+=bit;
    }
    owner.opaque_08=static_cast<std::uint32_t>(lua_gettop(owner.state_04));
    platform_chunk(owner,strings,"PC=true");
    platform_chunk(owner,strings,inputs.x360comp_0108ff20?"X360COMP=true":"X360COMP=false");
    const char* region=inputs.region_0108ff24.data();
    if(region && _stricmp(region,"EU")==0)platform_chunk(owner,strings,"REGION=\"EU\"");
    else {
        region=inputs.region_0108ff24.data();
        if(region && _stricmp(region,"USA")==0)platform_chunk(owner,strings,"REGION=\"USA\"");
        else {
            region=inputs.region_0108ff24.data();
            if(region && _stricmp(region,"JAP")==0)platform_chunk(owner,strings,"REGION=\"JAP\"",true);
        }
    }
    lua_pushcclosure(owner.state_04,inputs.do_file_00b69e00,0);lua_setfield(owner.state_04,LUA_GLOBALSINDEX,"DoFile");
    const auto length=inputs.get_fundamentals_00884770(inputs.fundamentals_context)->size_08;
    const char* const data=inputs.get_fundamentals_00884770(inputs.fundamentals_context)->bytes_04;
    (void)luaL_loadbuffer(owner.state_04,data,length,"Scripts\\fundamentals.lua");
    lua_call(owner.state_04,0,LUA_MULTRET);
}
} // namespace bsp
