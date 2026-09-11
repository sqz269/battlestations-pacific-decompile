#include "bsp/native_lua_objects.hpp"
#include "bsp/lua_numeric.hpp"
#include <cstring>
#include <new>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
namespace bsp {
NativeLuaObjectStorage* construct_native_lua_object_00b65f50(void* fresh) noexcept {
    auto* object=::new(fresh) NativeLuaObjectStorage;
    object->owner_00=nullptr;object->kind_04=0;object->index_08=-1;object->tracked_10=0;return object;
}
NativeLuaStateStorage* construct_native_lua_state_00b66bd0(void* fresh) noexcept {
    auto* owner=::new(fresh) NativeLuaStateStorage;
    owner->owns_00=0;owner->state_04=nullptr;owner->stack_offset_0c=0;owner->opaque_10=0;
    for(auto& slot:owner->slots_14)slot.count_14=0;owner->high_water_4c4=0;return owner;
}
NativeLuaStateStorage* construct_native_lua_borrowed_state_00b66c00(
    void* fresh,lua_State& state,int (*do_file)(lua_State*)){
    if(!do_file)throw std::invalid_argument("native borrowed Lua owner requires application's DoFile callback");
    auto* owner=construct_native_lua_state_00b66bd0(fresh);owner->owns_00=0;owner->state_04=&state;
    lua_pushcclosure(&state,do_file,0);lua_setfield(owner->state_04,LUA_GLOBALSINDEX,"DoFile");return owner;
}
int execute_native_lua_string_00b66c60(NativeLuaStateStorage& owner,const NativeString& code){
    const char* text=code.data();if(!text)text="";
    const int status=luaL_loadstring(owner.state_04,text);
    return status?status:lua_pcall(owner.state_04,0,LUA_MULTRET,0);
}
void close_native_lua_state_00b669a0(NativeLuaStateStorage& owner){
    auto* state=owner.state_04;if(state && owner.owns_00)lua_close(state);owner.state_04=nullptr;
}
NativeLuaObjectStorage* native_lua_globals_00b67980(NativeLuaStateStorage& owner,void* fresh){
    auto* object=::new(fresh) NativeLuaObjectStorage;
    object->owner_00=&owner;object->kind_04=1;object->index_08=LUA_GLOBALSINDEX;
    object->opaque_0c=0;object->tracked_10=0;return object;
}
NativeLuaObjectStorage* native_lua_get_by_name_00b67800(
    NativeLuaObjectStorage& table,void* fresh,const char* key){
    const bool globals=table.kind_04==3;
    (void)lua_checkstack(table.owner_00->state_04,1);
    const auto length=std::strlen(key);
    lua_pushlstring(table.owner_00->state_04,key,length);
    const auto index=globals?LUA_GLOBALSINDEX:table.index_08;
    lua_gettable(table.owner_00->state_04,index);
    const auto top=lua_gettop(table.owner_00->state_04);
    auto* const owner=table.owner_00;
    auto* object=::new(fresh) NativeLuaObjectStorage;
    object->owner_00=owner;object->kind_04=2;object->index_08=top;object->opaque_0c=0;object->tracked_10=1;
    const auto slot_index=static_cast<std::int32_t>(static_cast<std::uint32_t>(owner->stack_offset_0c)+static_cast<std::uint32_t>(top));
    if(owner->high_water_4c4<=slot_index)owner->high_water_4c4=slot_index+1;
    auto& slot=owner->slots_14[slot_index];slot.references_00[slot.count_14]=object;++slot.count_14;return object;
}
NativeLuaObjectStorage* native_lua_get_by_string_00b68100(
    NativeLuaObjectStorage& table,void* fresh,const NativeString& key){
    const char* text=key.data();if(!text)text="";native_lua_get_by_name_00b67800(table,fresh,text);
    return static_cast<NativeLuaObjectStorage*>(fresh);
}
bool native_lua_is_number_00b66050(const NativeLuaObjectStorage& object){
    return object.kind_04==2 && lua_type(object.owner_00->state_04,object.index_08)==LUA_TNUMBER;
}
float native_lua_number_00b66270(const NativeLuaObjectStorage& object){
    return lua_number_float32_00b66270(lua_tonumber(object.owner_00->state_04,object.index_08));
}
std::int32_t native_lua_integer_00b66290(const NativeLuaObjectStorage& object,const bool& crt_sse2_conversion){
    return lua_number_integer_00b66290(lua_tonumber(object.owner_00->state_04,object.index_08),crt_sse2_conversion);
}
void release_native_lua_tracked_object_00b66de0(
    NativeLuaStateStorage* owner,NativeLuaObjectStorage& object,std::int32_t index,std::uint8_t remove_stack){
    if(!object.tracked_10)return;
    const auto slot_index=static_cast<std::int32_t>(static_cast<std::uint32_t>(owner->stack_offset_0c)+static_cast<std::uint32_t>(index));
    auto* const first=&owner->slots_14[slot_index];
    const auto count=first->count_14;std::int32_t found=-1;
    for(std::int32_t i=0;i<count;++i)if(first->references_00[i]==&object){found=i;break;}
    if(found!=-1){const auto current=first->count_14;if(found!=current-1)first->references_00[found]=first->references_00[current-1];--first->count_14;}
    if(first->count_14!=0)return;
    if(remove_stack){
        const auto top=lua_gettop(owner->state_04);
        if(index==top)lua_settop(owner->state_04,-2);else lua_remove(owner->state_04,index);
    }
    if(slot_index==owner->high_water_4c4-1)return;
    auto* destination=first;
    while(destination!=&owner->slots_14[owner->high_water_4c4-1]){
        auto* source=destination+1;
        for(std::int32_t i=0;i<source->count_14;++i)destination->references_00[i]=source->references_00[i];
        destination->count_14=source->count_14;++destination;
    }
    destination->count_14=0;
    for(auto* slot=first;slot!=&owner->slots_14[owner->high_water_4c4];++slot)
        for(std::int32_t i=0;i<slot->count_14;++i)--slot->references_00[i]->index_08;
}
void destroy_native_lua_object_00b67700(NativeLuaObjectStorage& object){
    if(!object.kind_04)return;
    release_native_lua_tracked_object_00b66de0(object.owner_00,object,object.index_08,1);object.kind_04=0;
}
} // namespace bsp
