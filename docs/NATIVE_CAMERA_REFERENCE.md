# Camera references over the actual native owner

`NativeCameraReference` connects the completed native camera to the existing
queue and node lifetime interfaces. It borrows `NativeNodeStorage.references_04`
and registers its node interface in the same `GeneratedModelLifetimeRuntime`
used by the camera's hierarchy. Binding adds no reference and initializes no
native word. Runtime uniqueness enforces one adapter per transform identity.

`RenderCommandReference` now exposes an atomic reference. Its default constructor
creates the previous diagnostic count of one inside optional owned storage. Its
explicit borrowed constructor leaves that optional storage disengaged and binds
the supplied atomic directly. There is no second live count or synchronization
between counters. Existing retain, release and identity-assignment helpers retain
their sequentially consistent operations and publication/callback order.

## Separate logical and terminal release

| Native route | New interface behavior |
| --- | --- |
| `00B6DFA0`, ECX node, tail virtual `+18` | Reuse the existing complete hierarchy-unlink implementation with the camera adapter |
| `00B6F310`, ECX node, RET or tail virtual `+00` | Delegate to the shared borrowed-state traversal over actual `+44`, hierarchy and point-light descriptor |
| `00B6EC70(0)`, ECX descriptor, stack zero, RET4 | Reuse the extracted existing raw shrink loop; pointer, capacity and elements survive |
| `00B1D120`, ECX context, RET | Existing queue context release decrements actual camera `+04`, invokes terminal release only at zero, and clears its field afterward |
| `00BD30E0 ->00B71FE0(1)` | Validate the current camera profile/slots, run the completed camera destructor, then return its actual pool slot |

The shared logical-release implementation first walks live point-light links,
shrinks the borrowed descriptor, and releases live children. It tests `+44` only
after that work. On the first release it captures attachment `+A0`, clears the
hierarchy links, sets the release byte, unregisters the attachment, and releases
one self reference. The existing generated-model entry now delegates to the same
body with its diagnostic vector view. No camera pointer list is copied into a
vector. The raw camera view reads actual `+164/+168/+16C`; its shrink operation
does not perform the final array free owned by node destruction.

Final queued release has already decremented `+04`. It must neither decrement
again nor inspect `+44` as a deletion gate. The camera adapter checks the actual
current profile `00D62CF0`, virtual zero `00BD30E0`, and deleting slot
`00B71FE0`, then invokes the existing deleting destructor with flag one. Its
scene-removal adapter accepts the established camera/base-node `+54` routes.
Unsupported profiles terminate within the shared `noexcept` interface; they
never receive a guessed virtual implementation.

## Companion lifetime

The native slot and both host companions must remain alive while queued
references exist. Logical release does not destroy the adapter or the
`NativeCameraOwner`. Once bound, native terminal destruction belongs to the
adapter's zero path; directly destroying the owner would violate that contract.

The adapter keeps its hierarchy association available through native destruction
callbacks. The camera destructor already removes the scene association. After
pool return, the adapter removes only its separate lifetime association using
pointer identity, marks itself retired, and invokes its explicit companion
retirement callback as the last operation. That callback may dispose a heap
allocation or record retirement of stack/arena companions for later disposal.
No native slot is read after pool return, and no adapter is accessed after the
callback. Destroying a still-bound adapter terminates, since queued or hierarchy
references would otherwise dangle.

These callbacks must be nonthrowing to satisfy the existing queue and node
interfaces. The native camera implementation can propagate dependency failures
when used directly; this bridge does not establish native exception parity.
Callbacks must not resurrect a zero-count camera, invalidate the active queue
slot, or destroy a camera still being traversed. Concurrent hierarchy mutation
and invalid raw array descriptors remain outside the supported domain.

## Verification

The existing owner fixture was extended with one retained-camera trajectory.
It executes original `00B6DFA0`, `00B6F310`, `00B1D120`, and `00BD30E0` in
addition to the already checked complete camera/node/viewport/pose and pool
return code. The normal count sequence matches: creator one, queue two,
logical release one, repeated logical release one, final context release zero.
The actual shadow-order viewport clear precedes logical release. A queued
camera remains allocated and its companion remains live despite that clear;
queue ownership protects lifetime, not an earlier snapshot of camera fields.

Native and reconstructed paths match all 17 complete 1116-byte camera snapshots
in that trajectory, viewport snapshots, x87 status/control, MXCSR and nine
ordered allocation/renderer/retained-owner events. The final path frees the
remaining retained owner and returns the camera slot exactly once. The earlier
eight constructor trajectories and two host exception boundaries still pass.
The retained trajectory has an empty native hierarchy/light list; the shared
release component's separate host sequence covers diagnostic and raw lists,
live child insertion, repeat traversal, retained capacity and a terminal callback
that makes raw backing inaccessible before pointer-only association removal.

The strict Win32 build and both existing CTest checks pass. No tracked tests
were added. The shared state operations and camera adapter are new C++ interfaces,
not original ABI replacements. The installed rendering probe still uses its
diagnostic camera until actual renderer parameter storage/publication is
reconstructed; native context/command allocation and full render integration
remain separate work. See `reports/native_camera_reference_audit.json` for the
exact source, native-byte and fixture evidence of this integration batch.
