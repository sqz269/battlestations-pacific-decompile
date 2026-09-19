#include "bsp/native_session_message_tags_85_to87.hpp"
#include "bsp/native_bit_cursor_fields.hpp"

namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using B=std::uint8_t;using Context=NativeSessionMessage85To87Context;
U x87_copy(const volatile void* source){U result;__asm {mov eax,source
 fld dword ptr [eax]
 fstp dword ptr result}return result;}
// Preserve COMISS unordered branches and the distinct native maximum-load order.
U clamp_first(const U* source,const volatile float* minimum,const volatile float* maximum){U result;__asm {
 mov eax,source
 mov ecx,minimum
 mov edx,maximum
 movss xmm0,dword ptr [eax]
 movss xmm1,dword ptr [ecx]
 comiss xmm1,xmm0
 movss xmm2,dword ptr [edx]
 jbe first_upper
 movss result,xmm1
 jmp first_done
first_upper:
 comiss xmm0,xmm2
 jbe first_keep
 movss result,xmm2
 jmp first_done
first_keep:
 movss result,xmm0
first_done:
 }return result;}
U clamp_third(const U* source,const volatile float* minimum,const volatile float* maximum){U result;__asm {
 mov eax,source
 mov ecx,minimum
 mov edx,maximum
 movss xmm0,dword ptr [eax]
 movss xmm1,dword ptr [ecx]
 comiss xmm1,xmm0
 jbe third_upper
 movss result,xmm1
 jmp third_done
third_upper:
 movss xmm1,dword ptr [edx]
 comiss xmm0,xmm1
 jbe third_keep
 movss result,xmm1
 jmp third_done
third_keep:
 movss result,xmm0
third_done:
 }return result;}
U offset_and_clamp(const U* source,const volatile double* bias_address,const volatile double* upper,const volatile float* range){U temporary,result;__asm {
 mov eax,source
 mov ecx,bias_address
 fld dword ptr [eax]
 fadd qword ptr [ecx]
 fstp dword ptr temporary
 fld dword ptr temporary
 fldz
 fcomip st(0),st(1)
 jbe offset_upper
 xorps xmm0,xmm0
 fstp st(0)
 jmp offset_done
offset_upper:
 mov edx,upper
 fld qword ptr [edx]
 fxch st(1)
 fcomip st(0),st(1)
 fstp st(0)
 jbe offset_keep
 mov eax,range
 movss xmm0,dword ptr [eax]
 jmp offset_done
offset_keep:
 movss xmm0,dword ptr temporary
offset_done:
 movss result,xmm0
 }return result;}
U clamp_last(const U* source,const volatile float* cap){U result;__asm {
 mov eax,source
 mov ecx,cap
 movss xmm0,dword ptr [eax]
 movss xmm1,dword ptr [ecx]
 comiss xmm1,xmm0
 jbe last_cap
 movss result,xmm0
 jmp last_done
last_cap:
 movss result,xmm1
last_done:
 }return result;}
void subtract_offset(U* value,const volatile double* bias_address){__asm {mov eax,value
 mov ecx,bias_address
 fld dword ptr [eax]
 fsub qword ptr [ecx]
 fstp dword ptr [eax]}}
