#include "bsp/native_session_message_tags_21_22.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using M21=NativeSessionMessage21;using M22=NativeSessionMessage22;
M21* __fastcall scalar21(M21* m,void*,U f){return delete_native_session_message21_007643c0(m,f);}
M22* __fastcall scalar22(M22* m,void*,U f){return delete_native_session_message22_00764470(m,f);}
void __fastcall write21(const M21* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage21Profile*>(m->base.profile_00);write_native_session_message21_009064f0(m,c,*p.numeric,*p.maximum_00d7a248);}
void __fastcall read21(M21* m,void*,NativeSessionReadStream* s){const auto& p=*reinterpret_cast<const NativeSessionMessage21Profile*>(m->base.profile_00);read_native_session_message21_00906800(m,s,*p.numeric,*p.maximum_00d7a248);}
void __fastcall write22(const M22* m,void*,NativeBitCursor* c){write_native_session_message22_00906af0(m,c);}
void __fastcall read22(M22* m,void*,NativeSessionReadStream* s){read_native_session_message22_00906c20(m,s);}
bool __fastcall is21(const M21*,void*,U t){return native_session_message_is21_007643a0(t);}
bool __fastcall is22(const M22*,void*,U t){return native_session_message_is22_00764450(t);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
const U profile22[]={reinterpret_cast<U>(&scalar22),reinterpret_cast<U>(&write22),reinterpret_cast<U>(&read22),reinterpret_cast<U>(&is22),reinterpret_cast<U>(&yes)};
U x87_float_bits(const volatile void* p){U v;__asm {mov ecx,p
    fld dword ptr [ecx]
    fstp dword ptr v}return v;}
void w(NativeBitCursor* c,I v,U bits){write_native_signed_dword_bits_00429090(c,static_cast<U>(v),bits);}
void r(NativeBitCursor* c,I* v,U bits){read_native_i32_bits_00428d30(c,v,bits);}
void wf(NativeBitCursor* c,const U* v,const NativeBitNumericContext& n,const volatile float& maximum){
    const U scale=x87_float_bits(&maximum);const U value=x87_float_bits(v);
    write_native_numeric_float_004295c0(c,value,0,1,scale,32,n);
}
void rf(NativeBitCursor* c,U* v,const NativeBitNumericContext& n,const volatile float& maximum){
    const U scale=x87_float_bits(&maximum);read_native_numeric_float_004293f0(c,v,0,1,scale,32,n);
}
}
NativeSessionMessage21Profile::NativeSessionMessage21Profile(const NativeBitNumericContext& n,const volatile float& maximum)
    :slots{reinterpret_cast<U>(&scalar21),reinterpret_cast<U>(&write21),reinterpret_cast<U>(&read21),reinterpret_cast<U>(&is21),reinterpret_cast<U>(&yes)},numeric(&n),maximum_00d7a248(&maximum){}
