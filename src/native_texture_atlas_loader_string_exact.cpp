#include "bsp/native_texture_atlas_loader_string_exact.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_crt_strncpy.hpp"
#include "bsp/native_crt_strstr.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Exact raw atlas loader strings require MSVC Win32.
#endif
namespace bsp {
namespace {
using Word=std::uint32_t;
using Phase=NativeAtlasLoaderStringPhase;
static_assert(sizeof(void*)==4);
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
    std::int32_t result;
    std::memcpy(&result,&value,4);
    return result;
}
void return_pair(Word captured_data, Word size, NativeStringRawPoolContext& strings) {
    auto* const pool=native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool,pointer(captured_data),size,
        strings.actual_small_returns_disabled_01090aa4);
}
}
void* assign_native_atlas_string_header_00425f40_exact(void* destination,
    const void* source, NativeStringRawPoolContext& strings,
    const volatile Word& feature, NativeAtlasStringAssignFrame& f) {
    if(f.phase!=Phase::fresh)
        throw std::logic_error("exact raw atlas assignment Frame cannot replay");
    f.phase=Phase::entered;
    try {
        if(destination!=source) {                              // 425F48
            const Word requested=word(source);                // 425F4C
            f.preserve_home=1;
            f.active_site=0x00425f51;
            resize_native_string_header_0041dd40_exact(destination,strings,
                requested,&f.preserve_home,feature);
            if(word(source)!=0) {                             // 425F56
                const Word count=word(destination);           // 425F5B
                const void* const input=pointer(word(source,4));// 425F5D
                void* const output=pointer(word(destination,4));// 425F60
                f.active_site=0x00425f66;
                copy_native_crt_bytes_00bf7680(output,input,count,feature);
            }
        }
        f.phase=Phase::complete;
        return destination;
    } catch(...) {
        f.phase=Phase::failed;f.failure=std::current_exception();throw;
    }
}
void* substring_native_atlas_string_header_00469840_exact(const void* source,
    volatile Word (&homes)[3], NativeStringRawPoolContext& strings,
    const volatile Word& feature, NativeAtlasStringSubstringFrame& f) {
    if(f.phase!=Phase::fresh)
        throw std::logic_error("exact raw atlas substring Frame cannot replay");
    f.phase=Phase::entered;
    f.native_output_flag=0;                                    // 46985B
    put(f.native_temporary,0,0);put(f.native_temporary,4,0);     // 469860/64
    Word start=homes[1];                                      // 469868
    Word count=homes[2];                                      // 46986E
    f.native_state=1;                                        // 469872
    try {
        if(signed_word(start)<0) { count+=start;start=0; }      // 46987A..80
        else {
            const Word length=word(source);                   // 469882
            if(start>length)start=length;
        }
        const Word data=word(source,4);                       // 46988A
        if(data!=0) {
            const Word length=word(source);                   // 469895
            if(length!=0 && start<length) {
                if(start+count>length)count=length-start;      // 4698A5..AE
                f.captured_start=start;f.captured_count=count;
                f.captured_begin=pointer(data+start);          // 4698B7
                f.preserve_home=1;
                f.active_site=0x004698ba;
                resize_native_string_header_0041dd40_exact(f.native_temporary,
                    strings,count,&f.preserve_home,feature);
                f.captured_temporary_data=pointer(word(f.native_temporary,4));// 4698C1
                if(count!=0) {                                // 4698BF/C5
                    f.active_site=0x004698ca;
                    copy_native_crt_padded_bytes_00bf9280(
                        f.captured_temporary_data,f.captured_begin,count);
                }
                // Incoming output HOME is consumed HERE, after the strncpy.
                f.returned_output=pointer(homes[0]);           // 4698D2
                f.active_site=0x004698dd;
                copy_construct_native_string_header_00426060_exact(
                    f.returned_output,f.native_temporary,strings,feature,f.output_copy);
                f.native_output_flag=1;                       // 4698E4
                f.native_state=0;                             // 4698EC
                if(f.captured_temporary_data) {
                    const Word size=word(f.native_temporary)+1u;// 4698F2/F8
                    f.active_site=0x004698fd;
                    return_pair(bits(f.captured_temporary_data),size,strings);
                }
                f.phase=Phase::complete;
                return f.returned_output;
            }
        }
        f.returned_output=pointer(homes[0]);                   // 469920
        const bool identical=f.returned_output==f.native_temporary;// 469928
        put(f.returned_output,0,0);put(f.returned_output,4,0);   // 46992A/2C
        if(!identical) {
            f.preserve_home=1;
            f.active_site=0x00469936;
            resize_native_string_header_0041dd40_exact(f.returned_output,
                strings,0,&f.preserve_home,feature);
        }
        f.phase=Phase::complete;
        return f.returned_output;
    } catch(...) {
        f.phase=Phase::failed;f.failure=std::current_exception();throw;
    }
}
void replace_native_atlas_string_substrings_004cad40_exact(void* value,
    volatile Word (&homes)[3], NativeStringRawPoolContext& strings,
    const volatile Word& feature, NativeAtlasStringReplaceFrame& f) {
    if(f.phase!=Phase::fresh)
        throw std::logic_error("exact raw atlas replacement Frame cannot replay");
    f.phase=Phase::entered;
    try {
        const Word pattern=word(pointer(homes[0]),4);           // 4CAD4E/60
        const Word data=word(value,4);                         // 4CAD63
        Word position=0xffffffffu;
        if(data && pattern) {
            f.active_site=0x004cad70;
            void* const found=find_native_crt_substring_00bf9440(pointer(data),pointer(pattern));
            if(found)position=bits(found)-word(value,4);        // 4CAD7C
        }
        f.position=position;
        if(position==0xffffffffu) { f.phase=Phase::complete;return; }
        f.captured_replacement=pointer(homes[1]);              // 4CAD90, ONCE
        for(;;) {
            // Source-only metadata adopted before this iteration's first child.
            f.iterations.emplace_back();
            auto& i=f.iterations.back();
            i.phase=Phase::entered;
            auto* const joined=f.native_headers;
            auto* const first=f.native_headers+2;
            auto* const prefix=f.native_headers+4;
            auto* const suffix=f.native_headers+6;
            try {
                const void* const current_search=pointer(homes[0]);// 4CAD95
                const Word search_length=word(current_search);  // 4CAD99
                i.suffix_homes[2]=0x7fffffffu;
                i.suffix_homes[1]=search_length+position;
                i.suffix_homes[0]=bits(suffix);
                f.active_site=i.active_site=0x004cadaa;
                void* const suffix_return=substring_native_atlas_string_header_00469840_exact(
                    value,i.suffix_homes,strings,feature,i.suffix);
                i.prefix_homes[2]=position;i.prefix_homes[1]=0;
                i.prefix_homes[0]=bits(prefix);
                f.native_state=0;                             // 4CADBB
                f.active_site=i.active_site=0x004cadc3;
                void* const prefix_return=substring_native_atlas_string_header_00469840_exact(
                    value,i.prefix_homes,strings,feature,i.prefix);
                f.native_state=1;                             // 4CADD0
                f.active_site=i.active_site=0x004cadd5;
                void* const first_return=concatenate_native_string_headers_004261a0_exact(
                    prefix_return,first,f.captured_replacement,strings,feature,i.first_concat);
                f.native_state=2;                             // 4CADE2
                f.active_site=i.active_site=0x004cade7;
                void* const joined_return=concatenate_native_string_headers_004261a0_exact(
                    first_return,joined,suffix_return,strings,feature,i.second_concat);
                f.native_state=3;                             // 4CADF0
                if(value!=joined_return) {
                    const Word request=word(joined_return);   // 4CADF7
                    i.preserve_home=1;
                    f.active_site=i.active_site=0x004cadfe;
                    resize_native_string_header_0041dd40_exact(value,strings,
                        request,&i.preserve_home,feature);
                    if(word(joined_return)!=0) {               // 4CAE03
                        const Word count=word(value);          // 4CAE08
                        void* const output=pointer(word(value,4));// 4CAE0A
                        const void* const input=pointer(word(joined_return,4));// 4CAE0E
                        f.active_site=i.active_site=0x004cae13;
                        copy_native_crt_bytes_00bf7680(output,input,count,feature);
                    }
                }
                Word returned=word(joined,4);f.native_state=2; // 4CAE1B/21
                if(returned) {
                    const Word size=word(joined)+1u;
                    f.active_site=i.active_site=0x004cae33;
                    return_pair(returned,size,strings);
                }
                returned=word(first,4);f.native_state=1;        // 4CAE3F/45
                if(returned) {
                    const Word size=word(first)+1u;
                    f.active_site=i.active_site=0x004cae57;
                    return_pair(returned,size,strings);
                }
                returned=word(prefix,4);f.native_state=0;       // 4CAE63/69
                if(returned) {
                    const Word size=word(prefix)+1u;
                    f.active_site=i.active_site=0x004cae7b;
                    return_pair(returned,size,strings);
                }
                returned=word(suffix,4);f.native_state=-1;      // 4CAE87/8D
                if(returned) {
                    const Word size=word(suffix)+1u;
                    f.active_site=i.active_site=0x004cae9e;
                    return_pair(returned,size,strings);
                }
                Word next_start=word(f.captured_replacement);  // 4CAEAE
                const Word current_data=word(value,4);        // 4CAEB0
                const Word current_pattern=word(pointer(homes[0]),4);// 4CAEB3/B7
                next_start+=position;                         // 4CAEBA
                Word next_position=0xffffffffu;
                if(current_data && current_pattern) {
                    bool search=true;
                    if(signed_word(next_start)<0)next_start=0;
                    else if(next_start>word(value))search=false;
                    if(search) {
                        f.active_site=i.active_site=0x004caed4;
                        void* const found=find_native_crt_substring_00bf9440(
                            pointer(current_data+next_start),pointer(current_pattern));
                        if(found)next_position=bits(found)-word(value,4);// 4CAEE0
                    }
                }
                const Word remaining=homes[2]-1u;             // 4CAEE8
                homes[2]=remaining;
                position=next_position;f.position=position;    // 4CAEED
                i.phase=Phase::complete;
                if(remaining==0 || position==0xffffffffu)break;
            } catch(...) {
                i.phase=Phase::failed;i.failure=std::current_exception();throw;
            }
        }
        f.phase=Phase::complete;
    } catch(...) {
        f.phase=Phase::failed;f.failure=std::current_exception();throw;
    }
}
} // namespace bsp
