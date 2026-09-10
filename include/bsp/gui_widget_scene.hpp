#pragma once
// The GUI widget's scene node and its visibility rules: how the node at
// widget+4Ch is created, bound, attached and released, where the authored
// "Visible" flag lives, and how a visibility change walks the widget tree.
// Addresses: 00aab4c0, 00aa7170, 00a9cd20, 00aa8320, 00aa6a30, 00aa8530,
// 00aa8450, 00a9cce0, 00a9e0d0, 00a9e0b0, 00a9e070, 00aa6640, 00aa6720,
// 00aa6820, 00aa9520, 00aa9730, 00aaa5a0, 00ab73b0, 00a9c380.
// Supporting addresses read but not owned: 00aa6560 (the type factory),
// 00aaa710 (the property reader), 00aaaed0 (the descriptor table), 00aa9390
// (the base constructor), 00aa8bd0 (the hit-test walk), 00a9d030 (the per-frame
// update), 00b6da70 (BSP_SceneNode_SetVisibilityFactor), 00b6e680
// (BSP_Node_SetParent_Provisional), 00b6dfa0 (BSP_Node_UnlinkAndRelease),
// 00b748e0 (the node draw entry).
// Every name below is a hypothesis, not a recovered symbol. Evidence is in
// docs/GUI_WIDGET_SCENE_VISIBILITY.md. Nothing here is binary compatible with
// the original: the native widget is a 100h-byte base with a vtable and an
// intrusive std::list of children, and the scene node is a separate 184h-byte
// object owned by the render packets.
#include <cstddef>
#include <cstdint>

#include "bsp/gui_widget.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The scene node
// ---------------------------------------------------------------------------

// 00AA6663: the allocation size the widget factory asks for before it runs the
// node constructor at 00B75030. The node is not a widget field; the widget only
// holds the pointer at +4Ch.
inline constexpr std::size_t kGuiSceneNodeSize = 0x184;

// 00AA672B and 00AA9705: both routines that bind a node to a widget clear the
// low two bits of the node's flag word at node+138h immediately afterwards.
// What the two bits mean belongs to the scene packets; they are reproduced here
// because binding is the only place the GUI touches them.
inline constexpr std::uint32_t kGuiSceneNodeBindingClearMask = 0x3u;

// 00A9E0D7 and 00B748F1 both load the node's visibility factor at node+ACh and
// compare it against the float 0.0 at 00d7a218. The setter 00B6DA70 writes it.
// Nothing multiplies it down the tree: 00B6DA70 either writes one node or
// stamps the same value onto every descendant, chosen by its third argument.
inline constexpr float kGuiVisibleFactor = 1.0f;   // 00AA859C, FLD1
inline constexpr float kGuiHiddenFactor = 0.0f;    // 00AA8585, FLDZ

// ---------------------------------------------------------------------------
// The widget bytes this packet establishes
// ---------------------------------------------------------------------------

// The fields of the same native widget object that bsp::GuiWidgetTransform
// projects, at the offsets this packet establishes. They are kept apart from
// that type because each packet projects only the fields it has evidence for;
// a host binds the two projections together through GuiWidgetSceneHost::flags.
struct GuiWidgetSceneFlags {
    // +4Ch. Null until a node is bound. 00AA9390 clears it, 00AA6720 sets it,
    // 00AA8320 releases it and clears it again. Opaque here: the node class
    // belongs to the scene packets.
    void* scene_node{nullptr};

    // +75h. The third argument every SetVisible passes to 00B6DA70
    // (00AA8581/00AA8598). Clear means only this widget's own node takes the
    // new factor; set means the whole node subtree is stamped with it.
    // 00AA9390 clears it; 00AA6820 is the only setter, three instructions long.
    bool visibility_recurses{false};

    // +76h. Cleared by the constructor at 00AA9469 and forced to 1 by the copy
    // constructor at 00AA9602, so it marks a widget produced by a clone. No
    // reader was found in this packet.
    bool copy_constructed{false};

    // +77h. Cleared by the constructor at 00AA946C. 00A9C380 is the only other
    // writer: it stores 0 for a visible item and 1 for a hidden one, so the
    // byte is "hidden", not "visible". It short-circuits the hit-test walk
    // (00AA8C97) and the per-frame update (00A9D03D), and it is independent of
    // the node visibility factor.
    bool hidden{false};

    // +85h. Written only by 00AA6A30, the base of virtual +60h. The per-frame
    // update at 00A9D048 refuses to run while it is clear, and the navigation
    // handler at 00A9CA90 refuses a target whose byte is clear. "Active" is a
    // reading from those two uses; only the segment-75 control class reads it.
    bool active{false};

