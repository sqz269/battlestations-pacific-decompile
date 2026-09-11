#include "bsp/gui_layout_finished.hpp"
#include "bsp/gui_clip_box.hpp"
#include <stdexcept>
#include <typeinfo>

namespace bsp {
void base_gui_layout_finished24_00a9e0a0(GuiWidgetOwner&) noexcept {}

void dispatch_gui_layout_finished24(GuiWidgetOwner& owner) {
    const auto type = owner.layout().type;
    if (owner.layout().transform.type_id != static_cast<std::int32_t>(type))
        throw std::invalid_argument("GUI current24 requires matching owner/layout type tags");
    auto& implementation = owner.implementation();
    bool base_target = false;
    switch (type) {
    case GuiWidgetType::Screen:
        base_target = dynamic_cast<GuiScreenLayerImplementation*>(&implementation) != nullptr;
        break;
    case GuiWidgetType::Group:
        // Group is not final. An arbitrary derived companion does not prove
        // that its native current slot still points at the base RET.
        base_target = typeid(implementation) == typeid(GuiGroupTypeImplementation);
        break;
    case GuiWidgetType::Icon:
        base_target = dynamic_cast<GuiIconTypeImplementation*>(&implementation) != nullptr;
        break;
    case GuiWidgetType::FrameBox:
        base_target = dynamic_cast<GuiFrameBoxTypeImplementation*>(&implementation) != nullptr;
        break;
    case GuiWidgetType::ClipBox:
        if (auto* clip = dynamic_cast<GuiClipBoxTypeImplementation*>(&implementation)) {
            clip->update24_00ace120(owner);
            return;
        }
        break;
    default:
        throw std::invalid_argument("GUI current24 is not recovered for this widget type");
    }
    if (!base_target)
        throw std::invalid_argument("GUI current24 type tag does not match its concrete companion");
    base_gui_layout_finished24_00a9e0a0(owner);
}

GuiLayoutFinishedHost::GuiLayoutFinishedHost(GuiWidgetOwnerRuntime& owners,
    GuiWidgetTransformHost& native_host) noexcept
    : owners_(owners), native_host_(native_host) {}
bool GuiLayoutFinishedHost::widescreen_enabled() {
    return native_host_.widescreen_enabled();
}
void GuiLayoutFinishedHost::publish_local_transform(GuiWidgetTransform& widget,
    const GuiWidgetLocalTransform& transform) {
    native_host_.publish_local_transform(widget, transform);
}
void GuiLayoutFinishedHost::publish_local_bounds(GuiWidgetTransform& widget,
    const GuiWidgetBounds& bounds) {
    native_host_.publish_local_bounds(widget, bounds);
}
void GuiLayoutFinishedHost::notify_layout_changed(GuiWidgetTransform& widget) {
    native_host_.notify_layout_changed(widget);
}
void GuiLayoutFinishedHost::on_layout_finished(GuiWidgetTransform& widget) {
    dispatch_gui_layout_finished24(owners_.owner(widget));
}
} // namespace bsp
