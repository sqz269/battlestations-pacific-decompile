#pragma once
#include "bsp/gui_material_binding.hpp"
#include "bsp/native_material_parameters.hpp"
#include <exception>
#include <memory>

namespace bsp::game {
class GameNativeRendererApplication;
class GameNativeGuiEffectApplication;

// A borrow from the page's sole raw Line allocation/terminal authority. The
// reference aliases actual+04; page_token owns this SAME widget/layout and
// actual storage, get()==&widget. No allocation, registration or retain here.
struct GameNativeGuiLineIdentity {
    GuiWidgetOwner& widget;
    void* actual;
    RenderCommandReference& reference;
    std::shared_ptr<const void> page_token;
};
struct GameNativeGuiLineApplicationBindings {
    GuiWidgetOwnerRuntime& widgets;
    NativeMaterialDestructionAccess& materials;
    const GuiMaterialBindingServices& colors;
    const volatile float& one_00d7a24c;
    const volatile float& vertical_scale_00e12ff4;
    std::function<GameNativeGuiLineIdentity(GuiWidgetOwner&)> actual_identity;
    // Observe completed Step2 operation0: manager_offset28, reference_owned,
    // exact B319B0 result. The caller retains that SAME resource application
    // through all Line/material consumers. This explicit source service replaces
    // ONLY AC260D's manager getter/AC2612 slot load; it creates no GUI manager.
    std::function<void*()> initial_manager_texture_28;
};

// Actual Line GPU operations over the SAME raw Line, owner Model and native
// mesh/stream/section/material/layout domains. Point-vector storage/counters are
// read/written in the actual Line slot; no projected polyline/vertex owner.
// No widget constructor, point allocation, scalar deletion, factory arm, page,
// Screen/manager or second render queue is provided. Those belong to the
// independent canonical raw Line/page authority. New C++ service ABI; not
// native private register/FH3/SEH, malformed-pointer or game validation.
class GameNativeGuiLineApplication final {
public:
    GameNativeGuiLineApplication(GameNativeRendererApplication&,
        GameNativeGuiEffectApplication&, GameNativeGuiLineApplicationBindings);
    ~GameNativeGuiLineApplication() noexcept;
    GameNativeGuiLineApplication(const GameNativeGuiLineApplication&) = delete;
    GameNativeGuiLineApplication& operator=(const GameNativeGuiLineApplication&) = delete;

    void constructed74_00ac2430(GuiWidgetOwner&);
    void ensure_capacity_00ac2690(GuiWidgetOwner&, std::int32_t requested);
    void flush80_00ac29d0(GuiWidgetOwner&);
    void reset7c_00ac2950(GuiWidgetOwner&);
    // Full current78: SAME baseAA7170, current50 AA6870(current color), then80.
    void loaded78_00ac1bb0(GuiWidgetOwner&);

    // AFTER model/scene/queued resource retirement and before raw scalar entry.
    // Drops only our host page token. No vector free, native release or slot
    // return occurs. release_retired_line then forgets diagnostic metadata while
    // the authoritative page still keeps the same raw identity/layout alive.
    void release_line_borrow_guard(GuiWidgetOwner&);
    void release_retired_line(GuiLayoutWidget&);
    bool requires_process_retention() const noexcept;
    std::uint32_t failure_entry() const noexcept;
    const std::exception_ptr& failure() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
