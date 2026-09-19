#include "bsp/native_session_message_tags_90_to93.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/cruise_command.hpp"
#include <cstring>
namespace bsp {
namespace {
using U=std::uint32_t;using B=std::uint8_t;using Context=NativeSessionMessage90To93Context;
U x87_copy(const volatile float* source){U result;__asm {mov eax,source
 fld dword ptr [eax]
 fstp dword ptr result}return result;}
void sse_copy(volatile float* destination,const volatile float* source){__asm {mov eax,source
 mov edx,destination
 movss xmm0,dword ptr [eax]
 movss dword ptr [edx],xmm0}}
bool trailing_present(const float* value,const volatile float* threshold){unsigned char result;__asm {mov eax,value
 mov ecx,threshold
 movss xmm0,dword ptr [eax]
 comiss xmm0,dword ptr [ecx]
 setae result}return result!=0;}
void zero_header(NativeMessage75ExtendedHeader& h,const U* profile){volatile auto& x=h;x.base.field_08=0;x.base.field_0c=0;x.base.selected_owner_14=nullptr;x.base.type_10=0;x.sender_18=0;x.relay_1a=0;x.base.delivery_04=1;x.flag_1c=0;x.base.profile_00=profile;}
void write_header(const NativeMessage75ExtendedHeader& h,NativeBitCursor* cursor){write_native_session_message_header_0075b480(&h,cursor);write_native_bool_bit_004290b0(cursor,static_cast<const volatile NativeMessage75ExtendedHeader&>(h).flag_1c);}
void read_header(NativeMessage75ExtendedHeader& h,NativeSessionReadStream* stream){read_native_session_message_header_0075b4c0(&h,stream);read_native_bool_bit_00428d70(&stream->cursor_04,reinterpret_cast<bool*>(&h.flag_1c));}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
bool __fastcall is90(const void*,void*,U t){return native_session_message_is90_0071c640(t);}
bool __fastcall is91(const void*,void*,U t){return native_session_message_is91_0075aff0(t);}
bool __fastcall is92(const void*,void*,U t){return gameunit_set_command_message_is_category_0071c900(static_cast<std::int32_t>(t));}
bool __fastcall is93(const void*,void*,U t){return native_session_message_is93_0071c770(t);}
#define BSP_ADAPTERS(N,W,R,S) \
const NativeSessionMessage##N##Profile& profile##N(const NativeSessionMessage##N* m){return *reinterpret_cast<const NativeSessionMessage##N##Profile*>(m->header.base.profile_00);} \
NativeSessionMessage##N* __fastcall del##N(NativeSessionMessage##N* m,void*,U flags){return delete_native_session_message##N##_##S(m,flags);} \
void __fastcall write##N(const NativeSessionMessage##N* m,void*,NativeBitCursor* cursor){write_native_session_message##N##_##W(m,cursor,*profile##N(m).context);} \
void __fastcall read##N(NativeSessionMessage##N* m,void*,NativeSessionReadStream* stream){read_native_session_message##N##_##R(m,stream,*profile##N(m).context);}
BSP_ADAPTERS(90,0071c670,0071c6c0,0071e370)
BSP_ADAPTERS(91,00763fc0,00764030,007640b0)
BSP_ADAPTERS(92,0071c930,0071ca30,0071e3b0)
BSP_ADAPTERS(93,0071c7a0,0071c7e0,0071e390)
#undef BSP_ADAPTERS
}
#define BSP_PROFILE(N,D,S) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const Context& c):slots{reinterpret_cast<U>(&del##N),reinterpret_cast<U>(&write##N),reinterpret_cast<U>(&read##N),reinterpret_cast<U>(&is##N),reinterpret_cast<U>(&yes)},context(&c){} \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N* m){static_cast<volatile NativeSessionMessageStorage&>(m->header.base).profile_00=native_session_message_root_profile_00ce4974();} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* m,U flags){destroy_native_session_message##N##_##D(m);if(flags&1)singleton_lifetime_free(m);return m;}
BSP_PROFILE(90,0071c720,0071e370)
BSP_PROFILE(91,0075b020,007640b0)
BSP_PROFILE(92,0071cb40,0071e3b0)
BSP_PROFILE(93,0071c820,0071e390)
#undef BSP_PROFILE
bool native_session_message_is90_0071c640(U t){return t==90||t==89||t==73||t==70;}
bool native_session_message_is91_0075aff0(U t){return t==91||t==89||t==73||t==70;}
bool native_session_message_is93_0071c770(U t){return t==93||t==89||t==73||t==70;}
NativeSessionMessage90* construct_native_session_message90_0075af60(NativeSessionMessage90* m,const Context&,const NativeSessionMessage90Profile& p){zero_header(m->header,p.slots);return m;}
NativeSessionMessage91* construct_native_session_message91_0075af90(NativeSessionMessage91* m,const Context&,const NativeSessionMessage91Profile& p){zero_header(m->header,p.slots);static_cast<volatile NativeSessionMessage91&>(*m).value_24=1;static_cast<volatile NativeSessionMessage91&>(*m).value_28=5;return m;}
NativeSessionMessage93* construct_native_session_message93_0075b030(NativeSessionMessage93* m,const Context&,const NativeSessionMessage93Profile& p){zero_header(m->header,p.slots);return m;}
NativeSessionMessage92* construct_native_session_message92_00764f90(NativeSessionMessage92* m,const Context& c,const NativeSessionMessage92Profile& p){
    zero_header(m->header,p.slots);volatile auto& t=m->target_24;for(U i=0;i<3;++i)sse_copy(&t.position[i],c.target.default_vector_00f87574+i);
    *reinterpret_cast<volatile std::uint16_t*>(&t.kind)=0;t.object=nullptr;t.kind=0;t.object_id=0;const U zero=0;std::memcpy(&m->target_24.trailing,&zero,4);return m;
}
NativeSessionMessage92* construct_native_session_message92_value_0071c830(NativeSessionMessage92* m,const void* command,const SceneCommandTarget* target,B flag,const Context& c,const NativeSessionMessage92Profile& p){
    construct_native_session_message_0075b430(&m->header.base,92,*c.session);volatile auto& x=*m;x.flags_20=flag;x.header.base.delivery_04=1;x.header.base.profile_00=p.slots;x.header.sender_18=0;x.header.relay_1a=0;x.header.flag_1c=0;
    *reinterpret_cast<volatile std::uint16_t*>(&x.target_24.kind)=*reinterpret_cast<const volatile std::uint16_t*>(&target->kind);x.target_24.object_id=static_cast<const volatile SceneCommandTarget&>(*target).object_id;x.target_24.object=static_cast<const volatile SceneCommandTarget&>(*target).object;
    for(U i=0;i<3;++i){const U bits=x87_copy(&target->position[i]);std::memcpy(&m->target_24.position[i],&bits,4);}const U bits=x87_copy(&target->trailing);std::memcpy(&m->target_24.trailing,&bits,4);
    x.command_21=command?static_cast<B>(*reinterpret_cast<const volatile U*>(static_cast<const B*>(command)+4)):0xff;return m;
}
void write_native_session_message90_0071c670(const NativeSessionMessage90* m,NativeBitCursor* cursor,const Context&){write_header(m->header,cursor);const volatile auto& x=*m;write_native_unsigned_dword_bits_00429070(cursor,x.selector_20,4);const U value=x.value_24;const U width=x.selector_20==2?9u:1u;write_native_unsigned_dword_bits_00429070(cursor,value,width);}
void read_native_session_message90_0071c6c0(NativeSessionMessage90* m,NativeSessionReadStream* stream,const Context&){read_header(m->header,stream);auto* cursor=&stream->cursor_04;U selector,value;read_native_u32_bits_00428d10(cursor,&selector,4);m->selector_20=selector;read_native_u32_bits_00428d10(cursor,&value,selector==2?9u:1u);m->value_24=value;}
void write_native_session_message91_00763fc0(const NativeSessionMessage91* m,NativeBitCursor* cursor,const Context&){const volatile auto& x=*m;write_native_byte_bits_00428ff0(cursor,x.header.base.type_10,8);write_native_word_bits_00429120(cursor,x.header.sender_18,12);write_native_bool_bit_004290b0(cursor,x.header.relay_1a);write_native_bool_bit_004290b0(cursor,x.header.flag_1c);write_native_word_bits_00429120(cursor,x.value_20,16);write_native_unsigned_dword_bits_00429070(cursor,x.value_24,2);write_native_unsigned_dword_bits_00429070(cursor,x.value_28,4);}
void read_native_session_message91_00764030(NativeSessionMessage91* m,NativeSessionReadStream* stream,const Context&){auto* cursor=&stream->cursor_04;read_native_u8_bits_00428c70(cursor,&m->header.base.type_10,8);read_native_word_bits_00428e30(cursor,&m->header.sender_18,12);read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->header.relay_1a));read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->header.flag_1c));read_native_word_bits_00428e30(cursor,&m->value_20,16);U a,b;read_native_u32_bits_00428d10(cursor,&a,2);read_native_u32_bits_00428d10(cursor,&b,4);m->value_24=a;m->value_28=b;}
void write_native_session_message93_0071c7a0(const NativeSessionMessage93* m,NativeBitCursor* cursor,const Context&){write_header(m->header,cursor);write_native_bool_bit_004290b0(cursor,static_cast<const volatile NativeSessionMessage93&>(*m).flag_20);write_native_signed_dword_bits_00429090(cursor,static_cast<const volatile NativeSessionMessage93&>(*m).value_24,5);}
void read_native_session_message93_0071c7e0(NativeSessionMessage93* m,NativeSessionReadStream* stream,const Context&){read_header(m->header,stream);read_native_bool_bit_00428d70(&stream->cursor_04,reinterpret_cast<bool*>(&m->flag_20));read_native_i32_bits_00428d30(&stream->cursor_04,&m->value_24,5);}
void write_native_session_message92_0071c930(const NativeSessionMessage92* m,NativeBitCursor* cursor,const Context& c){
    write_header(m->header,cursor);const volatile auto& x=*m;write_native_bool_bit_004290b0(cursor,x.flags_20);write_native_byte_bits_00428ff0(cursor,x.command_21,8);
    write_native_bool_bit_004290b0(cursor,x.target_24.kind);if(x.target_24.kind)write_native_word_bits_00429120(cursor,x.target_24.object_id,13);
    write_native_bool_bit_004290b0(cursor,x.target_24.position_valid);if(x.target_24.position_valid){const U scale=x87_copy(c.position_scale_00cf9360);U values[3];std::memcpy(values,m->target_24.position,12);write_native_numeric_float_array_00429790(cursor,values,3,0,1,scale,24,*c.target.numeric);}
    if(trailing_present(&m->target_24.trailing,c.target.threshold_00d7a24c)){write_native_bool_bit_004290b0(cursor,1);const U scale=x87_copy(c.target.maximum_00d7a248);const U value=x87_copy(&m->target_24.trailing);write_native_numeric_float_004295c0(cursor,value,0,1,scale,32,*c.target.numeric);}else write_native_bool_bit_004290b0(cursor,0);
}
void read_native_session_message92_0071ca30(NativeSessionMessage92* m,NativeSessionReadStream* stream,const Context& c){
    read_header(m->header,stream);auto* cursor=&stream->cursor_04;read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->flags_20));read_native_u8_bits_00428c70(cursor,&m->command_21,8);
    bool present;read_native_bool_bit_00428d70(cursor,&present);if(present){std::uint16_t handle;read_native_word_bits_00428e30(cursor,&handle,13);m->target_24.kind=1;m->target_24.object_id=handle;m->target_24.object=nullptr;}
    read_native_bool_bit_00428d70(cursor,&present);if(present){U values[3];const U scale=x87_copy(c.position_scale_00cf9360);read_native_numeric_float_array_004294f0(cursor,values,3,0,1,scale,24,*c.target.numeric);m->target_24.position_valid=1;std::memcpy(m->target_24.position,values,12);}
    read_native_bool_bit_00428d70(cursor,&present);if(present){U value;const U scale=x87_copy(c.target.maximum_00d7a248);read_native_numeric_float_004293f0(cursor,&value,0,1,scale,32,*c.target.numeric);std::memcpy(&m->target_24.trailing,&value,4);}
}
} // namespace bsp
