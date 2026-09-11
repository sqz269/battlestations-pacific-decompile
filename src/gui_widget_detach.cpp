#include "bsp/gui_widget_detach.hpp"
#include <algorithm>
#include <stdexcept>

namespace bsp {
namespace {
NativeNodeBinding& required_node(GuiWidgetOwner& owner) {
    auto* node = owner.node_binding();
    if (!node) throw std::logic_error("GUI detach callback cleared a required live node binding");
    return *node;
}
}

std::unique_ptr<GuiLayoutWidget> detach_gui_widget_child_00aa83a0(
    GuiWidgetOwnerRuntime& owners, NativeNodeParentingRuntime& parenting,
    GuiLayoutWidget& parent, GuiLayoutWidget* child) {
    if (!child) return {}; //00AA83AA, before reading any owner/node.
    auto& child_owner = owners.owner(*child);
    if (child_owner.node_binding()) {
        set_native_node_parent_00b6e680(parenting,
            required_node(child_owner).transform, nullptr); //00AA83B7
        propagate_native_node_root_00b6d890(parenting.nodes,
            required_node(child_owner).transform, nullptr); //00AA83C1, reload+4C
        auto* parent_node = owners.owner(parent).node_binding(); //00AA83C6
        if (parent_node) {
            unlink_native_node_child_00b6d940(parent_node->transform,
                required_node(child_owner).transform); //00AA83D1, reload child+4C
        }
    }

    //00A9BD50 removes all matching list nodes, never the widget itself.
    //Unique ownership permits one owning entry for this actual widget.
    // Query after callbacks: they can have changed the live child list.
    std::unique_ptr<GuiLayoutWidget> detached;
    const auto found = std::find_if(parent.children.begin(), parent.children.end(),
        [child](const auto& entry) { return entry.get() == child; });
    if (found != parent.children.end()) {
        detached = std::move(*found);
        parent.children.erase(found);
    }
    auto& transforms = parent.transform.children;
    transforms.erase(std::remove(transforms.begin(), transforms.end(), &child->transform),
        transforms.end());
    child->parent = nullptr; //00AA83E3, even when the child was not in the list.
    child->transform.parent = nullptr; //Same GUI parent in the transform projection.
    return detached;
}
} // namespace bsp
