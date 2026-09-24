#include "bsp/native_lua_field_defaults.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include <cstdint>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua field defaults require MSVC Win32.
#endif
namespace bsp {
namespace {
using Word=std::uint32_t;
void* at(const void* p, Word offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+offset);
}
Word word(const void* p, Word offset=0) noexcept {
    return *static_cast<const volatile Word*>(at(p,offset));
}
void put(void* p, Word value) noexcept { *static_cast<volatile Word*>(p)=value; }
void* pointer(const void* p, Word offset=0) noexcept { return reinterpret_cast<void*>(word(p,offset)); }
void copy_float(void* destination,const void* source) noexcept {
    __asm { mov eax,source }
    __asm { mov edx,destination }
    __asm { fld dword ptr [eax] }
    __asm { fstp dword ptr [edx] }
}
} // namespace
void store_native_lua_field_default_00bd61c0(void* field,
    const void* fallback, NativeStringRawPoolContext& strings) {
    const Word tag=word(field);
    switch(tag) {
    case 0: {
        void* const destination=pointer(field,4);
        const void* const source=pointer(fallback,4);
        Word length=0;
        if(source) while(*static_cast<const volatile unsigned char*>(at(source,length))) ++length;
        resize_native_string_header_0041dd40(destination,strings,length,false); // BD61F4
        void* const current_data=pointer(destination,4);
        if(current_data) {
            const Word current_length=word(destination);
            // BF7680 admits overlap; its zero-byte branch performs no access.
            if(current_length) std::memmove(current_data,source,current_length); // BD6205
        }
        return;
    }
    case 1: case 4: {
        void* const destination=pointer(field,4);
        const Word value=word(fallback,4);
        put(destination,value);return;
    }
    case 2: case 10: {
        void* const destination=pointer(field,4);
        copy_float(destination,at(fallback,4));return;
    }
    case 3: {
        void* const destination=pointer(field,4);
        const auto value=*static_cast<const volatile unsigned char*>(at(fallback,4));
        *static_cast<volatile unsigned char*>(destination)=value;return;
    }
    case 5: case 6: {
        const void* const source=pointer(fallback,4);
        void* const destination=pointer(field,4);
        copy_float(destination,source);
        copy_float(at(destination,4),at(source,4));
        if(tag==5) copy_float(at(destination,8),at(source,8));
        return;
    }
    case 7: {
        const void* const source=pointer(fallback,4);
        void* const destination=pointer(field,4);
        (void)copy_native_camera_matrix_004134f0(destination,nullptr,source); // BD62B3 tail
        return;
    }
    case 8: {
        const void* const source=pointer(fallback,4);
        const Word first=word(source); // Native reads this BEFORE destination.
        void* const destination=pointer(field,4);
        put(destination,first);
        put(at(destination,4),word(source,4));
        put(at(destination,8),word(source,8));
        put(at(destination,12),word(source,12));return;
    }
    default:return;
    }
}
} // namespace bsp
