#include "bsp/native_session_message_tags_103_to106.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;
using B=std::uint8_t;
using Context=NativeSessionMessage103To106Context;
U sse_bits(const volatile float* source){U result;__asm {mov eax,source
 movss xmm0,dword ptr [eax]
 movss dword ptr result,xmm0}return result;}
U x87_bits(const volatile void* source){U result;__asm {mov eax,source
 fld dword ptr [eax]
 fstp dword ptr result}return result;}
const Context& profile_context(const void* m){
    const auto* slots=*static_cast<const U* const*>(m);
    const auto* scalar=reinterpret_cast<const NativeSessionMessage99To102ProfileStorage*>(slots)->context;
    return *reinterpret_cast<const Context*>(scalar);
}
void finish_header(NativeSessionMessage98ScalarStorage* m){
    volatile auto& h=m->header;h.sender_18=0;h.relay_1a=0;h.flag_1c=0;h.base.delivery_04=1;
}
void __fastcall write103(const NativeSessionMessage103* m,void*,NativeBitCursor* cursor){write_native_session_message103_0075c640(m,cursor,profile_context(m));}
void __fastcall read103(NativeSessionMessage103* m,void*,NativeSessionReadStream* stream){read_native_session_message103_0075c660(m,stream,profile_context(m));}
void __fastcall write106(const NativeSessionMessage106* m,void*,NativeBitCursor* cursor){write_native_session_message106_007617e0(m,cursor,profile_context(m));}
void __fastcall read106(NativeSessionMessage106* m,void*,NativeSessionReadStream* stream){read_native_session_message106_00761860(m,stream,profile_context(m));}
#define BSP_ADAPTERS(N,P,S) \
NativeSessionMessage##N* __fastcall del##N(NativeSessionMessage##N* m,void*,U flags){return delete_native_session_message##N##_##S(m,flags);} \
bool __fastcall is##N(const void*,void*,U type){return native_session_message_is##N##_##P(type);}
BSP_ADAPTERS(103,0075a240,0075a270)
BSP_ADAPTERS(104,00728f70,00729050)
BSP_ADAPTERS(105,00728ff0,00729070)
BSP_ADAPTERS(106,00761790,007617c0)
#undef BSP_ADAPTERS
// Reuse the same adapter addresses as profiles99/100/102, including context layout.
U scalar_slot(const Context& c,U index){return NativeSessionMessage100Profile(c.scalar).slots[index];}
}
#define BSP_PROFILE(N,P,D,S,W,R) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const Context& c):NativeSessionMessage99To102ProfileStorage(c.scalar,reinterpret_cast<U>(&del##N),W,R,reinterpret_cast<U>(&is##N)){} \
bool native_session_message_is##N##_##P(U t){return t==N||t==98||t==73||t==70;} \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N* m){*reinterpret_cast<const U* volatile*>(m)=native_session_message_root_profile_00ce4974();} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* m,U flags){destroy_native_session_message##N##_##D(m);if(flags&1)singleton_lifetime_free(m);return m;}
BSP_PROFILE(103,0075a240,0075a230,0075a270,reinterpret_cast<U>(&write103),reinterpret_cast<U>(&read103))
BSP_PROFILE(104,00728f70,00728f60,00729050,scalar_slot(c,1),scalar_slot(c,2))
BSP_PROFILE(105,00728ff0,00728fe0,00729070,scalar_slot(c,1),scalar_slot(c,2))
BSP_PROFILE(106,00761790,00761780,007617c0,reinterpret_cast<U>(&write106),reinterpret_cast<U>(&read106))
#undef BSP_PROFILE
NativeSessionMessage103* construct_native_session_message103_0075a1f0(NativeSessionMessage103* m,const Context& c,const NativeSessionMessage103Profile& p){
    const U value=sse_bits(c.scalar.default_00cf8b3c);volatile auto& h=m->scalar.header;
    h.base.field_08=0;h.base.field_0c=0;h.base.selected_owner_14=nullptr;h.base.type_10=0;
    h.sender_18=0;h.relay_1a=0;h.base.delivery_04=1;h.flag_1c=0;
    static_cast<volatile NativeSessionMessage103&>(*m).scalar.value_20=value;h.base.profile_00=p.slots;return m;
}
NativeSessionMessage104* construct_native_session_message104_00728f20(NativeSessionMessage104* m,const Context& c,const NativeSessionMessage104Profile& p){
    construct_native_session_message_0075b430(&m->header.base,104,*c.scalar.session);
    const U value=sse_bits(c.scalar.default_00cf8b3c);finish_header(m);
    static_cast<volatile NativeSessionMessage104&>(*m).value_20=value;static_cast<volatile NativeSessionMessage104&>(*m).header.base.profile_00=p.slots;return m;
}
NativeSessionMessage105* construct_native_session_message105_00728fa0(NativeSessionMessage105* m,const Context& c,const NativeSessionMessage105Profile& p){
    construct_native_session_message_0075b430(&m->header.base,105,*c.scalar.session);
    const U value=sse_bits(c.scalar.default_00cf8b3c);finish_header(m);
    static_cast<volatile NativeSessionMessage105&>(*m).value_20=value;static_cast<volatile NativeSessionMessage105&>(*m).header.base.profile_00=p.slots;return m;
}
NativeSessionMessage106* construct_native_session_message106_00761710(NativeSessionMessage106* m,const Context& c,const NativeSessionMessage106Profile& p){
    construct_native_session_message_0075b430(&m->scalar.header.base,106,*c.scalar.session);
    const U value=sse_bits(c.scalar.default_00cf8b3c);finish_header(&m->scalar);
    static_cast<volatile NativeSessionMessage106&>(*m).scalar.value_20=value;static_cast<volatile NativeSessionMessage106&>(*m).scalar.header.base.profile_00=p.slots;return m;
}
void write_native_session_message103_0075c640(const NativeSessionMessage103* m,NativeBitCursor* cursor,const Context& c){
    write_native_session_scalar98_006d2f20(&m->scalar,cursor,c.scalar);
    write_native_bool_bit_004290b0(cursor,static_cast<const volatile NativeSessionMessage103&>(*m).flag_24);
}
void read_native_session_message103_0075c660(NativeSessionMessage103* m,NativeSessionReadStream* stream,const Context& c){
    read_native_session_scalar98_006d1550(&m->scalar,stream,c.scalar);
    read_native_bool_bit_00428d70(&stream->cursor_04,reinterpret_cast<bool*>(&m->flag_24));
}
void write_native_session_message106_007617e0(const NativeSessionMessage106* m,NativeBitCursor* cursor,const Context& c){
    write_native_session_scalar98_006d2f20(&m->scalar,cursor,c.scalar);
    write_native_signed_dword_bits_00429090(cursor,static_cast<const volatile NativeSessionMessage106&>(*m).value_24,32);
    write_native_signed_dword_bits_00429090(cursor,static_cast<const volatile NativeSessionMessage106&>(*m).value_28,32);
    U scale=x87_bits(c.maximum_00d7a248);U value=x87_bits(&m->value_2c);
    write_native_numeric_float_004295c0(cursor,value,0,1,scale,32,*c.scalar.numeric);
    scale=x87_bits(c.maximum_00d7a248);value=x87_bits(&m->value_30);
    write_native_numeric_float_004295c0(cursor,value,0,1,scale,32,*c.scalar.numeric);
    write_native_signed_dword_bits_00429090(cursor,static_cast<const volatile NativeSessionMessage106&>(*m).value_34,32);
}
void read_native_session_message106_00761860(NativeSessionMessage106* m,NativeSessionReadStream* stream,const Context& c){
    read_native_session_scalar98_006d1550(&m->scalar,stream,c.scalar);auto* cursor=&stream->cursor_04;
    read_native_i32_bits_00428d30(cursor,&m->value_24,32);read_native_i32_bits_00428d30(cursor,&m->value_28,32);
    U scale=x87_bits(c.maximum_00d7a248);read_native_numeric_float_004293f0(cursor,&m->value_2c,0,1,scale,32,*c.scalar.numeric);
    scale=x87_bits(c.maximum_00d7a248);read_native_numeric_float_004293f0(cursor,&m->value_30,0,1,scale,32,*c.scalar.numeric);
    read_native_i32_bits_00428d30(cursor,&m->value_34,32);
}
} // namespace bsp
