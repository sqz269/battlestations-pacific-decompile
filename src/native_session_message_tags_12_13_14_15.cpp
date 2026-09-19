#include "bsp/native_session_message_tags_12_13_14_15.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using B=std::uint8_t;using M=NativeSessionMessageStorage;
using M12=NativeSessionMessage12;
M12* __fastcall scalar12(M12* m,void*,U f){return delete_native_session_message12_004b6030(m,f);}
void __fastcall write12(const M12* m,void*,NativeBitCursor* p){write_native_session_message12_004b5ff0(m,p);}
void __fastcall read12(M12* m,void*,NativeSessionReadStream* p){read_native_session_message12_004b6010(m,p);}
bool __fastcall is12(const M12*,void*,U t){return native_session_message_is12_004b5fe0(t);}
using M13=NativeSessionMessage13;
M13* __fastcall scalar13(M13* m,void*,U f){return delete_native_session_message13_0075d5b0(m,f);}
void __fastcall write13(const M13* m,void*,NativeBitCursor* p){write_native_session_message13_0075d550(m,p);}
void __fastcall read13(M13* m,void*,NativeSessionReadStream* p){read_native_session_message13_0075d580(m,p);}
bool __fastcall is13(const M13* m,void*,U t){return native_session_message_is13_0075d530(m,t);}
using M14=NativeSessionMessage14;
M14* __fastcall scalar14(M14* m,void*,U f){return delete_native_session_message14_0075d3d0(m,f);}
void __fastcall write14(const M14* m,void*,NativeBitCursor* p){write_native_session_message14_0075d370(m,p);}
void __fastcall read14(M14* m,void*,NativeSessionReadStream* p){read_native_session_message14_0075d3a0(m,p);}
bool __fastcall is14(const M14* m,void*,U t){return native_session_message_is14_0075d350(m,t);}
using M15=NativeSessionMessage15;
M15* __fastcall scalar15(M15* m,void*,U f){return delete_native_session_message15_0075ce40(m,f);}
void __fastcall write15(const M15* m,void*,NativeBitCursor* p){write_native_session_message15_0075cb10(m,p);}
void __fastcall read15(M15* m,void*,NativeSessionReadStream* p){read_native_session_message15_0075cb60(m,p);}
bool __fastcall is15(const M15*,void*,U t){return native_session_message_is15_0075cb00(t);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
const U profile12[]={reinterpret_cast<U>(&scalar12),reinterpret_cast<U>(&write12),reinterpret_cast<U>(&read12),reinterpret_cast<U>(&is12),reinterpret_cast<U>(&yes)};
const U profile13[]={reinterpret_cast<U>(&scalar13),reinterpret_cast<U>(&write13),reinterpret_cast<U>(&read13),reinterpret_cast<U>(&is13),reinterpret_cast<U>(&yes)};
const U profile14[]={reinterpret_cast<U>(&scalar14),reinterpret_cast<U>(&write14),reinterpret_cast<U>(&read14),reinterpret_cast<U>(&is14),reinterpret_cast<U>(&yes)};
const U profile15[]={reinterpret_cast<U>(&scalar15),reinterpret_cast<U>(&write15),reinterpret_cast<U>(&read15),reinterpret_cast<U>(&is15),reinterpret_cast<U>(&yes)};
void read_boolean(NativeBitCursor* c,B& out){bool value=false;read_native_bool_bit_00428d70(c,&value);out=value?1:0;}
}
const U* native_session_message12_profile_00ce74c8(){return profile12;}
M12* construct_native_session_message12_004b5fb0(M12* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(m,12,c);
    volatile auto& v=*m;
    v.delivery_04=1;v.profile_00=profile12;
    return m;
}
void write_native_session_message12_004b5ff0(const M12* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(m,c);
}
void read_native_session_message12_004b6010(M12* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(m,s);
}
bool native_session_message_is12_004b5fe0(U t){return t==12;}
M12* delete_native_session_message12_004b6030(M12* m,U f){delete_native_session_message_root_00449910(m,f);return m;}
const U* native_session_message13_profile_00d03034(){return profile13;}
M13* construct_native_session_message13_0075d4b0(M13* m,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,13,c);
    volatile auto& v=*m;
    v.base.delivery_04=1;v.base.profile_00=profile13;
    return m;
}
void write_native_session_message13_0075d550(const M13* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_bool_bit_004290b0(c,m->flag_18);
}
void read_native_session_message13_0075d580(M13* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    read_boolean(&s->cursor_04,m->flag_18);
}
bool native_session_message_is13_0075d530(const M13* m,U t){return t==13||t==m->base.type_10;}
M13* delete_native_session_message13_0075d5b0(M13* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
const U* native_session_message14_profile_00d0300c(){return profile14;}
M14* construct_native_session_message14_0075d2e0(M14* m,U raw_flag,const NativeSessionMessageContext& c){
    construct_native_session_message_0075b430(&m->base,14,c);
    volatile auto& v=*m;
    v.base.delivery_04=1;v.base.profile_00=profile14;
    v.flag_18=static_cast<B>(raw_flag);
    return m;
}
void write_native_session_message14_0075d370(const M14* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    write_native_bool_bit_004290b0(c,m->flag_18);
}
void read_native_session_message14_0075d3a0(M14* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    read_boolean(&s->cursor_04,m->flag_18);
}
bool native_session_message_is14_0075d350(const M14* m,U t){return t==14||t==m->base.type_10;}
M14* delete_native_session_message14_0075d3d0(M14* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
const U* native_session_message15_profile_00d02fbc(){return profile15;}
M15* construct_native_session_message15_0075caa0(M15* m,const NativeSessionMessageContext& c){
    volatile auto& v=*m;
    v.base.delivery_04=3;v.base.field_08=0;v.base.field_0c=0;
    v.base.profile_00=native_session_message_base_profile_00d02c68();v.base.type_10=15;
    auto* game=static_cast<B*>(c.current_game_00e188a8);
    const auto index=*reinterpret_cast<const volatile std::int32_t*>(game+0x18ec);
    // The valid branch clears mode/count BEFORE publishing the selected owner.
    if(index>=0&&index<=7){
        auto* selected=*reinterpret_cast<void* const volatile*>(game+0x18cc+static_cast<U>(index)*4);
        v.base.delivery_04=0;v.count_18=0;v.base.selected_owner_14=selected;
    }else{v.base.selected_owner_14=nullptr;v.base.delivery_04=0;v.count_18=0;}
    v.base.profile_00=profile15;
    return m;
}
void write_native_session_message15_0075cb10(const M15* m,NativeBitCursor* c){
    write_native_session_message_type_00449940(&m->base,c);
    const volatile auto& count=m->count_18;
    write_native_signed_word_bits_00429030(c,count,16);
    if(count!=0){std::int32_t i=0;do{
        write_native_byte_bits_00428ff0(c,0x21,8);
        const U current=count;++i;if(i>=static_cast<std::int32_t>(current))break;
    }while(true);}
}
void read_native_session_message15_0075cb60(M15* m,NativeSessionReadStream* s){
    read_native_session_message_type_00449960(&m->base,s);
    read_native_u16_bits_00428cb0(&s->cursor_04,&m->count_18,16);
    const volatile auto& count=m->count_18;
    if(count!=0){std::int32_t i=0;do{
        B discarded;read_native_u8_bits_00428c70(&s->cursor_04,&discarded,8);
        const U current=count;++i;if(i>=static_cast<std::int32_t>(current))break;
    }while(true);}
}
bool native_session_message_is15_0075cb00(U t){return t==15;}
M15* delete_native_session_message15_0075ce40(M15* m,U f){delete_native_session_message_root_00449910(&m->base,f);return m;}
} // namespace bsp
