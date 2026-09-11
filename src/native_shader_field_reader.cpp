#include "bsp/native_shader_field_reader.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <new>
#include <stdexcept>
#include <Windows.h>
namespace bsp {
void reserve_native_shader_field_pointers_00b34680(NativeShaderDescriptorArray& rows,std::int32_t request){
    if(request<10)request=10;if(request<=rows.capacity_08)return;
    const auto bytes=static_cast<std::uint32_t>(request)*4u;
    auto* const data=static_cast<void**>(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));
    auto cursor=reinterpret_cast<std::uintptr_t>(data);
    for(std::int32_t i=0;i<rows.count_04;++i,cursor+=4u)
        if(cursor)*reinterpret_cast<void**>(cursor)=static_cast<void**>(rows.data_00)[i];
    singleton_lifetime_free(rows.data_00);rows.data_00=data;rows.capacity_08=request;
}
NativeShaderFieldStorage* read_native_shader_field_00b573f0(NativeLuaObjectStorage& table,
    NativeStringStorage& strings,const bool& mode,NativeShaderFieldStackPreimage frame){
    NativeShaderFieldStorage* output=nullptr;
    NativeLuaObjectStorage key;construct_native_lua_object_00b65f50(&key);
    __try {
        NativeLuaObjectStorage value;construct_native_lua_object_00b65f50(&value);
        __try {
            NativeString name;std::uint32_t captured_length=0;char* captured_data=nullptr;
            bool name_live=true;std::uint32_t ordinal=0;std::int32_t semantic_index=0;
            __try {
                native_lua_iterate_first_00b67080(table,key,value);
                while(!native_lua_is_unbound_00b66420(value)){
                    if(native_lua_is_integer_number_00b66a60(key)){
                        if(ordinal==0){
                            NativeString temporary;native_lua_string_or_00b685c0(value,&temporary,"undef",strings);
                            __try {
                                copy_native_string_header_00be0a30_fragment(&name,strings,&temporary);
                                captured_length=name.length();captured_data=name.data();
                            } __finally {destroy_native_string_header_0041dd20(&temporary,strings);}
                        }else if(ordinal==1)frame.scalar_type=native_lua_integer_00b66290(value,mode);
                        else if(ordinal==2)frame.component_count=native_lua_integer_or_00b66380(value,0,mode);
                        else if(ordinal==3)frame.semantic=native_lua_integer_00b66290(value,mode);
                        else if(ordinal==4)semantic_index=native_lua_integer_or_00b66380(value,0,mode);
                        ++ordinal;
                    }
                    native_lua_iterate_next_00b67190(table,key,value);
                }
                void* const raw=singleton_lifetime_allocate({SingletonAllocationKind::object,0x1c,0x1c});
                bool complete=false;
                __try {
                    if(raw){
                        output=::new(raw) NativeShaderFieldStorage;
                        resize_native_string_header_0041dd40(&output->name_00,strings,captured_length,true);
                        if(captured_length && output->name_00.length())std::memcpy(output->name_00.data(),captured_data,output->name_00.length());
                        output->scalar_type_08=frame.scalar_type;output->component_count_0c=frame.component_count;
                        output->component_mask_10=0;output->semantic_14=frame.semantic;output->semantic_index_18=semantic_index;
                    }
                    complete=true;
                } __finally {if(!complete)singleton_lifetime_free(raw);}
                name_live=false;
                if(captured_data)strings.release(captured_data,captured_length+1u);
            } __finally {if(name_live)destroy_native_string_header_0041dd20(&name,strings);}
        } __finally {destroy_native_lua_object_00b67700(value);}
    } __finally {destroy_native_lua_object_00b67700(key);}
    return output;
}
void append_native_shader_field_table_00b419b0(NativeLuaObjectStorage& shader,
    NativeShaderDescriptorArray& rows,const NativeString& field,NativeStringStorage& strings,
    const bool& mode,const NativeShaderFieldStackInputs& stack){
    if(!stack.for_entry)throw std::invalid_argument("field reader requires explicit native stack preimage inputs");
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
                    const auto preimage=stack.for_entry(stack.context,value);
                    auto* const entry=read_native_shader_field_00b573f0(value,strings,mode,preimage);
                    if(rows.count_04==rows.capacity_08){
                        auto next=static_cast<std::int32_t>(static_cast<std::uint32_t>(rows.capacity_08)+5u);
                        if(next<=10)next=10;reserve_native_shader_field_pointers_00b34680(rows,next);
                    }
                    const auto slot=reinterpret_cast<std::uintptr_t>(rows.data_00)+static_cast<std::uint32_t>(rows.count_04)*4u;
                    if(slot)*reinterpret_cast<NativeShaderFieldStorage**>(slot)=entry;
                    ++rows.count_04;native_lua_iterate_next_00b67190(table,key,value);
                }
            } __finally {destroy_native_lua_object_00b67700(value);}
        } __finally {destroy_native_lua_object_00b67700(key);}
    } __finally {destroy_native_lua_object_00b67700(table);}
}
} // namespace bsp
