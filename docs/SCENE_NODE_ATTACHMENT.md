# Scene references and the light registry

`00B6ED80` assigns a scene reference to a node and optionally dispatches the same
request recursively through native virtual `+50`. The scene's container at `+14`
is a **light registry**. Its gate uses `cLight`'s runtime type ID, so the ordinary
`c3dObject` predicate at `006EF860` rejects it. Attaching a generated object to a
scene therefore changes reference ownership without inserting it into this
registry. A concrete light subtype can accept the gate through its own virtual
`+0C` override.

`scene_attachment.hpp` and `scene_attachment.cpp` reconstruct this service with
new C++ storage. They reuse `CameraTransform`'s existing child/sibling hierarchy
and implement the container internally. They are not native object overlays or
drop-in x86 replacements. Descriptive names are reconstruction labels.

## Native attachment ordering

The assembly for `00B6ED80..00B6EE0B` establishes this order:

1. Compare the node's current scene at `+170` with the requested scene.
2. If different and non-null, call `00B83EC0` on the old scene with this node.
3. Reload `node+170`. The removal type callback may have reentered attachment.
4. If the reloaded reference differs from the request, publish the request,
   atomically increment its `+4` reference count, and atomically decrement the
   reloaded old reference. Invoke the old scene's virtual `+0` if it reaches zero.
5. Reload `node+170` again and, if non-null, call `00B83D50` on this current scene.
   The destruction callback may have replaced the requested scene.
6. If recursion was requested, load `node+34` and call each child's virtual `+50`
   with the original requested scene and recursion set to one. Load that child's
   `+3C` next sibling **after** its callback returns.

The recursion runs even when this node already has the requested scene. The
routine does not assign camera/world/bounds cache flags or alter parent links.
`00B83D50` and `00B83EC0` capture their scene receiver before invoking the node's
type predicate; they operate on that receiver even if the predicate changes the
node's scene reference.

## Type identity evidence

The runtime values are IDs, not the addresses of the globals. The audit includes
saved-image/installed-PE comparisons for the leaf, initializers and supporting
helpers; no numeric runtime ID is guessed.

| Descriptor | Evidence | Runtime values |
| --- | --- | --- |
| `cRoot` | `00BEA780`, descriptor name string in its body | `0109DB84` |
| `c3dNode` | `00B6F110`, static initializer `00CD7D30..00CD7D72`, name `00D62C7C` | Own ID `0108FF90`, inherited root ID `0108FF94` |
| `c3dObject` | `00CD7E60..00CD7EAE`, name `00D62DD4` | Own ID `01090034`, copied node/root IDs `01090038/3C` |
| `cLight` | `00CD80A0..00CD80EE`, name `00D62F14` | Own ID `0109018C`, copied node/root IDs `01090190/94` |
| `cDirectionalLight` | `00CD80F0..00CD8190`, name `00D62F1C` | Copies the light/node/root IDs before allocating its own ID |
| `cPointLight` | `00CD81A0..00CD8240`, name `00D62F30` | Copies the light/node/root IDs before allocating its own ID |

`006FAC20` provides the shared singleton whose counter at `+4` starts at zero.
Each guarded initializer copies inherited IDs, consumes the current counter and
increments it. Thus initialized `c3dObject` and `cLight` are sibling types with
different own IDs. Their common node/root ancestors also have different IDs.
This identity conclusion assumes valid one-time type initialization and a
non-wrapped counter; it does not depend on static initializer order or invented
ID constants.

`00B7AA70` returns the value at `0109018C`. The 40-byte `006EF860..006EF887` leaf
compares its stack token to the three values at `01090034/38/3C` and returns AL
true only for a match. Its code was present but not defined as a Ghidra function
at initial capture. The primary integrator owns definition and annotation.

## Container and lifetime

`00B83700` is unique pointer-key insertion into a circular doubly linked list
partitioned by linear-hash bucket boundaries. It is not a tree. Native entries
allocated at `00B823B0` contain exactly three 32-bit fields: next, previous and
the copied pointer key. Insertion does not retain the pointed-to node.

The checked list-size increment at `00B82D30` is a returning function for valid
sizes. It raises `length_error` only when the increment exceeds the `3FFFFFFF`
limit. The existing `STL_xlen_throw` label alone is insufficient to infer its
control flow.

The hash XORs the pointer key with `DEADBEEF`, performs signed quotient/remainder
division by `1F31D` through `00C03DBE`, computes `remainder*41A7 - quotient*B14`
with 32-bit wrapping, and adds `7FFFFFFF` when the result's sign bit is set.
The mask and active bucket count choose a bucket; keys inside it are ordered by
unsigned pointer value.

