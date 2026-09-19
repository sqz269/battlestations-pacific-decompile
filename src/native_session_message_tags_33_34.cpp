#include "bsp/native_session_message_tags_33_34.hpp"
#include "bsp/native_bit_cursor_owned_string.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/singleton_lifetime.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using M33=NativeSessionMessage33;using M34=NativeSessionMessage34;
M33* __fastcall scalar33(M33* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage33Profile*>(m->base.profile_00);return delete_native_session_message33_00765d00(m,flags,*p.strings,p);}
M34* __fastcall scalar34(M34* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage34Profile*>(m->base.profile_00);return delete_native_session_message34_00765df0(m,flags,*p.strings,p);}
void __fastcall write33(const M33* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage33Profile*>(m->base.profile_00);write_native_session_message33_0076cd60(m,c,p.fallback_00e17669);}
void __fastcall read33(M33* m,void*,NativeSessionReadStream* s){const auto& p=*reinterpret_cast<const NativeSessionMessage33Profile*>(m->base.profile_00);read_native_session_message33_0076cdd0(m,s,*p.strings);}
void __fastcall write34(const M34* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage34Profile*>(m->base.profile_00);write_native_session_message34_0076ced0(m,c,p.fallback_00e17669);}
void __fastcall read34(M34* m,void*,NativeSessionReadStream* s){const auto& p=*reinterpret_cast<const NativeSessionMessage34Profile*>(m->base.profile_00);read_native_session_message34_0076cf00(m,s,*p.strings);}
bool __fastcall is33(const M33*,void*,U t){return native_session_message_is33_00765c40(t);}
bool __fastcall is34(const M34*,void*,U t){return native_session_message_is34_00765d80(t);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
}
NativeSessionMessage33Profile::NativeSessionMessage33Profile(NativeStringRawPoolContext& s,const char* fallback)
    :slots{reinterpret_cast<U>(&scalar33),reinterpret_cast<U>(&write33),reinterpret_cast<U>(&read33),reinterpret_cast<U>(&is33),reinterpret_cast<U>(&yes)},strings(&s),fallback_00e17669(fallback){}
NativeSessionMessage34Profile::NativeSessionMessage34Profile(NativeStringRawPoolContext& s,const char* fallback)
    :slots{reinterpret_cast<U>(&scalar34),reinterpret_cast<U>(&write34),reinterpret_cast<U>(&read34),reinterpret_cast<U>(&is34),reinterpret_cast<U>(&yes)},strings(&s),fallback_00e17669(fallback){}
M33* construct_native_session_message33_00765bd0(M33* m,const NativeSessionMessageContext& c,const NativeSessionMessage33Profile& p){
    construct_native_session_message_0075b430(&m->base,33,c);volatile auto& v=*m;
    v.base.delivery_04=1;v.base.profile_00=p.slots;
    v.length_18=0;v.data_1c=nullptr;v.length_28=0;v.data_2c=nullptr;v.length_30=0;v.data_34=nullptr;return m;
}
M34* construct_native_session_message34_00765d20(M34* m,const NativeSessionMessageContext& c,const NativeSessionMessage34Profile& p){
    construct_native_session_message_0075b430(&m->base,34,c);volatile auto& v=*m;
    v.base.delivery_04=1;v.base.profile_00=p.slots;v.length_18=0;v.data_1c=nullptr;return m;
}
void write_native_session_message33_0076cd60(const M33* m,NativeBitCursor* c,const char* fallback){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_owned_string_00429ac0(c,&m->length_18,fallback);
    write_native_unsigned_dword_bits_00429070(c,m->values_20[0],4);
    write_native_unsigned_dword_bits_00429070(c,m->values_20[1],4);
    write_native_owned_string_00429ac0(c,&m->length_28,fallback);
    write_native_owned_string_00429ac0(c,&m->length_30,fallback);
    const volatile U* words=m->words_38;const U high=words[1];const U low=words[0];
    write_native_u64_bits_00429180(c,low,high);
}
void read_native_session_message33_0076cdd0(M33* m,NativeSessionReadStream* s,NativeStringRawPoolContext& strings){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;
    read_native_owned_string_00429f20(c,&m->length_18,strings);
    read_native_u32_bits_00428d10(c,&m->values_20[0],4);
    read_native_u32_bits_00428d10(c,&m->values_20[1],4);
    read_native_owned_string_00429f20(c,&m->length_28,strings);
    read_native_owned_string_00429f20(c,&m->length_30,strings);
    read_native_u64_bits_00428e90(c,m->words_38);
}
void write_native_session_message34_0076ced0(const M34* m,NativeBitCursor* c,const char* fallback){
    write_native_session_message_type_00449940(&m->base,c);write_native_owned_string_00429ac0(c,&m->length_18,fallback);
}
void read_native_session_message34_0076cf00(M34* m,NativeSessionReadStream* s,NativeStringRawPoolContext& strings){
    read_native_session_message_type_00449960(&m->base,s);read_native_owned_string_00429f20(&s->cursor_04,&m->length_18,strings);
}
bool native_session_message_is33_00765c40(U t){return t==33;}
bool native_session_message_is34_00765d80(U t){return t==34;}
void destroy_native_session_message33_00765c50(M33* m,NativeStringRawPoolContext& strings,const NativeSessionMessage33Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    // DB87F8: state2 ->1 destroys28, state1 ->0 destroys18,
    // state0 ->-1 stampsroot. Each completed return advances the state.
    __try {destroy_native_string_header_0041dd20(&m->length_30,strings);}
    __finally {
        __try {destroy_native_string_header_0041dd20(&m->length_28,strings);}
        __finally {
            __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
            __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
        }
    }
}
void destroy_native_session_message34_00765d90(M34* m,NativeStringRawPoolContext& strings,const NativeSessionMessage34Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
    __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
}
M33* delete_native_session_message33_00765d00(M33* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage33Profile& p){
    destroy_native_session_message33_00765c50(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;
}
M34* delete_native_session_message34_00765df0(M34* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage34Profile& p){
    destroy_native_session_message34_00765d90(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;
}
} // namespace bsp
