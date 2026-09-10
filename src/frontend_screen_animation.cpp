#include "bsp/frontend_screen_animation.hpp"

#include <cmath>

// Reconstruction of the front-end screen scroll block, 00683300..00683A8F.
// Every expression below follows the x87 order in the listing: an FLD/FADD/FSUB
// chain keeps its intermediates at extended precision and rounds once at the
// closing FSTP, so those chains go through double here and cast to float exactly
// where the native code stores to a float slot. MSVC's double is 53 bits where
// the x87 stack is 64, so a chain with three or more roundable steps can differ
// in the last bit; none of the chains here has more than two.

namespace bsp {
namespace {

// The zero vector the compiler folded into FLDZ at 006836C9, 006837E9,
// 00683471 and 006839EE. In every case the block adds or subtracts a triple
// whose X and Z components the compiler had already proven to be zero.
GuiWidgetPoint offset_along_y(const GuiWidgetPoint& base, float dy) noexcept
{
    GuiWidgetPoint out;
    out.x = base.x + 0.0f;
    out.y = base.y + dy;
    out.z = base.z + 0.0f;
    return out;
}

// UCOMISS/LAHF/TEST AH,44h/JNP, the MSVC pattern for `x != y` on floats: NaN
// falls through as "not equal".
bool not_equal(float a, float b) noexcept
{
    return !(a == b);
}

void push_content_position(const FrontEndScreenScroller& scroller,
    FrontEndScrollerHost& host)
{
    if (scroller.content_widget != nullptr) {
        host.set_widget_position(scroller.content_widget,
            offset_along_y(scroller.content_base, scroller.scroll_offset));
    }
}
}

ScrollBarGeometry compute_scroll_bar_geometry(
    float range, float track, float min_height) noexcept
{
    ScrollBarGeometry geometry;
    // COMISS XMM1,XMM0 with XMM1 zero, JC taken when 0 < range. NaN leaves CF
    // set, so a NaN range takes the scrollable path.
    if (!(0.0f < range)) {
        geometry.thumb_travel = 0.0f;
        // FLD [+40h]; FADD [+30h]; FSTP [+44h] at 006834C8.
        geometry.thumb_height =
            static_cast<float>(static_cast<double>(min_height) + track);
        geometry.scrollable = false;
        return geometry;
    }
    geometry.scrollable = true;
    // FCOMI ST0,ST1 at 006834E4 with ST0 = range, ST1 = track; JC when
    // range < track.
    if (range < track) {
        // FADDP ST2,ST0 then FSUBP at 006834FD: (track + min_height) - range in
        // one x87 expression, stored to a float slot at 00683501.
        const float candidate =
            static_cast<float>((static_cast<double>(track) + min_height) - range);
        // FCOMIP/JBE at 0068350B: keep min_height unless candidate is greater.
        geometry.thumb_height = (min_height < candidate) ? candidate : min_height;
        geometry.thumb_travel = range;
    } else {
        geometry.thumb_height = min_height;
        geometry.thumb_travel = track;
    }
    return geometry;
}

float thumb_progress_from_scroll(float offset, float range, float travel) noexcept
{
    // FLD [+1Ch]; FDIV [ESP]; FMUL [+2Ch]; FSTP. The quotient is never narrowed
    // to float, so the divide and the multiply share one rounding.
    return static_cast<float>(
        static_cast<double>(offset) / static_cast<double>(range) * travel);
}

float scroll_offset_toward_start(float offset, float delta) noexcept
{
    // FLD delta; FADD [+1Ch]; FSTP: one float rounding, same as float addition.
    const float moved = delta + offset;
    // FCOMIP/JBE at 006836A9: clamp to zero once the offset would go positive.
    return (moved > 0.0f) ? 0.0f : moved;
}

float scroll_offset_toward_end(float offset, float delta, float range) noexcept
{
    const float moved = offset - delta;
    // FCHS at 00683730 then FCOMIP/JBE: clamp at -range.
    const float limit = -range;
    return (moved < limit) ? limit : moved;
}

float dragged_thumb_y(float thumb_base_y, float anchor_thumb_offset,
    float cursor_y, float anchor_cursor_y, float travel) noexcept
{
    // 00683918..0068392D: (cursor_y - anchor_cursor_y) added to
    // (thumb_base_y + anchor_thumb_offset), the whole sum in one x87
    // expression with a single store to a float slot.
    const double delta = static_cast<double>(cursor_y) - anchor_cursor_y;
    float y = static_cast<float>(
        (static_cast<double>(thumb_base_y) + anchor_thumb_offset) + delta);
    // FCOMI/JBE at 00683937, then FCOMIP/JBE at 0068395C.
    if (thumb_base_y > y) {
        y = thumb_base_y;
    }
    const float limit = static_cast<float>(static_cast<double>(travel) + thumb_base_y);
    if (y > limit) {
        y = limit;
    }
    return y;
}

float scroll_offset_from_thumb_y(
    float thumb_y, float thumb_base_y, float travel, float range) noexcept
{
    // FSUBRP then FDIVP at 00683980, stored to a float slot at 00683984 and
    // reloaded: the fraction rounds to float before the multiply.
    const float fraction = static_cast<float>(
        (static_cast<double>(thumb_y) - thumb_base_y) / travel);
    // FCHS then FMULP at 00683990: -range * fraction.
    return static_cast<float>(-static_cast<double>(range) * fraction);
}

AutoScrollCommand classify_scroll_axis(float axis) noexcept
{
    AutoScrollCommand command;
    if (!not_equal(axis, 0.0f)) {
        return command;
    }
    // COMISS XMM1,XMM0 then JA at 006838C7: a negative axis keeps 2.
    command.direction =
        (0.0f > axis) ? ScrollDirection::kTowardEnd : ScrollDirection::kTowardStart;
    // FMUL double [00D7A380] then FABS at 006838E3, each step storing to a
    // float slot.
    const float scaled = static_cast<float>(static_cast<double>(axis) * kScrollAxisStepScale);
    command.step = std::fabs(scaled);
    return command;
}

void construct_front_end_screen_scroller(FrontEndScreenScroller& scroller) noexcept
{
    // 00683610 writes every field below and leaves +4Ch and +50h untouched.
    scroller.content_widget = nullptr;
    scroller.arrow_start_widget = nullptr;
    scroller.arrow_end_widget = nullptr;
    scroller.scroll_range = 0.0f;
    scroller.content_base = GuiWidgetPoint{};
    scroller.scroll_offset = 0.0f;
    scroller.auto_scroll = ScrollDirection::kNone;
    scroller.auto_scroll_step = 0.0f;
    scroller.thumb_widget = nullptr;
    scroller.thumb_travel = 0.0f;
    scroller.thumb_track = 0.0f;
    scroller.thumb_base = GuiWidgetPoint{};
    scroller.thumb_min_height = 0.0f;
    scroller.thumb_height = 0.0f;
    scroller.dragging = false;
    scroller.drag_anchor_thumb_offset = 0.0f;
    scroller.auto_scroll_overridden = false;
    scroller.input_enabled = true;  // +59h = 1
    scroller.scrollable = false;    // +5Ah = 0
    scroller.tint_widgets = true;   // +5Bh = 1
}

void attach_scroll_bar(FrontEndScreenScroller& scroller, void* thumb_widget,
    float track, void* arrow_start, void* arrow_end, FrontEndScrollerHost& host)
{
    // 00683380. Both +30h and +2Ch take the track length; the first 006834A0
    // call narrows +2Ch to the real travel.
    scroller.thumb_widget = thumb_widget;
    scroller.thumb_track = track;
    scroller.thumb_travel = track;
    scroller.thumb_base = host.widget_position(thumb_widget);
    // 00AA6740 returns the widget's own size pair; only the height is kept, in
    // both +40h and +44h.
    const GuiWidgetSize size = host.widget_size(thumb_widget);
    scroller.thumb_height = size.height;
    scroller.thumb_min_height = size.height;
    scroller.arrow_start_widget = arrow_start;
    scroller.arrow_end_widget = arrow_end;
}

void attach_content(FrontEndScreenScroller& scroller, void* content_widget,
    FrontEndScrollerHost& host)
{
    // 00683790. A null argument leaves +00h and the captured base alone.
    if (content_widget != nullptr) {
        scroller.content_widget = content_widget;
        scroller.content_base = host.widget_position(content_widget);
    }
    scroller.scroll_offset = 0.0f;
    scroller.auto_scroll = ScrollDirection::kNone;
    scroller.auto_scroll_step = 0.0f;
    scroller.dragging = false;
    scroller.auto_scroll_overridden = false;
    scroller.drag_anchor_thumb_offset = 0.0f;
    push_content_position(scroller, host);
    sync_thumb_position(scroller, host);
}

void recapture_base_positions(FrontEndScreenScroller& scroller,
    FrontEndScrollerHost& host)
{
    // 006833F0, guarded separately on each widget.
    if (scroller.content_widget != nullptr) {
        scroller.content_base = host.widget_position(scroller.content_widget);
    }
    if (scroller.thumb_widget != nullptr) {
        scroller.thumb_base = host.widget_position(scroller.thumb_widget);
    }
}

void set_scroll_range(FrontEndScreenScroller& scroller, float range,
    FrontEndScrollerHost& host)
{
    // 006834A0. +0Ch and +5Ah are written before the null check on the thumb,
    // so a scroller with no scroll bar still records the range and always
    // reports itself as not scrollable.
    scroller.scroll_range = range;
    scroller.scrollable = false;
    if (scroller.thumb_widget == nullptr) {
        return;
    }
    const ScrollBarGeometry geometry = compute_scroll_bar_geometry(
        range, scroller.thumb_track, scroller.thumb_min_height);
    scroller.thumb_travel = geometry.thumb_travel;
    scroller.thumb_height = geometry.thumb_height;
    scroller.scrollable = geometry.scrollable;

    // 00683527: the thumb goes back to its captured base before it is resized.
    host.set_widget_position(scroller.thumb_widget, scroller.thumb_base);
    GuiWidgetSize size = host.widget_size(scroller.thumb_widget);
    size.height = scroller.thumb_height;
    host.widget_set_size(scroller.thumb_widget, size);

    if (!scroller.tint_widgets) {
        return;
    }
    const float thumb_alpha =
        scroller.scrollable ? kScrollActiveAlpha : kScrollThumbIdleAlpha;
    host.widget_set_alpha(scroller.thumb_widget, thumb_alpha);
    const float arrow_alpha =
        scroller.scrollable ? kScrollActiveAlpha : kScrollArrowIdleAlpha;
    if (scroller.arrow_start_widget != nullptr) {
        host.widget_set_alpha(scroller.arrow_start_widget, arrow_alpha);
    }
    if (scroller.arrow_end_widget != nullptr) {
        host.widget_set_alpha(scroller.arrow_end_widget, arrow_alpha);
    }
}

void sync_thumb_position(const FrontEndScreenScroller& scroller,
    FrontEndScrollerHost& host)
{
    // 00683440. The range test is `!=`, not `> 0`, unlike every other gate in
    // the block.
    if (!not_equal(scroller.scroll_range, 0.0f) || scroller.thumb_widget == nullptr) {
        return;
    }
    const float progress = thumb_progress_from_scroll(
        scroller.scroll_offset, scroller.scroll_range, scroller.thumb_travel);
    GuiWidgetPoint position;
    position.x = scroller.thumb_base.x - 0.0f;
    position.y = scroller.thumb_base.y - progress;
    position.z = scroller.thumb_base.z - 0.0f;
    host.set_widget_position(scroller.thumb_widget, position);
}

void scroll_toward_start(FrontEndScreenScroller& scroller, float delta,
    FrontEndScrollerHost& host)
{
    // 00683680, gated on `0 < range`.
    if (!(0.0f < scroller.scroll_range)) {
        return;
    }
    scroller.scroll_offset = scroll_offset_toward_start(scroller.scroll_offset, delta);
    push_content_position(scroller, host);
    sync_thumb_position(scroller, host);
}

void scroll_toward_end(FrontEndScreenScroller& scroller, float delta,
    FrontEndScrollerHost& host)
{
    // 00683700, same gate written as `0.0f < [+0Ch]` against 00D7A218.
    if (!(0.0f < scroller.scroll_range)) {
        return;
    }
    scroller.scroll_offset =
        scroll_offset_toward_end(scroller.scroll_offset, delta, scroller.scroll_range);
    push_content_position(scroller, host);
    sync_thumb_position(scroller, host);
}

void set_auto_scroll(FrontEndScreenScroller& scroller, ScrollDirection direction,
    float step, bool override_input) noexcept
{
    // 00683300, three unguarded stores.
    scroller.auto_scroll = direction;
    scroller.auto_scroll_step = step;
    scroller.auto_scroll_overridden = override_input;
}

void set_widget_alphas(const FrontEndScreenScroller& scroller, float thumb_alpha,
    float arrow_start_alpha, float arrow_end_alpha, FrontEndScrollerHost& host)
{
    // 00683330 dereferences the thumb without a null check.
    host.widget_set_alpha(scroller.thumb_widget, thumb_alpha);
    if (scroller.arrow_start_widget != nullptr) {
        host.widget_set_alpha(scroller.arrow_start_widget, arrow_start_alpha);
    }
    if (scroller.arrow_end_widget != nullptr) {
        host.widget_set_alpha(scroller.arrow_end_widget, arrow_end_alpha);
    }
}

void begin_thumb_drag(FrontEndScreenScroller& scroller, FrontEndScrollerHost& host)
{
    // 00683A40. Cancelling the auto-scroll direction is the first store; the
    // step in +24h is deliberately left as it was.
    scroller.auto_scroll = ScrollDirection::kNone;
    scroller.drag_anchor_cursor_x = host.cursor_x();
    scroller.drag_anchor_cursor_y = host.cursor_y();
    const GuiWidgetPoint thumb = host.widget_position(scroller.thumb_widget);
    scroller.dragging = true;
    scroller.drag_anchor_thumb_offset = thumb.y - scroller.thumb_base.y;
}

void end_thumb_drag(FrontEndScreenScroller& scroller) noexcept
{
    scroller.dragging = false;
}

void update_front_end_screen_scroller(FrontEndScreenScroller& scroller,
    bool allow_pad_fallback, FrontEndScrollerHost& host)
{
    // 00683820. The input read runs only while +59h is set and +58h is clear;
    // an override set through 00683300 is never cleared from here, because the
    // clear at 006838FE sits inside the branch the override skips.
    if (scroller.input_enabled && !scroller.auto_scroll_overridden) {
        float axis = host.scroll_axis();
        if (allow_pad_fallback && !not_equal(axis, 0.0f)) {
            double pad = 0.0;
            if (host.pad_axis(kScrollPadAxisIndex, pad)) {
                axis = static_cast<float>(pad * kScrollPadAxisScale);
            }
        }
        const AutoScrollCommand command = classify_scroll_axis(axis);
        scroller.auto_scroll = command.direction;
        scroller.auto_scroll_step = command.step;
        scroller.auto_scroll_overridden = false;
    }

    if (scroller.dragging) {
        const float y = dragged_thumb_y(scroller.thumb_base.y,
            scroller.drag_anchor_thumb_offset, host.cursor_y(),
            scroller.drag_anchor_cursor_y, scroller.thumb_travel);
        // 0068396D: the offset is recomputed only while the range is positive,
        // but the thumb is repositioned either way.
        if (scroller.scroll_range > 0.0f) {
            scroller.scroll_offset = scroll_offset_from_thumb_y(
                y, scroller.thumb_base.y, scroller.thumb_travel, scroller.scroll_range);
        }
        GuiWidgetPoint position;
        position.x = scroller.thumb_base.x;
        position.y = y;
        position.z = scroller.thumb_base.z;
        host.set_widget_position(scroller.thumb_widget, position);
    }

    // 006839CC. Only the third arm repositions the content directly; the other
    // two go through the scroll helpers, which also resync the thumb.
    switch (scroller.auto_scroll) {
    case ScrollDirection::kTowardStart:
        scroll_toward_start(scroller, scroller.auto_scroll_step, host);
        break;
    case ScrollDirection::kTowardEnd:
        scroll_toward_end(scroller, scroller.auto_scroll_step, host);
        break;
    case ScrollDirection::kNone:
    default:
        push_content_position(scroller, host);
        break;
    }
}
}
