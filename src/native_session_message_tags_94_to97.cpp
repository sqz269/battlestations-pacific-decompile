#include "bsp/native_session_message_tags_94_to97.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using B=std::uint8_t;using Context=NativeSessionMessage94To97Context;
U x87_copy(const volatile void* source){U result;__asm {mov eax,source
 fld dword ptr [eax]
 fstp dword ptr result}return result;}
void zero_header(NativeMessage75ExtendedHeader& h,const U* profile){volatile auto& x=h;x.base.field_08=0;x.base.field_0c=0;x.base.selected_owner_14=nullptr;x.base.type_10=0;x.sender_18=0;x.relay_1a=0;x.base.delivery_04=1;x.flag_1c=0;x.base.profile_00=profile;}
void write_header(const NativeMessage75ExtendedHeader& h,NativeBitCursor* cursor){write_native_session_message_header_0075b480(&h,cursor);write_native_bool_bit_004290b0(cursor,static_cast<const volatile NativeMessage75ExtendedHeader&>(h).flag_1c);}
void read_header(NativeMessage75ExtendedHeader& h,NativeSessionReadStream* stream){read_native_session_message_header_0075b4c0(&h,stream);read_native_bool_bit_00428d70(&stream->cursor_04,reinterpret_cast<bool*>(&h.flag_1c));}
void write_inline_header(const NativeMessage75ExtendedHeader& h,NativeBitCursor* cursor){const volatile auto& x=h;write_native_byte_bits_00428ff0(cursor,x.base.type_10,8);write_native_word_bits_00429120(cursor,x.sender_18,12);write_native_bool_bit_004290b0(cursor,x.relay_1a);write_native_bool_bit_004290b0(cursor,x.flag_1c);}
void read_inline_header(NativeMessage75ExtendedHeader& h,NativeBitCursor* cursor){read_native_u8_bits_00428c70(cursor,&h.base.type_10,8);read_native_word_bits_00428e30(cursor,&h.sender_18,12);read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&h.relay_1a));read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&h.flag_1c));}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
#define BSP_ADAPTERS(N,P,W,R,S) \
const NativeSessionMessage##N##Profile& profile##N(const NativeSessionMessage##N* m){return *reinterpret_cast<const NativeSessionMessage##N##Profile*>(m->header.base.profile_00);} \
NativeSessionMessage##N* __fastcall del##N(NativeSessionMessage##N* m,void*,U flags){return delete_native_session_message##N##_##S(m,flags);} \
void __fastcall write##N(const NativeSessionMessage##N* m,void*,NativeBitCursor* cursor){write_native_session_message##N##_##W(m,cursor,*profile##N(m).context);} \
void __fastcall read##N(NativeSessionMessage##N* m,void*,NativeSessionReadStream* stream){read_native_session_message##N##_##R(m,stream,*profile##N(m).context);} \
bool __fastcall is##N(const void*,void*,U type){return native_session_message_is##N##_##P(type);}
BSP_ADAPTERS(94,0075b090,007640d0,00764160,00764200)
BSP_ADAPTERS(95,0071cc70,0071cca0,0071cd30,0071e3f0)
BSP_ADAPTERS(96,0071cb80,0071cbb0,0071cbe0,0071e3d0)
BSP_ADAPTERS(97,0075a2c0,0075c700,0075c760,0075ca00)
#undef BSP_ADAPTERS
}
#define BSP_PROFILE(N,C,D,S) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const Context& c):slots{reinterpret_cast<U>(&del##N),reinterpret_cast<U>(&write##N),reinterpret_cast<U>(&read##N),reinterpret_cast<U>(&is##N),reinterpret_cast<U>(&yes)},context(&c){} \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N* m,const Context&,const NativeSessionMessage##N##Profile& p){zero_header(m->header,p.slots);return m;} \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N* m){static_cast<volatile NativeSessionMessageStorage&>(m->header.base).profile_00=native_session_message_root_profile_00ce4974();} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* m,U flags){destroy_native_session_message##N##_##D(m);if(flags&1)singleton_lifetime_free(m);return m;}
BSP_PROFILE(94,0075b060,0075b0c0,00764200)
BSP_PROFILE(95,0075b100,0071cdc0,0071e3f0)
BSP_PROFILE(96,0075b0d0,0071cc10,0071e3d0)
BSP_PROFILE(97,0075a290,source,0075ca00)
#undef BSP_PROFILE
bool native_session_message_is94_0075b090(U t){return t==94||t==89||t==73||t==70;}
bool native_session_message_is95_0071cc70(U t){return t==95||t==89||t==73||t==70;}
bool native_session_message_is96_0071cb80(U t){return t==96||t==89||t==73||t==70;}
bool native_session_message_is97_0075a2c0(U t){return t==97||t==73||t==70;}
NativeSessionMessage95* construct_native_session_message95_value_0071cc20(NativeSessionMessage95* m,const float* vector,B present,const Context& c,const NativeSessionMessage95Profile& p){construct_native_session_message_0075b430(&m->header.base,95,*c.session);volatile auto& x=*m;x.header.sender_18=0;x.header.relay_1a=0;x.header.flag_1c=0;x.header.base.delivery_04=1;x.header.base.profile_00=p.slots;for(U i=0;i<3;++i)x.vector_20[i]=x87_copy(vector+i);x.present_2c=present;return m;}
NativeSessionMessage96* construct_native_session_message96_value_0071cb50(NativeSessionMessage96* m,U value,const Context& c,const NativeSessionMessage96Profile& p){construct_native_session_message_0075b430(&m->header.base,96,*c.session);volatile auto& x=*m;x.header.sender_18=0;x.header.relay_1a=0;x.header.flag_1c=0;x.value_20=value;x.header.base.delivery_04=1;x.header.base.profile_00=p.slots;return m;}
void write_native_session_message94_007640d0(const NativeSessionMessage94* m,NativeBitCursor* cursor,const Context&){
    write_inline_header(m->header,cursor);const volatile auto& x=*m;const bool present=x.handle_20!=0;write_native_bool_bit_004290b0(cursor,present);
    if(present){write_native_bool_bit_004290b0(cursor,x.mode_22);const B mode=x.mode_22;const auto handle=x.handle_20;write_native_word_bits_00429120(cursor,handle,mode?12u:13u);}write_native_bool_bit_004290b0(cursor,x.flag_23);
}
void read_native_session_message94_00764160(NativeSessionMessage94* m,NativeSessionReadStream* stream,const Context&){
    auto* cursor=&stream->cursor_04;read_inline_header(m->header,cursor);bool present;read_native_bool_bit_00428d70(cursor,&present);
    if(present){read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->mode_22));read_native_word_bits_00428e30(cursor,&m->handle_20,m->mode_22?12u:13u);}else{m->mode_22=0;m->handle_20=0;}read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->flag_23));
}
void write_native_session_message95_0071cca0(const NativeSessionMessage95* m,NativeBitCursor* cursor,const Context& c){write_header(m->header,cursor);const volatile auto& x=*m;write_native_bool_bit_004290b0(cursor,x.present_2c);if(x.present_2c){for(U i=0;i<3;++i){const U scale=x87_copy(c.maximum_00d7a248);const U value=x87_copy(&x.vector_20[i]);write_native_numeric_float_004295c0(cursor,value,0,1,scale,32,*c.numeric);}}}
void read_native_session_message95_0071cd30(NativeSessionMessage95* m,NativeSessionReadStream* stream,const Context& c){read_header(m->header,stream);auto* cursor=&stream->cursor_04;read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->present_2c));if(m->present_2c){for(U i=0;i<3;++i){const U scale=x87_copy(c.maximum_00d7a248);read_native_numeric_float_004293f0(cursor,&m->vector_20[i],0,1,scale,32,*c.numeric);}}}
void write_native_session_message96_0071cbb0(const NativeSessionMessage96* m,NativeBitCursor* cursor,const Context&){write_header(m->header,cursor);write_native_unsigned_dword_bits_00429070(cursor,static_cast<const volatile NativeSessionMessage96&>(*m).value_20,8);}
void read_native_session_message96_0071cbe0(NativeSessionMessage96* m,NativeSessionReadStream* stream,const Context&){read_header(m->header,stream);read_native_u32_bits_00428d10(&stream->cursor_04,&m->value_20,8);}
void write_native_session_message97_0075c700(const NativeSessionMessage97* m,NativeBitCursor* cursor,const Context&){write_inline_header(m->header,cursor);const volatile auto& x=*m;write_native_unsigned_dword_bits_00429070(cursor,x.value_20,3);write_native_unsigned_dword_bits_00429070(cursor,x.value_24,3);}
void read_native_session_message97_0075c760(NativeSessionMessage97* m,NativeSessionReadStream* stream,const Context&){auto* cursor=&stream->cursor_04;read_inline_header(m->header,cursor);read_native_u32_bits_00428d10(cursor,&m->value_20,3);read_native_u32_bits_00428d10(cursor,&m->value_24,3);}
} // namespace bsp
