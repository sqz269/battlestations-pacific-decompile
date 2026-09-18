#include "bsp/native_language_catalog_storage.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_vfs_enumeration.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native language catalog storage requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;
using Op=NativeLanguageCatalogStorageOperation;using Phase=Op::Phase;
void* at(const void* p,U n=0) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
template<class T=U>T get(const void* p,U n=0) noexcept{T v;std::memcpy(&v,at(p,n),sizeof v);return v;}
void put(void* p,U n,U v) noexcept{std::memcpy(at(p,n),&v,4);}
void begin(void* p,Op& op) {
    if(op.phase!=Phase::fresh)throw std::logic_error("native language storage operation cannot replay");
    op.owner=p;op.phase=Phase::running;
}
void copy_row(void* p,const void* source,NativeStringRawPoolContext& strings,Op& op) {
    op.current_row=p;op.completed_members=0;op.row_unwind_state=-1;
    constexpr U sites[]{0x8d58c4,0x8d5904,0x8d5944,0x8d5986};
    for(U member=0;member<4;++member) {
        void* const d=at(p,member*8);const void* const s=at(source,member*8);
        put(d,0,0);put(d,4,0);
        if(d!=s) {
            const U requested=get(s);op.row_site=sites[member];
            resize_native_string_header_0041dd40(d,strings,requested,true);
            if(get(s)!=0) {
                const U count=get(d);void* const from=get<void*>(s,4);void* const to=get<void*>(d,4);
                std::memcpy(to,from,count);
            }
        }
        op.completed_members=member+1;op.row_unwind_state=member<3?static_cast<I>(member):2;
    }
}
void destroy_row(void* p,NativeStringRawPoolContext& strings,Op& op) {
    op.current_row=p;op.completed_members=0;
    constexpr U sites[]{0x8d4f86,0x8d4fa8,0x8d4fca,0x8d4fee};
    for(U member=0;member<4;++member) {
        op.row_unwind_state=2-static_cast<I>(member);op.row_site=sites[member];
        destroy_native_string_header_0041dd20(at(p,(3-member)*8),strings);
        op.completed_members=member+1;
    }
}
void reserve(void* p,I requested,NativeStringRawPoolContext& strings,
    NativeLanguageCatalogAllocationCalls& calls,Op& op) {
    if(requested<1)requested=1;
    if(get<I>(p,8)>=requested)return;
    op.native_site=0x8d5a02;
    void* const candidate=calls.allocate_00bf55be(static_cast<U>(requested)<<5);
    op.candidate=candidate;op.copied_rows=0;op.destroyed_rows=0;
    U index=0;
    while(static_cast<I>(index)<get<I>(p,4)) {
        void* const row=at(candidate,index<<5);op.unwind_state=0;
        if(row) {
            const void* const source=at(get<void*>(p),index<<5);op.native_site=0x8d5a3d;
            copy_row(row,source,strings,op);
        }
        ++index;op.copied_rows=index;op.unwind_state=-1;
    }
    index=0;
    while(static_cast<I>(index)<get<I>(p,4)) {
        void* const row=at(get<void*>(p),index<<5);op.native_site=0x8d5a64;
        destroy_row(row,strings,op);++index;op.destroyed_rows=index;
    }
    void* const old=get<void*>(p);op.native_site=0x8d5a7b;calls.free_00bf6989(old);
    put(p,0,reinterpret_cast<U>(candidate));put(p,8,static_cast<U>(requested));
}
}
void* NativeLanguageCatalogAllocationCalls::allocate_00bf55be(U n){return singleton_lifetime_allocate({SingletonAllocationKind::object,n,n});}
void NativeLanguageCatalogAllocationCalls::free_00bf6989(void* p){singleton_lifetime_free(p);}
Op::~NativeLanguageCatalogStorageOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void Op::acknowledge_diagnostic_cleanup() noexcept{scanner.acknowledge_diagnostic_cleanup();phase=Phase::diagnostic_retired;}
void* copy_construct_native_language_row_008d5890(void* p,const void* source,NativeStringRawPoolContext& strings,Op& op) {
    begin(p,op);op.source=source;
    try{copy_row(p,source,strings,op);op.phase=Phase::complete;return p;}
    catch(...){op.phase=Phase::failed;throw;}
}
void destroy_native_language_row_008d4f50(void* p,NativeStringRawPoolContext& strings,Op& op) {
    begin(p,op);
    try{destroy_row(p,strings,op);op.phase=Phase::complete;}
    catch(...){op.phase=Phase::failed;throw;}
}
void reserve_native_language_catalog_008d59c0(void* p,I n,NativeStringRawPoolContext& strings,NativeLanguageCatalogAllocationCalls& calls,Op& op) {
    begin(p,op);
    try{reserve(p,n,strings,calls,op);op.phase=Phase::complete;}
    catch(...){op.phase=Phase::failed;throw;}
}
void append_native_language_catalog_008d6af0(void* p,const void* source,NativeStringRawPoolContext& strings,NativeLanguageCatalogAllocationCalls& calls,Op& op) {
    begin(p,op);op.source=source;
    try{
        const U capacity=get(p,8);
        if(get(p,4)==capacity){const U doubled=capacity+capacity;const I n=static_cast<I>(doubled)>1?static_cast<I>(doubled):1;reserve(p,n,strings,calls,op);}
        void* const row=at(get<void*>(p),get(p,4)<<5);op.unwind_state=0;
        if(row){op.native_site=0x8d6b44;copy_row(row,source,strings,op);}
        put(p,4,get(p,4)+1);op.phase=Phase::complete;
    }catch(...){op.phase=Phase::failed;throw;}
}
void release_native_language_scanner_008d47f0(void* p,NativeStreamTextScannerContext& c,Op& op) {
    begin(p,op);
    try{
        if(void* const scanner=get<void*>(p)) {
            op.candidate=scanner;op.native_site=0x8d47fc;
            destroy_native_stream_text_scanner_00bef220(scanner,c,op.scanner);
            op.native_site=0x8d4802;c.calls.free_00bf65ac(scanner);put(p,0,0);
        }
        op.phase=Phase::complete;
    }catch(...){op.phase=Phase::failed;throw;}
}
U native_language_prefix_result_00553c80(const void* p,const char* prefix,U position) noexcept {
    const U data=get(p,4);
    if(!data||!prefix||position>get(p))return data&0xffffff00u;
    const auto* cursor=static_cast<const char*>(at(reinterpret_cast<void*>(data),position));
    while(*cursor&&*prefix&&*cursor==*prefix){++cursor;++prefix;}
    return *prefix==0?1u:0u;
}
void* NativeLanguageResourceEnumerationCalls::allocate_sentinel_004c3020(){return allocate_native_render_alias_sentinel_004c3020();}
void NativeLanguageResourceEnumerationCalls::normalize_00bee690(void* p,NativeStringRawPoolContext& c){normalize_native_resource_path_header_00bee690(p,c);}
void NativeLanguageResourceEnumerationCalls::enumerate_00bdd990(void* p,const void* d,const void* e,U f,void* out,NativeVfsEnumerationContext& c){enumerate_native_vfs_resources_00bdd990(p,d,e,f,out,c);}
NativeLanguageResourceEnumerationOperation::~NativeLanguageResourceEnumerationOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeLanguageResourceEnumerationOperation::acknowledge_diagnostic_cleanup() noexcept{phase=Phase::diagnostic_retired;}
void* enumerate_native_language_resources_00886280(void* manager,void* output,const void* directory,const void* extension,U flags,NativeStringRawPoolContext& strings,NativeVfsEnumerationContext& enumeration,NativeLanguageResourceEnumerationCalls& calls,NativeLanguageResourceEnumerationOperation& op) {
    if(op.phase!=Phase::fresh)throw std::logic_error("native language enumeration operation cannot replay");
    op.phase=Phase::running;op.output=output;op.unwind_state=0;
    try{
        op.native_site=0x8862ae;void* const head=calls.allocate_sentinel_004c3020();
        put(output,4,reinterpret_cast<U>(head));put(output,8,0);
        put(op.directory,0,0);put(op.directory,4,0);
        if(op.directory!=directory) {
            const U length=get(directory);op.native_site=0x8862e2;
            resize_native_string_header_0041dd40(op.directory,strings,length,true);
            if(get(directory)!=0){const U count=get(op.directory);const void* const from=get<void*>(directory,4);void* const to=get<void*>(op.directory,4);std::memcpy(to,from,count);}
        }
        op.unwind_state=1;op.native_site=0x88630d;calls.normalize_00bee690(op.directory,strings);
        op.native_site=0x886324;calls.enumerate_00bdd990(manager,op.directory,extension,flags,output,enumeration);
        op.unwind_state=0;op.native_site=0x886340;destroy_native_string_header_0041dd20(op.directory,strings);
        op.phase=Phase::complete;return output;
    }catch(...){op.phase=Phase::failed;throw;}
}
} // namespace bsp
