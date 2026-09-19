#include "bsp/native_session_message_tags_99_to102.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_render_batch_keys.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using B=std::uint8_t;using Storage=NativeSessionMessage98ScalarStorage;using Context=NativeSessionMessage99To102Context;
U sse_bits(const volatile float* source){U result;__asm {mov eax,source
 movss xmm0,dword ptr [eax]
 movss dword ptr result,xmm0}return result;}
bool value_present(const U* value,const volatile double* threshold){B result;__asm {mov eax,value
 fld dword ptr [eax]
 mov ecx,threshold
 fld qword ptr [ecx]
 fcomip st(0),st(1)
 fstp st(0)
 setb result}return result!=0;}
I scale_and_convert(const U* value,const volatile double* scale,const volatile U* mode_address){I result;__asm {mov eax,value
 fld dword ptr [eax]
 mov edx,scale
 fmul qword ptr [edx]
 mov ecx,mode_address
 call native_crt_truncate_st0_00bf7420
 mov result,eax}return result;}
void decode_scaled(U* destination,U value,const volatile float* bias_address,const volatile double* scale){__asm {fild dword ptr value
 test value,080000000h
 jz decode_scale
 mov eax,bias_address
 fadd dword ptr [eax]
decode_scale:
 mov ecx,scale
 fmul qword ptr [ecx]
 mov edx,destination
 fstp dword ptr [edx]}}
