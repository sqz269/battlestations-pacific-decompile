#include "bsp/native_session_message_tags_36_to_40.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
U x87_float_bits(const volatile void* p){U value;__asm {mov ecx,p
 fld dword ptr [ecx]
 fstp dword ptr value}return value;}
NativeSessionMessage36* __fastcall scalar36(NativeSessionMessage36* m,void*,U f){return delete_native_session_message36_00764890(m,f);}
void __fastcall write36(const NativeSessionMessage36* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage36Profile*>(m->base.profile_00);write_native_session_message36_007647a0(m,c,*p.numeric,*p.maximum_00d7a248);}
void __fastcall read36(NativeSessionMessage36* m,void*,NativeSessionReadStream* c){const auto& p=*reinterpret_cast<const NativeSessionMessage36Profile*>(m->base.profile_00);read_native_session_message36_00764820(m,c,*p.numeric,*p.maximum_00d7a248);}
bool __fastcall is36(const NativeSessionMessage36*,void*,U q){return native_session_message_is36_007646e0(q);}
NativeSessionMessage37* __fastcall scalar37(NativeSessionMessage37* m,void*,U f){return delete_native_session_message37_0075deb0(m,f);}
void __fastcall write37(const NativeSessionMessage37* m,void*,NativeBitCursor* c){write_native_session_message37_0075de10(m,c);}
void __fastcall read37(NativeSessionMessage37* m,void*,NativeSessionReadStream* c){read_native_session_message37_0075de60(m,c);}
bool __fastcall is37(const NativeSessionMessage37*,void*,U q){return native_session_message_is37_0075de00(q);}
const U profile37[]={reinterpret_cast<U>(&scalar37),reinterpret_cast<U>(&write37),reinterpret_cast<U>(&read37),reinterpret_cast<U>(&is37),reinterpret_cast<U>(&yes)};
NativeSessionMessage38* __fastcall scalar38(NativeSessionMessage38* m,void*,U f){return delete_native_session_message38_0075f760(m,f);}
void __fastcall write38(const NativeSessionMessage38* m,void*,NativeBitCursor* c){write_native_session_message38_0075f700(m,c);}
void __fastcall read38(NativeSessionMessage38* m,void*,NativeSessionReadStream* c){read_native_session_message38_0075f730(m,c);}
bool __fastcall is38(const NativeSessionMessage38*,void*,U q){return native_session_message_is38_0075f6f0(q);}
const U profile38[]={reinterpret_cast<U>(&scalar38),reinterpret_cast<U>(&write38),reinterpret_cast<U>(&read38),reinterpret_cast<U>(&is38),reinterpret_cast<U>(&yes)};
NativeSessionMessage39* __fastcall scalar39(NativeSessionMessage39* m,void*,U f){return delete_native_session_message39_0075f870(m,f);}
void __fastcall write39(const NativeSessionMessage39* m,void*,NativeBitCursor* c){write_native_session_message39_0075f810(m,c);}
void __fastcall read39(NativeSessionMessage39* m,void*,NativeSessionReadStream* c){read_native_session_message39_0075f840(m,c);}
bool __fastcall is39(const NativeSessionMessage39*,void*,U q){return native_session_message_is39_0075f800(q);}
const U profile39[]={reinterpret_cast<U>(&scalar39),reinterpret_cast<U>(&write39),reinterpret_cast<U>(&read39),reinterpret_cast<U>(&is39),reinterpret_cast<U>(&yes)};
NativeSessionMessage40* __fastcall scalar40(NativeSessionMessage40* m,void*,U f){return delete_native_session_message40_0075f9b0(m,f);}
void __fastcall write40(const NativeSessionMessage40* m,void*,NativeBitCursor* c){write_native_session_message40_0075f920(m,c);}
void __fastcall read40(NativeSessionMessage40* m,void*,NativeSessionReadStream* c){read_native_session_message40_0075f970(m,c);}
bool __fastcall is40(const NativeSessionMessage40*,void*,U q){return native_session_message_is40_0075f910(q);}
const U profile40[]={reinterpret_cast<U>(&scalar40),reinterpret_cast<U>(&write40),reinterpret_cast<U>(&read40),reinterpret_cast<U>(&is40),reinterpret_cast<U>(&yes)};
}
NativeSessionMessage36Profile::NativeSessionMessage36Profile(const NativeBitNumericContext& n,const volatile float& maximum)
 :slots{reinterpret_cast<U>(&scalar36),reinterpret_cast<U>(&write36),reinterpret_cast<U>(&read36),reinterpret_cast<U>(&is36),reinterpret_cast<U>(&yes)},numeric(&n),maximum_00d7a248(&maximum){}
