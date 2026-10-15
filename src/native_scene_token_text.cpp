#include "bsp/native_scene_token_text.hpp"
#include "bsp/entity_identity.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene token text wrappers require MSVC Win32.
#endif
namespace bsp {
namespace {
std::uint8_t byte(void* owner,std::uint32_t offset) noexcept {
    return *reinterpret_cast<const std::uint8_t*>(
        reinterpret_cast<std::uint32_t>(owner)+offset);
}
char* text(void* owner) noexcept {
    return reinterpret_cast<char*>(reinterpret_cast<std::uint32_t>(owner)+5);
}
}

bool expect_native_scene_token_008d9930(void* owner,const char* expected,
    NativeSceneTokenValueContext& context) {
    const char* const token=peek_native_scene_token_008d8a70(owner,context.tokenizer);
    if(compare_insensitive_00438e10(token,expected)==0) {
        consume_native_scene_token_008d8960(owner);
        return true;
    }
    recover_native_scene_whitespace_008d8f70(owner,context);
    return false;
}

char* read_native_scene_token_008d9980(void* owner,
    NativeSceneTokenValueContext& context) {
    peek_native_scene_token_008d8a70(owner,context.tokenizer);
    if(byte(owner,0x80a)==0) {
        char* const token=text(owner);
        // Original scans before consulting +4, including the empty case. Keep
        // its 32-bit cursor arithmetic and every read through the final NUL.
        std::uint32_t cursor=reinterpret_cast<std::uint32_t>(token);
        const std::uint32_t after_first=cursor+1;
        std::uint8_t current;
        do {
            current=*reinterpret_cast<const std::uint8_t*>(cursor);
            cursor+=1;
        } while(current!=0);
        if(cursor!=after_first||byte(owner,4)!=0) {
            consume_native_scene_token_008d8960(owner);
            return token;
        }
    }
    recover_native_scene_whitespace_008d8f70(owner,context);
    return peek_native_scene_token_008d8a70(owner,context.tokenizer);
}
} // namespace bsp
