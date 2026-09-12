#pragma once
// 00aa5e20: retained GUI manager resource state and initialization sequence.
// Original ABI: ECX = manager, no stack arguments, RET. New C++ API only.
// Names are hypotheses; evidence and ownership limits: docs/GUI_RESOURCE_OWNER.md.
#include <cstdint>
#include <array>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

#include "bsp/gui_layout_loader.hpp"

namespace bsp {
struct GuiManagerFrameServices;

// An opaque renderer texture identity, not a reconstructed renderer object.
// load_texture returns the native reference without an additional retain here.
struct GuiResourceTexture {
    void* value{nullptr};
    explicit operator bool() const noexcept { return value != nullptr; }
};

struct GuiResourceCallbacks {
    // 00bdf4c0 can rewrite the name. Only whiteGui takes this resolution path.
    std::function<bool(std::string&)> resolve_existing_name;
    // Renderer +64h, with flags == 0. A null result is still stored natively.
    std::function<GuiResourceTexture(std::string_view, std::uint32_t)> load_texture;
    // Atomic decrement of texture +4h; destroy is its virtual +0h when zero.
    std::function<std::int32_t(GuiResourceTexture)> decrement_texture_reference;
    std::function<void(GuiResourceTexture)> destroy_texture;
    // Widget virtual +34h, including the concrete class's scene/visibility work.
    // Updating GuiLayoutWidget::visible alone does not implement this callback.
    std::function<void(GuiLayoutWidget&, bool)> set_visibility;
};

// One canonical manager pointer-field owner. Disengaged values record native
// storage that has not yet been produced; zero is not a substitute preimage.
struct GuiManagerPointerFields {
    std::uint8_t enabled_48{};
    std::optional<std::array<float, 2>> position_5c;
    std::optional<std::array<float, 2>> delta_64;
    GuiLayoutWidget* exclusive_page_6c{};
};

struct GuiResourceState {
    GuiManagerPointerFields pointer;
    GuiResourceTexture white_gui{};        // manager +28h; raw assignment
    GuiResourceTexture transparent{};      // +2Ch; old reference released on replace
    GuiLayoutPage* mouse_page{nullptr};    // +4Ch; borrowed from page registry
    GuiLayoutWidget* current_cursor{nullptr};  // +50h; initially aliases +54h
    GuiLayoutWidget* frontend_cursor{nullptr}; // +54h
    GuiLayoutWidget* gui_cursor{nullptr};      // +58h
    GuiLayoutWidget* highlight_frame{nullptr}; // +74h
    GuiLayoutWidget* highlight_circle{nullptr}; // +78h
    GuiLayoutWidget* safezone_43{nullptr};      // +7Ch
    GuiLayoutWidget* safezone_169{nullptr};     // +80h
    // The native constructor leaves +84h unwritten. It is valid here only
    // after initialize_00aa5e20 returns; completion is separate C++ bookkeeping.
    std::uint8_t ready_flag{0};             // +84h; initialization writes zero last
    // Constructor AA5D70 leaves+70 unwritten. AA4F80 publishes its raw second
    // argument before callbacks and zero on normal completion only.
    std::optional<std::uint8_t> blocked_70;
};

// Holds the manager's persistent resource slots, composing the existing page
// registry/layout loader. Registry and callbacks' backing services must outlive
// this object. Texture teardown outside 00aa5e20 belongs to the caller: this
// bounded owner performs exactly the observed transparent replacement release,
// and does not infer a destructor or release borrowed page/child references.
class GuiResourceOwner {
public:
    GuiResourceOwner(GuiPageRegistry& registry, GuiLayoutHost& layout,
        GuiResourceCallbacks callbacks, GuiResourceState initial_state = {});
    GuiResourceOwner(const GuiResourceOwner&) = delete;
    GuiResourceOwner& operator=(const GuiResourceOwner&) = delete;
    GuiResourceOwner(GuiResourceOwner&&) = delete;
    GuiResourceOwner& operator=(GuiResourceOwner&&) = delete;

    // No null-skipping of required pages/children. A missing object throws at
    // its native dereference boundary, keeping already-performed state changes.
    // Missing callback contracts are rejected by the constructor before work.
    void initialize_00aa5e20();
    const GuiResourceState& state() const noexcept { return state_; }
    GuiManagerPointerFields& pointer_fields() noexcept { return state_.pointer; }
    bool initialization_completed() const noexcept { return initialized_; }
    GuiPageRegistry& pages() noexcept { return registry_; }

private:
    friend void update_gui_manager_00aa4f80(GuiResourceOwner&, float,
        std::uint8_t, const GuiManagerFrameServices&);
    void show_required(GuiLayoutWidget* widget, bool visible, std::string_view name);
    GuiPageRegistry& registry_;
    GuiLayoutHost& layout_;
    GuiResourceCallbacks callbacks_;
    GuiResourceState state_;
    bool initialized_{false};
};

} // namespace bsp
