# Native camera ownership: next implementation boundary

Read-only discovery against `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, starting from main `230a5dc`. The accompanying
`reports/native_camera_owner_next.json` records 52 live byte spans, 6,504 bytes,
all equal to the installed executable or its section zero fill. The later pool
fixture corrected the return wrapper to its complete 12-byte body, bringing the
checked span total to 6,506 bytes; the original truncated preimage is preserved
in the report's correction record. This packet
changes no C++, Ghidra annotations, ledgers, game files, or tests. No constructor,
destructor, renderer, or game was executed during this discovery.

The camera required by `00A8E2E0` is a real `45Ch` pool slot: the existing native
node prefix occupies `000..173`, camera fields occupy `174..457`, and pool slab
identity occupies `458..45B`. Its constructor is `00B71A80`, direct destructor
`00B71F10`, deleting wrapper `00B71FE0`, and final vtable `00D62CF0`. Camera
construction is still incomplete. The smallest independent implementation
packet ready from this discovery is its distinct pool, detailed below.

## Layout and construction

`00B71A80 [00B71A80,00B71CDC)` receives owner in ECX and a native string pointer
on the stack, returns the same owner in EAX, and uses `RET 4`. It first calls
the existing `00B6F5A0` node constructor, then installs camera table `00D62CF0`.
Do not initialize a second node/transform or overwrite pool identity `+458`.

| Camera offset | Native constructor and consumer evidence |
| --- | --- |
| `174`, `17C` | Both bytes become 1; padding `175..177`, `17D..17F` is untouched. |
| `178` | Raw `46EA6000` from `00CE77FC` (30,000). Meaning beyond the existing consumers remains provisional. |
| `180` | Owns the actual newly allocated `34h` viewport address; initial reference is the constructor's count 1. |
| `184` | Actual fog-owner pointer, initialized null. Existing fog fragment covers this store only. |
| `188`, `18C`, `190`, `194` | Clear flags 0, depth `3F800000`, color 0, stencil 0. |
| `198`, `19C` | Render mode 0 and its bit mask 1. `00B6FDF0(mode)` later stores mode and `1 << (mode & 31)`. |
| `1A0`, `1AC` | Target/direction vectors written through the look-at and world-matrix path, not independent defaults. |
| `1B8..1C3` | Three positive-zero float words. |
| `1C4`, `1C8` | FOV bits `3F32B8C3` from `00CE7D20`; aspect bits `3FAAAAAB` from `00D5BD98`. |
| `1CC`, `1D0`, `1D4`, `1D8`, `1DC` | Zero, zero, one, `47435000` (50,000), `44000000` (512). Keep the two intervening words between aspect and near plane. |
| `1E0`, `220`, `260`, `2A0` | Four 64-byte projection/view-projection/inverse/cache matrices. This constructor does not initialize these caches. |
| `2E0..2EF` | Extra clip-plane words, untouched by construction. |
| `2F0` | Final assignment is 1. Earlier look-at wrappers read/AND the existing word before this final assignment. |
| `2F4..433`, `434` | Sixteen 20-byte plane records, then count. `00B659D0` initializes all records, installs six identity-frustum planes with flags 7, and writes count 6. |
| `438` | Separate intrusive owner pointer, initialized null. Its complete pointee type/producer is not established. |
| `43C` | Separate borrowed float4 pointer, initialized null; getter `00B6FEB0`, setter `00B6FEC0`. Destructor does not release it. |
| `440..44B`, `44C..457` | Axis Y `(0,1,0)` and axis X `(1,0,0)`. |
| `458..45B` | Live pool slab identity. Camera and node constructors/destructors leave it untouched. |

After allocating/constructing the viewport, the camera writes `+184=0`, zeros
`+1B8..1C0`, calls `00B659D0(camera+2F4)`, then writes `+438/+43C=0`. It sets
viewport origin to zero through `00B1F920`. Renderer access has a specific order:
capture current `00F8D394`, invoke its current virtual `+30`, save returned
parameter `+10` (height), reload that captured renderer's vtable, invoke `+30`
again, then load returned `+0C` (width). `00B1F940` receives this width and the
saved height. This differs from the viewport constructor's width-first order.
Use the same renderer environment as the viewport owner; do not snapshot dimensions.
The camera then calls viewport `00B1F750(0)` and `00B1F760(1)`.

The look-at call is `00B700E0(camera, eye, target)` with eye `(0,100,0)` and
target `(0,100,100)`, using exact `42C80000` at `00CE3D08`. Its pseudocode loses
the second pointer argument. Assembly proves `RET 8`, first stack pointer in
EBX, second pointer reloaded at `00B70104`, and actual camera virtual `+30`
before the target copy. For the camera table, `+30=00B71400`, which calls base
world-position setter `00B6DAE0` and existing `00B70660`. The base setter edits
the existing world matrix translation and tail-dispatches camera virtual
`+34=00B71460` with that same world matrix. The later look-at path calls
`00B63F10` with world-up `(0,1,0)`, the existing inverse `00B63B30`, then current
virtual `+34` again. Preserve its x87 spills, live dispatch, and callback order;
do not replace this closure with identity plus a guessed translation.

`00B63F10` is a 978-byte x87 look-at builder with four `00419440` length calls,
two `004F9B30` cross calls, near-parallel handling and explicit float spill
boundaries. The vector kernels already exist privately in
`src/system_camera_axes.cpp`, with actual CRT access available through
`CameraAxesCrtAccess`; exposing/reusing them is a primary-owned shared change.
`00B652D0/00B659D0` are new plane-set construction wrappers around the existing
frustum extraction/assignment behavior. They are not recovered by calling
`get_camera_frustum`, whose dirty-cache and count behavior differs.

## Retention and destruction

`00B71990 [00B71990,00B719D1)` receives camera in ECX, viewport owner on stack,
and uses `RET 4`. Equal identity returns without access to either refcount.
Otherwise it publishes the new raw owner at `+180`, increments new `+04`, then
decrements the captured old `+04` and calls its current virtual `+00` on zero.
A release callback can observe/rewrite the newly published slot. Do not clear
or rewrite that slot after the old release.

`00B71F10 [00B71F10,00B71FD2)` installs the camera table, then releases current
`+180`, current `+184`, and current `+438`, in that order. Each nonnull field is
captured independently, decremented at actual `+04`, dispatched through current
virtual `+00` on zero, and cleared only after that callback returns. Each next
field must be reloaded after earlier callbacks. A null field skips its clear.
It then calls existing direct node destruction `00B6F440`, which changes the
vtable/dispatch phase and ends the node prefix. The camera tail remains the
same storage through this sequence. Do not release `+43C` or invent an owner
for it. Nonnull `+438` requires a real registered owner/count/zero callback;
the observed constructor's null default does not authorize a fake release.

`00B71FE0 [00B71FE0,00B72000)` calls that direct destructor, then, only for
`flags & 1`, calls `00B711E0` on the same pool at `0108FFB0` with the original
slot. It returns that original pointer even after storage return. Shadow
destruction first calls `00B71990(camera,null)`, then the existing
`00B6DFA0` unlink/virtual-18 release. A queued reference can keep the actual
camera alive; do not replace this with unconditional destruction.

The exact constructor EH descriptor `00DFAA38` references map `00DFAA20`:

| State | Action | Next |
| --- | --- | --- |
| 0 | `00CC1AD0 -> 00B6F440` on actual camera | -1 |
| 1 | `00CC1AD8`: raw in-progress viewport delete `00BF65AC`, followed by actual `POP ECX; RET` | 0 |
| 2 | `00CC1AE3 -> 00605FD0(camera+438)` | 0 |

State 1 is active only around viewport construction; state 2 begins after
`+438=0` and before later renderer calls. The map does **not** separately
release an already published `+180` or `+184`. Preserve that boundary even if
it seems unlike a preferred host cleanup policy. Direct destructor descriptor
`00DFAAA0`, map `00DFAA90`, has state 1 clear/release `+438` via
`00CC1B28/00605FD0`, then state 0 node destruction via `00CC1B20`. The direct
body changes to state 0 before its explicit `+438` release, so a throwing
`+438` callback is not repeated. No native exception path was executed here.

## Type and pool boundaries

The camera descriptor is `0108FFA0`, name literal `00D62CE4 = "cCamera"`,
guard `0108FF9C`. Virtual `+08=00B6FB60` loads its actual first token.
Virtual `+0C=00B71CE0` scans three initialized tokens at `0108FFA0/A4/A8` and
returns AL/`RET 4`; ECX owner is not used. Static `00CD7D80` and lazy
`00B719E0/00B71A30` set the guard, call existing node bootstrap `00B6F110`, copy
node/ref ancestry from `0108FF90/94`, and consume the **same** actual
`006FAC20` counter. Static zero-filled bytes are not runtime type IDs.
The camera constructor itself does not bootstrap these descriptors.

Camera pool `0108FFB0` has the existing allocator-list shape but independent
storage and table `00D62CE0`. `00B715F0` registers with actual `00E188B4`, starts
the real Win32 critical section, initializes fields, and reserves 32 slab
pointers. Each `8BC4h` slab holds 32 `45Ch` slots, 32 free-index `uint16` entries
at `8B80`, count at `8BC0`, and two untouched padding bytes at `8BC2`.
`00B6FED0` initializes reversed free indices and all slot `+458` identities.
`00B71770` acquires the section, increments recursion depth, allocates/grows as
needed, pops a free index, updates first available slab, decrements recursion,
and leaves the section. No allocation rollback/unlock EH map is present there.

`00B711E0` uses the actual `+458` identity to return a slot and update the first
available slab. `00B716D0` trims fully free slabs without an internal lock,
swaps the last slab into a hole, and rewrites **all 32** moved `+458` identities.
`00B71120` frees every slab and the pointer array, drains positive lock recursion,
deletes the section, then unlinks the allocator-list node. Raw bytes confirm
the post-free continuations that pseudocode incorrectly suppresses.
Constructor EH `00DFA9FC/00DFA9E4` uses pointer-array free `00B6FFC0`, actual
section cleanup `00402F70`, and allocator-list unlink `00403970` in reverse order.

Allocation entry `00B71930 -> 00B71770`, return entry `00B71350 -> 00B711E0`,
static construction `00CD7DD0`, and registered exit `00CE0E30 -> 00B71120` all
use `0108FFB0`. Return wrapper `00B71350..00B7135C` takes the slot in ECX,
pushes it, sets ECX to the canonical pool, calls `00B711E0`, and returns with
plain RET. Its bytes are `51 B9 B0 FF 08 01 E8 85 FE FF FF C3`; a 10-byte
prefix ends inside the CALL and is not the full wrapper. The current provisional `00B71930` scene-node allocator label
should become a camera-pool name when this implementation is annotated.

## Shared adapter integration required

The primary owns these shared edits; this discovery does not make them.

| Existing API | Required backing/ownership change |
| --- | --- |
| `NativeNodeBinding` and `CameraState` | `CameraState` must reference the already bound node transform, plus actual `220`, `1AC`, `1A0`; it must not construct another `CameraTransform`. Preserve existing diagnostic value construction through an owned fallback. |
| `CameraProjection` | References to actual `1C4`, `1C8`, `1D4`, `1D8`, `1E0`, `2A0`, `2F0`. Its getter currently uses `&fov + {0,1,2,3}`; replace with four explicit field addresses, preserving native far/near/aspect/FOV `FLD/FSTP` order at `00B6FCFF..00B6FD31`. |
| `CameraFrameState` | Bind actual `174`, `17C`, `188..198`, `260`, `2F4`, `43C`, `440`, `44C`, and the same `CameraState`. Keep one shared `2F0` word across projection, frustum, inverse and axes. |
| Frame viewport | Raw `+180` stores `NativeViewportOwner*`, not a `CameraViewport` view. Resolve a stable borrowed view of that same owner on native accesses. Its borrowed renderer identity must survive storage at renderer `+1904`. |
| Frame fog | Raw `+184` stores `SystemFogOwner*`, not `&owner.fields_08`. Add raw-owner access/overloads; preserve the legacy typed-view slot for diagnostics without mirroring either slot. |
| `set_system_fog_camera_owner_00B71940`, initialize/clear camera-slot fragments | Native raw-slot variants must publish/retain/release/clear the actual owner word, retaining current concrete fog lifetime and callback order. |
| `initialize_world_fog_004DF6A3` | Its reloaded `CameraFrameState` publication must route through the actual raw `+184` variant when bound. |
| `apply_environment_fog_0078D076` | Every current slot load stays live. Its scalar helper does `FLD source -> MOV owner from slot -> FSTP argument`; adding owner/view resolution must not move the slot capture across those operations. |
| `write_system_time_constants_00B46CB4` | At `00B46D79`, ESI captures actual camera `+184` before final byte conversion and `c75.y` store. Return/resolve that captured owner after preserving the native capture point. |
| `write_system_fog_constants_00B46D97` | Use captured fog for the initial scalar/directional writes, reload live owner at `00B46E69` for c37, and preserve `MOV color.w -> MOV raw +184 -> MOV destination.w` at `00B46E91/E94/E9A` before resolving underwater fields. |
| `build_and_upload_system_constants_00B46A70`, frame prepare/execute | Route the same frame's raw fog/viewport accessors; no local pointer cache to synchronize. Existing renderer viewport Z0..1 is native-correct despite the owner's separate `18/1C` fields. |

The existing `+43C` context getter already addresses its actual pointer slot;
reference binding can preserve that getter's two reads. The missing setter
`00B6FEC0` simply publishes the borrowed pointer and uses `RET 4`.

## Proposed packets and remaining dependencies

1. **Ready now: camera pool storage.** Own `00B6FED0`, `00B6FFC0`, `00B71120`,
   `00B711E0`, `00B71350`, `00B715F0`, `00B716D0`, `00B71770`, `00B71930`,
   `00CD7DD0`, `00CE0E30`; new `native_camera_pool.hpp/.cpp` plus its doc/report.
   Reuse actual `AllocatorListDomain`, Win32 section and shared CRT allocator;
   preserve every different stride/offset and the post-free/EH continuations.
   This does not depend on camera/frame backing or viewport implementation.
2. **Ready shared type extension:** primary owns the camera descriptor/getter/
   predicate and `00CD7D80/00B719E0/00B71A30`, using existing node/ref bootstrap
   and counter. No new private type counter.
3. **Ready bounded math preparation:** `00B652D0/00B659D0` plane-set construction
   can reuse existing extraction. Full look-at `00B63F10/00B700E0` plus
   `00B6DAE0/00B71400` needs the shared vector/CRT entrypoints and same-camera
   backing; retain explicit dependencies until those interfaces exist.
4. **Camera owner composition follows:** `00B71A80/00B71990/00B71F10/00B71FE0`,
   actual `45Ch` slots, viewport owner, exact staged unwind, fog and real `+438`
   ownership, and the shared adapters above. Existing node base lifetime is
   reusable. Copy constructor `00B71D10`, arbitrary `+438` producer/subclasses,
   and render-command ownership remain separate; none is implicitly complete.

Four shadow cameras additionally require authored names, viewport replacement,
render mode 2 (`00B6FDF0`), clear flags 2 (`00B6FE10`), visibility inputs and
the rest of `00A8E2E0`'s distinct EH map. The original camera's default viewport
is released when the shadow's retained viewport is published. The shadow and
camera then retain the same actual viewport owner.

Render-ready shadows remain blocked by active `00A8F3B0 -> 00A8EA20` and its
`00A8BD20` cascade setup, including `00B6FC30`, look-at math, explicit projection,
borrowed context setter `00B6FEC0`, extra clip-plane add/remove
`00B70410/00B70470` (and `00B65070/00B65990`), and `44h` render-command creation
`00B1F1F0`, queue/scene/tag/job ownership. The large cascade function was only
surveyed for dependencies, not completely reconstructed or validated here.
The complete shadow texture/target/viewport requirements remain listed in
`docs/DIRECTIONAL_SHADOW_OWNER_NEXT.md`.
