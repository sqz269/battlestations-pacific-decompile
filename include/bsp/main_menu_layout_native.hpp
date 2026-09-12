#pragma once
#include "bsp/main_menu_layout_binding.hpp"
#include <functional>

namespace bsp {
// Concrete derived-widget operations for the existing5861B0 layout binder.
// Every method resolves the SAME canonical owner. Movie storage is supplied
// by its actual owner; these GUI bindings do not fabricate a video player.
class MainMenuCanonicalLayoutNativeCalls final : public MainMenuLayoutNativeCalls {
public:
    MainMenuCanonicalLayoutNativeCalls(GuiWidgetOwnerRuntime&,
        std::function<MovieWidgetState&(GuiWidgetOwner&)> actual_movie_state);
    MovieWidgetState& movie_state(GuiLayoutWidget&) override;
    void call_00a9ac40(GuiLayoutWidget&, void* screen_plus_8) override;
    GuiTextColor color54(GuiLayoutWidget&) override;
    void text_color50(GuiLayoutWidget&, const GuiTextColor&) override;
    void text_state80(GuiLayoutWidget&, std::int32_t) override;
private:
    GuiWidgetOwnerRuntime& owners_;
    std::function<MovieWidgetState&(GuiWidgetOwner&)> actual_movie_state_;
};
} // namespace bsp
