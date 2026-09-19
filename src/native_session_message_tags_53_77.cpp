#include "bsp/native_session_message_tags_53_77.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_owned_string.hpp"
#include "bsp/singleton_lifetime.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using M53=NativeSessionMessage53;using M77=NativeSessionMessage77;
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
M77* __fastcall scalar77(M77* m,void*,U flags){return delete_native_session_message77_0075a010(m,flags);}
void __fastcall write77(const M77* m,void*,NativeBitCursor* c){write_native_session_message77_00759f90(m,c);}
void __fastcall read77(M77* m,void*,NativeSessionReadStream* s){read_native_session_message77_00759fd0(m,s);}
bool __fastcall is77(const M77* m,void*,U query){return native_session_message_is77_00759f70(m,query);}
const U profile77[]={reinterpret_cast<U>(&scalar77),reinterpret_cast<U>(&write77),reinterpret_cast<U>(&read77),reinterpret_cast<U>(&is77),reinterpret_cast<U>(&yes)};
M53* __fastcall scalar53(M53* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage53Profile*>(m->base.profile_00);return delete_native_session_message53_00733700(m,flags,*p.strings,p);}
void __fastcall write53(const M53* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage53Profile*>(m->base.profile_00);write_native_session_message53_00733640(m,c,p.fallback_00e17669);}
void __fastcall read53(M53* m,void*,NativeSessionReadStream* s){const auto& p=*reinterpret_cast<const NativeSessionMessage53Profile*>(m->base.profile_00);read_native_session_message53_00733670(m,s,*p.strings);}
bool __fastcall is53(const M53* m,void*,U query){return native_session_message_is53_00733620(m,query);}
}
const U* native_session_message77_profile_00d02cb8(){return profile77;}
M77* construct_native_session_message77_00759f40(M77* m){
    volatile auto& v=*m;v.base.field_08=0;v.base.field_0c=0;v.base.selected_owner_14=nullptr;
    v.base.type_10=0;v.base.delivery_04=1;v.base.profile_00=profile77;return m;
}
bool native_session_message_is77_00759f70(const M77* m,U query){
    if(query==77)return true;return query==static_cast<const volatile NativeSessionMessageStorage&>(m->base).type_10;
}
void write_native_session_message77_00759f90(const M77* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);const volatile auto& v=*m;
    write_native_unsigned_dword_bits_00429070(c,v.value_18,32);write_native_word_bits_00429120(c,v.value_1c,12);
}
void read_native_session_message77_00759fd0(M77* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    read_native_u32_bits_00428d10(&s->cursor_04,&m->value_18,32);read_native_word_bits_00428e30(&s->cursor_04,&m->value_1c,12);
}
M77* delete_native_session_message77_0075a010(M77* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
NativeSessionMessage53Profile::NativeSessionMessage53Profile(NativeStringRawPoolContext& s,const char* fallback)
 :slots{reinterpret_cast<U>(&scalar53),reinterpret_cast<U>(&write53),reinterpret_cast<U>(&read53),reinterpret_cast<U>(&is53),reinterpret_cast<U>(&yes)},strings(&s),fallback_00e17669(fallback){}
M53* create_native_session_message53_00733720(const NativeSessionMessageContext& c,const NativeSessionMessage53Profile& p){
    auto* m=static_cast<M53*>(singleton_lifetime_allocate({SingletonAllocationKind::object,0x20,sizeof(M53)}));
    if(m==nullptr)return nullptr;
    bool complete=false;
    __try {
        construct_native_session_message_0075b430(&m->base,53,c);volatile auto& v=*m;
        v.base.delivery_04=1;v.base.profile_00=p.slots;v.length_18=0;v.data_1c=nullptr;complete=true;
    } __finally {
        // DB4EC0 state0 -> -1 calls C85C20 to release the raw allocation.
        if(!complete)singleton_lifetime_free(m);
    }
    return m;
}
bool native_session_message_is53_00733620(const M53* m,U query){
    if(query==53)return true;return query==static_cast<const volatile NativeSessionMessageStorage&>(m->base).type_10;
}
void write_native_session_message53_00733640(const M53* m,NativeBitCursor* c,const char* fallback){
    write_native_session_message_type_00449940(&m->base,c);write_native_owned_string_00429ac0(c,&m->length_18,fallback);
}
void read_native_session_message53_00733670(M53* m,NativeSessionReadStream* s,NativeStringRawPoolContext& strings){
    read_native_session_message_type_00449960(&m->base,s);read_native_owned_string_00429f20(&s->cursor_04,&m->length_18,strings);
}
void destroy_native_session_message53_007336a0(M53* m,NativeStringRawPoolContext& strings,const NativeSessionMessage53Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
    __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
}
M53* delete_native_session_message53_00733700(M53* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage53Profile& p){
    destroy_native_session_message53_007336a0(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;
}
} // namespace bsp
