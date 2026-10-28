#include "bsp/game_native_gui_text_state.hpp"
#include <cstring>
#include <limits>

namespace bsp::game {
namespace {
static_assert(sizeof(float) == sizeof(std::uint32_t) && std::numeric_limits<float>::is_iec559);
float image_float(std::uint32_t word) noexcept {
    float result;
    std::memcpy(&result, &word, sizeof(result));
    return result;
}
} // namespace

// Exact initialized .data words: E12FC0=3FAAAAAB, E12FD4=3F400000,
// E12FD8..E12FE4={0,0,0,3F400000}. F8BE38[4], F8BE48 and F8BE54
// are in the .data virtual tail, outside SizeOfRawData: loader positive zero.
// Evidence: docs/CC12_NATIVE_GUI_TEXT_STATE_38.md and its installed PE hash.
// Base-widget additions: E12FC4=3F400000; F8BCCC[4]/F8BCDC and
// F8BCE0[4]/F8BCF0 are also loader-zero .data virtual-tail cells. Keep them
// distinct from F8BE38 and E12FD4. No lazy guard/color initializer runs here.
// Evidence: docs/CC15_GUI4_P3A2.md, verified installed DATA and live listing.
GameNativeGuiTextState::GameNativeGuiTextState() noexcept
    : aspect_ratio_(image_float(0x3faaaaabu)),
      vertical_scale_(image_float(0x3f400000u)),
      shadow_color_{image_float(0), image_float(0), image_float(0), image_float(0x3f400000u)},
      widget_y_scale_(image_float(0x3f400000u)) {}

float (&GameNativeGuiTextState::white_color_00ab7060(
    const volatile float& one_00d7a24c) noexcept)[4] {
    if ((default_color_mask_ & 1u) == 0) {
        const float one = one_00d7a24c; // 706D precedes mask OR and all four stores.
        default_color_mask_ |= 1u;
        for (auto& channel : default_color_) std::memcpy(&channel, &one, sizeof(channel));
    }
    return default_color_;
}

GameNativeGuiTextState& game_native_gui_text_state() {
    static auto* const process = new GameNativeGuiTextState;
    return *process;
}
} // namespace bsp::game
