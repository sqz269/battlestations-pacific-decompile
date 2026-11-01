#include "bsp/native_texture_atlas_parser_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_crt_memset.hpp"
#include "bsp/native_cube_texture_owner_array_reserve.hpp"
#include "bsp/native_lua_script_overrides.hpp"
#include "bsp/native_path_canonicalizer.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_raw_cstring_exact.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_texture_atlas_item_pointer_reserve.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw atlas parser requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word=std::uint32_t;
static_assert(sizeof(void*)==4);
static_assert(sizeof(NativePooledTextStorage)==4);
static_assert(sizeof(NativeTextureAtlasParserHomes)==0x44);
static_assert(offsetof(NativeTextureAtlasParserHomes,flags)==4);
static_assert(offsetof(NativeTextureAtlasParserHomes,manager_home)==0x14);
static_assert(offsetof(NativeTextureAtlasParserHomes,canonical_slash_allocation)==0x1c);
static_assert(offsetof(NativeTextureAtlasParserHomes,directory)==0x24);
static_assert(offsetof(NativeTextureAtlasParserHomes,texture_name)==0x2c);
static_assert(offsetof(NativeTextureAtlasParserHomes,item_name)==0x3c);
Word word(const void* raw, Word offset=0) noexcept {
    Word result;
    __asm {
        mov eax, raw
        mov edx, offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* raw, Word offset, Word value) noexcept {
    __asm {
        mov eax, raw
        mov edx, offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
Word bits(const void* raw) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(raw));
}
void* at(void* raw, Word offset) noexcept { return pointer(bits(raw)+offset); }
std::int32_t signed_word(Word value) noexcept {
    std::int32_t result;
    std::memcpy(&result,&value,sizeof result);
    return result;
}
Word doubled_capacity(Word capacity) noexcept {
    const Word doubled=capacity+capacity;
    return signed_word(doubled)>1 ? doubled : 1u;
}
void return_pair(Word captured_data, Word captured_size, NativeStringRawPoolContext& strings) {
    auto* const pool=native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool,pointer(captured_data),captured_size,
        strings.actual_small_returns_disabled_01090aa4);
}
void return_text(char* captured, NativeStringRawPoolContext& strings) {
    if(captured) release_native_pooled_text_bytes_00aee1e0(captured,strings);
}
}
NativeTextureAtlasParserContext::NativeTextureAtlasParserContext(
    const game::GameNativeReadOnlyData& data, NativeStringRawPoolContext& strings,
    const volatile Word& feature, char* scratch, NativePathCanonicalizerRawContext& canonical,
    game::GameNativeRendererApplication& renderer)
    :data_(data),strings_(strings),feature_(feature),scratch_(scratch),canonical_(canonical),
      renderer_(renderer),cache_(renderer.texture_cache()),items_(data,strings,feature),
      parameters_(data,strings,scratch),literals_{
        static_cast<const char*>(data.data_at(0x00d5d7a0,13)),
        static_cast<const char*>(data.data_at(0x00ce7898,2)),
        static_cast<const char*>(data.data_at(0x00ce4d38,2)),
        static_cast<const char*>(data.data_at(0x00ce4cd4,2)),
        static_cast<const char*>(data.data_at(0x00ce3a0c,1)),
        static_cast<const char*>(data.data_at(0x00d5d794,12))} {
    if(!same_native_string_raw_pool_cells(strings,canonical.strings) ||
       &feature!=&canonical.actual_feature_word_0109eea4 ||
       !same_native_string_storage_raw_pool_cells(cache_.strings,strings))
        throw std::logic_error("raw atlas parser requires one canonical pool and feature graph");
}
std::uint8_t parse_native_texture_atlas_storage_00aeeaf0(void* manager,
    volatile Word& actual_buffer_home, NativeTextureAtlasParserFrame& f,
    NativeTextureAtlasParserContext& c) {
    if(f.phase!=NativeTextureAtlasParserPhase::fresh)
        throw std::logic_error("raw atlas parser Frame cannot replay");
    f.phase=NativeTextureAtlasParserPhase::entered;
    f.actual_manager=manager;
    f.saved_buffer_receiver=pointer(actual_buffer_home);         // AEEB09
    f.native.manager_home=bits(manager);                         // AEEB0F
    f.native.flags=0;                                          // AEEB17
    try {
        f.active_site=0x00aeeb1b;
        rewind_native_text_buffer_00af55f0(f.saved_buffer_receiver);
        f.native.line.data=nullptr;                            // AEEB20
        f.native_state=0;                                      // AEEB2B
        f.active_site=0x00aeeb2f;
        if(!read_native_text_buffer_line_00af5740(f.saved_buffer_receiver,
            &f.native.line,c.strings_,c.scratch_)) {
            char* const line=f.native.line.data;                // AEEB38
            f.native_state=-1;                                 // AEEB3E
            f.active_site=0x00aeeb62;
            return_text(line,c.strings_);
            f.normal_result=0; f.phase=NativeTextureAtlasParserPhase::complete;
            return 0;
        }
        f.active_site=0x00aeeb8f;
        void* token=get_native_pooled_text_token_00aee3c0(&f.native.line,
            &f.native.header_or_item_name_token,0,c.strings_);
        const char* const header_token=static_cast<const char*>(pointer(word(token)));
        f.active_site=0x00aeeb9c;
        const bool bad_header=_stricmp(header_token,c.literals_[0])!=0;
        char* const header_return=f.native.header_or_item_name_token.data; // AEEBA1
        f.active_site=0x00aeebc8;
        return_text(header_return,c.strings_);
        if(bad_header) {
            char* const line=f.native.line.data;                // AEEBD8
            f.native_state=-1;                                 // AEEBDE
            f.active_site=0x00aeec02;
            return_text(line,c.strings_);
            f.normal_result=0; f.phase=NativeTextureAtlasParserPhase::complete;
            return 0;
        }
        f.active_site=0x00aeec30;
        token=get_native_pooled_text_token_00aee3c0(&f.native.line,
            &f.native.texture_token,1,c.strings_);
        const void* const texture_text=pointer(word(token));    // AEEC35
        f.native_state=1;                                      // AEEC3C
        f.active_site=0x00aeec41;
        construct_native_string_header_0041e870_exact(f.native.texture_name,
            texture_text,c.strings_,c.feature_);
        char* const texture_token=f.native.texture_token.data;  // AEEC46
        f.native_state=3;                                      // AEEC4C
        f.active_site=0x00aeec6a;
        return_text(texture_token,c.strings_);
        const void* const descriptor=at(f.saved_buffer_receiver,0xc); // AEEC76
        f.native.texture_token.data=nullptr;                   // AEEC7E
        f.active_site=0x00aeec82;
        copy_construct_native_string_header_00426060_exact(f.native.directory,
            descriptor,c.strings_,c.feature_,f.directory_copy);
        f.native_state=4;                                      // AEEC8F
        f.active_site=0x00aeec94;
        canonicalize_native_path_00bee390(f.native.canonical_slash_allocation,
            f.native.directory,c.canonical_);
        const Word canonical_data=word(f.native.canonical_slash_allocation,4); // AEEC99
        if(canonical_data!=0) {
            const Word size=word(f.native.canonical_slash_allocation)+1u;
            f.active_site=0x00aeecac;
            return_pair(canonical_data,size,c.strings_);
        }
        f.active_site=0x00aeecc6;
        const Word slash_position=reverse_find_native_string_bytes_004bcb80(
            f.native.directory,c.literals_[1],0x7fffffff);
        const Word old_directory_length=word(f.native.directory);// AEECCB
        f.preserve_home=1;
        f.active_site=0x00aeecd6;
        resize_native_string_header_0041dd40_exact(f.native.directory,c.strings_,
            slash_position,&f.preserve_home,c.feature_);
        const Word directory_length=word(f.native.directory);   // AEECDB
        if(old_directory_length<directory_length) {             // AEECDF/E1 unsigned
            const Word directory_data=word(f.native.directory,4); // AEECE3
            f.active_site=0x00aeecef;
            fill_native_crt_bytes_00bf79f0(pointer(directory_data+old_directory_length),
                0x20,directory_length-old_directory_length,c.feature_);
        }
        f.active_site=0x00aeed00;
        construct_native_string_header_0041e870_exact(f.native.canonical_slash_allocation,
            c.literals_[1],c.strings_,c.feature_);
        const Word slash_length=word(f.native.canonical_slash_allocation); // AEED05
        const Word slash_data=word(f.native.canonical_slash_allocation,4); // AEED0B
        f.native_state=5;                                      // AEED0F
        if(slash_length!=0) {
            const Word prior=word(f.native.directory);          // AEED16
            f.preserve_home=1;
            f.active_site=0x00aeed26;
            resize_native_string_header_0041dd40_exact(f.native.directory,c.strings_,
                prior+slash_length,&f.preserve_home,c.feature_);
            const Word destination=word(f.native.directory,4); // AEED2B
            f.active_site=0x00aeed34;
            copy_native_crt_bytes_00bf7680(pointer(destination+prior),
                pointer(slash_data),slash_length,c.feature_);
            f.saved_buffer_receiver=pointer(actual_buffer_home);// AEED39, reached only here
        }
        f.native_state=4;                                      // AEED42
        if(slash_data!=0) {
            f.active_site=0x00aeed50;
            return_pair(slash_data,slash_length+1u,c.strings_);
        }
        f.captured_renderer=c.renderer_.publication_00f8d394(); // AEED5C BEFORE concat
        f.active_site=0x00aeed70;
        void* const full_name=concatenate_native_string_headers_004261a0_exact(
            f.native.directory,f.native.full_name,f.native.texture_name,
            c.strings_,c.feature_,f.full_name_concat);
        f.captured_profile_address=word(f.captured_renderer);    // AEED75 AFTER concat
        const void* const profile=c.data_.data_at(f.captured_profile_address,0x68);
        f.captured_slot64=word(profile,0x64);                    // AEED7A
        f.native_state=6;                                      // AEED7F
        // Source-only admission over the genuine existing birth/provider graph.
        // It consumes captured values, never rereads publication/receiver/slot.
        NativeTextureCacheContext& admitted=c.renderer_.atlas_parser_texture_cache_for(
            f.captured_renderer,f.captured_profile_address,f.captured_slot64,c.data_);
        if(&admitted!=&c.cache_)
            throw std::logic_error("raw atlas renderer cache graph changed");
        f.cache_entered=true;
        f.active_site=0x00aeed84;
        f.texture_register=load_native_renderer_texture_00b319b0(f.captured_renderer,
            full_name,0,admitted,&f.texture_acquired);
        const Word full_name_data=word(f.native.full_name,4);    // AEED88
        f.native.saved_texture_home=bits(f.texture_register);   // AEED8E
        f.native_state=4;                                      // AEED92
        if(full_name_data!=0) {
            const Word size=word(f.native.full_name)+1u;
            f.active_site=0x00aeeda4;
            return_pair(full_name_data,size,c.strings_);
        }
        void* array=pointer(f.native.manager_home);             // AEEDB0
        Word capacity=word(array,0x18);                         // AEEDB4
        array=at(array,0x10);                                  // AEEDB7
        f.texture_append_header=array;
        if(word(array,4)==capacity) {                           // AEEDBA
            const Word request=doubled_capacity(capacity);
            f.active_site=0x00aeedce;
            reserve_native_cube_texture_owner_array_00735ff0(array,signed_word(request));
        }
        const Word texture_count=word(array,4);                 // AEEDD3
        const Word texture_table=word(array);                   // AEEDD6
        const Word texture_slot=texture_table+texture_count*4u; // AEEDD8
        f.texture_append_slot=texture_slot;
        if(texture_slot!=0) {
            put(pointer(texture_slot),0,bits(f.texture_register));// AEEDDF creator transfer
            f.texture_slot_written=true;
        }
        put(array,4,word(array,4)+1u);                           // AEEDE1 current increment
        f.texture_count_incremented=true;
        for(;;) {
            f.active_site=0x00aeedec;
            if(!read_native_text_buffer_line_00af5740(f.saved_buffer_receiver,
                &f.native.line,c.strings_,c.scratch_)) break;
            const char* const line=f.native.line.data;
            f.active_site=0x00aeedff;
            if(_stricmp(line,c.literals_[2])==0) break;
        }
        f.active_site=0x00aeee12;
        bool have_line=read_native_text_buffer_line_00af5740(f.saved_buffer_receiver,
            &f.native.line,c.strings_,c.scratch_);
        while(have_line) {
            const char* const closing_line=f.native.line.data;
            f.active_site=0x00aeee2a;
            if(_stricmp(closing_line,c.literals_[3])==0) break;
            const char* const empty_line=f.native.line.data;     // AEEE3A current after compare
            f.active_site=0x00aeee44;
            if(_stricmp(empty_line,c.literals_[4])!=0) {
                f.active_site=0x00aeee5f;
                token=get_native_pooled_text_token_00aee3c0(&f.native.line,
                    &f.native.keyword,0,c.strings_);
                const char* const keyword=static_cast<const char*>(pointer(word(token)));
                f.active_site=0x00aeee6c;
                const bool is_item=_stricmp(keyword,c.literals_[5])==0;
                char* const keyword_return=f.native.keyword.data; // AEEE71
                f.active_site=0x00aeee98;
                return_text(keyword_return,c.strings_);
                f.native.keyword.data=nullptr;                 // AEEEA6 BEFORE branch
                if(is_item) {
                    f.item_calls.emplace_back();               // Source-adopt BEFORE BF681B
                    auto& call=f.item_calls.back();
                    f.active_item=&call;
                    call.phase=NativeTextureAtlasParserPhase::entered;
                    f.active_site=call.active_site=0x00aeeeb6;
                    void* const allocation=singleton_lifetime_allocate({
                        SingletonAllocationKind::object,0x30,0x30});
                    call.allocation=allocation;
                    call.allocation_returned=true;
                    f.native.canonical_slash_allocation[0]=bits(allocation); // AEEEC0
                    f.native_state=7;                          // AEEEC6
                    if(allocation) {
                        f.active_site=call.active_site=0x00aeeed8;
                        token=get_native_pooled_text_token_00aee3c0(&f.native.line,
                            &f.native.header_or_item_name_token,1,c.strings_);
                        const void* const item_text=pointer(word(token)); // AEEEDD
                        f.native.flags|=1u;                    // AEEEDF
                        f.native_state=8;                      // AEEEE9
                        f.active_site=call.active_site=0x00aeeeee;
                        void* const name=construct_native_string_header_0041e870_exact(
                            f.native.item_name,item_text,c.strings_,c.feature_);
                        const Word buffer=actual_buffer_home;  // AEEEF3 CURRENT incoming HOME
                        f.native.flags|=2u;                    // AEEEF7
                        call.constructor_homes[2]=buffer+0xcu;
                        call.constructor_homes[1]=bits(f.texture_register);
                        call.constructor_homes[0]=bits(name);
                        f.native_state=9;                      // AEEF04
                        call.constructor_entered=true;
                        f.active_site=call.active_site=0x00aeef0c;
                        f.texture_register=construct_native_texture_atlas_item_00aee4d0(
                            allocation,call.constructor_homes,call.constructor,c.items_);
                    } else {
                        f.texture_register=nullptr;            // AEEF15, no recovery/default
                    }
                    call.item_return=f.texture_register;
                    Word flags=f.native.flags;                 // AEEF17 EBX snapshot
                    f.native_state=10;                         // AEEF1E
                    if((flags&2u)!=0) {
                        const Word name_data=word(f.native.item_name,4); // AEEF28
                        flags&=~2u;                            // AEEF2C
                        f.native.flags=flags;                  // AEEF31
                        if(name_data!=0) {
                            const Word size=word(f.native.item_name)+1u;
                            f.active_site=call.active_site=0x00aeef42;
                            return_pair(name_data,size,c.strings_);
                        }
                    }
                    f.native_state=4;                          // AEEF51
                    if((flags&1u)!=0) {
                        char* const item_token=f.native.header_or_item_name_token.data; // AEEF5B
                        flags&=~1u;                            // AEEF5F
                        f.native.flags=flags;                  // AEEF64
                        f.active_site=call.active_site=0x00aeef82;
                        return_text(item_token,c.strings_);
                        f.native.header_or_item_name_token.data=nullptr; // AEEF8E
                    }
                    // This child DWORD is DISTINCT backing. AEE620 later
                    // destroys its buffer representation for float/CW scratch.
                    call.parameter_buffer_home=actual_buffer_home; // AEEF96 CURRENT copy
                    call.parameters_entered=true;
                    f.active_site=call.active_site=0x00aeef9d;
                    read_native_texture_atlas_item_parameters_00aee620(f.texture_register,
                        call.parameter_buffer_home,call.parameters,c.parameters_);
                    array=pointer(f.native.manager_home);      // AEEFA2
                    capacity=word(array,0xc);                   // AEEFA6
                    array=at(array,4);                         // AEEFA9
                    call.append_header=array;
                    if(word(array,4)==capacity) {               // AEEFAC
                        const Word request=doubled_capacity(capacity);
                        f.active_site=call.active_site=0x00aeefc0;
                        reserve_native_texture_atlas_item_pointers_00735f90(array,signed_word(request));
                    }
                    const Word count=word(array,4);            // AEEFC5
                    const Word table=word(array);              // AEEFC8
                    const Word slot=table+count*4u;             // AEEFCA
                    call.append_slot=slot;
                    if(slot!=0) {
                        put(pointer(slot),0,bits(f.texture_register)); // AEEFD1
                        call.slot_written=true;
                    }
                    put(array,4,word(array,4)+1u);              // AEEFD3 CURRENT increment
                    call.count_incremented=true;
                    f.texture_register=pointer(f.native.saved_texture_home); // AEEFD7
                    call.phase=NativeTextureAtlasParserPhase::complete;
                }
                f.saved_buffer_receiver=pointer(actual_buffer_home); // AEEFDB, nonempty arm
            }
            f.active_site=0x00aeefe6;
            have_line=read_native_text_buffer_line_00af5740(f.saved_buffer_receiver,
                &f.native.line,c.strings_,c.scratch_);
        }
        const Word directory_data=word(f.native.directory,4);   // AEEFF3
        f.native_state=3;                                      // AEEFF9
        if(directory_data!=0) {
            const Word size=word(f.native.directory)+1u;
            f.active_site=0x00aef00b;
            return_pair(directory_data,size,c.strings_);
        }
        const Word texture_name_data=word(f.native.texture_name,4); // AEF017
        f.native_state=0;                                      // AEF01D
        if(texture_name_data!=0) {
            const Word size=word(f.native.texture_name)+1u;
            f.active_site=0x00aef02f;
            return_pair(texture_name_data,size,c.strings_);
        }
        char* const line=f.native.line.data;                    // AEF03B
        f.native_state=-1;                                     // AEF041
        f.active_site=0x00aef062;
        return_text(line,c.strings_);
        f.normal_result=1;
        f.phase=NativeTextureAtlasParserPhase::complete;
        return 1;                                              // AEF075, even EOF
    } catch(...) {
        f.phase=NativeTextureAtlasParserPhase::failed;
        f.failure=std::current_exception();
        if(f.active_item && f.active_item->phase==NativeTextureAtlasParserPhase::entered) {
            f.active_item->phase=NativeTextureAtlasParserPhase::failed;
            f.active_item->failure=f.failure;
        }
        throw;
    }
}
} // namespace bsp
