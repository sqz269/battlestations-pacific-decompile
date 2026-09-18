#include "bsp/native_language_catalog_producer.hpp"
#include "bsp/entity_identity.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_render_resource_record.hpp"
#include "bsp/native_vfs_package_scan.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native language catalog producer requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;using Op=NativeLanguageCatalogProducerOperation;using C=NativeLanguageCatalogProducerContext;using Phase=Op::Phase;
void* at(const void* p,U n=0) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
template<class T=U>T get(const void* p,U n=0) noexcept{T v;std::memcpy(&v,at(p,n),sizeof v);return v;}
void put(void* p,U n,U v) noexcept{std::memcpy(at(p,n),&v,4);}
NativeStringPoolStorage* pool(C& c){return native_string_pool_get_or_create_00419cc0(c.strings.actual_published_01090aa8,c.strings.actual_manager_publication_01090aa0);}
void release(C& c,Op& op,void* data,U n,U get_site,U return_site) {
    op.native_site=get_site;auto* const p=pool(c);op.native_site=return_site;
    return_native_string_pool_00bd1510(p,data,n,c.strings.actual_small_returns_disabled_01090aa4);
}
void destroy_header(C& c,Op& op,void* p,U get_site,U return_site) {
    if(void* const data=get<void*>(p,4)){const U n=get(p)+1;release(c,op,data,n,get_site,return_site);}
}
void assign(C& c,Op& op,U member,const char* value) {
    constexpr U zero_get[]{0x8d7e3c,0x8d7f0c,0x8d7fdc,0x8d80ac};
    constexpr U zero_return[]{0x8d7e43,0x8d7f13,0x8d7fe3,0x8d80b3};
    constexpr U allocate_get[]{0x8d7e55,0x8d7f25,0x8d7ff5,0x8d80c5};
    constexpr U allocate[]{0x8d7e62,0x8d7f32,0x8d8002,0x8d80d2};
    constexpr U old_get[]{0x8d7e7c,0x8d7f4c,0x8d801c,0x8d80ec};
    constexpr U old_return[]{0x8d7e83,0x8d7f53,0x8d8023,0x8d80f3};
    void* const p=at(op.row,member*8);op.current_member=p;
    const U length=value?static_cast<U>(std::strlen(value)):0;
    const U old_length=get(p);
    if(length!=old_length) {
        if(length==0) {
            if(void* const old=get<void*>(p,4))release(c,op,old,old_length+1,zero_get[member],zero_return[member]);
            put(p,4,0);put(p,0,0);return;
        }
        op.native_site=allocate_get[member];auto* const storage=pool(c);
        op.native_site=allocate[member];void* const data=allocate_native_string_pool_00bd1120(storage,length+1);
        op.pending_string_data=data;op.pending_string_bytes=length+1;
        if(void* const old=get<void*>(p,4)){const U old_bytes=get(p)+1;release(c,op,old,old_bytes,old_get[member],old_return[member]);}
        put(p,0,length);put(p,4,reinterpret_cast<U>(data));op.pending_string_data=nullptr;op.pending_string_bytes=0;
        *static_cast<char*>(at(data,length))=0;
    }
    const U current_length=get(p);void* const data=get<void*>(p,4);
    if(data){op.native_site=0x8d8116;std::memcpy(data,value,current_length);}
}
bool accepted_name(C& c,const void* scanner) {
    const char* name=get<const char*>(scanner,0x820);const char* prefix=c.allowed_prefixes[0];
    if(name) {
        while(*name&&*prefix&&*name==*prefix){++name;++prefix;}
        if(*prefix==0)return true;
    }
    for(U i=1;i<6;++i)if(static_cast<std::uint8_t>(native_language_prefix_result_00553c80(at(scanner,0x81c),c.allowed_prefixes[i],0))!=0)return true;
    return false;
}
}
void* NativeLanguageCatalogProducerCalls::allocate_scanner_00bf681b(){return singleton_lifetime_allocate({SingletonAllocationKind::object,0x828,0x828});}
void NativeLanguageCatalogProducerCalls::free_00bf65ac(void* p){singleton_lifetime_free(p);}
int NativeLanguageCatalogProducerCalls::compare_00438e10(const char* a,const char* b){return compare_insensitive_00438e10(a,b);}
Op::NativeLanguageCatalogProducerOperation(U preimage) noexcept{put(list,0,preimage);}
Op::~NativeLanguageCatalogProducerOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void Op::acknowledge_diagnostic_cleanup() noexcept {
    if(enumeration)enumeration->acknowledge_diagnostic_cleanup();
    if(scanner_operation)scanner_operation->acknowledge_diagnostic_cleanup();
    if(hints)hints->acknowledge_diagnostic_cleanup();
    if(storage)storage->acknowledge_diagnostic_cleanup();
    phase=Phase::diagnostic_retired;
}
void build_native_language_catalog_008d7bc0(C& c,Op& op) {
    if(op.phase!=Phase::fresh)throw std::logic_error("native language catalog producer operation cannot replay");
    op.phase=Phase::running;
    try {
        if(get(c.actual_catalog_00f88974,4)!=0){op.phase=Phase::complete;return;}
        op.native_site=0x8d7bf3;construct_native_string_header_0041e870(op.extension,c.strings,c.extension_00d15e10);
        op.extension_live=true;op.unwind_state=0;
        op.native_site=0x8d7c05;construct_native_string_header_0041e870(op.directory,c.strings,c.directory_00d15e08);
        op.directory_live=true;op.unwind_state=1;op.native_site=0x8d7c25;
        enumerate_native_language_resources_00886280(c.scanner.actual_vfs_0109ceec,op.list,op.directory,op.extension,0,c.strings,c.enumeration,c.enumeration_calls,op.enumeration.emplace());
        op.list_live=true;op.unwind_state=3;destroy_header(c,op,op.directory,0x8d7c42,0x8d7c49);op.directory_live=false;
        op.unwind_state=4;destroy_header(c,op,op.extension,0x8d7c66,0x8d7c6d);op.extension_live=false;
        while(get(op.list,8)!=0) {
            op.native_site=0x8d7c81;void* const allocation=c.calls.allocate_scanner_00bf681b();
            op.scanner=allocation;op.scanner_allocation_live=allocation!=nullptr;op.scanner_constructed=false;op.unwind_state=5;
            if(allocation) {
                op.native_site=0x8d7ca7;pop_native_vfs_package_name_00557a90(op.list,op.filename_argument,c.actual_strings,c.invalid_parameters);op.filename_argument_live=true;
                op.native_site=0x8d7cae;op.scanner=construct_native_stream_text_scanner_00bef2e0(allocation,op.filename_argument,nullptr,c.scanner,op.scanner_operation.emplace());
                op.filename_argument_live=false;op.scanner_constructed=true;
            }
            op.unwind_state=4;op.scanner_holder=op.scanner;op.native_site=0x8d7cca;
            void* const hints=get_native_profile_hints_owner_004c1e90(c.hints,op.hints.emplace());
            ++op.descriptors;
            if(get(hints,8)==0&&!accepted_name(c,op.scanner)) {
                op.native_site=0x8d7d6e;release_native_language_scanner_008d47f0(&op.scanner_holder,c.scanner,op.storage.emplace());
                op.scanner_allocation_live=false;op.scanner_constructed=false;continue;
            }
            for(U n=0;n<32;n+=4)put(op.row,n,0);
            op.row_live=true;op.unwind_state=6;
            void* working=nullptr;
            for(;;) {
                working=op.scanner;op.captured_scanner=working;op.native_site=0x8d7da6;
                peek_native_stream_text_token_00bee8e0(working,c.scanner,c.scanner_stack_preimages);
                if(get<std::uint8_t>(working,0x806)!=0)break;
                if(std::strlen(static_cast<const char*>(at(working,1)))==0&&get<std::uint8_t>(working)==0)break;
                constexpr U peek_sites[]{0x8d7dd7,0x8d7ea2,0x8d7f72,0x8d8042};
                constexpr U compare_sites[]{0x8d7de3,0x8d7eae,0x8d7f7e,0x8d804e};
                constexpr U accept_sites[]{0x8d7df6,0x8d7ec1,0x8d7f91,0x8d8061};
                constexpr U read_sites[]{0x8d7e02,0x8d7ecd,0x8d7f9d,0x8d806d};
                for(U member=0;member<4;++member) {
                    op.native_site=peek_sites[member];const char* const key=peek_native_stream_text_token_00bee8e0(working,c.scanner,c.scanner_stack_preimages);
                    op.native_site=compare_sites[member];
                    if(c.calls.compare_00438e10(key,c.keywords[member])!=0)continue;
                    working=op.scanner;op.native_site=accept_sites[member];accept_native_stream_text_token_00bee800(working);
                    op.native_site=read_sites[member];const char* const value=read_native_stream_text_string_00bef020(working,&op.ignored_string_success,c.scanner,c.scanner_stack_preimages);
                    assign(c,op,member,value);break;
                }
            }
            op.native_site=0x8d812d;append_native_language_catalog_008d6af0(c.actual_catalog_00f88974,op.row,c.strings,c.catalog_calls,op.storage.emplace());++op.appended_rows;
            op.native_site=0x8d8134;destroy_native_stream_text_scanner_00bef220(working,c.scanner,op.scanner_operation.emplace());op.scanner_constructed=false;
            op.native_site=0x8d813a;c.calls.free_00bf65ac(working);op.scanner_allocation_live=false;
            op.unwind_state=4;op.native_site=0x8d814b;destroy_native_language_row_008d4f50(op.row,c.strings,op.storage.emplace());op.row_live=false;
        }
        op.unwind_state=-1;op.native_site=0x8d8166;clear_native_render_resource_aliases_004d05e0(op.list,c.strings);
        void* const sentinel=get<void*>(op.list,4);op.native_site=0x8d8170;c.calls.free_00bf65ac(sentinel);op.list_live=false;
        op.phase=Phase::complete;
    }catch(...){op.phase=Phase::failed;throw;}
}
} // namespace bsp
