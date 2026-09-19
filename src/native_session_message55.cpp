#include "bsp/native_session_message55.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_owned_string.hpp"
#include <cstring>
namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;
void invalid(const NativeMessage55Context& x){x.invalid_parameters->invalid_parameter(x.invalid_parameters->context);}
U numeric_scale(const NativeMessage55Context& x){const volatile float* p=x.common->maximum_00d7a248;U value;__asm {mov ecx,p
 fld dword ptr [ecx]
 fstp dword ptr value}return value;}
U receive_count(const volatile NativeMessage55Vector& v){const U begin=reinterpret_cast<U>(v.begin_04);return begin==0?0:static_cast<U>(static_cast<I>(reinterpret_cast<U>(v.end_08)-begin)/28);}
NativeMessage55Receive* receive_at(const volatile NativeMessage55Vector& v,U index){return reinterpret_cast<NativeMessage55Receive*>(reinterpret_cast<U>(v.begin_04)+index*28u);}
void* resolve_handle(U id,const ObjectHandleTables& t){U index;const void* entries;
    if(static_cast<I>(id)<t.first_end_00f89a10){index=id-static_cast<U>(t.first_begin_00f89a0c);entries=t.first_entries_00f89a54;}
    else{index=id-static_cast<U>(t.second_begin_00f89a60);entries=t.second_entries_00f89aa8;}
    return *reinterpret_cast<void* const volatile*>(reinterpret_cast<U>(entries)+(index<<4)+0xcu);
}
NativeMessage55Entry* __fastcall entry_scalar(NativeMessage55Entry* e,void*,U flags){const auto& p=*reinterpret_cast<const NativeMessage55EntryProfile*>(e->profile_00);return delete_native_message55_entry_008dd440(e,flags,*p.context,p);}
NativeSessionMessage55* __fastcall scalar(NativeSessionMessage55* m,void*,U flags){const auto& p=*reinterpret_cast<const NativeSessionMessage55Profile*>(m->base.profile_00);return delete_native_session_message55_008e1510(m,flags,*p.context,p);}
void __fastcall write(NativeSessionMessage55* m,void*,NativeBitCursor* c){const auto& p=*reinterpret_cast<const NativeSessionMessage55Profile*>(m->base.profile_00);write_native_session_message55_008e0600(m,c,*p.context);}
void __fastcall read(NativeSessionMessage55* m,void*,NativeSessionReadStream* s){const auto& p=*reinterpret_cast<const NativeSessionMessage55Profile*>(m->base.profile_00);read_native_session_message55_008e1930(m,s,*p.context,*p.entry_profile);}
bool __fastcall matches(const NativeSessionMessage55* m,void*,U type){return native_session_message_is55_008e0320(m,type);}
bool __fastcall resolve(NativeSessionMessage55* m,void*){const auto& p=*reinterpret_cast<const NativeSessionMessage55Profile*>(m->base.profile_00);return resolve_native_message55_handles_008e0350(m,*p.context);}
NativeMessage55Entry* new_entry(const NativeMessage55Context& x,const NativeMessage55EntryProfile& p){
    auto* e=static_cast<NativeMessage55Entry*>(singleton_lifetime_allocate({SingletonAllocationKind::object,0x2c,sizeof(NativeMessage55Entry)}));
    if(e==nullptr)return nullptr;bool complete=false;
    __try{construct_native_message55_entry_008e08f0(e,x,p);complete=true;}
    __finally{if(!complete)singleton_lifetime_free(e);}return e;
}
}
NativeMessage55EntryProfile::NativeMessage55EntryProfile(const NativeMessage55Context& x):slots{reinterpret_cast<U>(&entry_scalar)},context(&x){}
NativeSessionMessage55Profile::NativeSessionMessage55Profile(const NativeMessage55Context& x,const NativeMessage55EntryProfile& p):slots{reinterpret_cast<U>(&scalar),reinterpret_cast<U>(&write),reinterpret_cast<U>(&read),reinterpret_cast<U>(&matches),reinterpret_cast<U>(&resolve)},context(&x),entry_profile(&p){}
NativeMessage55Entry* construct_native_message55_entry_008e08f0(NativeMessage55Entry* e,const NativeMessage55Context& x,const NativeMessage55EntryProfile& p){
    volatile auto& v=*e;v.profile_00=p.slots;v.length_04=0;v.data_08=nullptr;bool complete=false;
    __try{
        v.length_0c=0;v.data_10=nullptr;
        __try{v.positions_20.sentinel_04=x.library->sentinel_008db4f0(&e->positions_20);v.positions_20.count_08=0;complete=true;}
        __finally{if(!complete)destroy_native_string_header_0041dd20(&e->length_0c,*x.common->strings);}
    }__finally{if(!complete)destroy_native_string_header_0041dd20(&e->length_04,*x.common->strings);}
    return e;
}
void destroy_native_message55_entry_008dc100(NativeMessage55Entry* e,const NativeMessage55Context& x,const NativeMessage55EntryProfile& p){
    static_cast<volatile NativeMessage55Entry&>(*e).profile_00=p.slots;volatile auto& list=e->positions_20;
    auto* node=static_cast<volatile NativeMessage55Node&>(*list.sentinel_04).next;
    // EH state0 releases only header+4. It does not promise inner-list or
    // header+C cleanup if an earlier operation faults.
    __try{
        while(node!=list.sentinel_04){
            if(node==list.sentinel_04)invalid(x);
            void* payload=static_cast<volatile NativeMessage55Node&>(*node).payload;
            if(payload!=nullptr){singleton_lifetime_free(payload);static_cast<volatile NativeMessage55Node&>(*node).payload=nullptr;}
            if(node==list.sentinel_04)invalid(x);node=static_cast<volatile NativeMessage55Node&>(*node).next;
        }
        auto* head=list.sentinel_04;node=static_cast<volatile NativeMessage55Node&>(*head).next;
        static_cast<volatile NativeMessage55Node&>(*head).next=head;
        head=list.sentinel_04;static_cast<volatile NativeMessage55Node&>(*head).previous=head;list.count_08=0;
        while(node!=list.sentinel_04){auto* next=static_cast<volatile NativeMessage55Node&>(*node).next;singleton_lifetime_free(node);node=next;}
        singleton_lifetime_free(list.sentinel_04);list.sentinel_04=nullptr;
        destroy_native_string_header_0041dd20(&e->length_0c,*x.common->strings);
    }__finally{destroy_native_string_header_0041dd20(&e->length_04,*x.common->strings);}
}
NativeMessage55Entry* delete_native_message55_entry_008dd440(NativeMessage55Entry* e,U flags,const NativeMessage55Context& x,const NativeMessage55EntryProfile& p){destroy_native_message55_entry_008dc100(e,x,p);if((flags&1u)!=0)singleton_lifetime_free(e);return e;}
NativeSessionMessage55* construct_native_session_message55_008e0170(NativeSessionMessage55* m,const NativeMessage55Context& x,const NativeSessionMessage55Profile& p){
    construct_native_session_message_0075b430(&m->base,55,*x.common->session);volatile auto& v=*m;v.base.delivery_04=1;v.base.profile_00=p.slots;bool complete=false;
    __try{v.entries_18.sentinel_04=x.library->sentinel_008db560(&m->entries_18);v.entries_18.count_08=0;v.received_24.begin_04=nullptr;v.received_24.end_08=nullptr;v.received_24.capacity_0c=nullptr;complete=true;}
    __finally{if(!complete)v.base.profile_00=native_session_message_root_profile_00ce4974();}return m;
}
bool native_session_message_is55_008e0320(const NativeSessionMessage55* m,U query){if(query==55||query==54)return true;return query==static_cast<const volatile NativeSessionMessageStorage&>(m->base).type_10;}
bool resolve_native_message55_handles_008e0350(NativeSessionMessage55* m,const NativeMessage55Context& x){
    if(!native_session_message_process_flag_008dae20(*x.common->session))return false;
    volatile auto& v=m->received_24;U index=0,offset=0;
    while(index<receive_count(v)){
        if(v.begin_04==nullptr||index>=receive_count(v))invalid(x);
        auto* row=reinterpret_cast<volatile NativeMessage55Receive*>(reinterpret_cast<U>(v.begin_04)+offset);
        if(row->has_handle_08!=0&&row->resolved_0c==nullptr){void* result=resolve_handle(row->handle_0a,*x.common->handles);row->resolved_0c=result;if(result==nullptr)return false;}
        ++index;offset+=28u;
    }return true;
}
void write_native_session_message55_008e0600(const NativeSessionMessage55* m,NativeBitCursor* c,const NativeMessage55Context& x){
    const volatile auto& owner=*m;write_native_byte_bits_00428ff0(c,owner.base.type_10,8);
    write_native_unsigned_dword_bits_00429070(c,owner.entries_18.count_08,8);
    auto* outer=static_cast<volatile NativeMessage55Node&>(*owner.entries_18.sentinel_04).next;
    while(outer!=owner.entries_18.sentinel_04){
        if(outer==owner.entries_18.sentinel_04)invalid(x);
        auto* e=static_cast<NativeMessage55Entry*>(static_cast<volatile NativeMessage55Node&>(*outer).payload);const volatile auto& v=*e;
        write_native_owned_string_00429ac0(c,&e->length_04,x.common->fallback_00e17669);write_native_owned_string_00429ac0(c,&e->length_0c,x.common->fallback_00e17669);
        write_native_unsigned_dword_bits_00429070(c,v.value_18,2);write_native_unsigned_dword_bits_00429070(c,v.value_1c,2);write_native_unsigned_dword_bits_00429070(c,v.positions_20.count_08,8);
        auto* inner=static_cast<volatile NativeMessage55Node&>(*v.positions_20.sentinel_04).next;
        while(inner!=v.positions_20.sentinel_04){
            if(inner==v.positions_20.sentinel_04)invalid(x);
            auto* position=static_cast<NativeMessage55Position*>(static_cast<volatile NativeMessage55Node&>(*inner).payload);
            const bool present=static_cast<volatile NativeMessage55Position&>(*position).object_00!=nullptr;write_native_bool_bit_004290b0(c,present);
            if(present){const U object=reinterpret_cast<U>(static_cast<volatile NativeMessage55Position&>(*position).object_00);write_native_word_bits_00429120(c,*reinterpret_cast<const volatile std::uint16_t*>(object+0x174u),12);}
            else{const U scale=numeric_scale(x);write_native_numeric_float_array_00429790(c,position->position_04,3,0,1,scale,32,*x.common->numeric);}
            if(inner==v.positions_20.sentinel_04)invalid(x);inner=static_cast<volatile NativeMessage55Node&>(*inner).next;
        }
        if(outer==owner.entries_18.sentinel_04)invalid(x);outer=static_cast<volatile NativeMessage55Node&>(*outer).next;
    }
}
void read_native_session_message55_008e1930(NativeSessionMessage55* m,NativeSessionReadStream* stream,const NativeMessage55Context& x,const NativeMessage55EntryProfile& p){
    auto* c=&stream->cursor_04;read_native_u8_bits_00428c70(c,&m->base.type_10,8);U count;read_native_u32_bits_00428d10(c,&count,8);
    x.library->resize_entries_008e0ec0(&m->entries_18,count,nullptr);volatile auto& list=m->entries_18;auto* node=static_cast<volatile NativeMessage55Node&>(*list.sentinel_04).next;
    while(node!=list.sentinel_04){
        auto* e=new_entry(x,p);if(node==list.sentinel_04)invalid(x);static_cast<volatile NativeMessage55Node&>(*node).payload=e;
        read_native_owned_string_00429f20(c,&e->length_04,*x.common->strings);read_native_owned_string_00429f20(c,&e->length_0c,*x.common->strings);
        read_native_u32_bits_00428d10(c,&e->value_18,2);read_native_u32_bits_00428d10(c,&e->value_1c,2);read_native_u32_bits_00428d10(c,&count,8);
        for(U i=0;i<count;++i){
            volatile auto& v=m->received_24;const U requested=receive_count(v)+1u;
            NativeMessage55Receive value;value.length_00=0;value.data_04=nullptr;
            x.library->resize_received_008e17c0(&m->received_24,requested,value);
            const U index=receive_count(v)-1u;if(v.begin_04==nullptr||index>=receive_count(v))invalid(x);
            auto* row=receive_at(v,index);
            if(reinterpret_cast<void*>(row)!=reinterpret_cast<void*>(&e->length_04)){
                resize_native_string_header_0041dd40(&row->length_00,*x.common->strings,static_cast<volatile NativeMessage55Entry&>(*e).length_04,true);
                if(static_cast<volatile NativeMessage55Entry&>(*e).length_04!=0)std::memmove(static_cast<volatile NativeMessage55Receive&>(*row).data_04,static_cast<volatile NativeMessage55Entry&>(*e).data_08,static_cast<volatile NativeMessage55Receive&>(*row).length_00);
            }
            read_native_bool_bit_00428d70(c,reinterpret_cast<bool*>(&row->has_handle_08));
            if(static_cast<volatile NativeMessage55Receive&>(*row).has_handle_08!=0){read_native_word_bits_00428e30(c,&row->handle_0a,12);static_cast<volatile NativeMessage55Receive&>(*row).resolved_0c=nullptr;}
            else{const U scale=numeric_scale(x);read_native_numeric_float_array_004294f0(c,row->position_10,3,0,1,scale,32,*x.common->numeric);}
        }
        if(node==list.sentinel_04)invalid(x);node=static_cast<volatile NativeMessage55Node&>(*node).next;
    }
}
void destroy_native_session_message55_008e1480(NativeSessionMessage55* m,const NativeMessage55Context& x,const NativeSessionMessage55Profile& p){
    volatile auto& v=*m;v.base.profile_00=p.slots;bool complete=false;
    __try{
        auto* begin=v.received_24.begin_04;if(begin!=nullptr){auto* end=v.received_24.end_08;x.library->destroy_received_008de660(begin,end,&m->received_24,m);singleton_lifetime_free(v.received_24.begin_04);}
        v.received_24.begin_04=nullptr;v.received_24.end_08=nullptr;v.received_24.capacity_0c=nullptr;x.library->destroy_entries_008db580(&m->entries_18);complete=true;
    }__finally{
        __try{if(!complete)x.library->unwind_entries_008dbab0(&m->entries_18);}
        __finally{v.base.profile_00=native_session_message_root_profile_00ce4974();}
    }
}
NativeSessionMessage55* delete_native_session_message55_008e1510(NativeSessionMessage55* m,U flags,const NativeMessage55Context& x,const NativeSessionMessage55Profile& p){destroy_native_session_message55_008e1480(m,x,p);if((flags&1u)!=0)singleton_lifetime_free(m);return m;}
NativeSessionMessageStorage* create_native_session_messages55_to62_008e1530(U selector,const NativeMessages55To62Profiles& p){
    const U index=selector-55u;if(index>7u)return nullptr;
    constexpr U sizes[8]={0x34,0x30,0x24,0x24,0x2c,0x2c,0x2c,0x2c};void* allocation=singleton_lifetime_allocate({SingletonAllocationKind::object,sizes[index],sizes[index]});if(allocation==nullptr)return nullptr;bool complete=false;
    __try{
        switch(selector){
        case 55:construct_native_session_message55_008e0170(static_cast<NativeSessionMessage55*>(allocation),*p.type55->context,*p.type55);break;
        case 56:construct_native_session_message56_008dc6a0(static_cast<NativeSessionMessage56*>(allocation),*p.type56->context->session,*p.type56);break;
        case 57:construct_native_session_message57_008dc840(static_cast<NativeSessionMessage57*>(allocation),*p.type57->context->session,*p.type57);break;
        case 58:construct_native_session_message58_008dca00(static_cast<NativeSessionMessage58*>(allocation),*p.type58->context->session,*p.type58);break;
        case 59:construct_native_session_message59_008dcbc0(static_cast<NativeSessionMessage59*>(allocation),*p.type59->context->session,*p.type59);break;
        case 60:construct_native_session_message60_008dcd40(static_cast<NativeSessionMessage60*>(allocation),*p.type60->context->session,*p.type60);break;
        case 61:construct_native_session_message61_008dcec0(static_cast<NativeSessionMessage61*>(allocation),*p.type61->context->session,*p.type61);break;
        case 62:construct_native_session_message62_008dd040(static_cast<NativeSessionMessage62*>(allocation),*p.type62->context->session,*p.type62);break;
        }complete=true;
    }__finally{if(!complete)singleton_lifetime_free(allocation);}return static_cast<NativeSessionMessageStorage*>(allocation);
}
} // namespace bsp
