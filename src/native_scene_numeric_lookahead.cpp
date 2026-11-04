#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "bsp/native_scene_numeric_lookahead.hpp"
#include "bsp/engine_seg56_cc13.hpp"

#include <cctype>
#include <cstdio>
#include <cstring>

namespace bsp::cc13_seg56 {
// 008D8F10, the existing scene_token_is_int body moved from engine_seg56_cc13.cpp
// with its address added to the identifier; the predicate itself is unchanged.
// Evidence: bsp.py show 008d8f10: peek, sscanf("%d"), assignment-count == 1.
// This interface consumes the real SceneLexer token already peeked by its
// caller. It preserves decimal-prefix acceptance; it does not consume tokens
// or convert/store the ignored integer, and supplies no native tokenizer ABI.
bool scene_token_is_int_008d8f10(const std::string& token)
{
    // sscanf("%d") assigns a field when, after optional white space and one sign, a digit
    // follows; overflow still counts as an assignment.
    std::size_t i = 0;
    while (i < token.size() && std::isspace(static_cast<unsigned char>(token[i]))) {
        ++i;
    }
    if (i < token.size() && (token[i] == '+' || token[i] == '-')) {
        ++i;
    }
    return i < token.size() && std::isdigit(static_cast<unsigned char>(token[i])) != 0;
}
} // namespace bsp::cc13_seg56

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
