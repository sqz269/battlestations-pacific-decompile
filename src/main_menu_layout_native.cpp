#include "bsp/main_menu_layout_native.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
GuiTextRuntimeImplementation& actual_text(GuiWidgetOwner& owner) {
    auto* implementation = dynamic_cast<GuiTextRuntimeImplementation*>(&owner.implementation());
    if (!implementation || !owner.text_lifetime() ||
        &implementation->lifetime() != owner.text_lifetime())
        throw std::logic_error("Main-menu Text dispatch requires its actual canonical lifetime");
    return *implementation;
}
}
MainMenuCanonicalLayoutNativeCalls::MainMenuCanonicalLayoutNativeCalls(
    GuiWidgetOwnerRuntime& owners, std::function<MovieWidgetState&(GuiWidgetOwner&)> movie)
    : owners_(owners), actual_movie_state_(std::move(movie)) {}
MovieWidgetState& MainMenuCanonicalLayoutNativeCalls::movie_state(GuiLayoutWidget& layout) {
    if (!actual_movie_state_)
        throw std::logic_error("Main-menu movie lookup requires actual movie-owner storage");
    return actual_movie_state_(owners_.owner(layout));
}
void MainMenuCanonicalLayoutNativeCalls::call_00a9ac40(
    GuiLayoutWidget& layout, void* screen_plus_8) {
    auto& owner = owners_.owner(layout);
    auto* implementation = dynamic_cast<GuiListboxTypeImplementation*>(&owner.implementation());
    if (!implementation || &implementation->runtime().owner() != &owner)
        throw std::logic_error("Main-menu listener binding requires the actual Listbox companion");
    implementation->runtime().set_listener_00a9ac40(screen_plus_8);
}
GuiTextColor MainMenuCanonicalLayoutNativeCalls::color54(GuiLayoutWidget& layout) {
    auto& owner = owners_.owner(layout);
    float rgba[4];
    owner.implementation().read_color54(owner, rgba);
    GuiTextColor color;
    static_assert(sizeof(color) == sizeof(rgba));
    std::memcpy(&color, rgba, sizeof(color));
    return color;
}
void MainMenuCanonicalLayoutNativeCalls::text_color50(
    GuiLayoutWidget& layout, const GuiTextColor& color) {
    float rgba[4];
    static_assert(sizeof(color) == sizeof(rgba));
    std::memcpy(rgba, &color, sizeof(rgba));
    actual_text(owners_.owner(layout)).set_color50_00ab6b50(rgba);
}
void MainMenuCanonicalLayoutNativeCalls::text_state80(GuiLayoutWidget& layout, std::int32_t state) {
    actual_text(owners_.owner(layout)).set_state80_00ab7200(state);
}
} // namespace bsp