NativeSessionMessage36* construct_native_session_message36_00764670(NativeSessionMessage36* m,const NativeSessionMessageContext& c,const NativeSessionMessage36Profile& p) {
    construct_native_session_message_0075b430(&m->base,36,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;return m;
}
bool native_session_message_is36_007646e0(U query){return query==36;}
NativeSessionMessage36* delete_native_session_message36_00764890(NativeSessionMessage36* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
void write_native_session_message36_007647a0(const NativeSessionMessage36* m,NativeBitCursor* c,const NativeBitNumericContext& n,const volatile float& maximum){
    write_native_session_message_type_00449940(&m->base,c);const volatile auto& v=*m;
    write_native_unsigned_dword_bits_00429070(c,v.value_18,3);write_native_unsigned_dword_bits_00429070(c,v.value_1c,2);
    {const U scale=x87_float_bits(&maximum);const U value=x87_float_bits(&v.float_bits_20);write_native_numeric_float_004295c0(c,value,0,1,scale,32,n);}
    {const U scale=x87_float_bits(&maximum);const U value=x87_float_bits(&v.float_bits_24);write_native_numeric_float_004295c0(c,value,0,1,scale,32,n);}
}
void read_native_session_message36_00764820(NativeSessionMessage36* m,NativeSessionReadStream* s,const NativeBitNumericContext& n,const volatile float& maximum){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;
    read_native_u32_bits_00428d10(c,&m->value_18,3);read_native_u32_bits_00428d10(c,&m->value_1c,2);
    {const U scale=x87_float_bits(&maximum);read_native_numeric_float_004293f0(c,&m->float_bits_20,0,1,scale,32,n);}
    {const U scale=x87_float_bits(&maximum);read_native_numeric_float_004293f0(c,&m->float_bits_24,0,1,scale,32,n);}
}
const U* native_session_message37_profile_00d030c0(){return profile37;}
NativeSessionMessage37* construct_native_session_message37_0075dd80(NativeSessionMessage37* m,const NativeSessionMessageContext& c) {
    construct_native_session_message_0075b430(&m->base,37,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile37;return m;
}
bool native_session_message_is37_0075de00(U query){return query==37;}
NativeSessionMessage37* delete_native_session_message37_0075deb0(NativeSessionMessage37* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
void write_native_session_message37_0075de10(const NativeSessionMessage37* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);const volatile auto& v=*m;
    write_native_signed_dword_bits_00429090(c,static_cast<U>(v.value_18),6);write_native_unsigned_dword_bits_00429070(c,v.value_1c,32);write_native_unsigned_dword_bits_00429070(c,v.value_20,32);
}
void read_native_session_message37_0075de60(NativeSessionMessage37* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;
    read_native_i32_bits_00428d30(c,&m->value_18,6);read_native_u32_bits_00428d10(c,&m->value_1c,32);read_native_u32_bits_00428d10(c,&m->value_20,32);
}
const U* native_session_message38_profile_00d0323c(){return profile38;}
NativeSessionMessage38* construct_native_session_message38_0075f670(NativeSessionMessage38* m,const NativeSessionMessageContext& c) {
    construct_native_session_message_0075b430(&m->base,38,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile38;return m;
}
bool native_session_message_is38_0075f6f0(U query){return query==38;}
NativeSessionMessage38* delete_native_session_message38_0075f760(NativeSessionMessage38* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
void write_native_session_message38_0075f700(const NativeSessionMessage38* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);const volatile auto& v=*m;
    {const U high=v.words_18[1];const U low=v.words_18[0];write_native_u64_bits_00429180(c,low,high);}
}
void read_native_session_message38_0075f730(NativeSessionMessage38* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;
    read_native_u64_bits_00428e90(c,m->words_18);
}
const U* native_session_message39_profile_00d03250(){return profile39;}
NativeSessionMessage39* construct_native_session_message39_0075f780(NativeSessionMessage39* m,const NativeSessionMessageContext& c) {
    construct_native_session_message_0075b430(&m->base,39,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile39;return m;
}
bool native_session_message_is39_0075f800(U query){return query==39;}
NativeSessionMessage39* delete_native_session_message39_0075f870(NativeSessionMessage39* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
void write_native_session_message39_0075f810(const NativeSessionMessage39* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);const volatile auto& v=*m;
    {const U high=v.words_18[1];const U low=v.words_18[0];write_native_u64_bits_00429180(c,low,high);}
}
void read_native_session_message39_0075f840(NativeSessionMessage39* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;
    read_native_u64_bits_00428e90(c,m->words_18);
}
const U* native_session_message40_profile_00d03264(){return profile40;}
NativeSessionMessage40* construct_native_session_message40_0075f890(NativeSessionMessage40* m,const NativeSessionMessageContext& c) {
    construct_native_session_message_0075b430(&m->base,40,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=profile40;return m;
}
bool native_session_message_is40_0075f910(U query){return query==40;}
NativeSessionMessage40* delete_native_session_message40_0075f9b0(NativeSessionMessage40* m,U flags){delete_native_session_message_root_00449910(&m->base,flags);return m;}
void write_native_session_message40_0075f920(const NativeSessionMessage40* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);const volatile auto& v=*m;
    {const U high=v.words_18[1];const U low=v.words_18[0];write_native_u64_bits_00429180(c,low,high);}
    {const U high=v.words_20[1];const U low=v.words_20[0];write_native_u64_bits_00429180(c,low,high);}
    write_native_bool_bit_004290b0(c,v.flag_28!=0);
}
void read_native_session_message40_0075f970(NativeSessionMessage40* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;
    read_native_u64_bits_00428e90(c,m->words_18);
    read_native_u64_bits_00428e90(c,m->words_20);
    read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&m->flag_28));
}
} // namespace bsp
