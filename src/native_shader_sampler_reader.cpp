#include "bsp/native_shader_sampler_reader.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <Windows.h>
namespace bsp {
void reserve_native_shader_sampler_pointers_00b34620(NativeShaderDescriptorArray& rows,std::int32_t request){
    if(request<6)request=6;if(request<=rows.capacity_08)return;
    const auto bytes=static_cast<std::uint32_t>(request)*4u;
    auto* const data=static_cast<void**>(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));
    auto cursor=reinterpret_cast<std::uintptr_t>(data);
    for(std::int32_t i=0;i<rows.count_04;++i,cursor+=4u)
        if(cursor)*reinterpret_cast<void**>(cursor)=static_cast<void**>(rows.data_00)[i];
    singleton_lifetime_free(rows.data_00);rows.data_00=data;rows.capacity_08=request;
}
namespace {
using Predicate=bool (*)(const NativeLuaObjectStorage&);
bool gate(NativeLuaObjectStorage& table,const char* field,Predicate predicate){
    NativeLuaObjectStorage value;native_lua_get_by_name_00b67800(table,&value,field);bool result=false;
    __try {result=predicate(value);} __finally {destroy_native_lua_object_00b67700(value);}
    return result;
}
void read_name(NativeLuaObjectStorage& table,const char* field,NativeString& output,NativeStringStorage& strings){
    NativeLuaObjectStorage value;native_lua_get_by_name_00b67800(table,&value,field);
    __try {
        NativeString temporary;temporary.assign_0041e870(strings,native_lua_string_00b662b0(value));
        __try {copy_native_string_header_00be0a30_fragment(&output,strings,&temporary);}
        __finally {destroy_native_string_header_0041dd20(&temporary,strings);}
    } __finally {destroy_native_lua_object_00b67700(value);}
}
void read_integer(NativeLuaObjectStorage& table,const char* field,std::int32_t& output,const bool& mode){
    NativeLuaObjectStorage value;native_lua_get_by_name_00b67800(table,&value,field);
    __try {output=native_lua_integer_00b66290(value,mode);} __finally {destroy_native_lua_object_00b67700(value);}
}
NativeShaderStateListStorage* read_states(NativeLuaObjectStorage& entry,const char* field,
    NativeShaderStateDefinitionArray& definitions,NativeShaderStateListPool& pool,NativeStringStorage& strings,const bool& mode){
    const bool table=gate(entry,field,&native_lua_is_table_00b661b0);
    void* const slot=pool.allocate_slot_00b62630();
    auto* const output=slot?initialize_native_shader_state_list_00b621a0(slot):nullptr;
    if(table){
        NativeLuaObjectStorage value;native_lua_get_by_name_00b67800(entry,&value,field);
        __try {read_native_shader_state_table_00b579b0(*output,definitions,value,strings,mode);}
        __finally {destroy_native_lua_object_00b67700(value);}
    }
    return output;
}
}
NativeShaderSamplerStorage* read_native_shader_sampler_00b57b50(
    NativeShaderStateDefinitionsStorage& definitions,NativeLuaObjectStorage& entry,
    NativeShaderStateListPool& pool,NativeStringStorage& strings,const bool& mode){
    void* const raw=singleton_lifetime_allocate({SingletonAllocationKind::object,0x2c,0x2c});
    auto* const output=raw?initialize_native_shader_sampler_00b57b50_fragment(raw):nullptr;
    read_name(entry,"Name",output->name_04,strings);
    read_integer(entry,"Type",output->type_10,mode);
    if(gate(entry,"TextureSource",&native_lua_is_integer_number_00b66a60)){
        std::int32_t source;read_integer(entry,"TextureSource",source,mode);output->texture_source_14=source;
        if(source==1 || source==3)read_name(entry,"TextureSourceName",output->source_name_18,strings);
    }
    if(gate(entry,"Index",&native_lua_is_integer_number_00b66a60))read_integer(entry,"Index",output->index_20,mode);
    if(gate(entry,"VertexSampler",&native_lua_is_boolean_00b66000)){
        NativeLuaObjectStorage value;native_lua_get_by_name_00b67800(entry,&value,"VertexSampler");
        __try {output->vertex_sampler_0c=native_lua_boolean_or_00b662f0(value,0);}
        __finally {destroy_native_lua_object_00b67700(value);}
    }
    output->sampler_states_24=read_states(entry,"SamplerStates",definitions.sampler_10,pool,strings,mode);
    output->texture_stage_states_28=read_states(entry,"TextureStageStates",definitions.texture_stage_1c,pool,strings,mode);
    return output;
}
void read_native_shader_samplers_00b41830(NativeShaderDescriptorStorage& descriptor,
    NativeLuaObjectStorage& shader,NativeShaderStateDefinitionsStorage* volatile& published,
    NativeShaderSamplerClassBinding& binding,const bool& mode){
    if(!gate(shader,"Samplers",&native_lua_is_table_00b661b0))return;
    NativeLuaObjectStorage table;native_lua_get_by_name_00b67800(shader,&table,"Samplers");
    __try {
        NativeLuaObjectStorage key;construct_native_lua_object_00b65f50(&key);
        __try {
            NativeLuaObjectStorage value;construct_native_lua_object_00b65f50(&value);
            __try {
                native_lua_iterate_first_00b67080(table,key,value);
                while(!native_lua_is_unbound_00b66420(value)){
                    if(native_lua_is_integer_number_00b66a60(key)){
                        auto* const sampler=read_native_shader_sampler_00b57b50(*published,value,binding.pool(),binding.strings(),mode);
                        binding.bind(*sampler);
                        auto& rows=descriptor.virtual_owners_c4;
                        if(rows.count_04==rows.capacity_08){
                            auto next=static_cast<std::int32_t>(static_cast<std::uint32_t>(rows.capacity_08)+4u);
                            if(next<=6)next=6;reserve_native_shader_sampler_pointers_00b34620(rows,next);
                        }
                        const auto slot=reinterpret_cast<std::uintptr_t>(rows.data_00)+static_cast<std::uint32_t>(rows.count_04)*4u;
                        if(slot)*reinterpret_cast<NativeShaderSamplerStorage**>(slot)=sampler;
                        ++rows.count_04;
                    }
                    native_lua_iterate_next_00b67190(table,key,value);
                }
            } __finally {destroy_native_lua_object_00b67700(value);}
        } __finally {destroy_native_lua_object_00b67700(key);}
    } __finally {destroy_native_lua_object_00b67700(table);}
}
} // namespace bsp
