#include "bsp/native_session_message_tags_63_64_65.hpp"
#include "bsp/native_bit_cursor_fields.hpp"

namespace bsp {
namespace {
using U=std::uint32_t;using B=std::uint8_t;using M=NativeSessionMessage63;
U x87_copy(const volatile void* source){U result;__asm {mov ecx,source
 fld dword ptr [ecx]
 fstp dword ptr result}return result;}
M* construct_position(M* message,const NativeSessionMessageContext& context,const U* profile,B type){
    volatile auto& m=message->base;
    m.delivery_04=3;m.field_08=0;m.field_0c=0;
    m.profile_00=native_session_message_base_profile_00d02c68();m.type_10=type;
    const auto* game=static_cast<const B*>(context.current_game_00e188a8);
    const auto owner=*reinterpret_cast<const volatile std::int32_t*>(game+0x18ec);
    m.selected_owner_14=owner>=0&&owner<=7?*reinterpret_cast<void* const volatile*>(game+0x18cc+static_cast<U>(owner)*4):nullptr;
    m.delivery_04=1;m.profile_00=profile;return message;
}
void write_position(const M* message,NativeBitCursor* cursor,const NativeBitNumericContext& numeric,const volatile float& maximum){
    const volatile auto& m=*message;
    write_native_byte_bits_00428ff0(cursor,m.base.type_10,8);
    write_native_signed_dword_bits_00429090(cursor,static_cast<U>(m.value_18),4);
    write_native_bool_bit_004290b0(cursor,m.has_handle_1c);
    // The native writer reloads the flag after the Boolean write.
    if(m.has_handle_1c){write_native_word_bits_00429120(cursor,m.handle_2c,12);return;}
    for(U i=0;i<3;++i){const U scale=x87_copy(&maximum);const U value=x87_copy(&m.position_20[i]);write_native_numeric_float_004295c0(cursor,value,0,1,scale,32,numeric);}
}
void read_position(M* message,NativeSessionReadStream* stream,const NativeBitNumericContext& numeric,const volatile float& maximum){
    auto* cursor=&stream->cursor_04;
    read_native_u8_bits_00428c70(cursor,&message->base.type_10,8);
    read_native_i32_bits_00428d30(cursor,&message->value_18,4);
    read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&message->has_handle_1c));
    if(static_cast<const volatile M&>(*message).has_handle_1c){read_native_word_bits_00428e30(cursor,&message->handle_2c,12);return;}
    for(U i=0;i<3;++i){const U scale=x87_copy(&maximum);read_native_numeric_float_004293f0(cursor,&message->position_20[i],0,1,scale,32,numeric);}
}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
#define BSP_POSITION_ADAPTERS(N,C,W,R,P,D) \
NativeSessionMessage##N* __fastcall del##N(NativeSessionMessage##N* m,void*,U f){return delete_native_session_message##N##_##D(m,f);} \
void __fastcall write##N(const NativeSessionMessage##N* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage##N##Profile*>(m->base.profile_00);write_native_session_message##N##_##W(m,c,*p.numeric,*p.maximum_00d7a248);} \
void __fastcall read##N(NativeSessionMessage##N* m,void*,NativeSessionReadStream* s){const auto& p=*reinterpret_cast<const NativeSessionMessage##N##Profile*>(m->base.profile_00);read_native_session_message##N##_##R(m,s,*p.numeric,*p.maximum_00d7a248);} \
bool __fastcall is##N(const NativeSessionMessage##N* m,void*,U t){return native_session_message_is##N##_##P(m,t);}
BSP_POSITION_ADAPTERS(63,00763720,007637c0,00763870,007637a0,00763910)
BSP_POSITION_ADAPTERS(64,00763930,007639d0,00763a80,007639b0,00763b20)
#undef BSP_POSITION_ADAPTERS
NativeSessionMessage65* __fastcall del65(NativeSessionMessage65* m,void*,U f){return delete_native_session_message65_0075b200(m,f);}
void __fastcall write65(const NativeSessionMessage65* m,void*,NativeBitCursor* c){write_native_session_message65_0075b180(m,c);}
void __fastcall read65(NativeSessionMessage65* m,void*,NativeSessionReadStream* s){read_native_session_message65_0075b1c0(m,s);}
bool __fastcall is65(const NativeSessionMessage65* m,void*,U t){return native_session_message_is65_0075b160(m,t);}
const U profile65[]={reinterpret_cast<U>(&del65),reinterpret_cast<U>(&write65),reinterpret_cast<U>(&read65),reinterpret_cast<U>(&is65),reinterpret_cast<U>(&yes)};
}
NativeSessionMessage63Profile::NativeSessionMessage63Profile(const NativeBitNumericContext& n,const volatile float& m):slots{reinterpret_cast<U>(&del63),reinterpret_cast<U>(&write63),reinterpret_cast<U>(&read63),reinterpret_cast<U>(&is63),reinterpret_cast<U>(&yes)},numeric(&n),maximum_00d7a248(&m){}
NativeSessionMessage64Profile::NativeSessionMessage64Profile(const NativeBitNumericContext& n,const volatile float& m):slots{reinterpret_cast<U>(&del64),reinterpret_cast<U>(&write64),reinterpret_cast<U>(&read64),reinterpret_cast<U>(&is64),reinterpret_cast<U>(&yes)},numeric(&n),maximum_00d7a248(&m){}
const U* native_session_message65_profile_00d02eac(){return profile65;}
NativeSessionMessage63* construct_native_session_message63_00763720(M* m,const NativeSessionMessageContext& c,const NativeSessionMessage63Profile& p){return construct_position(m,c,p.slots,63);}
NativeSessionMessage64* construct_native_session_message64_00763930(M* m,const NativeSessionMessageContext& c,const NativeSessionMessage64Profile& p){return construct_position(m,c,p.slots,64);}
NativeSessionMessage65* construct_native_session_message65_0075b130(NativeSessionMessage65* message){volatile auto& m=message->base;m.field_08=0;m.field_0c=0;m.selected_owner_14=nullptr;m.type_10=0;m.delivery_04=1;m.profile_00=profile65;return message;}
bool native_session_message_is63_007637a0(const M* m,U type){return type==63||type==static_cast<const volatile M&>(*m).base.type_10;}
bool native_session_message_is64_007639b0(const M* m,U type){return type==64||type==static_cast<const volatile M&>(*m).base.type_10;}
bool native_session_message_is65_0075b160(const NativeSessionMessage65* m,U type){return type==65||type==static_cast<const volatile NativeSessionMessage65&>(*m).base.type_10;}
void write_native_session_message63_007637c0(const M* m,NativeBitCursor* c,const NativeBitNumericContext& n,const volatile float& scale){write_position(m,c,n,scale);}
void read_native_session_message63_00763870(M* m,NativeSessionReadStream* s,const NativeBitNumericContext& n,const volatile float& scale){read_position(m,s,n,scale);}
void write_native_session_message64_007639d0(const M* m,NativeBitCursor* c,const NativeBitNumericContext& n,const volatile float& scale){write_position(m,c,n,scale);}
void read_native_session_message64_00763a80(M* m,NativeSessionReadStream* s,const NativeBitNumericContext& n,const volatile float& scale){read_position(m,s,n,scale);}
void write_native_session_message65_0075b180(const NativeSessionMessage65* message,NativeBitCursor* c){const volatile auto& m=*message;write_native_byte_bits_00428ff0(c,m.base.type_10,8);write_native_unsigned_dword_bits_00429070(c,m.value_18,11);write_native_bool_bit_004290b0(c,m.flag_1c);}
void read_native_session_message65_0075b1c0(NativeSessionMessage65* m,NativeSessionReadStream* s){auto* c=&s->cursor_04;read_native_u8_bits_00428c70(c,&m->base.type_10,8);read_native_u32_bits_00428d10(c,&m->value_18,11);read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&m->flag_1c));}
void destroy_native_session_message63_00763790(M* m){static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
void destroy_native_session_message64_007639a0(M* m){static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
void destroy_native_session_message65_0075b150(NativeSessionMessage65* m){static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
NativeSessionMessage63* delete_native_session_message63_00763910(M* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
NativeSessionMessage64* delete_native_session_message64_00763b20(M* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
NativeSessionMessage65* delete_native_session_message65_0075b200(NativeSessionMessage65* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
} // namespace bsp
