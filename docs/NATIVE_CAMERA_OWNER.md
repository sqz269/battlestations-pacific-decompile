# Native camera owner and construction dependencies

The camera now uses its actual `0x45C` pool slot for construction, viewport
assignment, ordinary destruction and conditional pool return. `NativeCameraOwner`
is an external companion: its node, transform, projection, frame and pose views
all address fields in that one slot. It does not add another intrusive count or
replace the original pool identity. Names remain descriptive hypotheses and the
new C++ interfaces are not drop-in binary replacements.

| Entry | Original ABI | Reconstructed behavior |
| --- | --- | --- |
| `00B71A80..00B71CDC` | ECX slot; stack name; EAX slot; RET4 | Construct node, viewport, planes, pose and final camera defaults |
| `00B71990..00B719D1` | ECX camera; stack viewport; RET4 | Identity check, publish new, retain new, release captured old |
| `00B71F10..00B71FD2` | ECX camera; RET | Release viewport, fog and retained `+438`, then destroy node |
| `00B71FE0..00B72000` | ECX camera; stack flags; EAX original address; RET4 | Ordinary destruction, then return same slot only for flags bit zero |
| `00B658E0..00B6598B` | ECX plane set; stack six-plane source and flags; RET8 | Six forward x87 coefficient copies with flags; count untouched |

Endpoints are exclusive. The last helper is now exposed from the existing frame
implementation; its full native extent is 171 bytes.

## Storage and construction

The node prefix occupies `000..173`, the camera tail `174..457`, and the pool ID
`458..45B`. The tail is `0x2E4` bytes. Preparation establishes C++ lifetimes while
preserving all original bytes, binds addresses without reading the uninitialized
node `+A0`, and creates only the external scene association. Abandoning preparation
removes that association and ends typed lifetimes without returning the slot.
Before native construction reaches the camera phase, reentry sees the actual
base-node dispatch, including its type test and scene/world callbacks.

`00B71A80` invokes the existing complete node constructor on this same prefix.
It then installs camera dispatch, allocates the real `0x34` viewport, publishes
`+180`, clears fog `+184`, initializes the plane set, clears retained `+438`, and
then clears borrowed context `+43C`. Viewport initialization retains its own
renderer-global reloads. The camera's later size setup captures the renderer
once and calls its current parameter virtual twice, for height then width.
Viewport setters reload the current `+180` owner at their native call sites.
Minimum and maximum depth retain the `FLDZ/FSTP32` and `FLD1/FSTP32` sequences.

The initial eye/target are `(0,100,0)` and `(0,100,100)` using the actual constant
word. The full pose chain performs the recovered position/world dispatch,
normalization, look-at matrix and inversion before final projection, clear and
axis defaults are written. Constructor-unwritten cache, plane and padding bytes
survive. The same actual `00D7A24C` word is read by viewport, plane setup, pose and
look-at code at their original load sites. The look-at builder's final `MOVSS`
occurs after CRT callbacks; pose captures its up-axis word before calling the
builder. Existing convenience overloads retain their installed constant profile.
Older transform/node numerical helpers still have their separately documented
installed-constant boundary.

The concrete pose adapter reads the actual owner profile and current table slots.
It handles camera `00D62CF0` entries `00B71400/00B71460` and base-node
`00D62C88` entries `00B6DAE0/00B6E870`. Other profiles require their real
implementations and are rejected instead of receiving guessed overrides.

## Cleanup and exception boundaries

Viewport replacement skips identical identities. Otherwise it publishes first,
retains the actual new `+04` count, and releases the captured old viewport.
Ordinary destruction releases current viewport `+180`, fog `+184`, and retained
owner `+438` in order, clearing each field after its callback. Later fields are
loaded after earlier callbacks. Retained `+438` uses the node runtime's actual
owner binding and count; borrowed `+43C` is never released. Node destruction
then removes the scene association and ends the native prefix/tail lifetimes.
The companion must remain alive until this explicit native destruction occurs.

Constructor state one frees an unpublished viewport allocation when its own
constructor fails. State two releases `+438` before node cleanup. The native
camera unwind map does **not** release already published `+180/+184`, and this
implementation preserves that boundary. Ordinary destruction switches its
unwind state before the explicit `+438` release, avoiding a repeated throwing
release. A throwing destructor prevents the subsequent pool return.
The fixture checks host exceptions at renderer calls before and after viewport
publication; original native exception dispatch has not been executed.

## Integrated dependencies and evidence

This batch also integrates the look-at builder and shared vector kernels, camera
type bootstrap, pose dispatch, sixteen-plane initialization, clip-plane updates,
configuration leaves, concrete D3D9 surface pool, and the concrete resource-support
singleton. Their detailed ABI, byte ranges and limitations remain in their
component audits. The surface pool is independent of camera construction and
prepares the next shadow-surface owner work. Resource support uses the same real
singleton lifetime manager; it does not invent a private registry.

`reports/native_camera_owner_audit.json` records current source hashes, independent
PE comparisons, component reruns, Ghidra annotation preimages and export refreshes.
The owner fixture executes complete original camera/node/viewport/pose/plane
code and the real canonical camera pool return, with only allocator/CRT and
controlled renderer boundaries. Across four x87 rounding modes and two live
constant profiles it matches 104 complete 1116-byte camera checkpoints per path,
plus viewport bytes, x87 status/control, MXCSR and 72 ordered boundary events.
Each path allocates/frees sixteen viewports across the eight trajectories.
The focused domain uses an empty name/hierarchy, null fog, and a concrete retained
`+438` terminal owner. It does not establish every hierarchy or failure path.

An independent preparation check verifies abandoned preparation, same-slot
rebinding, unchanged bytes/pool counts and base dispatch during real string-pool
allocation reentry. Current look-at and pose code also pass their existing
96-matrix and 48-phase native comparisons after the live-constant correction.
The strict MSVC Win32 build and both existing CTest checks pass. No tracked test
cases were added.

The installed D3D9 probe still uses a diagnostic camera. Its existing constant,
light-lifetime and visible-mesh checks pass, but this is neither native-camera
render integration nor visual/gameplay validation. Remaining work includes a
stable command/lifetime companion sharing actual camera `+04`, once-only logical
node release through actual `+44` and point-light storage, native batch/context
owners, actual renderer parameter publication, and complete surface/target
ownership. Logical release and final zero-count camera deletion are distinct
operations; a queued reference must keep its companion alive until final deletion.
