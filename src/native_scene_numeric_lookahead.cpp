#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "bsp/native_scene_numeric_lookahead.hpp"

#include <cstdio>
#include <cstring>

namespace bsp {
static_assert(sizeof(std::uintptr_t) == 4 && sizeof(int) == 4 && sizeof(float) == 4);

bool next_native_scene_token_is_int_008d8f10(void* tokenizer,
    NativeSceneTokenizerContext& context) {
    int ignored;
    const auto owner_bits = reinterpret_cast<std::uintptr_t>(tokenizer);
    std::memcpy(&ignored, &owner_bits, sizeof ignored);
    const char* const token = peek_native_scene_token_008d8a70(tokenizer, context);
    return std::sscanf(token, "%d", &ignored) == 1;
}

bool next_native_scene_token_is_float_008d8f40(void* tokenizer,
    NativeSceneTokenizerContext& context) {
    float ignored;
    const auto owner_bits = reinterpret_cast<std::uintptr_t>(tokenizer);
    std::memcpy(&ignored, &owner_bits, sizeof ignored);
    const char* const token = peek_native_scene_token_008d8a70(tokenizer, context);
    return std::sscanf(token, "%f", &ignored) == 1;
}

} // namespace bsp
