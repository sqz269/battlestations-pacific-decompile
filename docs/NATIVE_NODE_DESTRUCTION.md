# Direct native node destruction

This packet reconstructs the direct `00B6F440..00B6F569` node destructor over
the actual `NativeNodeStorage` prefix and its existing `NativeNodeBinding`.
It includes the requested-parent-null path of `00B6E680`, hierarchy unlink
`00B6D940`, recursive attachment removal `00B6D850`, root propagation
`00B6D890`, and root prepend `00B721F0`. It does not require a generated
model's earlier virtual-18 release or a precleared hierarchy.

The destructor's native ABI is ECX owner, no stack arguments, `RET`. The
reconstructed C++ functions receive existing owners and dispatch bindings
explicitly. They are not drop-in binary replacements. All sizes, addresses,
byte hashes, relocation details and validation limits are recorded in
`reports/native_node_destruction_audit.json`.

## One set of live bindings

The existing `CameraTransform` references the actual node prefix. The same
`SceneNodeAttachment` and `SceneAttachmentRuntime` supply current virtual-0C,
40, 50 and 54 dispatch. There is no additional node map or hierarchy. Newly
added world-change/remove callbacks are unbound until explicitly supplied;
an exercised missing callback is an error, never a substitute no-op.

The shared `RenderNodeRootList` borrows actual root head `+0C` and scene `+1C`
slots. Its scene reference is the same owner slot consumed by
`SystemSceneResourceSlot`. Root propagation reads it where `00B72110` does,
after capturing the current virtual-50 dispatch. No root or scene is copied.

An actual nonnull `+A0` identity resolves through the existing
`GeneratedModelLifetimeRuntime` attachment associations to its already supplied
`GeneratedModelAttachmentLinks` backlinks. The native unregister operation is
reused; no new reverse-link array is created. The host notification callback is
cleared when native `+A0` is cleared, so it still describes the current owner.

A nonnull `+130` identity requires a `NativeNodeRetainedOwnerBinding`. Its
atomic reference is required to be at that actual object's `+04`, and its
required final-release callback acts on that identity. The runtime keeps only
this association; it does not introduce another count. The terminal callback
may destroy/unregister its binding and mutate the node.

## Native phase and operation order

The destructor first publishes vtable `00D62C88` and switches only its dying
scene binding into node phase: current virtual-40 is `00B6DBE0`, virtual-50 is
`00B6ED80`, and virtual-54 is `00B6EE10`. Virtual-0C is a required supplied
binding for **`00B6F570`**. This leaf compares the two initialized node/root
tokens at `0108FF90/94`; the existing three-token object predicate `006EF860`
is different. Type-token startup belongs to its separate packet. Child
bindings retain their own current virtual dispatch.

The body unregisters current `+A0`, then requests a null parent. Equal-null
parent returns immediately. Otherwise it unlinks the child from its old parent
with `child.parent = null` before sibling/head/count writes, unregisters its
attachment, captures current root, clears parent/root/attachment, and propagates
that captured root. It then recursively unregisters attachments, conditionally
invalidates flags/caches and calls the current virtual-40.

Root propagation returns early for a matching root only when the node still
has a parent. A parentless root is unlinked and prepended even for the same
root. A missing scene comes from current root `+1C`, or the current parent's
scene, through actual virtual-50 with recursion false. Child propagation keeps
the requested root value and reloads each child's next link after returning.

After null-parent processing, the destructor reloads root and parent. Unless
root is null and parent is nonnull, it unlinks any parentless root registration,
captures first child, clears root and propagates null root through those live
children. It captures `+130`, decrements the actual reference, invokes actual
virtual-00 on zero and clears the field after the callback. It reloads current
scene `+170`, repeats the `+130` clear and calls generic scene removal recursively.

Generic `00B6EE10` removes only a scene matching its captured argument, reloads
the field after the registry/type call and publishes null before releasing the
current scene. A scene installed by that terminal callback survives. Children
receive the same captured expected scene through their current virtual-54;
next sibling is read after each callback. Changes to hierarchy or scenes are
preserved rather than rejected or normalized.

Finally the actual point-light array `+164/+168/+16C` shrinks to count zero and
its allocation is freed without releasing borrowed lights. Its pointer and
capacity words remain unchanged. The actual name allocation returns to the
existing string pool with length plus one; its words remain unchanged. Vtable
phases `00D5C104`, then `00CEB130` finish the base tail without changing the
reference count. The C++ prefix lifetime ends there. Every `+174..+1EF` byte,
including pool slab ID `+1EC`, is preserved.

The `00CC1A01` handler's descriptor `00DFA900` and map `00DFA8E8` order state-2
unwind as `00B6F3E0` array cleanup, `0041DD20` name return, then
`00AA6E10 -> 00BD30F0` reference-base cleanup. The reconstructed catch path
performs those concrete nonthrowing cleanups and propagates the exception.

Physical slot return and host association disposal remain with the caller.
After native cleanup, `SceneAttachmentRuntime::forget_destroyed_binding`
forgets the association without reading dead fields, detaching a replacement
scene or changing a registry. The external companion can then be destroyed and
the actual pool slot returned. Ordinary live `unbind` retains its existing
detach-first requirement.

## Verification boundary

The focused Win32 fixture passed under C++20 `/W4 /WX /EHsc /fp:strict /MD`.
Nineteen live Ghidra spans, totaling 1,455 bytes, matched the installed PE.
The native run executed original `00B6F440` and the relevant paths in twelve
copied helper bodies against a node with a nonnull parent and nonempty root
list. Four complete 496-byte slots matched the reconstructed result after
normalizing only their differing pointer identities. Parent head/count, root
head, stale sibling fields, cache flags, untouched prefix bytes and all pool
tails were checked.

Seventeen 32-bit operands and three vtable entries were relocated. The native
`free(null)` call used the real host CRT free function. No native A0, retained
owner, nonnull scene, name allocation, point-array allocation or exception path
was executed in that differential run. Its native branch coverage is recorded
separately from the following reconstructed-host checks.

The same fixture exercised actual attachment backlinks, nonempty pooled name
and allocated point-array cleanup, a child virtual-50 changing the next sibling,
actual `+130` final release changing the scene, a scene-zero callback installing
a replacement scene, and current child virtual-54 dispatch. A single thrown
retained-owner callback also reached array/name/base unwind cleanup while
preserving the unreached `+130` clear and pool tail. These passed without a
precleared-hierarchy or terminal-scene restriction.

The fixture remains an ignored local artifact; no test suite was added. The
primary integrator owns CMake registration and the final `scripts/build.ps1`
gate. Nonnull-parent reparenting through `00B6E010`, runtime type bootstrap,
derived light construction/destruction, physical pool return, gameplay and
visual validation are outside this packet.
