#pragma once
#include <array>
#include <cstdint>

namespace bsp::game {
// Source storage for the loader-zero vector at F87574/F87578/F8757C.
// References escape in the native program: preserve mutable, contiguous cells
// and process lifetime rather than replacing reads with constant zeroes.
class GameNativeGeometryGlobals final {
public:
    GameNativeGeometryGlobals(const GameNativeGeometryGlobals&) = delete;
    GameNativeGeometryGlobals& operator=(const GameNativeGeometryGlobals&) = delete;
    std::array<std::uint32_t,3>& zero_vector_00f87574() noexcept { return zero_vector_; }
private:
    friend GameNativeGeometryGlobals& game_native_geometry_globals();
    GameNativeGeometryGlobals() = default;
    std::array<std::uint32_t,3> zero_vector_{};
};
// No reset or renderer lifecycle is hidden behind this process accessor.
GameNativeGeometryGlobals& game_native_geometry_globals();
} // namespace bsp::game
