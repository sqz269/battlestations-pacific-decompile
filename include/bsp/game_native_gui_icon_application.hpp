#pragma once
#include "bsp/gui_material_binding.hpp"
#include "bsp/native_material_parameters.hpp"
#include <exception>
#include <memory>

namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameNativeGuiEffectApplication;

// Borrow the actual Icon allocation's existing canonical companion. The page
// allocation/terminal transport supplies it, with SAME +04 and current D5C4C0.
// This association performs no allocation/initialization/retain or registration.
struct GameNativeGuiIconIdentity {
    GuiWidgetOwner& widget;
    void* actual;
    RenderCommandReference& reference;
};
struct GameNativeGuiIconApplicationBindings {
    GuiWidgetOwnerRuntime& widgets;
    NativeMaterialDestructionAccess& materials;
    NativeMaterialParameterAccess& parameters;
    // SAME canonical widget/material domain and authoritative page token.
    // retain_widget must keep this owner's sole layout and parameter fields
    // alive; get()==&owner. No standalone copy of a page layout is accepted.
    const GuiMaterialBindingServices& widget_materials;
    const bool& crt_sse2_conversion;
    const float& vertical_scale_00e12fd0;
    const GuiIconStateTextureConstants& texture_constants;
    // Borrow the actual loaded GUI atlas's stable item view. Items retain their
    // SAME actual textures in the supplied renderer domain; no second atlas.
    std::function<const TextureAtlasItem*(std::string_view)> find_atlas_item_00aefb20;
    std::function<bool()> platform_allows_point_filter;
    std::function<GameNativeGuiIconIdentity(GuiWidgetOwner&)> actual_identity;
};

// Deferred type6 factory over one existing GuiWidgetOwnerRuntime. Reuses Icon's
// current74/property18/current78/state logic, but publishes ONLY canonical raw
// mesh/stream/section/material/layout objects. Real renderer38/5C, HAL mapping,
// numeric535320 effects, AA9F10/B18A40 and current50 are composed here.
//
// No manager/page, widget allocation/terminal, transform, count, index buffer,
// render queue or runtime switch is created. The original Icon uses a four-
// vertex nonindexed strip. Raw Icon identity plus authoritative page lifetime
// are required external prerequisites; absence fails before native effects.
// Keep all providers through native retirement and retained failure frames.
// New MSVC Win32 C++ ABI, not native widget/container/FH3/SEH ABI parity.
class GameNativeGuiIconApplication final {
public:
    GameNativeGuiIconApplication(GameHostLog&, GameNativeRendererApplication&,
        GameNativeGuiEffectApplication&, GameNativeGuiIconApplicationBindings);
    ~GameNativeGuiIconApplication() noexcept;
    GameNativeGuiIconApplication(const GameNativeGuiIconApplication&) = delete;
    GameNativeGuiIconApplication& operator=(const GameNativeGuiIconApplication&) = delete;

    std::unique_ptr<GuiWidgetTypeImplementation> make_type(GuiWidgetOwner&);
    // AFTER genuine scene/model/queued-material retirement, BEFORE terminal
    // page/type destruction. Requires node==null and actual Icon+04==1 (only
    // the authoritative creator remains). Drops our parameter-field page token
    // so the page authority can admit its one terminal retirement.
    void release_material_borrow_guard(GuiWidgetOwner&);
    // Pure metadata completion AFTER this same Icon implementation has died,
    // all its actual material borrowers retired, and while its sole layout and
    // raw identity remain valid in the page's authoritative teardown transport.
    // It never deletes/returns raw slots or changes a native reference count.
    void release_retired_icon(GuiLayoutWidget& same_layout);
    bool requires_process_retention() const noexcept;
    std::uint32_t failure_entry() const noexcept;
    const std::exception_ptr& failure() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