void capture_base(Storage* m,B type,const Context& c){construct_native_session_message_0075b430(&m->header.base,type,*c.session);}
void finish_header(Storage* m){volatile auto& h=m->header;h.sender_18=0;h.relay_1a=0;h.flag_1c=0;h.base.delivery_04=1;}
const Context& profile_context(const void* m){const auto* slots=*static_cast<const U* const*>(m);return *reinterpret_cast<const NativeSessionMessage99To102ProfileStorage*>(slots)->context;}
void __fastcall write_scalar(const Storage* m,void*,NativeBitCursor* cursor){write_native_session_scalar98_006d2f20(m,cursor,profile_context(m));}
void __fastcall read_scalar(Storage* m,void*,NativeSessionReadStream* stream){read_native_session_scalar98_006d1550(m,stream,profile_context(m));}
void __fastcall write101(const NativeSessionMessage101* m,void*,NativeBitCursor* cursor){write_native_session_message101_0075c520(m,cursor,profile_context(m));}
void __fastcall read101(NativeSessionMessage101* m,void*,NativeSessionReadStream* stream){read_native_session_message101_0075c540(m,stream,profile_context(m));}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
#define BSP_ADAPTERS(N,P,S) \
NativeSessionMessage##N* __fastcall del##N(NativeSessionMessage##N* m,void*,U flags){return delete_native_session_message##N##_##S(m,flags);} \
bool __fastcall is##N(const void*,void*,U type){return native_session_message_is##N##_##P(type);}
BSP_ADAPTERS(99,0075c400,0075c430)
BSP_ADAPTERS(100,0075c4d0,0075c500)
BSP_ADAPTERS(101,0075a1a0,0075a1d0)
BSP_ADAPTERS(102,0075c5f0,0075c620)
#undef BSP_ADAPTERS
}
NativeSessionMessage99To102ProfileStorage::NativeSessionMessage99To102ProfileStorage(const Context& c,U scalar,U writer,U reader,U predicate):slots{scalar,writer,reader,predicate,reinterpret_cast<U>(&yes)},context(&c){}
#define BSP_PROFILE(N,P,D,S,W,R) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const Context& c):NativeSessionMessage99To102ProfileStorage(c,reinterpret_cast<U>(&del##N),reinterpret_cast<U>(&W),reinterpret_cast<U>(&R),reinterpret_cast<U>(&is##N)){} \
bool native_session_message_is##N##_##P(U t){return t==N||t==98||t==73||t==70;} \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N* m){*reinterpret_cast<const U* volatile*>(m)=native_session_message_root_profile_00ce4974();} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* m,U flags){destroy_native_session_message##N##_##D(m);if(flags&1)singleton_lifetime_free(m);return m;}
BSP_PROFILE(99,0075c400,0075c3f0,0075c430,write_scalar,read_scalar)
BSP_PROFILE(100,0075c4d0,0075c4c0,0075c500,write_scalar,read_scalar)
BSP_PROFILE(101,0075a1a0,0075a190,0075a1d0,write101,read101)
BSP_PROFILE(102,0075c5f0,0075c5e0,0075c620,write_scalar,read_scalar)
#undef BSP_PROFILE
NativeSessionMessage99* construct_native_session_message99_0075c380(NativeSessionMessage99* m,const Context& c,const NativeSessionMessage99Profile& p){capture_base(m,99,c);const U value=sse_bits(&c.numeric->minus_one_00d7a260);finish_header(m);static_cast<volatile NativeSessionMessage99&>(*m).header.base.profile_00=p.slots;static_cast<volatile NativeSessionMessage99&>(*m).value_20=value;return m;}
NativeSessionMessage100* construct_native_session_message100_0075c450(NativeSessionMessage100* m,const Context& c,const NativeSessionMessage100Profile& p){capture_base(m,100,c);const U value=sse_bits(c.default_00cf8b3c);finish_header(m);static_cast<volatile NativeSessionMessage100&>(*m).value_20=value;static_cast<volatile NativeSessionMessage100&>(*m).header.base.profile_00=p.slots;return m;}
NativeSessionMessage102* construct_native_session_message102_0075c570(NativeSessionMessage102* m,const Context& c,const NativeSessionMessage102Profile& p){capture_base(m,102,c);const U value=sse_bits(c.default_00cf8b3c);finish_header(m);static_cast<volatile NativeSessionMessage102&>(*m).value_20=value;static_cast<volatile NativeSessionMessage102&>(*m).header.base.profile_00=p.slots;return m;}
NativeSessionMessage101* construct_native_session_message101_0075a150(NativeSessionMessage101* m,const Context& c,const NativeSessionMessage101Profile& p){const U value=sse_bits(c.default_00cf8b3c);volatile auto& h=m->scalar.header;h.base.field_08=0;h.base.field_0c=0;h.base.selected_owner_14=nullptr;h.base.type_10=0;h.sender_18=0;h.relay_1a=0;h.base.delivery_04=1;h.flag_1c=0;static_cast<volatile NativeSessionMessage101&>(*m).scalar.value_20=value;h.base.profile_00=p.slots;return m;}
void write_native_session_scalar98_006d2f20(const Storage* m,NativeBitCursor* cursor,const Context& c){
    write_native_session_message_header_0075b480(m,cursor);write_native_bool_bit_004290b0(cursor,static_cast<const volatile Storage&>(*m).header.flag_1c);
    if(!value_present(&m->value_20,c.threshold_00d7a3a0)){write_native_bool_bit_004290b0(cursor,0);return;}
    write_native_bool_bit_004290b0(cursor,1);I value=scale_and_convert(&m->value_20,c.write_scale_00d7a328,&c.numeric->conversion_mode_0109eea4);if(value<0)value=0;else if(value>255)value=255;write_native_unsigned_dword_bits_00429070(cursor,static_cast<U>(value),8);
}
void read_native_session_scalar98_006d1550(Storage* m,NativeSessionReadStream* stream,const Context& c){
    read_native_session_message_header_0075b4c0(m,stream);auto* cursor=&stream->cursor_04;read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->header.flag_1c));bool present;read_native_bool_bit_00428d70(cursor,&present);
    if(present){U value;read_native_u32_bits_00428d10(cursor,&value,8);decode_scaled(&m->value_20,value,&c.numeric->uint32_bias_00ce3978,c.read_scale_00d7a348);}else m->value_20=sse_bits(&c.numeric->minus_one_00d7a260);
}
void write_native_session_message101_0075c520(const NativeSessionMessage101* m,NativeBitCursor* cursor,const Context& c){write_native_session_scalar98_006d2f20(&m->scalar,cursor,c);write_native_unsigned_dword_bits_00429070(cursor,static_cast<const volatile NativeSessionMessage101&>(*m).value_24,2);}
void read_native_session_message101_0075c540(NativeSessionMessage101* m,NativeSessionReadStream* stream,const Context& c){read_native_session_scalar98_006d1550(&m->scalar,stream,c);read_native_u32_bits_00428d10(&stream->cursor_04,&m->value_24,2);}
} // namespace bsp
