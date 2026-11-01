#pragma once
#include "bsp/gui_texture.hpp"

namespace bsp { class NativeRenderActualOwnerRegistry; }
namespace bsp::game {
// Narrow captured-only view over the SAME Root renderer registry. This object
// creates no Native owner/count or lifetime pin. Keep it and every actual
// texture/atomic/companion/provider alive through calls, failure and last use.
class GameNativeCapturedTextureDimensions final {
public:
    explicit GameNativeCapturedTextureDimensions(NativeRenderActualOwnerRegistry&);
    ~GameNativeCapturedTextureDimensions() noexcept = default;
    GameNativeCapturedTextureDimensions(const GameNativeCapturedTextureDimensions&) = delete;
    GameNativeCapturedTextureDimensions& operator=(const GameNativeCapturedTextureDimensions&) = delete;
    GameNativeCapturedTextureDimensions(GameNativeCapturedTextureDimensions&&) = delete;
    GameNativeCapturedTextureDimensions& operator=(GameNativeCapturedTextureDimensions&&) = delete;
    // Only dimension_from_captured is bound. The five legacy routes remain
    // unbound; this view is not a complete legacy GuiTextureCallbacks provider.
    const GuiTextureCallbacks& captured_callbacks() const noexcept;
private:
    std::uint32_t dimension(void* actual_texture, std::uint32_t captured_profile,
        std::uint32_t captured_target) const;
    NativeRenderActualOwnerRegistry& owners_;
    GuiTextureCallbacks callbacks_;
};
} // namespace bsp::game
