#include "bsp/native_session_message_tags_16_17_18_19_20.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using B=std::uint8_t;using M=NativeSessionMessageStorage;
using M16=NativeSessionMessage16;
M16* __fastcall scalar16(M16* m,void*,U f){return delete_native_session_message16_0075ce60(m,f);}
void __fastcall write16(const M16* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage16Profile*>(m->base.profile_00);write_native_session_message16_0075cd40(m,c,*p.numeric);}
void __fastcall read16(M16* m,void*,NativeSessionReadStream* s){const auto& p=*reinterpret_cast<const NativeSessionMessage16Profile*>(m->base.profile_00);read_native_session_message16_0075cdb0(m,s,*p.numeric,*p.smoothing);}
bool __fastcall is16(const M16*,void*,U t){return native_session_message_is16_0075cd30(t);}
using M17=NativeSessionMessage17;
M17* __fastcall scalar17(M17* m,void*,U f){return delete_native_session_message17_0075d490(m,f);}
void __fastcall write17(const M17* m,void*,NativeBitCursor* c){write_native_session_message_type_00449940(m,c);}
void __fastcall read17(M17* m,void*,NativeSessionReadStream* s){read_native_session_message_type_00449960(m,s);}
bool __fastcall is17(const M17* m,void*,U t){return native_session_message_is17_0075d470(m,t);}
using M18=NativeSessionMessage18;
M18* __fastcall scalar18(M18* m,void*,U f){return delete_native_session_message18_0075e810(m,f);}
void __fastcall write18(const M18* m,void*,NativeBitCursor* c){write_native_session_message18_0075e7d0(m,c);}
void __fastcall read18(M18* m,void*,NativeSessionReadStream* s){read_native_session_message18_0075e7f0(m,s);}
bool __fastcall is18(const M18*,void*,U t){return native_session_message_is18_0075e7c0(t);}
using M19=NativeSessionMessage19;
M19* __fastcall scalar19(M19* m,void*,U f){return delete_native_session_message19_004b60f0(m,f);}
void __fastcall write19(const M19* m,void*,NativeBitCursor* c){write_native_session_message19_004b6090(m,c);}
void __fastcall read19(M19* m,void*,NativeSessionReadStream* s){read_native_session_message19_004b60c0(m,s);}
bool __fastcall is19(const M19*,void*,U t){return native_session_message_is19_004b6080(t);}
using M20=NativeSessionMessage20;
M20* __fastcall scalar20(M20* m,void*,U f){return delete_native_session_message20_0075e920(m,f);}
void __fastcall write20(const M20* m,void*,NativeBitCursor* c){write_native_session_message20_0075e8c0(m,c);}
void __fastcall read20(M20* m,void*,NativeSessionReadStream* s){read_native_session_message20_0075e8f0(m,s);}
bool __fastcall is20(const M20*,void*,U t){return native_session_message_is20_0075e8b0(t);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
const U profile17[]={reinterpret_cast<U>(&scalar17),reinterpret_cast<U>(&write17),reinterpret_cast<U>(&read17),reinterpret_cast<U>(&is17),reinterpret_cast<U>(&yes)};
const U profile18[]={reinterpret_cast<U>(&scalar18),reinterpret_cast<U>(&write18),reinterpret_cast<U>(&read18),reinterpret_cast<U>(&is18),reinterpret_cast<U>(&yes)};
const U profile19[]={reinterpret_cast<U>(&scalar19),reinterpret_cast<U>(&write19),reinterpret_cast<U>(&read19),reinterpret_cast<U>(&is19),reinterpret_cast<U>(&yes)};
const U profile20[]={reinterpret_cast<U>(&scalar20),reinterpret_cast<U>(&write20),reinterpret_cast<U>(&read20),reinterpret_cast<U>(&is20),reinterpret_cast<U>(&yes)};
U x87_float_bits(const volatile void* p){U v;__asm {mov ecx,p
 fld dword ptr [ecx]
 fstp dword ptr v}return v;}
U movss_float_bits(const volatile void* p){U v;__asm {mov ecx,p
 movss xmm0,dword ptr [ecx]
 movss v,xmm0}return v;}
void read_boolean(NativeBitCursor* c,B& out){bool b=false;read_native_bool_bit_00428d70(c,&b);out=b?1:0;}
}
NativeSessionMessage16Profile::NativeSessionMessage16Profile(const NativeBitNumericContext& n,const NativeTripletSmoothingContext& s)
    :slots{reinterpret_cast<U>(&scalar16),reinterpret_cast<U>(&write16),reinterpret_cast<U>(&read16),reinterpret_cast<U>(&is16),reinterpret_cast<U>(&yes)},numeric(&n),smoothing(&s){}
