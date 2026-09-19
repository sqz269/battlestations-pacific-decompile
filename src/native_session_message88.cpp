#include "bsp/native_session_message88.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/cruise_command.hpp"
#include <cstring>

namespace bsp {
namespace {
using U=std::uint32_t;using Context=NativeSessionMessage88Context;
U x87_copy(const volatile float* source){U result;__asm {mov eax,source
 fld dword ptr [eax]
 fstp dword ptr result}return result;}
void sse_copy(volatile float* destination,const volatile float* source){__asm {
 mov eax,source
 mov edx,destination
 movss xmm0,dword ptr [eax]
 movss dword ptr [edx],xmm0}}
// COMISS/JB rejects unordered and values below the borrowed 1.0 threshold.
bool trailing_present(const float* value,const volatile float* threshold){unsigned char result;__asm {
 mov eax,value
 mov ecx,threshold
 movss xmm0,dword ptr [eax]
 comiss xmm0,dword ptr [ecx]
 setae result}return result!=0;}
const NativeSessionMessage88Profile& profile(const NativeSessionMessage88* m){return *reinterpret_cast<const NativeSessionMessage88Profile*>(m->header.base.profile_00);}
NativeSessionMessage88* __fastcall del(NativeSessionMessage88* m,void*,U flags){return delete_native_session_message88_00764d20(m,flags);}
void __fastcall write(const NativeSessionMessage88* m,void*,NativeBitCursor* cursor){write_native_session_message88_00764d40(m,cursor,*profile(m).context);}
void __fastcall read(NativeSessionMessage88* m,void*,NativeSessionReadStream* stream){read_native_session_message88_00764e60(m,stream,*profile(m).context);}
bool __fastcall is(const void*,void*,U category){return entity_command_message_is_category_00764d00(static_cast<std::int32_t>(category));}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
}
NativeSessionMessage88Profile::NativeSessionMessage88Profile(const Context& c):slots{reinterpret_cast<U>(&del),reinterpret_cast<U>(&write),reinterpret_cast<U>(&read),reinterpret_cast<U>(&is),reinterpret_cast<U>(&yes)},context(&c){}
NativeSessionMessage88* construct_native_session_message88_00764c80(NativeSessionMessage88* m,const Context& c,const NativeSessionMessage88Profile& p){
    volatile auto& h=m->header;h.base.field_08=0;h.base.field_0c=0;h.base.selected_owner_14=nullptr;h.base.type_10=0;h.sender_18=0;h.relay_1a=0;h.base.delivery_04=1;h.flag_1c=0;h.base.profile_00=p.slots;
    volatile auto& target=m->target_24;
    for(U i=0;i<3;++i)sse_copy(&target.position[i],c.default_vector_00f87574+i);
    *reinterpret_cast<volatile std::uint16_t*>(&target.kind)=0;target.object=nullptr;target.kind=0;target.object_id=0;
    const U zero=0;std::memcpy(&m->target_24.trailing,&zero,4);return m;
}
void destroy_native_session_message88_00764cf0(NativeSessionMessage88* m){m->header.base.profile_00=native_session_message_root_profile_00ce4974();}
NativeSessionMessage88* delete_native_session_message88_00764d20(NativeSessionMessage88* m,U flags){destroy_native_session_message88_00764cf0(m);if(flags&1)singleton_lifetime_free(m);return m;}
void write_native_session_message88_00764d40(const NativeSessionMessage88* m,NativeBitCursor* cursor,const Context& c){
    const volatile auto& x=*m;
    write_native_byte_bits_00428ff0(cursor,x.header.base.type_10,8);write_native_word_bits_00429120(cursor,x.header.sender_18,12);
    write_native_bool_bit_004290b0(cursor,x.header.relay_1a);write_native_bool_bit_004290b0(cursor,x.header.flag_1c);
    write_native_byte_bits_00428ff0(cursor,x.command_20,8);write_native_bool_bit_004290b0(cursor,x.flags_21);
    write_native_bool_bit_004290b0(cursor,x.target_24.kind);
    if(x.target_24.kind)write_native_word_bits_00429120(cursor,x.target_24.object_id,13);
    write_native_bool_bit_004290b0(cursor,x.target_24.position_valid);
    if(x.target_24.position_valid){const U scale=x87_copy(c.maximum_00d7a248);U values[3];std::memcpy(values,m->target_24.position,sizeof(values));write_native_numeric_float_array_00429790(cursor,values,3,0,1,scale,32,*c.numeric);}
    if(trailing_present(&m->target_24.trailing,c.threshold_00d7a24c)){
        write_native_bool_bit_004290b0(cursor,1);const U scale=x87_copy(c.maximum_00d7a248);const U value=x87_copy(&m->target_24.trailing);write_native_numeric_float_004295c0(cursor,value,0,1,scale,32,*c.numeric);
    }else write_native_bool_bit_004290b0(cursor,0);
}
void read_native_session_message88_00764e60(NativeSessionMessage88* m,NativeSessionReadStream* stream,const Context& c){
    auto* cursor=&stream->cursor_04;
    read_native_u8_bits_00428c70(cursor,&m->header.base.type_10,8);read_native_word_bits_00428e30(cursor,&m->header.sender_18,12);
    read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->header.relay_1a));read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->header.flag_1c));
    read_native_u8_bits_00428c70(cursor,&m->command_20,8);read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->flags_21));
    bool present;read_native_bool_bit_00428d70(cursor,&present);
    if(present){std::uint16_t handle;read_native_word_bits_00428e30(cursor,&handle,13);m->target_24.kind=1;m->target_24.object_id=handle;m->target_24.object=nullptr;}
    read_native_bool_bit_00428d70(cursor,&present);
    if(present){U values[3];const U scale=x87_copy(c.maximum_00d7a248);read_native_numeric_float_array_004294f0(cursor,values,3,0,1,scale,32,*c.numeric);m->target_24.position_valid=1;std::memcpy(m->target_24.position,values,sizeof(values));}
    read_native_bool_bit_00428d70(cursor,&present);
    if(present){U value;const U scale=x87_copy(c.maximum_00d7a248);read_native_numeric_float_004293f0(cursor,&value,0,1,scale,32,*c.numeric);std::memcpy(&m->target_24.trailing,&value,4);}
}
} // namespace bsp
