#include "bsp/native_session_message_tags_26_to_32.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using B=std::uint8_t;
void read_flag(NativeBitCursor* c,B* out){bool value;read_native_bool_bit_00428d70(c,&value);*out=static_cast<B>(value);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
NativeSessionMessage26* __fastcall scalar26(NativeSessionMessage26* m,void*,U flags){return delete_native_session_message26_0075dc00(m,flags);}
void __fastcall write26(const NativeSessionMessage26* m,void*,NativeBitCursor* c){write_native_session_message25_0075dad0(m,c);}
void __fastcall read26(NativeSessionMessage26* m,void*,NativeSessionReadStream* s){read_native_session_message25_0075db10(m,s);}
bool __fastcall is26(const NativeSessionMessage26*,void*,U query){return native_session_message_is26_0075dbf0(query);}
const U profile26[]={reinterpret_cast<U>(&scalar26),reinterpret_cast<U>(&write26),reinterpret_cast<U>(&read26),reinterpret_cast<U>(&is26),reinterpret_cast<U>(&yes)};
NativeSessionMessage27* __fastcall scalar27(NativeSessionMessage27* m,void*,U flags){return delete_native_session_message27_0075dcb0(m,flags);}
void __fastcall write27(const NativeSessionMessage27* m,void*,NativeBitCursor* c){write_native_session_message25_0075dad0(m,c);}
void __fastcall read27(NativeSessionMessage27* m,void*,NativeSessionReadStream* s){read_native_session_message25_0075db10(m,s);}
bool __fastcall is27(const NativeSessionMessage27*,void*,U query){return native_session_message_is27_0075dca0(query);}
const U profile27[]={reinterpret_cast<U>(&scalar27),reinterpret_cast<U>(&write27),reinterpret_cast<U>(&read27),reinterpret_cast<U>(&is27),reinterpret_cast<U>(&yes)};
NativeSessionMessage28* __fastcall scalar28(NativeSessionMessage28* m,void*,U flags){return delete_native_session_message28_0075dd60(m,flags);}
void __fastcall write28(const NativeSessionMessage28* m,void*,NativeBitCursor* c){write_native_session_message25_0075dad0(m,c);}
void __fastcall read28(NativeSessionMessage28* m,void*,NativeSessionReadStream* s){read_native_session_message25_0075db10(m,s);}
bool __fastcall is28(const NativeSessionMessage28*,void*,U query){return native_session_message_is28_0075dd50(query);}
const U profile28[]={reinterpret_cast<U>(&scalar28),reinterpret_cast<U>(&write28),reinterpret_cast<U>(&read28),reinterpret_cast<U>(&is28),reinterpret_cast<U>(&yes)};
NativeSessionMessage29* __fastcall scalar29(NativeSessionMessage29* m,void*,U flags){return delete_native_session_message29_0075dfe0(m,flags);}
void __fastcall write29(const NativeSessionMessage29* m,void*,NativeBitCursor* c){write_native_session_message29_0075df60(m,c);}
void __fastcall read29(NativeSessionMessage29* m,void*,NativeSessionReadStream* s){read_native_session_message29_0075dfa0(m,s);}
bool __fastcall is29(const NativeSessionMessage29*,void*,U query){return native_session_message_is29_0075df50(query);}
const U profile29[]={reinterpret_cast<U>(&scalar29),reinterpret_cast<U>(&write29),reinterpret_cast<U>(&read29),reinterpret_cast<U>(&is29),reinterpret_cast<U>(&yes)};
NativeSessionMessage30* __fastcall scalar30(NativeSessionMessage30* m,void*,U flags){return delete_native_session_message30_0075e250(m,flags);}
void __fastcall write30(const NativeSessionMessage30* m,void*,NativeBitCursor* c){write_native_session_message30_0075e1d0(m,c);}
void __fastcall read30(NativeSessionMessage30* m,void*,NativeSessionReadStream* s){read_native_session_message30_0075e210(m,s);}
bool __fastcall is30(const NativeSessionMessage30*,void*,U query){return native_session_message_is30_0075e1c0(query);}
const U profile30[]={reinterpret_cast<U>(&scalar30),reinterpret_cast<U>(&write30),reinterpret_cast<U>(&read30),reinterpret_cast<U>(&is30),reinterpret_cast<U>(&yes)};
NativeSessionMessage31* __fastcall scalar31(NativeSessionMessage31* m,void*,U flags){return delete_native_session_message31_0075e380(m,flags);}
void __fastcall write31(const NativeSessionMessage31* m,void*,NativeBitCursor* c){write_native_session_message31_0075e300(m,c);}
void __fastcall read31(NativeSessionMessage31* m,void*,NativeSessionReadStream* s){read_native_session_message31_0075e340(m,s);}
bool __fastcall is31(const NativeSessionMessage31*,void*,U query){return native_session_message_is31_0075e2f0(query);}
const U profile31[]={reinterpret_cast<U>(&scalar31),reinterpret_cast<U>(&write31),reinterpret_cast<U>(&read31),reinterpret_cast<U>(&is31),reinterpret_cast<U>(&yes)};
NativeSessionMessage32* __fastcall scalar32(NativeSessionMessage32* m,void*,U flags){return delete_native_session_message32_0075e4b0(m,flags);}
void __fastcall write32(const NativeSessionMessage32* m,void*,NativeBitCursor* c){write_native_session_message32_0075e430(m,c);}
void __fastcall read32(NativeSessionMessage32* m,void*,NativeSessionReadStream* s){read_native_session_message32_0075e470(m,s);}
bool __fastcall is32(const NativeSessionMessage32*,void*,U query){return native_session_message_is32_0075e420(query);}
const U profile32[]={reinterpret_cast<U>(&scalar32),reinterpret_cast<U>(&write32),reinterpret_cast<U>(&read32),reinterpret_cast<U>(&is32),reinterpret_cast<U>(&yes)};
}
const U* native_session_message26_profile_00d03084(){return profile26;}
NativeSessionMessage26* construct_native_session_message26_0075db70(NativeSessionMessage26* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,26,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile26;return m;
}
bool native_session_message_is26_0075dbf0(U query){return query==26;}
NativeSessionMessage26* delete_native_session_message26_0075dc00(NativeSessionMessage26* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
const U* native_session_message27_profile_00d03098(){return profile27;}
NativeSessionMessage27* construct_native_session_message27_0075dc20(NativeSessionMessage27* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,27,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile27;return m;
}
bool native_session_message_is27_0075dca0(U query){return query==27;}
NativeSessionMessage27* delete_native_session_message27_0075dcb0(NativeSessionMessage27* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
const U* native_session_message28_profile_00d030ac(){return profile28;}
NativeSessionMessage28* construct_native_session_message28_0075dcd0(NativeSessionMessage28* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,28,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile28;return m;
}
bool native_session_message_is28_0075dd50(U query){return query==28;}
NativeSessionMessage28* delete_native_session_message28_0075dd60(NativeSessionMessage28* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
const U* native_session_message29_profile_00d030d4(){return profile29;}
NativeSessionMessage29* construct_native_session_message29_0075ded0(NativeSessionMessage29* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,29,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile29;return m;
}
bool native_session_message_is29_0075df50(U query){return query==29;}
NativeSessionMessage29* delete_native_session_message29_0075dfe0(NativeSessionMessage29* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
void write_native_session_message29_0075df60(const NativeSessionMessage29* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_unsigned_dword_bits_00429070(c,m->value_18,6);
    write_native_bool_bit_004290b0(c,m->first_1c);
    write_native_bool_bit_004290b0(c,m->second_1d);
}
void read_native_session_message29_0075dfa0(NativeSessionMessage29* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    auto* c=&s->cursor_04;read_native_u32_bits_00428d10(c,&m->value_18,6);
    read_flag(c,&m->first_1c);read_flag(c,&m->second_1d);
}
const U* native_session_message30_profile_00d030fc(){return profile30;}
NativeSessionMessage30* construct_native_session_message30_0075e140(NativeSessionMessage30* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,30,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile30;return m;
}
bool native_session_message_is30_0075e1c0(U query){return query==30;}
NativeSessionMessage30* delete_native_session_message30_0075e250(NativeSessionMessage30* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
void write_native_session_message30_0075e1d0(const NativeSessionMessage30* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_unsigned_dword_bits_00429070(c,m->value_18,6);
    write_native_bool_bit_004290b0(c,m->flag_1c);
}
void read_native_session_message30_0075e210(NativeSessionMessage30* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    auto* c=&s->cursor_04;read_native_u32_bits_00428d10(c,&m->value_18,6);
    read_flag(c,&m->flag_1c);
}
const U* native_session_message31_profile_00d03110(){return profile31;}
NativeSessionMessage31* construct_native_session_message31_0075e270(NativeSessionMessage31* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,31,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile31;return m;
}
bool native_session_message_is31_0075e2f0(U query){return query==31;}
NativeSessionMessage31* delete_native_session_message31_0075e380(NativeSessionMessage31* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
void write_native_session_message31_0075e300(const NativeSessionMessage31* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_unsigned_dword_bits_00429070(c,m->value_18,6);
    const B value=*reinterpret_cast<const volatile B*>(&m->value_1c);
    write_native_byte_bits_00428ff0(c,value,3);
}
void read_native_session_message31_0075e340(NativeSessionMessage31* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    auto* c=&s->cursor_04;read_native_u32_bits_00428d10(c,&m->value_18,6);
    B value;read_native_u8_bits_00428c70(c,&value,3);
    m->value_1c=value; // Native local decode precedes the full DWORD store.
}
const U* native_session_message32_profile_00d03124(){return profile32;}
NativeSessionMessage32* construct_native_session_message32_0075e3a0(NativeSessionMessage32* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,32,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile32;return m;
}
bool native_session_message_is32_0075e420(U query){return query==32;}
NativeSessionMessage32* delete_native_session_message32_0075e4b0(NativeSessionMessage32* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
void write_native_session_message32_0075e430(const NativeSessionMessage32* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_unsigned_dword_bits_00429070(c,m->value_18,6);
    write_native_bool_bit_004290b0(c,m->flag_1c);
}
void read_native_session_message32_0075e470(NativeSessionMessage32* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    auto* c=&s->cursor_04;read_native_u32_bits_00428d10(c,&m->value_18,6);
    read_flag(c,&m->flag_1c);
}
} // namespace bsp
