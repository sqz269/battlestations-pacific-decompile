#include "bsp/native_session_message_tags_56_to_62.hpp"
#include "bsp/native_bit_cursor_owned_string.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/singleton_lifetime.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;
U x87_bits(const volatile void* p){U bits;__asm {mov ecx,p
 fld dword ptr [ecx]
 fstp dword ptr bits}return bits;}
bool handle_present(U id,const ObjectHandleTables& t){
    U index;const void* entries;
    if(static_cast<I>(id)<t.first_end_00f89a10){index=id-static_cast<U>(t.first_begin_00f89a0c);entries=t.first_entries_00f89a54;}
    else{index=id-static_cast<U>(t.second_begin_00f89a60);entries=t.second_entries_00f89aa8;}
    const U address=reinterpret_cast<U>(entries)+(index<<4)+0xcu;
    // These inlined native gates do not treat id zero as a null handle.
    return *reinterpret_cast<void* const volatile*>(address)!=nullptr;
}
NativeSessionMessage56* __fastcall scalar56(NativeSessionMessage56* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage56Profile*>(m->base.profile_00);return delete_native_session_message56_008dc820(m,flags,*p.context->strings,p);}
void __fastcall write56(const NativeSessionMessage56* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage56Profile*>(m->base.profile_00);write_native_session_message56_008dc700(m,c,*p.context);}
void __fastcall read56(NativeSessionMessage56* m,void*,NativeSessionReadStream* c){const auto& p=*reinterpret_cast<const NativeSessionMessage56Profile*>(m->base.profile_00);read_native_session_message56_008dc750(m,c,*p.context);}
bool __fastcall is56(const NativeSessionMessage56* m,void*,U query){return native_session_message_is56_008dc6d0(m,query);}
bool __fastcall gate56(const NativeSessionMessage56* m,void*){const auto& p=*reinterpret_cast<const NativeSessionMessage56Profile*>(m->base.profile_00);return native_session_message_process_flag_008dae20(*p.context->session);}
NativeSessionMessage57* __fastcall scalar57(NativeSessionMessage57* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage57Profile*>(m->base.profile_00);return delete_native_session_message57_008dc9e0(m,flags,*p.context->strings,p);}
void __fastcall write57(const NativeSessionMessage57* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage57Profile*>(m->base.profile_00);write_native_session_message57_008dc8a0(m,c,*p.context);}
void __fastcall read57(NativeSessionMessage57* m,void*,NativeSessionReadStream* c){const auto& p=*reinterpret_cast<const NativeSessionMessage57Profile*>(m->base.profile_00);read_native_session_message57_008dc8e0(m,c,*p.context);}
bool __fastcall is57(const NativeSessionMessage57* m,void*,U query){return native_session_message_is57_008dc870(m,query);}
bool __fastcall gate57(const NativeSessionMessage57* m,void*){const auto& p=*reinterpret_cast<const NativeSessionMessage57Profile*>(m->base.profile_00);return native_session_message57_available_008dc920(m,*p.context);}
NativeSessionMessage58* __fastcall scalar58(NativeSessionMessage58* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage58Profile*>(m->base.profile_00);return delete_native_session_message58_008dcba0(m,flags,*p.context->strings,p);}
void __fastcall write58(const NativeSessionMessage58* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage58Profile*>(m->base.profile_00);write_native_session_message58_008dca60(m,c,*p.context);}
void __fastcall read58(NativeSessionMessage58* m,void*,NativeSessionReadStream* c){const auto& p=*reinterpret_cast<const NativeSessionMessage58Profile*>(m->base.profile_00);read_native_session_message58_008dcaa0(m,c,*p.context);}
bool __fastcall is58(const NativeSessionMessage58* m,void*,U query){return native_session_message_is58_008dca30(m,query);}
bool __fastcall gate58(const NativeSessionMessage58* m,void*){const auto& p=*reinterpret_cast<const NativeSessionMessage58Profile*>(m->base.profile_00);return native_session_message58_available_008dcae0(m,*p.context);}
NativeSessionMessage59* __fastcall scalar59(NativeSessionMessage59* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage59Profile*>(m->base.profile_00);return delete_native_session_message59_008dcd20(m,flags,*p.context->strings,p);}
void __fastcall write59(const NativeSessionMessage59* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage59Profile*>(m->base.profile_00);write_native_session_message59_008dcc20(m,c,*p.context);}
void __fastcall read59(NativeSessionMessage59* m,void*,NativeSessionReadStream* c){const auto& p=*reinterpret_cast<const NativeSessionMessage59Profile*>(m->base.profile_00);read_native_session_message59_008dcc70(m,c,*p.context);}
bool __fastcall is59(const NativeSessionMessage59* m,void*,U query){return native_session_message_is59_008dcbf0(m,query);}
bool __fastcall gate59(const NativeSessionMessage59* m,void*){const auto& p=*reinterpret_cast<const NativeSessionMessage59Profile*>(m->base.profile_00);return native_session_message_process_flag_008dae20(*p.context->session);}
NativeSessionMessage60* __fastcall scalar60(NativeSessionMessage60* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage60Profile*>(m->base.profile_00);return delete_native_session_message60_008dcea0(m,flags,*p.context->strings,p);}
void __fastcall write60(const NativeSessionMessage60* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage60Profile*>(m->base.profile_00);write_native_session_message60_008dcda0(m,c,*p.context);}
void __fastcall read60(NativeSessionMessage60* m,void*,NativeSessionReadStream* c){const auto& p=*reinterpret_cast<const NativeSessionMessage60Profile*>(m->base.profile_00);read_native_session_message60_008dcdf0(m,c,*p.context);}
bool __fastcall is60(const NativeSessionMessage60* m,void*,U query){return native_session_message_is60_008dcd70(m,query);}
bool __fastcall gate60(const NativeSessionMessage60* m,void*){const auto& p=*reinterpret_cast<const NativeSessionMessage60Profile*>(m->base.profile_00);return native_session_message_process_flag_008dae20(*p.context->session);}
NativeSessionMessage61* __fastcall scalar61(NativeSessionMessage61* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage61Profile*>(m->base.profile_00);return delete_native_session_message61_008dd020(m,flags,*p.context->strings,p);}
void __fastcall write61(const NativeSessionMessage61* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage61Profile*>(m->base.profile_00);write_native_session_message61_008dcf20(m,c,*p.context);}
void __fastcall read61(NativeSessionMessage61* m,void*,NativeSessionReadStream* c){const auto& p=*reinterpret_cast<const NativeSessionMessage61Profile*>(m->base.profile_00);read_native_session_message61_008dcf60(m,c,*p.context);}
bool __fastcall is61(const NativeSessionMessage61* m,void*,U query){return native_session_message_is61_008dcef0(m,query);}
bool __fastcall gate61(const NativeSessionMessage61* m,void*){const auto& p=*reinterpret_cast<const NativeSessionMessage61Profile*>(m->base.profile_00);return native_session_message_process_flag_008dae20(*p.context->session);}
NativeSessionMessage62* __fastcall scalar62(NativeSessionMessage62* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage62Profile*>(m->base.profile_00);return delete_native_session_message62_008dd1a0(m,flags,*p.context->strings,p);}
void __fastcall write62(const NativeSessionMessage62* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage62Profile*>(m->base.profile_00);write_native_session_message62_008dd0a0(m,c,*p.context);}
void __fastcall read62(NativeSessionMessage62* m,void*,NativeSessionReadStream* c){const auto& p=*reinterpret_cast<const NativeSessionMessage62Profile*>(m->base.profile_00);read_native_session_message62_008dd0e0(m,c,*p.context);}
bool __fastcall is62(const NativeSessionMessage62* m,void*,U query){return native_session_message_is62_008dd070(m,query);}
bool __fastcall gate62(const NativeSessionMessage62* m,void*){const auto& p=*reinterpret_cast<const NativeSessionMessage62Profile*>(m->base.profile_00);return native_session_message_process_flag_008dae20(*p.context->session);}
}
bool native_session_message_process_flag_008dae20(const NativeSessionMessageContext& c){
    const auto* game=static_cast<const std::uint8_t*>(c.current_game_00e188a8);
    return *reinterpret_cast<const volatile U*>(game+0x21a4)!=0;
}
NativeSessionMessage56Profile::NativeSessionMessage56Profile(const NativeSessionMessage56To62Context& c):slots{reinterpret_cast<U>(&scalar56),reinterpret_cast<U>(&write56),reinterpret_cast<U>(&read56),reinterpret_cast<U>(&is56),reinterpret_cast<U>(&gate56)},context(&c){}
NativeSessionMessage56* construct_native_session_message56_008dc6a0(NativeSessionMessage56* m,const NativeSessionMessageContext& c,const NativeSessionMessage56Profile& p){
    construct_native_session_message_0075b430(&m->base,56,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;v.length_18=0;v.data_1c=nullptr;v.length_20=0;v.data_24=nullptr;return m;
}
bool native_session_message_is56_008dc6d0(const NativeSessionMessage56* m,U query){if(query==56||query==54)return true;return query==static_cast<const volatile NativeSessionMessageStorage&>(m->base).type_10;}
void write_native_session_message56_008dc700(const NativeSessionMessage56* m,NativeBitCursor* c,const NativeSessionMessage56To62Context& x){
    write_native_session_message_type_00449940(&m->base,c);write_native_owned_string_00429ac0(c,&m->length_18,x.fallback_00e17669);
    write_native_owned_string_00429ac0(c,&m->length_20,x.fallback_00e17669);
    write_native_unsigned_dword_bits_00429070(c,static_cast<const volatile NativeSessionMessage56&>(*m).value_28,2);write_native_bool_bit_004290b0(c,static_cast<const volatile NativeSessionMessage56&>(*m).flag_2c!=0);
}
void read_native_session_message56_008dc750(NativeSessionMessage56* m,NativeSessionReadStream* s,const NativeSessionMessage56To62Context& x){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;read_native_owned_string_00429f20(c,&m->length_18,*x.strings);
    read_native_owned_string_00429f20(c,&m->length_20,*x.strings);
    read_native_u32_bits_00428d10(c,&m->value_28,2);read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&m->flag_2c));
}
void destroy_native_session_message56_008dc7a0(NativeSessionMessage56* m,NativeStringRawPoolContext& strings,const NativeSessionMessage56Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    __try {destroy_native_string_header_0041dd20(&m->length_20,strings);}
    __finally {
    __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
    __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
    }
}
NativeSessionMessage56* delete_native_session_message56_008dc820(NativeSessionMessage56* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage56Profile& p){destroy_native_session_message56_008dc7a0(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;}
NativeSessionMessage57Profile::NativeSessionMessage57Profile(const NativeSessionMessage56To62Context& c):slots{reinterpret_cast<U>(&scalar57),reinterpret_cast<U>(&write57),reinterpret_cast<U>(&read57),reinterpret_cast<U>(&is57),reinterpret_cast<U>(&gate57)},context(&c){}
NativeSessionMessage57* construct_native_session_message57_008dc840(NativeSessionMessage57* m,const NativeSessionMessageContext& c,const NativeSessionMessage57Profile& p){
    construct_native_session_message_0075b430(&m->base,57,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;v.length_18=0;v.data_1c=nullptr;return m;
}
bool native_session_message_is57_008dc870(const NativeSessionMessage57* m,U query){if(query==57||query==54)return true;return query==static_cast<const volatile NativeSessionMessageStorage&>(m->base).type_10;}
bool native_session_message57_available_008dc920(const NativeSessionMessage57* m,const NativeSessionMessage56To62Context& c){if(!native_session_message_process_flag_008dae20(*c.session))return false;return handle_present(static_cast<const volatile NativeSessionMessage57&>(*m).handle_20,*c.handles);}
void write_native_session_message57_008dc8a0(const NativeSessionMessage57* m,NativeBitCursor* c,const NativeSessionMessage56To62Context& x){
    write_native_session_message_type_00449940(&m->base,c);write_native_owned_string_00429ac0(c,&m->length_18,x.fallback_00e17669);
    write_native_word_bits_00429120(c,static_cast<const volatile NativeSessionMessage57&>(*m).handle_20,12);
}
void read_native_session_message57_008dc8e0(NativeSessionMessage57* m,NativeSessionReadStream* s,const NativeSessionMessage56To62Context& x){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;read_native_owned_string_00429f20(c,&m->length_18,*x.strings);
    read_native_word_bits_00428e30(c,&m->handle_20,12);
}
void destroy_native_session_message57_008dc980(NativeSessionMessage57* m,NativeStringRawPoolContext& strings,const NativeSessionMessage57Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
    __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
}
NativeSessionMessage57* delete_native_session_message57_008dc9e0(NativeSessionMessage57* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage57Profile& p){destroy_native_session_message57_008dc980(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;}
NativeSessionMessage58Profile::NativeSessionMessage58Profile(const NativeSessionMessage56To62Context& c):slots{reinterpret_cast<U>(&scalar58),reinterpret_cast<U>(&write58),reinterpret_cast<U>(&read58),reinterpret_cast<U>(&is58),reinterpret_cast<U>(&gate58)},context(&c){}
NativeSessionMessage58* construct_native_session_message58_008dca00(NativeSessionMessage58* m,const NativeSessionMessageContext& c,const NativeSessionMessage58Profile& p){
    construct_native_session_message_0075b430(&m->base,58,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;v.length_18=0;v.data_1c=nullptr;return m;
}
bool native_session_message_is58_008dca30(const NativeSessionMessage58* m,U query){if(query==58||query==54)return true;return query==static_cast<const volatile NativeSessionMessageStorage&>(m->base).type_10;}
bool native_session_message58_available_008dcae0(const NativeSessionMessage58* m,const NativeSessionMessage56To62Context& c){if(!native_session_message_process_flag_008dae20(*c.session))return false;return handle_present(static_cast<const volatile NativeSessionMessage58&>(*m).handle_20,*c.handles);}
void write_native_session_message58_008dca60(const NativeSessionMessage58* m,NativeBitCursor* c,const NativeSessionMessage56To62Context& x){
    write_native_session_message_type_00449940(&m->base,c);write_native_owned_string_00429ac0(c,&m->length_18,x.fallback_00e17669);
    write_native_word_bits_00429120(c,static_cast<const volatile NativeSessionMessage58&>(*m).handle_20,12);
}
void read_native_session_message58_008dcaa0(NativeSessionMessage58* m,NativeSessionReadStream* s,const NativeSessionMessage56To62Context& x){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;read_native_owned_string_00429f20(c,&m->length_18,*x.strings);
    read_native_word_bits_00428e30(c,&m->handle_20,12);
}
void destroy_native_session_message58_008dcb40(NativeSessionMessage58* m,NativeStringRawPoolContext& strings,const NativeSessionMessage58Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
    __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
}
NativeSessionMessage58* delete_native_session_message58_008dcba0(NativeSessionMessage58* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage58Profile& p){destroy_native_session_message58_008dcb40(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;}
NativeSessionMessage59Profile::NativeSessionMessage59Profile(const NativeSessionMessage56To62Context& c):slots{reinterpret_cast<U>(&scalar59),reinterpret_cast<U>(&write59),reinterpret_cast<U>(&read59),reinterpret_cast<U>(&is59),reinterpret_cast<U>(&gate59)},context(&c){}
NativeSessionMessage59* construct_native_session_message59_008dcbc0(NativeSessionMessage59* m,const NativeSessionMessageContext& c,const NativeSessionMessage59Profile& p){
    construct_native_session_message_0075b430(&m->base,59,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;v.length_18=0;v.data_1c=nullptr;return m;
}
bool native_session_message_is59_008dcbf0(const NativeSessionMessage59* m,U query){if(query==59||query==54)return true;return query==static_cast<const volatile NativeSessionMessageStorage&>(m->base).type_10;}
void write_native_session_message59_008dcc20(const NativeSessionMessage59* m,NativeBitCursor* c,const NativeSessionMessage56To62Context& x){
    write_native_session_message_type_00449940(&m->base,c);write_native_owned_string_00429ac0(c,&m->length_18,x.fallback_00e17669);
    const U scale=x87_bits(x.maximum_00d7a248);write_native_numeric_float_array_00429790(c,m->float_bits_20,3,0,1,scale,32,*x.numeric);
}
void read_native_session_message59_008dcc70(NativeSessionMessage59* m,NativeSessionReadStream* s,const NativeSessionMessage56To62Context& x){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;read_native_owned_string_00429f20(c,&m->length_18,*x.strings);
    const U scale=x87_bits(x.maximum_00d7a248);read_native_numeric_float_array_004294f0(c,m->float_bits_20,3,0,1,scale,32,*x.numeric);
}
void destroy_native_session_message59_008dccc0(NativeSessionMessage59* m,NativeStringRawPoolContext& strings,const NativeSessionMessage59Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
    __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
}
NativeSessionMessage59* delete_native_session_message59_008dcd20(NativeSessionMessage59* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage59Profile& p){destroy_native_session_message59_008dccc0(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;}
NativeSessionMessage60Profile::NativeSessionMessage60Profile(const NativeSessionMessage56To62Context& c):slots{reinterpret_cast<U>(&scalar60),reinterpret_cast<U>(&write60),reinterpret_cast<U>(&read60),reinterpret_cast<U>(&is60),reinterpret_cast<U>(&gate60)},context(&c){}
NativeSessionMessage60* construct_native_session_message60_008dcd40(NativeSessionMessage60* m,const NativeSessionMessageContext& c,const NativeSessionMessage60Profile& p){
    construct_native_session_message_0075b430(&m->base,60,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;v.length_18=0;v.data_1c=nullptr;return m;
}
bool native_session_message_is60_008dcd70(const NativeSessionMessage60* m,U query){if(query==60||query==54)return true;return query==static_cast<const volatile NativeSessionMessageStorage&>(m->base).type_10;}
void write_native_session_message60_008dcda0(const NativeSessionMessage60* m,NativeBitCursor* c,const NativeSessionMessage56To62Context& x){
    write_native_session_message_type_00449940(&m->base,c);write_native_owned_string_00429ac0(c,&m->length_18,x.fallback_00e17669);
    const U scale=x87_bits(x.maximum_00d7a248);write_native_numeric_float_array_00429790(c,m->float_bits_20,3,0,1,scale,32,*x.numeric);
}
void read_native_session_message60_008dcdf0(NativeSessionMessage60* m,NativeSessionReadStream* s,const NativeSessionMessage56To62Context& x){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;read_native_owned_string_00429f20(c,&m->length_18,*x.strings);
    const U scale=x87_bits(x.maximum_00d7a248);read_native_numeric_float_array_004294f0(c,m->float_bits_20,3,0,1,scale,32,*x.numeric);
}
void destroy_native_session_message60_008dce40(NativeSessionMessage60* m,NativeStringRawPoolContext& strings,const NativeSessionMessage60Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
    __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
}
NativeSessionMessage60* delete_native_session_message60_008dcea0(NativeSessionMessage60* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage60Profile& p){destroy_native_session_message60_008dce40(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;}
NativeSessionMessage61Profile::NativeSessionMessage61Profile(const NativeSessionMessage56To62Context& c):slots{reinterpret_cast<U>(&scalar61),reinterpret_cast<U>(&write61),reinterpret_cast<U>(&read61),reinterpret_cast<U>(&is61),reinterpret_cast<U>(&gate61)},context(&c){}
NativeSessionMessage61* construct_native_session_message61_008dcec0(NativeSessionMessage61* m,const NativeSessionMessageContext& c,const NativeSessionMessage61Profile& p){
    construct_native_session_message_0075b430(&m->base,61,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;v.length_18=0;v.data_1c=nullptr;v.length_20=0;v.data_24=nullptr;return m;
}
bool native_session_message_is61_008dcef0(const NativeSessionMessage61* m,U query){if(query==61||query==54)return true;return query==static_cast<const volatile NativeSessionMessageStorage&>(m->base).type_10;}
void write_native_session_message61_008dcf20(const NativeSessionMessage61* m,NativeBitCursor* c,const NativeSessionMessage56To62Context& x){
    write_native_session_message_type_00449940(&m->base,c);write_native_owned_string_00429ac0(c,&m->length_18,x.fallback_00e17669);
    write_native_owned_string_00429ac0(c,&m->length_20,x.fallback_00e17669);
    write_native_bool_bit_004290b0(c,static_cast<const volatile NativeSessionMessage61&>(*m).flag_28!=0);
}
void read_native_session_message61_008dcf60(NativeSessionMessage61* m,NativeSessionReadStream* s,const NativeSessionMessage56To62Context& x){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;read_native_owned_string_00429f20(c,&m->length_18,*x.strings);
    read_native_owned_string_00429f20(c,&m->length_20,*x.strings);
    read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&m->flag_28));
}
void destroy_native_session_message61_008dcfa0(NativeSessionMessage61* m,NativeStringRawPoolContext& strings,const NativeSessionMessage61Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    __try {destroy_native_string_header_0041dd20(&m->length_20,strings);}
    __finally {
    __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
    __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
    }
}
NativeSessionMessage61* delete_native_session_message61_008dd020(NativeSessionMessage61* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage61Profile& p){destroy_native_session_message61_008dcfa0(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;}
NativeSessionMessage62Profile::NativeSessionMessage62Profile(const NativeSessionMessage56To62Context& c):slots{reinterpret_cast<U>(&scalar62),reinterpret_cast<U>(&write62),reinterpret_cast<U>(&read62),reinterpret_cast<U>(&is62),reinterpret_cast<U>(&gate62)},context(&c){}
NativeSessionMessage62* construct_native_session_message62_008dd040(NativeSessionMessage62* m,const NativeSessionMessageContext& c,const NativeSessionMessage62Profile& p){
    construct_native_session_message_0075b430(&m->base,62,c);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;v.length_18=0;v.data_1c=nullptr;v.length_20=0;v.data_24=nullptr;return m;
}
bool native_session_message_is62_008dd070(const NativeSessionMessage62* m,U query){if(query==62||query==54)return true;return query==static_cast<const volatile NativeSessionMessageStorage&>(m->base).type_10;}
void write_native_session_message62_008dd0a0(const NativeSessionMessage62* m,NativeBitCursor* c,const NativeSessionMessage56To62Context& x){
    write_native_session_message_type_00449940(&m->base,c);write_native_owned_string_00429ac0(c,&m->length_18,x.fallback_00e17669);
    write_native_owned_string_00429ac0(c,&m->length_20,x.fallback_00e17669);
    write_native_bool_bit_004290b0(c,static_cast<const volatile NativeSessionMessage62&>(*m).flag_28!=0);
}
void read_native_session_message62_008dd0e0(NativeSessionMessage62* m,NativeSessionReadStream* s,const NativeSessionMessage56To62Context& x){
    read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;read_native_owned_string_00429f20(c,&m->length_18,*x.strings);
    read_native_owned_string_00429f20(c,&m->length_20,*x.strings);
    read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&m->flag_28));
}
void destroy_native_session_message62_008dd120(NativeSessionMessage62* m,NativeStringRawPoolContext& strings,const NativeSessionMessage62Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    __try {destroy_native_string_header_0041dd20(&m->length_20,strings);}
    __finally {
    __try {destroy_native_string_header_0041dd20(&m->length_18,strings);}
    __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
    }
}
NativeSessionMessage62* delete_native_session_message62_008dd1a0(NativeSessionMessage62* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage62Profile& p){destroy_native_session_message62_008dd120(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;}
} // namespace bsp
