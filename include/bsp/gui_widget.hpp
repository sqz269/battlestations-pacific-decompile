#pragma once
// GUI widget base transform: the parent chain, the pivot/size/scale pairs and
// the two routines that publish a widget's local matrix and bounds.
// Addresses: 00aa6740, 00aa6750, 00aa8240, 00aa70e0, 00aa7220.
// Supporting addresses read but not owned: 00aa9390 (base constructor),
// 00aaaed0 (property descriptor table), 00aa8710 (recursive layout pass),
// 00aa83a0 (detach child), 00413920 (matrix multiply), 00b64780 (Z rotation).
// Every name below is a hypothesis, not a recovered symbol. Evidence is in
// docs/GUI_WIDGET_TRANSFORM.md. Nothing here is binary compatible with the
// original: the native object is a 100h-byte base with a vtable, an intrusive
// std::list of children and a scene node pointer, none of which are reproduced.
#include <cstdint>
#include <vector>

#include "bsp/frontend_screen_animation.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Constants the transform reads
// ---------------------------------------------------------------------------

// 00e12fc4, a float 0.75 loaded once in 00AA7220 and applied to the local Y
// translation and to the Y half of the pivot offset, but never to X. 3/4 is the
// 4:3 aspect ratio, so the widget space is X-normalised and Y is compressed to
// keep the pivot square; that reading is a hypothesis from the value alone.
inline constexpr float kGuiAspectYScale = 0.75f;

// 00d7a258, a double 0.0. Both position accessors subtract or add it on the Z
// lane (00AA67B7, 00AA82AF), so Z passes through untouched in both directions.
// It is reproduced here only to explain why Z has no parent term.
inline constexpr double kGuiResolvedZBias = 0.0;

// 00d7a280, a double 0.5, used by 00AA70E0 for the half extents and the radius.
inline constexpr float kGuiBoundsHalfScale = 0.5f;

// 00d5c118, the double the wide-screen layout pass adds to or subtracts from a
// widget's authored X at 00AA8764 and 00AA8774. The stored bits are
// 3FC2222240000000h; the value is exactly representable as a float, so it was
// almost certainly authored as one. What it is a fraction of was not recovered.
inline constexpr float kGuiWideScreenShift = 0.14166605472564697265625f;

// Widget +E0h, the "WideScreenAlign" property of the descriptor table at
// 00AAAED0. Tested at 00AA874C and 00AA875C. The table names two enumerators
// "Left" (00ce92e4) and "Right" (00ce92dc); which literal is 1 and which is 2
// was not established, so the names below describe the arithmetic instead.
enum class GuiWideScreenAlign : std::int32_t {
    None = 0,          // 00AA877C: the authored X is used unchanged
    ShiftNegativeX = 1, // 00AA8761: authored X minus kGuiWideScreenShift
    ShiftPositiveX = 2, // 00AA8771: authored X plus kGuiWideScreenShift
};

// ---------------------------------------------------------------------------
// The base widget layout
// ---------------------------------------------------------------------------

// A projection of the fields of the GUI widget base that the transform reads.
// The native object is built by 00AA9390 and its offsets are quoted per field.
// The names of the serialised fields come from the descriptor table at
// 00AAAED0, which pairs each field address with a literal; the rest are named
// from use. This is not the whole object: +30h..+44h, +7Ch..+A0h, +A4h..+C4h
// (LowColor, HighColor, BlendFactor) and +D8h..+E4h are not modelled here.
struct GuiWidgetTransform {
    // +8h. The authored X. 00AA8710 recomputes +0Ch from it on every layout
    // pass, so writing +0Ch directly is overwritten by the next pass. 00AA7D80
    // is the only writer found.
    float authored_x{0.0f};

    // +0Ch..+14h, the "Pos" property (00cf168c). The widget's own translation,
    // relative to its parent's pivot corner. Default (0,0,0).
    GuiWidgetPoint position{};

    // +18h/+1Ch, the "Pivot" property (00d5c228). A fraction of the widget's
    // own size, not a scale: 00AA6750 multiplies it by +20h/+24h to get the
    // offset it subtracts from every child, and 00AA7220 turns the same product
    // into a translation matrix. Default (0,0), i.e. the corner.
    float pivot_x{0.0f};
    float pivot_y{0.0f};

