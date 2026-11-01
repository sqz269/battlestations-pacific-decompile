#include "bsp/native_texture_atlas_item_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_crt_memset.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_raw_header_return.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include <cstddef>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw atlas item storage requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
Word word(const void* address, Word byte_offset=0) {
    Word result;
    __asm {
        mov eax, address
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* address, Word byte_offset, Word value) {
    __asm {
        mov eax, address
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits)); }
void* at(void* raw, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(raw)+offset);
}
}
NativeTextureAtlasItemStorageContext::NativeTextureAtlasItemStorageContext(
    const game::GameNativeReadOnlyData& data, NativeStringRawPoolContext& strings,
    const volatile Word& feature)
    : strings_(strings), feature_(feature), dot_00ce3a70_(data.data_at(0x00ce3a70, 2)) {}
void* construct_native_texture_atlas_item_00aee4d0(void* actual_item,
    volatile Word (&homes)[3], NativeTextureAtlasItemConstructFrame& f,
    NativeTextureAtlasItemStorageContext& c) {
    if(f.phase!=NativeTextureAtlasItemStoragePhase::fresh)
        throw std::logic_error("raw atlas item construction Frame cannot replay");
    f.phase=NativeTextureAtlasItemStoragePhase::entered;
    f.actual_item=actual_item;
    try {
        put(actual_item,0,0); put(actual_item,4,0);                // AEE4F4/4F6
        void* const name=at(actual_item,0xc);                     // AEE4F9
        f.native_state=0;                                       // AEE4FC
        put(name,0,0); put(name,4,0);                             // AEE500/502
        const void* const descriptor=pointer(homes[2]);           // AEE505
        f.native_state=1;                                       // AEE50B
        if(actual_item!=descriptor) {
            const Word request=word(descriptor);
            f.preserve_home=1;
            f.active_site=0x00aee517;
            resize_native_string_header_0041dd40_exact(actual_item,c.strings_,request,
                &f.preserve_home,c.feature_);
            if(word(descriptor)!=0) {                            // AEE51C
                const Word bytes=word(actual_item);              // AEE520
                const void* const source=pointer(word(descriptor,4)); // AEE522
                void* const destination=pointer(word(actual_item,4)); // AEE525
                f.active_site=0x00aee52b;
                copy_native_crt_bytes_00bf7680(destination,source,bytes,c.feature_);
            }
        }
        const void* const authored_name=pointer(homes[0]);        // AEE533
        if(name!=authored_name) {
            const Word request=word(authored_name);
            f.preserve_home=1;
            f.active_site=0x00aee542;
            resize_native_string_header_0041dd40_exact(name,c.strings_,request,
                &f.preserve_home,c.feature_);
            if(word(authored_name)!=0) {                         // AEE547
                const Word bytes=word(name);                     // AEE54B
                const void* const source=pointer(word(authored_name,4)); // AEE54D
                void* const destination=pointer(word(name,4));    // AEE550
                f.active_site=0x00aee556;
                copy_native_crt_bytes_00bf7680(destination,source,bytes,c.feature_);
            }
        }
        put(f.dot_header,0,0); put(f.dot_header,4,0);              // AEE566/56A
        f.preserve_home=1;
        f.active_site=0x00aee56e;
        resize_native_string_header_0041dd40_exact(f.dot_header,c.strings_,1,
            &f.preserve_home,c.feature_);
        f.captured_dot_data=pointer(word(f.dot_header,4));         // AEE573
        if(f.captured_dot_data) {
            const Word bytes=word(f.dot_header)+1u;              // AEE57B/57F
            f.active_site=0x00aee589;
            copy_native_crt_bytes_00bf7680(f.captured_dot_data,c.dot_00ce3a70_,bytes,c.feature_);
        }
        f.active_site=0x00aee59d;
        f.dot_index=static_cast<Word>(reverse_find_native_string_header_00467cf0(
            name,f.dot_header,0x7fffffffu));
        if(f.captured_dot_data) {                                // AEE5A2
            const Word bytes=word(f.dot_header)+1u;              // AEE5A8/5AE
            f.active_site=0x00aee5b3;
            auto* const pool=native_string_pool_get_or_create_00419cc0(
                c.strings_.actual_published_01090aa8,c.strings_.actual_manager_publication_01090aa0);
            f.active_site=0x00aee5ba;
            return_native_string_pool_00bd1510(pool,f.captured_dot_data,bytes,
                c.strings_.actual_small_returns_disabled_01090aa4);
        }
        if(f.dot_index!=0xffffffffu) {                           // AEE5BF
            const Word before=word(name);                        // AEE5C4
            f.preserve_home=1;
            f.active_site=0x00aee5cb;
            resize_native_string_header_0041dd40_exact(name,c.strings_,f.dot_index,
                &f.preserve_home,c.feature_);
            const Word after=word(name);                         // AEE5D0
            if(before<after) {
                void* const destination=pointer(word(name,4)+before);
                const Word bytes=after-before;
                f.active_site=0x00aee5e1;
                fill_native_crt_bytes_00bf79f0(destination,0x20,bytes,c.feature_);
            }
        }
        put(actual_item,8,homes[1]);                              // AEE5ED/5F6/5FF
        f.phase=NativeTextureAtlasItemStoragePhase::complete;
        return actual_item;
    } catch(...) {
        f.failure=std::current_exception();
        f.phase=NativeTextureAtlasItemStoragePhase::failed;
        throw;
    }
}
void destroy_native_texture_atlas_item_00aee220(void* actual_item,
    NativeTextureAtlasItemDestroyFrame& f, NativeTextureAtlasItemStorageContext& c) {
    if(f.phase!=NativeTextureAtlasItemStoragePhase::fresh)
        throw std::logic_error("raw atlas item destruction Frame cannot replay");
    f.phase=NativeTextureAtlasItemStoragePhase::entered;
    f.actual_item=actual_item;
    try {
        f.native_state=0;                                       // AEE242
        f.active_site=0x00aee256;
        return_native_string_header_buffer_0041dd20_exact(at(actual_item,0xc),c.strings_);
        f.name_returned=true;
        f.native_state=-1;                                      // AEE267
        f.active_site=0x00aee27a;
        return_native_string_header_buffer_0041dd20_exact(actual_item,c.strings_);
        f.descriptor_returned=true;
        f.phase=NativeTextureAtlasItemStoragePhase::complete;
    } catch(...) {
        f.failure=std::current_exception();
        f.phase=NativeTextureAtlasItemStoragePhase::failed;
        throw;
    }
}
} // namespace bsp
