# Native node parenting and attached groups

Addresses: `00b6e680`, `00b6e010`, `00b6d7b0`, `00b8f460`, `00b8f4f0`, `00b8e600`.
Descriptive C++ names are hypotheses. These new interfaces are not binary replacements.

`set_native_node_parent_00b6e680` now reconstructs the nonnull-parent path as
well as composing the existing null-parent implementation. It uses the same
`CameraTransform`, scene bindings, root list and attachment backlinks as native
node destruction. Its runtime contains dispatch dependencies, with no second tree.

Equal parent returns immediately. Otherwise the current parent is unlinked and
the current attachment unregistered, or a root node first propagates a null root.
The requested parent's actual virtual0C is called with the initialized cGroup
token at `0109032c` (getter `00b8e600`). The selected attachment identity is the
actual group parent, or the requested parent's current+A0 for other types. This
identity is captured before `00b6e010` and survives its callbacks.

`00b6e010` publishes child.parent before propagating the parent's root. If the
child still lacks a scene it dispatches current virtual50 with the parent's
scene and recursion=true. Only afterward does it prepend the child, update the
old first child's previous pointer and increment the count. Valid world state
clears auxiliary bits30h and valid bitsAh, invalidates live descendants, then
dispatches current virtual40. The outer reparent body attaches or unregisters
the subtree and repeats invalidation plus virtual40. Both calls are intentional;
callbacks may reassert validity between them. No transform conversion, refcount
increment, cycle check, stable-order insertion or callback suppression is added.

Node virtual1C `00b6d7b0` unregisters the old attachment, assigns+A0, calls group
registration, then dispatches each live child's current virtual1C with a fresh
next-sibling load. Group override `00b8f4f0` performs the same owner assignment
without walking children. Registration `00b8f460` only appends if node+A0 differs
from the group, growing the backlink array by double/minimum1.
The assignment-before-registration sequence therefore skips insertion on these
setter routes, as independently documented in `CAMERA_ATTACHED_CALLBACK.md`.
Do not reorder it to manufacture backlinks. Registration now calls the shared
append adapter: actual group bindings grow and append through their live raw
178h/17Ch/180h descriptor, and unregistration erases from that same descriptor.
The existing vector is retained only for diagnostic bindings. See
`NATIVE_GROUP_OWNER.md` for raw allocation and lifetime boundaries.

Original ABI for the five mutation routines is ECX receiver, one stack pointer,
RET4. `00b8e600` is a no-argument DWORD getter. Assembly confirmed all receiver
and callback arguments. `00b8f4f0` was missing as a Ghidra function; disk listing
ends in RET4 at `00b8f536` (three bytes, end exclusive `00b8f539`).

Validation: Win32 /W4 /WX /fp:strict build and both existing math CTests passed.
An ignored focused fixture executed the original134-byte `00b6e010` body with
explicit root/invalidation call substitutions and supplied virtual callbacks.
Four branch/state/callback cases matched the reconstruction, including same-parent,
preexisting scene and reentrant world notification. Root propagation itself and
descendant invalidation were not differentially tested in that fixture. A host
fixture checked the full parent's double callback/equal-parent behavior and the
attachment assignment/backlink ordering. No new permanent tests were added.

Actual initialized group type storage, current virtual1C selection and group3C
notification remain required services. `GuiNativeScene` supplies these through
the concrete group owner and shared lifetime runtime; see `GUI_NATIVE_SCENE.md`.
This is reconstructed and fixture-tested behavior. ABI compatibility, GUI
rendering and gameplay remain unvalidated.
