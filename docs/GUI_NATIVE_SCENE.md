# GUI native page-root and child scene ownership

Addresses: `00aa5840` fragment `00aa58fa..00aa5949`, `00b6e680`, `00b8f460`,
`00b6d890`, `00b6dbc0`. Names are descriptive hypotheses; the C++ interfaces
are not replacements for the original binary ABI.

`GuiNativeScene` allocates the plain scripted page's actual 18Ch group slot
from the canonical group pool, constructs its 188h object, and binds the same
node to `GuiWidgetOwnerRuntime::construct_root`. Assembly at `00aa58fa`
loads188h, calls `00b8f450`, then passes the name to `00b8f5e0`. The layer
constructor at `00aa5944` receives name, node, and the exact screen flag. The
caller must prepare those per-page inputs in the real type factory first.

Child models remain owned by the existing widget runtime. Parenting resolves
the canonical model or group reference through the existing node-lifetime
registry and dispatches the current table's virtual1C. Group bounds notification
also resolves that same registry by actual node identity before dispatching
`00b6dbc0`; no array-context cast or duplicate hierarchy is used. The callback
signature has no context parameter, so this adapter has one explicit process
binding and rejects a second live instance. This is C++ dispatch bookkeeping,
not a claimed recovered global. The environment, scene adapter, and widget
runtime must outlive all layouts and retained references.

`00b8f460` now shares the concrete group's live178h/17Ch/180h descriptor with
unregistration and destruction. The existing vector is used only by diagnostic
bindings. The native assignment-before-registration behavior remains intact:
normal virtual1C routes publish+A0 before the equality guard and skip insertion.
The explicit append route grows the actual array and publishes its backlink.

The layout's existing cleanup callback recursively performs actual logical
release. A queued reference can keep the group or a child model alive afterward;
its final release invokes the correct current deleting destructor, returns the
actual pool slot, and removes its companion. Page-root registration cleanup
uses `00b6d890` with null. Allocation/factory failures are explicit errors; the
native null-allocation path, complete native exception unwinding, model-backed
pages, Screen camera/store setup, and rendering are outside this fragment.

The integrated Win32 build and two existing CTests passed. One ignored focused
fixture checked actual group/model slot identity, shared parent/child links,
the skipped setter insertion, explicit raw append, notification masks, local
transform and visibility, layout logical release, and final retained-reference
retirement. Its widget virtuals deliberately use a controlled base profile;
this does not validate Screen construction, camera setup, GUI rendering, or
gameplay. No permanent tests were added. See `reports/gui_native_scene.json`.

## Page disposal correction

`00aa31f0` resolves the former unload question. After finding the page in the
manager vector it shifts following entries, reduces end by4 at `00aa325e`,
calls current virtual20 at `00aa326a`, then current deleting virtual04 with1
at `00aa3276`. The containing method is ECX manager, stack page, RET4 at
`00aa327c` (three bytes; end exclusive `00aa327f`). It deletes the page
directly; this is not a page-reference decrement.

The runtime now follows that disposal order. `release_scene_nodes_00aa8320`
is a pure logical pass and never invokes derived teardown. `retire_tree`
performs that pass first, then derived teardown followed by the base
`00aa9730` logical-release pass and child companion destruction. This is the
disposal fragment only; manager-vector removal is not supplied by the runtime.
The old combined callback could destroy a final outer scene while widgets
still pointed at its roots. The native manager's prior logical pass empties
the root chain and nulls widget node bindings before the scene is destroyed.

An added branch in the same ignored fixture composes the actual outer scene,
actual group/model owners and concrete Screen type. It verifies the root chain
is empty and widgets are unbound when derived cleanup finally releases the
scene, using no extra node retains. Camera/store acquisition and the weak-base
provider remain controlled fixture boundaries. The updated type-dispatch
fixture and the Win32 build with both CTests also passed.
