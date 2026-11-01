#include "bsp/native_string_copy_concat_exact.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Exact raw string copy and concat require MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
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
}
void* copy_construct_native_string_header_00426060_exact(void* output,
    const void* source, NativeStringRawPoolContext& strings,
    const volatile Word& feature, NativeStringHeaderCopyExactFrame& f) {
    if(f.phase!=NativeStringCopyConcatPhase::fresh)
        throw std::logic_error("exact raw string copy Frame cannot replay");
    f.phase=NativeStringCopyConcatPhase::entered;
    try {
        const bool identical=output==source;                    // 426068
        put(output,0,0); put(output,4,0);                       // 42606A/70
        if(!identical) {
            const Word request=word(source);                   // 426079
            f.preserve_home=1;
            f.active_site=0x0042607e;
            resize_native_string_header_0041dd40_exact(output,strings,request,
                &f.preserve_home,feature);
            if(word(source)!=0) {                              // 426083
                const Word copied=word(output);                // 426088
                const void* const input=pointer(word(source,4));// 42608A
                void* const destination=pointer(word(output,4));// 42608D
                f.active_site=0x00426093;
                copy_native_crt_bytes_00bf7680(destination,input,copied,feature);
            }
        }
        f.phase=NativeStringCopyConcatPhase::complete;
        return output;
    } catch(...) {
        f.phase=NativeStringCopyConcatPhase::failed;
        f.failure=std::current_exception();
        throw;
    }
}
void* concatenate_native_string_headers_004261a0_exact(const void* left,
    void* output, const void* right, NativeStringRawPoolContext& strings,
    const volatile Word& feature, NativeStringHeaderConcatExactFrame& f) {
    if(f.phase!=NativeStringCopyConcatPhase::fresh)
        throw std::logic_error("exact raw string concat Frame cannot replay");
    f.phase=NativeStringCopyConcatPhase::entered;
    f.native_output_owned=0;                                    // 4261BB
    try {
        const bool identical=output==left;                      // 4261C6
        f.native_state=0;                                      // 4261C8
        put(output,0,0); put(output,4,0);                       // 4261CC/CE
        if(!identical) {
            const Word request=word(left);                     // 4261D3
            f.preserve_home=1;
            f.active_site=0x004261da;
            resize_native_string_header_0041dd40_exact(output,strings,request,
                &f.preserve_home,feature);
            if(word(left)!=0) {                                // 4261DF
                const Word copied=word(output);                // 4261E3
                void* const destination=pointer(word(output,4));// 4261E5
                const void* const input=pointer(word(left,4));  // 4261E9
                f.active_site=0x004261ee;
                copy_native_crt_bytes_00bf7680(destination,input,copied,feature);
            }
        }
        const Word appended=word(right);                       // 4261FA
        f.native_state=0;                                      // 4261FF
        f.native_output_owned=1;                                // 426203
        if(appended!=0) {
            const Word old_length=word(output);                 // 42620D
            f.preserve_home=1;
            f.active_site=0x00426217;
            resize_native_string_header_0041dd40_exact(output,strings,
                old_length+appended,&f.preserve_home,feature);
            const void* const input=pointer(word(right,4));     // 42621C
            const Word destination=word(output,4);              // 42621F
            f.active_site=0x00426227;
            copy_native_crt_bytes_00bf7680(pointer(destination+old_length),
                input,appended,feature);
        }
        f.phase=NativeStringCopyConcatPhase::complete;
        return output;
    } catch(...) {
        f.phase=NativeStringCopyConcatPhase::failed;
        f.failure=std::current_exception();
        throw;
    }
}
} // namespace bsp
