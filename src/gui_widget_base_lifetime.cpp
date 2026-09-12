#include "bsp/gui_widget_base_lifetime.hpp"
#include "bsp/gui_text_child_lifetime.hpp"
#include "bsp/gui_widget_detach.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include "bsp/gui_section_runtime.hpp"
#include <algorithm>

namespace bsp {
void GuiTextChildDeletion::require_scalar_storage(GuiLayoutWidget& child,
    std::uint32_t flags) {
    const bool attached = child.parent && std::any_of(child.parent->children.begin(),
        child.parent->children.end(), [&child](const auto& handle) { return handle.get() == &child; });
    if ((flags & 1u) && find_storage(child) == detached_.end() && !attached)
        throw std::invalid_argument("widget flags1 requires the same transferred or parent-owned allocation");
}
void GuiTextChildDeletion::set_base_phase(GuiWidgetOwner& owner,
    GuiWidgetBaseDeletionPhase phase) noexcept {
    owner.base_lifetime_.phase = phase;
    if (auto* text = owner.text_lifetime_) {
        switch (phase) {
        case GuiWidgetBaseDeletionPhase::scene_nodes:
            text->scalar_phase_ = GuiTextScalarDeletionPhase::base_scene_nodes; break;
        case GuiWidgetBaseDeletionPhase::children:
            text->scalar_phase_ = GuiTextScalarDeletionPhase::base_children; break;
        case GuiWidgetBaseDeletionPhase::detach:
            text->scalar_phase_ = GuiTextScalarDeletionPhase::base_detach; break;
        case GuiWidgetBaseDeletionPhase::main_node:
            text->scalar_phase_ = GuiTextScalarDeletionPhase::base_main_node; break;
        case GuiWidgetBaseDeletionPhase::entries:
            text->scalar_phase_ = GuiTextScalarDeletionPhase::base_entries; break;
        case GuiWidgetBaseDeletionPhase::containers:
            text->scalar_phase_ = GuiTextScalarDeletionPhase::base_containers; break;
        case GuiWidgetBaseDeletionPhase::complete:
            text->scalar_phase_ = GuiTextScalarDeletionPhase::complete; break;
        default: break;
        }
    }
}
void GuiTextChildDeletion::erase_completed_owner(GuiWidgetOwner& owner,
    std::uint32_t flags) {
    auto& child = owner.layout_;
    owner.text_lifetime_ = nullptr;
    child.before_destroy = {};
    owners_.widgets_.erase(&child);
    if (flags & 1u) {
        const auto found = find_storage(child);
        if (found == detached_.end()) std::terminate();
        detached_.erase(found);
    }
}
void GuiTextChildDeletion::delete_widget_virtual4(GuiLayoutWidget& child,
    std::uint32_t flags) {
    auto& owner = owners_.owner(child);
    if (child.type == GuiWidgetType::Text) {
        delete_text_child_virtual4(child, flags);
        return;
    }
    const bool group = child.type == GuiWidgetType::Group && child.transform.type_id == 2 &&
        dynamic_cast<GuiGroupTypeImplementation*>(&owner.implementation());
    auto* section = child.type == GuiWidgetType::Section && child.transform.type_id == 17
        ? dynamic_cast<GuiSectionRuntimeImplementation*>(&owner.implementation()) : nullptr;
    if (owner.text_lifetime_ || (!group && !section))
        throw GuiTextDeletionBoundary("current delete4 has no established canonical Group/Text/Section implementation");
    owner.require_no_active_owned_operation();
    owner.implementation().before_scalar_deletion4(owner);
    require_scalar_storage(child, flags);
    owner.base_lifetime_.scalar_flags = flags;
    owner.base_lifetime_.phase = GuiWidgetBaseDeletionPhase::derived;
    // ABF593 -> ABF4F0 releases current texture114, then current stringEC,
    // before ABF578 enters AA9730. The companion never invokes the base again.
    if (section) section->before_scene_release(owner);
    // AC73E3 installs the Group profile, then AC73E9 directly calls AA9730.
    // Flags are tested only AFTER base teardown. C++ wrapper ownership replaces
    // the F0 Group slot/AC7260 or Section pool ABEC40 transport, not raw ABI.
    destroy_gui_widget_base_00aa9730(owner, *this);
}
void destroy_gui_widget_base_00aa9730(GuiWidgetOwner& owner,
    GuiTextChildDeletion& calls) {
    if (&owner.runtime_ != &calls.owners_ ||
        owner.base_lifetime_.phase != GuiWidgetBaseDeletionPhase::derived)
        throw GuiTextDeletionBoundary("base teardown requires its original active scalar owner");
    auto& child = owner.layout_;
    const auto flags = owner.base_lifetime_.scalar_flags;
    calls.set_base_phase(owner, GuiWidgetBaseDeletionPhase::scene_nodes);
    // AA9752 installs the base table before AA9760. Current object's lineage
    // query is therefore base-only; child20 still uses each child's actual type.
    const auto child_count = child.children.size();
    for (std::size_t index = 0; index < child_count; ++index) {
        auto* current = child.children[index].get();
        if (!current) throw GuiTextDeletionBoundary("base child20 requires a live child");
        calls.owners_.owner(*current).release_scene_nodes_00aa8320();
        if (child.children.size() != child_count || child.children[index].get() != current)
            throw GuiTextDeletionBoundary("child20 changed the borrowed host child collection");
    }
    calls.release_primary(owner);

    calls.set_base_phase(owner, GuiWidgetBaseDeletionPhase::children);
    while (!child.children.empty()) {
        auto* current = child.children.front().get();
        if (!current) throw GuiTextDeletionBoundary("base child4 contains a non-progressing null entry");
        // AA9797: child remains attached during derived destruction; its own
        // base tail detaches it. Reload the head/count after every return.
        calls.delete_widget_virtual4(*current, 1);
    }
    calls.set_base_phase(owner, GuiWidgetBaseDeletionPhase::detach);
    if (auto* parent = child.parent)
        calls.accept_detached_child(detach_gui_widget_child_00aa83a0(
            calls.owners_, calls.parenting_, *parent, &child));
    calls.set_base_phase(owner, GuiWidgetBaseDeletionPhase::main_node);
    calls.release_primary(owner); // AA97EE: re-test after detach callbacks.
    calls.set_base_phase(owner, GuiWidgetBaseDeletionPhase::entries);
    owner.retire_timed_entries_00aa9730_fragment();
    calls.set_base_phase(owner, GuiWidgetBaseDeletionPhase::containers);
    if ((flags & 1u) && calls.find_storage(child) == calls.detached_.end())
        throw GuiTextDeletionBoundary("callback removed the scalar allocation transport");
    // AA9951..AA998E drains the source list/sentinel; the same emptied logical
    // list projections release their C++ allocations at this phase, flags0 too.
    decltype(child.children){}.swap(child.children);
    decltype(child.transform.children){}.swap(child.transform.children);
    // AA99A1 only resets the base profile; reference count is not decremented.
    calls.set_base_phase(owner, GuiWidgetBaseDeletionPhase::complete);
    calls.erase_completed_owner(owner, flags);
}
} // namespace bsp
