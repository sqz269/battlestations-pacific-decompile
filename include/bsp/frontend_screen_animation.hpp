#pragma once
#include <cstdint>

// Front-end screen scroll block (00683610 and its twelve sibling methods).
//
// The packet that produced this file was named `front_end_screen_animation`
// because docs/MULTI_MENU_SCREENS.md read 00683610 as "a fade/slide animation
// record". The listing does not support that reading: the block is the vertical
// scroll controller that front-end screens embed for a scrollable content
// widget plus its scroll bar. There is no phase, duration, easing curve or
// frame delta anywhere in the thirteen methods. Everything below comes from the
// assembly, not from the pseudocode; see docs/FRONTEND_SCREEN_ANIMATION.md.

namespace bsp {

// Widget position triple. 00AA6750 fills three floats through an out-pointer
// and returns it in EAX; 00AA8240 consumes the same triple. The repository has
// no shared single-precision vector type (bsp::Vec3d in include/bsp/math.hpp is
// the double-precision one used by the x87 math routines), so this local pair
// of aggregates exists only to give the native argument a name.
struct GuiWidgetPoint {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// Widget size pair. 00AA6740 is `LEA EAX,[ECX+20h]; RET`, so the widget stores
// its size as two floats at +20h/+24h. Virtual +58h takes a pointer to the same
// pair.
struct GuiWidgetSize {
    float width{0.0f};
    float height{0.0f};
};

// ---------------------------------------------------------------------------
// Constants taken from the listing
// ---------------------------------------------------------------------------

// Alpha the scroll bar thumb gets while the list does not scroll (006834A0 at
// 00683572, single-precision 00CE54A0 = 0.2f).
inline constexpr float kScrollThumbIdleAlpha = 0.2f;
// Alpha the two arrow widgets get in the same case (006835A9 and 006835DD,
// single-precision 00CE3868 = 0.25f).
inline constexpr float kScrollArrowIdleAlpha = 0.25f;
// Alpha all three get once the list scrolls (00D7A24C = 1.0f).
inline constexpr float kScrollActiveAlpha = 1.0f;
// Per-frame step scale applied to the raw axis (00683820 at 006838D5,
// `FMUL double ptr [00D7A380]`, the double 0.025).
inline constexpr double kScrollAxisStepScale = 0.025;
// Scale applied to the pad fallback axis (006838A3, `FMUL double ptr
// [00CEC9E0]`, the double -0.5). The sign flip is why the pad reads the
// opposite way round from the input-manager axis.
inline constexpr double kScrollPadAxisScale = -0.5;
// Axis index pushed to the pad object's virtual +24h at 0068389F.
inline constexpr int kScrollPadAxisIndex = 10;

// Auto-scroll direction stored in +20h.
enum class ScrollDirection : std::int32_t {
    kNone = 0,      // 006838F2
    kTowardStart = 1, // 006838C9, taken when the axis is positive
    kTowardEnd = 2,   // 006838C2, taken when the axis is negative
};

// ---------------------------------------------------------------------------
// Recovered field layout
// ---------------------------------------------------------------------------

// 5Ch bytes. Every offset below is written or read by at least one of the
// thirteen methods; the constructor 00683610 writes all of them except +4Ch and
// +50h, which only 00683A40 (begin drag) writes. Nothing above +5Bh is touched,
// so 5Ch is the lower bound on the size; the tightest spacing between two
// copies inside one screen is 74h (00626630 embeds blocks at +128h and +19Ch
// with nothing initialised between them), so the true size is 5Ch..74h.
//
// This supersedes bsp::MultiMenuScreenAnimation in include/bsp/multi_menu_screens.hpp,
// which is a placeholder built from the pseudocode alone and also records the
// mode selector's embed offset as +6Ch where the listing shows +3Ch.
struct FrontEndScreenScroller {
    // +00h. Widget whose position the scroll offset moves. Set by 00683790.
    void* content_widget{nullptr};
    // +04h, +08h. Two optional widgets that follow the thumb's alpha. Set by
    // 00683380 from its third and fourth stack arguments. 00683330 sets them
    // and the thumb directly.
    void* arrow_start_widget{nullptr};
    void* arrow_end_widget{nullptr};
    // +0Ch. Total scrollable distance. `> 0` is the gate in 00683680, 00683700
    // and 00683820, and `!= 0` the gate in 00683440. Set by 006834A0.
    float scroll_range{0.0f};
    // +10h..+18h. Content widget position captured when it was attached.
    GuiWidgetPoint content_base{};
    // +1Ch. Current scroll offset, always in [-scroll_range, 0].
    float scroll_offset{0.0f};
    // +20h, +24h. Auto-scroll direction and per-frame step.
    ScrollDirection auto_scroll{ScrollDirection::kNone};
    float auto_scroll_step{0.0f};
    // +28h. Scroll bar thumb widget. Set by 00683380; a null thumb disables
    // 006834A0, 00683440 and the drag branch of 00683820 outright.
    void* thumb_widget{nullptr};
    // +2Ch. Distance the thumb may travel, min(scroll_range, thumb_track).
    float thumb_travel{0.0f};
    // +30h. Track length minus the minimum thumb height. Set by 00683380 from
    // its second stack argument, which also seeds +2Ch.
    float thumb_track{0.0f};
    // +34h..+3Ch. Thumb position captured when the bar was attached.
    GuiWidgetPoint thumb_base{};
    // +40h. Minimum thumb height: the thumb widget's own height at the moment
    // 00683380 ran, read through 00AA6740.
    float thumb_min_height{0.0f};
    // +44h. Current thumb height, pushed to the widget by 006834A0.
    float thumb_height{0.0f};
    // +48h. Thumb drag in progress. Set by 00683A40, cleared by 00683320 and
    // by 00683790.
    bool dragging{false};
    // +4Ch, +50h. GUI manager cursor position latched when the drag began
    // (manager +5Ch and +60h). Only +50h is read afterwards; +4Ch has no
    // reader in these thirteen routines.
    float drag_anchor_cursor_x{0.0f};
    float drag_anchor_cursor_y{0.0f};
    // +54h. Thumb offset from thumb_base.y at the moment the drag began.
    float drag_anchor_thumb_offset{0.0f};
    // +58h. When set, 00683820 skips the whole input read, so the direction and
    // step a caller pushed through 00683300 survive across frames.
    bool auto_scroll_overridden{false};
    // +59h. Master input gate, set to 1 by the constructor. Nothing in these
    // thirteen routines clears it, so its writer is outside the block.
    bool input_enabled{true};
    // +5Ah. Recomputed by every 006834A0 call: true once the list actually
    // scrolls. Drives the alpha choice.
    bool scrollable{false};
    // +5Bh. Set to 1 by the constructor; when false 006834A0 leaves all three
    // widget alphas alone.
    bool tint_widgets{true};
};

// ---------------------------------------------------------------------------
// Pure kernels
// ---------------------------------------------------------------------------

// Result of 006834A0's geometry step.
struct ScrollBarGeometry {
    float thumb_travel{0.0f};
    float thumb_height{0.0f};
    bool scrollable{false};
};

// 006834A0 at 006834C0..0068351B. `range <= 0` (or NaN) gives a full-track
// thumb and no travel; otherwise the thumb shrinks to its minimum as the range
// grows past the track. thumb_height + thumb_travel is min_height + track in
// every branch.
ScrollBarGeometry compute_scroll_bar_geometry(
    float range, float track, float min_height) noexcept;

// 00683440 at 00683461..00683486. The thumb sits at thumb_base minus
// (0, progress, 0). offset is negative, so the thumb moves along +Y as the
// content scrolls. The quotient is not rounded to float between the divide and
// the multiply, which is why this goes through double.
float thumb_progress_from_scroll(float offset, float range, float travel) noexcept;

// 00683680 at 00683693..006836AF: move toward the start of the list and clamp
// at 0.
float scroll_offset_toward_start(float offset, float delta) noexcept;

// 00683700 at 0068371A..00683748: move toward the end and clamp at -range.
float scroll_offset_toward_end(float offset, float delta, float range) noexcept;

// 00683820 at 00683911..00683966. Thumb Y for the current cursor position,
// clamped to [thumb_base_y, thumb_base_y + travel]. The unclamped sum is formed
// in one x87 expression, so the intermediates stay wider than float.
float dragged_thumb_y(float thumb_base_y, float anchor_thumb_offset,
    float cursor_y, float anchor_cursor_y, float travel) noexcept;

// 00683820 at 0068397C..00683994. The fraction is stored to a float slot before
// the multiply, so it rounds twice; both roundings are reproduced here.
float scroll_offset_from_thumb_y(
    float thumb_y, float thumb_base_y, float travel, float range) noexcept;

// One frame's worth of the axis classification at 006838B6..006838F9.
struct AutoScrollCommand {
    ScrollDirection direction{ScrollDirection::kNone};
    float step{0.0f};
};
AutoScrollCommand classify_scroll_axis(float axis) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

// One method per native call site the block reaches. Nothing here has a default
// implementation; none of it stands in for unrecovered game behaviour.
struct FrontEndScrollerHost {
    virtual ~FrontEndScrollerHost() = default;
    // 00AA6750, __thiscall(widget, Vec3* out) RET 4. Walks the widget's parent
    // chain at +70h, so the result is not the widget's own +0Ch..+14h triple.
    virtual GuiWidgetPoint widget_position(void* widget) = 0;
    // 00AA8240, __thiscall(widget, const Vec3*) RET 4. The inverse of the above.
    virtual void set_widget_position(void* widget, const GuiWidgetPoint& position) = 0;
    // 00AA6740, __thiscall(widget) RET, `LEA EAX,[ECX+20h]`.
    virtual GuiWidgetSize widget_size(void* widget) = 0;
    // Widget vtable +58h, called with a pointer to a two-float pair at
    // 00683556. Naming it a size setter follows from 00AA6740 feeding its
    // first component.
    virtual void widget_set_size(void* widget, const GuiWidgetSize& size) = 0;
    // Widget vtable +4Ch, called with one float by value at 00683590, 006835C4
    // and 006835F8. Naming it an alpha setter follows from the three constants
    // being 0.2, 0.25 and 1.0.
    virtual void widget_set_alpha(void* widget, float alpha) = 0;
    // Input manager 004BEC00, instance +4h, float at +36B4h; when that reads
    // zero the block falls back to +3714h at 00683864.
    virtual float scroll_axis() = 0;
    // Pad fallback: object from 004BA6D0(1, 0) on the singleton at 00F8BBF4,
    // then its virtual +24h with kScrollPadAxisIndex. Returns the raw x87
    // value; the caller applies kScrollPadAxisScale. Returning no value models
    // the null-object branch at 00683896.
    virtual bool pad_axis(int axis_index, double& value) = 0;
    // GUI manager 004C12B0, cursor floats at +5Ch and +60h.
    virtual float cursor_x() = 0;
    virtual float cursor_y() = 0;
};

// ---------------------------------------------------------------------------
// Routines, one per native address
// ---------------------------------------------------------------------------

// 00683610 __thiscall(this) RET. Field initialisation only, no callees.
void construct_front_end_screen_scroller(FrontEndScreenScroller& scroller) noexcept;

// 00683380 __thiscall(this, widget, float track, void* arrow_start,
// void* arrow_end) RET 10h. Captures the thumb's position and height.
void attach_scroll_bar(FrontEndScreenScroller& scroller, void* thumb_widget,
    float track, void* arrow_start, void* arrow_end, FrontEndScrollerHost& host);

// 00683790 __thiscall(this, widget) RET 4. A null widget keeps the previous
// content widget and its captured base but still resets the scroll state.
void attach_content(FrontEndScreenScroller& scroller, void* content_widget,
    FrontEndScrollerHost& host);

// 006833F0 __fastcall(this) RET. Re-reads both base positions from the widgets
// that are currently attached.
void recapture_base_positions(FrontEndScreenScroller& scroller,
    FrontEndScrollerHost& host);

// 006834A0 __thiscall(this, float range) RET 4. Resizes and repositions the
// thumb and retints the three widgets.
void set_scroll_range(FrontEndScreenScroller& scroller, float range,
    FrontEndScrollerHost& host);

// 00683440 __fastcall(this) RET. Pushes the thumb to the position the current
// scroll offset implies.
void sync_thumb_position(const FrontEndScreenScroller& scroller,
    FrontEndScrollerHost& host);

// 00683680 __thiscall(this, float delta) RET 4.
void scroll_toward_start(FrontEndScreenScroller& scroller, float delta,
    FrontEndScrollerHost& host);

// 00683700 __thiscall(this, float delta) RET 4.
void scroll_toward_end(FrontEndScreenScroller& scroller, float delta,
    FrontEndScrollerHost& host);

// 00683300 __thiscall(this, int direction, float step, bool override) RET 0Ch.
void set_auto_scroll(FrontEndScreenScroller& scroller, ScrollDirection direction,
    float step, bool override_input) noexcept;

// 00683330 __thiscall(this, float thumb_alpha, float arrow_start_alpha,
// float arrow_end_alpha) RET 0Ch. Ghidra reads this as two arguments because it
// loses four bytes of stack across each of the three virtual calls; the listing
// reloads from [ESP+4], [ESP+0Ch] and [ESP+10h], which are three distinct slots.
void set_widget_alphas(const FrontEndScreenScroller& scroller, float thumb_alpha,
    float arrow_start_alpha, float arrow_end_alpha, FrontEndScrollerHost& host);

// 00683A40 __fastcall(this) RET.
void begin_thumb_drag(FrontEndScreenScroller& scroller, FrontEndScrollerHost& host);

// 00683320 __fastcall(this) RET, `MOV byte ptr [ECX+48h],0`.
void end_thumb_drag(FrontEndScreenScroller& scroller) noexcept;

// 00683820 __thiscall(this, bool allow_pad_fallback) RET 4. The per-frame pump;
// callers are the screens' update virtuals, for example 00599DB0. It takes no
// frame delta: the step in +24h is applied once per call.
void update_front_end_screen_scroller(FrontEndScreenScroller& scroller,
    bool allow_pad_fallback, FrontEndScrollerHost& host);
}
