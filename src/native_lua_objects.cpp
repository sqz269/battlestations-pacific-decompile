#include "bsp/native_lua_objects.hpp"
#include "bsp/lua_numeric.hpp"
#include "bsp/gui_lua_reader.hpp"
#include <cstring>
#include <new>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <ldo.h>
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
namespace {
struct NamedLookupOperation {
    NativeLuaObjectStorage* table;
    void* fresh;
    const char* key;
    NativeLuaObjectStorage* result;
};
void named_lookup_operation(lua_State*,void* context) {
    auto& operation=*static_cast<NamedLookupOperation*>(context);
    operation.result=native_lua_get_by_name_00b67800(*operation.table,operation.fresh,operation.key);
}
} // namespace
NativeLuaObjectStorage* native_lua_get_by_name_protected(
    NativeLuaObjectStorage& table,void* fresh,const char* key){
    auto* const state=table.owner_00->state_04;
    const int entry_top=lua_gettop(state);
    NamedLookupOperation operation{&table,fresh,key,nullptr};
    // Pinned Lua5.1.1 private API: unlike lua_pcall/lua_cpcall this invokes the
    // trivial callback without introducing another Lua C frame. Include the
    // allocation in checkstack and key creation in the protected operation.
    const int status=luaD_pcall(state,&named_lookup_operation,&operation,
        savestack(state,state->top),state->errfunc);
    if(status){
        lua_settop(state,entry_top);
        throw NativeLuaOperationError{status};
    }
    return operation.result;
}
NativeLuaObjectStorage* native_lua_get_by_string_00b68100(
    NativeLuaObjectStorage& table,void* fresh,const NativeString& key){
    const char* text=key.data();if(!text)text="";native_lua_get_by_name_00b67800(table,fresh,text);
    return static_cast<NativeLuaObjectStorage*>(fresh);
}
bool native_lua_is_number_00b66050(const NativeLuaObjectStorage& object){
    return object.kind_04==2 && lua_type(object.owner_00->state_04,object.index_08)==LUA_TNUMBER;
}
NativeLuaObjectStorage* native_lua_call_frame_00b679b0(NativeLuaStateStorage& owner,void* fresh){
    auto* object=::new(fresh) NativeLuaObjectStorage;
    object->owner_00=&owner;object->kind_04=3;object->index_08=1;object->opaque_0c=0;object->tracked_10=0;
    object->opaque_0c=static_cast<std::uint32_t>(lua_gettop(owner.state_04));return object;
}
NativeLuaObjectStorage* native_lua_get_by_index_00b67720(NativeLuaObjectStorage& input,void* fresh,std::int32_t index){
    if(input.kind_04==2){
        (void)lua_checkstack(input.owner_00->state_04,1);lua_pushnumber(input.owner_00->state_04,static_cast<double>(index));
        lua_gettable(input.owner_00->state_04,input.index_08);
        const auto top=lua_gettop(input.owner_00->state_04);auto* const owner=input.owner_00;
        auto* object=::new(fresh) NativeLuaObjectStorage;
        object->index_08=top;object->owner_00=owner;object->kind_04=2;object->opaque_0c=0;object->tracked_10=1;
        const auto slot_index=static_cast<std::int32_t>(static_cast<std::uint32_t>(owner->stack_offset_0c)+static_cast<std::uint32_t>(top));
        if(owner->high_water_4c4<=slot_index)owner->high_water_4c4=slot_index+1;
        auto& slot=owner->slots_14[slot_index];slot.references_00[slot.count_14]=object;++slot.count_14;return object;
    }
    const auto base=input.index_08;auto* const owner=input.owner_00;
    auto* object=::new(fresh) NativeLuaObjectStorage;object->opaque_0c=0;object->tracked_10=0;object->owner_00=owner;object->kind_04=2;
    object->index_08=static_cast<std::int32_t>(static_cast<std::uint32_t>(base)+static_cast<std::uint32_t>(index));return object;
}
NativeLuaObjectStorage* native_lua_argument_at_00b677e0(NativeLuaObjectStorage& input,void* fresh,std::int32_t index){
    native_lua_get_by_index_00b67720(input,fresh,index);return static_cast<NativeLuaObjectStorage*>(fresh);
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
NativeLuaObjectStorage* assign_native_lua_object_00b67690(
    NativeLuaObjectStorage& destination,const NativeLuaObjectStorage& source){
    if(destination.kind_04){
        release_native_lua_tracked_object_00b66de0(
            destination.owner_00,destination,destination.index_08,1);
        destination.kind_04=0;
    }
    // Capture after release: source can alias destination or another tracked
    // object whose index was shifted by that release. Native leaves0C/padding.
    const auto tracked=source.tracked_10;
    const auto kind=source.kind_04;
    const auto index=source.index_08;
    auto* const owner=source.owner_00;
    destination.kind_04=kind;destination.owner_00=owner;
    destination.index_08=index;destination.tracked_10=tracked;
    if(tracked){
        const auto slot_index=static_cast<std::int32_t>(
            static_cast<std::uint32_t>(owner->stack_offset_0c)+static_cast<std::uint32_t>(index));
        if(owner->high_water_4c4<=slot_index)owner->high_water_4c4=slot_index+1;
        auto& slot=owner->slots_14[slot_index];
        slot.references_00[slot.count_14]=&destination;++slot.count_14;
    }
    return &destination;
}
bool native_lua_is_boolean_00b66000(const NativeLuaObjectStorage& object){
    return object.kind_04==2 && lua_type(object.owner_00->state_04,object.index_08)==LUA_TBOOLEAN;
}
bool native_lua_is_nil_00b65fb0(const NativeLuaObjectStorage& object){
    return object.kind_04==2 && lua_type(object.owner_00->state_04,object.index_08)==LUA_TNIL;
}
bool native_lua_boolean_00b66250(const NativeLuaObjectStorage& object){
    return lua_toboolean(object.owner_00->state_04,object.index_08)!=0;
}
bool native_lua_is_table_00b661b0(const NativeLuaObjectStorage& object){
    if(!object.kind_04)return false;
    return object.kind_04!=2 || lua_type(object.owner_00->state_04,object.index_08)==LUA_TTABLE;
}
bool native_lua_is_integer_number_00b66a60(const NativeLuaObjectStorage& object){
    return native_lua_is_number_00b66050(object) &&
        gui_lua_is_integer_number_00b66a60(lua_tonumber(object.owner_00->state_04,object.index_08));
}
const char* native_lua_string_00b662b0(const NativeLuaObjectStorage& object){
    return lua_tolstring(object.owner_00->state_04,object.index_08,nullptr);
}
namespace {
struct StringOperation {
    const NativeLuaObjectStorage* object;
    const char* result;
};
void string_operation(lua_State*,void* context) {
    auto& operation=*static_cast<StringOperation*>(context);
    operation.result=native_lua_string_00b662b0(*operation.object);
}
} // namespace
const char* native_lua_string_protected(const NativeLuaObjectStorage& object){
    auto* const state=object.owner_00->state_04;
    const int entry_top=lua_gettop(state);
    StringOperation operation{&object,nullptr};
    const int status=luaD_pcall(state,&string_operation,&operation,
        savestack(state,state->top),state->errfunc);
    if(status){
        // Allocation can fail before conversion; GC/finalizers can fail after
        // conversion. Keep the actual TValue and remove only the error slot.
        lua_settop(state,entry_top);
        throw NativeLuaOperationError{status};
    }
    return operation.result;
}
std::uint8_t native_lua_boolean_or_00b662f0(const NativeLuaObjectStorage& object,std::uint8_t fallback){
    if(!native_lua_is_boolean_00b66000(object))return fallback;
    return lua_toboolean(object.owner_00->state_04,object.index_08)!=0?1:0;
}
bool native_lua_is_unbound_00b66420(const NativeLuaObjectStorage& object) noexcept {return object.kind_04==0;}
bool native_lua_is_string_00b660a0(const NativeLuaObjectStorage& object){
    return object.kind_04==2 && lua_type(object.owner_00->state_04,object.index_08)==LUA_TSTRING;
}
float native_lua_number_or_00b66330(const NativeLuaObjectStorage& object,float fallback){
    return native_lua_is_number_00b66050(object)?native_lua_number_00b66270(object):fallback;
}
std::int32_t native_lua_integer_or_00b66380(const NativeLuaObjectStorage& object,std::int32_t fallback,const bool& mode){
    return native_lua_is_number_00b66050(object)?native_lua_integer_00b66290(object,mode):fallback;
}
NativeString* native_lua_string_or_00b685c0(const NativeLuaObjectStorage& object,void* fresh,const char* fallback,NativeStringStorage& strings){
    const char* text=fallback;
    if(object.kind_04==2 && lua_type(object.owner_00->state_04,object.index_08)==LUA_TSTRING)
        text=native_lua_string_00b662b0(object);
    auto* output=::new(fresh) NativeString;output->assign_0041e870(strings,text);return output;
}
namespace {
void publish_iteration_object(NativeLuaObjectStorage& table,NativeLuaObjectStorage& object,int index){
    auto* const owner=table.owner_00;
    object.owner_00=owner;object.kind_04=2;object.index_08=index;object.tracked_10=1;
    const auto slot_index=static_cast<std::int32_t>(static_cast<std::uint32_t>(owner->stack_offset_0c)+static_cast<std::uint32_t>(index));
    if(owner->high_water_4c4<=slot_index)owner->high_water_4c4=slot_index+1;
    auto& slot=owner->slots_14[slot_index];slot.references_00[slot.count_14]=&object;++slot.count_14;
}
void finish_iteration(NativeLuaObjectStorage& table,NativeLuaObjectStorage& key,NativeLuaObjectStorage& value){
    if(!lua_next(table.owner_00->state_04,table.index_08))return;
    const auto key_index=lua_gettop(table.owner_00->state_04)-1;publish_iteration_object(table,key,key_index);
    const auto value_index=lua_gettop(table.owner_00->state_04);publish_iteration_object(table,value,value_index);
}
void release_iteration_objects(NativeLuaObjectStorage& key,NativeLuaObjectStorage& value){
    destroy_native_lua_object_00b67700(value);destroy_native_lua_object_00b67700(key);
}
void start_iteration(NativeLuaObjectStorage& table,NativeLuaObjectStorage& key,NativeLuaObjectStorage& value){
    (void)lua_checkstack(table.owner_00->state_04,2);lua_pushnil(table.owner_00->state_04);
    finish_iteration(table,key,value);
}
void prepare_next_iteration(NativeLuaObjectStorage& table,NativeLuaObjectStorage& key,NativeLuaObjectStorage& value){
    destroy_native_lua_object_00b67700(value);
    const auto key_index=key.index_08;
    if(key_index==lua_gettop(table.owner_00->state_04)){
        if(key.kind_04){release_native_lua_tracked_object_00b66de0(key.owner_00,key,key_index,0);key.kind_04=0;}
    }else{
        lua_pushvalue(table.owner_00->state_04,key_index);
        if(key.kind_04){release_native_lua_tracked_object_00b66de0(key.owner_00,key,key.index_08,1);key.kind_04=0;}
    }
}
} // namespace
void native_lua_iterate_first_00b67080(NativeLuaObjectStorage& table,NativeLuaObjectStorage& key,NativeLuaObjectStorage& value){
    release_iteration_objects(key,value);
    start_iteration(table,key,value);
}
void native_lua_iterate_next_00b67190(NativeLuaObjectStorage& table,NativeLuaObjectStorage& key,NativeLuaObjectStorage& value){
    prepare_next_iteration(table,key,value);
    finish_iteration(table,key,value);
}
namespace {
struct IterationOperation {
    NativeLuaObjectStorage* table;
    NativeLuaObjectStorage* key;
    NativeLuaObjectStorage* value;
    bool first;
};
void iteration_operation(lua_State*,void* context) {
    auto& operation=*static_cast<IterationOperation*>(context);
    // lua_next can raise before it returns a pair. The following publication
    // uses nonallocating Lua getters and plain tracking stores only.
    if(operation.first)start_iteration(*operation.table,*operation.key,*operation.value);
    else finish_iteration(*operation.table,*operation.key,*operation.value);
}
void protect_iteration(IterationOperation& operation,int discard_top){
    auto* const state=operation.table->owner_00->state_04;
    const int failure_top=lua_gettop(state)-discard_top;
    const int status=luaD_pcall(state,&iteration_operation,&operation,
        savestack(state,state->top-discard_top),state->errfunc);
    if(status){
        // This is the post-release stack. Reinstating the entry height would
        // resurrect removed value/key slots without their tracking records.
        lua_settop(state,failure_top);
        throw NativeLuaOperationError{status};
    }
}
} // namespace
void native_lua_iterate_first_protected(NativeLuaObjectStorage& table,NativeLuaObjectStorage& key,NativeLuaObjectStorage& value){
    release_iteration_objects(key,value);
    IterationOperation operation{&table,&key,&value,true};
    protect_iteration(operation,0);
}
void native_lua_iterate_next_protected(NativeLuaObjectStorage& table,NativeLuaObjectStorage& key,NativeLuaObjectStorage& value){
    prepare_next_iteration(table,key,value);
    // The reader owns key/value exclusively. The top key is now detached;
    // retain all prior releases/shifts and consume this work slot on failure.
    IterationOperation operation{&table,&key,&value,false};
    protect_iteration(operation,1);
}
} // namespace bsp
