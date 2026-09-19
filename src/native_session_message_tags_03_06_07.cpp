#include "bsp/native_session_message_tags_03_06_07.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using B=std::uint8_t;
using M3=NativeSessionMessage03;using M6=NativeSessionMessage06;using M7=NativeSessionMessage07;
M3* __fastcall delete03(M3* m,void*,U f){return delete_native_session_message03_0075cff0(m,f);}
M6* __fastcall delete06(M6* m,void*,U f){return delete_native_session_message06_0075da80(m,f);}
M7* __fastcall delete07(M7* m,void*,U f){return delete_native_session_message07_0075ea10(m,f);}
void __fastcall write03(const M3* m,void*,NativeBitCursor* c){
    const auto& p=*reinterpret_cast<const NativeSessionMessage03Profile*>(m->base.profile_00);
    write_native_session_message03_0075cef0(m,c,*p.numeric,*p.maximum_float_00d7a248);
}
void __fastcall read03(M3* m,void*,NativeSessionReadStream* s){
    const auto& p=*reinterpret_cast<const NativeSessionMessage03Profile*>(m->base.profile_00);
    read_native_session_message03_0075cf70(m,s,*p.numeric,*p.maximum_float_00d7a248);
}
void __fastcall write06(const M6* m,void*,NativeBitCursor* c){write_native_session_message06_0075da20(m,c);}
void __fastcall read06(M6* m,void*,NativeSessionReadStream* s){read_native_session_message06_0075da50(m,s);}
void __fastcall write07(const M7* m,void*,NativeBitCursor* c){write_native_session_message07_0075e9d0(m,c);}
void __fastcall read07(M7* m,void*,NativeSessionReadStream* s){read_native_session_message07_0075e9f0(m,s);}
bool __fastcall is03(const M3*,void*,U t){return native_session_message_is03_0075cfe0(t);}
bool __fastcall is06(const M6*,void*,U t){return native_session_message_is06_0075da10(t);}
bool __fastcall is07(const M7*,void*,U t){return native_session_message_is07_0075e9c0(t);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
const U profile06[]={reinterpret_cast<U>(&delete06),reinterpret_cast<U>(&write06),reinterpret_cast<U>(&read06),reinterpret_cast<U>(&is06),reinterpret_cast<U>(&yes)};
const U profile07[]={reinterpret_cast<U>(&delete07),reinterpret_cast<U>(&write07),reinterpret_cast<U>(&read07),reinterpret_cast<U>(&is07),reinterpret_cast<U>(&yes)};
// FLD/FSTP is observable for signaling NaNs; memcpy is not equivalent here.
U x87_float_bits(const volatile void* p){
    U v;
    __asm {
        mov ecx,p
        fld dword ptr [ecx]
        fstp dword ptr v
    }
    return v;
}
}
NativeSessionMessage03Profile::NativeSessionMessage03Profile(const NativeBitNumericContext& c,const volatile float& maximum)
    :slots{reinterpret_cast<U>(&delete03),reinterpret_cast<U>(&write03),reinterpret_cast<U>(&read03),reinterpret_cast<U>(&is03),reinterpret_cast<U>(&yes)},numeric(&c),maximum_float_00d7a248(&maximum){}
const U* native_session_message06_profile_00d0305c(){return profile06;}
const U* native_session_message07_profile_00d03188(){return profile07;}
NativeSessionMessage03* construct_native_session_message03_0075ce80(M3* message,const NativeSessionMessageContext& c,const NativeSessionMessage03Profile& p){
    volatile auto& m=message->base;
    m.delivery_04=3;m.field_08=0;m.field_0c=0;m.profile_00=native_session_message_base_profile_00d02c68();m.type_10=3;
    auto* game=static_cast<B*>(c.current_game_00e188a8);
    const auto index=*reinterpret_cast<const volatile std::int32_t*>(game+0x18ec);
    if(index>=0&&index<=7){
        auto* selected=*reinterpret_cast<void* const volatile*>(game+0x18cc+static_cast<U>(index)*4);
        m.delivery_04=0;m.selected_owner_14=selected;
    }else{m.selected_owner_14=nullptr;m.delivery_04=0;}
    m.profile_00=p.slots;return message;
}
NativeSessionMessage06* construct_native_session_message06_0075d990(M6* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,6,c);
    static_cast<volatile NativeSessionMessageStorage&>(m->base).delivery_04=1;
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=profile06;return m;
}
NativeSessionMessage07* construct_native_session_message07_0075e940(M7* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(m,7,c);
    static_cast<volatile M7&>(*m).delivery_04=1;static_cast<volatile M7&>(*m).profile_00=profile07;return m;
}
void write_native_session_message03_0075cef0(const M3* m,NativeBitCursor* cursor,const NativeBitNumericContext& c,const volatile float& maximum){
    write_native_session_message_type_00449940(&m->base,cursor);
    for(U i=0;i<3;++i){const U scale=x87_float_bits(&maximum);const U value=x87_float_bits(&m->value_bits_18[i]);write_native_numeric_float_004295c0(cursor,value,0,1,scale,32,c);}
}
void read_native_session_message03_0075cf70(M3* m,NativeSessionReadStream* stream,const NativeBitNumericContext& c,const volatile float& maximum){
    read_native_session_message_type_00449960(&m->base,stream);
    for(U i=0;i<3;++i){const U scale=x87_float_bits(&maximum);read_native_numeric_float_004293f0(&stream->cursor_04,&m->value_bits_18[i],0,1,scale,32,c);}
}
void write_native_session_message06_0075da20(const M6* m,NativeBitCursor* c){write_native_session_message_type_00449940(&m->base,c);write_native_signed_dword_bits_00429090(c,static_cast<U>(m->value_18),6);}
void read_native_session_message06_0075da50(M6* m,NativeSessionReadStream* s){read_native_session_message_type_00449960(&m->base,s);read_native_i32_bits_00428d30(&s->cursor_04,&m->value_18,6);}
void write_native_session_message07_0075e9d0(const M7* m,NativeBitCursor* c){write_native_session_message_type_00449940(m,c);}
void read_native_session_message07_0075e9f0(M7* m,NativeSessionReadStream* s){read_native_session_message_type_00449960(m,s);}
bool native_session_message_is03_0075cfe0(U t){return t==3;}
bool native_session_message_is06_0075da10(U t){return t==6;}
bool native_session_message_is07_0075e9c0(U t){return t==7;}
NativeSessionMessage03* delete_native_session_message03_0075cff0(M3* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
NativeSessionMessage06* delete_native_session_message06_0075da80(M6* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
NativeSessionMessage07* delete_native_session_message07_0075ea10(M7* m,U f){return delete_native_session_message_root_00449910(m,f);}
} // namespace bsp
