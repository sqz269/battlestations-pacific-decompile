#pragma once
#include "bsp/gui_native_scene.hpp"
#include <exception>
#include <memory>

namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameVfsHost;
class GameNativeReadOnlyData;

// Application composition of existing AA6640/B75030/B8F5E0 bodies. The
// renderer supplies its SAME node/scene/reference domain; VFS supplies its
// SAME string storage and initialized type descriptors; process startup owns
// the distinct 01090054 Model and 010902F4 Group pools. No native manager,
// page/widget projection, transform, hierarchy or reference count is added.
// All borrowed applications/constants/factory captures survive native drain.
// New MSVC Win32 C++ API, not the original pooled-widget or FH3/SEH ABI.
class GameNativeGuiNodeApplication final {
public:
    // Supply a factory before allocating widgets. It may defer its Text arm
    // to a subsequently constructed GameNativeTextApplication borrowing
    // widgets()/parenting(). No factory call or native construction occurs here.
    GameNativeGuiNodeApplication(GameHostLog&, GameNativeRendererApplication&,
        GameVfsHost&, GameNativeReadOnlyData&, GuiWidgetImplementationFactory);
    ~GameNativeGuiNodeApplication() noexcept;
    GameNativeGuiNodeApplication(const GameNativeGuiNodeApplication&) = delete;
    GameNativeGuiNodeApplication& operator=(const GameNativeGuiNodeApplication&) = delete;

    GuiWidgetOwnerRuntime& widgets() noexcept;
    NativeNodeParentingRuntime& parenting() noexcept;
    GuiNativeScene& scene() noexcept;
    NativeModelEnvironment& models() noexcept;

    // Retain the sole authored layout BEFORE entering the actual Text factory,
    // AA6560 Model allocation/binding and current74 (AA6640). Caller establishes
    // the constructor-unwritten authored_x/visible preimages. No page, script,
    // parent, properties, current78 or render submission is synthesized.
    // The returned owner's model_reference()/node_binding() are canonical.
    GuiWidgetOwner& create_standalone_text(std::unique_ptr<GuiLayoutWidget>);

    // Genuine same-owner C++ lifetime token for GuiMaterialBindingServices.
    // get()==&owner; it owns this runtime AND the sole owning layout record.
    // It adds no native retain/count and is supported only for app-owned Texts.
    // Release every such token before transferring/deleting that Text.
    std::shared_ptr<const void> retain_widget(GuiWidgetOwner&);
    // Pure ownership transfer after idle/token validation. Pass the sole result
    // immediately to GameNativeTextApplication::retire_owned_text: that SAME
    // application's child transport performs AB8EE0 flags1 and AB75A0. The
    // caller retains both graphs on failure. This contract selects the original
    // flags1 caller, rather than host-tree retirement. Do not drop the wrapper.
    std::unique_ptr<GuiLayoutWidget> take_standalone_for_retirement(GuiWidgetOwner&);

    // Later page-loader prerequisite only. The caller still owns the page/root
    // layout and prepares its actual cGuiLayer factory/flag before this call.
    // Creates one actual B8F5E0 cGroup and binds it to that SAME root owner.
    GuiWidgetOwner& create_plain_page_root_00aa5840_fragment(GuiLayoutWidget&);
    void retire_borrowed_page_tree(GuiLayoutWidget&);

    // Metadata only. Serialize every entry with native startup/frames/drain.
    // An entered failure is sticky and nonreplayable; preserve the complete
    // graph and exit before manager/CRT drain. Destruction requires no live
    // layouts, Models, Groups, tokens or failed/entered calls.
    bool requires_process_retention() const noexcept;
    std::uint32_t failure_entry() const noexcept;
    const std::exception_ptr& failure() const noexcept;
private:
    struct Impl;
    std::shared_ptr<Impl> impl_;
};
} // namespace bsp::game
