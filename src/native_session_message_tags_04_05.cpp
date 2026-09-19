#include "bsp/native_session_message_tags_04_05.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using B=std::uint8_t;using M4=NativeSessionMessage04;using M5=NativeSessionMessage05;
M4* __fastcall delete04(M4* m,void*,U f){return delete_native_session_message04_0075d970(m,f);}
M5* __fastcall delete05(M5* m,void*,U f){return delete_native_session_message05_0075d2c0(m,f);}
void __fastcall write04(const M4* m,void*,NativeBitCursor* c){write_native_session_message04_0075d660(m,c);}
void __fastcall read04(M4* m,void*,NativeSessionReadStream* s){read_native_session_message04_0075d7e0(m,s);}
void __fastcall write05(const M5* m,void*,NativeBitCursor* c){
    const auto& p=*reinterpret_cast<const NativeSessionMessage05Profile*>(m->base.profile_00);
    write_native_session_message05_0075d0e0(m,c,*p.numeric);
}
void __fastcall read05(M5* m,void*,NativeSessionReadStream* s){
    const auto& p=*reinterpret_cast<const NativeSessionMessage05Profile*>(m->base.profile_00);
    read_native_session_message05_0075d1d0(m,s,*p.numeric);
}
bool __fastcall is04(const M4*,void*,U t){return native_session_message_is04_0075d650(t);}
bool __fastcall is05(const M5*,void*,U t){return native_session_message_is05_0075d0d0(t);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
const U profile04[]={reinterpret_cast<U>(&delete04),reinterpret_cast<U>(&write04),reinterpret_cast<U>(&read04),reinterpret_cast<U>(&is04),reinterpret_cast<U>(&yes)};
void read_boolean(NativeBitCursor* c,B& out){bool value;read_native_bool_bit_00428d70(c,&value);out=static_cast<B>(value);}
// Native writers read the low DWORD again after its write, then shift the
// reloaded pair right32 via __aullshr. Keep that load and the later high load.
void write_split_qword(NativeBitCursor* c,const volatile U* words){
    write_native_unsigned_dword_bits_00429070(c,words[0],32);
    const U low_again=words[0];const U high=words[1];(void)low_again;
    write_native_unsigned_dword_bits_00429070(c,high,32);
}
// __allmul(high,0,0,1) is exactly high<<32. Preserve native intermediate
// low/zero-high stores before the second read and the subsequent ADD/ADC stores.
void read_split_qword(NativeBitCursor* c,volatile U* words){
    U value;read_native_u32_bits_00428d10(c,&value,32);
    words[0]=value;words[1]=0;
    read_native_u32_bits_00428d10(c,&value,32);
    words[0]=words[0]+0u;words[1]=words[1]+value;
}
}
NativeSessionMessage05Profile::NativeSessionMessage05Profile(const NativeBitNumericContext& c)
    :slots{reinterpret_cast<U>(&delete05),reinterpret_cast<U>(&write05),reinterpret_cast<U>(&read05),reinterpret_cast<U>(&is05),reinterpret_cast<U>(&yes)},numeric(&c){}