M16* construct_native_session_message16_0075cbc0(M16* m,const NativeSessionMessageContext& c,const NativeSessionMessage16Profile& p){
    construct_native_session_message_0075b430(&m->base,16,c);
    volatile auto& v=*m;
    const U one=movss_float_bits(&p.numeric->plus_one_00d7a24c);v.base.profile_00=p.slots;
    for(U i=0;i<8;++i){v.records_18[i].first_bits=0;v.records_18[i].second_bits=0;v.records_18[i].remainder_bits=one;}
    v.base.delivery_04=0;
    return m;
}
void write_native_session_message16_0075cd40(const M16* m,NativeBitCursor* c,const NativeBitNumericContext& n){
    write_native_session_message_type_00449940(&m->base,c);
    for(U i=0;i<8;++i){
        const U first=x87_float_bits(&m->records_18[i].first_bits);write_native_numeric_float_004295c0(c,first,0,0,0x3f800000,10,n);
        const U second=x87_float_bits(&m->records_18[i].second_bits);write_native_numeric_float_004295c0(c,second,0,0,0x3f800000,10,n);
    }
}
void read_native_session_message16_0075cdb0(M16* m,NativeSessionReadStream* s,const NativeBitNumericContext& n,const NativeTripletSmoothingContext& smoothing){
    read_native_session_message_type_00449960(&m->base,s);
    for(U i=0;i<8;++i){auto& record=m->records_18[i];
        read_native_numeric_float_004293f0(&s->cursor_04,&record.first_bits,0,0,0x3f800000,10,n);
        read_native_numeric_float_004293f0(&s->cursor_04,&record.second_bits,0,0,0x3f800000,10,n);
        const U second=x87_float_bits(&record.second_bits);const U first=x87_float_bits(&record.first_bits);
        update_native_smoothed_remainder_triplet_007862c0(&record,first,second,smoothing);
    }
}
bool native_session_message_is16_0075cd30(U t){return t==16;}
M16* delete_native_session_message16_0075ce60(M16* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
const U* native_session_message17_profile_00d03020(){return profile17;}
M17* construct_native_session_message17_0075d3f0(M17* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(m,17,c);
    volatile auto& v=*m;
    v.delivery_04=1;v.profile_00=profile17;
    return m;
}
bool native_session_message_is17_0075d470(const M17* m,U t){return t==17||t==m->type_10;}
M17* delete_native_session_message17_0075d490(M17* m,U f){delete_native_session_message_root_00449910(m,f);return m;}
const U* native_session_message18_profile_00d03160(){return profile18;}
M18* construct_native_session_message18_0075e740(M18* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(m,18,c);
    volatile auto& v=*m;
    v.delivery_04=1;v.profile_00=profile18;
    return m;
}
void write_native_session_message18_0075e7d0(const M18* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(m,c);
}
void read_native_session_message18_0075e7f0(M18* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(m,s);
}
bool native_session_message_is18_0075e7c0(U t){return t==18;}
M18* delete_native_session_message18_0075e810(M18* m,U f){delete_native_session_message_root_00449910(m,f);return m;}
const U* native_session_message19_profile_00ce74dc(){return profile19;}
M19* construct_native_session_message19_004b6050(M19* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,19,c);
    volatile auto& v=*m;
    v.base.delivery_04=1;v.base.profile_00=profile19;
    v.flag_18=0;
    return m;
}
void write_native_session_message19_004b6090(const M19* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_bool_bit_004290b0(c,m->flag_18);
}
void read_native_session_message19_004b60c0(M19* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    read_boolean(&s->cursor_04,m->flag_18);
}
bool native_session_message_is19_004b6080(U t){return t==19;}
M19* delete_native_session_message19_004b60f0(M19* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
const U* native_session_message20_profile_00d03174(){return profile20;}
M20* construct_native_session_message20_0075e830(M20* m,const NativeSessionMessageContext& c){
    volatile auto& v=*m;
    v.base.delivery_04=3;v.base.field_08=0;v.base.field_0c=0;v.base.profile_00=native_session_message_base_profile_00d02c68();v.base.type_10=20;
    auto* game=static_cast<B*>(c.current_game_00e188a8);const auto index=*reinterpret_cast<const volatile std::int32_t*>(game+0x18ec);
    // In the valid branch the flag is cleared before publishing the owner.
    if(index>=0&&index<=7){auto* owner=*reinterpret_cast<void* const volatile*>(game+0x18cc+static_cast<U>(index)*4);v.flag_18=0;v.base.selected_owner_14=owner;}
    else{v.base.selected_owner_14=nullptr;v.flag_18=0;}
    v.base.delivery_04=1;v.base.profile_00=profile20;
    return m;
}
void write_native_session_message20_0075e8c0(const M20* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_bool_bit_004290b0(c,m->flag_18);
}
void read_native_session_message20_0075e8f0(M20* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    read_boolean(&s->cursor_04,m->flag_18);
}
bool native_session_message_is20_0075e8b0(U t){return t==20;}
M20* delete_native_session_message20_0075e920(M20* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
} // namespace bsp
