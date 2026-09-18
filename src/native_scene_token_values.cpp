#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "bsp/native_scene_token_values.hpp"
#include <cstdio>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene token value reads require MSVC Win32.
#endif
namespace bsp {
namespace {
std::uint8_t& byte(void* p,std::uint32_t offset) noexcept {
    return *reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uint32_t>(p)+offset);
}
std::int32_t owner_preimage(void* p) noexcept {
    const auto bits=reinterpret_cast<std::uint32_t>(p);
    std::int32_t value;std::memcpy(&value,&bits,4);return value;
}
char* text(void* p) noexcept {return reinterpret_cast<char*>(reinterpret_cast<std::uint32_t>(p)+5);}
}
int NativeSceneTokenValueCalls::scan_decimal_00bf7533(const char* input,const char* format,std::int32_t* result) {
    return std::sscanf(input,format,result);
}
void recover_native_scene_whitespace_008d8f70(void* p,NativeSceneTokenValueContext& c) {
    while(!byte(p,0x809)) {
        const char current=peek_native_scene_byte_008d8900(p);
        if(!std::strchr(c.tokenizer.whitespace_00e0c940,current))break;
        const char reloaded=peek_native_scene_byte_008d8900(p);
        if(std::strchr(c.recovery_stop_00d15f34,reloaded))break;
        advance_native_scene_byte_008d8a50(p);
    }
    byte(p,0x80a)=byte(p,0x809);
}
bool peek_native_scene_bool_008d8e50(void* p,std::uint8_t* ok,NativeSceneTokenValueContext& c) {
    std::int32_t value=owner_preimage(p);
    const char* token=peek_native_scene_token_008d8a70(p,c.tokenizer);
    const int converted=c.calls.scan_decimal_00bf7533(token,c.decimal_format_00ce3a34,&value);
    *ok=static_cast<std::uint8_t>((converted==1&&value==0)||value==1);
    return value==1;
}
char* read_native_scene_nonempty_string_008d99f0(void* p,std::uint8_t* ok,NativeSceneTokenValueContext& c) {
    peek_native_scene_token_008d8a70(p,c.tokenizer);
    if(!byte(p,0x80a)&&(std::strlen(text(p))!=0||byte(p,4)!=0)) {
        if(std::strlen(peek_native_scene_token_008d8a70(p,c.tokenizer))!=0) {
            peek_native_scene_token_008d8a70(p,c.tokenizer);
            consume_native_scene_token_008d8960(p);
            *ok=1;return text(p);
        }
    }
    recover_native_scene_whitespace_008d8f70(p,c);
    *ok=0;return peek_native_scene_token_008d8a70(p,c.tokenizer);
}
bool read_native_scene_bool_008d9a80(void* p,std::uint8_t* ok,NativeSceneTokenValueContext& c) {
    const bool value=peek_native_scene_bool_008d8e50(p,ok,c);
    if(*ok) {
        consume_native_scene_token_008d8960(p);return value;
    }
    recover_native_scene_whitespace_008d8f70(p,c);*ok=0;return false;
}
std::int32_t read_native_scene_int_008d9ad0(void* p,std::uint8_t* ok,NativeSceneTokenValueContext& c) {
    std::int32_t value=owner_preimage(p);
    const char* token=peek_native_scene_token_008d8a70(p,c.tokenizer);
    const int converted=c.calls.scan_decimal_00bf7533(token,c.decimal_format_00ce3a34,&value);
    *ok=static_cast<std::uint8_t>(converted==1);
    if(converted==1) {
        consume_native_scene_token_008d8960(p);return value;
    }
    recover_native_scene_whitespace_008d8f70(p,c);*ok=0;return 0;
}
} // namespace bsp
