# System lighting owner discovery

The next bounded implementation should create the actual ambient and
scene-resource owners, then bind their live fields to the existing system
prefix. Construction does **not** initialize every shader input. In particular,
mode 3 ambient, effective specular, mode 3 diffuse and the directional vector
retain allocation contents until a later operation writes them.

This is read-only discovery at `6363a09`. Function names below are descriptive
hypotheses except the class-name strings found in native type metadata. No
source implementation, Ghidra annotation, ledger change, test, build or game
validation was performed in this packet. Exact extents, ABI notes and matching
live/disk SHA256 hashes are in
[`system_lighting_owner_next.json`](../reports/system_lighting_owner_next.json).
All live queries verified project `bsp` and program
`/battlestationspacific.exe`; the configured project remains
`C:/Users/sqz269/bsp.gpr`.

## Actual owner chain

| Prefix view | Native construction and ownership |
| --- | --- |
| Outer scene, getter `00B72110` | `00B724E0` constructs a `0x24` owner, installs `D62D48`, and writes its `+1C` resource slot null. `00B723F0` assigns that slot by publishing the new pointer, retaining it, then releasing the old pointer. |
| Lighting resource at outer `+1C` | `00B83C50` constructs a `0x3C` scene resource with vtable `D63168`, refcount 1, copied name, and its existing linear-hash light registry at `+14`. It creates a `0x98` ambient owner and retains it through `00B825D0` into `+10`. |
| Ambient/environment at resource `+10` | `00B7C290`, vtable `D62F3C`, actual type name `cAmbientLight`. Type initializer `00CD8010` binds name `D62F04` and token `01090110`; its vtable getter `00B7AA00` returns that token. |
| First light from sentinel list | `00B7C6B0` calls the light base constructor `00B7C4C0` and installs directional vtable `D62FB0`. The base uses node constructor `00B6F5A0`. |

The scene resource is not another independent light collection. Existing
`SceneResource` and `SceneNodeRegistry` already implement membership and native
linear-hash ordering in `scene_attachment`. Extend or bind those same owners
when exposing stable sentinel/first-node views to the shader prefix.

The directional virtual `+50` is **`00B7C020`**, shared with the light base.
It checks its retained-scene vector at `+178/+17C/+180`, appends a distinct
resource, invokes the existing `00B83D50` light-registration gate, increments
the resource's reference count, then optionally recurses through child virtual
`+50`. It does not perform the ordinary node `+170` scene assignment. A generic
node-attachment override is insufficient for the actual first light.
Virtual `+54`, currently undefined as a Ghidra function at `00B7BD60`, is the
matching detach: erase-first/swap-last from the retained vector, unregister,
release the scene, then optionally recurse through child virtual `+54`.

## Constructor writes and allocator contents

| Owner field | Constructor result |
| --- | --- |
| Ambient `+18..27` | `(0,0,0,1)` |
| Ambient `+28..37` mode 3 | **Untouched** |
| Ambient six faces `+38..97` | Six copies of `(0,0,0,1)` |
| Ambient `+8/+C/+10` | Empty borrowed scene-backlink vector; these are not color fields |
| Ambient `+14` | `1.0`; its purpose is not renamed here |
| Light `+174` shadow | Null; no shadow-map object is constructed here |
| Light `+178/+17C/+180` | Empty retained-scene vector |
| Light `+184..193` effective diffuse | `(1,1,1,1)` |
| Light `+194..1A3` effective specular | **Untouched** |
| Light `+1A4..1B3` base diffuse | `(1,1,1,1)` |
| Light `+1B4..1C3` mode 3 diffuse | **Untouched** |
| Light `+1C4..1D3` base specular | `(0,0,0,1)` |
| Light `+1D4`, `+1D8` | `64.0`, `1.0` |
| Light `+1DC` specular scale | **Untouched** |
| Directional `+1E0..1EB` | **Untouched** |

These untouched-field findings include the node base: `00B6F5A0` writes only
through `+170`, and its three 64-byte matrix destinations are `+B0`, `+F0` and
`+60`. It does not initialize the derived regions in the table. Immutable native
words were checked: `D7A24C=3F800000`, `CE7820=42800000`, `CE3800=3F000000`.

Directional allocation is pooled. The currently named
`CG_static_dtor_stub_00B7BD40` is actually `MOV ECX,01090154; JMP 00B7BAC0`, an
allocation entry. Static initializer `00CD8080` constructs that pool through
`00B7B940` and registers `00CE0EB0`, which calls pool destructor `00B7B230`.
Each `0x3E44` slab holds 32 slots of `0x1F0`; `00B7AC90` writes only each slot's
slab index at `+1EC` plus slab free-index metadata. Object bodies keep malloc or
prior-use bytes. Directional deletion returns the slot through `00B7B2F0` to
the same pool; it does not CRT-free the object.

## Empty sentinel and lifetime

