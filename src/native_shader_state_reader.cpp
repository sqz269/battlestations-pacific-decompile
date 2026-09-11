#include "bsp/native_shader_state_reader.hpp"
#include <cstring>
#include <Windows.h>
namespace bsp {
void read_native_shader_state_table_00b579b0(NativeShaderStateListStorage& output,
    NativeShaderStateDefinitionArray& definitions,NativeLuaObjectStorage& table,
    NativeStringStorage& strings,const bool& crt_sse2_conversion){
    auto payload=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&table));
    for(std::uint32_t index=0;index<static_cast<std::uint32_t>(definitions.count_04);++index){
        auto* source=reinterpret_cast<NativeShaderStateDefinition*>(
            reinterpret_cast<std::uintptr_t>(definitions.data_00)+index*16u);
        NativeShaderStateDefinition definition;
        copy_native_shader_state_definition_00b57630(&definition,*source,strings);
        __try {
            NativeLuaObjectStorage gate;native_lua_get_by_string_00b68100(table,&gate,definition.name_00);
            bool number=false;
            __try {number=native_lua_is_number_00b66050(gate);}
            __finally {destroy_native_lua_object_00b67700(gate);}
            if(number){
                if(definition.conversion_0c==0){
                    NativeLuaObjectStorage value;native_lua_get_by_string_00b68100(table,&value,definition.name_00);
                    __try {payload=static_cast<std::uint32_t>(native_lua_integer_00b66290(value,crt_sse2_conversion));}
                    __finally {destroy_native_lua_object_00b67700(value);}
                }else if(definition.conversion_0c==1){
                    NativeLuaObjectStorage value;native_lua_get_by_string_00b68100(table,&value,definition.name_00);
                    __try {const float converted=native_lua_number_00b66270(value);std::memcpy(&payload,&converted,4);}
                    __finally {destroy_native_lua_object_00b67700(value);}
                }
                append_unique_native_shader_state_pair_00b567b0(output,definition.state_08,payload);
            }
        } __finally {destroy_native_string_header_0041dd20(&definition.name_00,strings);}
    }
}
} // namespace bsp
