#include "bsp/gui_widget_attach.hpp"
#include <algorithm>
#include <stdexcept>

namespace bsp {
void append_gui_widget_child_00aaa5a0(GuiWidgetOwnerRuntime& owners,
    NativeNodeParentingRuntime& parenting, GuiLayoutWidget& parent,
    std::unique_ptr<GuiLayoutWidget>& detached) {
    if (!detached) return;
    if (&owners.environment().models.nodes != &parenting.nodes)
        throw std::logic_error("GUI append requires the same actual parenting owner domain");
    auto& child = *detached;
    owners.owner(parent);
    owners.owner(child);
    if (&parent == &child || child.parent || child.transform.parent)
        throw std::invalid_argument("GUI append requires a detached canonical child allocation");
    if (std::any_of(parent.children.begin(), parent.children.end(),
        [&child](const auto& entry) { return entry.get() == &child; }) ||
        std::find(parent.transform.children.begin(), parent.transform.children.end(),
            &child.transform) != parent.transform.children.end())
        throw std::logic_error("GUI append would duplicate its owning allocation");
    // Allocation failures are outside this typed list transport. Reserve both
    // before publication so no half-published C++ list is mistaken for native
    // list insertion. Native-visible calls start only after these stores.
    parent.children.reserve(parent.children.size() + 1);
    parent.transform.children.reserve(parent.transform.children.size() + 1);
    parent.children.push_back(std::move(detached));
    parent.transform.children.push_back(&child.transform);
    child.parent = &parent;
    child.transform.parent = &parent.transform;
    // AAA616 loads the parent node BEFORE the child node. Validate at this
    // phase; a missing node must not undo the already completed GUI insertion.
    auto* const parent_node = owners.owner(parent).node_binding();
    auto* const child_node = owners.owner(child).node_binding();
    if (!parent_node || !child_node)
        throw std::logic_error("GUI append reached native parenting without both actual node owners");
    set_native_node_parent_00b6e680(parenting, child_node->transform, &parent_node->transform);
}

void set_gui_widget_listener_00aa6bc0(GuiWidgetOwner& owner, void* listener,
    std::uint8_t raw_flag) noexcept {
    owner.extra_fields().layout_listener_dc = listener;
    owner.extra_fields().byte_79 = raw_flag;
}

void set_gui_widget_pivot_00a9e0b0(GuiWidgetOwner& owner, const float* pair) {
    auto* const x = &owner.layout().transform.pivot_x;
    auto* const y = &owner.layout().transform.pivot_y;
    __asm {
        mov eax, pair
        mov ecx, x
        fld dword ptr [eax]
        fstp dword ptr [ecx]
        mov ecx, y
        fld dword ptr [eax + 4]
        fstp dword ptr [ecx]
    }
    owner.recompose_00aa7220();
}
} // namespace bsp
