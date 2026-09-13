#include "bsp/gui_text_child_lifetime.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
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
        auto& owner = owners_.owner(child);
        if (owner.base_lifetime_.phase != GuiWidgetBaseDeletionPhase::not_started)
            throw GuiTextDeletionBoundary("unfinished widget deletion must retain its wrapper");
        auto* lifetime = owner.text_lifetime();
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
    require_scalar_storage(child, flags);

    owner.base_lifetime_.scalar_flags = flags;
    owner.base_lifetime_.phase = GuiWidgetBaseDeletionPhase::derived;
    lifetime->scalar_flags_ = flags;
    lifetime->scalar_phase_ = GuiTextScalarDeletionPhase::derived;
    if (auto* text = dynamic_cast<GuiTextRuntimeImplementation*>(&owner.implementation()))
        begin_native_gui_text_identity_destruction_00ab8250_fragment(text->native_identity().storage());
    lifetime->destroy_derived_00ab8250_fragment(); // AB8EE3; includes native AB80C0.

    destroy_gui_widget_base_00aa9730(owner, *this);

}
} // namespace bsp
