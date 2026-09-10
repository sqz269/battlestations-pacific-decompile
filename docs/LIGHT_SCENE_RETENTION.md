# Light scene retention

`attach_light_scene_00b7c020` and `remove_light_scene_00b7bd60` implement the
complete light virtual50/54 bodies through new C++ interfaces. Both native
methods take ECX light, stack scene and recursion byte, and return with RET8.
The verified ranges are `[00B7C020,00B7C0BF)` and `[00B7BD60,00B7BDEA)`.
The latter function was undefined in Ghidra; the primary verified its bytes,
defined the 138-byte body and saved the project. Function names are hypotheses.

`LightSceneRetention` borrows the existing `SceneNodeAttachment` and the actual
12-byte pointer/count/capacity array at light+178/+17C/+180. Each array member
owns one scene reference. `SystemAmbientBacklinks` is reused as the exact array
record and its shared reserve/erase helpers do not perform ownership operations.
The generic node+170 scene slot remains separate. No second registry or ordered
light list is created.

On a new attach, the code grows the array with native signed capacity arithmetic,
reloads its pointer/count after allocation, publishes the scene, increments
count, registers the node through its current type predicate, then retains the
requested scene. A duplicate skips these operations but still recurses when
requested. Removal searches membership, erases by moving the last item into the
first match, unregisters the node, and releases the requested scene, including
its actual terminal callback. An absent scene still allows child recursion.

Both loops load the actual child virtual50/54 each turn and read that child's
next sibling after the callback. The first child is read after any scene-zero
callback. The callbacks live on the same `SceneNodeAttachment` used by all scene
operations. A missing callback is an explicit host binding error. The concrete
dispatch adapters require the node's context to point to its stable
`LightSceneRetention`; they do not select a native vtable phase themselves.

Inputs must describe valid live Win32 storage. Attach requires a nonnull scene;
no null no-op or rollback is added to the native publication path. Malformed
counts, pointer arithmetic overflow, arbitrary concurrent mutation and native
fault/exception ABI are outside this typed interface's validity contract.

The independent source/assembly review found no actionable defect and verified
seven native byte ranges against the installed PE. One ignored host fixture
checks publication/retention order, duplicate/absent recursion, swap removal,
actual final scene destruction, callback changes to both the first child and
next sibling, untouched generic+170, root-slot aliasing/value copies, and
association-only forgetting after the native prefix lifetime ends. It does not
execute original light virtual50/54 machine code. Existing strict Win32 build
and both CTests pass; no tracked test suite was added.

Evidence and exact hashes: `reports/light_scene_retention_audit.json`,
`reports/light_scene_retention_review.json`, and
`reports/light_scene_defined_functions.json`. Full directional-owner creation,
type bootstrap, shadow ownership, startup wiring, binary ABI and gameplay remain
separate from these two methods.
