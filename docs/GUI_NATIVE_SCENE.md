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
