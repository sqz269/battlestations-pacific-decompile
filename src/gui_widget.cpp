#include "bsp/gui_widget.hpp"

namespace bsp {
namespace {

// 00AA675F..00AA6774 and 00AA8253..00AA8268 are the same four instructions in
// both accessors: the size lane is loaded and the pivot lane is multiplied in.
// The operand order is kept because the doc records it, not because the product
// could differ.
float pivot_term(float size_lane, float pivot_lane) noexcept
{
    return size_lane * pivot_lane;
}

} // namespace

GuiWidgetSize widget_size(const GuiWidgetTransform& widget) noexcept
{
    return widget.size;
}

GuiWidgetSize pivot_offset(const GuiWidgetTransform& widget) noexcept
{
    GuiWidgetSize offset{};
    offset.width = pivot_term(widget.size.width, widget.pivot_x);
    offset.height = pivot_term(widget.size.height, widget.pivot_y);
    return offset;
}

GuiWidgetPoint resolved_position(const GuiWidgetTransform& widget) noexcept
{
    // 00AA67C6: no parent, the local translation is the resolved position.
    if (widget.parent == nullptr) {
        return widget.position;
    }

    // 00AA675C..00AA6774: the parent's pivot offset is evaluated before the
    // recursion, from the parent's own size and pivot.
    const GuiWidgetSize parent_pivot = pivot_offset(*widget.parent);

    // 00AA6778: direct recursion on the parent.
    const GuiWidgetPoint parent_resolved = resolved_position(*widget.parent);

    // 00AA677D..00AA679B: the sums reach the frame first.
    const float sum_x = parent_resolved.x + widget.position.x;
    const float sum_y = parent_resolved.y + widget.position.y;
    const float sum_z = parent_resolved.z + widget.position.z;

    // 00AA679F..00AA67BD: only then is the pivot subtracted. Z subtracts the
    // 0.0 double at 00d7a258, which cannot change a finite value, so it is
    // written out rather than emitted as an operation.
    GuiWidgetPoint out{};
    out.x = sum_x - parent_pivot.width;
    out.y = sum_y - parent_pivot.height;
    out.z = static_cast<float>(static_cast<double>(sum_z) - kGuiResolvedZBias);
    return out;
}

GuiWidgetPoint local_position_for_resolved(
    const GuiWidgetTransform& widget, const GuiWidgetPoint& world) noexcept
{
    // 00AA82E1: no parent, the resolved position is stored verbatim.
    if (widget.parent == nullptr) {
        return world;
    }

    const GuiWidgetSize parent_pivot = pivot_offset(*widget.parent);
    const GuiWidgetPoint parent_resolved = resolved_position(*widget.parent);

    // 00AA8275..00AA828F: the differences reach the frame first.
    const float diff_x = world.x - parent_resolved.x;
    const float diff_y = world.y - parent_resolved.y;
    const float diff_z = world.z - parent_resolved.z;

    // 00AA8293..00AA82B5: the pivot is added back afterwards.
    GuiWidgetPoint local{};
    local.x = diff_x + parent_pivot.width;
    local.y = diff_y + parent_pivot.height;
    local.z = static_cast<float>(static_cast<double>(diff_z) + kGuiResolvedZBias);
    return local;
}

GuiWidgetLocalTransform local_transform(const GuiWidgetTransform& widget) noexcept
{
    GuiWidgetLocalTransform out{};

    // 00AA7231..00AA724E: the local Y is multiplied by the aspect factor; X and
    // Z are copied unchanged (00AA7234, 00AA723F).
    out.translation.x = widget.position.x;
    out.translation.y = widget.position.y * kGuiAspectYScale;
    out.translation.z = widget.position.z;

    // 00AA7273..00AA7284: the X pivot lane is negated, then multiplied by the
    // width. No aspect factor on this lane.
    out.pivot_translation_x = (-widget.pivot_x) * widget.size.width;

    // 00AA72A5..00AA72E1: the Y pivot lane is negated, multiplied by the height
    // and only then multiplied by the aspect factor still on the x87 stack.
    out.pivot_translation_y =
        ((-widget.pivot_y) * widget.size.height) * kGuiAspectYScale;

    // 00AA7345..00AA739F: the scale matrix diagonal, Z fixed at 1.
    out.scale_x = widget.scale_x;
    out.scale_y = widget.scale_y;

    // 00AA7246..00AA7250: XMM0 is loaded with the -0.0f at 00d7a208 and the
    // Rotate field is subtracted from it.
    out.rotation_z = -0.0f - widget.rotate;

    return out;
}

GuiWidgetBounds local_bounds(const GuiWidgetTransform& widget) noexcept
{
    GuiWidgetBounds bounds{};

    // 00AA70FF..00AA7120: each size lane times 0.5.
    bounds.half_width = widget.size.width * kGuiBoundsHalfScale;
    bounds.half_height = widget.size.height * kGuiBoundsHalfScale;

    // 00AA7108: the third lane is cleared with XORPS.
    bounds.half_depth = 0.0f;

    // 00AA7132..00AA714E: the height is the value in the slot and the width
    // replaces it only when the height is strictly smaller, so a tie keeps the
    // height. 00AA7154 then multiplies 0.5 by that larger lane.
    float larger = widget.size.height;
    if (larger < widget.size.width) {
        larger = widget.size.width;
    }
    bounds.radius = kGuiBoundsHalfScale * larger;

    return bounds;
}

float widescreen_local_x(
    float authored_x, GuiWideScreenAlign align, bool widescreen_enabled,
    float current_x) noexcept
{
    // 00AA874C and 00AA8756: both the align value and the platform flag have to
    // be set before either shift applies.
    if (align == GuiWideScreenAlign::None || !widescreen_enabled) {
        return authored_x; // 00AA877C
    }
    if (align == GuiWideScreenAlign::ShiftNegativeX) {
        return authored_x - kGuiWideScreenShift; // 00AA8761
    }
    if (align == GuiWideScreenAlign::ShiftPositiveX) {
        return authored_x + kGuiWideScreenShift; // 00AA8771
    }
    // 00AA876F jumps past the store, so an out-of-range align leaves the
    // position where it was.
    return current_x;
}

void recompose_local_transform(
    GuiWidgetTransform& widget, GuiWidgetTransformHost& host)
{
    host.publish_local_transform(widget, local_transform(widget));
}

void refresh_local_bounds(
    GuiWidgetTransform& widget, GuiWidgetTransformHost& host)
{
    // 00AA70E6: the whole body is behind the +74h flag.
    if (!widget.bounds_enabled) {
        return;
    }
    host.publish_local_bounds(widget, local_bounds(widget));
}

void set_resolved_position(GuiWidgetTransform& widget,
    const GuiWidgetPoint& world, GuiWidgetTransformHost& host)
{
    widget.position = local_position_for_resolved(widget, world);
    // 00AA82CE then 00AA82D5, and the same pair on the no-parent path at
    // 00AA8302 and 00AA8309.
    recompose_local_transform(widget, host);
    refresh_local_bounds(widget, host);
}

void apply_widescreen_layout(
    GuiWidgetTransform& widget, GuiWidgetTransformHost& host)
{
    // 00AA8714..00AA8744: the children are walked first, in list order, and the
    // loop is bounded by the stored count rather than by the sentinel.
    for (GuiWidgetTransform* child : widget.children) {
        if (child != nullptr) {
            apply_widescreen_layout(*child, host);
        }
    }

    widget.position.x = widescreen_local_x(widget.authored_x,
        widget.widescreen_align, host.widescreen_enabled(), widget.position.x);

    // 00AA8782: the +DCh listener. That pointer is not modelled in the layout
    // projection, so the null gate at 00AA8788 lives on the host side.
    host.notify_layout_changed(widget);

    // 00AA8793 then 00AA879A, then the tail jump at 00AA87AB.
    recompose_local_transform(widget, host);
    refresh_local_bounds(widget, host);
    host.on_layout_finished(widget);
}

} // namespace bsp
