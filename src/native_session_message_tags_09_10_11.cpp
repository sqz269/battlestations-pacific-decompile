#include "bsp/native_session_message_tags_09_10_11.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
namespace bsp {
namespace {
using U=std::uint32_t;using M9=NativeSessionMessage09;using M10=NativeSessionMessage10;using M11=NativeSessionMessage11;
M9* __fastcall scalar09(M9* m,void*,U flags){
    const auto& p=*reinterpret_cast<const NativeSessionMessage09Profile*>(m->base.profile_00);
    return delete_native_session_message09_007663d0(m,flags,*p.strings,p);
}
M10* __fastcall scalar10(M10* m,void*,U flags){return delete_native_session_message10_0075e610(m,flags);}
M11* __fastcall scalar11(M11* m,void*,U flags){return delete_native_session_message11_0075e720(m,flags);}
void __fastcall write09(const M9* m,void*,NativeBitCursor* c){
    const auto& p=*reinterpret_cast<const NativeSessionMessage09Profile*>(m->base.profile_00);
    write_native_session_message09_00766240(m,c,p.empty_00f1af25);
}
void __fastcall read09(M9* m,void*,NativeSessionReadStream* s){
    const auto& p=*reinterpret_cast<const NativeSessionMessage09Profile*>(m->base.profile_00);
    read_native_session_message09_00766300(m,s,*p.strings);
}
void __fastcall write10(const M10* m,void*,NativeBitCursor* c){write_native_session_message10_0075e5d0(m,c);}
void __fastcall read10(M10* m,void*,NativeSessionReadStream* s){read_native_session_message10_0075e5f0(m,s);}
void __fastcall write11(const M11* m,void*,NativeBitCursor* c){write_native_session_message11_0075e6c0(m,c);}
void __fastcall read11(M11* m,void*,NativeSessionReadStream* s){read_native_session_message11_0075e6f0(m,s);}
bool __fastcall is09(const M9*,void*,U t){return native_session_message_is09_00766230(t);}
bool __fastcall is10(const M10*,void*,U t){return native_session_message_is10_0075e5c0(t);}
bool __fastcall is11(const M11*,void*,U t){return native_session_message_is11_0075e6b0(t);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
const U profile10[]={reinterpret_cast<U>(&scalar10),reinterpret_cast<U>(&write10),reinterpret_cast<U>(&read10),reinterpret_cast<U>(&is10),reinterpret_cast<U>(&yes)};
const U profile11[]={reinterpret_cast<U>(&scalar11),reinterpret_cast<U>(&write11),reinterpret_cast<U>(&read11),reinterpret_cast<U>(&is11),reinterpret_cast<U>(&yes)};
}
NativeSessionMessage09Profile::NativeSessionMessage09Profile(NativeStringRawPoolContext& s,const char* empty)
    :slots{reinterpret_cast<U>(&scalar09),reinterpret_cast<U>(&write09),reinterpret_cast<U>(&read09),reinterpret_cast<U>(&is09),reinterpret_cast<U>(&yes)},strings(&s),empty_00f1af25(empty){}
const U* native_session_message10_profile_00d03138(){return profile10;}
const U* native_session_message11_profile_00d0314c(){return profile11;}
NativeSessionMessage09* construct_native_session_message09_007661d0(M9* m,const NativeSessionMessageContext& c,const NativeSessionMessage09Profile& p){
    construct_native_session_message_0075b430(&m->base,9,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;v.length_18=0;v.data_1c=nullptr;
    return m;
}
NativeSessionMessage10* construct_native_session_message10_0075e540(M10* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(m,10,c);
    static_cast<volatile M10&>(*m).delivery_04=1;static_cast<volatile M10&>(*m).profile_00=profile10;return m;
}
NativeSessionMessage11* construct_native_session_message11_0075e630(M11* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,11,c);
    static_cast<volatile NativeSessionMessageStorage&>(m->base).delivery_04=1;
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=profile11;return m;
}
void write_native_session_message09_00766240(const M9* m,NativeBitCursor* c,const char* empty){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_unsigned_dword_bits_00429070(c,m->value_20,8);
    const char* text=m->data_1c;write_native_bit_string_004290d0(c,text!=nullptr?text:empty);
    const volatile U* words=m->words_28;
    write_native_unsigned_dword_bits_00429070(c,words[0],32);
    const U low_again=words[0];const U high=words[1];(void)low_again;
    write_native_unsigned_dword_bits_00429070(c,high,32); // original __aullshr32
}
void read_native_session_message09_00766300(M9* m,NativeSessionReadStream* s,NativeStringRawPoolContext& strings){
    auto* c=&s->cursor_04;read_native_session_message_type_00449960(&m->base,s);
    read_native_u32_bits_00428d10(c,&m->value_20,8);
    char* temporary;allocate_native_bit_string_00428da0(c,&temporary);
    U count=0;
    if(temporary!=nullptr){
        const volatile char* end=temporary;while(*end!=0)++end;
        count=static_cast<U>(reinterpret_cast<std::uintptr_t>(end)-reinterpret_cast<std::uintptr_t>(temporary));
    }
    // Preserve the native lack of local temporary cleanup if resize escapes.
    resize_native_string_header_0041dd40(&m->length_18,strings,count,false);
    char* const destination=m->data_1c;
    if(destination!=nullptr)std::memmove(destination,temporary,m->length_18);
    singleton_lifetime_free(temporary);
    // The post-free native tail remains part of the reader: low/zero-high
    // stores precede the second read, then __allmul(high,0,0,1) and ADD/ADC.
    U value;read_native_u32_bits_00428d10(c,&value,32);
    volatile U* words=m->words_28;words[0]=value;words[1]=0;
    read_native_u32_bits_00428d10(c,&value,32);
    words[0]=words[0]+0u;words[1]=words[1]+value;
}
void write_native_session_message10_0075e5d0(const M10* m,NativeBitCursor* c){write_native_session_message_type_00449940(m,c);}
void read_native_session_message10_0075e5f0(M10* m,NativeSessionReadStream* s){read_native_session_message_type_00449960(m,s);}
void write_native_session_message11_0075e6c0(const M11* m,NativeBitCursor* c){write_native_session_message_type_00449940(&m->base,c);write_native_unsigned_dword_bits_00429070(c,m->value_18,32);}
void read_native_session_message11_0075e6f0(M11* m,NativeSessionReadStream* s){read_native_session_message_type_00449960(&m->base,s);read_native_u32_bits_00428d10(&s->cursor_04,&m->value_18,32);}
bool native_session_message_is09_00766230(U type){return type==9;}
bool native_session_message_is10_0075e5c0(U type){return type==10;}
bool native_session_message_is11_0075e6b0(U type){return type==11;}
void destroy_native_session_message09_007662a0(M9* m,NativeStringRawPoolContext& strings,const NativeSessionMessage09Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    // DB88C0 state0 -> -1 invokes C88C40/4499D0 on unwind as well.
    __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
    __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
}
NativeSessionMessage09* delete_native_session_message09_007663d0(M9* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage09Profile& p){
    destroy_native_session_message09_007662a0(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;
}
NativeSessionMessage10* delete_native_session_message10_0075e610(M10* m,U flags){return delete_native_session_message_root_00449910(m,flags);}
NativeSessionMessage11* delete_native_session_message11_0075e720(M11* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
} // namespace bsp
