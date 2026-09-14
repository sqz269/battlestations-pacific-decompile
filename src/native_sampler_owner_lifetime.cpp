#include "bsp/native_sampler_owner_lifetime.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
using Op=NativeSamplerOwnerLifetimeOperation;
using Context=NativeSamplerOwnerLifetimeContext;
static_assert(sizeof(void*)==4);
void* at(const void* p,U offset=0) noexcept {return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);}
U word(const void* p,U offset=0) noexcept {return *static_cast<const volatile U*>(at(p,offset));}
void put(void* p,U offset,U v) noexcept {*static_cast<volatile U*>(at(p,offset))=v;}
void* pointer(const void* p,U offset=0) noexcept {return reinterpret_cast<void*>(word(p,offset));}
std::int32_t signed_bits(U bits) noexcept {std::int32_t n;std::memcpy(&n,&bits,4);return n;}
void begin(Op& a,U fn,void* owner,Context& c){
    if(a.phase!=Op::Phase::fresh)throw std::logic_error("sampler owner operation is one-shot");
    a.phase=Op::Phase::running;a.function=fn;a.owner=owner;a.context=&c;
}
template<class F> void invoke(Op& a,F body){try{body();a.phase=Op::Phase::complete;}catch(...){a.phase=Op::Phase::failed;throw;}}
void resize(void* array,std::int32_t requested,Op& a){
    a.array=array;
    if(requested>signed_bits(word(array,8))){a.native_site=0x4dc43a;reserve_native_resource_record_vector_004da180(*static_cast<NativeResourceRecordVectorStorage*>(array),requested,a.context->strings,a.context->validation);}
    U index=word(array,4);a.resize_index=index;
    while(signed_bits(index)<requested){
        auto* record=static_cast<NativeRenderResourceRecord*>(at(pointer(array),index*0x2cu));a.record=record;
        try{
            if(record){
                ::new(record) NativeRenderResourceRecord;
                put(record,0,0);put(record,4,0);a.resize_name_armed=true;
                a.native_site=0x4dc470;auto* sentinel=allocate_native_render_alias_sentinel_004c3020();
                put(record,0xc,reinterpret_cast<U>(sentinel));put(record,0x10,0);
                put(record,0x24,0);put(record,0x20,0);put(record,0x1c,0);put(record,0x18,0);put(record,0x14,0);
            }
        }catch(...){
            if(a.resize_name_armed){a.resize_name_armed=false;destroy_native_string_header_0041dd20(record,a.context->strings);}
            // C669A0 computes current array base+captured index*2Ch and saved
            // placement pointer, then calls RET-only401130. No allocation free.
            (void)at(pointer(array),index*0x2cu);throw;
        }
        ++index;a.resize_index=index;a.resize_name_armed=false;
    }
    while(requested<signed_bits(word(array,4))){
        put(array,4,word(array,4)-1u);
        const U current_index=word(array,4);
        auto* record=static_cast<NativeRenderResourceRecord*>(at(pointer(array),current_index*0x2cu));a.record=record;
        a.native_site=0x4dc4bc;destroy_native_resource_record_004d45a0(*record,a.context->strings);
    }
    put(array,4,static_cast<U>(requested));
}
void release(void* resource,Op& a){
    a.resource=resource;a.release_started=true;a.release_returned=false;a.native_site=0x4ddb49;
    release_native_render_actual_owner(a.context->owners,resource);
    a.release_returned=true;
}
void clear(void* cache,Op& a){
    a.secondary=cache;
    while(word(cache,8)!=0){
        const U count=word(cache,8);void* const data=pointer(cache,4);
        void* const resource=pointer(at(data,count*0x2cu-4u));a.resource=resource;
        const void* profile;
        switch(word(cache)){
        case 0x00ce7d08:profile=a.context->actual_cache_profile_00ce7d08;break;
        case 0x00ce7d24:profile=a.context->actual_cache_profile_00ce7d24;break;
        default:throw std::invalid_argument("Unimplemented sampler cache release profile");
        }
        a.native_site=0x4dda65;
        if(word(profile,0x10)!=0x004ddb40)throw std::invalid_argument("Unimplemented current sampler cache release slot");
        release(resource,a);
        const U current_count=word(cache,8);
        if(current_count){
            auto* record=static_cast<NativeRenderResourceRecord*>(at(pointer(cache,4),current_count*0x2cu-0x2cu));a.record=record;
            a.native_site=0x4dda78;destroy_native_resource_record_004d45a0(*record,a.context->strings);
            put(cache,8,word(cache,8)-1u);
        }
    }
    a.native_site=0x4dda8c;resize(at(cache,4),0,a);
}
void array_destroy(void* array,Op& a){
    a.native_site=0x4ddaa5;resize(array,0,a);
    a.array_allocation=pointer(array);a.array_free_started=true;a.array_free_returned=false;a.native_site=0x4ddaad;
    singleton_lifetime_free(a.array_allocation);a.array_free_returned=true;
}
void cache_destroy(void* cache,Op& a){
    a.secondary=cache;put(cache,0,0x00ce7d08);
    try{a.native_site=0x4de2bb;clear(cache,a);}catch(...){array_destroy(at(cache,4),a);throw;}
    // State -1 precedes the normal resize/free tail; no repeated unwind here.
    a.native_site=0x4de2cf;resize(at(cache,4),0,a);
    a.array_allocation=pointer(at(cache,4));a.array_free_started=true;a.array_free_returned=false;a.native_site=0x4de2d7;
    singleton_lifetime_free(a.array_allocation);a.array_free_returned=true;
}
void base_destroy(void* owner,Op& a){
    a.context->actual_owner_00f8d420=nullptr;a.publication_cleared=true;put(owner,0,0x00ce3818);
}
void owner_destroy(void* owner,Op& a){
    put(owner,0,0x00ce7d38);put(owner,4,0x00ce7d24);
    try{a.native_site=0xb1b6b4;cache_destroy(at(owner,4),a);}catch(...){base_destroy(owner,a);throw;}
    base_destroy(owner,a);
}
void owner_delete(void* owner,U flags,Op& a){
    a.flags=flags;a.native_site=0x4de343;owner_destroy(owner,a);
    if(flags&1u){a.owner_free_started=true;a.native_site=0x4de350;singleton_lifetime_free(owner);a.owner_free_returned=true;}
}
} // namespace
NativeSamplerOwnerLifetimeOperation::~NativeSamplerOwnerLifetimeOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeSamplerOwnerLifetimeOperation::acknowledge_diagnostic_cleanup() noexcept {if(phase==Phase::failed)phase=Phase::diagnostic_retired;}
void resize_native_sampler_records_004dc410(NativeResourceRecordVectorStorage& v,std::int32_t n,Context& c,Op& a){begin(a,0x4dc410,&v,c);invoke(a,[&]{resize(&v,n,a);});}
void release_native_sampler_resource_004ddb40(void* r,Context& c,Op& a){begin(a,0x4ddb40,r,c);invoke(a,[&]{release(r,a);});}
void clear_native_sampler_cache_004dda40(void* p,Context& c,Op& a){begin(a,0x4dda40,p,c);invoke(a,[&]{clear(p,a);});}
void destroy_native_sampler_record_array_004ddaa0(NativeResourceRecordVectorStorage& v,Context& c,Op& a){begin(a,0x4ddaa0,&v,c);invoke(a,[&]{array_destroy(&v,a);});}
void destroy_native_sampler_cache_004de290(void* p,Context& c,Op& a){begin(a,0x4de290,p,c);invoke(a,[&]{cache_destroy(p,a);});}
void destroy_native_sampler_owner_base_004b4f10(void* p,Context& c,Op& a){begin(a,0x4b4f10,p,c);invoke(a,[&]{base_destroy(p,a);});}
void destroy_native_sampler_owner_00b1b680(NativeSamplerLoaderSingletonStorage& p,Context& c,Op& a){begin(a,0xb1b680,&p,c);invoke(a,[&]{owner_destroy(&p,a);});}
void* delete_native_sampler_owner_004de340(void* p,U flags,Context& c,Op& a){begin(a,0x4de340,p,c);invoke(a,[&]{owner_delete(p,flags,a);});return p;}
void* delete_native_sampler_owner_secondary_004de360(void* p,U flags,Context& c,Op& a){void* const owner=at(p,0xfffffffcu);begin(a,0x4de360,owner,c);invoke(a,[&]{owner_delete(owner,flags,a);});return owner;}
} // namespace bsp
