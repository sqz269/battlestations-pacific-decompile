#pragma once
#include "bsp/native_scene_file_tokenizer.hpp"

namespace bsp::game {
struct GameNativeVfsRawServices;

// Owns only Source calls/contexts, not a native tokenizer/VFS/pool/stream.
// Borrow the SAME initialized VFS services/raw strings and genuine current
// E0C940/E0C944/0109EEA4 cells. Retain this binding and all services through
// every constructed tokenizer's explicit destruction, before shared drain.
// Construction performs no native startup, publication, allocation or I/O.
class GameNativeSceneFileTokenizerBindings final {
public:
    GameNativeSceneFileTokenizerBindings(const GameNativeVfsRawServices&,
        NativeStringRawPoolContext& actual_strings,
        const char* const volatile& whitespace_00e0c940,
        const char* const volatile& default_delimiters_00e0c944,
        const char* unknown_file_00d15fe8,
        const volatile std::uint32_t& actual_crt_feature_word_0109eea4) noexcept;
    GameNativeSceneFileTokenizerBindings(const GameNativeSceneFileTokenizerBindings&) = delete;
    GameNativeSceneFileTokenizerBindings& operator=(const GameNativeSceneFileTokenizerBindings&) = delete;
    GameNativeSceneFileTokenizerBindings(GameNativeSceneFileTokenizerBindings&&) = delete;
    GameNativeSceneFileTokenizerBindings& operator=(GameNativeSceneFileTokenizerBindings&&) = delete;
    void* construct(void* actual_838h_owner, NativeString& consumed_filename,
        const char* extra_delimiters);
    // Use this SAME context for accepted token operations and explicit
    // 8D9C30/8D9F00 destruction. Fresh destruction operation supplied by caller.
    NativeSceneTokenizerContext& tokenizer_context() noexcept;
private:
    NativeSceneFileTokenizerVfsCalls calls_;
    NativeSceneFileTokenizerContext file_;
    NativeSceneTokenizerContext tokenizer_;
};
} // namespace bsp::game
