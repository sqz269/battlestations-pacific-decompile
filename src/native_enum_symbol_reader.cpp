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

class RawEnumSymbolReader final : public SceneEnumSymbolReaderHost {
public:
    RawEnumSymbolReader(void* tokenizer, NativeEnumSymbolReaderContext& context)
        : tokenizer_(tokenizer), context_(context) {}
    const char* peek() override {
        return peek_native_scene_token_008d8a70(tokenizer_,
            context_.actual_token_values.tokenizer);
    }
    void consume() override { consume_native_scene_token_008d8960(tokenizer_); }
    bool expect(const char* text) override {
        return expect_native_scene_token_008d9930(tokenizer_, text, context_.actual_token_values);
    }
    std::int32_t read_int(std::uint8_t* accepted) override {
        return read_native_scene_int_008d9ad0(tokenizer_, accepted, context_.actual_token_values);
    }
    char* symbol_scratch() override { return context_.actual_symbol_scratch_00f89758; }
    const char* close_token() const override { return context_.actual_close_00ce4cd4; }
    const char* equals_token() const override { return context_.actual_equals_00ce55bc; }
    const char* semicolon_token() const override { return context_.actual_semicolon_00ce5698; }
    void declare_symbol(NativeSceneEnumOwnerStorage& owner, const char* text,
        std::int32_t value) override {
        declare_native_enum_symbol_008f2e40(&owner, text, value, context_.actual_enum_declaration);
    }
private:
    void* tokenizer_;
    NativeEnumSymbolReaderContext& context_;
};
}
bool read_native_enum_symbols_008f31a0(void* owner,void* tokenizer,
    NativeEnumSymbolReaderContext& context) {
    RawEnumSymbolReader host(tokenizer, context);
    return read_native_enum_symbols_008f31a0(
        *static_cast<NativeSceneEnumOwnerStorage*>(owner), host);
}
bool read_native_enum_symbols_008f31a0(NativeSceneEnumOwnerStorage& owner,
    SceneEnumSymbolReaderHost& host) {
    const char* token=host.peek();
    while(!closing_token(token,host.close_token())) {
        // Original re-peeks even though the loop check has already cached it.
        const char* source=host.peek();
        char* destination=host.symbol_scratch();
        char current;
        do {
            current=*source++;
            *destination++=current;
        } while(current!=0);
        host.consume();
        std::uint8_t accepted=static_cast<std::uint8_t>(host.expect(host.equals_token()));
        const std::int32_t value=host.read_int(&accepted);
        token=host.peek();
        if(compare_insensitive_00438e10(token,host.semicolon_token())==0)
            host.consume();
        host.declare_symbol(owner,host.symbol_scratch(),value);
        token=host.peek();
    }
    return host.expect(host.close_token());
}
} // namespace bsp
