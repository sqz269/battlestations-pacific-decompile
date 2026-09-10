# Generated model release and terminal ownership

`generated_model_lifetime.hpp/.cpp` closes the model virtual `+18` operation
required by the existing render-command/group teardown. It performs the actual
point-light backlink removal, recursive child release, attached-owner unregister,
self reference release and terminal scene/resource cleanup. It uses the same
`CameraTransform`, `SceneNodeAttachment` and `GeneratedInstanceGeometry` objects
as the render/scene pipeline.

This is a new C++ interface targeting MSVC Win32. It is not a native object
overlay or a drop-in ABI replacement. Descriptive names are reconstruction
names. The constructor consumes explicit observed state; it does not recover a
native constructor or infer fields from zeroed host storage.

## Native path and evidence

The installed PE is `I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every live query verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 language and image base through `bsp.py ghidra`.
[The audit](../reports/generated_model_lifetime_audit.json) records complete
PE/live byte identities, original prototype/comment preimages, proposed
annotations, recovered ABI and per-function coverage. Shared raw exports remain
under ignored `exports/bsp`; complete original-byte disassembly used for the
audit is in ignored `local/generated_model_raw_<address>.txt`.

| Address | Established behavior | Original ABI |
| --- | --- | --- |
| `00B6F310` | Remove point-light backlinks and child owners; byte `+44` gates self release | ECX node, RET or tail virtual `+0` |
| `00B7C1A0` | Remove node from point-light `+1E0` borrowed array | ECX point-light, stack node, RET 4 |
| `00B7BED0` | Erase first equal pointer by swapping in last | ECX array, stack address of pointer, AL result, RET 4 |
| `00B6EC70` | Resize borrowed point-light array, null new slots, retain capacity when shrinking | ECX array, stack count, RET 4 |
| `00B6E500` | Reserve minimum-one exact requested native capacity | ECX array, stack capacity, RET 4 |
| `00B8F4C0` | Unregister matching attached owner and clear node `+A0` | ECX owner, stack node, RET 4 |
| `00B750C0` | Release/clear `+174`, then geometry `+180`, then node base destructor | ECX model, RET |
| `00B6F440` | Base node terminal ownership cleanup; bounded here to the path after virtual `+18` | ECX node, RET |
| `00B6EE10` | Remove matching scene with callback-safe publish/release order, recurse virtual `+54` | ECX node, stack scene/recurse, RET 8 |
| `00B75290` | Destroy model, then return physical storage if deletion flags contain bit 0 | ECX model, stack flags, EAX this, RET 4 |
| `00B74750` | Return model slot to the native `0x188`-byte object pool | ECX pool, stack model, RET 4 |
| `00B6E680` | The only used path is existing-parent-equals-null immediate return | ECX node, stack parent, RET 4 |

Generated-model vtable `00D62DE8` resolves `+18` to `00B6F310`, `+54` to
`00B6EE10`, and `+4` to `00B75290`. Its `+0` is the already-reviewed
`00BD30E0` trampoline, which calls `+4` with deletion flag 1.

Two suspect no-return annotations required original-byte inspection:

- `00B6E54C` calls `free`; `00B6E551` resumes with stack cleanup, publishes the
  new array pointer and capacity, restores EBX and returns at `00B6E55C`.
- `00B6F519` calls `free`; `00B6F51E` resumes with name-pointer load and stack
  cleanup, returns pooled name storage using length `+54` plus one, performs
  refcount-base destruction and returns at `00B6F568`.

The audit includes those continuation instructions and full exclusive bounds
`00B6E55F` and `00B6F569`. The worker performed no Ghidra mutation; the primary
integrator owns annotation/ledger changes, export refresh and any body repair.

## Two release phases

`release_generated_model_00b6f310` first removes this node from every linked
point-light's borrowed reverse-link array and shrinks its own `+164` list to zero.
It then repeatedly removes the current first child from the shared hierarchy
and invokes that child's actual virtual `+18`. The new head's previous sibling
is cleared, and the detached child's parent and next sibling are cleared before
the call. The first-child pointer is reloaded afterward. The native routine
does **not** decrement or reset child count `+38`; the reconstruction preserves
that observation.

These backlink/child operations run even when released byte `+44` is already
set. Only the first call clears the remaining root/hierarchy links, sets the
byte, unregisters `+A0` and drops one self reference. If references remain,
owned scene `+170` and geometry `+180` remain alive. Repeated virtual release
does not drop another self reference.

When the last reference reaches zero, the recovered terminal order is retained
owner `+174`, geometry owner `+180`, attached-owner cleanup, retained owner
`+130`, current scene, point-light array storage, pooled name storage, then
physical model storage. Each retained field is released before the field is
cleared; later fields are loaded after prior terminal callbacks.

The full base destructor calls `00B6E680(0)` and contains root/child registration
logic. On the concrete virtual-release path, parent/root/first-child are already
null. The parent helper's equality branch returns immediately and the
registration loop is empty. The interface explicitly requires that state at
terminal release, and rejects a terminal call before virtual `+18`. General
direct destruction and callbacks that rebuild the dying hierarchy or reattach
its scene are outside this bounded destructor contract.

## Scene and identity semantics

`remove_generated_model_scene_00b6ee10` operates only when the current scene
equals its expected scene argument. It uses the reconstructed scene registry
and actual type predicate, reloads the current scene after that callback,
publishes null, and then releases the current scene reference. A new scene
installed by the terminal callback remains published; this standalone helper
does not clear it afterward. Recursion invokes each child's actual virtual
`+54` and reloads the next sibling after the call. This differs from simply
calling the existing scene setter with null.

Native `+A0` is projected by the shared transform's `notification_context`.
`GeneratedModelAttachmentLinks` associates that same owner identity with its
actual borrowed reverse links; it does not add another node-owned attachment
field. Unregistering a matching owner clears both the projected context and its
notification function so subsequent transform notifications observe detachment.
Pointer erase removes the first match by moving the last element; it never
releases pointed objects.

## Integration and remaining boundaries

`GeneratedModelConstructionState` transfers already-retained `+174`, typed
geometry `+180`, and `+130` references, along with owned array/name storage. It
requires the actual positive self-reference count and released-byte state.
`GeneratedModelGeometryReference` owns the existing shared generated geometry,
whose stream/material references are released when its final owner disappears.
Opaque retained fields require actual `RenderCommandReference` implementations
when present.

The caller must bind the stable scene attachment in `SceneAttachmentRuntime`
and the actual model operation in `GeneratedModelLifetimeRuntime`. Every
traversed child and every nonnull attached-owner identity needs its actual
association. A missing association is a contract failure; there is no no-op
fallback. Terminal cleanup detaches and **unbinds both model associations**
before invoking the storage owner, so a containing model destructor must not
unbind them again.

`GeneratedModelStorageOwner::dispose_model_storage` is the remaining required
physical allocation policy. It must delete the actual host model allocation or
return it to its real pool; it may delete the lifetime object and no later
operation accesses that object. The native `00B74750` pool bookkeeping uses
block index `+184`, slot stride `0x188`, free-index storage and a critical
section. Reproducing that physical allocator is outside this interface.
Diagnostic bytes return to their actual supplied `std::pmr::memory_resource`
with length plus one and alignment one; native global size-class allocation
is not claimed.

Invalid/negative arrays, overflow, allocation-failure behavior, Win32 exception
unwinding, general reparent/animation systems and original object layout are
outside the contract. Valid point-light backlink entries must be nonnull. Host
containers preserve the relevant ownership and capacity behavior without
claiming native allocator layout or failure semantics.

## Validation

- All eight existing native seed ranges matched the installed PE.
- The module and one ignored local fixture compiled as MSVC 19.51 x86 C++17
  with `/O2 /MD /W4 /WX /fp:strict`.
- The fixture passed recursive child deletion, first-match swap-last reverse
  links, unchanged child count, retained array capacity, repeated byte gate,
  scene/geometry survival while the parent still has a reference, and final
  `174 -> 180 -> 130 -> scene -> name -> physical storage` order. Its scene
  callback observed the null scene publication; actual geometry/model
  allocations were deleted and pooled name extent was checked. Detached scene
  bindings could be rebound after terminal cleanup.
- `scripts/build.ps1` now builds the integrated source and passes both existing
  CTests. The installed mesh probe exercises actual command/group teardown and
  three physical model disposals through the shared control adapter. The
  corrected point-light fixture also passes against the integrated source; see
  `reports/frame_bounds_integration_validation.json`.

The new lifecycle is source/evidence reviewed and fixture/build tested. It has
not been native differential tested as a lifecycle, linked as a binary
replacement, or validated in the running game.

The model `+164` list contains point lights, as established by the shared
`00B6DC50` getter and both `00B55780` and `00B42350`: light `+1EC` supplies
position/radius, `+184` supplies color, and `+1E0` holds model backlinks.
`GeneratedModelPointLightLinks` therefore shares borrowed light identity, values
and backlinks; it does not bind a generated mesh to this list. Geometry `+180`
is a separate retained owner. The installed diagnostic scene supplies no point
lights, and its upload writer reads this same empty list. This corrects the
initial geometry interpretation; earlier pointer-order fixture results alone
did not establish the object type.
