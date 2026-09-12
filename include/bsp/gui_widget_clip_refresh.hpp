#pragma once
#include "bsp/gui_native_clip_parameters.hpp"
#include <optional>

namespace bsp {
// Borrow the SAME canonical owner/material/string/parameter domains. These
// services and the live aspect slot must survive every pending operation and
// every later material consumer of the registered source addresses.
struct GuiWidgetClipRefreshServices {
    GuiWidgetOwnerRuntime& widgets;
    NativeMaterialDestructionAccess& materials;
    NativeMaterialParameterAccess& parameters;
    const float& aspect_ratio_00e12fc0;
};

// Current70=00AAA3E0 is proven for the existing Screen1, Group2, Icon6,
// ClipBox16 and FrameBox18 profiles. Text3 instead selects00AB7A40. This
// predicate establishes dispatch identity, not a non-Model root's geometry ABI.
bool gui_widget_uses_base_clip70_profile(GuiWidgetType) noexcept;

// Borrowed frame immediately BEFORE00AAA452 current child virtual70. Uses the
// SAME transform.children vector; membership/order must stay stable during
// traversal. Parent/children/runtime must survive. Duplicate payload entries
// remain distinct by index. No native intrusive-list mutation ABI is claimed.
struct GuiWidgetClipRefreshContinuation {
    GuiWidgetOwner& parent;
    GuiWidgetOwnerRuntime& widgets;
    std::size_t child_index;
    GuiWidgetTransform* pending_child;
    GuiWidgetOwner& child_owner() const;
};

//00AAA3E0 ECX widget/no stack/RET00AAA465. Actual captured primary Model ->
// mesh -> section0 material -> existing00AA9F10, then select first live child.
// Null node/mesh and zero sections are ordinary skips. Nonnull nodes must be
// canonical live Models, including on Screen profiles; cGroup roots are not
// interpreted as Models. Nullopt reaches the native ordinary return.
std::optional<GuiWidgetClipRefreshContinuation>
begin_gui_widget_clip_refresh_00aaa3e0(GuiWidgetOwner&, GuiWidgetClipRefreshServices&);

// ONLY after the pending child's actual current70 completed. Consume this
// frame, advance and reload the live end/next payload. Never rerun materials.
std::optional<GuiWidgetClipRefreshContinuation>
resume_gui_widget_clip_refresh_after_child70_00aaa3e0(GuiWidgetClipRefreshContinuation&);

// Retained operation on the same owner. Runs actual child.refresh_clip70()
// synchronously before each resume. A throwing child leaves its exact frame
// pending; begin() cannot retry it. Complete that child (including its nested
// continuations) before explicit resume_after_child70(). A currently running
// call also counts as pending so deletion/reentrancy checks cannot erase it.
// A prefix/traversal failure without a resumable child call is terminal: it
// remains pending but cannot restart/resume, because partial native effects
// cannot be rolled back or replayed by this interface.
// Destruction with unfinished work terminates; owner integration must reject
// deletion before starting any destructive native work. This host lifecycle
// guard is not another native field or a replacement for child dispatch.
class GuiWidgetClipRefreshOperation final {
public:
    GuiWidgetClipRefreshOperation(GuiWidgetOwner&, GuiWidgetClipRefreshServices&);
    ~GuiWidgetClipRefreshOperation();
    GuiWidgetClipRefreshOperation(const GuiWidgetClipRefreshOperation&) = delete;
    GuiWidgetClipRefreshOperation& operator=(const GuiWidgetClipRefreshOperation&) = delete;
    void begin();
    void resume_after_child70();
    bool has_pending() const noexcept;
    bool failed_unresumable() const noexcept { return failed_; }
    GuiWidgetOwner& pending_child_owner() const;
private:
    GuiWidgetOwner& owner_;
    GuiWidgetClipRefreshServices& services_;
    std::optional<GuiWidgetClipRefreshContinuation> continuation_;
    bool active_{};
    bool failed_{};
    void advance_after_child();
    void continue_children();
};
} // namespace bsp
