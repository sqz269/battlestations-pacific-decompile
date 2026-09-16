#include "bsp/game_native_vertex_declarations.hpp"
#include "bsp/game_native_graphics_pools.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include <cstdlib>
#include <exception>
#include <stdexcept>

namespace bsp::game {
namespace { GameNativeVertexDeclarationsProcess* process; }
GameNativeVertexDeclarationsProcess::GameNativeVertexDeclarationsProcess(GameNativeReadOnlyData& data)
    : data_(data), loading_{game_native_string_process().strings(),
          &game_native_graphics_pool_process().vertex_declaration_pool_0108fd38(),
          static_cast<const std::uint32_t*>(data.data_at(0x00d61cc0, 18 * 4)),
          static_cast<const std::uint32_t*>(data.data_at(0x00d61d1c, 8)),
          usages_0108d678_.data(), types_0108d5a8_.data(), initialized_0108d6d8_,
          usage_count_00e13074_, type_count_00e13070_, this, &register_cleanup} {}
int GameNativeVertexDeclarationsProcess::register_cleanup(void* opaque, std::uint32_t token) noexcept {
    if (opaque != process) std::terminate();
    if (token == 0x00ce0c10) return std::atexit(&destroy_types);
    if (token == 0x00ce0c30) return std::atexit(&destroy_usages);
    std::terminate();
}
void GameNativeVertexDeclarationsProcess::destroy_types() noexcept {
    destroy_native_vertex_format_types_00ce0c10(process->loading_);
}
void GameNativeVertexDeclarationsProcess::destroy_usages() noexcept {
    destroy_native_vertex_format_usages_00ce0c30(process->loading_);
}
GameNativeVertexDeclarationsProcess& game_native_vertex_declarations_process(GameNativeReadOnlyData& data) {
    static auto* const retained = [](GameNativeReadOnlyData& owner) {
        auto* const created = new GameNativeVertexDeclarationsProcess(owner);
        process = created;
        return created;
    }(data);
    if (&retained->data_ != &data) throw std::logic_error("vertex tokens require the canonical immutable data owner");
    return *retained;
}
} // namespace bsp::game
