#include "bsp/native_lua_field_values.hpp"
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua field values require MSVC Win32.
#endif
namespace bsp {
namespace {
using Word=std::uint32_t;
void* at(const void* p,Word offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+offset);
}
Word word(const void* p,Word offset=0) noexcept {
    return *static_cast<const volatile Word*>(at(p,offset));
}
void put(void* p,Word value) noexcept { *static_cast<volatile Word*>(p)=value; }
void* pointer(const void* p,Word offset=0) noexcept { return reinterpret_cast<void*>(word(p,offset)); }
NativeLuaObjectStorage& object(void* p) noexcept { return *static_cast<NativeLuaObjectStorage*>(p); }
void copy_float(void* destination,const void* source) noexcept {
    __asm { mov eax,source }
    __asm { mov edx,destination }
    __asm { fld dword ptr [eax] }
    __asm { fstp dword ptr [edx] }
}
// Consume the actual ST0 return directly: a C++ float temporary could insert
// an extra FLD/FSTP and change denormal/NaN exception flags.
void number_into(const NativeLuaObjectStorage* input,void* destination) {
    __asm { push input }
    __asm { call native_lua_number_00b66270 }
    __asm { add esp,4 }
    __asm { mov eax,destination }
    __asm { fstp dword ptr [eax] }
}
} // namespace
void store_native_lua_field_value_00bd63b0(NativeLuaObjectStorage& input,
    void* field,NativeLuaFieldValueScratch& scratch,NativeLuaFieldValueBindings& bindings) {
    void* const s=scratch.bytes;
    // Offsets relative to s (native ESP+10h).
    void* const a=at(s,0x10);void* const b=at(s,0x24);void* const c=at(s,0x38);
    void* const d=at(s,0x4c);void* const e=at(s,0x60);
    switch(word(field)) {
    case 0: {
        void* const destination=pointer(field,4); // before Lua conversion
        const char* const source=native_lua_string_00b662b0(input);
        Word length=0;
        if(source) while(*static_cast<const volatile unsigned char*>(at(source,length))) ++length;
        resize_native_string_header_0041dd40(destination,bindings.strings,length,false);
        void* const current_data=pointer(destination,4);
        if(current_data) {
            const Word current_length=word(destination);
            if(current_length) std::memmove(current_data,source,current_length);
        }
        return;
    }
    case 1: {
        void* const destination=pointer(field,4);
        const auto value=native_lua_integer_00b66290(input,bindings.crt_sse2_conversion);
        put(destination,static_cast<Word>(value));return;
    }
    case 2: {
        void* const destination=pointer(field,4);number_into(&input,destination);return;
    }
    case 3: {
        void* const destination=pointer(field,4);
        const auto value=native_lua_boolean_00b66250(input);
        *static_cast<volatile unsigned char*>(destination)=static_cast<unsigned char>(value);return;
    }
    case 4: {
        if(native_lua_is_integer_number_00b66a60(input)) {
            const auto value=native_lua_integer_00b66290(input,bindings.crt_sse2_conversion);
            void* const destination=pointer(field,4);
            const Word result=bindings.resolver_0109ced4(value);put(destination,result);
        } else if(native_lua_is_table_00b661b0(input)) {
            void* const destination=pointer(field,4);
            const Word result=bindings.resolver_0109ced8(&input);put(destination,result);
        }
        return;
    }
    case 5: {
        auto* const third=native_lua_get_by_index_00b67720(input,c,3);
        auto* const second=native_lua_get_by_index_00b67720(input,b,2);
        auto* const first=native_lua_get_by_index_00b67720(input,a,1);
        number_into(first,s);number_into(second,at(s,4));number_into(third,at(s,8));
        void* const destination=pointer(field,4);
        copy_float(destination,s);copy_float(at(destination,4),at(s,4));copy_float(at(destination,8),at(s,8));
        destroy_native_lua_object_00b67700(object(a));
        destroy_native_lua_object_00b67700(object(b));
        destroy_native_lua_object_00b67700(object(c));return;
    }
    case 6: {
        auto* const second=native_lua_get_by_index_00b67720(input,b,2);
        auto* const first=native_lua_get_by_index_00b67720(input,c,1);
        number_into(first,at(s,8));number_into(second,at(s,4));
        void* const destination=pointer(field,4);
        copy_float(destination,at(s,8));copy_float(at(destination,4),at(s,4));
        destroy_native_lua_object_00b67700(object(c));
        destroy_native_lua_object_00b67700(object(b));return;
    }
    case 7: {
        void* destination=pointer(field,4); // captured before all table lookups
        for(std::int32_t row=1;row<=4;++row) {
            native_lua_get_by_index_00b67720(input,a,row);
            for(std::int32_t column=1;column<=4;++column) {
                auto* const value=native_lua_get_by_index_00b67720(object(a),c,column);
                number_into(value,destination); // store before releasing child
                destroy_native_lua_object_00b67700(object(c));destination=at(destination,4);
            }
            destroy_native_lua_object_00b67700(object(a));
        }
        return;
    }
    case 8: {
        auto* const fourth=native_lua_get_by_index_00b67720(input,e,4);
        auto* const third=native_lua_get_by_index_00b67720(input,d,3);
        auto* const second=native_lua_get_by_index_00b67720(input,b,2);
        put(at(s,8),reinterpret_cast<Word>(second));
        auto* const first=native_lua_get_by_index_00b67720(input,c,1);
        number_into(first,at(s,4));
        number_into(static_cast<NativeLuaObjectStorage*>(pointer(s,8)),at(s,8));
        number_into(third,s);number_into(fourth,at(s,12));
        // The first FLD precedes reading the current destination pointer.
        void* destination;
        __asm { mov edx,s }
        __asm { fld dword ptr [edx+4] }
        __asm { mov ecx,field }
        __asm { mov eax,dword ptr [ecx+4] }
        __asm { mov destination,eax }
        __asm { fstp dword ptr [edx+10h] }
        // Subsequent FLDs precede the previous component's integer store.
        __asm { mov ecx,dword ptr [edx+10h] }
        __asm { fld dword ptr [edx+8] }
        __asm { mov dword ptr [eax],ecx }
        __asm { fstp dword ptr [edx+14h] }
        __asm { mov ecx,dword ptr [edx+14h] }
        __asm { fld dword ptr [edx] }
        __asm { mov dword ptr [eax+4],ecx }
        __asm { fstp dword ptr [edx+18h] }
        __asm { mov ecx,dword ptr [edx+18h] }
        __asm { fld dword ptr [edx+0ch] }
        __asm { mov dword ptr [eax+8],ecx }
        __asm { fstp dword ptr [edx+1ch] }
        __asm { mov ecx,dword ptr [edx+1ch] }
        __asm { mov dword ptr [eax+0ch],ecx }
        (void)destination;
        destroy_native_lua_object_00b67700(object(c));
        destroy_native_lua_object_00b67700(object(b));
        destroy_native_lua_object_00b67700(object(d));
        destroy_native_lua_object_00b67700(object(e));return;
    }
    case 10: {
        const char* const text=native_lua_string_00b662b0(input);
        // BF8417 delegates to strtol(text,nullptr,10); MSVC long is signed32.
        const long parsed=std::strtol(text,nullptr,10);
        put(at(s,12),static_cast<Word>(parsed));
        void* const destination=pointer(field,4);put(destination,word(s,12));return;
    }
    default:return;
    }
}
} // namespace bsp
