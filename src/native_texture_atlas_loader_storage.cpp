#include "bsp/native_texture_atlas_loader_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_logical_texture_named_base.hpp"
#include "bsp/native_particle_resource_loading.hpp"
#include "bsp/native_particle_text_loader.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_render_resource_record.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_raw_assign_cstring.hpp"
#include "bsp/native_string_raw_cstring_exact.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_vfs_enumeration.hpp"
#include "bsp/native_vfs_package_scan.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw atlas loader requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word=std::uint32_t;
using Phase=NativeTextureAtlasLoaderPhase;
static_assert(sizeof(void*)==4);
static_assert(sizeof(NativeTextureAtlasSplitNameHomes)==0x30);
static_assert(offsetof(NativeTextureAtlasSplitNameHomes,candidate)==8);
static_assert(offsetof(NativeTextureAtlasSplitNameHomes,requested)==0x10);
static_assert(offsetof(NativeTextureAtlasSplitNameHomes,temporary)==0x18);
static_assert(offsetof(NativeTextureAtlasSplitNameHomes,joined)==0x20);
static_assert(offsetof(NativeTextureAtlasSplitNameHomes,stem)==0x28);
static_assert(sizeof(NativeTextureAtlasLoaderHomes)==0x2c);
static_assert(offsetof(NativeTextureAtlasLoaderHomes,list)==0x20);
Word word(const void* raw, Word byte_offset=0) noexcept {
    Word result;
    __asm {
        mov eax, raw
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* raw, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, raw
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
Word bits(const void* p) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
std::int32_t signed_word(Word value) noexcept {
    std::int32_t result;std::memcpy(&result,&value,4);return result;
}
void return_pair(Word captured_data, Word size, NativeStringRawPoolContext& strings) {
    auto* const pool=native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool,pointer(captured_data),size,
        strings.actual_small_returns_disabled_01090aa4);
}
// Only an ordinary Source composition of the native inline clear/copy. No
// numeric426060 call is invented: caller sites below retain original address.
void clone_inline(void* destination, const void* source, Word& preserve_home,
    NativeStringRawPoolContext& strings,const volatile Word& feature,
    Word& site,Word resize_site,Word copy_site) {
    const bool identical=destination==source;
    put(destination,0,0);put(destination,4,0);
    if(!identical) {
        const Word request=word(source);
        preserve_home=1;site=resize_site;
        resize_native_string_header_0041dd40_exact(destination,strings,request,
            &preserve_home,feature);
        if(word(source)!=0) {
            const Word count=word(destination);
            const void* const input=pointer(word(source,4));
            void* const output=pointer(word(destination,4));
            site=copy_site;
            copy_native_crt_bytes_00bf7680(output,input,count,feature);
        }
    }
}
}
NativeTextureAtlasLoaderContext::NativeTextureAtlasLoaderContext(
    const game::GameNativeReadOnlyData& data,NativeStringRawPoolContext& strings,
    const volatile Word& feature,game::GameNativeVfsRawServices& vfs,
    void* volatile& atlas_publication,const char* empty_name,
    NativeTextureAtlasParserContext& parser)
    :data_(data),strings_(strings),feature_(feature),vfs_(vfs),
      atlas_publication_(atlas_publication),empty_name_(empty_name),parser_(parser),
      extension_ats_(data.data_at(0x00d5d7cc,4)),
      directory_default_(data.data_at(0x00d5d7d0,3)),
      slash_(data.data_at(0x00ce7898,2)),backslash_(data.data_at(0x00ce7894,2)),
      underscore_(data.data_at(0x00ce7890,2)),
      extension_dot_ats_(data.data_at(0x00d5d23c,5)) {
    if(!same_native_string_storage_raw_pool_cells(vfs.strings,strings) ||
       &vfs.enumeration.strings!=&vfs.strings || !empty_name)
        throw std::logic_error("raw atlas loader requires one genuine VFS/raw pool and actual empty role");
}
std::uint8_t match_native_texture_atlas_split_name_00aef3c0(
    const void* requested,const void* candidate,NativeTextureAtlasSplitNameFrame& f,
    NativeTextureAtlasLoaderContext& c) {
    if(f.phase!=Phase::fresh)
        throw std::logic_error("raw atlas matcher Frame cannot replay");
    f.phase=Phase::entered;
    auto& n=f.native;
    try {
        n.auxiliary[0]=bits(candidate);                        // AEF3EC
        clone_inline(n.requested,requested,f.preserve_home,c.strings_,c.feature_,
            f.active_site,0x00aef403,0x00aef41a);
        f.native_state=0;f.active_site=0x00aef42a;
        lowercase_native_string_header_004bcc00(n.requested);
        put(n.candidate,0,0);put(n.candidate,4,0);               // AEF437/3B
        f.preserve_home=1;f.active_site=0x00aef43f;
        resize_native_string_header_0041dd40_exact(n.candidate,c.strings_,1,
            &f.preserve_home,c.feature_);
        f.captured_slash_data=word(n.candidate,4);              // AEF444
        f.captured_slash_length=word(n.candidate);              // AEF44A
        if(f.captured_slash_data) {
            f.active_site=0x00aef45a;
            copy_native_crt_bytes_00bf7680(pointer(f.captured_slash_data),c.slash_,
                f.captured_slash_length+1u,c.feature_);
        }
        f.native_state=1;                                     // AEF46A
        put(n.temporary,0,0);put(n.temporary,4,0);
        f.preserve_home=1;f.active_site=0x00aef477;
        resize_native_string_header_0041dd40_exact(n.temporary,c.strings_,1,
            &f.preserve_home,c.feature_);
        f.captured_backslash_data=word(n.temporary,4);          // AEF47C
        f.captured_backslash_length=word(n.temporary);          // AEF482
        if(f.captured_backslash_data) {
            f.active_site=0x00aef492;
            copy_native_crt_bytes_00bf7680(pointer(f.captured_backslash_data),c.backslash_,
                f.captured_backslash_length+1u,c.feature_);
        }
        f.first_replace_homes[2]=0x7fffffffu;
        f.first_replace_homes[1]=bits(n.candidate);
        f.first_replace_homes[0]=bits(n.temporary);
        f.native_state=2;f.active_site=0x00aef4b2;
        replace_native_atlas_string_substrings_004cad40_exact(n.requested,
            f.first_replace_homes,c.strings_,c.feature_,f.first_replace);
        f.native_state=1;
        if(f.captured_backslash_data) {
            f.active_site=0x00aef4c7;
            return_pair(f.captured_backslash_data,f.captured_backslash_length+1u,c.strings_);
        }
        f.native_state=0;
        if(f.captured_slash_data) {
            f.active_site=0x00aef4e3;
            return_pair(f.captured_slash_data,f.captured_slash_length+1u,c.strings_);
        }
        const void* const saved_candidate=pointer(n.auxiliary[0]);// AEF4EF
        clone_inline(n.candidate,saved_candidate,f.preserve_home,c.strings_,c.feature_,
            f.active_site,0x00aef50c,0x00aef524);
        f.native_state=3;f.active_site=0x00aef535;
        lowercase_native_string_header_004bcc00(n.candidate);
        put(n.temporary,0,0);put(n.temporary,4,0);
        f.preserve_home=1;f.active_site=0x00aef54c;
        resize_native_string_header_0041dd40_exact(n.temporary,c.strings_,1,
            &f.preserve_home,c.feature_);
        f.captured_slash_data=word(n.temporary,4);              // AEF551
        f.captured_slash_length=word(n.temporary);              // AEF557
        if(f.captured_slash_data) {
            f.active_site=0x00aef567;
            copy_native_crt_bytes_00bf7680(pointer(f.captured_slash_data),c.slash_,
                f.captured_slash_length+1u,c.feature_);
        }
        f.native_state=4;
        put(n.auxiliary,0,0);put(n.auxiliary,4,0);
        f.preserve_home=1;f.active_site=0x00aef584;
        resize_native_string_header_0041dd40_exact(n.auxiliary,c.strings_,1,
            &f.preserve_home,c.feature_);
        f.captured_backslash_data=word(n.auxiliary,4);           // AEF589
        f.captured_backslash_length=word(n.auxiliary);          // AEF58F
        if(f.captured_backslash_data) {
            f.active_site=0x00aef59f;
            copy_native_crt_bytes_00bf7680(pointer(f.captured_backslash_data),c.backslash_,
                f.captured_backslash_length+1u,c.feature_);
        }
        f.second_replace_homes[2]=0x7fffffffu;
        f.second_replace_homes[1]=bits(n.temporary);
        f.second_replace_homes[0]=bits(n.auxiliary);
        f.native_state=5;f.active_site=0x00aef5bf;
        replace_native_atlas_string_substrings_004cad40_exact(n.candidate,
            f.second_replace_homes,c.strings_,c.feature_,f.second_replace);
        f.native_state=4;
        if(f.captured_backslash_data) {
            f.active_site=0x00aef5d4;
            return_pair(f.captured_backslash_data,f.captured_backslash_length+1u,c.strings_);
        }
        f.native_state=3;
        if(f.captured_slash_data) {
            f.active_site=0x00aef5f0;
            return_pair(f.captured_slash_data,f.captured_slash_length+1u,c.strings_);
        }
        f.active_site=0x00aef605;
        if(equal_native_string_headers_00435c40(n.candidate,n.requested)) {
            Word returned=word(n.candidate,4);f.native_state=0; // AEF60E/14
            if(returned) {
                const Word size=word(n.candidate)+1u;
                f.active_site=0x00aef626;return_pair(returned,size,c.strings_);
            }
            returned=word(n.requested,4);f.native_state=-1;
            if(returned) {
                const Word size=word(n.requested)+1u;
                f.active_site=0x00aef64d;return_pair(returned,size,c.strings_);
            }
            f.normal_result=1;f.phase=Phase::complete;return 1;
        }
        f.requested_length_register=word(n.requested);          // AEF66E
        f.candidate_length_register=word(n.candidate);          // AEF672
        bool early_reject=f.candidate_length_register<f.requested_length_register;
        if(!early_reject) {
            f.active_site=0x00aef6db;
            construct_native_string_header_0041e870_exact(n.temporary,c.slash_,c.strings_,c.feature_);
            f.active_site=0x00aef6ee;
            f.requested_slash_index=reverse_find_native_string_header_00467cf0(
                n.requested,n.temporary,0x7fffffffu);
            Word returned=word(n.temporary,4);
            if(returned) {
                const Word size=word(n.temporary)+1u;
                f.active_site=0x00aef708;return_pair(returned,size,c.strings_);
            }
            f.active_site=0x00aef71d;
            construct_native_string_header_0041e870_exact(n.temporary,c.slash_,c.strings_,c.feature_);
            f.active_site=0x00aef730;
            f.candidate_slash_index=reverse_find_native_string_header_00467cf0(
                n.candidate,n.temporary,0x7fffffffu);
            returned=word(n.temporary,4);
            if(returned) {
                const Word size=word(n.temporary)+1u;
                f.active_site=0x00aef74a;return_pair(returned,size,c.strings_);
            }
            early_reject=f.requested_slash_index!=f.candidate_slash_index;
        }
        if(early_reject) {                                    // AEF67A path
            Word returned=word(n.candidate,4);f.native_state=0;
            if(returned) {
                f.active_site=0x00aef68e;
                return_pair(returned,f.candidate_length_register+1u,c.strings_);
            }
            returned=word(n.requested,4);f.native_state=-1;
            if(returned) {
                f.active_site=0x00aef6b1;
                return_pair(returned,f.requested_length_register+1u,c.strings_);
            }
            f.normal_result=0;f.phase=Phase::complete;return 0;
        }
        bool rejected=false;
        if(f.requested_slash_index>=0) {
            const Word slash_index=static_cast<Word>(f.requested_slash_index);
            f.candidate_directory_homes[2]=slash_index;f.candidate_directory_homes[1]=0;
            f.candidate_directory_homes[0]=bits(n.auxiliary);
            f.active_site=0x00aef772;
            void* const candidate_directory=substring_native_atlas_string_header_00469840_exact(
                n.candidate,f.candidate_directory_homes,c.strings_,c.feature_,f.candidate_directory);
            f.requested_directory_homes[2]=slash_index;f.requested_directory_homes[1]=0;
            f.requested_directory_homes[0]=bits(n.temporary);
            f.native_state=6;f.active_site=0x00aef78a;
            void* const requested_directory=substring_native_atlas_string_header_00469840_exact(
                n.requested,f.requested_directory_homes,c.strings_,c.feature_,f.requested_directory);
            f.active_site=0x00aef792;
            const bool different=unequal_native_string_headers_00449af0(
                requested_directory,candidate_directory);
            Word returned=word(n.temporary,4);
            if(returned) {
                const Word size=word(n.temporary)+1u;
                f.active_site=0x00aef7ac;return_pair(returned,size,c.strings_);
            }
            returned=word(n.auxiliary,4);f.native_state=3;
            if(returned) {
                const Word size=word(n.auxiliary)+1u;
                f.active_site=0x00aef7d0;return_pair(returned,size,c.strings_);
            }
            rejected=different;
            if(!different) {
                f.requested_tail_homes[2]=0x7fffffffu;f.requested_tail_homes[1]=slash_index+1u;
                f.requested_tail_homes[0]=bits(n.temporary);
                f.active_site=0x00aef826;
                void* const tail=substring_native_atlas_string_header_00469840_exact(
                    n.requested,f.requested_tail_homes,c.strings_,c.feature_,f.requested_tail);
                f.native_state=7;f.active_site=0x00aef835;
                assign_native_atlas_string_header_00425f40_exact(n.requested,tail,
                    c.strings_,c.feature_,f.requested_assignment);
                f.native_state=3;f.active_site=0x00aef843;
                destroy_native_string_header_0041dd20(n.temporary,c.strings_);
                f.candidate_tail_homes[2]=0x7fffffffu;f.candidate_tail_homes[1]=slash_index+1u;
                f.candidate_tail_homes[0]=bits(n.temporary);
                f.active_site=0x00aef857;
                void* const other_tail=substring_native_atlas_string_header_00469840_exact(
                    n.candidate,f.candidate_tail_homes,c.strings_,c.feature_,f.candidate_tail);
                f.native_state=8;f.active_site=0x00aef866;
                assign_native_atlas_string_header_00425f40_exact(n.candidate,other_tail,
                    c.strings_,c.feature_,f.candidate_assignment);
                f.native_state=3;f.active_site=0x00aef874;
                destroy_native_string_header_0041dd20(n.temporary,c.strings_);
                f.requested_length_register=word(n.requested);  // AEF879
            }
        }
        if(!rejected) {
            f.active_site=0x00aef886;
            construct_native_string_header_0041e870_exact(n.temporary,c.extension_dot_ats_,c.strings_,c.feature_);
            const Word request_stem_count=f.requested_length_register-4u;// AEF890
            f.requested_suffix_homes[2]=0x7fffffffu;
            f.requested_suffix_homes[1]=request_stem_count;f.requested_suffix_homes[0]=bits(n.auxiliary);
            f.native_state=9;f.active_site=0x00aef8a2;
            void* const requested_suffix=substring_native_atlas_string_header_00469840_exact(
                n.requested,f.requested_suffix_homes,c.strings_,c.feature_,f.requested_suffix);
            f.active_site=0x00aef8ae;
            bool different=unequal_native_string_headers_00449af0(requested_suffix,n.temporary);
            Word returned=word(n.auxiliary,4);
            if(returned) {
                const Word size=word(n.auxiliary)+1u;
                f.active_site=0x00aef8c8;return_pair(returned,size,c.strings_);
            }
            returned=word(n.temporary,4);f.native_state=3;
            if(returned) {
                const Word size=word(n.temporary)+1u;
                f.active_site=0x00aef8ec;return_pair(returned,size,c.strings_);
            }
            if(!different) {
                f.active_site=0x00aef909;
                construct_native_string_header_0041e870_exact(n.temporary,c.extension_dot_ats_,c.strings_,c.feature_);
                const Word candidate_count=word(n.candidate)-4u;// AEF90E/917
                f.candidate_suffix_homes[2]=0x7fffffffu;
                f.candidate_suffix_homes[1]=candidate_count;f.candidate_suffix_homes[0]=bits(n.auxiliary);
                f.native_state=10;f.active_site=0x00aef929;
                void* const candidate_suffix=substring_native_atlas_string_header_00469840_exact(
                    n.candidate,f.candidate_suffix_homes,c.strings_,c.feature_,f.candidate_suffix);
                f.active_site=0x00aef935;
                different=unequal_native_string_headers_00449af0(candidate_suffix,n.temporary);
                f.active_site=0x00aef940;
                destroy_native_string_header_0041dd20(n.auxiliary,c.strings_);
                f.native_state=3;f.active_site=0x00aef94e;
                destroy_native_string_header_0041dd20(n.temporary,c.strings_);
                if(!different) {
                    f.active_site=0x00aef964;
                    construct_native_string_header_0041e870_exact(n.temporary,c.underscore_,c.strings_,c.feature_);
                    f.stem_homes[2]=request_stem_count;f.stem_homes[1]=0;
                    f.stem_homes[0]=bits(n.stem);
                    f.native_state=11;f.active_site=0x00aef97a;
                    void* const stem=substring_native_atlas_string_header_00469840_exact(
                        n.requested,f.stem_homes,c.strings_,c.feature_,f.stem);
                    f.native_state=12;f.active_site=0x00aef990;
                    void* const joined=concatenate_native_string_headers_004261a0_exact(
                        stem,n.joined,n.temporary,c.strings_,c.feature_,f.underscore_concat);
                    const Word request_prefix_count=f.requested_length_register-3u;
                    f.candidate_prefix_homes[2]=request_prefix_count;f.candidate_prefix_homes[1]=0;
                    f.candidate_prefix_homes[0]=bits(n.auxiliary);
                    f.native_state=13;f.active_site=0x00aef9ab;
                    void* const candidate_prefix=substring_native_atlas_string_header_00469840_exact(
                        n.candidate,f.candidate_prefix_homes,c.strings_,c.feature_,f.candidate_prefix);
                    f.active_site=0x00aef9b3;
                    f.normal_result=equal_native_string_headers_00435c40(candidate_prefix,joined)?1u:0u;
                    f.active_site=0x00aef9be;
                    destroy_native_string_header_0041dd20(n.auxiliary,c.strings_);
                    f.native_state=12;f.active_site=0x00aef9cc;
                    destroy_native_string_header_0041dd20(n.joined,c.strings_);
                    f.native_state=11;f.active_site=0x00aef9da;
                    destroy_native_string_header_0041dd20(n.stem,c.strings_);
                    f.native_state=3;f.active_site=0x00aef9e8;
                    destroy_native_string_header_0041dd20(n.temporary,c.strings_);
                    f.native_state=0;f.active_site=0x00aef9f6;
                    destroy_native_string_header_0041dd20(n.candidate,c.strings_);
                    f.native_state=-1;f.active_site=0x00aefa07;
                    destroy_native_string_header_0041dd20(n.requested,c.strings_);
                    f.phase=Phase::complete;return f.normal_result;
                }
            }
        }
        f.native_state=0;f.active_site=0x00aef7e9;
        destroy_native_string_header_0041dd20(n.candidate,c.strings_);
        f.native_state=-1;f.active_site=0x00aef7fa;
        destroy_native_string_header_0041dd20(n.requested,c.strings_);
        f.normal_result=0;f.phase=Phase::complete;return 0;
    } catch(...) {
        f.phase=Phase::failed;f.failure=std::current_exception();throw;
    }
}
void register_native_texture_atlas_descriptor_00aef280(void* ignored_receiver,
    volatile Word& descriptor_home,NativeTextureAtlasDescriptorFrame& f,
    NativeTextureAtlasLoaderContext& c) {
    if(f.phase!=Phase::fresh)
        throw std::logic_error("raw atlas descriptor Frame cannot replay");
    f.phase=Phase::entered;f.ignored_receiver=ignored_receiver;
    try {
        // This retained record and both acquisitions/parser frame preexist the
        // allocation. No allocation/refund/rollback is hidden in a local owner.
        f.allocation_entered=true;f.active_site=0x00aef29b;
        f.allocation_return=singleton_lifetime_allocate({SingletonAllocationKind::object,0x1c,0x1c});
        f.allocation_returned=true;f.native_state=0;           // AEF2A3/A9
        if(f.allocation_return) {
            f.constructor_entered=true;f.active_site=0x00aef2b5;
            f.text_register=initialize_native_text_buffer_00af5600(f.allocation_return);
        } else f.text_register=nullptr;
        // Original descriptor argument HOME is first consumed after the ctor.
        f.captured_descriptor=pointer(descriptor_home);         // AEF2C1
        const bool identical=f.native_name==f.captured_descriptor;// AEF2C9
        f.native_state=-1;
        put(f.native_name,0,0);put(f.native_name,4,0);
        if(!identical) {
            const Word request=word(f.captured_descriptor);    // AEF2E5
            f.preserve_home=1;f.active_site=0x00aef2ee;
            resize_native_string_header_0041dd40_exact(f.native_name,c.strings_,
                request,&f.preserve_home,c.feature_);
            if(word(f.captured_descriptor)!=0) {               // AEF2F3
                const Word count=word(f.native_name);          // AEF2F8
                const void* const input=pointer(word(f.captured_descriptor,4));// AEF2FC
                void* const output=pointer(word(f.native_name,4));// AEF2FF
                f.active_site=0x00aef306;
                copy_native_crt_bytes_00bf7680(output,input,count,c.feature_);
            }
        }
        void* const admission_manager=c.vfs_.actual_vfs_publication_0109ceec;// AEF313
        f.native_state=1;f.admission_entered=true;f.active_site=0x00aef321;
        f.admission_result=resolve_native_vfs_existing_name_00bdf4c0(
            admission_manager,f.native_name,c.vfs_.name_resolution,f.admission_resolution);
        // This read occurs unconditionally after the AL test, before its branch.
        const Word descriptor_data=word(f.captured_descriptor,4);// AEF328
        f.diagnostic_name=static_cast<const char*>(descriptor_data?pointer(descriptor_data):c.empty_name_);
        if(f.admission_result) {
            f.file_entered=true;f.active_site=0x00aef339;
            (void)load_native_particle_text_buffer_00af5850(f.text_register,f.diagnostic_name,
                c.vfs_.actual_vfs_publication_0109ceec,c.vfs_.bindings,
                c.vfs_.name_resolution,f.file_resolution,c.strings_);
            // AF5850's EAX is not tested. The original RELOADS the publication
            // after all of that provider's effects and ignores entry ECX.
            f.captured_parser_manager=c.atlas_publication_;    // AEF33E
            f.parser_buffer_home=bits(f.text_register);        // AEF344
            f.parser_entered=true;f.active_site=0x00aef345;
            (void)parse_native_texture_atlas_storage_00aeeaf0(
                f.captured_parser_manager,f.parser_buffer_home,f.parser,c.parser_);
        } else {
            // Actual4254B0 is one bare RET. Preserve its selected pointer/read
            // epoch; do not substitute a logging callback with new effects.
            f.active_site=0x00aef35b;
        }
        if(f.text_register) {                                 // AEF363/66
            f.active_site=0x00aef36a;
            destroy_native_particle_text_buffer_00af5620(f.text_register,c.strings_);
            f.text_destroyed=true;f.active_site=0x00aef370;
            singleton_lifetime_free(f.text_register);f.text_freed=true;
        }
        const Word returned=word(f.native_name,4);             // AEF378
        f.native_state=-1;
        if(returned) {
            const Word size=word(f.native_name)+1u;
            f.active_site=0x00aef394;return_pair(returned,size,c.strings_);
        }
        f.phase=Phase::complete;
    } catch(...) {
        f.phase=Phase::failed;f.failure=std::current_exception();throw;
    }
}
void load_native_texture_atlas_storage_00af0060(void* manager,
    volatile Word& request_home,NativeTextureAtlasLoaderFrame& f,
    NativeTextureAtlasLoaderContext& c) {
    if(f.phase!=Phase::fresh)
        throw std::logic_error("raw atlas loader Frame cannot replay");
    f.phase=Phase::entered;
    auto& n=f.native;
    try {
        f.captured_request=pointer(request_home);              // AF007C
        const Word log_data=word(f.captured_request,4);        // AF0080
        f.captured_manager=manager;                            // AF0087
        f.diagnostic_name=static_cast<const char*>(log_data?pointer(log_data):c.empty_name_);
        f.active_site=0x00af0096; // actual4254B0 bare RET, no callback.
        clone_inline(n.lowercase_request,f.captured_request,f.preserve_home,
            c.strings_,c.feature_,f.active_site,0x00af00b7,0x00af00ce);
        f.native_state=0;f.active_site=0x00af00de;
        lowercase_native_string_header_004bcc00(n.lowercase_request);
        const Word suffix_start=word(n.lowercase_request)-4u;   // AF00E3/EC
        f.suffix_homes[2]=0x7fffffffu;f.suffix_homes[1]=suffix_start;
        f.suffix_homes[0]=bits(n.temporary);
        f.active_site=0x00af00f9;
        void* const suffix=substring_native_atlas_string_header_00469840_exact(
            n.lowercase_request,f.suffix_homes,c.strings_,c.feature_,f.suffix);
        const Word suffix_data=word(suffix,4);                 // AF00FE
        if(!suffix_data)f.has_ats_suffix=false;
        else {
            f.active_site=0x00af010f;
            f.has_ats_suffix=_stricmp(static_cast<const char*>(pointer(suffix_data)),
                static_cast<const char*>(c.extension_dot_ats_))==0;
        }
        Word returned=word(n.temporary,4);                     // AF011C
        if(returned) {
            const Word size=word(n.temporary)+1u;
            f.active_site=0x00af012f;return_pair(returned,size,c.strings_);
        }
        if(f.has_ats_suffix) {
            put(n.directory,0,0);put(n.directory,4,0);           // AF0143/47
            f.native_state=1;f.active_site=0x00af0159;
            construct_native_string_header_0041e870_exact(n.temporary,c.backslash_,c.strings_,c.feature_);
            f.native_state=2;f.active_site=0x00af0171;
            const auto backslash=reverse_find_native_string_header_00467cf0(
                n.lowercase_request,n.temporary,0x7fffffffu);
            f.active_site=0x00af0181;
            construct_native_string_header_0041e870_exact(n.candidate,c.slash_,c.strings_,c.feature_);
            f.active_site=0x00af0194;
            const auto slash=reverse_find_native_string_header_00467cf0(
                n.lowercase_request,n.candidate,0x7fffffffu);
            f.final_separator_index=slash>backslash?slash:backslash;// AF0199
            returned=word(n.candidate,4);
            if(returned) {
                const Word size=word(n.candidate)+1u;
                f.active_site=0x00af01b2;return_pair(returned,size,c.strings_);
            }
            returned=word(n.temporary,4);f.native_state=1;
            if(returned) {
                const Word size=word(n.temporary)+1u;
                f.active_site=0x00af01d6;return_pair(returned,size,c.strings_);
            }
            if(f.final_separator_index>=0) {
                f.directory_homes[2]=static_cast<Word>(f.final_separator_index)+1u;
                f.directory_homes[1]=0;f.directory_homes[0]=bits(n.temporary);
                f.active_site=0x00af01f5;
                void* const directory=substring_native_atlas_string_header_00469840_exact(
                    n.lowercase_request,f.directory_homes,c.strings_,c.feature_,f.directory_substring);
                f.native_state=3;
                // This arm is ASSIGNMENT to the already-zero directory; no
                // additional clear or constructor is inserted at AF0200.
                if(static_cast<void*>(n.directory)!=directory) {
                    const Word request=word(directory);
                    f.preserve_home=1;f.active_site=0x00af0212;
                    resize_native_string_header_0041dd40_exact(n.directory,c.strings_,request,
                        &f.preserve_home,c.feature_);
                    if(word(directory)!=0) {
                        const Word count=word(n.directory);
                        const void* const input=pointer(word(directory,4));
                        void* const output=pointer(word(n.directory,4));
                        f.active_site=0x00af022a;
                        copy_native_crt_bytes_00bf7680(output,input,count,c.feature_);
                    }
                }
                returned=word(n.temporary,4);f.native_state=1;
                if(returned) {
                    const Word size=word(n.temporary)+1u;
                    f.active_site=0x00af024a;return_pair(returned,size,c.strings_);
                }
            } else {
                f.active_site=0x00af0261;
                assign_native_string_cstring_0041e350(n.directory,
                    static_cast<const char*>(c.directory_default_),c.strings_,c.feature_);
            }
            f.active_site=0x00af026f;
            construct_native_string_header_0041e870_exact(n.temporary,c.extension_ats_,c.strings_,c.feature_);
            void* const enumeration_manager=c.vfs_.actual_vfs_publication_0109ceec;// AF027B
            f.native_state=4;f.active_site=0x00af0290;
            enumerate_native_language_resources_00886280(enumeration_manager,n.list,n.directory,
                n.temporary,0,c.strings_,c.vfs_.enumeration,c.enumeration_calls_,f.enumeration);
            returned=word(n.temporary,4);f.native_state=6;
            if(returned) {
                const Word size=word(n.temporary)+1u;
                f.active_site=0x00af02ad;return_pair(returned,size,c.strings_);
            }
            while(word(n.list,8)!=0) {                         // AF02B9/378
                // Nonmoving child history adopted before pop/any candidate
                // effects. It never replaces the actual intrusive list/header.
                f.candidates.emplace_back();
                auto& i=f.candidates.back();i.phase=Phase::entered;
                try {
                    f.active_site=i.active_site=0x00af02cd;
                    pop_native_vfs_package_name_00557a90(n.list,n.candidate,
                        c.vfs_.strings,c.vfs_.invalid_parameters);
                    f.native_state=7;f.active_site=i.active_site=0x00af02dd;
                    i.matcher_result=match_native_texture_atlas_split_name_00aef3c0(
                        f.captured_request,n.candidate,i.matcher,c)!=0;
                    if(i.matcher_result) {
                        i.requested_already_present=false;i.texture_index=0;
                        for(;;) {
                            // Reload CURRENT count, CURRENT array, CURRENT
                            // texture/name for every scan step of every match.
                            if(signed_word(i.texture_index)>=signed_word(word(f.captured_manager,0x14))) {
                                if(!i.requested_already_present) {
                                    i.descriptor_home=bits(n.candidate);
                                    i.descriptor_entered=true;
                                    f.active_site=i.active_site=0x00af034f;
                                    register_native_texture_atlas_descriptor_00aef280(
                                        f.captured_manager,i.descriptor_home,i.descriptor,c);
                                }
                                break;
                            }
                            i.current_texture_array=pointer(word(f.captured_manager,0x10));
                            i.current_texture=pointer(word(pointer(bits(i.current_texture_array)+i.texture_index*4u)));
                            f.active_site=i.active_site=0x00af02fe;
                            i.current_texture_name=native_logical_texture_name_address_00b33e40(i.current_texture);
                            const Word stored_length=word(i.current_texture_name);// AF0303
                            const Word requested_length=word(f.captured_request);// AF0305
                            bool equal=false;
                            if(stored_length==requested_length) {
                                if(stored_length==0)equal=requested_length==0;
                                else if(requested_length==0)equal=false;
                                else {
                                    const char* const requested_data=static_cast<const char*>(pointer(word(f.captured_request,4)));// AF0324
                                    const char* const stored_data=static_cast<const char*>(pointer(word(i.current_texture_name,4)));// AF0327
                                    f.active_site=i.active_site=0x00af032c;
                                    equal=_stricmp(stored_data,requested_data)==0;
                                }
                            }
                            i.requested_already_present=i.requested_already_present||equal;
                            ++i.texture_index;
                            if(i.requested_already_present)break;
                        }
                    }
                    returned=word(n.candidate,4);f.native_state=6;
                    if(returned) {
                        const Word size=word(n.candidate)+1u;
                        f.active_site=i.active_site=0x00af036c;
                        return_pair(returned,size,c.strings_);
                    }
                    i.phase=Phase::complete;
                } catch(...) {
                    i.phase=Phase::failed;i.failure=std::current_exception();throw;
                }
            }
            f.native_state=1;f.active_site=0x00af038c;
            clear_native_render_resource_aliases_004d05e0(n.list,c.strings_);
            f.list_cleared=true;
            void* const current_sentinel=pointer(word(n.list,4));// AF0391
            f.active_site=0x00af0396;
            singleton_lifetime_free(current_sentinel);f.sentinel_freed=true;
            returned=word(n.directory,4);                      // AF039B
            put(n.list,4,0);f.native_state=0;                   // AF03A4/AC
            if(returned) {
                const Word size=word(n.directory)+1u;
                f.active_site=0x00af03be;return_pair(returned,size,c.strings_);
            }
        }
        returned=word(n.lowercase_request,4);f.native_state=-1;
        if(returned) {
            const Word size=word(n.lowercase_request)+1u;
            f.active_site=0x00af03ea;return_pair(returned,size,c.strings_);
        }
        f.phase=Phase::complete;
    } catch(...) {
        f.phase=Phase::failed;f.failure=std::current_exception();throw;
    }
}
} // namespace bsp
