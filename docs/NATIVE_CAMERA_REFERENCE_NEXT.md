# Native camera references: next integration boundary

The queued camera reference must bind the actual `NativeNodeStorage.references_04`
word. A second counter in a `RenderCommandReference` subclass would let queued
ownership and native ownership disagree. Final queued release must follow the
camera's `+00` deleting-destructor route. Logical owner release through virtual
`+18` is a separate operation and must remain separate.

This is read-only discovery from `c14f9bd`. The native camera owner under review
is the primary's uncommitted implementation in the main checkout; its exact
header/source hashes are recorded in `reports/native_camera_reference_next.json`.
No C++, Ghidra annotations, ledgers, shared metadata, tests, or game files were
changed. Twelve complete functions, one caller fragment and two vtables were
checked: **1,067 code bytes plus 176 data bytes** match live Ghidra and the
installed executable. Every live query verifies `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. This packet executes no native or host
fixture and makes no build, ABI compatibility, or gameplay claim.

## Two paths sharing one count

The actual camera table `00D62CF0` has `+00=00BD30E0`, `+04=00B71FE0`,
`+18=00B6F310`, and `+54=00B6EE10`. The node table `00D62C88` shares virtual
`+18/+54`, but its deleting destructor is `00B6F8D0`. These are actual table
entries, not inferred class names.

| Entry | Original ABI | Required behavior |
| --- | --- | --- |
| `00B1D120` | ECX context, RET0 | For camera `+08`, decrement actual camera `+04`; call current virtual `+00` only at zero; clear context camera after callback. Repeat for scene `+0C` and target `+14`. |
| `00BD30E0` | ECX owner, RET0 | Null is a no-op. Otherwise call current vtable `+04` with flag 1. This trampoline does not decrement a count. |
| `00B71FE0` | ECX camera, stack flags, EAX original slot, RET4 | Call `00B71F10`, then return the same slot to pool `0108FFB0` only if `flags&1`. Returned EAX may identify freed storage. |
| `00B6DFA0` | ECX node, tail current virtual `+18`, no stack arguments | Unlink the actual parent/siblings or root registration first, then dispatch logical release. |
| `00B6F310` | ECX node, RET0 or tail current virtual `+00` | Remove point-light backlinks and release children on every call. Only a zero byte `+44` permits the final hierarchy clear, attachment unregister, byte assignment and one self decrement. |

The import table identifies `00CE221C/00CE2220` as
`KERNEL32!InterlockedIncrement/InterlockedDecrement`. Existing queue helpers
use sequentially consistent atomic operations. They should keep those helpers
and bind their accessed atomic directly to the camera's real `+04` storage.

A count trace explains the required integration; it is an inference from the
inspected instructions, not an executed fixture:

1. Native construction leaves count 1 and released byte 0.
2. Context acquisition retains the same count to 2.
3. The logical owner calls `00B6DFA0 ->00B6F310`; links are released, byte `+44`
   becomes 1, and count becomes 1. The camera, still-published retained fields,
   scene association and stable host companion remain alive.
4. Context teardown decrements count to 0 and reaches
   `00BD30E0 ->00B71FE0(1) ->00B71F10`, followed by actual camera-pool return.

Calling `00B6F310` from a queue zero callback is incorrect. The queue has
already decremented the count: an unset byte `+44` would decrement it again;
a set byte would suppress the decrement and return without destruction.
Neither behavior implements the native zero callback. Likewise, direct queued
zero release has no byte-44 precondition. `00B71FE0/00B71F10` do not require a
previous logical release or an already cleared hierarchy.

There is a concrete logical caller: `[00A8DF84,00A8DF9C)` in the shadow-owner
destructor calls `00B71990(camera,null)`, reloads its camera slot, conditionally
calls `00B6DFA0`, then clears the owning slot. The viewport setter precedes the
null test, so this fragment does not establish a null-safe camera setter. It
also does not authorize replacing the logical release with immediate camera
destruction when a queue retains the camera.
The queue protects the camera lifetime, not a snapshot of its fields: this
shadow path still clears the viewport immediately. Do not preserve a separate
viewport or promise that later queued execution remains meaningful after its
logical owner has torn down required state.

## Existing interfaces and the missing storage views

| Existing interface | Present behavior | Camera integration requirement |
| --- | --- | --- |
| `RenderCommandReference` | Owns `reference_count{1}`. Shared helpers access that member directly. Terminal callback is `noexcept`. | Add a borrowed-count construction/view path that neither initializes nor copies actual `node.references_04`. Keep existing owned diagnostic behavior for default users. |
| `RenderCommandModelLifetime` | Supplies one `CameraTransform&` and actual virtual-18 release; existing `unlink_and_release_render_model_00b6dfa0` already performs the full unlink body. | A camera adapter can implement it through `GeneratedModelNodeLifetime`; reuse the unlink function unchanged. Its current model-oriented name is not a layout requirement. |
| `RenderCommandModelLifetimes` | Resolves generated `InstanceUploadModel` identities used by instance groups. | Do not fabricate an `InstanceUploadModel` for a camera. A camera's explicit logical caller can pass its lifetime adapter directly to the existing unlink helper. |
| `GeneratedModelNodeLifetime` and runtime | Stable transform/scene identity, virtual `+18/+54`, and mixed-child dispatch associations. | Bind the camera in the same `owner.environment.nodes.attachments` runtime and expose `owner.node.transform` plus `owner.node.scene_attachment`. Do not create another hierarchy or scene map. |
| `GeneratedModelLifetime` | Owns model-specific byte 44, point-light vector, geometry `+180`, retained `+174/+130`, diagnostic name and a model-specific terminal destructor. | Do not instantiate or cast this concrete model owner to stand in for a camera. Its terminal destructor releases the wrong fields and requires the model's prior logical-release state. |
| `NativeCameraOwner` | Actual node/tail storage, reference word, byte 44, scene binding, general camera destruction and actual pool return already exist. | Compose one stable camera lifetime/reference adapter over this owner and delegate its zero path to the existing camera destructor closure. |

The smallest shared counter change can preserve the current helper call sites:
a constructor accepts `std::atomic<std::int32_t>&` and makes the exposed count
refer to that actual word. Default users may retain owned diagnostic storage.
The borrowed path must not reset the word to one, take an extra reference, or
maintain a mirrored counter. The primary must settle this shared API once for
camera and batch owners; this discovery does not select an implementation of
its storage representation.

Virtual `+18` is already reconstructed, but its current implementation only
accepts `GeneratedModelLifetime`. Extract its existing traversal into one
shared operation with borrowed access to:

- The same `CameraTransform`, the existing `GeneratedModelLifetimeRuntime`,
  the actual byte `released_44`, and the same counted self reference.
- A point-light view offering live count/element reads and shrink-to-zero.
  For generated models this addresses the existing vector; for the camera it
  addresses actual `NativeNodePointLightArray.begin/count/capacity` at
  `+164/+168/+16C`. It must not copy the camera array into a vector.

The existing generated-model entry should delegate to that extracted operation.
The camera adapter should delegate with references into its native storage.
This is a storage-interface refactor, not another implementation of `00B6F310`.
Reuse the existing point-light backlink removal, attachment unregister and
child runtime dispatch. The raw shrink-zero operation can be extracted from
the existing native-node array cleanup; logical release must stop before its
free operation. Pointer and capacity stay unchanged until final node destruction.

## Ordering and association lifetime

`00B6F310` first reloads the point-light pointer/count during its forward walk,
removes borrowed backlinks with `00B7C1A0 ->00B7BED0`, and runs resize-zero.
It does not release the light owners. Then it repeatedly publishes the current
child's next pointer as the new first child, clears the new head's previous
pointer, clears the detached child's parent/next, and invokes that child's
actual virtual `+18`. The first-child pointer is reloaded after each call.
It does **not** decrement or reset this node's child count `+38`.

Only after that work does it test byte `+44`. On the first logical release it
captures `+A0`, clears root/parent/first/previous/next, writes byte 1,
unregisters the actual attachment and clears `+A0`, then decrements actual
`+04`. A repeated call still processes newly installed light links and children
before skipping the self decrement. Do not move the byte test to the top.

The same canonical camera adapter must serve all queue references to a given
owner. Queue assignment compares adapter pointer identity; multiple wrappers
around one camera would violate native equal-owner behavior. Register its
node-lifetime interface in the existing runtime, whose uniqueness check uses
the same transform identity. Binding adds no counted ownership. Only publish
the queue reference after successful native camera construction.

The adapter and `NativeCameraOwner` companion must survive logical release
while any queue reference remains. Keeping only the raw pool slot alive is
insufficient: queued execution still needs the companion's frame, environment,
and dispatch bindings. Resetting an outer `unique_ptr` at logical release would
leave queued references dangling even if the raw count is correct. Supply an
explicit host owner/disposal contract for the stable companion allocation;
do not assume it was allocated with `new` or use an arbitrary `delete this`.

At terminal zero, delegate the camera body and physical return to existing
`delete_native_camera_00b71fe0(owner,1)`. Do not rerun virtual `+18`, decrement
again, or substitute the generated-model terminal destructor. Keep lifetime
associations available through native cleanup callbacks. The camera owner
already forgets its scene association after node destruction: do not call
`scenes.unbind` again, since that API reads a now-dead scene slot. Remove the
adapter's separate `GeneratedModelLifetimeRuntime` association by identity,
then retire the host companion through its supplied disposal contract. No
operation may touch native storage after the pool return, or the adapter after
its terminal disposal callback.

Queued camera preparation should resolve that same reference to `owner.frame`
and the same scene/system bindings through the existing
`RenderCommandSceneOperations`. Execution captures its camera before target
binding and adds no temporary reference. The existing contract still requires
the captured camera to survive those callbacks. A separate camera or frame
snapshot would defeat this ownership bridge.

## ABI, reentry and exception limits

These are proposed C++ bindings over existing storage, not native thiscall
or vtable replacements. Native `+04` interlocked operations are distinct from
host interface-object pointers. Valid array bounds, actual nonnull point-light
and attachment bindings, stable pool/runtime/environment lifetimes, and a
single canonical adapter remain explicit requirements.

Current queue release, model logical-release and child interfaces are
`noexcept`; the native camera direct/deleting functions can propagate failures
from their existing runtime bindings. A first bridge must therefore require
nonthrowing terminal dependencies throughout that call closure, including
retained `+438`, node/scene callbacks and host disposal. Missing bindings or
unsupported dispatch cannot become a silent successful release. Do not swallow
an exception, reset the count, or continue queue teardown as if destruction
succeeded. Full throwing callback/native EH parity requires separate queue and
shared-interface work and is not established by these reference bindings.

Retain the supported live callback order: publication before old release,
field clear after terminal callback, later field loads after earlier callbacks,
and child-head reload after each virtual release. Callbacks must keep active
queue slots and captured objects valid. They must not destroy an object still
being traversed or resurrect the zero-count camera while its vtable/lifetime
is changing. Concurrent mutation remains outside the existing hierarchy/runtime
contract. A concrete camera zero dispatcher must use or validate the actual
current camera profile; do not silently route an unsupported current table to
the cached camera destructor. Base-phase construction/teardown references must
not be published as completed queued cameras.

## Smallest ready packets

1. **Primary-owned shared prerequisite:** settle borrowed-count
   `RenderCommandReference` for camera and batch users, and extract the existing
   `00B6F310` traversal behind the borrowed release-byte/point-light view. Preserve
   default diagnostic behavior and existing generated-model delegates.
2. **Concrete camera bridge:** implement one stable multi-interface adapter,
   actual count binding, existing runtime association, camera-specific zero
   delegation and explicit companion disposal. Reuse `00B6DFA0`, the shared
   logical release, native scene removal, native camera destruction and pool.
   Do not fold batch lifecycle or command/context physical allocation into it.
3. **Focused validation after implementation:** extend the existing camera
   fixture with one retained-camera sequence: acquisition to count 2, actual
   logical release to count 1 with byte 44 set, repeated logical release without
   another self decrement, then queued final release and exactly one camera
   pool return. Inspect actual count/storage/association and existing light/child
   behavior; do not add a second suite or claim this discovery already ran it.

The current camera-owner batch should explicitly leave queued `+04` integration
and shared once-only logical release unresolved until those packets land.