The empty state established by `00B83BE0` has nine iterator boundaries, mask
one and one active bucket. Insertion splits one bucket when
`active_bucket_count <= size/4`, **before** it tests for a duplicate. A split
moves qualifying entries to the list tail and updates boundaries. Growing the
boundary array doubles its bucket capacity. Existing entries remain stable.

`00B82650` finds the equal range; `00B820B0` counts it; `00B83D90` erases it.
Unique insertion limits an erase-by-key result to zero or one. Deleting the last
entry takes the whole-range clear path and resets the bucket state. Single-entry
erasure at `00B827C0` unlinks and frees the list node, decrements list size and
returns the following iterator. Neither erase nor clear releases model/light
references. The scene registry borrows bindings; only `node+170` owns a scene
reference in this reconstructed service.

The subsequent [owner integration](SYSTEM_OWNER_INTEGRATION.md) replaces the
embedded host sentinel and reduced boundary pointers with canonical raw12-byte
links and8-byte owner/node iterators. A native-shaped list owner puts head at+4
and count at+8. Sentinel payload is retained from allocation; clearing preserves
the iterator allocation/capacity. System lighting reads these same nodes through
live accessors. Concrete directional-light attachment remains separate: its
actual virtual+50/+54 uses the light's scene array, not the ordinary node+170
assignment reconstructed here. A focused native growth/clear/reuse comparison is
recorded in `reports/scene_registry_native_check.json`.

The saved disassembly had two gaps after calls to `_free`. Both full function
ranges match the installed executable. The audit preserves the exact bytes for
the primary integrator's serialized analysis repair:

| Function and call | Missing continuation | Established behavior |
| --- | --- | --- |
| `00B827C0`, call `00B828D8` | `00B828DD`: `83 C4 04 83 46 0C FF` | Pop call argument and decrement registry list size |
| `00B83BE0`, call `00B83C07` | `00B83C0C`: `83 C4 04 3B 5E 04 8B C3 75 EE 5B` | Continue freeing all entries, then reset boundaries |

## Consumer contract

Use one stable `SceneNodeAttachment` for each existing `CameraTransform` and
call `SceneAttachmentRuntime::bind` for every node that recursion may visit.
The explicit `pointer_key` is the node's native pointer value when available;
the reconstruction's Win32 consumer uses the actual stable binding address.
Synthetic asset IDs are not native pointer identities.

Supply the actual virtual `+0C` predicate and virtual `+50` override. Generated
objects whose native vtable selects `006EF860` use
`object_accepts_scene_type_006ef860`; its `object_type_tokens` must be the three
object/node/root values, distinct from `registry_type_token` for light. A test
may use distinct surrogate IDs preserving this relationship. Positive light
branch tests require a separate light predicate; setting the ordinary object's
type IDs equal to the light ID would encode false native behavior.

`SceneResource` requires an explicit initial reference count and zero-reference
destruction callback. That callback represents native virtual `+0` and may
delete the scene or reenter the attachment service. Constructors do not acquire
a hidden owner reference. Destructors do not automatically detach nodes; that
behavior has not been recovered from the actual native node destructor.

Detach nodes explicitly through `set_node_scene_00b6ed80` while their scenes,
bindings and transforms are alive, then unbind them. Removing a binding while
its scene reference is non-null is rejected by the new host interface. The
caller must also remove links that would traverse an unbound transform. A child
callback may change sibling links, but the current child must remain alive for
the post-callback sibling load. Reference counters are atomic; registry and
hierarchy mutations require the caller's serialization, as the native routines
provide none.

The new interface uses C++ allocation and omits native debug-iterator ownership
wrappers/invalid-iterator diagnostic machinery. It implements valid-container
behavior and borrowed pointer lifetime. Native vector allocator growth,
low-memory diagnostic ABI and complete scene/model destructors remain outside
this packet.

## Validation and evidence boundary

`reports/scene_node_attachment_audit.json` records native ABIs, prior names and
comments, full PE range hashes, free-call continuations, type identity proof and
annotation proposals. The primary integrator applies shared ledger/Ghidra
changes; this worker does not mutate the saved analysis.

The source compiles with MSVC Win32, `/W4 /WX /fp:strict`, and a focused local
check passes with runtime checks enabled. It crosses the initial bucket
capacity, verifies insertion and erasure against a set, tests duplicate-triggered
splitting and last-entry reset, confirms scene-release reentry, verifies ordinary
object rejection of the light gate, and checks a callback changing its next
sibling. The positive gate uses an explicit test light override with distinct
surrogate IDs. No broad test suite was added.

`scripts/build.ps1` passes its two existing tests after verified native seeds.
That worktree's CMake source list is controlled by the primary integrator, so
the new source was also compiled and exercised directly. Native code byte parity
is verified; native ABI interchangeability, native differential execution of the
complete scene service and in-game behavior are not claimed.
