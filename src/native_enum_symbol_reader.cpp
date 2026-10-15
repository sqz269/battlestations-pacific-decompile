#include "bsp/native_enum_symbol_reader.hpp"
#include "bsp/entity_identity.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native enum symbol reader requires MSVC Win32.
#endif
namespace bsp {
namespace {
bool closing_token(const char* token,const char* close) noexcept {
    // REPE CMPSB with count2: never read the second pair after first mismatch.
    return token[0]==close[0]&&token[1]==close[1];
}
}
bool read_native_enum_symbols_008f31a0(void* owner,void* tokenizer,
    NativeEnumSymbolReaderContext& context) {
    auto& values=context.actual_token_values;
    char* token=peek_native_scene_token_008d8a70(tokenizer,values.tokenizer);
    while(!closing_token(token,context.actual_close_00ce4cd4)) {
        // Original re-peeks even though the loop check has already cached it.
        const char* source=peek_native_scene_token_008d8a70(tokenizer,values.tokenizer);
        char* destination=context.actual_symbol_scratch_00f89758;
        char current;
        do {
            current=*source++;
            *destination++=current;
        } while(current!=0);
        consume_native_scene_token_008d8960(tokenizer);
        std::uint8_t accepted=static_cast<std::uint8_t>(expect_native_scene_token_008d9930(
            tokenizer,context.actual_equals_00ce55bc,values));
        const std::int32_t value=read_native_scene_int_008d9ad0(tokenizer,&accepted,values);
        token=peek_native_scene_token_008d8a70(tokenizer,values.tokenizer);
        if(compare_insensitive_00438e10(token,context.actual_semicolon_00ce5698)==0)
            consume_native_scene_token_008d8960(tokenizer);
        declare_native_enum_symbol_008f2e40(owner,context.actual_symbol_scratch_00f89758,
            value,context.actual_enum_declaration);
        token=peek_native_scene_token_008d8a70(tokenizer,values.tokenizer);
    }
    return expect_native_scene_token_008d9930(tokenizer,context.actual_close_00ce4cd4,values);
}
} // namespace bsp
