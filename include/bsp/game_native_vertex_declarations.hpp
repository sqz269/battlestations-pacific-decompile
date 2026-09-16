#pragma once
#include "bsp/native_vertex_declaration_loading.hpp"
#include <array>
#include <cstddef>

namespace bsp::game {
class GameNativeReadOnlyData;
// Canonical process token cells and the decoder's actual pool/string domain.
// Lazy native initialization and its two real CRT callbacks remain in B2DBD0.
class GameNativeVertexDeclarationsProcess final {
public:
    NativeVertexDeclarationLoadingContext& loading() noexcept { return loading_; }
    GameNativeVertexDeclarationsProcess(const GameNativeVertexDeclarationsProcess&) = delete;
    GameNativeVertexDeclarationsProcess& operator=(const GameNativeVertexDeclarationsProcess&) = delete;
private:
    friend GameNativeVertexDeclarationsProcess& game_native_vertex_declarations_process(GameNativeReadOnlyData&);
    explicit GameNativeVertexDeclarationsProcess(GameNativeReadOnlyData&);
    static int register_cleanup(void*, std::uint32_t) noexcept;
    static void destroy_types() noexcept;
    static void destroy_usages() noexcept;
    GameNativeReadOnlyData& data_;
    alignas(4) std::array<std::byte, 17 * 12> types_0108d5a8_{};
    alignas(4) std::array<std::byte, 8 * 12> usages_0108d678_{};
    volatile std::uint32_t initialized_0108d6d8_{};
    volatile std::int32_t type_count_00e13070_{17};
    volatile std::int32_t usage_count_00e13074_{8};
    NativeVertexDeclarationLoadingContext loading_;
};
// Intentionally retained through CRT callbacks, including callbacks after the
// application's raw manager drain. No destructor repeats token cleanup.
GameNativeVertexDeclarationsProcess& game_native_vertex_declarations_process(GameNativeReadOnlyData&);
} // namespace bsp::game
