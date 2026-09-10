// Reconstruction of the GUI widget's scene-node binding and visibility rules.
// Evidence and addresses are in docs/GUI_WIDGET_SCENE_VISIBILITY.md and in the
// per-declaration comments of include/bsp/gui_widget_scene.hpp.
#include "bsp/gui_widget_scene.hpp"

namespace bsp {

bool widget_is_visible(
    const GuiWidgetSceneFlags& flags, float node_visibility_factor) noexcept
{
    // 00A9E0D0: MOV EAX,[ECX+4Ch]; TEST EAX,EAX; JZ false; then COMISS against
    // the float 0.0 at 00d7a218 and JBE false. An unordered compare takes the
    // JBE arm, so a NaN factor is not visible; `>` reproduces that.
    if (flags.scene_node == nullptr) {
        return false;
    }
    return node_visibility_factor > 0.0f;
}

float visibility_factor_for(bool visible) noexcept
{
    // 00AA857D: TEST BL,BL; JNZ 00AA8598. The set arm loads FLD1, the clear arm
    // FLDZ, and both spill to the stack argument of 00B6DA70.
    return visible ? kGuiVisibleFactor : kGuiHiddenFactor;
}

bool widget_accepts_hit_test(
    const GuiWidgetTransform& widget, const GuiWidgetSceneFlags& flags) noexcept
{
    // 00AA8C8D: CMP byte [ESI+78h],0; JZ end. 00AA8C97: CMP byte [ESI+77h],0;
    // JNZ end. The MouseHit byte must be set and the hidden byte clear.
    return widget.mouse_hit && !flags.hidden;
}

bool widget_accepts_update(
    const GuiWidgetSceneFlags& flags, bool self_visible) noexcept
{
    // 00A9D03D, 00A9D048, 00A9D055: hidden clear, +85h set, virtual +38h true.
    return !flags.hidden && flags.active && self_visible;
}

GuiWidgetEffectiveVisibility effective_visibility(
    const GuiWidgetVisibilityArgs& args, bool self_visible) noexcept
{
    GuiWidgetEffectiveVisibility result{};

    // 00AA8460..00AA8477: the virtual is asked only when argument 1 is set, so
    // a widget under an already-hidden chain never dispatches +38h for `before`.
    result.before = args.old_ancestor_visible && self_visible;

    // 00AA8479..00AA84A2: when argument 2 is clear the answer is false without
    // any further test. Otherwise argument 5 chooses between the requested
    // value and this widget's own virtual +38h.
    if (args.new_ancestor_visible) {
        result.after = args.apply_requested ? args.requested : self_visible;
    } else {
        result.after = false;
    }
    return result;
}

GuiWidgetVisibilityArgs child_visibility_args(
    const GuiWidgetVisibilityArgs& args, bool old_effective,
    bool new_effective) noexcept
{
    // 00AA84F4..00AA8511. Arguments 3 and 4 pass through untouched; the parent's
    // pair of effective values becomes the child's arguments 1 and 2; argument 5
    // survives only while argument 4 is set (MOV DL,BL; NEG DL; SBB DL,DL; AND
    // EDX,arg5, which is a branchless `recurse ? apply_requested : 0`).
    GuiWidgetVisibilityArgs child{};
    child.old_ancestor_visible = old_effective;
    child.new_ancestor_visible = new_effective;
    child.requested = args.requested;
    child.recurse = args.recurse;
    child.apply_requested = args.recurse && args.apply_requested;
    return child;
}

void propagate_visibility(
    GuiWidgetTransform& widget, const GuiWidgetVisibilityArgs& args,
    GuiWidgetSceneHost& host)
{
    // 00AA8456: a widget with no node stops the walk, children included.
    if (host.flags(widget).scene_node == nullptr) {
        return;
    }

    // The virtual is dispatched at most twice per widget (00AA846D, 00AA8498)
    // and it is a pure read of the node factor in the base, so one call is
    // enough here. Where a derived override has side effects this differs; none
    // was found.
    const bool self_visible = host.is_visible(widget);
    const GuiWidgetEffectiveVisibility effective =
        effective_visibility(args, self_visible);

    // 00AA84A4: CMP BL,AL; JZ past the call. The hook fires on the edge only.
    if (effective.before != effective.after) {
        host.on_effective_visibility_changed(widget, effective.after);
    }

    const GuiWidgetVisibilityArgs child =
        child_visibility_args(args, effective.before, effective.after);
    for (GuiWidgetTransform* item : widget.children) {
        if (item != nullptr) {
            propagate_visibility(*item, child, host);
        }
    }
}

void set_widget_visible(
    GuiWidgetTransform& widget, bool visible, GuiWidgetSceneHost& host)
{
    // 00AA8536..00AA8559: BL starts at 1 and the loop ANDs in each ancestor's
    // virtual +38h, breaking as soon as BL goes clear (JZ 00AA8559). A root
    // widget skips the loop entirely and keeps the seed.
    bool ancestors_visible = true;
    for (GuiWidgetTransform* ancestor = widget.parent;
         ancestor != nullptr && ancestors_visible;
         ancestor = ancestor->parent) {
        ancestors_visible = host.is_visible(*ancestor);
    }

    GuiWidgetSceneFlags& flags = host.flags(widget);

    // 00AA8561..00AA8571. Arguments 1 and 2 are both the ancestor answer, so
    // the difference the walk reports for this widget comes from its own node
    // factor changing under it, not from the chain.
    GuiWidgetVisibilityArgs args{};
    args.old_ancestor_visible = ancestors_visible;
    args.new_ancestor_visible = ancestors_visible;
    args.requested = visible;
    args.recurse = flags.visibility_recurses;
    args.apply_requested = true;
    propagate_visibility(widget, args, host);

    // 00AA8576: the node factor is written after the walk, which is why the
    // walk still observes the old value through virtual +38h.
    if (flags.scene_node != nullptr) {
        host.set_node_visibility_factor(
            flags.scene_node, visibility_factor_for(visible),
            flags.visibility_recurses);
    }
}

void release_scene_nodes(GuiWidgetTransform& widget, GuiWidgetSceneHost& host)
{
    // 00AA8325..00AA8360: the children go first, each through its own vtable
    // +20h, in list order.
    for (GuiWidgetTransform* item : widget.children) {
        if (item != nullptr) {
            release_scene_nodes(*item, host);
        }
    }

    // 00AA8362..00AA837B: vtable +0Ch with the descriptor 00AB6A30 returns,
    // then the extra node of that class.
    if (host.is_kind_of_glyph_owner(widget)) {
        host.release_secondary_node(widget);
    }

    // 00AA8380..00AA838C: the widget's own node, then the pointer is cleared,
    // which makes a second call a no-op.
    GuiWidgetSceneFlags& flags = host.flags(widget);
    if (flags.scene_node != nullptr) {
        host.unlink_and_release_node(flags.scene_node);
        flags.scene_node = nullptr;
    }
}

void deactivate_and_refresh_bounds(
    GuiWidgetTransform& widget, GuiWidgetSceneHost& host)
{
    // 00AA7173..00AA717A: vtable +60h with the single argument 0. The base
    // stores it in +85h; the segment-75 override also re-states its items.
    host.set_active(widget, false);

    // 00AA717C: MOV ECX,ESI; POP ESI; JMP 00AA70E0, a tail jump.
    host.refresh_local_bounds(widget);
}

void attach_child(
    GuiWidgetTransform& parent, GuiWidgetTransform& child,
    GuiWidgetSceneHost& host)
{
    // 00AAA5AD..00AAA60E: the list half, including the detach from an old
    // parent. Owned by the layout-loader packet, so it is a host call here.
    host.link_child(parent, child);

    // 00AAA613: the widget parent pointer.
    child.parent = &parent;

    // 00AAA616: ECX is the child's node and the argument is the parent's node.
    // Neither is tested, so a widget with no node still reparents a null.
    host.set_node_parent(
        host.flags(child).scene_node, host.flags(parent).scene_node);
}

GuiWidgetTransform* clone_subtree(
    GuiWidgetTransform& widget, GuiWidgetTransform* parent_clone,
    GuiWidgetSceneHost& host)
{
    // 00AAB4D0: the type tag in ECX and the widget itself in EDX. The factory
    // runs the copy constructor 00AA9520, which clones the node through the
    // node's own vtable +10h and leaves +8h, +85h and +E4h untouched.
    GuiWidgetTransform* clone =
        host.create_widget_of_type(widget.type_id, widget);

    // 00AAB4DA..00AAB4F2: attach when a parent clone was handed in, or, at the
    // top of the walk, under this widget's own parent. `clone` is tested only
    // on the second arm, exactly as the native compare chain does.
    if (parent_clone != nullptr) {
        attach_child(*parent_clone, *clone, host);
    } else if (clone != nullptr && widget.parent != nullptr) {
        attach_child(*widget.parent, *clone, host);
    }

    // 00AAB4F7..00AAB52F: every child clones itself with this clone as its
    // parent, in list order.
    for (GuiWidgetTransform* item : widget.children) {
        if (item != nullptr) {
            clone_subtree(*item, clone, host);
        }
    }

    if (clone == nullptr) {
        return clone;
    }

    // 00AAB531..00AAB55B: the original's +8h is saved, then the clone's Pos is
    // written back over itself. The self-assignment survives in the binary as
    // three loads and three stores of the same three floats; it is left out
    // here because it cannot change a value.
    const float authored_x = widget.authored_x;

    // 00AAB560 and 00AAB567.
    host.recompose_local_transform(*clone);
    host.refresh_local_bounds(*clone);

    // 00AAB573: the authored X arrives after both publishes, so a clone whose
    // wide-screen pass has not run yet still carries the original's value.
    clone->authored_x = authored_x;
    return clone;
}

} // namespace bsp