    // +20h/+24h, the "Size" property (00cff278). 00AA6740 hands out the address
    // of this pair; virtual +58h overwrites it. Default (0,0).
    GuiWidgetSize size{};

    // +28h/+2Ch, the "Scale" property (00ce60ac). The diagonal of the scale
    // matrix in 00AA7220. Default (1,1) from 00AA93B4. It plays no part in the
    // parent chain: only the pivot and the size do.
    float scale_x{1.0f};
    float scale_y{1.0f};

    // +48h, the "Rotate" property (00d5c220). Fed negated to the Z rotation
    // builder at 00AA7250. Default 0.
    float rotate{0.0f};

    // +5Ch, the alpha lane of the four-float "Color" property at +50h..+5Ch
    // (00ce93f8). Virtual +4Ch (00AA6980) writes it and then pushes it into the
    // node's material. Default 1. The RGB lanes at +50h..+58h are not modelled.
    float alpha{1.0f};

    // +60h, the type tag the base constructor takes as its only argument.
    // Virtual +5Ch (00a9e110) returns it: two instructions, MOV EAX,[ECX+60h].
    std::int32_t type_id{0};

    // +70h. Null for a root. 00AA83A0 clears it when a child is detached.
    GuiWidgetTransform* parent{nullptr};

    // +64h..+6Ch, an std::list<GuiWidget*>: a stateless allocator at +64h, the
    // sentinel node at +68h (allocated by 00A9B720, which links a 12-byte node
    // to itself) and the count at +6Ch. Node payloads sit at +8h of each node
    // (00AA8325..00AA872E). A vector is used here because nothing in this
    // packet depends on the list's node identity.
    std::vector<GuiWidgetTransform*> children{};

    // +74h. 00AA70E0 returns immediately when it is clear, so a widget with no
    // bounds never gets a bounding volume. Default 0.
    bool bounds_enabled{false};

    // +78h and +84h, the "MouseHit" (00d5c1cc) and "MouseBlock" (00d5c1d8)
    // properties. Carried for completeness; the transform does not read them.
    bool mouse_hit{false};
    bool mouse_block{false};

    // +E0h. See GuiWideScreenAlign.
    GuiWideScreenAlign widescreen_align{GuiWideScreenAlign::None};
};

// ---------------------------------------------------------------------------
// Pure transform kernels
// ---------------------------------------------------------------------------

// 00AA6740, two instructions: LEA EAX,[ECX+20h]; RET. The native routine hands
// back the address of the pair so the caller can read or overwrite it; here the
// value is returned because the address of a projected field means nothing.
GuiWidgetSize widget_size(const GuiWidgetTransform& widget) noexcept;

// The pivot offset in widget units, the only term a parent contributes besides
// its own resolved position. Both accessors compute it with the size loaded
// first and the pivot multiplied in (00AA675F/00AA6766 and 00AA676E/00AA6771).
GuiWidgetSize pivot_offset(const GuiWidgetTransform& widget) noexcept;

// 00AA6750, __thiscall(widget, GuiWidgetPoint* out) -> out in EAX, RET 4.
// Walks the parent chain by direct recursion on itself (00AA6778). With no
// parent it copies the local translation. With one it evaluates
//     (parent_resolved + own_position) - parent_pivot_offset
// in that order: the sum is stored to the frame first (00AA677D..00AA679B) and
// only then is the pivot subtracted (00AA679F..00AA67BD), which is not the same
// float result as subtracting first. Z takes the same shape with a 0.0 bias.
GuiWidgetPoint resolved_position(const GuiWidgetTransform& widget) noexcept;

// The local translation that puts a widget at a given resolved position, the
// arithmetic half of 00AA8240. With no parent the position is stored verbatim
// (00AA82E5, SSE moves). With one it evaluates
//     (world - parent_resolved) + parent_pivot_offset
// again in that order, the difference reaching the frame before the pivot is
// added back (00AA8275..00AA82B5). It is the exact inverse of the getter.
GuiWidgetPoint local_position_for_resolved(
    const GuiWidgetTransform& widget, const GuiWidgetPoint& world) noexcept;

// The four matrices 00AA7220 builds before it multiplies them. Row-major, and
// the multiply at 00413920 is left*right on row vectors, so a point is pivoted,
// then scaled, then rotated, then translated.
struct GuiWidgetLocalTransform {
    // Translation matrix M1, rows at 00AA7333..00AA733C. Y carries the aspect
    // factor; X and Z do not.
    GuiWidgetPoint translation{};