const U* native_session_message04_profile_00d03048(){return profile04;}
NativeSessionMessage04* construct_native_session_message04_0075d5d0(M4* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,4,c);
    static_cast<volatile NativeSessionMessageStorage&>(m->base).delivery_04=1;
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=profile04;return m;
}
NativeSessionMessage05* construct_native_session_message05_0075d010(M5* m,const NativeSessionMessageContext& c,const NativeSessionMessage05Profile& p){
    construct_native_session_message_0075b430(&m->base,5,c);
    static_cast<volatile NativeSessionMessageStorage&>(m->base).delivery_04=1;
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;return m;
}
void write_native_session_message04_0075d660(const M4* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    const U high=m->words_20[1];const U low=m->words_20[0];write_native_u64_bits_00429180(c,low,high);
    write_native_signed_dword_bits_00429090(c,static_cast<U>(m->signed_18),6);
    write_native_unsigned_dword_bits_00429070(c,m->value_54,2);
    write_native_bit_string_004290d0(c,m->text_28);write_native_bit_string_004290d0(c,m->text_48);
    write_native_bool_bit_004290b0(c,m->flag_58);
    write_native_unsigned_dword_bits_00429070(c,m->value_5c,32);
    write_native_bool_bit_004290b0(c,m->flag_9e);
    write_native_byte_bits_00428ff0(c,m->value_60,5);write_native_byte_bits_00428ff0(c,m->value_61,2);
    write_native_unsigned_dword_bits_00429070(c,m->value_50,32);
    for(U i=0;i<36;++i)write_native_byte_bits_00428ff0(c,m->bytes_62[i],8);
    for(U i=0;i<16;++i)write_native_byte_bits_00428ff0(c,m->bytes_86[i],8);
    for(U i=0;i<8;++i)write_native_byte_bits_00428ff0(c,m->bytes_96[i],8);
    for(U i=0;i<3;++i)write_native_signed_dword_bits_00429090(c,static_cast<U>(m->signed_a0[i]),32);
    write_split_qword(c,m->words_b0);
    write_native_unsigned_dword_bits_00429070(c,m->value_b8,32);write_native_unsigned_dword_bits_00429070(c,m->value_bc,32);
}
void read_native_session_message04_0075d7e0(M4* m,NativeSessionReadStream* s){
    auto* c=&s->cursor_04;read_native_session_message_type_00449960(&m->base,s);
    read_native_u64_bits_00428e90(c,m->words_20);read_native_i32_bits_00428d30(c,&m->signed_18,6);
    read_native_u32_bits_00428d10(c,&m->value_54,2);
    read_native_bit_string_00428df0(c,m->text_28);read_native_bit_string_00428df0(c,m->text_48);
    read_boolean(c,m->flag_58);read_native_u32_bits_00428d10(c,&m->value_5c,32);
    read_boolean(c,m->flag_9e);read_native_u8_bits_00428c70(c,&m->value_60,5);read_native_u8_bits_00428c70(c,&m->value_61,2);
    read_native_u32_bits_00428d10(c,&m->value_50,32);
    for(U i=0;i<36;++i)read_native_u8_bits_00428c70(c,&m->bytes_62[i],8);
    for(U i=0;i<16;++i)read_native_u8_bits_00428c70(c,&m->bytes_86[i],8);
    for(U i=0;i<8;++i)read_native_u8_bits_00428c70(c,&m->bytes_96[i],8);
    for(U i=0;i<3;++i)read_native_i32_bits_00428d30(c,&m->signed_a0[i],32);
    read_split_qword(c,m->words_b0);
    read_native_u32_bits_00428d10(c,&m->value_b8,32);read_native_u32_bits_00428d10(c,&m->value_bc,32);
}
void write_native_session_message05_0075d0e0(const M5* m,NativeBitCursor* c,const NativeBitNumericContext& numeric){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_signed_byte_bits_00429010(c,static_cast<B>(m->signed_18),5);write_native_signed_byte_bits_00429010(c,static_cast<B>(m->signed_19),5);
    write_native_bit_string_004290d0(c,m->text_1a);write_native_bool_bit_004290b0(c,m->flag_3b);
    write_native_signed_byte_bits_00429010(c,static_cast<B>(m->signed_3c),5);write_native_signed_byte_bits_00429010(c,static_cast<B>(m->signed_3d),5);
    write_native_signed_byte_bits_00429010(c,static_cast<B>(m->signed_44),6);write_native_signed_byte_bits_00429010(c,static_cast<B>(m->signed_45),6);
    for(U i=0;i<13;++i)write_native_unsigned_dword_bits_00429070(c,m->values_50[i],4);
    U scale,value;const auto* input=&m->float_bits_40;
    __asm {
        fld1
        fstp dword ptr scale
        mov ecx,input
        fld dword ptr [ecx]
        fstp dword ptr value
    }
    write_native_numeric_float_004295c0(c,value,0,0,scale,8,numeric);
    write_split_qword(c,m->words_48);
}
void read_native_session_message05_0075d1d0(M5* m,NativeSessionReadStream* s,const NativeBitNumericContext& numeric){
    auto* c=&s->cursor_04;read_native_session_message_type_00449960(&m->base,s);
    read_native_i8_bits_00428c80(c,&m->signed_18,5);read_native_i8_bits_00428c80(c,&m->signed_19,5);
    read_native_bit_string_00428df0(c,m->text_1a);read_boolean(c,m->flag_3b);
    read_native_i8_bits_00428c80(c,&m->signed_3c,5);read_native_i8_bits_00428c80(c,&m->signed_3d,5);
    read_native_i8_bits_00428c80(c,&m->signed_44,6);read_native_i8_bits_00428c80(c,&m->signed_45,6);
    for(U i=0;i<13;++i)read_native_u32_bits_00428d10(c,&m->values_50[i],4);
    U scale;
    __asm {
        fld1
        fstp dword ptr scale
    }
    read_native_numeric_float_004293f0(c,&m->float_bits_40,0,0,scale,8,numeric);
    read_split_qword(c,m->words_48);
}
bool native_session_message_is04_0075d650(U t){return t==4;}
bool native_session_message_is05_0075d0d0(U t){return t==5;}
NativeSessionMessage04* delete_native_session_message04_0075d970(M4* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
NativeSessionMessage05* delete_native_session_message05_0075d2c0(M5* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
} // namespace bsp
