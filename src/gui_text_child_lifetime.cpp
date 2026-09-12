#include "bsp/gui_text_child_lifetime.hpp"
#include <algorithm>
#include <exception>

namespace bsp {
GuiTextChildDeletion::GuiTextChildDeletion(GuiWidgetOwnerRuntime& owners,
    NativeNodeParentingRuntime& parenting) : owners_(owners), parenting_(parenting) {
    if (&owners.environment().models.nodes != &parenting.nodes)
        throw std::invalid_argument("Text deletion requires the same native node lifetime domain");
}
GuiTextChildDeletion::~GuiTextChildDeletion() noexcept {
    for (const auto& handle : detached_)
        if (handle->before_destroy) std::terminate();
    // Only already completed flags0 C++ wrapper allocations can reach here.
}
GuiTextChildDeletion::Handles::iterator GuiTextChildDeletion::find_storage(
    GuiLayoutWidget& child) noexcept {
    return std::find_if(detached_.begin(), detached_.end(),
        [&child](const auto& handle) { return handle.get() == &child; });
}
void GuiTextChildDeletion::accept_detached_child(
    std::unique_ptr<GuiLayoutWidget> child) noexcept {
    if (child) detached_.push_back(std::move(child));
}
std::unique_ptr<GuiLayoutWidget> GuiTextChildDeletion::take_detached_storage(
    GuiLayoutWidget& child) {
    if (child.before_destroy) {
        auto* lifetime = owners_.owner(child).text_lifetime();
        if (lifetime && lifetime->scalar_deletion_phase() !=
            GuiTextScalarDeletionPhase::not_started)
            throw GuiTextDeletionBoundary("unfinished Text deletion must retain its wrapper");
    }
    const auto found = find_storage(child);
    if (found == detached_.end()) return {};
    auto result = std::move(*found);
    detached_.erase(found);
    return result;
}
void GuiTextChildDeletion::release_primary(GuiWidgetOwner& owner) {
    if (auto* node = owner.node_) {
        auto& actual = parenting_.nodes.attachments.resolve(node->transform);
        unlink_and_release_render_model_00b6dfa0(actual);
        // Both AA838C and AA97F3 clear AFTER the terminal callback.
        owner.node_ = nullptr;
        owner.scene_.scene_node = nullptr;
        owner.layout_.node_id = 0;
    }
}
void GuiTextChildDeletion::delete_text_child_virtual4(GuiLayoutWidget& child,
    std::uint32_t flags) {
    auto& owner = owners_.owner(child);
    auto* lifetime = owner.text_lifetime_;
    if (!lifetime || child.type != GuiWidgetType::Text || child.transform.type_id != 3)
        throw GuiTextDeletionBoundary("current delete4 requires the exact canonical Text companion");
    if (&lifetime->widget_ != &owner || &lifetime->buffers_.widgets != &owners_ ||
        &lifetime->buffers_.parenting != &parenting_ || &lifetime->child_calls_ != this)
        throw std::logic_error("Text child deletion must use its original owner and transport domains");
    if (lifetime->phase_ != GuiTextLifetime::Phase::live ||
        lifetime->scalar_phase_ != GuiTextScalarDeletionPhase::not_started)
        throw GuiTextDeletionBoundary("constructing, reentrant or suspended Text deletion is outside this interface");
    // A canonical type implementation may retain mapped content or child70
    // continuations. Reject their destruction before native phase/flag stores,
    // resource release or child traversal; destructor-time detection is too late.
    owner.require_no_active_owned_operation();
    owner.implementation().before_scalar_deletion4(owner);
    // Flag1 must have real C++ ownership transport BEFORE running effects.
    // Attached storage stays in its parent's one list until native self-detach.
    const bool in_parent = child.parent && std::any_of(child.parent->children.begin(),
        child.parent->children.end(), [&child](const auto& p) { return p.get() == &child; });
    if ((flags & 1u) && find_storage(child) == detached_.end() && !in_parent)
        throw std::invalid_argument("Text flags1 requires the same transferred or parent-owned allocation");

    lifetime->scalar_flags_ = flags;
    lifetime->scalar_phase_ = GuiTextScalarDeletionPhase::derived;
    lifetime->destroy_derived_00ab8250_fragment(); // AB8EE3; includes native AB80C0.

    // AB83B9 -> AA9730 has installed the BASE vtable. Do not call the derived
    // before_scene_release hook again, nor the page manager's virtual20 prepass.
    lifetime->scalar_phase_ = GuiTextScalarDeletionPhase::base_scene_nodes;
    for (const auto& current : child.children)
        owners_.owner(*current).release_scene_nodes_00aa8320();
    // Current object's base type-query cannot dispatch Text's secondary hook.
    release_primary(owner); // AA9760 -> AA8387, after child20 traversal.

    lifetime->scalar_phase_ = GuiTextScalarDeletionPhase::base_children;
    while (!child.children.empty()) {
        auto* current = child.children.front().get();
        if (!current)
            throw GuiTextDeletionBoundary("native base child list contains a non-progressing null entry");
        // AA9797 current child virtual4(1), still attached during its derived
        // destructor. Its own base tail removes the entry. Reload head/count.
        delete_text_child_virtual4(*current, 1);
    }

    lifetime->scalar_phase_ = GuiTextScalarDeletionPhase::base_detach;
    if (auto* parent = child.parent) {
        // Exact same sequence as AA97B9/C2/D2/DF: current node parent/root/
        // unlink callbacks, logical list removal, then parent-word clearing.
        accept_detached_child(detach_gui_widget_child_00aa83a0(
            owners_, parenting_, *parent, &child));
    }
    lifetime->scalar_phase_ = GuiTextScalarDeletionPhase::base_main_node;
    release_primary(owner); // AA97EE, only if a live node remains after callbacks.

    lifetime->scalar_phase_ = GuiTextScalarDeletionPhase::base_entries;
    owner.retire_timed_entries_00aa9730_fragment();

    lifetime->scalar_phase_ = GuiTextScalarDeletionPhase::base_containers;
    if ((flags & 1u) && find_storage(child) == detached_.end())
        throw GuiTextDeletionBoundary("callback removed the required scalar-delete allocation transport");
    // AA9951..AA998E resets/destroys the ONE logical child list and sentinel.
    // Both legacy projections are empty here and release their C++ allocation
    // at this same phase, including flags0. No children are implicitly deleted.
    decltype(child.children){}.swap(child.children);
    decltype(child.transform.children){}.swap(child.transform.children);
    // AA99A1/BD30F0 only changes the native vtable. C++ phase/association loss
    // represents completed typed base lifetime; reference count is not changed.
    lifetime->scalar_phase_ = GuiTextScalarDeletionPhase::complete;
    owner.text_lifetime_ = nullptr;
    child.before_destroy = {};
    owners_.widgets_.erase(&child); // Same owner/implementation, no retire_tree.
    if (flags & 1u) {
        const auto found = find_storage(child); // Reload after all callbacks.
        if (found == detached_.end()) std::terminate(); // Invalid C++ implementation destructor mutation.
        detached_.erase(found); // C++ storage release, NOT native AB75A0 pool ABI.
    }
}
} // namespace bsp
