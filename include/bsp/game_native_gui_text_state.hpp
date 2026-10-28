#pragma once
#include <cstdint>

namespace bsp::game {
// Sole typed Source backing for the recovered mutable GUI/Text cells. Initial
// words are the supported Original PE's .data preimages or loader zero fill;
// these addresses are outside GameNativeReadOnlyData. This is not storage at
// the Original VAs, an Original object layout, or a game ABI replacement.
//
// All GUI material, clip, Text constructor/property/layout/content services
// must borrow these SAME references. No per-widget white quartet, scalar copy,
// early white initialization or reset is supplied. Serialize all access with
// GUI construction, property reads, rendering and teardown. This process owner
// remains alive through retained material parameters and CRT callbacks.
class GameNativeGuiTextState final {
public:
    GameNativeGuiTextState(const GameNativeGuiTextState&) = delete;
    GameNativeGuiTextState& operator=(const GameNativeGuiTextState&) = delete;

    float& aspect_ratio_00e12fc0() noexcept { return aspect_ratio_; }
    float& vertical_scale_00e12fd4() noexcept { return vertical_scale_; }
    float (&shadow_color_00e12fd8() noexcept)[4] { return shadow_color_; }
    float (&default_color_00f8be38() noexcept)[4] { return default_color_; }
    std::uint32_t& default_color_mask_00f8be48() noexcept { return default_color_mask_; }
    float& blend_factor_00f8be54() noexcept { return blend_factor_; }

    // Base-widget AAA710/AAAED0 cells, distinct from Text's F8BE38 quartet.
    // These are raw backing views only. The existing property body performs
    // its CURRENT guard tests, OR and ordered stores; borrowing never warms
    // either color or resets a guard. Every raw widget property provider must
    // use this same process backing, including callbacks between reads.
    float (&widget_white_color_00f8bce0() noexcept)[4] { return widget_white_color_; }
    std::uint32_t& widget_white_guard_00f8bcf0() noexcept { return widget_white_guard_; }
    float (&widget_black_color_00f8bccc() noexcept)[4] { return widget_black_color_; }
    std::uint32_t& widget_black_guard_00f8bcdc() noexcept { return widget_black_guard_; }
    // AA7220's live Y translation/pivot multiplier. This is the distinct
    // initialized E12FC4 cell, not Text's E12FD4 vertical scale or a copy of it.
    float& widget_y_scale_00e12fc4() noexcept { return widget_y_scale_; }

    // Complete 00AB7060..70A0 behavior over this SAME backing. The Original
    // no-argument entry returns F8BE38 in EAX, plain RET. This new typed
    // interface instead borrows the caller's established D7A24C float cell.
    // Mask bit0 set: no load/store. Otherwise load one, OR mask with 1, store
    // all four channels in order, then return the same quartet. Preserve all
    // other mask bits. Existing ABB630 inline writers use the accessors above.
    float (&white_color_00ab7060(const volatile float& one_00d7a24c) noexcept)[4];

private:
    friend GameNativeGuiTextState& game_native_gui_text_state();
    GameNativeGuiTextState() noexcept;
    float aspect_ratio_;
    float vertical_scale_;
    float shadow_color_[4];
    float default_color_[4]{};
    std::uint32_t default_color_mask_{};
    float blend_factor_{};
    float widget_white_color_[4]{};
    std::uint32_t widget_white_guard_{};
    float widget_black_color_[4]{};
    std::uint32_t widget_black_guard_{};
    float widget_y_scale_;
};

// Intentionally retained process lifetime, including after GUI/font retirement.
// No Text lineage, Text pool, type-counter or native Text factory is created.
GameNativeGuiTextState& game_native_gui_text_state();
} // namespace bsp::game
