# Fog and scene-lighting owner integration

The system prefix now accepts live outer-scene, scene-resource and environment
accessors and reads the same canonical light registry used by scene attachment.
The concrete fog, ambient-light and scene-resource modules are registered in the
Win32 build. These are new C++ interfaces; the original whole-object ABI and
world/camera/directional-light construction remain separate work.

## Fog construction and environment writes

`SystemFogOwner` has the verified `0x94` Win32 layout, with the existing
`SystemFogState` at `+8`. Its constructor preserves all 64 allocation bytes at
`+28..67`. The camera and D64518 receiver setters publish the new owner before retaining it
and releasing the old owner. Camera destruction leaves the old slot visible
through release/free, then clears it. Concrete destruction uses the recovered
`D63180` owner profile and the shared CRT allocator. The D64518 receiver belongs
to `game+19E8`, distinct from the world at `game+19CC`; the core API's existing
`world_owner` spelling is historical (see `SYSTEM_FOG_WORLD_FACTORY_NEXT.md`).

The interior `0078D076..0078D180` reads the actual environment regions and reloads
the camera's fog slot for every setter. Scalar writes retain the native
`FLD(source) -> owner reload -> FSTP(argument)` ordering. The nonmonotonic scalar
mapping, forward color overlap and unchecked native field meaning are documented
in [the environment fragment](ENVIRONMENT_FOG_APPLY.md). Its checked null-owner
error preserves earlier writes; it does not emulate the original pointer fault.

The installed-mesh diagnostic allocates this owner, transfers creator ownership
to the camera, populates it through the recovered environment fragment, and
clears the camera after use. Its scalar/color values are still explicit
diagnostic inputs. See [the owner evidence](SYSTEM_FOG_OWNER.md).

The preceding authored-data fragment `0078CAA4..0078CCE0` now writes the same
environment regions and updates the environment's distinct private `+B4` fog
owner. It captures that owner after the first scalar store and before the next
ten scalar copies; the later color call still uses that captured owner. Subsequent
calls reload the private slot. The private underwater color is not set by this
fragment. [Its evidence](AUTHORED_FOG_SOURCE.md) covers overlapping authored and
environment storage, raw copy order and eight x87 control words.

## One canonical registry

`SceneNodeRegistry` now owns raw 12-byte Win32 links containing next, previous
and a 32-bit key. Its sentinel retains the actual allocation's key preimage.
The embedded list owner puts the head at `+4` and count at `+8`. Each eight-byte
iterator stores that owner's address and a link address. Host attachment
associations are kept separately and contain no ordering links.

The native constructor allocates the sentinel before nine iterator records.
Deferred initialization lets the scene-resource constructor copy its pooled
name before making those allocations. Destruction frees iterator backing before
draining links and freeing the sentinel. Clearing restores nine iterator records
while retaining the existing allocation and capacity.

The private resize helper covers the registry's reachable capacities
`9,17,33,65,...`, where the requested count dominates native geometric growth.
It also preserves retained capacity after clear/reuse. It is not a general
replacement for `00B82FD0`. The full typed registry has additional host binding
storage and is not a native registry overlay. The [storage review](SCENE_REGISTRY_STORAGE_REVIEW.md)
records exact bytes, omitted post-free continuations and scope limits.

## Live owner and light selection

`SystemSceneResourceSlot` borrows the actual outer `SceneResource*` slot.
The resource's embedded lighting view reads its actual ambient-owner slot and
exposes its existing registry. No pointer slot or list is copied for the shader
builder. `SceneAttachmentRuntime` resolves real bound keys to explicitly supplied
directional-light projections, including a globally bound light outside the
current registry. An unbound nonzero key is a host binding error.

The prefix captures the sentinel and first-node identity once. If an empty-list
invalid-parameter handler returns, it retains that first identity and then reads
the live camera mode, environment and raw light key. Appending a new list node
inside the handler does not replace the retained sentinel. The existing
`00B46A70` register order and independent VS/PS uploads are unchanged.

Concrete ambient construction leaves mode-3 color `+28..37` untouched. Assigning
the same ambient owner removes and reappends its borrowed scene backlink without
a reference-count delta. Scene destruction releases its ambient directly; it
does not remove a backlink from an ambient that survives through another owner.
These native behaviors are retained in [the owner module](SYSTEM_LIGHTING_OWNERS.md).

## Validation and remaining work

The integrated `scripts/build.ps1` and both existing CTests pass. The installed
probe verifies all 77 system registers in both shader stages before and after
the two material uploads, the real fog camera ownership path, particle lifetime,
and D3D9 state restoration. It still draws 2,499 nonblack pixels and 54 colors.
Its bitmap matches the preceding diagnostic byte-for-byte and remains very dark;
this is not visual parity or gameplay validation.

Component evidence includes the fog owner's native constructor/setter/lifetime
comparison, the environment fragment's native finite/special-value comparison,
and the ambient/scene fixture covering live slots and a returning handler that
mutates the registry. The authored fragment passed 16 native comparisons across
two layouts and eight control words. One registry sequence matched 242 canonical
states and 328 allocation/free observations against the original code through
growth, duplicate insertion, partial erasure, clear, reuse and occupied
destruction. It preserves a seeded nonzero sentinel payload and retained capacity.
The fixture does not exercise allocation failures or native unwinding. Exact
logs and hashes are recorded in `reports/system_owner_integration_audit.json`.

The installed draw still supplies a diagnostic directional light through the
borrowed accessor helper. Concrete directional-light pool/base/virtual lifetime,
full outer-scene weak-handle ownership, authored world construction, shadow
texture classes and required game validation remain open.
