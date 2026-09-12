#pragma once
#include <cstdint>

namespace bsp {
class GuiWidgetOwner;
class GuiTextChildDeletion;

// Host lifetime metadata only. Native widget values remain in the canonical
// layout, node association and timed header. A failed phase is not resumable.
enum class GuiWidgetBaseDeletionPhase {
    not_started, derived, scene_nodes, children, detach, main_node, entries,
    containers, complete
};
struct GuiWidgetBaseLifetimeState {
    GuiWidgetBaseDeletionPhase phase{GuiWidgetBaseDeletionPhase::not_started};
    std::uint32_t scalar_flags{};
};

// AA9730 after the derived destructor, under the source base vtable. Uses the
// same child allocation transport and canonical tree. Current child4 dispatch
// is established for Group/Text/Section; an unsupported child rejects there.
// Does not implement native widget pools, SEH or intrusive-list allocation ABI.
// On return the companion is gone; flags1 also releases the wrapper allocation.
void destroy_gui_widget_base_00aa9730(GuiWidgetOwner&, GuiTextChildDeletion&);
} // namespace bsp
