#include "bsp/native_session_message_tags_66_67_68.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_owned_string.hpp"
#include "bsp/native_input_settings_vector_storage.hpp"

namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using B=std::uint8_t;
using Context=NativeSessionMessage66To68Context;using Vector=NativeMessage66Dwords;
void base(NativeSessionMessageStorage* message,B type,const U* profile,const Context& c){
    volatile auto& m=*message;m.delivery_04=3;m.field_08=0;m.field_0c=0;m.profile_00=native_session_message_base_profile_00d02c68();m.type_10=type;
    const auto* game=static_cast<const B*>(c.session->current_game_00e188a8);
    const I owner=*reinterpret_cast<const volatile I*>(game+0x18ec);
    m.selected_owner_14=owner>=0&&owner<=7?*reinterpret_cast<void* const volatile*>(game+0x18cc+static_cast<U>(owner)*4):nullptr;
    m.delivery_04=1;m.profile_00=profile;
}
void clear(Vector* v){volatile auto& x=*v;x.begin_04=nullptr;x.end_08=nullptr;x.capacity_0c=nullptr;}
U count(const Vector* vector){const volatile auto& v=*vector;const U begin=reinterpret_cast<U>(v.begin_04);return begin?static_cast<U>(static_cast<I>(reinterpret_cast<U>(v.end_08)-begin)>>2):0;}
void check(const Vector* v,U i,const Context& c){if(i>=count(v))c.invalid_parameters->invalid_parameter(c.invalid_parameters->context);}
void write_vector(const Vector* vector,U width,NativeBitCursor* cursor,const Context& c){
    write_native_unsigned_dword_bits_00429070(cursor,count(vector),4);
    for(U i=0;i<count(vector);++i){check(vector,i,c);const volatile auto& v=*vector;const U value=static_cast<U>(v.begin_04[i]);write_native_signed_dword_bits_00429090(cursor,value,width);}
}
void read_vector(Vector* vector,U width,NativeBitCursor* cursor,const Context& c){
    U length;read_native_u32_bits_00428d10(cursor,&length,4);
    // Zero wire count deliberately retains the previous vector, including size.
    if(length){resize_native_input_settings_words_00492210(vector,length,0);for(U i=0;i<length;++i){check(vector,i,c);const volatile auto& v=*vector;read_native_i32_bits_00428d30(cursor,v.begin_04+i,width);}}
}
void destroy_vector(Vector* vector){const auto* p=static_cast<const volatile Vector&>(*vector).begin_04;if(p)singleton_lifetime_free(const_cast<I*>(p));clear(vector);}
U x87_copy(const volatile void* source){U result;__asm {mov ecx,source
 fld dword ptr [ecx]
 fstp dword ptr result}return result;}
