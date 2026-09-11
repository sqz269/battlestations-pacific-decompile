# Retained GUI child detachment

Addresses: 00AA83A0 00A9BD50 00B6E680 00B6D890 00B6D940

`detach_gui_widget_child_00aa83a0` reconstructs the complete normal-path body
`00AA83A0..00AA83EE`: native ECX parent, one child pointer argument, `RET4`
at `00AA83EC` (three bytes). The existing name is descriptive, not recovered.

A null child returns before accessing owners. When its current node exists,
the routine sets that node's parent to null, reloads the child's node and
propagates a null root registration. It then reads the parent's current node
and, when present, unlinks the child's current node. Only afterward does it
remove matching child-list entries and clear the widget parent pointer.
It does not destroy the widget, release its scene node, or refresh bounds.

The new interface uses the existing `GuiWidgetOwnerRuntime`, actual
`NativeNodeBinding` objects and `NativeNodeParentingRuntime`. It reloads node
bindings around native callbacks. The same GUI parent/list state is reflected
in the existing layout and transform projections. A missing retained owner or
a callback clearing a subsequently required node produces a diagnostic failure;
all objects must remain alive through these calls.

Native child lists borrow pointers. `GuiLayoutWidget::children` owns
`unique_ptr` entries, so the interface returns ownership of the same detached
widget. This prevents list removal from silently invoking its destructor.
There can be one owning entry per instance; all matching entries in the
borrowed transform view are removed. Null/absent children return an empty
handle, while a non-null absent child still receives the native node and
backpointer effects. The caller must transport the returned ownership to its
actual allocation-lifetime owner before reentrant slot reads. Dropping this
handle is not a binding for Text virtual04: the existing layout cleanup
fallback uses page retirement, whose virtual20-before-delete sequence differs.

`00A9BD50` removes **all** matching native list nodes. Its pseudocode initially
returned after the first `_free` because `00A9BDC0` had a false `CALL_RETURN`
override. Disk and live bytes establish the seven-byte continuation at
`00A9BDC5..00A9BDCB`: stack cleanup and count decrement, followed by the loop
at `00A9BDCC..00A9BDD2`. The official locked repair cleared that override,
disassembled the gap and saved the project. The corrected export has no
remaining call gap. Prior flow values are in `reports/gui_widget_detach_flow.json`.
The library helper is analyzed as a dependency; it is not duplicated in C++.

Four named callers were inspected: `00AAA5A0`, `00AAA650`, `00AAA710` and
`00AB80C0`. Their detach calls pass the current old parent and child before
reattachment or deletion. Live xrefs also reveal `005D1F2E`, whose five bytes
`E8 6D 64 4D 00` target this routine. Ghidra has no containing function there;
that caller's owner/argument interpretation remains unresolved and is not
assigned to the nearest indexed function.

The focused Win32 probe uses actual retained Group widgets and native models,
links their actual node hierarchy, then detaches. It verifies transferred
allocation identity, unchanged live model/companion ownership, both GUI child
views and parent pointers cleared, and native parent/child links removed.
The borrowed transform view includes a duplicate pointer to exercise removal
of all matches without duplicate C++ ownership. Explicit fixture cleanup uses
page retirement afterward; it does not claim Text deletion behavior. See
`local/gui_widget_detach_probe.log`. Native ABI, reentrant destruction,
installed-layout execution and gameplay remain unvalidated.
