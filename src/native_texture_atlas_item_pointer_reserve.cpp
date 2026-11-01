#include "bsp/native_texture_atlas_item_pointer_reserve.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw atlas item pointer reserve requires MSVC Win32.
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
Word bits(const void* raw) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(raw));
}
std::int32_t signed_word(Word value) noexcept {
    std::int32_t result;
    std::memcpy(&result,&value,sizeof result);
    return result;
}
}
void reserve_native_texture_atlas_item_pointers_00735f90(
    void* header, std::int32_t request) {
    if(request<1) request=1;                                    // 735F96..9D
    if(signed_word(word(header,8))>=request) return;             // 735FA2..A5
    const Word bytes=static_cast<Word>(request)*4u;              // 735FA7
    void* const replacement=singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots,bytes,bytes});    // BF55BE
    Word index=0;
    if(signed_word(word(header,4))>0) {                          // 735FBC
        Word destination=bits(replacement);                     // 735FC1
        do {
            if(destination!=0) {                               // 735FC3..C5
                const Word table=word(header);                 // 735FC7
                const Word value=word(pointer(table+index*4u)); // 735FC9
                put(pointer(destination),0,value);             // 735FCC
            }
            ++index;
            destination+=4u;
        } while(signed_word(index)<signed_word(word(header,4)));// 735FD4..D7
    }
    singleton_lifetime_free(pointer(word(header)));             // 735FD9..DC
    put(header,0,bits(replacement));                            // 735FE4
    put(header,8,static_cast<Word>(request));                    // 735FE6
}
} // namespace bsp