void write_float(const U* value,NativeBitCursor* cursor,const Context& c){const U scale=x87_copy(c.maximum_00d7a248);const U bits=x87_copy(value);write_native_numeric_float_004295c0(cursor,bits,0,1,scale,32,*c.numeric);}
void read_float(U* value,NativeBitCursor* cursor,const Context& c){const U scale=x87_copy(c.maximum_00d7a248);read_native_numeric_float_004293f0(cursor,value,0,1,scale,32,*c.numeric);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
#define BSP_MESSAGE_ADAPTERS(N,C,P,W,R,D,S) \
const NativeSessionMessage##N##Profile& profile##N(const NativeSessionMessage##N* m){return *reinterpret_cast<const NativeSessionMessage##N##Profile*>(m->base.profile_00);} \
NativeSessionMessage##N* __fastcall del##N(NativeSessionMessage##N* m,void*,U f){const auto& p=profile##N(m);return delete_native_session_message##N##_##S(m,f,*p.context,p);} \
void __fastcall write##N(const NativeSessionMessage##N* m,void*,NativeBitCursor* c){write_native_session_message##N##_##W(m,c,*profile##N(m).context);} \
void __fastcall read##N(NativeSessionMessage##N* m,void*,NativeSessionReadStream* s){read_native_session_message##N##_##R(m,s,*profile##N(m).context);} \
bool __fastcall is##N(const void*,void*,U type){return native_session_message_is##N##_##P(type);}
BSP_MESSAGE_ADAPTERS(66,00766d00,00766d90,00943810,00947e40,00766da0,00766e00)
BSP_MESSAGE_ADAPTERS(67,00765760,007657c0,008e47e0,008e4850,007657d0,00765830)
BSP_MESSAGE_ADAPTERS(68,00765850,007658b0,008e48c0,008e4990,007658c0,00765920)
#undef BSP_MESSAGE_ADAPTERS
}
#define BSP_MESSAGE_COMMON(N,C,P,W,R,D,S) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const Context& c):slots{reinterpret_cast<U>(&del##N),reinterpret_cast<U>(&write##N),reinterpret_cast<U>(&read##N),reinterpret_cast<U>(&is##N),reinterpret_cast<U>(&yes)},context(&c){} \
bool native_session_message_is##N##_##P(U type){return type==N;} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* m,U flags,const Context& c,const NativeSessionMessage##N##Profile& p){auto* captured=m;destroy_native_session_message##N##_##D(m,c,p);if(flags&1u)singleton_lifetime_free(m);return captured;}
BSP_MESSAGE_COMMON(66,00766d00,00766d90,00943810,00947e40,00766da0,00766e00)
BSP_MESSAGE_COMMON(67,00765760,007657c0,008e47e0,008e4850,007657d0,00765830)
BSP_MESSAGE_COMMON(68,00765850,007658b0,008e48c0,008e4990,007658c0,00765920)
#undef BSP_MESSAGE_COMMON
NativeSessionMessage66* construct_native_session_message66_00766d00(NativeSessionMessage66* m,const Context& c,const NativeSessionMessage66Profile& p){base(&m->base,66,p.slots,c);clear(&m->first_24);clear(&m->second_34);clear(&m->third_44);return m;}
NativeSessionMessage67* construct_native_session_message67_00765760(NativeSessionMessage67* m,const Context& c,const NativeSessionMessage67Profile& p){base(&m->base,67,p.slots,c);volatile auto& v=*m;v.length_1c=0;v.data_20=nullptr;return m;}
NativeSessionMessage68* construct_native_session_message68_00765850(NativeSessionMessage68* m,const Context& c,const NativeSessionMessage68Profile& p){base(&m->base,68,p.slots,c);volatile auto& v=*m;v.length_3c=0;v.data_40=nullptr;return m;}
void write_native_session_message66_00943810(const NativeSessionMessage66* message,NativeBitCursor* cursor,const Context& c){
    const volatile auto& m=*message;write_native_byte_bits_00428ff0(cursor,m.base.type_10,8);write_native_signed_dword_bits_00429090(cursor,static_cast<U>(m.value_18),5);write_native_word_bits_00429120(cursor,m.word_20,13);write_native_signed_dword_bits_00429090(cursor,static_cast<U>(m.value_1c),4);
    write_native_bool_bit_004290b0(cursor,m.has_handle_56);if(m.has_handle_56)write_native_word_bits_00429120(cursor,m.handle_54,12);write_native_unsigned_dword_bits_00429070(cursor,m.value_58,16);
    write_vector(&message->first_24,10,cursor,c);write_vector(&message->second_34,4,cursor,c);write_vector(&message->third_44,4,cursor,c);
}
void read_native_session_message66_00947e40(NativeSessionMessage66* m,NativeSessionReadStream* stream,const Context& c){
    auto* cursor=&stream->cursor_04;read_native_u8_bits_00428c70(cursor,&m->base.type_10,8);read_native_i32_bits_00428d30(cursor,&m->value_18,5);read_native_word_bits_00428e30(cursor,&m->word_20,13);read_native_i32_bits_00428d30(cursor,&m->value_1c,4);
    read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->has_handle_56));if(static_cast<const volatile NativeSessionMessage66&>(*m).has_handle_56)read_native_word_bits_00428e30(cursor,&m->handle_54,12);read_native_u32_bits_00428d10(cursor,&m->value_58,16);
    read_vector(&m->first_24,10,cursor,c);read_vector(&m->second_34,4,cursor,c);read_vector(&m->third_44,4,cursor,c);
}
void destroy_native_session_message66_00766da0(NativeSessionMessage66* m,const Context&,const NativeSessionMessage66Profile& p){volatile auto& b=m->base;b.profile_00=p.slots;destroy_vector(&m->third_44);destroy_vector(&m->second_34);destroy_vector(&m->first_24);b.profile_00=native_session_message_root_profile_00ce4974();}
void write_native_session_message67_008e47e0(const NativeSessionMessage67* message,NativeBitCursor* cursor,const Context& c){
    const volatile auto& m=*message;write_native_byte_bits_00428ff0(cursor,m.base.type_10,8);write_native_signed_dword_bits_00429090(cursor,static_cast<U>(m.value_18),4);write_native_owned_string_00429ac0(cursor,&message->length_1c,c.fallback_00e17669);write_native_signed_dword_bits_00429090(cursor,static_cast<U>(m.value_24),32);write_native_unsigned_dword_bits_00429070(cursor,m.value_28,4);write_native_word_bits_00429120(cursor,m.word_2c,12);write_native_word_bits_00429120(cursor,m.word_2e,12);
}
void read_native_session_message67_008e4850(NativeSessionMessage67* m,NativeSessionReadStream* stream,const Context& c){
    auto* cursor=&stream->cursor_04;read_native_u8_bits_00428c70(cursor,&m->base.type_10,8);read_native_i32_bits_00428d30(cursor,&m->value_18,4);read_native_owned_string_00429f20(cursor,&m->length_1c,*c.strings);read_native_i32_bits_00428d30(cursor,&m->value_24,32);read_native_u32_bits_00428d10(cursor,&m->value_28,4);read_native_word_bits_00428e30(cursor,&m->word_2c,12);read_native_word_bits_00428e30(cursor,&m->word_2e,12);
}
void destroy_native_session_message67_007657d0(NativeSessionMessage67* m,const Context& c,const NativeSessionMessage67Profile& p){volatile auto& b=m->base;b.profile_00=p.slots;__try{destroy_native_string_header_0041dd20(&m->length_1c,*c.strings);}__finally{b.profile_00=native_session_message_root_profile_00ce4974();}}
void write_native_session_message68_008e48c0(const NativeSessionMessage68* message,NativeBitCursor* cursor,const Context& c){
    const volatile auto& m=*message;write_native_byte_bits_00428ff0(cursor,m.base.type_10,8);write_native_unsigned_dword_bits_00429070(cursor,m.value_18,4);write_float(&message->float_20,cursor,c);write_native_word_bits_00429120(cursor,m.word_24,12);write_native_word_bits_00429120(cursor,m.word_26,12);write_native_signed_dword_bits_00429090(cursor,static_cast<U>(m.value_28),32);write_native_unsigned_dword_bits_00429070(cursor,m.value_30,3);write_float(&message->float_34,cursor,c);write_native_unsigned_dword_bits_00429070(cursor,m.value_38,4);write_native_word_bits_00429120(cursor,m.word_1c,12);write_native_signed_dword_bits_00429090(cursor,static_cast<U>(m.value_2c),5);write_native_owned_string_00429ac0(cursor,&message->length_3c,c.fallback_00e17669);
}
void read_native_session_message68_008e4990(NativeSessionMessage68* m,NativeSessionReadStream* stream,const Context& c){
    auto* cursor=&stream->cursor_04;read_native_u8_bits_00428c70(cursor,&m->base.type_10,8);read_native_u32_bits_00428d10(cursor,&m->value_18,4);read_float(&m->float_20,cursor,c);read_native_word_bits_00428e30(cursor,&m->word_24,12);read_native_word_bits_00428e30(cursor,&m->word_26,12);read_native_i32_bits_00428d30(cursor,&m->value_28,32);read_native_u32_bits_00428d10(cursor,&m->value_30,3);read_float(&m->float_34,cursor,c);read_native_u32_bits_00428d10(cursor,&m->value_38,4);read_native_word_bits_00428e30(cursor,&m->word_1c,12);read_native_i32_bits_00428d30(cursor,&m->value_2c,5);read_native_owned_string_00429f20(cursor,&m->length_3c,*c.strings);
}
void destroy_native_session_message68_007658c0(NativeSessionMessage68* m,const Context& c,const NativeSessionMessage68Profile& p){volatile auto& b=m->base;b.profile_00=p.slots;__try{destroy_native_string_header_0041dd20(&m->length_3c,*c.strings);}__finally{b.profile_00=native_session_message_root_profile_00ce4974();}}
} // namespace bsp