    // +E4h, the "Visible" property of the descriptor table (name literal at
    // 00d5c1e4, type 3, default 1). The writer emits the record only when
    // virtual +5Ch does not return 1 (00AAB2D5). The reader at 00AAAC09 parses
    // into this byte and then calls virtual +34h with it (00AAAC2A). Neither
    // constructor initialises it; see the uncertainties in the doc.
    bool authored_visible{true};
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 00A9E0D0, widget vtable +38h, __thiscall(widget) -> bool in EAX, RET, no
// arguments. A widget is visible when it has a node and that node's visibility
// factor is strictly greater than zero (COMISS then JBE, so a NaN factor is
// not visible either).
bool widget_is_visible(
    const GuiWidgetSceneFlags& flags, float node_visibility_factor) noexcept;

// 00AA8585 and 00AA859C: the factor 00AA8530 hands to 00B6DA70.
float visibility_factor_for(bool visible) noexcept;

// 00AA8C8D..00AA8C9B, the two byte gates the hit-test walk applies to a widget
// before it tests its own rectangle. The recursion into a child is gated
// separately by that child's virtual +38h (00AA8C5F), not by these bytes.
bool widget_accepts_hit_test(
    const GuiWidgetTransform& widget, const GuiWidgetSceneFlags& flags) noexcept;

// 00A9D03D..00A9D05E, the gate the per-frame update applies before it does any
// work: not hidden, active, and visible by virtual +38h. The update has a
// fourth gate on a manager byte, which is not modelled here.
bool widget_accepts_update(
    const GuiWidgetSceneFlags& flags, bool self_visible) noexcept;

// ---------------------------------------------------------------------------
// The visibility walk
// ---------------------------------------------------------------------------

// The five stack arguments of 00AA8450, RET 14h. They are named from the way
// 00AA8530 seeds them and the way the recursion derives a child's set.
struct GuiWidgetVisibilityArgs {
    // Argument 1 (00AA8460). The effective visibility of the parent chain
    // before the change. Combined with this widget's own virtual +38h to give
    // the value the change is compared against.
    bool old_ancestor_visible{false};

    // Argument 2 (00AA8479). The effective visibility of the parent chain
    // after the change. Clear forces this widget's new effective value false
    // without asking anything else.
    bool new_ancestor_visible{false};

    // Argument 3 (00AA848B). The value SetVisible was called with. Used only
    // where apply_requested is set.
    bool requested{false};

    // Argument 4 (00AA84BD). The +75h byte of the widget the change started
    // at, carried unchanged down the whole walk. It decides whether a child
    // keeps taking the requested value.
    bool recurse{false};

    // Argument 5 (00AA8484). Set means take `requested` as this widget's new
    // own visibility; clear means ask this widget's virtual +38h instead. The
    // seed is 1 and a child inherits `recurse ? apply_requested : false`
    // (00AA84FC..00AA8502), which mirrors the third argument of 00B6DA70.
    bool apply_requested{false};
};

// 00AA84F4..00AA8511, the argument set a child receives.
GuiWidgetVisibilityArgs child_visibility_args(
    const GuiWidgetVisibilityArgs& args, bool old_effective,
    bool new_effective) noexcept;

// 00AA8460..00AA84A2, the two effective values one widget computes.
struct GuiWidgetEffectiveVisibility {
    bool before{false};
    bool after{false};
};
GuiWidgetEffectiveVisibility effective_visibility(
    const GuiWidgetVisibilityArgs& args, bool self_visible) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

// The calls these routines make into objects this packet does not own. Each is
// one native call site. There are no default implementations: nothing here
// stands in for unrecovered behaviour.
struct GuiWidgetSceneHost {
    virtual ~GuiWidgetSceneHost() = default;

    // The bytes of GuiWidgetSceneFlags for a widget the tree walk reached.
    // Not a native call; the projection split is an artefact of this workspace.
    virtual GuiWidgetSceneFlags& flags(GuiWidgetTransform& widget) = 0;

    // Widget vtable +38h (base 00A9E0D0). Dispatched, so a derived class may
    // answer differently; that is why the walk asks the virtual rather than
    // reading the node factor itself.
    virtual bool is_visible(GuiWidgetTransform& widget) = 0;

    // Widget vtable +3Ch (base 00A9E100, `RET 4`, a no-op). 00AA84B2 calls it
    // with the new effective value, and only when it differs from the old one.
    virtual void on_effective_visibility_changed(
        GuiWidgetTransform& widget, bool visible) = 0;

    // 00B6DA70 BSP_SceneNode_SetVisibilityFactor, __thiscall(node, float, bool),
    // RET 8. Writes node+ACh and, when the third argument is set, stamps the
    // same value onto every descendant through node+34h/+3Ch.
    virtual void set_node_visibility_factor(
        void* node, float factor, bool recurse) = 0;

    // 00AA6669..00AA6681: allocate kGuiSceneNodeSize bytes and run the node
    // constructor 00B75030 with a name string.
    virtual void* create_scene_node(const char* name) = 0;

    // 00AA96EA..00AA96FC: node vtable +10h on the source widget's node, with
    // the per-type name from the table at 00d5c0b8 indexed by the type tag.
    virtual void* clone_scene_node(void* source_node, std::int32_t type_id) = 0;

