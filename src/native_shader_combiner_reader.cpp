#include "bsp/native_shader_combiner_reader.hpp"
#include <cstring>
#include <stdexcept>
#include <Windows.h>
namespace bsp {
void read_native_shader_combiner_00b437f0(NativeString* slots,NativeLuaObjectStorage& entry,
    NativeStringStorage& strings,const bool& numeric_mode,std::int32_t mode){
    NativeLuaObjectStorage key;construct_native_lua_object_00b65f50(&key);
    __try {
        NativeLuaObjectStorage value;construct_native_lua_object_00b65f50(&value);
        __try {
            NativeString name;std::uint32_t cached_length=0,ordinal=0;bool name_live=true;
            __try {
                native_lua_iterate_first_00b67080(entry,key,value);
                while(!native_lua_is_unbound_00b66420(value)){
                    if(native_lua_is_integer_number_00b66a60(key)){
                        if(ordinal==0)mode=native_lua_integer_or_00b66380(value,13,numeric_mode);
                        else if(ordinal==1){
                            NativeString temporary;native_lua_string_or_00b685c0(value,&temporary,"",strings);
                            __try {
                                copy_native_string_header_00be0a30_fragment(&name,strings,&temporary);cached_length=name.length();
                            } __finally {destroy_native_string_header_0041dd20(&temporary,strings);}
                        }
                    }
                    ++ordinal;native_lua_iterate_next_00b67190(entry,key,value);
                }
                auto* const destination=reinterpret_cast<NativeString*>(reinterpret_cast<std::uintptr_t>(slots)+static_cast<std::uint32_t>(mode)*8u);
                if(destination!=&name){
                    resize_native_string_header_0041dd40(destination,strings,cached_length,true);
                    if(cached_length && destination->length())std::memcpy(destination->data(),name.data(),destination->length());
                }
                char* const data=name.data();name_live=false;if(data)strings.release(data,cached_length+1u);
            } __finally {if(name_live)destroy_native_string_header_0041dd20(&name,strings);}
        } __finally {destroy_native_lua_object_00b67700(value);}
    } __finally {destroy_native_lua_object_00b67700(key);}
}
void read_native_shader_combiner_table_00b439c0(NativeString* slots,NativeLuaObjectStorage& shader,
    const NativeString& field,NativeStringStorage& strings,const bool& mode,const NativeShaderCombinerStackInputs& stack){
    if(!stack.mode_for_entry)throw std::invalid_argument("combiner reader requires explicit native mode stack inputs");
    NativeLuaObjectStorage gate;native_lua_get_by_string_00b68100(shader,&gate,field);bool present=false;
    __try {present=native_lua_is_table_00b661b0(gate);} __finally {destroy_native_lua_object_00b67700(gate);}
    if(!present)return;
    NativeLuaObjectStorage table;native_lua_get_by_string_00b68100(shader,&table,field);
    __try {
        NativeLuaObjectStorage key;construct_native_lua_object_00b65f50(&key);
        __try {
            NativeLuaObjectStorage value;construct_native_lua_object_00b65f50(&value);
            __try {
                native_lua_iterate_first_00b67080(table,key,value);
                while(!native_lua_is_unbound_00b66420(value)){
                    if(native_lua_is_integer_number_00b66a60(key))
                        read_native_shader_combiner_00b437f0(slots,value,strings,mode,stack.mode_for_entry(stack.context,value));
                    native_lua_iterate_next_00b67190(table,key,value);
                }
            } __finally {destroy_native_lua_object_00b67700(value);}
        } __finally {destroy_native_lua_object_00b67700(key);}
    } __finally {destroy_native_lua_object_00b67700(table);}
}
} // namespace bsp
