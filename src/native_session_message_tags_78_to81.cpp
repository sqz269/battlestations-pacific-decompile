#include "bsp/native_session_message_tags_78_to81.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"

namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using B=std::uint8_t;
void zero_extended(NativeMessage75ExtendedHeader* h,const U* profile){volatile auto& x=*h;x.base.field_08=0;x.base.field_0c=0;x.base.selected_owner_14=nullptr;x.base.type_10=0;x.sender_18=0;x.relay_1a=0;x.base.delivery_04=1;x.flag_1c=0;x.base.profile_00=profile;}
void write_header(const NativeMessage75ExtendedHeader* h,NativeBitCursor* c){
    const volatile auto& x=*h;write_native_byte_bits_00428ff0(c,x.base.type_10,8);write_native_word_bits_00429120(c,x.sender_18,12);write_native_bool_bit_004290b0(c,x.relay_1a);write_native_bool_bit_004290b0(c,x.flag_1c);
}
void read_header(NativeMessage75ExtendedHeader* h,NativeBitCursor* c){read_native_u8_bits_00428c70(c,&h->base.type_10,8);read_native_word_bits_00428e30(c,&h->sender_18,12);read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&h->relay_1a));read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&h->flag_1c));}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
#define BSP_MESSAGE_ADAPTERS(N,C,P,W,R,D,S) \
NativeSessionMessage##N* __fastcall del##N(NativeSessionMessage##N* m,void*,U flags){return delete_native_session_message##N##_##S(m,flags);} \
void __fastcall write##N(const NativeSessionMessage##N* m,void*,NativeBitCursor* c){write_native_session_message##N##_##W(m,c);} \
void __fastcall read##N(NativeSessionMessage##N* m,void*,NativeSessionReadStream* s){read_native_session_message##N##_##R(m,s);} \
bool __fastcall is##N(const void*,void*,U type){return native_session_message_is##N##_##P(type);}
BSP_MESSAGE_ADAPTERS(78,0075bd50,0075bdc0,0075be00,0075be60,0075bdb0,0075bde0)
BSP_MESSAGE_ADAPTERS(79,0075a030,0075a070,0075bec0,0075bf10,0075a060,0075a090)
BSP_MESSAGE_ADAPTERS(80,0075a0b0,0075a0e0,0075bf60,0075bfb0,0075a100,0075c000)
BSP_MESSAGE_ADAPTERS(81,005f9660,005f9690,005f96b0,005f96d0,005f96f0,005f9a00)
#undef BSP_MESSAGE_ADAPTERS
}
#define BSP_MESSAGE_PROFILE(N,C,P,W,R,D,S) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const NativeSessionMessageContext& c):slots{reinterpret_cast<U>(&del##N),reinterpret_cast<U>(&write##N),reinterpret_cast<U>(&read##N),reinterpret_cast<U>(&is##N),reinterpret_cast<U>(&yes)},context(&c){} \
bool native_session_message_is##N##_##P(U t){return t==N||t==73||t==70;} \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N* m){static_cast<volatile NativeSessionMessageStorage&>(m->header.base).profile_00=native_session_message_root_profile_00ce4974();} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* m,U flags){destroy_native_session_message##N##_##D(m);if(flags&1)singleton_lifetime_free(m);return m;}
BSP_MESSAGE_PROFILE(78,0075bd50,0075bdc0,0075be00,0075be60,0075bdb0,0075bde0)
BSP_MESSAGE_PROFILE(79,0075a030,0075a070,0075bec0,0075bf10,0075a060,0075a090)
BSP_MESSAGE_PROFILE(80,0075a0b0,0075a0e0,0075bf60,0075bfb0,0075a100,0075c000)
BSP_MESSAGE_PROFILE(81,005f9660,005f9690,005f96b0,005f96d0,005f96f0,005f9a00)
#undef BSP_MESSAGE_PROFILE
NativeSessionMessage78* construct_native_session_message78_0075bd50(NativeSessionMessage78* m,const NativeSessionMessageContext& c,const NativeSessionMessage78Profile& p){
    volatile auto& h=m->header;h.base.delivery_04=3;h.base.field_08=0;h.base.field_0c=0;h.base.profile_00=native_session_message_base_profile_00d02c68();h.base.type_10=78;
    const auto* game=static_cast<const B*>(c.current_game_00e188a8);const I owner=*reinterpret_cast<const volatile I*>(game+0x18ec);h.base.selected_owner_14=owner>=0&&owner<=7?*reinterpret_cast<void* const volatile*>(game+0x18cc+static_cast<U>(owner)*4):nullptr;
    h.sender_18=0;h.relay_1a=0;h.flag_1c=0;h.base.delivery_04=1;h.base.profile_00=p.slots;return m;
}
NativeSessionMessage79* construct_native_session_message79_0075a030(NativeSessionMessage79* m,const NativeSessionMessageContext&,const NativeSessionMessage79Profile& p){zero_extended(&m->header,p.slots);return m;}
NativeSessionMessage80* construct_native_session_message80_0075a0b0(NativeSessionMessage80* m,const NativeSessionMessageContext&,const NativeSessionMessage80Profile& p){zero_extended(&m->header,p.slots);return m;}
NativeSessionMessage81* construct_native_session_message81_005f9660(NativeSessionMessage81* m,const NativeSessionMessageContext& c,const NativeSessionMessage81Profile& p){construct_native_session_message_0075b430(&m->header.base,81,c);volatile auto& h=m->header;h.sender_18=0;h.relay_1a=0;h.flag_1c=0;h.base.delivery_04=1;h.base.profile_00=p.slots;return m;}
void write_native_session_message78_0075be00(const NativeSessionMessage78* m,NativeBitCursor* c){const volatile auto& x=*m;write_header(&m->header,c);write_native_bool_bit_004290b0(c,x.flag_20);write_native_unsigned_dword_bits_00429070(c,x.value_24,4);}
void read_native_session_message78_0075be60(NativeSessionMessage78* m,NativeSessionReadStream* s){auto* c=&s->cursor_04;read_header(&m->header,c);read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&m->flag_20));read_native_u32_bits_00428d10(c,&m->value_24,4);}
void write_native_session_message79_0075bec0(const NativeSessionMessage79* m,NativeBitCursor* c){write_header(&m->header,c);write_native_unsigned_dword_bits_00429070(c,static_cast<const volatile NativeSessionMessage79&>(*m).value_20,4);}
void read_native_session_message79_0075bf10(NativeSessionMessage79* m,NativeSessionReadStream* s){auto* c=&s->cursor_04;read_header(&m->header,c);read_native_u32_bits_00428d10(c,&m->value_20,4);}
void write_native_session_message80_0075bf60(const NativeSessionMessage80* m,NativeBitCursor* c){write_header(&m->header,c);write_native_signed_dword_bits_00429090(c,static_cast<U>(static_cast<const volatile NativeSessionMessage80&>(*m).value_20),4);}
void read_native_session_message80_0075bfb0(NativeSessionMessage80* m,NativeSessionReadStream* s){auto* c=&s->cursor_04;read_header(&m->header,c);read_native_i32_bits_00428d30(c,&m->value_20,4);}
void write_native_session_message81_005f96b0(const NativeSessionMessage81* m,NativeBitCursor* c){write_native_session_message_header_0075b480(m,c);write_native_bool_bit_004290b0(c,static_cast<const volatile NativeSessionMessage81&>(*m).header.flag_1c);}
void read_native_session_message81_005f96d0(NativeSessionMessage81* m,NativeSessionReadStream* s){read_native_session_message_header_0075b4c0(m,s);read_native_bool_bit_00428d70(&s->cursor_04,reinterpret_cast<bool*>(&m->header.flag_1c));}
} // namespace bsp
