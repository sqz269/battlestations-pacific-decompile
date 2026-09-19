#include "bsp/native_session_message_tag_08.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
namespace bsp {
namespace {
using U=std::uint32_t;using M=NativeSessionMessage08;
M* __fastcall scalar(M* m,void*,U flags){
    const auto& p=*reinterpret_cast<const NativeSessionMessage08Profile*>(m->base.profile_00);
    return delete_native_session_message08_00766040(m,flags,*p.strings,p);
}
void __fastcall write(const M* m,void*,NativeBitCursor* c){
    const auto& p=*reinterpret_cast<const NativeSessionMessage08Profile*>(m->base.profile_00);
    write_native_session_message08_00765e80(m,c,p.empty_00f1af25);
}
void __fastcall read(M* m,void*,NativeSessionReadStream* s){
    const auto& p=*reinterpret_cast<const NativeSessionMessage08Profile*>(m->base.profile_00);
    read_native_session_message08_00765f50(m,s,*p.strings);
}
bool __fastcall is_type(const M*,void*,U t){return native_session_message_is08_00765e70(t);}
bool __fastcall yes(const M*,void*){return native_session_message_always_true_004499c0();}
void read_string(NativeBitCursor* c,U* length,char** data,NativeStringRawPoolContext& strings){
    char* temporary;
    allocate_native_bit_string_00428da0(c,&temporary);
    U count=0;
    if(temporary!=nullptr){
        const volatile char* end=temporary;
        while(*end!=0)++end;
        count=static_cast<U>(reinterpret_cast<std::uintptr_t>(end)-reinterpret_cast<std::uintptr_t>(temporary));
    }
    // No temporary cleanup on a resize exception: the native reader has no
    // local EH registration. Preserve its normal order, including equal-size
    // resize/no-data behavior and embedded-NUL truncation after wire consumption.
    resize_native_string_header_0041dd40(length,strings,count,false);
    char* const destination=*data;
    if(destination!=nullptr)std::memmove(destination,temporary,*length);
    singleton_lifetime_free(temporary); // native BF6989 array-delete free thunk
}
}
NativeSessionMessage08Profile::NativeSessionMessage08Profile(NativeStringRawPoolContext& s,const char* empty)
    :slots{reinterpret_cast<U>(&scalar),reinterpret_cast<U>(&write),reinterpret_cast<U>(&read),reinterpret_cast<U>(&is_type),reinterpret_cast<U>(&yes)},strings(&s),empty_00f1af25(empty){}
NativeSessionMessage08* construct_native_session_message08_00765e10(M* m,const NativeSessionMessageContext& c,const NativeSessionMessage08Profile& p){
    construct_native_session_message_0075b430(&m->base,8,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;
    v.first_length_18=0;v.first_data_1c=nullptr;v.second_length_20=0;v.second_data_24=nullptr;
    return m;
}
bool native_session_message_is08_00765e70(U type){return type==8;}
void write_native_session_message08_00765e80(const M* m,NativeBitCursor* c,const char* empty){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_signed_dword_bits_00429090(c,static_cast<U>(m->value_28),10);
    const char* first=m->first_data_1c;write_native_bit_string_004290d0(c,first!=nullptr?first:empty);
    const char* second=m->second_data_24;write_native_bit_string_004290d0(c,second!=nullptr?second:empty);
}
void read_native_session_message08_00765f50(M* m,NativeSessionReadStream* s,NativeStringRawPoolContext& strings){
    auto* c=&s->cursor_04;read_native_session_message_type_00449960(&m->base,s);
    read_native_i32_bits_00428d30(c,&m->value_28,10);
    read_string(c,&m->first_length_18,&m->first_data_1c,strings);
    read_string(c,&m->second_length_20,&m->second_data_24,strings);
}
void destroy_native_session_message08_00765ed0(M* m,NativeStringRawPoolContext& strings,const NativeSessionMessage08Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    // DB8860: state1 -> state0 destroys the first string via C88C08/41DD20;
    // state0 -> -1 stamps the root via C88C00/4499D0. Both apply if a getter
    // throws while returning the second string. Do not clear either header.
    __try {
        __try {
            destroy_native_string_header_0041dd20(&m->second_length_20,strings);
        } __finally {
            destroy_native_string_header_0041dd20(&m->first_length_18,strings);
        }
    } __finally {
        static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();
    }
}
NativeSessionMessage08* delete_native_session_message08_00766040(M* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage08Profile& p){
    destroy_native_session_message08_00765ed0(m,strings,p);
    if((flags&1u)!=0)singleton_lifetime_free(m);
    return m;
}
} // namespace bsp
