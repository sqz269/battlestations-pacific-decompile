#include "bsp/native_session_message_tags_111_112.hpp"
#include "bsp/native_bit_cursor_fields.hpp"

namespace bsp {
namespace {
using U=std::uint32_t;
using Context=NativeSessionMessage111To112Context;
U x87_bits(const volatile void* source){U result;__asm {mov eax,source
 fld dword ptr [eax]
 fstp dword ptr result}return result;}
#define BSP_ADAPTERS(N,P,S,W,R) \
const Context& context##N(const NativeSessionMessage##N* m){const auto* slots=*reinterpret_cast<const U* const*>(m);return *reinterpret_cast<const NativeSessionMessage##N##Profile*>(slots)->context;} \
NativeSessionMessage##N* __fastcall del##N(NativeSessionMessage##N* m,void*,U flags){return delete_native_session_message##N##_##S(m,flags);} \
bool __fastcall is##N(const void*,void*,U type){return native_session_message_is##N##_##P(type);} \
void __fastcall write##N(const NativeSessionMessage##N* m,void*,NativeBitCursor* cursor){write_native_session_message##N##_##W(m,cursor,context##N(m));} \
void __fastcall read##N(NativeSessionMessage##N* m,void*,NativeSessionReadStream* stream){read_native_session_message##N##_##R(m,stream,context##N(m));}
BSP_ADAPTERS(111,00761c90,00761cb0,00761cd0,00761d30)
BSP_ADAPTERS(112,00761df0,00762640,00761e20,00761ec0)
#undef BSP_ADAPTERS
bool __fastcall always(const void*,void*){return native_session_message_always_true_004499c0();}
}
#define BSP_PROFILE(N,D,S) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const Context& c):slots{reinterpret_cast<U>(&del##N),reinterpret_cast<U>(&write##N),reinterpret_cast<U>(&read##N),reinterpret_cast<U>(&is##N),reinterpret_cast<U>(&always)},context(&c){} \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N* m){*reinterpret_cast<const U* volatile*>(m)=native_session_message_root_profile_00ce4974();} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* m,U flags){destroy_native_session_message##N##_##D(m);if(flags&1)singleton_lifetime_free(m);return m;}
BSP_PROFILE(111,00761c80,00761cb0)
BSP_PROFILE(112,00761e10,00762640)
#undef BSP_PROFILE
bool native_session_message_is111_00761c90(U type){return type==111||type==73||type==70;}
bool native_session_message_is112_00761df0(U type){return type==112||type==70;}
NativeSessionMessage111* construct_native_session_message111_00761c20(NativeSessionMessage111* m,const Context& c,const NativeSessionMessage111Profile& p){
    // Native body inlines this provider's exact base-store/owner-capture sequence.
    construct_native_session_message_0075b430(&m->header.base,111,*c.session);
    volatile auto& h=m->header;h.sender_18=0;h.relay_1a=0;h.flag_1c=0;
    h.base.delivery_04=1;h.base.profile_00=p.slots;return m;
}
NativeSessionMessage112* construct_native_session_message112_00761d90(NativeSessionMessage112* m,const Context& c,const NativeSessionMessage112Profile& p){
    construct_native_session_message_0075b430(&m->base,112,*c.session);
    volatile auto& v=*m;v.sender_18=0;v.relay_1a=0;v.value_1c=0;
    v.base.delivery_04=1;v.base.profile_00=p.slots;return m;
}
void write_native_session_message111_00761cd0(const NativeSessionMessage111* m,NativeBitCursor* cursor,const Context&){
    write_native_session_message_header_0075b480(m,cursor);
    write_native_bool_bit_004290b0(cursor,static_cast<const volatile NativeSessionMessage111&>(*m).header.flag_1c);
    for(U i=0;i<6;++i)write_native_unsigned_dword_bits_00429070(cursor,static_cast<const volatile NativeSessionMessage111&>(*m).values_20[i],2);
}
void read_native_session_message111_00761d30(NativeSessionMessage111* m,NativeSessionReadStream* stream,const Context&){
    read_native_session_message_header_0075b4c0(m,stream);auto* cursor=&stream->cursor_04;
    read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->header.flag_1c));
    for(U i=0;i<6;++i)read_native_u32_bits_00428d10(cursor,&m->values_20[i],2);
}
void write_native_session_message112_00761e20(const NativeSessionMessage112* m,NativeBitCursor* cursor,const Context& c){
    write_native_session_message_header_0075b480(m,cursor);
    write_native_word_bits_00429120(cursor,static_cast<const volatile NativeSessionMessage112&>(*m).value_1c,12);
    for(U i=0;i<3;++i){const U scale=x87_bits(c.maximum_00d7a248);const U value=x87_bits(&m->float_bits_20[i]);
        write_native_numeric_float_004295c0(cursor,value,0,1,scale,32,*c.numeric);}
}
void read_native_session_message112_00761ec0(NativeSessionMessage112* m,NativeSessionReadStream* stream,const Context& c){
    read_native_session_message_header_0075b4c0(m,stream);auto* cursor=&stream->cursor_04;
    read_native_word_bits_00428e30(cursor,&m->value_1c,12);
    for(U i=0;i<3;++i){const U scale=x87_bits(c.maximum_00d7a248);
        read_native_numeric_float_004293f0(cursor,&m->float_bits_20[i],0,1,scale,32,*c.numeric);}
}
} // namespace bsp
