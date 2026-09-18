#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>
#include "bsp/native_settings_text.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_open_logging.hpp"
#include "bsp/sound_stream_runtime.hpp"
#include <cstdio>
#include <cstring>
#include <exception>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native settings text requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using Phase=NativeSettingsTextPhase;
void* at(const void* p,U n=0) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
template<class T=U>T get(const void* p,U n=0) noexcept{T v;std::memcpy(&v,at(p,n),sizeof v);return v;}
void put(void* p,U n,U v) noexcept{std::memcpy(at(p,n),&v,4);}
void begin(Phase& p){if(p!=Phase::fresh)throw std::logic_error("native settings text operation cannot replay");p=Phase::running;}
void check_complete(Phase p) noexcept{if(p==Phase::running||p==Phase::failed)std::terminate();}
void release(NativeStringRawPoolContext& c,void* data,U bytes,U get_site,U return_site,U& site) {
    site=get_site;auto* const pool=native_string_pool_get_or_create_00419cc0(c.actual_published_01090aa8,c.actual_manager_publication_01090aa0);
    site=return_site;return_native_string_pool_00bd1510(pool,data,bytes,c.actual_small_returns_disabled_01090aa4);
}
}
int NativeSettingsTextCalls::personal_folder_00ce22f0(char* p){return SHGetSpecialFolderPathA(nullptr,p,5,TRUE);}
int NativeSettingsTextCalls::create_directory_00ce227c(const char* p){return CreateDirectoryA(p,nullptr);}
#pragma warning(push)
#pragma warning(disable:4996) // Preserve the native fopen result contract.
void* NativeSettingsTextCalls::open_00bf838e(const char* p,const char* m){return std::fopen(p,m);}
#pragma warning(pop)
U NativeSettingsTextCalls::write_00bfb08b(const void* p,U s,U n,void* f){return static_cast<U>(std::fwrite(p,s,n,static_cast<FILE*>(f)));}
int NativeSettingsTextCalls::close_00bf8081(void* f){return std::fclose(static_cast<FILE*>(f));}
NativeSettingsPathOperation::~NativeSettingsPathOperation(){check_complete(phase);}
void NativeSettingsPathOperation::acknowledge_diagnostic_cleanup() noexcept{phase=Phase::diagnostic_retired;}
NativeSettingsBuilderCopyOperation::~NativeSettingsBuilderCopyOperation(){check_complete(phase);}
void NativeSettingsBuilderCopyOperation::acknowledge_diagnostic_cleanup() noexcept{phase=Phase::diagnostic_retired;}
NativeSettingsTextOperation::~NativeSettingsTextOperation(){check_complete(phase);}
void NativeSettingsTextOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(copy_operation)copy_operation->acknowledge_diagnostic_cleanup();
    if(path_operation)path_operation->acknowledge_diagnostic_cleanup();
    phase=Phase::diagnostic_retired;
}
void* build_native_settings_path_008d5150(void* out,NativeSettingsPathContext& c,NativeSettingsPathOperation& op) {
    begin(op.phase);op.output=out;
    try {
        op.native_site=0x8d5186;c.calls.personal_folder_00ce22f0(op.personal_buffer.data());
        put(op.directory,0,0);put(op.directory,4,0);op.native_site=0x8d519c;
        resize_native_string_header_0041dd40(op.directory,c.strings,23,true);
        if(void* const data=get<void*>(op.directory,4)){const U n=get(op.directory)+1;op.native_site=0x8d51b7;std::memcpy(data,c.directory_suffix_00d15af0,n);}
        op.directory_live=true;op.unwind_state=1;put(op.temporary,0,0);put(op.temporary,4,0);
        const U root_length=static_cast<U>(std::strlen(op.personal_buffer.data()));op.native_site=0x8d51f2;
        resize_native_string_header_0041dd40(op.temporary,c.strings,root_length,true);
        op.captured_temporary=get<void*>(op.temporary,4);op.captured_length=get(op.temporary);op.temporary_live=true;
        if(op.captured_temporary){op.native_site=0x8d520d;std::memcpy(op.captured_temporary,op.personal_buffer.data(),op.captured_length+1);}
        op.unwind_state=2;op.native_site=0x8d522e;
        concatenate_native_string_headers_004261a0(op.temporary,out,op.directory,c.strings);op.output_constructed=true;
        op.unwind_state=1;
        if(op.captured_temporary)release(c.strings,op.captured_temporary,op.captured_length+1,0x8d524e,0x8d5255,op.native_site);
        op.temporary_live=false;op.unwind_state=0;
        if(void* const data=get<void*>(op.directory,4)){const U n=get(op.directory)+1;release(c.strings,data,n,0x8d5275,0x8d527c,op.native_site);}
        op.directory_live=false;
        const char* path=get<const char*>(out,4);if(!path)path=c.empty_00f88a3c;
        op.native_site=0x8d528f;c.calls.create_directory_00ce227c(path);
        put(op.temporary,0,0);put(op.temporary,4,0);op.native_site=0x8d52a5;
        resize_native_string_header_0041dd40(op.temporary,c.strings,12,true);
        op.captured_temporary=get<void*>(op.temporary,4);op.captured_length=get(op.temporary);op.temporary_live=true;
        if(op.captured_temporary){op.native_site=0x8d52c0;std::memcpy(op.captured_temporary,c.options_suffix_00d15ae0,op.captured_length+1);}
        op.unwind_state=3;
        if(op.captured_length!=0) {
            const U old_length=get(out);op.native_site=0x8d52e2;
            resize_native_string_header_0041dd40(out,c.strings,old_length+op.captured_length,true);
            void* const to=at(get<void*>(out,4),old_length);op.native_site=0x8d52ef;
            std::memcpy(to,op.captured_temporary,op.captured_length);
        }
        op.unwind_state=0;
        if(op.captured_temporary)release(c.strings,op.captured_temporary,op.captured_length+1,0x8d530c,0x8d5313,op.native_site);
        op.temporary_live=false;op.phase=Phase::complete;return out;
    }catch(...){op.phase=Phase::failed;throw;}
}
void* copy_native_settings_builder_008d5340(void* out,const void* source,NativeStringRawPoolContext& c,NativeSettingsBuilderCopyOperation& op) {
    begin(op.phase);op.output=out;op.source=source;
    try {
        constexpr U resize_sites[]{0x8d5373,0x8d53b3,0x8d53f5};constexpr U copy_sites[]{0x8d5387,0x8d53c9,0x8d540a};
        for(U i=0;i<3;++i) {
            void* const d=at(out,i*8);const void* const s=at(source,i*8);put(d,0,0);put(d,4,0);
            if(d!=s) {
                const U n=get(s);op.native_site=resize_sites[i];resize_native_string_header_0041dd40(d,c,n,true);
                if(get(s)!=0) {
                    const U count=get(d);void* to;const void* from;
                    if(i==0){to=get<void*>(d,4);from=get<const void*>(s,4);}else{from=get<const void*>(s,4);to=get<void*>(d,4);}
                    op.native_site=copy_sites[i];std::memcpy(to,from,count);
                }
            }
            op.completed_members=i+1;op.unwind_state=i==0?0:1;
        }
        op.phase=Phase::complete;return out;
    }catch(...){op.phase=Phase::failed;throw;}
}
void write_native_settings_text_008d6170(void* owner,NativeSettingsTextContext& c,NativeSettingsTextOperation& op) {
    begin(op.phase);op.owner=owner;
    try {
        op.native_site=0x8d618f;void* chain=construct_native_log_builder_00426500(op.builder,c.strings);op.builder_live=true;
        const U index=get(owner,4);const void* const catalog=c.actual_catalog_00f88974;
        const char* language=get<const char*>(at(catalog,index<<5),4);if(!language)language=c.path.empty_00f88a3c;
        op.captured_language=language;op.unwind_state=0;
        // These loads correspond to arguments pushed in reverse line order.
        auto& v=op.captured_values;
        v[14]=get<std::uint8_t>(owner,0x95);v[13]=get<std::uint8_t>(owner,0x94);v[12]=get<std::uint8_t>(owner,0x24);
        v[11]=get<I>(owner,0x54);v[10]=get<I>(owner,0x68);v[9]=get<std::uint8_t>(owner,0x6c);
        v[8]=get<std::uint8_t>(owner,0x84);v[7]=get<std::uint8_t>(owner,0x86);v[6]=get<std::uint8_t>(owner,0x74);
        v[5]=get<I>(owner,0x58);v[4]=get<I>(owner,0x88);v[3]=get<std::uint8_t>(owner,0x60);
        v[2]=get<I>(owner,0x18);v[1]=get<I>(owner,0x14);v[0]=get<std::uint8_t>(owner,0x1e);
        auto append_text=[&](const char* text){op.native_site=0x8d62a6+op.append_calls*7;chain=append_native_log_cstring_00bd1a60(chain,text,c.strings);++op.append_calls;};
        auto append_int=[&](I value){op.native_site=0x8d62a6+op.append_calls*7;chain=append_native_log_int_00bd1bb0(chain,value,c.strings,c.null_integer_format_01090ab4);++op.append_calls;};
        append_text(c.labels[0]);append_text(language);append_text(c.newline_00ce4390);
        append_text(c.labels[1]);append_int(v[0]);append_text(c.newline_00ce4390);
        append_text(c.labels[2]);append_int(v[1]);append_text(c.space_00ce3a90);append_int(v[2]);append_text(c.newline_00ce4390);
        for(U i=3;i<15;++i){append_text(c.labels[i]);append_int(v[i]);append_text(c.newline_00ce4390);}
        op.native_site=0x8d63f2;copy_native_settings_builder_008d5340(op.copy,chain,c.path.strings,op.copy_operation.emplace());op.copy_live=true;op.unwind_state=2;
        op.native_site=0x8d6400;destroy_native_log_builder_00425f80(op.builder,c.strings);op.builder_live=false;
        op.native_site=0x8d640c;build_native_settings_path_008d5150(op.path_header,c.path,op.path_operation.emplace(c.personal_buffer_preimage));op.path_live=true;
        const char* path=get<const char*>(op.path_header,4);if(!path)path=c.path.empty_00f88a3c;
        op.native_site=0x8d6424;void* const file=c.path.calls.open_00bf838e(path,c.write_mode_00d15b38);op.file=file;op.file_live=file!=nullptr;
        if(file) {
            const void* data=get<const void*>(op.copy,4);if(!data)data=c.path.empty_00f88a3c;
            const U n=get(op.copy);op.native_site=0x8d6448;c.path.calls.write_00bfb08b(data,1,n,file);
            op.native_site=0x8d644e;c.path.calls.close_00bf8081(file);op.file_live=false;
        }
        if(void* const data=get<void*>(op.path_header,4)){const U n=get(op.path_header)+1;release(c.path.strings,data,n,0x8d6469,0x8d6470,op.native_site);}
        op.path_live=false;op.unwind_state=-1;op.native_site=0x8d6481;destroy_native_log_builder_00425f80(op.copy,c.strings);op.copy_live=false;
        op.phase=Phase::complete;
    }catch(...){op.phase=Phase::failed;throw;}
}
} // namespace bsp
