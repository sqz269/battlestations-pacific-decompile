# Actual widget append and glyph-child owner leaves

The new helpers supply three operations consumed by the optional Text child
tail, over the same GuiLayoutWidget and GuiWidgetOwnerRuntime.

`append_gui_widget_child_00aaa5a0` implements the null insert-position branch
for a detached allocation. It transfers the caller's unique_ptr into the
parent's existing children vector, updates its borrowed transform.children
view, publishes child.parent, then calls actual B6E680. Existing-parent
detachment remains the already reconstructed AA83A0 and must supply this same
allocation handle. Null handles are inert. The API does not implement the
native nonnull list-iterator insertion branch.

At AAA616 the native loads parent+4C before child+4C. The implementation
preserves that order after publishing the GUI links. A missing actual child
node fails at this phase and does not undo the list effects. A null parent
node is valid and selects the established native null-parent path. It adds no bounds, visibility,
construction74 or loaded78 call. Both vectors reserve before typed publication;
host allocation exceptions and the original std::list/SEH ABI are excluded.
Callbacks use the supplied same NativeNodeParentingRuntime, without another
node tree. Successful node parenting may still trigger its existing callbacks.

AA6BC0 takes listener and low-byte flag on the stack, stores+DC then+79, and
returns with RET8 at AA6BD1. `GuiWidgetBaseExtraFields::byte_79` now uses uint8_t
so this leaf preserves the raw byte rather than normalizing it. Existing
false assignments still store zero. The listener remains borrowed.

A9E0B0 takes a float-pair pointer, copies it using sequential x87 FLD/FSTP
into GUI pivot+18/+1C, calls AA7220 at A9E0BF and RET4 at A9E0C4. The helper
preserves alias-sensitive load/store order and performs actual owner
recomposition, with no extra bounds refresh.

These are typed owner operations, not native ABI replacements. Their exact
call sites and function extents are recorded in gui_widget_attach_actual.json.
The combined build is recorded by the Text factory batch; no gameplay or
rendered-child validation is implied.
