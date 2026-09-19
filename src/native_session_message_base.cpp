#include "bsp/native_session_message_base.hpp"
#include "bsp/singleton_lifetime.hpp"
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native session message profiles require MSVC Win32.
#endif
extern "C" int __cdecl _purecall();
namespace bsp {
namespace {
using U=std::uint32_t;using B=std::uint8_t;using M=NativeSessionMessageStorage;
M* __fastcall root_delete(M* m,void*,U f){return delete_native_session_message_root_00449910(m,f);}
M* __fastcall base_delete(M* m,void*,U f){return delete_native_session_message_base_00759cd0(m,f);}
M* __fastcall one_delete(M* m,void*,U f){return delete_native_session_message_one_004499e0(m,f);}
M* __fastcall zero_delete(M* m,void*,U f){return delete_native_session_message_zero_0075b880(m,f);}
M* __fastcall two_delete(M* m,void*,U f){return delete_native_session_message_two_0075b8c0(m,f);}
void __fastcall write_type(const M* m,void*,NativeBitCursor* c){write_native_session_message_type_00449940(m,c);}
void __fastcall read_type(M* m,void*,NativeSessionReadStream* s){read_native_session_message_type_00449960(m,s);}
bool __fastcall type_equals(const M* m,void*,U t){return native_session_message_type_equals_004499a0(m,t);}
bool __fastcall always_true(const M*,void*){return native_session_message_always_true_004499c0();}
const U root_profile[]={reinterpret_cast<U>(&root_delete),reinterpret_cast<U>(&_purecall),reinterpret_cast<U>(&_purecall)};
const U base_profile[]={reinterpret_cast<U>(&base_delete),reinterpret_cast<U>(&write_type),reinterpret_cast<U>(&read_type),reinterpret_cast<U>(&type_equals),reinterpret_cast<U>(&always_true)};
const U one_profile[]={reinterpret_cast<U>(&one_delete),reinterpret_cast<U>(&write_type),reinterpret_cast<U>(&read_type),reinterpret_cast<U>(&type_equals),reinterpret_cast<U>(&always_true)};
const U zero_profile[]={reinterpret_cast<U>(&zero_delete),reinterpret_cast<U>(&write_type),reinterpret_cast<U>(&read_type),reinterpret_cast<U>(&type_equals),reinterpret_cast<U>(&always_true)};
const U two_profile[]={reinterpret_cast<U>(&two_delete),reinterpret_cast<U>(&write_type),reinterpret_cast<U>(&read_type),reinterpret_cast<U>(&type_equals),reinterpret_cast<U>(&always_true)};
M* scalar_delete(M* message,U flags){
    static_cast<volatile M&>(*message).profile_00=root_profile;
    if((flags&1u)!=0)singleton_lifetime_free(message);
    return message;
}
}
const U* native_session_message_root_profile_00ce4974(){return root_profile;}
const U* native_session_message_base_profile_00d02c68(){return base_profile;}
const U* native_session_message_one_profile_00ce4980(){return one_profile;}
const U* native_session_message_zero_profile_00d02ee8(){return zero_profile;}
const U* native_session_message_two_profile_00d02efc(){return two_profile;}
NativeSessionMessageStorage* construct_native_session_message_0075b430(M* message,U type,const NativeSessionMessageContext& c){
    volatile auto& m=*message;
    m.delivery_04=3;m.field_08=0;m.field_0c=0;m.profile_00=base_profile;m.type_10=static_cast<B>(type);
    auto* game=static_cast<B*>(c.current_game_00e188a8);
    const auto index=*reinterpret_cast<const volatile std::int32_t*>(game+0x18ec);
    if(index>=0&&index<=7)m.selected_owner_14=*reinterpret_cast<void* const volatile*>(game+0x18cc+static_cast<U>(index)*4);
    else m.selected_owner_14=nullptr;
    return message;
}
M* construct_native_session_message_one_00449980(M* m,U t,const NativeSessionMessageContext& c){construct_native_session_message_0075b430(m,t,c);static_cast<volatile M&>(*m).profile_00=one_profile;static_cast<volatile M&>(*m).delivery_04=1;return m;}
M* construct_native_session_message_zero_0075b860(M* m,U t,const NativeSessionMessageContext& c){construct_native_session_message_0075b430(m,t,c);static_cast<volatile M&>(*m).profile_00=zero_profile;static_cast<volatile M&>(*m).delivery_04=0;return m;}
M* construct_native_session_message_two_0075b8a0(M* m,U t,const NativeSessionMessageContext& c){construct_native_session_message_0075b430(m,t,c);static_cast<volatile M&>(*m).profile_00=two_profile;static_cast<volatile M&>(*m).delivery_04=2;return m;}
M* delete_native_session_message_root_00449910(M* m,U f){return scalar_delete(m,f);}
M* delete_native_session_message_one_004499e0(M* m,U f){return scalar_delete(m,f);}
M* delete_native_session_message_base_00759cd0(M* m,U f){return scalar_delete(m,f);}
M* delete_native_session_message_zero_0075b880(M* m,U f){return scalar_delete(m,f);}
M* delete_native_session_message_two_0075b8c0(M* m,U f){return scalar_delete(m,f);}
void write_native_session_message_type_00449940(const M* m,NativeBitCursor* c){write_native_byte_bits_00428ff0(c,m->type_10,8);}
void read_native_session_message_type_00449960(M* m,NativeSessionReadStream* s){read_native_u8_bits_00428c70(&s->cursor_04,&m->type_10,8);}
bool native_session_message_type_equals_004499a0(const M* m,U type){return type==m->type_10;}
bool native_session_message_always_true_004499c0(){return true;}
void write_native_session_message_header_0075b480(const void* message,NativeBitCursor* c){
    auto* m=static_cast<const B*>(message);
    write_native_byte_bits_00428ff0(c,m[0x10],8);
    write_native_word_bits_00429120(c,*reinterpret_cast<const std::uint16_t*>(m+0x18),12);
    write_native_bool_bit_004290b0(c,m[0x1a]);
}
void read_native_session_message_header_0075b4c0(void* message,NativeSessionReadStream* s){
    auto* m=static_cast<B*>(message);
    read_native_u8_bits_00428c70(&s->cursor_04,m+0x10,8);
    read_native_word_bits_00428e30(&s->cursor_04,reinterpret_cast<std::uint16_t*>(m+0x18),12);
    read_native_bool_bit_00428d70(&s->cursor_04,reinterpret_cast<bool*>(m+0x1a));
}
} // namespace bsp