`00B83600` creates the registry's list and nine iterator pairs. Its sentinel
factory `00B82390` allocates 12 bytes and initializes only `+0` and `+4` to self.
The payload at **sentinel `+8` remains untouched**. The registry starts empty,
with count zero, mask 1 and one active bucket.

The shader prefix's `first==sentinel` check invokes `00BF6713`; that handler can
return. Its subsequent load from sentinel `+8` must retain the native preimage
semantics. The existing host registry's value-initialized sentinel key is not
evidence for a native null light. Do not turn this path into absence/success or
replace the loaded pointer with a guessed value.

`00B825D0` removes a resource from the old ambient backlink vector before
comparing pointers. It publishes/retains/releases only when the pointer changes,
then appends the resource to the current ambient vector even on a same-pointer
assignment. The vector borrows resource pointers; the resource holds the strong
ambient reference.

Ambient scalar destructor `00B7C7E0` calls `00B7C450`, whose complete body
resizes/frees its vector, installs `D5C104`, and calls `00BD30F0`; flag bit 0 then
controls owner free. Resource scalar destructor `00B83410` calls `00B82ED0`:
release ambient, free bucket vector, destroy list nodes/sentinel, release name,
destroy root base. List keys are borrowed lights and are not decremented there.
Directional destructor `00B7C820` calls `00B7C5B0`: unregister/release retained
scenes from the back, release shadow, free scene vector, run node destructor,
then optionally return the slot to its pool.

Incorrect no-return annotations at free wrappers hide essential instructions.
Use the full recorded extents: ambient destructor `B7C450..B7C4B2`, light
destructor `B7C5B0..B7C6A9`, list destruction `B829D0..B82A17`. Pointer reserve
also has required post-free publication at `B7B3E1..B7B3E9`.

## Bounded next packet

Implement `system_ambient_and_scene_resource_owners` in new
`system_lighting_owners.hpp/.cpp`, with the primary integrator owning shared
header and registry-view integration. The eight primary entries are:

| Entry | Complete byte extent, end exclusive | Native ABI |
| --- | --- | --- |
| Ambient construct | `B7C290..B7C419` | ECX owner, EAX owner, RET |
| Ambient destroy / scalar delete | `B7C450..B7C4B3`, `B7C7E0..B7C7FE` | ECX owner; scalar stack flags, RET 4 |
| Resource construct | `B83C50..B83D4D` | ECX owner, stack name reference, EAX owner, RET 4 |
| Resource set ambient | `B825D0..B82629` | ECX resource, stack ambient pointer, RET 4 |
| Resource destroy / scalar delete | `B82ED0..B82F82`, `B83410..B8342E` | ECX owner; scalar stack flags, RET 4 |
| Outer resource assignment | `B723F0..B7242B` | ECX actual outer owner, stack resource pointer, RET 4 |

Its bounded helper closure is `B7B390/B7B620/B7BC70/B7BD50/B7BF90` for
backlinks, and `B83600/B82390/B83220/B81B90/B82570/B829D0` for the native empty
registry storage. Reuse existing string pooling, allocation, refcounted root
semantics and live registry ordering. `B82570` is a custom register/stack ABI:
ECX destination, EDX pair count, first stack argument source pair, three other
stack words ignored, **RET 16**. Do not trust its pseudocode parameters.

This yields actual ambient/resource ownership and assignment to an already
supplied outer slot. A complete cold outer-scene/directional factory still has
named, incomplete dependencies:

- Outer constructor/destructor `B724E0/B72430/B72580` require weak-handle base
  `925490/925540`, manager/lock `924480`, allocator `9242F0`, actual weak-handle
  virtual destruction, and root-node unlink/release. The base creates an actual
  handle with a backlink, so it cannot be replaced by just refcount 1.
- Directional `B7C4C0/B7C6B0/B7C5B0/B7C820` requires the node base lifecycle,
  the actual `01090154` slab pool, type boot, and concrete `B7C020/B7BD60`
  attachment/detachment.
  The null constructor shadow slot needs no invented shadow object; a later
  nonnull owner still requires its real destruction and texture dispatch.
- Later lighting setup `004BACD0` is a separate bounded operation: ECX ambient,
  EDX directional, stack optional configuration, RET 4. With no configuration it
  uses guarded fallback colors `(0.5,0.5,0.5,1)`, base/effective diffuse via
  `004B62E0`, live direction globals `F8758C/90/94`, and base specular `(0,0,0,1)`.
  It **does not write effective specular or its scale on that branch**. A
  nonnull configuration supplies fields `F48..1008` and calls both `B7AF90` and
  `B7B010`. Its direction helper `004B4D80`, mutable global captures, and exact
  x87 multiply/spill sequence remain explicit work.

Fog construction, camera `+184`, material effect planes, whole-world setup and
full shadow/texture classes are outside this packet. No additional tests were
created for this discovery.