    // Scale matrix M2, the diagonal written at 00AA7345..00AA739F. Z is 1.
    float scale_x{1.0f};
    float scale_y{1.0f};

    // Z rotation, built by 00b64780 from the single float at 00AA7250, which is
    // -0.0f minus the widget's Rotate. Kept in that form rather than as a plain
    // negation so the sign of a zero angle matches the original.
    float rotation_z{0.0f};

    // Pivot matrix M3, the translation written at 00AA73E4..00AA73ED. Both
    // lanes are negated; only Y is scaled by the aspect factor.
    float pivot_translation_x{0.0f};
    float pivot_translation_y{0.0f};
};

// 00AA7220 up to its matrix multiplies, __thiscall(widget), RET. The native
// body then computes ((M3 * M2) * RotZ) * M1 through 00413920 three times and
// hands the result to the scene node at widget+4Ch through its vtable +38h. The
// composition is left to GuiWidgetTransformHost because that node belongs to
// the render packets.
GuiWidgetLocalTransform local_transform(const GuiWidgetTransform& widget) noexcept;

// The bounding volume 00AA70E0 publishes: three half extents followed by a
// radius, laid out as four consecutive floats in the native frame.
struct GuiWidgetBounds {
    float half_width{0.0f};
    float half_height{0.0f};
    float half_depth{0.0f}; // always 0; the native slot is cleared by XORPS
    float radius{0.0f};
};

// 00AA70E0's arithmetic. The radius uses the larger of the two size lanes, with
// the comparison written so that a tie keeps the height (00AA7132: the height
// is the value already in the slot and the width replaces it only when it is
// strictly larger).
GuiWidgetBounds local_bounds(const GuiWidgetTransform& widget) noexcept;

// 00AA8710's X fixup. An align value outside 0..2 with wide screen on leaves the
// position untouched, because the compare chain at 00AA876C jumps past the
// store; that is reproduced rather than folded into the None case.
float widescreen_local_x(
    float authored_x, GuiWideScreenAlign align, bool widescreen_enabled,
    float current_x) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

// The calls the transform makes into objects this packet does not own. Each is
// one native call site. There are no default implementations: nothing here
// stands in for unrecovered behaviour.
struct GuiWidgetTransformHost {
    virtual ~GuiWidgetTransformHost() = default;

    // 00AA8750: byte +0Dh of the platform object at 0109cf04.
    virtual bool widescreen_enabled() = 0;

    // 00AA7220 tail: the scene node at widget+4Ch, virtual +38h, called with
    // the composed matrix. The node class is not identified.
    virtual void publish_local_transform(
        GuiWidgetTransform& widget, const GuiWidgetLocalTransform& transform) = 0;

    // 00AA70E0 tail: 00b855b0 on the object reached through the node at
    // widget+4Ch by 00b74640(0,0) then 00b732c0.
    virtual void publish_local_bounds(
        GuiWidgetTransform& widget, const GuiWidgetBounds& bounds) = 0;

    // 00AA8782: the object at widget+DCh, virtual +1Ch, no arguments, called
    // before the recompose only when that pointer is non-null.
    virtual void notify_layout_changed(GuiWidgetTransform& widget) = 0;

    // 00AA87A1: the widget's own vtable +24h, tail-jumped at the end of the
    // layout pass.
    virtual void on_layout_finished(GuiWidgetTransform& widget) = 0;
};

// 00AA7220 as a whole.
void recompose_local_transform(
    GuiWidgetTransform& widget, GuiWidgetTransformHost& host);

// 00AA70E0 as a whole, including the +74h gate.
void refresh_local_bounds(
    GuiWidgetTransform& widget, GuiWidgetTransformHost& host);

// 00AA8240 as a whole, __thiscall(widget, const GuiWidgetPoint*), RET 4: the
// local write followed by the recompose and the bounds refresh, in that order.
void set_resolved_position(GuiWidgetTransform& widget,
    const GuiWidgetPoint& world, GuiWidgetTransformHost& host);

// 00AA8710, __thiscall(widget), RET. Children first, in list order, then the
// wide-screen X fixup, then the layout notification, the recompose, the bounds
// refresh and the widget's own vtable +24h.
void apply_widescreen_layout(
    GuiWidgetTransform& widget, GuiWidgetTransformHost& host);

} // namespace bsp
