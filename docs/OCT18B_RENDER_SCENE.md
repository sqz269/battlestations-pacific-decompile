# OCT18B diagnostic scene ownership

`MissionShipSceneGraph` in `src/game_native_mission_scene.inc` composes existing
native scene and root APIs. Instantiate it only in the enabled mission-render
branch. Its constructor creates metadata; `create()` performs the native work.
This adds no unique reconstruction credit or second simulation world.

## Verified scene and Operator contracts

The current survey's outer scene is the original `24h` object constructed by
`00B724E0` and stored at `game+19EC`. This is distinct from the optional `3Ch`
lighting resource reached through outer scene `+1C`.

The saved original `004DE610` listing shows:

- `004DE73F` requests `458h` from class allocator `00B71930`.
- `004DE75E` supplies literal `00CE7E04`; `004DE782` calls `00B71A80`.
- `004DE78E` publishes that result to `game+19FC`.

Verified live bytes at `00CE7E04` are `Operator\0`, followed at `00CE7E10` by
`World\0`. Therefore Operator is the existing camera owner, not a separate
camera-controller object required by this diagnostic scene. The primary's real
camera/viewport/fog graph remains authoritative. Native render contexts carry
the outer scene and camera separately, so this graph does not attach the camera
as another model root.

Live queries went through `bsp.py ghidra`, whose client verifies configured
project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 image
base `00400000` before each batch. No Ghidra mutation was needed.

## Actual scene and root storage

`create()` allocates exactly `24h`, creates the actual pooled `World` name,
executes `construct_native_gui_scene_storage_00b724e0()` and binds one
`NativeGuiSceneReference` over its actual `+04` count. The graph borrows the
resource graph's same weak owner, raw-name pool, node environment, actual-owner
registry, current import cells and native profile. Allocation preimages are
preserved by the established constructor. The copied caller name is returned
once through its raw pooled-string cleanup.

The raw constructor establishes `scene+0C = null`, `scene+1C = null` and
`scene+20 = 0`. Thus the optional lighting branch is valid without inventing a
lighting resource. `raw()` returns this actual outer scene for `00B1F4D0` and
`00B72190`; it never returns a host scene wrapper or a root-list projection.

Each `attach(raw, NativeModelReference&)` verifies the exact canonical companion,
actual model profile, shared node environment, original live creator, null parent,
empty child list, null raw root and null scene resource. It then executes the
complete existing raw `00B6D890` root propagation. Native node `+A4` receives the
actual outer scene address; `scene+0C` and model `+3C/+40` remain genuine root
links and backlinks. Authored local matrices and model identities are unchanged.

Because outer `+1C` is null, current model slot `+50` reaches the genuine
`00B6ED80(null, recurse=0)` path. `RootDispatch` validates the current numeric
target and executes the existing canonical model-bound setter. The original
listing's equal-null comparison jumps to the nonrecursive return, so no raw
`3Ch` registry or logical `SceneResource` is consumed. Unknown profiles, nonnull
lighting, child trees and a different recursion request are rejected; there is
no successful missing-target callback.

## Reference and retirement order

`00B6D890` itself does not retain a model. The diagnostic association explicitly
holds one additional real native `+04` credit while the model graph retains its
original creator. It creates no second companion or replacement count.

At shutdown, call `retire_before_native_drain()` **before**
`model.retire_drawables()`. In reverse association order it:

1. Executes raw `00B6D890(model, null)` to remove each root and clear actual
   `+A4`, preserving native links and backlinks.
2. Releases exactly the association's additional native credit through
   `release_native_render_actual_owner_observed()`. It does not invoke
   `00B6F310` a second time. The model's original logical creator remains for
   `model.retire_drawables()`.
3. Verifies the actual scene root list is empty, then releases the scene creator.

A queued ordinary command may still hold the scene. Its later canonical
count-zero terminal runs `00B72580 -> 00B72430` after queue retirement. The root
list is already empty, so no unbound raw model-tree terminal is reached. This
also clears raw root identities before the existing model cleanup uses its
legacy root-view interface.

Call `after_native_drain()` before destroying any borrowed provider. It checks
the actual scene terminal completed and freed the native allocation, then drops
only retired metadata. It never dereferences a freed model or scene. A normal
released scene still held by a queue is allowed by `requires_process_retention()`;
interrupted create/attach/retire operations and failures require process retention.

## Integration

Include the new fragment after resource/model declarations. Add the existing
`native_gui_scene_storage.hpp`, `native_node_root_propagation.hpp` and
`native_node_scene_attachment.hpp` headers before the application's namespace.

Construct with `(log, mission_resources.scenes, actual_owners,
increment_iat_00ce221c, decrement_iat_00ce2220, actual_World_literal_00ce7e10)`.
After `create()`, attach each `model.drawable(i)` and
`model.drawable_reference(i)`, then give `raw()` to A2's scene submission route.
Keep this graph, the resource scene environment and model companions stable
through their explicit retire/drain/closure order.

Source review and `git diff --check` passed. Primary integration owns the Win32
build and mission execution. No game/build was started by this subpacket. These
are new retained C++ interfaces over existing reconstructed bodies, not original
register/FH3/SEH or unrestricted scene/lighting/child-tree ABI proof. Native
`00B724E0` and `00B6D890` use ECX receiver/one stack word/RET4; `00B72430` uses
ECX/no stack words/RET, and `00B72580` uses ECX/flags/RET4.
