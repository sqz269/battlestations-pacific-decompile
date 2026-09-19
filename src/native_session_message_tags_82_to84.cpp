#include "bsp/native_session_message_tags_82_to84.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_owned_string.hpp"

namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using B=std::uint8_t;using Context=NativeSessionMessage82To84Context;
void base(NativeSessionMessageStorage* m,B type,const Context& c){
    volatile auto& b=*m;b.delivery_04=3;b.field_08=0;b.field_0c=0;b.profile_00=native_session_message_base_profile_00d02c68();b.type_10=type;
    const auto* game=static_cast<const B*>(c.session->current_game_00e188a8);const I owner=*reinterpret_cast<const volatile I*>(game+0x18ec);b.selected_owner_14=owner>=0&&owner<=7?*reinterpret_cast<void* const volatile*>(game+0x18cc+static_cast<U>(owner)*4):nullptr;
}
void write_header(const NativeMessage75ExtendedHeader* h,NativeBitCursor* c){const volatile auto& x=*h;write_native_byte_bits_00428ff0(c,x.base.type_10,8);write_native_word_bits_00429120(c,x.sender_18,12);write_native_bool_bit_004290b0(c,x.relay_1a);write_native_bool_bit_004290b0(c,x.flag_1c);}
void read_header(NativeMessage75ExtendedHeader* h,NativeBitCursor* c){read_native_u8_bits_00428c70(c,&h->base.type_10,8);read_native_word_bits_00428e30(c,&h->sender_18,12);read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&h->relay_1a));read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&h->flag_1c));}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
#define BSP_MESSAGE_ADAPTERS(N,C,P,W,R,D,S) \
const NativeSessionMessage##N##Profile& profile##N(const NativeSessionMessage##N* m){return *reinterpret_cast<const NativeSessionMessage##N##Profile*>(m->header.base.profile_00);} \
NativeSessionMessage##N* __fastcall del##N(NativeSessionMessage##N* m,void*,U flags){return delete_native_session_message##N##_##S(m,flags,*profile##N(m).context);} \
void __fastcall write##N(const NativeSessionMessage##N* m,void*,NativeBitCursor* c){write_native_session_message##N##_##W(m,c,*profile##N(m).context);} \
void __fastcall read##N(NativeSessionMessage##N* m,void*,NativeSessionReadStream* s){read_native_session_message##N##_##R(m,s,*profile##N(m).context);} \
bool __fastcall is##N(const void*,void*,U type){return native_session_message_is##N##_##P(type);}
BSP_MESSAGE_ADAPTERS(82,0075a3b0,0075a3e0,0075c900,0075c950,0075a400,0075ca60)
BSP_MESSAGE_ADAPTERS(83,0075c9a0,004b5c50,004b5c70,004b5ca0,source,004bd550)
BSP_MESSAGE_ADAPTERS(84,00765a20,00765a90,00765ab0,00765b10,00765b70,00766830)
#undef BSP_MESSAGE_ADAPTERS
}
#define BSP_MESSAGE_PROFILE(N,C,P,W,R,D,S) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const Context& c):slots{reinterpret_cast<U>(&del##N),reinterpret_cast<U>(&write##N),reinterpret_cast<U>(&read##N),reinterpret_cast<U>(&is##N),reinterpret_cast<U>(&yes)},context(&c){} \
bool native_session_message_is##N##_##P(U t){return t==N||t==73||t==70;} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* m,U flags,const Context& c){destroy_native_session_message##N##_##D(m,c);if(flags&1)singleton_lifetime_free(m);return m;}
BSP_MESSAGE_PROFILE(82,0075a3b0,0075a3e0,0075c900,0075c950,0075a400,0075ca60)
BSP_MESSAGE_PROFILE(83,0075c9a0,004b5c50,004b5c70,004b5ca0,source,004bd550)
BSP_MESSAGE_PROFILE(84,00765a20,00765a90,00765ab0,00765b10,00765b70,00766830)
#undef BSP_MESSAGE_PROFILE
NativeSessionMessage82* construct_native_session_message82_0075a3b0(NativeSessionMessage82* m,const Context&,const NativeSessionMessage82Profile& p){volatile auto& h=m->header;h.base.field_08=0;h.base.field_0c=0;h.base.selected_owner_14=nullptr;h.base.type_10=0;h.sender_18=0;h.relay_1a=0;h.base.delivery_04=1;h.flag_1c=0;h.base.profile_00=p.slots;return m;}
NativeSessionMessage83* construct_native_session_message83_0075c9a0(NativeSessionMessage83* m,const Context& c,const NativeSessionMessage83Profile& p){base(&m->header.base,83,c);volatile auto& h=m->header;h.sender_18=0;h.relay_1a=0;h.flag_1c=0;h.base.delivery_04=1;h.base.profile_00=p.slots;return m;}
NativeSessionMessage83* construct_native_session_message83_value_004b5c00(NativeSessionMessage83* m,U value,const Context& c,const NativeSessionMessage83Profile& p){construct_native_session_message_0075b430(&m->header.base,83,*c.session);volatile auto& h=m->header;h.sender_18=0;h.relay_1a=0;h.flag_1c=0;static_cast<volatile NativeSessionMessage83&>(*m).value_20=value;h.base.delivery_04=1;h.base.profile_00=p.slots;return m;}
NativeSessionMessage84* construct_native_session_message84_00765a20(NativeSessionMessage84* m,const Context& c,const NativeSessionMessage84Profile& p){base(&m->header.base,84,c);volatile auto& x=*m;x.header.sender_18=0;x.header.relay_1a=0;x.header.base.delivery_04=1;x.header.flag_1c=0;x.header.base.profile_00=p.slots;x.length_20=0;x.data_24=nullptr;x.flag_28=0;return m;}
void write_native_session_message82_0075c900(const NativeSessionMessage82* m,NativeBitCursor* c,const Context&){write_header(&m->header,c);write_native_bool_bit_004290b0(c,static_cast<const volatile NativeSessionMessage82&>(*m).flag_20);}
void read_native_session_message82_0075c950(NativeSessionMessage82* m,NativeSessionReadStream* s,const Context&){read_header(&m->header,&s->cursor_04);read_native_bool_bit_00428d70(&s->cursor_04,reinterpret_cast<bool*>(&m->flag_20));}
void write_native_session_message83_004b5c70(const NativeSessionMessage83* m,NativeBitCursor* c,const Context&){write_native_session_message_header_0075b480(m,c);const volatile auto& x=*m;write_native_bool_bit_004290b0(c,x.header.flag_1c);write_native_unsigned_dword_bits_00429070(c,x.value_20,5);}
void read_native_session_message83_004b5ca0(NativeSessionMessage83* m,NativeSessionReadStream* s,const Context&){read_native_session_message_header_0075b4c0(m,s);read_native_bool_bit_00428d70(&s->cursor_04,reinterpret_cast<bool*>(&m->header.flag_1c));read_native_u32_bits_00428d10(&s->cursor_04,&m->value_20,5);}
void write_native_session_message84_00765ab0(const NativeSessionMessage84* m,NativeBitCursor* cursor,const Context& c){write_header(&m->header,cursor);write_native_owned_string_00429ac0(cursor,&m->length_20,c.fallback_00e17669);write_native_bool_bit_004290b0(cursor,static_cast<const volatile NativeSessionMessage84&>(*m).flag_28);}
void read_native_session_message84_00765b10(NativeSessionMessage84* m,NativeSessionReadStream* s,const Context& c){read_header(&m->header,&s->cursor_04);read_native_owned_string_00429f20(&s->cursor_04,&m->length_20,*c.strings);read_native_bool_bit_00428d70(&s->cursor_04,reinterpret_cast<bool*>(&m->flag_28));}
void destroy_native_session_message82_0075a400(NativeSessionMessage82* m,const Context&){static_cast<volatile NativeSessionMessageStorage&>(m->header.base).profile_00=native_session_message_root_profile_00ce4974();}
void destroy_native_session_message83_source(NativeSessionMessage83* m,const Context&){static_cast<volatile NativeSessionMessageStorage&>(m->header.base).profile_00=native_session_message_root_profile_00ce4974();}
void destroy_native_session_message84_00765b70(NativeSessionMessage84* m,const Context& c){__try{destroy_native_string_header_0041dd20(&m->length_20,*c.strings);}__finally{static_cast<volatile NativeSessionMessageStorage&>(m->header.base).profile_00=native_session_message_root_profile_00ce4974();}}
} // namespace bsp
