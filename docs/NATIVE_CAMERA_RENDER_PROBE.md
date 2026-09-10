# Native camera in the installed mesh probe

The installed mesh draw now constructs a camera in the actual 45Ch camera-pool
slot and uses its same node, pose, projection, frame, fog and viewport fields.
It no longer creates the diagnostic `CameraState` and `CameraFrameState` storage
for that draw. This is an isolated probe composition; it does not reconstruct
the game camera factory, complete renderer, command queue or gameplay.

## Renderer parameters reach the actual viewport

The probe owns one `NativeRendererParametersOwner`, initialized by the five
stores within native `00B32410`. The device-startup prefix writes that same
owner's width and height before `GetDeviceCaps` and `CreateDevice`. The owner
then travels by reference through the font/mesh entry points into the draw.
Its storage is distinct from `D3DPRESENT_PARAMETERS`, whose dimensions a device
callback can change independently.

The draw's actual `D3D9StateCache` has an explicit current parameter dispatch.
`D3D9ViewportRendererAccess` resolves that dispatch on each call and returns
references to the actual region. The viewport constructor reloads the published
renderer separately for width and height; the camera constructor captures it
once for its later height/width calls. These are the existing reconstructed
call sequences, not extra lookups in the adapter. Unbound or mismatched states
fail explicitly. Full native renderer publication and mode/reset updates remain
outside this partial renderer representation.

The installed run confirms constructor dimensions **640×480** from this same
startup owner. The probe then uses the recovered viewport size setter to choose
its established **256×256** draw rectangle. Its selected world matrix, projection,
FOV scalar, clear color and fog/light values remain explicit scene inputs.
Native camera defaults, including its initialized plane set, are retained.

## One camera allocation and counted context

`NativeCameraProbe` composes the completed camera pool, type bootstrap, owner,
reference adapter and actual 18h render context. The camera and directional light
share the same type counter, root/node descriptors, string pool, scene/runtime
bindings and allocator-list domain. A deterministic zero preimage is chosen for
the camera slot's first458h bytes; the actual pool ID at458h is preserved.

The actual camera constructor allocates its real34h viewport. A stable
`CameraViewport` view references that same owner, and the camera's frame resolves
only this identity. Rendering, system constants and material constants all use
the same camera transform/projection/frame backing. The compiled probe embeds
checked native table identities and constant words; it does not execute those
table addresses as host function pointers.

An explicit isolated18h context allocation uses the recovered placement fragment
within `00B1EDC0`. Its raw camera field receives `&camera_owner.storage.node`,
then retains that camera's actual04 count. The canonical companions contribute
no second count. The observed sequence is creator1, context2, logical release1,
context teardown0. The raw context destructor resolves the canonical camera
only when its actual count reaches zero; the camera then runs its complete
deleting destructor and returns its pool slot. Both companions retire afterward.
This does not imply construction of the surrounding44h command or a native queue.

The probe keeps one temporary fog observation reference across camera destruction
and confirms that destruction releases the camera's fog reference. It then
releases the observation reference. Pool teardown checks all32 camera slots free,
trims the empty slab, destroys its real Win32 critical section and unlinks it.
Probe-only guards remove camera/light host dispatch bindings after native pool
teardown, including constructor failure before successful initialization. This
host bookkeeping does not change the recovered native pool functions.

## Verification and limits

The strict MSVC Win32 build passes both existing tests. The independent context
fixture rerun matches **343 observations** with actual nested context owners.
The renderer fixture rerun matches original constructor/getter instructions and
startup callback snapshots, including zero dimensions, preserved padding,
divergent presentation/parameter fields and current-dispatch replacement.
The earlier camera owner/reference differential remains the instruction-level
evidence for its constructor, math, logical release and deleting destructor.

The installed probe exits0. Its actual camera backing, startup-owner identity,
viewport/clear/frustum calls, context count sequence, pool disposal, fog/light
cleanup, material/system constants and restored device state all check true.
The image has2499 visible pixels and54 colors and matches the previous diagnostic
camera image byte for byte: SHA-256
`226d25c25f50a04c2c8288f198ae2e0aa1341961418562f6186c98a4618c5f51`.
This is image regression evidence for the isolated mesh, not native-game visual
parity. The audit records exact native/source hashes and the review correction.

No tracked tests were added. Native exception dispatch, arbitrary derived owner
profiles, complete command/group/queue and texture ownership, the full renderer,
world startup and gameplay remain separate. The exposed C++ interfaces are not
drop-in native ABI replacements.
