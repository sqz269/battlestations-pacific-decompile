#include "bsp/native_session_message35.hpp"
#include "bsp/native_bit_cursor_owned_string.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/singleton_lifetime.hpp"
namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using B=std::uint8_t;using W=std::uint16_t;using M=NativeSessionMessage35;
M* __fastcall scalar(M* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage35Profile*>(m->base.profile_00);return delete_native_session_message35_0052b010(m,flags,*p.strings,p);}
void __fastcall write(const M* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage35Profile*>(m->base.profile_00);write_native_session_message35_005294f0(m,c,p.fallback_00e17669);}
void __fastcall read(M* m,void*,NativeSessionReadStream* s){const auto& p=*reinterpret_cast<const NativeSessionMessage35Profile*>(m->base.profile_00);read_native_session_message35_0052abe0(m,s,*p.strings);}
bool __fastcall is_type(const M*,void*,U type){return native_session_message_is35_0052af40(type);}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
void read_flag(NativeBitCursor* c,B* out){bool value;read_native_bool_bit_00428d70(c,&value);*out=static_cast<B>(value);}
}
NativeSessionMessage35Profile::NativeSessionMessage35Profile(NativeStringRawPoolContext& s,const char* fallback)
    :slots{reinterpret_cast<U>(&scalar),reinterpret_cast<U>(&write),reinterpret_cast<U>(&read),reinterpret_cast<U>(&is_type),reinterpret_cast<U>(&yes)},strings(&s),fallback_00e17669(fallback){}
M* construct_native_session_message35_0052aef0(M* m,const NativeSessionMessageContext& c,const NativeSessionMessage35Profile& p){
    construct_native_session_message_0075b430(&m->base,35,c);volatile auto& v=*m;
    v.base.delivery_04=1;v.base.profile_00=p.slots;
    v.length_28=0;v.data_2c=nullptr;v.length_30=0;v.data_34=nullptr;
    v.words_40.data_00=nullptr;v.words_40.size_04=0;v.words_40.capacity_08=0;
    v.length_4c=0;v.data_50=nullptr;v.flag_25=0;v.flag_24=0;v.subtype_20=3;return m;
}
bool native_session_message_is35_0052af40(U type){return type==35;}
void write_native_session_message35_005294f0(const M* m,NativeBitCursor* c,const char* fallback){
    const volatile auto& v=*m;write_native_session_message_type_00449940(&m->base,c);
    write_native_unsigned_dword_bits_00429070(c,v.subtype_20,4);
    if(v.subtype_20==1)write_native_unsigned_dword_bits_00429070(c,v.value_18,8);
    write_native_unsigned_dword_bits_00429070(c,v.value_1c,3);
    write_native_bool_bit_004290b0(c,v.flag_24);
    const U subtype=v.subtype_20;
    if(subtype==0){
        if(v.flag_24!=0)return;
        write_native_unsigned_dword_bits_00429070(c,static_cast<U>(v.words_40.size_04),32);
        I i=0;
        while(i<v.words_40.size_04){
            const auto* word=reinterpret_cast<const volatile W*>(reinterpret_cast<U>(v.words_40.data_00)+static_cast<U>(i)*2u);
            write_native_word_bits_00429120(c,*word,12);i=static_cast<I>(static_cast<U>(i)+1u);
        }
        write_native_owned_string_00429ac0(c,&m->length_4c,fallback);
    }else if(subtype==1){
        if(v.flag_24!=0)return;
        write_native_owned_string_00429ac0(c,&m->length_28,fallback);
        write_native_owned_string_00429ac0(c,&m->length_30,fallback);
        write_native_bool_bit_004290b0(c,v.flag_25);
        if(v.flag_25!=0){write_native_unsigned_dword_bits_00429070(c,v.values_38[0],3);write_native_unsigned_dword_bits_00429070(c,v.values_38[1],3);}
    }
}
void read_native_session_message35_0052abe0(M* m,NativeSessionReadStream* s,NativeStringRawPoolContext& strings){
    volatile auto& v=*m;read_native_session_message_type_00449960(&m->base,s);auto* c=&s->cursor_04;
    read_native_u32_bits_00428d10(c,&m->subtype_20,4);
    if(v.subtype_20==1)read_native_u32_bits_00428d10(c,&m->value_18,8);
    read_native_u32_bits_00428d10(c,&m->value_1c,3);read_flag(c,&m->flag_24);
    const U subtype=v.subtype_20;
    if(subtype==0){
        if(v.flag_24!=0)return;
        U count;read_native_u32_bits_00428d10(c,&count,32);resize_native_word_array_00529980(&m->words_40,0);
        for(U i=0;i<count;++i){
            W value;read_native_word_bits_00428e30(c,&value,12);
            if(v.words_40.size_04==v.words_40.capacity_08){
                I capacity=static_cast<I>(static_cast<U>(v.words_40.capacity_08)*2u);if(capacity<2)capacity=1;
                reserve_native_word_array_005296a0(&m->words_40,capacity);
            }
            const U destination=reinterpret_cast<U>(v.words_40.data_00)+static_cast<U>(v.words_40.size_04)*2u;
            if(destination!=0)*reinterpret_cast<volatile W*>(destination)=value;
            v.words_40.size_04=static_cast<I>(static_cast<U>(v.words_40.size_04)+1u);
        }
        read_native_owned_string_00429f20(c,&m->length_4c,strings);
    }else if(subtype==1){
        if(v.flag_24!=0)return;
        read_native_owned_string_00429f20(c,&m->length_28,strings);
        read_native_owned_string_00429f20(c,&m->length_30,strings);read_flag(c,&m->flag_25);
        if(v.flag_25!=0){U value;read_native_u32_bits_00428d10(c,&value,3);v.values_38[0]=value;read_native_u32_bits_00428d10(c,&value,3);v.values_38[1]=value;}
    }
}
void destroy_native_session_message35_0052af50(M* m,NativeStringRawPoolContext& strings,const NativeSessionMessage35Profile& p){
    static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=p.slots;
    // D950D0 states3/2/1/0 unwind array40, string30, string28, root.
    __try {destroy_native_string_header_0041dd20(&m->length_4c,strings);}
    __finally {
        __try {destroy_native_word_array_0052ad30(&m->words_40);}
        __finally {
            __try {destroy_native_string_header_0041dd20(&m->length_30,strings);}
            __finally {
                __try {destroy_native_string_header_0041dd20(&m->length_28,strings);}
                __finally {static_cast<volatile NativeSessionMessageStorage&>(m->base).profile_00=native_session_message_root_profile_00ce4974();}
            }
        }
    }
}
M* delete_native_session_message35_0052b010(M* m,U flags,NativeStringRawPoolContext& strings,const NativeSessionMessage35Profile& p){
    destroy_native_session_message35_0052af50(m,strings,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;
}
} // namespace bsp
