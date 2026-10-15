#include "bsp/game_native_scene_file_tokenizer.hpp"
#include "bsp/game_native_vfs_runtime.hpp"

namespace bsp::game {
GameNativeSceneFileTokenizerBindings::GameNativeSceneFileTokenizerBindings(
    const GameNativeVfsRawServices& vfs, NativeStringRawPoolContext& strings,
    const char* const volatile& whitespace, const char* const volatile& delimiters,
    const char* unknown_file, const volatile std::uint32_t& feature) noexcept
    : calls_(vfs.bindings),
      file_{strings, vfs.retained_memory, calls_, vfs.actual_vfs_publication_0109ceec,
          delimiters, feature},
      tokenizer_{strings, vfs.retained_memory, calls_, whitespace, delimiters, unknown_file} {}

void* GameNativeSceneFileTokenizerBindings::construct(void* owner,
    NativeString& filename, const char* extra) {
    return construct_native_scene_file_tokenizer_008d9cf0(owner, filename, extra, file_);
}
NativeSceneTokenizerContext& GameNativeSceneFileTokenizerBindings::tokenizer_context() noexcept {
    return tokenizer_;
}
} // namespace bsp::game
