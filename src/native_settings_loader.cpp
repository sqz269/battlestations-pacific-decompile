#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "bsp/native_settings_loader.hpp"
#include "bsp/entity_identity.hpp"
#include "bsp/native_material_compiler_providers.hpp"
#include "bsp/native_memory_stream.hpp"
#include "bsp/native_renderer_resolution_enumeration.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdio>
#include <cstring>
#include <exception>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native settings loader requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;using C=NativeSettingsLoaderContext;using Op=NativeSettingsLoaderOperation;using Phase=Op::Phase;
void* at(const void* p,U n=0) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
template<class T=U>T get(const void* p,U n=0) noexcept{return *static_cast<const volatile T*>(at(p,n));}
template<class T=U>void put(void* p,U n,T v) noexcept{*static_cast<volatile T*>(at(p,n))=v;}
I signed_bits(U n) noexcept{I v;std::memcpy(&v,&n,4);return v;}
void select_language(C& c,Op& op,const char* name,U site){op.native_site=site;select_native_settings_language_008d56c0(op.owner,name,c.language,op.language.emplace());}
void parse_file(C& c,Op& op) {
    auto& tokenizer=c.values.tokenizer;auto& memory=tokenizer.memory;void* const owner=op.owner;
    op.native_site=0x8d821e;c.calls.seek_00bfb72b(op.file,0,2);
    op.native_site=0x8d8224;op.file_length=c.calls.tell_00bfb636(op.file);const I length=op.file_length;
    op.native_site=0x8d8230;c.calls.seek_00bfb72b(op.file,0,0);
    op.native_site=0x8d8237;op.backing=c.calls.allocate_backing_00bf681b();op.backing_live=op.backing!=nullptr;
    if(op.backing){op.native_site=0x8d8252;construct_native_memory_backing_008d43c0(op.backing,length,memory);}
    op.native_site=0x8d826d;c.calls.read_00bfb483(get<void*>(op.backing,8),1,static_cast<U>(length),op.file);
    op.native_site=0x8d8273;c.text.path.calls.close_00bf8081(op.file);op.file_live=false;
    op.native_site=0x8d827d;op.stream=create_native_memory_stream_from_backing_00bef6d0(op.backing,memory);op.stream_live=op.stream!=nullptr;
    op.native_site=0x8d828b;construct_native_scene_tokenizer_008d9f20(op.tokenizer.data(),op.stream,nullptr,tokenizer,op.tokenizer_operation.emplace());
    op.tokenizer_live=true;op.unwind_state=2;void* const scan=op.tokenizer.data();
    for(;;) {
        op.native_site=0x8d82a4;peek_native_scene_token_008d8a70(scan,tokenizer);
        if(get<std::uint8_t>(scan,0x80a))break;
        if(std::strlen(static_cast<const char*>(at(scan,5)))==0&&get<std::uint8_t>(scan,4)==0)break;
        bool matched=false;
        constexpr U peek_sites[]{0x8d82db,0x8d8311,0x8d834a,0x8d8383,0x8d83bc};
        constexpr U compare_sites[]{0x8d82e7,0x8d831d,0x8d8356,0x8d838f,0x8d83c8};
        constexpr U consume_sites[]{0x8d82f4,0x8d832a,0x8d8363,0x8d839c,0x8d83d9};
        constexpr U int_sites[]{0,0x8d8338,0x8d8371,0x8d83aa,0x8d83e7};
        constexpr U byte_offsets[]{0,0x1e,0x1d,0x1c};
        for(U k=0;k<5;++k) {
            op.native_site=peek_sites[k];const char* const token=peek_native_scene_token_008d8a70(scan,tokenizer);
            op.native_site=compare_sites[k];if(c.calls.compare_00438e10(token,c.keywords[k])!=0)continue;
            op.native_site=consume_sites[k];consume_native_scene_token_008d8960(scan);matched=true;
            if(k==0){op.native_site=0x8d8302;const char* const name=read_native_scene_nonempty_string_008d99f0(scan,&op.ignored_value_success,c.values);select_language(c,op,name,0x8d830a);}
            else {
                op.native_site=int_sites[k];const I n=read_native_scene_int_008d9ad0(scan,&op.ignored_value_success,c.values);
                if(k<4)put<std::uint8_t>(owner,byte_offsets[k],n!=0?1u:0u);
                else {
                    put(owner,0x14,static_cast<U>(n));op.native_site=0x8d83f8;
                    const I height=read_native_scene_int_008d9ad0(scan,&op.ignored_value_success,c.values);put(owner,0x18,static_cast<U>(height));
                    U pair[]{get(owner,0x14),static_cast<U>(height)};op.native_site=0x8d8415;
                    if(find_native_resolution_pair_008d46c0(c.actual_resolution_header_00f8895c,0,pair)==-1){put(owner,0x14,640u);put(owner,0x18,480u);}
                    void* const h=c.actual_resolution_header_00f8895c;void* table=get<void*>(h);U index=0;
                    while(signed_bits(index)<signed_bits(get(h,4))) {
                        void* const row=at(table,index*8);
                        if(get(row)==get(owner,0x14)&&get(row,4)==get(owner,0x18)){put(owner,0x78,index);table=get<void*>(h);}
                        ++index;
                    }
                }
            }
            break;
        }
        if(matched)continue;
        constexpr U match_sites[]{0x8d8470,0x8d8494,0x8d84bf,0x8d84e6,0x8d850a,0x8d8535,0x8d8562,0x8d8586,0x8d85ae,0x8d85d2,0x8d85e8};
        constexpr U value_sites[]{0x8d8482,0x8d84a6,0x8d84d4,0x8d84f8,0x8d851c,0x8d8547,0x8d8574,0x8d8598,0x8d85c0,0,0x8d85fa};
        constexpr U offsets[]{0x60,0x88,0x58,0x74,0x86,0x84,0x6c,0x68,0x54,0,0x94};
        for(U k=0;k<11;++k) {
            op.native_site=match_sites[k];if(!match_native_scene_keyword_00467cc0(scan,c.keywords[k+5],tokenizer,c.calls))continue;
            matched=true;if(k==9)break; // SoundEnabled never reads its value.
            if(k==2)put(owner,0x5c,0u);
            op.native_site=value_sites[k];
            if(k==1||k==2||k==7||k==8)put(owner,offsets[k],static_cast<U>(read_native_scene_int_008d9ad0(scan,&op.ignored_value_success,c.values)));
            else {
                const auto value=static_cast<std::uint8_t>(read_native_scene_bool_008d9a80(scan,&op.ignored_value_success,c.values));
                put<std::uint8_t>(owner,offsets[k],value);if(k==5)put<std::uint8_t>(owner,0x85,value);
            }
            break;
        }
        if(matched)continue;
        op.native_site=0x8d860a;const char* const token=peek_native_scene_token_008d8a70(scan,tokenizer);
        op.native_site=0x8d8615;c.calls.unknown_token_004254b0(c.unknown_format_00d15e60,token);
        op.native_site=0x8d8621;consume_native_scene_token_008d8960(scan);
    }
    const auto decrement=c.decrement_import_00ce2220;
    op.native_site=0x8d8635;
    if(decrement(static_cast<volatile I*>(at(op.backing,4)))==0){op.native_site=0x8d8641;c.calls.memory_zero_reference_slot0(op.backing,memory);}
    op.backing_live=false;
    if(op.stream){op.native_site=0x8d864b;if(decrement(static_cast<volatile I*>(at(op.stream,4)))==0){op.native_site=0x8d8658;c.calls.memory_zero_reference_slot0(op.stream,memory);}}
    op.stream_live=false;op.unwind_state=0;op.native_site=0x8d8666;
    destroy_native_scene_tokenizer_008d9c30(scan,tokenizer,op.tokenizer_operation.emplace());op.tokenizer_live=false;
}
void missing_file(C& c,Op& op) {
    void* const owner=op.owner;put<std::uint8_t>(owner,0x1e,1);
    op.native_site=0x8d867a;void* const window=c.calls.desktop_window_00ce2360();
    op.native_site=0x8d8681;c.calls.window_rect_00ce235c(window,op.desktop_rectangle.data());
    const U width=op.desktop_rectangle[2]-op.desktop_rectangle[0],height=op.desktop_rectangle[3]-op.desktop_rectangle[1];
    put(owner,0x14,width);put(owner,0x18,height);op.native_site=0x8d86a4;
    c.calls.desktop_size_004254b0(c.desktop_format_00d15e4c,signed_bits(width),signed_bits(height));
    void* const h=c.actual_resolution_header_00f8895c;const U count=get(h,4);
    if(signed_bits(count)>0) {
        const U current_width=get(owner,0x14);void* const table=get<void*>(h);
        for(U index=0;signed_bits(index)<signed_bits(count);++index)if(get(table,index*8)==current_width&&get(table,index*8+4)==get(owner,0x18)){put(owner,0x78,index);break;}
    }
    op.native_site=0x8d86f1;
    if(c.calls.registry_open_00ce2010(0x80000002,c.registry_path_00d15e24,0,0x20019,&op.registry_key)==0) {
        op.registry_live=true;op.registry_bytes=0x400;op.native_site=0x8d8725;
        if(c.calls.registry_query_00ce200c(op.registry_key,c.registry_value_00d15e18,&op.registry_type,op.registry_buffer.data(),&op.registry_bytes)==0&&op.registry_type==4) {
            U language=0;switch(get(op.registry_buffer.data())){case 0x407:language=1;break;case 0x40a:language=2;break;case 0x40c:language=3;break;case 0x410:language=4;break;default:break;}
            select_language(c,op,c.registry_languages[language],0x8d8778);
        }
        op.native_site=0x8d8782;c.calls.registry_close_00ce2008(op.registry_key);op.registry_live=false;
    }
    op.native_site=0x8d878a;write_native_settings_text_008d6170(owner,c.text,op.writer.emplace());
}
}
int NativeSettingsLoaderCalls::seek_00bfb72b(void* f,I n,int origin){return std::fseek(static_cast<FILE*>(f),n,origin);}
I NativeSettingsLoaderCalls::tell_00bfb636(void* f){return static_cast<I>(std::ftell(static_cast<FILE*>(f)));}
U NativeSettingsLoaderCalls::read_00bfb483(void* p,U size,U count,void* f){return static_cast<U>(std::fread(p,size,count,static_cast<FILE*>(f)));}
void* NativeSettingsLoaderCalls::allocate_backing_00bf681b(){return singleton_lifetime_allocate({SingletonAllocationKind::object,0x10,0x10});}
int NativeSettingsLoaderCalls::compare_00438e10(const char* a,const char* b){return compare_insensitive_00438e10(a,b);}
void NativeSettingsLoaderCalls::unknown_token_004254b0(const char*,const char*){}
void NativeSettingsLoaderCalls::desktop_size_004254b0(const char*,I,I){}
void* NativeSettingsLoaderCalls::desktop_window_00ce2360(){return GetDesktopWindow();}
int NativeSettingsLoaderCalls::window_rect_00ce235c(void* w,void* r){return GetWindowRect(static_cast<HWND>(w),static_cast<RECT*>(r));}
I NativeSettingsLoaderCalls::registry_open_00ce2010(U root,const char* path,U options,U access,U* key){return static_cast<I>(RegOpenKeyExA(reinterpret_cast<HKEY>(root),path,options,access,reinterpret_cast<HKEY*>(key)));}
I NativeSettingsLoaderCalls::registry_query_00ce200c(U key,const char* value,U* type,void* data,U* size){return static_cast<I>(RegQueryValueExA(reinterpret_cast<HKEY>(key),value,nullptr,reinterpret_cast<DWORD*>(type),static_cast<BYTE*>(data),reinterpret_cast<DWORD*>(size)));}
I NativeSettingsLoaderCalls::registry_close_00ce2008(U key){return static_cast<I>(RegCloseKey(reinterpret_cast<HKEY>(key)));}
const void* NativeSettingsLoaderCalls::renderer_capabilities_slot104(void* renderer,const volatile U* profile) {
    if(get(renderer)!=0xd5f0a8||!profile||profile[0x104/4]!=0xb1ff50)throw std::logic_error("settings require the recovered renderer capability slot");
    return get_native_compiler_renderer_capabilities_00b1ff50(renderer);
}
void NativeSettingsLoaderCalls::memory_zero_reference_slot0(void* p,NativeRetainedMemoryOwnerContext& c){dispatch_native_memory_owner_zero_reference(p,c);}
bool match_native_scene_keyword_00467cc0(void* scanner,const char* keyword,NativeSceneTokenizerContext& c,NativeSettingsLoaderCalls& calls) {
    const char* const token=peek_native_scene_token_008d8a70(scanner,c);
    if(calls.compare_00438e10(token,keyword)!=0)return false;
    consume_native_scene_token_008d8960(scanner);return true;
}
Op::NativeSettingsLoaderOperation(const NativeSettingsLoaderPreimages& p) noexcept:tokenizer(p.tokenizer),desktop_rectangle(p.desktop_rectangle),registry_buffer(p.registry_buffer),registry_key(p.registry_key),registry_type(p.registry_type){}
Op::~NativeSettingsLoaderOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void Op::acknowledge_diagnostic_cleanup() noexcept {
    if(catalog)catalog->acknowledge_diagnostic_cleanup();if(path)path->acknowledge_diagnostic_cleanup();
    if(tokenizer_operation)tokenizer_operation->acknowledge_diagnostic_cleanup();if(language)language->acknowledge_diagnostic_cleanup();if(writer)writer->acknowledge_diagnostic_cleanup();phase=Phase::diagnostic_retired;
}
void load_native_game_settings_008d8190(void* owner,C& c,Op& op) {
    if(op.phase!=Phase::fresh)throw std::logic_error("native settings loader operation cannot replay");
    op.phase=Phase::running;op.owner=owner;
    try {
        op.native_site=0x8d81ae;build_native_language_catalog_008d7bc0(c.catalog,op.catalog.emplace(c.preimages.catalog_list_word));
        op.native_site=0x8d81b9;void* const resolutions=native_renderer_resolution_header_00b1fff0(c.actual_renderer_00f8d394);
        op.native_site=0x8d81c4;assign_native_settings_pairs_008d4ea0(c.actual_resolution_header_00f8895c,resolutions,c.choices);
        op.native_site=0x8d81cf;put(owner,0x88,static_cast<U>(native_renderer_shader_ceiling_00b200b0(c.actual_renderer_00f8d394)));
        op.native_site=0x8d81e1;build_native_settings_path_008d5150(op.path_header,c.text.path,op.path.emplace(c.text.personal_buffer_preimage));op.path_live=true;op.unwind_state=0;
        const char* path=get<const char*>(op.path_header,4);if(!path)path=c.text.path.empty_00f88a3c;
        op.native_site=0x8d8206;op.file=c.text.path.calls.open_00bf838e(path,c.read_mode_00d15f28);op.file_live=op.file!=nullptr;
        if(op.file)parse_file(c,op);else missing_file(c,op);
        if(signed_bits(get(owner,0x88))<1){op.native_site=0x8d879e;put(owner,0x88,static_cast<U>(native_renderer_shader_ceiling_00b200b0(c.actual_renderer_00f8d394)));}
        op.native_site=0x8d87af;const I ceiling=native_renderer_shader_ceiling_00b200b0(c.actual_renderer_00f8d394);
        const I selected=signed_bits(get(owner,0x88));put(owner,0x88,static_cast<U>(selected>ceiling?ceiling:selected));
        op.native_site=0x8d87cd;select_native_renderer_shader_00b200c0(c.actual_renderer_00f8d394,0,signed_bits(get(owner,0x88)));
        op.native_site=0x8d87e0;const void* const capabilities=c.calls.renderer_capabilities_slot104(c.actual_renderer_00f8d394,c.actual_renderer_profile_00d5f0a8);
        if(get(capabilities,0x28)<0x200){put<std::uint8_t>(owner,0x84,0);put<std::uint8_t>(owner,0x85,0);put(owner,0x90,0u);}
        const bool special_format=get<std::uint8_t>(owner,0x8c)!=0;void* const renderer=c.actual_renderer_00f8d394;const U format=special_format?0x71u:0x15u;
        op.native_site=0x8d8818;rebuild_native_renderer_antialias_00b295c0(renderer,format,c.choices);
        op.native_site=0x8d8823;void* const samples=native_renderer_antialias_header_00b20000(c.actual_renderer_00f8d394);
        op.native_site=0x8d882e;void* const h=c.actual_antialias_header_00f88968;assign_native_settings_dwords_008d4df0(h,samples,c.choices);
        U count=get(h,4);
        if(count) {
            const U selected_sample=get(owner,0x58);U index=0;
            do{if(get(get<void*>(h),index*4)==selected_sample){put(owner,0x5c,index);count=get(h,4);}++index;}while(index<count);
        }
        U index=get(owner,0x5c);if(signed_bits(index)>=signed_bits(count)){index=count-1;put(owner,0x5c,index);}
        put(owner,0x58,get(get<void*>(h),index*4));
        op.unwind_state=-1;
        if(void* const data=get<void*>(op.path_header,4)) {
            const U n=get(op.path_header)+1;auto& raw=c.text.path.strings;op.native_site=0x8d8898;
            auto* const pool=native_string_pool_get_or_create_00419cc0(raw.actual_published_01090aa8,raw.actual_manager_publication_01090aa0);
            op.native_site=0x8d889f;return_native_string_pool_00bd1510(pool,data,n,raw.actual_small_returns_disabled_01090aa4);
        }
        op.path_live=false;op.phase=Phase::complete;
    }catch(...){op.phase=Phase::failed;throw;}
}
} // namespace bsp
