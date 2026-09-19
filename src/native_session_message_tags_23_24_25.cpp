#include "bsp/native_session_message_tags_23_24_25.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_bit_cursor_owned_string.hpp"
#include "bsp/singleton_lifetime.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using M23=NativeSessionMessage23;using M24=NativeSessionMessage24;using M25=NativeSessionMessage25;
M23* __fastcall scalar23(M23* m,void*,U f){return delete_native_session_message23_0075e120(m,f);}
M24* __fastcall scalar24(M24* m,void*,U f){const auto& p=*reinterpret_cast<const NativeSessionMessage24Profile*>(m->base.profile_00);return delete_native_session_message24_007661b0(m,f,*p.strings,p);}
M25* __fastcall scalar25(M25* m,void*,U f){return delete_native_session_message25_0075db50(m,f);}
void __fastcall write23(const M23* m,void*,NativeBitCursor* c){write_native_session_message23_0075e0a0(m,c);}
void __fastcall read23(M23* m,void*,NativeSessionReadStream* s){read_native_session_message23_0075e0e0(m,s);}
void __fastcall write24(const M24* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage24Profile*>(m->base.profile_00);write_native_session_message24_007660d0(m,c,p.fallback_00e17669);}
void __fastcall read24(M24* m,void*,NativeSessionReadStream* s){const auto& p=*reinterpret_cast<const NativeSessionMessage24Profile*>(m->base.profile_00);read_native_session_message24_00766110(m,s,*p.strings);}
void __fastcall write25(const M25* m,void*,NativeBitCursor* c){write_native_session_message25_0075dad0(m,c);}
void __fastcall read25(M25* m,void*,NativeSessionReadStream* s){read_native_session_message25_0075db10(m,s);}
bool __fastcall is23(const M23*,void*,U t){return native_session_message_is23_0075e090(t);}
bool __fastcall is24(const M24*,void*,U t){return native_session_message_is24_007660c0(t);}
bool __fastcall is25(const M25*,void*,U t){return native_session_message_is25_0075dac0(t);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
const U profile23[]={reinterpret_cast<U>(&scalar23),reinterpret_cast<U>(&write23),reinterpret_cast<U>(&read23),reinterpret_cast<U>(&is23),reinterpret_cast<U>(&yes)};
const U profile25[]={reinterpret_cast<U>(&scalar25),reinterpret_cast<U>(&write25),reinterpret_cast<U>(&read25),reinterpret_cast<U>(&is25),reinterpret_cast<U>(&yes)};
}
NativeSessionMessage24Profile::NativeSessionMessage24Profile(NativeStringRawPoolContext& s,const char* fallback)
    :slots{reinterpret_cast<U>(&scalar24),reinterpret_cast<U>(&write24),reinterpret_cast<U>(&read24),reinterpret_cast<U>(&is24),reinterpret_cast<U>(&yes)},strings(&s),fallback_00e17669(fallback){}
const U* native_session_message23_profile_00d030e8(){return profile23;}
const U* native_session_message25_profile_00d03070(){return profile25;}
NativeSessionMessage23* construct_native_session_message23_0075e000(M23* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,23,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile23;
    for(U i=0;i<14;++i)v.values_18[i]=0;return m;
}
NativeSessionMessage24* construct_native_session_message24_00766060(M24* m,const NativeSessionMessageContext& c,const NativeSessionMessage24Profile& p){
    construct_native_session_message_0075b430(&m->base,24,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;v.length_18=0;v.data_1c=nullptr;return m;
}
NativeSessionMessage25* construct_native_session_message25_0075daa0(M25* m,U type,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,type,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile25;return m;
}
void write_native_session_message23_0075e0a0(const M23* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    for(U i=0;i<14;++i)write_native_unsigned_dword_bits_00429070(c,m->values_18[i],4);
}
void read_native_session_message23_0075e0e0(M23* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    for(U i=0;i<14;++i)read_native_u32_bits_00428d10(&s->cursor_04,&m->values_18[i],4);
}
void write_native_session_message24_007660d0(const M24* m,NativeBitCursor* c,const char* fallback){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_owned_string_00429ac0(c,&m->length_18,fallback);
    write_native_signed_dword_bits_00429090(c,static_cast<U>(m->value_20),32);
}
void read_native_session_message24_00766110(M24* m,NativeSessionReadStream* s,NativeStringRawPoolContext& strings){
    read_native_session_message_type_00449960(&m->base,s);
    read_native_owned_string_00429f20(&s->cursor_04,&m->length_18,strings);
    read_native_i32_bits_00428d30(&s->cursor_04,&m->value_20,32);
}
void write_native_session_message25_0075dad0(const M25* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_signed_dword_bits_00429090(c,static_cast<U>(m->values_18[0]),6);
    write_native_signed_dword_bits_00429090(c,static_cast<U>(m->values_18[1]),6);
}
void read_native_session_message25_0075db10(M25* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    read_native_i32_bits_00428d30(&s->cursor_04,&m->values_18[0],6);
    read_native_i32_bits_00428d30(&s->cursor_04,&m->values_18[1],6);
}
bool native_session_message_is23_0075e090(U t){return t==23;}
bool native_session_message_is24_007660c0(U t){return t==24;}
bool native_session_message_is25_0075dac0(U t){return t==25;}
void destroy_native_session_message24_00766150(M24* m,NativeStringRawPoolContext& strings,const NativeSessionMessage24Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    // DB8894 state0 -> -1 invokes C88C20/4499D0 on unwind as well.
    __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
    __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
}
NativeSessionMessage23* delete_native_session_message23_0075e120(M23* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
NativeSessionMessage24* delete_native_session_message24_007661b0(M24* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage24Profile& p){
    destroy_native_session_message24_00766150(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;
}
NativeSessionMessage25* delete_native_session_message25_0075db50(M25* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
} // namespace bsp