void write_float(const U* value,const volatile float* scale,U sign,U width,NativeBitCursor* cursor,const Context& c){const U scale_bits=x87_copy(scale);const U bits=x87_copy(value);write_native_numeric_float_004295c0(cursor,bits,0,sign,scale_bits,width,*c.numeric);}
void read_float(U* value,const volatile float* scale,U sign,U width,NativeBitCursor* cursor,const Context& c){const U scale_bits=x87_copy(scale);read_native_numeric_float_004293f0(cursor,value,0,sign,scale_bits,width,*c.numeric);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
#define BSP_MESSAGE_ADAPTERS(N,C,P,W,R,D,S) \
const NativeSessionMessage##N##Profile& profile##N(const NativeSessionMessage##N* m){return *reinterpret_cast<const NativeSessionMessage##N##Profile*>(*reinterpret_cast<const U* const*>(m));} \
NativeSessionMessage##N* __fastcall del##N(NativeSessionMessage##N* m,void*,U f){return delete_native_session_message##N##_##S(m,f);} \
void __fastcall write##N(const NativeSessionMessage##N* m,void*,NativeBitCursor* cursor){write_native_session_message##N##_##W(m,cursor,*profile##N(m).context);} \
void __fastcall read##N(NativeSessionMessage##N* m,void*,NativeSessionReadStream* stream){read_native_session_message##N##_##R(m,stream,*profile##N(m).context);} \
bool __fastcall is##N(const void*,void*,U type){return native_session_message_is##N##_##P(type);}
BSP_MESSAGE_ADAPTERS(85,0075c020,0075c080,0075c190,0075c090,source,0075c360)
BSP_MESSAGE_ADAPTERS(86,0075a470,0075a4a0,0075fdb0,0075fe00,0075a4c0,0075fe70)
BSP_MESSAGE_ADAPTERS(87,00521f30,00521f60,00521f80,00521fa0,00521fc0,00522ec0)
#undef BSP_MESSAGE_ADAPTERS
}
#define BSP_MESSAGE_PROFILE(N,C,P,W,R,D,S) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const Context& c):slots{reinterpret_cast<U>(&del##N),reinterpret_cast<U>(&write##N),reinterpret_cast<U>(&read##N),reinterpret_cast<U>(&is##N),reinterpret_cast<U>(&yes)},context(&c){} \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N* m){*reinterpret_cast<const U* volatile*>(m)=native_session_message_root_profile_00ce4974();} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* m,U flags){destroy_native_session_message##N##_##D(m);if(flags&1)singleton_lifetime_free(m);return m;}
BSP_MESSAGE_PROFILE(85,0075c020,0075c080,0075c190,0075c090,source,0075c360)
BSP_MESSAGE_PROFILE(86,0075a470,0075a4a0,0075fdb0,0075fe00,0075a4c0,0075fe70)
BSP_MESSAGE_PROFILE(87,00521f30,00521f60,00521f80,00521fa0,00521fc0,00522ec0)
#undef BSP_MESSAGE_PROFILE
bool native_session_message_is85_0075c080(U t){return t==85;}
bool native_session_message_is86_0075a4a0(U t){return t==86||t==73||t==70;}
bool native_session_message_is87_00521f60(U t){return t==87||t==73||t==70;}
NativeSessionMessage85* construct_native_session_message85_0075c020(NativeSessionMessage85* m,const Context& c,const NativeSessionMessage85Profile& p){
    volatile auto& b=m->base;b.delivery_04=3;b.field_08=0;b.field_0c=0;b.profile_00=native_session_message_base_profile_00d02c68();b.type_10=85;
    const auto* game=static_cast<const B*>(c.session->current_game_00e188a8);const I owner=*reinterpret_cast<const volatile I*>(game+0x18ec);
    if(owner>=0&&owner<=7){void* const selected=*reinterpret_cast<void* const volatile*>(game+0x18cc+static_cast<U>(owner)*4);b.delivery_04=0;b.selected_owner_14=selected;b.profile_00=p.slots;}
    else{b.selected_owner_14=nullptr;b.delivery_04=0;b.profile_00=p.slots;}return m;
}
NativeSessionMessage86* construct_native_session_message86_0075a470(NativeSessionMessage86* m,const Context&,const NativeSessionMessage86Profile& p){volatile auto& h=m->header;h.base.field_08=0;h.base.field_0c=0;h.base.selected_owner_14=nullptr;h.base.type_10=0;h.sender_18=0;h.relay_1a=0;h.base.delivery_04=1;h.flag_1c=0;h.base.profile_00=p.slots;return m;}
NativeSessionMessage87* construct_native_session_message87_00521f30(NativeSessionMessage87* m,const Context& c,const NativeSessionMessage87Profile& p){construct_native_session_message_0075b430(&m->header.base,87,*c.session);volatile auto& h=m->header;h.sender_18=0;h.relay_1a=0;h.flag_1c=0;h.base.delivery_04=1;h.base.profile_00=p.slots;return m;}
void write_native_session_message85_0075c190(const NativeSessionMessage85* m,NativeBitCursor* cursor,const Context& c){
    write_native_byte_bits_00428ff0(cursor,static_cast<const volatile NativeSessionMessage85&>(*m).base.type_10,8);
    U value=clamp_first(&m->float_18,c.minimum_00d02f68,c.maximum_00d02f64);write_float(&value,c.maximum_00d02f64,1,16,cursor,c);
    value=offset_and_clamp(&m->float_1c,c.offset_00cf0dd8,c.upper_00cf0aa0,c.range_00d02f60);write_float(&value,c.range_00d02f60,0,16,cursor,c);
    value=clamp_third(&m->float_20,c.minimum_00d02f68,c.maximum_00d02f64);write_float(&value,c.maximum_00d02f64,1,16,cursor,c);
    write_float(&m->float_24,c.angle_scale_00d7a264,1,10,cursor,c);write_float(&m->float_28,c.angle_scale_00d7a264,1,10,cursor,c);write_float(&m->float_2c,c.angle_scale_00d7a264,1,10,cursor,c);
    value=clamp_last(&m->float_30,c.cap_00ce380c);write_float(&value,c.scale_00ce3c64,0,8,cursor,c);write_native_bool_bit_004290b0(cursor,static_cast<const volatile NativeSessionMessage85&>(*m).flag_34);
}
void read_native_session_message85_0075c090(NativeSessionMessage85* m,NativeSessionReadStream* stream,const Context& c){
    auto* cursor=&stream->cursor_04;read_native_u8_bits_00428c70(cursor,&m->base.type_10,8);read_float(&m->float_18,c.maximum_00d02f64,1,16,cursor,c);read_float(&m->float_1c,c.range_00d02f60,0,16,cursor,c);subtract_offset(&m->float_1c,c.offset_00cf0dd8);read_float(&m->float_20,c.maximum_00d02f64,1,16,cursor,c);read_float(&m->float_24,c.angle_scale_00d7a264,1,10,cursor,c);read_float(&m->float_28,c.angle_scale_00d7a264,1,10,cursor,c);read_float(&m->float_2c,c.angle_scale_00d7a264,1,10,cursor,c);read_float(&m->float_30,c.scale_00ce3c64,0,8,cursor,c);read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->flag_34));
}
void write_native_session_message86_0075fdb0(const NativeSessionMessage86* m,NativeBitCursor* c,const Context&){const volatile auto& x=*m;write_native_byte_bits_00428ff0(c,x.header.base.type_10,8);write_native_word_bits_00429120(c,x.header.sender_18,12);write_native_bool_bit_004290b0(c,x.header.relay_1a);write_native_bool_bit_004290b0(c,x.header.flag_1c);write_native_unsigned_dword_bits_00429070(c,x.value_20,2);}
void read_native_session_message86_0075fe00(NativeSessionMessage86* m,NativeSessionReadStream* s,const Context&){auto* c=&s->cursor_04;read_native_u8_bits_00428c70(c,&m->header.base.type_10,8);read_native_word_bits_00428e30(c,&m->header.sender_18,12);read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&m->header.relay_1a));read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&m->header.flag_1c));read_native_u32_bits_00428d10(c,&m->value_20,2);}
void write_native_session_message87_00521f80(const NativeSessionMessage87* m,NativeBitCursor* c,const Context&){write_native_session_message_header_0075b480(m,c);write_native_bool_bit_004290b0(c,static_cast<const volatile NativeSessionMessage87&>(*m).header.flag_1c);}
void read_native_session_message87_00521fa0(NativeSessionMessage87* m,NativeSessionReadStream* s,const Context&){read_native_session_message_header_0075b4c0(m,s);read_native_bool_bit_00428d70(&s->cursor_04,reinterpret_cast<bool*>(&m->header.flag_1c));}
} // namespace bsp