    // 00B6E680 BSP_Node_SetParent_Provisional, __thiscall(child_node, parent),
    // RET 4. Called with the child widget's node in ECX and the parent
    // widget's node as the argument (00AAA616).
    virtual void set_node_parent(void* child_node, void* parent_node) = 0;

    // 00B6DFA0 BSP_Node_UnlinkAndRelease, __thiscall(node), RET.
    virtual void unlink_and_release_node(void* node) = 0;

    // Widget vtable +0Ch (base 00A9E070), called with the class descriptor at
    // 00f8be28 fetched by 00AB6A30. The base implementation scans the two-entry
    // table 00f8bc88..00f8bc90, so it is a hand-rolled kind-of test.
    virtual bool is_kind_of_glyph_owner(GuiWidgetTransform& widget) = 0;

    // 00AB73B0, __thiscall(widget), RET: releases a second node held at
    // widget+188h by the class 00AA8320's kind-of test selects.
    virtual void release_secondary_node(GuiWidgetTransform& widget) = 0;

    // Widget vtable +60h (base 00AA6A30, which writes +85h). Dispatched: the
    // segment-75 control overrides it at 00A9CD20 and re-states its items.
    virtual void set_active(GuiWidgetTransform& widget, bool active) = 0;

    // 00AA70E0 BSP_GuiWidget_RefreshLocalBounds and 00AA7220
    // BSP_GuiWidget_RecomposeLocalTransform, both __thiscall(widget), RET.
    virtual void refresh_local_bounds(GuiWidgetTransform& widget) = 0;
    virtual void recompose_local_transform(GuiWidgetTransform& widget) = 0;

    // 00AA6560, __fastcall(type_tag, source_widget). The factory switch that
    // builds a widget of a given type from an existing one. Owned by the GUI
    // layout loader packet.
    virtual GuiWidgetTransform* create_widget_of_type(
        std::int32_t type_id, GuiWidgetTransform& source) = 0;

    // 00AAA5A0's list half, __thiscall(parent, child, position), RET 8: detach
    // the child from its old parent if it has one, then splice it into the
    // parent's std::list at +64h. The node half is reproduced by
    // attach_child_node below.
    virtual void link_child(
        GuiWidgetTransform& parent, GuiWidgetTransform& child) = 0;
};

// ---------------------------------------------------------------------------
// Native routines
// ---------------------------------------------------------------------------

// 00AA8450, __thiscall(widget, args), RET 14h. Returns immediately for a widget
// with no node. Computes the pair of effective values, fires vtable +3Ch when
// they differ, then recurses over the children in list order. The native body
// walks the intrusive list; the order is the only thing that matters here.
void propagate_visibility(
    GuiWidgetTransform& widget, const GuiWidgetVisibilityArgs& args,
    GuiWidgetSceneHost& host);

// 00AA8530, widget vtable +34h in the 00d5bed0 table,
// __thiscall(widget, bool visible), RET 4. Walks up the parent chain asking
// each ancestor's virtual +38h and stopping at the first false, seeds
// propagate_visibility with that answer, and only then writes the node factor.
// The order matters: the walk reads the old factor through virtual +38h.
void set_widget_visible(
    GuiWidgetTransform& widget, bool visible, GuiWidgetSceneHost& host);

// 00AA8320, widget vtable +20h, __thiscall(widget), RET. Children first, in
// list order, each through its own vtable +20h; then the kind-of test and the
// secondary node; then this widget's own node is unlinked, released and the
// pointer cleared. It is the scene half of the destructor 00AA9730.
void release_scene_nodes(GuiWidgetTransform& widget, GuiWidgetSceneHost& host);

// 00AA7170, widget vtable +78h in the 00d5bed0 table, __thiscall(widget), RET.
// Two calls: virtual +60h with 0, then a tail jump into 00AA70E0.
void deactivate_and_refresh_bounds(
    GuiWidgetTransform& widget, GuiWidgetSceneHost& host);

// The node half of 00AAA5A0, __thiscall(parent, child, position), RET 8: the
// parent pointer at +70h, then BSP_Node_SetParent_Provisional with the child's
// node in ECX and the parent's node as the argument. Both node pointers may be
// null; the native code passes them through without a test.
void attach_child(
    GuiWidgetTransform& parent, GuiWidgetTransform& child,
    GuiWidgetSceneHost& host);

// 00AAB4C0, __thiscall(widget, parent_clone), RET 4, returns the clone in EAX.
// Builds a clone through the type factory, attaches it under `parent_clone` (or
// under this widget's own parent at the top of the walk), clones every child
// with the new clone as their parent, then republishes the transform and the
// bounds and copies the authored X at +8h, which the copy constructor skips
// because it starts at +0Ch (00AA9557).
GuiWidgetTransform* clone_subtree(
    GuiWidgetTransform& widget, GuiWidgetTransform* parent_clone,
    GuiWidgetSceneHost& host);

} // namespace bsp