const U* native_session_message22_profile_00d035cc(){return profile22;}
M21* construct_native_session_message21_00764340(M21* m,const NativeSessionMessageContext& c,const NativeSessionMessage21Profile& p){
    construct_native_session_message_0075b430(&m->base,21,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;v.subject_1c=-1;return m;
}
M22* construct_native_session_message22_007643e0(M22* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,22,c);
    volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile22;return m;
}
void write_native_session_message21_009064f0(const M21* m,NativeBitCursor* c,const NativeBitNumericContext& n,const volatile float& maximum){
    write_native_session_message_type_00449940(&m->base,c);w(c,m->subject_1c,6);w(c,m->subtype_18,8);
    // Dispatch on the reloaded full DWORD, not its encoded low byte.
    switch(static_cast<U>(m->subtype_18)){
    case 0:
        w(c,m->value_2c,3);w(c,m->value_30,4);w(c,m->value_34,4);w(c,m->value_78,5);
        w(c,m->value_68,3);w(c,m->value_6c,3);w(c,m->value_74,4);w(c,m->value_70,4);
        for(U i=0;i<7;++i)w(c,m->values_38[i],32);
        wf(c,&m->value_bits_28,n,maximum);wf(c,&m->value_bits_20,n,maximum);wf(c,&m->value_bits_24,n,maximum);
        for(U i=0;i<4;++i)w(c,m->values_54[i],6);
        w(c,m->value_64,32);return;
    case 1:wf(c,&m->value_bits_28,n,maximum);return;
    case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:case 10:case 19:case 20:
        write_native_bit_string_004290d0(c,m->text_7c);break;
    case 11:case 12:case 13:case 14:w(c,m->value_180,6);w(c,m->value_17c,3);w(c,m->value_184,6);break;
    case 15:w(c,m->value_180,6);wf(c,&m->value_bits_18c,n,maximum);return;
    case 16:w(c,m->value_180,6);w(c,m->value_17c,3);break;
    case 17:w(c,m->value_180,6);break;
    case 18:w(c,m->values_38[6],32);return;
    case 21:case 22:case 23:write_native_bit_string_004290d0(c,m->text_7c);return;
    default:return;
    }
    w(c,m->value_188,32);
}
void read_native_session_message21_00906800(M21* m,NativeSessionReadStream* s,const NativeBitNumericContext& n,const volatile float& maximum){
    auto* c=&s->cursor_04;read_native_session_message_type_00449960(&m->base,s);r(c,&m->subject_1c,6);
    I subtype;r(c,&subtype,8);m->subtype_18=subtype;
    switch(static_cast<U>(subtype)){
    case 0:
        r(c,&m->value_2c,3);r(c,&m->value_30,4);r(c,&m->value_34,4);r(c,&m->value_78,5);
        r(c,&m->value_68,3);r(c,&m->value_6c,3);r(c,&m->value_74,4);r(c,&m->value_70,4);
        for(U i=0;i<7;++i)r(c,&m->values_38[i],32);
        rf(c,&m->value_bits_28,n,maximum);rf(c,&m->value_bits_20,n,maximum);rf(c,&m->value_bits_24,n,maximum);
        for(U i=0;i<4;++i)r(c,&m->values_54[i],6);
        r(c,&m->value_64,32);return;
    case 1:rf(c,&m->value_bits_28,n,maximum);return;
    case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:case 10:case 19:case 20:
        read_native_bit_string_00428df0(c,m->text_7c);break;
    case 11:case 12:case 13:case 14:r(c,&m->value_180,6);r(c,&m->value_17c,3);r(c,&m->value_184,6);break;
    case 15:r(c,&m->value_180,6);rf(c,&m->value_bits_18c,n,maximum);return;
    case 16:r(c,&m->value_180,6);r(c,&m->value_17c,3);break;
    case 17:r(c,&m->value_180,6);break;
    case 18:r(c,&m->values_38[6],32);return;
    case 21:case 22:case 23:read_native_bit_string_00428df0(c,m->text_7c);return;
    default:return;
    }
    r(c,&m->value_188,32);
}
void write_native_session_message22_00906af0(const M22* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);w(c,m->subtype_18,4);
    switch(static_cast<U>(m->subtype_18)){
    case 0:case 1:case 2:case 3:case 4:case 5:write_native_bit_string_004290d0(c,m->bytes_1c);return;
    case 6:{
        for(U i=0;i<6;++i)w(c,m->counts_11c[i],8);
        const volatile auto& v=*m;
        U count=static_cast<U>(v.counts_11c[4]);count+=static_cast<U>(v.counts_11c[0]);
        count+=static_cast<U>(v.counts_11c[5]);count+=static_cast<U>(v.counts_11c[1]);
        count+=static_cast<U>(v.counts_11c[2]);count+=static_cast<U>(v.counts_11c[3]);
        const I total=static_cast<I>(count);
        for(I i=0;i<total;++i){
            const auto* p=reinterpret_cast<const std::int8_t*>(reinterpret_cast<std::uintptr_t>(m)+0x1cu+static_cast<U>(i));
            write_native_unsigned_dword_bits_00429070(c,static_cast<U>(static_cast<I>(*p)),8);
        }
        return;
    }
    case 7:write_native_byte_bits_00428ff0(c,m->value_134,3);return;
    default:return;
    }
}
void read_native_session_message22_00906c20(M22* m,NativeSessionReadStream* s){
    auto* c=&s->cursor_04;read_native_session_message_type_00449960(&m->base,s);
    I subtype;r(c,&subtype,4);m->subtype_18=subtype;
    switch(static_cast<U>(subtype)){
    case 0:case 1:case 2:case 3:case 4:case 5:read_native_bit_string_00428df0(c,m->bytes_1c);return;
    case 6:{
        for(U i=0;i<6;++i)r(c,&m->counts_11c[i],8);
        const volatile auto& v=*m;
        U count=static_cast<U>(v.counts_11c[3]);count+=static_cast<U>(v.counts_11c[2]);
        count+=static_cast<U>(v.counts_11c[5]);count+=static_cast<U>(v.counts_11c[1]);
        count+=static_cast<U>(v.counts_11c[4]);count+=static_cast<U>(v.counts_11c[0]);
        const I total=static_cast<I>(count);
        for(I i=0;i<total;++i){
            U value;read_native_u32_bits_00428d10(c,&value,8);
            auto* p=reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(m)+0x1cu+static_cast<U>(i));
            *p=static_cast<std::uint8_t>(value);
        }
        return;
    }
    case 7:read_native_u8_bits_00428c70(c,&m->value_134,3);return;
    default:return;
    }
}
bool native_session_message_is21_007643a0(U t){return t==21;}
bool native_session_message_is22_00764450(U t){return t==22;}
M21* delete_native_session_message21_007643c0(M21* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
M22* delete_native_session_message22_00764470(M22* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
} // namespace bsp
