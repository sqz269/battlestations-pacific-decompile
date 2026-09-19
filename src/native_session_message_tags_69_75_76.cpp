#include "bsp/native_session_message_tags_69_75_76.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_bit_cursor_owned_string.hpp"
#include "bsp/native_checked_string_storage.hpp"
#include "bsp/global_config.hpp"
#include <new>
#include <stdexcept>
#include <vector>

namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using W=std::uint16_t;using B=std::uint8_t;
using Context=NativeSessionMessage69To76Context;
template<class T> void clear(NativeMessage69Vector<T>* v){volatile auto& x=*v;x.begin_04=nullptr;x.end_08=nullptr;x.capacity_0c=nullptr;}
template<class T> U count(const NativeMessage69Vector<T>* v){const volatile auto& x=*v;const U begin=reinterpret_cast<U>(x.begin_04);return begin?static_cast<U>(static_cast<I>(reinterpret_cast<U>(x.end_08)-begin)>>(sizeof(T)==2?1:3)):0;}
template<class T> void check(const NativeMessage69Vector<T>* v,U i,const Context& c){if(i>=count(v))c.invalid_parameters->invalid_parameter(c.invalid_parameters->context);}
template<class T> void destroy_vector(NativeMessage69Vector<T>* v){auto* p=static_cast<const volatile NativeMessage69Vector<T>&>(*v).begin_04;if(p)singleton_lifetime_free(p);clear(v);}
// Bind the installed STL for the end-insertion contract of 007263B0/00725D30.
// Raw allocator avoids modern large-allocation alignment headers. This is a
// compiler-specific storage binding, not a port of the old STL implementation.
template<class T> struct RawAllocator {
    using value_type=T;using size_type=U;using difference_type=std::int64_t;
    using is_always_equal=std::true_type;
    template<class Other> struct rebind {using other=RawAllocator<Other>;};
    RawAllocator() noexcept=default;
    template<class Other> RawAllocator(const RawAllocator<Other>&) noexcept{}
    T* allocate(U n){if(n>0xffffffffu/sizeof(T))throw std::bad_alloc();const U bytes=n*static_cast<U>(sizeof(T));auto* p=static_cast<T*>(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));if(p==nullptr&&n)throw std::bad_alloc();return p;}
    void deallocate(T* p,U) noexcept{singleton_lifetime_free(p);}
    U max_size() const noexcept{return 0x7fffffffu;}
};
template<class T,class V> bool operator==(const RawAllocator<T>&,const RawAllocator<V>&) noexcept{return true;}
template<class T,class V> bool operator!=(const RawAllocator<T>&,const RawAllocator<V>&) noexcept{return false;}
using Words=std::vector<W,RawAllocator<W>>;
static_assert(_ITERATOR_DEBUG_LEVEL!=0 || sizeof(Words)==12);
void append_word(NativeMessage69Vector<W>* words,W value,const Context& c){
    volatile auto& v=*words;const U begin=reinterpret_cast<U>(v.begin_04);
    if(begin && count(words)<static_cast<U>(static_cast<I>(reinterpret_cast<U>(v.capacity_0c)-begin)>>1)){
        auto* end=v.end_08;*end=value;v.end_08=end+1;
    }else{
        auto* end=v.end_08;if(reinterpret_cast<U>(end)<reinterpret_cast<U>(v.begin_04))c.invalid_parameters->invalid_parameter(c.invalid_parameters->context);
        if(sizeof(void*)!=4||_ITERATOR_DEBUG_LEVEL!=0)throw std::logic_error("Message69 WORD storage requires Win32 iterator-debug-level0");
        if(v.begin_04==nullptr)new(static_cast<void*>(&words->begin_04)) Words;
        reinterpret_cast<Words*>(&words->begin_04)->push_back(value);
    }
}
void extended(NativeMessage75ExtendedHeader* h,const U* profile){
    volatile auto& x=*h;x.base.field_08=0;x.base.field_0c=0;x.base.selected_owner_14=nullptr;x.base.type_10=0;x.sender_18=0;x.relay_1a=0;x.base.delivery_04=1;x.flag_1c=0;x.base.profile_00=profile;
}
void write_header(const NativeMessage75ExtendedHeader* h,NativeBitCursor* c){const volatile auto& x=*h;write_native_byte_bits_00428ff0(c,x.base.type_10,8);write_native_word_bits_00429120(c,x.sender_18,12);write_native_bool_bit_004290b0(c,x.relay_1a);write_native_bool_bit_004290b0(c,x.flag_1c);}
void read_header(NativeMessage75ExtendedHeader* h,NativeBitCursor* c){read_native_u8_bits_00428c70(c,&h->base.type_10,8);read_native_word_bits_00428e30(c,&h->sender_18,12);read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&h->relay_1a));read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&h->flag_1c));}
bool __fastcall yes(const void*,void*){return native_session_message_always_true_004499c0();}
#define BSP_MESSAGE_ADAPTERS(N,C,P,W,R,D,S) \
const NativeSessionMessage##N##Profile& profile##N(const NativeSessionMessage##N* m){return *reinterpret_cast<const NativeSessionMessage##N##Profile*>(*reinterpret_cast<const U* const*>(m));} \
NativeSessionMessage##N* __fastcall del##N(NativeSessionMessage##N* m,void*,U f){const auto& p=profile##N(m);return delete_native_session_message##N##_##S(m,f,*p.context,p);} \
void __fastcall write##N(const NativeSessionMessage##N* m,void*,NativeBitCursor* c){write_native_session_message##N##_##W(m,c,*profile##N(m).context);} \
void __fastcall read##N(NativeSessionMessage##N* m,void*,NativeSessionReadStream* s){read_native_session_message##N##_##R(m,s,*profile##N(m).context);} \
bool __fastcall is##N(const void*,void*,U type){return native_session_message_is##N##_##P(type);}
BSP_MESSAGE_ADAPTERS(69,00767570,00767600,008e4a60,008eb7e0,00767610,007676a0)
BSP_MESSAGE_ADAPTERS(75,00759e40,00759e80,0075b950,0075b9e0,00759e70,00759ea0)
BSP_MESSAGE_ADAPTERS(76,00759ec0,00759f00,0075ba70,0075bad0,00759ef0,00759f20)
#undef BSP_MESSAGE_ADAPTERS
}
NativeSessionMessage69To76Context::NativeSessionMessage69To76Context(const NativeSessionMessageContext& s,NativeStringRawPoolContext& p,const char* f,const SingletonLifetimeCallbacks* i):session(&s),strings(&p),fallback_00e17669(f),invalid_parameters(i),checked_strings(p.actual_published_01090aa8,p.actual_small_returns_disabled_01090aa4,p.actual_manager_publication_01090aa0){}
#define BSP_MESSAGE_PROFILE(N,C,P,W,R,D,S) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const Context& c):slots{reinterpret_cast<U>(&del##N),reinterpret_cast<U>(&write##N),reinterpret_cast<U>(&read##N),reinterpret_cast<U>(&is##N),reinterpret_cast<U>(&yes)},context(&c){} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* m,U flags,const Context& c,const NativeSessionMessage##N##Profile& p){destroy_native_session_message##N##_##D(m,c,p);if(flags&1)singleton_lifetime_free(m);return m;}
BSP_MESSAGE_PROFILE(69,00767570,00767600,008e4a60,008eb7e0,00767610,007676a0)
BSP_MESSAGE_PROFILE(75,00759e40,00759e80,0075b950,0075b9e0,00759e70,00759ea0)
BSP_MESSAGE_PROFILE(76,00759ec0,00759f00,0075ba70,0075bad0,00759ef0,00759f20)
#undef BSP_MESSAGE_PROFILE
NativeSessionMessage69* construct_native_session_message69_00767570(NativeSessionMessage69* m,const Context& c,const NativeSessionMessage69Profile& p){
    volatile auto& b=m->base;b.delivery_04=3;b.field_08=0;b.field_0c=0;b.profile_00=native_session_message_base_profile_00d02c68();b.type_10=69;
    const auto* game=static_cast<const B*>(c.session->current_game_00e188a8);const I owner=*reinterpret_cast<const volatile I*>(game+0x18ec);
    b.selected_owner_14=owner>=0&&owner<=7?*reinterpret_cast<void* const volatile*>(game+0x18cc+static_cast<U>(owner)*4):nullptr;
    b.delivery_04=1;b.profile_00=p.slots;clear(&m->words_18);clear(&m->strings_28);return m;
}
bool native_session_message_is69_00767600(U t){return t==69;}
void write_native_session_message69_008e4a60(const NativeSessionMessage69* m,NativeBitCursor* cursor,const Context& c){
    write_native_byte_bits_00428ff0(cursor,static_cast<const volatile NativeSessionMessageStorage&>(m->base).type_10,8);
    write_native_signed_dword_bits_00429090(cursor,count(&m->words_18),8);
    for(U i=0;i<count(&m->words_18);++i){
        check(&m->words_18,i,c);write_native_word_bits_00429120(cursor,static_cast<const volatile NativeMessage69Vector<W>&>(m->words_18).begin_04[i],12);
        check(&m->strings_28,i,c);write_native_owned_string_00429ac0(cursor,static_cast<const volatile NativeMessage69Vector<NativeMessage69String>&>(m->strings_28).begin_04+i,c.fallback_00e17669);
    }
}
void read_native_session_message69_008eb7e0(NativeSessionMessage69* m,NativeSessionReadStream* stream,const Context& c){
    auto* cursor=&stream->cursor_04;read_native_u8_bits_00428c70(cursor,&m->base.type_10,8);NativeMessage69String temporary{0,nullptr};
    __try{I count_value;read_native_i32_bits_00428d30(cursor,&count_value,8);for(I i=0;i<count_value;++i){W word;read_native_word_bits_00428e30(cursor,&word,12);append_word(&m->words_18,word,c);read_native_owned_string_00429f20(cursor,&temporary,*c.strings);append_checked_native_string_storage(&m->strings_28,&temporary,c.checked_strings);}}
    __finally{destroy_native_string_header_0041dd20(&temporary,*c.strings);}
}
void destroy_native_session_message69_00767610(NativeSessionMessage69* m,const Context& c,const NativeSessionMessage69Profile& p){
    volatile auto& b=m->base;b.profile_00=p.slots;
    __try{auto* begin=static_cast<const volatile NativeMessage69Vector<NativeMessage69String>&>(m->strings_28).begin_04;if(begin){auto* end=static_cast<const volatile NativeMessage69Vector<NativeMessage69String>&>(m->strings_28).end_08;destroy_global_config_name_range_00432050(begin,end,c.checked_strings);singleton_lifetime_free(static_cast<const volatile NativeMessage69Vector<NativeMessage69String>&>(m->strings_28).begin_04);}clear(&m->strings_28);}
    __finally{destroy_vector(&m->words_18);b.profile_00=native_session_message_root_profile_00ce4974();}
}
NativeSessionMessage75* construct_native_session_message75_00759e40(NativeSessionMessage75* m,const Context&,const NativeSessionMessage75Profile& p){extended(&m->header,p.slots);return m;}
NativeSessionMessage76* construct_native_session_message76_00759ec0(NativeSessionMessage76* m,const Context&,const NativeSessionMessage76Profile& p){extended(&m->header,p.slots);return m;}
bool native_session_message_is75_00759e80(U t){return t==75||t==73||t==70;}
bool native_session_message_is76_00759f00(U t){return t==76||t==73||t==70;}
void write_native_session_message75_0075b950(const NativeSessionMessage75* m,NativeBitCursor* cursor,const Context&){
    const volatile auto& x=*m;write_header(&m->header,cursor);write_native_bool_bit_004290b0(cursor,x.flag_20);write_native_unsigned_dword_bits_00429070(cursor,x.value_28,6);
    if(!x.flag_20){write_native_unsigned_dword_bits_00429070(cursor,x.value_24,9);write_native_unsigned_dword_bits_00429070(cursor,x.value_2c,1);write_native_bool_bit_004290b0(cursor,x.flag_30);}
}
void read_native_session_message75_0075b9e0(NativeSessionMessage75* m,NativeSessionReadStream* stream,const Context&){
    auto* cursor=&stream->cursor_04;read_header(&m->header,cursor);read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->flag_20));read_native_u32_bits_00428d10(cursor,&m->value_28,6);
    if(!static_cast<const volatile NativeSessionMessage75&>(*m).flag_20){read_native_u32_bits_00428d10(cursor,&m->value_24,9);read_native_u32_bits_00428d10(cursor,&m->value_2c,1);read_native_bool_bit_00428d70(cursor,reinterpret_cast<bool*>(&m->flag_30));}
}
void write_native_session_message76_0075ba70(const NativeSessionMessage76* m,NativeBitCursor* cursor,const Context&){const volatile auto& x=*m;write_header(&m->header,cursor);write_native_unsigned_dword_bits_00429070(cursor,x.value_20,9);write_native_unsigned_dword_bits_00429070(cursor,x.value_24,6);}
void read_native_session_message76_0075bad0(NativeSessionMessage76* m,NativeSessionReadStream* stream,const Context&){auto* cursor=&stream->cursor_04;read_header(&m->header,cursor);read_native_u32_bits_00428d10(cursor,&m->value_20,9);read_native_u32_bits_00428d10(cursor,&m->value_24,6);}
void destroy_native_session_message75_00759e70(NativeSessionMessage75* m,const Context&,const NativeSessionMessage75Profile&){static_cast<volatile NativeSessionMessageStorage&>(m->header.base).profile_00=native_session_message_root_profile_00ce4974();}
void destroy_native_session_message76_00759ef0(NativeSessionMessage76* m,const Context&,const NativeSessionMessage76Profile&){static_cast<volatile NativeSessionMessageStorage&>(m->header.base).profile_00=native_session_message_root_profile_00ce4974();}
} // namespace bsp
